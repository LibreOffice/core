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

#ifndef _SAL_TYPES_H_
#define _SAL_TYPES_H_

#include <sal/config.h>
#include <sal/macros.h>

#include <sal/typesizes.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined ( __MINGW32__ ) && !defined ( __USE_MINGW_ANSI_STDIO )
/* Define to use the C99 formating string for coherence reasons.
 * In mingw-w64 some functions are ported to the ms formating string
 * some are not yet. This is the only way to make the formatting
 * strings work all the time
 */
#define __USE_MINGW_ANSI_STDIO 1
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

#ifdef _MSC_VER
    typedef __int64                  sal_Int64;
    typedef unsigned __int64         sal_uInt64;

    /*  The following are macros that will add the 64 bit constant suffix. */
    #define SAL_CONST_INT64(x)       x##i64
    #define SAL_CONST_UINT64(x)      x##ui64

    #define SAL_PRIdINT64 "I64d"
    #define SAL_PRIuUINT64 "I64u"
    #define SAL_PRIxUINT64 "I64x"
    #define SAL_PRIXUINT64 "I64X"
#elif defined(__SUNPRO_CC) || defined(__SUNPRO_C) || defined (__GNUC__) || defined (sgi)
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

        #define SAL_PRIdINT64 "lld"
        #define SAL_PRIuUINT64 "llu"
        #define SAL_PRIxUINT64 "llx"
        #define SAL_PRIXUINT64 "llX"
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
    // http://msdn.microsoft.com/en-us/library/s3f49ktz%28v=vs.80%29.aspx
    // "By default wchar_t is a typedef for unsigned short."
    // But MinGW has a native wchar_t, and on many places, we cannot deal with
    // that, so sal_Unicode has to be explicitly typedef'd as sal_uInt16 there.
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
#if defined(_MSC_VER)
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

#if defined(SAL_W32) || defined(SAL_UNX)
#   define SAL_MAX_ENUM 0x7fffffff
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#   define SAL_DLLPUBLIC_EXPORT    __declspec(dllexport)
#   define SAL_JNI_EXPORT          __declspec(dllexport)
#if defined(_MSC_VER)
#   define SAL_DLLPUBLIC_IMPORT    __declspec(dllimport)
#else
#   define SAL_DLLPUBLIC_IMPORT
#endif // defined(_MSC_VER)
#   define SAL_DLLPRIVATE
#   define SAL_DLLPUBLIC_TEMPLATE
#   define SAL_CALL         __cdecl
#   define SAL_CALL_ELLIPSE __cdecl
#elif defined SAL_UNX
#   if   defined(__SUNPRO_CC) && (__SUNPRO_CC >= 0x550)
#     define SAL_DLLPUBLIC_EXPORT  __global
#     define SAL_JNI_EXPORT        __global
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE        __hidden
#     define SAL_DLLPUBLIC_TEMPLATE
#   elif defined(__SUNPRO_C ) && (__SUNPRO_C  >= 0x550)
#     define SAL_DLLPUBLIC_EXPORT  __global
#     define SAL_JNI_EXPORT        __global
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE        __hidden
#     define SAL_DLLPUBLIC_TEMPLATE
#   elif defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
#     if defined(DISABLE_DYNLOADING)
#       define SAL_DLLPUBLIC_EXPORT  __attribute__ ((visibility("hidden")))
#       define SAL_JNI_EXPORT        __attribute__ ((visibility("default")))
#       define SAL_DLLPUBLIC_IMPORT  __attribute__ ((visibility("hidden")))
#       define SAL_DLLPRIVATE        __attribute__ ((visibility("hidden")))
#       define SAL_DLLPUBLIC_TEMPLATE __attribute__ ((visibility("hidden")))
#     else
#       define SAL_DLLPUBLIC_EXPORT  __attribute__ ((visibility("default")))
#       define SAL_JNI_EXPORT        __attribute__ ((visibility("default")))
#       define SAL_DLLPUBLIC_IMPORT  __attribute__ ((visibility("default")))
#       define SAL_DLLPRIVATE        __attribute__ ((visibility("hidden")))
#       define SAL_DLLPUBLIC_TEMPLATE __attribute__ ((visibility("default")))
#     endif
#   else
#     define SAL_DLLPUBLIC_EXPORT
#     define SAL_JNI_EXPORT
#     define SAL_DLLPUBLIC_IMPORT
#     define SAL_DLLPRIVATE
#     define SAL_DLLPUBLIC_TEMPLATE
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
#   if defined(DISABLE_DYNLOADING)
#      define SAL_EXCEPTION_DLLPUBLIC_EXPORT __attribute__((visibility("default")))
#    else
#      define SAL_EXCEPTION_DLLPUBLIC_EXPORT SAL_DLLPUBLIC_EXPORT
#    endif
#    define SAL_EXCEPTION_DLLPRIVATE SAL_DLLPRIVATE
#else
#    define SAL_EXCEPTION_DLLPUBLIC_EXPORT
#    define SAL_EXCEPTION_DLLPRIVATE
#endif

/** Use this as markup for functions and methods whose return value must be
    checked.

    Compilers that support a construct of this nature will emit a compile
    time warning on unchecked return value.
*/
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#   define SAL_WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#else
#   define SAL_WARN_UNUSED_RESULT
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

#if defined( SAL_W32)
#pragma pack(pop)
#endif

/** Definition of function throw clause macros.  These have been introduced
    to reduce code size by balancing out compiler bugs.

    These macros are ONLY for function declarations,
    use common C++ throw statement for throwing exceptions, e.g.
    throw RuntimeException();

    SAL_THROW()          should be used for all C++ functions, e.g. SAL_THROW(())
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
}
#endif /* __cplusplus */

#ifdef __cplusplus

enum __sal_NoAcquire
{
    /** definition of a no acquire enum for ctors
    */
    SAL_NO_ACQUIRE
};

namespace com { namespace sun { namespace star { } } }

/** short-circuit extra-verbose API namespaces

 @since LibreOffice 4.0
*/
namespace css = ::com::sun::star;

/** C++11 "= delete" feature.

    With HAVE_CXX11_DELETE, calling a deleted function will cause a compile-time
    error, while otherwise it will only cause a link-time error as the declared
    function is not defined.

    @since LibreOffice 4.1
*/
#if HAVE_CXX11_DELETE
#define SAL_DELETED_FUNCTION = delete
#else
#define SAL_DELETED_FUNCTION
#endif

/** C++11 "override" feature.

    With HAVE_CXX11_OVERRIDE, force the method to override a existing method in
    parent, error out if the method with the correct signature does not exist.

    @since LibreOffice 4.1
*/
#if HAVE_CXX11_OVERRIDE
#define SAL_OVERRIDE override
#else
#define SAL_OVERRIDE
#endif

/** C++11 "final" feature.

    With HAVE_CXX11_FINAL, mark a class as non-derivable or a method as non-overridable.

    @since LibreOffice 4.1
*/
#if HAVE_CXX11_FINAL
#define SAL_FINAL final
#else
#define SAL_FINAL
#endif

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

/**
    Use as follows:
        SAL_DEPRECATED("Dont use, its evil.") void doit(int nPara);
*/

#if HAVE_GCC_DEPRECATED_MESSAGE
#    define SAL_DEPRECATED(message) __attribute__((deprecated(message)))
#elif (__GNUC__)
#    define SAL_DEPRECATED(message) __attribute__((deprecated))
#elif defined(_MSC_VER)
#    define SAL_DEPRECATED(message) __declspec(deprecated(message))
#else
#    define SAL_DEPRECATED(message)
#endif

/**
   This macro is used to tag interfaces that are deprecated for both
   internal and external API users, but where we are still writing
   out the internal usage. Ultimately these should be replaced by
   SAL_DEPRECATED, and then removed.

   Use as follows:
        SAL_DEPRECATED_INTERNAL("Dont use, its evil.") void doit(int nPara);
 */
#ifdef LIBO_INTERNAL_ONLY
#    define SAL_DEPRECATED_INTERNAL(message)
#else
#    define SAL_DEPRECATED_INTERNAL(message) SAL_DEPRECATED(message)
#endif

/**
    Use as follows:
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        \::std::auto_ptr<X> ...
        SAL_WNODEPRECATED_DECLARATIONS_POP
*/

#if HAVE_GCC_PRAGMA_DIAGNOSTIC_MODIFY && HAVE_GCC_PRAGMA_DIAGNOSTIC_SCOPE && HAVE_GCC_PRAGMA_OPERATOR
#define SAL_WNODEPRECATED_DECLARATIONS_PUSH \
    _Pragma(SAL_STRINGIFY_ARG(GCC diagnostic push)) \
    _Pragma(SAL_STRINGIFY_ARG(GCC diagnostic ignored "-Wdeprecated-declarations"))
#define SAL_WNODEPRECATED_DECLARATIONS_POP \
    _Pragma(SAL_STRINGIFY_ARG(GCC diagnostic pop))
#else
#   define SAL_WNODEPRECATED_DECLARATIONS_PUSH
#   define SAL_WNODEPRECATED_DECLARATIONS_POP
#endif

/**
   Use as follows:

   SAL_WNOUNREACHABLE_CODE_PUSH

   function definition

   SAL_WNOUNREACHABLE_CODE_POP

   Useful in cases where the compiler is "too clever" like when doing
   link-time code generation, and noticing that a called function
   always throws, and fixing the problem cleanly so that it produceds
   no warnings in normal non-LTO compilations either is not easy.

*/

#ifdef _MSC_VER
#define SAL_WNOUNREACHABLE_CODE_PUSH \
    __pragma(warning(push)) \
    __pragma(warning(disable:4702))
#define SAL_WNOUNREACHABLE_CODE_POP \
    __pragma(warning(pop))
#else
/* Add definitions for GCC and Clang if needed */
#define SAL_WNOUNREACHABLE_CODE_PUSH
#define SAL_WNOUNREACHABLE_CODE_POP
#endif

/** Annotate unused but required C++ function parameters.

    An unused parameter is required if the function needs to adhere to a given
    type (e.g., if its address is assigned to a function pointer of a specific
    type, or if it is called from template code).  This annotation helps static
    analysis tools suppress false warnings.  In the case of virtual functions
    (where unused required parameters are common, too), the annotation is not
    required (as static analysis tools can themselves derive the information
    whether a function is virtual).

    Use the annotation in function definitions like

      void f(SAL_UNUSED_PARAMETER int) {}

    C does not allow unnamed parameters, anyway, so a function definition like
    the above needs to be written there as

      void f(int dummy) { (void) dummy; / * avoid warnings * / }

    without a SAL_UNUSED_PARAMETER annotation.

    @since LibreOffice 3.6
 */
#if defined __cplusplus
#if defined __GNUC__
#define SAL_UNUSED_PARAMETER __attribute__ ((unused))
#else
#define SAL_UNUSED_PARAMETER
#endif
#endif

/**

 Annotate classes where a compiler should warn if an instance is unused.

 The compiler cannot warn about unused instances if they have non-trivial
 or external constructors or destructors. Classes marked with SAL_WARN_UNUSED
 will be warned about.

 @since LibreOffice 4.0

*/

#if HAVE_GCC_ATTRIBUTE_WARN_UNUSED
#define SAL_WARN_UNUSED __attribute__((warn_unused))
#elif defined __clang__
#define SAL_WARN_UNUSED __attribute__((annotate("lo_warn_unused")))
#else
#define SAL_WARN_UNUSED
#endif

#endif /*_SAL_TYPES_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
