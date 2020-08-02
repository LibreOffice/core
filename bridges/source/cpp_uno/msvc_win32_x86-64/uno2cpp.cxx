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

#include <cassert>

#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <uno/data.h>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>

#include <msvc/except.hxx>

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace ::com::sun::star;

namespace
{

bool cpp_call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis,
    bridges::cpp_uno::shared::VtableSlot aVtableSlot,
    typelib_TypeDescriptionReference * pReturnTypeRef,
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void * pUnoReturn,
    void * pUnoArgs[],
    uno_Any ** ppUnoExc ) throw ()
{
    const int MAXPARAMS = 32;

    if ( nParams > MAXPARAMS )
    {
        // We have a hard limit on the number of parameters so that we
        // don't need any assembler code here but can call the
        // function using normal C++.

        return false;
    }

    // Table with this pointer, optional complex return value ptr, and the parameters
    union {
        sal_Int64 i;
        void *p;
        double d;
    } aCppArgs[MAXPARAMS+2], uRetVal;
    int nCppParamIndex = 0;

    // return type
    typelib_TypeDescription * pReturnTD = nullptr;
    TYPELIB_DANGER_GET( &pReturnTD, pReturnTypeRef );
    assert(pReturnTD);

    // 'this'
    void * pAdjustedThisPtr = reinterpret_cast<void **>( pThis->getCppI() ) + aVtableSlot.offset;
    aCppArgs[nCppParamIndex++].p = pAdjustedThisPtr;

    enum class ReturnKind { Void, Simple, Complex, ComplexConvert };
    ReturnKind retKind;
    if (pUnoReturn == nullptr) {
        retKind = ReturnKind::Void;
    } else {
        assert(pReturnTD != nullptr);
        if (bridges::cpp_uno::shared::isSimpleType(pReturnTD)) {
            retKind = ReturnKind::Simple;
        } else if (bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTD))
        {
            retKind = ReturnKind::ComplexConvert;
            aCppArgs[nCppParamIndex++].p = alloca(pReturnTD->nSize);
        } else {
            retKind = ReturnKind::Complex;
            aCppArgs[nCppParamIndex++].p = pUnoReturn;
        }
    }

    // indexes of values this have to be converted (interface conversion C++<=>UNO)
    int pTempCppIndexes[MAXPARAMS];
    int pTempIndexes[MAXPARAMS];
    int nTempIndexes = 0;

    // type descriptions for reconversions
    typelib_TypeDescription *pTempParamTD[MAXPARAMS];

    for (int nPos = 0; nPos < nParams; ++nPos, ++nCppParamIndex)
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTD = nullptr;
        TYPELIB_DANGER_GET( &pParamTD, rParam.pTypeRef );

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTD))
        {
            ::uno_copyAndConvertData(
                &aCppArgs[nCppParamIndex], pUnoArgs[nPos], pParamTD,
                pThis->getBridge()->getUno2Cpp() );

            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTD );
        }
        else // ptr to complex value | ref
        {
            if (!rParam.bIn) // Is pure out
            {
                // C++ out is constructed mem, UNO out is not!
                ::uno_constructData(
                    aCppArgs[nCppParamIndex].p = alloca( pParamTD->nSize ),
                    pParamTD );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;

                // default constructed for C++ call
                pTempIndexes[nTempIndexes] = nPos;

                // will be released at reconversion
                pTempParamTD[nTempIndexes++] = pParamTD;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTD))
            {
                ::uno_copyAndConvertData(
                    aCppArgs[nCppParamIndex].p = alloca( pParamTD->nSize ),
                    pUnoArgs[nPos], pParamTD,
                    pThis->getBridge()->getUno2Cpp() );

                pTempCppIndexes[nTempIndexes] = nCppParamIndex;

                // has to be reconverted
                pTempIndexes[nTempIndexes] = nPos;

                // will be released at reconversion
                pTempParamTD[nTempIndexes++] = pParamTD;
            }
            else // direct way
            {
                aCppArgs[nCppParamIndex].p = pUnoArgs[nPos];
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTD );
            }
        }
    }

    __try
    {
        // The first real parameter is always 'this'.

        // The Windows x64 calling convention is very regular and
        // elegant (even if perhaps then slightly slower than the
        // Linux x64 one): The first four parameters, never more, are
        // passed in registers, as long as they are a qword in size
        // or less. (If larger, a pointer to a temp copy is passed, so
        // it's equivalent anyway.) Floating point values are passed
        // in XMM0..3 registers, others in RCX, RDX, R8, R9.

        // Now, the nice thing for us is that when calling varargs
        // functions, floating-point parameters among the four first
        // ones are always passed *both* in an XMM and integer
        // register. So we don't need to bother here calling the
        // method different ways depending on what types of parameters
        // it actually expects. We just pretend parameters 3..4 are
        // doubles, and they will be passed both in XMM and integer
        // registers, and the callee will find them where it
        // expects. (The callee is not actually varargs, of course.)

        sal_Int64 (*pIMethod)(sal_Int64, ...) =
            reinterpret_cast<sal_Int64 (*)(sal_Int64, ...)>(
                (*static_cast<sal_uInt64 **>(pAdjustedThisPtr))[aVtableSlot.index]);

        double (*pFMethod)(sal_Int64, ...) =
            reinterpret_cast<double (*)(sal_Int64, ...)>(
                (*static_cast<sal_uInt64 **>(pAdjustedThisPtr))[aVtableSlot.index]);

        // Pass parameters 2..4 as if it was a floating-point value so
        // that it gets put in both XMM and integer registers per the
        // calling convention. It doesn't matter if it actually is a
        // fp or not.

        if ( pReturnTD &&
             (pReturnTD->eTypeClass == typelib_TypeClass_FLOAT ||
              pReturnTD->eTypeClass == typelib_TypeClass_DOUBLE) )
            uRetVal.d =
                pFMethod (aCppArgs[0].i, aCppArgs[1].d, aCppArgs[2].d, aCppArgs[3].d,
                          aCppArgs[4].i, aCppArgs[5].i, aCppArgs[6].i, aCppArgs[7].i,
                          aCppArgs[8].i, aCppArgs[9].i, aCppArgs[10].i, aCppArgs[11].i,
                          aCppArgs[12].i, aCppArgs[13].i, aCppArgs[14].i, aCppArgs[15].i,
                          aCppArgs[16].i, aCppArgs[17].i, aCppArgs[18].i, aCppArgs[19].i,
                          aCppArgs[20].i, aCppArgs[21].i, aCppArgs[22].i, aCppArgs[23].i,
                          aCppArgs[24].i, aCppArgs[25].i, aCppArgs[26].i, aCppArgs[27].i,
                          aCppArgs[28].i, aCppArgs[29].i, aCppArgs[30].i, aCppArgs[31].i );
        else
            uRetVal.i =
                pIMethod (aCppArgs[0].i, aCppArgs[1].d, aCppArgs[2].d, aCppArgs[3].d,
                          aCppArgs[4].i, aCppArgs[5].i, aCppArgs[6].i, aCppArgs[7].i,
                          aCppArgs[8].i, aCppArgs[9].i, aCppArgs[10].i, aCppArgs[11].i,
                          aCppArgs[12].i, aCppArgs[13].i, aCppArgs[14].i, aCppArgs[15].i,
                          aCppArgs[16].i, aCppArgs[17].i, aCppArgs[18].i, aCppArgs[19].i,
                          aCppArgs[20].i, aCppArgs[21].i, aCppArgs[22].i, aCppArgs[23].i,
                          aCppArgs[24].i, aCppArgs[25].i, aCppArgs[26].i, aCppArgs[27].i,
                          aCppArgs[28].i, aCppArgs[29].i, aCppArgs[30].i, aCppArgs[31].i );
    }
    __except (msvc_filterCppException(
                  GetExceptionInformation(),
                  *ppUnoExc, pThis->getBridge()->getCpp2Uno() ))
   {
        // *ppUnoExc was constructed by filter function
        // temporary params
        while (nTempIndexes--)
        {
            int nCppIndex = pTempCppIndexes[nTempIndexes];
            // destroy temp C++ param => C++: every param was constructed
            ::uno_destructData(
                aCppArgs[nCppIndex].p, pTempParamTD[nTempIndexes],
                uno::cpp_release );
            TYPELIB_DANGER_RELEASE( pTempParamTD[nTempIndexes] );
        }

        // return type
        if (pReturnTD)
            TYPELIB_DANGER_RELEASE( pReturnTD );

        return true;
    }

    // NO exception occurred
    *ppUnoExc = nullptr;

    // reconvert temporary params
    while (nTempIndexes--)
    {
        int nCppIndex = pTempCppIndexes[nTempIndexes];
        int nIndex = pTempIndexes[nTempIndexes];
        typelib_TypeDescription * pParamTD =
            pTempParamTD[nTempIndexes];

        if (pParams[nIndex].bIn)
        {
            if (pParams[nIndex].bOut) // inout
            {
                ::uno_destructData(
                    pUnoArgs[nIndex], pParamTD, nullptr ); // destroy UNO value
                ::uno_copyAndConvertData(
                    pUnoArgs[nIndex], aCppArgs[nCppIndex].p, pParamTD,
                    pThis->getBridge()->getCpp2Uno() );
            }
        }
        else // pure out
        {
            ::uno_copyAndConvertData(
                pUnoArgs[nIndex], aCppArgs[nCppIndex].p, pParamTD,
                pThis->getBridge()->getCpp2Uno() );
        }

        // destroy temp C++ param => C++: every param was constructed
        ::uno_destructData(
            aCppArgs[nCppIndex].p, pParamTD, uno::cpp_release );

        TYPELIB_DANGER_RELEASE( pParamTD );
    }

    // return value
    switch (retKind) {
    case ReturnKind::Void:
        break;
    case ReturnKind::Simple:
        *static_cast<sal_Int64*>(pUnoReturn) = uRetVal.i;
        break;
    case ReturnKind::Complex:
        assert(uRetVal.p == pUnoReturn);
        break;
    case ReturnKind::ComplexConvert:
        assert(uRetVal.p == aCppArgs[1].p);
        ::uno_copyAndConvertData(
            pUnoReturn, uRetVal.p, pReturnTD,
            pThis->getBridge()->getCpp2Uno() );
        ::uno_destructData(
            uRetVal.p, pReturnTD, uno::cpp_release );
        break;
    }

    // return type
    if ( pReturnTD )
        TYPELIB_DANGER_RELEASE( pReturnTD );

    return true;
}

} // namespace

namespace bridges::cpp_uno::shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberTD,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    // is my surrogate
    bridges::cpp_uno::shared::UnoInterfaceProxy * pThis
        = static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
#if OSL_DEBUG_LEVEL > 0
    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
#endif

    switch (pMemberTD->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberTD)->nPosition;
        assert(nMemberPos < pTypeDescr->nAllMembers);
#endif
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberTD)));
        if ( pReturn )
        {
            // is GET
            cpp_call(
                pThis, aVtableSlot,
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(pMemberTD)->pAttributeTypeRef,
                0, nullptr, // no params
                pReturn, pArgs, ppException );
        }
        else
        {
            // is SET
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(pMemberTD)->pAttributeTypeRef;
            aParam.bIn      = true;
            aParam.bOut     = false;

            typelib_TypeDescriptionReference * pReturnTypeRef = nullptr;
            OUString aVoidName("void");
            typelib_typedescriptionreference_new(
                &pReturnTypeRef, typelib_TypeClass_VOID, aVoidName.pData );

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
#if OSL_DEBUG_LEVEL > 0
        // determine vtable call index
        sal_Int32 nMemberPos = reinterpret_cast<typelib_InterfaceMemberTypeDescription const *>(pMemberTD)->nPosition;
        assert(nMemberPos < pTypeDescr->nAllMembers);
#endif
        VtableSlot aVtableSlot(
            getVtableSlot(
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberTD)));

        switch (aVtableSlot.index)
        {
        case 1: // acquire UNO interface
            (*pUnoI->acquire)( pUnoI );
            *ppException = nullptr;
            break;
        case 2: // release UNO interface
            (*pUnoI->release)( pUnoI );
            *ppException = nullptr;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = nullptr;
            TYPELIB_DANGER_GET( &pTD, static_cast< uno::Type * >( pArgs[0] )->getTypeLibType() );

            if ( pTD )
            {
                uno_Interface * pInterface = nullptr;
                (*pThis->getBridge()->getUnoEnv()->getRegisteredInterface)(
                    pThis->getBridge()->getUnoEnv(),
                    reinterpret_cast<void **>(&pInterface), pThis->oid.pData, reinterpret_cast<typelib_InterfaceTypeDescription *>(pTD) );

                if ( pInterface )
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
            typelib_InterfaceMethodTypeDescription const* pMethodTD
                = reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberTD);

            if (!cpp_call(pThis, aVtableSlot, pMethodTD->pReturnTypeRef, pMethodTD->nParams,
                          pMethodTD->pParams, pReturn, pArgs, ppException))
            {
                uno::RuntimeException aExc( "Too many parameters!" );

                uno::Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
                ::uno_type_any_construct(*ppException, &aExc, rExcType.getTypeLibType(), nullptr);
            }
        }
        break;
    }
    default:
    {
        uno::RuntimeException aExc("Illegal member type description!", uno::Reference<uno::XInterface>());

        uno::Type const & rExcType = cppu::UnoType<decltype(aExc)>::get();
        // binary identical null reference
        ::uno_type_any_construct(*ppException, &aExc, rExcType.getTypeLibType(), nullptr);
    }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
