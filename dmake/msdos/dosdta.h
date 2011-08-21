#ifndef _DOSDTA_
#define _DOSDTA_

#include "stdmacs.h"
typedef struct {
   char   fcb[21];
   char   attr;
   short  time;
   short  date;
   long   size;
   char   name[13];
} DTA;

extern DTA  *findfirst ANSI((char *, DTA *));
extern DTA  *findnext  ANSI((DTA *));
#endif
