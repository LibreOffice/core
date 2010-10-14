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


#ifndef _VOS_MACROS_HXX_
#define _VOS_MACROS_HXX_


#include <osl/endian.h>

// *********************************************************************
// Macro definitions

#ifndef VOS_CAST
#   define VOS_CAST(type,value)    (*((type*)&(value)))
#endif

#ifndef VOS_UNUSED
#   define VOS_UNUSED(x)           (x=x)
#endif

#ifndef VOS_FOREVER
#   define VOS_FOREVER             for(;;)
#endif

#ifndef VOS_MAX
#   define VOS_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef VOS_MIN
#   define VOS_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef VOS_ABS
#   define VOS_ABS(a)              (((a) < 0) ? (-(a)) : (a))
#endif
#ifndef VOS_SIGN
#   define VOS_SIGN(a)             ( ((a) < 0) ? (-1) : (((a) > 0) ? (1) : (0)) )
#endif

#ifndef VOS_BOUND
#   define VOS_BOUND(x,l,h)        ((x) <= (l) ? (l) : ((x) >= (h) ? (h) : (x)))
#endif

#ifndef VOS_SWAP
#   define VOS_SWAP(a,b)           ((a) ^= (b) ^= (a) ^= (b))
#endif

#ifndef VOS_BYTEBOUND
#   define VOS_BYTEBOUND(a)        (((a) + 7) / 8)
#endif

#ifndef VOS_WORDBOUND
#   define VOS_WORDBOUND(a)        ((((a) + 15) / 16) * 2)
#endif

#ifndef VOS_DWORDBOUND
#   define VOS_DWORDBOUND(a)       ((((a) + 31) / 32) * 4)
#endif

#ifndef VOS_MAKEDWORD
#   define VOS_MAKEDWORD(wl, wh)   ((sal_uInt32)((wl) & 0xFFFF) | (((sal_uInt32)(wh) & 0xFFFF) << 16))
#endif
#ifndef VOS_LOWORD
#   define VOS_LOWORD(d)           ((sal_uInt16)((sal_uInt32)(d) & 0xFFFF))
#endif
#ifndef VOS_HIWORD
#   define VOS_HIWORD(d)           ((sal_uInt16)(((sal_uInt32)(d) >> 16) & 0xFFFF))
#endif
#ifndef VOS_MAKEWORD
#   define VOS_MAKEWORD(bl, bh)    ((sal_uInt16)((bl) & 0xFF) | (((sal_uInt16)(bh) & 0xFF) << 8))
#endif
#ifndef VOS_LOBYTE
#   define VOS_LOBYTE(w)           ((sal_uInt8)((sal_uInt16)(w) & 0xFF))
#endif
#ifndef VOS_HIBYTE
#   define VOS_HIBYTE(w)           ((sal_uInt8)(((sal_uInt16)(w) >> 8) & 0xFF))
#endif
#ifndef VOS_MAKEBYTE
#   define VOS_MAKEBYTE(nl, nh)    ((sal_uInt8)(((nl) & 0x0F) | (((nh) & 0x0F) << 4)))
#endif
#ifndef VOS_LONIBBLE
#   define VOS_LONIBBLE(b)         ((sal_uInt8)((b) & 0x0F))
#endif
#ifndef VOS_HINIBBLE
#   define VOS_HINIBBLE(b)         ((sal_uInt8)(((b) >> 4) & 0x0F))
#endif

#ifndef VOS_SWAPWORD
#   define VOS_SWAPWORD(w)         VOS_MAKEWORD(VOS_HIBYTE(w),VOS_LOBYTE(w))
#endif
#ifndef VOS_SWAPDWORD
#   define VOS_SWAPDWORD(d)        VOS_MAKEDWORD(VOS_SWAPWORD(VOS_HIWORD(d)),VOS_SWAPWORD(VOS_LOWORD(d)))
#endif

#ifdef OSL_BIGENDIAN
#ifndef VOS_NETWORD
#   define VOS_NETWORD(w)          (sal_uInt16)(w)
#endif
#ifndef VOS_NETDWORD
#   define VOS_NETDWORD(d)         (sal_uInt32)(d)
#endif
#else  // OSL_LITENDIAN
#ifndef VOS_NETWORD
#   define VOS_NETWORD(w)          VOS_MAKEWORD(VOS_HIBYTE(w),VOS_LOBYTE(w))
#endif
#ifndef VOS_NETDWORD
#   define VOS_NETDWORD(d)         VOS_MAKEDWORD(VOS_NETWORD(VOS_HIWORD(d)),VOS_NETWORD(VOS_LOWORD(d)))
#endif
#endif // OSL_BIGENDIAN

#ifdef _OSL_MEMSEG
#   define VOS_MAKEPTR(base, off)  ((void _far *)VOS_MAKEDWORD((off), (base)))
#   define VOS_BASEOF(ptr)         VOS_HIWORD(ptr)
#   define VOS_OFSTOF(ptr)         VOS_LOWORD(ptr)
#else
#   define VOS_MAKEPTR(base, off)  ((void *)((base) + (off)))
#   define VOS_BASEOF(ptr)         (ptr)
#   define VOS_OFSTOF(ptr)         0
#endif

#ifndef VOS_FIELDOFFSET
#   define VOS_FIELDOFFSET(type, field) ((sal_Int32)(&((type *)1)->field) - 1)
#endif

// def. for arbitrary namespace
#define VOS_NAMESPACE(class_name, name_space) name_space::class_name

// sal_Int16 def. for namespace vos
#define NAMESPACE_VOS(class_name) vos::class_name

// sal_Int16 def. for namespace std
#define NAMESPACE_STD(class_name) std::class_name

#endif //_VOS_MACROS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
