/*
 * Source:
 * Rain FC-37 sensor usage -  https://randomnerdtutorials.com/guide-for-rain-sensor-fc-37-or-yl-83-with-arduino/ 
 * Wiring: 
 * GND=GND
 * Vcc=Vcc
 * NanoA0=RainA0
 * GSM module usage
 * GSM A6 module usage - https://lastminuteengineers.com/a6-gsm-gprs-module-arduino-tutorial/
 * Wiring: 
 * GND=GND
 * Vcc=Vcc
 * NanoD3=GSM8Tx
 * NanoD2=GSM9Rx
 * NanoD2=GSM13PWR
 * GSM External power via uUSB
 */

#include <SoftwareSerial.h>

//Create software serial object to communicate with A6
SoftwareSerial GSMSerial(3, 2); //A6 Tx & Rx is connected to Arduino #3 & #2

// ###########################################################################
// ###################### Global Variables ###################################
// ###########################################################################

int hour;      // value will be replaced by real value from GSM
int minute;    // value will be replaced by real value from GSM
String SMSmessage = "START:";
String sdata = "";  // For reading the SerialData from GSM serial port

// ###########################################################################
// ###################### S E T U P ##########################################
// ###########################################################################

void setup(){
  Serial.begin(9600);  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  delay(1000);
  Serial.println("Arduino OK");
  Serial.println();
  PowerOnGSM();
  Serial.println();
  GetTime();
  Serial.println();
  ParseTime();
  }

// ###########################################################################
// ###################### F U N C T I O N S ##################################
// ###########################################################################

void PowerOnGSM() {         
  // power ON A6 GSM module by sending 2 second High to pin 5 where PWR pin of A6 is attached
  // source: https://lastminuteengineers.com/a6-gsm-gprs-module-arduino-tutorial/

  int GSMPWRPin = 5;
  pinMode(GSMPWRPin, OUTPUT);
  Serial.println("Pwr ON GSM A6...");

  digitalWrite(GSMPWRPin, LOW);
  digitalWrite(GSMPWRPin, HIGH);
  delay(2000);
  digitalWrite(GSMPWRPin, LOW);
  delay(5000);
  
  GSMSerial.begin(9600);     //Start serial with GSM A6
  delay(2000);
  Serial.println("WF GSMSerial...");   // Wait 10 sec and blink
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 1 ; i <= 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH); 
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print("AT attempt # ");
    Serial.println(i);
    GSMSerial.println("AT");        //Once the handshake test is successful, it will return OK
    updateSerial();
    }  
  }

// ################################# 

void GetTime() {                    //outcome: +CCLK: "20/07/31,14:26:01+02"
  delay(500);
  Serial.println("Get time"); 
  GSMSerial.println("AT+CCLK?"); 

  GSMSerialRead();

  PowerOffGSM();
}

// ################################# 

void GSMSerialRead() {
  // source: https://www.best-microcontroller-projects.com/arduino-string.html

  delay(500);
  byte ch;
   while(GSMSerial.available()) 
      {
        ch = GSMSerial.read();
        sdata += (char)ch;
      }
  Serial.println("SDATA Check:");     // should return like: +CCLK: "20/07/31,14:26:01+02" followed by OK
  Serial.println(sdata);
}

// ################################# 

void ParseTime() {
 //source: https://www.arduino.cc/en/Tutorial/StringRemove
 // set to integer: https://www.arduino.cc/en/Tutorial.StringToIntExample
  
/* +CCLK: "20/08/01,20:48:58+02"
 * parse hour:minute
 */

  String sHour, sMinute;

  Serial.println("Parse time:");
  Serial.println(sdata);

  sdata.remove(0,20);                   // Crop unwanted info from GSM Time
  sdata.remove(20);
  Serial.println(sdata);

  SMSmessage += sdata;                  // add the timestamp to SMS message
  SMSmessage += " --RainReport: ";      // and add Report to SMS message
  Serial.println(SMSmessage);

  sdata.remove(0,9);                    // Crop further to get hour and minute
  sdata.remove(5);

  Serial.print("SDATA POST-CUT ");
  Serial.println(sdata);

  sHour = sdata;
  sMinute = sdata;
  
  sHour.remove(2);
  sMinute.remove(0,3);
  hour = (sHour.toInt());
  minute = (sMinute.toInt());

  Serial.print(hour," ");               // display the acquired hour/min info
  Serial.println(minute);
  }

// #################################

void AlignDelay(){                // we need to start checking on half or whole hour
  long RCDelay;                   //must be long due miliseconds used
  
  if (minute>0 && minute<30){     //we will wait till next halfhour
      RCDelay = 30 - minute;
      Serial.print("RCDelay min <30: ");
      Serial.println(RCDelay);
      RCDelay = RCDelay*60000;  //*60000 from miliseconds to minutes - SET TO PRODUCTION
      delay(RCDelay);
      minute=30;
      }
  
  if (minute>30){     //we will wait till next hour
    RCDelay = 60 - minute;
    Serial.print("RCDelay min >30: ");
    Serial.println(RCDelay);
    RCDelay = RCDelay*60000;    //*60000 from miliseconds to minutes - SET TO PRODUCTION
    delay(RCDelay);
    hour++;
    if (hour == 24) hour = 0;
    minute=0;
    }
  Serial.print ("Check hour: ");
  Serial.println (hour);
  }

//###########################

void CheckRain() {
  int rainPin = A0;
  int thresholdValue = 500;   // you can adjust the rain sensing threshold value
  
  int sensorValue = analogRead(rainPin);    // read the input on analog pin 0:
  // Serial.print(sensorValue," ");
  if(sensorValue < thresholdValue) SMSmessage += "Y";   // if there's rain detected, add "Y" to SMS message (after timestamp)
    else SMSmessage += "n";                             // otherwise put "n" there
  }

// ################################# 

void SendSMS(){
/* 
 * SMS format 160 characters
 *          10        20        30        40        50        60        70        80        90        100       110       120       130       140       150
 * 1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
 * START:20/08/01,20:30:39+02 --RainReport: 13YN14NN15NN16NN17NN18NN19NN20NN21NN22NN23NN00NN01NN02NN03NN04NN05NN06NN07NY08YY09YY10YN11NN12NY --RainReportEND
 */

  PowerOnGSM();

  GSMSerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  GSMSerial.println("AT+CMGS=\"+421987123456\"");         //change with phone number to sms in international form - SET TO PRODUCTION
  updateSerial();
  // GSMSerial.print("Test message"); //text content
  GSMSerial.print(SMSmessage); 
  updateSerial();
  GSMSerial.write(26);           //close and send code

  delay(10000);
  PowerOffGSM();                 // After successful send, we will power off the GSM module
  }

// #################################

void PowerOffGSM() {         // power Off A6 GSM module by AT command
  GSMSerial.println("AT+CPOF");
  updateSerial();
  }

//###########################
  
void updateSerial(){
  delay(500);
  while (Serial.available()) 
  {
    GSMSerial.write(Serial.read());   //Forward what Serial received to Software Serial Port
  }
  while(GSMSerial.available()) 
  {
    Serial.write(GSMSerial.read());   //Forward what Software Serial received to Serial Port
  }
}

//###########################

void(* resetFunc) (void) = 0;    //declare reset function @ address 0 => arduino SW reset

//#### FAKE #################

void FakeCheckRain(){           // we generate "random" measure
  int isRain;
  isRain = random(10);
  if (isRain>=5) SMSmessage += "Y";
    else SMSmessage += "n";
  }

//#### FAKE #################

void FakeSendSMS(){
  // real data replaced by blankSMS and random time - SET TO PRODUCTION
  hour = random(23);
  minute = random(60);
  SMSmessage = "START: YY/MM/DD,HH:SS+02 --Rain report: ";
  PowerOffGSM();
  }

// ###########################################################################
// ###################### L O O P ############################################
// ###########################################################################

void loop() {
  delay(1000);
  AlignDelay();           // Waiting for exact half or exact whole hour

  SMSmessage += hour;     // adding hour timestamp to SMS message

  if (minute==0){
    // FakeCheckRain();                   // - REMOVE FROM PRODUCTION
    CheckRain();                                // - SET TO PRODUCTION
    // Serial.println("Min=0, wait 30 min to next check.");
    delay (1800000);       // 30 minute wait - SET TO PRODUCTION
    minute=30;
    }

  // Serial.println(SMSmessage);    //test print
  
  if (minute==30){
    // FakeCheckRain();                   // - REMOVE FROM PRODUCTION
    CheckRain();                                // - SET TO PRODUCTION
    // Serial.println("Min=30, wait 30 min to next check.");
    delay (1800000);       // 30 minute wait - SET TO PRODUCTION
    minute=0;
    }

  Serial.println(SMSmessage);    //test print

  hour++;
  if (hour == 24) hour = 0;  // midnight

  if (hour == 12 && minute == 0) {          // time to send SMS, amend if required - SET TO PRODUCTION
    SMSmessage += " --RainReportEND";
    Serial.println("Sending SMS");
    Serial.println(SMSmessage);
    SendSMS();                                 // - SET TO PRODUCTION
    // FakeSendSMS();                                // - REMOVE FROM PRODUCTION

    Serial.println("############");
    delay(10000);
    resetFunc();        // - SET TO PRODUCTION
    }
  //Serial.println("I'm alive!");
  }
