/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <sal/config.h>

#include <vector>
#include <map>

#include <com/sun/star/bridge/XBridgeFactory2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cpo/uno/Reference.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <sal/types.h>

namespace binaryurp {

// That BridgeFactory derives from XComponent appears to be a historic mistake;
// the implementation does not care about a disposed state:

typedef
    cppu::WeakComponentImplHelper<
        css::lang::XServiceInfo,
        css::bridge::XBridgeFactory2 >
    BridgeFactoryBase;

class BridgeFactory : private cppu::BaseMutex, public BridgeFactoryBase
{
public:
    void removeBridge(
        cpo::uno::Reference< css::bridge::XBridge >
            const & bridge);

    using BridgeFactoryBase::acquire;
    using BridgeFactoryBase::release;

    BridgeFactory(const BridgeFactory&) = delete;
    BridgeFactory& operator=(const BridgeFactory&) = delete;

    BridgeFactory();

    virtual ~BridgeFactory() override;

private:
    virtual OUString getImplementationName() override;

    virtual bool supportsService(OUString const & ServiceName) override;

    virtual cpo::uno::Sequence< OUString >
    getSupportedServiceNames() override;

    virtual cpo::uno::Reference< css::bridge::XBridge >
    createBridge(
        OUString const & sName, OUString const & sProtocol,
        cpo::uno::Reference< css::connection::XConnection > const & aConnection,
        cpo::uno::Reference< css::bridge::XInstanceProvider > const &
                anInstanceProvider) override;

    virtual cpo::uno::Reference< css::bridge::XBridge >
    getBridge(
        OUString const & sName) override;

    virtual
    cpo::uno::Sequence< cpo::uno::Reference< css::bridge::XBridge > >
    getExistingBridges() override;

    void disposing() override;

    typedef
        std::vector< cpo::uno::Reference< css::bridge::XBridge > >
        BridgeVector;

    typedef
        std::map<
            OUString,
            cpo::uno::Reference< css::bridge::XBridge > >
        BridgeMap;

    BridgeVector unnamed_;
    BridgeMap named_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
