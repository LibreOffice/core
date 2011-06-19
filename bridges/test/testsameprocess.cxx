/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"
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
using namespace ::rtl;
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

    while ( sal_True )
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
                    OUString( RTL_CONSTASCII_USTRINGPARAM("iiop")) ,
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
            OUString( RTL_CONSTASCII_USTRINGPARAM( "client.rdb" )  ) );

        Reference < XConnector > rConnector(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("connector.uno" SAL_DLLEXTENSION)),
                             rSMgr ),
            UNO_QUERY );

        Reference < XAcceptor > rAcceptor(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("acceptor.uno" SAL_DLLEXTENSION)),
                             rSMgr ),
            UNO_QUERY );

        // just ensure that it is registered
//      createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
//                       OUString( RTL_CONSTASCII_USTRINGPARAM("iiopbrdg" SAL_DLLEXTENSION)),
//                       rSMgr );

        Reference < XBridgeFactory > rFactory(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("bridgefac.uno" SAL_DLLEXTENSION)),
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
                    OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")),
                    OUString( RTL_CONSTASCII_USTRINGPARAM("iiop")),
                    rConnection,
                    Reference < XInstanceProvider > () );

                Reference < XInterface > rInitialObject
                    = rBridge->getInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("bla")) );

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
                OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")) );
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


        rComp = Reference < XComponent > ( rSMgr , UNO_QUERY );
        rComp->dispose();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
