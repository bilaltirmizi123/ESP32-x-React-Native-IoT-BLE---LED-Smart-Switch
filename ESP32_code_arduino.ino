/*
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  updated by chegewara and MoThunderz
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Initialize all pointers
BLEServer* pServer = NULL;                        // Pointer to the server
BLECharacteristic* pCharacteristic_1 = NULL;      // Pointer to Characteristic 1
BLECharacteristic* pCharacteristic_2 = NULL;      // Pointer to Characteristic 2
BLECharacteristic* pCharacteristic_3 = NULL;      // Pointer to Characteristic 3

BLE2902 *pBLE2902_1;                              // Pointer to BLE2902 of Characteristic 1
BLE2902 *pBLE2902_2;                              // Pointer to BLE2902 of Characteristic 2
BLE2902 *pBLE2902_3;                              // Pointer to BLE2902 of Characteristic 3

// Some variables to keep track on device connected
bool deviceConnected = false;
bool oldDeviceConnected = false;

//Defining pins for output of LEDs respectively
int RED_LED_PIN = 15;
int GREEN_LED_PIN = 2;
int YELLOW_LED_PIN = 4;

int RED_LED_ACTIVE = 1;
int GREEN_LED_ACTIVE = 1;
int YELLOW_LED_ACTIVE = 1;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
// UUIDs used in this example:
#define SERVICE_UUID          "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_1 "beb5483e-36e1-4688-b7f5-ea07361b26a8"  // CHARACTERISTIC UUID FOR RED LED
#define CHARACTERISTIC_UUID_2 "1c95d5e3-d8f7-413a-bf3d-7a2e5d7be87e"  // CHARACTERISTIC UUID FOR GREEN LED
#define CHARACTERISTIC_UUID_3 "5b316b93-f7ec-4faf-84b7-35522cdeadfb"  // CHARACTERISTIC UUID FOR YELLOW LED


// Callback function that is called whenever a client is connected or disconnected
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  

  // Create the BLE Device
  BLEDevice::init("ESP32 Microcontroller");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic_1 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_1,
                      BLECharacteristic::PROPERTY_WRITE
                    );                   

  pCharacteristic_2 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_2,
                      BLECharacteristic::PROPERTY_WRITE                    
                    );  

  pCharacteristic_3 = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_3,
                      BLECharacteristic::PROPERTY_WRITE                    
                    ); 

  // Create a BLE Descriptor  
  pBLE2902_1 = new BLE2902();
  pBLE2902_1->setNotifications(true);                 
  pCharacteristic_1->addDescriptor(pBLE2902_1);

  pBLE2902_2 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_2->addDescriptor(pBLE2902_2);

  pBLE2902_3 = new BLE2902();
  pBLE2902_2->setNotifications(true);
  pCharacteristic_3->addDescriptor(pBLE2902_3);

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");


}

int last_rxValue1 = 0;
int last_rxValue2 = 0;
int last_rxValue3 = 0;

void loop() {

    if (deviceConnected) {
    
      std::string rxValue1 = pCharacteristic_1->getValue();
      std::string rxValue2 = pCharacteristic_2->getValue();
      std::string rxValue3 = pCharacteristic_3->getValue();

      if(rxValue1.length() > 0 && last_rxValue1 != atoi(rxValue1.c_str())){

         Serial.print("Characteristic - RED LED : ");
         Serial.println(rxValue1.c_str());

         if(atoi(rxValue1.c_str()) == RED_LED_ACTIVE){
              digitalWrite(RED_LED_PIN, HIGH);
         } else {
              digitalWrite(RED_LED_PIN, LOW);
         }

         last_rxValue1 = atoi(rxValue1.c_str());
      }

        if(rxValue2.length() > 0 && last_rxValue2 != atoi(rxValue2.c_str())){

         Serial.print("Characteristic - GREEN LED : ");
         Serial.println(rxValue2.c_str());

         if(atoi(rxValue2.c_str()) == GREEN_LED_ACTIVE){
              digitalWrite(GREEN_LED_PIN, HIGH);
         } else {
              digitalWrite(GREEN_LED_PIN, LOW);
         }

             last_rxValue2 = atoi(rxValue2.c_str());
      }


        if(rxValue3.length() > 0 && last_rxValue3 != atoi(rxValue3.c_str())){

         Serial.print("Characteristic - YELLOW LED : ");
         Serial.println(rxValue3.c_str());

         if(atoi(rxValue3.c_str()) == YELLOW_LED_ACTIVE){
              digitalWrite(YELLOW_LED_PIN, HIGH);
         } else {
              digitalWrite(YELLOW_LED_PIN, LOW);
         }

              last_rxValue3 = atoi(rxValue3.c_str());
      }


      // In this example "delay" is used to delay with one second. This is of course a very basic 
      // implementation to keep things simple. I recommend to use millis() for any production code
      delay(1000);
    }
    // The code below keeps the connection status uptodate:
    // Disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // Connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}