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

#include <osl/time.h>

#include <osl/mutex.hxx>
#include <osl/thread.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/weak.hxx>

#include <test/XTestFactory.hpp>


using namespace ::test;
using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::connection;

#ifdef SAL_W32
#include <conio.h>
#endif

#include "testcomp.h"
#include "osl/mutex.h"

/*********
 *
 ********/

class MyThread :
    public Thread
{
public:
    MyThread( const Reference< XAcceptor > &r ,
              const Reference< XBridgeFactory > &rFactory,
              const OUString &sConnectionDescription) :
        m_rAcceptor( r ),
        m_rBridgeFactory ( rFactory ),
        m_sConnectionDescription( sConnectionDescription )
        {}
    virtual void SAL_CALL run();

private:
    Reference < XAcceptor > m_rAcceptor;
    Reference < XBridgeFactory > m_rBridgeFactory;
    OUString m_sConnectionDescription;
};



void MyThread::run()
{

    while ( true )
    {
        try
        {
            Reference < XConnection > rConnection =
                m_rAcceptor->accept( m_sConnectionDescription );

            if( ! rConnection.is() )
            {
                break;
            }

            Reference < XBridge > rBridge =
                m_rBridgeFactory->createBridge(
                    OUString() ,
                    OUString("iiop") ,
                    rConnection ,
                    (XInstanceProvider * ) new OInstanceProvider );


        }
        catch ( ... )
        {
            printf( "Exception was thrown by acceptor thread\n" );
            break;
        }
    }
}


int main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
        printf( "usage : testsamprocess host:port\n" );
        return 0;
    }

    {
        Reference< XMultiServiceFactory > rSMgr = createRegistryServiceFactory(
            OUString( "client.rdb" ) );

        Reference < XConnector > rConnector(
            createComponent( OUString("com.sun.star.connection.Connector"),
                             OUString( "connector.uno" SAL_DLLEXTENSION),
                             rSMgr ),
            UNO_QUERY );

        Reference < XAcceptor > rAcceptor(
            createComponent( OUString("com.sun.star.connection.Acceptor"),
                             OUString( "acceptor.uno" SAL_DLLEXTENSION),
                             rSMgr ),
            UNO_QUERY );

        // just ensure that it is registered
//      createComponent( OUString("com.sun.star.bridge.Bridge.iiop"),
//                       OUString( "iiopbrdg" SAL_DLLEXTENSION),
//                       rSMgr );

        Reference < XBridgeFactory > rFactory(
            createComponent( OUString("com.sun.star.bridge.BridgeFactory"),
                             OUString( "bridgefac.uno" SAL_DLLEXTENSION),
                             rSMgr ),
            UNO_QUERY );


        MyThread threadAcceptor( rAcceptor , rFactory , OUString::createFromAscii( argv[1] ) );

        threadAcceptor.create();
        TimeValue value={2,0};
        osl_waitThread( &value );

        try
        {
            Reference < XConnection > rConnection =
                rConnector->connect( OUString::createFromAscii( argv[1] ) );

            printf( "%s\n" , OUStringToOString( rConnection->getDescription(),
                                                RTL_TEXTENCODING_ASCII_US ).pData->buffer );

            if( rFactory.is() )
            {

                Reference < XBridge > rBridge = rFactory->createBridge(
                    OUString("bla blub"),
                    OUString("iiop"),
                    rConnection,
                    Reference < XInstanceProvider > () );

                Reference < XInterface > rInitialObject
                    = rBridge->getInstance( OUString("bla") );

                if( rInitialObject.is() )
                {
                    printf( "got the remote object\n" );
                    testRemote( rInitialObject );
                }
                printf( "Closing...\n" );
                TimeValue timeValue={2,0};
                osl_waitThread( &timeValue );
            }

            Reference < XBridge > rBridge = rFactory->getBridge(
                OUString("bla blub") );
            OSL_ASSERT( ! rBridge.is() );

        }
        catch( Exception & )
        {
            printf( "Login failed, got an Exception !\n" );
        }

        rAcceptor->stopAccepting();
        threadAcceptor.join();

        Reference < XComponent > rComp( rFactory , UNO_QUERY );
        rComp->dispose();


        rComp.set( rSMgr , UNO_QUERY );
        rComp->dispose();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
