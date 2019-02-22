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

#include "acceptor.hxx"
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/connection/Acceptor.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

using namespace css::bridge;
using namespace css::connection;
using namespace css::lang;
using namespace css::uno;

namespace desktop
{

extern "C" {

static void offacc_workerfunc (void * acc)
{
    osl_setThreadName("URP Acceptor");

    static_cast<Acceptor*>(acc)->run();
}

}

Acceptor::Acceptor( const Reference< XComponentContext >& rxContext )
    : m_thread(nullptr)
    , m_rContext(rxContext)
    , m_bInit(false)
    , m_bDying(false)
{
    m_rAcceptor = css::connection::Acceptor::create(m_rContext);
    m_rBridgeFactory = BridgeFactory::create(m_rContext);
}


Acceptor::~Acceptor()
{
    m_rAcceptor->stopAccepting();
    oslThread t;
    {
        osl::MutexGuard g(m_aMutex);
        t = m_thread;
    }
    //prevent locking if the thread is still waiting
    m_bDying = true;
    m_cEnable.set();
    osl_joinWithThread(t);
    osl_destroyThread(t);
    {
        // Make the final state of m_bridges visible to this thread (since
        // m_thread is joined, the code that follows is the only one left
        // accessing m_bridges):
        osl::MutexGuard g(m_aMutex);
    }
    for (;;) {
        css::uno::Reference< css::bridge::XBridge > b(m_bridges.remove());
        if (!b.is()) {
            break;
        }
        css::uno::Reference< css::lang::XComponent >(
            b, css::uno::UNO_QUERY_THROW)->dispose();
    }
}

void Acceptor::run()
{
    SAL_INFO( "desktop.offacc", "Acceptor::run" );
    for (;;)
    {
        try
        {
            // wait until we get enabled
            SAL_INFO( "desktop.offacc",
                "Acceptor::run waiting for office to come up");
            m_cEnable.wait();
            if (m_bDying) //see destructor
                break;
            SAL_INFO( "desktop.offacc",
                "Acceptor::run now enabled and continuing");

            // accept connection
            Reference< XConnection > rConnection = m_rAcceptor->accept( m_aConnectString );
            // if we return without a valid connection we mus assume that the acceptor
            // is destructed so we break out of the run method terminating the thread
            if (! rConnection.is()) break;
            OUString aDescription = rConnection->getDescription();
            SAL_INFO( "desktop.offacc", "Acceptor::run connection " << aDescription );

            // create instanceprovider for this connection
            Reference< XInstanceProvider > rInstanceProvider(new AccInstanceProvider(m_rContext));
            // create the bridge. The remote end will have a reference to this bridge
            // thus preventing the bridge from being disposed. When the remote end releases
            // the bridge, it will be destructed.
            Reference< XBridge > rBridge = m_rBridgeFactory->createBridge(
                "", m_aProtocol, rConnection, rInstanceProvider);
            osl::MutexGuard g(m_aMutex);
            m_bridges.add(rBridge);
        } catch (const Exception& e) {
            SAL_WARN("desktop.offacc", "caught " << e);
            // connection failed...
            // something went wrong during connection setup.
            // just wait for a new connection to accept
        }
    }
}

// XInitialize
void Acceptor::initialize( const Sequence<Any>& aArguments )
{
    // prevent multiple initialization
    osl::ClearableMutexGuard aGuard( m_aMutex );
    SAL_INFO( "desktop.offacc", "Acceptor::initialize()" );

    bool bOk = false;

    // arg count
    int nArgs = aArguments.getLength();

    // not yet initialized and accept-string
    if (!m_bInit && nArgs > 0 && (aArguments[0] >>= m_aAcceptString))
    {
        SAL_INFO( "desktop.offacc", "Acceptor::initialize string=" << m_aAcceptString );

        // get connect string and protocol from accept string
        // "<connectString>;<protocol>"
        sal_Int32 nIndex1 = m_aAcceptString.indexOf( ';' );
        if (nIndex1 < 0)
            throw IllegalArgumentException(
                    "Invalid accept-string format", m_rContext, 1);
        m_aConnectString = m_aAcceptString.copy( 0 , nIndex1 ).trim();
        nIndex1++;
        sal_Int32 nIndex2 = m_aAcceptString.indexOf( ';' , nIndex1 );
        if (nIndex2 < 0) nIndex2 = m_aAcceptString.getLength();
        m_aProtocol = m_aAcceptString.copy( nIndex1, nIndex2 - nIndex1 );

        // start accepting in new thread...
        m_thread = osl_createThread(offacc_workerfunc, this);
        m_bInit = true;
        bOk = true;
    }

    // do we want to enable accepting?
    bool bEnable = false;
    if (((nArgs == 1 && (aArguments[0] >>= bEnable)) ||
         (nArgs == 2 && (aArguments[1] >>= bEnable))) &&
        bEnable )
    {
        m_cEnable.set();
        bOk = true;
    }

    if (!bOk)
    {
        throw IllegalArgumentException( "invalid initialization", m_rContext, 1);
    }
}

// XServiceInfo
OUString Acceptor::impl_getImplementationName()
{
    return OUString("com.sun.star.office.comp.Acceptor");
}
OUString Acceptor::getImplementationName()
{
    return Acceptor::impl_getImplementationName();
}
Sequence<OUString> Acceptor::impl_getSupportedServiceNames()
{
    return { "com.sun.star.office.Acceptor" };
}
Sequence<OUString> Acceptor::getSupportedServiceNames()
{
    return Acceptor::impl_getSupportedServiceNames();
}

sal_Bool Acceptor::supportsService(OUString const & ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

// Factory
Reference< XInterface > Acceptor::impl_getInstance( const Reference< XMultiServiceFactory >& aFactory )
{
    try {
        return static_cast<cppu::OWeakObject *>(
            new Acceptor(comphelper::getComponentContext(aFactory)));
    } catch ( const Exception& ) {
        return css::uno::Reference<css::uno::XInterface>();
    }
}

// InstanceProvider
AccInstanceProvider::AccInstanceProvider(const Reference<XComponentContext>& rxContext)
{
    m_rContext = rxContext;
}

AccInstanceProvider::~AccInstanceProvider()
{
}

Reference<XInterface> AccInstanceProvider::getInstance (const OUString& aName )
{

    Reference<XInterface> rInstance;

    if ( aName == "StarOffice.ServiceManager" )
    {
        rInstance.set( m_rContext->getServiceManager() );
    }
    else if ( aName == "StarOffice.ComponentContext" )
    {
        rInstance = m_rContext;
    }
    else if ( aName == "StarOffice.NamingService" )
    {
        Reference< XNamingService > rNamingService(
            m_rContext->getServiceManager()->createInstanceWithContext("com.sun.star.uno.NamingService", m_rContext),
            UNO_QUERY );
        if ( rNamingService.is() )
        {
            rNamingService->registerObject( "StarOffice.ServiceManager", m_rContext->getServiceManager() );
            rNamingService->registerObject( "StarOffice.ComponentContext", m_rContext );
            rInstance = rNamingService;
        }
    }
    return rInstance;
}

}

// component management stuff...

extern "C"
{
using namespace desktop;

SAL_DLLPUBLIC_EXPORT void * offacc_component_getFactory(char const *pImplementationName, void *pServiceManager, void *)
{
    void* pReturn = nullptr ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            static_cast< XMultiServiceFactory* >(pServiceManager));

        if (desktop::Acceptor::impl_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory.set( cppu::createSingleFactory(
                xServiceManager, desktop::Acceptor::impl_getImplementationName(),
                desktop::Acceptor::impl_getInstance, desktop::Acceptor::impl_getSupportedServiceNames()) );
        }

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    // Return with result of this operation.
    return pReturn ;
}

} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
