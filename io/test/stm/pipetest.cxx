/*************************************************************************
 *
 *  $RCSfile: pipetest.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:24:19 $
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
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/factory.hxx>

#include <cppuhelper/implbase1.hxx>      // OWeakObject

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <vos/thread.hxx>

#include <assert.h>
#include <string.h>

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
// streams

#include "testfactreg.hxx"
#define IMPLEMENTATION_NAME "test.com.sun.star.comp.extensions.stm.Pipe"
#define SERVICE_NAME        "test.com.sun.star.io.Pipe"


class WriteToStreamThread :
        public OThread
{

public:

    WriteToStreamThread( Reference< XOutputStream >  xOutput , int iMax )
    {
        m_output = xOutput;
        m_iMax = iMax;
    }

    virtual ~WriteToStreamThread() {}


protected:

    /// Working method which should be overridden.
    virtual void SAL_CALL run() {
        for( int i = 0 ; i < m_iMax ; i ++ ) {
            m_output->writeBytes( createIntSeq(i) );
        }
        m_output->closeOutput();
    }

    /** Called when run() is done.
    * You might want to override it to do some cleanup.
    */
    virtual void SAL_CALL onTerminated()
    {
        delete this;
    }


private:

    Reference < XOutputStream >  m_output;
    int m_iMax;
};



class OPipeTest : public WeakImplHelper1 < XSimpleTest >
{
public:
    OPipeTest( const Reference< XMultiServiceFactory >  & rFactory );
    ~OPipeTest();

public: // implementation names
    static Sequence< OUString >     getSupportedServiceNames_Static(void) throw();
    static OUString                 getImplementationName_Static() throw();

public:
    virtual void SAL_CALL testInvariant(const OUString& TestName, const Reference < XInterface >& TestObject)
        throw  ( IllegalArgumentException, RuntimeException) ;

    virtual sal_Int32 SAL_CALL test(    const OUString& TestName,
                                        const Reference < XInterface >& TestObject,
                                        sal_Int32 hTestHandle)
        throw  (    IllegalArgumentException,
                    RuntimeException);

    virtual sal_Bool SAL_CALL testPassed(void)                              throw  (    RuntimeException) ;
    virtual Sequence< OUString > SAL_CALL getErrors(void)               throw  (RuntimeException) ;
    virtual Sequence< Any > SAL_CALL getErrorExceptions(void)       throw  (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings(void)                 throw  (RuntimeException);

private:
    void testSimple( const Reference < XInterface > & );
    void testBufferResizing( const Reference < XInterface >  & );
    void testMultithreading( const Reference < XInterface > & );

private:
    Sequence<Any>  m_seqExceptions;
    Sequence<OUString> m_seqErrors;
    Sequence<OUString> m_seqWarnings;

};



OPipeTest::OPipeTest( const Reference< XMultiServiceFactory > &rFactory )
{

}

OPipeTest::~OPipeTest()
{

}



void OPipeTest::testInvariant( const OUString& TestName, const Reference < XInterface >& TestObject )
    throw  (    IllegalArgumentException,
                RuntimeException)
{
    Reference< XServiceInfo > info( TestObject, UNO_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService(
            OUString( RTL_CONSTASCII_USTRINGPARAM("bla bluzb") ) ), "XServiceInfo test failed" );
    }

}


sal_Int32 OPipeTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw  (    IllegalArgumentException, RuntimeException)
{
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pipe") ) == TestName )  {
        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else if( 1 == hTestHandle ) {
                testSimple( TestObject );
            }
            else if( 2 == hTestHandle ) {
                testBufferResizing( TestObject );
            }
            else if( 3 == hTestHandle ) {
                testMultithreading( TestObject );
            }
        }
        catch( Exception & e )
        {
            OString s = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            BUILD_ERROR( 0 , s.getStr() );
        }
        catch( ... )
        {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( 4 == hTestHandle )
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



sal_Bool OPipeTest::testPassed(void)        throw  (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OPipeTest::getErrors(void)     throw  (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OPipeTest::getErrorExceptions(void)                     throw  (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OPipeTest::getWarnings(void)                       throw  (RuntimeException)
{
    return m_seqWarnings;
}


/***
* the test methods
*
****/


void OPipeTest::testSimple( const Reference < XInterface > &r )
{

    Reference< XInputStream > input( r , UNO_QUERY );
    Reference < XOutputStream > output( r , UNO_QUERY );

    ERROR_ASSERT( input.is()  , "queryInterface on XInputStream failed" );
    ERROR_ASSERT( output.is() , "queryInterface onXOutputStream failed" );

    // basic read/write
    Sequence<sal_Int8> seqWrite = createSeq( "Hallo, du Ei !" );

    Sequence<sal_Int8> seqRead;
    for( int i = 0 ; i < 5000 ; i ++ ) {
        output->writeBytes( seqWrite );
        input->readBytes( seqRead , input->available() );

        ERROR_ASSERT( ! strcmp( (char *) seqWrite.getArray() , (char * )seqRead.getArray() ) ,
                      "error during read/write/skip" );
        ERROR_ASSERT( 0 == input->available() ,
                      "error during read/write/skip" );

        // available shouldn't return a negative value
        input->skipBytes( seqWrite.getLength() - 5 );
        ERROR_ASSERT( 0 == input->available() , "wrong available after skip" );

        // 5 bytes should be available
        output->writeBytes( seqWrite );
        ERROR_ASSERT( 5 == input->available() , "wrong available after skip/write " );

        input->readBytes( seqRead , 5 );
        ERROR_ASSERT(   ! strcmp(   (char*) seqRead.getArray() ,
                            (char*) &( seqWrite.getArray()[seqWrite.getLength()-5] ) ),
                        "write/read mismatich" );

    }

    output->writeBytes( seqWrite );
    ERROR_ASSERT( seqWrite.getLength() == input->available(), "wrong available() after write" );

    ERROR_ASSERT( 10 == input->readSomeBytes( seqRead , 10 ) , "maximal number of bytes ignored" );
    ERROR_ASSERT( seqWrite.getLength() -10 == input->readSomeBytes( seqRead , 100 ) ,
                                                            "something wrong with readSomeBytes" );


    output->closeOutput();
    try{
        output->writeBytes( Sequence<sal_Int8> (100) );
        ERROR_ASSERT( 0 , "writing on a closed stream does not cause an exception" );
    }
    catch (IOException & )
    {
    }

    ERROR_ASSERT(! input->readBytes( seqRead , 1 ), "eof not found !" );

    input->closeInput();
    try
    {
        input->readBytes( seqRead , 1 );
        ERROR_ASSERT( 0 , "reading from a closed stream does not cause an exception" );
    }
    catch( IOException & ) {
    }

}

void OPipeTest::testBufferResizing( const Reference < XInterface > &r )
{

    int iMax = 20000;
    Reference< XInputStream > input( r , UNO_QUERY );
    Reference < XOutputStream > output( r , UNO_QUERY );

    ERROR_ASSERT( input.is()  , "queryInterface on XInputStream failed" );
    ERROR_ASSERT( output.is() , "queryInterface on XOutputStream failed" );

    Sequence<sal_Int8> seqRead;

    // this is just to better check the
    // internal buffers
    output->writeBytes( Sequence<sal_Int8>(100) );
    input->readBytes( Sequence<sal_Int8>() , 100);

    for( int i = 0 ; i < iMax ; i ++ ) {
        output->writeBytes( createIntSeq( i ) );
    }

    for( i = 0 ; i < iMax ; i ++ ) {
        input->readBytes( seqRead, createIntSeq(i).getLength() );
        ERROR_ASSERT( ! strcmp(     (char*) seqRead.getArray() ,
                                    (char*) createIntSeq(i).getArray() ) ,
                        "written/read mismatch\n" );
    }

    output->closeOutput();
    ERROR_ASSERT( ! input->readBytes( seqRead , 1 ) , "eof not reached !" );
    input->closeInput();
}



void OPipeTest::testMultithreading( const Reference < XInterface > &r )
{


    int iMax = 30000;

    Reference< XInputStream > input( r , UNO_QUERY );
    Reference < XOutputStream > output( r , UNO_QUERY );

    ERROR_ASSERT( input.is()  , "queryInterface on XInputStream failed"  );
    ERROR_ASSERT( output.is() , "queryInterface on XOutputStream failed" );

    Sequence<sal_Int8> seqRead;

    // deletes itself
    OThread *p = new WriteToStreamThread( output,  iMax );

    ERROR_ASSERT( p , "couldn't create thread for testing !\n" );

    p->create();

    for(int  i = 0 ; sal_True ; i ++ ) {
        if( 0 == input->readBytes( seqRead, createIntSeq(i).getLength() ) ) {
            // eof reached !
            break;
        }

        ERROR_ASSERT( ! strcmp(     (char*) seqRead.getArray() ,
                                    (char*) createIntSeq(i).getArray() ) ,
                        "written/read mismatch\n" );
    }

    ERROR_ASSERT( i == iMax , "less elements read than written !");
    input->closeInput();
}



/**
* for external binding
*
*
**/
Reference < XInterface > SAL_CALL OPipeTest_CreateInstance( const Reference< XMultiServiceFactory>  & rSMgr ) throw (Exception)
{
    OPipeTest *p = new OPipeTest( rSMgr );
    Reference< XInterface > x ( SAL_STATIC_CAST( OWeakObject * , p ) );
    return x;
}



Sequence<OUString> OPipeTest_getSupportedServiceNames(void) throw()
{
      Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OPipeTest_getServiceName();

      return aRet;
}

OUString     OPipeTest_getServiceName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICE_NAME ) );
}

OUString    OPipeTest_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPLEMENTATION_NAME ) );
}
