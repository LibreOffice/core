/*************************************************************************
 *
 *  $RCSfile: types.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:15 $
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

#ifdef __cplusplus
extern "C" {
#endif

/********************************************************************************/
/* Data types
*/

/* Boolean */

typedef unsigned char sal_Bool;
#   define sal_False ((unsigned char)0)
#   define sal_True  ((unsigned char)1)
/*
typedef int sal_Bool;
#   define sal_False 0
#   define sal_True  1
*/
typedef signed char         sal_Int8;
typedef unsigned char       sal_uInt8;
typedef signed short        sal_Int16;
typedef unsigned short      sal_uInt16;
typedef signed long         sal_Int32;
typedef unsigned long       sal_uInt32;

#   if (_MSC_VER >= 1000)
typedef __int64             sal_Int64;
typedef unsigned __int64    sal_uInt64;
#   elif defined(__SUNPRO_CC) || defined(__SUNPRO_C) || defined (__GNUC__) || defined (__MWERKS__) || defined(__hpux)
typedef long long           sal_Int64;
typedef unsigned long long  sal_uInt64;
#   else
#   define SAL_INT64_IS_STRUCT
typedef struct
    {
        sal_uInt32  Part1;
        sal_uInt32  Part2;
    } sal_Int64;

typedef struct
    {
        sal_uInt32  Part1;
        sal_uInt32  Part2;
    } sal_uInt64;
#endif

typedef char                sal_Char;
typedef signed char         sal_sChar;
typedef unsigned char       sal_uChar;
#if defined(SAL_W32)
typedef wchar_t             sal_Unicode;
#else
#define SAL_UNICODE_NOTEQUAL_WCHAR_T
typedef sal_uInt16          sal_Unicode;
#endif

typedef void*               sal_Handle;
typedef unsigned long       sal_Size;

#if defined(SAL_W32) || defined(SAL_OS2) || defined(SAL_UNX) || defined(SAL_MAC)
#define SAL_MAX_ENUM 0x7fffffff
#elif defined(SAL_W16)
#define SAL_MAX_ENUM 0x7fff
#endif

#ifdef SAL_W32
#   define SAL_DLLEXPORT        __declspec(dllexport)
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
#elif defined SAL_MAC
#   define SAL_DLLEXPORT
#   define SAL_CALL
#   define SAL_CALL_ELLIPSE
#elif defined SAL_W16 /* BR: kopiert von SAL_W32 */
#   define SAL_DLLEXPORT        __declspec(dllexport)
#   define SAL_CALL         __cdecl
#   define SAL_CALL_ELLIPSE __cdecl
#else
#   error("unknown platform")
#endif

void SAL_CALL sal_setInt64(sal_Int64* newInt, sal_uInt32 lowInt, sal_Int32 highInt);
void SAL_CALL sal_getInt64(sal_Int64 newInt, sal_uInt32* lowInt, sal_Int32* highInt);

void SAL_CALL sal_setUInt64(sal_uInt64* newInt, sal_uInt32 lowInt, sal_uInt32 highInt);
void SAL_CALL sal_getUInt64(sal_uInt64 newInt, sal_uInt32* lowInt, sal_uInt32* highInt);

#define SAL_PACK_PUSH
#define SAL_PACK_POP

#ifdef SAL_W16
#define SAL_HUGE
#else
#define SAL_HUGE
#endif


#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
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
#if defined SAL_W32 || defined SOLARIS || defined LINUX
#define SAL_CONST_CAST(type, expr) (const_cast< type >(expr))
#define SAL_REINTERPRET_CAST(type, expr) (reinterpret_cast< type >(expr))
#define SAL_STATIC_CAST(type, expr) (static_cast< type >(expr))
#else /* SAL_W32, SOLARIS, LINUX */
#define SAL_CONST_CAST(type, expr) ((type) (expr))
#define SAL_REINTERPRET_CAST(type, expr) ((type) (expr))
#define SAL_STATIC_CAST(type, expr) ((type) (expr))
#endif /* SAL_W32, SOLARIS, LINUX */
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif


#endif /*_SAL_TYPES_H_ */

