/*************************************************************************
 *
 *  $RCSfile: pipetest.cxx,v $
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
#include <smart/com/sun/star/io/XOutputStream.hxx>
#include <smart/com/sun/star/io/XInputStream.hxx>

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
#define IMPLEMENTATION_NAME L"test.com.sun.star.comp.extensions.stm.Pipe"
#define SERVICE_NAME        L"test.com.sun.star.io.Pipe"

#ifndef _VOS_NO_NAMESPACE
using namespace vos;
using namespace usr;
#endif

class WriteToStreamThread :
        public OThread
{

public:

    WriteToStreamThread( XOutputStreamRef xOutput , int iMax )
    {
        m_output = xOutput;
        m_iMax = iMax;
    }

    virtual ~WriteToStreamThread() {}


protected:

    /// Working method which should be overridden.
    virtual void run() {
        for( int i = 0 ; i < m_iMax ; i ++ ) {
            m_output->writeBytes( createIntSeq(i) );
        }
        m_output->closeOutput();
    }

    /** Called when run() is done.
    * You might want to override it to do some cleanup.
    */
    virtual void onTerminated()
    {
        delete this;
    }


private:

    XOutputStreamRef m_output;
    int m_iMax;
};



class OPipeTest :
        public XSimpleTest,
        public OWeakObject
{
public:
    OPipeTest( const XMultiServiceFactoryRef & rFactory );
    ~OPipeTest();

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
    void testSimple( const XInterfaceRef & );
    void testBufferResizing( const XInterfaceRef  & );
    void testMultithreading( const XInterfaceRef & );

private:
    Sequence<UsrAny>  m_seqExceptions;
    Sequence<UString> m_seqErrors;
    Sequence<UString> m_seqWarnings;

};



OPipeTest::OPipeTest( const XMultiServiceFactoryRef &rFactory )
{

}

OPipeTest::~OPipeTest()
{

}


BOOL OPipeTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return OWeakObject::queryInterface( uik , rOut );
    }
    return TRUE;
}


void OPipeTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
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


INT32 OPipeTest::test(  const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.Pipe" == TestName )  {
        TRY {
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
        CATCH( Exception , e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }
        END_CATCH;

        hTestHandle ++;

        if( 4 == hTestHandle ) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        THROW( IllegalArgumentException() );
    }
    return hTestHandle;
}



BOOL OPipeTest::testPassed(void)                                        THROWS( (UsrSystemException) )
{
    return m_seqErrors.getLen() == 0;
}


Sequence< UString > OPipeTest::getErrors(void)                          THROWS( (UsrSystemException) )
{
    return m_seqErrors;
}


Sequence< UsrAny > OPipeTest::getErrorExceptions(void)                  THROWS( (UsrSystemException) )
{
    return m_seqExceptions;
}


Sequence< UString > OPipeTest::getWarnings(void)                        THROWS( (UsrSystemException) )
{
    return m_seqWarnings;
}


/***
* the test methods
*
****/


void OPipeTest::testSimple( const XInterfaceRef &r )
{

    XInputStreamRef input( r , USR_QUERY );
    XOutputStreamRef output( r , USR_QUERY );

    ERROR_ASSERT( input.is()  , "queryInterface on XInputStream failed" );
    ERROR_ASSERT( output.is() , "queryInterface onXOutputStream failed" );

    // basic read/write
    Sequence<BYTE> seqWrite = createSeq( "Hallo, du Ei !" );

    Sequence<BYTE> seqRead;
    for( int i = 0 ; i < 5000 ; i ++ ) {
        output->writeBytes( seqWrite );
        input->readBytes( seqRead , input->available() );

        ERROR_ASSERT( ! strcmp( (char *) seqWrite.getArray() , (char * )seqRead.getArray() ) ,
                      "error during read/write/skip" );
        ERROR_ASSERT( 0 == input->available() ,
                      "error during read/write/skip" );

        // available shouldn't return a negative value
        input->skipBytes( seqWrite.getLen() - 5 );
        ERROR_ASSERT( 0 == input->available() , "wrong available after skip" );

        // 5 bytes should be available
        output->writeBytes( seqWrite );
        ERROR_ASSERT( 5 == input->available() , "wrong available after skip/write " );

        input->readBytes( seqRead , 5 );
        ERROR_ASSERT(   ! strcmp(   (char*) seqRead.getArray() ,
                            (char*) &( seqWrite.getArray()[seqWrite.getLen()-5] ) ),
                        "write/read mismatich" );

    }

    output->writeBytes( seqWrite );
    ERROR_ASSERT( seqWrite.getLen() == input->available(), "wrong available() after write" );

    ERROR_ASSERT( 10 == input->readSomeBytes( seqRead , 10 ) , "maximal number of bytes ignored" );
    ERROR_ASSERT( seqWrite.getLen() -10 == input->readSomeBytes( seqRead , 100 ) ,
                                                            "something wrong with readSomeBytes" );


    output->closeOutput();
    TRY {
        output->writeBytes( Sequence<BYTE> (100) );
        ERROR_ASSERT( 0 , "writing on a closed stream does not cause an exception" );
    }
    CATCH (IOException , e ) {
        e;      // just to suppress warning during compile
    }
    END_CATCH;

    ERROR_ASSERT(! input->readBytes( seqRead , 1 ), "eof not found !" );

    input->closeInput();
    TRY {
        input->readBytes( seqRead , 1 );
        ERROR_ASSERT( 0 , "reading from a closed stream does not cause an exception" );
    }
    CATCH( IOException , e ) {
        e;          // just to suppress warning during compile
    }
    END_CATCH;

}

void OPipeTest::testBufferResizing( const XInterfaceRef &r )
{

    int iMax = 20000;
    XInputStreamRef input( r , USR_QUERY );
    XOutputStreamRef output( r , USR_QUERY );

    ERROR_ASSERT( input.is()  , "queryInterface on XInputStream failed" );
    ERROR_ASSERT( output.is() , "queryInterface on XOutputStream failed" );

    Sequence<BYTE> seqRead;

    // this is just to better check the
    // internal buffers
    output->writeBytes( Sequence<BYTE>(100) );
    input->readBytes( Sequence<BYTE>() , 100);

    for( int i = 0 ; i < iMax ; i ++ ) {
        output->writeBytes( createIntSeq( i ) );
    }

    for( i = 0 ; i < iMax ; i ++ ) {
        input->readBytes( seqRead, createIntSeq(i).getLen() );
        ERROR_ASSERT( ! strcmp(     (char*) seqRead.getArray() ,
                                    (char*) createIntSeq(i).getArray() ) ,
                        "written/read mismatch\n" );
    }

    output->closeOutput();
    ERROR_ASSERT( ! input->readBytes( seqRead , 1 ) , "eof not reached !" );
    input->closeInput();
}



void OPipeTest::testMultithreading( const XInterfaceRef &r )
{


    int iMax = 30000;

    XInputStreamRef input( r , USR_QUERY );
    XOutputStreamRef output( r , USR_QUERY );

    ERROR_ASSERT( input.is()  , "queryInterface on XInputStream failed"  );
    ERROR_ASSERT( output.is() , "queryInterface on XOutputStream failed" );

    Sequence<BYTE> seqRead;

    // deletes itself
    OThread *p = new WriteToStreamThread( output,  iMax );

    ERROR_ASSERT( p , "couldn't create thread for testing !\n" );

    p->create();

    for(int  i = 0 ; TRUE ; i ++ ) {
        if( 0 == input->readBytes( seqRead, createIntSeq(i).getLen() ) ) {
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

/*  {
        TRY {
            XInterfaceRef x = xSMgr->createInstance( strService );

            XInputStreamRef input( x , USR_QUERY );
            XOutputStreamRef output( x , USR_QUERY );

            assert( output.is() );
            while(  TRUE ) {
                // basic read/write
                Sequence<BYTE> seqWrite( 500 );
                output->writeBytes( seqWrite );

            }
        }
        CATCH( IOException , e ) {
            printf( "%s %s\n" , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() ,
                                UStringToString( e.Message , CHARSET_SYSTEM ).GetCharStr() );
        }
    }
*/




/**
* for external binding
*
*
**/
XInterfaceRef OPipeTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    OPipeTest *p = new OPipeTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}



Sequence<UString> OPipeTest_getSupportedServiceNames(void) THROWS( () )
{
    Sequence<UString> aRet(1);
    aRet.getArray()[0] = OPipeTest_getImplementationName();

    return aRet;
}

UString     OPipeTest_getServiceName() THROWS( () )
{
    return SERVICE_NAME;
}

UString     OPipeTest_getImplementationName() THROWS( () )
{
    return IMPLEMENTATION_NAME;
}
