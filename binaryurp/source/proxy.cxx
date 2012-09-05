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

#include "sal/config.h"

#include <cassert>
#include <exception>
#include <vector>

#include "cppuhelper/exc_hlp.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "typelib/typedescription.hxx"
#include "uno/any2.h"
#include "uno/dispatcher.h"
#include "uno/dispatcher.hxx"

#include "binaryany.hxx"
#include "bridge.hxx"
#include "proxy.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

extern "C" void SAL_CALL proxy_acquireInterface(uno_Interface * pInterface) {
    assert(pInterface != 0);
    static_cast< Proxy * >(pInterface)->do_acquire();
}

extern "C" void SAL_CALL proxy_releaseInterface(uno_Interface * pInterface) {
    assert(pInterface != 0);
    static_cast< Proxy * >(pInterface)->do_release();
}

extern "C" void SAL_CALL proxy_dispatchInterface(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberType,
    void * pReturn, void ** pArgs, uno_Any ** ppException)
{
    assert(pUnoI != 0);
    static_cast< Proxy * >(pUnoI)->do_dispatch(
        pMemberType, pReturn, pArgs, ppException);
}

}

Proxy::Proxy(
    rtl::Reference< Bridge > const & bridge, OUString const & oid,
    css::uno::TypeDescription const & type):
    bridge_(bridge), oid_(oid), type_(type), references_(1)
{
    assert(bridge.is());
    acquire = &proxy_acquireInterface;
    release = &proxy_releaseInterface;
    pDispatcher = &proxy_dispatchInterface;
}

OUString Proxy::getOid() const {
    return oid_;
}

css::uno::TypeDescription Proxy::getType() const {
    return type_;
}

void Proxy::do_acquire() {
    if (osl_incrementInterlockedCount(&references_) == 1) {
        bridge_->resurrectProxy(*this);
    }
}

void Proxy::do_release() {
    if (osl_decrementInterlockedCount(&references_) == 0) {
        bridge_->revokeProxy(*this);
    }
}

void Proxy::do_free() {
    bridge_->freeProxy(*this);
    delete this;
}

void Proxy::do_dispatch(
    typelib_TypeDescription const * member, void * returnValue,
    void ** arguments, uno_Any ** exception) const
{
    try {
        try {
            do_dispatch_throw(member, returnValue, arguments, exception);
        } catch (const std::exception & e) {
            throw css::uno::RuntimeException(
                ("caught C++ exception: " +
                rtl::OStringToOUString(
                    rtl::OString(e.what()), RTL_TEXTENCODING_ASCII_US)),
                css::uno::Reference< css::uno::XInterface >());
                // best-effort string conversion
        }
    } catch (const css::uno::RuntimeException &) {
        css::uno::Any exc(cppu::getCaughtException());
        uno_copyAndConvertData(
            *exception, &exc,
            (css::uno::TypeDescription(cppu::UnoType< css::uno::Any >::get()).
             get()),
            bridge_->getCppToBinaryMapping().get());
    }
}

bool Proxy::isProxy(
    rtl::Reference< Bridge > const & bridge,
    css::uno::UnoInterfaceReference const & object, OUString * oid)
{
    assert(object.is());
    return object.m_pUnoI->acquire == &proxy_acquireInterface &&
        static_cast< Proxy * >(object.m_pUnoI)->isProxy(bridge, oid);
}

Proxy::~Proxy() {}

void Proxy::do_dispatch_throw(
    typelib_TypeDescription const * member, void * returnValue,
    void ** arguments, uno_Any ** exception) const
{
    //TODO: Optimize queryInterface:
    assert(member != 0);
    bool setter = false;
    std::vector< BinaryAny > inArgs;
    switch (member->eTypeClass) {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        setter = returnValue == 0;
        if (setter) {
            inArgs.push_back(
                BinaryAny(
                    css::uno::TypeDescription(
                        reinterpret_cast<
                            typelib_InterfaceAttributeTypeDescription const * >(
                                member)->
                        pAttributeTypeRef),
                    arguments[0]));
        }
        break;
    case typelib_TypeClass_INTERFACE_METHOD:
        {
            typelib_InterfaceMethodTypeDescription const * mtd =
                reinterpret_cast<
                    typelib_InterfaceMethodTypeDescription const * >(member);
            for (sal_Int32 i = 0; i != mtd->nParams; ++i) {
                if (mtd->pParams[i].bIn) {
                    inArgs.push_back(
                        BinaryAny(
                            css::uno::TypeDescription(mtd->pParams[i].pTypeRef),
                            arguments[i]));
                }
            }
            break;
        }
    default:
        assert(false); // this cannot happen
        break;
    }
    BinaryAny ret;
    std::vector< BinaryAny > outArgs;
    if (bridge_->makeCall(
            oid_,
            css::uno::TypeDescription(
                const_cast< typelib_TypeDescription * >(member)),
            setter, inArgs, &ret, &outArgs))
    {
        assert(ret.getType().get()->eTypeClass == typelib_TypeClass_EXCEPTION);
        uno_any_construct(
            *exception, ret.getValue(ret.getType()), ret.getType().get(), 0);
    } else {
        switch (member->eTypeClass) {
        case typelib_TypeClass_INTERFACE_ATTRIBUTE:
            if (!setter) {
                css::uno::TypeDescription t(
                    reinterpret_cast<
                        typelib_InterfaceAttributeTypeDescription const * >(
                            member)->
                    pAttributeTypeRef);
                uno_copyData(returnValue, ret.getValue(t), t.get(), 0);
            }
            break;
        case typelib_TypeClass_INTERFACE_METHOD:
            {
                typelib_InterfaceMethodTypeDescription const * mtd =
                    reinterpret_cast<
                        typelib_InterfaceMethodTypeDescription const * >(
                            member);
                css::uno::TypeDescription t(mtd->pReturnTypeRef);
                if (t.get()->eTypeClass != typelib_TypeClass_VOID) {
                    uno_copyData(returnValue, ret.getValue(t), t.get(), 0);
                }
                std::vector< BinaryAny >::iterator i(outArgs.begin());
                for (sal_Int32 j = 0; j != mtd->nParams; ++j) {
                    if (mtd->pParams[j].bOut) {
                        css::uno::TypeDescription pt(mtd->pParams[j].pTypeRef);
                        if (mtd->pParams[j].bIn) {
                            uno_assignData(
                                arguments[j], pt.get(), i++->getValue(pt),
                                pt.get(), 0, 0, 0);
                        } else {
                            uno_copyData(
                                arguments[j], i++->getValue(pt), pt.get(), 0);
                        }
                    }
                }
                assert(i == outArgs.end());
                break;
            }
        default:
            assert(false); // this cannot happen
            break;
        }
        *exception = 0;
    }
}

bool Proxy::isProxy(
    rtl::Reference< Bridge > const & bridge, OUString * oid) const
{
    assert(oid != 0);
    if (bridge == bridge_) {
        *oid = oid_;
        return true;
    } else {
        return false;
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
