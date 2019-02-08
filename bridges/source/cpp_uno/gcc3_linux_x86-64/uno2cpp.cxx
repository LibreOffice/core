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

#include <sal/alloca.h>

#include <exception>
#include <typeinfo>

#include <rtl/alloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <o3tl/runtimetooustring.hxx>
#include <uno/data.h>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>

#include "abi.hxx"
#include "callvirtualmethod.hxx"
#include "share.hxx"

using namespace ::com::sun::star::uno;

namespace {

// Functions for easier insertion of values to registers or stack
// pSV - pointer to the source
// nr - order of the value [will be increased if stored to register]
// pFPR, pGPR - pointer to the registers
// pDS - pointer to the stack [will be increased if stored here]

// The value in %xmm register is already prepared to be retrieved as a float,
// thus we treat float and double the same
void INSERT_FLOAT_DOUBLE(
    void const * pSV, sal_uInt32 & nr, double * pFPR, sal_uInt64 *& pDS)
{
    if ( nr < x86_64::MAX_SSE_REGS )
        pFPR[nr++] = *static_cast<double const *>( pSV );
    else
        *pDS++ = *static_cast<sal_uInt64 const *>( pSV ); // verbatim!
}

void INSERT_INT64(
    void const * pSV, sal_uInt32 & nr, sal_uInt64 * pGPR, sal_uInt64 *& pDS)
{
    if ( nr < x86_64::MAX_GPR_REGS )
        pGPR[nr++] = *static_cast<sal_uInt64 const *>( pSV );
    else
        *pDS++ = *static_cast<sal_uInt64 const *>( pSV );
}

void INSERT_INT32(
    void const * pSV, sal_uInt32 & nr, sal_uInt64 * pGPR, sal_uInt64 *& pDS)
{
    if ( nr < x86_64::MAX_GPR_REGS )
        pGPR[nr++] = *static_cast<sal_uInt32 const *>( pSV );
    else
        *pDS++ = *static_cast<sal_uInt32 const *>( pSV );
}

void INSERT_INT16(
    void const * pSV, sal_uInt32 & nr, sal_uInt64 * pGPR, sal_uInt64 *& pDS)
{
    if ( nr < x86_64::MAX_GPR_REGS )
        pGPR[nr++] = *static_cast<sal_uInt16 const *>( pSV );
    else
        *pDS++ = *static_cast<sal_uInt16 const *>( pSV );
}

void INSERT_INT8(
    void const * pSV, sal_uInt32 & nr, sal_uInt64 * pGPR, sal_uInt64 *& pDS)
{
    if ( nr < x86_64::MAX_GPR_REGS )
        pGPR[nr++] = *static_cast<sal_uInt8 const *>( pSV );
    else
        *pDS++ = *static_cast<sal_uInt8 const *>( pSV );
}

}

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // Maximum space for [complex ret ptr], values | ptr ...
    // (but will be used less - some of the values will be in pGPR and pFPR)
    sal_uInt64 *pStack = static_cast<sal_uInt64 *>(__builtin_alloca( (nParams + 3) * sizeof(sal_uInt64) ));
    sal_uInt64 *pStackStart = pStack;

    sal_uInt64 pGPR[x86_64::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    double pFPR[x86_64::MAX_SSE_REGS];
    sal_uInt32 nFPR = 0;

    // Return
    typelib_TypeDescription * pReturnTypeDescr = nullptr;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    assert(pReturnTypeDescr);

    void * pCppReturn = nullptr; // if != 0 && != pUnoReturn, needs reconversion (see below)

    bool bSimpleReturn = true;
    if ( pReturnTypeDescr )
    {
        if ( x86_64::return_in_hidden_param( pReturnTypeRef ) )
            bSimpleReturn = false;

        if ( bSimpleReturn )
            pCppReturn = pUnoReturn; // direct way for simple types
        else
        {
            // complex return via ptr
            pCppReturn = bridges::cpp_uno::shared::relatesToInterfaceType( pReturnTypeDescr )?
                         __builtin_alloca( pReturnTypeDescr->nSize ) : pUnoReturn;
            INSERT_INT64( &pCppReturn, nGPR, pGPR, pStack );
        }
    }

    // Push "this" pointer
    void * pAdjustedThisPtr = reinterpret_cast< void ** >( pThis->getCppI() ) + aVtableSlot.offset;
    INSERT_INT64( &pAdjustedThisPtr, nGPR, pGPR, pStack );

    // Args
    void ** pCppArgs = static_cast<void **>(alloca( 3 * sizeof(void *) * nParams ));
    // Indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = reinterpret_cast<sal_Int32 *>(pCppArgs + nParams);
    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = reinterpret_cast<typelib_TypeDescription **>(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            pCppArgs[nPos] = alloca( 8 );
            uno_copyAndConvertData( pCppArgs[nPos], pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                INSERT_INT64( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
                INSERT_INT32( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_UNSIGNED_SHORT:
                INSERT_INT16( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                INSERT_INT8( pCppArgs[nPos], nGPR, pGPR, pStack );
                break;
            case typelib_TypeClass_FLOAT:
            case typelib_TypeClass_DOUBLE:
                INSERT_FLOAT_DOUBLE( pCppArgs[nPos], nFPR, pFPR, pStack );
                break;
            default:
                break;
            }

            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            if (! rParam.bIn) // is pure out
            {
                // cpp out is constructed mem, uno out is not!
                pCppArgs[nPos] = alloca( pParamTypeDescr->nSize );
                uno_constructData( pCppArgs[nPos], pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                pCppArgs[nPos] = alloca( pParamTypeDescr->nSize );
                uno_copyAndConvertData(
                    pCppArgs[nPos], pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
            INSERT_INT64( &(pCppArgs[nPos]), nGPR, pGPR, pStack );
        }
    }

    try
    {
        try {
            CPPU_CURRENT_NAMESPACE::callVirtualMethod(
                pAdjustedThisPtr, aVtableSlot.index,
                pCppReturn, pReturnTypeRef, bSimpleReturn,
                pStackStart, ( pStack - pStackStart ),
                pGPR, pFPR );
        } catch (const Exception &) {
            throw;
        } catch (const std::exception & e) {
            throw RuntimeException(
                "C++ code threw " + o3tl::runtimeToOUString(typeid(e).name())
                + ": " + o3tl::runtimeToOUString(e.what()));
        } catch (...) {
            throw RuntimeException("C++ code threw unknown exception");
        }

        *ppUnoExc = nullptr;

        // reconvert temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndices];

            if (pParams[nIndex].bIn)
            {
                if (pParams[nIndex].bOut) // inout
                {
                    uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, nullptr ); // destroy uno value
                    uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                            pThis->getBridge()->getCpp2Uno() );
                }
            }
            else // pure out
            {
                uno_copyAndConvertData( pUnoArgs[nIndex], pCppArgs[nIndex], pParamTypeDescr,
                                        pThis->getBridge()->getCpp2Uno() );
            }
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );

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
        CPPU_CURRENT_NAMESPACE::fillUnoException(*ppUnoExc, pThis->getBridge()->getCpp2Uno());

        // temporary params
        for ( ; nTempIndices--; )
        {
            sal_Int32 nIndex = pTempIndices[nTempIndices];
            // destroy temp cpp param => cpp: every param was constructed
            uno_destructData( pCppArgs[nIndex], ppTempParamTypeDescr[nTempIndices], cpp_release );
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

    switch (pMemberDescr->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        assert(
            (reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberDescr)
             ->nPosition)
            < pThis->pTypeDescr->nAllMembers);
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
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(pMemberDescr)->pAttributeTypeRef,
                0, nullptr, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(pMemberDescr)->pAttributeTypeRef;
            aParam.bIn      = true;
            aParam.bOut     = false;

            typelib_TypeDescriptionReference * pReturnTypeRef = nullptr;
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
        assert(
            (reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberDescr)
             ->nPosition)
            < pThis->pTypeDescr->nAllMembers);
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
            *ppException = nullptr;
            break;
        case 2: // release uno interface
            (*pUnoI->release)( pUnoI );
            *ppException = nullptr;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = nullptr;
            TYPELIB_DANGER_GET( &pTD, static_cast< Type * >( pArgs[0] )->getTypeLibType() );
            if (pTD)
            {
                uno_Interface * pInterface = nullptr;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    reinterpret_cast<void **>(&pInterface), pThis->oid.pData, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );

                if (pInterface)
                {
                    ::uno_any_construct(
                        static_cast< uno_Any * >( pReturn ),
                        &pInterface, pTD, nullptr );
                    (*pInterface->release)( pInterface );
                    TYPELIB_DANGER_RELEASE( pTD );
                    *ppException = nullptr;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
            [[fallthrough]]; // else perform queryInterface()
        }
        default:
            // dependent dispatch
            cpp_call(
                pThis, aVtableSlot,
                reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberDescr)->pReturnTypeRef,
                reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberDescr)->nParams,
                reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberDescr)->pParams,
                pReturn, pArgs, ppException );
        }
        break;
    }
    default:
    {
        ::com::sun::star::uno::RuntimeException aExc(
            "illegal member type description!",
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );

        Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), nullptr );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
