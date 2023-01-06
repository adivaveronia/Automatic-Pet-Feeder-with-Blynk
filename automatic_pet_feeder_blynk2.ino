// BLYNK CONFIGURATION
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_DEVICE_NAME ""
#define BLYNK_AUTH_TOKEN ""
#define BLYNK_PRINT Serial

// Libraries
#include <Servo.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <TinyGPSPlus.h>
#include <BlynkSimpleEsp32.h>

// Define sensor pin
const int trigPin = 5;
const int echoPin = 18;

char auth[] = BLYNK_AUTH_TOKEN;

// Access Point Credentials
char ssid[] = "";
char pass[] = "";

// Define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701
// Define GPS pin
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);

long duration;
float distanceCm;
float distanceInch;
float dist_remnant;

BlynkTimer timer;
TinyGPSPlus gps;
unsigned long previousMillis = 0;
const long interval = 1000; 

Servo myservo;
int pos = 0;
int distance = 0;
int count = 0;

BLYNK_WRITE(V0) 
{
  int pinValue = param.asInt();
  // Untuk servo
  if (pinValue == 1) {    // if Button sends 1
    for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      Serial.println("Buka Servo");
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
      Serial.println("Tutup Servo");
      myservo.write(pos);              // tell servo to go to position in variable 'pos'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
    int pinValue = 0;  // Set V3 status to 0 to quit, unless button is still pushed (as per below)
    Blynk.syncVirtual(V3); // ...Then force BLYNK_WRITE(V3) function check of button status to determine if repeating or done.
  }
}

void setup() {
  Serial.begin(9600); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  myservo.attach(13);
  Blynk.begin(auth, ssid, pass);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(2000);
}

void loop() {
  Blynk.run();
  
  // read trigger pin from sensor
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read sensor data
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  dist_remnant = 15 - distanceCm;
  if( distanceCm >= 14){
    Serial.println("Makanan Habis!");
    delay (1000);
  }else {
    Serial.println("Makanan Masih Ada!");
    delay (1000);
  }
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance remnant (cm): ");
  Serial.println(dist_remnant);
  
  Blynk.virtualWrite(V4, dist_remnant);
  
  delay(1000);
  
  // check if GPS is available
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
      }
    }
  }

  //If GPS newData is true
  if(newData == true) {
    newData = false;
    Serial.print("Satellite value: ");
    Serial.println(gps.satellites.value());
    read_gps_data();
  }
  else {
    Serial.println("No Data");
  } 
}
void read_gps_data() {      
  if (gps.location.isValid() == 1)
  {
   //String gps_speed = String(gps.speed.kmph());
    // Read latitude, longitude,and wind speed from GPS
    double latitude = gps.location.lat();
    double longitude = gps.location.lng();
    double wind_speed = gps.speed.kmph();

    // Show data in serial
    Serial.print("Lat: ");
    Serial.println(gps.location.lat(),6);

    Serial.print("Lng: ");
    Serial.println(gps.location.lng(),6);

    Serial.print("Speed: ");
    Serial.println(gps.speed.kmph());

    // Send data to Blynk
    Blynk.virtualWrite(V5, latitude);
    Blynk.virtualWrite(V6, longitude);
    Blynk.virtualWrite(V3, wind_speed);
  }
  else
  {
    Serial.println("Data Unavailable");
  }  

}
