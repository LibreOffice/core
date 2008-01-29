/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testclient.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:40:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"
#include <string.h>

#if OSL_DEBUG_LEVEL == 0
#undef NDEBUG
#define NDEBUG
#endif
#include <assert.h>

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <osl/mutex.hxx>
#include <osl/module.h>
#include <osl/thread.h>
#include <osl/conditn.h>
#include <osl/diagnose.h>

#include <uno/mapping.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMain.hpp>

#include <com/sun/star/test/performance/XPerformanceTest.hpp>

#include <cppuhelper/weak.hxx>

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#include <test/XTestFactory.hpp>


using namespace ::test;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::test::performance;

#include "testcomp.h"


void doPerformanceTest( const Reference < XPerformanceTest > & /* xBench */)
{
    printf( "not implemented\n" );
//      sal_Int32 i,nLoop = 2000;
//      sal_Int32 tStart, tEnd , tEnd2;
//      //------------------------------------
//      // oneway calls
//      i = nLoop;
//      tStart = GetTickCount();
//      while (i--)
//          xBench->async();
//      tEnd = GetTickCount();
//      xBench->sync();
//      tEnd2 = GetTickCount();
//      printf( "%d %d %d\n" , nLoop, tEnd - tStart , tEnd2 -tStart );
//      // synchron calls
//      i = nLoop;
//      tStart = GetTickCount();
//      while (i--)
//          xBench->sync();
//      tEnd = GetTickCount();
//      printf( "%d %d \n" , nLoop, tEnd - tStart );

}

void testLatency( const Reference < XConnection > &r , sal_Bool /* bReply */)
{
    sal_Int32 nLoop = 10000;
    TimeValue aStartTime, aEndTime;
    osl_getSystemTime( &aStartTime );

    sal_Int32 i;
    for( i = 0 ; i < nLoop ; i++ )
    {
        Sequence< sal_Int8 >  s1( 200 );
        r->write( s1 );
        r->read( s1 , 12 );
        r->read( s1 , 48 );
    }
    osl_getSystemTime( &aEndTime );

    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

    printf( "System latency per call : %g\n" , (( fEnd-fStart )/2.) / ((double)(nLoop)) );
}

int main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
        printf(
            "usage : testclient [-r] connectionstring\n"
            "        -r reverse call me test (server calls client)"
             );
        return 0;
    }

    OUString sConnectionString;
    OUString sProtocol;
    sal_Bool bLatency = sal_False;
    sal_Bool bReverse = sal_False;

    parseCommandLine( argv , &sConnectionString , &sProtocol , &bLatency , &bReverse );

    {
        Reference< XMultiServiceFactory > rSMgr = createRegistryServiceFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM("client.rdb")) );


          Reference < XConnector > rConnector(
                createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector")),
                                 OUString( RTL_CONSTASCII_USTRINGPARAM("connector.uno" SAL_DLLEXTENSION)),
                                 rSMgr ),
                UNO_QUERY );


            try
            {
                Reference < XConnection > rConnection =
                    rConnector->connect( sConnectionString );

                printf( "%s\n" , OUStringToOString( rConnection->getDescription(),
                                                    RTL_TEXTENCODING_ASCII_US ).pData->buffer );


                if( bLatency )
                {
                    testLatency( rConnection , sal_False );
                    testLatency( rConnection , sal_True );
                }
                else
                {
                    // just ensure that it is registered
                    createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge.uno" SAL_DLLEXTENSION)),
                                     rSMgr );

                    Reference < XBridgeFactory > rFactory(
                        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("bridgefac.uno" SAL_DLLEXTENSION)),
                                         rSMgr ),
                        UNO_QUERY );

                    if( rFactory.is() )
                    {

                        Reference < XBridge > rBridge = rFactory->createBridge(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")),
                            sProtocol,
                            rConnection,
                            new OInstanceProvider );
                        {
                            // test the factory
                              Reference < XBridge > rBridge2 = rFactory->getBridge( OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")) );
                              assert( rBridge2.is() );
                              assert( rBridge2->getDescription() == rBridge->getDescription( ) );
                              assert( rBridge2->getName() == rBridge->getName() );
                              assert( rBridge2 == rBridge );
                        }


                        Reference < XInterface > rInitialObject = rBridge->getInstance(
                            OUString( RTL_CONSTASCII_USTRINGPARAM("bridges-testobject")) );

                        if( rInitialObject.is() )
                        {
                            printf( "got the remote object\n" );
                            if( ! bReverse )
                            {
  //                                Reference < XComponent > rPerfTest( rInitialObject , UNO_QUERY );
//                                  if( rPerfTest.is() )
//                                  {
//  //                                      doPerformanceTest( rPerfTest );
//                                  }
//                                  else
//                                  {
                                    testRemote( rInitialObject );
//                                  }
                            }
                        }
//                          Reference < XComponent > rComp( rBridge , UNO_QUERY );
//                          rComp->dispose();

                        rInitialObject = Reference < XInterface > ();
                        printf( "Waiting...\n" );
                        TimeValue value={bReverse ?1000 :2,0};
                        osl_waitThread( &value );
                        printf( "Closing...\n" );
                    }

                    Reference < XBridge > rBridge = rFactory->getBridge( OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")) );
//                      assert( ! rBridge.is() );
                }

            }
            catch( DisposedException & e )
            {
                OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
                printf( "A remote object reference became invalid\n%s\n" , o.pData->buffer );
            }
            catch( Exception &e )
            {
                OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
                printf( "Login failed, got an Exception !\n%s\n" , o.pData->buffer );
            }


        Reference < XComponent > rComp( rSMgr , UNO_QUERY );
        rComp->dispose();
    }
    printf( "Closed\n" );
    return 0;
}

