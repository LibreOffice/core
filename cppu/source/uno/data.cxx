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


#include <cstddef>
#include <stdio.h>

#include "cppu/macros.hxx"
#include "osl/mutex.hxx"
#include "sal/log.hxx"
#include "uno/data.h"

#include "constr.hxx"
#include "destr.hxx"
#include "copy.hxx"
#include "assign.hxx"
#include "eq.hxx"

using namespace ::cppu;
using namespace ::osl;

namespace cppu
{

// Sequence<>() (default ctor) relies on this being static:
uno_Sequence g_emptySeq = { 1, 0, { 0 } };
typelib_TypeDescriptionReference * g_pVoidType = nullptr;


void * binuno_queryInterface( void * pUnoI, typelib_TypeDescriptionReference * pDestType )
{
    // init queryInterface() td
    static typelib_TypeDescription * g_pQITD = nullptr;
    if (nullptr == g_pQITD)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (nullptr == g_pQITD)
        {
            typelib_TypeDescriptionReference * type_XInterface =
                * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );
            typelib_InterfaceTypeDescription * pTXInterfaceDescr = nullptr;
            TYPELIB_DANGER_GET( reinterpret_cast<typelib_TypeDescription **>(&pTXInterfaceDescr), type_XInterface );
            assert(pTXInterfaceDescr->ppAllMembers);
            typelib_typedescriptionreference_getDescription(
                &g_pQITD, pTXInterfaceDescr->ppAllMembers[ 0 ] );
            TYPELIB_DANGER_RELEASE( &pTXInterfaceDescr->aBase );
        }
    }

    uno_Any aRet, aExc;
    uno_Any * pExc = &aExc;
    void * aArgs[ 1 ];
    aArgs[ 0 ] = &pDestType;
    (*static_cast<uno_Interface *>(pUnoI)->pDispatcher)(
        static_cast<uno_Interface *>(pUnoI), g_pQITD, &aRet, aArgs, &pExc );

    uno_Interface * ret = nullptr;
    if (nullptr == pExc)
    {
        typelib_TypeDescriptionReference * ret_type = aRet.pType;
        switch (ret_type->eTypeClass)
        {
        case typelib_TypeClass_VOID: // common case
            typelib_typedescriptionreference_release( ret_type );
            break;
        case typelib_TypeClass_INTERFACE:
            // tweaky... avoiding acquire/ release pair
            typelib_typedescriptionreference_release( ret_type );
            ret = static_cast<uno_Interface *>(aRet.pReserved); // serving acquired interface
            break;
        default:
            _destructAny( &aRet, nullptr );
            break;
        }
    }
    else
    {
        SAL_WARN(
            "cppu",
            "exception occurred querying for interface "
                << OUString(pDestType->pTypeName) << ": ["
                << OUString(pExc->pType->pTypeName) << "] "
                << *static_cast<OUString const *>(pExc->pData));
                    // Message is very first member
        uno_any_destruct( pExc, nullptr );
    }
    return ret;
}


void defaultConstructStruct(
    void * pMem,
    typelib_CompoundTypeDescription * pCompType )
{
    _defaultConstructStruct( pMem, pCompType );
}

void copyConstructStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    _copyConstructStruct( pDest, pSource, pTypeDescr, acquire, mapping );
}

void destructStruct(
    void * pValue,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
{
    _destructStruct( pValue, pTypeDescr, release );
}

bool equalStruct(
    void * pDest, void *pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
{
    return _equalStruct( pDest, pSource, pTypeDescr, queryInterface, release );
}

bool assignStruct(
    void * pDest, void * pSource,
    typelib_CompoundTypeDescription * pTypeDescr,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
{
    return _assignStruct( pDest, pSource, pTypeDescr, queryInterface, acquire, release );
}


uno_Sequence * copyConstructSequence(
    uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_AcquireFunc acquire, uno_Mapping * mapping )
{
    return icopyConstructSequence( pSource, pElementType, acquire, mapping );
}


void destructSequence(
    uno_Sequence * pSequence,
    typelib_TypeDescriptionReference * pType,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
{
    idestructSequence( pSequence, pType, pTypeDescr, release );
}


bool equalSequence(
    uno_Sequence * pDest, uno_Sequence * pSource,
    typelib_TypeDescriptionReference * pElementType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
{
    return _equalSequence( pDest, pSource, pElementType, queryInterface, release );
}

}

extern "C"
{

void SAL_CALL uno_type_constructData(
    void * pMem, typelib_TypeDescriptionReference * pType )
    SAL_THROW_EXTERN_C()
{
    _defaultConstructData( pMem, pType, nullptr );
}

void SAL_CALL uno_constructData(
    void * pMem, typelib_TypeDescription * pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    _defaultConstructData( pMem, pTypeDescr->pWeakRef, pTypeDescr );
}

void SAL_CALL uno_type_destructData(
    void * pValue, typelib_TypeDescriptionReference * pType,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructData( pValue, pType, nullptr, release );
}

void SAL_CALL uno_destructData(
    void * pValue,
    typelib_TypeDescription * pTypeDescr,
    uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    _destructData( pValue, pTypeDescr->pWeakRef, pTypeDescr, release );
}

void SAL_CALL uno_type_copyData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    _copyConstructData( pDest, pSource, pType, nullptr, acquire, nullptr );
}

void SAL_CALL uno_copyData(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_AcquireFunc acquire )
    SAL_THROW_EXTERN_C()
{
    _copyConstructData( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, acquire, nullptr );
}

void SAL_CALL uno_type_copyAndConvertData(
    void * pDest, void * pSource,
    typelib_TypeDescriptionReference * pType,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    _copyConstructData( pDest, pSource, pType, nullptr, nullptr, mapping );
}

void SAL_CALL uno_copyAndConvertData(
    void * pDest, void * pSource,
    typelib_TypeDescription * pTypeDescr,
    uno_Mapping * mapping )
    SAL_THROW_EXTERN_C()
{
    _copyConstructData( pDest, pSource, pTypeDescr->pWeakRef, pTypeDescr, nullptr, mapping );
}

sal_Bool SAL_CALL uno_type_equalData(
    void * pVal1, typelib_TypeDescriptionReference * pVal1Type,
    void * pVal2, typelib_TypeDescriptionReference * pVal2Type,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return _equalData(
        pVal1, pVal1Type, nullptr,
        pVal2, pVal2Type,
        queryInterface, release );
}

sal_Bool SAL_CALL uno_equalData(
    void * pVal1, typelib_TypeDescription * pVal1TD,
    void * pVal2, typelib_TypeDescription * pVal2TD,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return _equalData(
        pVal1, pVal1TD->pWeakRef, pVal1TD,
        pVal2, pVal2TD->pWeakRef,
        queryInterface, release );
}

sal_Bool SAL_CALL uno_type_assignData(
    void * pDest, typelib_TypeDescriptionReference * pDestType,
    void * pSource, typelib_TypeDescriptionReference * pSourceType,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return _assignData(
        pDest, pDestType, nullptr,
        pSource, pSourceType, nullptr,
        queryInterface, acquire, release );
}

sal_Bool SAL_CALL uno_assignData(
    void * pDest, typelib_TypeDescription * pDestTD,
    void * pSource, typelib_TypeDescription * pSourceTD,
    uno_QueryInterfaceFunc queryInterface, uno_AcquireFunc acquire, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    return _assignData(
        pDest, pDestTD->pWeakRef, pDestTD,
        pSource, pSourceTD->pWeakRef, pSourceTD,
        queryInterface, acquire, release );
}

sal_Bool SAL_CALL uno_type_isAssignableFromData(
    typelib_TypeDescriptionReference * pAssignable,
    void * pFrom, typelib_TypeDescriptionReference * pFromType,
    uno_QueryInterfaceFunc queryInterface, uno_ReleaseFunc release )
    SAL_THROW_EXTERN_C()
{
    if (::typelib_typedescriptionreference_isAssignableFrom( pAssignable, pFromType ))
        return sal_True;
    if (typelib_TypeClass_INTERFACE != pFromType->eTypeClass ||
        typelib_TypeClass_INTERFACE != pAssignable->eTypeClass)
    {
        return sal_False;
    }

    // query
    if (nullptr == pFrom)
        return sal_False;
    void * pInterface = *static_cast<void **>(pFrom);
    if (nullptr == pInterface)
        return sal_False;

    if (nullptr == queryInterface)
        queryInterface = binuno_queryInterface;
    void * p = (*queryInterface)( pInterface, pAssignable );
    _release( p, release );
    return (nullptr != p);
}

}

#if OSL_DEBUG_LEVEL > 1

namespace cppu {

#if defined( SAL_W32)
#pragma pack(push, 8)
#endif

// Why hardcode like this instead of using the (generated)
// <sal/typesizes.h> ?

#if (defined(INTEL) \
    && (defined(__GNUC__) && (defined(LINUX) || defined(FREEBSD) || defined(NETBSD) || defined(OPENBSD)) \
        || defined(MACOSX) || defined(DRAGONFLY))) \
    || defined(IOS)
#define MAX_ALIGNMENT_4
#endif

#define OFFSET_OF( s, m ) reinterpret_cast< size_t >((char *)&((s *)16)->m -16)

#define BINTEST_VERIFY( c ) \
    if (! (c))      \
    {               \
        fprintf( stderr, "### binary compatibility test failed: %s [line %d]!!!\n", #c, __LINE__ ); \
        abort();    \
    }

#define BINTEST_VERIFYOFFSET( s, m, n ) \
    if (OFFSET_OF(s, m) != static_cast<size_t>(n))  \
    {                                               \
        fprintf(stderr, "### OFFSET_OF(" #s ", "  #m ") = %" SAL_PRI_SIZET "u instead of expected %" SAL_PRI_SIZET "u!!!\n", \
            OFFSET_OF(s, m), static_cast<size_t>(n));                    \
        abort();                                    \
    }

#define BINTEST_VERIFYSIZE( s, n ) \
    if (sizeof(s) != static_cast<size_t>(n))        \
    {                                               \
        fprintf(stderr, "### sizeof(" #s ") = %" SAL_PRI_SIZET "u instead of expected %" SAL_PRI_SIZET "u!!!\n", \
            sizeof(s), static_cast<size_t>(n));    \
        abort(); \
    }

struct C1
{
    sal_Int16 n1;
};
struct C2 : public C1
{
    sal_Int32 n2 CPPU_GCC3_ALIGN( C1 );
};
struct C3 : public C2
{
    double d3;
    sal_Int32 n3;
};
struct C4 : public C3
{
    sal_Int32 n4 CPPU_GCC3_ALIGN( C3 );
    double d4;
};
struct C5 : public C4
{
    sal_Int64 n5;
    sal_Bool b5;
};
struct C6 : public C1
{
    C5 c6 CPPU_GCC3_ALIGN( C1 );
    sal_Bool b6;
};

struct D
{
    sal_Int16 d;
    sal_Int32 e;
};
struct E
{
    sal_Bool a;
    sal_Bool b;
    sal_Bool c;
    sal_Int16 d;
    sal_Int32 e;
};

struct M
{
    sal_Int32   n;
    sal_Int16   o;
};

struct N : public M
{
    sal_Int16   p CPPU_GCC3_ALIGN( M );
};
struct N2
{
    M m;
    sal_Int16   p;
};

struct O : public M
{
    double  p;
    sal_Int16 q;
};
struct O2 : public O
{
    sal_Int16 p2 CPPU_GCC3_ALIGN( O );
};

struct P : public N
{
    double  p2;
};

struct empty
{
};
struct second : public empty
{
    int a;
};

struct AlignSize_Impl
{
    sal_Int16   nInt16;
    double      dDouble;
};

struct Char1
{
    char c1;
};
struct Char2 : public Char1
{
    char c2 CPPU_GCC3_ALIGN( Char1 );
};
struct Char3 : public Char2
{
    char c3 CPPU_GCC3_ALIGN( Char2 );
};
struct Char4
{
    Char3 chars;
    char c;
};
class Ref
{
    void * p;
};
enum Enum
{
    v = SAL_MAX_ENUM
};


class BinaryCompatible_Impl
{
public:
    BinaryCompatible_Impl();
};
BinaryCompatible_Impl::BinaryCompatible_Impl()
{
    static_assert( ((sal_Bool) true) == sal_True &&
                         (1 != 0) == sal_True, "must be binary compatible" );
    static_assert( ((sal_Bool) false) == sal_False &&
                         (1 == 0) == sal_False, "must be binary compatible" );
#ifdef MAX_ALIGNMENT_4
    // max alignment is 4
    BINTEST_VERIFYOFFSET( AlignSize_Impl, dDouble, 4 );
    BINTEST_VERIFYSIZE( AlignSize_Impl, 12 );
#else
    // max alignment is 8
    BINTEST_VERIFYOFFSET( AlignSize_Impl, dDouble, 8 );
    BINTEST_VERIFYSIZE( AlignSize_Impl, 16 );
#endif

    // sequence
    BINTEST_VERIFY( (SAL_SEQUENCE_HEADER_SIZE % 8) == 0 );
    // enum
    BINTEST_VERIFY( sizeof( Enum ) == sizeof( sal_Int32 ) );
    // any
    BINTEST_VERIFY( sizeof(void *) >= sizeof(sal_Int32) );
    BINTEST_VERIFY( sizeof( uno_Any ) == sizeof( void * ) * 3 );
    BINTEST_VERIFYOFFSET( uno_Any, pType, 0 );
    BINTEST_VERIFYOFFSET( uno_Any, pData, 1 * sizeof (void *) );
    BINTEST_VERIFYOFFSET( uno_Any, pReserved, 2 * sizeof (void *) );
    // interface
    BINTEST_VERIFY( sizeof( Ref ) == sizeof( void * ) );
    // string
    BINTEST_VERIFY( sizeof( OUString ) == sizeof( rtl_uString * ) );
    // struct
    BINTEST_VERIFYSIZE( M, 8 );
    BINTEST_VERIFYOFFSET( M, o, 4 );
    BINTEST_VERIFYSIZE( N, 12 );
    BINTEST_VERIFYOFFSET( N, p, 8 );
    BINTEST_VERIFYSIZE( N2, 12 );
    BINTEST_VERIFYOFFSET( N2, p, 8 );
#ifdef MAX_ALIGNMENT_4
    BINTEST_VERIFYSIZE( O, 20 );
#else
    BINTEST_VERIFYSIZE( O, 24 );
#endif
    BINTEST_VERIFYSIZE( D, 8 );
    BINTEST_VERIFYOFFSET( D, e, 4 );
    BINTEST_VERIFYOFFSET( E, d, 4 );
    BINTEST_VERIFYOFFSET( E, e, 8 );

    BINTEST_VERIFYSIZE( C1, 2 );
    BINTEST_VERIFYSIZE( C2, 8 );
    BINTEST_VERIFYOFFSET( C2, n2, 4 );

#ifdef MAX_ALIGNMENT_4
    BINTEST_VERIFYSIZE( C3, 20 );
    BINTEST_VERIFYOFFSET( C3, d3, 8 );
    BINTEST_VERIFYOFFSET( C3, n3, 16 );
    BINTEST_VERIFYSIZE( C4, 32 );
    BINTEST_VERIFYOFFSET( C4, n4, 20 );
    BINTEST_VERIFYOFFSET( C4, d4, 24 );
    BINTEST_VERIFYSIZE( C5, 44 );
    BINTEST_VERIFYOFFSET( C5, n5, 32 );
    BINTEST_VERIFYOFFSET( C5, b5, 40 );
    BINTEST_VERIFYSIZE( C6, 52 );
    BINTEST_VERIFYOFFSET( C6, c6, 4 );
    BINTEST_VERIFYOFFSET( C6, b6, 48 );

    BINTEST_VERIFYSIZE( O2, 24 );
    BINTEST_VERIFYOFFSET( O2, p2, 20 );
#else
    BINTEST_VERIFYSIZE( C3, 24 );
    BINTEST_VERIFYOFFSET( C3, d3, 8 );
    BINTEST_VERIFYOFFSET( C3, n3, 16 );
    BINTEST_VERIFYSIZE( C4, 40 );
    BINTEST_VERIFYOFFSET( C4, n4, 24 );
    BINTEST_VERIFYOFFSET( C4, d4, 32 );
    BINTEST_VERIFYSIZE( C5, 56 );
    BINTEST_VERIFYOFFSET( C5, n5, 40 );
    BINTEST_VERIFYOFFSET( C5, b5, 48 );
    BINTEST_VERIFYSIZE( C6, 72 );
    BINTEST_VERIFYOFFSET( C6, c6, 8 );
    BINTEST_VERIFYOFFSET( C6, b6, 64 );

    BINTEST_VERIFYSIZE( O2, 32 );
    BINTEST_VERIFYOFFSET( O2, p2, 24 );
#endif

    BINTEST_VERIFYSIZE( Char3, 3 );
    BINTEST_VERIFYOFFSET( Char4, c, 3 );

#ifdef MAX_ALIGNMENT_4
    // max alignment is 4
    BINTEST_VERIFYSIZE( P, 20 );
#else
    // alignment of P is 8, because of P[] ...
    BINTEST_VERIFYSIZE( P, 24 );
    BINTEST_VERIFYSIZE( second, sizeof( int ) );
#endif
}

#ifdef SAL_W32
#   pragma pack(pop)
#endif

static BinaryCompatible_Impl aTest;

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
