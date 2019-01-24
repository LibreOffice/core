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


#include <sal/log.hxx>
#include <uno/data.h>

#include "constr.hxx"
#include "destr.hxx"
#include "copy.hxx"
#include "assign.hxx"
#include "eq.hxx"

using namespace ::cppu;

namespace cppu
{

// Sequence<>() (default ctor) relies on this being static:
uno_Sequence g_emptySeq = { 1, 0, { 0 } };
typelib_TypeDescriptionReference * g_pVoidType = nullptr;


void * binuno_queryInterface( void * pUnoI, typelib_TypeDescriptionReference * pDestType )
{
    // init queryInterface() td
    static typelib_TypeDescription* g_pQITD = []() {
        typelib_TypeDescriptionReference* type_XInterface
            = *typelib_static_type_getByTypeClass(typelib_TypeClass_INTERFACE);
        typelib_InterfaceTypeDescription* pTXInterfaceDescr = nullptr;
        TYPELIB_DANGER_GET(reinterpret_cast<typelib_TypeDescription**>(&pTXInterfaceDescr),
                           type_XInterface);
        assert(pTXInterfaceDescr->ppAllMembers);
        typelib_TypeDescription* pQITD = nullptr;
        typelib_typedescriptionreference_getDescription(&pQITD,
                                                        pTXInterfaceDescr->ppAllMembers[0]);
        // coverity[callee_ptr_arith] - not a bug
        TYPELIB_DANGER_RELEASE(&pTXInterfaceDescr->aBase);
        return pQITD;
    }();

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
        return true;
    if (typelib_TypeClass_INTERFACE != pFromType->eTypeClass ||
        typelib_TypeClass_INTERFACE != pAssignable->eTypeClass)
    {
        return false;
    }

    // query
    if (nullptr == pFrom)
        return false;
    void * pInterface = *static_cast<void **>(pFrom);
    if (nullptr == pInterface)
        return false;

    if (nullptr == queryInterface)
        queryInterface = binuno_queryInterface;
    void * p = (*queryInterface)( pInterface, pAssignable );
    _release( p, release );
    return (nullptr != p);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
