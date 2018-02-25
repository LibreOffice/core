/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OSL_ENDIAN_H
#define INCLUDED_OSL_ENDIAN_H

#include "sal/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Define the platform byte order as OSL_BIGENDIAN or OSL_LITENDIAN.
 */

#if defined _WIN32
#   if defined _M_ALPHA || defined _M_AMD64 || defined _M_IX86 \
            || defined _M_MRX000 || defined _M_PPC
#       define OSL_LITENDIAN
#   endif
#elif defined ANDROID || defined LINUX || defined HAIKU
#   include <endian.h>
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#       define OSL_LITENDIAN
#   elif __BYTE_ORDER == __BIG_ENDIAN
#       define OSL_BIGENDIAN
#   endif
#elif defined IOS || defined MACOSX || defined NETBSD
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define OSL_LITENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define OSL_BIGENDIAN
#   endif
#elif defined FREEBSD
#   include <sys/param.h>
#   include <machine/endian.h>
#   if defined _LITTLE_ENDIAN
#       define OSL_LITENDIAN
#   elif defined _BIG_ENDIAN
#       define OSL_BIGENDIAN
#   endif
#elif defined AIX
#   include <sys/machine.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define OSL_LITENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define OSL_BIGENDIAN
#   endif
#elif defined __sun
#   include <sys/isa_defs.h>
#   if defined _LITTLE_ENDIAN
#       define OSL_LITENDIAN
#   elif defined _BIG_ENDIAN
#       define OSL_BIGENDIAN
#   endif
#else
#   error "Target platform not specified !"
#endif
#if defined OSL_LITENDIAN == defined OSL_BIGENDIAN
#   error undetermined endianness
#endif


/** Define macros for byte order manipulation.
 */
#ifndef OSL_MAKEBYTE
#   define OSL_MAKEBYTE(nl, nh)    ((sal_uInt8)(((nl) & 0x0F) | (((nh) & 0x0F) << 4)))
#endif
#ifndef OSL_LONIBBLE
#   define OSL_LONIBBLE(b)         ((sal_uInt8)((b) & 0x0F))
#endif
#ifndef OSL_HINIBBLE
#   define OSL_HINIBBLE(b)         ((sal_uInt8)(((b) >> 4) & 0x0F))
#endif

#ifndef OSL_MAKEWORD
#   define OSL_MAKEWORD(bl, bh)    ((sal_uInt16)((sal_uInt16)((bl) & 0xFF) | (((sal_uInt16)(bh) & 0xFF) << 8)))
#endif
#ifndef OSL_LOBYTE
#   define OSL_LOBYTE(w)           ((sal_uInt8)((sal_uInt16)(w) & 0xFF))
#endif
#ifndef OSL_HIBYTE
#   define OSL_HIBYTE(w)           ((sal_uInt8)(((sal_uInt16)(w) >> 8) & 0xFF))
#endif

#ifndef OSL_MAKEDWORD
#   define OSL_MAKEDWORD(wl, wh)   ((sal_uInt32)((wl) & 0xFFFF) | (((sal_uInt32)(wh) & 0xFFFF) << 16))
#endif
#ifndef OSL_LOWORD
#   define OSL_LOWORD(d)           ((sal_uInt16)((sal_uInt32)(d) & 0xFFFF))
#endif
#ifndef OSL_HIWORD
#   define OSL_HIWORD(d)           ((sal_uInt16)(((sal_uInt32)(d) >> 16) & 0xFFFF))
#endif


/** Define macros for swapping between host and network byte order.
 */
#ifdef OSL_BIGENDIAN
#ifndef OSL_NETWORD
#   define OSL_NETWORD(w)          (sal_uInt16)(w)
#endif
#ifndef OSL_NETDWORD
#   define OSL_NETDWORD(d)         (sal_uInt32)(d)
#endif
#else  /* OSL_LITENDIAN */
#ifndef OSL_NETWORD
#   define OSL_NETWORD(w)          OSL_MAKEWORD(OSL_HIBYTE(w),OSL_LOBYTE(w))
#endif
#ifndef OSL_NETDWORD
#   define OSL_NETDWORD(d)         OSL_MAKEDWORD(OSL_NETWORD(OSL_HIWORD(d)),OSL_NETWORD(OSL_LOWORD(d)))
#endif
#endif /* OSL_BIGENDIAN */


/** Define macros for swapping between byte orders.
 */
#ifndef OSL_SWAPWORD
#   define OSL_SWAPWORD(w)         OSL_MAKEWORD(OSL_HIBYTE(w),OSL_LOBYTE(w))
#endif
#ifndef OSL_SWAPDWORD
#   define OSL_SWAPDWORD(d)        OSL_MAKEDWORD(OSL_SWAPWORD(OSL_HIWORD(d)),OSL_SWAPWORD(OSL_LOWORD(d)))
#endif


#ifdef __cplusplus
}
#endif

#endif // INCLUDED_OSL_ENDIAN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
