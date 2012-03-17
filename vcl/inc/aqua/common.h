
#ifndef _VCL_AQUA_COMMON_H
#define _VCL_AQUA_COMMON_H

#include <sal/types.h>
#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

// CoreFoundation designers, in their wisdom, decided that CFRelease of NULL
// cause a Crash, yet few API can return NULL when asking for the creation
// of an object, which force us to peper the code with egly if construct everywhere
// and open the door to very nasty crash on rare occasion
// this macro hide the mess
#define SafeCFRelease(a) do { if(a) { CFRelease(a); } } while(false)
#endif
