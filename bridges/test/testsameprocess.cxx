/*************************************************************************
 *
 *  $RCSfile: testsameprocess.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <assert.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/bridge/XBridgeFactory.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/weak.hxx>

#include <test/XTestFactory.hpp>

#include <vos/thread.hxx>

using namespace ::test;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::registry;

#ifdef SAL_W32
#include <conio.h>
#endif

#include "testcomp.h"
#include "osl/mutex.h"

/*********
 *
 ********/

class MyThread :
    public OThread
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


void main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
        printf( "usage : testsamprocess host:port\n" );
        exit(1 );
    }

    {
#if SUPD>582
        Reference< XMultiServiceFactory > rSMgr = createRegistryServiceFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "client.rdb" )  ) );
#else
        Reference< XMultiServiceFactory > rSMgr = createRegistryServiceFactory( "client.rdb" );
#endif


        Reference < XConnector > rConnector(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("connectr")),
                             rSMgr ),
            UNO_QUERY );

        Reference < XAcceptor > rAcceptor(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("acceptor")),
                             rSMgr ),
            UNO_QUERY );

        // just ensure that it is registered
        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("iiopbrdg")),
                         rSMgr );

        Reference < XBridgeFactory > rFactory(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("brdgfctr")),
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
                TimeValue value={2,0};
                osl_waitThread( &value );
            }

            Reference < XBridge > rBridge = rFactory->getBridge(
                OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")) );
            assert( ! rBridge.is() );

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
}
