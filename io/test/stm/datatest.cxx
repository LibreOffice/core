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
#include <cppuhelper/implbase.hxx>

#include <osl/conditn.hxx>
#include <osl/mutex.hxx>

#include <string.h>

using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::test;
using namespace ::com::sun::star::beans;
// streams

#include "testfactreg.hxx"

#define DATASTREAM_TEST_MAX_HANDLE 1

/*
 * The following test class tests XDataInputStream and XDataOutputStream at equal terms,
 * so when errors occur, it may be in either one implementation.
 * The class also  uses com.sun.star.io.pipe. If problems occur, make sure to run also the
 * pipe test routines ( test.com.sun.star.io.pipe ).
 */

class ODataStreamTest :
        public WeakImplHelper< XSimpleTest >
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

    virtual sal_Bool SAL_CALL testPassed()                              throw ( RuntimeException);
    virtual Sequence< OUString > SAL_CALL getErrors()               throw (RuntimeException);
    virtual Sequence< Any > SAL_CALL getErrorExceptions()       throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getWarnings()                 throw (RuntimeException);

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
    if( OUString("com.sun.star.io.DataInputStream") == TestName ) {
        Reference < XConnectable > connect( TestObject , UNO_QUERY );
        Reference < XActiveDataSink > active( TestObject , UNO_QUERY );
        Reference < XInputStream >  input( TestObject , UNO_QUERY );
        Reference < XDataInputStream > dataInput( TestObject , UNO_QUERY );

        WARNING_ASSERT( connect.is(), "XConnectable cannot be queried" );
        WARNING_ASSERT( active.is() , "XActiveDataSink cannot be queried" );
        ERROR_ASSERT( input.is() , "XInputStream cannot be queried" );
        ERROR_ASSERT( dataInput.is() , "XDataInputStream cannot be queried" );


    }
    else if( OUString("com.sun.star.io.DataOutputStream") == TestName ) {
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
        ERROR_ASSERT( ! info->supportsService("bla bluzb") , "XServiceInfo test failed" );
    }

}


sal_Int32 ODataStreamTest::test(
    const OUString& TestName,
    const Reference < XInterface >& TestObject,
    sal_Int32 hTestHandle)
    throw ( IllegalArgumentException,
            RuntimeException)
{
    if( OUString("com.sun.star.io.DataInputStream") == TestName ||
        OUString("com.sun.star.io.DataOutputStream") == TestName )  {

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
                    "com.sun.star.io.Pipe" );

                Reference < XInputStream >   rPipeInput( x , UNO_QUERY );
                Reference < XOutputStream >  rPipeOutput( x , UNO_QUERY );

                if( ! rSink.is() ) {
                    x = m_rFactory->createInstance(
                        "com.sun.star.io.DataInputStream" );
                    rInput = Reference < XDataInputStream > ( x , UNO_QUERY);
                    rSink = Reference<  XActiveDataSink > ( x , UNO_QUERY );
                }
                else if ( !rSource.is() )
                {
                    x = m_rFactory->createInstance(
                        "com.sun.star.io.DataOutputStream" );
                    rOutput = Reference< XDataOutputStream > ( x , UNO_QUERY );
                    rSource = Reference< XActiveDataSource > ( x, UNO_QUERY );
                }

                OSL_ASSERT( rPipeInput.is() );
                OSL_ASSERT( rPipeOutput.is() );
                rSink->setInputStream( rPipeInput );
                rSource->setOutputStream( rPipeOutput );

                OSL_ASSERT( rSink->getInputStream().is() );
                OSL_ASSERT( rSource->getOutputStream().is() );

                if( 1 == hTestHandle ) {
                    testSimple( rInput , rOutput );
                }
            }
        }
        catch( const Exception & e )
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



sal_Bool ODataStreamTest::testPassed()                                      throw (RuntimeException)
{
    return m_seqErrors.getLength() == 0;
}


Sequence< OUString > ODataStreamTest::getErrors()                           throw (RuntimeException)
{
    return m_seqErrors;
}


Sequence< Any > ODataStreamTest::getErrorExceptions()                   throw (RuntimeException)
{
    return m_seqExceptions;
}


Sequence< OUString > ODataStreamTest::getWarnings()                         throw (RuntimeException)
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

    rOutput->writeHyper( 0x123456789abcdefLL );
    ERROR_ASSERT( rInput->readHyper() == 0x123456789abcdefLL , "int64 read/write mismatch" );

    rOutput->writeUTF( OUString("Live long and prosper !") );
    ERROR_ASSERT( rInput->readUTF() == "Live long and prosper !",
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

    Sequence<sal_Int8> dummy (1);
    ERROR_ASSERT( ! rInput->readBytes( dummy , 1 ),
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
    return Reference < XInterface > ( (static_cast< OWeakObject *  >(p)) );
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
        return OUString( "test.com.sun.star.io.DataInputStream" );
    }
    else {
        return OUString( "test.com.sun.star.io.DataOutputStream" );
    }
}

OUString    ODataStreamTest_getImplementationName( int i) throw ()
{
    if( 1 == i ) {
        return OUString(
            "test.com.sun.star.comp.extensions.stm.DataInputStream" );
    }
    else {
        return OUString( "test.com.sun.star.comp.extensions.stm.DataOutputStream"  );
    }
}

class MyPersistObject : public WeakImplHelper< XPersistObject , XPropertySet >
{
public:
    MyPersistObject( ) : m_sServiceName( OMyPersistObject_getServiceName() ) ,
        m_l( -392 ),
        m_f( 7883.2 ),
        m_d( -123923.5 ),
        m_b( sal_True ),
        m_byte( 42 ),
        m_c( 429 ),
        m_s( OUString( "foo"  ) )
        {}
    MyPersistObject( const OUString & sServiceName ) : m_sServiceName( sServiceName )
        {}


public:
    virtual OUString SAL_CALL getServiceName()  throw (RuntimeException);
    virtual void SAL_CALL write( const Reference< XObjectOutputStream >& OutStream )
        throw (IOException, RuntimeException);
    virtual void SAL_CALL read(const Reference< XObjectInputStream >& InStream)
        throw (IOException, RuntimeException);

public:

    virtual Reference< XPropertySetInfo > SAL_CALL getPropertySetInfo()
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



Reference <XPropertySetInfo > MyPersistObject::getPropertySetInfo()
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
    if( aPropertyName.equalsAscii("long")  ) {
        aValue >>= m_l;
    }
    else if ( aPropertyName.equalsAscii("float") ) {
        aValue >>= m_f;
    }
    else if( aPropertyName.equalsAscii("double") ) {
        aValue >>= m_d;
    }
    else if( aPropertyName.equalsAscii("bool") ) {
        aValue >>= m_b;
    }
    else if( aPropertyName.equalsAscii("byte" ) ) {
        aValue >>= m_byte;
    }
    else if( aPropertyName.equalsAscii("char") ) {
        aValue >>= m_c;
    }
    else if( aPropertyName.equalsAscii("string") ) {
        aValue >>= m_s;
    }
    else if( aPropertyName.equalsAscii("object") ) {
        if( aValue.getValueType() == cppu::UnoType<XPersistObject>::get())
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
    if( aPropertyName.equalsAscii("long" )  ) {
        aValue <<= m_l;
    }
    else if ( aPropertyName.equalsAscii("float") ) {
        aValue <<= m_f;
    }
    else if( aPropertyName.equalsAscii("double") ) {
        aValue <<= m_d;
    }
    else if( aPropertyName.equalsAscii("bool") ) {
        aValue <<= m_b;
    }
    else if( aPropertyName.equalsAscii("byte") ) {
        aValue <<= m_byte;
    }
    else if( aPropertyName.equalsAscii("char" ) ) {
        aValue <<= m_c;
    }
    else if( aPropertyName.equalsAscii("string") ) {
        aValue <<= m_s;
    }
    else if( aPropertyName.equalsAscii("object" ) )
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
    return Reference < XInterface > ( (static_cast< OWeakObject *  >(p)) );
}

Sequence<OUString> OMyPersistObject_getSupportedServiceNames( ) throw ()
{
    Sequence<OUString> aRet(1);
    aRet.getArray()[0] = OMyPersistObject_getImplementationName();
    return aRet;
}

OUString     OMyPersistObject_getServiceName( ) throw ()
{
    return OUString( "test.com.sun.star.io.PersistTest" );
}

OUString    OMyPersistObject_getImplementationName( ) throw ()
{
    return OUString( "test.com.sun.star.io.PersistTest"  );
}

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
    void testObject(     const Reference <XObjectOutputStream > &rOut,
                         const Reference <XObjectInputStream> &rIn );

private:
};


void OObjectStreamTest::testInvariant( const OUString& TestName,
                                       const Reference < XInterface >& TestObject )
    throw ( IllegalArgumentException, RuntimeException)
{

    if( OUString( "com.sun.star.io.ObjectInputStream"  )
                  == TestName )
    {
        ODataStreamTest::testInvariant( TestName , TestObject );
        Reference< XObjectInputStream > dataInput( TestObject , UNO_QUERY );
        Reference< XMarkableStream >  markable( TestObject , UNO_QUERY );
        ERROR_ASSERT( dataInput.is() , "XObjectInputStream cannot be queried" );
        ERROR_ASSERT(   markable.is() , "XMarkableStream cannot be queried" );
    }
    else if( OUString( "com.sun.star.io.ObjectOutputStream" )
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
        ERROR_ASSERT( ! info->supportsService("bla bluzb") , "XServiceInfo test failed" );
    }

}

sal_Int32 OObjectStreamTest::test(  const OUString& TestName,
                        const Reference < XInterface >& TestObject,
                        sal_Int32 hTestHandle)
    throw ( IllegalArgumentException,
            RuntimeException)
{
    if( TestName.equalsAscii("com.sun.star.io.ObjectInputStream") ||
        TestName.equalsAscii("com.sun.star.io.ObjectOutputStream") )  {

        try
        {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else if( DATASTREAM_TEST_MAX_HANDLE >= hTestHandle ) {
                sal_Int32 hOldHandle = hTestHandle;
                hTestHandle = ODataStreamTest::test(
                    OUString( "com.sun.star.io.DataInputStream" ),
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
                    "com.sun.star.io.Pipe" );

                Reference <XInputStream > rPipeInput( x , UNO_QUERY );
                Reference <XOutputStream >  rPipeOutput( x , UNO_QUERY );

                x = m_rFactory->createInstance(
                    "com.sun.star.io.MarkableInputStream" );

                Reference <XInputStream > markableInput( x , UNO_QUERY );
                Reference <XActiveDataSink> markableSink( x , UNO_QUERY );

                x = m_rFactory->createInstance( OUString(
                    "com.sun.star.io.MarkableOutputStream"  ) );
                Reference <XOutputStream >  markableOutput( x , UNO_QUERY );
                Reference <XActiveDataSource >  markableSource( x , UNO_QUERY );

                OSL_ASSERT( markableInput.is()  );
                OSL_ASSERT( markableOutput.is() );
                OSL_ASSERT( markableSink.is()   );
                OSL_ASSERT( markableSource.is() );

                markableSink->setInputStream( rPipeInput );
                markableSource->setOutputStream( rPipeOutput );

                if( ! rSink.is() ) {
                    x = m_rFactory->createInstance(
                        "com.sun.star.io.ObjectInputStream" );
                    rInput = Reference < XObjectInputStream > ( x , UNO_QUERY );
                    rSink = Reference < XActiveDataSink > ( x , UNO_QUERY );
                }
                else if ( !rSource.is() ) {
                    x = m_rFactory->createInstance(
                        "com.sun.star.io.ObjectOutputStream" );
                    rOutput = Reference <XObjectOutputStream > ( x , UNO_QUERY );
                    rSource = Reference <XActiveDataSource>( x, UNO_QUERY );
                }

                OSL_ASSERT( rPipeInput.is() );
                OSL_ASSERT( rPipeOutput.is() );

                rSink->setInputStream( markableInput );
                rSource->setOutputStream( markableOutput );

                OSL_ASSERT( rSink->getInputStream().is() );
                OSL_ASSERT( rSource->getOutputStream().is() );

                if( 1 + DATASTREAM_TEST_MAX_HANDLE == hTestHandle ) {
                    testObject( rOutput , rInput);
                }
                rInput->closeInput();
                rOutput->closeOutput();

            }
        }
        catch( const Exception &e )  {
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

    if( r1->getPropertyValue("long").getValueType() == cppu::UnoType<void>::get() ||
        r2->getPropertyValue("long").getValueType() == cppu::UnoType<void>::get() ) {

        // one of the objects is not the correct propertyset !
        fprintf( stderr, "compareMyPropertySet: 1\n" );
        return sal_False;
    }

    b = b && (  r1->getPropertyValue("long") ==
                r2->getPropertyValue("long") );
    if( ! b ) fprintf( stderr, "compareMyPropertySet: 2\n" );

    b = b && (  r1->getPropertyValue("float") ==
                r2->getPropertyValue("float") );
    if( ! b ){
        float f1(0.0);
        float f2(0.0);
        r1->getPropertyValue("float") >>= f1;
        r2->getPropertyValue("float") >>= f2;
        fprintf( stderr, "compareMyPropertySet: %f %f 3\n",f1,f2 );
    }

    b = b && (  r1->getPropertyValue("double") ==
                r2->getPropertyValue("double") );
    if( ! b ) fprintf( stderr, "compareMyPropertySet: 4\n" );

    sal_Bool b1(sal_False), b2(sal_False);
    Any a =r1->getPropertyValue("bool");
    a >>= b1;
    a = r2->getPropertyValue("bool");
    a >>= b2;
    b = b && ( (b1 && b2) || b1 == b2 );
    if( ! b ) fprintf( stderr, "compareMyPropertySet: 5\n" );

//      b = b &&    r1->getPropertyValue("bool") ==
//                  r2->getPropertyValue("bool") );

    b = b && (  r1->getPropertyValue("byte") ==
                r2->getPropertyValue("byte") );
    if( ! b ) fprintf( stderr, "compareMyPropertySet: 6\n" );

    b = b && (  r1->getPropertyValue("char") ==
                r2->getPropertyValue("char") );
    if( ! b ) fprintf( stderr, "compareMyPropertySet: 7\n" );

    b = b && (  r1->getPropertyValue("string") ==
                r2->getPropertyValue("string"));
    if( ! b ) fprintf( stderr, "compareMyPropertySet: 8\n" );

    Any o1 = r1->getPropertyValue("object");
    Any o2 = r2->getPropertyValue("object");

    if( o1.getValueType() == cppu::UnoType<XPersistObject>::get()) {

        if( o2.getValueType() == cppu::UnoType<XPersistObject>::get()) {
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
        if( ! b ) fprintf( stderr, "compareMyPropertySet: 9\n" );
    }
    else {
        if( o2.getValueType()  == cppu::UnoType<XPersistObject>::get()) {
            b = sal_False;
        }
        if( ! b ) fprintf( stderr, "compareMyPropertySet: 10\n" );
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
            OUString( "bla blubs") );

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
            "test.com.sun.star.io.PersistTest");
        Reference< XPersistObject > persistRef( x , UNO_QUERY );

        ERROR_ASSERT( persistRef.is() , "couldn't instantiate PersistTest object" );

        Reference < XPropertySet > rProp( persistRef , UNO_QUERY );
        ERROR_ASSERT( rProp.is() , "test object is no property set " );

        Any any;
        sal_Int32 i = 0x83482;
        any <<= i;
        rProp->setPropertyValue("long", any );

        float f = (float)42.23;
        any <<= f;
        rProp->setPropertyValue("float", any );

        double d = 233.321412;
        any <<= d;
        rProp->setPropertyValue("double", any );

        sal_Bool b = sal_True;
        any.setValue( &b , cppu::UnoType<bool>::get() );
        rProp->setPropertyValue("bool", any );

        sal_Int8 by = 120;
        any <<= by;
        rProp->setPropertyValue("byte", any );

        sal_Unicode c = 'h';
        any.setValue( &c , cppu::UnoType<cppu::UnoCharType>::get() );
        rProp->setPropertyValue("char", any );

        OUString str( "hi du !"  );
        any <<= str;
        rProp->setPropertyValue("string", any );

        any <<= persistRef;
        rProp->setPropertyValue("object", any );

        // do read and write
        rOut->writeObject( persistRef );
        ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );
        Reference< XPersistObject > xReadPersist    =   rIn->readObject( );

        Reference< XPropertySet >  rPropRead( xReadPersist , UNO_QUERY );
        ERROR_ASSERT( compareMyPropertySet( rProp , rPropRead ) , "objects has not been read properly !" );

        // destroy selfreferences
        rProp->setPropertyValue("object", Any() );
        rPropRead->setPropertyValue("object", Any() );
    }

    {
        Reference< XMarkableStream > markableOut( rOut , UNO_QUERY );
        ERROR_ASSERT( markableOut.is() , "markable stream cannot be queried" );

        // do the same thing multiple times to check if
        // buffering and marks work correctly
        for( int i = 0 ; i < 2000 ; i ++ ) {

            Reference < XInterface > x = m_rFactory->createInstance("test.com.sun.star.io.PersistTest");
            Reference< XPersistObject >  persistRef( x , UNO_QUERY );

            Reference < XPropertySet >  rProp( persistRef , UNO_QUERY );
            ERROR_ASSERT( rProp.is() , "test object is no property set " );

            Any any;
            sal_Int32 i = 0x83482;
            any <<= i;
            rProp->setPropertyValue("long", any );

            float f = 42.23;
            any <<= f;
            rProp->setPropertyValue("float", any );

            double d = 233.321412;
            any <<= d;
            rProp->setPropertyValue("double", any );

            sal_Bool b = sal_True;
            any.setValue( &b , cppu::UnoType<bool>::get() );
            rProp->setPropertyValue("bool", any );

            sal_Int8 by = 120;
            any <<= by;
            rProp->setPropertyValue("byte", any );

            sal_Unicode c = 'h';
            any.setValue( &c , cppu::UnoType<cppu::UnoCharType>::get() );
            rProp->setPropertyValue("char", any );

            OUString str( "hi du !"  );
            any <<= str;
            rProp->setPropertyValue("string", any );

            x = m_rFactory->createInstance("test.com.sun.star.io.PersistTest");
            Reference <XPersistObject > persist2ndRef( x , UNO_QUERY );

            // Note : persist2ndRef contains coincident values, but also coincident values must be
            // saved properly !
            any <<= persist2ndRef;
            rProp->setPropertyValue("object", any );

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
    return Reference < XInterface > ( (static_cast< OWeakObject *  >(p)) );
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
        return OUString( "test.com.sun.star.io.ObjectInputStream" );
    }
    else {
        return OUString( "test.com.sun.star.io.ObjectOutputStream");
    }
}

OUString    OObjectStreamTest_getImplementationName( int i) throw ()
{
    if( 1 == i ) {
        return OUString( "test.com.sun.star.comp.extensions.stm.ObjectInputStream" );
    }
    else {
        return OUString( "test.com.sun.star.comp.extensions.stm.ObjectOutputStream");
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
