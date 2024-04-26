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
#include <msvc/arm64.hxx>

namespace
{
extern "C" void callVirtualFunction(sal_uInt64* regs, sal_uInt64* stack, sal_Int32 sp,
                                    sal_uInt64 function);

void pushArgument(sal_uInt64 value, sal_uInt64* stack, sal_Int32& sp, sal_uInt64* regs,
                  sal_Int32& nregs)
{
    (nregs != 8 ? regs[nregs++] : stack[sp++]) = value;
}

void call(bridges::cpp_uno::shared::UnoInterfaceProxy* pProxy,
          bridges::cpp_uno::shared::VtableSlot slot, typelib_TypeDescriptionReference* returnType,
          const sal_Int32 count, typelib_MethodParameter* parameters, void* returnValue,
          void** arguments, uno_Any** exception)
{
    static_assert(sizeof(sal_uInt64) == sizeof(void*));
    typelib_TypeDescription* aReturnTD = nullptr;
    TYPELIB_DANGER_GET(&aReturnTD, returnType);
    const ReturnKind eRetKind = getReturnKind(aReturnTD);
    const bool retConv = bridges::cpp_uno::shared::relatesToInterfaceType(aReturnTD);
    void* ret = retConv ? alloca(aReturnTD->nSize) : returnValue;

    sal_uInt64** thisPtr = reinterpret_cast<sal_uInt64**>(pProxy->getCppI()) + slot.offset;

    void** cppArgs = static_cast<void**>(alloca(count * sizeof(void*)));
    typelib_TypeDescription** ptds
        = static_cast<typelib_TypeDescription**>(alloca(count * sizeof(typelib_TypeDescription*)));

    sal_uInt64* gpr = static_cast<sal_uInt64*>(alloca(16 * sizeof(sal_uInt64)));
    sal_uInt64* fpr = &gpr[8];
    sal_uInt64* stack = static_cast<sal_uInt64*>(alloca(count * sizeof(sal_uInt64)));

    sal_Int32 sp = 0;
    sal_Int32 nGPR = 0;
    sal_Int32 nFPR = 0;
    gpr[nGPR++] = reinterpret_cast<sal_uInt64>(thisPtr);
    if (eRetKind == RETURN_KIND_INDIRECT)
    {
        gpr[nGPR++] = reinterpret_cast<sal_uInt64>(ret);
    }

    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (!parameters[i].bOut && bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            cppArgs[i] = 0;
            switch (parameters[i].pTypeRef->eTypeClass)
            {
                case typelib_TypeClass_BOOLEAN:
                    pushArgument(*static_cast<sal_Bool*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_BYTE:
                    pushArgument(*static_cast<sal_Int8*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_SHORT:
                    pushArgument(*static_cast<sal_Int16*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_UNSIGNED_SHORT:
                    pushArgument(*static_cast<sal_uInt16*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_LONG:
                case typelib_TypeClass_ENUM:
                    pushArgument(*static_cast<sal_Int32*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_UNSIGNED_LONG:
                    pushArgument(*static_cast<sal_uInt32*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_HYPER:
                    pushArgument(*static_cast<sal_Int64*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_UNSIGNED_HYPER:
                    pushArgument(*static_cast<sal_uInt64*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                case typelib_TypeClass_FLOAT:
                    pushArgument(*static_cast<sal_uInt32*>(arguments[i]), stack, sp, fpr, nFPR);
                    break;
                case typelib_TypeClass_DOUBLE:
                    pushArgument(*static_cast<sal_uInt64*>(arguments[i]), stack, sp, fpr, nFPR);
                    break;
                case typelib_TypeClass_CHAR:
                    pushArgument(*static_cast<sal_Unicode*>(arguments[i]), stack, sp, gpr, nGPR);
                    break;
                default:
                    assert(false);
            }
        }
        else
        {
            typelib_TypeDescription* ptd = 0;
            TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
            if (!parameters[i].bIn)
            {
                cppArgs[i] = alloca(ptd->nSize);
                uno_constructData(cppArgs[i], ptd);
                ptds[i] = ptd;
                pushArgument(reinterpret_cast<sal_uInt64>(cppArgs[i]), stack, sp, gpr, nGPR);
            }
            else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd))
            {
                cppArgs[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(cppArgs[i], arguments[i], ptd,
                                       pProxy->getBridge()->getUno2Cpp());
                ptds[i] = ptd;
                pushArgument(reinterpret_cast<sal_uInt64>(cppArgs[i]), stack, sp, gpr, nGPR);
            }
            else
            {
                cppArgs[i] = 0;
                pushArgument(reinterpret_cast<sal_uInt64>(arguments[i]), stack, sp, gpr, nGPR);
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
    }

    __try
    {
        callVirtualFunction(gpr, stack, sp, (*thisPtr)[slot.index]);
    }
    __except (msvc_filterCppException(GetExceptionInformation(), *exception,
                                      pProxy->getBridge()->getCpp2Uno()))
    {
        for (sal_Int32 i = 0; i != count; ++i)
        {
            if (cppArgs[i] != 0)
            {
                uno_destructData(cppArgs[i], ptds[i],
                                 reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
                TYPELIB_DANGER_RELEASE(ptds[i]);
            }
        }
        TYPELIB_DANGER_RELEASE(aReturnTD);
        return;
    }

    *exception = 0;
    for (sal_Int32 i = 0; i != count; ++i)
    {
        if (cppArgs[i] != 0)
        {
            if (parameters[i].bOut)
            {
                if (parameters[i].bIn)
                {
                    uno_destructData(arguments[i], ptds[i], 0);
                }
                uno_copyAndConvertData(arguments[i], cppArgs[i], ptds[i],
                                       pProxy->getBridge()->getCpp2Uno());
            }
            uno_destructData(cppArgs[i], ptds[i],
                             reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
            TYPELIB_DANGER_RELEASE(ptds[i]);
        }
    }

    switch (eRetKind)
    {
        case RETURN_KIND_REG:
            switch (aReturnTD->eTypeClass)
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
                case typelib_TypeClass_STRUCT:
                    std::memcpy(ret, gpr, aReturnTD->nSize);
                    break;
                case typelib_TypeClass_FLOAT:
                case typelib_TypeClass_DOUBLE:
                    std::memcpy(ret, fpr, aReturnTD->nSize);
                    break;
                default:
                    assert(false);
            }
            break;
        case RETURN_KIND_INDIRECT:
            break;
    }

    if (retConv)
    {
        uno_copyAndConvertData(returnValue, ret, aReturnTD, pProxy->getBridge()->getCpp2Uno());
        uno_destructData(ret, aReturnTD, reinterpret_cast<uno_ReleaseFunc>(css::uno::cpp_release));
    }
    TYPELIB_DANGER_RELEASE(aReturnTD);
}
}

namespace bridges::cpp_uno::shared
{
void unoInterfaceProxyDispatch(uno_Interface* pUnoI, typelib_TypeDescription const* pMemberDescr,
                               void* pReturn, void** pArgs, uno_Any** ppException)
{
    UnoInterfaceProxy* pProxy = static_cast<UnoInterfaceProxy*>(pUnoI);
    switch (pMemberDescr->eTypeClass)
    {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            typelib_InterfaceAttributeTypeDescription const* atd
                = reinterpret_cast<typelib_InterfaceAttributeTypeDescription const*>(pMemberDescr);
            VtableSlot slot(getVtableSlot(atd));
            if (pReturn != 0)
            { // getter
                call(pProxy, slot, atd->pAttributeTypeRef, 0, 0, pReturn, pArgs, ppException);
            }
            else
            { // setter
                typelib_MethodParameter param = { 0, atd->pAttributeTypeRef, true, false };
                typelib_TypeDescriptionReference* pReturnTD = nullptr;
                typelib_typedescriptionreference_new(&pReturnTD, typelib_TypeClass_VOID,
                                                     OUString("void").pData);
                slot.index += 1;
                call(pProxy, slot, pReturnTD, 1, &param, pReturn, pArgs, ppException);
                typelib_typedescriptionreference_release(pReturnTD);
            }
            break;
        }
        case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription const* mtd
                = reinterpret_cast<typelib_InterfaceMethodTypeDescription const*>(pMemberDescr);
            VtableSlot slot(getVtableSlot(mtd));
            switch (slot.index)
            {
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
                    typelib_TypeDescription* td = 0;
                    TYPELIB_DANGER_GET(
                        &td, (reinterpret_cast<css::uno::Type*>(pArgs[0])->getTypeLibType()));
                    if (td != 0)
                    {
                        uno_Interface* ifc = 0;
                        pProxy->pBridge->getUnoEnv()->getRegisteredInterface(
                            pProxy->pBridge->getUnoEnv(), reinterpret_cast<void**>(&ifc),
                            pProxy->oid.pData,
                            reinterpret_cast<typelib_InterfaceTypeDescription*>(td));
                        if (ifc != 0)
                        {
                            uno_any_construct(reinterpret_cast<uno_Any*>(pReturn), &ifc, td, 0);
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
                    call(pProxy, slot, mtd->pReturnTypeRef, mtd->nParams, mtd->pParams, pReturn,
                         pArgs, ppException);
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
