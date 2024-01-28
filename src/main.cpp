#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <LiquidCrystal_I2C.h>

// Define a custom service UUID and characteristics UUIDs
#define SERVICE_UUID "734b5896-7faf-457a-bb83-15c0e3bc2453"
#define WRITE_CHARACTERISTIC_UUID "36ac8b55-5dc7-4686-8dd2-99267a3341d8"
#define READ_CHARACTERISTIC_UUID "ce527533-3c33-4346-8cbf-83e079805460"

#define ESP32_SDA 21
#define ESP32_SCL 22

// Create a BLE server
BLEServer *pServer = NULL;

//Create Characteristics
BLECharacteristic *pWriteCharacteristic = NULL;
BLECharacteristic *pReadCharacteristic = NULL;

// Create the BLE Service
 BLEService *pService;

 // Define pins for LCD Display
LiquidCrystal_I2C  lcd(0x27,16,2);

// Callbacks for server events
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
      //print connected msg on LCD display
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Connected!!");
  }

  void onDisconnect(BLEServer* pServer) {


    }
};


//Callback for Characteristic writing
class MyCharacteristicCb: public BLECharacteristicCallbacks{
    void onWrite(BLECharacteristic* pCharacteristic)
    {
      //clear display
      lcd.clear();
      lcd.setCursor(0,0);

      if(pCharacteristic->getValue().length() > 16)
      {
          //create an auxiliar buffer to print the splitted msg
          char aux[16];
          memset(aux,0,16);

          //copy the first part of msg and print it.
          memcpy(aux,pCharacteristic->getValue().c_str(),16);
          lcd.print(aux);

          //take the last part of msg
          memset(aux,0,16);
          memcpy(aux,pCharacteristic->getValue().c_str()+16,(pCharacteristic->getValue().length() - 16));

          //set the cursor to second line
          lcd.setCursor(0,1);

          //print msg
          lcd.print(aux);

      }
      else
      {
          //in this case, we're safe with the msg length and just print it
          lcd.print(pCharacteristic->getValue().c_str());
      }

      //notify data
      pReadCharacteristic->setValue(pCharacteristic->getValue().c_str());
      pReadCharacteristic->notify();

    }
};


void setup() {

  // Create the BLE Device
  BLEDevice::init("RN_BLE_IOT");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pWriteCharacteristic = pService->createCharacteristic(
                      WRITE_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  //add characteristic callback
  pWriteCharacteristic->setCallbacks(new MyCharacteristicCb());

  // Create a BLE Characteristic
  pReadCharacteristic = pService->createCharacteristic(
                      READ_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Add a descriptor for the characteristic
  pReadCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  //Advertising
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->start();

  //LCD

  //start I2C bus line
  Wire.begin(ESP32_SDA, ESP32_SCL);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,0);

  lcd.print("Hello !!");

  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("Connection");


}

void loop() {
  // put your main code here, to run repeatedly:
}

