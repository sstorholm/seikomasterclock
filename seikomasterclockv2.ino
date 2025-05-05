#include <WiFi.h>
#include "time.h"

const char* ssid       = "ssid";
const char* password   = "password";

const char*     ntpServer = "pool.ntp.org";
const long      gmtOffset_sec = 7200;
const int       daylightOffset_sec = 3600;
int             printPulse = 0;
unsigned long   lastEpochTime = 0;
int             pulses = 0;
int             lastHour = 0;

// Booleans to keep track of last coil polarity, necessary since we need to alternate coil polarity with every pulse
bool        FwdDir = false;
bool        RevDir = false;

//LEDs

int LED = 2;

// Clock Foward Pins
int FwdPin1 = 27; 
int FwdPin2 = 26; 
int enableFwd = 14; 

// Clock Reversing Pins
int RevPin1 = 32; 
int RevPin2 = 33; 
int enableRev = 25;

//Button Structure

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
};

//Initializing Button struct on with pin number, number of presses 0 and default pressed state as false

Button buttonFwdHour = {0, 0, false};
Button buttonFwdHalf = {4, 0, false};
Button buttonRevHour = {5, 0, false};
Button buttonRevHalf = {6, 0, false};

// interupt routine for buttonFwdHour

void IRAM_ATTR isr() {
  buttonFwdHour.numberKeyPresses += 1;
  buttonFwdHour.pressed = true;
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

//Function to pulse clock forward necessary steps

void pulseClockForward (int i)
{
    for ( ; i > 0; i--)
    {
        if (FwdDir == false) {
        digitalWrite(enableFwd, HIGH);
        digitalWrite(FwdPin1, HIGH);
        digitalWrite(FwdPin2, LOW);
        delay(50);
        digitalWrite(enableFwd, LOW);
        FwdDir = !FwdDir;
        digitalWrite(LED, HIGH);
        } else {
        digitalWrite(enableFwd, HIGH);
        digitalWrite(FwdPin1, LOW);
        digitalWrite(FwdPin2, HIGH);
        delay(50);
        digitalWrite(enableFwd, LOW);
        FwdDir = !FwdDir;
        digitalWrite(LED, LOW);
        }
        delay(50);
    }
}

//Function to pulse clock backwards necessary steps

void pulseClockBackward (int i)
{
  Serial.println("Starting Backward");
  for ( ; i < 0; i++)
  {
    Serial.println("In FOR loop");
    if (RevDir == false) {
      digitalWrite(enableRev, HIGH);
      digitalWrite(RevPin1, HIGH);
      digitalWrite(RevPin2, LOW);
      delay(50);
      digitalWrite(enableRev, LOW);
      Serial.println(RevDir);
      RevDir = !RevDir;
    } else {
      digitalWrite(enableRev, HIGH);
      digitalWrite(RevPin1, LOW);
      digitalWrite(RevPin2, HIGH);
      delay(50);
      digitalWrite(enableRev, LOW);
      RevDir = !RevDir;
      Serial.println("Else");
    }
    delay(50);
  }
}

//Returns Epoch Time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

//Computes pulses since last pulsing event

int pulseCalc ()
{
  unsigned long epochNow = getTime();
  int hoursNow = 0;
  int pulses = 0;
  int diff = 0; 
  int remain = 0;
  int dst = 0;

  //Take current epoch time and compare it to the old epoch time
  //Calculate how many 30 second segments has passed and the remainder
  diff = epochNow - lastEpochTime;
  pulses = diff / 30;
  remain = diff % 30;

  pulses = pulses + dst;

  //Remove remainder from previous calculation from the stored epoch time, otherwise clock will drift
  lastEpochTime = epochNow - remain;

  return pulses;
}


void setup()
{
  Serial.begin(115200);

  // sets the pins as outputs:
  pinMode(LED, OUTPUT);
  pinMode(FwdPin1, OUTPUT);
  pinMode(FwdPin2, OUTPUT);
  pinMode(enableFwd, OUTPUT);
  pinMode(RevPin1, OUTPUT);
  pinMode(RevPin2, OUTPUT);
  pinMode(enableRev, OUTPUT);
  pinMode(buttonFwdHour.PIN, INPUT_PULLUP);
  
  // attach intereupt to button1 pin
  
  attachInterrupt(buttonFwdHour.PIN, isr, FALLING);
  
  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");
  
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //Set last known epoch time to something sane so that the clock doesn't go crazy
  lastEpochTime = getTime();
  
  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop()
{

  if (buttonFwdHour.pressed) {
      Serial.println("Stepping forward 1 hour!");
      pulseClockBackward(-20);
      buttonFwdHour.pressed = false;
  }
  if (buttonFwdHalf.pressed) {
      pulseClockForward(1);
      buttonFwdHalf.pressed = false;
  }
  if (buttonRevHour.pressed) {
      pulseClockBackward(-120);
      buttonRevHour.pressed = false;
  }
  if (buttonRevHalf.pressed) {
      pulseClockBackward(-1);
      buttonRevHalf.pressed = false;
  }
  
  delay(1000);

  pulses = pulseCalc(); 

  printLocalTime();
  Serial.println("Current Time:");
  Serial.println(getTime());
  Serial.println("Last Time:");
  Serial.println(lastEpochTime);
  Serial.println("Difference:");
  Serial.println(getTime() - lastEpochTime);
  Serial.println("Pulses:");
  Serial.println(pulses);

  if (pulses > 0)
    {
      pulseClockForward(pulses);
    }

  if (pulses < 0)
    {
      pulseClockBackward(pulses);
    }

  //Detach Interrupt after 1 Minute
  //static uint32_t lastMillis = 0;
  //if (millis() - lastMillis > 60000) {
  //  lastMillis = millis();
  //  detachInterrupt(button1.PIN);
  //Serial.println("Interrupt Detached!");
  //}

}
