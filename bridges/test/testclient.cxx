/*************************************************************************
 *
 *  $RCSfile: testclient.cxx,v $
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
#include <string.h>
#include <assert.h>

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


#ifdef SOLARIS
extern "C" void ChangeGlobalInit();
#endif


#ifdef UNX
#define REG_PREFIX      "lib"
#define DLL_POSTFIX     ".so"
#else
#define REG_PREFIX      ""
#define DLL_POSTFIX     ".dll"
#endif


#include <vos/socket.hxx>

void doPerformanceTest( const Reference < XPerformanceTest > & xBench )
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

void testLatency( const Reference < XConnection > &r , sal_Bool bReply )
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

void main( int argc, char *argv[] )
{
    sal_Bool bUseNew = ( 3 == argc );
#ifdef SOLARIS
    ChangeGlobalInit(); // Switch on threads !
#endif
    if( argc < 2 )
    {
        printf(
            "usage : testclient [-r] connectionstring\n"
            "        -r reverse call me test (server calls client)"
             );
        return;
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
                                 OUString( RTL_CONSTASCII_USTRINGPARAM("connectr")),
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
                                     OUString( RTL_CONSTASCII_USTRINGPARAM("iiopbrdg")),
                                     rSMgr );

                    Reference < XBridgeFactory > rFactory(
                        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
                                         OUString( RTL_CONSTASCII_USTRINGPARAM("brdgfctr")),
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
            catch( Exception &e )
            {
                OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
                printf( "Login failed, got an Exception !\n%s\n" , o.pData->buffer );
            }


        Reference < XComponent > rComp( rSMgr , UNO_QUERY );
        rComp->dispose();
    }
    printf( "Closed\n" );
}

