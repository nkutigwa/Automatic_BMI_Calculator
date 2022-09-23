#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>.

#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
HX711_ADC LoadCell(18, 5);
LiquidCrystal_I2C lcd(0x27,16,2); 
#define mySerial Serial2
int count = 0;
String msg;
String s,s1;
char call;
char text;
String Age = "", Age_total = "", Gender = "", BMI_st = "";
int x, y, max_Age = 1;
boolean Age_state = true, Gender_state = true, weight_state = true, height_state = true;
String mob = "+255757650442";
volatile boolean newDataReady;
long t = 0 ,t1 =0, t2 =0, t3 = 0, t4 = 0;
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {33, 25, 26, 27}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 13, 2, 4}; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
const int trigPin = 23;
const int echoPin = 19;
double duration, distance, Height, Distance_total = 185, wgt = 0, wgt_rec, BMI = 0; //198.12
void setup(){
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  mySerial.begin(9600);
  lcd.init();       
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("  ***VPIMS***");
   gsm_init();
   Serial2.println("AT");
   Serial2.println("AT+CMGF=1"); 
   Serial2.println("AT+CNMI=2,2,0,0,0"); 
     LoadCell.begin();
     float calibrationValue; 
     calibrationValue = 24.4;
     unsigned long stabilizingtime = 2000; 
     boolean _tare = true; 
     LoadCell.start(stabilizingtime, _tare);
     if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
     }
  else {
    LoadCell.setCalFactor(calibrationValue); 
    Serial.println("Startup is complete");
   }
    attachInterrupt(digitalPinToInterrupt(18), dataReadyISR, FALLING);
   }
   
  void dataReadyISR() {
  if (LoadCell.update()) {
    newDataReady = 1;
  }
}
void loop(){
  if (Age_state == true){ Age_Set();}
  if (Age_state == false && Gender_state == true){ Gender_Set();}
  if (Age_state == false && Gender_state == false){Height_Weight();}
}
 void Height_Weight(){
        while(1){
          if (weight_state == true && height_state == true && Age_state == false && Gender_state == false){
          Weight();
          }
          if (weight_state == false && height_state == true && Age_state == false && Gender_state == false){
            Height_measure();
          }
          if (weight_state == false && height_state == false && Age_state == false && Gender_state == false){
              BMI = 0.001*wgt_rec / (0.0001* Height* Height);
              if (BMI < 18.5){ BMI_st = "underweight"; }
              else if (BMI >= 18.5 && BMI <= 24.9 ){ BMI_st = "normalweight"; }
              else if (BMI >= 25.0 && BMI <= 29.9 ){ BMI_st = "overweight"; }
              else if(BMI > 30.0){ BMI_st = "obesity";}
              if(millis() - t1 > 500){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("H:"+String(Height*0.0328084)+"ft" + " W:"+String(0.001*wgt_rec) + "KG");
              lcd.setCursor(0,1);
              lcd.print(BMI_st);
              t1 = millis();
              }
            }
 
          
           if (millis() - t3 >= 5000){
                SendMessage();
                 height_state = true;
                 weight_state = true; 
                 Gender_state = true;
                 Age_state = true; 
                 BMI_st = "";
                 lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("      Done");
                delay(1000);
                 break;
           }
          }
         }
  void Height_measure (){
       while(1){
         char customKey = customKeypad.getKey();
        if (customKey){
        if (customKey == 'D'){ height_state = false; t3 = millis(); break;}
        }
     digitalWrite(trigPin, LOW);
     delayMicroseconds(10);
     digitalWrite(trigPin, HIGH);
     delayMicroseconds(10);
     digitalWrite(trigPin, LOW);
     duration = pulseIn(echoPin, HIGH);
     distance = duration * 0.036/2;
     Height = Distance_total - distance; 
     if (millis() - t2 >= 300){
     lcd.clear();
     lcd.setCursor(0,0);
     lcd.print("Measure height");
     lcd.setCursor(0,1);
     lcd.print("Height: "+String(Height*0.0328084)+"ft");
     Serial.println(Height);
     t2 = millis();
     }
    } 
  }      
  void Weight(){
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Step on scale");
      long w_t = 0;
      wgt = 0;
      wgt_rec = 0;
      while(1){
        char customKey = customKeypad.getKey();
        if (customKey){
        if (customKey == 'D'){ wgt_rec = wgt; weight_state = false; t3 = millis(); break;}
        }
         const int serialPrintInterval = 0;
        if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        wgt = (1 * i);
        if ( wgt <= 0){ wgt = 0;}
        if (millis() - w_t >= 100){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Step on scale");
            lcd.setCursor(0,1);
            lcd.print("Weight: "+ String(0.001*wgt)+"kg");
            w_t = millis();
          }
          
        newDataReady = 0;
        
         t = millis();
        }
        }
          if (Serial.available() > 0) {
          char inByte = Serial.read();
         if (inByte == 't') LoadCell.tareNoDelay();
       }
       
       if (LoadCell.getTareStatus() == true) {
       Serial.println("Tare complete");
       }
     }
  }

  void Gender_Set(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Choose gender:");
    lcd.setCursor(0,1);
    lcd.print("1.male  2.female");
    while(1){
     char customKey = customKeypad.getKey();
     if (customKey){
      if (customKey == '1'){ Gender = "male"; Gender_state = false; break;}
      else if (customKey == '2'){ Gender = "female"; Gender_state = false; break;}
    }
    }
  }
  
  void gsm_init() {
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  Serial.print("Finding Module..");
  boolean at_flag = 1;
  while (at_flag) {
    mySerial.println("AT");
    while (mySerial.available() > 0) {
      if (mySerial.find("OK"))
        at_flag = 0;
    }

    delay(1000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  ***VPIMS***");
  lcd.setCursor(0, 1);
  lcd.print("Checking Service");
  Serial.println("Module Connected..");
  delay(1000);
  
  boolean echo_flag = 1;
  while (echo_flag) {
    mySerial.println("ATE1");
    while (mySerial.available() > 0) {
      if (mySerial.find("OK"))
        echo_flag = 0;
    }
    delay(1000);
  }
  
  delay(1000);
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("  ***VPIMS***");
  lcd.setCursor(0, 1);
  lcd.print("Finding Network..");
  Serial.println("Finding Network..");

  boolean net_flag = 1;
  while (net_flag) {
    mySerial.println("AT+CPIN?");
    while (mySerial.available() > 0) {
      if (mySerial.find("+CPIN: READY"))
        net_flag = 0;
    }
    delay(1000);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  ***VPIMS***");
  lcd.setCursor(0, 1);
  lcd.print("Network found..");
  Serial.println("network found..");
  delay(1000);
}

void Age_Set(){
      Age =  "\0";
      Age_total = "\0";
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter Age(5-19)");
       y = 1;
       x = 0;
      lcd.blink();
      lcd.setCursor(x, y);
      while(1){
         char custom = customKeypad.getKey();
    if (custom) {
        if (custom == '1') {
        x++;
        Age = "1";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("1");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '2') {

        x++;

        Age = "2";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("2");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '3') {

        x++;

        Age = "3";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("3");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '4') {

        x++;

        Age = "4";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("4");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '5') {

        x++;

        Age = "5";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("5");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '6') {

        x++;

        Age = "6";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("6");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '7') {

        x++;

        Age = "7";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("7");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '8') {

        x++;

        Age = "8";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("8");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '9') {
        x++;
        Age = "9";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("9");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      } else if (custom == '0') {
        x++;
        Age = "0";
        if (Age_total.length() <= max_Age) {
          Age_total = Age_total + Age;
          lcd.blink();
          lcd.setCursor(x - 1, y);
          lcd.print("0");
        } else if (Age_total.length() > max_Age) {

          lcd.blink();
          x = Age_total.length();
          lcd.setCursor(x, y);
          lcd.print("");
          Serial.println("maximum size");
        }
      }  else if (custom == 'A') {
       
        lcd.noBlink();
        x--;
        Age_total.remove(Age_total.length() - 1);
        if (Age_total.length() == 0) {
          x = 0;
          lcd.setCursor(x, y);
          lcd.print(" ");
          lcd.setCursor(x - 1, y);
          lcd.blink();
          lcd.setCursor(x, y);
        } else if (Age_total.length() > 0) {
          lcd.setCursor(x, y);
          lcd.print(" ");
          lcd.setCursor(x - 1, y);
          lcd.blink();
          lcd.setCursor(x, y);
        }
      } else if (custom == 'B') {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Exiting...");
        Age_total = "/0";
        Age = "/0";
        delay(1000);
         break;
       
      } else if (custom == 'D') {
          if( Age_total.toInt() < 5 || Age_total.toInt() > 19){
               lcd.clear();
               lcd.noBlink();
               lcd.setCursor(0,0);
               lcd.print("Please enter");
               lcd.setCursor(0,1);
               lcd.print("the correct Age");
               Age =  "\0";
               Age_total = "\0";
               delay(2000);
               lcd.clear();
               lcd.setCursor(0,0);
               lcd.print("Enter Age(5-19)");
                y = 1;
                x = 0;
                lcd.blink();
                lcd.setCursor(x, y);
              }
           else {
         lcd.clear();
         lcd.setCursor(0,0);
         lcd.noBlink();
         Age_state = false;
         Serial.println(Age_total.toInt());
         break;
           }
    }
   }
  }
}
 
     void SendMessage()
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Sending SMS...");
  Serial2.println("AT+CMGF=1"); 
  Serial.println("AT+CMGF=1");
  delay(1000); 
  Serial2.print("AT+CMGS="); Serial2.print("\"") ;Serial2.print(mob);Serial2.print("\"");Serial2.println("\r");
  Serial.print("AT+CMGS=");  Serial.print("\"") ; Serial.print(mob); Serial.print("\""); Serial.println("\r");
  delay(1000);
 Serial2.print("VPIMS "+String(Age_total)); Serial2.print(" ");
 if (Gender.equals("male")){Gender = "M"; }
     else{ Gender = "F";}
 Serial2.print(Gender); Serial2.print(" ");
 Serial2.print(String(Height*0.0328084)); Serial2.print(" ");
 Serial2.print(String(0.001*wgt_rec)); Serial2.print(" ");
 Serial2.println(BMI);
 Serial2.println(BMI_st);

  delay(100);
  Serial2.println((char)26);// ASCII code of CTRL+Z
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   Done");
  delay(1000);
  
  }
     
