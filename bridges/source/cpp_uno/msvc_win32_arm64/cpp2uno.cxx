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
#include <limits>
#include <typeinfo>

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

#include <msvc/arm64.hxx>

#include "abi.hxx"

extern "C" IMAGE_DOS_HEADER const __ImageBase;

extern "C" void vtableSlotCall();

using namespace ::com::sun::star;

namespace
{
void call(bridges::cpp_uno::shared::CppInterfaceProxy* proxy,
          uno::TypeDescription const& description, typelib_TypeDescriptionReference* returnType,
          sal_Int32 count, typelib_MethodParameter* parameters, sal_uInt64* gpr, sal_uInt64* fpr,
          sal_uInt64* stack, void* indirectRet)
{
    typelib_TypeDescription* rtd = 0;
    if (returnType != 0)
        TYPELIB_DANGER_GET(&rtd, returnType);

    ReturnKind retKind = rtd == 0 ? RETURN_KIND_REG : getReturnKind(rtd);
    bool retConv = rtd != 0 && bridges::cpp_uno::shared::relatesToInterfaceType(rtd);

    void* retin = retKind == RETURN_KIND_INDIRECT && !retConv ? indirectRet
                                                              : rtd == 0 ? 0 : alloca(rtd->nSize);
    void** args = static_cast<void**>(alloca(count * sizeof(void*)));
    void** cppArgs = static_cast<void**>(alloca(count * sizeof(void*)));
    typelib_TypeDescription** argtds
        = static_cast<typelib_TypeDescription**>(alloca(count * sizeof(typelib_TypeDescription*)));

    sal_Int32 ngpr = retKind == RETURN_KIND_INDIRECT ? 2 : 1;
    sal_Int32 nfpr = 0;
    sal_Int32 sp = 0;
    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (!parameters[i].bOut && bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            switch (parameters[i].pTypeRef->eTypeClass)
            {
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
                default:
                    assert(false);
            }
            argtds[i] = 0;
        }
        else
        {
            cppArgs[i] = reinterpret_cast<void*>(ngpr == 8 ? stack[sp++] : gpr[ngpr++]);
            typelib_TypeDescription* ptd = 0;
            TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
            if (!parameters[i].bIn)
            {
                args[i] = alloca(ptd->nSize);
                argtds[i] = ptd;
            }
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd))
            {
                args[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(args[i], cppArgs[i], ptd, proxy->getBridge()->getCpp2Uno());
                argtds[i] = ptd;
            }
            else
            {
                args[i] = cppArgs[i];
                argtds[i] = 0;
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
    }

    uno_Any exc;
    uno_Any* pexc = &exc;
    proxy->getUnoI()->pDispatcher(proxy->getUnoI(), description.get(), retin, args, &pexc);
    if (pexc != 0)
    {
        for (sal_Int32 i = 0; i != count; ++i)
        {
            if (argtds[i] == 0)
                continue;
            if (parameters[i].bIn)
                uno_destructData(args[i], argtds[i], 0);
            TYPELIB_DANGER_RELEASE(argtds[i]);
        }
        if (rtd != 0)
            TYPELIB_DANGER_RELEASE(rtd);
        assert(pexc == &exc);
        msvc_raiseException(&exc, proxy->getBridge()->getUno2Cpp());
    }

    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (argtds[i] != 0)
        {
            if (parameters[i].bOut)
            {
                uno_destructData(cppArgs[i], argtds[i],
                                 reinterpret_cast<uno_ReleaseFunc>(uno::cpp_release));
                uno_copyAndConvertData(cppArgs[i], args[i], argtds[i],
                                       proxy->getBridge()->getUno2Cpp());
            }
            uno_destructData(args[i], argtds[i], 0);
            TYPELIB_DANGER_RELEASE(argtds[i]);
        }
    }

    void* retout = 0; // avoid false -Werror=maybe-uninitialized
    switch (retKind)
    {
        case RETURN_KIND_REG:
            switch (rtd == 0 ? typelib_TypeClass_VOID : rtd->eTypeClass)
            {
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
                    if (retConv)
                    {
                        retout = gpr;
                    }
                    else
                    {
                        std::memcpy(gpr, retin, rtd->nSize);
                    }
                    break;
                default:
                    assert(false);
            }
            break;
        case RETURN_KIND_HFA_FLOAT:
            assert(rtd != 0);
            switch (rtd->nSize)
            {
                case 16:
                    std::memcpy(fpr + 3, static_cast<char*>(retin) + 12, 4);
                    [[fallthrough]];
                case 12:
                    std::memcpy(fpr + 2, static_cast<char*>(retin) + 8, 4);
                    [[fallthrough]];
                case 8:
                    std::memcpy(fpr + 1, static_cast<char*>(retin) + 4, 4);
                    [[fallthrough]];
                case 4:
                    std::memcpy(fpr, retin, 4);
                    break;
                default:
                    assert(false);
            }
            assert(!retConv);
            break;
        case RETURN_KIND_HFA_DOUBLE:
            assert(rtd != 0);
            std::memcpy(fpr, retin, rtd->nSize);
            assert(!retConv);
            break;
        case RETURN_KIND_INDIRECT:
            retout = indirectRet;
            gpr[0] = reinterpret_cast<sal_uInt64>(retout);
            break;
    }

    if (retConv)
    {
        uno_copyAndConvertData(retout, retin, rtd, proxy->getBridge()->getUno2Cpp());
        uno_destructData(retin, rtd, 0);
    }

    if (rtd != 0)
        TYPELIB_DANGER_RELEASE(rtd);
}

extern "C" void vtableCall(sal_Int32 functionIndex, sal_Int32 vtableOffset, sal_uInt64* gpr,
                           sal_uInt64* fpr, sal_uInt64* stack)
{
    bridges::cpp_uno::shared::CppInterfaceProxy* proxy
        = bridges::cpp_uno::shared::CppInterfaceProxy::castInterfaceToProxy(
            reinterpret_cast<char*>(gpr[0]) - vtableOffset);
    void* indirectRet = reinterpret_cast<void*>(gpr[1]);
    typelib_InterfaceTypeDescription* pInterfaceTD = proxy->getTypeDescr();
    assert(functionIndex < pInterfaceTD->nMapFunctionIndexToMemberIndex);
    sal_Int32 nMemberPos = pInterfaceTD->pMapFunctionIndexToMemberIndex[functionIndex];
    assert(nMemberPos < pInterfaceTD->nAllMembers);
    uno::TypeDescription aMemberDescr(pInterfaceTD->ppAllMembers[nMemberPos]);

    switch (aMemberDescr.get()->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_TypeDescriptionReference* pAttrTypeRef
                = reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(aMemberDescr.get())
                      ->pAttributeTypeRef;

            if (pInterfaceTD->pMapMemberIndexToFunctionIndex[nMemberPos] == functionIndex)
            {
                // Getter:
                call(proxy, aMemberDescr, pAttrTypeRef, 0, 0, gpr, fpr, stack, indirectRet);
            }
            else
            {
                // Setter:
                typelib_MethodParameter param = { 0, pAttrTypeRef, true, false };
                call(proxy, aMemberDescr, 0, 1, &param, gpr, fpr, stack, indirectRet);
            }
        }
        break;
        case typelib_TypeClass_INTERFACE_METHOD:
            switch (functionIndex)
            {
                case 1:
                    proxy->acquireProxy();
                    break;
                case 2:
                    proxy->releaseProxy();
                    break;
                case 0:
                {
                    typelib_TypeDescription* td = nullptr;
                    TYPELIB_DANGER_GET(&td,
                                       (reinterpret_cast<uno::Type*>(gpr[1])->getTypeLibType()));
                    if (td != 0 && td->eTypeClass == typelib_TypeClass_INTERFACE)
                    {
                        uno::XInterface* ifc = nullptr;
                        proxy->getBridge()->getCppEnv()->getRegisteredInterface(
                            proxy->getBridge()->getCppEnv(), reinterpret_cast<void**>(&ifc),
                            proxy->getOid().pData,
                            reinterpret_cast<typelib_InterfaceTypeDescription*>(td));
                        if (ifc != 0)
                        {
                            uno_any_construct(reinterpret_cast<uno_Any*>(indirectRet), &ifc, td,
                                              reinterpret_cast<uno_AcquireFunc>(uno::cpp_acquire));
                            ifc->release();
                            TYPELIB_DANGER_RELEASE(td);
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(td);
                    }
                }
                    [[fallthrough]];
                default:
                    typelib_InterfaceMethodTypeDescription* pMethodTD
                        = reinterpret_cast<typelib_InterfaceMethodTypeDescription*>(
                            aMemberDescr.get());
                    call(proxy, aMemberDescr, pMethodTD->pReturnTypeRef, pMethodTD->nParams,
                         pMethodTD->pParams, gpr, fpr, stack, indirectRet);
            }
            break;
        default:
            assert(false);
    }
}

std::size_t const codeSnippetSize = 8 * 4;

unsigned char* GenerateVTableSlotTrampoline(unsigned char* code, sal_Int32 functionIndex,
                                            sal_Int32 vtableOffset)
{
    // movz x9, <low functionIndex>
    reinterpret_cast<unsigned int*>(code)[0] = 0xD2800009 | ((functionIndex & 0xFFFF) << 5);
    // movk x9, <high functionIndex>, LSL #16
    reinterpret_cast<unsigned int*>(code)[1] = 0xF2A00009 | ((functionIndex >> 16) << 5);
    // movz x10, <low vtableOffset>
    reinterpret_cast<unsigned int*>(code)[2] = 0xD280000A | ((vtableOffset & 0xFFFF) << 5);
    // movk x10, <high vtableOffset>, LSL #16
    reinterpret_cast<unsigned int*>(code)[3] = 0xF2A0000A | ((vtableOffset >> 16) << 5);
    // ldr x11, +2*4
    reinterpret_cast<unsigned int*>(code)[4] = 0x5800004B;
    // br x11
    reinterpret_cast<unsigned int*>(code)[5] = 0xD61F0160;
    reinterpret_cast<void**>(code)[3] = reinterpret_cast<void*>(&vtableSlotCall);
    return code + codeSnippetSize;
}
}

namespace bridges::cpp_uno::shared
{
struct bridges::cpp_uno::shared::VtableFactory::Slot
{
    void* fn;
};

bridges::cpp_uno::shared::VtableFactory::Slot*
bridges::cpp_uno::shared::VtableFactory::mapBlockToVtable(void* block)
{
    return static_cast<Slot*>(block) + 1;
}

std::size_t bridges::cpp_uno::shared::VtableFactory::getBlockSize(sal_Int32 slotCount)
{
    return (slotCount + 1) * sizeof(Slot) + slotCount * codeSnippetSize;
}

static sal_uInt32 imageRelative(void const* p)
{
    assert(reinterpret_cast<sal_uIntPtr>(p) >= reinterpret_cast<sal_uIntPtr>(&__ImageBase)
           && reinterpret_cast<sal_uIntPtr>(p) - reinterpret_cast<sal_uIntPtr>(&__ImageBase)
                  <= std::numeric_limits<sal_uInt32>::max());
    return reinterpret_cast<sal_uIntPtr>(p) - reinterpret_cast<sal_uIntPtr>(&__ImageBase);
}

namespace
{
// Some dummy type whose RTTI is used in the synthesized proxy vtables to make uses of dynamic_cast
// on such proxy objects not crash:
struct ProxyRtti
{
};

// The following vtable RTTI data is based on how the code at
// <https://github.com/llvm/llvm-project/blob/main/clang/lib/CodeGen/MicrosoftCXXABI.cpp> computes
// such data, and on how <https://devblogs.microsoft.com/oldnewthing/20041025-00/?p=37483>
// "Accessing the current module’s HINSTANCE from a static library" obtians __ImageBase:

struct RttiClassHierarchyDescriptor;

#pragma warning(push)
#pragma warning(disable : 4324) // "structure was padded due to alignment specifier"

struct alignas(16) RttiBaseClassDescriptor
{
    sal_uInt32 n0 = imageRelative(&typeid(ProxyRtti));
    sal_uInt32 n1 = 0;
    sal_uInt32 n2 = 0;
    sal_uInt32 n3 = 0xFFFFFFFF;
    sal_uInt32 n4 = 0;
    sal_uInt32 n5 = 0x40;
    sal_uInt32 n6;
    RttiBaseClassDescriptor(RttiClassHierarchyDescriptor const* chd)
        : n6(imageRelative(chd))
    {
    }
};

struct alignas(4) RttiBaseClassArray
{
    sal_uInt32 n0;
    sal_uInt32 n1 = 0;
    RttiBaseClassArray(RttiBaseClassDescriptor const* bcd)
        : n0(imageRelative(bcd))
    {
    }
};

struct alignas(8) RttiClassHierarchyDescriptor
{
    sal_uInt32 n0 = 0;
    sal_uInt32 n1 = 0;
    sal_uInt32 n2 = 1;
    sal_uInt32 n3;
    RttiClassHierarchyDescriptor(RttiBaseClassArray const* bca)
        : n3(imageRelative(bca))
    {
    }
};

struct alignas(16) RttiCompleteObjectLocator
{
    sal_uInt32 n0 = 1;
    sal_uInt32 n1 = 0;
    sal_uInt32 n2 = 0;
    sal_uInt32 n3 = imageRelative(&typeid(ProxyRtti));
    sal_uInt32 n4;
    sal_uInt32 n5 = imageRelative(this);
    RttiCompleteObjectLocator(RttiClassHierarchyDescriptor const* chd)
        : n4(imageRelative(chd))
    {
    }
};

struct Rtti
{
    RttiBaseClassDescriptor bcd;
    RttiBaseClassArray bca;
    RttiClassHierarchyDescriptor chd;
    RttiCompleteObjectLocator col;
    Rtti()
        : bcd(&chd)
        , bca(&bcd)
        , chd(&bca)
        , col(&chd)
    {
    }
};

#pragma warning(pop)
}

bridges::cpp_uno::shared::VtableFactory::Slot*
bridges::cpp_uno::shared::VtableFactory::initializeBlock(void* block, sal_Int32 slotCount,
                                                         sal_Int32,
                                                         typelib_InterfaceTypeDescription*)
{
    static Rtti rtti;

    Slot* slots = mapBlockToVtable(block);
    slots[-1].fn = &rtti.col;
    return slots + slotCount;
}

unsigned char* VtableFactory::addLocalFunctions(VtableFactory::Slot** slots, unsigned char* code,
                                                typelib_InterfaceTypeDescription const* type,
                                                sal_Int32 functionOffset, sal_Int32 functionCount,
                                                sal_Int32 vtableOffset)
{
    (*slots) -= functionCount;
    VtableFactory::Slot* s = *slots;
    for (sal_Int32 i = 0; i != type->nMembers; ++i)
    {
        typelib_TypeDescription* td = nullptr;
        TYPELIB_DANGER_GET(&td, type->ppMembers[i]);
        assert(td != 0);
        switch (td->eTypeClass)
        {
            case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            {
                typelib_InterfaceAttributeTypeDescription* atd
                    = reinterpret_cast<typelib_InterfaceAttributeTypeDescription*>(td);
                // Getter:
                (s++)->fn = code;
                code = GenerateVTableSlotTrampoline(code, functionOffset++, vtableOffset);
                // Setter:
                if (!atd->bReadOnly)
                {
                    (s++)->fn = code;
                    code = GenerateVTableSlotTrampoline(code, functionOffset++, vtableOffset);
                }
                break;
            }
            case typelib_TypeClass_INTERFACE_METHOD:
                (s++)->fn = code;
                code = GenerateVTableSlotTrampoline(code, functionOffset++, vtableOffset);
                break;
            default:
                assert(false);
        }
        TYPELIB_DANGER_RELEASE(td);
    }
    return code;
}

void VtableFactory::flushCode(unsigned char const* begin, unsigned char const* end)
{
    FlushInstructionCache(GetCurrentProcess(), begin, end - begin);
}

} // namespace bridges::cpp_uno::shared

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
