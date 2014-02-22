/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "sal/config.h"

#include "acceptor.hxx"
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/connection/Acceptor.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace css::bridge;
using namespace css::connection;
using namespace css::container;
using namespace css::lang;
using namespace css::uno;

namespace desktop
{

extern "C" void offacc_workerfunc (void * acc)
{
    ((Acceptor*)acc)->run();
}

Acceptor::Acceptor( const Reference< XComponentContext >& rxContext )
    : m_thread(NULL)
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
    
    m_bDying = true;
    m_cEnable.set();
    osl_joinWithThread(t);
    {
        
        
        
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
            
            SAL_INFO( "desktop.offacc",
                "Acceptor::run waiting for office to come up");
            m_cEnable.wait();
            if (m_bDying) 
                break;
            SAL_INFO( "desktop.offacc",
                "Acceptor::run now enabled and continuing");

            
            Reference< XConnection > rConnection = m_rAcceptor->accept( m_aConnectString );
            
            
            if (! rConnection.is()) break;
            OUString aDescription = rConnection->getDescription();
            SAL_INFO( "desktop.offacc", "Acceptor::run connection " << aDescription );

            
            Reference< XInstanceProvider > rInstanceProvider(
                new AccInstanceProvider(m_rContext, rConnection));
            
            
            
            Reference< XBridge > rBridge = m_rBridgeFactory->createBridge(
                "", m_aProtocol, rConnection, rInstanceProvider);
            osl::MutexGuard g(m_aMutex);
            m_bridges.add(rBridge);
        } catch (const Exception& e) {
            SAL_WARN("desktop.offacc", "caught Exception \"" << e.Message << "\"");
            
            
            
        }
    }
}


void Acceptor::initialize( const Sequence<Any>& aArguments )
    throw( Exception )
{
    
    osl::ClearableMutexGuard aGuard( m_aMutex );
    SAL_INFO( "desktop.offacc", "Acceptor::initialize()" );

    bool bOk = false;

    
    int nArgs = aArguments.getLength();

    
    if (!m_bInit && nArgs > 0 && (aArguments[0] >>= m_aAcceptString))
    {
        SAL_INFO( "desktop.offacc", "Acceptor::initialize string=" << m_aAcceptString );

        
        
        sal_Int32 nIndex1 = m_aAcceptString.indexOf( ';' );
        if (nIndex1 < 0) throw IllegalArgumentException(
            OUString("Invalid accept-string format"), m_rContext, 1);
        m_aConnectString = m_aAcceptString.copy( 0 , nIndex1 ).trim();
        nIndex1++;
        sal_Int32 nIndex2 = m_aAcceptString.indexOf( ';' , nIndex1 );
        if (nIndex2 < 0) nIndex2 = m_aAcceptString.getLength();
        m_aProtocol = m_aAcceptString.copy( nIndex1, nIndex2 - nIndex1 );

        
        m_thread = osl_createThread(offacc_workerfunc, this);
        m_bInit = true;
        bOk = true;
    }

    
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
        throw IllegalArgumentException(
            OUString("invalid initialization"), m_rContext, 1);
    }
}


OUString Acceptor::impl_getImplementationName()
{
    return OUString("com.sun.star.office.comp.Acceptor");
}
OUString Acceptor::getImplementationName()
    throw (RuntimeException)
{
    return Acceptor::impl_getImplementationName();
}
Sequence<OUString> Acceptor::impl_getSupportedServiceNames()
{
    Sequence<OUString> aSequence(1);
    aSequence[0] = "com.sun.star.office.Acceptor";
    return aSequence;
}
Sequence<OUString> Acceptor::getSupportedServiceNames()
    throw (RuntimeException)
{
    return Acceptor::impl_getSupportedServiceNames();
}

sal_Bool Acceptor::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}


Reference< XInterface > Acceptor::impl_getInstance( const Reference< XMultiServiceFactory >& aFactory )
{
    try {
        return (XComponent*) new Acceptor( comphelper::getComponentContext(aFactory) );
    } catch ( const Exception& ) {
        return (XComponent*) NULL;
    }
}


AccInstanceProvider::AccInstanceProvider(const Reference<XComponentContext>& rxContext, const Reference<XConnection>& rConnection)
{
    m_rContext = rxContext;
    m_rConnection = rConnection;
}

AccInstanceProvider::~AccInstanceProvider()
{
}

Reference<XInterface> AccInstanceProvider::getInstance (const OUString& aName )
        throw ( NoSuchElementException )
{

    Reference<XInterface> rInstance;

    if ( aName.equalsAscii( "StarOffice.ServiceManager" ) )
    {
        rInstance = Reference< XInterface >( m_rContext->getServiceManager() );
    }
    else if(aName.equalsAscii( "StarOffice.ComponentContext" ) )
    {
        rInstance = m_rContext;
    }
    else if ( aName.equalsAscii("StarOffice.NamingService" ) )
    {
        Reference< XNamingService > rNamingService(
            m_rContext->getServiceManager()->createInstanceWithContext("com.sun.star.uno.NamingService", m_rContext),
            UNO_QUERY );
        if ( rNamingService.is() )
        {
            rNamingService->registerObject(
                OUString("StarOffice.ServiceManager" ), m_rContext->getServiceManager() );
            rNamingService->registerObject(
                OUString("StarOffice.ComponentContext" ), m_rContext );
            rInstance = rNamingService;
        }
    }
    return rInstance;
}

}



extern "C"
{
using namespace desktop;

SAL_DLLPUBLIC_EXPORT void * SAL_CALL offacc_component_getFactory(char const *pImplementationName, void *pServiceManager, void *)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (desktop::Acceptor::impl_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = Reference< XSingleServiceFactory >( cppu::createSingleFactory(
                xServiceManager, desktop::Acceptor::impl_getImplementationName(),
                desktop::Acceptor::impl_getInstance, desktop::Acceptor::impl_getSupportedServiceNames()) );
        }

        
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }

    
    return pReturn ;
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
