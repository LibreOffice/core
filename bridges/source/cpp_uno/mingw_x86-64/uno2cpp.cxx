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

#include <exception>
#include <typeinfo>

#include "rtl/alloc.h"
#include "rtl/ustrbuf.hxx"

#include <com/sun/star/uno/genfunc.hxx>
#include "com/sun/star/uno/RuntimeException.hpp"
#include <uno/data.h>
#include <sal/alloca.h>

#include <bridge.hxx>
#include <types.hxx>
#include "unointerfaceproxy.hxx"
#include "vtables.hxx"

#include "abi.hxx"
#include "callvirtualmethod.hxx"
#include "share.hxx"

using namespace ::com::sun::star::uno;

// Macros for easier insertion of values to registers or stack
// pSV - pointer to the source
// nr - order of the value [will be increased if stored to register]
// pFPR, pGPR - pointer to the registers
// pDS - pointer to the stack [will be increased if stored here]

// The value in %xmm register is already prepared to be retrieved as a float,
// thus we treat float and double the same
#define INSERT_FLOAT_DOUBLE( pSV, nr, pFPR, pDS ) \
    if ( nr < x86_64::MAX_SSE_REGS ) \
        pFPR[nr++] = *reinterpret_cast<double *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV ); // verbatim!

#define INSERT_INT64( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt64 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt64 *>( pSV );

#define INSERT_INT32( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt32 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt32 *>( pSV );

#define INSERT_INT16( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt16 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt16 *>( pSV );

#define INSERT_INT8( pSV, nr, pGPR, pDS ) \
    if ( nr < x86_64::MAX_GPR_REGS ) \
        pGPR[nr++] = *reinterpret_cast<sal_uInt8 *>( pSV ); \
    else \
        *pDS++ = *reinterpret_cast<sal_uInt8 *>( pSV );


namespace {

void appendCString(OUStringBuffer & buffer, char const * text) {
    if (text != 0) {
        buffer.append(
            OStringToOUString(OString(text), RTL_TEXTENCODING_ISO_8859_1));
            // use 8859-1 to avoid conversion failure
    }
}

}

static void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
    // Maxium space for [complex ret ptr], values | ptr ...
    // (but will be used less - some of the values will be in pGPR and pFPR)
      sal_uInt64 *pStack = (sal_uInt64 *)__builtin_alloca( (nParams + 3) * sizeof(sal_uInt64) );
      sal_uInt64 *pStackStart = pStack;

    sal_uInt64 pGPR[x86_64::MAX_GPR_REGS];
    sal_uInt32 nGPR = 0;

    double pFPR[x86_64::MAX_SSE_REGS];
    sal_uInt32 nFPR = 0;

    // Return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    assert(pReturnTypeDescr);

    void * pCppReturn = 0; // if != 0 && != pUnoReturn, needs reconversion (see below)

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
    void ** pCppArgs = (void **)alloca( 3 * sizeof(void *) * nParams );
    // Indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = (sal_Int32 *)(pCppArgs + nParams);
    // Type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = alloca( 8 ), pUnoArgs[nPos], pParamTypeDescr,
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
                uno_constructData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType( pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr, pThis->getBridge()->getUno2Cpp() );

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
                pGPR, nGPR,
                pFPR, nFPR );
        } catch (const Exception &) {
            throw;
        } catch (const std::exception & e) {
            OUStringBuffer buf;
            buf.append("C++ code threw ");
            appendCString(buf, typeid(e).name());
            buf.append(": ");
            appendCString(buf, e.what());
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
        fillUnoException( __cxa_get_globals()->caughtExceptions, *ppUnoExc, pThis->getBridge()->getCpp2Uno() );

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
        assert(nMemberPos < pTypeDescr->nAllMembers);
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
        assert( nMemberPos < pTypeDescr->nAllMembers);
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

        Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct( *ppException, &aExc, rExcType.getTypeLibType(), 0 );
    }
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
