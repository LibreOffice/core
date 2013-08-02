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


#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

#include <boost/unordered_map.hpp>
#include <cassert>
#include <list>
#include <set>
#include <vector>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sal/alloca.h>
#include <new>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/alloc.h>
#include <rtl/instance.hxx>
#include <osl/diagnose.h>
#include <typelib/typedescription.h>
#include <uno/any2.h>

using namespace std;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::rtl::OString;

#ifdef SAL_W32
#pragma pack(push, 8)
#endif

/**
 * The double member determin the alignment.
 * Under Os2 and MS-Windows the Alignment is min( 8, sizeof( type ) ).
 * The aligment of a strukture is min( 8, sizeof( max basic type ) ), the greatest basic type
 * determine the aligment.
 */
struct AlignSize_Impl
{
    sal_Int16 nInt16;
#ifdef AIX
    //double: doubleword aligned if -qalign=natural/-malign=natural
    //which isn't the default ABI. Otherwise word aligned, While a long long int
    //is always doubleword aligned, so use that instead.
    sal_Int64 dDouble;
#else
    double dDouble;
#endif
};

#ifdef SAL_W32
#pragma pack(pop)
#endif

// the value of the maximal alignment
static sal_Int32 nMaxAlignment = (sal_Int32)( (sal_Size)(&((AlignSize_Impl *) 16)->dDouble) - 16);

static inline sal_Int32 adjustAlignment( sal_Int32 nRequestedAlignment )
    SAL_THROW(())
{
    if( nRequestedAlignment > nMaxAlignment )
        nRequestedAlignment = nMaxAlignment;
    return nRequestedAlignment;
}

/**
 * Calculate the new size of the struktur.
 */
static inline sal_Int32 newAlignedSize(
    sal_Int32 OldSize, sal_Int32 ElementSize, sal_Int32 NeededAlignment )
    SAL_THROW(())
{
    NeededAlignment = adjustAlignment( NeededAlignment );
    return (OldSize + NeededAlignment -1) / NeededAlignment * NeededAlignment + ElementSize;
}

static inline sal_Bool reallyWeak( typelib_TypeClass eTypeClass )
    SAL_THROW(())
{
    return TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( eTypeClass );
}

static inline sal_Int32 getDescriptionSize( typelib_TypeClass eTypeClass )
    SAL_THROW(())
{
    OSL_ASSERT( typelib_TypeClass_TYPEDEF != eTypeClass );

    sal_Int32 nSize;
    // The reference is the description
    // if the description is empty, than it must be filled with
    // the new description
    switch( eTypeClass )
    {
        case typelib_TypeClass_ARRAY:
            nSize = (sal_Int32)sizeof( typelib_ArrayTypeDescription );
        break;

        case typelib_TypeClass_SEQUENCE:
            nSize = (sal_Int32)sizeof( typelib_IndirectTypeDescription );
        break;

        case typelib_TypeClass_UNION:
            nSize = (sal_Int32)sizeof( typelib_UnionTypeDescription );
        break;

        case typelib_TypeClass_STRUCT:
            nSize = (sal_Int32)sizeof( typelib_StructTypeDescription );
        break;

        case typelib_TypeClass_EXCEPTION:
            nSize = (sal_Int32)sizeof( typelib_CompoundTypeDescription );
        break;

        case typelib_TypeClass_ENUM:
            nSize = (sal_Int32)sizeof( typelib_EnumTypeDescription );
        break;

        case typelib_TypeClass_INTERFACE:
            nSize = (sal_Int32)sizeof( typelib_InterfaceTypeDescription );
        break;

        case typelib_TypeClass_INTERFACE_METHOD:
            nSize = (sal_Int32)sizeof( typelib_InterfaceMethodTypeDescription );
        break;

        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            nSize = (sal_Int32)sizeof( typelib_InterfaceAttributeTypeDescription );
        break;

        default:
            nSize = (sal_Int32)sizeof( typelib_TypeDescription );
    }
    return nSize;
}


//-----------------------------------------------------------------------------
extern "C" void SAL_CALL typelib_typedescriptionreference_getByName(
    typelib_TypeDescriptionReference ** ppRet, rtl_uString * pName )
    SAL_THROW_EXTERN_C();

//-----------------------------------------------------------------------------
struct equalStr_Impl
{
    sal_Bool operator()(const sal_Unicode * const & s1, const sal_Unicode * const & s2) const SAL_THROW(())
        { return 0 == rtl_ustr_compare( s1, s2 ); }
};

//-----------------------------------------------------------------------------
struct hashStr_Impl
{
    size_t operator()(const sal_Unicode * const & s) const SAL_THROW(())
        { return rtl_ustr_hashCode( s ); }
};


//-----------------------------------------------------------------------------
// Heavy hack, the const sal_Unicode * is hold by the typedescription reference
typedef boost::unordered_map< const sal_Unicode *, typelib_TypeDescriptionReference *,
                  hashStr_Impl, equalStr_Impl > WeakMap_Impl;

typedef pair< void *, typelib_typedescription_Callback > CallbackEntry;
typedef list< CallbackEntry > CallbackSet_Impl;
typedef list< typelib_TypeDescription * > TypeDescriptionList_Impl;

// # of cached elements
static sal_Int32 nCacheSize = 256;

struct TypeDescriptor_Init_Impl
{
    //sal_Bool          bDesctructorCalled;
    // all type description references
    WeakMap_Impl *              pWeakMap;
    // all type description callbacks
    CallbackSet_Impl *          pCallbacks;
    // A cache to hold descriptions
    TypeDescriptionList_Impl *  pCache;
    // The mutex to guard all type library accesses
    Mutex *                     pMutex;

    inline Mutex & getMutex() SAL_THROW(());

    inline void callChain( typelib_TypeDescription ** ppRet, rtl_uString * pName ) SAL_THROW(());

#if OSL_DEBUG_LEVEL > 1
    // only for debugging
    sal_Int32           nTypeDescriptionCount;
    sal_Int32           nCompoundTypeDescriptionCount;
    sal_Int32           nUnionTypeDescriptionCount;
    sal_Int32           nIndirectTypeDescriptionCount;
    sal_Int32           nArrayTypeDescriptionCount;
    sal_Int32           nEnumTypeDescriptionCount;
    sal_Int32           nInterfaceMethodTypeDescriptionCount;
    sal_Int32           nInterfaceAttributeTypeDescriptionCount;
    sal_Int32           nInterfaceTypeDescriptionCount;
    sal_Int32           nTypeDescriptionReferenceCount;
#endif

    TypeDescriptor_Init_Impl():
        pWeakMap(0), pCallbacks(0), pCache(0), pMutex(0)
#if OSL_DEBUG_LEVEL > 1
        , nTypeDescriptionCount(0), nCompoundTypeDescriptionCount(0),
        nUnionTypeDescriptionCount(0), nIndirectTypeDescriptionCount(0),
        nArrayTypeDescriptionCount(0), nEnumTypeDescriptionCount(0),
        nInterfaceMethodTypeDescriptionCount(0),
        nInterfaceAttributeTypeDescriptionCount(0),
        nInterfaceTypeDescriptionCount(0), nTypeDescriptionReferenceCount(0)
#endif
    {}

    ~TypeDescriptor_Init_Impl() SAL_THROW(());
};
//__________________________________________________________________________________________________
inline Mutex & TypeDescriptor_Init_Impl::getMutex() SAL_THROW(())
{
    if( !pMutex )
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if( !pMutex )
            pMutex = new Mutex();
    }
    return * pMutex;
}
//__________________________________________________________________________________________________
inline void TypeDescriptor_Init_Impl::callChain(
    typelib_TypeDescription ** ppRet, rtl_uString * pName )
    SAL_THROW(())
{
    assert(ppRet != 0);
    assert(*ppRet == 0);
    if (pCallbacks)
    {
        CallbackSet_Impl::const_iterator aIt = pCallbacks->begin();
        while( aIt != pCallbacks->end() )
        {
            const CallbackEntry & rEntry = *aIt;
            (*rEntry.second)( rEntry.first, ppRet, pName );
            if( *ppRet )
                return;
            ++aIt;
        }
    }
}

//__________________________________________________________________________________________________
TypeDescriptor_Init_Impl::~TypeDescriptor_Init_Impl() SAL_THROW(())
{
    if( pCache )
    {
        TypeDescriptionList_Impl::const_iterator aIt = pCache->begin();
        while( aIt != pCache->end() )
        {
            typelib_typedescription_release( (*aIt) );
            ++aIt;
        }
        delete pCache;
        pCache = 0;
    }

    if( pWeakMap )
    {
        std::vector< typelib_TypeDescriptionReference * > ppTDR;
        // save al weak references
        WeakMap_Impl::const_iterator aIt = pWeakMap->begin();
        while( aIt != pWeakMap->end() )
        {
            ppTDR.push_back( (*aIt).second );
            typelib_typedescriptionreference_acquire( ppTDR.back() );
            ++aIt;
        }

        for( std::vector< typelib_TypeDescriptionReference * >::iterator i(
                 ppTDR.begin() );
             i != ppTDR.end(); ++i )
        {
            typelib_TypeDescriptionReference * pTDR = *i;
            OSL_ASSERT( pTDR->nRefCount > pTDR->nStaticRefCount );
            pTDR->nRefCount -= pTDR->nStaticRefCount;

            if( pTDR->pType && !pTDR->pType->bOnDemand )
            {
                pTDR->pType->bOnDemand = sal_True;
                typelib_typedescription_release( pTDR->pType );
            }
            typelib_typedescriptionreference_release( pTDR );
        }

#if OSL_DEBUG_LEVEL > 1
        aIt = pWeakMap->begin();
        while( aIt != pWeakMap->end() )
        {
            typelib_TypeDescriptionReference * pTDR = (*aIt).second;
            if (pTDR)
            {
                OString aTypeName( rtl::OUStringToOString( pTDR->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
                OSL_TRACE(
                    "### remaining type: %s; ref count = %d", aTypeName.getStr(), pTDR->nRefCount );
            }
            else
            {
                OSL_TRACE( "### remaining null type entry!?" );
            }
            ++aIt;
        }
#endif

        delete pWeakMap;
        pWeakMap = 0;
    }
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( !nTypeDescriptionCount, "### nTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nCompoundTypeDescriptionCount, "### nCompoundTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nUnionTypeDescriptionCount, "### nUnionTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nIndirectTypeDescriptionCount, "### nIndirectTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nArrayTypeDescriptionCount, "### nArrayTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nEnumTypeDescriptionCount, "### nEnumTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nInterfaceMethodTypeDescriptionCount, "### nInterfaceMethodTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nInterfaceAttributeTypeDescriptionCount, "### nInterfaceAttributeTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nInterfaceTypeDescriptionCount, "### nInterfaceTypeDescriptionCount is not zero" );
    OSL_ENSURE( !nTypeDescriptionReferenceCount, "### nTypeDescriptionReferenceCount is not zero" );

    OSL_ENSURE( !pCallbacks || pCallbacks->empty(), "### pCallbacks is not NULL or empty" );
#endif

    delete pCallbacks;
    pCallbacks = 0;

    if( pMutex )
    {
        delete pMutex;
        pMutex = 0;
    }
};

namespace { struct Init : public rtl::Static< TypeDescriptor_Init_Impl, Init > {}; }

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_registerCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
    SAL_THROW_EXTERN_C()
{
    // todo mt safe: guard is no solution, can not acquire while calling callback!
    TypeDescriptor_Init_Impl &rInit = Init::get();
//      OslGuard aGuard( rInit.getMutex() );
    if( !rInit.pCallbacks )
        rInit.pCallbacks = new CallbackSet_Impl;
    rInit.pCallbacks->push_back( CallbackEntry( pContext, pCallback ) );
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_revokeCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
    SAL_THROW_EXTERN_C()
{
    TypeDescriptor_Init_Impl &rInit = Init::get();
    if( rInit.pCallbacks )
    {
        // todo mt safe: guard is no solution, can not acquire while calling callback!
//          OslGuard aGuard( rInit.getMutex() );
        CallbackEntry aEntry( pContext, pCallback );
        CallbackSet_Impl::iterator iPos( rInit.pCallbacks->begin() );
        while (!(iPos == rInit.pCallbacks->end()))
        {
            if (*iPos == aEntry)
            {
                rInit.pCallbacks->erase( iPos );
                iPos = rInit.pCallbacks->begin();
            }
            else
            {
                ++iPos;
            }
        }
    }
}

extern "C" sal_Int32 SAL_CALL typelib_typedescription_getAlignedUnoSize(
    const typelib_TypeDescription * pTypeDescription,
    sal_Int32 nOffset, sal_Int32 & rMaxIntegralTypeSize )
    SAL_THROW_EXTERN_C();

//------------------------------------------------------------------------
static inline void typelib_typedescription_initTables(
    typelib_TypeDescription * pTD )
    SAL_THROW(())
{
    typelib_InterfaceTypeDescription * pITD = (typelib_InterfaceTypeDescription *)pTD;

    sal_Bool * pReadWriteAttributes = (sal_Bool *)alloca( pITD->nAllMembers );
    for ( sal_Int32 i = pITD->nAllMembers; i--; )
    {
        pReadWriteAttributes[i] = sal_False;
        if( typelib_TypeClass_INTERFACE_ATTRIBUTE == pITD->ppAllMembers[i]->eTypeClass )
        {
            typelib_TypeDescription * pM = 0;
            TYPELIB_DANGER_GET( &pM, pITD->ppAllMembers[i] );
            OSL_ASSERT( pM );
            if (pM)
            {
                pReadWriteAttributes[i] = !((typelib_InterfaceAttributeTypeDescription *)pM)->bReadOnly;
                TYPELIB_DANGER_RELEASE( pM );
            }
#if OSL_DEBUG_LEVEL > 1
            else
            {
                OString aStr( rtl::OUStringToOString( pITD->ppAllMembers[i]->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
                OSL_TRACE( "\n### cannot get attribute type description: %s", aStr.getStr() );
            }
#endif
        }
    }

    MutexGuard aGuard( Init::get().getMutex() );
    if( !pTD->bComplete )
    {
        // create the index table from member to function table
        pITD->pMapMemberIndexToFunctionIndex = new sal_Int32[ pITD->nAllMembers ];
        sal_Int32 nAdditionalOffset = 0; // +1 for read/write attributes
        sal_Int32 i;
        for( i = 0; i < pITD->nAllMembers; i++ )
        {
            // index to the get method of the attribute
            pITD->pMapMemberIndexToFunctionIndex[i] = i + nAdditionalOffset;
            // extra offset if it is a read/write attribute?
            if( pReadWriteAttributes[i] )
            {
                // a read/write attribute
                nAdditionalOffset++;
            }
        }

        // create the index table from function to member table
        pITD->pMapFunctionIndexToMemberIndex = new sal_Int32[ pITD->nAllMembers + nAdditionalOffset ];
        nAdditionalOffset = 0; // +1 for read/write attributes
        for( i = 0; i < pITD->nAllMembers; i++ )
        {
            // index to the get method of the attribute
            pITD->pMapFunctionIndexToMemberIndex[i + nAdditionalOffset] = i;
            // extra offset if it is a read/write attribute?
            if( pReadWriteAttributes[i] )
            {
                // a read/write attribute
                pITD->pMapFunctionIndexToMemberIndex[i + ++nAdditionalOffset] = i;
            }
        }
        // must be the last action after all initialization is done
        pITD->nMapFunctionIndexToMemberIndex = pITD->nAllMembers + nAdditionalOffset;
        pTD->bComplete = sal_True;
    }
}

namespace {

// In some situations (notably typelib_typedescription_newInterfaceMethod and
// typelib_typedescription_newInterfaceAttribute), only the members nMembers,
// ppMembers, nAllMembers, and ppAllMembers of an incomplete interface type
// description are necessary, but not the additional
// pMapMemberIndexToFunctionIndex, nMapFunctionIndexToMemberIndex, and
// pMapFunctionIndexToMemberIndex (which are computed by
// typelib_typedescription_initTables).  Furthermore, in those situations, it
// might be illegal to compute those tables, as the creation of the interface
// member type descriptions would recursively require a complete interface type
// description.  The parameter initTables controls whether or not to call
// typelib_typedescription_initTables in those situations.
bool complete(typelib_TypeDescription ** ppTypeDescr, bool initTables) {
    if (! (*ppTypeDescr)->bComplete)
    {
        OSL_ASSERT( (typelib_TypeClass_STRUCT == (*ppTypeDescr)->eTypeClass ||
                     typelib_TypeClass_EXCEPTION == (*ppTypeDescr)->eTypeClass ||
                     typelib_TypeClass_UNION == (*ppTypeDescr)->eTypeClass ||
                     typelib_TypeClass_ENUM == (*ppTypeDescr)->eTypeClass ||
                     typelib_TypeClass_INTERFACE == (*ppTypeDescr)->eTypeClass) &&
                    !reallyWeak( (*ppTypeDescr)->eTypeClass ) );

        if (typelib_TypeClass_INTERFACE == (*ppTypeDescr)->eTypeClass &&
            ((typelib_InterfaceTypeDescription *)*ppTypeDescr)->ppAllMembers)
        {
            if (initTables) {
                typelib_typedescription_initTables( *ppTypeDescr );
            }
            return true;
        }

        typelib_TypeDescription * pTD = 0;
        // on demand access of complete td
        TypeDescriptor_Init_Impl &rInit = Init::get();
        rInit.callChain( &pTD, (*ppTypeDescr)->pTypeName );
        if (pTD)
        {
            if (typelib_TypeClass_TYPEDEF == pTD->eTypeClass)
            {
                typelib_typedescriptionreference_getDescription(
                    &pTD, ((typelib_IndirectTypeDescription *)pTD)->pType );
                OSL_ASSERT( pTD );
                if (! pTD)
                    return false;
            }

            OSL_ASSERT( typelib_TypeClass_TYPEDEF != pTD->eTypeClass );
            // typedescription found
            // set to on demand
            pTD->bOnDemand = sal_True;

            if (pTD->eTypeClass == typelib_TypeClass_INTERFACE
                && !pTD->bComplete && initTables)
            {
                // mandatory info from callback chain
                OSL_ASSERT( ((typelib_InterfaceTypeDescription *)pTD)->ppAllMembers );
                // complete except of tables init
                typelib_typedescription_initTables( pTD );
                pTD->bComplete = sal_True;
            }

            // The type description is hold by the reference until
            // on demand is activated.
            ::typelib_typedescription_register( &pTD ); // replaces incomplete one
            OSL_ASSERT( pTD == *ppTypeDescr ); // has to merge into existing one

            // insert into the chache
            MutexGuard aGuard( rInit.getMutex() );
            if( !rInit.pCache )
                rInit.pCache = new TypeDescriptionList_Impl;
            if( (sal_Int32)rInit.pCache->size() >= nCacheSize )
            {
                typelib_typedescription_release( rInit.pCache->front() );
                rInit.pCache->pop_front();
            }
            // descriptions in the cache must be acquired!
            typelib_typedescription_acquire( pTD );
            rInit.pCache->push_back( pTD );

            OSL_ASSERT(
                pTD->bComplete
                || (pTD->eTypeClass == typelib_TypeClass_INTERFACE
                    && !initTables));

            ::typelib_typedescription_release( *ppTypeDescr );
            *ppTypeDescr = pTD;
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            OString aStr(
                rtl::OUStringToOString( (*ppTypeDescr)->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
            OSL_TRACE( "\n### type cannot be completed: %s", aStr.getStr() );
#endif
            return false;
        }
    }
    return true;
}

}

//------------------------------------------------------------------------
extern "C" void SAL_CALL typelib_typedescription_newEmpty(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass, rtl_uString * pTypeName )
    SAL_THROW_EXTERN_C()
{
    if( *ppRet )
    {
        typelib_typedescription_release( *ppRet );
        *ppRet = 0;
    }

    OSL_ASSERT( typelib_TypeClass_TYPEDEF != eTypeClass );

    typelib_TypeDescription * pRet;
    switch( eTypeClass )
    {
        case typelib_TypeClass_ARRAY:
        {
            typelib_ArrayTypeDescription * pTmp = new typelib_ArrayTypeDescription();
            typelib_IndirectTypeDescription * pIndirect = (typelib_IndirectTypeDescription *)pTmp;
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nArrayTypeDescriptionCount );
#endif
            pIndirect->pType = 0;
            pTmp->nDimensions = 0;
            pTmp->nTotalElements = 0;
            pTmp->pDimensions = 0;
        }
        break;

        case typelib_TypeClass_SEQUENCE:
        {
            typelib_IndirectTypeDescription * pTmp = new typelib_IndirectTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nIndirectTypeDescriptionCount );
#endif
            pTmp->pType = 0;
        }
        break;

        case typelib_TypeClass_UNION:
        {
            typelib_UnionTypeDescription * pTmp;
            pTmp = new typelib_UnionTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nUnionTypeDescriptionCount );
#endif
            pTmp->nMembers = 0;
            pTmp->pDiscriminantTypeRef = 0;
            pTmp->pDiscriminants = 0;
            pTmp->ppTypeRefs = 0;
            pTmp->ppMemberNames = 0;
            pTmp->pDefaultTypeRef = 0;
        }
        break;

        case typelib_TypeClass_STRUCT:
        {
            // FEATURE_EMPTYCLASS
            typelib_StructTypeDescription * pTmp;
            pTmp = new typelib_StructTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nCompoundTypeDescriptionCount );
#endif
            pTmp->aBase.pBaseTypeDescription = 0;
            pTmp->aBase.nMembers = 0;
            pTmp->aBase.pMemberOffsets = 0;
            pTmp->aBase.ppTypeRefs = 0;
            pTmp->aBase.ppMemberNames = 0;
            pTmp->pParameterizedTypes = 0;
        }
        break;

        case typelib_TypeClass_EXCEPTION:
        {
            // FEATURE_EMPTYCLASS
            typelib_CompoundTypeDescription * pTmp;
            pTmp = new typelib_CompoundTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nCompoundTypeDescriptionCount );
#endif
            pTmp->pBaseTypeDescription = 0;
            pTmp->nMembers = 0;
            pTmp->pMemberOffsets = 0;
            pTmp->ppTypeRefs = 0;
            pTmp->ppMemberNames = 0;
        }
        break;

        case typelib_TypeClass_ENUM:
        {
            typelib_EnumTypeDescription * pTmp = new typelib_EnumTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nEnumTypeDescriptionCount );
#endif
            pTmp->nDefaultEnumValue = 0;
            pTmp->nEnumValues       = 0;
            pTmp->ppEnumNames       = 0;
            pTmp->pEnumValues       = 0;
        }
        break;

        case typelib_TypeClass_INTERFACE:
        {
            typelib_InterfaceTypeDescription * pTmp = new typelib_InterfaceTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nInterfaceTypeDescriptionCount );
#endif
            pTmp->pBaseTypeDescription = 0;
            pTmp->nMembers = 0;
            pTmp->ppMembers = 0;
            pTmp->nAllMembers = 0;
            pTmp->ppAllMembers = 0;
            pTmp->nMapFunctionIndexToMemberIndex = 0;
            pTmp->pMapFunctionIndexToMemberIndex = 0;
            pTmp->pMapMemberIndexToFunctionIndex= 0;
            pTmp->nBaseTypes = 0;
            pTmp->ppBaseTypes = 0;
        }
        break;

        case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription * pTmp = new typelib_InterfaceMethodTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nInterfaceMethodTypeDescriptionCount );
#endif
            pTmp->aBase.pMemberName = 0;
            pTmp->pReturnTypeRef = 0;
            pTmp->nParams = 0;
            pTmp->pParams = 0;
            pTmp->nExceptions = 0;
            pTmp->ppExceptions = 0;
            pTmp->pInterface = 0;
            pTmp->pBaseRef = 0;
            pTmp->nIndex = 0;
        }
        break;

        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_InterfaceAttributeTypeDescription * pTmp = new typelib_InterfaceAttributeTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nInterfaceAttributeTypeDescriptionCount );
#endif
            pTmp->aBase.pMemberName = 0;
            pTmp->pAttributeTypeRef = 0;
            pTmp->pInterface = 0;
            pTmp->pBaseRef = 0;
            pTmp->nIndex = 0;
            pTmp->nGetExceptions = 0;
            pTmp->ppGetExceptions = 0;
            pTmp->nSetExceptions = 0;
            pTmp->ppSetExceptions = 0;
        }
        break;

        default:
        {
            pRet = new typelib_TypeDescription();
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_increment( &Init::get().nTypeDescriptionCount );
#endif
        }
    }

    pRet->nRefCount = 1; // reference count is initially 1
    pRet->nStaticRefCount = 0;
    pRet->eTypeClass = eTypeClass;
    pRet->pTypeName = 0;
    pRet->pUniqueIdentifier = 0;
    pRet->pReserved = 0;
    rtl_uString_acquire( pRet->pTypeName = pTypeName );
    pRet->pSelf = pRet;
    pRet->bComplete = sal_True;
    pRet->nSize = 0;
    pRet->nAlignment = 0;
    pRet->pWeakRef = 0;
    pRet->bOnDemand = sal_False;
    *ppRet = pRet;
}

//------------------------------------------------------------------------
namespace {

void newTypeDescription(
    typelib_TypeDescription ** ppRet, typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName, typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers, typelib_CompoundMember_Init * pCompoundMembers,
    typelib_StructMember_Init * pStructMembers)
{
    OSL_ASSERT(
        (pCompoundMembers == 0 || pStructMembers == 0)
        && (pStructMembers == 0 || eTypeClass == typelib_TypeClass_STRUCT));
    if (typelib_TypeClass_TYPEDEF == eTypeClass)
    {
        OSL_TRACE( "### unexpected typedef!" );
        typelib_typedescriptionreference_getDescription( ppRet, pType );
        return;
    }

    typelib_typedescription_newEmpty( ppRet, eTypeClass, pTypeName );

    switch( eTypeClass )
    {
        case typelib_TypeClass_SEQUENCE:
        {
            OSL_ASSERT( nMembers == 0 );
            typelib_typedescriptionreference_acquire( pType );
            ((typelib_IndirectTypeDescription *)*ppRet)->pType = pType;
        }
        break;

        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_STRUCT:
        {
            // FEATURE_EMPTYCLASS
            typelib_CompoundTypeDescription * pTmp = (typelib_CompoundTypeDescription*)*ppRet;

            sal_Int32 nOffset = 0;
            if( pType )
            {
                typelib_typedescriptionreference_getDescription(
                    (typelib_TypeDescription **)&pTmp->pBaseTypeDescription, pType );
                nOffset = ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nSize;
                OSL_ENSURE( newAlignedSize( 0, ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nSize, ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nAlignment ) == ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nSize, "### unexpected offset!" );
            }
            if( nMembers )
            {
                pTmp->nMembers = nMembers;
                pTmp->pMemberOffsets = new sal_Int32[ nMembers ];
                pTmp->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
                pTmp->ppMemberNames = new rtl_uString *[ nMembers ];
                bool polymorphic = eTypeClass == typelib_TypeClass_STRUCT
                    && rtl::OUString::unacquired(&pTypeName).indexOf('<') >= 0;
                OSL_ASSERT(!polymorphic || pStructMembers != 0);
                if (polymorphic) {
                    reinterpret_cast< typelib_StructTypeDescription * >(pTmp)->
                        pParameterizedTypes = new sal_Bool[nMembers];
                }
                for( sal_Int32 i = 0 ; i < nMembers; i++ )
                {
                    // read the type and member names
                    pTmp->ppTypeRefs[i] = 0;
                    if (pCompoundMembers != 0) {
                        typelib_typedescriptionreference_new(
                            pTmp->ppTypeRefs +i, pCompoundMembers[i].eTypeClass,
                            pCompoundMembers[i].pTypeName );
                        rtl_uString_acquire(
                            pTmp->ppMemberNames[i]
                            = pCompoundMembers[i].pMemberName );
                    } else {
                        typelib_typedescriptionreference_new(
                            pTmp->ppTypeRefs +i,
                            pStructMembers[i].aBase.eTypeClass,
                            pStructMembers[i].aBase.pTypeName );
                        rtl_uString_acquire(
                            pTmp->ppMemberNames[i]
                            = pStructMembers[i].aBase.pMemberName );
                    }
                    // write offset
                    sal_Int32 size;
                    sal_Int32 alignment;
                    if (pTmp->ppTypeRefs[i]->eTypeClass ==
                        typelib_TypeClass_SEQUENCE)
                    {
                        // Take care of recursion like
                        // struct S { sequence<S> x; };
                        size = sizeof(void *);
                        alignment = adjustAlignment(size);
                    } else {
                        typelib_TypeDescription * pTD = 0;
                        TYPELIB_DANGER_GET( &pTD, pTmp->ppTypeRefs[i] );
                        OSL_ENSURE( pTD->nSize, "### void member?" );
                        size = pTD->nSize;
                        alignment = pTD->nAlignment;
                        TYPELIB_DANGER_RELEASE( pTD );
                    }
                    nOffset = newAlignedSize( nOffset, size, alignment );
                    pTmp->pMemberOffsets[i] = nOffset - size;

                    if (polymorphic) {
                        reinterpret_cast< typelib_StructTypeDescription * >(
                            pTmp)->pParameterizedTypes[i]
                            = pStructMembers[i].bParameterizedType;
                    }
                }
            }
        }
        break;

        default:
        break;
    }

    if( !reallyWeak( eTypeClass ) )
        (*ppRet)->pWeakRef = (typelib_TypeDescriptionReference *)*ppRet;
    if( eTypeClass != typelib_TypeClass_VOID )
    {
        // sizeof( void ) not allowed
        (*ppRet)->nSize = typelib_typedescription_getAlignedUnoSize( (*ppRet), 0, (*ppRet)->nAlignment );
        (*ppRet)->nAlignment = adjustAlignment( (*ppRet)->nAlignment );
    }
}

}

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_new(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers,
    typelib_CompoundMember_Init * pMembers )
    SAL_THROW_EXTERN_C()
{
    newTypeDescription(
        ppRet, eTypeClass, pTypeName, pType, nMembers, pMembers, 0);
}

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newStruct(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers,
    typelib_StructMember_Init * pMembers )
    SAL_THROW_EXTERN_C()
{
    newTypeDescription(
        ppRet, typelib_TypeClass_STRUCT, pTypeName, pType, nMembers, 0,
        pMembers);
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newUnion(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pDiscriminantTypeRef,
    sal_Int64 nDefaultDiscriminant,
    typelib_TypeDescriptionReference * pDefaultTypeRef,
    sal_Int32 nMembers,
    typelib_Union_Init * pMembers )
    SAL_THROW_EXTERN_C()
{
    typelib_typedescription_newEmpty( ppRet, typelib_TypeClass_UNION, pTypeName );
    // discriminant type
    typelib_UnionTypeDescription * pTmp = (typelib_UnionTypeDescription *)*ppRet;
    typelib_typedescriptionreference_acquire( pTmp->pDiscriminantTypeRef = pDiscriminantTypeRef );

    sal_Int32 nPos;

    pTmp->nMembers = nMembers;
    // default discriminant
    if (nMembers)
    {
        pTmp->pDiscriminants = new sal_Int64[ nMembers ];
        for ( nPos = nMembers; nPos--; )
        {
            pTmp->pDiscriminants[nPos] = pMembers[nPos].nDiscriminant;
        }
    }
    // default default discriminant
    pTmp->nDefaultDiscriminant = nDefaultDiscriminant;

    // union member types
    pTmp->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
    for ( nPos = nMembers; nPos--; )
    {
        typelib_typedescriptionreference_acquire( pTmp->ppTypeRefs[nPos] = pMembers[nPos].pTypeRef );
    }
    // union member names
    pTmp->ppMemberNames = new rtl_uString *[ nMembers ];
    for ( nPos = nMembers; nPos--; )
    {
        rtl_uString_acquire( pTmp->ppMemberNames[nPos] = pMembers[nPos].pMemberName );
    }

    // default union type
    typelib_typedescriptionreference_acquire( pTmp->pDefaultTypeRef = pDefaultTypeRef );

    if (! reallyWeak( typelib_TypeClass_UNION ))
        (*ppRet)->pWeakRef = (typelib_TypeDescriptionReference *)*ppRet;
    (*ppRet)->nSize = typelib_typedescription_getAlignedUnoSize( (*ppRet), 0, (*ppRet)->nAlignment );
    (*ppRet)->nAlignment = adjustAlignment( (*ppRet)->nAlignment );
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newEnum(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_Int32 nDefaultValue,
    sal_Int32 nEnumValues,
    rtl_uString ** ppEnumNames,
    sal_Int32 * pEnumValues )
    SAL_THROW_EXTERN_C()
{
    typelib_typedescription_newEmpty( ppRet, typelib_TypeClass_ENUM, pTypeName );
    typelib_EnumTypeDescription * pEnum = (typelib_EnumTypeDescription *)*ppRet;

    pEnum->nDefaultEnumValue = nDefaultValue;
    pEnum->nEnumValues       = nEnumValues;
    pEnum->ppEnumNames       = new rtl_uString * [ nEnumValues ];
    for ( sal_Int32 nPos = nEnumValues; nPos--; )
    {
        rtl_uString_acquire( pEnum->ppEnumNames[nPos] = ppEnumNames[nPos] );
    }
    pEnum->pEnumValues      = new sal_Int32[ nEnumValues ];
    ::memcpy( pEnum->pEnumValues, pEnumValues, nEnumValues * sizeof(sal_Int32) );

    (*ppRet)->pWeakRef = (typelib_TypeDescriptionReference *)*ppRet;
    // sizeof( void ) not allowed
    (*ppRet)->nSize = typelib_typedescription_getAlignedUnoSize( (*ppRet), 0, (*ppRet)->nAlignment );
    (*ppRet)->nAlignment = adjustAlignment( (*ppRet)->nAlignment );
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newArray(
    typelib_TypeDescription ** ppRet,
    typelib_TypeDescriptionReference * pElementTypeRef,
    sal_Int32 nDimensions,
    sal_Int32 * pDimensions )
    SAL_THROW_EXTERN_C ()
{
    OUStringBuffer aBuf( 32 );
    aBuf.append( pElementTypeRef->pTypeName );
    sal_Int32 nElements = 1;
    for (sal_Int32 i=0; i < nDimensions; i++)
    {
        aBuf.appendAscii("[");
        aBuf.append(pDimensions[i]);
        aBuf.appendAscii("]");
        nElements *= pDimensions[i];
    }
    OUString aTypeName( aBuf.makeStringAndClear() );


    typelib_typedescription_newEmpty( ppRet, typelib_TypeClass_ARRAY, aTypeName.pData );
    typelib_ArrayTypeDescription * pArray = (typelib_ArrayTypeDescription *)*ppRet;

    pArray->nDimensions = nDimensions;
    pArray->nTotalElements = nElements;
    pArray->pDimensions = new sal_Int32[ nDimensions ];
    ::memcpy( pArray->pDimensions, pDimensions, nDimensions * sizeof(sal_Int32) );

    typelib_typedescriptionreference_acquire(pElementTypeRef);
    ((typelib_IndirectTypeDescription*)pArray)->pType = pElementTypeRef;

    (*ppRet)->pWeakRef = (typelib_TypeDescriptionReference *)*ppRet;
    // sizeof( void ) not allowed
    (*ppRet)->nSize = typelib_typedescription_getAlignedUnoSize( *ppRet, 0, (*ppRet)->nAlignment );
    (*ppRet)->nAlignment = adjustAlignment( (*ppRet)->nAlignment );
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newInterface(
    typelib_InterfaceTypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_uInt32 nUik1, sal_uInt16 nUik2, sal_uInt16 nUik3, sal_uInt32 nUik4, sal_uInt32 nUik5,
    typelib_TypeDescriptionReference * pBaseInterface,
    sal_Int32 nMembers,
    typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C()
{
    typelib_typedescription_newMIInterface(
        ppRet, pTypeName, nUik1, nUik2, nUik3, nUik4, nUik5,
        pBaseInterface == 0 ? 0 : 1, &pBaseInterface, nMembers, ppMembers);
}

//------------------------------------------------------------------------

namespace {

class BaseList {
public:
    struct Entry {
        sal_Int32 memberOffset;
        sal_Int32 directBaseIndex;
        sal_Int32 directBaseMemberOffset;
        typelib_InterfaceTypeDescription const * base;
    };

    typedef std::vector< Entry > List;

    BaseList(typelib_InterfaceTypeDescription const * desc);

    List const & getList() const { return list; }

    sal_Int32 getBaseMembers() const { return members; }

private:
    typedef std::set< rtl::OUString > Set;

    void calculate(
        sal_Int32 directBaseIndex, Set & directBaseSet,
        sal_Int32 * directBaseMembers,
        typelib_InterfaceTypeDescription const * desc);

    Set set;
    List list;
    sal_Int32 members;
};

BaseList::BaseList(typelib_InterfaceTypeDescription const * desc) {
    members = 0;
    for (sal_Int32 i = 0; i < desc->nBaseTypes; ++i) {
        Set directBaseSet;
        sal_Int32 directBaseMembers = 0;
        calculate(i, directBaseSet, &directBaseMembers, desc->ppBaseTypes[i]);
    }
}

void BaseList::calculate(
    sal_Int32 directBaseIndex, Set & directBaseSet,
    sal_Int32 * directBaseMembers,
    typelib_InterfaceTypeDescription const * desc)
{
    for (sal_Int32 i = 0; i < desc->nBaseTypes; ++i) {
        calculate(
            directBaseIndex, directBaseSet, directBaseMembers,
            desc->ppBaseTypes[i]);
    }
    if (set.insert(desc->aBase.pTypeName).second) {
        Entry e;
        e.memberOffset = members;
        e.directBaseIndex = directBaseIndex;
        e.directBaseMemberOffset = *directBaseMembers;
        e.base = desc;
        list.push_back(e);
        OSL_ASSERT(desc->ppAllMembers != 0);
        members += desc->nMembers;
    }
    if (directBaseSet.insert(desc->aBase.pTypeName).second) {
        OSL_ASSERT(desc->ppAllMembers != 0);
        *directBaseMembers += desc->nMembers;
    }
}

}

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newMIInterface(
    typelib_InterfaceTypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_uInt32 nUik1, sal_uInt16 nUik2, sal_uInt16 nUik3, sal_uInt32 nUik4, sal_uInt32 nUik5,
    sal_Int32 nBaseInterfaces,
    typelib_TypeDescriptionReference ** ppBaseInterfaces,
    sal_Int32 nMembers,
    typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C()
{
    if (*ppRet != 0) {
        typelib_typedescription_release(&(*ppRet)->aBase);
        *ppRet = 0;
    }

    typelib_InterfaceTypeDescription * pITD = 0;
    typelib_typedescription_newEmpty(
        (typelib_TypeDescription **)&pITD, typelib_TypeClass_INTERFACE, pTypeName );

    pITD->nBaseTypes = nBaseInterfaces;
    pITD->ppBaseTypes = new typelib_InterfaceTypeDescription *[nBaseInterfaces];
    for (sal_Int32 i = 0; i < nBaseInterfaces; ++i) {
        pITD->ppBaseTypes[i] = 0;
        typelib_typedescriptionreference_getDescription(
            reinterpret_cast< typelib_TypeDescription ** >(
                &pITD->ppBaseTypes[i]),
            ppBaseInterfaces[i]);
        if (pITD->ppBaseTypes[i] == 0
            || !complete(
                reinterpret_cast< typelib_TypeDescription ** >(
                    &pITD->ppBaseTypes[i]),
                false))
        {
            OSL_ASSERT(false);
            return;
        }
        OSL_ASSERT(pITD->ppBaseTypes[i] != 0);
    }
    if (nBaseInterfaces > 0) {
        pITD->pBaseTypeDescription = pITD->ppBaseTypes[0];
    }
    // set the
    pITD->aUik.m_Data1 = nUik1;
    pITD->aUik.m_Data2 = nUik2;
    pITD->aUik.m_Data3 = nUik3;
    pITD->aUik.m_Data4 = nUik4;
    pITD->aUik.m_Data5 = nUik5;

    BaseList aBaseList(pITD);
    pITD->nAllMembers = nMembers + aBaseList.getBaseMembers();
    pITD->nMembers = nMembers;

    if( pITD->nAllMembers )
    {
        // at minimum one member exist, allocate the memory
        pITD->ppAllMembers = new typelib_TypeDescriptionReference *[ pITD->nAllMembers ];
        sal_Int32 n = 0;

        BaseList::List const & rList = aBaseList.getList();
        for (BaseList::List::const_iterator i(rList.begin()); i != rList.end();
             ++i)
        {
            typelib_InterfaceTypeDescription const * pBase = i->base;
            typelib_InterfaceTypeDescription const * pDirectBase
                = pITD->ppBaseTypes[i->directBaseIndex];
            OSL_ASSERT(pBase->ppAllMembers != 0);
            for (sal_Int32 j = 0; j < pBase->nMembers; ++j) {
                typelib_TypeDescriptionReference const * pDirectBaseMember
                    = pDirectBase->ppAllMembers[i->directBaseMemberOffset + j];
                rtl::OUStringBuffer aBuf(pDirectBaseMember->pTypeName);
                aBuf.appendAscii(RTL_CONSTASCII_STRINGPARAM(":@"));
                aBuf.append(i->directBaseIndex);
                aBuf.append(static_cast< sal_Unicode >(','));
                aBuf.append(i->memberOffset + j);
                aBuf.append(static_cast< sal_Unicode >(':'));
                aBuf.append(pITD->aBase.pTypeName);
                rtl::OUString aName(aBuf.makeStringAndClear());
                typelib_TypeDescriptionReference * pDerivedMember = 0;
                typelib_typedescriptionreference_new(
                    &pDerivedMember, pDirectBaseMember->eTypeClass,
                    aName.pData);
                pITD->ppAllMembers[n++] = pDerivedMember;
            }
        }

        if( nMembers )
        {
            pITD->ppMembers = pITD->ppAllMembers + aBaseList.getBaseMembers();
        }

        // add own members
        for( sal_Int32 i = 0; i < nMembers; i++ )
        {
            typelib_typedescriptionreference_acquire( ppMembers[i] );
            pITD->ppAllMembers[n++] = ppMembers[i];
        }
    }

    typelib_TypeDescription * pTmp = (typelib_TypeDescription *)pITD;
    if( !reallyWeak( typelib_TypeClass_INTERFACE ) )
        pTmp->pWeakRef = (typelib_TypeDescriptionReference *)pTmp;
    pTmp->nSize = typelib_typedescription_getAlignedUnoSize( pTmp, 0, pTmp->nAlignment );
    pTmp->nAlignment = adjustAlignment( pTmp->nAlignment );
    pTmp->bComplete = sal_False;

    *ppRet = pITD;
}

//------------------------------------------------------------------------

namespace {

typelib_TypeDescriptionReference ** copyExceptions(
    sal_Int32 count, rtl_uString ** typeNames)
{
    OSL_ASSERT(count >= 0);
    if (count == 0) {
        return 0;
    }
    typelib_TypeDescriptionReference ** p
        = new typelib_TypeDescriptionReference *[count];
    for (sal_Int32 i = 0; i < count; ++i) {
        p[i] = 0;
        typelib_typedescriptionreference_new(
            p + i, typelib_TypeClass_EXCEPTION, typeNames[i]);
    }
    return p;
}

}

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newInterfaceMethod(
    typelib_InterfaceMethodTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    sal_Bool bOneWay,
    rtl_uString * pTypeName,
    typelib_TypeClass eReturnTypeClass,
    rtl_uString * pReturnTypeName,
    sal_Int32 nParams,
    typelib_Parameter_Init * pParams,
    sal_Int32 nExceptions,
    rtl_uString ** ppExceptionNames )
    SAL_THROW_EXTERN_C()
{
    if (*ppRet != 0) {
        typelib_typedescription_release(&(*ppRet)->aBase.aBase);
        *ppRet = 0;
    }
    sal_Int32 nOffset = rtl_ustr_lastIndexOfChar_WithLength(
        pTypeName->buffer, pTypeName->length, ':');
    if (nOffset <= 0 || pTypeName->buffer[nOffset - 1] != ':') {
        OSL_FAIL("Bad interface method type name");
        return;
    }
    rtl::OUString aInterfaceTypeName(pTypeName->buffer, nOffset - 1);
    typelib_InterfaceTypeDescription * pInterface = 0;
    typelib_typedescription_getByName(
        reinterpret_cast< typelib_TypeDescription ** >(&pInterface),
        aInterfaceTypeName.pData);
    if (pInterface == 0
        || pInterface->aBase.eTypeClass != typelib_TypeClass_INTERFACE
        || !complete(
            reinterpret_cast< typelib_TypeDescription ** >(&pInterface), false))
    {
        OSL_FAIL("No interface corresponding to interface method");
        return;
    }

    typelib_typedescription_newEmpty(
        (typelib_TypeDescription **)ppRet, typelib_TypeClass_INTERFACE_METHOD, pTypeName );
    typelib_TypeDescription * pTmp = (typelib_TypeDescription *)*ppRet;

    rtl_uString_newFromStr_WithLength( &(*ppRet)->aBase.pMemberName,
                                       pTypeName->buffer + nOffset +1,
                                       pTypeName->length - nOffset -1 );
    (*ppRet)->aBase.nPosition = nAbsolutePosition;
    (*ppRet)->bOneWay = bOneWay;
    typelib_typedescriptionreference_new( &(*ppRet)->pReturnTypeRef, eReturnTypeClass, pReturnTypeName );
    (*ppRet)->nParams = nParams;
    if( nParams )
    {
        (*ppRet)->pParams = new typelib_MethodParameter[ nParams ];

        for( sal_Int32 i = 0; i < nParams; i++ )
        {
            // get the name of the parameter
            (*ppRet)->pParams[ i ].pName = 0;
            rtl_uString_acquire( (*ppRet)->pParams[ i ].pName = pParams[i].pParamName );
            (*ppRet)->pParams[ i ].pTypeRef = 0;
            // get the type name of the parameter and create the weak reference
            typelib_typedescriptionreference_new(
                &(*ppRet)->pParams[ i ].pTypeRef, pParams[i].eTypeClass, pParams[i].pTypeName );
            (*ppRet)->pParams[ i ].bIn = pParams[i].bIn;
            (*ppRet)->pParams[ i ].bOut = pParams[i].bOut;
        }
    }
    (*ppRet)->nExceptions = nExceptions;
    (*ppRet)->ppExceptions = copyExceptions(nExceptions, ppExceptionNames);
    (*ppRet)->pInterface = pInterface;
    (*ppRet)->pBaseRef = 0;
    OSL_ASSERT(
        (nAbsolutePosition >= pInterface->nAllMembers - pInterface->nMembers)
        && nAbsolutePosition < pInterface->nAllMembers);
    (*ppRet)->nIndex = nAbsolutePosition
        - (pInterface->nAllMembers - pInterface->nMembers);
    if( !reallyWeak( typelib_TypeClass_INTERFACE_METHOD ) )
        pTmp->pWeakRef = (typelib_TypeDescriptionReference *)pTmp;
}


//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newInterfaceAttribute(
    typelib_InterfaceAttributeTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    rtl_uString * pTypeName,
    typelib_TypeClass eAttributeTypeClass,
    rtl_uString * pAttributeTypeName,
    sal_Bool bReadOnly )
    SAL_THROW_EXTERN_C()
{
    typelib_typedescription_newExtendedInterfaceAttribute(
        ppRet, nAbsolutePosition, pTypeName, eAttributeTypeClass,
        pAttributeTypeName, bReadOnly, 0, 0, 0, 0);
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_newExtendedInterfaceAttribute(
    typelib_InterfaceAttributeTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    rtl_uString * pTypeName,
    typelib_TypeClass eAttributeTypeClass,
    rtl_uString * pAttributeTypeName,
    sal_Bool bReadOnly,
    sal_Int32 nGetExceptions, rtl_uString ** ppGetExceptionNames,
    sal_Int32 nSetExceptions, rtl_uString ** ppSetExceptionNames )
    SAL_THROW_EXTERN_C()
{
    if (*ppRet != 0) {
        typelib_typedescription_release(&(*ppRet)->aBase.aBase);
        *ppRet = 0;
    }
    sal_Int32 nOffset = rtl_ustr_lastIndexOfChar_WithLength(
        pTypeName->buffer, pTypeName->length, ':');
    if (nOffset <= 0 || pTypeName->buffer[nOffset - 1] != ':') {
        OSL_FAIL("Bad interface attribute type name");
        return;
    }
    rtl::OUString aInterfaceTypeName(pTypeName->buffer, nOffset - 1);
    typelib_InterfaceTypeDescription * pInterface = 0;
    typelib_typedescription_getByName(
        reinterpret_cast< typelib_TypeDescription ** >(&pInterface),
        aInterfaceTypeName.pData);
    if (pInterface == 0
        || pInterface->aBase.eTypeClass != typelib_TypeClass_INTERFACE
        || !complete(
            reinterpret_cast< typelib_TypeDescription ** >(&pInterface), false))
    {
        OSL_FAIL("No interface corresponding to interface attribute");
        return;
    }

    typelib_typedescription_newEmpty(
        (typelib_TypeDescription **)ppRet, typelib_TypeClass_INTERFACE_ATTRIBUTE, pTypeName );
    typelib_TypeDescription * pTmp = (typelib_TypeDescription *)*ppRet;

    rtl_uString_newFromStr_WithLength( &(*ppRet)->aBase.pMemberName,
                                       pTypeName->buffer + nOffset +1,
                                       pTypeName->length - nOffset -1 );
    (*ppRet)->aBase.nPosition = nAbsolutePosition;
    typelib_typedescriptionreference_new( &(*ppRet)->pAttributeTypeRef, eAttributeTypeClass, pAttributeTypeName );
    (*ppRet)->bReadOnly = bReadOnly;
    (*ppRet)->pInterface = pInterface;
    (*ppRet)->pBaseRef = 0;
    OSL_ASSERT(
        (nAbsolutePosition >= pInterface->nAllMembers - pInterface->nMembers)
        && nAbsolutePosition < pInterface->nAllMembers);
    (*ppRet)->nIndex = nAbsolutePosition
        - (pInterface->nAllMembers - pInterface->nMembers);
    (*ppRet)->nGetExceptions = nGetExceptions;
    (*ppRet)->ppGetExceptions = copyExceptions(
        nGetExceptions, ppGetExceptionNames);
    (*ppRet)->nSetExceptions = nSetExceptions;
    (*ppRet)->ppSetExceptions = copyExceptions(
        nSetExceptions, ppSetExceptionNames);
    if( !reallyWeak( typelib_TypeClass_INTERFACE_ATTRIBUTE ) )
        pTmp->pWeakRef = (typelib_TypeDescriptionReference *)pTmp;
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_acquire(
    typelib_TypeDescription * pTypeDescription )
    SAL_THROW_EXTERN_C()
{
    osl_atomic_increment( &pTypeDescription->nRefCount );
}

//------------------------------------------------------------------------

namespace {

void deleteExceptions(
    sal_Int32 count, typelib_TypeDescriptionReference ** exceptions)
{
    for (sal_Int32 i = 0; i < count; ++i) {
        typelib_typedescriptionreference_release(exceptions[i]);
    }
    delete[] exceptions;
}

}

// frees anything except typelib_TypeDescription base!
static inline void typelib_typedescription_destructExtendedMembers(
    typelib_TypeDescription * pTD )
    SAL_THROW(())
{
    OSL_ASSERT( typelib_TypeClass_TYPEDEF != pTD->eTypeClass );

    switch( pTD->eTypeClass )
    {
    case typelib_TypeClass_ARRAY:
        if( ((typelib_IndirectTypeDescription*)pTD)->pType )
            typelib_typedescriptionreference_release( ((typelib_IndirectTypeDescription*)pTD)->pType );
        delete [] ((typelib_ArrayTypeDescription *)pTD)->pDimensions;
        break;
    case typelib_TypeClass_SEQUENCE:
        if( ((typelib_IndirectTypeDescription*)pTD)->pType )
            typelib_typedescriptionreference_release( ((typelib_IndirectTypeDescription*)pTD)->pType );
        break;
    case typelib_TypeClass_UNION:
    {
        typelib_UnionTypeDescription * pUnionTD = (typelib_UnionTypeDescription *)pTD;
        typelib_typedescriptionreference_release( pUnionTD->pDiscriminantTypeRef );
        typelib_typedescriptionreference_release( pUnionTD->pDefaultTypeRef );

        sal_Int32 nPos;
        typelib_TypeDescriptionReference ** ppTypeRefs = pUnionTD->ppTypeRefs;
        for ( nPos = pUnionTD->nMembers; nPos--; )
        {
            typelib_typedescriptionreference_release( ppTypeRefs[nPos] );
        }

        rtl_uString ** ppMemberNames = pUnionTD->ppMemberNames;
        for ( nPos = pUnionTD->nMembers; nPos--; )
        {
            rtl_uString_release( ppMemberNames[nPos] );
        }
        delete [] pUnionTD->ppMemberNames;
        delete [] pUnionTD->pDiscriminants;
        delete [] pUnionTD->ppTypeRefs;
    }
    break;
    case typelib_TypeClass_STRUCT:
        delete[] reinterpret_cast< typelib_StructTypeDescription * >(pTD)->
            pParameterizedTypes;
        // Fall-through intentional
    case typelib_TypeClass_EXCEPTION:
    {
        typelib_CompoundTypeDescription * pCTD = (typelib_CompoundTypeDescription*)pTD;
        if( pCTD->pBaseTypeDescription )
            typelib_typedescription_release( (typelib_TypeDescription *)pCTD->pBaseTypeDescription );
        sal_Int32 i;
        for( i = 0; i < pCTD->nMembers; i++ )
        {
            typelib_typedescriptionreference_release( pCTD->ppTypeRefs[i] );
        }
        if (pCTD->ppMemberNames)
        {
            for ( i = 0; i < pCTD->nMembers; i++ )
            {
                rtl_uString_release( pCTD->ppMemberNames[i] );
            }
            delete [] pCTD->ppMemberNames;
        }
        delete [] pCTD->ppTypeRefs;
        delete [] pCTD->pMemberOffsets;
    }
    break;
    case typelib_TypeClass_INTERFACE:
    {
        typelib_InterfaceTypeDescription * pITD = (typelib_InterfaceTypeDescription*)pTD;
        for( sal_Int32 i = 0; i < pITD->nAllMembers; i++ )
        {
            typelib_typedescriptionreference_release( pITD->ppAllMembers[i] );
        }
        delete [] pITD->ppAllMembers;
        delete [] pITD->pMapMemberIndexToFunctionIndex;
        delete [] pITD->pMapFunctionIndexToMemberIndex;
        for (sal_Int32 i = 0; i < pITD->nBaseTypes; ++i) {
            typelib_typedescription_release(
                reinterpret_cast< typelib_TypeDescription * >(
                    pITD->ppBaseTypes[i]));
        }
        delete[] pITD->ppBaseTypes;
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        typelib_InterfaceMethodTypeDescription * pIMTD = (typelib_InterfaceMethodTypeDescription*)pTD;
        if( pIMTD->pReturnTypeRef )
            typelib_typedescriptionreference_release( pIMTD->pReturnTypeRef );
        for( sal_Int32 i = 0; i < pIMTD->nParams; i++ )
        {
            rtl_uString_release( pIMTD->pParams[ i ].pName );
            typelib_typedescriptionreference_release( pIMTD->pParams[ i ].pTypeRef );
        }
        delete [] pIMTD->pParams;
        deleteExceptions(pIMTD->nExceptions, pIMTD->ppExceptions);
        rtl_uString_release( pIMTD->aBase.pMemberName );
        typelib_typedescription_release(&pIMTD->pInterface->aBase);
        if (pIMTD->pBaseRef != 0) {
            typelib_typedescriptionreference_release(pIMTD->pBaseRef);
        }
    }
    break;
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        typelib_InterfaceAttributeTypeDescription * pIATD = (typelib_InterfaceAttributeTypeDescription*)pTD;
        deleteExceptions(pIATD->nGetExceptions, pIATD->ppGetExceptions);
        deleteExceptions(pIATD->nSetExceptions, pIATD->ppSetExceptions);
        if( pIATD->pAttributeTypeRef )
            typelib_typedescriptionreference_release( pIATD->pAttributeTypeRef );
        if( pIATD->aBase.pMemberName )
            rtl_uString_release( pIATD->aBase.pMemberName );
        typelib_typedescription_release(&pIATD->pInterface->aBase);
        if (pIATD->pBaseRef != 0) {
            typelib_typedescriptionreference_release(pIATD->pBaseRef);
        }
    }
    break;
    case typelib_TypeClass_ENUM:
    {
        typelib_EnumTypeDescription * pEnum = (typelib_EnumTypeDescription *)pTD;
        for ( sal_Int32 nPos = pEnum->nEnumValues; nPos--; )
        {
            rtl_uString_release( pEnum->ppEnumNames[nPos] );
        }
        delete [] pEnum->ppEnumNames;
        delete [] pEnum->pEnumValues;
    }
    break;
    default:
    break;
    }
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_release(
    typelib_TypeDescription * pTD )
    SAL_THROW_EXTERN_C()
{
    sal_Int32 ref = osl_atomic_decrement( &pTD->nRefCount );
    OSL_ASSERT(ref >= 0);
    if (0 == ref)
    {
        TypeDescriptor_Init_Impl &rInit = Init::get();
        if( reallyWeak( pTD->eTypeClass ) )
        {
            if( pTD->pWeakRef )
            {
                {
                MutexGuard aGuard( rInit.getMutex() );
                // remove this description from the weak reference
                pTD->pWeakRef->pType = 0;
                }
                typelib_typedescriptionreference_release( pTD->pWeakRef );
            }
        }
        else
        {
            // this description is a reference too, so remove it from the hash table
            if( rInit.pWeakMap )
            {
                MutexGuard aGuard( rInit.getMutex() );
                WeakMap_Impl::iterator aIt = rInit.pWeakMap->find( (sal_Unicode*)pTD->pTypeName->buffer );
                if( aIt != rInit.pWeakMap->end() && (void *)(*aIt).second == (void *)pTD )
                {
                    // remove only if it contains the same object
                    rInit.pWeakMap->erase( aIt );
                }
            }
        }

        typelib_typedescription_destructExtendedMembers( pTD );
        rtl_uString_release( pTD->pTypeName );

#if OSL_DEBUG_LEVEL > 1
        switch( pTD->eTypeClass )
        {
        case typelib_TypeClass_ARRAY:
            osl_atomic_decrement( &rInit.nArrayTypeDescriptionCount );
            break;
        case typelib_TypeClass_SEQUENCE:
            osl_atomic_decrement( &rInit.nIndirectTypeDescriptionCount );
            break;
        case typelib_TypeClass_UNION:
            osl_atomic_decrement( &rInit.nUnionTypeDescriptionCount );
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            osl_atomic_decrement( &rInit.nCompoundTypeDescriptionCount );
            break;
        case typelib_TypeClass_INTERFACE:
            osl_atomic_decrement( &rInit.nInterfaceTypeDescriptionCount );
            break;
        case typelib_TypeClass_INTERFACE_METHOD:
            osl_atomic_decrement( &rInit.nInterfaceMethodTypeDescriptionCount );
            break;
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            osl_atomic_decrement( &rInit.nInterfaceAttributeTypeDescriptionCount );
            break;
        case typelib_TypeClass_ENUM:
            osl_atomic_decrement( &rInit.nEnumTypeDescriptionCount );
            break;
        default:
            osl_atomic_decrement( &rInit.nTypeDescriptionCount );
        }
#endif

        delete pTD;
    }
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_register(
    typelib_TypeDescription ** ppNewDescription )
    SAL_THROW_EXTERN_C()
{
    // connect the description with the weak reference
    TypeDescriptor_Init_Impl &rInit = Init::get();
    ClearableMutexGuard aGuard( rInit.getMutex() );

    typelib_TypeDescriptionReference * pTDR = 0;
    typelib_typedescriptionreference_getByName( &pTDR, (*ppNewDescription)->pTypeName );

    OSL_ASSERT( (*ppNewDescription)->pWeakRef || reallyWeak( (*ppNewDescription)->eTypeClass ) );
    if( pTDR )
    {
        OSL_ASSERT( (*ppNewDescription)->eTypeClass == pTDR->eTypeClass );
        if( pTDR->pType )
        {
            if (reallyWeak( pTDR->eTypeClass ))
            {
                // pRef->pType->pWeakRef == 0 means that the description is empty
                if (pTDR->pType->pWeakRef)
                {
                    if (osl_atomic_increment( &pTDR->pType->nRefCount ) > 1)
                    {
                        // The refence is incremented. The object cannot be destroyed.
                        // Release the guard at the earliest point.
                        aGuard.clear();
                        ::typelib_typedescription_release( *ppNewDescription );
                        *ppNewDescription = pTDR->pType;
                        ::typelib_typedescriptionreference_release( pTDR );
                        return;
                    }
                    else
                    {
                        // destruction of this type in progress (another thread!)
                        osl_atomic_decrement( &pTDR->pType->nRefCount );
                    }
                }
                // take new descr
                pTDR->pType = *ppNewDescription;
                OSL_ASSERT( ! (*ppNewDescription)->pWeakRef );
                (*ppNewDescription)->pWeakRef = pTDR;
                return;
            }
            // !reallyWeak

            if (((void *)pTDR != (void *)*ppNewDescription) && // if different
                (!pTDR->pType->pWeakRef || // uninit: ref data only set
                 // new one is complete:
                 (!pTDR->pType->bComplete && (*ppNewDescription)->bComplete) ||
                 // new one may be partly initialized interface (except of tables):
                 (typelib_TypeClass_INTERFACE == pTDR->pType->eTypeClass &&
                  !((typelib_InterfaceTypeDescription *)pTDR->pType)->ppAllMembers &&
                  (*(typelib_InterfaceTypeDescription **)ppNewDescription)->ppAllMembers)))
            {
                // uninitialized or incomplete

                if (pTDR->pType->pWeakRef) // if init
                {
                    typelib_typedescription_destructExtendedMembers( pTDR->pType );
                }

                // pTDR->pType->pWeakRef == 0 means that the description is empty
                // description is not weak and the not the same
                sal_Int32 nSize = getDescriptionSize( (*ppNewDescription)->eTypeClass );

                // copy all specific data for the descriptions
                memcpy(
                    pTDR->pType +1,
                    *ppNewDescription +1,
                    nSize - sizeof(typelib_TypeDescription) );

                pTDR->pType->bComplete = (*ppNewDescription)->bComplete;
                pTDR->pType->nSize = (*ppNewDescription)->nSize;
                pTDR->pType->nAlignment = (*ppNewDescription)->nAlignment;

                memset(
                    *ppNewDescription +1,
                    0,
                    nSize - sizeof( typelib_TypeDescription ) );

                if( pTDR->pType->bOnDemand && !(*ppNewDescription)->bOnDemand )
                {
                    // switch from OnDemand to !OnDemand, so the description must be acquired
                    typelib_typedescription_acquire( pTDR->pType );
                }
                else if( !pTDR->pType->bOnDemand && (*ppNewDescription)->bOnDemand )
                {
                    // switch from !OnDemand to OnDemand, so the description must be relesed
                    typelib_typedescription_release( pTDR->pType );
                }

                pTDR->pType->bOnDemand = (*ppNewDescription)->bOnDemand;
                // initialized
                pTDR->pType->pWeakRef = pTDR;
            }

            typelib_typedescription_release( *ppNewDescription );
            // pTDR was acquired by getByName(), so it must not be acquired again
            *ppNewDescription = pTDR->pType;
            return;
        }
    }
    else if( reallyWeak( (*ppNewDescription)->eTypeClass) )
    {
        typelib_typedescriptionreference_new(
            &pTDR, (*ppNewDescription)->eTypeClass, (*ppNewDescription)->pTypeName );
    }
    else
    {
        pTDR = (typelib_TypeDescriptionReference *)*ppNewDescription;
        if( !rInit.pWeakMap )
            rInit.pWeakMap = new WeakMap_Impl;

        // description is the weak itself, so register it
        (*rInit.pWeakMap)[pTDR->pTypeName->buffer] = pTDR;
        OSL_ASSERT( (void *)*ppNewDescription == (void *)pTDR );
    }

    // By default this reference is not really weak. The reference hold the description
    // and the description hold the reference.
    if( !(*ppNewDescription)->bOnDemand )
    {
        // nor OnDemand so the description must be acquired if registered
        typelib_typedescription_acquire( *ppNewDescription );
    }

    pTDR->pType = *ppNewDescription;
    (*ppNewDescription)->pWeakRef = pTDR;
    OSL_ASSERT( rtl_ustr_compare( pTDR->pTypeName->buffer, (*ppNewDescription)->pTypeName->buffer ) == 0 );
    OSL_ASSERT( pTDR->eTypeClass == (*ppNewDescription)->eTypeClass );
}

//------------------------------------------------------------------------
static inline sal_Bool type_equals(
    typelib_TypeDescriptionReference * p1, typelib_TypeDescriptionReference * p2 )
    SAL_THROW(())
{
    return (p1 == p2 ||
            (p1->eTypeClass == p2->eTypeClass &&
             p1->pTypeName->length == p2->pTypeName->length &&
             rtl_ustr_compare( p1->pTypeName->buffer, p2->pTypeName->buffer ) == 0));
}
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescription_equals(
    const typelib_TypeDescription * p1, const typelib_TypeDescription * p2 )
    SAL_THROW_EXTERN_C()
{
    return type_equals(
        (typelib_TypeDescriptionReference *)p1, (typelib_TypeDescriptionReference *)p2 );
}

//------------------------------------------------------------------------
extern "C" sal_Int32 SAL_CALL typelib_typedescription_getAlignedUnoSize(
    const typelib_TypeDescription * pTypeDescription,
    sal_Int32 nOffset, sal_Int32 & rMaxIntegralTypeSize )
    SAL_THROW_EXTERN_C()
{
    sal_Int32 nSize;
    if( pTypeDescription->nSize )
    {
        // size and alignment are set
        rMaxIntegralTypeSize = pTypeDescription->nAlignment;
        nSize = pTypeDescription->nSize;
    }
    else
    {
        nSize = 0;
        rMaxIntegralTypeSize = 1;

        OSL_ASSERT( typelib_TypeClass_TYPEDEF != pTypeDescription->eTypeClass );

        switch( pTypeDescription->eTypeClass )
        {
            case typelib_TypeClass_INTERFACE:
                // FEATURE_INTERFACE
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( void * ));
                break;
            case typelib_TypeClass_UNION:
                {
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof(sal_Int64));
                for ( sal_Int32 nPos = ((typelib_UnionTypeDescription *)pTypeDescription)->nMembers; nPos--; )
                {
                    typelib_TypeDescription * pTD = 0;
                    TYPELIB_DANGER_GET( &pTD, ((typelib_UnionTypeDescription *)pTypeDescription)->ppTypeRefs[nPos] );
                    sal_Int32 nMaxIntegralTypeSize;
                    sal_Int32 nMemberSize = typelib_typedescription_getAlignedUnoSize( pTD, (sal_Int32)(sizeof(sal_Int64)), nMaxIntegralTypeSize );
                    TYPELIB_DANGER_RELEASE( pTD );
                    if (nSize < nMemberSize)
                        nSize = nMemberSize;
                    if (rMaxIntegralTypeSize < nMaxIntegralTypeSize)
                        rMaxIntegralTypeSize = nMaxIntegralTypeSize;
                }
                ((typelib_UnionTypeDescription *)pTypeDescription)->nValueOffset = rMaxIntegralTypeSize;
                }
                break;
            case typelib_TypeClass_ENUM:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( typelib_TypeClass ));
                break;
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
                // FEATURE_EMPTYCLASS
                {
                typelib_CompoundTypeDescription * pTmp = (typelib_CompoundTypeDescription *)pTypeDescription;
                sal_Int32 nStructSize = 0;
                if( pTmp->pBaseTypeDescription )
                {
                    // inherit structs extends the base struct.
                    nStructSize = pTmp->pBaseTypeDescription->aBase.nSize;
                    rMaxIntegralTypeSize = pTmp->pBaseTypeDescription->aBase.nAlignment;
                 }
                for( sal_Int32 i = 0; i < pTmp->nMembers; i++ )
                {
                    typelib_TypeDescription * pMemberType = 0;
                    typelib_TypeDescriptionReference * pMemberRef = pTmp->ppTypeRefs[i];

                    sal_Int32 nMaxIntegral;
                    if (pMemberRef->eTypeClass == typelib_TypeClass_INTERFACE
                        || pMemberRef->eTypeClass == typelib_TypeClass_SEQUENCE)
                    {
                        nMaxIntegral = (sal_Int32)(sizeof(void *));
                        nStructSize = newAlignedSize( nStructSize, nMaxIntegral, nMaxIntegral );
                    }
                    else
                    {
                        TYPELIB_DANGER_GET( &pMemberType, pMemberRef );
                        nStructSize = typelib_typedescription_getAlignedUnoSize(
                            pMemberType, nStructSize, nMaxIntegral );
                        TYPELIB_DANGER_RELEASE( pMemberType );
                    }
                    if( nMaxIntegral > rMaxIntegralTypeSize )
                        rMaxIntegralTypeSize = nMaxIntegral;
                }
#ifdef __m68k__
                // Anything that is at least 16 bits wide is aligned on a 16-bit
                // boundary on the m68k default abi
                sal_Int32 nMaxAlign = (rMaxIntegralTypeSize > 2) ? 2 : rMaxIntegralTypeSize;
                nStructSize = (nStructSize + nMaxAlign -1) / nMaxAlign * nMaxAlign;
#else
                // Example: A { double; int; } structure has a size of 16 instead of 10. The
                // compiler must follow this rule if it is possible to access members in arrays through:
                // (Element *)((char *)pArray + sizeof( Element ) * ElementPos)
                nStructSize = (nStructSize + rMaxIntegralTypeSize -1)
                                / rMaxIntegralTypeSize * rMaxIntegralTypeSize;
#endif
                nSize += nStructSize;
                }
                break;
            case typelib_TypeClass_ARRAY:
                {
                typelib_TypeDescription * pTD = 0;
                TYPELIB_DANGER_GET( &pTD, ((typelib_IndirectTypeDescription *)pTypeDescription)->pType );
                rMaxIntegralTypeSize = pTD->nSize;
                TYPELIB_DANGER_RELEASE( pTD );
                nSize = ((typelib_ArrayTypeDescription *)pTypeDescription)->nTotalElements * rMaxIntegralTypeSize;
                }
                break;
            case typelib_TypeClass_SEQUENCE:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( void * ));
                break;
            case typelib_TypeClass_ANY:
                // FEATURE_ANY
                nSize = (sal_Int32)(sizeof( uno_Any ));
                rMaxIntegralTypeSize = (sal_Int32)(sizeof( void * ));
                break;
            case typelib_TypeClass_TYPE:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( typelib_TypeDescriptionReference * ));
                break;
            case typelib_TypeClass_BOOLEAN:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( sal_Bool ));
                break;
            case typelib_TypeClass_CHAR:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( sal_Unicode ));
                break;
            case typelib_TypeClass_STRING:
                // FEATURE_STRING
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( rtl_uString * ));
                break;
            case typelib_TypeClass_FLOAT:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( float ));
                break;
            case typelib_TypeClass_DOUBLE:
#ifdef AIX
                //See previous AIX ifdef comment for an explanation
                nSize = (sal_Int32)(sizeof(double));
                rMaxIntegralTypeSize = (sal_Int32)(sizeof(void*));
#else
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( double ));
#endif
                break;
            case typelib_TypeClass_BYTE:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( sal_Int8 ));
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( sal_Int16 ));
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( sal_Int32 ));
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                nSize = rMaxIntegralTypeSize = (sal_Int32)(sizeof( sal_Int64 ));
                break;
            case typelib_TypeClass_UNKNOWN:
            case typelib_TypeClass_SERVICE:
            case typelib_TypeClass_MODULE:
            default:
                OSL_FAIL( "not convertible type" );
        };
    }

    return newAlignedSize( nOffset, nSize, rMaxIntegralTypeSize );
}

//------------------------------------------------------------------------

namespace {

typelib_TypeDescriptionReference ** copyExceptions(
    sal_Int32 count, typelib_TypeDescriptionReference ** source)
{
    typelib_TypeDescriptionReference ** p
        = new typelib_TypeDescriptionReference *[count];
    for (sal_Int32 i = 0; i < count; ++i) {
        typelib_typedescriptionreference_acquire(p[i] = source[i]);
    }
    return p;
}

bool createDerivedInterfaceMemberDescription(
    typelib_TypeDescription ** result, rtl::OUString const & name,
    typelib_TypeDescriptionReference * baseRef,
    typelib_TypeDescription const * base, typelib_TypeDescription * interface,
    sal_Int32 index, sal_Int32 position)
{
    if (baseRef != 0 && base != 0 && interface != 0) {
        switch (base->eTypeClass) {
        case typelib_TypeClass_INTERFACE_METHOD:
            {
                typelib_typedescription_newEmpty(
                    result, typelib_TypeClass_INTERFACE_METHOD, name.pData);
                typelib_InterfaceMethodTypeDescription const * baseMethod
                    = reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(base);
                typelib_InterfaceMethodTypeDescription * newMethod
                    = reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription * >(*result);
                newMethod->aBase.nPosition = position;
                rtl_uString_acquire(
                    newMethod->aBase.pMemberName
                    = baseMethod->aBase.pMemberName);
                typelib_typedescriptionreference_acquire(
                    newMethod->pReturnTypeRef = baseMethod->pReturnTypeRef);
                newMethod->nParams = baseMethod->nParams;
                newMethod->pParams = new typelib_MethodParameter[
                    newMethod->nParams];
                for (sal_Int32 i = 0; i < newMethod->nParams; ++i) {
                    rtl_uString_acquire(
                        newMethod->pParams[i].pName
                        = baseMethod->pParams[i].pName);
                    typelib_typedescriptionreference_acquire(
                        newMethod->pParams[i].pTypeRef
                        = baseMethod->pParams[i].pTypeRef);
                    newMethod->pParams[i].bIn = baseMethod->pParams[i].bIn;
                    newMethod->pParams[i].bOut = baseMethod->pParams[i].bOut;
                }
                newMethod->nExceptions = baseMethod->nExceptions;
                newMethod->ppExceptions = copyExceptions(
                    baseMethod->nExceptions, baseMethod->ppExceptions);
                newMethod->bOneWay = baseMethod->bOneWay;
                newMethod->pInterface
                    = reinterpret_cast< typelib_InterfaceTypeDescription * >(
                        interface);
                newMethod->pBaseRef = baseRef;
                newMethod->nIndex = index;
                return true;
            }

        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            {
                typelib_typedescription_newEmpty(
                    result, typelib_TypeClass_INTERFACE_ATTRIBUTE, name.pData);
                typelib_InterfaceAttributeTypeDescription const * baseAttribute
                    = reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(base);
                typelib_InterfaceAttributeTypeDescription * newAttribute
                    = reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription * >(*result);
                newAttribute->aBase.nPosition = position;
                rtl_uString_acquire(
                    newAttribute->aBase.pMemberName
                    = baseAttribute->aBase.pMemberName);
                newAttribute->bReadOnly = baseAttribute->bReadOnly;
                typelib_typedescriptionreference_acquire(
                    newAttribute->pAttributeTypeRef
                    = baseAttribute->pAttributeTypeRef);
                newAttribute->pInterface
                    = reinterpret_cast< typelib_InterfaceTypeDescription * >(
                        interface);
                newAttribute->pBaseRef = baseRef;
                newAttribute->nIndex = index;
                newAttribute->nGetExceptions = baseAttribute->nGetExceptions;
                newAttribute->ppGetExceptions = copyExceptions(
                    baseAttribute->nGetExceptions,
                    baseAttribute->ppGetExceptions);
                newAttribute->nSetExceptions = baseAttribute->nSetExceptions;
                newAttribute->ppSetExceptions = copyExceptions(
                    baseAttribute->nSetExceptions,
                    baseAttribute->ppSetExceptions);
                return true;
            }

        default:
            break;
        }
    }
    return false;
}

}

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescription_getByName(
    typelib_TypeDescription ** ppRet, rtl_uString * pName )
    SAL_THROW_EXTERN_C()
{
    if( *ppRet )
    {
        typelib_typedescription_release( (*ppRet) );
        *ppRet = 0;
    }

    static sal_Bool bInited = sal_False;
    TypeDescriptor_Init_Impl &rInit = Init::get();

    if( !bInited )
    {
        // guard against multi thread access
        MutexGuard aGuard( rInit.getMutex() );
        if( !bInited )
        {
            // avoid recursion during the next ...new calls
            bInited = sal_True;

            rtl_uString * pTypeName = 0;
            typelib_TypeDescription * pType = 0;
            rtl_uString_newFromAscii( &pTypeName, "type" );
            typelib_typedescription_new( &pType, typelib_TypeClass_TYPE, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "void" );
            typelib_typedescription_new( &pType, typelib_TypeClass_VOID, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "boolean" );
            typelib_typedescription_new( &pType, typelib_TypeClass_BOOLEAN, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "char" );
            typelib_typedescription_new( &pType, typelib_TypeClass_CHAR, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "byte" );
            typelib_typedescription_new( &pType, typelib_TypeClass_BYTE, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "string" );
            typelib_typedescription_new( &pType, typelib_TypeClass_STRING, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "short" );
            typelib_typedescription_new( &pType, typelib_TypeClass_SHORT, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "unsigned short" );
            typelib_typedescription_new( &pType, typelib_TypeClass_UNSIGNED_SHORT, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "long" );
            typelib_typedescription_new( &pType, typelib_TypeClass_LONG, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "unsigned long" );
            typelib_typedescription_new( &pType, typelib_TypeClass_UNSIGNED_LONG, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "hyper" );
            typelib_typedescription_new( &pType, typelib_TypeClass_HYPER, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "unsigned hyper" );
            typelib_typedescription_new( &pType, typelib_TypeClass_UNSIGNED_HYPER, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "float" );
            typelib_typedescription_new( &pType, typelib_TypeClass_FLOAT, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "double" );
            typelib_typedescription_new( &pType, typelib_TypeClass_DOUBLE, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            rtl_uString_newFromAscii( &pTypeName, "any" );
            typelib_typedescription_new( &pType, typelib_TypeClass_ANY, pTypeName, 0, 0, 0 );
            typelib_typedescription_register( &pType );
            typelib_typedescription_release( pType );
            rtl_uString_release( pTypeName );
        }
    }

    typelib_TypeDescriptionReference * pTDR = 0;
    typelib_typedescriptionreference_getByName( &pTDR, pName );
    if( pTDR )
    {
        {
        // guard against multi thread access
        MutexGuard aGuard( rInit.getMutex() );
        // pTDR->pType->pWeakRef == 0 means that the description is empty
        if( pTDR->pType && pTDR->pType->pWeakRef )
        {
            typelib_typedescription_acquire( pTDR->pType );
            *ppRet = pTDR->pType;
        }
        }
        typelib_typedescriptionreference_release( pTDR );
    }

    if (0 == *ppRet)
    {
        // check for sequence
        OUString const & name = *reinterpret_cast< OUString const * >( &pName );
        if (2 < name.getLength() && '[' == name[ 0 ])
        {
            OUString element_name( name.copy( 2 ) );
            typelib_TypeDescription * element_td = 0;
            typelib_typedescription_getByName( &element_td, element_name.pData );
            if (0 != element_td)
            {
                typelib_typedescription_new(
                    ppRet, typelib_TypeClass_SEQUENCE, pName, element_td->pWeakRef, 0, 0 );
                // register?
                typelib_typedescription_release( element_td );
            }
        }
        if (0 == *ppRet)
        {
            // Check for derived interface member type:
            sal_Int32 i1 = name.lastIndexOf(
                rtl::OUString(":@"));
            if (i1 >= 0) {
                sal_Int32 i2 = i1 + RTL_CONSTASCII_LENGTH(":@");
                sal_Int32 i3 = name.indexOf(',', i2);
                if (i3 >= 0) {
                    sal_Int32 i4 = name.indexOf(':', i3);
                    if (i4 >= 0) {
                        typelib_TypeDescriptionReference * pBaseRef = 0;
                        typelib_TypeDescription * pBase = 0;
                        typelib_TypeDescription * pInterface = 0;
                        typelib_typedescriptionreference_getByName(
                            &pBaseRef, name.copy(0, i1).pData);
                        if (pBaseRef != 0) {
                            typelib_typedescriptionreference_getDescription(
                                &pBase, pBaseRef);
                        }
                        typelib_typedescription_getByName(
                            &pInterface, name.copy(i4 + 1).pData);
                        if (!createDerivedInterfaceMemberDescription(
                                ppRet, name, pBaseRef, pBase, pInterface,
                                name.copy(i2, i3 - i2).toInt32(),
                                name.copy(i3 + 1, i4 - i3 - 1).toInt32()))
                        {
                            if (pInterface != 0) {
                                typelib_typedescription_release(pInterface);
                            }
                            if (pBase != 0) {
                                typelib_typedescription_release(pBase);
                            }
                            if (pBaseRef != 0) {
                                typelib_typedescriptionreference_release(
                                    pBaseRef);
                            }
                        }
                    }
                }
            }
        }
        if (0 == *ppRet)
        {
            // on demand access
            rInit.callChain( ppRet, pName );
        }

        if( *ppRet )
        {
            // typedescription found
            if (typelib_TypeClass_TYPEDEF == (*ppRet)->eTypeClass)
            {
                typelib_TypeDescription * pTD = 0;
                typelib_typedescriptionreference_getDescription(
                    &pTD, ((typelib_IndirectTypeDescription *)*ppRet)->pType );
                typelib_typedescription_release( *ppRet );
                *ppRet = pTD;
            }
            else
            {
                // set to on demand
                (*ppRet)->bOnDemand = sal_True;
                // The type description is hold by the reference until
                // on demand is activated.
                typelib_typedescription_register( ppRet );

                // insert into the chache
                MutexGuard aGuard( rInit.getMutex() );
                if( !rInit.pCache )
                    rInit.pCache = new TypeDescriptionList_Impl;
                if( (sal_Int32)rInit.pCache->size() >= nCacheSize )
                {
                    typelib_typedescription_release( rInit.pCache->front() );
                    rInit.pCache->pop_front();
                }
                // descriptions in the cache must be acquired!
                typelib_typedescription_acquire( *ppRet );
                rInit.pCache->push_back( *ppRet );
            }
        }
    }
}

extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_newByAsciiName(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    const sal_Char * pTypeName )
    SAL_THROW_EXTERN_C()
{
    OUString aTypeName( OUString::createFromAscii( pTypeName ) );
    typelib_typedescriptionreference_new( ppTDR, eTypeClass, aTypeName.pData );
}
//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_new(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass, rtl_uString * pTypeName )
    SAL_THROW_EXTERN_C()
{
    TypeDescriptor_Init_Impl &rInit = Init::get();
    if( eTypeClass == typelib_TypeClass_TYPEDEF )
    {
        // on demand access
        typelib_TypeDescription * pRet = 0;
        rInit.callChain( &pRet, pTypeName );
        if( pRet )
        {
            // typedescription found
            if (typelib_TypeClass_TYPEDEF == pRet->eTypeClass)
            {
                typelib_typedescriptionreference_acquire(
                    ((typelib_IndirectTypeDescription *)pRet)->pType );
                if (*ppTDR)
                    typelib_typedescriptionreference_release( *ppTDR );
                *ppTDR = ((typelib_IndirectTypeDescription *)pRet)->pType;
                typelib_typedescription_release( pRet );
            }
            else
            {
                // set to on demand
                pRet->bOnDemand = sal_True;
                // The type description is hold by the reference until
                // on demand is activated.
                typelib_typedescription_register( &pRet );

                // insert into the chache
                MutexGuard aGuard( rInit.getMutex() );
                if( !rInit.pCache )
                    rInit.pCache = new TypeDescriptionList_Impl;
                if( (sal_Int32)rInit.pCache->size() >= nCacheSize )
                {
                    typelib_typedescription_release( rInit.pCache->front() );
                    rInit.pCache->pop_front();
                }
                rInit.pCache->push_back( pRet );
                // pRet kept acquired for cache

                typelib_typedescriptionreference_acquire( pRet->pWeakRef );
                if (*ppTDR)
                    typelib_typedescriptionreference_release( *ppTDR );
                *ppTDR = pRet->pWeakRef;
            }
        }
        else if (*ppTDR)
        {
#if OSL_DEBUG_LEVEL > 1
            OString aStr( rtl::OUStringToOString( pTypeName, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( !"### typedef not found: ", aStr.getStr() );
#endif
            typelib_typedescriptionreference_release( *ppTDR );
            *ppTDR = 0;
        }
        return;
    }

    MutexGuard aGuard( rInit.getMutex() );
    typelib_typedescriptionreference_getByName( ppTDR, pTypeName );
    if( *ppTDR )
        return;

    if( reallyWeak( eTypeClass ) )
    {
        typelib_TypeDescriptionReference * pTDR = new typelib_TypeDescriptionReference();
#if OSL_DEBUG_LEVEL > 1
        osl_atomic_increment( &rInit.nTypeDescriptionReferenceCount );
#endif
        pTDR->nRefCount = 1;
        pTDR->nStaticRefCount = 0;
        pTDR->eTypeClass = eTypeClass;
        pTDR->pUniqueIdentifier = 0;
        pTDR->pReserved = 0;
        rtl_uString_acquire( pTDR->pTypeName = pTypeName );
        pTDR->pType = 0;
        *ppTDR = pTDR;
    }
    else
    {
        typelib_typedescription_newEmpty( (typelib_TypeDescription ** )ppTDR, eTypeClass, pTypeName );
        // description will be registered but not acquired
        (*(typelib_TypeDescription ** )ppTDR)->bOnDemand = sal_True;
        (*(typelib_TypeDescription ** )ppTDR)->bComplete = sal_False;
    }

    if( !rInit.pWeakMap )
        rInit.pWeakMap = new WeakMap_Impl;
    // Heavy hack, the const sal_Unicode * is hold by the typedescription reference
    // not registered
    rInit.pWeakMap->operator[]( (*ppTDR)->pTypeName->buffer ) = *ppTDR;
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_acquire(
    typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C()
{
    osl_atomic_increment( &pRef->nRefCount );
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_release(
    typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C()
{
    // Is it a type description?
    if( reallyWeak( pRef->eTypeClass ) )
    {
        if( ! osl_atomic_decrement( &pRef->nRefCount ) )
        {
            TypeDescriptor_Init_Impl &rInit = Init::get();
            if( rInit.pWeakMap )
            {
                MutexGuard aGuard( rInit.getMutex() );
                WeakMap_Impl::iterator aIt = rInit.pWeakMap->find( (sal_Unicode*)pRef->pTypeName->buffer );
                if( !(aIt == rInit.pWeakMap->end()) && (*aIt).second == pRef )
                {
                    // remove only if it contains the same object
                    rInit.pWeakMap->erase( aIt );
                }
            }

            rtl_uString_release( pRef->pTypeName );
            OSL_ASSERT( pRef->pType == 0 );
#if OSL_DEBUG_LEVEL > 1
            osl_atomic_decrement( &rInit.nTypeDescriptionReferenceCount );
#endif
            delete pRef;
        }
    }
    else
    {
        typelib_typedescription_release( (typelib_TypeDescription *)pRef );
    }
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_getDescription(
    typelib_TypeDescription ** ppRet, typelib_TypeDescriptionReference * pRef )
    SAL_THROW_EXTERN_C()
{
    if( *ppRet )
    {
        typelib_typedescription_release( *ppRet );
        *ppRet = 0;
    }

    if( !reallyWeak( pRef->eTypeClass ) && pRef->pType && pRef->pType->pWeakRef )
    {
        // reference is a description and initialized
        osl_atomic_increment( &((typelib_TypeDescription *)pRef)->nRefCount );
        *ppRet = (typelib_TypeDescription *)pRef;
        return;
    }

    {
    MutexGuard aGuard( Init::get().getMutex() );
    // pRef->pType->pWeakRef == 0 means that the description is empty
    if( pRef->pType && pRef->pType->pWeakRef )
    {
        sal_Int32 n = osl_atomic_increment( &pRef->pType->nRefCount );
        if( n > 1 )
        {
            // The refence is incremented. The object cannot be destroyed.
            // Release the guard at the earliest point.
            *ppRet = pRef->pType;
            return;
        }
        else
        {
            osl_atomic_decrement( &pRef->pType->nRefCount );
            // detruction of this type in progress (another thread!)
            // no acces through this weak reference
            pRef->pType = 0;
        }
    }
    }

    typelib_typedescription_getByName( ppRet, pRef->pTypeName );
    OSL_ASSERT( !*ppRet || rtl_ustr_compare( pRef->pTypeName->buffer, (*ppRet)->pTypeName->buffer ) == 0 );
    OSL_ASSERT( !*ppRet || pRef->eTypeClass == (*ppRet)->eTypeClass );
    OSL_ASSERT( !*ppRet || pRef == (*ppRet)->pWeakRef );
    pRef->pType = *ppRet;
}

//------------------------------------------------------------------------
extern "C" void SAL_CALL typelib_typedescriptionreference_getByName(
    typelib_TypeDescriptionReference ** ppRet, rtl_uString * pName )
    SAL_THROW_EXTERN_C()
{
    if( *ppRet )
    {
        typelib_typedescriptionreference_release( *ppRet );
        *ppRet = 0;
    }
    TypeDescriptor_Init_Impl &rInit = Init::get();
    if( rInit.pWeakMap )
    {
        MutexGuard aGuard( rInit.getMutex() );
        WeakMap_Impl::const_iterator aIt = rInit.pWeakMap->find( (sal_Unicode*)pName->buffer );
        if( !(aIt == rInit.pWeakMap->end()) ) // != failed on msc4.2
        {
            sal_Int32 n = osl_atomic_increment( &(*aIt).second->nRefCount );
            if( n > 1 )
            {
                // The refence is incremented. The object cannot be destroyed.
                // Release the guard at the earliest point.
                *ppRet = (*aIt).second;
            }
            else
            {
                // detruction of this type in progress (another thread!)
                // no acces through this weak reference
                osl_atomic_decrement( &(*aIt).second->nRefCount );
            }
        }
    }
}

//------------------------------------------------------------------------
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescriptionreference_equals(
    const typelib_TypeDescriptionReference * p1,
    const typelib_TypeDescriptionReference * p2 )
    SAL_THROW_EXTERN_C()
{
    return (p1 == p2 ||
            (p1->eTypeClass == p2->eTypeClass &&
             p1->pTypeName->length == p2->pTypeName->length &&
             rtl_ustr_compare( p1->pTypeName->buffer, p2->pTypeName->buffer ) == 0));
}

//##################################################################################################
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_typedescriptionreference_assign(
    typelib_TypeDescriptionReference ** ppDest,
    typelib_TypeDescriptionReference * pSource )
    SAL_THROW_EXTERN_C()
{
    if (*ppDest != pSource)
    {
        ::typelib_typedescriptionreference_acquire( pSource );
        ::typelib_typedescriptionreference_release( *ppDest );
        *ppDest = pSource;
    }
}

//##################################################################################################
extern "C" CPPU_DLLPUBLIC void SAL_CALL typelib_setCacheSize( sal_Int32 nNewSize )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( nNewSize >= 0, "### illegal cache size given!" );
    if (nNewSize >= 0)
    {
        TypeDescriptor_Init_Impl &rInit = Init::get();
        MutexGuard aGuard( rInit.getMutex() );
        if ((nNewSize < nCacheSize) && rInit.pCache)
        {
            while ((sal_Int32)rInit.pCache->size() != nNewSize)
            {
                typelib_typedescription_release( rInit.pCache->front() );
                rInit.pCache->pop_front();
            }
        }
        nCacheSize = nNewSize;
    }
}


static const sal_Bool s_aAssignableFromTab[11][11] =
{
                         /* from CH,BO,BY,SH,US,LO,UL,HY,UH,FL,DO */
/* TypeClass_CHAR */            { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BOOLEAN */         { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_BYTE */            { 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
/* TypeClass_SHORT */           { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_SHORT */  { 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
/* TypeClass_LONG */            { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_UNSIGNED_LONG */   { 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
/* TypeClass_HYPER */           { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_UNSIGNED_HYPER */  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
/* TypeClass_FLOAT */           { 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0 },
/* TypeClass_DOUBLE */          { 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1 }
};

//##################################################################################################
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescriptionreference_isAssignableFrom(
    typelib_TypeDescriptionReference * pAssignable,
    typelib_TypeDescriptionReference * pFrom )
    SAL_THROW_EXTERN_C()
{
    if (pAssignable && pFrom)
    {
        typelib_TypeClass eAssignable = pAssignable->eTypeClass;
        typelib_TypeClass eFrom       = pFrom->eTypeClass;

        if (eAssignable == typelib_TypeClass_ANY) // anything can be assigned to an any .)
            return sal_True;
        if (eAssignable == eFrom)
        {
            if (type_equals( pAssignable, pFrom )) // first shot
            {
                return sal_True;
            }
            else
            {
                switch (eAssignable)
                {
                case typelib_TypeClass_STRUCT:
                case typelib_TypeClass_EXCEPTION:
                {
                    typelib_TypeDescription * pFromDescr = 0;
                    TYPELIB_DANGER_GET( &pFromDescr, pFrom );
                    if (! ((typelib_CompoundTypeDescription *)pFromDescr)->pBaseTypeDescription)
                    {
                        TYPELIB_DANGER_RELEASE( pFromDescr );
                        return sal_False;
                    }
                    sal_Bool bRet = typelib_typedescriptionreference_isAssignableFrom(
                        pAssignable,
                        ((typelib_TypeDescription *)((typelib_CompoundTypeDescription *)pFromDescr)->pBaseTypeDescription)->pWeakRef );
                    TYPELIB_DANGER_RELEASE( pFromDescr );
                    return bRet;
                }
                case typelib_TypeClass_INTERFACE:
                {
                    typelib_TypeDescription * pFromDescr = 0;
                    TYPELIB_DANGER_GET( &pFromDescr, pFrom );
                    typelib_InterfaceTypeDescription * pFromIfc
                        = reinterpret_cast<
                            typelib_InterfaceTypeDescription * >(pFromDescr);
                    bool bRet = false;
                    for (sal_Int32 i = 0; i < pFromIfc->nBaseTypes; ++i) {
                        if (typelib_typedescriptionreference_isAssignableFrom(
                                pAssignable,
                                pFromIfc->ppBaseTypes[i]->aBase.pWeakRef))
                        {
                            bRet = true;
                            break;
                        }
                    }
                    TYPELIB_DANGER_RELEASE( pFromDescr );
                    return bRet;
                }
                default:
                {
                    return sal_False;
                }
                }
            }
        }
        return (eAssignable >= typelib_TypeClass_CHAR && eAssignable <= typelib_TypeClass_DOUBLE &&
                eFrom >= typelib_TypeClass_CHAR && eFrom <= typelib_TypeClass_DOUBLE &&
                s_aAssignableFromTab[eAssignable-1][eFrom-1]);
    }
    return sal_False;
}
//##################################################################################################
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescription_isAssignableFrom(
    typelib_TypeDescription * pAssignable,
    typelib_TypeDescription * pFrom )
    SAL_THROW_EXTERN_C()
{
    return typelib_typedescriptionreference_isAssignableFrom(
        pAssignable->pWeakRef, pFrom->pWeakRef );
}

//##################################################################################################
extern "C" CPPU_DLLPUBLIC sal_Bool SAL_CALL typelib_typedescription_complete(
    typelib_TypeDescription ** ppTypeDescr )
    SAL_THROW_EXTERN_C()
{
    return complete(ppTypeDescr, true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
