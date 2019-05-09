#include "const.h"
enum rooms{
  kitchen,
  living_room
};

enum device{
  lamp,
  music,
  radiator
};

enum preference{
  lightning,
  volume,
  heating
};

// Set up number of lamps in Kitchen
#define kitchen_number 1 // Antall lamper på kjøkkenet
int kitchen_lamps[kitchen_number] = {LAMP1};

// Set up number of lamps in living room
#define living_room_number 2
int living_room_lamps[living_room_number] = {LAMP2,LAMP3};






