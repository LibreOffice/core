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

#ifndef _SAL_TYPES_H_
#define _SAL_TYPES_H_

#include <sal/config.h>

/* Grab __SIZEOFxxx constants from typesconfig tool on Unix */
#if defined UNX
  #include <sal/typesizes.h>
#elif defined(WNT) || defined(OS2)
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
    #define SAL_PRIdINT32 "ld"
    #define SAL_PRIuUINT32 "lu"
    #define SAL_PRIxUINT32 "lx"
    #define SAL_PRIXUINT32 "lX"
#elif SAL_TYPES_SIZEOFINT == 4
    typedef signed int        sal_Int32;
    typedef unsigned int      sal_uInt32;
    #define SAL_PRIdINT32 "d"
    #define SAL_PRIuUINT32 "u"
    #define SAL_PRIxUINT32 "x"
    #define SAL_PRIXUINT32 "X"
#else
     #error "Could not find 32-bit type, add support for your architecture"
#endif

#if (_MSC_VER >= 1000)
    typedef __int64                  sal_Int64;
    typedef unsigned __int64         sal_uInt64;

    /*  The following are macros that will add the 64 bit constant suffix. */
    #define SAL_CONST_INT64(x)       x##i64
    #define SAL_CONST_UINT64(x)      x##ui64

    #define SAL_PRIdINT64 "I64d"
    #define SAL_PRIuUINT64 "I64u"
    #define SAL_PRIxUINT64 "I64x"
    #define SAL_PRIXUINT64 "I64X"
#elif defined(__SUNPRO_CC) || defined(__SUNPRO_C) || defined (__GNUC__) || defined(__hpux) || defined (sgi)
    #if SAL_TYPES_SIZEOFLONG == 8
        typedef signed long int         sal_Int64;
        typedef unsigned long int       sal_uInt64;


        /*  The following are macros that will add the 64 bit constant suffix. */
        #define SAL_CONST_INT64(x)       x##l
        #define SAL_CONST_UINT64(x)      x##ul

        #define SAL_PRIdINT64 "ld"
        #define SAL_PRIuUINT64 "lu"
        #define SAL_PRIxUINT64 "lx"
        #define SAL_PRIXUINT64 "lX"
    #elif SAL_TYPES_SIZEOFLONGLONG == 8
        typedef signed long long    sal_Int64;
        typedef unsigned long long  sal_uInt64;

        /*  The following are macros that will add the 64 bit constant suffix. */
        #define SAL_CONST_INT64(x)       x##ll
        #define SAL_CONST_UINT64(x)      x##ull

    #ifdef __MINGW32__
    #define SAL_PRIdINT64 "I64d"
    #define SAL_PRIuUINT64 "I64u"
    #define SAL_PRIxUINT64 "I64x"
    #define SAL_PRIXUINT64 "I64X"
    #else
        #define SAL_PRIdINT64 "lld"
        #define SAL_PRIuUINT64 "llu"
        #define SAL_PRIxUINT64 "llx"
        #define SAL_PRIXUINT64 "llX"
    #endif
    #else
        #error "Could not find 64-bit type, add support for your architecture"
    #endif
#else
    #error "Please define the 64-bit types for your architecture/compiler in sal/inc/sal/types.h"
#endif

typedef char                     sal_Char;
typedef signed char              sal_sChar;
typedef unsigned char            sal_uChar;

#if ( defined(SAL_W32) && !defined(__MINGW32__) )
    typedef wchar_t             sal_Unicode;
#else
    #define SAL_UNICODE_NOTEQUAL_WCHAR_T
    typedef sal_uInt16          sal_Unicode;
#endif

typedef void *                   sal_Handle;

/* sal_Size should currently be the native width of the platform */
#if SAL_TYPES_SIZEOFPOINTER == 4
    typedef sal_uInt32          sal_Size;
    typedef sal_Int32           sal_sSize;
#elif SAL_TYPES_SIZEOFPOINTER == 8
    typedef sal_uInt64          sal_Size;
    typedef sal_Int64           sal_sSize;
#else
    #error "Please make sure SAL_TYPES_SIZEOFPOINTER is defined for your architecture/compiler"
#endif

/* sal_PtrDiff holds the result of a pointer subtraction */
#if SAL_TYPES_SIZEOFPOINTER == 4
    typedef sal_Int32           sal_PtrDiff;
#elif SAL_TYPES_SIZEOFPOINTER == 8
    typedef sal_Int64           sal_PtrDiff;
#else
    #error "Please make sure SAL_TYPES_SIZEOFPOINTER is defined for your architecture/compiler"
#endif

/* printf-style conversion specification length modifiers for size_t and
   ptrdiff_t (most platforms support C99, MSC has its own extension) */
#if defined(_MSC_VER) || defined(__MINGW32__)
    #define SAL_PRI_SIZET "I"
    #define SAL_PRI_PTRDIFFT "I"
#else
    #define SAL_PRI_SIZET "z"
    #define SAL_PRI_PTRDIFFT "t"
#endif

/* sal_IntPtr, sal_uIntPtr are integer types designed to hold pointers so that any valid
 * pointer to void can be converted to this type and back to a pointer to void and the
 * result will compare to the original pointer */
#if SAL_TYPES_SIZEOFPOINTER == 4
    typedef sal_Int32           sal_IntPtr;
    typedef sal_uInt32          sal_uIntPtr;
    #define SAL_PRIdINTPTR SAL_PRIdINT32
    #define SAL_PRIuUINTPTR SAL_PRIuUINT32
    #define SAL_PRIxUINTPTR SAL_PRIxUINT32
    #define SAL_PRIXUINTPTR SAL_PRIXUINT32
#elif SAL_TYPES_SIZEOFPOINTER == 8
    typedef sal_Int64           sal_IntPtr;
    typedef sal_uInt64          sal_uIntPtr;
    #define SAL_PRIdINTPTR SAL_PRIdINT64
    #define SAL_PRIuUINTPTR SAL_PRIuUINT64
    #define SAL_PRIxUINTPTR SAL_PRIxUINT64
    #define SAL_PRIXUINTPTR SAL_PRIXUINT64
#else
    #error "Please make sure SAL_TYPES_SIZEOFPOINTER is defined for your architecture/compiler"
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

#if SAL_TYPES_SIZEOFLONG == 4
#define SAL_MAX_SSIZE       SAL_MAX_INT32
#define SAL_MAX_SIZE        SAL_MAX_UINT32
#elif SAL_TYPES_SIZEOFLONG == 8
#define SAL_MAX_SSIZE       SAL_MAX_INT64
#define SAL_MAX_SIZE        SAL_MAX_UINT64
#endif

#if defined(SAL_W32) || defined(SAL_OS2) || defined(SAL_UNX)
#   define SAL_MAX_ENUM 0x7fffffff
#elif defined(SAL_W16)
#   define SAL_MAX_ENUM 0x7fff
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#   define SAL_DLLPUBLIC_EXPORT    __declspec(dllexport)
#if defined(_MSC_VER)
#   define SAL_DLLPUBLIC_IMPORT    __declspec(dllimport)
#else
#   define SAL_DLLPUBLIC_IMPORT
#endif // defined(_MSC_VER)
#   define SAL_DLLPRIVATE
#if defined(_MSC_VER)
#   define SAL_CALL         __cdecl
#   define SAL_CALL_ELLIPSE __cdecl
#else
#   define SAL_CALL
#   define SAL_CALL_ELLIPSE
#endif
#elif defined SAL_OS2 // YD
/* YD 25/09/2007 gcc doesn't like imports inside class members */
#   define SAL_DLLPUBLIC_EXPORT
#   define SAL_DLLPUBLIC_IMPORT
#   define SAL_DLLPRIVATE
#   define SAL_CALL
#   define SAL_CALL_ELLIPSE
#elif defined SAL_UNX
#   if   defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#     define SAL_DLLPUBLIC_EXPORT  __global
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE        __hidden
#   elif defined(__SUNPRO_C ) && (__SUNPRO_C  >= 0x550)
#     define SAL_DLLPUBLIC_EXPORT  __global
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE        __hidden
#   elif defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
#     define SAL_DLLPUBLIC_EXPORT  __attribute__ ((visibility("default")))
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE        __attribute__ ((visibility("hidden")))
#   else
#     define SAL_DLLPUBLIC_EXPORT
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE
#   endif
#   define SAL_CALL
#   define SAL_CALL_ELLIPSE
#else
#   error("unknown platform")
#endif

/**
   Exporting the symbols necessary for exception handling on GCC.

   These macros are used for inline declarations of exception classes, as in
   rtl/malformeduriexception.hxx.
*/
#if defined(__GNUC__) && ! defined(__MINGW32__)
#define SAL_EXCEPTION_DLLPUBLIC_EXPORT SAL_DLLPUBLIC_EXPORT
#define SAL_EXCEPTION_DLLPRIVATE SAL_DLLPRIVATE
#else
#define SAL_EXCEPTION_DLLPUBLIC_EXPORT
#define SAL_EXCEPTION_DLLPRIVATE
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
#   pragma pack(push, 4)
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

#if defined( SAL_W32) ||  defined(SAL_OS2)
#pragma pack(pop)
#endif


/** Wrap C++ const_cast, reinterpret_cast and static_cast expressions in
    macros to keep code portable to old compilers (since most compilers still
    lack RTTI support, dynamic_cast is not included here).
 */
#ifdef __cplusplus
#if defined SAL_W32 || defined SOLARIS || defined LINUX || defined MACOSX || defined FREEBSD || defined NETBSD || defined AIX || defined OS2
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

#ifdef __cplusplus

namespace sal {

/**
   A static_cast between integral types, to avoid C++ compiler warnings.

   In C++ source code, use sal::static_int_cast<T>(n) instead of
   static_cast<T>(n) whenever a compiler warning about integral type problems
   shall be silenced.  That way, source code that needs to be modified when the
   type of any of the expressions involved in the compiler warning is changed
   can be found more easily.

   Both template arguments T1 and T2 must be integral types.
*/
template< typename T1, typename T2 > inline T1 static_int_cast(T2 n) {
    return static_cast< T1 >(n);
}

}

#else /* __cplusplus */

/**
   A cast between integer types, to avoid C compiler warnings.

   In C source code, use SAL_INT_CAST(type, expr) instead of ((type) (expr))
   whenever a compiler warning about integer type problems shall be silenced.
   That way, source code that needs to be modified when the type of any of the
   expressions involved in the compiler warning is changed can be found more
   easily.

   The argument 'type' must be an integer type and the argument 'expr' must be
   an integer expression.  Both arguments are evaluated exactly once.
*/
#define SAL_INT_CAST(type, expr) ((type) (expr))

#endif /* __cplusplus */

#endif /*_SAL_TYPES_H_ */

