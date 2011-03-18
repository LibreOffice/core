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
#include "precompiled_extensions.hxx"
#include <smart/com/sun/star/test/XSimpleTest.hxx>
#include <smart/com/sun/star/io/XMarkableStream.hxx>
#include <smart/com/sun/star/io/XActiveDataSink.hxx>
#include <smart/com/sun/star/io/XActiveDataSource.hxx>
#include <smart/com/sun/star/io/XConnectable.hxx>

#include <smart/com/sun/star/lang/XServiceInfo.hxx>


#include <usr/factoryhlp.hxx>

#include <usr/reflserv.hxx>  // for EXTERN_SERVICE_CALLTYPE
#include <usr/weak.hxx>      // OWeakObject

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.hxx>

#include <string.h>

#include "testfactreg.hxx"


using namespace usr;

class OMarkableOutputStreamTest :
        public XSimpleTest,
        public OWeakObject
{
public:
    OMarkableOutputStreamTest( const XMultiServiceFactoryRef & rFactory );
    ~OMarkableOutputStreamTest();

public: // refcounting
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public: // implementation names
    static Sequence< UString >  getSupportedServiceNames_Static(void) THROWS( () );
    static UString              getImplementationName_Static() THROWS( () );

public:
    virtual void testInvariant(const UString& TestName, const XInterfaceRef& TestObject)
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual INT32 test( const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual BOOL testPassed(void)                               THROWS( (   UsrSystemException) );
    virtual Sequence< UString > getErrors(void)                 THROWS( (UsrSystemException) );
    virtual Sequence< UsrAny > getErrorExceptions(void)         THROWS( (UsrSystemException) );
    virtual Sequence< UString > getWarnings(void)               THROWS( (UsrSystemException) );

private:
    void testSimple( const XOutputStreamRef &r, const XInputStreamRef &rInput );

private:
    Sequence<UsrAny>  m_seqExceptions;
    Sequence<UString> m_seqErrors;
    Sequence<UString> m_seqWarnings;
    XMultiServiceFactoryRef m_rFactory;

};

OMarkableOutputStreamTest::OMarkableOutputStreamTest( const XMultiServiceFactoryRef &rFactory )
        : m_rFactory( rFactory )
{

}

OMarkableOutputStreamTest::~OMarkableOutputStreamTest()
{

}


BOOL OMarkableOutputStreamTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return OWeakObject::queryInterface( uik , rOut );
    }
    return TRUE;
}


void OMarkableOutputStreamTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    XServiceInfoRef info( TestObject, USR_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService( L"bla bluzb" ) , "XServiceInfo test failed" );
    }
}


INT32 OMarkableOutputStreamTest::test(  const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.MarkableOutputStream" == TestName )  {
        try {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else  {
                XInterfaceRef x = m_rFactory->createInstance( L"com.sun.star.io.Pipe");
                XOutputStreamRef  rPipeOutput( x , USR_QUERY );
                XInputStreamRef  rPipeInput( x , USR_QUERY );

                XActiveDataSourceRef source( TestObject , USR_QUERY );
                source->setOutputStream( rPipeOutput );

                XOutputStreamRef rOutput( TestObject , USR_QUERY );

                OSL_ASSERT( rPipeInput.is() );
                OSL_ASSERT( rOutput.is() );
                if( 1 == hTestHandle ) {
                    // checks usual streaming
                    testSimple( rOutput , rPipeInput );
                }
            }

        }
        catch( Exception& e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        catch(...) {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( 2 == hTestHandle ) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        THROW( IllegalArgumentException() );
    }
    return hTestHandle;
}



BOOL OMarkableOutputStreamTest::testPassed(void)                        THROWS( (UsrSystemException) )
{
    return m_seqErrors.getLen() == 0;
}


Sequence< UString > OMarkableOutputStreamTest::getErrors(void)      THROWS( (UsrSystemException) )
{
    return m_seqErrors;
}


Sequence< UsrAny > OMarkableOutputStreamTest::getErrorExceptions(void) THROWS( (UsrSystemException) )
{
    return m_seqExceptions;
}


Sequence< UString > OMarkableOutputStreamTest::getWarnings(void)        THROWS( (UsrSystemException) )
{
    return m_seqWarnings;
}


void OMarkableOutputStreamTest::testSimple(     const XOutputStreamRef &rOutput ,
                                                const XInputStreamRef &rInput )
{
    XMarkableStreamRef rMarkable( rOutput , USR_QUERY );

    ERROR_ASSERT( rMarkable.is() , "no MarkableStream implemented" );

    // first check normal input/output facility
    char pcStr[] = "Live long and prosper !";

    Sequence<BYTE> seqWrite( strlen( pcStr )+1 );
    memcpy( seqWrite.getArray() , pcStr , seqWrite.getLen() );

    Sequence<BYTE> seqRead( seqWrite.getLen() );

    int nMax = 10,i;

    for( i = 0 ; i < nMax ; i ++ ) {
        rOutput->writeBytes( seqWrite );
        rInput->readBytes( seqRead , rInput->available() );
        ERROR_ASSERT( ! strcmp( (char *) seqWrite.getArray() , (char * )seqRead.getArray() ) ,
                      "error during read/write/skip" );
    }

    // Check buffer resizing
    nMax = 3000;
    for( i = 0 ; i < nMax ; i ++ ) {
        rOutput->writeBytes( seqWrite );
    }

    for( i = 0 ; i < nMax ; i ++ ) {
        rInput->readBytes( seqRead , seqWrite.getLen() );
        ERROR_ASSERT( ! strcmp( (char *) seqWrite.getArray() , (char * )seqRead.getArray() ) ,
                      "error during read/write" );
    }

    // Check creating marks !
    INT32 nMark = rMarkable->createMark();

    for( i = 0 ; i < nMax ; i ++ ) {
        rOutput->writeBytes( seqWrite );
    }

    ERROR_ASSERT( 0 == rInput->available() , "bytes available though mark is holded" );

    ERROR_ASSERT( nMax*seqWrite.getLen() == rMarkable->offsetToMark( nMark ) ,
                                             "offsetToMark failure" );

    rMarkable->deleteMark( nMark );
    ERROR_ASSERT( nMax*seqWrite.getLen() == rInput->available(),"bytes are not available though mark has been deleted" );

    rInput->skipBytes( nMax*seqWrite.getLen() );
    ERROR_ASSERT( 0 == rInput->available(), "skip bytes failure" );

    try {
        rMarkable->jumpToMark( nMark );
        ERROR_ASSERT( 0 , "jump to non existing mark possible !" );
    }
    catch ( IllegalArgumentException& e )
    {
        e;// ok, exception was thrown
    }

    // test putting marks not at the end of the stream!
    ERROR_ASSERT( 0 == rInput->available(), "stream isn't clean" );
    {
        Sequence< BYTE > aByte(256);

        for( i = 0 ; i < 256 ; i ++ )
        {
            aByte.getArray()[i] = i;
        }
        INT32 nMark1 = rMarkable->createMark();

        rOutput->writeBytes( aByte );
        rMarkable->jumpToMark( nMark1 );
        aByte.realloc( 10 );
        rOutput->writeBytes( aByte );

        INT32 nMark2 = rMarkable->createMark( );

        for( i = 0 ; i < 10 ; i ++ )
        {
            aByte.getArray()[i] = i+10;
        }

        rOutput->writeBytes( aByte );

        // allow the bytes to be written !
        rMarkable->jumpToFurthest();
        rMarkable->deleteMark( nMark1 );
        rMarkable->deleteMark( nMark2 );

        ERROR_ASSERT( 256 == rInput->available(), "in between mark failure" );
        rInput->readBytes( aByte ,256);
        for( i = 0 ; i < 256 ; i ++ )
        {
            ERROR_ASSERT( i == aByte.getArray()[i] , "in between mark failure" );
        }
    }

    {
        // now a more extensive mark test !
        Sequence<BYTE> as[4];
        INT32 an[4];

        for( i = 0 ; i < 4 ; i ++ ) {
            as[i].realloc(1);
            as[i].getArray()[0] = i;
            an[i] = rMarkable->createMark();
            rOutput->writeBytes( as[i] );
        }

        // check offset to mark
        for( i = 0 ; i < 4 ; i ++ ) {
            ERROR_ASSERT( rMarkable->offsetToMark( an[i] ) == 4-i , "offsetToMark failure" );
        }

        rMarkable->jumpToMark( an[1] );
        ERROR_ASSERT( rMarkable->offsetToMark( an[3] ) == -2 , "offsetToMark failure" );

        rMarkable->jumpToFurthest( );
        ERROR_ASSERT( rMarkable->offsetToMark( an[0] ) == 4 , "offsetToMark failure" );

        // now do a rewrite !
        for( i = 0 ; i < 4 ; i ++ ) {
            rMarkable->jumpToMark( an[3-i] );
            rOutput->writeBytes( as[i] );
        }
        // NOTE : CursorPos 1

        // now delete the marks !
        for( i = 0 ; i < 4 ; i ++ ) {
            rMarkable->deleteMark( an[i] );
        }
        ERROR_ASSERT( rInput->available() == 1 , "wrong number of bytes flushed" );

        rMarkable->jumpToFurthest();

        ERROR_ASSERT( rInput->available() == 4 , "wrong number of bytes flushed" );

        rInput->readBytes( seqRead , 4 );

        ERROR_ASSERT( 3 == seqRead.getArray()[0] , "rewrite didn't work" );
        ERROR_ASSERT( 2 == seqRead.getArray()[1] , "rewrite didn't work" );
        ERROR_ASSERT( 1 == seqRead.getArray()[2] , "rewrite didn't work" );
        ERROR_ASSERT( 0 == seqRead.getArray()[3] , "rewrite didn't work" );

        rOutput->closeOutput();
        rInput->closeInput();
    }

}

/**
* for external binding
**/
XInterfaceRef OMarkableOutputStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    OMarkableOutputStreamTest *p = new OMarkableOutputStreamTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}



Sequence<UString> OMarkableOutputStreamTest_getSupportedServiceNames(void) THROWS( () )
{
    Sequence<UString> aRet(1);
    aRet.getArray()[0] = OMarkableOutputStreamTest_getImplementationName();

    return aRet;
}

UString     OMarkableOutputStreamTest_getServiceName() THROWS( () )
{
    return L"test.com.sun.star.io.MarkableOutputStream";
}

UString     OMarkableOutputStreamTest_getImplementationName() THROWS( () )
{
    return L"test.com.sun.starextensions.stm.MarkableOutputStream";
}







//-----------------------------------------------------
// Input stream

class OMarkableInputStreamTest :
        public XSimpleTest,
        public OWeakObject
{
public:
    OMarkableInputStreamTest( const XMultiServiceFactoryRef & rFactory );
    ~OMarkableInputStreamTest();

public: // refcounting
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public: // implementation names
    static Sequence< UString >  getSupportedServiceNames_Static(void) THROWS( () );
    static UString              getImplementationName_Static() THROWS( () );

public:
    virtual void testInvariant(const UString& TestName, const XInterfaceRef& TestObject)
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual INT32 test( const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual BOOL testPassed(void)                               THROWS( (   UsrSystemException) );
    virtual Sequence< UString > getErrors(void)                 THROWS( (UsrSystemException) );
    virtual Sequence< UsrAny > getErrorExceptions(void)         THROWS( (UsrSystemException) );
    virtual Sequence< UString > getWarnings(void)               THROWS( (UsrSystemException) );

private:
    void testSimple( const XOutputStreamRef &r, const XInputStreamRef &rInput );

private:
    Sequence<UsrAny>  m_seqExceptions;
    Sequence<UString> m_seqErrors;
    Sequence<UString> m_seqWarnings;
    XMultiServiceFactoryRef m_rFactory;

};

OMarkableInputStreamTest::OMarkableInputStreamTest( const XMultiServiceFactoryRef &rFactory )
        : m_rFactory( rFactory )
{

}

OMarkableInputStreamTest::~OMarkableInputStreamTest()
{

}


BOOL OMarkableInputStreamTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return OWeakObject::queryInterface( uik , rOut );
    }
    return TRUE;
}


void OMarkableInputStreamTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.MarkableInputStream" == TestName )  {
        XServiceInfoRef info( TestObject, USR_QUERY );
        ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
        if( info.is() )
        {
            ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
            ERROR_ASSERT( ! info->supportsService( L"bla bluzb" ) , "XServiceInfo test failed" );
        }
    }
    else {
        THROW( IllegalArgumentException() );
    }
}


INT32 OMarkableInputStreamTest::test(   const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.MarkableInputStream" == TestName )  {
        try {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else  {
                XInterfaceRef x = m_rFactory->createInstance( L"com.sun.star.io.Pipe");
                XOutputStreamRef  rPipeOutput( x , USR_QUERY );
                XInputStreamRef  rPipeInput( x , USR_QUERY );

                XActiveDataSinkRef sink( TestObject , USR_QUERY );
                sink->setInputStream( rPipeInput );

                XInputStreamRef rInput( TestObject , USR_QUERY );

                OSL_ASSERT( rPipeOutput.is() );
                OSL_ASSERT( rInput.is() );
                if( 1 == hTestHandle ) {
                    // checks usual streaming
                    testSimple( rPipeOutput , rInput );
                }
            }

        }
        catch( Exception& e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        catch(...) {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( 2 == hTestHandle ) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        THROW( IllegalArgumentException() );
    }
    return hTestHandle;
}



BOOL OMarkableInputStreamTest::testPassed(void)                                         THROWS( (UsrSystemException) )
{
    return m_seqErrors.getLen() == 0;
}


Sequence< UString > OMarkableInputStreamTest::getErrors(void)                           THROWS( (UsrSystemException) )
{
    return m_seqErrors;
}


Sequence< UsrAny > OMarkableInputStreamTest::getErrorExceptions(void)                   THROWS( (UsrSystemException) )
{
    return m_seqExceptions;
}


Sequence< UString > OMarkableInputStreamTest::getWarnings(void)                         THROWS( (UsrSystemException) )
{
    return m_seqWarnings;
}


void OMarkableInputStreamTest::testSimple(      const XOutputStreamRef &rOutput ,
                                                const XInputStreamRef &rInput )
{
    XMarkableStreamRef rMarkable( rInput , USR_QUERY );

    Sequence<BYTE> seqWrite( 256 );
    Sequence<BYTE> seqRead(10);

    for( int i = 0 ; i < 256 ; i ++ )
    {
        seqWrite.getArray()[i] = i;
    }

    rOutput->writeBytes( seqWrite );
    ERROR_ASSERT( 256 == rInput->available() , "basic read/write failure" );

    rInput->readBytes( seqRead , 10 );
    ERROR_ASSERT( 9 == seqRead.getArray()[9] , "basic read/write failure" );

    INT32 nMark = rMarkable->createMark();

    rInput->skipBytes( 50 );
    ERROR_ASSERT( 256-10-50 == rInput->available() , "marking error" );
    ERROR_ASSERT( 50 == rMarkable->offsetToMark( nMark ) , "marking error" );

    rMarkable->jumpToMark( nMark );
    ERROR_ASSERT( 256-10 == rInput->available() , "marking error" );

    rInput->readBytes( seqRead , 10 );
    ERROR_ASSERT( 10 == seqRead.getArray()[0] , "marking error" );

    // pos 20
    {
        INT32 nInBetweenMark = rMarkable->createMark( );
        rMarkable->jumpToMark( nMark );
        rMarkable->jumpToMark( nInBetweenMark );

        rInput->readBytes( seqRead , 10 );
        ERROR_ASSERT( 20 == seqRead.getArray()[0] , "Inbetween mark failed!\n" );

        rMarkable->deleteMark( nMark );

        // Check if releasing the first bytes works correct.
        rMarkable->jumpToMark( nInBetweenMark);
        rInput->readBytes( seqRead , 10 );
        ERROR_ASSERT( 20 == seqRead.getArray()[0] , "Inbetween mark failed!\n" );

        rMarkable->deleteMark( nInBetweenMark );
    }

    rMarkable->jumpToFurthest();
    ERROR_ASSERT( 256-10-50 == rInput->available() , "marking error" );


    ERROR_ASSERT( 100 == rInput->readSomeBytes( seqRead , 100   ) , "wrong results using readSomeBytes" );
    ERROR_ASSERT( 96 == rInput->readSomeBytes( seqRead , 1000) , "wrong results using readSomeBytes" );
    rOutput->closeOutput();
    rInput->closeInput();
}

/**
* for external binding
**/
XInterfaceRef OMarkableInputStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    OMarkableInputStreamTest *p = new OMarkableInputStreamTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}



Sequence<UString> OMarkableInputStreamTest_getSupportedServiceNames(void) THROWS( () )
{
    Sequence<UString> aRet(1);
    aRet.getArray()[0] = OMarkableInputStreamTest_getImplementationName();

    return aRet;
}

UString     OMarkableInputStreamTest_getServiceName() THROWS( () )
{
    return L"test.com.sun.star.io.MarkableInputStream";
}

UString     OMarkableInputStreamTest_getImplementationName() THROWS( () )
{
    return L"test.com.sun.star.extensions.stm.MarkableInputStream";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
