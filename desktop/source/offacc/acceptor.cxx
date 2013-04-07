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


#include "acceptor.hxx"
#include <com/sun/star/bridge/BridgeFactory.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>

namespace desktop
{

extern "C" void workerfunc (void * acc)
{
    ((Acceptor*)acc)->run();
}

Mutex Acceptor::m_aMutex;

Acceptor::Acceptor( const Reference< XMultiServiceFactory >& rFactory )
    : m_thread(NULL)
    , m_aAcceptString()
    , m_aConnectString()
    , m_aProtocol()
    , m_bInit(sal_False)
    , m_bDying(false)
{
    m_rSMgr = rFactory;
    // get component context
    m_rContext = comphelper::getComponentContext(m_rSMgr);
    m_rAcceptor = Reference< XAcceptor > (m_rSMgr->createInstance(
        OUString("com.sun.star.connection.Acceptor" )),
        UNO_QUERY );
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
    {
        // Make the final state of m_bridges visible to this thread (since
        // m_thread is joined, the code that follows is the only one left
        // accessing m_bridges):
        osl::MutexGuard g(m_aMutex);
    }
    for (;;) {
        com::sun::star::uno::Reference< com::sun::star::bridge::XBridge > b(
            m_bridges.remove());
        if (!b.is()) {
            break;
        }
        com::sun::star::uno::Reference< com::sun::star::lang::XComponent >(
            b, com::sun::star::uno::UNO_QUERY_THROW)->dispose();
    }
}

void SAL_CALL Acceptor::run()
{
    while ( m_rAcceptor.is() )
    {
        RTL_LOGFILE_CONTEXT( aLog, "desktop (lo119109) Acceptor::run" );
        try
        {
            // wait until we get enabled
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109)"\
                "Acceptor::run waiting for office to come up");
            m_cEnable.wait();
            if (m_bDying) //see destructor
                break;
            RTL_LOGFILE_CONTEXT_TRACE( aLog, "desktop (lo119109)"\
                "Acceptor::run now enabled and continuing");

            // accept connection
            Reference< XConnection > rConnection = m_rAcceptor->accept( m_aConnectString );
            // if we return without a valid connection we mus assume that the acceptor
            // is destructed so we break out of the run method terminating the thread
            if (! rConnection.is()) break;
            OUString aDescription = rConnection->getDescription();
            RTL_LOGFILE_CONTEXT_TRACE1( aLog, "desktop (lo119109) Acceptor::run connection %s",
                OUStringToOString(aDescription, RTL_TEXTENCODING_ASCII_US).getStr());

            // create instanceprovider for this connection
            Reference< XInstanceProvider > rInstanceProvider(
                (XInstanceProvider*)new AccInstanceProvider(m_rSMgr, rConnection));
            // create the bridge. The remote end will have a reference to this bridge
            // thus preventing the bridge from being disposed. When the remote end releases
            // the bridge, it will be destructed.
            Reference< XBridge > rBridge = m_rBridgeFactory->createBridge(
                OUString() ,m_aProtocol ,rConnection ,rInstanceProvider );
            osl::MutexGuard g(m_aMutex);
            m_bridges.add(rBridge);
        } catch (const Exception& e) {
            SAL_WARN("desktop", "caught Exception \"" << e.Message << "\"");
            // connection failed...
            // something went wrong during connection setup.
            // just wait for a new connection to accept
        }
    }
}

// XInitialize
void SAL_CALL Acceptor::initialize( const Sequence<Any>& aArguments )
    throw( Exception )
{
    // prevent multiple initialization
    ClearableMutexGuard aGuard( m_aMutex );
    RTL_LOGFILE_CONTEXT( aLog, "destop (lo119109) Acceptor::initialize()" );

    sal_Bool bOk = sal_False;

    // arg count
    int nArgs = aArguments.getLength();

    // not yet initialized and acceptstring
    if (!m_bInit && nArgs > 0 && (aArguments[0] >>= m_aAcceptString))
    {
        RTL_LOGFILE_CONTEXT_TRACE1( aLog, "desktop (lo119109) Acceptor::initialize string=%s",
            OUStringToOString(m_aAcceptString, RTL_TEXTENCODING_ASCII_US).getStr());

        // get connect string and protocol from accept string
        // "<connectString>;<protocol>"
        sal_Int32 nIndex1 = m_aAcceptString.indexOf( (sal_Unicode) ';' );
        if (nIndex1 < 0) throw IllegalArgumentException(
            OUString("Invalid accept-string format"), m_rContext, 1);
        m_aConnectString = m_aAcceptString.copy( 0 , nIndex1 ).trim();
        nIndex1++;
        sal_Int32 nIndex2 = m_aAcceptString.indexOf( (sal_Unicode) ';' , nIndex1 );
        if (nIndex2 < 0) nIndex2 = m_aAcceptString.getLength();
        m_aProtocol = m_aAcceptString.copy( nIndex1, nIndex2 - nIndex1 );

        // start accepting in new thread...
        m_thread = osl_createThread(workerfunc, this);
        m_bInit = sal_True;
        bOk = sal_True;
    }

    // do we want to enable accepting?
    sal_Bool bEnable = sal_False;
    if (((nArgs == 1 && (aArguments[0] >>= bEnable)) ||
         (nArgs == 2 && (aArguments[1] >>= bEnable))) &&
        bEnable )
    {
        m_cEnable.set();
        bOk = sal_True;
    }

    if (!bOk)
    {
        throw IllegalArgumentException(
            OUString("invalid initialization"), m_rContext, 1);
    }
}

// XServiceInfo
const sal_Char *Acceptor::serviceName = "com.sun.star.office.Acceptor";
const sal_Char *Acceptor::implementationName = "com.sun.star.office.comp.Acceptor";
const sal_Char *Acceptor::supportedServiceNames[] = {"com.sun.star.office.Acceptor", NULL};
OUString Acceptor::impl_getImplementationName()
{
    return OUString::createFromAscii( implementationName );
}
OUString SAL_CALL Acceptor::getImplementationName()
    throw (RuntimeException)
{
    return Acceptor::impl_getImplementationName();
}
Sequence<OUString> Acceptor::impl_getSupportedServiceNames()
{
    Sequence<OUString> aSequence;
    for (int i=0; supportedServiceNames[i]!=NULL; i++) {
        aSequence.realloc(i+1);
        aSequence[i]=(OUString::createFromAscii(supportedServiceNames[i]));
    }
    return aSequence;
}
Sequence<OUString> SAL_CALL Acceptor::getSupportedServiceNames()
    throw (RuntimeException)
{
    return Acceptor::impl_getSupportedServiceNames();
}

sal_Bool Acceptor::supportsService(OUString const & ServiceName)
    throw (css::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

// Factory
Reference< XInterface > Acceptor::impl_getInstance( const Reference< XMultiServiceFactory >& aFactory )
{
    try {
        return (XComponent*) new Acceptor( aFactory );
    } catch ( const Exception& ) {
        return (XComponent*) NULL;
    }
}

// InstanceProvider
AccInstanceProvider::AccInstanceProvider(const Reference<XMultiServiceFactory>& aFactory, const Reference<XConnection>& rConnection)
{
    m_rSMgr = aFactory;
    m_rConnection = rConnection;
}

AccInstanceProvider::~AccInstanceProvider()
{
}

Reference<XInterface> SAL_CALL AccInstanceProvider::getInstance (const OUString& aName )
        throw ( NoSuchElementException )
{

    Reference<XInterface> rInstance;

    if ( aName.compareToAscii( "StarOffice.ServiceManager" ) == 0)
    {
        rInstance = Reference< XInterface >( m_rSMgr );
    }
    else if(aName.compareToAscii( "StarOffice.ComponentContext" ) == 0 )
    {
        rInstance = comphelper::getComponentContext( m_rSMgr );
    }
    else if ( aName.compareToAscii("StarOffice.NamingService" ) == 0 )
    {
        Reference< XNamingService > rNamingService(
            m_rSMgr->createInstance( OUString("com.sun.star.uno.NamingService" )),
            UNO_QUERY );
        if ( rNamingService.is() )
        {
            rNamingService->registerObject(
                OUString("StarOffice.ServiceManager" ), m_rSMgr );
            rNamingService->registerObject(
                OUString("StarOffice.ComponentContext" ), comphelper::getComponentContext( m_rSMgr ));
            rInstance = rNamingService;
        }
    }
    return rInstance;
}

}

// component management stuff...
// ----------------------------------------------------------------------------
extern "C"
{
using namespace desktop;

SAL_DLLPUBLIC_EXPORT void * SAL_CALL offacc_component_getFactory(const sal_Char *pImplementationName, void *pServiceManager, void *)
{
    void* pReturn = NULL ;
    if  ( pImplementationName && pServiceManager )
    {
        // Define variables which are used in following macros.
        Reference< XSingleServiceFactory > xFactory;
        Reference< XMultiServiceFactory >  xServiceManager(
            reinterpret_cast< XMultiServiceFactory* >(pServiceManager));

        if (Acceptor::impl_getImplementationName().equalsAscii( pImplementationName ) )
        {
            xFactory = Reference< XSingleServiceFactory >( cppu::createSingleFactory(
                xServiceManager, Acceptor::impl_getImplementationName(),
                Acceptor::impl_getInstance, Acceptor::impl_getSupportedServiceNames()) );
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
