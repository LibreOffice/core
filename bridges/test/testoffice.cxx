/*************************************************************************
 *
 *  $RCSfile: testoffice.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-14 09:25:39 $
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

#ifndef _OSL_TIME_H_
#include <osl/time.h>
#endif

#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>

#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <com/sun/star/text/XTextDocument.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/frame/XComponentLoader.hpp>

#include <cppuhelper/weak.hxx>

#include <test/XTestFactory.hpp>

using namespace ::test;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;

#include "testcomp.h"

#ifdef SAL_W32
#include <conio.h>
#endif


void mygetchar()
{
#ifdef SAL_W32
    _getch();
#else
    getchar();
#endif
}


void testPipe( const Reference < XMultiServiceFactory > & rSmgr )
{
    Reference < XOutputStream > rOut(
        rSmgr->createInstance( OUString::createFromAscii( "com.sun.star.io.Pipe" ) ),
        UNO_QUERY );

    assert( rOut.is() );

    {
        Sequence < sal_Int8 > seq( 10 );
        seq.getArray()[0] = 42;
        rOut->writeBytes( seq );
    }


    {
        Sequence < sal_Int8 > seq;
        Reference < XInputStream > rIn( rOut , UNO_QUERY );
        if( ! ( rIn->available() == 10) )
            printf( "wrong bytes available\n" );
        if( ! ( rIn->readBytes( seq , 10 ) == 10 ) )
            printf( "wrong bytes read\n" );
        if( ! ( 42 == seq.getArray()[0] ) )
            printf( "wrong element in sequence\n" );

//          assert( 0 );
    }
}
#include<stdio.h>
#include<string.h>

void testWriter(  const Reference < XComponent > & rCmp )
{

    Reference< XTextDocument > rTextDoc( rCmp , UNO_QUERY );

    Reference< XText > rText = rTextDoc->getText();
    Reference< XTextCursor > rCursor = rText->createTextCursor();
    Reference< XTextRange > rRange ( rCursor , UNO_QUERY );

    char pcText[1024];
    pcText[0] = 0;
    printf( "pleast type any text\n" );
    while( sal_True )
    {
        scanf( "%s" , pcText );

        if( !strcmp( pcText , "end" ) )
        {
            break;
        }

        strcat( pcText , " " );
        rText->insertString( rRange , OUString::createFromAscii( pcText ) , sal_False );
    }
}

void testDocument( const Reference < XMultiServiceFactory > & rSmgr )
{
    Reference < XComponentLoader > rLoader(
        rSmgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ))),
        UNO_QUERY );

    assert( rLoader.is() );

    sal_Char *urls[] = {
        "private:factory/swriter",
        "private:factory/scalc",
        "private:factory/sdraw",
        "http://www.heise.de",
        "file://h|/remote_interfaces.sdw"
    };

    sal_Char *docu[]= {
        "a new writer document ...\n",
        "a new calc document ...\n",
        "a new draw document ...\n",
        "www.heise.de\n",
        "the remote_interfaces.sdw doc\n"
    };

    sal_Int32 i;
    for( i = 0 ; i < 1 ; i ++ )
    {
        printf( "press any key to open %s\n" , docu[i] );
        mygetchar();

        Reference< XComponent > rComponent =
            rLoader->loadComponentFromURL(
                OUString::createFromAscii( urls[i] ) ,
                OUString( RTL_CONSTASCII_USTRINGPARAM("_blank")),
                0 ,
                Sequence < ::com::sun::star::beans::PropertyValue >() );

        testWriter( rComponent );
        printf( "press any key to close the document\n" );
        mygetchar();
        rComponent->dispose();
    }

}

void doSomething( const  Reference < XInterface > &r )
{
    Reference < XNamingService > rName( r, UNO_QUERY );
    if( rName.is() )
    {
        printf( "got the remote naming service !\n" );
        Reference < XInterface > rXsmgr = rName->getRegisteredObject(
            OUString::createFromAscii( "StarOffice.ServiceManager" ) );

        Reference < XMultiServiceFactory > rSmgr( rXsmgr , UNO_QUERY );
        if( rSmgr.is() )
        {
            printf( "got the remote service manager !\n" );
            testPipe( rSmgr );
            testDocument( rSmgr );
        }
    }
}


void main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
        printf( "usage : testclient host:port" );
        exit( 1 );
    }

    OUString sConnectionString;
    OUString sProtocol;
    sal_Bool bLatency = sal_False;
    sal_Bool bReverse = sal_False;
    parseCommandLine( argv , &sConnectionString , &sProtocol , &bLatency , &bReverse );
    {
        Reference< XMultiServiceFactory > rSMgr = createRegistryServiceFactory(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "client.rdb" )  ) );

        // just ensure that it is registered

        Reference < XConnector > rConnector(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("connectr")),
                             rSMgr ),
            UNO_QUERY );

        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge")),
                         rSMgr );

        Reference < XBridgeFactory > rFactory(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("brdgfctr")),
                             rSMgr ),
            UNO_QUERY );

        try
        {
            if( rFactory.is() && rConnector.is() )
            {
                Reference < XConnection > rConnection =
                    rConnector->connect( sConnectionString );

                Reference < XBridge > rBridge = rFactory->createBridge(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("bla blub")),
                    sProtocol,
                    rConnection,
                    Reference < XInstanceProvider > () );

                Reference < XInterface > rInitialObject
                    = rBridge->getInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("NamingService")) );

                if( rInitialObject.is() )
                {
                    printf( "got the remote object\n" );
                    doSomething( rInitialObject );
                }
                TimeValue value={2,0};
                osl_waitThread( &value );
            }
        }
        catch (... ) {
            printf( "Exception thrown\n" );
        }

        Reference < XComponent > rComp( rSMgr , UNO_QUERY );
        rComp->dispose();
    }
    //_getch();
}
