#include "house.h"
#include "SPI.h"
#include "const.h"
#include "MFRC522.h"

String RFID_new_id = "";
#define pinRST 9 // RFID reader
#define pinSDA 10 // RFID reader
#define speakerPin 6 
#define pinPHOTO A1
#define pinTEMP A0

// We'll also set up some global variables for the light level:

int lightLevel, high = 0, low = 1023;
MFRC522 mfrc522(pinSDA, pinRST);

  // melody 1
int len1 = 5; // the number of notes
char notes1[] = "cegC "; // a space represents a rest
int beats1[] = { 1,1,1,2 };
int tempo1 = 100;

// melody 2
int len2 = 3; // the number of notes
char notes2[] = "Cc "; // a space represents a rest
int beats2[] = { 1,1 };
int tempo2 = 100;

int kitchen_intensity = -1;

void setup() {
  SPI.begin();
  Serial.begin(9600);
  pinMode(LAMP1,OUTPUT);
  pinMode(LAMP2,OUTPUT);
  pinMode(LAMP3,OUTPUT);
  mfrc522.PCD_Init();
  pinMode(speakerPin, OUTPUT);

}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if(mfrc522.PICC_ReadCardSerial()) {

      //**************PUT RFID_ID INTO VARIABLE****************
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        RFID_new_id += String(mfrc522.uid.uidByte[i], HEX);
      }
      RFID_new_id.toUpperCase();
      
      send_msg_id(RFID_new_id,1);
      mfrc522.PICC_HaltA();
      RFID_new_id = "";
      
      
    }
  }
  
  String msg = listen_msg();
  int temp_intensity = -1;
    if(msg.length()>0){
    temp_intensity = execute_msg(msg);
    msg = "";
    }

    if(temp_intensity != -1){
      kitchen_intensity = temp_intensity;  
    }
    if (kitchen_intensity == 0){
      digitalWrite(LAMP1,0);
    }
    else{
    lightLevel = analogRead(pinPHOTO);
    tuning();
    if(kitchen_intensity == -1){
      analogWrite(LAMP1, 0);
      }
    else{
      analogWrite(LAMP1, 200-lightLevel);
      }
    }
  
}

/////////// SEND ID MESSAGES /////////////
void send_msg_id(String RFID_id, int room){
  String msg = "";
  msg += '_';
  int len = 5+RFID_id.length(); // 5: 2 length, 1 acknowledge, 1 msg_type, 1 room
  if (len > 9){
    msg += len;
    }
  else{
    msg += '0';
    msg += len;  
    }
  msg += '3'; //msg_type
  msg += room; //room
  msg += RFID_id; //msg
  msg += '0'; //acknowledge
  Serial.println(msg);
}

//////////// SEND MESSAGES MUSIC AND RADIATOR ////////////
void send_msg(int room, String input, int msg_type){
  String msg = "";
  msg += '_';
  int len = 5+input.length();
  if (len > 9){
    msg += len;
    }
  else{
    msg += '0';
    msg += len;  
    }
  msg += msg_type; //msg_type
  msg += room;
  msg += input; //msg
  msg += '0'; //acknowledge
  Serial.println(msg);
}

////////////// LISTEN FOR MESSAGE ///////////
String listen_msg(){
  String msg = "";
  int start_reading = 0;
  int L_one;
  int L_two;
  int len;
   while(Serial.available()){
    delay(3);
    if(Serial.available() > 0){
      if(Serial.read() == '_'){
        start_reading = 1;
        L_one = Serial.read() - '0';
        delay(3);
        L_two = Serial.read() - '0';
        len = 10*L_one + L_two;
        delay(3);
        msg += String(L_one);
        delay(3);
        msg += String(L_two);
      }
      while(start_reading == 1){
        char c = Serial.read();
        msg += c;
        delay(3);
        if(msg.length() == len || Serial.available() == 0){
          delay(3);
          start_reading = 0;
        } 
      }
    }
    delay(10);
  }
  delay(100);
  return msg;
  };
////////////// DECODE MESSAGE //////////////

int execute_msg(String msg){
  int mode = msg.substring(2,3).toInt();
  int room = msg.substring(3,4).toInt();
  int temp_intensity = -1;
  
  int num_pref = (msg.length()-5)/2; // Subtracts position reserved for length(2), room(1), msg_type(1), acknowledge(1)
  if (mode == 0){
   playMusic1(notes1,beats1,len1,tempo1);
   }
  if (mode == 1){
   playMusic2(notes2,beats2,len2,tempo2);
   }
  
  for (int i = 0; i < num_pref; i++){
    int pref_value;
    int pref_type = msg.substring(4+2*i,5+2*i).toInt();
    String temp = msg.substring(5+2*i,6+2*i);
    
    if (!strcmp(temp.c_str(),"a")){
      pref_value = 10*10;
      }
      else{
      pref_value = msg.substring(5+2*i,6+2*i).toInt()*10;    
      }
      if(room == kitchen && pref_type == lightning){
         temp_intensity = pref_value;
        }
    switch (mode) {
      case 0: // login
        adjust_room(room, pref_type, pref_value);
        break;
      case 1: // logout
        //pref_value = 0;
        adjust_room(room, pref_type, pref_value);
        break;
      case 2: // mode change
        // statements
        break;
      case 3: // acknowledge
        // statements
        break;
      default:
        break;
        // statements
      }
    }
    return temp_intensity;
  };


/////////////// ADJUST ROOM ///////////////
void adjust_room(int room, int pref_type, int pref_value){
    switch (pref_type){
    case lightning:
        set_lamps(room,pref_value);
      break; 
     case volume:
      set_music(room, pref_value);
      break;
     case heating:
      set_radiator(room, pref_value);
      break; 
    }
};

/////////////// COMMANDS ///////////////
void set_lamps(int room,int intensity){ 
  if(room == living_room){
    if (0 < intensity && intensity <= 50){
      digitalWrite(LAMP2,1);
      digitalWrite(LAMP3,0);
    }
    else if (100 >= intensity && intensity > 50){
      digitalWrite(LAMP2,1);
      digitalWrite(LAMP3,1);    
    }
    else if (intensity == 0){
      digitalWrite(LAMP2,0);
      digitalWrite(LAMP3,0);
      }
    else{
      Serial.println("Intensity out of bound");      
    }
  }
};

void set_music(int room, int pref_value){
  String msg = "";
  int msg_type = 5;
  msg += music;
  if(pref_value == 100){
    msg += "a";
    }
  else{
    msg += pref_value/10;
    }
  send_msg(room, msg, msg_type);
};

void set_radiator(int room, int pref_value){
  if(room == kitchen){
  float voltage, degreesC;
  voltage = get_voltage(pinTEMP);
  degreesC = (voltage - 0.5) * 100.0;
  degreesC = round(degreesC);

  String msg = "";
  int msg_type = 5;
  msg += radiator;
  if(pref_value == 100){
    msg += "a";
    }
    else{
      msg += pref_value/10;
      }
  send_msg(room, msg, msg_type);
  }
  
    
};

  ///////////// PLAY MUSIC LOGIN/LOGOUT ///////////////
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
};

void playNote1(char note, int duration) {
  char names[] = { 'c', 'e', 'g', 'C' };
  int tones[] = { 1915, 1519, 1275, 956 };
  for (int i = 0; i < 4; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
};

void playMusic1(char notes[],int beats[], int len, int tempo){
    for (int i = 0; i < len; i++) {
      if (notes[i] == ' ') {
        digitalWrite(speakerPin, LOW);
      } else {
        playNote1(notes[i], beats[i]*tempo);
      }
    delay(tempo / 2); 
  }
};

void playNote2(char note, int duration) {
  char names[] = { 'C', 'c' };
  int tones[] = { 956, 1915 };
  for (int i = 0; i < 2; i++) {
    if (names[i] == note) {
      playTone(tones[i], duration);
    }
  }
};

void playMusic2(char notes[],int beats[], int len, int tempo){
    for (int i = 0; i < len; i++) {
      if (notes[i] == ' ') {
        digitalWrite(speakerPin, LOW);
      } else {
        playNote2(notes[i], beats[i]*tempo);
      }
    delay(tempo / 2); 
  }
};

//////////////// TUNE PHOTORESISTOR ////////////

void tuning(){
  lightLevel = map(lightLevel, 300, 800, 0, 200);
  lightLevel = constrain(lightLevel, 0, 200);
}; 

///////////// READING TEMPERATURE /////////////////
float get_voltage(int pin){
  return (analogRead(pin) * 0.004882814);
};

