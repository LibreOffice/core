/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#include "sal/config.h"

#include <exception>
#include <vector>

#include "cppuhelper/exc_hlp.hxx"
#include "osl/diagnose.h"
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
    OSL_ASSERT(pInterface != 0);
    static_cast< Proxy * >(pInterface)->do_acquire();
}

extern "C" void SAL_CALL proxy_releaseInterface(uno_Interface * pInterface) {
    OSL_ASSERT(pInterface != 0);
    static_cast< Proxy * >(pInterface)->do_release();
}

extern "C" void SAL_CALL proxy_dispatchInterface(
    uno_Interface * pUnoI, typelib_TypeDescription const * pMemberType,
    void * pReturn, void ** pArgs, uno_Any ** ppException)
{
    OSL_ASSERT(pUnoI != 0);
    static_cast< Proxy * >(pUnoI)->do_dispatch(
        pMemberType, pReturn, pArgs, ppException);
}

}

Proxy::Proxy(
    rtl::Reference< Bridge > const & bridge, rtl::OUString const & oid,
    css::uno::TypeDescription const & type):
    bridge_(bridge), oid_(oid), type_(type), references_(1)
{
    OSL_ASSERT(bridge.is());
    acquire = &proxy_acquireInterface;
    release = &proxy_releaseInterface;
    pDispatcher = &proxy_dispatchInterface;
}

rtl::OUString Proxy::getOid() const {
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
        } catch (std::exception & e) {
            throw css::uno::RuntimeException(
                (rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("caught C++ exception: ")) +
                 rtl::OStringToOUString(
                     rtl::OString(e.what()), RTL_TEXTENCODING_ASCII_US)),
                css::uno::Reference< css::uno::XInterface >());
                // best-effort string conversion
        }
    } catch (css::uno::RuntimeException &) {
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
    css::uno::UnoInterfaceReference const & object, rtl::OUString * oid)
{
    OSL_ASSERT(object.is());
    return object.m_pUnoI->acquire == &proxy_acquireInterface &&
        static_cast< Proxy * >(object.m_pUnoI)->isProxy(bridge, oid);
}

Proxy::~Proxy() {}

void Proxy::do_dispatch_throw(
    typelib_TypeDescription const * member, void * returnValue,
    void ** arguments, uno_Any ** exception) const
{
    //TODO: Optimize queryInterface:
    OSL_ASSERT(member != 0);
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
        OSL_ASSERT(false); // this cannot happen
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
        OSL_ASSERT(
            ret.getType().get()->eTypeClass == typelib_TypeClass_EXCEPTION);
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
                OSL_ASSERT(i == outArgs.end());
                break;
            }
        default:
            OSL_ASSERT(false); // this cannot happen
            break;
        }
        *exception = 0;
    }
}

bool Proxy::isProxy(
    rtl::Reference< Bridge > const & bridge, rtl::OUString * oid) const
{
    OSL_ASSERT(oid != 0);
    if (bridge == bridge_) {
        *oid = oid_;
        return true;
    } else {
        return false;
    }
}

}
