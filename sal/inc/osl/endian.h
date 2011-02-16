/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _OSL_ENDIAN_H_
#define _OSL_ENDIAN_H_

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Determine the platform byte order as _BIG_ENDIAN, _LITTLE_ENDIAN, ...
 */
#ifdef _WIN32
#   if defined(_M_IX86)
#       define _LITTLE_ENDIAN
#   elif defined(_M_AMD64)
#       define _LITTLE_ENDIAN
#   elif defined(_M_MRX000)
#       define _LITTLE_ENDIAN
#   elif defined(_M_ALPHA)
#       define _LITTLE_ENDIAN
#   elif defined(_M_PPC)
#       define _LITTLE_ENDIAN
#   endif
#endif

#ifdef LINUX
#   include <endian.h>
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#           ifndef _LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#           endif
#   elif __BYTE_ORDER == __BIG_ENDIAN
#           ifndef _BIG_ENDIAN
#       define _BIG_ENDIAN
#           endif
#   elif __BYTE_ORDER == __PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef NETBSD
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#   undef _BIG_ENDIAN
#   undef _PDP_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#   undef _LITTLE_ENDIAN
#   undef _PDP_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#   undef _LITTLE_ENDIAN
#   undef _BIG_ENDIAN
#   endif
#endif

#ifdef FREEBSD
#   include <sys/param.h>
#   include <machine/endian.h>
#if __FreeBSD_version < 500000
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif
#endif

#ifdef AIX
#   include <sys/machine.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       ifndef _LITTLE_ENDIAN
#           define _LITTLE_ENDIAN
#       endif
#   elif BYTE_ORDER == BIG_ENDIAN
#       ifndef _BIG_ENDIAN
#           define _BIG_ENDIAN
#       endif
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef _WIN16
#   define  _LITTLE_ENDIAN
#endif

#ifdef OS2
#   include <machine/endian.h>
#endif

#ifdef SOLARIS
#   include <sys/isa_defs.h>
#endif

#ifdef MACOSX
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       ifndef _LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#       endif
#   elif BYTE_ORDER == BIG_ENDIAN
#       ifndef _BIG_ENDIAN
#       define _BIG_ENDIAN
#       endif
#   elif BYTE_ORDER == PDP_ENDIAN
#       ifndef _PDP_ENDIAN
#       define _PDP_ENDIAN
#       endif
#   endif
#endif

/** Check supported platform.
 */
#if !defined(_WIN32)  && !defined(_WIN16) && !defined(OS2)     && \
    !defined(LINUX)   && !defined(NETBSD) && \
    !defined(AIX)     && !defined(OPENBSD) && \
    !defined(SOLARIS) && !defined(MACOSX) && !defined(FREEBSD) && \
    !defined(DRAGONFLY)
#   error "Target platform not specified !"
#endif


/** Define the determined byte order as OSL_BIGENDIAN or OSL_LITENDIAN.
 */
#if defined _LITTLE_ENDIAN
#   define OSL_LITENDIAN
#elif defined _BIG_ENDIAN
#   define OSL_BIGENDIAN
#else
#   error undetermined endianess
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
#   define OSL_MAKEWORD(bl, bh)    ((sal_uInt16)((bl) & 0xFF) | (((sal_uInt16)(bh) & 0xFF) << 8))
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

#endif /*_OSL_ENDIAN_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
