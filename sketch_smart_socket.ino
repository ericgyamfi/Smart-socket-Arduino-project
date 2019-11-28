#include<LiquidCrystal.h>

#define relayPin 8
#define buzzerPin 9
#define manualSwitchPin 10

#define TURN_OFF_DELAY 3000 

#define rs 7 
#define en 6
#define d4 5 
#define d5 4
#define d6 3 
#define d7 2

//properly connect buzzer
//properly connect relay

const double EMPTY_PLUG_VOLTS = 2500;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

bool isRelayStateHigh;
double currentVolts;
int prevMillis;
bool thresholdCrossed;
bool i;
bool loopedOnce;

String command;

 /*TERMINAL COMMANDS
  * 
   * DEFUALTS
   * [-o|outlets] = 1
   * [-v|volts] = 300 
   * 
   * #TYPES
   * dev -- straight connection devices (tv, heater, rice-cooker, etc)
   * ext -- extension - (charger|ext-boards|multi-sockets)
   * 
   * #SYNTAX
   * plugin type dev_name -v volts //plugs a new device - max 1
   * plugin-to ext-name dev_name -v volts //plus device into an extention
   * plugout dev_name //plugout device
   * 
   * 
   * ei.commands 
   * 
   * plugin dev heater
   * plugin dev tv -v 500
   * 
   * plugin ext charger
   * plugin ext multi-charger -o 5 
   * plugin ext e1 -o 7
   * 
   * plugin-to charger phone -v 100
   * plugin-to e1 heater -v 1000
   * 
   * plugout phone
   * plugout charger
   * plugout el
   *  
   */

   
void setup() {
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(manualSwitchPin, INPUT);
  // put your setup code here, to run once:

  isRelayStateHigh = false;
  thresholdCrossed = false; 
  currentVolts = 2500;
  command = "";
  i = false;

  //init lcd
  lcd.begin(16,1);
}

void playTone(){
  //digitalWrite(buzzerPin, HIGH);
  tone(buzzerPin, 1960, 100);
}

int getCurrentVolts(){
  int ac = analogRead(A0); 
  return (ac / 1024.0) * 5000;
}

void lcdPrint(String line){
  lcd.setCursor(0,0);
  String space = multiplyStr(" ", (int) ((16 - line.length())/2));
  lcd.print(space + line + space);   
}

String multiplyStr(String str, int j){
  String ret = "";
  for (int i=0;i<j;i++){
    ret += str;
  }
  return ret;
}

void autoTurnOffSocket(){
  //send high to relay
  
  if (getCurrentVolts() != 2500){
    if (isRelayStateHigh){
        digitalWrite(relayPin, LOW);
    }
    else{
        digitalWrite(relayPin, HIGH);
    }
  }
  else{
    if (isRelayStateHigh){
        digitalWrite(relayPin, HIGH);
    }
    else{
        digitalWrite(relayPin, LOW);
    }
  }
  lcdPrint("");
}

void loop() {
  // put your main code here, to run repeatedly:

  //read code from serial and interprete

  if (Serial.available()){
     double volts = Serial.parseInt();
     if (volts){
        if (currentVolts + volts > currentVolts){
          lcdPrint("Device Added");    
          loopedOnce = false;
        }
        else if (volts > 0){
            lcdPrint("Device Removed");
        }
        
        //update new volts reading
        currentVolts += volts;

     } 
     Serial.flush();
  }
  else{
    if (getCurrentVolts() != 2500){
      //lcdPrint("Socket In Use");
    }
  }

  if (!loopedOnce && !thresholdCrossed && currentVolts <= EMPTY_PLUG_VOLTS){
    thresholdCrossed = true;
  }

  if (thresholdCrossed && !i){
    i = true; 
    prevMillis = millis();
    Serial.println("TURNOFF threshold crossed!");
  }
  
  if (i){
    if (millis() - prevMillis > 3000){
      //reset current volts
      thresholdCrossed = false;
      loopedOnce = true;
      autoTurnOffSocket(); 
      currentVolts = 2500;
      playTone(); 
      i = false;
    }
    else{
      //print with serial
//      Serial.print("DEBUG: ");
//      Serial.print(3000 - (millis() - prevMillis));
//      Serial.println(" seconds left to turn off");
      
      if (currentVolts > EMPTY_PLUG_VOLTS){
         i = false;
         thresholdCrossed = false;
      }
      
      lcdPrint("ShutDown: "+ String((int) (3000 - (millis() - prevMillis))) + "s");
     
    }  
  }

  if (loopedOnce && getCurrentVolts() != 2500 && currentVolts == 2500){
     autoTurnOffSocket();
  }
  
}
