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

#include <sal/config.h>

#include <algorithm>
#include <cassert>

#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>

#include <typelib/typedescription.h>
#include "typelib.hxx"


using namespace osl;

namespace
{
    Mutex& typelib_StaticInitMutex()
    {
        static Mutex SINGLETON;
        return SINGLETON;
    }
}

extern "C"
{


#ifdef _WIN32
#pragma pack(push, 8)
#endif

namespace {

/**
 * The double member determines the alignment.
 * Under OS2 and MS-Windows the Alignment is min( 8, sizeof( type ) ).
 * The alignment of a structure is min( 8, sizeof( max basic type ) ), the greatest basic type
 * determines the alignment.
 */
struct AlignSize_Impl
{
    sal_Int16 nInt16;
    double dDouble;
};

}

#ifdef _WIN32
#pragma pack(pop)
#endif

// the value of the maximal alignment
const sal_Int32 nMaxAlignment = static_cast<sal_Int32>( reinterpret_cast<sal_Size>(&reinterpret_cast<AlignSize_Impl *>(16)->dDouble) - 16);

static sal_Int32 adjustAlignment( sal_Int32 nRequestedAlignment )
{
    if( nRequestedAlignment > nMaxAlignment )
        nRequestedAlignment = nMaxAlignment;
    return nRequestedAlignment;
}

/**
 * Calculate the new size of the struktur.
 */
static sal_Int32 newAlignedSize(
    sal_Int32 OldSize, sal_Int32 ElementSize, sal_Int32 NeededAlignment )
{
    NeededAlignment = adjustAlignment( NeededAlignment );
    return (OldSize + NeededAlignment -1) / NeededAlignment * NeededAlignment + ElementSize;
}


// !for NOT REALLY WEAK TYPES only!
static typelib_TypeDescriptionReference * igetTypeByName( rtl_uString const * pTypeName )
{
    typelib_TypeDescriptionReference * pRef = nullptr;
    ::typelib_typedescriptionreference_getByName( &pRef, pTypeName );
    if (pRef && pRef->pType && pRef->pType->pWeakRef) // found initialized td
    {
        return pRef;
    }
    return nullptr;
}

extern "C"
{

typelib_TypeDescriptionReference ** SAL_CALL typelib_static_type_getByTypeClass(
    typelib_TypeClass eTypeClass ) noexcept
{
    static typelib_TypeDescriptionReference * s_aTypes[] = {
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
        nullptr, nullptr, nullptr };

    if (! s_aTypes[eTypeClass])
    {
        MutexGuard aGuard( typelib_StaticInitMutex() );
        if (! s_aTypes[eTypeClass])
        {
            static constexpr OUString s_aTypeNames[] = {
                u"void"_ustr, u"char"_ustr, u"boolean"_ustr, u"byte"_ustr,
                u"short"_ustr, u"unsigned short"_ustr, u"long"_ustr, u"unsigned long"_ustr,
                u"hyper"_ustr, u"unsigned hyper"_ustr, u"float"_ustr, u"double"_ustr,
                u"string"_ustr, u"type"_ustr, u"any"_ustr };

            switch (eTypeClass)
            {
            case typelib_TypeClass_EXCEPTION:
            case typelib_TypeClass_INTERFACE:
            {
                // type
                if (! s_aTypes[typelib_TypeClass_TYPE])
                {
                    OUString sTypeName(u"type"_ustr);
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_TYPE], typelib_TypeClass_TYPE, sTypeName.pData );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_TYPE]->nStaticRefCount;
                }
                // any
                if (! s_aTypes[typelib_TypeClass_ANY])
                {
                    OUString sTypeName(u"any"_ustr);
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_ANY], typelib_TypeClass_ANY, sTypeName.pData );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_ANY]->nStaticRefCount;
                }
                // string
                if (! s_aTypes[typelib_TypeClass_STRING])
                {
                    OUString sTypeName(u"string"_ustr);
                    ::typelib_typedescriptionreference_new(
                        &s_aTypes[typelib_TypeClass_STRING], typelib_TypeClass_STRING, sTypeName.pData );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_STRING]->nStaticRefCount;
                }
                // XInterface
                if (! s_aTypes[typelib_TypeClass_INTERFACE])
                {
                    OUString sTypeName(u"com.sun.star.uno.XInterface"_ustr);

                    typelib_InterfaceTypeDescription * pTD = nullptr;

                    typelib_TypeDescriptionReference * pMembers[3] = { nullptr,nullptr,nullptr };
                    OUString sMethodName0(u"com.sun.star.uno.XInterface::queryInterface"_ustr);
                    ::typelib_typedescriptionreference_new(
                        &pMembers[0], typelib_TypeClass_INTERFACE_METHOD, sMethodName0.pData );
                    OUString sMethodName1(u"com.sun.star.uno.XInterface::acquire"_ustr);
                    ::typelib_typedescriptionreference_new(
                        &pMembers[1], typelib_TypeClass_INTERFACE_METHOD, sMethodName1.pData );
                    OUString sMethodName2(u"com.sun.star.uno.XInterface::release"_ustr);
                    ::typelib_typedescriptionreference_new(
                        &pMembers[2], typelib_TypeClass_INTERFACE_METHOD, sMethodName2.pData );

                    ::typelib_typedescription_newInterface(
                        &pTD, sTypeName.pData, 0, 0, 0, 0, 0, nullptr, 3, pMembers );

                    ::typelib_typedescription_register( reinterpret_cast<typelib_TypeDescription **>(&pTD) );
                    s_aTypes[typelib_TypeClass_INTERFACE] = pTD->aBase.pWeakRef;
                    ::typelib_typedescriptionreference_acquire(
                        s_aTypes[typelib_TypeClass_INTERFACE] );
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_INTERFACE]->nStaticRefCount;
                    ::typelib_typedescription_release( &pTD->aBase );

                    ::typelib_typedescriptionreference_release( pMembers[0] );
                    ::typelib_typedescriptionreference_release( pMembers[1] );
                    ::typelib_typedescriptionreference_release( pMembers[2] );
                    // Exception
                    assert( ! s_aTypes[typelib_TypeClass_EXCEPTION] );
                    {
                    typelib_TypeDescription * pTD1 = nullptr;
                    OUString sTypeName1(u"com.sun.star.uno.Exception"_ustr);

                    typelib_CompoundMember_Init aMembers[2];
                    OUString sMemberType0(u"string"_ustr);
                    OUString sMemberName0(u"Message"_ustr);
                    aMembers[0].eTypeClass = typelib_TypeClass_STRING;
                    aMembers[0].pTypeName = sMemberType0.pData;
                    aMembers[0].pMemberName = sMemberName0.pData;
                    OUString sMemberType1(u"com.sun.star.uno.XInterface"_ustr);
                    OUString sMemberName1(u"Context"_ustr);
                    aMembers[1].eTypeClass = typelib_TypeClass_INTERFACE;
                    aMembers[1].pTypeName = sMemberType1.pData;
                    aMembers[1].pMemberName = sMemberName1.pData;

                    ::typelib_typedescription_new(
                        &pTD1, typelib_TypeClass_EXCEPTION, sTypeName1.pData, nullptr, 2, aMembers );
                    typelib_typedescription_register( &pTD1 );
                    s_aTypes[typelib_TypeClass_EXCEPTION] = pTD1->pWeakRef;
                    typelib_typedescriptionreference_acquire(
                        s_aTypes[typelib_TypeClass_EXCEPTION]);
                    // another static ref:
                    ++s_aTypes[typelib_TypeClass_EXCEPTION]->nStaticRefCount;
                    // RuntimeException
                    OUString sTypeName2(u"com.sun.star.uno.RuntimeException"_ustr);
                    ::typelib_typedescription_new(
                        &pTD1, typelib_TypeClass_EXCEPTION, sTypeName2.pData, s_aTypes[typelib_TypeClass_EXCEPTION], 0, nullptr );
                    ::typelib_typedescription_register( &pTD1 );
                    ::typelib_typedescription_release( pTD1 );
                    }
                    // XInterface members
                    typelib_InterfaceMethodTypeDescription * pMethod = nullptr;
                    typelib_Parameter_Init aParameters[1];
                    OUString sParamName0(u"aType"_ustr);
                    OUString sParamType0(u"type"_ustr);
                    aParameters[0].pParamName = sParamName0.pData;
                    aParameters[0].eTypeClass = typelib_TypeClass_TYPE;
                    aParameters[0].pTypeName = sParamType0.pData;
                    aParameters[0].bIn = true;
                    aParameters[0].bOut = false;
                    rtl_uString * pExceptions[1];
                    OUString sExceptionName0(u"com.sun.star.uno.RuntimeException"_ustr);
                    pExceptions[0] = sExceptionName0.pData;
                    OUString sReturnType0(u"any"_ustr);
                    typelib_typedescription_newInterfaceMethod(
                        &pMethod, 0, false, sMethodName0.pData,
                        typelib_TypeClass_ANY, sReturnType0.pData,
                        1, aParameters, 1, pExceptions );
                    ::typelib_typedescription_register( reinterpret_cast<typelib_TypeDescription**>(&pMethod) );

                    OUString sReturnType1(u"void"_ustr);
                    ::typelib_typedescription_newInterfaceMethod(
                        &pMethod, 1, true, sMethodName1.pData,
                        typelib_TypeClass_VOID, sReturnType1.pData, 0, nullptr, 0, nullptr );
                    ::typelib_typedescription_register( reinterpret_cast<typelib_TypeDescription**>(&pMethod) );

                    ::typelib_typedescription_newInterfaceMethod(
                        &pMethod, 2, true, sMethodName2.pData,
                        typelib_TypeClass_VOID, sReturnType1.pData,
                        0, nullptr, 0, nullptr );
                    ::typelib_typedescription_register( reinterpret_cast<typelib_TypeDescription**>(&pMethod) );
                    ::typelib_typedescription_release( &pMethod->aBase.aBase );
                }
                break;
            }
            default:
            {
                OUString aTypeName( s_aTypeNames[eTypeClass] );
                ::typelib_typedescriptionreference_new( &s_aTypes[eTypeClass], eTypeClass, aTypeName.pData );
                // another static ref:
                ++s_aTypes[eTypeClass]->nStaticRefCount;
            }
            }
        }
    }
    return &s_aTypes[eTypeClass];
}

void SAL_CALL typelib_static_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const char * pTypeName ) noexcept
{
    if (! *ppRef)
    {
        MutexGuard aGuard( typelib_StaticInitMutex() );
        if (! *ppRef)
        {
            OUString aTypeName( OUString::createFromAscii( pTypeName ) );
            ::typelib_typedescriptionreference_new( ppRef, eTypeClass, aTypeName.pData );

            assert(*ppRef && "coverity[var_deref_op] - shouldn't be possible");
            ++((*ppRef)->nStaticRefCount);
        }
    }
}

void SAL_CALL typelib_static_sequence_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeDescriptionReference * pElementType ) noexcept
{
    if ( *ppRef)
        return;

    MutexGuard aGuard( typelib_StaticInitMutex() );
    if ( *ppRef)
        return;

    OUString aTypeName = "[]" + OUString::unacquired(&pElementType->pTypeName);

    static_assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_SEQUENCE) );
    *ppRef = igetTypeByName( aTypeName.pData );
    if (!*ppRef)
    {
        typelib_TypeDescription * pReg = nullptr;
        ::typelib_typedescription_new(
            &pReg, typelib_TypeClass_SEQUENCE,
            aTypeName.pData, pElementType, 0, nullptr );

        ::typelib_typedescription_register( &pReg );
        *ppRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        assert( *ppRef == pReg->pWeakRef );
    }
    // another static ref:
    ++((*ppRef)->nStaticRefCount);
}


namespace {

void init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers,
    sal_Bool const * pParameterizedTypes)
{
    assert( eTypeClass == typelib_TypeClass_STRUCT || eTypeClass == typelib_TypeClass_EXCEPTION );

    if ( *ppRef)
        return;

    MutexGuard aGuard( typelib_StaticInitMutex() );
    if ( *ppRef)
        return;

    assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(eTypeClass) );
    OUString aTypeName( OUString::createFromAscii( pTypeName ) );
    *ppRef = igetTypeByName( aTypeName.pData );
    if (!*ppRef)
    {
        typelib_CompoundTypeDescription * pComp = nullptr;
        ::typelib_typedescription_newEmpty(
            reinterpret_cast<typelib_TypeDescription **>(&pComp), eTypeClass, aTypeName.pData );

        sal_Int32 nOffset = 0;
        if (pBaseType)
        {
            ::typelib_typedescriptionreference_getDescription(
                reinterpret_cast<typelib_TypeDescription **>(&pComp->pBaseTypeDescription), pBaseType );
            assert( pComp->pBaseTypeDescription );
            nOffset = pComp->pBaseTypeDescription->aBase.nSize;
            assert( newAlignedSize( 0, pComp->pBaseTypeDescription->aBase.nSize, pComp->pBaseTypeDescription->aBase.nAlignment ) == pComp->pBaseTypeDescription->aBase.nSize ); // unexpected offset
        }

        if (nMembers)
        {
            pComp->nMembers = nMembers;
            pComp->pMemberOffsets = new sal_Int32[ nMembers ];
            pComp->ppTypeRefs = new typelib_TypeDescriptionReference *[ nMembers ];
            if (pParameterizedTypes != nullptr) {
                reinterpret_cast< typelib_StructTypeDescription * >(
                    pComp)->pParameterizedTypes
                    = new sal_Bool[nMembers];
            }
            for ( sal_Int32 i = 0 ; i < nMembers; ++i )
            {
                pComp->ppTypeRefs[i] = ppMembers[i];
                ::typelib_typedescriptionreference_acquire(
                    pComp->ppTypeRefs[i] );
                // write offset
                typelib_TypeDescription * pTD = nullptr;
                TYPELIB_DANGER_GET( &pTD, pComp->ppTypeRefs[i] );
                assert( pTD->nSize ); // void member?
                nOffset = newAlignedSize( nOffset, pTD->nSize, pTD->nAlignment );
                pComp->pMemberOffsets[i] = nOffset - pTD->nSize;
                TYPELIB_DANGER_RELEASE( pTD );

                if (pParameterizedTypes != nullptr) {
                    reinterpret_cast< typelib_StructTypeDescription * >(
                        pComp)->pParameterizedTypes[i]
                        = pParameterizedTypes[i];
                }
            }
        }

        typelib_TypeDescription * pReg = &pComp->aBase;
        pReg->pWeakRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        // sizeof(void) not allowed
        pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
        pReg->nAlignment = adjustAlignment( pReg->nAlignment );
        pReg->bComplete = false;

        ::typelib_typedescription_register( &pReg );
        *ppRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        assert( *ppRef == pReg->pWeakRef );
    }
    // another static ref:
    ++((*ppRef)->nStaticRefCount);
}

}

void SAL_CALL typelib_static_compound_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    typelib_TypeClass eTypeClass, const char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers ) noexcept
{
    init(ppRef, eTypeClass, pTypeName, pBaseType, nMembers, ppMembers, nullptr);
}

void SAL_CALL typelib_static_struct_type_init(
    typelib_TypeDescriptionReference ** ppRef, const char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType,
    sal_Int32 nMembers, typelib_TypeDescriptionReference ** ppMembers,
    sal_Bool const * pParameterizedTypes ) noexcept
{
    init(
        ppRef, typelib_TypeClass_STRUCT, pTypeName, pBaseType, nMembers,
        ppMembers, pParameterizedTypes);
}

void SAL_CALL typelib_static_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const char * pTypeName,
    typelib_TypeDescriptionReference * pBaseType ) noexcept
{
    // coverity[callee_ptr_arith] - not a bug
    typelib_static_mi_interface_type_init(
        ppRef, pTypeName, pBaseType == nullptr ? 0 : 1, &pBaseType);
}

void SAL_CALL typelib_static_mi_interface_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const char * pTypeName,
    sal_Int32 nBaseTypes,
    typelib_TypeDescriptionReference ** ppBaseTypes ) noexcept
{
    if ( *ppRef)
        return;

    MutexGuard aGuard( typelib_StaticInitMutex() );
    if ( *ppRef)
        return;

    static_assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_INTERFACE) );
    OUString aTypeName( OUString::createFromAscii( pTypeName ) );
    *ppRef = igetTypeByName( aTypeName.pData );
    if (!*ppRef)
    {
        typelib_InterfaceTypeDescription * pIface = nullptr;
        ::typelib_typedescription_newEmpty(
            reinterpret_cast<typelib_TypeDescription **>(&pIface), typelib_TypeClass_INTERFACE, aTypeName.pData );

        pIface->nBaseTypes = std::max< sal_Int32 >(nBaseTypes, 1);
        pIface->ppBaseTypes = new typelib_InterfaceTypeDescription *[
            pIface->nBaseTypes];
        if (nBaseTypes > 0)
        {
            for (sal_Int32 i = 0; i < nBaseTypes; ++i) {
                pIface->ppBaseTypes[i] = nullptr;
                ::typelib_typedescriptionreference_getDescription(
                    reinterpret_cast<typelib_TypeDescription **>(&pIface->ppBaseTypes[i]), ppBaseTypes[i] );
                assert( pIface->ppBaseTypes[i] );
            }
        }
        else
        {
            pIface->ppBaseTypes[0] = nullptr;
            ::typelib_typedescriptionreference_getDescription(
                reinterpret_cast<typelib_TypeDescription **>(&pIface->ppBaseTypes[0]),
                * ::typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE ) );
            assert( pIface->ppBaseTypes[0] );
        }
        pIface->pBaseTypeDescription = pIface->ppBaseTypes[0];
        typelib_typedescription_acquire(
            &pIface->pBaseTypeDescription->aBase);

        typelib_TypeDescription * pReg = &pIface->aBase;
        pReg->pWeakRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        // sizeof(void) not allowed
        pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );

        pReg->nAlignment = adjustAlignment( pReg->nAlignment );
        pReg->bComplete = false;

        ::typelib_typedescription_register( &pReg );
        *ppRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        assert( *ppRef == pReg->pWeakRef );
    }
    // another static ref:
    ++((*ppRef)->nStaticRefCount);
}


void SAL_CALL typelib_static_enum_type_init(
    typelib_TypeDescriptionReference ** ppRef,
    const char * pTypeName,
    sal_Int32 nDefaultValue ) noexcept
{
    if ( *ppRef)
        return;

    MutexGuard aGuard( typelib_StaticInitMutex() );
    if ( *ppRef)
        return;

    static_assert( ! TYPELIB_TYPEDESCRIPTIONREFERENCE_ISREALLYWEAK(typelib_TypeClass_ENUM) );
    OUString aTypeName( OUString::createFromAscii( pTypeName ) );
    *ppRef = igetTypeByName( aTypeName.pData );
    if (!*ppRef)
    {
        typelib_TypeDescription * pReg = nullptr;
        ::typelib_typedescription_newEmpty(
            &pReg, typelib_TypeClass_ENUM, aTypeName.pData );
        typelib_EnumTypeDescription * pEnum = reinterpret_cast<typelib_EnumTypeDescription *>(pReg);

        pEnum->nDefaultEnumValue = nDefaultValue;

        pReg->pWeakRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        // sizeof(void) not allowed
        pReg->nSize = ::typelib_typedescription_getAlignedUnoSize( pReg, 0, pReg->nAlignment );
        pReg->nAlignment = ::adjustAlignment( pReg->nAlignment );
        pReg->bComplete = false;

        ::typelib_typedescription_register( &pReg );
        *ppRef = reinterpret_cast<typelib_TypeDescriptionReference *>(pReg);
        assert( *ppRef == pReg->pWeakRef );
    }
    // another static ref:
    ++((*ppRef)->nStaticRefCount);
}

} // extern "C"

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
