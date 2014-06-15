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

#if defined (FREEBSD) || defined(NETBSD) || defined(OPENBSD) || defined(DRAGONFLY)
#include <stdlib.h>
#else
#include <alloca.h>
#endif
#include <exception>
#include <typeinfo>

#include <fficonfig.h>
#include <ffi.h>

#include <rtl/alloc.h>
#include <rtl/ustrbuf.hxx>

#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <uno/data.h>

#include <bridges/cpp_uno/shared/bridge.hxx>
#include <bridges/cpp_uno/shared/types.hxx>
#include <bridges/cpp_uno/shared/unointerfaceproxy.hxx>
#include <bridges/cpp_uno/shared/vtables.hxx>

#include "share.hxx"

using namespace ::com::sun::star::uno;

static ffi_type* unoTypeToFFI(_typelib_TypeClass eTypeClass)
{
    switch (eTypeClass)
    {
    case typelib_TypeClass_VOID:           return &ffi_type_void;
    case typelib_TypeClass_CHAR:           return &ffi_type_sint16;
    case typelib_TypeClass_BOOLEAN:        return &ffi_type_uint8;
    case typelib_TypeClass_BYTE:           return &ffi_type_sint8;
    case typelib_TypeClass_SHORT:          return &ffi_type_sint16;
    case typelib_TypeClass_UNSIGNED_SHORT: return &ffi_type_uint16;
    case typelib_TypeClass_LONG:           return &ffi_type_sint32;
    case typelib_TypeClass_UNSIGNED_LONG:  return &ffi_type_uint32;
    case typelib_TypeClass_HYPER:          return &ffi_type_sint64;
    case typelib_TypeClass_UNSIGNED_HYPER: return &ffi_type_uint64;
    case typelib_TypeClass_FLOAT:          return &ffi_type_float;
    case typelib_TypeClass_DOUBLE:         return &ffi_type_double;
    case typelib_TypeClass_ENUM:           return &ffi_type_uint32;
    default:                               return &ffi_type_pointer;
    }
}

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // Argument types & values
    // Has to be larger, to fit the "this" pointer as the 1st param
    ffi_type* pTypes[nParams + 1];
    void ** pValues = (void **)alloca(3 * sizeof(void *) * (nParams + 1));

    // Push "this" pointer
    void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
    pValues[0] = pAdjustedThisPtr;
    pTypes[0] = &ffi_type_pointer;

    // Indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pValues + nParams + 1);

    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pValues + (2 * nParams));

    sal_Int32 nTempIndices = 0;

    for (sal_Int32 in = 0, out = 1; in < nParams; ++in, ++out)
    {
        const typelib_MethodParameter & rParam = pParams[in];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            pValues[out] = alloca(8);
            uno_copyAndConvertData(pValues[out], pUnoArgs[in], pParamTypeDescr,
                                   pThis->getBridge()->getUno2Cpp());

            pTypes[out] = unoTypeToFFI(pParamTypeDescr->eTypeClass);

            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            if (! rParam.bIn) // is pure out
            {
                // cpp out is constructed mem, uno out is not!
                pValues[out] = alloca(pParamTypeDescr->nSize);
                uno_constructData(pValues[out], pParamTypeDescr);

                // default constructed for cpp call
                pTempIndices[nTempIndices] = in;

                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTypeDescr)) // is in/inout
            {
                pValues[out] = alloca(pParamTypeDescr->nSize);
                uno_copyAndConvertData(pValues[out], pUnoArgs[in], pParamTypeDescr, pThis->getBridge()->getUno2Cpp());

                // has to be reconverted
                pTempIndices[nTempIndices] = in;

                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pValues[out] = pUnoArgs[in];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }

            // TODO FIXME this is wrong, we need the complete FFI type info
            // here
            pTypes[out] = &ffi_type_pointer;
        }
    }

    // Return type / value
    typelib_TypeDescription *pReturnTypeDescr = NULL;
    TYPELIB_DANGER_GET(&pReturnTypeDescr, pReturnTypeRef);
    OSL_ENSURE(pReturnTypeDescr, "### expected return type description!");

    void * pCppReturn = NULL;
    if (pReturnTypeDescr)
    {
        pCppReturn = bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTypeDescr)?
            __builtin_alloca(pReturnTypeDescr->nSize) : pUnoReturn;
    }

    try
    {
        try {
            ffi_cif cif;
            if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, nParams + 1, unoTypeToFFI(pReturnTypeDescr->eTypeClass), pTypes) == FFI_OK)
            {
                // pAdjustedThisPtr is the pointer to start of vtable:
                // dereference it + find the method in the vtable (on the
                // aVtableSlot.index position), and dereference again
                sal_IntPtr pMethod =
                    *reinterpret_cast<sal_IntPtr*>(
                            *reinterpret_cast<sal_IntPtr*>(pAdjustedThisPtr) + sizeof(void*) * aVtableSlot.index);

                // perform the call
                ffi_call(&cif, reinterpret_cast<void (*)()>(pMethod), pCppReturn, pValues);
            }
        } catch (const Exception &) {
            throw;
        } catch (const std::exception & e) {
            OUStringBuffer buf;
            buf.append("C++ code threw ");
            buf.appendAscii(typeid(e).name());
            buf.append(": ");
            buf.appendAscii(e.what());
            throw RuntimeException(buf.makeStringAndClear());
        } catch (...) {
            throw RuntimeException("C++ code threw unknown exception");
        }

        *ppUnoExc = 0;

        // reconvert temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) // inout
                {
                    uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 ); // destroy uno value
                    uno_copyAndConvertData(pUnoArgs[nIndex], pValues[nIndex], pParamTypeDescr,
                                           pThis->getBridge()->getCpp2Uno());
                }
            }
            else // pure out
            {
                uno_copyAndConvertData(pUnoArgs[nIndex], pValues[nIndex], pParamTypeDescr,
                                       pThis->getBridge()->getCpp2Uno());
            }

            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData(pValues[nIndex], pParamTypeDescr, cpp_release);

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return value
        if (pCppReturn && pUnoReturn != pCppReturn)
        {
            uno_copyAndConvertData( pUnoReturn, pCppReturn, pReturnTypeDescr,
                                    pThis->getBridge()->getCpp2Uno() );
            uno_destructData( pCppReturn, pReturnTypeDescr, cpp_release );
        }
    }
    catch (...)
    {
        // fill uno exception
#ifdef _LIBCPP_VERSION
        CPPU_CURRENT_NAMESPACE::fillUnoException(
            reinterpret_cast< __cxxabiv1::__cxa_eh_globals * >(
                __cxxabiv1::__cxa_get_globals())->caughtExceptions,
            *ppUnoExc, pThis->getBridge()->getCpp2Uno());
#else
        fillUnoException(
            reinterpret_cast< CPPU_CURRENT_NAMESPACE::__cxa_eh_globals * >(
                __cxxabiv1::__cxa_get_globals())->caughtExceptions,
            *ppUnoExc, pThis->getBridge()->getCpp2Uno());
#endif

        // temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData(pValues[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release);
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndices] );
        }
        // return type
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
    }
}


namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, const typelib_TypeDescription * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
#if OSL_DEBUG_LEVEL > 0
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
#endif

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif
        VtableSlot aVtableSlot(
                getVtableSlot(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberDescr)));

        if (pReturn)
        {
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef,
                0, 0, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)pMemberDescr)->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            typelib_TypeDescriptionReference * pReturnTypeRef = 0;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

            // dependent dispatch
            aVtableSlot.index += 1; // get, then set method
            cpp_call(
                pThis, aVtableSlot, // get, then set method
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = ((typelib_InterfaceMemberTypeDescription *)pMemberDescr)->nPosition;
        OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### member pos out of range!" );
#endif
        VtableSlot aVtableSlot(
                getVtableSlot(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberDescr)));

        switch (aVtableSlot.index)
        {
            // standard calls
        case 1: // acquire uno interface
            (*pUnoI->acquire)( pUnoI );
            *ppException = 0;
            break;
        case 2: // release uno interface
            (*pUnoI->release)( pUnoI );
            *ppException = 0;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pArgs[0] )->getTypeLibType() );
            if (pTD)
            {
                uno_Interface * pInterface = 0;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( pReturn ),
                        &pInterface, pTD, 0 );
                    (*pInterface->release)( pInterface );
                    TYPELIB_DANGER_RELEASE( pTD );
                    *ppException = 0;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } // else perform queryInterface()
        default:
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->nParams,
                ((typelib_InterfaceMethodTypeDescription *)pMemberDescr)->pParams,
                pReturn, pArgs, ppException );
        }
        break;
    }
    default:
    {
        ::com::sun::star::uno::RuntimeException aExc(
            OUString("illegal member type description!"),
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = ::getCppuType( &aExc );
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
