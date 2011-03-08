/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_bridges.hxx"
#include "sal/config.h"

#include <algorithm>
#include <cstddef>
#include <cstring>

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/types.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"
#include "bridges/cpp_uno/shared/vtables.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/genfunc.hxx"
#include "osl/diagnose.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/alloca.h"
#include "sal/types.h"
#include "typelib/typeclass.h"
#include "typelib/typedescription.h"
#include "uno/any2.h"
#include "uno/data.h"

#include "callvirtualmethod.hxx"
#include "exceptions.hxx"
#include "fp.hxx"
#include "isdirectreturntype.hxx"

namespace {

namespace css = com::sun::star;

void storeFpRegsToStruct(typelib_TypeDescription * type, void * data) {
    for (typelib_CompoundTypeDescription * t =
             reinterpret_cast< typelib_CompoundTypeDescription * >(type);
         t != NULL; t = t->pBaseTypeDescription)
    {
        for (sal_Int32 i = 0; i < t->nMembers; ++i) {
            switch (t->ppTypeRefs[i]->eTypeClass) {
            case typelib_TypeClass_FLOAT:
                switch (t->pMemberOffsets[i]) {
                case 0:
                    fp_storef0(reinterpret_cast< float * >(data));
                    break;
                case 4:
                    fp_storef1(reinterpret_cast< float * >(data) + 1);
                    break;
                case 8:
                    fp_storef2(reinterpret_cast< float * >(data) + 2);
                    break;
                case 12:
                    fp_storef3(reinterpret_cast< float * >(data) + 3);
                    break;
                case 16:
                    fp_storef4(reinterpret_cast< float * >(data) + 4);
                    break;
                case 20:
                    fp_storef5(reinterpret_cast< float * >(data) + 5);
                    break;
                case 24:
                    fp_storef6(reinterpret_cast< float * >(data) + 6);
                    break;
                case 28:
                    fp_storef7(reinterpret_cast< float * >(data) + 7);
                    break;
                default:
                    OSL_ASSERT(false);
                    break;
                }
                break;
            case typelib_TypeClass_DOUBLE:
                switch (t->pMemberOffsets[i]) {
                case 0:
                    fp_stored0(reinterpret_cast< double * >(data));
                    break;
                case 8:
                    fp_stored2(reinterpret_cast< double * >(data) + 1);
                    break;
                case 16:
                    fp_stored4(reinterpret_cast< double * >(data) + 2);
                    break;
                case 24:
                    fp_stored6(reinterpret_cast< double * >(data) + 3);
                    break;
                default:
                    OSL_ASSERT(false);
                    break;
                }
                break;
            case typelib_TypeClass_STRUCT:
                {
                    typelib_TypeDescription * td = NULL;
                    TYPELIB_DANGER_GET(&td, t->ppTypeRefs[i]);
                    storeFpRegsToStruct(td, data);
                    TYPELIB_DANGER_RELEASE(td);
                    break;
                }
            }
        }
    }
}

void call(
    bridges::cpp_uno::shared::UnoInterfaceProxy * proxy,
    bridges::cpp_uno::shared::VtableSlot slot,
    typelib_TypeDescriptionReference * returnType, sal_Int32 count,
    typelib_MethodParameter * parameters, void * returnValue, void ** arguments,
    uno_Any ** exception)
{
    bool directRet = bridges::cpp_uno::cc5_solaris_sparc64::isDirectReturnType(
        returnType);
    long * stack = static_cast< long * >(
        alloca(
            std::max< sal_Int32 >(count + (directRet ? 1 : 2), 4) *
            sizeof (long)));
    sal_Int32 sp = 0;
    typelib_TypeDescription * rtd = NULL;
    TYPELIB_DANGER_GET(&rtd, returnType);
    bool retconv = bridges::cpp_uno::shared::relatesToInterfaceType(rtd);
    OSL_ASSERT(!(directRet && retconv));
    void * ret;
    if (!directRet) {
        ret = retconv ? alloca(rtd->nSize) : returnValue;
        stack[sp++] = reinterpret_cast< long >(ret);
    }
    unsigned long ** thisPtr = reinterpret_cast< unsigned long ** >(
        proxy->getCppI()) + slot.offset;
    stack[sp++] = reinterpret_cast< long >(thisPtr);
    void ** cppArgs = static_cast< void ** >(alloca(count * sizeof (void *)));
    typelib_TypeDescription ** ptds =
        static_cast< typelib_TypeDescription ** >(
            alloca(count * sizeof (typelib_TypeDescription *)));
    for (sal_Int32 i = 0; i < count; ++i) {
        if (!parameters[i].bOut &&
            bridges::cpp_uno::shared::isSimpleType(parameters[i].pTypeRef))
        {
            cppArgs[i] = NULL;
            switch (parameters[i].pTypeRef->eTypeClass) {
            case typelib_TypeClass_BOOLEAN:
                stack[sp] = *static_cast< sal_Bool * >(arguments[i]);
                break;
            case typelib_TypeClass_BYTE:
                stack[sp] = *static_cast< sal_Int8 * >(arguments[i]);
                break;
            case typelib_TypeClass_SHORT:
                stack[sp] = *static_cast< sal_Int16 * >(arguments[i]);
                break;
            case typelib_TypeClass_UNSIGNED_SHORT:
                stack[sp] = *static_cast< sal_uInt16 * >(arguments[i]);
                break;
            case typelib_TypeClass_LONG:
            case typelib_TypeClass_ENUM:
                stack[sp] = *static_cast< sal_Int32 * >(arguments[i]);
                break;
            case typelib_TypeClass_UNSIGNED_LONG:
                stack[sp] = *static_cast< sal_uInt32 * >(arguments[i]);
                break;
            case typelib_TypeClass_HYPER:
                stack[sp] = *static_cast< sal_Int64 * >(arguments[i]);
                break;
            case typelib_TypeClass_UNSIGNED_HYPER:
                stack[sp] = *static_cast< sal_uInt64 * >(arguments[i]);
                break;
            case typelib_TypeClass_FLOAT:
                {
                    float * f = static_cast< float * >(arguments[i]);
                    switch (sp) {
                    case 1:
                        fp_loadf3(f);
                        break;
                    case 2:
                        fp_loadf5(f);
                        break;
                    case 3:
                        fp_loadf7(f);
                        break;
                    case 4:
                        fp_loadf9(f);
                        break;
                    case 5:
                        fp_loadf11(f);
                        break;
                    case 6:
                        fp_loadf13(f);
                        break;
                    case 7:
                        fp_loadf15(f);
                        break;
                    case 8:
                        fp_loadf17(f);
                        break;
                    case 9:
                        fp_loadf19(f);
                        break;
                    case 10:
                        fp_loadf21(f);
                        break;
                    case 11:
                        fp_loadf23(f);
                        break;
                    case 12:
                        fp_loadf25(f);
                        break;
                    case 13:
                        fp_loadf27(f);
                        break;
                    case 14:
                        fp_loadf29(f);
                        break;
                    case 15:
                        fp_loadf31(f);
                        break;
                    default:
                        reinterpret_cast< float * >(stack + sp)[1] = *f;
                        break;
                    }
                    break;
                }
            case typelib_TypeClass_DOUBLE:
                {
                    double * d = static_cast< double * >(arguments[i]);
                    switch (sp) {
                    case 1:
                        fp_loadd2(d);
                        break;
                    case 2:
                        fp_loadd4(d);
                        break;
                    case 3:
                        fp_loadd6(d);
                        break;
                    case 4:
                        fp_loadd8(d);
                        break;
                    case 5:
                        fp_loadd10(d);
                        break;
                    case 6:
                        fp_loadd12(d);
                        break;
                    case 7:
                        fp_loadd14(d);
                        break;
                    case 8:
                        fp_loadd16(d);
                        break;
                    case 9:
                        fp_loadd18(d);
                        break;
                    case 10:
                        fp_loadd20(d);
                        break;
                    case 11:
                        fp_loadd22(d);
                        break;
                    case 12:
                        fp_loadd24(d);
                        break;
                    case 13:
                        fp_loadd26(d);
                        break;
                    case 14:
                        fp_loadd28(d);
                        break;
                    case 15:
                        fp_loadd30(d);
                        break;
                    default:
                        *reinterpret_cast< double * >(stack + sp) = *d;
                        break;
                    }
                    break;
                }
            case typelib_TypeClass_CHAR:
                stack[sp] = *static_cast< sal_Unicode * >(arguments[i]);
                break;
            default:
                OSL_ASSERT(false);
                break;
            }
        } else {
            typelib_TypeDescription * ptd = NULL;
            TYPELIB_DANGER_GET(&ptd, parameters[i].pTypeRef);
            if (!parameters[i].bIn) {
                cppArgs[i] = alloca(ptd->nSize);
                uno_constructData(cppArgs[i], ptd);
                ptds[i] = ptd;
                *reinterpret_cast< void ** >(stack + sp) = cppArgs[i];
            } else if (bridges::cpp_uno::shared::relatesToInterfaceType(ptd)) {
                cppArgs[i] = alloca(ptd->nSize);
                uno_copyAndConvertData(
                    cppArgs[i], arguments[i], ptd,
                    proxy->getBridge()->getUno2Cpp());
                ptds[i] = ptd;
                *reinterpret_cast< void ** >(stack + sp) = cppArgs[i];
            } else {
                cppArgs[i] = NULL;
                *reinterpret_cast< void ** >(stack + sp) = arguments[i];
                TYPELIB_DANGER_RELEASE(ptd);
            }
        }
        ++sp;
    }
    try {
        callVirtualMethod(
            (*thisPtr)[slot.index + 2], stack,
            std::max< sal_Int32 >(sp - 6, 0) * sizeof (long));
    } catch (css::uno::Exception &) {
        void * exc = __Crun::ex_get();
        char const * name = __Cimpl::ex_name();
        bridges::cpp_uno::cc5_solaris_sparc64::fillUnoException(
            exc, name, *exception, proxy->getBridge()->getCpp2Uno());
        for (sal_Int32 i = 0; i < count; ++i) {
            if (cppArgs[i] != NULL) {
                uno_destructData(
                    cppArgs[i], ptds[i],
                    reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release));
                TYPELIB_DANGER_RELEASE(ptds[i]);
            }
        }
        TYPELIB_DANGER_RELEASE(rtd);
        return;
    }
    *exception = NULL;
    for (sal_Int32 i = 0; i < count; ++i) {
        if (cppArgs[i] != NULL) {
            if (parameters[i].bOut) {
                if (parameters[i].bIn) {
                    uno_destructData(arguments[i], ptds[i], NULL);
                }
                uno_copyAndConvertData(
                    arguments[i], cppArgs[i], ptds[i],
                    proxy->getBridge()->getCpp2Uno());
            }
            uno_destructData(
                cppArgs[i], ptds[i],
                reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release));
            TYPELIB_DANGER_RELEASE(ptds[i]);
        }
    }
    if (directRet) {
        switch (rtd->eTypeClass) {
        case typelib_TypeClass_FLOAT:
            fp_storef0(reinterpret_cast< float * >(returnValue));
            break;
        case typelib_TypeClass_DOUBLE:
            fp_stored0(reinterpret_cast< double * >(returnValue));
            break;
        case typelib_TypeClass_STRUCT:
            storeFpRegsToStruct(rtd, stack);
            // fall through
        case typelib_TypeClass_ANY:
            std::memcpy(returnValue, stack, rtd->nSize);
            break;
        default:
            OSL_ASSERT(rtd->nSize <= 8);
            std::memcpy(
                returnValue,
                reinterpret_cast< char * >(stack) + (8 - rtd->nSize),
                rtd->nSize);
            break;
        }
    } else if (retconv) {
        uno_copyAndConvertData(
            returnValue, ret, rtd, proxy->getBridge()->getCpp2Uno());
        uno_destructData(
            ret, rtd,
            reinterpret_cast< uno_ReleaseFunc >(css::uno::cpp_release));
    }
    TYPELIB_DANGER_RELEASE(rtd);
}

}

namespace bridges { namespace cpp_uno { namespace shared {

void unoInterfaceProxyDispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberDescr,
    void * pReturn, void * pArgs[], uno_Any ** ppException)
{
    bridges::cpp_uno::shared::UnoInterfaceProxy * proxy =
        static_cast< bridges::cpp_uno::shared::UnoInterfaceProxy * >(pUnoI);
    switch (pMemberDescr->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        {
            VtableSlot slot(
                getVtableSlot(
                    reinterpret_cast<
                    typelib_InterfaceAttributeTypeDescription const * >(
                        pMemberDescr)));
            if (pReturn != NULL) {
                // Getter:
                call(
                    proxy, slot,
                    (reinterpret_cast<
                     typelib_InterfaceAttributeTypeDescription const * >(
                         pMemberDescr)->pAttributeTypeRef),
                    0, NULL, pReturn, pArgs, ppException);
            } else {
                // Setter:
                typelib_MethodParameter param = {
                    NULL,
                    (reinterpret_cast<
                     typelib_InterfaceAttributeTypeDescription const * >(
                         pMemberDescr)->pAttributeTypeRef),
                    true, false };
                typelib_TypeDescriptionReference * rtd = NULL;
                typelib_typedescriptionreference_new(
                    &rtd, typelib_TypeClass_VOID,
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("void")).pData);
                slot.index += 1;
                call(proxy, slot, rtd, 1, &param, pReturn, pArgs, ppException);
                typelib_typedescriptionreference_release(rtd);
            }
            break;
        }
    case typelib_TypeClass_INTERFACE_METHOD:
        {
            VtableSlot slot(
                getVtableSlot(
                    reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(
                        pMemberDescr)));
            switch (slot.index) {
            case 1:
                pUnoI->acquire(pUnoI);
                *ppException = NULL;
                break;
            case 2:
                pUnoI->release(pUnoI);
                *ppException = NULL;
                break;
            case 0:
                {
                    typelib_TypeDescription * td = NULL;
                    TYPELIB_DANGER_GET(
                        &td,
                        reinterpret_cast< css::uno::Type * >(
                            pArgs[0])->getTypeLibType());
                    if (td != NULL) {
                        uno_Interface * ifc = NULL;
                        proxy->pBridge->getUnoEnv()->getRegisteredInterface(
                            proxy->pBridge->getUnoEnv(),
                            reinterpret_cast< void ** >(&ifc),
                            proxy->oid.pData,
                            (reinterpret_cast<
                             typelib_InterfaceTypeDescription * >(td)));
                        if (ifc != NULL) {
                            uno_any_construct(
                                reinterpret_cast< uno_Any * >(pReturn),
                                &ifc, td, NULL);
                            ifc->release(ifc);
                            TYPELIB_DANGER_RELEASE(td);
                            *ppException = NULL;
                            break;
                        }
                        TYPELIB_DANGER_RELEASE(td);
                    }
                } // fall through
            default:
                call(
                    proxy, slot,
                    (reinterpret_cast<
                     typelib_InterfaceMethodTypeDescription const * >(
                         pMemberDescr)->pReturnTypeRef),
                    (reinterpret_cast<
                     typelib_InterfaceMethodTypeDescription const * >(
                         pMemberDescr)->nParams),
                    (reinterpret_cast<
                     typelib_InterfaceMethodTypeDescription const * >(
                         pMemberDescr)->pParams),
                    pReturn, pArgs, ppException);
            }
            break;
        }
    default:
        OSL_ASSERT(false);
        break;
    }
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
