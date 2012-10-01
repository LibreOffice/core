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

#include <algorithm>
#include <cassert>

#include "com/sun/star/connection/XConnection.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

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

OUString BridgeFactory::static_getImplementationName() {
    return OUString("com.sun.star.comp.bridge.BridgeFactory");
}

css::uno::Sequence< OUString >
BridgeFactory::static_getSupportedServiceNames() {
    OUString name("com.sun.star.bridge.BridgeFactory");
    return css::uno::Sequence< OUString >(&name, 1);
}

void BridgeFactory::removeBridge(
    css::uno::Reference< css::bridge::XBridge > const & bridge)
{
    assert(bridge.is());
    OUString n(bridge->getName());
    osl::MutexGuard g(*this);
    if (n.isEmpty()) {
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
    assert(context.is());
}

BridgeFactory::~BridgeFactory() {}

OUString BridgeFactory::getImplementationName()
    throw (css::uno::RuntimeException)
{
    return static_getImplementationName();
}

sal_Bool BridgeFactory::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< OUString > s(getSupportedServiceNames());
    for (sal_Int32 i = 0; i != s.getLength(); ++i) {
        if (ServiceName == s[i]) {
            return true;
        }
    }
    return false;
}

css::uno::Sequence< OUString > BridgeFactory::getSupportedServiceNames()
    throw (css::uno::RuntimeException)
{
    return static_getSupportedServiceNames();
}

css::uno::Reference< css::bridge::XBridge > BridgeFactory::createBridge(
    OUString const & sName, OUString const & sProtocol,
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
        if (sProtocol != "urp" || !aConnection.is()) {
            throw css::lang::IllegalArgumentException(
                ("BridgeFactory::createBridge: sProtocol != urp ||"
                 " aConnection == null"),
                static_cast< cppu::OWeakObject * >(this), -1);
        }
        b.set(new Bridge(this, sName, aConnection, anInstanceProvider));
        if (sName.isEmpty()) {
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
    OUString const & sName) throw (css::uno::RuntimeException)
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
            "BridgeFactory::getExistingBridges: too many",
            static_cast< cppu::OWeakObject * >(this));
    }
    sal_Int32 n = static_cast< sal_Int32 >(unnamed_.size());
    if (named_.size() > static_cast< sal_uInt32 >(SAL_MAX_INT32 - n)) {
        throw css::uno::RuntimeException(
            "BridgeFactory::getExistingBridges: too many",
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
      &cppu::createOneInstanceComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL binaryurp_component_getFactory(
    char const * pImplName, void * pServiceManager, void * pRegistryKey)
{
    return cppu::component_getFactoryHelper(
        pImplName, pServiceManager, pRegistryKey, services);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
