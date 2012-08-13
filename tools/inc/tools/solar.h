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

#ifndef _SOLAR_H
#define _SOLAR_H

#include <sal/types.h>
#include <osl/endian.h>
#include <comphelper/fileformat.h>

#ifdef _SOLAR__PRIVATE
#undef _SOLAR__PRIVATE
#endif
#define _SOLAR__PRIVATE 1

/** Intermediate type to solve type clash with Windows headers.
 Should be removed as soon as all code parts have been reviewed
 and the correct type is known. Most of the times ULONG is meant
 to be a 32-Bit unsigned integer type as sal_uInt32 is often
 used for data exchange or for similiar method args. */
typedef sal_uIntPtr    sal_uLong; /* Replaces type ULONG */

// misc. macros to leverage platform and compiler differences

#define DELETEZ( p )    ( delete p,p = 0 )

#ifdef WNT
#if defined (_MSC_VER) && ( _MSC_VER < 1200 )
#define __LOADONCALLAPI _cdecl
#else
#define __LOADONCALLAPI __cdecl
#endif
#else
#define __LOADONCALLAPI
#endif

#if defined UNX
#define ILLEGAL_POINTER ((void*)1)
#else
#define ILLEGAL_POINTER NULL
#endif

// solar binary types

/* Solar (portable) Binary (exchange) Type; OSI 6 subset
   always little endian;
   not necessarily aligned */

typedef sal_uInt8   SVBT8[1];
typedef sal_uInt8   SVBT16[2];
typedef sal_uInt8   SVBT32[4];
typedef sal_uInt8   SVBT64[8];

#ifdef __cplusplus

inline sal_uInt8  SVBT8ToByte  ( const SVBT8  p ) { return p[0]; }
inline sal_uInt16 SVBT16ToShort( const SVBT16 p ) { return (sal_uInt16)p[0]
                                                   + ((sal_uInt16)p[1] <<  8); }
inline sal_uInt32 SVBT32ToUInt32 ( const SVBT32 p ) { return (sal_uInt32)p[0]
                                                   + ((sal_uInt32)p[1] <<  8)
                                                   + ((sal_uInt32)p[2] << 16)
                                                   + ((sal_uInt32)p[3] << 24); }
#if defined OSL_LITENDIAN
inline double   SVBT64ToDouble( const SVBT64 p ) { double n;
                                                    ((sal_uInt8*)&n)[0] = p[0];
                                                    ((sal_uInt8*)&n)[1] = p[1];
                                                    ((sal_uInt8*)&n)[2] = p[2];
                                                    ((sal_uInt8*)&n)[3] = p[3];
                                                    ((sal_uInt8*)&n)[4] = p[4];
                                                    ((sal_uInt8*)&n)[5] = p[5];
                                                    ((sal_uInt8*)&n)[6] = p[6];
                                                    ((sal_uInt8*)&n)[7] = p[7];
                                                    return n; }
#else
inline double   SVBT64ToDouble( const SVBT64 p ) { double n;
                                                    ((sal_uInt8*)&n)[0] = p[7];
                                                    ((sal_uInt8*)&n)[1] = p[6];
                                                    ((sal_uInt8*)&n)[2] = p[5];
                                                    ((sal_uInt8*)&n)[3] = p[4];
                                                    ((sal_uInt8*)&n)[4] = p[3];
                                                    ((sal_uInt8*)&n)[5] = p[2];
                                                    ((sal_uInt8*)&n)[6] = p[1];
                                                    ((sal_uInt8*)&n)[7] = p[0];
                                                    return n; }
#endif

inline void     ByteToSVBT8  ( sal_uInt8   n, SVBT8 p ) { p[0] = n; }
inline void     ShortToSVBT16( sal_uInt16 n, SVBT16 p ) { p[0] = (sal_uInt8) n;
                                                      p[1] = (sal_uInt8)(n >>  8); }
inline void     UInt32ToSVBT32 ( sal_uInt32  n, SVBT32 p ) { p[0] = (sal_uInt8) n;
                                                      p[1] = (sal_uInt8)(n >>  8);
                                                      p[2] = (sal_uInt8)(n >> 16);
                                                      p[3] = (sal_uInt8)(n >> 24); }
#if defined OSL_LITENDIAN
inline void     DoubleToSVBT64( double n, SVBT64 p ) { p[0] = ((sal_uInt8*)&n)[0];
                                                       p[1] = ((sal_uInt8*)&n)[1];
                                                       p[2] = ((sal_uInt8*)&n)[2];
                                                       p[3] = ((sal_uInt8*)&n)[3];
                                                       p[4] = ((sal_uInt8*)&n)[4];
                                                       p[5] = ((sal_uInt8*)&n)[5];
                                                       p[6] = ((sal_uInt8*)&n)[6];
                                                       p[7] = ((sal_uInt8*)&n)[7]; }
#else
inline void     DoubleToSVBT64( double n, SVBT64 p ) { p[0] = ((sal_uInt8*)&n)[7];
                                                       p[1] = ((sal_uInt8*)&n)[6];
                                                       p[2] = ((sal_uInt8*)&n)[5];
                                                       p[3] = ((sal_uInt8*)&n)[4];
                                                       p[4] = ((sal_uInt8*)&n)[3];
                                                       p[5] = ((sal_uInt8*)&n)[2];
                                                       p[6] = ((sal_uInt8*)&n)[1];
                                                       p[7] = ((sal_uInt8*)&n)[0]; }
#endif
#endif

#ifndef __cplusplus
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif

#ifdef __cplusplus
template<typename T> inline T Min(T a, T b) { return (a<b?a:b); }
template<typename T> inline T Max(T a, T b) { return (a>b?a:b); }
template<typename T> inline T Abs(T a) { return (a>=0?a:-a); }
#endif

#ifdef __cplusplus
#define EXTERN_C    extern "C"
#else
#define EXTERN_C
#endif

#ifdef NOHACKS
#define HACK( comment ) #error hack: comment
#else
#define HACK( comment )
#endif

#define _LF     ((char)0x0A)
#define _CR     ((char)0x0D)

// pragmas

#if defined _MSC_VER
/* deletion of pointer to incomplete type '...'; no destructor called
 serious error, memory deleted without call of dtor */
#pragma warning( error: 4150 )
// warning C4002: too many actual parameters for macro
// warning C4003: not enough actual parameters for macro
#pragma warning(error : 4002 4003)
#endif

// dll file extensions

#if defined WNT
  #define __DLLEXTENSION "lo"
#elif defined MACOSX
  #define __DLLEXTENSION "lo.dylib"
#elif defined UNX
  #define __DLLEXTENSION "lo.so"
#else
  #error unknown platform
#endif

#define UniString       String
#define XubString       String
#define xub_Unicode     sal_Unicode
#define xub_StrLen      sal_uInt16

#define LIBRARY_CONCAT3( s1, s2, s3 ) \
    s1 s2 s3
#define LIBRARY_CONCAT4( s1, s2, s3, s4 ) \
    s1 s2 s3 s4

#if defined WNT
#define SVLIBRARY( Base ) \
    LIBRARY_CONCAT3( Base, __DLLEXTENSION, ".DLL" )
#elif defined UNX
#define SVLIBRARY( Base ) \
    LIBRARY_CONCAT3( "lib", Base, __DLLEXTENSION )
#else
#define SVLIBRARY( Base ) \
    LIBRARY_CONCAT2( Base, __DLLEXTENSION )
#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
