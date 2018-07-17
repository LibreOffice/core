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
    catch ( const Exception &e)
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
                osl::Thread::wait(std::chrono::seconds(1));
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


int SAL_CALL main( int argc, char * argv[] )
{
    Reference< XMultiServiceFactory > xMgr(
        createRegistryServiceFactory( OUString( "applicat.rdb") ) );

    Reference< XImplementationRegistration > xImplReg(
        xMgr->createInstance("com.sun.star.registry.ImplementationRegistration"), UNO_QUERY );
    OSL_ENSURE( xImplReg.is(), "### no impl reg!" );

    OUString aLibName = "connector.uno" SAL_DLLEXTENSION;
    xImplReg->registerImplementation(
        OUString("com.sun.star.loader.SharedLibrary"), aLibName, Reference< XSimpleRegistry >() );

    aLibName = "acceptor.uno" SAL_DLLEXTENSION;
    xImplReg->registerImplementation(
        OUString("com.sun.star.loader.SharedLibrary"), aLibName, Reference< XSimpleRegistry >() );

    Reference < XAcceptor >  rAcceptor(
        xMgr->createInstance( "com.sun.star.connection.Acceptor" ) , UNO_QUERY );

    Reference < XAcceptor >  rAcceptorPipe(
        xMgr->createInstance( "com.sun.star.connection.Acceptor" ) , UNO_QUERY );

    Reference < XConnector >  rConnector(
        xMgr->createInstance("com.sun.star.connection.Connector") , UNO_QUERY );


    printf( "Testing sockets" );
    fflush( stdout );
    testConnection( OUString("socket,host=localhost,port=2001"), rAcceptor , rConnector );
    printf( " Done\n" );

    printf( "Testing pipe" );
    fflush( stdout );
    testConnection( OUString("pipe,name=bla") , rAcceptorPipe , rConnector );
    printf( " Done\n" );

    // check, if errornous strings make any problem
    rAcceptor.set(
        xMgr->createInstance("com.sun.star.connection.Acceptor"),
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


    MyThread thread( rAcceptor , OUString("socket,host=localhost,port=2001") );
    thread.create();

    osl::Thread::wait(std::chrono::nanoseconds(1));
    try
    {
        rAcceptor->accept( OUString("socket,host=localhost,port=2001") );
        OSL_FAIL( "already existing exception expected" );
    }
    catch( AlreadyAcceptingException & )
    {
        // everything is fine
    }
    catch( ... )
    {
        OSL_FAIL( "unknown exception, already existing exception expected" );
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
