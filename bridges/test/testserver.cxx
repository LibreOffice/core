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
#include <string.h>
#include <osl/time.h>

#include <osl/mutex.hxx>
#include <osl/conditn.h>

#include <osl/thread.hxx>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/connection/XAcceptor.hpp>
#include <com/sun/star/connection/XConnection.hpp>

#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>


#include <test/XTestFactory.hpp>

#include <cppuhelper/weak.hxx>

using namespace ::test;
using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::connection;
#include "testcomp.h"
#ifdef SAL_W32
#include <conio.h>
#endif

/*********
 *
 ********/



class MyThread :
    public Thread
{
public:
    MyThread( const Reference< XAcceptor > &r ,
              const Reference< XBridgeFactory > &rFactory,
              const Reference< XMultiServiceFactory > &rSMgr,
              const OUString &sConnectionDescription,
              const OUString &sProtocol,
              sal_Bool bReverse,
              sal_Bool bLatency ) :
        m_rAcceptor( r ),
        m_rBridgeFactory ( rFactory ),
        m_rSMgr( rSMgr ),
        m_sConnectionDescription( sConnectionDescription ),
        m_sProtocol( sProtocol ),
        m_bReverse( bReverse ),
        m_bLatency( bLatency )
        {}
    virtual void SAL_CALL run();

    void latencyTest( const Reference< XConnection > &r );

private:
    Reference < XAcceptor > m_rAcceptor;
    Reference < XBridgeFactory > m_rBridgeFactory;
    Reference < XMultiServiceFactory > m_rSMgr;
    OUString m_sConnectionDescription;
    OUString m_sProtocol;
    sal_Bool m_bReverse;
    sal_Bool m_bLatency;
};


void MyThread::latencyTest( const Reference< XConnection > &r )
{
    Sequence < sal_Int8 > s;
    while( 12 == r->read( s , 12 ) )
    {
        r->read( s , 188 );
        s = Sequence < sal_Int8 >(60);
        r->write( s );
    }
}

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
            if( m_bLatency )
            {
                latencyTest( rConnection );
            }
            else
            {

                Reference < XBridge > rBridge =
                    m_rBridgeFactory->createBridge(
                        OUString() ,
                        m_sProtocol,
                        rConnection ,
                        (XInstanceProvider * ) new OInstanceProvider(m_rSMgr) );


                if( m_bReverse )
                {
                    printf( "doing reverse callme test (test is ok, when on each line a +- appears\n" );
                    Reference < XInterface > r = rBridge->getInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("blubber"  )));
                    Reference < XTestFactory > rFactory( r , UNO_QUERY );
                    Reference < XCallMe > rCallMe = rFactory->createCallMe();

                    for( sal_Int32 i = 0 ; i < 1  ; i ++ )
                    {
                        rCallMe->callOneway(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("my test string")) , 2 );
                    }
                    printf( "all oneway are send\n" );
                    rCallMe->call( OUString( RTL_CONSTASCII_USTRINGPARAM( "reverse call me test finished" )) , 0 );
                printf( "revers callme test finished\n" );
                }
            }
        }
        catch ( Exception & e )
        {
            printf( "Exception was thrown by acceptor \n" );
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            printf( "%s\n" , o.getStr() );
            break;
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
//  testserver();

    if( argc < 2 )
    {
        printf( "usage : testserver [-r] connectionstring\n"
                "        -r does a reverse test (server calls client)\n" );
        return 0;
    }

    OUString sConnectionString;
    OUString sProtocol;
    sal_Bool bReverse = sal_False;
    sal_Bool bLatency = sal_False;

    parseCommandLine( argv , &sConnectionString , &sProtocol , &bLatency , &bReverse );

    {
        Reference< XMultiServiceFactory > rSMgr = createRegistryServiceFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "server.rdb" )  ) );

        Reference < XBridgeFactory > rBridgeFactory ( createComponent(
            OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
            OUString( RTL_CONSTASCII_USTRINGPARAM("bridgefac.uno" SAL_DLLEXTENSION )),
            rSMgr ),
                                                     UNO_QUERY );


        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge.uno" SAL_DLLEXTENSION)),
                         rSMgr );


        Reference < XAcceptor > rAcceptor(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("acceptor.uno" SAL_DLLEXTENSION)),
                             rSMgr ) ,
            UNO_QUERY );

        MyThread thread( rAcceptor ,
                         rBridgeFactory,
                         rSMgr,
                         sConnectionString,
                         sProtocol,
                         bReverse,
                         bLatency);
        thread.create();

#ifdef SAL_W32
        _getch();
#elif  SOLARIS
        getchar();
#elif LINUX
        TimeValue value={360,0};
        osl_waitThread( &value );
#endif
        printf( "Closing...\n" );

        rAcceptor->stopAccepting();
        thread.join();

        printf( "Closed\n" );

        Reference < XComponent > rComp2( rBridgeFactory , UNO_QUERY );
        rComp2->dispose();
        Reference < XComponent > rComp( rSMgr, UNO_QUERY );
        rComp->dispose();
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
