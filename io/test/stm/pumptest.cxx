/*************************************************************************
 *
 *  $RCSfile: pumptest.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 09:27:01 $
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
#include <stdio.h>

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
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/factory.hxx>
#include <osl/mutex.hxx>
#include <osl/thread.h>
#include <list>




using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;

#include "testfactreg.hxx"

class OPumpTest : public WeakImplHelper1 < XSimpleTest >
{
public:
    OPumpTest( const Reference< XMultiServiceFactory >  & rFactory );
    ~OPumpTest();

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
    void testWrongUsage( const Reference < XInterface > & );

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
//      ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
//          ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService(
            OUString( RTL_CONSTASCII_USTRINGPARAM("bla bluzb") ) ), "XServiceInfo test failed" );
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
    if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pump") ) == TestName )  {
        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else if ( 1 == hTestHandle )
            {
                testWrongUsage( TestObject);
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



sal_Bool OPumpTest::testPassed(void)        throw  (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > OPumpTest::getErrors(void)     throw  (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > OPumpTest::getErrorExceptions(void)                     throw  (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > OPumpTest::getWarnings(void)                       throw  (RuntimeException)
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

void OPumpTest::testWrongUsage( const Reference< XInterface > &r )
{
    Reference< XActiveDataSource > rSource ( r, UNO_QUERY );
    Reference< XActiveDataSink > rSink( r , UNO_QUERY );
    Reference< XActiveDataControl > rControl( r, UNO_QUERY );

    Reference< XInputStream > rIn( m_rSmgr->createInstance(
        OUString::createFromAscii( "com.sun.star.io.DataInputStream" )),UNO_QUERY);
    Reference< XOutputStream > rOut( m_rSmgr->createInstance(
        OUString::createFromAscii( "com.sun.star.io.DataOutputStream" )),UNO_QUERY);

    rSink->setInputStream( rIn );
    rSource->setOutputStream( rOut );

    rControl->start();

    //wait a second, so that the pumpthread can terminate
    TimeValue w = {1,1};
    osl_waitThread( &w );
}

Reference< XInterface > SAL_CALL OPumpTest_CreateInstance( const Reference< XMultiServiceFactory > & rSMgr ) throw( Exception )
{
    return *new OPumpTest( rSMgr );
}

Sequence<OUString> OPumpTest_getSupportedServiceNames(void) throw()
{
    OUString s = OPumpTest_getServiceName();
    Sequence< OUString > seq( &s , 1 );
    return seq;

}
OUString     OPumpTest_getServiceName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "test.com.sun.star.io.Pump" ) );
}

OUString    OPumpTest_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "test.com.sun.star.comp.io.Pump") );
}
