/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef _VCL_AQUA_COMMON_H
#define _VCL_AQUA_COMMON_H

#include <sal/types.h>
#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>
#include <tools/debug.hxx>

// CoreFoundation designers, in their wisdom, decided that CFRelease of NULL
// cause a Crash, yet few API can return NULL when asking for the creation
// of an object, which force us to peper the code with egly if construct everywhere
// and open the door to very nasty crash on rare occasion
// this macro hide the mess
#define SafeCFRelease(a) do { if(a) { CFRelease(a); (a)=NULL; } } while(false)


#define round_to_long(a) ((a) >= 0 ? ((long)((a) + 0.5)) : ((long)((a) - 0.5)))

#include "vcl/salgtype.hxx"
#endif/* _VCL_AQUA_COMMON_H */

//#define msgs_debug(c,f,...)
//    fprintf(stderr, "%s::%s(%p:%04.4x)\n", this, #c, __func__, 0, __VA_ARGS__ )

#define msgs_debug(c,f,...)                                             \
    fprintf(stderr, "%s::%s(%p:%4.4u)" f "\n", #c, __func__, this, (unsigned int)((sal_uInt64)pthread_self() & 0x1FFF), ##__VA_ARGS__ );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
