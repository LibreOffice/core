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

#include <osl/time.h>

#include <osl/mutex.hxx>
#include <osl/thread.h>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/connection/XConnector.hpp>

#include <com/sun/star/bridge/XBridgeFactory.hpp>

#include <com/sun/star/uno/XNamingService.hpp>
#include <com/sun/star/io/Pipe.hpp>

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
    Reference < XPipe > rPipe( Pipe::create(comphelper::ComponentContext(rSmgr).getUNOContext()), UNO_QUERY_THROW );

    {
        Sequence < sal_Int8 > seq( 10 );
        seq.getArray()[0] = 42;
        rPipe->writeBytes( seq );
    }


    {
        Sequence < sal_Int8 > seq;
        if( ! ( rPipe->available() == 10) )
            printf( "wrong bytes available\n" );
        if( ! ( rPipe->readBytes( seq , 10 ) == 10 ) )
            printf( "wrong bytes read\n" );
        if( ! ( 42 == seq.getArray()[0] ) )
            printf( "wrong element in sequence\n" );

//          OSL_ASSERT( 0 );
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

        if ( strlen( pcText ) < sizeof(pcText)-1 )
            strcat( pcText , " " ); // #100211# - checked

        rText->insertString( rRange , OUString::createFromAscii( pcText ) , sal_False );
    }
}

void testDocument( const Reference < XMultiServiceFactory > & rSmgr )
{
    Reference < XComponentLoader > rLoader(
        rSmgr->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ))),
        UNO_QUERY );

    OSL_ASSERT( rLoader.is() );

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
            OUString( RTL_CONSTASCII_USTRINGPARAM( "StarOffice.ServiceManager" )) );

        Reference < XMultiServiceFactory > rSmgr( rXsmgr , UNO_QUERY );
        if( rSmgr.is() )
        {
            printf( "got the remote service manager !\n" );
            testPipe( rSmgr );
            testDocument( rSmgr );
        }
    }
}


int main( int argc, char *argv[] )
{
    if( argc < 2 )
    {
        printf( "usage : testclient host:port" );
        return 0;
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
                             OUString( RTL_CONSTASCII_USTRINGPARAM("connector.uno" SAL_DLLEXTENSION)),
                             rSMgr ),
            UNO_QUERY );

        createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.Bridge.iiop")),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("remotebridge.uno" SAL_DLLEXTENSION)),
                         rSMgr );

        Reference < XBridgeFactory > rFactory(
            createComponent( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.BridgeFactory")),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("bridgefac.uno" SAL_DLLEXTENSION)),
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
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
