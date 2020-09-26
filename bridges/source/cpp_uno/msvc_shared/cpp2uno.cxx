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

#include <malloc.h>

#include <com/sun/star/uno/genfunc.hxx>
#include <sal/log.hxx>
#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include "bridge.hxx"
#include "cppinterfaceproxy.hxx"
#include "types.hxx"
#include "vtablefactory.hxx"

#include <msvc/except.hxx>
#include <msvc/cpp2uno.hxx>

using namespace ::com::sun::star;

static typelib_TypeClass
cpp2uno_call(bridges::cpp_uno::shared::CppInterfaceProxy* pThis,
             const typelib_TypeDescription* pMemberTD,
             typelib_TypeDescriptionReference* pReturnTypeRef, // nullptr indicates void return
             const sal_Int32 nParams, typelib_MethodParameter* pParams, void** pCallStack,
             void** const pReturnAddr)
{
    // return type
    typelib_TypeDescription* pReturnTD = nullptr;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET(&pReturnTD, pReturnTypeRef);

    // if we don't return via register, the first stack parameter is the return value
    int nFirstRealParam = 2;
    if (pReturnAddr == pCallStack)
        ++nFirstRealParam;

    void* pUnoReturn = nullptr;
    // complex return ptr: if != nullptr && != pUnoReturn, reconversion needed
    void* pCppReturn = nullptr;

    if (pReturnTD)
    {
        if (bridges::cpp_uno::shared::isSimpleType(pReturnTD))
            pUnoReturn = pReturnAddr;
        else
        {
            pCppReturn = pCallStack[nFirstRealParam++];
            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTD)
                              ? alloca(pReturnTD->nSize)
                              : pCppReturn);
        }
    }

    // parameters passed to the UNO function
    void** pUnoArgs = static_cast<void**>(alloca(sizeof(void*) * nParams));

    // parameters received from C++
    void** pCppArgs = static_cast<void**>(alloca(sizeof(void*) * nParams));

    // TODO: switch to typedef std::pair<sal_Int32, typelib_TypeDescription*> ReconversationInfo;
    sal_Int32 nTempIndex = 0;
    // indexes of values this have to be converted (interface conversion C++<=>UNO)
    sal_Int32* pTempIndexes = static_cast<sal_Int32*>(alloca(sizeof(sal_Int32) * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription** ppTempParamTD
        = static_cast<typelib_TypeDescription**>(alloca(sizeof(void*) * nParams));

    for (std::pair<sal_Int32, void**> p(0, pCallStack + nFirstRealParam); p.first < nParams;
         ++p.first, ++p.second)
    {
        const auto& nPos = p.first;
        auto& pCppIncomingParams = p.second;

        const typelib_MethodParameter& rParam = pParams[nPos];
        typelib_TypeDescription* pParamTD = nullptr;
        TYPELIB_DANGER_GET(&pParamTD, rParam.pTypeRef);

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTD))
        {
            pCppArgs[nPos] = pCppIncomingParams;
            pUnoArgs[nPos] = pCppIncomingParams;
            if (sizeof(void*) == sizeof(sal_Int32)) // account 64bit types on 32bit arch
            {
                switch (pParamTD->eTypeClass)
                {
                    case typelib_TypeClass_HYPER:
                    case typelib_TypeClass_UNSIGNED_HYPER:
                    case typelib_TypeClass_DOUBLE:
                        ++pCppIncomingParams;
                        break;
                    default:
                        break;
                }
            }
            TYPELIB_DANGER_RELEASE(pParamTD);
        }
        else // ptr to complex value | ref
        {
            pCppArgs[nPos] = *pCppIncomingParams;

            if (!rParam.bIn) // is pure out
            {
                // UNO out is unconstructed memory
                pUnoArgs[nPos] = alloca(pParamTD->nSize);
                // pParamTD will be released at reconversion
                pTempIndexes[nTempIndex] = nPos;
                ppTempParamTD[nTempIndex++] = pParamTD;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTD))
            {
                ::uno_copyAndConvertData(pUnoArgs[nPos] = alloca(pParamTD->nSize),
                                         *pCppIncomingParams, pParamTD,
                                         pThis->getBridge()->getCpp2Uno());
                // pParamTD will be released at reconversion
                pTempIndexes[nTempIndex] = nPos;
                ppTempParamTD[nTempIndex++] = pParamTD;
            }
            else // direct way
            {
                pUnoArgs[nPos] = *pCppIncomingParams;
                TYPELIB_DANGER_RELEASE(pParamTD);
            }
        }
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any* pUnoExc = &aUnoExc;

    // invoke UNO dispatch call
    pThis->getUnoI()->pDispatcher(pThis->getUnoI(), pMemberTD, pUnoReturn, pUnoArgs, &pUnoExc);

    // in case an exception occurred...
    if (pUnoExc)
    {
        // destruct temporary in/inout params
        while (nTempIndex--)
        {
            const sal_Int32 nIndex = pTempIndexes[nTempIndex];
            if (pParams[nIndex].bIn) // is in/inout => was constructed
                ::uno_destructData(pUnoArgs[nIndex], ppTempParamTD[nTempIndex], nullptr);
            TYPELIB_DANGER_RELEASE(ppTempParamTD[nTempIndex]);
        }
        if (pReturnTD)
            TYPELIB_DANGER_RELEASE(pReturnTD);

        msvc_raiseException(&aUnoExc, pThis->getBridge()->getUno2Cpp()); // has to destruct the any

        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // no exception occurred...
    {
        // handle temporary params
        while (nTempIndex--)
        {
            const sal_Int32 nIndex = pTempIndexes[nTempIndex];
            typelib_TypeDescription* pParamTD = ppTempParamTD[nTempIndex];

            if (pParams[nIndex].bOut) // inout/out
            {
                // convert and assign
                ::uno_destructData(pCppArgs[nIndex], pParamTD, uno::cpp_release);
                ::uno_copyAndConvertData(pCppArgs[nIndex], pUnoArgs[nIndex], pParamTD,
                                         pThis->getBridge()->getUno2Cpp());
            }
            // destroy temp UNO param
            ::uno_destructData(pUnoArgs[nIndex], pParamTD, nullptr);

            TYPELIB_DANGER_RELEASE(pParamTD);
        }

        // handle return
        if (pCppReturn) // has complex return
        {
            if (pUnoReturn != pCppReturn) // needs reconversion
            {
                ::uno_copyAndConvertData(pCppReturn, pUnoReturn, pReturnTD,
                                         pThis->getBridge()->getUno2Cpp());
                // destroy temp UNO return
                ::uno_destructData(pUnoReturn, pReturnTD, nullptr);
            }
            *pReturnAddr = pCppReturn;
        }

        if (!pReturnTD)
            return typelib_TypeClass_VOID;
        else
        {
            typelib_TypeClass eRet = pReturnTD->eTypeClass;
            TYPELIB_DANGER_RELEASE(pReturnTD);
            return eRet;
        }
    }
}

typelib_TypeClass __cdecl cpp_mediate(void** pCallStack, const sal_Int32 nFunctionIndex,
                                      const sal_Int32 nVtableOffset,
                                      sal_Int64* const pRegisterReturn)
{
    // pCallStack:
    // x64: ret value, ret adr, this, [complex ret *], cpp params
    // x86: ret adr, this, [complex ret *], cpp params
    //
    // pRegisterReturn is just set on x86
    // the return value is either the direct set for simply types, or it is set
    // to "complex ret *" and the real return value is constructed at that memory.

    // if we don't return via register, the first stack parameter is the return value
    void** const pReturnAddr
        = pRegisterReturn ? reinterpret_cast<void**>(pRegisterReturn) : pCallStack;

    void* const pThis = static_cast<char*>(pCallStack[pRegisterReturn ? 1 : 2]) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy* pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(pThis);

    typelib_InterfaceTypeDescription* pInterfaceTD = pCppI->getTypeDescr();

    SAL_INFO("bridges", "cpp_vtable_call: pCallStack=["
                            << std::hex << pCallStack[0] << "," << pCallStack[1] << ","
                            << pCallStack[2] << ",...], pThis=" << pThis << ", pCppI=" << pCppI
                            << std::dec << ", nFunctionIndex=" << nFunctionIndex
                            << ", nVtableOffset=" << nVtableOffset);
    SAL_INFO("bridges", "name=" << OUString::unacquired(&pInterfaceTD->aBase.pTypeName));

    if (nFunctionIndex >= pInterfaceTD->nMapFunctionIndexToMemberIndex)
    {
        OUString sError = "illegal " + OUString::unacquired(&pInterfaceTD->aBase.pTypeName)
                          + " vtable index " + OUString::number(nFunctionIndex) + "/"
                          + OUString::number(pInterfaceTD->nMapFunctionIndexToMemberIndex);
        SAL_WARN("bridges", sError);
        throw uno::RuntimeException(sError, reinterpret_cast<uno::XInterface*>(pThis));
    }

    // determine called method
    sal_Int32 nMemberPos = pInterfaceTD->pMapFunctionIndexToMemberIndex[nFunctionIndex];
    assert(nMemberPos < pInterfaceTD->nAllMembers);

    uno::TypeDescription aMemberDescr(pInterfaceTD->ppAllMembers[nMemberPos]);

    SAL_INFO("bridges", "Calling " << OUString::unacquired(&aMemberDescr.get()->pTypeName));

    typelib_TypeClass eRet = typelib_TypeClass_VOID;
    switch (aMemberDescr.get()->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference* pAttrTypeRef
                = reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(aMemberDescr.get())
                      ->pAttributeTypeRef;

            if (pInterfaceTD->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
            { // is GET method
                eRet = cpp2uno_call(pCppI, aMemberDescr.get(), pAttrTypeRef, 0, nullptr, pCallStack,
                                    pReturnAddr);
            }
            else
            { // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn = true;
                aParam.bOut = false;

                eRet = cpp2uno_call(pCppI, aMemberDescr.get(), nullptr, 1, &aParam, pCallStack,
                                    pReturnAddr);
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            // is METHOD
            switch (nFunctionIndex)
            {
                // standard XInterface vtable calls
                case 1: // acquire()
                    pCppI->acquireProxy(); // non virtual call!
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 2: // release()
                    pCppI->releaseProxy(); // non virtual call!
                    eRet = typelib_TypeClass_VOID;
                    break;
                case 0: // queryInterface() opt
                {
                    const unsigned int nCppStackPos = (pReturnAddr == pCallStack) ? 4 : 3;
                    typelib_TypeDescription* pQueryTD = nullptr;
                    TYPELIB_DANGER_GET(
                        &pQueryTD,
                        static_cast<uno::Type*>(pCallStack[nCppStackPos])->getTypeLibType());
                    if (pQueryTD)
                    {
                        uno::XInterface* pInterface = nullptr;

                        pCppI->getBridge()->getCppEnv()->getRegisteredInterface(
                            pCppI->getBridge()->getCppEnv(), reinterpret_cast<void**>(&pInterface),
                            pCppI->getOid().pData,
                            reinterpret_cast<typelib_InterfaceTypeDescription*>(pQueryTD));

                        if (pInterface)
                        {
                            const unsigned int nReturnAddrPos = nCppStackPos - 1;
                            ::uno_any_construct(static_cast<uno_Any*>(pCallStack[nReturnAddrPos]),
                                                &pInterface, pQueryTD, uno::cpp_acquire);
                            pInterface->release();
                            TYPELIB_DANGER_RELEASE(pQueryTD);

                            *pReturnAddr = pCallStack[nReturnAddrPos];
                            eRet = typelib_TypeClass_ANY;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(pQueryTD);
                    }
                    [[fallthrough]];
                }
                default: // perform queryInterface()
                {
                    typelib_InterfaceMethodTypeDescription* pMethodTD
                        = reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(
                            aMemberDescr.get());

                    eRet = cpp2uno_call(pCppI, aMemberDescr.get(), pMethodTD->pReturnTypeRef,
                                        pMethodTD->nParams, pMethodTD->pParams, pCallStack,
                                        pReturnAddr);
                }
            }
            break;
        }
        default:
            throw uno::RuntimeException("no member description found!",
                                        reinterpret_cast<uno::XInterface*>(pThis));
    }

    return eRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
