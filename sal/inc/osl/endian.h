/*************************************************************************
 *
 *  $RCSfile: endian.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: svesik $ $Date: 2001-04-08 20:10:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _OSL_ENDIAN_H_
#define _OSL_ENDIAN_H_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Determine the platform byte order as _BIG_ENDIAN, _LITTLE_ENDIAN, ...
 */
#ifdef _WIN32
#   if defined(_M_IX86)
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
#       define _LITTLE_ENDIAN
#   elif __BYTE_ORDER == __BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif __BYTE_ORDER == __PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef NETBSD
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef FREEBSD
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef SCO
#   include <sys/types.h>
#   include <sys/byteorder.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef AIX
#   include <sys/machine.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef HPUX
#   include <machine/param.h>
#endif

#ifdef IRIX
#   include <sys/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef S390
#   define  _BIG_ENDIAN
#endif

#ifdef _WIN16
#   define  _LITTLE_ENDIAN
#endif

#ifdef OS2
#   define  _LITTLE_ENDIAN
#endif

#ifdef SOLARIS
#   include <sys/isa_defs.h>
#endif

#ifdef MACOSX
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif

#ifdef MACOS
#   include <premac.h>
#   include <TextUtils.h>
#   include <Endian.h>
#   include <postmac.h>
#   if TARGET_RT_LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif TARGET_RT_BIG_ENDIAN
#       define _BIG_ENDIAN
#   endif
#endif


/** Check supported platform.
 */
#if !defined(_WIN32)  && !defined(_WIN16) && !defined(OS2)   && \
    !defined(LINUX)   && !defined(NETBSD) && !defined(SCO)   && \
    !defined(AIX)     && !defined(HPUX)   && !defined(S390)  && \
    !defined(SOLARIS) && !defined(IRIX)   && !defined(MACOS) && \
    !defined(MACOSX) && !defined(FREEBSD)
#   error "Target plattform not specified !"
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

