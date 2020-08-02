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

#include <msvc/x86.hxx>

using namespace ::com::sun::star;

namespace
{

static inline typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTD,
    typelib_TypeDescriptionReference * pReturnTypeRef, // nullptr indicates void return
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */)
{
    // pCallStack: ret, this, [complex return ptr], params
    char * pCppCallStack = (char *)(pCallStack +2);

    // return type
    typelib_TypeDescription * pReturnTD = nullptr;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET(&pReturnTD, pReturnTypeRef);

    void * pUnoReturn = nullptr;
    void * pCppReturn = nullptr; // complex return ptr: if != nullptr && != pUnoReturn, reconversion need

    if (pReturnTD)
    {
        if (bridges::cpp_uno::shared::isSimpleType(pReturnTD))
            pUnoReturn = pRegisterReturn;
        else
        {
            pCppReturn = *(void **)pCppCallStack;
            pCppCallStack += sizeof(void *);

            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTD)
                          ? alloca(pReturnTD->nSize) : pCppReturn);
        }
    }

    static_assert(sizeof(void *) == sizeof(sal_Int32), "### unexpected size!");

    // parameters passed to the UNO function
    void ** pUnoArgs = static_cast<void **>(alloca(4 * sizeof(void *) * nParams));

    // parameters received from C++
    void ** pCppArgs = pUnoArgs + nParams;

    // indexes of values this have to be converted (interface conversion C++<=>UNO)
    sal_Int32 * pTempIndexes = reinterpret_cast<sal_Int32 *>(pUnoArgs + (2 * nParams));

    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTD =
        reinterpret_cast<typelib_TypeDescription **>(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndex = 0;

    for (sal_Int32 nPos = 0; nPos < nParams; ++nPos)
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTD = nullptr;
        TYPELIB_DANGER_GET(&pParamTD, rParam.pTypeRef);

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTD))
        {
            pCppArgs[nPos] = pCppCallStack;
            pUnoArgs[nPos] = pCppCallStack;
            switch (pParamTD->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppCallStack += sizeof(sal_Int32); // extra long
                break;
            default:
                break;
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE(pParamTD);
        }
        else // ptr to complex value | ref
        {
            pCppArgs[nPos] = *(void **)pCppCallStack;

            if (!rParam.bIn) // is pure out
            {
                // UNO out is unconstructed mem
                pUnoArgs[nPos] = alloca(pParamTD->nSize);
                pTempIndexes[nTempIndex] = nPos;
                // pParamTD will be released at reconversion
                ppTempParamTD[nTempIndex++] = pParamTD;
            }
            // is in/inout
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTD))
            {
                ::uno_copyAndConvertData(
                    pUnoArgs[nPos] = alloca(pParamTD->nSize),
                    *(void **)pCppCallStack, pParamTD,
                    pThis->getBridge()->getCpp2Uno());
                pTempIndexes[nTempIndex] = nPos; // has to be reconverted
                // pParamTD will be released at reconversion
                ppTempParamTD[nTempIndex++] = pParamTD;
            }
            else // direct way
            {
                pUnoArgs[nPos] = *(void **)pCppCallStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE(pParamTD);
            }
        }
        pCppCallStack += sizeof(sal_Int32); // standard parameter length
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke UNO dispatch call
    (*pThis->getUnoI()->pDispatcher)(
        pThis->getUnoI(), pMemberTD, pUnoReturn, pUnoArgs, &pUnoExc);

    // in case an exception occurred...
    if (pUnoExc)
    {
        // destruct temporary in/inout params
        while (nTempIndex--)
        {
            sal_Int32 nIndex = pTempIndexes[nTempIndex];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                ::uno_destructData(pUnoArgs[nIndex], ppTempParamTD[nTempIndex], nullptr);
            TYPELIB_DANGER_RELEASE(ppTempParamTD[nTempIndex]);
        }
        if (pReturnTD)
            TYPELIB_DANGER_RELEASE(pReturnTD);

        msvc_raiseException(
            &aUnoExc, pThis->getBridge()->getUno2Cpp()); // has to destruct the any

        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // no exception occurred...
    {
        // temporary params
        while (nTempIndex--)
        {
            sal_Int32 nIndex = pTempIndexes[nTempIndex];
            typelib_TypeDescription * pParamTD = ppTempParamTD[nTempIndex];

            if (pParams[nIndex].bOut) // inout/out
            {
                // convert and assign
                ::uno_destructData(pCppArgs[nIndex], pParamTD, uno::cpp_release);
                ::uno_copyAndConvertData(
                    pCppArgs[nIndex], pUnoArgs[nIndex], pParamTD,
                    pThis->getBridge()->getUno2Cpp());
            }
            // destroy temp UNO param
            ::uno_destructData(pUnoArgs[nIndex], pParamTD, nullptr);

            TYPELIB_DANGER_RELEASE(pParamTD);
        }

        // return
        if (pCppReturn) // has complex return
        {
            if (pUnoReturn != pCppReturn) // needs reconversion
            {
                ::uno_copyAndConvertData(
                    pCppReturn, pUnoReturn, pReturnTD,
                    pThis->getBridge()->getUno2Cpp());
                // destroy temp UNO return
                ::uno_destructData(pUnoReturn, pReturnTD, nullptr);
            }
            // complex return ptr is set to eax
            *(void **)pRegisterReturn = pCppReturn;
        }

        if (pReturnTD)
        {
            typelib_TypeClass eRet = pReturnTD->eTypeClass;
            TYPELIB_DANGER_RELEASE(pReturnTD);
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}

static typelib_TypeClass __cdecl cpp_mediate(
    void ** pCallStack,
    sal_Int32 nFunctionIndex,
    sal_Int32 nVtableOffset,
    sal_Int64 * pRegisterReturn) // space for register return
{
    static_assert(sizeof(sal_Int32)==sizeof(void *), "### unexpected!");

    // pCallStack: ret adr, this, [ret *], params
    // pRegisterReturn is either the direct "return value", or is set to
    // "ret *" and the real return value is constructed at that pointer.

    void* pThis = static_cast<char*>(pCallStack[1]) - nVtableOffset;
    bridges::cpp_uno::shared::CppInterfaceProxy* pCppI
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(pThis);

    typelib_InterfaceTypeDescription * pInterfaceTD = pCppI->getTypeDescr();

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

    SAL_INFO( "bridges", "Calling " << OUString::unacquired(&aMemberDescr.get()->pTypeName) );

    typelib_TypeClass eRet = typelib_TypeClass_VOID;
    switch (aMemberDescr.get()->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference *pAttrTypeRef
                = reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>(aMemberDescr.get())
                      ->pAttributeTypeRef;

            if (pInterfaceTD->pMapMemberIndexToFunctionIndex[nMemberPos] == nFunctionIndex)
            {
                // is GET method
                eRet = cpp2uno_call(pCppI, aMemberDescr.get(), pAttrTypeRef, 0, nullptr, pCallStack,
                                    pRegisterReturn);
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn = true;
                aParam.bOut = false;

                eRet = cpp2uno_call(pCppI, aMemberDescr.get(), nullptr, 1, &aParam, pCallStack,
                                    pRegisterReturn);
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
                    typelib_TypeDescription* pQueryTD = nullptr;
                    TYPELIB_DANGER_GET(&pQueryTD,
                                       static_cast<uno::Type*>(pCallStack[3])->getTypeLibType());
                    if (pQueryTD)
                    {
                        uno::XInterface* pInterface = nullptr;

                        pCppI->getBridge()->getCppEnv()->getRegisteredInterface(
                            pCppI->getBridge()->getCppEnv(), reinterpret_cast<void**>(&pInterface),
                            pCppI->getOid().pData,
                            reinterpret_cast<typelib_InterfaceTypeDescription*>(pQueryTD));

                        if (pInterface)
                        {
                            ::uno_any_construct(static_cast<uno_Any*>(pCallStack[2]), &pInterface,
                                                pQueryTD, uno::cpp_acquire);
                            pInterface->release();
                            TYPELIB_DANGER_RELEASE(pQueryTD);

                            *(void **)pRegisterReturn = pCallStack[2];
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
                                        pRegisterReturn);
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

/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static __declspec(naked) void __cdecl cpp_vtable_call()
{
__asm
    {
        sub     esp, 8      // space for immediate return type
        push    esp
        push    edx         // vtable offset
        push    eax         // function index
        mov     eax, esp
        add     eax, 20
        push    eax         // original stack ptr

        call    cpp_mediate
        add     esp, 16

        cmp     eax, typelib_TypeClass_FLOAT
        je      Lfloat
        cmp     eax, typelib_TypeClass_DOUBLE
        je      Ldouble
        cmp     eax, typelib_TypeClass_HYPER
        je      Lhyper
        cmp     eax, typelib_TypeClass_UNSIGNED_HYPER
        je      Lhyper
        // rest is eax
        pop     eax
        add     esp, 4
        ret
Lhyper:
        pop     eax
        pop     edx
        ret
Lfloat:
        fld     dword ptr [esp]
        add     esp, 8
        ret
Ldouble:
        fld     qword ptr [esp]
        add     esp, 8
        ret
    }
}

int const codeSnippetSize = 16;

unsigned char * codeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset)
{
    unsigned char * p = code;
    static_assert(sizeof (sal_Int32) == 4, "boo");
    // mov eax, functionIndex:
    *p++ = 0xB8;
    *reinterpret_cast< sal_Int32 * >(p) = functionIndex;
    p += sizeof (sal_Int32);
    // mov edx, vtableOffset:
    *p++ = 0xBA;
    *reinterpret_cast< sal_Int32 * >(p) = vtableOffset;
    p += sizeof (sal_Int32);
    // jmp rel32 cpp_vtable_call:
    *p++ = 0xE9;
    *reinterpret_cast< sal_Int32 * >(p)
        = ((unsigned char *) cpp_vtable_call) - p - sizeof (sal_Int32);
    p += sizeof (sal_Int32);
    assert(p - code <= codeSnippetSize);
    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block)
{
    return static_cast< Slot * >(block) + 1;
}

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 1) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
{
    struct Rtti {
        sal_Int32 n0, n1, n2;
        type_info * rtti;
        Rtti():
            n0(0), n1(0), n2(0),
            rtti(RTTInfos::get("com.sun.star.uno.XInterface"))
        {}
    };
    static Rtti rtti;

    Slot * slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
    typelib_InterfaceTypeDescription const *, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i < functionCount; ++i) {
        (s++)->fn = code;
        code = codeSnippet(code, functionOffset++, vtableOffset);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *,
    unsigned char const *)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
