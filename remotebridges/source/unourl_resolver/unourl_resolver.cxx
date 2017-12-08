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

#include <rtl/malformeduriexception.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/unourl.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::connection;
using namespace com::sun::star::bridge;
using namespace com::sun::star::registry;

#define IMPLNAME        "com.sun.star.comp.bridge.UnoUrlResolver"

namespace unourl_resolver
{

Sequence< OUString > resolver_getSupportedServiceNames()
{
    Sequence< OUString > seqNames { "com.sun.star.bridge.UnoUrlResolver" };
    return seqNames;
}

OUString resolver_getImplementationName()
{
    return OUString(IMPLNAME);
}

class ResolverImpl : public WeakImplHelper< XServiceInfo, XUnoUrlResolver >
{
    Reference< XMultiComponentFactory > _xSMgr;
    Reference< XComponentContext > _xCtx;

public:
    explicit ResolverImpl( const Reference< XComponentContext > & xSMgr );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XUnoUrlResolver
    virtual Reference< XInterface > SAL_CALL resolve( const OUString & rUnoUrl ) override;
};

ResolverImpl::ResolverImpl( const Reference< XComponentContext > & xCtx )
    : _xSMgr( xCtx->getServiceManager() )
    , _xCtx( xCtx )
{}

// XServiceInfo
OUString ResolverImpl::getImplementationName()
{
    return resolver_getImplementationName();
}

sal_Bool ResolverImpl::supportsService( const OUString & rServiceName )
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > ResolverImpl::getSupportedServiceNames()
{
    return resolver_getSupportedServiceNames();
}

// XUnoUrlResolver
Reference< XInterface > ResolverImpl::resolve( const OUString & rUnoUrl )
{
    OUString aProtocolDescr;
    OUString aConnectDescr;
    OUString aInstanceName;
    try
    {
        cppu::UnoUrl aUrl(rUnoUrl);
        aProtocolDescr = aUrl.getProtocol().getDescriptor();
        aConnectDescr = aUrl.getConnection().getDescriptor();
        aInstanceName = aUrl.getObjectName();
    }
    catch (const rtl::MalformedUriException & rEx)
    {
        throw ConnectionSetupException(rEx.getMessage(), nullptr);
    }

    Reference< XConnector > xConnector(
        _xSMgr->createInstanceWithContext( "com.sun.star.connection.Connector", _xCtx ),
        UNO_QUERY_THROW );
    Reference< XConnection > xConnection( xConnector->connect( aConnectDescr ) );

    // As soon as singletons are ready, switch to singleton !
    Reference< XBridgeFactory2 > xBridgeFactory( BridgeFactory::create(_xCtx) );

    // bridge
    Reference< XBridge > xBridge( xBridgeFactory->createBridge(
        OUString(), aProtocolDescr,
        xConnection, Reference< XInstanceProvider >() ) );

    Reference< XInterface > xRet( xBridge->getInstance( aInstanceName ) );

    return xRet;
}

static Reference< XInterface > ResolverImpl_create( const Reference< XComponentContext > & xCtx )
{
    return Reference< XInterface >( *new ResolverImpl( xCtx ) );
}

}

using namespace unourl_resolver;

static const struct ImplementationEntry g_entries[] =
{
    {
        ResolverImpl_create, resolver_getImplementationName,
        resolver_getSupportedServiceNames, createSingleComponentFactory,
        nullptr, 0
    },
    { nullptr, nullptr, nullptr, nullptr, nullptr, 0 }
};

extern "C" SAL_DLLPUBLIC_EXPORT void * uuresolver_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    return component_getFactoryHelper( pImplName, pServiceManager, pRegistryKey , g_entries );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
