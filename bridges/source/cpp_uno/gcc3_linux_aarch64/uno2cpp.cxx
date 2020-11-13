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
#include <cstring>
#include <exception>
#include <typeinfo>

#include <bridge.hxx>
#include <types.hxx>
#include <unointerfaceproxy.hxx>
#include <vtables.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/genfunc.hxx>
#include <rtl/textenc.h>
#include <rtl/ustring.hxx>
#include <sal/alloca.h>
#include <sal/types.h>
#include <typelib/typeclass.h>
#include <typelib/typedescription.h>
#include <uno/any2.h>
#include <uno/data.h>

#include "abi.hxx"
#include "callvirtualfunction.hxx"

namespace {

void pushArgument(
#ifdef MACOSX
    typelib_TypeClass typeclass,
    sal_Int32 * const subsp,
#endif
    unsigned long value, unsigned long * const stack, sal_Int32 * const sp,
    unsigned long * const regs, sal_Int32 * const nregs)
{
#ifdef MACOSX
    if (*nregs != 8)
    {
        regs[(*nregs)++] = value;
    }
    else
    {
        switch (typeclass) {
        case typelib_TypeClass_BOOLEAN:
        case typelib_TypeClass_BYTE:
            *reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(stack + *sp) + *subsp) = value;
            (*subsp) += 1;
            if (*subsp == 8)
            {
                (*sp)++;
                *subsp = 0;
            }
            break;
        case typelib_TypeClass_SHORT:
        case typelib_TypeClass_UNSIGNED_SHORT:
        case typelib_TypeClass_CHAR:
            *subsp = (*subsp + 1) & ~0x1;
            if (*subsp == 8)
            {
                (*sp)++;
                *subsp = 0;
            }
            *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(stack + *sp) + *subsp) = value;
            (*subsp) += 2;
            if (*subsp == 8)
            {
                (*sp)++;
                *subsp = 0;
            }
            break;
        case typelib_TypeClass_LONG:
        case typelib_TypeClass_UNSIGNED_LONG:
        case typelib_TypeClass_ENUM:
        case typelib_TypeClass_FLOAT:
            *subsp = (*subsp + 3) & ~0x3;
            if (*subsp == 8)
            {
                (*sp)++;
                *subsp = 0;
            }
            *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(stack + *sp) + *subsp) = value;
            (*subsp) += 4;
            if (*subsp == 8)
            {
                (*sp)++;
                *subsp = 0;
            }
            break;
        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
        default:
            if (*subsp > 0)
            {
                (*sp)++;
                *subsp = 0;
            }
            stack[*sp] = value;
            (*sp)++;
            break;
        }
    }
#else
    (*nregs != 8 ? regs[(*nregs)++] : stack[(*sp)++]) = value;
#endif
}

void call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * proxy,
    bridges::cpp_uno::shared::VtableSlot slot,
    typelib_TypeDescriptionReference * returnType, sal_Int32 count,
    typelib_MethodParameter * parameters, void * returnValue, void ** arguments,
    uno_Any ** exception)
{
    typelib_TypeDescription * rtd = 0;
    TYPELIB_DANGER_GET(&rtd, returnType);
    abi_aarch64::ReturnKind retKind = abi_aarch64::getReturnKind(rtd);
    bool retConv = bridges::cpp_uno::shared::relatesToInterfaceType(rtd);
    void * ret = retConv ? alloca(rtd->nSize) : returnValue;
    unsigned long ** thisPtr
        = reinterpret_cast<unsigned long **>(proxy->getCppI()) + slot.offset;
    unsigned long * stack = static_cast<unsigned long *>(
        alloca(count * sizeof (unsigned long)));
    sal_Int32 sp = 0;
#ifdef MACOSX
    sal_Int32 subsp = 0;
#endif
    unsigned long gpr[8];
    sal_Int32 ngpr = 0;
    unsigned long fpr[8];
    sal_Int32 nfpr = 0;
    gpr[ngpr++] = reinterpret_cast<unsigned long>(thisPtr);
    void ** cppArgs = static_cast<void **>(alloca(count * sizeof (void *)));
    typelib_TypeDescription ** ptds =
        static_cast<typelib_TypeDescription **>(
            alloca(count * sizeof (typelib_TypeDescription *)));
    for (sal_Int32 i = 0; i != count; ++i) {
        if (!parameters[i].bOut &&
            bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            cppArgs[i] = 0;
            switch (parameters[i].pTypeRef->eTypeClass) {
            case typelib_TypeClass_BOOLEAN:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_Bool *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_BYTE:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_Int8 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_SHORT:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_Int16 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_UNSIGNED_SHORT:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_uInt16 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_ENUM:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_Int32 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_UNSIGNED_LONG:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_uInt32 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_HYPER:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_Int64 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_UNSIGNED_HYPER:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_uInt64 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_FLOAT:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<unsigned int *>(arguments[i]), stack, &sp, fpr,
                    &nfpr);
                break;
            case typelib_TypeClass_DOUBLE:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<unsigned long *>(arguments[i]), stack, &sp,
                    fpr, &nfpr);
                break;
            case typelib_TypeClass_CHAR:
                pushArgument(
#ifdef MACOSX
                    parameters[i].pTypeRef->eTypeClass, &subsp,
#endif
                    *static_cast<sal_Unicode *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            default:
                assert(false);
            }
        } else {
            typelib_TypeDescription * ptd = 0;
            TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
            if (!parameters[i].bIn) {
                cppArgs[i] = alloca(ptd->nSize);
                uno_constructData(cppArgs[i], ptd);
                ptds[i] = ptd;
                pushArgument(
#ifdef MACOSX
                    typelib_TypeClass_HYPER, &subsp,
#endif
                    reinterpret_cast<unsigned long>(cppArgs[i]), stack, &sp,
                    gpr, &ngpr);
            } else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd)) {
                cppArgs[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(
                    cppArgs[i], arguments[i], ptd,
                    proxy->getBridge()->getUno2Cpp());
                ptds[i] = ptd;
                pushArgument(
#ifdef MACOSX
                    typelib_TypeClass_HYPER, &subsp,
#endif
                    reinterpret_cast<unsigned long>(cppArgs[i]), stack, &sp,
                    gpr, &ngpr);
            } else {
                cppArgs[i] = 0;
                pushArgument(
#ifdef MACOSX
                    typelib_TypeClass_HYPER, &subsp,
#endif
                    reinterpret_cast<unsigned long>(arguments[i]), stack, &sp,
                    gpr, &ngpr);
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
    }
    try {
        try {
            callVirtualFunction(
                (*thisPtr)[slot.index], gpr, fpr, stack, sp, ret);
        } catch (css::uno::Exception &) {
            throw;
        } catch (std::exception & e) {
            throw css::uno::RuntimeException(
                "C++ code threw "
                + OStringToOUString(typeid(e).name(), RTL_TEXTENCODING_UTF8)
                + ": " + OStringToOUString(e.what(), RTL_TEXTENCODING_UTF8));
        } catch (...) {
            throw css::uno::RuntimeException(
                "C++ code threw unknown exception");
        }
    } catch (css::uno::Exception &) {
        __cxxabiv1::__cxa_exception * header = reinterpret_cast<__cxxabiv1::__cxa_eh_globals *>(
            __cxxabiv1::__cxa_get_globals())->caughtExceptions;
#if !defined MACOSX && defined _LIBCPPABI_VERSION // detect libc++abi
        // Very bad HACK to find out whether we run against a libcxxabi that has a new
        // __cxa_exception::reserved member at the start, introduced with LLVM 10
        // <https://github.com/llvm/llvm-project/commit/674ec1eb16678b8addc02a4b0534ab383d22fa77>
        // "[libcxxabi] Insert padding in __cxa_exception struct for compatibility".  The layout of
        // the start of __cxa_exception is
        //
        //  [8 byte  void *reserve]
        //   8 byte  size_t referenceCount
        //
        // where the (bad, hacky) assumption is that reserve (if present) is null
        // (__cxa_allocate_exception in at least LLVM 11 zero-fills the object, and nothing actively
        // sets reserve) while referenceCount is non-null (__cxa_throw sets it to 1, and
        // __cxa_decrement_exception_refcount destroys the exception as soon as it drops to 0; for a
        // __cxa_dependent_exception, the referenceCount member is rather
        //
        //   8 byte  void* primaryException
        //
        // but which also will always be set to a non-null value in
        // __cxa_rethrow_primary_exception).  As described in the definition of __cxa_exception
        // (bridges/source/cpp_uno/gcc3_linux_aarch64/abi.hxx), this hack (together with the
        // "#ifdef MACOSX" there) can be dropped once we can be sure that we only run against new
        // libcxxabi that has the reserve member:
        if (*reinterpret_cast<void **>(header) == nullptr) {
            header = reinterpret_cast<__cxxabiv1::__cxa_exception*>(
                reinterpret_cast<void **>(header) + 1);
        }
#endif
        abi_aarch64::mapException(
            header,
            __cxxabiv1::__cxa_current_exception_type(), *exception,
            proxy->getBridge()->getCpp2Uno());
        for (sal_Int32 i = 0; i != count; ++i) {
            if (cppArgs[i] != 0) {
                uno_destructData(
                    cppArgs[i], ptds[i],
                    reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
                TYPELIB_DANGER_RELEASE(ptds[i]);
            }
        }
        TYPELIB_DANGER_RELEASE(rtd);
        return;
    }
    *exception = 0;
    for (sal_Int32 i = 0; i != count; ++i) {
        if (cppArgs[i] != 0) {
            if (parameters[i].bOut) {
                if (parameters[i].bIn) {
                    uno_destructData(arguments[i], ptds[i], 0);
                }
                uno_copyAndConvertData(
                    arguments[i], cppArgs[i], ptds[i],
                    proxy->getBridge()->getCpp2Uno());
            }
            uno_destructData(
                cppArgs[i], ptds[i],
                reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
            TYPELIB_DANGER_RELEASE(ptds[i]);
        }
    }
    switch (retKind) {
    case abi_aarch64::RETURN_KIND_REG:
        switch (rtd->eTypeClass) {
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
        case typelib_TypeClass_STRUCT:
            std::memcpy(ret, gpr, rtd->nSize);
            break;
        case typelib_TypeClass_FLOAT:
        case typelib_TypeClass_DOUBLE:
            std::memcpy(ret, fpr, rtd->nSize);
            break;
        default:
            assert(false);
        }
        break;
    case abi_aarch64::RETURN_KIND_HFA_FLOAT:
        switch (rtd->nSize) {
        case 16:
            std::memcpy(static_cast<char *>(ret) + 12, fpr + 3, 4);
            [[fallthrough]];
        case 12:
            std::memcpy(static_cast<char *>(ret) + 8, fpr + 2, 4);
            [[fallthrough]];
        case 8:
            std::memcpy(static_cast<char *>(ret) + 4, fpr + 1, 4);
            [[fallthrough]];
        case 4:
            std::memcpy(ret, fpr, 4);
            break;
        default:
            assert(false);
        }
        break;
    case abi_aarch64::RETURN_KIND_HFA_DOUBLE:
        std::memcpy(ret, fpr, rtd->nSize);
        break;
    case abi_aarch64::RETURN_KIND_INDIRECT:
        break;
    }
    if (retConv) {
        uno_copyAndConvertData(
            returnValue, ret, rtd, proxy->getBridge()->getCpp2Uno());
        uno_destructData(
            ret, rtd, reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
    }
    TYPELIB_DANGER_RELEASE(rtd);
}

}

namespace bridges::cpp_uno::shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
    void * pReturn, void ** pArgs, uno_Any ** ppException)
{
    UnoInterfaceProxy * proxy = static_cast<UnoInterfaceProxy *>(pUnoI);
    switch (pMemberDescr->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_InterfaceAttributeTypeDescription const * atd
                = reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const *>(
                        pMemberDescr);
            VtableSlot slot(getVtableSlot(atd));
            if (pReturn != 0) { // getter
                call(
                    proxy, slot, atd->pAttributeTypeRef, 0, 0, pReturn, pArgs,
                    ppException);
            } else { // setter
                typelib_MethodParameter param = {
                    0, atd->pAttributeTypeRef, true, false };
                typelib_TypeDescriptionReference * rtd = 0;
                typelib_typedescriptionreference_new(
                    &rtd, typelib_TypeClass_VOID, OUString("void").pData);
                slot.index += 1;
                call(proxy, slot, rtd, 1, &param, pReturn, pArgs, ppException);
                typelib_typedescriptionreference_release(rtd);
            }
            break;
        }
    case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription const * mtd
                = reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const *>(
                        pMemberDescr);
            VtableSlot slot(getVtableSlot(mtd));
            switch (slot.index) {
            case 1:
                pUnoI->acquire(pUnoI);
                *ppException = 0;
                break;
            case 2:
                pUnoI->release(pUnoI);
                *ppException = 0;
                break;
            case 0:
                {
                    typelib_TypeDescription * td = 0;
                    TYPELIB_DANGER_GET(
                        &td,
                        (reinterpret_cast<css::uno::Type *>(pArgs[0])
                         ->getTypeLibType()));
                    if (td != 0) {
                        uno_Interface * ifc = 0;
                        proxy->pBridge->getUnoEnv()->getRegisteredInterface(
                            proxy->pBridge->getUnoEnv(),
                            reinterpret_cast<void **>(&ifc), proxy->oid.pData,
                            reinterpret_cast<
                                typelib_InterfaceTypeDescription *>(td));
                        if (ifc != 0) {
                            uno_any_construct(
                                reinterpret_cast<uno_Any *>(pReturn), &ifc, td,
                                0);
                            ifc->release(ifc);
                            TYPELIB_DANGER_RELEASE(td);
                            *ppException = 0;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(td);
                    }
                }
                [[fallthrough]];
            default:
                call(
                    proxy, slot, mtd->pReturnTypeRef, mtd->nParams,
                    mtd->pParams, pReturn, pArgs, ppException);
                break;
            }
            break;
        }
    default:
        assert(false);
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
