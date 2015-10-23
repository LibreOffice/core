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
#include <com/sun/star/test/XSimpleTest.hpp>

#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataControl.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <list>




using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;

#include "testfactreg.hxx"

static void mywait()
{
    TimeValue a = { 0, 10000 };
    osl_waitThread( &a );
    osl_yieldThread();
    osl_yieldThread();
}

class OPumpTest : public WeakImplHelper < XSimpleTest >
{
public:
    OPumpTest( const Reference< XMultiServiceFactory >  & rFactory );
    ~OPumpTest();

public: // implementation names
    static Sequence< OUString >     getSupportedServiceNames_Static() throw();
    static OUString                 getImplementationName_Static() throw();

public:
    virtual void SAL_CALL testInvariant(const OUString& TestName, const Reference < XInterface >& TestObject)
        throw  ( IllegalArgumentException, RuntimeException) ;

    virtual sal_Int32 SAL_CALL test(    const OUString& TestName,
                                        const Reference < XInterface >& TestObject,
                                        sal_Int32 hTestHandle)
        throw  (    IllegalArgumentException,
                    RuntimeException);

    virtual sal_Bool SAL_CALL testPassed()                              throw  (    RuntimeException) ;
    virtual Sequence< OUString > SAL_CALL getErrors()               throw  (RuntimeException) ;
    virtual Sequence< Any > SAL_CALL getErrorExceptions()       throw  (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings()                 throw  (RuntimeException);

private:
    void testSimple( const Reference < XInterface > & );
    void testWrongUsage( const Reference < XInterface > & );
    void testClose( const Reference< XInterface >& );
    void testTerminate( const Reference< XInterface >& );
    void testFunction( const Reference< XInterface >& );
private:
    Sequence<Any>  m_seqExceptions;
    Sequence<OUString> m_seqErrors;
    Sequence<OUString> m_seqWarnings;
    Reference< XMultiServiceFactory > m_rSmgr;

};

OPumpTest::OPumpTest( const Reference< XMultiServiceFactory > &rFactory ) :
    m_rSmgr( rFactory )
{

}

OPumpTest::~OPumpTest()
{

}



void OPumpTest::testInvariant( const OUString& TestName, const Reference < XInterface >& TestObject )
    throw  (    IllegalArgumentException,
                RuntimeException)
{
    Reference< XServiceInfo > info( TestObject, UNO_QUERY );
      ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
          ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService(
            OUString( "bla bluzb" ) ), "XServiceInfo test failed" );
    }

    Reference < XActiveDataSource > xActiveDataSource( TestObject, UNO_QUERY );
    Reference < XActiveDataSink > xActiveDataSink( TestObject, UNO_QUERY );
    Reference < XActiveDataControl > xActiveDataControl( TestObject , UNO_QUERY );
    Reference < XConnectable > xConnectable( TestObject , UNO_QUERY );

    ERROR_ASSERT( xActiveDataSource.is() && xActiveDataSink.is() && xActiveDataControl.is () &&
                  xConnectable.is(), "specified interface not supported" );
}


sal_Int32 OPumpTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw  (    IllegalArgumentException, RuntimeException)
{
    if( OUString( "com.sun.star.io.Pump" ) == TestName )  {
        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else if ( 1 == hTestHandle )
            {
                testWrongUsage( TestObject);
            }
            else if ( 2 == hTestHandle )
            {
                testClose( TestObject);
            }
            else if ( 3 == hTestHandle )
            {
                testTerminate( TestObject );
            }
            else if ( 4 == hTestHandle )
            {
                testFunction( TestObject );
            }
        }
        catch( const Exception & e )
        {
            OString s = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            BUILD_ERROR( 0 , s.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( 5 == hTestHandle )
        {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        throw IllegalArgumentException();
    }
    return hTestHandle;
}



sal_Bool OPumpTest::testPassed()        throw  (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OPumpTest::getErrors()     throw  (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OPumpTest::getErrorExceptions()                     throw  (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OPumpTest::getWarnings()                       throw  (RuntimeException)
{
    return m_seqWarnings;
}


/***
* the test methods
*
****/


void OPumpTest::testSimple( const Reference < XInterface > &r )
{
    // jbu todo: add sensible test

}

class TestListener: public WeakImplHelper< XStreamListener >
{
public:
    sal_Bool m_bStarted;
    sal_Bool m_bClosed;
    sal_Bool m_bTerminated;
    sal_Bool m_bError;
    sal_Bool m_bDisposed;
    TestListener() : m_bStarted (sal_False),
                     m_bClosed (sal_False),
                     m_bTerminated ( sal_False ),
                     m_bError( sal_False ),
                     m_bDisposed( sal_False )
    {}

    virtual void SAL_CALL disposing( const EventObject &obj  ) throw (css::uno::RuntimeException)
    {
        m_bDisposed = sal_True;
//         printf( "disposing called\n");
    }

    virtual void SAL_CALL started(  ) throw (css::uno::RuntimeException)
    {
        m_bStarted = sal_True;
//         printf( "started called\n");
    }
    virtual void SAL_CALL closed(  ) throw (css::uno::RuntimeException)
    {
        m_bClosed = sal_True;
//         printf( "closed called\n");
    }
    virtual void SAL_CALL terminated(  ) throw (css::uno::RuntimeException)
    {
        m_bTerminated = sal_True;
//         printf( "terminated called\n");
    }
    virtual void SAL_CALL error( const css::uno::Any& aException )
        throw (css::uno::RuntimeException)
    {
        m_bError = sal_True;
        Exception e;
        aException >>= e;
//         printf( "error called %s\n", OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US).getStr() );
    }
};

class TestCase
{
public:
    TestCase( const Reference< XMultiServiceFactory > & rSMgr,
              const Reference< XInterface > &r ) : m_rSmgr( rSMgr ), m_pTestListener( 0 )
    {
        m_rControl = Reference<XActiveDataControl>( r, UNO_QUERY );

        Reference< XActiveDataSource > rSource ( r, UNO_QUERY );
        Reference< XActiveDataSink > rSink( r , UNO_QUERY );

        m_rOutSource = Reference< XOutputStream > ( createPipe() );
        rSink->setInputStream(Reference< XInputStream> (m_rOutSource,UNO_QUERY));

        Reference< XOutputStream > rOutSink( createPipe() );
        m_rInSink = Reference< XInputStream > ( rOutSink, UNO_QUERY );
        rSource->setOutputStream( rOutSink );

        m_pTestListener = new TestListener();
        m_pTestListener->acquire();
        m_rControl->addListener( m_pTestListener );
    }

    ~TestCase()
    {
        if( m_pTestListener )
            m_pTestListener->release();
    }

    TestListener *m_pTestListener;
    Reference< XActiveDataControl > m_rControl;
    Reference< XOutputStream > m_rOutSource;
    Reference< XInputStream > m_rInSink;
    Reference< XMultiServiceFactory > m_rSmgr;

private:
    Reference< XOutputStream > createPipe()
    {
        Reference< XOutputStream > rOut( m_rSmgr->createInstance(
             OUString("com.sun.star.io.Pipe")),UNO_QUERY);
        return rOut;
    }
};



void OPumpTest::testClose( const Reference< XInterface > &r )
{
    TestCase t( m_rSmgr, r );

    ERROR_ASSERT( ! t.m_pTestListener->m_bStarted , "started too early" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bTerminated , "terminiation unexpected" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bClosed, "unexpected clase" );

    t.m_rControl->start();
    mywait();

    ERROR_ASSERT( t.m_pTestListener->m_bStarted , "should have been started already" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bTerminated , "terminiation unexpected" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bClosed, "unexpected clase" );

    Reference< XStreamListener > rListener( new TestListener() );
    t.m_rControl->addListener( rListener );
    t.m_rControl->removeListener( rListener );

    t.m_rOutSource->closeOutput();
    mywait();
    ERROR_ASSERT( t.m_pTestListener->m_bStarted , "should have been started already" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bTerminated , "should be terminiated already" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( t.m_pTestListener->m_bClosed, "should be closed already" );
}

void OPumpTest::testTerminate( const Reference< XInterface > &r )
{
    TestCase t( m_rSmgr, r );

    ERROR_ASSERT( ! t.m_pTestListener->m_bStarted , "started too early" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bTerminated , "terminiation unexpected" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bClosed, "unexpected clase" );

    t.m_rControl->start();
    mywait();

    ERROR_ASSERT( t.m_pTestListener->m_bStarted , "should have been started already" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bTerminated , "terminiation unexpected" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bClosed, "unexpected clase" );

    t.m_rControl->terminate();

    mywait();
    ERROR_ASSERT( t.m_pTestListener->m_bStarted , "should have been started already" );
    ERROR_ASSERT( t.m_pTestListener->m_bTerminated , "should be terminiated already" );
    // terminate leads to an error, that is no surprise, in fact
    // one can't tell whether the error occurs because of the terminate
    // call or for some other reason!
//     ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( t.m_pTestListener->m_bClosed, "should be closed already" );
}

void OPumpTest::testFunction( const Reference< XInterface > &r )
{
    TestCase t( m_rSmgr, r );

    t.m_rControl->start();

    t.m_rOutSource->writeBytes( Sequence< sal_Int8 > ( 5 ) );

    Sequence< sal_Int8 > dummy;
    ERROR_ASSERT( 5 == t.m_rInSink->readBytes( dummy , 5 ), "couldn't read the expected number of bytes" );

    t.m_rOutSource->closeOutput();
    mywait();

    ERROR_ASSERT( t.m_pTestListener->m_bStarted , "should have been started already" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bTerminated , "should be terminiated already" );
    ERROR_ASSERT( ! t.m_pTestListener->m_bError, "unexpected error" );
    ERROR_ASSERT( t.m_pTestListener->m_bClosed, "should be closed already" );
}

void OPumpTest::testWrongUsage( const Reference< XInterface > &r )
{
    Reference< XActiveDataSource > rSource ( r, UNO_QUERY );
    Reference< XActiveDataSink > rSink( r , UNO_QUERY );
    Reference< XActiveDataControl > rControl( r, UNO_QUERY );

    Reference< XInputStream > rIn( m_rSmgr->createInstance(
        OUString("com.sun.star.io.DataInputStream")),UNO_QUERY);
    Reference< XOutputStream > rOut( m_rSmgr->createInstance(
        OUString("com.sun.star.io.DataOutputStream")),UNO_QUERY);

    rSink->setInputStream( rIn );
    rSource->setOutputStream( rOut );

    rControl->start();

    mywait();
}

Reference< XInterface > SAL_CALL OPumpTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw( Exception )
{
    return *new OPumpTest( rSMgr );
}

Sequence<OUString> OPumpTest_getSupportedServiceNames() throw()
{
    OUString s = OPumpTest_getServiceName();
    Sequence< OUString > seq( &s , 1 );
    return seq;

}
OUString     OPumpTest_getServiceName() throw()
{
    return OUString( "test.com.sun.star.io.Pump"  );
}

OUString    OPumpTest_getImplementationName() throw()
{
    return OUString( "test.com.sun.star.comp.io.Pump" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
