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
#include "precompiled_io.hxx"
#include <stdio.h>
#include <osl/time.h>

#include <osl/diagnose.h>
#include <osl/thread.hxx>

#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#include <com/sun/star/registry/XImplementationRegistration.hpp>

#include <com/sun/star/connection/XConnector.hpp>
#include <com/sun/star/connection/XAcceptor.hpp>

using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::connection;


class MyThread :
    public Thread
{
public:
    MyThread( const Reference< XAcceptor > &r , const OUString & sConnectionDescription) :
        m_rAcceptor( r ),
        m_sConnectionDescription( sConnectionDescription )
        {}
    virtual void SAL_CALL run();

    Reference < XAcceptor > m_rAcceptor;
private:
    Reference < XConnection > m_rConnection;
    OUString m_sConnectionDescription;
};

void doWrite( const Reference < XConnection > &r )
{
    Sequence < sal_Int8 > seq(10);
    for( sal_Int32 i = 0 ; i < 10 ; i ++ )
    {
        seq.getArray()[i] = i;
    }

    r->write( seq );
}

void doRead( const Reference < XConnection > &r )
{
    Sequence < sal_Int8 > seq(10);

    OSL_ASSERT( 10 == r->read( seq , 10 ) );

    for( sal_Int32 i = 0 ; i < 10 ; i ++ )
    {
        OSL_ASSERT( seq.getConstArray()[i] == i );
    }
}


void MyThread::run()
{
    try
    {
        m_rConnection = m_rAcceptor->accept( m_sConnectionDescription );
    }
    catch ( Exception &e)
    {
        OString tmp= OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
        printf( "Exception was thrown by acceptor thread: %s\n", tmp.getStr() );
    }

    if( m_rConnection.is() )
    {
        Sequence < sal_Int8 > seq(12);
        try
        {
            doWrite( m_rConnection );
            doRead( m_rConnection );
        }
        catch (... )
        {
            printf( "unknown exception was thrown\n" );
            throw;
        }
    }

}





void testConnection( const OUString &sConnectionDescription  ,
                     const Reference < XAcceptor > &rAcceptor,
                     const Reference < XConnector > &rConnector )
{
    {
        MyThread thread( rAcceptor , sConnectionDescription );
        thread.create();

        sal_Bool bGotit = sal_False;
        Reference < XConnection > r;

        while( ! bGotit )
        {
            try
            {
                // Why is this wait necessary ????
                TimeValue value = {1,0};
                osl_waitThread( &value );
                r = rConnector->connect( sConnectionDescription );
                OSL_ASSERT( r.is() );
                doWrite( r );
                doRead( r );
                bGotit = sal_True;
            }
            catch( ... )
            {
                printf( "Couldn't connect, retrying ...\n" );

            }
        }

        r->close();

        try
        {
            Sequence < sal_Int8 > seq(10);
            r->write( seq );
            OSL_FAIL( "expected exception not thrown" );
        }
        catch ( IOException & )
        {
            // everything is ok
        }
        catch ( ... )
        {
            OSL_FAIL( "wrong exception was thrown" );
        }

        thread.join();
    }
}


#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Reference< XMultiServiceFactory > xMgr(
        createRegistryServiceFactory( OUString( RTL_CONSTASCII_USTRINGPARAM("applicat.rdb")) ) );

    Reference< XImplementationRegistration > xImplReg(
        xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.ImplementationRegistration")) ), UNO_QUERY );
    OSL_ENSURE( xImplReg.is(), "### no impl reg!" );

    OUString aLibName =
        OUString(RTL_CONSTASCII_USTRINGPARAM( "connector.uno" SAL_DLLEXTENSION ));
    xImplReg->registerImplementation(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")), aLibName, Reference< XSimpleRegistry >() );

    aLibName = OUString(RTL_CONSTASCII_USTRINGPARAM( "acceptor.uno" SAL_DLLEXTENSION ));
    xImplReg->registerImplementation(
        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary")), aLibName, Reference< XSimpleRegistry >() );

    Reference < XAcceptor >  rAcceptor(
        xMgr->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")) ) , UNO_QUERY );

    Reference < XAcceptor >  rAcceptorPipe(
        xMgr->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")) ) , UNO_QUERY );

    Reference < XConnector >  rConnector(
        xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Connector")) ) , UNO_QUERY );


    printf( "Testing sockets" );
    fflush( stdout );
    testConnection( OUString(RTL_CONSTASCII_USTRINGPARAM("socket,host=localhost,port=2001")), rAcceptor , rConnector );
    printf( " Done\n" );

    printf( "Testing pipe" );
    fflush( stdout );
    testConnection( OUString(RTL_CONSTASCII_USTRINGPARAM("pipe,name=bla")) , rAcceptorPipe , rConnector );
    printf( " Done\n" );

    // check, if errornous strings make any problem
    rAcceptor = Reference< XAcceptor > (
        xMgr->createInstance( OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.connection.Acceptor")) ),
        UNO_QUERY );

    try
    {
        rAcceptor->accept( OUString() );
        OSL_FAIL( "empty connection string" );
    }
    catch( IllegalArgumentException & )
    {
        // everything is fine
    }
    catch( ... )
    {
        OSL_FAIL( "unexpected akexception with empty connection string" );
    }

    try
    {
        rConnector->connect( OUString() );
        OSL_FAIL( "empty connection string" );
    }
    catch( ConnectionSetupException & )
    {
        // everything is fine
    }
    catch( ... )
    {
        OSL_FAIL( "unexpected exception with empty connection string" );
    }


    MyThread thread( rAcceptor , OUString(RTL_CONSTASCII_USTRINGPARAM("socket,host=localhost,port=2001")) );
    thread.create();

    TimeValue value = {0,1};
    osl_waitThread( &value );
    try
    {
        rAcceptor->accept( OUString(RTL_CONSTASCII_USTRINGPARAM("socket,host=localhost,port=2001")) );
        OSL_FAIL( "already existing exception expected" );
    }
    catch( AlreadyAcceptingException & e)
    {
        // everything is fine
    }
    catch( ... )
    {
        OSL_FAIL( "unknown exception, already existing existing expected" );
    }

    rAcceptor->stopAccepting();
    thread.join();

    Reference < XComponent > rComp( xMgr , UNO_QUERY );
    if( rComp.is() )
    {
        rComp->dispose();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
