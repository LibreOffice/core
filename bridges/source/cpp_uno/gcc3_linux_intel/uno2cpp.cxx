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

#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <uno/data.h>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>

#include "callvirtualmethod.hxx"
#include "share.hxx"

using namespace ::com::sun::star::uno;

namespace
{

void cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void * pUnoReturn, void * pUnoArgs[], uno_Any ** ppUnoExc )
{
      // max space for: [complex ret ptr], values|ptr ...
      char * pCppStack      =
          static_cast<char *>(alloca( sizeof(sal_Int32) + ((nParams+2) * sizeof(sal_Int64)) ));
      char * pCppStackStart = pCppStack;

    // return
    typelib_TypeDescription * pReturnTypeDescr = nullptr;
    TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );
    assert(pReturnTypeDescr);

    void * pCppReturn = nullptr; // if != 0 && != pUnoReturn, needs reconversion
    bool bSimpleReturn = true;

    if (pReturnTypeDescr)
    {
        bSimpleReturn = x86::isSimpleReturnType(pReturnTypeDescr);
        if (bSimpleReturn)
        {
            pCppReturn = pUnoReturn; // direct way for simple types
        }
        else
        {
            // complex return via ptr
            pCppReturn = *reinterpret_cast<void **>(pCppStack)
                = (bridges::cpp_uno::shared::relatesToInterfaceType(
                       pReturnTypeDescr )
                   ? alloca( pReturnTypeDescr->nSize )
                   : pUnoReturn); // direct way
            pCppStack += sizeof(void *);
        }
    }
    // push this
    void * pAdjustedThisPtr = reinterpret_cast< void ** >(pThis->getCppI())
        + aVtableSlot.offset;
    *reinterpret_cast<void **>(pCppStack) = pAdjustedThisPtr;
    pCppStack += sizeof( void* );

    // stack space
    static_assert(sizeof(void *) == sizeof(sal_Int32), "### unexpected size!");
    // args
    void ** pCppArgs  = static_cast<void **>(alloca( 3 * sizeof(void *) * nParams ));
    // indices of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndices = reinterpret_cast<sal_Int32 *>(pCppArgs + nParams);
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = reinterpret_cast<typelib_TypeDescription **>(pCppArgs + (2 * nParams));

    sal_Int32 nTempIndices   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = nullptr;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut
            && bridges::cpp_uno::shared::isSimpleType( pParamTypeDescr ))
        {
            uno_copyAndConvertData( pCppArgs[nPos] = pCppStack, pUnoArgs[nPos], pParamTypeDescr,
                                    pThis->getBridge()->getUno2Cpp() );

            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); // extra long
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
                    *reinterpret_cast<void **>(pCppStack) = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pParamTypeDescr );
                pTempIndices[nTempIndices] = nPos; // default constructed for cpp call
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(
                         pParamTypeDescr ))
            {
                uno_copyAndConvertData(
                    *reinterpret_cast<void **>(pCppStack) = pCppArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                    pUnoArgs[nPos], pParamTypeDescr,
                    pThis->getBridge()->getUno2Cpp() );

                pTempIndices[nTempIndices] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndices++] = pParamTypeDescr;
            }
            else // direct way
            {
                *reinterpret_cast<void **>(pCppStack) = pCppArgs[nPos] = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }

    try
    {
        assert( !( (pCppStack - pCppStackStart ) & 3) && "UNALIGNED STACK !!! (Please DO panic)" );
        CPPU_CURRENT_NAMESPACE::callVirtualMethod(
            pAdjustedThisPtr, aVtableSlot.index,
            pCppReturn, pReturnTypeDescr, bSimpleReturn,
            reinterpret_cast<sal_Int32 *>(pCppStackStart), (pCppStack - pCppStackStart) / sizeof(sal_Int32) );
        // NO exception occurred...
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
         fillUnoException(
             reinterpret_cast< CPPU_CURRENT_NAMESPACE::__cxa_eh_globals * >(
                 __cxxabiv1::__cxa_get_globals())->caughtExceptions,
             *ppUnoExc, pThis->getBridge()->getCpp2Uno());

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

}

namespace x86
{
    bool isSimpleReturnType(typelib_TypeDescription * pTD, bool recursive)
    {
        if (bridges::cpp_uno::shared::isSimpleType( pTD ))
            return true;
#if defined(FREEBSD) || defined(NETBSD) || defined(OPENBSD) || \
    defined(MACOSX) || defined(DRAGONFLY)
        // Only structs of exactly 1, 2, 4, or 8 bytes are returned through
        // registers, see <http://developer.apple.com/documentation/DeveloperTools/
        // Conceptual/LowLevelABI/Articles/IA32.html>:
        if (pTD->eTypeClass == typelib_TypeClass_STRUCT &&
            (recursive || pTD->nSize <= 2 || pTD->nSize == 4 || pTD->nSize == 8))
        {
            typelib_CompoundTypeDescription *const pCompTD =
                (typelib_CompoundTypeDescription *) pTD;
            for ( sal_Int32 pos = pCompTD->nMembers; pos--; ) {
                typelib_TypeDescription * pMemberTD = 0;
                TYPELIB_DANGER_GET( &pMemberTD, pCompTD->ppTypeRefs[pos] );
                bool const b = isSimpleReturnType(pMemberTD, true);
                TYPELIB_DANGER_RELEASE( pMemberTD );
                if (! b)
                    return false;
            }
            return true;
        }
#else
        (void)recursive;
#endif
        return false;
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
                pThis, aVtableSlot,
                pReturnTypeRef,
                1, &aParam,
                pReturn, pArgs, ppException );

            typelib_typedescriptionreference_release( pReturnTypeRef );
        }

        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
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
                (*pThis->pBridge->getUnoEnv()->getRegisteredInterface)(
                    pThis->pBridge->getUnoEnv(),
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
            SAL_FALLTHROUGH; // else perform queryInterface()
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
