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

#ifndef _SOLAR_H
#define _SOLAR_H

#include <sal/types.h>
#include <osl/endian.h>
#include <comphelper/fileformat.h>

/*** common solar defines ***********************************/

#ifdef _SOLAR__PRIVATE
#undef _SOLAR__PRIVATE
#endif
#define _SOLAR__PRIVATE 1
#define __REFERENCED    0

/************************************************************
 Intermediate type to solve type clash with Windows headers.
 Should be removed as soon as all code parts have been reviewed
 and the correct type is known. Most of the times ULONG is meant
 to be a 32-Bit unsigned integer type as sal_uInt32 is often
 used for data exchange or for similiar method args.
*************************************************************/
typedef sal_uIntPtr    sal_uLong; /* Replaces type ULONG */

/*** misc. macros to leverage platform and compiler differences ********/

typedef int             FASTBOOL;
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

/*** solar binary types **********************************************/

#ifndef _SOLAR_NOSVBT
/* Solar (portable) Binary (exchange) Type; OSI 6 subset
   always little endian;
   not necessarily aligned */

typedef sal_uInt8               SVBT8[1];
typedef sal_uInt8               SVBT16[2];
typedef sal_uInt8               SVBT32[4];
typedef sal_uInt8               SVBT64[8];

#ifdef __cplusplus

inline sal_uInt8    SVBT8ToByte  ( const SVBT8  p ) { return p[0]; }
inline sal_uInt16   SVBT16ToShort( const SVBT16 p ) { return (sal_uInt16)p[0]
                                                   + ((sal_uInt16)p[1] <<  8); }
inline sal_uInt32   SVBT32ToUInt32 ( const SVBT32 p ) { return (sal_uInt32)p[0]
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
#endif


/*** standard floating point definitions *******************************/

#ifndef F_PI
#define F_PI        3.14159265358979323846
#endif
#ifndef F_PI2
#define F_PI2       1.57079632679489661923
#endif
#ifndef F_PI4
#define F_PI4       0.785398163397448309616
#endif
#ifndef F_PI180
#define F_PI180     0.01745329251994
#endif
#ifndef F_PI1800
#define F_PI1800    0.001745329251994
#endif
#ifndef F_PI18000
#define F_PI18000   0.0001745329251994
#endif
#ifndef F_2PI
#define F_2PI       6.28318530717958647694
#endif


/*** standard macros *****************************************/

#define SWAPSHORT(x) ((((x) >> 8) & 0x00FF) | (((x) & 0x00FF) << 8))
#define SWAPLONG(x)  ((((x) >> 24) & 0x000000FF) | (((x) & 0x00FF0000) >> 8) | \
                      (((x) & 0x0000FF00) <<  8) | (((x) & 0x000000FF) << 24))

#ifndef __cplusplus
#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#endif



/*** standard inline functions *****************************************/

#ifdef __cplusplus
template<typename T> inline T Min(T a, T b) { return (a<b?a:b); }
template<typename T> inline T Max(T a, T b) { return (a>b?a:b); }
template<typename T> inline T Abs(T a) { return (a>=0?a:-a); }
#endif


/*** C / C++ - macros **************************************************/

#ifdef __cplusplus
#define BEGIN_C     extern "C" {
#define END_C       }
#define EXTERN_C    extern "C"
#else
#define BEGIN_C
#define END_C
#define EXTERN_C
#endif


/*** macros ************************************************************/

#ifdef NOHACKS
#define HACK( comment ) #error hack: comment
#else
#define HACK( comment )
#endif

#define _MAKE_NUMSTR( n )           # n
#define MAKE_NUMSTR( n )            _MAKE_NUMSTR( n )

#define _LF     ((char)0x0A)
#define _CR     ((char)0x0D)


/*** pragmas ************************************************************/

#if defined _MSC_VER
/* deletion of pointer to incomplete type '...'; no destructor called
 serious error, memory deleted without call of dtor */
#pragma warning( error: 4150 )
// warning C4002: too many actual parameters for macro
// warning C4003: not enough actual parameters for macro
#pragma warning(error : 4002 4003)
#endif


/* dll file extensions *******************************************************/

#if defined WNT && defined __MINGW32__ && defined INTEL
  #define __DLLEXTENSION "gi"
#elif defined WNT && defined __MINGW32__ && defined X86_64
  #define __DLLEXTENSION "gx"
#elif defined WNT && defined _MSC_VER && defined INTEL
  #define __DLLEXTENSION "mi"
#elif defined WNT && defined _MSC_VER && defined X86_64
  #define __DLLEXTENSION "mx"
#elif defined OS2
  #define __DLLEXTENSION "go"
#elif defined AIX
  #define __DLLEXTENSION "ap.so"
#elif defined SOLARIS && defined SPARC && defined IS_LP64
  #define __DLLEXTENSION "su.so"
#elif defined SOLARIS && defined SPARC && !defined __GNUC__
  #define __DLLEXTENSION "ss.so"
#elif defined SOLARIS && defined SPARC && defined __GNUC__
  #define __DLLEXTENSION "sogs.so"
#elif defined SOLARIS && defined INTEL && !defined __GNUC__
  #define __DLLEXTENSION "si.so"
#elif defined SOLARIS && defined INTEL && defined __GNUC__
  #define __DLLEXTENSION "sogi.so"
#elif defined NETBSD && defined X86
  #define __DLLEXTENSION "bi.so"
#elif defined NETBSD && defined X86_64
  #define __DLLEXTENSION "bx.so"
#elif defined NETBSD && defined ARM32
  #define __DLLEXTENSION "ba.so"
#elif defined NETBSD && defined SPARC
  #define __DLLEXTENSION "bs.so"
#elif defined NETBSD && defined POWERPC
  #define __DLLEXTENSION "bp.so"
#elif defined LINUX && defined X86
  #define __DLLEXTENSION "li.so"
#elif defined LINUX && defined POWERPC
  #define __DLLEXTENSION "lp.so"
#elif defined LINUX && defined S390
  #define __DLLEXTENSION "l3.so"
#elif defined LINUX && defined ARM32
  #define __DLLEXTENSION "lr.so"
#elif defined LINUX && defined SPARC
  #define __DLLEXTENSION "ls.so"
#elif defined LINUX && defined __x86_64__
  #define __DLLEXTENSION "lx.so"
#elif defined LINUX && defined MIPS
  #define __DLLEXTENSION "lm.so"
#elif defined LINUX && defined IA64
  #define __DLLEXTENSION "la.so"
#elif defined LINUX && defined M68K
  #define __DLLEXTENSION "lm.so"
#elif defined LINUX && defined HPPA
  #define __DLLEXTENSION "lh.so"
#elif defined LINUX && defined AXP
  #define __DLLEXTENSION "ll.so"
#elif defined FREEBSD && defined X86
  #define __DLLEXTENSION "fi.so"
#elif defined FREEBSD && defined X86_64
  #define __DLLEXTENSION "fx.so"
#elif defined MACOSX && defined POWERPC
  #define __DLLEXTENSION "mxp.dylib"
#elif defined MACOSX && defined X86
  #define __DLLEXTENSION "mxi.dylib"
#elif defined OPENBSD
  #define __DLLEXTENSION "ob.so"
#elif defined DRAGONFLY
  #define __DLLEXTENSION "df.so"
#else
  #error unknown plattform
#endif

// -----------------------------------------------------------------------

#ifndef NOREPLACESTRING
#define UniString       String
#define XubString       String
#else
#define XubString       UniString
#endif
#define xub_Unicode     sal_Unicode
#define xub_uUnicode    sal_Unicode
#ifdef STRING32
#define xub_StrLen      sal_uInt32
#else
#define xub_StrLen      sal_uInt16
#endif

// -- moved here from libcall.hxx ----------------------------------------

#define LIBRARY_STR(s)      # s
#define LIBRARY_STRING(s)   LIBRARY_STR(s)

#define GETFUNCTION( s ) GetFunction( s )
#define LIBRARY_CONCAT3( s1, s2, s3 ) \
    s1 s2 s3
#define LIBRARY_CONCAT4( s1, s2, s3, s4 ) \
    s1 s2 s3 s4

#if defined WNT || defined OS2
#define SVLIBRARY( Base ) \
    LIBRARY_CONCAT3( Base, __DLLEXTENSION, ".DLL" )
#define SVLIBRARYLANG( Base, Lang ) \
    LIBRARY_CONCAT3( Base, Lang, ".DLL" )
#elif defined UNX
#define SVLIBRARY( Base ) \
    LIBRARY_CONCAT3( "lib", Base, __DLLEXTENSION )
#define SVLIBRARYLANG( Base, Lang ) \
    LIBRARY_CONCAT3( "lib", Base, Lang )
#else
#define SVLIBRARY( Base ) \
    LIBRARY_CONCAT2( Base, __DLLEXTENSION )
#define SVLIBRARYLANG( Base, Lang ) \
    LIBRARY_CONCAT2( Base, Lang )
#endif

#if defined MACOSX
#define SV_LIBFILENAME(str) \
    LIBRARYFILENAME_CONCAT2( str, __DLLEXTENSION )
#elif defined UNX
#define SV_LIBFILENAME(str) \
    LIBRARYFILENAME_CONCAT2( str, __DLLEXTENSION )
#else
#define SV_LIBFILENAME(str) \
    LIBRARYFILENAME_CONCAT3( str, __DLLEXTENSION, ".dll" )
#endif

#endif  /* _SOLAR_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
