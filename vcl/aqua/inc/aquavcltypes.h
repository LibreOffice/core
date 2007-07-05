#ifndef _AQUAVCLTYPES_H
#define _AQUAVCLTYPES_H

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

typedef WindowRef CarbonWindowRef;
typedef ControlRef CarbonViewRef;

// Constants for menu property back pointers passing
#define APP_PROPERTY_CREATOR 'OOo2'
#define APP_PROPERTY_TAG_MENU_POINTER 'mptr'
#define APP_PROPERTY_TAG_MENU_ITEM_POINTER 'iptr'

#endif _AQUAVCLTYPES_H
