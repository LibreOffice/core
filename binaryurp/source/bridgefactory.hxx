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

#ifndef INCLUDED_BINARYURP_SOURCE_BRIDGEFACTORY_HXX
#define INCLUDED_BINARYURP_SOURCE_BRIDGEFACTORY_HXX

#include <sal/config.h>

#include <exception>
#include <vector>
#include <map>

#include <com/sun/star/bridge/XBridgeFactory2.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase.hxx>
#include <sal/types.h>

namespace com::sun::star {
    namespace connection { class XConnection; }
    namespace uno {
        class XComponentContext;
        class XInterface;
    }
}

namespace binaryurp {

// That BridgeFactory derives from XComponent appears to be a historic mistake;
// the implementation does not care about a disposed state:

typedef
    cppu::WeakComponentImplHelper<
        com::sun::star::lang::XServiceInfo,
        com::sun::star::bridge::XBridgeFactory2 >
    BridgeFactoryBase;

class BridgeFactory : private cppu::BaseMutex, public BridgeFactoryBase
{
public:
    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL static_create(
        com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
            const & xContext);

    static OUString SAL_CALL static_getImplementationName();

    static com::sun::star::uno::Sequence< OUString > SAL_CALL
    static_getSupportedServiceNames();

    void removeBridge(
        com::sun::star::uno::Reference< com::sun::star::bridge::XBridge >
            const & bridge);

    using BridgeFactoryBase::acquire;
    using BridgeFactoryBase::release;

private:
    BridgeFactory(const BridgeFactory&) = delete;
    BridgeFactory& operator=(const BridgeFactory&) = delete;

    BridgeFactory();

    virtual ~BridgeFactory() override;

    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames() override;

    virtual com::sun::star::uno::Reference< com::sun::star::bridge::XBridge >
    SAL_CALL createBridge(
        OUString const & sName, OUString const & sProtocol,
        com::sun::star::uno::Reference<
            com::sun::star::connection::XConnection > const & aConnection,
        com::sun::star::uno::Reference<
            com::sun::star::bridge::XInstanceProvider > const &
                anInstanceProvider) override;

    virtual com::sun::star::uno::Reference< com::sun::star::bridge::XBridge >
    SAL_CALL getBridge(
        OUString const & sName) override;

    virtual
    com::sun::star::uno::Sequence<
        com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > >
    SAL_CALL getExistingBridges() override;

    void SAL_CALL disposing() override;

    typedef
        std::vector<
            com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > >
        BridgeVector;

    typedef
        std::map<
            OUString,
            com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > >
        BridgeMap;

    BridgeVector unnamed_;
    BridgeMap named_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
