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

#include <cxxabi.h>

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
    unsigned long value, unsigned long * stack, sal_Int32 * sp,
    unsigned long * regs, sal_Int32 * nregs)
{
    (*nregs != 8 ? regs[(*nregs)++] : stack[(*sp)++]) = value;
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
                    *static_cast<sal_Bool *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_BYTE:
                pushArgument(
                    *static_cast<sal_Int8 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_SHORT:
                pushArgument(
                    *static_cast<sal_Int16 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_UNSIGNED_SHORT:
                pushArgument(
                    *static_cast<sal_uInt16 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_ENUM:
                pushArgument(
                    *static_cast<sal_Int32 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_UNSIGNED_LONG:
                pushArgument(
                    *static_cast<sal_uInt32 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_HYPER:
                pushArgument(
                    *static_cast<sal_Int64 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_UNSIGNED_HYPER:
                pushArgument(
                    *static_cast<sal_uInt64 *>(arguments[i]), stack, &sp, gpr,
                    &ngpr);
                break;
            case typelib_TypeClass_FLOAT:
                pushArgument(
                    *static_cast<unsigned int *>(arguments[i]), stack, &sp, fpr,
                    &nfpr);
                break;
            case typelib_TypeClass_DOUBLE:
                pushArgument(
                    *static_cast<unsigned long *>(arguments[i]), stack, &sp,
                    fpr, &nfpr);
                break;
            case typelib_TypeClass_CHAR:
                pushArgument(
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
                    reinterpret_cast<unsigned long>(cppArgs[i]), stack, &sp,
                    gpr, &ngpr);
            } else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd)) {
                cppArgs[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(
                    cppArgs[i], arguments[i], ptd,
                    proxy->getBridge()->getUno2Cpp());
                ptds[i] = ptd;
                pushArgument(
                    reinterpret_cast<unsigned long>(cppArgs[i]), stack, &sp,
                    gpr, &ngpr);
            } else {
                cppArgs[i] = 0;
                pushArgument(
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
        abi_aarch64::mapException(
            reinterpret_cast<abi_aarch64::__cxa_eh_globals *>(
                __cxxabiv1::__cxa_get_globals())->caughtExceptions,
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
            // fall through
        case 12:
            std::memcpy(static_cast<char *>(ret) + 8, fpr + 2, 4);
            // fall through
        case 8:
            std::memcpy(static_cast<char *>(ret) + 4, fpr + 1, 4);
            // fall through
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

namespace bridges { namespace cpp_uno { namespace shared {

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
                // fall through
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

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
