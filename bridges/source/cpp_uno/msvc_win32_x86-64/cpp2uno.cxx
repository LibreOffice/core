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

#include <bridge.hxx>
#include <cppinterfaceproxy.hxx>
#include <types.hxx>
#include <vtablefactory.hxx>

#include "call.hxx"
#include <msvc/amd64.hxx>

using namespace ::com::sun::star;

static typelib_TypeClass cpp2uno_call(
    bridges::cpp_uno::shared::CppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTD,
    typelib_TypeDescriptionReference * pReturnTypeRef, // nullptr indicates void return
    sal_Int32 nParams,
    typelib_MethodParameter * pParams,
    void ** pCallStack)
{
    // return type
    typelib_TypeDescription * pReturnTD = nullptr;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET(&pReturnTD, pReturnTypeRef);

    int nFirstRealParam = 3;    // Index into pCallStack, past return
                                // value, return address and 'this'
                                // pointer.

    void * pUnoReturn = nullptr;
    void * pCppReturn = nullptr; // complex return ptr: if != nullptr && != pUnoReturn, reconversion need

    if (pReturnTD)
    {
        if (bridges::cpp_uno::shared::isSimpleType(pReturnTD))
            pUnoReturn = pCallStack;
        else
        {
            pCppReturn = pCallStack[nFirstRealParam++];
            pUnoReturn = (bridges::cpp_uno::shared::relatesToInterfaceType(pReturnTD)
                          ? alloca(pReturnTD->nSize) : pCppReturn);
        }
    }

    void ** pCppIncomingParams = pCallStack + nFirstRealParam;

    // Unlike this method for other archs, prefer clarity to
    // micro-optimization, and allocate these array separately

    // parameters passed to the UNO function
    void ** pUnoArgs = static_cast<void **>(alloca(sizeof(void *) * nParams));

    // parameters received from C++
    void ** pCppArgs = static_cast<void **>(alloca(sizeof(void *) * nParams));

    // indexes of values this have to be converted (interface conversion C++<=>UNO)
    sal_Int32 * pTempIndexes = static_cast<sal_Int32 *>(alloca(sizeof(sal_Int32) * nParams));

    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTD =
        static_cast<typelib_TypeDescription **>(alloca(sizeof(void *) * nParams));

    sal_Int32 nTempIndex = 0;

    for (sal_Int32 nPos = 0; nPos < nParams; ++nPos)
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTD = nullptr;
        TYPELIB_DANGER_GET(&pParamTD, rParam.pTypeRef);

        if (!rParam.bOut && bridges::cpp_uno::shared::isSimpleType(pParamTD))
        {
            pCppArgs[nPos] = pUnoArgs[nPos] = pCppIncomingParams++;
            TYPELIB_DANGER_RELEASE(pParamTD);
        }
        else // ptr to complex value | ref
        {
            void * pCppCallStack;

            pCppArgs[nPos] = pCppCallStack = *pCppIncomingParams++;

            if (!rParam.bIn) // is pure out
            {
                // UNO out is unconstructed mem
                pUnoArgs[nPos] = alloca(pParamTD->nSize);
                pTempIndexes[nTempIndex] = nPos;
                // pParamTD will be released at reconversion
                ppTempParamTD[nTempIndex++] = pParamTD;
            }
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(pParamTD))
            {
                ::uno_copyAndConvertData(
                    pUnoArgs[nPos] = alloca(pParamTD->nSize),
                    pCppCallStack, pParamTD,
                    pThis->getBridge()->getCpp2Uno());
                pTempIndexes[nTempIndex] = nPos; // has to be reconverted
                // pParamTD will be released at reconversion
                ppTempParamTD[nTempIndex++] = pParamTD;
            }
            else // direct way
            {
                pUnoArgs[nPos] = pCppCallStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE(pParamTD);
            }
        }
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
            pCallStack[0] = pCppReturn;
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

extern "C" typelib_TypeClass cpp_vtable_call(
    sal_Int64 nOffsetAndIndex,
    void ** pCallStack)
{
    sal_Int32 nFunctionIndex = (nOffsetAndIndex & 0xFFFFFFFF);
    sal_Int32 nVtableOffset = ((nOffsetAndIndex >> 32) & 0xFFFFFFFF);

    // pCallStack: ret value, ret adr, this, [ret *], params
    // pCallStack[0] is either the direct "return value", or is set to "ret *" and
    // the real return value is constructed at that memory.

    void* pThis = static_cast<char*>(pCallStack[2]) - nVtableOffset;
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
                eRet = cpp2uno_call(pCppI, aMemberDescr.get(), pAttrTypeRef, 0, nullptr, pCallStack);
            }
            else
            {
                // is SET method
                typelib_MethodParameter aParam;
                aParam.pTypeRef = pAttrTypeRef;
                aParam.bIn = true;
                aParam.bOut = false;

                eRet = cpp2uno_call(pCppI, aMemberDescr.get(), nullptr, 1, &aParam, pCallStack);
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            // is METHOD
            switch (nFunctionIndex)
            {
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
                                       static_cast<uno::Type*>(pCallStack[4])->getTypeLibType());

                    if (pQueryTD)
                    {
                        uno::XInterface* pInterface = nullptr;

                        pCppI->getBridge()->getCppEnv()->getRegisteredInterface(
                            pCppI->getBridge()->getCppEnv(), reinterpret_cast<void**>(&pInterface),
                            pCppI->getOid().pData,
                            reinterpret_cast<typelib_InterfaceTypeDescription*>(pQueryTD));

                        if (pInterface)
                        {
                            // pCallStack[3] = hidden return value pointer
                            ::uno_any_construct(static_cast<uno_Any*>(pCallStack[3]), &pInterface,
                                                pQueryTD, uno::cpp_acquire);
                            pInterface->release();
                            TYPELIB_DANGER_RELEASE(pQueryTD);

                            pCallStack[0] = pCallStack[3];
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
                                        pMethodTD->nParams, pMethodTD->pParams, pCallStack);
                }
            }
            break;
        }
        default:
            throw uno::RuntimeException("No member description found!",
                                        reinterpret_cast<uno::XInterface*>(pThis));
    }

    return eRet;
}

int const codeSnippetSize = 48;
typedef enum { REGPARAM_INT, REGPARAM_FLT } RegParamKind;

extern "C" char privateSnippetExecutor;

// This function generates the code that acts as a proxy for the UNO function to be called.
// The generated code does the following:
// - Spills register parameters on stack
// - Loads functionIndex and vtableOffset into scratch registers
// - Jumps to privateSnippetExecutor

static unsigned char * codeSnippet(
    unsigned char * code,
    RegParamKind param_kind[4],
    sal_Int32 nFunctionIndex,
    sal_Int32 nVtableOffset )
{
    sal_uInt64 nOffsetAndIndex = ( static_cast<sal_uInt64>(nVtableOffset) << 32 ) | static_cast<sal_uInt64>(nFunctionIndex);
    unsigned char *p = code;

    // Spill parameters
    if (param_kind[0] == REGPARAM_INT)
    {
        // mov qword ptr 8[rsp], rcx
        *p++ = 0x48; *p++ = 0x89; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x08;
    }
    else
    {
        // movsd qword ptr 8[rsp], xmm0
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x44; *p++ = 0x24; *p++ = 0x08;
    }
    if ( param_kind[1] == REGPARAM_INT )
    {
        // mov qword ptr 16[rsp], rdx
        *p++ = 0x48; *p++ = 0x89; *p++ = 0x54; *p++ = 0x24; *p++ = 0x10;
    }
    else
    {
        // movsd qword ptr 16[rsp], xmm1
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x10;
    }
    if ( param_kind[2] == REGPARAM_INT )
    {
        // mov qword ptr 24[rsp], r8
        *p++ = 0x4C; *p++ = 0x89; *p++ = 0x44; *p++ = 0x24; *p++ = 0x18;
    }
    else
    {
        // movsd qword ptr 24[rsp], xmm2
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x54; *p++ = 0x24; *p++ = 0x18;
    }
    if ( param_kind[3] == REGPARAM_INT )
    {
        // mov qword ptr 32[rsp], r9
        *p++ = 0x4C;*p++ = 0x89; *p++ = 0x4C; *p++ = 0x24; *p++ = 0x20;
    }
    else
    {
        // movsd qword ptr 32[rsp], xmm3
        *p++ = 0xF2; *p++ = 0x0F; *p++ = 0x11; *p++ = 0x5C; *p++ = 0x24; *p++ = 0x20;
    }

    // mov rcx, nOffsetAndIndex
    *p++ = 0x48; *p++ = 0xB9;
    *reinterpret_cast<sal_uInt64 *>(p) = nOffsetAndIndex; p += 8;

    // mov r11, privateSnippetExecutor
    *p++ = 0x49; *p++ = 0xBB;
    *reinterpret_cast<void **>(p) = &privateSnippetExecutor; p += 8;

    // jmp r11
    *p++ = 0x41; *p++ = 0xFF; *p++ = 0xE3;

    assert(p < code + codeSnippetSize);

    return code + codeSnippetSize;
}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(
    void * block )
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
    void * block,
    sal_Int32 slotCount,
    sal_Int32, typelib_InterfaceTypeDescription *)
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
    Slot ** slots,
    unsigned char * code,
    typelib_InterfaceTypeDescription const * type,
    sal_Int32 nFunctionOffset,
    sal_Int32 functionCount,
    sal_Int32 nVtableOffset )
{
    (*slots) -= functionCount;
    Slot * s = *slots;

    for (int member = 0; member < type->nMembers; ++member) {
        typelib_TypeDescription * pTD = nullptr;

        TYPELIB_DANGER_GET( &pTD, type->ppMembers[ member ] );
        assert(pTD);

        RegParamKind param_kind[4];
        int nr = 0;

        for (int i = 0; i < 4; ++i)
            param_kind[i] = REGPARAM_INT;

        // 'this'
        ++nr;

        if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE )
        {
            typelib_InterfaceAttributeTypeDescription * pIfaceAttrTD =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>( pTD );

            // Getter

            (s++)->fn = code;
            code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );
            if ( ! pIfaceAttrTD->bReadOnly )
            {
                typelib_TypeDescription * pAttrTD = nullptr;
                TYPELIB_DANGER_GET( &pAttrTD, pIfaceAttrTD->pAttributeTypeRef );
                assert(pAttrTD);

                // Setter
                if ( pAttrTD->eTypeClass == typelib_TypeClass_FLOAT ||
                     pAttrTD->eTypeClass == typelib_TypeClass_DOUBLE )
                    param_kind[nr++] = REGPARAM_FLT;

                TYPELIB_DANGER_RELEASE( pAttrTD );

                (s++)->fn = code;
                code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );
            }
        }
        else if ( pTD->eTypeClass == typelib_TypeClass_INTERFACE_METHOD )
        {
            typelib_InterfaceMethodTypeDescription * pMethodTD =
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>( pTD );

            typelib_TypeDescription * pReturnTD = nullptr;
            TYPELIB_DANGER_GET( &pReturnTD, pMethodTD->pReturnTypeRef );
            assert(pReturnTD);

            if ( !bridges::cpp_uno::shared::isSimpleType( pReturnTD ) )
            {
                // Return value
                ++nr;
            }

            for (int param = 0; nr < 4 && param < pMethodTD->nParams; ++param, ++nr)
            {
                typelib_TypeDescription * pParamTD = nullptr;

                TYPELIB_DANGER_GET( &pParamTD, pMethodTD->pParams[param].pTypeRef );
                assert(pParamTD);

                if ( pParamTD->eTypeClass == typelib_TypeClass_FLOAT ||
                     pParamTD->eTypeClass == typelib_TypeClass_DOUBLE )
                    param_kind[nr] = REGPARAM_FLT;

                TYPELIB_DANGER_RELEASE( pParamTD );
            }
            (s++)->fn = code;
            code = codeSnippet( code, param_kind, nFunctionOffset++, nVtableOffset );

            TYPELIB_DANGER_RELEASE( pReturnTD );
        }
        else
            assert(false);

        TYPELIB_DANGER_RELEASE( pTD );
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const *,
    unsigned char const *)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
