/*************************************************************************
 *
 *  $RCSfile: testserver.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-16 09:42:20 $
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
#include <string.h>
#include <assert.h>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

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
using namespace ::com::sun::star::registry;
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
        m_sConnectionDescription( sConnectionDescription ),
        m_sProtocol( sProtocol ),
        m_bReverse( bReverse ),
        m_bLatency( bLatency ),
        m_rSMgr( rSMgr )
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
                    rCallMe->call( OUString::createFromAscii( "reverse call me test finished" ) , 0 );
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


void main( int argc, char *argv[] )
{
//  testserver();

    if( argc < 2 )
    {
        printf( "usage : testserver [-r] connectionstring\n"
                "        -r does a reverse test (server calls client)\n" );
        exit(1 );
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("brdgfctr" )),
            rSMgr ),
                                                     UNO_QUERY );


        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge")),
                         rSMgr );


        Reference < XAcceptor > rAcceptor(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("acceptor")),
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
}
