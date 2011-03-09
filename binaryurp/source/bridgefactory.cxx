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

#include <algorithm>

#include "com/sun/star/connection/XConnection.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "osl/diagnose.h"
#include "rtl/ref.hxx"
#include "sal/types.h"
#include "uno/lbnames.h"

#include "bridge.hxx"
#include "bridgefactory.hxx"

namespace binaryurp {

namespace {

namespace css = com::sun::star;

}

css::uno::Reference< css::uno::XInterface > BridgeFactory::static_create(
    css::uno::Reference< css::uno::XComponentContext > const & xContext)
    SAL_THROW((css::uno::Exception))
{
    return static_cast< cppu::OWeakObject * >(new BridgeFactory(xContext));
}

rtl::OUString BridgeFactory::static_getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM(
            "com.sun.star.comp.bridge.BridgeFactory"));
}

css::uno::Sequence< rtl::OUString >
BridgeFactory::static_getSupportedServiceNames() {
    rtl::OUString name(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory"));
    return css::uno::Sequence< rtl::OUString >(&name, 1);
}

void BridgeFactory::removeBridge(
    css::uno::Reference< css::bridge::XBridge > const & bridge)
{
    OSL_ASSERT(bridge.is());
    rtl::OUString n(bridge->getName());
    osl::MutexGuard g(*this);
    if (n.getLength() == 0) {
        BridgeList::iterator i(
            std::find(unnamed_.begin(), unnamed_.end(), bridge));
        if (i != unnamed_.end()) {
            unnamed_.erase(i);
        }
    } else {
        BridgeMap::iterator i(named_.find(n));
        if (i != named_.end() && i->second == bridge) {
            named_.erase(i);
        }
    }
}

BridgeFactory::BridgeFactory(
    css::uno::Reference< css::uno::XComponentContext > const & context):
    BridgeFactoryBase(*static_cast< osl::Mutex * >(this)), context_(context)
{
    OSL_ASSERT(context.is());
}

BridgeFactory::~BridgeFactory() {}

rtl::OUString BridgeFactory::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return static_getImplementationName();
}

sal_Bool BridgeFactory::supportsService(rtl::OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > s(getSupportedServiceNames());
    for (sal_Int32 i = 0; i != s.getLength(); ++i) {
        if (ServiceName == s[i]) {
            return true;
        }
    }
    return false;
}

css::uno::Sequence< rtl::OUString > BridgeFactory::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return static_getSupportedServiceNames();
}

css::uno::Reference< css::bridge::XBridge > BridgeFactory::createBridge(
    rtl::OUString const & sName, rtl::OUString const & sProtocol,
    css::uno::Reference< css::connection::XConnection > const & aConnection,
    css::uno::Reference< css::bridge::XInstanceProvider > const &
        anInstanceProvider)
    throw (
        css::bridge::BridgeExistsException, css::lang::IllegalArgumentException,
        css::uno::RuntimeException)
{
    rtl::Reference< Bridge > b;
    {
        osl::MutexGuard g(*this);
        if (named_.find(sName) != named_.end()) {
            throw css::bridge::BridgeExistsException(
                sName, static_cast< cppu::OWeakObject * >(this));
        }
        if (!(sProtocol.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("urp")) &&
              aConnection.is()))
        {
            throw css::lang::IllegalArgumentException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "BridgeFactory::createBridge: sProtocol != urp ||"
                        " aConnection == null")),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        b.set(new Bridge(this, sName, aConnection, anInstanceProvider));
        if (sName.getLength() == 0) {
            unnamed_.push_back(
                css::uno::Reference< css::bridge::XBridge >(b.get()));
        } else {
            named_[sName] = b.get();
        }
    }
    b->start();
    return css::uno::Reference< css::bridge::XBridge >(b.get());
}

css::uno::Reference< css::bridge::XBridge > BridgeFactory::getBridge(
    rtl::OUString const & sName) throw (css::uno::RuntimeException)
{
    osl::MutexGuard g(*this);
    BridgeMap::iterator i(named_.find(sName));
    return i == named_.end()
        ? css::uno::Reference< css::bridge::XBridge >() : i->second;
}

css::uno::Sequence< css::uno::Reference< css::bridge::XBridge > >
BridgeFactory::getExistingBridges() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(*this);
    if (unnamed_.size() > SAL_MAX_INT32) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "BridgeFactory::getExistingBridges: too many")),
            static_cast< cppu::OWeakObject * >(this));
    }
    sal_Int32 n = static_cast< sal_Int32 >(unnamed_.size());
    if (named_.size() > static_cast< sal_uInt32 >(SAL_MAX_INT32 - n)) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "BridgeFactory::getExistingBridges: too many")),
            static_cast< cppu::OWeakObject * >(this));
    }
    n = static_cast< sal_Int32 >(n + named_.size());
    css::uno::Sequence< css::uno::Reference< css::bridge::XBridge > > s(n);
    sal_Int32 i = 0;
    for (BridgeList::iterator j(unnamed_.begin()); j != unnamed_.end(); ++j) {
        s[i++] = *j;
    }
    for (BridgeMap::iterator j(named_.begin()); j != named_.end(); ++j) {
        s[i++] = j->second;
    }
    return s;
}

}

namespace {

static cppu::ImplementationEntry const services[] = {
    { &binaryurp::BridgeFactory::static_create,
      &binaryurp::BridgeFactory::static_getImplementationName,
      &binaryurp::BridgeFactory::static_getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

extern "C" SAL_DLLPUBLIC_EXPORT void SAL_CALL
component_getImplementationEnvironment(
    char const ** ppEnvTypeName, uno_Environment **)
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
