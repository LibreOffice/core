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
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include <dlfcn.h>

#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <sal/alloca.h>
#include <sal/types.h>
#include <typelib/typeclass.h>
#include <typelib/typedescription.h>
#include <typelib/typedescription.hxx>

#include <bridge.hxx>
#include <cppinterfaceproxy.hxx>
#include <types.hxx>
#include <vtablefactory.hxx>

#include "abi.hxx"
#include "vtablecall.hxx"

namespace {

void call(
    bridges::cpp_uno::shared::CppInterfaceProxy * proxy,
    css::uno::TypeDescription const & description,
    typelib_TypeDescriptionReference * returnType, sal_Int32 count,
    typelib_MethodParameter * parameters, unsigned long * gpr,
    unsigned long * fpr, unsigned long * stack, void * indirectRet)
{
    typelib_TypeDescription * rtd = nullptr;
    if (returnType != nullptr) {
        TYPELIB_DANGER_GET(&rtd, returnType);
    }
    abi_aarch64::ReturnKind retKind = rtd == nullptr
        ? abi_aarch64::RETURN_KIND_REG : abi_aarch64::getReturnKind(rtd);
    bool retConv = rtd != nullptr
        && bridges::cpp_uno::shared::relatesToInterfaceType(rtd);
    void * retin = retKind == abi_aarch64::RETURN_KIND_INDIRECT && !retConv
        ? indirectRet : rtd == nullptr ? nullptr : alloca(rtd->nSize);
    void ** args = static_cast< void ** >(alloca(count * sizeof (void *)));
    void ** cppArgs = static_cast< void ** >(alloca(count * sizeof (void *)));
    typelib_TypeDescription ** argtds = static_cast<typelib_TypeDescription **>(
        alloca(count * sizeof (typelib_TypeDescription *)));
    sal_Int32 ngpr = 1;
    sal_Int32 nfpr = 0;
    sal_Int32 sp = 0;
#ifdef MACOSX
    sal_Int32 subsp = 0;
#endif
    for (sal_Int32 i = 0; i != count; ++i) {
        if (!parameters[i].bOut
            && bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            switch (parameters[i].pTypeRef->eTypeClass) {
#ifdef MACOSX
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                if (ngpr < 8)
                {
                    args[i] = gpr + ngpr;
                    ngpr++;
                }
                else
                {
                    args[i] = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(stack + sp) + subsp);
                    subsp += 1;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                }
                break;
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
            case typelib_TypeClass_CHAR:
                if (ngpr < 8)
                {
                    args[i] = gpr + ngpr;
                    ngpr++;
                }
                else
                {
                    subsp = (subsp + 1) & ~0x1;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                    args[i] = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(stack + sp) + subsp);
                    subsp += 2;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                }
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_ENUM:
                if (ngpr < 8)
                {
                    args[i] = gpr + ngpr;
                    ngpr++;
                }
                else
                {
                    subsp = (subsp + 3) & ~0x3;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                    args[i] = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(stack + sp) + subsp);
                    subsp += 4;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                }
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
                if (ngpr < 8)
                {
                    args[i] = gpr + ngpr;
                    ngpr++;
                }
                else
                {
                    if (subsp > 0)
                    {
                        sp++;
                        subsp = 0;
                    }
                    args[i] = stack + sp;
                    sp++;
                }
                break;
            case typelib_TypeClass_FLOAT:
                if (nfpr < 8)
                {
                    args[i] = fpr + nfpr;
                    nfpr++;
                }
                else
                {
                    subsp = (subsp + 3) & ~0x3;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                    args[i] = reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(stack + sp) + subsp);
                    subsp += 4;
                    if (subsp == 8)
                    {
                        sp++;
                        subsp = 0;
                    }
                }
                break;
            case typelib_TypeClass_DOUBLE:
                if (nfpr < 8)
                {
                    args[i] = fpr + nfpr;
                    nfpr++;
                }
                else
                {
                    if (subsp > 0)
                    {
                        sp++;
                        subsp = 0;
                    }
                    args[i] = stack + sp;
                    sp++;
                }
                break;
#else
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_UNSIGNED_LONG:
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_ENUM:
                args[i] = ngpr == 8 ? stack + sp++ : gpr + ngpr++;
                break;
            case typelib_TypeClass_FLOAT:
            case typelib_TypeClass_DOUBLE:
                args[i] = nfpr == 8 ? stack + sp++ : fpr + nfpr++;
                break;
#endif
            default:
                assert(false);
            }
            argtds[i] = nullptr;
        } else {
#ifdef MACOSX
            if (subsp > 0)
            {
                sp++;
                subsp = 0;
            }
#endif
            cppArgs[i] = reinterpret_cast<void *>(
                ngpr == 8 ? stack[sp++] : gpr[ngpr++]);
            typelib_TypeDescription * ptd = nullptr;
            TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
            if (!parameters[i].bIn) {
                args[i] = alloca(ptd->nSize);
                argtds[i] = ptd;
            } else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd)) {
                args[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(
                    args[i], cppArgs[i], ptd, proxy->getBridge()->getCpp2Uno());
                argtds[i] = ptd;
            } else {
                args[i] = cppArgs[i];
                argtds[i] = nullptr;
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
    }
    uno_Any exc;
    uno_Any * pexc = &exc;
    proxy->getUnoI()->pDispatcher(
        proxy->getUnoI(), description.get(), retin, args, &pexc);
    if (pexc != nullptr) {
        for (sal_Int32 i = 0; i != count; ++i) {
            if (argtds[i] != nullptr) {
                if (parameters[i].bIn) {
                    uno_destructData(args[i], argtds[i], nullptr);
                }
                TYPELIB_DANGER_RELEASE(argtds[i]);
            }
        }
        if (rtd != nullptr) {
            TYPELIB_DANGER_RELEASE(rtd);
        }
        abi_aarch64::raiseException(&exc, proxy->getBridge()->getUno2Cpp());
    }
    for (sal_Int32 i = 0; i != count; ++i) {
        if (argtds[i] != nullptr) {
            if (parameters[i].bOut) {
                uno_destructData(
                    cppArgs[i], argtds[i],
                    reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
                uno_copyAndConvertData(
                    cppArgs[i], args[i], argtds[i],
                    proxy->getBridge()->getUno2Cpp());
            }
            uno_destructData(args[i], argtds[i], nullptr);
            TYPELIB_DANGER_RELEASE(argtds[i]);
        }
    }
    void * retout = nullptr; // avoid false -Werror=maybe-uninitialized
    switch (retKind) {
    case abi_aarch64::RETURN_KIND_REG:
        switch (rtd == nullptr ? typelib_TypeClass_VOID : rtd->eTypeClass) {
        case typelib_TypeClass_VOID:
            break;
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_CHAR:
        case typelib_TypeClass_ENUM:
            std::memcpy(gpr, retin, rtd->nSize);
            assert(!retConv);
            break;
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
            std::memcpy(fpr, retin, rtd->nSize);
            assert(!retConv);
            break;
        case typelib_TypeClass_STRUCT:
            if (retConv) {
                retout = gpr;
            } else {
                std::memcpy(gpr, retin, rtd->nSize);
            }
            break;
        default:
            assert(false);
        }
        break;
    case abi_aarch64::RETURN_KIND_HFA_FLOAT:
        assert(rtd != nullptr);
        switch (rtd->nSize) {
        case 16:
            std::memcpy(fpr + 3, static_cast<char *>(retin) + 12, 4);
            [[fallthrough]];
        case 12:
            std::memcpy(fpr + 2, static_cast<char *>(retin) + 8, 4);
            [[fallthrough]];
        case 8:
            std::memcpy(fpr + 1, static_cast<char *>(retin) + 4, 4);
            [[fallthrough]];
        case 4:
            std::memcpy(fpr, retin, 4);
            break;
        default:
            assert(false);
        }
        assert(!retConv);
        break;
    case abi_aarch64::RETURN_KIND_HFA_DOUBLE:
        assert(rtd != nullptr);
        std::memcpy(fpr, retin, rtd->nSize);
        assert(!retConv);
        break;
    case abi_aarch64::RETURN_KIND_INDIRECT:
        retout = indirectRet;
        break;
    }
    if (retConv) {
        uno_copyAndConvertData(
            retout, retin, rtd, proxy->getBridge()->getUno2Cpp());
        uno_destructData(retin, rtd, nullptr);
    }
    if (rtd != nullptr) {
        TYPELIB_DANGER_RELEASE(rtd);
    }
}

}

void vtableCall(
    sal_Int32 functionIndex, sal_Int32 vtableOffset,
    unsigned long * gpr, unsigned long * fpr, unsigned long  * stack,
    void * indirectRet)
{
    bridges::cpp_uno::shared::CppInterfaceProxy * proxy
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            reinterpret_cast<char *>(gpr[0]) - vtableOffset);
    typelib_InterfaceTypeDescription * type = proxy->getTypeDescr();
    assert(functionIndex < type->nMapFunctionIndexToMemberIndex);
    sal_Int32 pos = type->pMapFunctionIndexToMemberIndex[functionIndex];
    css::uno::TypeDescription desc(type->ppAllMembers[pos]);
    switch (desc.get()->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        if (type->pMapMemberIndexToFunctionIndex[pos] == functionIndex) {
            // Getter:
            call(
                proxy, desc,
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>(
                    desc.get())->pAttributeTypeRef,
                0, nullptr, gpr, fpr, stack, indirectRet);
        } else {
            // Setter:
            typelib_MethodParameter param = {
                nullptr,
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription *>(
                    desc.get())->pAttributeTypeRef,
                true, false };
            call(proxy, desc, nullptr, 1, &param, gpr, fpr, stack, indirectRet);
        }
        break;
    case typelib_TypeClass_INTERFACE_METHOD:
        switch (functionIndex) {
        case 1:
            proxy->acquireProxy();
            break;
        case 2:
            proxy->releaseProxy();
            break;
        case 0:
            {
                typelib_TypeDescription * td = nullptr;
                TYPELIB_DANGER_GET(
                    &td,
                    (reinterpret_cast<css::uno::Type *>(gpr[1])
                     ->getTypeLibType()));
                if (td != nullptr && td->eTypeClass == typelib_TypeClass_INTERFACE) {
                    css::uno::XInterface * ifc = nullptr;
                    proxy->getBridge()->getCppEnv()->getRegisteredInterface(
                        proxy->getBridge()->getCppEnv(),
                        reinterpret_cast<void **>(&ifc), proxy->getOid().pData,
                        reinterpret_cast<typelib_InterfaceTypeDescription *>(
                            td));
                    if (ifc != nullptr) {
                        uno_any_construct(
                            static_cast<uno_Any *>(indirectRet), &ifc, td,
                            reinterpret_cast<uno_AcquireFunc>(
                                css::uno::cpp_acquire));
                        ifc->release();
                        TYPELIB_DANGER_RELEASE(td);
                        break;
                    }
                    TYPELIB_DANGER_RELEASE(td);
                }
            }
            [[fallthrough]];
        default:
            call(
                proxy, desc,
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(
                    desc.get())->pReturnTypeRef,
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(
                    desc.get())->nParams,
                reinterpret_cast<typelib_InterfaceMethodTypeDescription *>(
                    desc.get())->pParams,
                gpr, fpr, stack, indirectRet);
        }
        break;
    default:
        assert(false);
    }
}

namespace {

std::size_t const codeSnippetSize = 8 * 4;

unsigned char * generateCodeSnippet(
    unsigned char * code, sal_Int32 functionIndex, sal_Int32 vtableOffset)
{
    // movz x9, <low functionIndex>
    reinterpret_cast<unsigned int *>(code)[0] = 0xD2800009
        | ((functionIndex & 0xFFFF) << 5);
    // movk x9, <high functionIndex>, LSL #16
    reinterpret_cast<unsigned int *>(code)[1] = 0xF2A00009
        | ((functionIndex >> 16) << 5);
    // movz x10, <low vtableOffset>
    reinterpret_cast<unsigned int *>(code)[2] = 0xD280000A
        | ((vtableOffset & 0xFFFF) << 5);
    // movk x10, <high vtableOffset>, LSL #16
    reinterpret_cast<unsigned int *>(code)[3] = 0xF2A0000A
        | ((vtableOffset >> 16) << 5);
    // ldr x11, +2*4
    reinterpret_cast<unsigned int *>(code)[4] = 0x5800004B;
    // br x11
    reinterpret_cast<unsigned int *>(code)[5] = 0xD61F0160;
    reinterpret_cast<unsigned long *>(code)[3]
        = reinterpret_cast<unsigned long>(&vtableSlotCall);
    return code + codeSnippetSize;
}

}

struct bridges::cpp_uno::shared::VtableFactory::Slot { void * fn; };

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void * block) {
    return static_cast<Slot *>(block) + 2;
}

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(
    sal_Int32 slotCount)
{
    return (slotCount + 2) * sizeof (Slot) + slotCount * codeSnippetSize;
}

bridges::cpp_uno::shared::VtableFactory::Slot *
bridges::cpp_uno::shared::VtableFactory::initializeBlock(
    void * block, sal_Int32 slotCount, sal_Int32,
    typelib_InterfaceTypeDescription *)
{
    Slot * slots = mapBlockToVtable(block);
    slots[-2].fn = nullptr;
    slots[-1].fn = nullptr;
    return slots + slotCount;
}

unsigned char * bridges::cpp_uno::shared::VtableFactory::addLocalFunctions(
    Slot ** slots, unsigned char * code,
#ifdef USE_DOUBLE_MMAP
    sal_PtrDiff writetoexecdiff,
#endif
    typelib_InterfaceTypeDescription const * type, sal_Int32 functionOffset,
    sal_Int32 functionCount, sal_Int32 vtableOffset)
{
#ifndef USE_DOUBLE_MMAP
    constexpr sal_PtrDiff writetoexecdiff = 0;
#endif
    (*slots) -= functionCount;
    Slot * s = *slots;
    for (sal_Int32 i = 0; i != type->nMembers; ++i) {
        typelib_TypeDescription * td = nullptr;
        TYPELIB_DANGER_GET(&td, type->ppMembers[i]);
        assert(td != nullptr);
        switch (td->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            {
                typelib_InterfaceAttributeTypeDescription * atd
                    = reinterpret_cast<
                        typelib_InterfaceAttributeTypeDescription *>(td);
                // Getter:
                (s++)->fn = code + writetoexecdiff;
                code = generateCodeSnippet(
                    code, functionOffset++, vtableOffset);
                // Setter:
                if (!atd->bReadOnly) {
                    (s++)->fn = code + writetoexecdiff;
                    code = generateCodeSnippet(
                        code, functionOffset++, vtableOffset);
                }
                break;
            }
        case typelib_TypeClass_INTERFACE_METHOD:
            (s++)->fn = code + writetoexecdiff;
            code = generateCodeSnippet(code, functionOffset++, vtableOffset);
            break;
        default:
            assert(false);
        }
        TYPELIB_DANGER_RELEASE(td);
    }
    return code;
}

void bridges::cpp_uno::shared::VtableFactory::flushCode(
    unsigned char const * begin, unsigned char const * end)
{
#if !defined ANDROID && !defined MACOSX
   static void (*clear_cache)(unsigned char const *, unsigned char const *)
       = (void (*)(unsigned char const *, unsigned char const *)) dlsym(
           RTLD_DEFAULT, "__clear_cache");
   (*clear_cache)(begin, end);
#else
    // GCC clarified with
    // <http://gcc.gnu.org/git/?p=gcc.git;a=commit;h=a90b0cdd444f6dde1084a439862cf507f6d3b2ae>
    // "extend.texi (__clear_cache): Correct signature" that __builtin___clear_cache takes void*
    // parameters, while Clang uses char* ever since
    // <https://github.com/llvm/llvm-project/commit/c491a8d4577052bc6b3b4c72a7db6a7cfcbc2ed0> "Add
    // support for __builtin___clear_cache in Clang" (TODO: see
    // <https://bugs.llvm.org/show_bug.cgi?id=48489> "__builtin___clear_cache() has a different
    // prototype than GCC"; once fixed for our Clang baseline, we can drop the reinterpret_casts):
    __builtin___clear_cache(
        reinterpret_cast<char *>(const_cast<unsigned char *>(begin)),
        reinterpret_cast<char *>(const_cast<unsigned char *>(end)));
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
