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

#include "sal/config.h"

#include <algorithm>
#include <cassert>
#include <stdarg.h>

#include <osl/mutex.hxx>
#include <osl/interlck.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>

#include <typelib/typedescription.h>


using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

extern "C"
{

//------------------------------------------------------------------------
sal_Int32 SAL_CALL typelib_typedescription_getAlignedUnoSize(
    const typelib_TypeDescription * pTypeDescription,
    sal_Int32 nOffset,
    sal_Int32 & rMaxIntegralTypeSize )
    SAL_THROW_EXTERN_C();
//------------------------------------------------------------------------
void SAL_CALL typelib_typedescription_newEmpty(
    typelib_TypeDescription ** ppRet,
    typelib_TypeClass eTypeClass,
    rtl_uString * pTypeName )
    SAL_THROW_EXTERN_C();
//-----------------------------------------------------------------------------
void SAL_CALL typelib_typedescriptionreference_getByName(
    typelib_TypeDescriptionReference ** ppRet,
    rtl_uString * pName )
    SAL_THROW_EXTERN_C();

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
    sal_Int64   dDouble;
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

//--------------------------------------------------------------------------------------------------

namespace
{
    struct typelib_StaticInitMutex : public rtl::Static< Mutex, typelib_StaticInitMutex > {};
}

// !for NOT REALLY WEAK TYPES only!
static inline typelib_TypeDescriptionReference * igetTypeByName( rtl_uString * pTypeName )
    SAL_THROW(())
{
    typelib_TypeDescriptionReference * pRef = 0;
    ::typelib_typedescriptionreference_getByName( &pRef, pTypeName );
    if (pRef && pRef->pType && pRef->pType->pWeakRef) // found initialized td
    {
        return pRef;
    }
    else
    {
        return 0;
    }
}

extern "C"
{
//##################################################################################################
CPPU_DLLPUBLIC typelib_TypeDescriptionReference ** SAL_CALL typelib_static_type_getByTypeClass(
    typelib_TypeClass eTypeClass )
    SAL_THROW_EXTERN_C()
{
    static typelib_TypeDescriptionReference * s_aTypes[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0 };

    if (! s_aTypes[eTypeClass])
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! s_aTypes[eTypeClass])
        {
            static const char * s_aTypeNames[] = {
                "void", "char", "boolean", "byte",
                "short", "unsigned short", "long", "unsigned long",
                "hyper", "unsigned hyper", "float", "double",
                "string", "type", "any" };

            switch (eTypeClass)
            {
            case typelib_TypeClass_EXCEPTION:
            case typelib_TypeClass_INTERFACE:
            {
                // type
                if (! s_aTypes[typelib_TypeClass_TYPE])
                {
                    OUString sTypeName("type");
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_TYPE], typelib_TypeClass_TYPE, sTypeName.pData );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_TYPE]->nStaticRefCount;
                }
                // any
                if (! s_aTypes[typelib_TypeClass_ANY])
                {
                    OUString sTypeName("any");
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_ANY], typelib_TypeClass_ANY, sTypeName.pData );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_ANY]->nStaticRefCount;
                }
                // string
                if (! s_aTypes[typelib_TypeClass_STRING])
                {
                    OUString sTypeName("string");
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_STRING], typelib_TypeClass_STRING, sTypeName.pData );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_STRING]->nStaticRefCount;
                }
                // XInterface
                if (! s_aTypes[typelib_TypeClass_INTERFACE])
                {
                    OUString sTypeName("com.sun.star.uno.XInterface");

                    typelib_InterfaceTypeDescription * pTD = 0;

                    typelib_TypeDescriptionReference * pMembers[3] = { 0,0,0 };
                    OUString sMethodName0("com.sun.star.uno.XInterface::queryInterface");
                    ::typelib_typedescriptionreference_new(
                        &pMembers[0], typelib_TypeClass_INTERFACE_METHOD, sMethodName0.pData );
                    OUString sMethodName1("com.sun.star.uno.XInterface::acquire");
                    ::typelib_typedescriptionreference_new(
                        &pMembers[1], typelib_TypeClass_INTERFACE_METHOD, sMethodName1.pData );
                    OUString sMethodName2("com.sun.star.uno.XInterface::release");
                    ::typelib_typedescriptionreference_new(
                        &pMembers[2], typelib_TypeClass_INTERFACE_METHOD, sMethodName2.pData );

                    ::typelib_typedescription_newInterface(
                        &pTD, sTypeName.pData, 0, 0, 0, 0, 0, 0, 3, pMembers );

                    ::typelib_typedescription_register( (typelib_TypeDescription **)&pTD );
                    ::typelib_typedescriptionreference_acquire(
                        s_aTypes[typelib_TypeClass_INTERFACE] = ((typelib_TypeDescription *)pTD)->pWeakRef );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_INTERFACE]->nStaticRefCount;
                    ::typelib_typedescription_release( (typelib_TypeDescription*)pTD );

                    ::typelib_typedescriptionreference_release( pMembers[0] );
                    ::typelib_typedescriptionreference_release( pMembers[1] );
                    ::typelib_typedescriptionreference_release( pMembers[2] );
                    // Exception
                    assert( ! s_aTypes[typelib_TypeClass_EXCEPTION] );
                    {
                    typelib_TypeDescription * pTD1 = 0;
                    OUString sTypeName1("com.sun.star.uno.Exception");

                    typelib_CompoundMember_Init aMembers[2];
                    OUString sMemberType0("string");
                    OUString sMemberName0("Message");
                    aMembers[0].eTypeClass = typelib_TypeClass_STRING;
                    aMembers[0].pTypeName = sMemberType0.pData;
                    aMembers[0].pMemberName = sMemberName0.pData;
                    OUString sMemberType1("com.sun.star.uno.XInterface");
                    OUString sMemberName1("Context");
                    aMembers[1].eTypeClass = typelib_TypeClass_INTERFACE;
                    aMembers[1].pTypeName = sMemberType1.pData;
                    aMembers[1].pMemberName = sMemberName1.pData;

                    ::typelib_typedescription_new(
                        &pTD1, typelib_TypeClass_EXCEPTION, sTypeName1.pData, 0, 2, aMembers );
                    typelib_typedescription_register( &pTD1 );
                    typelib_typedescriptionreference_acquire(
                        s_aTypes[typelib_TypeClass_EXCEPTION] = pTD1->pWeakRef );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_EXCEPTION]->nStaticRefCount;
                    // RuntimeException
                    OUString sTypeName2("com.sun.star.uno.RuntimeException");
                    ::typelib_typedescription_new(
                        &pTD1, typelib_TypeClass_EXCEPTION, sTypeName2.pData, s_aTypes[typelib_TypeClass_EXCEPTION], 0, 0 );
                    ::typelib_typedescription_register( &pTD1 );
                    ::typelib_typedescription_release( pTD1 );
                    }
                    // XInterface members
                    typelib_InterfaceMethodTypeDescription * pMethod = 0;
                    typelib_Parameter_Init aParameters[1];
                    OUString sParamName0("aType");
                    OUString sParamType0("type");
                    aParameters[0].pParamName = sParamName0.pData;
                    aParameters[0].eTypeClass = typelib_TypeClass_TYPE;
                    aParameters[0].pTypeName = sParamType0.pData;
                    aParameters[0].bIn = sal_True;
                    aParameters[0].bOut = sal_False;
                    rtl_uString * pExceptions[1];
                    OUString sExceptionName0("com.sun.star.uno.RuntimeException");
                    pExceptions[0] = sExceptionName0.pData;
                    OUString sReturnType0("any");
                    typelib_typedescription_newInterfaceMethod(
                        &pMethod, 0, sal_False, sMethodName0.pData,
                        typelib_TypeClass_ANY, sReturnType0.pData,
                        1, aParameters, 1, pExceptions );
                    ::typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );

                    OUString sReturnType1("void");
                    ::typelib_typedescription_newInterfaceMethod(
                        &pMethod, 1, sal_True, sMethodName1.pData,
                        typelib_TypeClass_VOID, sReturnType1.pData, 0, 0, 0, 0 );
                    ::typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );

                    ::typelib_typedescription_newInterfaceMethod(
                        &pMethod, 2, sal_True, sMethodName2.pData,
                        typelib_TypeClass_VOID, sReturnType1.pData,
                        0, 0, 0, 0 );
                    ::typelib_typedescription_register( (typelib_TypeDescription**)&pMethod );
                    ::typelib_typedescription_release( (typelib_TypeDescription*)pMethod );
                }
                break;
            }
            default:
            {
                OUString aTypeName( OUString::createFromAscii( s_aTypeNames[eTypeClass] ) );
                ::typelib_typedescriptionreference_new( &s_aTypes[eTypeClass], eTypeClass, aTypeName.pData );
                // another static ref:
                ++s_aTypes[eTypeClass]->nStaticRefCount;
            }
            }
        }
    }
    return &s_aTypes[eTypeClass];
}

//##################################################################################################
CPPU_DLLPUBLIC void SAL_CALL typelib_static_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName )
    SAL_THROW_EXTERN_C()
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! *ppRef)
        {
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            ::typelib_typedescriptionreference_new( ppRef, eTypeClass, aTypeName.pData );

            // another static ref:
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

//##################################################################################################
CPPU_DLLPUBLIC void SAL_CALL typelib_static_sequence_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementType )
    SAL_THROW_EXTERN_C()
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! *ppRef)
        {
            OUStringBuffer aBuf( 32 );
            aBuf.appendAscii( "[]" );
            aBuf.append( pElementType->pTypeName );
            OUString aTypeName( aBuf.makeStringAndClear() );

            assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_SEQUENCE) );
            *ppRef = igetTypeByName( aTypeName.pData );
            if (!*ppRef)
            {
                typelib_TypeDescription * pReg = 0;
                ::typelib_typedescription_new(
                    &pReg, typelib_TypeClass_SEQUENCE,
                    aTypeName.pData, pElementType, 0, 0 );

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                assert( *ppRef == pReg->pWeakRef );
            }
            // another static ref:
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

//##################################################################################################
namespace {

void init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers,
    sal_Bool const * pParameterizedTypes)
{
    assert( eTypeClass == typelib_TypeClass_STRUCT || eTypeClass == typelib_TypeClass_EXCEPTION );

    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! *ppRef)
        {
            assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(eTypeClass) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            *ppRef = igetTypeByName( aTypeName.pData );
            if (!*ppRef)
            {
                typelib_CompoundTypeDescription * pComp = 0;
                ::typelib_typedescription_newEmpty(
                    (typelib_TypeDescription **)&pComp, eTypeClass, aTypeName.pData );

                sal_Int32 nOffset = 0;
                if (pBaseType)
                {
                    ::typelib_typedescriptionreference_getDescription(
                        (typelib_TypeDescription **)&pComp->pBaseTypeDescription, pBaseType );
                    assert( pComp->pBaseTypeDescription );
                    nOffset = ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nSize;
                    assert( newAlignedSize( 0, ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nSize, ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nAlignment ) == ((typelib_TypeDescription *)pComp->pBaseTypeDescription)->nSize ); // unexpected offset
                }

                if (nMembers)
                {
                    pComp->nMembers = nMembers;
                    pComp->pMemberOffsets = new sal_Int32[ nMembers ];
                    pComp->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
                    if (pParameterizedTypes != 0) {
                        reinterpret_cast< typelib_StructTypeDescription * >(
                            pComp)->pParameterizedTypes
                            = new sal_Bool[nMembers];
                    }
                    for ( sal_Int32 i = 0 ; i < nMembers; ++i )
                    {
                        ::typelib_typedescriptionreference_acquire(
                            pComp->ppTypeRefs[i] = ppMembers[i] );
                        // write offset
                        typelib_TypeDescription * pTD = 0;
                        TYPELIB_DANGER_GET( &pTD, pComp->ppTypeRefs[i] );
                        assert( pTD->nSize ); // void member?
                        nOffset = newAlignedSize( nOffset, pTD->nSize, pTD->nAlignment );
                        pComp->pMemberOffsets[i] = nOffset - pTD->nSize;
                        TYPELIB_DANGER_RELEASE( pTD );

                        if (pParameterizedTypes != 0) {
                            reinterpret_cast< typelib_StructTypeDescription * >(
                                pComp)->pParameterizedTypes[i]
                                = pParameterizedTypes[i];
                        }
                    }
                }

                typelib_TypeDescription * pReg = (typelib_TypeDescription *)pComp;
                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
                pReg->nAlignment = adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                assert( *ppRef == pReg->pWeakRef );
            }
            // another static ref:
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

}

CPPU_DLLPUBLIC void SAL_CALL typelib_static_compound_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers )
    SAL_THROW_EXTERN_C()
{
    init(ppRef, eTypeClass, pTypeName, pBaseType, nMembers, ppMembers, 0);
}

CPPU_DLLPUBLIC void SAL_CALL typelib_static_struct_type_init(
    typelib_TypeDescriptionReference ** ppRef, const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers,
    sal_Bool const * pParameterizedTypes )
    SAL_THROW_EXTERN_C()
{
    init(
        ppRef, typelib_TypeClass_STRUCT, pTypeName, pBaseType, nMembers,
        ppMembers, pParameterizedTypes);
}

//##################################################################################################
CPPU_DLLPUBLIC void SAL_CALL typelib_static_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType )
    SAL_THROW_EXTERN_C()
{
    typelib_static_mi_interface_type_init(
        ppRef, pTypeName, pBaseType == 0 ? 0 : 1, &pBaseType);
}

//##################################################################################################
CPPU_DLLPUBLIC void SAL_CALL typelib_static_mi_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    sal_Int32 nBaseTypes,
    typelib_TypeDescriptionReference ** ppBaseTypes )
    SAL_THROW_EXTERN_C()
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! *ppRef)
        {
            assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_INTERFACE) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            *ppRef = igetTypeByName( aTypeName.pData );
            if (!*ppRef)
            {
                typelib_InterfaceTypeDescription * pIface = 0;
                ::typelib_typedescription_newEmpty(
                    (typelib_TypeDescription **)&pIface, typelib_TypeClass_INTERFACE, aTypeName.pData );

                pIface->nBaseTypes = std::max< sal_Int32 >(nBaseTypes, 1);
                pIface->ppBaseTypes = new typelib_InterfaceTypeDescription *[
                    pIface->nBaseTypes];
                if (nBaseTypes > 0)
                {
                    for (sal_Int32 i = 0; i < nBaseTypes; ++i) {
                        pIface->ppBaseTypes[i] = 0;
                        ::typelib_typedescriptionreference_getDescription(
                            (typelib_TypeDescription **)&pIface->ppBaseTypes[i], ppBaseTypes[i] );
                        assert( pIface->ppBaseTypes[i] );
                    }
                }
                else
                {
                    pIface->ppBaseTypes[0] = 0;
                    ::typelib_typedescriptionreference_getDescription(
                        (typelib_TypeDescription **)&pIface->ppBaseTypes[0],
                        * ::typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE ) );
                    assert( pIface->ppBaseTypes[0] );
                }
                pIface->pBaseTypeDescription = pIface->ppBaseTypes[0];
                typelib_typedescription_acquire(
                    &pIface->pBaseTypeDescription->aBase);

                typelib_TypeDescription * pReg = (typelib_TypeDescription *)pIface;
                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );

                pReg->nAlignment = adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                assert( *ppRef == pReg->pWeakRef );
            }
            // another static ref:
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

//##################################################################################################
CPPU_DLLPUBLIC void SAL_CALL typelib_static_enum_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const sal_Char * pTypeName,
    sal_Int32 nDefaultValue )
    SAL_THROW_EXTERN_C()
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! *ppRef)
        {
            assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_ENUM) );
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            *ppRef = igetTypeByName( aTypeName.pData );
            if (!*ppRef)
            {
                typelib_TypeDescription * pReg = 0;
                ::typelib_typedescription_newEmpty(
                    &pReg, typelib_TypeClass_ENUM, aTypeName.pData );
                typelib_EnumTypeDescription * pEnum = (typelib_EnumTypeDescription *)pReg;

                pEnum->nDefaultEnumValue = nDefaultValue;

                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
                pReg->nAlignment = ::adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_False;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                assert( *ppRef == pReg->pWeakRef );
            }
            // another static ref:
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

//##################################################################################################
CPPU_DLLPUBLIC void SAL_CALL typelib_static_array_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementTypeRef,
    sal_Int32 nDimensions, ... )
    SAL_THROW_EXTERN_C()
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex::get() );
        if (! *ppRef)
        {
            OUStringBuffer aBuf( 32 );
            aBuf.append( pElementTypeRef->pTypeName );

            va_list dimArgs;
            va_start( dimArgs, nDimensions );
            sal_Int32 dim = 0;
            sal_Int32 nElements = 1;
            sal_Int32* pDimensions = new sal_Int32[nDimensions];
            for (sal_Int32 i=0; i < nDimensions; i++)
            {
                dim = va_arg( dimArgs, int);
                pDimensions[i] = dim;
                aBuf.appendAscii("[");
                aBuf.append(dim);
                aBuf.appendAscii("]");
                nElements *= dim;
            }
            va_end( dimArgs );
            OUString aTypeName( aBuf.makeStringAndClear() );

            assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_ARRAY) );
            *ppRef = igetTypeByName( aTypeName.pData );
            if (!*ppRef)
            {
                typelib_TypeDescription * pReg = 0;
                ::typelib_typedescription_newEmpty(
                    &pReg, typelib_TypeClass_ARRAY, aTypeName.pData );
                typelib_ArrayTypeDescription * pArray = (typelib_ArrayTypeDescription *)pReg;

                pArray->nDimensions = nDimensions;
                pArray->nTotalElements = nElements;
                pArray->pDimensions = pDimensions;

                typelib_typedescriptionreference_acquire(pElementTypeRef);
                ((typelib_IndirectTypeDescription*)pArray)->pType = pElementTypeRef;

                pReg->pWeakRef = (typelib_TypeDescriptionReference *)pReg;
                // sizeof( void ) not allowed
                pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
                pReg->nAlignment = ::adjustAlignment( pReg->nAlignment );
                pReg->bComplete = sal_True;

                ::typelib_typedescription_register( &pReg );
                *ppRef = (typelib_TypeDescriptionReference *)pReg;
                assert( *ppRef == pReg->pWeakRef );
            } else
                delete [] pDimensions;
            // another static ref:
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

} // extern "C"

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
