/*************************************************************************
 *
 *  $RCSfile: marktest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jbu $ $Date: 2001-03-15 17:58:02 $
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
#include <com/sun/star/test/XSimpleTest.hpp>
#include <com/sun/star/io/XActiveDataSink.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <assert.h>
#include <string.h>

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
// streams

#include "testfactreg.hxx"


class OMarkableOutputStreamTest : public WeakImplHelper1< XSimpleTest >
{
public:
    OMarkableOutputStreamTest( const Reference< XMultiServiceFactory > & rFactory );
    ~OMarkableOutputStreamTest();

public: // implementation names
    static Sequence< OUString >     getSupportedServiceNames_Static(void) throw ();
    static OUString                 getImplementationName_Static() throw ();

public:
    virtual void SAL_CALL testInvariant(
        const OUString& TestName,
        const Reference < XInterface >& TestObject)
        throw ( IllegalArgumentException,
                RuntimeException) ;

    virtual sal_Int32 SAL_CALL  test(   const OUString& TestName,
                        const Reference < XInterface >& TestObject,
                        sal_Int32 hTestHandle)
        throw ( IllegalArgumentException, RuntimeException);
    virtual sal_Bool SAL_CALL testPassed(void)
        throw ( RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors(void)
        throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions(void)
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings(void)
        throw (RuntimeException);

private:
    void testSimple( const Reference< XOutputStream > &r, const Reference < XInputStream > &rInput );

private:
    Sequence<Any>  m_seqExceptions;
    Sequence<OUString> m_seqErrors;
    Sequence<OUString> m_seqWarnings;
    Reference< XMultiServiceFactory > m_rFactory;

};

OMarkableOutputStreamTest::OMarkableOutputStreamTest( const Reference< XMultiServiceFactory > &rFactory )
        : m_rFactory( rFactory )
{

}

OMarkableOutputStreamTest::~OMarkableOutputStreamTest()
{

}




void OMarkableOutputStreamTest::testInvariant( const OUString& TestName,
                                               const Reference < XInterface >& TestObject )
    throw ( IllegalArgumentException, RuntimeException)
{
    Reference< XServiceInfo > info( TestObject, UNO_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService(
            OUString( RTL_CONSTASCII_USTRINGPARAM("bla bluzb")) ) , "XServiceInfo test failed" );
    }
}


sal_Int32 OMarkableOutputStreamTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw ( IllegalArgumentException, RuntimeException)
{
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.MarkableOutputStream") )
                  == TestName  )  {
        try
        {
            if( 0 == hTestHandle )
            {
                testInvariant( TestName , TestObject );
            }
            else
            {
                Reference < XInterface > x = m_rFactory->createInstance( OUString::createFromAscii("com.sun.star.io.Pipe"));
                Reference< XOutputStream >  rPipeOutput( x , UNO_QUERY );
                Reference < XInputStream >  rPipeInput( x , UNO_QUERY );

                Reference<  XActiveDataSource >  source( TestObject , UNO_QUERY );
                source->setOutputStream( rPipeOutput );

                Reference< XOutputStream > rOutput( TestObject , UNO_QUERY );

                assert( rPipeInput.is() );
                assert( rOutput.is() );
                if( 1 == hTestHandle ) {
                    // checks usual streaming
                    testSimple( rOutput , rPipeInput );
                }
            }

        }
        catch( Exception &e )
        {
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            BUILD_ERROR( 0 , o.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( 2 == hTestHandle )
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



sal_Bool OMarkableOutputStreamTest::testPassed(void)                        throw (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OMarkableOutputStreamTest::getErrors(void)         throw (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OMarkableOutputStreamTest::getErrorExceptions(void) throw (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OMarkableOutputStreamTest::getWarnings(void)       throw (RuntimeException)
{
    return m_seqWarnings;
}


void OMarkableOutputStreamTest::testSimple(     const Reference< XOutputStream > &rOutput ,
                                                const Reference< XInputStream > &rInput )
{
    Reference < XMarkableStream > rMarkable( rOutput , UNO_QUERY );

    ERROR_ASSERT( rMarkable.is() , "no MarkableStream implemented" );

    // first check normal input/output facility
    char pcStr[] = "Live long and prosper !";

    Sequence<sal_Int8> seqWrite( strlen( pcStr )+1 );
    memcpy( seqWrite.getArray() , pcStr , seqWrite.getLength() );

    Sequence<sal_Int8> seqRead( seqWrite.getLength() );

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
        rInput->readBytes( seqRead , seqWrite.getLength() );
        ERROR_ASSERT( ! strcmp( (char *) seqWrite.getArray() , (char * )seqRead.getArray() ) ,
                      "error during read/write" );
    }

    // Check creating marks !
    sal_Int32 nMark = rMarkable->createMark();

    for( i = 0 ; i < nMax ; i ++ ) {
        rOutput->writeBytes( seqWrite );
    }

    ERROR_ASSERT( 0 == rInput->available() , "bytes available though mark is holded" );

    ERROR_ASSERT( nMax*seqWrite.getLength() == rMarkable->offsetToMark( nMark ) ,
                                             "offsetToMark failure" );

    rMarkable->deleteMark( nMark );
    ERROR_ASSERT( nMax*seqWrite.getLength() == rInput->available(),"bytes are not available though mark has been deleted" );

    rInput->skipBytes( nMax*seqWrite.getLength() );
    ERROR_ASSERT( 0 == rInput->available(), "skip bytes failure" );

    try
    {
        rMarkable->jumpToMark( nMark );
        ERROR_ASSERT( 0 , "jump to non existing mark possible !" );
    }
    catch ( IllegalArgumentException & )
    {
        // ok, exception was thrown
    }

    // test putting marks not at the end of the stream!
    ERROR_ASSERT( 0 == rInput->available(), "stream isn't clean" );
    {
        Sequence< sal_Int8 > aByte(256);

        for( i = 0 ; i < 256 ; i ++ )
        {
            aByte.getArray()[i] = i;
        }
        sal_Int32 nMark1 = rMarkable->createMark();

        rOutput->writeBytes( aByte );
        rMarkable->jumpToMark( nMark1 );
        aByte.realloc( 10 );
        rOutput->writeBytes( aByte );

        sal_Int32 nMark2 = rMarkable->createMark( );

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
            ERROR_ASSERT( i == ((sal_uInt8*)(aByte.getArray()))[i] , "in between mark failure" );
        }
    }

    {
        // now a more extensive mark test !
        Sequence<sal_Int8> as[4];
        sal_Int32 an[4];

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
Reference < XInterface > SAL_CALL OMarkableOutputStreamTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    OMarkableOutputStreamTest *p = new OMarkableOutputStreamTest( rSMgr );
    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}



Sequence<OUString> OMarkableOutputStreamTest_getSupportedServiceNames(void) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OMarkableOutputStreamTest_getImplementationName();

    return aRet;
}

OUString     OMarkableOutputStreamTest_getServiceName() throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.MarkableOutputStream"));
}

OUString    OMarkableOutputStreamTest_getImplementationName() throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.starextensions.stm.MarkableOutputStream"));
}







//-----------------------------------------------------
// Input stream


class OMarkableInputStreamTest : public WeakImplHelper1< XSimpleTest >
{
public:
    OMarkableInputStreamTest( const Reference< XMultiServiceFactory > & rFactory );
    ~OMarkableInputStreamTest();

public: // implementation names
    static Sequence< OUString >     getSupportedServiceNames_Static(void) throw () ;
    static OUString                 getImplementationName_Static() throw () ;

public:
    virtual void SAL_CALL testInvariant(
        const OUString& TestName,
        const Reference < XInterface >& TestObject)
        throw ( IllegalArgumentException, RuntimeException) ;

    virtual sal_Int32 SAL_CALL test(
        const OUString& TestName,
        const Reference < XInterface >& TestObject,
        sal_Int32 hTestHandle)
        throw ( IllegalArgumentException,
                RuntimeException) ;

    virtual sal_Bool SAL_CALL testPassed(void)
        throw ( RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors(void)
        throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions(void)
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings(void)
        throw (RuntimeException);

private:
    void testSimple( const Reference< XOutputStream > &r,
                     const Reference < XInputStream > &rInput );

private:
    Sequence<Any>  m_seqExceptions;
    Sequence<OUString> m_seqErrors;
    Sequence<OUString> m_seqWarnings;
    Reference< XMultiServiceFactory > m_rFactory;

};

OMarkableInputStreamTest::OMarkableInputStreamTest( const Reference< XMultiServiceFactory > &rFactory )
        : m_rFactory( rFactory )
{

}

OMarkableInputStreamTest::~OMarkableInputStreamTest()
{

}



void OMarkableInputStreamTest::testInvariant(
    const OUString& TestName, const Reference < XInterface >& TestObject )
    throw ( IllegalArgumentException, RuntimeException)
{
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.MarkableInputStream"))
        == TestName )  {
        Reference <XServiceInfo >  info( TestObject, UNO_QUERY );
        ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
        if( info.is() )
        {
            ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
            ERROR_ASSERT(
                ! info->supportsService(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("bla bluzb")) ) ,
                "XServiceInfo test failed" );
        }
    }
    else
    {
        throw IllegalArgumentException();
    }
}


sal_Int32 OMarkableInputStreamTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)  throw ( IllegalArgumentException, RuntimeException)
{
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.MarkableInputStream")) == TestName )
    {
        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else  {
                Reference < XInterface > x = m_rFactory->createInstance( OUString::createFromAscii("com.sun.star.io.Pipe"));
                Reference< XOutputStream >  rPipeOutput( x , UNO_QUERY );
                Reference < XInputStream >  rPipeInput( x , UNO_QUERY );

                Reference < XActiveDataSink >  sink( TestObject , UNO_QUERY );
                sink->setInputStream( rPipeInput );

                Reference < XInputStream > rInput( TestObject , UNO_QUERY );

                assert( rPipeOutput.is() );
                assert( rInput.is() );
                if( 1 == hTestHandle ) {
                    // checks usual streaming
                    testSimple( rPipeOutput , rInput );
                }
            }

        }
        catch( Exception & e )
        {
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            BUILD_ERROR( 0 , o.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( 2 == hTestHandle ) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else
    {
        throw IllegalArgumentException();
    }
    return hTestHandle;
}



sal_Bool OMarkableInputStreamTest::testPassed(void)                                         throw (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OMarkableInputStreamTest::getErrors(void)                          throw (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OMarkableInputStreamTest::getErrorExceptions(void)                  throw (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OMarkableInputStreamTest::getWarnings(void)                        throw (RuntimeException)
{
    return m_seqWarnings;
}


void OMarkableInputStreamTest::testSimple(      const Reference< XOutputStream > &rOutput ,
                                                const Reference < XInputStream > &rInput )
{
    Reference < XMarkableStream > rMarkable( rInput , UNO_QUERY );

    Sequence<sal_Int8> seqWrite( 256 );
    Sequence<sal_Int8> seqRead(10);

    for( int i = 0 ; i < 256 ; i ++ )
    {
        seqWrite.getArray()[i] = i;
    }

    rOutput->writeBytes( seqWrite );
    ERROR_ASSERT( 256 == rInput->available() , "basic read/write failure" );

    rInput->readBytes( seqRead , 10 );
    ERROR_ASSERT( 9 == seqRead.getArray()[9] , "basic read/write failure" );

    sal_Int32 nMark = rMarkable->createMark();

    rInput->skipBytes( 50 );
    ERROR_ASSERT( 256-10-50 == rInput->available() , "marking error" );
    ERROR_ASSERT( 50 == rMarkable->offsetToMark( nMark ) , "marking error" );

    rMarkable->jumpToMark( nMark );
    ERROR_ASSERT( 256-10 == rInput->available() , "marking error" );

    rInput->readBytes( seqRead , 10 );
    ERROR_ASSERT( 10 == seqRead.getArray()[0] , "marking error" );

    // pos 20
    {
        sal_Int32 nInBetweenMark = rMarkable->createMark( );
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
Reference < XInterface > SAL_CALL OMarkableInputStreamTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    OMarkableInputStreamTest *p = new OMarkableInputStreamTest( rSMgr );
    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}



Sequence<OUString> OMarkableInputStreamTest_getSupportedServiceNames(void) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OMarkableInputStreamTest_getImplementationName();

    return aRet;
}

OUString     OMarkableInputStreamTest_getServiceName() throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.MarkableInputStream"));
}

OUString    OMarkableInputStreamTest_getImplementationName() throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.extensions.stm.MarkableInputStream" ));
}
