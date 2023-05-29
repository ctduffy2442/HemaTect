
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
float hematomaWarn = 0; 

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CLK 3
#define DT 4
#define buzzerPin 5
#define ledPin 9

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir = "";

float tickLength;
float netCircChange;
bool timerOn = false;
unsigned long startTime;
unsigned long elapsedTime;


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

  // Create the BLE Device
  BLEDevice::init("BLE_TEST"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ | 
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

  tickLength = (PI / 24) * 2.433;  // pi/24 times spool diameter - via PR2
netCircChange = 0;
elapsedTime = 0;

pinMode(CLK, INPUT);
pinMode(DT, INPUT);
pinMode(buzzerPin, OUTPUT);
pinMode(ledPin, OUTPUT);

ledcAttachPin(6, 0);
ledcSetup(0, 1000, 8);
lastStateCLK = digitalRead(CLK);
}

void loop() {
  if (deviceConnected) {
    txValue = hematomaWarn; // This could be an actual sensor reading!
    char txString[8]; // make sure this is big enuffz
    dtostrf(txValue, 1, 2, txString); // float_val, min_width, digits_after_decimal, char_buffer
    pCharacteristic->setValue(txString);
    pCharacteristic->notify(); // Send the value to the app!
    Serial.print("*** Sent Value: ");
    Serial.print(txString);
    Serial.println(" ***");
    currentStateCLK = digitalRead(CLK);
    if (currentStateCLK != lastStateCLK && currentStateCLK == 1) {
      if (digitalRead(DT) != currentStateCLK) {
        netCircChange -= tickLength;  //decrease in circumference
        currentDir = "CCW";
      } else {
        netCircChange += tickLength;  //increase in circumference
        currentDir = "CW";
      }
    }

    Serial.print(" | Start Time: ");
    Serial.print(startTime);
    Serial.print(" | Direction: ");
    Serial.print(currentDir);
    Serial.print(" | Total Change: ");
    Serial.println(netCircChange);

    if (netCircChange >= 2) {
      if (elapsedTime >= 6000) {
        digitalWrite(ledPin, HIGH);
        tone(buzzerPin, 500);
        noTone(buzzerPin);
        delay(100);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(100);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(350);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(100);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(360);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(100);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(100);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(350);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        delay(100);
        tone(buzzerPin, 500);
        delay(150);
        noTone(buzzerPin);
        elapsedTime = 0;
        hematomaWarn = true; 

      } else if (!timerOn) {
        startTime = millis();
        timerOn = true;
      }

      elapsedTime = millis() - startTime;
      Serial.print("| Timer: ");
      Serial.print(elapsedTime);
    } else {
      timerOn = false;
      startTime = 0;
      noTone(buzzerPin); 
    }

    if (netCircChange < 2) {
      timerOn = false;
      startTime = 0;
      digitalWrite(ledPin, LOW);
      noTone(buzzerPin);
      elapsedTime = 0;
    }

    delay(1);
    lastStateCLK = currentStateCLK;
}
}
