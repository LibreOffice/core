/*************************************************************************
 *
 *  $RCSfile: datatest.cxx,v $
 *
 *  $Revision: 1.5 $
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
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/io/XMarkableStream.hpp>
#include <com/sun/star/io/XConnectable.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>

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
//using namespace ::vos;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::beans;
// streams

#include "testfactreg.hxx"

#define DATASTREAM_TEST_MAX_HANDLE 1

/****
* The following test class tests XDataInputStream and XDataOutputStream at equal terms,
* so when errors occur, it may be in either one implementation.
* The class also  uses stardiv.uno.io.pipe. If problems occur, make sure to run also the
* pipe test routines ( test.com.sun.star.io.pipe ).
*
*
*****/

class ODataStreamTest :
        public WeakImplHelper1< XSimpleTest >
{
public:
    ODataStreamTest( const Reference < XMultiServiceFactory > & rFactory ) :
        m_rFactory( rFactory )
        {}

public:
    virtual void SAL_CALL testInvariant(const OUString& TestName, const Reference < XInterface >& TestObject)
        throw ( IllegalArgumentException,
                    RuntimeException);

    virtual sal_Int32 SAL_CALL test(    const OUString& TestName,
                                        const Reference < XInterface >& TestObject,
                                        sal_Int32 hTestHandle)
        throw ( IllegalArgumentException,
                RuntimeException);

    virtual sal_Bool SAL_CALL testPassed(void)                              throw ( RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors(void)               throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions(void)       throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings(void)                 throw (RuntimeException);

private:
    void testSimple( const Reference < XDataInputStream > & , const Reference < XDataOutputStream > &);

protected:
    Sequence<Any>  m_seqExceptions;
    Sequence<OUString> m_seqErrors;
    Sequence<OUString> m_seqWarnings;

    Reference < XMultiServiceFactory > m_rFactory;
};




void ODataStreamTest::testInvariant(
    const OUString& TestName,
    const Reference < XInterface >& TestObject )
    throw ( IllegalArgumentException,
            RuntimeException)
{
    if( OUString::createFromAscii("com.sun.star.io.DataInputStream") == TestName ) {
        Reference < XConnectable > connect( TestObject , UNO_QUERY );
        Reference < XActiveDataSink > active( TestObject , UNO_QUERY );
        Reference < XInputStream >  input( TestObject , UNO_QUERY );
        Reference < XDataInputStream > dataInput( TestObject , UNO_QUERY );

        WARNING_ASSERT( connect.is(), "XConnectable cannot be queried" );
        WARNING_ASSERT( active.is() , "XActiveDataSink cannot be queried" );
        ERROR_ASSERT( input.is() , "XInputStream cannot be queried" );
        ERROR_ASSERT( dataInput.is() , "XDataInputStream cannot be queried" );


    }
    else if( OUString::createFromAscii("com.sun.star.io.DataInputStream") == TestName ) {
        Reference < XConnectable >  connect( TestObject , UNO_QUERY );
        Reference < XActiveDataSource > active( TestObject , UNO_QUERY );
        Reference < XOutputStream > output( TestObject , UNO_QUERY );
        Reference < XDataOutputStream > dataOutput( TestObject , UNO_QUERY );

        WARNING_ASSERT( connect.is(),       "XConnectable cannot be queried" );
        WARNING_ASSERT( active.is() ,   "XActiveDataSink cannot be queried" );
        ERROR_ASSERT(   output.is() ,   "XInputStream cannot be queried" );
        ERROR_ASSERT(   dataOutput.is(),    "XDataInputStream cannot be queried" );

    }

    Reference < XServiceInfo >  info( TestObject, UNO_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService( OUString::createFromAscii("bla bluzb") ) , "XServiceInfo test failed" );
    }

}


sal_Int32 ODataStreamTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw ( IllegalArgumentException,
            RuntimeException)
{
    if( OUString::createFromAscii("com.sun.star.io.DataInputStream") == TestName ||
        OUString::createFromAscii("com.sun.star.io.DataOutputStream") == TestName )  {

        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else {
                Reference <XActiveDataSink > rSink( TestObject, UNO_QUERY );
                Reference <XActiveDataSource > rSource( TestObject , UNO_QUERY );

                Reference < XDataInputStream > rInput( TestObject , UNO_QUERY );
                Reference < XDataOutputStream > rOutput( TestObject , UNO_QUERY );


                Reference < XInterface > x = m_rFactory->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pipe" )));

                Reference < XInputStream >   rPipeInput( x , UNO_QUERY );
                Reference < XOutputStream >  rPipeOutput( x , UNO_QUERY );

                if( ! rSink.is() ) {
                    x = m_rFactory->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.DataInputStream")) );
                    rInput = Reference < XDataInputStream > ( x , UNO_QUERY);
                    rSink = Reference<  XActiveDataSink > ( x , UNO_QUERY );
                }
                else if ( !rSource.is() )
                {
                    x = m_rFactory->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.DataOutputStream") ) );
                    rOutput = Reference< XDataOutputStream > ( x , UNO_QUERY );
                    rSource = Reference< XActiveDataSource > ( x, UNO_QUERY );
                }

                assert( rPipeInput.is() );
                assert( rPipeOutput.is() );
                rSink->setInputStream( rPipeInput );
                rSource->setOutputStream( rPipeOutput );

                assert( rSink->getInputStream().is() );
                assert( rSource->getOutputStream().is() );

                if( 1 == hTestHandle ) {
                    testSimple( rInput , rOutput );
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

        if( hTestHandle >= 2) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        BUILD_ERROR( 0 , "service not supported by test." );
    }
    return hTestHandle;
}



sal_Bool ODataStreamTest::testPassed(void)                                      throw (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > ODataStreamTest::getErrors(void)                           throw (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > ODataStreamTest::getErrorExceptions(void)                   throw (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > ODataStreamTest::getWarnings(void)                         throw (RuntimeException)
{
    return m_seqWarnings;
}

void ODataStreamTest::testSimple(   const Reference < XDataInputStream > &rInput,
                                    const Reference < XDataOutputStream > &rOutput )
{
    rOutput->writeLong( 0x34ff3c );
    rOutput->writeLong( 0x34ff3d );
    rOutput->writeLong( -1027 );

    ERROR_ASSERT( 0x34ff3c ==  rInput->readLong() , "long read/write mismatch" );
    ERROR_ASSERT( 0x34ff3d ==  rInput->readLong() , "long read/write mismatch" );
    ERROR_ASSERT( -1027    ==  rInput->readLong() , "long read/write mismatch" );

    rOutput->writeByte( 0x77 );
    ERROR_ASSERT( 0x77 == rInput->readByte() , "byte read/write mismatch" );

    rOutput->writeBoolean( 25 );
    ERROR_ASSERT( rInput->readBoolean() , "boolean read/write mismatch" );

    rOutput->writeBoolean( sal_False );
    ERROR_ASSERT( ! rInput->readBoolean() , "boolean read/write mismatch" );

    rOutput->writeFloat( (float) 42.42 );
    ERROR_ASSERT( rInput->readFloat() == ((float)42.42) , "float read/write mismatch" );

    rOutput->writeDouble( (double) 42.42 );
    ERROR_ASSERT( rInput->readDouble() == 42.42 , "double read/write mismatch" );

    rOutput->writeHyper( 0x123456789abcdef );
    ERROR_ASSERT( rInput->readHyper() == 0x123456789abcdef , "int64 read/write mismatch" );

    rOutput->writeUTF( OUString::createFromAscii("Live long and prosper !") );
    ERROR_ASSERT( rInput->readUTF() == OUString::createFromAscii("Live long and prosper !") ,
                    "UTF read/write mismatch" );

    Sequence<sal_Unicode> wc(0x10001);
    for( int i = 0 ; i < 0x10000  ; i ++ ) {
        wc.getArray()[i] = L'c';
    }
    wc.getArray()[0x10000] = 0;
    OUString str( wc.getArray() , 0x10000 );
    rOutput->writeUTF( str );
    ERROR_ASSERT( rInput->readUTF() == str , "error reading 64k block" );

    rOutput->closeOutput();
    try
    {
        rInput->readLong();
        ERROR_ASSERT( 0 , "eof-exception does not occur !" );
    }
    catch ( IOException & )
    {
        //ok
    }
    catch( ... )
    {
        ERROR_ASSERT( 0 , "wrong exception after reading beyond eof" );
    }

    ERROR_ASSERT( ! rInput->readBytes( Sequence<sal_Int8> (1) , 1 ),
                    "stream must be on eof !" );

    rInput->closeInput();

    try
    {
        rOutput->writeByte( 1 );
        ERROR_ASSERT( 0 , "writing still possible though chain must be interrupted" );
    }
    catch( IOException & )
    {
        // ok
    }
    catch( ... ) {
        ERROR_ASSERT( 0 , "IOException  expected, but another exception was thrown" );
    }

}



/**
* for external binding
*
*
**/
Reference < XInterface > SAL_CALL ODataStreamTest_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    ODataStreamTest *p = new ODataStreamTest( rSMgr );
    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}

Sequence<OUString> ODataStreamTest_getSupportedServiceNames( int i) throw ()
{
    Sequence<OUString> aRet(1);

    aRet.getArray()[0] = ODataStreamTest_getImplementationName( i);


    return aRet;
}

OUString     ODataStreamTest_getServiceName( int i) throw ()
{
    if( 1 == i ) {
        return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.DataInputStream" ));
    }
    else {
        return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.DataOutputStream" ));
    }
}

OUString    ODataStreamTest_getImplementationName( int i) throw ()
{
    if( 1 == i ) {
        return OUString(
            RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.comp.extensions.stm.DataInputStream") );
    }
    else {
        return OUString( RTL_CONSTASCII_USTRINGPARAM(
            "test.com.sun.star.comp.extensions.stm.DataOutputStream" ) );
    }
}


/**------------------------------------------------------
*
*
*
*
*
*------------------------------------------------------*/
class MyPersistObject : public WeakImplHelper2< XPersistObject , XPropertySet >
{
public:
    MyPersistObject( ) : m_sServiceName( OMyPersistObject_getServiceName() )
        {}
    MyPersistObject( const OUString & sServiceName ) : m_sServiceName( sServiceName )
        {}


public:
    virtual OUString SAL_CALL getServiceName(void)  throw (RuntimeException);
    virtual void SAL_CALL write( const Reference< XObjectOutputStream >& OutStream )
        throw (IOException, RuntimeException);
    virtual void SAL_CALL read(const Reference< XObjectInputStream >& InStream)
        throw (IOException, RuntimeException);

public:

    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo(void)
        throw (RuntimeException);

    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName, const Any& aValue)
        throw ( UnknownPropertyException,
                PropertyVetoException,
                IllegalArgumentException,
                WrappedTargetException,
                RuntimeException);
    virtual Any SAL_CALL getPropertyValue(const OUString& PropertyName)
        throw ( UnknownPropertyException,
                WrappedTargetException,
                RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& aPropertyName,
        const Reference < XPropertyChangeListener > & xListener)
        throw ( UnknownPropertyException,
                WrappedTargetException,
                RuntimeException);

    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& aPropertyName,
        const Reference< XPropertyChangeListener > & aListener)
        throw ( UnknownPropertyException,
                WrappedTargetException,
                RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& PropertyName,
        const Reference< XVetoableChangeListener > & aListener)
        throw ( UnknownPropertyException,
                WrappedTargetException,
                RuntimeException);

    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& PropertyName,
        const Reference<  XVetoableChangeListener >& aListener)
        throw ( UnknownPropertyException,
                WrappedTargetException,
                RuntimeException);

public:
    sal_Int32   m_l;
    float   m_f;
    double  m_d;
    sal_Bool    m_b;
    sal_Int8    m_byte;
    sal_Unicode m_c;
    OUString    m_s;
    Reference< XPersistObject > m_ref;
    OUString m_sServiceName;
};



Reference <XPropertySetInfo > MyPersistObject::getPropertySetInfo(void)
    throw (RuntimeException)
{
    return Reference< XPropertySetInfo >();
}

void MyPersistObject::setPropertyValue(
    const OUString& aPropertyName,
    const Any& aValue)
    throw ( UnknownPropertyException,
            PropertyVetoException,
            IllegalArgumentException,
            WrappedTargetException,
            RuntimeException)
{
    if( 0 == aPropertyName.compareToAscii("long")  ) {
        aValue >>= m_l;
    }
    else if ( 0 == aPropertyName.compareToAscii("float") ) {
        aValue >>= m_f;
    }
    else if( 0 == aPropertyName.compareToAscii("double") ) {
        aValue >>= m_d;
    }
    else if( 0  == aPropertyName.compareToAscii("bool") ) {
        aValue >>= m_b;
    }
    else if( 0 == aPropertyName.compareToAscii("byte" ) ) {
        aValue >>= m_byte;
    }
    else if( 0 == aPropertyName.compareToAscii("char") ) {
        aValue >>= m_c;
    }
    else if( 0 == aPropertyName.compareToAscii("string") ) {
        aValue >>= m_s;
    }
    else if( 0 == aPropertyName.compareToAscii("object") ) {
        if( aValue.getValueType() == getCppuType( (Reference< XPersistObject> *)0 ) )
        {
            aValue >>= m_ref;
        }
        else
        {
            m_ref = 0;
        }
    }
}


Any MyPersistObject::getPropertyValue(const OUString& aPropertyName)
    throw ( UnknownPropertyException,
            WrappedTargetException,
            RuntimeException)
{
    Any aValue;
    if( 0 == aPropertyName.compareToAscii("long" )  ) {
        aValue <<= m_l;
    }
    else if ( 0 == aPropertyName.compareToAscii("float") ) {
        aValue <<= m_f;
    }
    else if( 0 == aPropertyName.compareToAscii("double") ) {
        aValue <<= m_d;
    }
    else if( 0 == aPropertyName.compareToAscii("bool") ) {
        aValue <<= m_b;
    }
    else if( 0 == aPropertyName.compareToAscii("byte") ) {
        aValue <<= m_byte;
    }
    else if( 0 == aPropertyName.compareToAscii("char" ) ) {
        aValue <<= m_c;
    }
    else if( 0 == aPropertyName.compareToAscii("string") ) {
        aValue <<= m_s;
    }
    else if( 0 == aPropertyName.compareToAscii("object" ) )
    {
        aValue <<= m_ref;
    }
    return aValue;
}


void MyPersistObject::addPropertyChangeListener(
    const OUString& aPropertyName,
    const Reference< XPropertyChangeListener > & xListener)
    throw ( UnknownPropertyException,
            WrappedTargetException,
            RuntimeException)
{

}

void MyPersistObject::removePropertyChangeListener(
    const OUString& aPropertyName,
    const Reference < XPropertyChangeListener > & aListener)
    throw ( UnknownPropertyException,
            WrappedTargetException,
            RuntimeException)
{
}


void MyPersistObject::addVetoableChangeListener(
    const OUString& PropertyName,
    const Reference <XVetoableChangeListener >& aListener)
    throw ( UnknownPropertyException,
            WrappedTargetException,
            RuntimeException)
{

}

void MyPersistObject::removeVetoableChangeListener(
    const OUString& PropertyName,
    const Reference < XVetoableChangeListener > & aListener)
    throw ( UnknownPropertyException,
            WrappedTargetException,
            RuntimeException)
{

}




OUString MyPersistObject::getServiceName() throw (RuntimeException)
{
    return m_sServiceName;
}

void MyPersistObject::write( const Reference< XObjectOutputStream >  & rOut )
    throw (IOException,RuntimeException)
{
    rOut->writeLong( m_l);
    rOut->writeFloat( m_f );
    rOut->writeDouble( m_d );
    rOut->writeBoolean( m_b );
    rOut->writeByte( m_byte );
    rOut->writeChar( m_c );
    rOut->writeUTF( m_s );
    rOut->writeObject( m_ref );
}


void MyPersistObject::read( const Reference< XObjectInputStream > & rIn )
    throw (IOException, RuntimeException)
{
    m_l = rIn->readLong();
    m_f = rIn->readFloat();
    m_d = rIn->readDouble();
    m_b = rIn->readBoolean();
    m_byte = rIn->readByte();
    m_c = rIn->readChar();
    m_s = rIn->readUTF();
    m_ref = rIn->readObject();
}

Reference < XInterface > SAL_CALL OMyPersistObject_CreateInstance(
    const Reference < XMultiServiceFactory > & rSMgr )
    throw(Exception)
{
    MyPersistObject *p = new MyPersistObject( );
    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}

Sequence<OUString> OMyPersistObject_getSupportedServiceNames( ) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OMyPersistObject_getImplementationName();
    return aRet;
}

OUString     OMyPersistObject_getServiceName( ) throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.PersistTest" ));
}

OUString    OMyPersistObject_getImplementationName( ) throw ()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "test.com.sun.star.io.PersistTest" ) );
}


// ---------------------------------------------
// -----------------------------------------------
class OObjectStreamTest :
        public ODataStreamTest
{
public:
    OObjectStreamTest( const Reference < XMultiServiceFactory > &r) : ODataStreamTest(r) {}

public:
    virtual void SAL_CALL testInvariant(const OUString& TestName,
                               const Reference < XInterface >& TestObject)
        throw ( IllegalArgumentException,
                RuntimeException);

    virtual sal_Int32 SAL_CALL test(
        const OUString& TestName,
        const Reference < XInterface >& TestObject,
        sal_Int32 hTestHandle)
        throw ( IllegalArgumentException,
                RuntimeException);


private:
    void OObjectStreamTest::testObject(     const Reference <XObjectOutputStream > &rOut,
                                            const Reference <XObjectInputStream> &rIn );

private:
};


void OObjectStreamTest::testInvariant( const OUString& TestName,
                                       const Reference < XInterface >& TestObject )
    throw ( IllegalArgumentException, RuntimeException)
{

    if( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.io.ObjectInputStream" ) )
                  == TestName )
    {
        ODataStreamTest::testInvariant( TestName , TestObject );
        Reference< XObjectInputStream > dataInput( TestObject , UNO_QUERY );
        Reference< XMarkableStream >  markable( TestObject , UNO_QUERY );
        ERROR_ASSERT( dataInput.is() , "XObjectInputStream cannot be queried" );
        ERROR_ASSERT(   markable.is() , "XMarkableStream cannot be queried" );
    }
    else if( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.ObjectOutputStream") )
             == TestName )
    {
        ODataStreamTest::testInvariant( TestName , TestObject );
        Reference < XMarkableStream > markable( TestObject , UNO_QUERY );
        Reference < XObjectOutputStream > dataOutput( TestObject , UNO_QUERY );
        ERROR_ASSERT(   dataOutput.is(),    "XObjectOutputStream cannot be queried" );
        ERROR_ASSERT(   markable.is() , "XMarkableStream cannot be queried" );
    }

    Reference < XServiceInfo > info( TestObject, UNO_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService( OUString::createFromAscii("bla bluzb") ) , "XServiceInfo test failed" );
    }

}

sal_Int32 OObjectStreamTest::test(  const OUString& TestName,
                        const Reference < XInterface >& TestObject,
                        sal_Int32 hTestHandle)
    throw ( IllegalArgumentException,
            RuntimeException)
{
    if( 0 == TestName.compareToAscii("com.sun.star.io.ObjectInputStream") ||
        0 == TestName.compareToAscii("com.sun.star.io.ObjectOutputStream" ) )  {

        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else if( DATASTREAM_TEST_MAX_HANDLE >= hTestHandle ) {
                sal_Int32 hOldHandle = hTestHandle;
                hTestHandle = ODataStreamTest::test(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.DataInputStream" )),
                    TestObject , hTestHandle );
                if( hTestHandle == -1 ){
                    hTestHandle = hOldHandle;
                }
            }
            else {

                Reference<XActiveDataSink >  rSink( TestObject, UNO_QUERY );
                Reference<XActiveDataSource >  rSource( TestObject , UNO_QUERY );

                Reference< XObjectInputStream >  rInput( TestObject , UNO_QUERY );
                Reference< XObjectOutputStream > rOutput( TestObject , UNO_QUERY );


                Reference < XInterface > x = m_rFactory->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.Pipe" )) );

                Reference <XInputStream > rPipeInput( x , UNO_QUERY );
                Reference <XOutputStream >  rPipeOutput( x , UNO_QUERY );

                x = m_rFactory->createInstance(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.MarkableInputStream") ) );

                Reference <XInputStream > markableInput( x , UNO_QUERY );
                Reference <XActiveDataSink> markableSink( x , UNO_QUERY );

                x = m_rFactory->createInstance( OUString(
                    RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.MarkableOutputStream" ) ) );
                Reference <XOutputStream >  markableOutput( x , UNO_QUERY );
                Reference <XActiveDataSource >  markableSource( x , UNO_QUERY );

                assert( markableInput.is()  );
                assert( markableOutput.is() );
                assert( markableSink.is()   );
                assert( markableSource.is() );

                markableSink->setInputStream( rPipeInput );
                markableSource->setOutputStream( rPipeOutput );

                if( ! rSink.is() ) {
                    x = m_rFactory->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.ObjectInputStream") ));
                    rInput = Reference < XObjectInputStream > ( x , UNO_QUERY );
                    rSink = Reference < XActiveDataSink > ( x , UNO_QUERY );
                }
                else if ( !rSource.is() ) {
                    x = m_rFactory->createInstance(
                        OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.io.ObjectOutputStream" )));
                    rOutput = Reference <XObjectOutputStream > ( x , UNO_QUERY );
                    rSource = Reference <XActiveDataSource>( x, UNO_QUERY );
                }

                assert( rPipeInput.is() );
                assert( rPipeOutput.is() );

                rSink->setInputStream( markableInput );
                rSource->setOutputStream( markableOutput );

                assert( rSink->getInputStream().is() );
                assert( rSource->getOutputStream().is() );

                if( 1 + DATASTREAM_TEST_MAX_HANDLE == hTestHandle ) {
                    testObject( rOutput , rInput);
                }
                rInput->closeInput();
                rOutput->closeOutput();

            }
        }
        catch( Exception &e )  {
            OString o = OUStringToOString( e.Message , RTL_TEXTENCODING_ASCII_US );
            BUILD_ERROR( 0 , o.getStr() );
        }
        catch( ... ) {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }

        hTestHandle ++;

        if( hTestHandle > 1 +DATASTREAM_TEST_MAX_HANDLE ) {
            // all tests finished.
            hTestHandle = -1;
        }
    }
    else {
        BUILD_ERROR( 0 , "service not supported by test." );
    }
    return hTestHandle;
}


sal_Bool compareMyPropertySet( Reference< XPropertySet > &r1 , Reference < XPropertySet > &r2 )
{
    sal_Bool b = sal_True;

    if( r1->getPropertyValue( OUString::createFromAscii("long") ).getValueType() == getCppuVoidType() ||
        r2->getPropertyValue( OUString::createFromAscii("long") ).getValueType() == getCppuVoidType() ) {

        // one of the objects is not the correct propertyset !
        return sal_False;
    }

    b = b && (  r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("long")) ) ==
                r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("long")) ) );

    b = b && (  r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("float")) ) ==
                r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("float")) ) );

    b = b && (  r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("double")) ) ==
                r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("double" ))) );

    sal_Bool b1 ,b2;
    Any a =r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("bool")) );
    a >>= b1;
    a = r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("bool")) );
    a >>= b2;
    b = b && ( (b1 && b2) || b1 == b2 );

//      b = b &&    r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("bool")) ) ==
//                  r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("bool")) ) );

    b = b && (  r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("byte")) ) ==
                r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("byte")) ) );

    b = b && (  r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("char")) ) ==
                r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("char")) ) );

    b = b && (  r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("string")) ) ==
                r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("string")) ));

    Any o1 = r1->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("object")) );
    Any o2 = r2->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("object")) );

    if( o1.getValueType() == getCppuType( (Reference<XPersistObject>*)0 ) ) {

        if( o2.getValueType() == getCppuType( (Reference<XPersistObject>*)0 ) ) {
            Reference < XPersistObject > rPersist1;
            Reference < XPersistObject > rPersist2;
            o1 >>= rPersist1;
            o2 >>= rPersist2;
            Reference <XPropertySet > rProp1( rPersist1 , UNO_QUERY );
            Reference < XPropertySet > rProp2( rPersist2 , UNO_QUERY );

            if( rProp1.is() && rProp2.is() && ! ( rProp1 == rProp2 )
                &&( rProp1 != r1 )) {
                    b = b && compareMyPropertySet( rProp1 , rProp2 );
            }
        }
        else {
            b = sal_False;
        }
    }
    else {
        if( o2.getValueType()  == getCppuType( (Reference<XPersistObject>*)0 ) ) {
            b = sal_False;
        }
    }

    return b;
}

void OObjectStreamTest::testObject(     const Reference<  XObjectOutputStream >  &rOut,
                                        const Reference < XObjectInputStream > &rIn )
{
    ERROR_ASSERT( rOut.is() , "no objectOutputStream" );
    ERROR_ASSERT( rIn.is() , "no objectInputStream" );



    // tests, if saving an object with an unknown service name allows
    // reading the data behind the object !
    {
        Reference < XInterface > x = * new MyPersistObject(
            OUString( RTL_CONSTASCII_USTRINGPARAM("bla blubs")) );

        Reference< XPersistObject > persistRef( x , UNO_QUERY );
        ERROR_ASSERT( persistRef.is() , "couldn't instantiate PersistTest object" );

        rOut->writeObject( persistRef );
        rOut->writeLong( (sal_Int32) 0xdeadbeef );

        ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );

        try
        {
            Reference <XPersistObject >  xReadPersistRef = rIn->readObject();
            ERROR_ASSERT( 0 , "expected exception not thrown" );
        }
        catch( IOException & )
        {
            // all is ok
        }

        ERROR_ASSERT( (sal_Int32) 0xdeadbeef == rIn->readLong() ,
                      "wrong data after object with unknown service name." );
    }

    {
        Reference < XInterface > x = m_rFactory->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.PersistTest")));
        Reference< XPersistObject > persistRef( x , UNO_QUERY );

        ERROR_ASSERT( persistRef.is() , "couldn't instantiate PersistTest object" );

        Reference < XPropertySet > rProp( persistRef , UNO_QUERY );
        ERROR_ASSERT( rProp.is() , "test object is no property set " );

        Any any;
        sal_Int32 i = 0x83482;
        any <<= i;
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("long")) , any );

        float f = (float)42.23;
        any <<= f;
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("float")) , any );

        double d = 233.321412;
        any <<= d;
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("double")) , any );

        sal_Bool b = sal_True;
        any.setValue( &b , getCppuBooleanType() );
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("bool")) , any );

        sal_Int8 by = 120;
        any <<= by;
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("byte")) , any );

        sal_Unicode c = 'h';
        any.setValue( &c , getCppuCharType() );
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("char")) , any );

        OUString str( RTL_CONSTASCII_USTRINGPARAM( "hi du !" ) );
        any <<= str;
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("string")) , any );

        any <<= persistRef;
        rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("object")) , any );

        // do read and write
        rOut->writeObject( persistRef );
        ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );
        Reference< XPersistObject > xReadPersist    =   rIn->readObject( );

        Reference< XPropertySet >  rPropRead( xReadPersist , UNO_QUERY );
        ERROR_ASSERT( compareMyPropertySet( rProp , rPropRead ) , "objects has not been read properly !" );

        // destroy selfreferences
        rProp->setPropertyValue( OUString::createFromAscii("object"), Any() );
        rPropRead->setPropertyValue( OUString::createFromAscii("object"), Any() );
    }

    {
        Reference< XMarkableStream > markableOut( rOut , UNO_QUERY );
        ERROR_ASSERT( markableOut.is() , "markable stream cannot be queried" );

        // do the same thing multiple times to check if
        // buffering and marks work correctly
        for( int i = 0 ; i < 2000 ; i ++ ) {

            Reference < XInterface > x = m_rFactory->createInstance(OUString::createFromAscii("test.com.sun.star.io.PersistTest"));
            Reference< XPersistObject >  persistRef( x , UNO_QUERY );

            Reference < XPropertySet >  rProp( persistRef , UNO_QUERY );
            ERROR_ASSERT( rProp.is() , "test object is no property set " );

            Any any;
            sal_Int32 i = 0x83482;
            any <<= i;
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("long")) , any );

            float f = 42.23;
            any <<= f;
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("float")) , any );

            double d = 233.321412;
            any <<= d;
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("double")) , any );

            sal_Bool b = sal_True;
            any.setValue( &b , getCppuBooleanType() );
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("bool")) , any );

            sal_Int8 by = 120;
            any <<= by;
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("byte")) , any );

            sal_Unicode c = 'h';
            any.setValue( &c , getCppuCharType() );
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("char")) , any );

            OUString str( RTL_CONSTASCII_USTRINGPARAM( "hi du !" ) );
            any <<= str;
            rProp->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM("string")) , any );

            x = m_rFactory->createInstance(OUString::createFromAscii("test.com.sun.star.io.PersistTest"));
            Reference <XPersistObject > persist2ndRef( x , UNO_QUERY );

            // Note : persist2ndRef contains coincident values, but also coincident values must be
            // saved properly !
            any <<= persist2ndRef;
            rProp->setPropertyValue( OUString::createFromAscii("object") , any );

            // simply test, if markable operations and object operations do not interfere
            sal_Int32 nMark = markableOut->createMark();

            // do read and write
            rOut->writeObject( persistRef );

            // further markable tests !
            sal_Int32 nOffset = markableOut->offsetToMark( nMark );
            markableOut->jumpToMark( nMark );
            markableOut->deleteMark( nMark );
            markableOut->jumpToFurthest();





            ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );
            Reference < XPersistObject > xReadPersistRef    =   rIn->readObject( );

            Reference< XPropertySet > rProp1( persistRef , UNO_QUERY );
            Reference< XPropertySet >  rProp2( xReadPersistRef , UNO_QUERY );
            ERROR_ASSERT( compareMyPropertySet( rProp1, rProp2) ,
                          "objects has not been read properly !" );
        }
    }
}


Reference < XInterface > SAL_CALL OObjectStreamTest_CreateInstance( const Reference < XMultiServiceFactory > & rSMgr ) throw(Exception)
{
    OObjectStreamTest *p = new OObjectStreamTest( rSMgr );
    return Reference < XInterface > ( SAL_STATIC_CAST( OWeakObject * , p ) );
}

Sequence<OUString> OObjectStreamTest_getSupportedServiceNames( int i) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OObjectStreamTest_getImplementationName( i);
    return aRet;
}

OUString     OObjectStreamTest_getServiceName( int i) throw ()
{
    if( 1 == i ) {
        return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.ObjectInputStream" ));
    }
    else {
        return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.io.ObjectOutputStream"));
    }
}

OUString    OObjectStreamTest_getImplementationName( int i) throw ()
{
    if( 1 == i ) {
        return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.comp.extensions.stm.ObjectInputStream" ));
    }
    else {
        return OUString( RTL_CONSTASCII_USTRINGPARAM("test.com.sun.star.comp.extensions.stm.ObjectOutputStream"));
    }
}


