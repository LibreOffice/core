/*************************************************************************
 *
 *  $RCSfile: typelib.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2000-09-28 14:47:56 $
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

#ifdef CPPU_ASSERTIONS
#define CPPU_TRACE(x) OSL_TRACE(x)
#else
#define CPPU_TRACE(x)
#endif

#include <stl/hash_map>
#include <stl/list>

#include <stdarg.h>
#include <stdlib.h>
#include <memory.h>
#ifdef MACOSX
#include <sys/types.h>
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#if defined(SOLARIS)
#include <alloca.h>
#endif
#include <new.h>

#ifndef _OSL_INTERLCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _UNO_ANY2_H_
#include <uno/any2.h>
#endif

using namespace rtl;
using namespace std;
using namespace osl;


//------------------------------------------------------------------------
//------------------------------------------------------------------------
#ifdef SAL_W32
#pragma pack(push, 8)
#elif defined(SAL_OS2)
#pragma pack(8)
#endif

/**
 * The double member determin the alignment.
 * Under Os2 and MS-Windows the Alignment is min( 8, sizeof( type ) ).
 * The aligment of a strukture is min( 8, sizeof( max basic type ) ), the greatest basic type
 * determine the aligment.
 */
struct AlignSize_Impl
{
    sal_Int16   nInt16;
    double      dDouble;
};

#ifdef SAL_W32
#pragma pack(pop)
#elif defined(SAL_OS2)
#pragma pack()
#endif

// the value of the maximal alignment
static sal_Int32 nMaxAlignment = (sal_Int32)&((AlignSize_Impl *) 16)->dDouble - 16;

static inline sal_Int32 adjustAlignment( sal_Int32 nRequestedAlignment )
{
    if( nRequestedAlignment > nMaxAlignment )
        nRequestedAlignment = nMaxAlignment;
    return nRequestedAlignment;
}

/**
 * Calculate the new size of the struktur.
 */
static inline sal_Int32 newAlignedSize( sal_Int32 OldSize, sal_Int32 ElementSize, sal_Int32 NeededAlignment )
{
    NeededAlignment = adjustAlignment( NeededAlignment );
    return (OldSize + NeededAlignment -1) / NeededAlignment * NeededAlignment + ElementSize;
}

inline sal_Bool reallyWeak( typelib_TypeClass eTypeClass )
{
    return TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK( eTypeClass );
}

inline sal_Int32 getDescriptionSize( typelib_TypeClass eTypeClass )
{
    OSL_ASSERT( typelib_TypeClass_TYPEDEF != eTypeClass );

    sal_Int32 nSize;
    // The reference is the description
    // if the description is empty, than it must be filled with
    // the new description
    switch( eTypeClass )
    {
        case typelib_TypeClass_ARRAY:
        case typelib_TypeClass_SEQUENCE:
            nSize = (sal_Int32)sizeof( typelib_IndirectTypeDescription );
        break;

        case typelib_TypeClass_UNION:
            nSize = (sal_Int32)sizeof( typelib_UnionTypeDescription );
        break;

        case typelib_TypeClass_STRUCT:
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
void SAL_CALL typelib_typedescriptionreference_getByName(
    typelib_TypeDescriptionReference ** ppRet,
    rtl_uString * pName );

//-----------------------------------------------------------------------------
struct equalStr_Impl
{
  sal_Bool operator()(const sal_Unicode * const & s1, const sal_Unicode * const & s2) const
        { return 0 == rtl_ustr_compare( s1, s2 ); }
};

//-----------------------------------------------------------------------------
struct hashStr_Impl
{
    size_t operator()(const sal_Unicode * const & s) const
        { return rtl_ustr_hashCode( s ); }
};


//-----------------------------------------------------------------------------
// Heavy hack, the const sal_Unicode * is hold by the typedescription reference
typedef hash_map< const sal_Unicode *, typelib_TypeDescriptionReference *,
                  hashStr_Impl, equalStr_Impl > WeakMap_Impl;

typedef pair< void *, typelib_typedescription_Callback > CallbackEntry;
typedef list< CallbackEntry > CallbackSet_Impl;
typedef list< typelib_TypeDescription * > TypeDescriptionList_Impl;

// # of cached elements
static sal_Int32 nCacheSize = 256;

//-----------------------------------------------------------------------------
/**
 * All members must set initial to 0 and no constructor is needed. So it
 * doesn't care, when this class is static initialized.<BR>
 */
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

    inline Mutex & getMutex();

    inline void callChain(
        typelib_TypeDescription ** ppRet, rtl_uString * pName );

#ifdef CPPU_ASSERTIONS
    // only for debugging
    sal_Int32           nTypeDescriptionCount;
    sal_Int32           nCompoundTypeDescriptionCount;
    sal_Int32           nUnionTypeDescriptionCount;
    sal_Int32           nIndirectTypeDescriptionCount;
    sal_Int32           nEnumTypeDescriptionCount;
    sal_Int32           nInterfaceMethodTypeDescriptionCount;
    sal_Int32           nInterfaceAttributeTypeDescriptionCount;
    sal_Int32           nInterfaceTypeDescriptionCount;
    sal_Int32           nTypeDescriptionReferenceCount;
#endif
    ~TypeDescriptor_Init_Impl();
};
//__________________________________________________________________________________________________
inline Mutex & TypeDescriptor_Init_Impl::getMutex()
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
{
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
    if (*ppRet)
    {
        typelib_typedescription_release( *ppRet );
        *ppRet = 0;
    }
}

// never called
#if defined(CPPU_LEAK_STATIC_DATA) && defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
static void dumb_sunpro5_must_have_dtor_stl_hashmap_code_if_compiled_with_minus_g()
{
    delete (WeakMap_Impl *)0xbeef1e;
}
#endif
//__________________________________________________________________________________________________
TypeDescriptor_Init_Impl::~TypeDescriptor_Init_Impl()
{
#ifndef CPPU_LEAK_STATIC_DATA
    if( pCache )
    {
        TypeDescriptionList_Impl::const_iterator aIt = pCache->begin();
        while( aIt != pCache->end() )
        {
            typelib_typedescription_release( (*aIt) );
            aIt++;
        }
        delete pCache;
        pCache = 0;
    }

    if( pWeakMap )
    {
        sal_Int32 nSize = pWeakMap->size();
        typelib_TypeDescriptionReference ** ppTDR = new typelib_TypeDescriptionReference *[ nSize ];
        // save al weak references
        WeakMap_Impl::const_iterator aIt = pWeakMap->begin();
        sal_Int32 i = 0;
        while( aIt != pWeakMap->end() )
        {
            typelib_typedescriptionreference_acquire( ppTDR[i++] = (*aIt).second );
            ++aIt;
        }

        for( i = 0; i < nSize; i++ )
        {
            sal_Int32 nStaticCounts = (sal_Int32)ppTDR[i]->pReserved;
            OSL_ASSERT( ppTDR[i]->nRefCount > nStaticCounts );
            ppTDR[i]->nRefCount -= nStaticCounts;

            if( ppTDR[i]->pType && !ppTDR[i]->pType->bOnDemand )
            {
                ppTDR[i]->pType->bOnDemand = sal_True;
                typelib_typedescription_release( ppTDR[i]->pType );
            }
            typelib_typedescriptionreference_release( ppTDR[i] );
        }

        delete[] ppTDR;

#ifdef CPPU_ASSERTIONS
        aIt = pWeakMap->begin();
        while( aIt != pWeakMap->end() )
        {
            CPPU_TRACE( "\n" );
            typelib_TypeDescriptionReference * pTDR = (*aIt).second;
            if (pTDR)
            {
                OString aTypeName( OUStringToOString( pTDR->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
                OString aRef( OString::valueOf( pTDR->nRefCount ) );
                CPPU_TRACE( "### remaining type: " );
                CPPU_TRACE( aTypeName.getStr() );
                CPPU_TRACE( ", ref count = " );
                CPPU_TRACE( aRef.getStr() );
            }
            else
            {
                CPPU_TRACE( "### remaining null type entry!?" );
            }
            ++aIt;
        }
#endif

        delete pWeakMap;
        pWeakMap = 0;
    }
#ifdef CPPU_ASSERTIONS
    OSL_ASSERT( nTypeDescriptionCount == 0 );
    OSL_ASSERT( nCompoundTypeDescriptionCount == 0 );
    OSL_ASSERT( nUnionTypeDescriptionCount == 0 );
    OSL_ASSERT( nIndirectTypeDescriptionCount == 0 );
    OSL_ASSERT( nEnumTypeDescriptionCount == 0 );
    OSL_ASSERT( nInterfaceMethodTypeDescriptionCount == 0 );
    OSL_ASSERT( nInterfaceAttributeTypeDescriptionCount == 0 );
    OSL_ASSERT( nInterfaceTypeDescriptionCount == 0 );
    OSL_ASSERT( nTypeDescriptionReferenceCount == 0 );

    OSL_ASSERT( !pCallbacks || pCallbacks->empty() );
#endif
    delete pCallbacks;
    pCallbacks = 0;
#endif // CPPU_LEAK_STATIC_DATA

    // todo: maybe into leak block
    if( pMutex )
    {
        delete pMutex;
        pMutex = 0;
    }
};

static TypeDescriptor_Init_Impl aInit;


//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_registerCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
{
    // todo mt safe: guard is no solution, can not acquire while calling callback!
//      OslGuard aGuard( aInit.getMutex() );
    if( !aInit.pCallbacks )
        aInit.pCallbacks = new CallbackSet_Impl;
    aInit.pCallbacks->push_back( CallbackEntry( pContext, pCallback ) );
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_revokeCallback(
    void * pContext, typelib_typedescription_Callback pCallback )
{
    if( aInit.pCallbacks )
    {
        // todo mt safe: guard is no solution, can not acquire while calling callback!
//          OslGuard aGuard( aInit.getMutex() );
        CallbackEntry aEntry( pContext, pCallback );
        CallbackSet_Impl::iterator iPos( aInit.pCallbacks->begin() );
        while (!(iPos == aInit.pCallbacks->end()))
        {
            if (*iPos == aEntry)
            {
                aInit.pCallbacks->erase( iPos );
                iPos = aInit.pCallbacks->begin();
            }
            else
            {
                ++iPos;
            }
        }
    }
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
sal_Int32 SAL_CALL typelib_typedescription_getAlignedUnoSize(
    const typelib_TypeDescription * pTypeDescription,
    sal_Int32 nOffset,
    sal_Int32 & rMaxIntegralTypeSize );

//------------------------------------------------------------------------
inline static void typelib_typedescription_initTables(
    typelib_TypeDescription * pTD )
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
#ifdef CPPU_ASSERTIONS
            else
            {
                OString aStr( OUStringToOString( pITD->ppAllMembers[i]->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
                CPPU_TRACE( "\n### cannot get attribute type description: " );
                CPPU_TRACE( aStr.getStr() );
            }
#endif
        }
    }

    MutexGuard aGuard( aInit.getMutex() );
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

//------------------------------------------------------------------------
void SAL_CALL typelib_typedescription_newEmpty(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName )
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
        case typelib_TypeClass_SEQUENCE:
        {
            typelib_IndirectTypeDescription * pTmp = new typelib_IndirectTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nIndirectTypeDescriptionCount );
#endif
            pTmp->pType = 0;
        }
        break;

        case typelib_TypeClass_UNION:
        {
            typelib_UnionTypeDescription * pTmp;
            pTmp = new typelib_UnionTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nUnionTypeDescriptionCount );
#endif
            pTmp->nMembers = 0;
            pTmp->pDiscriminantTypeRef = 0;
            pTmp->pDiscriminants = 0;
            pTmp->ppTypeRefs = 0;
            pTmp->ppMemberNames = 0;
            pTmp->pDefaultTypeRef = 0;
        }
        break;

        case typelib_TypeClass_EXCEPTION:
        case typelib_TypeClass_STRUCT:
        {
            // FEATURE_EMPTYCLASS
            typelib_CompoundTypeDescription * pTmp;
            pTmp = new typelib_CompoundTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nCompoundTypeDescriptionCount );
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
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nEnumTypeDescriptionCount );
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
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nInterfaceTypeDescriptionCount );
#endif
            pTmp->pBaseTypeDescription = 0;
            pTmp->nMembers = 0;
            pTmp->ppMembers = 0;
            pTmp->nAllMembers = 0;
            pTmp->ppAllMembers = 0;
            pTmp->nMapFunctionIndexToMemberIndex = 0;
            pTmp->pMapFunctionIndexToMemberIndex = 0;
            pTmp->pMapMemberIndexToFunctionIndex= 0;
        }
        break;

        case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription * pTmp = new typelib_InterfaceMethodTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nInterfaceMethodTypeDescriptionCount );
#endif
            pTmp->aBase.pMemberName = 0;
            pTmp->pReturnTypeRef = 0;
            pTmp->nParams = 0;
            pTmp->pParams = 0;
            pTmp->nExceptions = 0;
            pTmp->ppExceptions = 0;
        }
        break;

        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_InterfaceAttributeTypeDescription * pTmp = new typelib_InterfaceAttributeTypeDescription();
            pRet = (typelib_TypeDescription *)pTmp;
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nInterfaceAttributeTypeDescriptionCount );
#endif
            pTmp->aBase.pMemberName = 0;
            pTmp->pAttributeTypeRef = 0;
        }
        break;

        default:
        {
            pRet = new typelib_TypeDescription();
#ifdef CPPU_ASSERTIONS
            osl_incrementInterlockedCount( &aInit.nTypeDescriptionCount );
#endif
        }
    }

    pRet->nRefCount = 1; // reference count is initially 1
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
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_new(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pType,
    sal_Int32 nMembers,
    typelib_CompoundMember_Init * pMembers )
{
    if (typelib_TypeClass_TYPEDEF == eTypeClass)
    {
        CPPU_TRACE( "### unexpected typedef!" );
        typelib_typedescriptionreference_getDescription( ppRet, pType );
        return;
    }

    typelib_typedescription_newEmpty( ppRet, eTypeClass, pTypeName );

    switch( eTypeClass )
    {
        case typelib_TypeClass_ARRAY:
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
                OSL_ENSHURE( newAlignedSize( 0, ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nSize, ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nAlignment ) == ((typelib_TypeDescription *)pTmp->pBaseTypeDescription)->nSize, "### unexpected offset!" );
            }
            if( nMembers )
            {
                pTmp->nMembers = nMembers;
                pTmp->pMemberOffsets = new sal_Int32[ nMembers ];
                pTmp->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
                pTmp->ppMemberNames = new rtl_uString *[ nMembers ];
                for( sal_Int32 i = 0 ; i < nMembers; i++ )
                {
                    // read the type name
                    pTmp->ppTypeRefs[i] = 0;
                    typelib_typedescriptionreference_new(
                        pTmp->ppTypeRefs +i, pMembers[i].eTypeClass, pMembers[i].pTypeName );
                    // read the member name
                    rtl_uString_acquire( pTmp->ppMemberNames[i] = pMembers[i].pMemberName );
                    // write offset
                    typelib_TypeDescription * pTD = 0;
                    TYPELIB_DANGER_GET( &pTD, pTmp->ppTypeRefs[i] );
                    OSL_ENSHURE( pTD->nSize, "### void member?" );
                    nOffset = newAlignedSize( nOffset, pTD->nSize, pTD->nAlignment );
                    pTmp->pMemberOffsets[i] = nOffset - pTD->nSize;
                    TYPELIB_DANGER_RELEASE( pTD );
                }
            }
        }
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

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newUnion(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    typelib_TypeDescriptionReference * pDiscriminantTypeRef,
    sal_Int64 nDefaultDiscriminant,
    typelib_TypeDescriptionReference * pDefaultTypeRef,
    sal_Int32 nMembers,
    typelib_Union_Init * pMembers )
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
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newEnum(
    typelib_TypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_Int32 nDefaultValue,
    sal_Int32 nEnumValues,
    rtl_uString ** ppEnumNames,
    sal_Int32 * pEnumValues )
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
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newInterface(
    typelib_InterfaceTypeDescription ** ppRet,
    rtl_uString * pTypeName,
    sal_uInt32 nUik1, sal_uInt16 nUik2, sal_uInt16 nUik3, sal_uInt32 nUik4, sal_uInt32 nUik5,
    typelib_TypeDescriptionReference * pBaseInterface,
    sal_Int32 nMembers,
    typelib_TypeDescriptionReference ** ppMembers )
{
    typelib_InterfaceTypeDescription * pITD = 0;
    typelib_typedescription_newEmpty(
        (typelib_TypeDescription **)&pITD, typelib_TypeClass_INTERFACE, pTypeName );

    if( pBaseInterface )
    {
        // set the base interface, may be 0
        ::typelib_typedescriptionreference_getDescription(
            (typelib_TypeDescription **)&pITD->pBaseTypeDescription, pBaseInterface );
        OSL_ASSERT( pITD->pBaseTypeDescription );
    }
    // set the
    pITD->aUik.m_Data1 = nUik1;
    pITD->aUik.m_Data2 = nUik2;
    pITD->aUik.m_Data3 = nUik3;
    pITD->aUik.m_Data4 = nUik4;
    pITD->aUik.m_Data5 = nUik5;

    sal_Int32 nSuperMembers = pITD->pBaseTypeDescription ? pITD->pBaseTypeDescription->nAllMembers : 0;
    pITD->nAllMembers = nMembers + nSuperMembers;
    pITD->nMembers = nMembers;

    if( pITD->nAllMembers )
    {
        // at minimum one member exist, allocate the memory
        pITD->ppAllMembers = new typelib_TypeDescriptionReference *[ pITD->nAllMembers ];
        // the superclass references must not acquired
        if( nSuperMembers )
        {
            rtl_moveMemory( pITD->ppAllMembers, pITD->pBaseTypeDescription->ppAllMembers,
                            nSuperMembers * sizeof( void * ) );
        }

        if( nMembers )
        {
            pITD->ppMembers = pITD->ppAllMembers + nSuperMembers;
        }

        sal_Int32 n = 0;
        // add own members
        for( sal_Int32 i = nSuperMembers; i < pITD->nAllMembers; i++ )
        {
            typelib_typedescriptionreference_acquire( ppMembers[n] );
            pITD->ppAllMembers[i] = ppMembers[n];
            ++n;
        }
    }

    typelib_TypeDescription * pTmp = (typelib_TypeDescription *)pITD;
    if( !reallyWeak( typelib_TypeClass_INTERFACE ) )
        pTmp->pWeakRef = (typelib_TypeDescriptionReference *)pTmp;
    pTmp->nSize = typelib_typedescription_getAlignedUnoSize( pTmp, 0, pTmp->nAlignment );
    pTmp->nAlignment = adjustAlignment( pTmp->nAlignment );
    pTmp->bComplete = sal_False;

    if (*ppRet)
        ::typelib_typedescription_release( (typelib_TypeDescription *)*ppRet );
    *ppRet = pITD;
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newInterfaceMethod(
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
{
    typelib_typedescription_newEmpty(
        (typelib_TypeDescription **)ppRet, typelib_TypeClass_INTERFACE_METHOD, pTypeName );
    typelib_TypeDescription * pTmp = (typelib_TypeDescription *)*ppRet;

    sal_Int32 nOffset = rtl_ustr_lastIndexOfChar_WithLength(
        pTypeName->buffer, pTypeName->length, ':' );

    if( nOffset == -1 )
    {
        // not found
        rtl_uString_acquire( (*ppRet)->aBase.pMemberName = pTypeName );
    }
    else
    {
        rtl_uString_newFromStr_WithLength( &(*ppRet)->aBase.pMemberName,
                                           pTypeName->buffer + nOffset +1,
                                           pTypeName->length - nOffset -1 );
    }

    (*ppRet)->aBase.nPosition = nAbsolutePosition;
    (*ppRet)->bOneWay = bOneWay;
    typelib_typedescriptionreference_new( &(*ppRet)->pReturnTypeRef, eReturnTypeClass, pReturnTypeName );
    (*ppRet)->nParams = nParams;
    (*ppRet)->nExceptions = nExceptions;
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
    if( nExceptions )
    {
        (*ppRet)->ppExceptions  = new typelib_TypeDescriptionReference *[ nExceptions ];

        for( sal_Int32 i = 0; i < nExceptions; i++ )
        {
            (*ppRet)->ppExceptions[i] = 0;
            typelib_typedescriptionreference_new(
                (*ppRet)->ppExceptions + i, typelib_TypeClass_EXCEPTION, ppExceptionNames[i] );
        }
    }
    if( !reallyWeak( typelib_TypeClass_INTERFACE_METHOD ) )
        pTmp->pWeakRef = (typelib_TypeDescriptionReference *)pTmp;
}


//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_newInterfaceAttribute(
    typelib_InterfaceAttributeTypeDescription ** ppRet,
    sal_Int32 nAbsolutePosition,
    rtl_uString * pTypeName,
    typelib_TypeClass eAttributeTypeClass,
    rtl_uString * pAttributeTypeName,
    sal_Bool bReadOnly )
{
    typelib_typedescription_newEmpty(
        (typelib_TypeDescription **)ppRet, typelib_TypeClass_INTERFACE_ATTRIBUTE, pTypeName );
    typelib_TypeDescription * pTmp = (typelib_TypeDescription *)*ppRet;

    sal_Int32 nOffset = rtl_ustr_lastIndexOfChar_WithLength(
        pTypeName->buffer, pTypeName->length, ':' );

    if( nOffset == -1 )
    {
        // not found
        rtl_uString_acquire( (*ppRet)->aBase.pMemberName = pTypeName );
    }
    else
    {
        rtl_uString_newFromStr_WithLength( &(*ppRet)->aBase.pMemberName,
                                           pTypeName->buffer + nOffset +1,
                                           pTypeName->length - nOffset -1 );
    }

    (*ppRet)->aBase.nPosition = nAbsolutePosition;
    typelib_typedescriptionreference_new( &(*ppRet)->pAttributeTypeRef, eAttributeTypeClass, pAttributeTypeName );
    (*ppRet)->bReadOnly = bReadOnly;

    if( !reallyWeak( typelib_TypeClass_INTERFACE_ATTRIBUTE ) )
        pTmp->pWeakRef = (typelib_TypeDescriptionReference *)pTmp;
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_acquire(
    typelib_TypeDescription * pTypeDescription )
{
    osl_incrementInterlockedCount( &pTypeDescription->nRefCount );
}

//------------------------------------------------------------------------
// frees anything except typelib_TypeDescription base!
static inline void typelib_typedescription_destructExtendedMembers(
    typelib_TypeDescription * pTD )
{
    OSL_ASSERT( typelib_TypeClass_TYPEDEF != pTD->eTypeClass );

    switch( pTD->eTypeClass )
    {
    case typelib_TypeClass_ARRAY:
    case typelib_TypeClass_SEQUENCE:
        if( ((typelib_IndirectTypeDescription*)pTD)->pType )
            typelib_typedescriptionreference_release( ((typelib_IndirectTypeDescription*)pTD)->pType );
        break;
    case typelib_TypeClass_UNION:
    {
        typelib_typedescriptionreference_release(
            ((typelib_UnionTypeDescription *)pTD)->pDiscriminantTypeRef );
        typelib_typedescriptionreference_release(
            ((typelib_UnionTypeDescription *)pTD)->pDefaultTypeRef );
        sal_Int32 nPos;
        for ( nPos = ((typelib_UnionTypeDescription *)pTD)->nMembers; nPos--; )
        {
            typelib_typedescriptionreference_release(
                ((typelib_UnionTypeDescription *)pTD)->ppTypeRefs[nPos] );
        }

        for ( nPos = ((typelib_UnionTypeDescription *)pTD)->nMembers; nPos--; )
        {
            rtl_uString_release(
                ((typelib_UnionTypeDescription *)pTD)->ppMemberNames[nPos] );
        }
        delete [] ((typelib_UnionTypeDescription *)pTD)->ppMemberNames;
        delete [] ((typelib_UnionTypeDescription *)pTD)->pDiscriminants;
        delete [] ((typelib_UnionTypeDescription *)pTD)->ppTypeRefs;
    }
    break;
    case typelib_TypeClass_STRUCT:
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
        // The members in this array are not allocated
        sal_Int32 nSuperMembers = pITD->pBaseTypeDescription ? pITD->pBaseTypeDescription->nAllMembers : 0;
        // release only the descriptions of this class and not the one of the superclass
        for( sal_Int32 i = nSuperMembers; i < pITD->nAllMembers; i++ )
        {
            typelib_typedescriptionreference_release( pITD->ppAllMembers[i] );
        }
        delete [] pITD->ppAllMembers;
        delete [] pITD->pMapMemberIndexToFunctionIndex;
        delete [] pITD->pMapFunctionIndexToMemberIndex;
        if (pITD->pBaseTypeDescription)
            typelib_typedescription_release( (typelib_TypeDescription *)pITD->pBaseTypeDescription );
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        typelib_InterfaceMethodTypeDescription * pIMTD = (typelib_InterfaceMethodTypeDescription*)pTD;
        if( pIMTD->pReturnTypeRef )
            typelib_typedescriptionreference_release( pIMTD->pReturnTypeRef );
        sal_Int32 i;
        for( i = 0; i < pIMTD->nParams; i++ )
        {
            rtl_uString_release( pIMTD->pParams[ i ].pName );
            typelib_typedescriptionreference_release( pIMTD->pParams[ i ].pTypeRef );
        }
        delete [] pIMTD->pParams;
        for( i = 0; i < pIMTD->nExceptions; i++ )
        {
            typelib_typedescriptionreference_release( pIMTD->ppExceptions[ i ] );
        }
        delete [] pIMTD->ppExceptions;
        rtl_uString_release( pIMTD->aBase.pMemberName );
    }
    break;
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        typelib_InterfaceAttributeTypeDescription * pIATD = (typelib_InterfaceAttributeTypeDescription*)pTD;
        if( pIATD->pAttributeTypeRef )
            typelib_typedescriptionreference_release( pIATD->pAttributeTypeRef );
        if( pIATD->aBase.pMemberName )
            rtl_uString_release( pIATD->aBase.pMemberName );
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
    }
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_release(
    typelib_TypeDescription * pTD )
{
    OSL_ASSERT(pTD->nRefCount > 0);

    if( !osl_decrementInterlockedCount( &pTD->nRefCount ) )
    {
        if( reallyWeak( pTD->eTypeClass ) )
        {
            if( pTD->pWeakRef )
            {
                {
                MutexGuard aGuard( aInit.getMutex() );
                // remove this description from the weak reference
                pTD->pWeakRef->pType = 0;
                }
                typelib_typedescriptionreference_release( pTD->pWeakRef );
            }
        }
        else
        {
            // this description is a reference too, so remove it from the hash table
            if( aInit.pWeakMap )
            {
                MutexGuard aGuard( aInit.getMutex() );
                WeakMap_Impl::iterator aIt = aInit.pWeakMap->find( (sal_Unicode*)pTD->pTypeName->buffer );
                if( aIt != aInit.pWeakMap->end() && (void *)(*aIt).second == (void *)pTD )
                {
                    // remove only if it contains the same object
                    aInit.pWeakMap->erase( aIt );
                }
            }
        }

        typelib_typedescription_destructExtendedMembers( pTD );
        rtl_uString_release( pTD->pTypeName );

#ifdef CPPU_ASSERTIONS
        switch( pTD->eTypeClass )
        {
        case typelib_TypeClass_ARRAY:
        case typelib_TypeClass_SEQUENCE:
            osl_decrementInterlockedCount( &aInit.nIndirectTypeDescriptionCount );
            break;
        case typelib_TypeClass_UNION:
            osl_decrementInterlockedCount( &aInit.nUnionTypeDescriptionCount );
            break;
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
            osl_decrementInterlockedCount( &aInit.nCompoundTypeDescriptionCount );
            break;
        case typelib_TypeClass_INTERFACE:
            osl_decrementInterlockedCount( &aInit.nInterfaceTypeDescriptionCount );
            break;
        case typelib_TypeClass_INTERFACE_METHOD:
            osl_decrementInterlockedCount( &aInit.nInterfaceMethodTypeDescriptionCount );
            break;
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            osl_decrementInterlockedCount( &aInit.nInterfaceAttributeTypeDescriptionCount );
            break;
        case typelib_TypeClass_ENUM:
            osl_decrementInterlockedCount( &aInit.nEnumTypeDescriptionCount );
            break;
        default:
            osl_decrementInterlockedCount( &aInit.nTypeDescriptionCount );
        }
#endif

        delete pTD;
    }
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_register(
    typelib_TypeDescription ** ppNewDescription )
{
    // connect the description with the weak reference
    ClearableMutexGuard aGuard( aInit.getMutex() );

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
                    if (osl_incrementInterlockedCount( &pTDR->pType->nRefCount ) > 1)
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
                        osl_decrementInterlockedCount( &pTDR->pType->nRefCount );
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
                ::rtl_copyMemory(
                    pTDR->pType +1,
                    *ppNewDescription +1,
                    nSize - sizeof(typelib_TypeDescription) );

                pTDR->pType->bComplete = (*ppNewDescription)->bComplete;
                pTDR->pType->nSize = (*ppNewDescription)->nSize;
                pTDR->pType->nAlignment = (*ppNewDescription)->nAlignment;

                ::rtl_zeroMemory(
                    *ppNewDescription +1, nSize - sizeof( typelib_TypeDescription ) );

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
        if( !aInit.pWeakMap )
            aInit.pWeakMap = new WeakMap_Impl;

        // description is the weak itself, so register it
        (*aInit.pWeakMap)[pTDR->pTypeName->buffer] = pTDR;
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
inline static sal_Bool type_equals(
    typelib_TypeDescriptionReference * p1, typelib_TypeDescriptionReference * p2 )
{
    return (p1 == p2 ||
            (p1->eTypeClass == p2->eTypeClass &&
             p1->pTypeName->length == p2->pTypeName->length &&
             rtl_ustr_compare( p1->pTypeName->buffer, p2->pTypeName->buffer ) == 0));
}
extern "C" SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescription_equals(
    const typelib_TypeDescription * p1, const typelib_TypeDescription * p2 )
{
    return type_equals(
        (typelib_TypeDescriptionReference *)p1, (typelib_TypeDescriptionReference *)p2 );
}

//------------------------------------------------------------------------
sal_Int32 SAL_CALL typelib_typedescription_getAlignedUnoSize(
    const typelib_TypeDescription * pTypeDescription,
    sal_Int32 nOffset,
    sal_Int32 & rMaxIntegralTypeSize )
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
                nSize = rMaxIntegralTypeSize = sizeof( void * );
                break;
            case typelib_TypeClass_UNION:
            {
                nSize = rMaxIntegralTypeSize = sizeof(sal_Int64);
                for ( sal_Int32 nPos = ((typelib_UnionTypeDescription *)pTypeDescription)->nMembers; nPos--; )
                {
                    typelib_TypeDescription * pTD = 0;
                    TYPELIB_DANGER_GET( &pTD, ((typelib_UnionTypeDescription *)pTypeDescription)->ppTypeRefs[nPos] );
                    sal_Int32 nMaxIntegralTypeSize;
                    sal_Int32 nMemberSize = typelib_typedescription_getAlignedUnoSize( pTD, sizeof(sal_Int64), nMaxIntegralTypeSize );
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
                nSize = rMaxIntegralTypeSize = sizeof( typelib_TypeClass );
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
                    if (pMemberRef->eTypeClass == typelib_TypeClass_INTERFACE ||
                        pMemberRef->eTypeClass == typelib_TypeClass_SEQUENCE)
                    {
                        nMaxIntegral = sizeof(void *);
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
                // Example: A { double; int; } structure has a size of 16 instead of 10. The
                // compiler must follow this rule if it is possible to access memvers in arrays through:
                // (Element *)((char *)pArray + sizeof( Element ) * ElementPos)
                nStructSize = (nStructSize + rMaxIntegralTypeSize -1)
                                / rMaxIntegralTypeSize * rMaxIntegralTypeSize;
                nSize += nStructSize;
                }
                break;
            case typelib_TypeClass_ARRAY:
                OSL_ENSHURE( sal_False, "not implemented" );
                break;
            case typelib_TypeClass_SEQUENCE:
                nSize = rMaxIntegralTypeSize = sizeof( void * );
                break;
            case typelib_TypeClass_ANY:
                // FEATURE_ANY
                nSize = sizeof( uno_Any );
                rMaxIntegralTypeSize = sizeof( void * );
                break;
            case typelib_TypeClass_TYPE:
                nSize = rMaxIntegralTypeSize = sizeof( typelib_TypeDescriptionReference * );
                break;
            case typelib_TypeClass_BOOLEAN:
                nSize = rMaxIntegralTypeSize = sizeof( sal_Bool );
                break;
            case typelib_TypeClass_CHAR:
                nSize = rMaxIntegralTypeSize = sizeof( sal_Unicode );
                break;
            case typelib_TypeClass_STRING:
                // FEATURE_STRING
                nSize = rMaxIntegralTypeSize = sizeof( rtl_uString * );
                break;
            case typelib_TypeClass_FLOAT:
                nSize = rMaxIntegralTypeSize = sizeof( float );
                break;
            case typelib_TypeClass_DOUBLE:
                nSize = rMaxIntegralTypeSize = sizeof( double );
                break;

            case typelib_TypeClass_BYTE:
                nSize = rMaxIntegralTypeSize = sizeof( sal_Int8 );
                break;

            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                nSize = rMaxIntegralTypeSize = sizeof( sal_Int16 );
                break;

            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
                nSize = rMaxIntegralTypeSize = sizeof( sal_Int32 );
                break;

            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                nSize = rMaxIntegralTypeSize = sizeof( sal_Int64 );
                break;

            case typelib_TypeClass_UNKNOWN:
            case typelib_TypeClass_SERVICE:
            case typelib_TypeClass_MODULE:
            default:
                OSL_ENSHURE( sal_False, "not convertable type" );
        };
    }

    return newAlignedSize( nOffset, nSize, rMaxIntegralTypeSize );
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescription_getByName(
    typelib_TypeDescription ** ppRet, rtl_uString * pName )
{
    if( *ppRet )
    {
        typelib_typedescription_release( (*ppRet) );
        *ppRet = 0;
    }

    static sal_Bool bInited = sal_False;

    if( !bInited )
    {
        // guard against multi thread access
        MutexGuard aGuard( aInit.getMutex() );
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
        MutexGuard aGuard( aInit.getMutex() );
        // pTDR->pType->pWeakRef == 0 means that the description is empty
        if( pTDR->pType && pTDR->pType->pWeakRef )
        {
            typelib_typedescription_acquire( pTDR->pType );
            *ppRet = pTDR->pType;
        }
        }
        typelib_typedescriptionreference_release( pTDR );
    }

    if( !*ppRet )
    {
        // on demand access
        aInit.callChain( ppRet, pName );
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
                MutexGuard aGuard( aInit.getMutex() );
                if( !aInit.pCache )
                    aInit.pCache = new TypeDescriptionList_Impl;
                if( aInit.pCache->size() >= nCacheSize )
                {
                    typelib_typedescription_release( aInit.pCache->front() );
                    aInit.pCache->pop_front();
                }
                // descriptions in the cache must be acquired!
                typelib_typedescription_acquire( *ppRet );
                aInit.pCache->push_back( *ppRet );
            }
        }
    }
}


//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_newByAsciiName(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    const sal_Char * pTypeName )
{
    OUString aTypeName( OUString::createFromAscii( pTypeName ) );
    typelib_typedescriptionreference_new( ppTDR, eTypeClass, aTypeName.pData );
}
//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_new(
    typelib_TypeDescriptionReference ** ppTDR,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName )
{
    if( eTypeClass == typelib_TypeClass_TYPEDEF )
    {
        // on demand access
        typelib_TypeDescription * pRet = 0;
        aInit.callChain( &pRet, pTypeName );
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
                MutexGuard aGuard( aInit.getMutex() );
                if( !aInit.pCache )
                    aInit.pCache = new TypeDescriptionList_Impl;
                if( aInit.pCache->size() >= nCacheSize )
                {
                    typelib_typedescription_release( aInit.pCache->front() );
                    aInit.pCache->pop_front();
                }
                aInit.pCache->push_back( pRet );
                // pRet kept acquired for cache

                typelib_typedescriptionreference_acquire( pRet->pWeakRef );
                if (*ppTDR)
                    typelib_typedescriptionreference_release( *ppTDR );
                *ppTDR = pRet->pWeakRef;
            }
        }
        else if (*ppTDR)
        {
#ifdef CPPU_ASSERTIONS
            OString aStr( OUStringToOString( pTypeName, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( !"### typedef not found: ", aStr.getStr() );
#endif
            typelib_typedescriptionreference_release( *ppTDR );
            *ppTDR = 0;
        }
        return;
    }

    MutexGuard aGuard( aInit.getMutex() );
    typelib_typedescriptionreference_getByName( ppTDR, pTypeName );
    if( *ppTDR )
        return;

    if( reallyWeak( eTypeClass ) )
    {
        typelib_TypeDescriptionReference * pTDR = new typelib_TypeDescriptionReference();
#ifdef CPPU_ASSERTIONS
        osl_incrementInterlockedCount( &aInit.nTypeDescriptionReferenceCount );
#endif
        pTDR->nRefCount = 1;
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

    if( !aInit.pWeakMap )
        aInit.pWeakMap = new WeakMap_Impl;
    // Heavy hack, the const sal_Unicode * is hold by the typedescription reference
    // not registered
    aInit.pWeakMap->operator[]( (*ppTDR)->pTypeName->buffer ) = *ppTDR;
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_acquire(
    typelib_TypeDescriptionReference * pRef )
{
    osl_incrementInterlockedCount( &pRef->nRefCount );
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_release(
    typelib_TypeDescriptionReference * pRef )
{
    // Is it a type description?
    if( reallyWeak( pRef->eTypeClass ) )
    {
        if( !osl_decrementInterlockedCount( &pRef->nRefCount ) )
        {
            if( aInit.pWeakMap )
            {
                MutexGuard aGuard( aInit.getMutex() );
                WeakMap_Impl::iterator aIt = aInit.pWeakMap->find( (sal_Unicode*)pRef->pTypeName->buffer );
                if( !(aIt == aInit.pWeakMap->end()) && (*aIt).second == pRef )
                {
                    // remove only if it contains the same object
                    aInit.pWeakMap->erase( aIt );
                }
            }

            rtl_uString_release( pRef->pTypeName );
            OSL_ASSERT( pRef->pType == 0 );
#ifdef CPPU_ASSERTIONS
            osl_decrementInterlockedCount( &aInit.nTypeDescriptionReferenceCount );
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
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_getDescription(
    typelib_TypeDescription ** ppRet,
    typelib_TypeDescriptionReference * pRef )
{
    if( *ppRet )
    {
        typelib_typedescription_release( *ppRet );
        *ppRet = 0;
    }

    if( !reallyWeak( pRef->eTypeClass ) && pRef->pType && pRef->pType->pWeakRef )
    {
        // reference is a description and initialized
        osl_incrementInterlockedCount( &((typelib_TypeDescription *)pRef)->nRefCount );
        *ppRet = (typelib_TypeDescription *)pRef;
        return;
    }

    {
    MutexGuard aGuard( aInit.getMutex() );
    // pRef->pType->pWeakRef == 0 means that the description is empty
    if( pRef->pType && pRef->pType->pWeakRef )
    {
        sal_Int32 n = osl_incrementInterlockedCount( &pRef->pType->nRefCount );
        if( n > 1 )
        {
            // The refence is incremented. The object cannot be destroyed.
            // Release the guard at the earliest point.
            *ppRet = pRef->pType;
            return;
        }
        else
        {
            osl_decrementInterlockedCount( &pRef->pType->nRefCount );
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
void SAL_CALL typelib_typedescriptionreference_getByName(
    typelib_TypeDescriptionReference ** ppRet,
    rtl_uString * pName )
{
    if( *ppRet )
    {
        typelib_typedescriptionreference_release( *ppRet );
        *ppRet = 0;
    }
    if( aInit.pWeakMap )
    {
        MutexGuard aGuard( aInit.getMutex() );
        WeakMap_Impl::const_iterator aIt = aInit.pWeakMap->find( (sal_Unicode*)pName->buffer );
        if( !(aIt == aInit.pWeakMap->end()) ) // != failed on msc4.2
        {
            sal_Int32 n = osl_incrementInterlockedCount( &(*aIt).second->nRefCount );
            if( n > 1 )
                // The refence is incremented. The object cannot be destroyed.
                // Release the guard at the earliest point.
                *ppRet = (*aIt).second;
            else
                // detruction of this type in progress (another thread!)
                // no acces through this weak reference
                osl_decrementInterlockedCount( &(*aIt).second->nRefCount );
        }
    }
}

//------------------------------------------------------------------------
extern "C" SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescriptionreference_equals(
    const typelib_TypeDescriptionReference * p1,
    const typelib_TypeDescriptionReference * p2 )
{
    return (p1 == p2 ||
            (p1->eTypeClass == p2->eTypeClass &&
             p1->pTypeName->length == p2->pTypeName->length &&
             rtl_ustr_compare( p1->pTypeName->buffer, p2->pTypeName->buffer ) == 0));
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_typedescriptionreference_assign(
    typelib_TypeDescriptionReference ** ppDest,
    typelib_TypeDescriptionReference * pSource )
{
    if (*ppDest != pSource)
    {
        ::typelib_typedescriptionreference_acquire( pSource );
        ::typelib_typedescriptionreference_release( *ppDest );
        *ppDest = pSource;
    }
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL typelib_setCacheSize( sal_Int32 nNewSize )
{
    OSL_ENSHURE( nNewSize >= 0, "### illegal cache size given!" );
    if (nNewSize >= 0)
    {
        MutexGuard aGuard( aInit.getMutex() );
        if ((nNewSize < nCacheSize) && aInit.pCache)
        {
            while (aInit.pCache->size() != nNewSize)
            {
                typelib_typedescription_release( aInit.pCache->front() );
                aInit.pCache->pop_front();
            }
        }
        nCacheSize = nNewSize;
    }
}


static sal_Bool s_aAssignableFromTab[11][11] =
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
extern "C" SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescriptionreference_isAssignableFrom(
    typelib_TypeDescriptionReference * pAssignable,
    typelib_TypeDescriptionReference * pFrom )
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
                    if ( !((typelib_InterfaceTypeDescription *)pFromDescr)->pBaseTypeDescription )
                    {
                        TYPELIB_DANGER_RELEASE( pFromDescr );
                        return sal_False;
                    }
                    sal_Bool bRet = typelib_typedescriptionreference_isAssignableFrom(
                        pAssignable,
                        ((typelib_TypeDescription *)((typelib_InterfaceTypeDescription *)pFromDescr)->pBaseTypeDescription)->pWeakRef );
                    TYPELIB_DANGER_RELEASE( pFromDescr );
                    return bRet;
                }
                }
                return sal_False;
            }
        }
        return (eAssignable >= typelib_TypeClass_CHAR && eAssignable <= typelib_TypeClass_DOUBLE &&
                eFrom >= typelib_TypeClass_CHAR && eFrom <= typelib_TypeClass_DOUBLE &&
                s_aAssignableFromTab[eAssignable-1][eFrom-1]);
    }
    return sal_False;
}
//##################################################################################################
extern "C" SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescription_isAssignableFrom(
    typelib_TypeDescription * pAssignable,
    typelib_TypeDescription * pFrom )
{
    return typelib_typedescriptionreference_isAssignableFrom(
        pAssignable->pWeakRef, pFrom->pWeakRef );
}

//##################################################################################################
extern "C" SAL_DLLEXPORT sal_Bool SAL_CALL typelib_typedescription_complete(
    typelib_TypeDescription ** ppTypeDescr )
{
    if (! (*ppTypeDescr)->bComplete)
    {
        OSL_ASSERT( (typelib_TypeClass_STRUCT == (*ppTypeDescr)->eTypeClass ||
                    typelib_TypeClass_EXCEPTION == (*ppTypeDescr)->eTypeClass ||
                    typelib_TypeClass_UNION == (*ppTypeDescr)->eTypeClass ||
                    typelib_TypeClass_INTERFACE == (*ppTypeDescr)->eTypeClass) &&
                    !reallyWeak( (*ppTypeDescr)->eTypeClass ) );

        if (typelib_TypeClass_INTERFACE == (*ppTypeDescr)->eTypeClass &&
            ((typelib_InterfaceTypeDescription *)*ppTypeDescr)->ppAllMembers)
        {
            typelib_typedescription_initTables( *ppTypeDescr );
            return sal_True;
        }
        // obsolete
//          else
//          {
//              MutexGuard aGuard( aInit.getMutex() );
//              typelib_TypeDescriptionReference * pRef = 0;
//              ::typelib_typedescriptionreference_getByName( &pRef, (*ppTypeDescr)->pTypeName );
//              if (pRef)
//              {
//                  if (pRef->pType && pRef->pType->pWeakRef && pRef->pType->bComplete)
//                  {
//                      // found registered and complete td
//                      ::typelib_typedescription_release( *ppTypeDescr );
//                      *ppTypeDescr = pRef->pType;
//                      return sal_True;
//                  }
//                  ::typelib_typedescriptionreference_release( pRef );
//              }
//          }

        typelib_TypeDescription * pTD = 0;
        // on demand access of complete td
        aInit.callChain( &pTD, (*ppTypeDescr)->pTypeName );
        if (pTD)
        {
            if (typelib_TypeClass_TYPEDEF == pTD->eTypeClass)
            {
                typelib_typedescriptionreference_getDescription(
                    &pTD, ((typelib_IndirectTypeDescription *)pTD)->pType );
                OSL_ASSERT( pTD );
                if (! pTD)
                    return sal_False;
            }

            OSL_ASSERT( typelib_TypeClass_TYPEDEF != pTD->eTypeClass );
            // typedescription found
            // set to on demand
            pTD->bOnDemand = sal_True;

            if (typelib_TypeClass_INTERFACE == pTD->eTypeClass && !pTD->bComplete)
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
            MutexGuard aGuard( aInit.getMutex() );
            if( !aInit.pCache )
                aInit.pCache = new TypeDescriptionList_Impl;
            if( aInit.pCache->size() >= nCacheSize )
            {
                typelib_typedescription_release( aInit.pCache->front() );
                aInit.pCache->pop_front();
            }
            // descriptions in the cache must be acquired!
            typelib_typedescription_acquire( pTD );
            aInit.pCache->push_back( pTD );

            OSL_ASSERT( pTD->bComplete );

            ::typelib_typedescription_release( *ppTypeDescr );
            *ppTypeDescr = pTD;
        }
        else
        {
#ifdef CPPU_ASSERTIONS
            OString aStr( OUStringToOString( (*ppTypeDescr)->pTypeName, RTL_TEXTENCODING_ASCII_US ) );
            CPPU_TRACE( "\n### type cannot be completed: " );
            CPPU_TRACE( aStr.getStr() );
#endif
            return sal_False;
        }
    }
    return sal_True;
}


