/*************************************************************************
 *
 *  $RCSfile: types.h,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 13:26:41 $
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

#ifndef _SAL_TYPES_H_
#define _SAL_TYPES_H_

#include <sal/config.h>

/* Grab __SIZEOFxxx constants from typesconfig tool on Unix */
#if defined UNX
  #include <sal/typesizes.h>
#elif defined WNT
  /* FIXME: autogeneration of type sizes on Win32/Win64? */
  #define SAL_TYPES_ALIGNMENT2      1
  #define SAL_TYPES_ALIGNMENT4      1
  #define SAL_TYPES_ALIGNMENT8      1
  #define SAL_TYPES_SIZEOFSHORT     2
  #define SAL_TYPES_SIZEOFINT       4
  #define SAL_TYPES_SIZEOFLONG      4
  #define SAL_TYPES_SIZEOFLONGLONG      8
  #define SAL_TYPES_SIZEOFPOINTER       4
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************/
/* Data types
*/

/* Boolean */
typedef unsigned char sal_Bool;
#   define sal_False ((sal_Bool)0)
#   define sal_True  ((sal_Bool)1)

/* char is assumed to always be 1 byte long */
typedef signed char         sal_Int8;
typedef unsigned char       sal_uInt8;

#if SAL_TYPES_SIZEOFSHORT == 2
    typedef signed short      sal_Int16;
    typedef unsigned short    sal_uInt16;
#else
     #error "Could not find 16-bit type, add support for your architecture"
#endif

#if SAL_TYPES_SIZEOFLONG == 4
    typedef signed long       sal_Int32;
    typedef unsigned long     sal_uInt32;
#elif SAL_TYPES_SIZEOFINT == 4
    typedef signed int        sal_Int32;
    typedef unsigned int      sal_uInt32;
#else
     #error "Could not find 32-bit type, add support for your architecture"
#endif

#if (_MSC_VER >= 1000)
    typedef __int64                  sal_Int64;
    typedef unsigned __int64         sal_uInt64;

    /*  The following are macros that will add the 64 bit constant suffix. */
    #define SAL_CONST_INT64(x)       x##i64
    #define SAL_CONST_UINT64(x)      x##ui64
#elif defined(__SUNPRO_CC) || defined(__SUNPRO_C) || defined (__GNUC__) || defined(__hpux) || defined (sgi)
    #if SAL_TYPES_SIZEOFLONGLONG == 8
        typedef signed long long    sal_Int64;
        typedef unsigned long long  sal_uInt64;

        /*  The following are macros that will add the 64 bit constant suffix. */
        #define SAL_CONST_INT64(x)       x##ll
        #define SAL_CONST_UINT64(x)      x##ull

    #elif SAL_TYPES_SIZEOFLONG == 8
        typedef signed long         sal_Int64;
        typedef unsigned long       sal_uInt64;


        /*  The following are macros that will add the 64 bit constant suffix. */
        #define SAL_CONST_INT64(x)       x##l
        #define SAL_CONST_UINT64(x)      x##ul
    #else
        #error "Could not find 64-bit type, add support for your architecture"
    #endif
#else
    #error "Please define the 64-bit types for your architecture/compiler in sal/inc/sal/types.h"
#endif

typedef char                     sal_Char;
typedef signed char              sal_sChar;
typedef unsigned char            sal_uChar;

#if defined(SAL_W32)
    typedef wchar_t             sal_Unicode;
#else
    #define SAL_UNICODE_NOTEQUAL_WCHAR_T
    typedef sal_uInt16          sal_Unicode;
#endif

typedef void *                   sal_Handle;

/* sal_Size should currently be the native width of the platform */
#if SAL_TYPES_SIZEOFLONG == 4
    typedef sal_uInt32          sal_Size;
    typedef sal_Int32           sal_sSize;
#elif SAL_TYPES_SIZEOFLONG == 8
    typedef sal_uInt64          sal_Size;
    typedef sal_Int64           sal_sSize;
#else
    #error "Please make sure SAL_TYPES_SIZEOFLONG is defined for your architecture/compiler"
#endif


/********************************************************************************/
/* Useful defines
 */

/* The following SAL_MIN_INTn defines codify the assumption that the signed
 * sal_Int types use two's complement representation.  Defining them as
 * "-0x7F... - 1" instead of as "-0x80..." prevents warnings about applying the
 * unary minus operator to unsigned quantities.
 */
#define SAL_MIN_INT8          ((sal_Int8)   (-0x7F - 1))
#define SAL_MAX_INT8          ((sal_Int8)   0x7F)
#define SAL_MAX_UINT8         ((sal_uInt8)  0xFF)
#define SAL_MIN_INT16         ((sal_Int16)  (-0x7FFF - 1))
#define SAL_MAX_INT16         ((sal_Int16)  0x7FFF)
#define SAL_MAX_UINT16        ((sal_uInt16) 0xFFFF)
#define SAL_MIN_INT32         ((sal_Int32)  (-0x7FFFFFFF - 1))
#define SAL_MAX_INT32         ((sal_Int32)  0x7FFFFFFF)
#define SAL_MAX_UINT32        ((sal_uInt32) 0xFFFFFFFF)
#define SAL_MIN_INT64         ((sal_Int64)  (SAL_CONST_INT64(-0x7FFFFFFFFFFFFFFF) - 1))
#define SAL_MAX_INT64         ((sal_Int64)  SAL_CONST_INT64(0x7FFFFFFFFFFFFFFF))
#define SAL_MAX_UINT64        ((sal_uInt64) SAL_CONST_UINT64(0xFFFFFFFFFFFFFFFF))

#if defined(SAL_W32) || defined(SAL_OS2) || defined(SAL_UNX)
#   define SAL_MAX_ENUM 0x7fffffff
#elif defined(SAL_W16)
#   define SAL_MAX_ENUM 0x7fff
#endif

#ifdef _MSC_VER
#   define SAL_DLLEXPORT    __declspec(dllexport)
#   define SAL_CALL         __cdecl
#   define SAL_CALL_ELLIPSE __cdecl
#elif defined SAL_OS2
#   define SAL_DLLEXPORT
#   define SAL_CALL
#   define SAL_CALL_ELLIPSE
#elif defined SAL_UNX
#   define SAL_DLLEXPORT
#   define SAL_CALL
#   define SAL_CALL_ELLIPSE
#else
#   error("unknown platform")
#endif

/** Use this for pure virtual classes, e.g. class SAL_NO_VTABLE Foo { ...
    This hinders the compiler from setting a generic vtable stating that
    a pure virtual function was called and thus slightly reduces code size.
*/
#ifdef _MSC_VER
#   define SAL_NO_VTABLE __declspec(novtable)
#else
#   define SAL_NO_VTABLE
#endif

#ifdef SAL_W32
#   pragma pack(push, 8)
#elif defined(SAL_OS2)
#   pragma pack(8)
#endif

/** This is the binary specification of a SAL sequence.
    <br>
*/
typedef struct _sal_Sequence
{
    /** reference count of sequence<br>
    */
    sal_Int32           nRefCount;
    /** element count<br>
    */
    sal_Int32           nElements;
    /** elements array<br>
    */
    char                elements[1];
} sal_Sequence;

#define SAL_SEQUENCE_HEADER_SIZE ((sal_Size)&((sal_Sequence *)0)->elements)

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif


/** Wrap C++ const_cast, reinterpret_cast and static_cast expressions in
    macros to keep code portable to old compilers (since most compilers still
    lack RTTI support, dynamic_cast is not included here).
 */
#ifdef __cplusplus
#if defined SAL_W32 || defined SOLARIS || defined LINUX || defined MACOSX || defined FREEBSD || defined NETBSD || defined AIX
#define SAL_CONST_CAST(type, expr) (const_cast< type >(expr))
#define SAL_REINTERPRET_CAST(type, expr) (reinterpret_cast< type >(expr))
#define SAL_STATIC_CAST(type, expr) (static_cast< type >(expr))
#else /* SAL_W32, SOLARIS, LINUX */
#define SAL_CONST_CAST(type, expr) ((type) (expr))
#define SAL_REINTERPRET_CAST(type, expr) ((type) (expr))
#define SAL_STATIC_CAST(type, expr) ((type) (expr))
#endif /* SAL_W32, SOLARIS, LINUX */
#endif /* __cplusplus */

/** Definition of function throw clause macros.  These have been introduced
    to reduce code size by balancing out compiler bugs.

    These macros are ONLY for function declarations,
    use common C++ throw statement for throwing exceptions, e.g.
    throw RuntimeException();

    SAL_THROW()          should be used for all C++ functions, e.g. SAL_THROW( () )
    SAL_THROW_EXTERN_C() should be used for all C functions
*/
#ifdef __cplusplus
#if defined(__GNUC__) || defined(__SUNPRO_CC) || defined(__sgi)
#define SAL_THROW( exc )
#else /* MSVC, all other */
#define SAL_THROW( exc ) throw exc
#endif /* __GNUC__, __SUNPRO_CC */
#define SAL_THROW_EXTERN_C() throw ()
#else /* ! __cplusplus */
/* SAL_THROW() must not be used in C headers, only SAL_THROW_EXTERN_C() is defined */
#define SAL_THROW_EXTERN_C()
#endif



#ifdef __cplusplus
enum __sal_NoAcquire
{
    /** definition of a no acquire enum for ctors
    */
    SAL_NO_ACQUIRE
};
#endif /* __cplusplus */


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /*_SAL_TYPES_H_ */

