/*************************************************************************
 *
 *  $RCSfile: marktest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:56 $
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
#include <smart/com/sun/star/test/XSimpleTest.hxx>
#include <smart/com/sun/star/io/XMarkableStream.hxx>
#include <smart/com/sun/star/io/XActiveDataSink.hxx>
#include <smart/com/sun/star/io/XActiveDataSource.hxx>
#include <smart/com/sun/star/io/XConnectable.hxx>

#include <smart/com/sun/star/lang/XServiceInfo.hxx>


#include <usr/factoryhlp.hxx>

#include <usr/reflserv.hxx>  // for EXTERN_SERVICE_CALLTYPE
#include <usr/weak.hxx>      // OWeakObject

#include <vos/conditn.hxx>
#include <vos/mutex.hxx>
#include <vos/thread.hxx>

#include <assert.h>
#include <string.h>

#include "testfactreg.hxx"


#ifndef _VOS_NO_NAMESPACE
using namespace vos;
using namespace usr;
#endif



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
        TRY {
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

                assert( rPipeInput.is() );
                assert( rOutput.is() );
                if( 1 == hTestHandle ) {
                    // checks usual streaming
                    testSimple( rOutput , rPipeInput );
                }
            }

        }
        CATCH( Exception , e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }
        END_CATCH;

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

    TRY {
        rMarkable->jumpToMark( nMark );
        ERROR_ASSERT( 0 , "jump to non existing mark possible !" );
    }
    CATCH ( IllegalArgumentException , e )
    {
        e;// ok, exception was thrown
    }
    END_CATCH;

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

/***
* the test methods
*
****/





/**
* for external binding
*
*
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
        TRY {
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

                assert( rPipeOutput.is() );
                assert( rInput.is() );
                if( 1 == hTestHandle ) {
                    // checks usual streaming
                    testSimple( rPipeOutput , rInput );
                }
            }

        }
        CATCH( Exception , e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }
        END_CATCH;

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

/***
* the test methods
*
****/





/**
* for external binding
*
*
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
