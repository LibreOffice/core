/*************************************************************************
 *
 *  $RCSfile: datatest.cxx,v $
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
#include <smart/com/sun/star/io/XObjectOutputStream.hxx>
#include <smart/com/sun/star/io/XObjectInputStream.hxx>
#include <smart/com/sun/star/io/XActiveDataSink.hxx>
#include <smart/com/sun/star/io/XActiveDataSource.hxx>
#include <smart/com/sun/star/io/XConnectable.hxx>
#include <smart/com/sun/star/io/XMarkableStream.hxx>

#include <smart/com/sun/star/lang/XServiceInfo.hxx>

#include <smart/com/sun/star/beans/XPropertySet.hxx>

#include <usr/services.hxx>
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
        public XSimpleTest,
        public OWeakObject
{
public:
    ODataStreamTest( const XMultiServiceFactoryRef & rFactory ) : m_rFactory( rFactory ){}

public: // refcounting
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

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
    void testSimple( const XDataInputStreamRef & , const XDataOutputStreamRef &);

protected:
    Sequence<UsrAny>  m_seqExceptions;
    Sequence<UString> m_seqErrors;
    Sequence<UString> m_seqWarnings;

    XMultiServiceFactoryRef m_rFactory;
};


BOOL ODataStreamTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return OWeakObject::queryInterface( uik , rOut );
    }
    return TRUE;
}


void ODataStreamTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.DataInputStream" == TestName ) {
        XConnectableRef connect( TestObject , USR_QUERY );
        XActiveDataSinkRef active( TestObject , USR_QUERY );
        XInputStreamRef input( TestObject , USR_QUERY );
        XDataInputStreamRef dataInput( TestObject , USR_QUERY );

        WARNING_ASSERT( connect.is(), "XConnectable cannot be queried" );
        WARNING_ASSERT( active.is() , "XActiveDataSink cannot be queried" );
        ERROR_ASSERT( input.is() , "XInputStream cannot be queried" );
        ERROR_ASSERT( dataInput.is() , "XDataInputStream cannot be queried" );


    }
    else if( L"com.sun.star.io.DataInputStream" == TestName ) {
        XConnectableRef connect( TestObject , USR_QUERY );
        XActiveDataSourceRef active( TestObject , USR_QUERY );
        XOutputStreamRef output( TestObject , USR_QUERY );
        XDataOutputStreamRef dataOutput( TestObject , USR_QUERY );

        WARNING_ASSERT( connect.is(),       "XConnectable cannot be queried" );
        WARNING_ASSERT( active.is() ,   "XActiveDataSink cannot be queried" );
        ERROR_ASSERT(   output.is() ,   "XInputStream cannot be queried" );
        ERROR_ASSERT(   dataOutput.is(),    "XDataInputStream cannot be queried" );

    }

    XServiceInfoRef info( TestObject, USR_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService( L"bla bluzb" ) , "XServiceInfo test failed" );
    }

}


INT32 ODataStreamTest::test(    const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.DataInputStream" == TestName ||
        L"com.sun.star.io.DataOutputStream" == TestName )  {

        TRY {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else {
                XActiveDataSinkRef rSink( TestObject, USR_QUERY );
                XActiveDataSourceRef rSource( TestObject , USR_QUERY );

                XDataInputStreamRef rInput( TestObject , USR_QUERY );
                XDataOutputStreamRef rOutput( TestObject , USR_QUERY );


                XInterfaceRef x = m_rFactory->createInstance( L"com.sun.star.io.Pipe");

                XInputStreamRef  rPipeInput( x , USR_QUERY );
                XOutputStreamRef rPipeOutput( x , USR_QUERY );

                if( ! rSink.is() ) {
                    x = m_rFactory->createInstance( L"com.sun.star.io.DataInputStream" );
                    x->queryInterface( XDataInputStream::getSmartUik() , rInput );
                    x->queryInterface( XActiveDataSink::getSmartUik() , rSink );
                }
                else if ( !rSource.is() ) {

                    x = m_rFactory->createInstance( L"com.sun.star.io.DataOutputStream" );
                    x->queryInterface( XDataOutputStream::getSmartUik() , rOutput );
                    x->queryInterface( XActiveDataSource::getSmartUik() , rSource );
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
        CATCH( Exception , e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }
        END_CATCH;

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



BOOL ODataStreamTest::testPassed(void)                                      THROWS( (UsrSystemException) )
{
    return m_seqErrors.getLen() == 0;
}


Sequence< UString > ODataStreamTest::getErrors(void)                            THROWS( (UsrSystemException) )
{
    return m_seqErrors;
}


Sequence< UsrAny > ODataStreamTest::getErrorExceptions(void)                    THROWS( (UsrSystemException) )
{
    return m_seqExceptions;
}


Sequence< UString > ODataStreamTest::getWarnings(void)                      THROWS( (UsrSystemException) )
{
    return m_seqWarnings;
}

void ODataStreamTest::testSimple(   const XDataInputStreamRef &rInput,
                                    const XDataOutputStreamRef &rOutput )
{
    rOutput->writeLong( 0x34ff3c );
    rOutput->writeLong( 0x34ff3d );

    ERROR_ASSERT( 0x34ff3c ==  rInput->readLong() , "long read/write mismatch" );
    ERROR_ASSERT( 0x34ff3d ==  rInput->readLong() , "long read/write mismatch" );

    rOutput->writeByte( 0x87 );
    ERROR_ASSERT( 0x87 == rInput->readByte() , "byte read/write mismatch" );

    rOutput->writeBoolean( 25 );
    ERROR_ASSERT( rInput->readBoolean() , "boolean read/write mismatch" );

    rOutput->writeBoolean( FALSE );
    ERROR_ASSERT( ! rInput->readBoolean() , "boolean read/write mismatch" );

    rOutput->writeFloat( (float) 42.42 );
    ERROR_ASSERT( rInput->readFloat() == ((float)42.42) , "float read/write mismatch" );

    rOutput->writeDouble( (double) 42.42 );
    ERROR_ASSERT( rInput->readDouble() == 42.42 , "double read/write mismatch" );

    rOutput->writeUTF( L"Live long and prosper !" );
    ERROR_ASSERT( rInput->readUTF() == L"Live long and prosper !" ,
                    "UTF read/write mismatch" );

    Sequence<wchar_t> wc(0x10001);
    for( int i = 0 ; i < 0x10000  ; i ++ ) {
        wc.getArray()[i] = L'c';
    }
    wc.getArray()[0x10000] = 0;
    UString str( wc.getArray() , 0x10000 );
    rOutput->writeUTF( str );
    ERROR_ASSERT( rInput->readUTF() == str , "error reading 64k block" );

    rOutput->closeOutput();
    TRY {
        rInput->readLong();
        ERROR_ASSERT( 0 , "eof-exception does not occur !" );
    }
    CATCH ( IOException ,e ){
        //ok
        e.getName();
    }
    AND_CATCH_ALL() {
        ERROR_ASSERT( 0 , "wrong exception after reading beyond eof" );
    }
    END_CATCH;

    ERROR_ASSERT( ! rInput->readBytes( Sequence<BYTE> (1) , 1 ),
                    "stream must be on eof !" );

    rInput->closeInput();

    TRY {
        rOutput->writeByte( 1 );
        ERROR_ASSERT( 0 , "writing still possible though chain must be interrupted" );
    }
    CATCH( IOException , e )
    {
        e.getName();
        // ok
    }
    AND_CATCH_ALL() {
        ERROR_ASSERT( 0 , "IOException  expected, but another exception was thrown" );
    }
    END_CATCH;

}



/**
* for external binding
*
*
**/
XInterfaceRef ODataStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    ODataStreamTest *p = new ODataStreamTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}

Sequence<UString> ODataStreamTest_getSupportedServiceNames( int i) THROWS( () )
{
    Sequence<UString> aRet(1);

    aRet.getArray()[0] = ODataStreamTest_getImplementationName( i);


    return aRet;
}

UString     ODataStreamTest_getServiceName( int i) THROWS( () )
{
    if( 1 == i ) {
        return L"test.com.sun.star.io.DataInputStream";
    }
    else {
        return L"test.com.sun.star.io.DataOutputStream";
    }
}

UString     ODataStreamTest_getImplementationName( int i) THROWS( () )
{
    if( 1 == i ) {
        return L"test.com.sun.star.comp.extensions.stm.DataInputStream";
    }
    else {
        return L"test.com.sun.star.comp.extensions.stm.DataOutputStream";
    }
}


/**------------------------------------------------------
*
*
*
*
*
*------------------------------------------------------*/
class MyPersistObject :
            public XPersistObject,
            public XPropertySet,
            public OWeakObject
{
public:
    MyPersistObject( ) : m_sServiceName( OMyPersistObject_getServiceName() )
        {}
    MyPersistObject( const UString & sServiceName ) : m_sServiceName( sServiceName )
        {}
    ~MyPersistObject()
        {}

public:
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );
    void                        acquire()                        { OWeakObject::acquire(); }
    void                        release()                        { OWeakObject::release(); }
    void*                       getImplementation(Reflection *p) { return OWeakObject::getImplementation(p); }

public:
    virtual UString getServiceName(void) const THROWS( (UsrSystemException) );
    virtual void write(const XObjectOutputStreamRef& OutStream) THROWS( (IOException, UsrSystemException) );
    virtual void read(const XObjectInputStreamRef& InStream) THROWS( (IOException, UsrSystemException) );

public:

    virtual XPropertySetInfoRef getPropertySetInfo(void) const  THROWS( (UsrSystemException) );
    virtual void setPropertyValue(const UString& aPropertyName, const UsrAny& aValue)
                                                                THROWS( (   UnknownPropertyException,
                                                                            PropertyVetoException,
                                                                            IllegalArgumentException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) );
    virtual UsrAny getPropertyValue(const UString& PropertyName) const
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) );
    virtual void addPropertyChangeListener( const UString& aPropertyName,
                                            const XPropertyChangeListenerRef& xListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) );
    virtual void removePropertyChangeListener(  const UString& aPropertyName,
                                                const XPropertyChangeListenerRef& aListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) );
    virtual void addVetoableChangeListener(     const UString& PropertyName,
                                                const XVetoableChangeListenerRef& aListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) );
    virtual void removeVetoableChangeListener(  const UString& PropertyName,
                                                const XVetoableChangeListenerRef& aListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) );

public:
    INT32   m_l;
    float   m_f;
    double  m_d;
    BOOL    m_b;
    BYTE    m_byte;
    wchar_t m_c;
    UString m_s;
    XPersistObjectRef   m_ref;
    UString m_sServiceName;
};



XPropertySetInfoRef MyPersistObject::getPropertySetInfo(void) const     THROWS( (UsrSystemException) )
{
    return XPropertySetInfoRef();
}

void MyPersistObject::setPropertyValue(const UString& aPropertyName, const UsrAny& aValue)
                                                                THROWS( (   UnknownPropertyException,
                                                                            PropertyVetoException,
                                                                            IllegalArgumentException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) )
{
    if( L"long" == aPropertyName  ) {
        m_l = aValue.getINT32();
    }
    else if ( L"float" == aPropertyName ) {
        m_f = aValue.getFloat();
    }
    else if( L"double" == aPropertyName ) {
        m_d = aValue.getDouble();
    }
    else if( L"bool" == aPropertyName ) {
        m_b = aValue.getBOOL();
    }
    else if( L"byte" == aPropertyName ) {
        m_byte = aValue.getBYTE();
    }
    else if( L"char" == aPropertyName ) {
        m_c = aValue.getChar();
    }
    else if( L"string" == aPropertyName ) {
        m_s = aValue.getString();
    }
    else if( L"object" == aPropertyName ) {
        if( aValue.getReflection() == XPersistObject_getReflection() ) {
            XPersistObjectRef *pRef = (XPersistObjectRef*) aValue.get();
            if( pRef ) {
                m_ref = *pRef;
            }
            else {
                m_ref = 0;
            }
        }
        else
        {
            m_ref = 0;
        }
    }
}


UsrAny MyPersistObject::getPropertyValue(const UString& aPropertyName) const
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) )
{
    UsrAny aValue;
    if( L"long" == aPropertyName  ) {
        aValue.setINT32( m_l );
    }
    else if ( L"float" == aPropertyName ) {
        aValue.setFloat( m_f );
    }
    else if( L"double" == aPropertyName ) {
        aValue.setDouble( m_d );
    }
    else if( L"bool" == aPropertyName ) {
        aValue.setBOOL( m_b );
    }
    else if( L"byte" == aPropertyName ) {
        aValue.setBYTE( m_byte );
    }
    else if( L"char" == aPropertyName ) {
        aValue.setChar( m_c );
    }
    else if( L"string" == aPropertyName ) {
        aValue.setString( m_s );
    }
    else if( L"object" == aPropertyName ) {
        aValue.set( &m_ref , XPersistObject_getReflection() );
    }
    return aValue;
}


void MyPersistObject::addPropertyChangeListener(    const UString& aPropertyName,
                                                    const XPropertyChangeListenerRef& xListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) )
{

}

void MyPersistObject::removePropertyChangeListener( const UString& aPropertyName,
                                                    const XPropertyChangeListenerRef& aListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) )
{
}
void MyPersistObject::addVetoableChangeListener(const UString& PropertyName,
                                                const XVetoableChangeListenerRef& aListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) )
{

}

void MyPersistObject::removeVetoableChangeListener( const UString& PropertyName,
                                    const XVetoableChangeListenerRef& aListener)
                                                                THROWS( (   UnknownPropertyException,
                                                                            WrappedTargetException,
                                                                            UsrSystemException) )
{

}



BOOL MyPersistObject::queryInterface( Uik aUik , XInterfaceRef &rOut )
{
    if( XPersistObject::getSmartUik() == aUik ) {
        rOut = ( XPersistObject * ) this;
    }
    else if( XPropertySet::getSmartUik() == aUik ) {
        rOut = ( XPropertySet * ) this;
    }
    else
        return OWeakObject::queryInterface( aUik , rOut );

    return TRUE;
}

UString MyPersistObject::getServiceName() const THROWS( (UsrSystemException) )
{
    return m_sServiceName;
}

void MyPersistObject::write( const XObjectOutputStreamRef & rOut )
                                                    THROWS( (IOException,UsrSystemException))
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


void MyPersistObject::read( const XObjectInputStreamRef & rIn )
                                                    THROWS( (IOException, UsrSystemException) )
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

XInterfaceRef OMyPersistObject_CreateInstance( const XMultiServiceFactoryRef & rSMgr )
                                                    THROWS((Exception))
{
    MyPersistObject *p = new MyPersistObject( );
    XInterfaceRef xService = *p;
    return xService;
}

Sequence<UString> OMyPersistObject_getSupportedServiceNames( ) THROWS( () )
{
    Sequence<UString> aRet(1);
    aRet.getArray()[0] = OMyPersistObject_getImplementationName();
    return aRet;
}

UString     OMyPersistObject_getServiceName( ) THROWS( () )
{
    return L"test.com.sun.star.io.PersistTest";
}

UString     OMyPersistObject_getImplementationName( ) THROWS( () )
{
    return L"test.com.sun.star.io.PersistTest";
}


// ---------------------------------------------
// -----------------------------------------------
class OObjectStreamTest :
        public ODataStreamTest
{
public:
    OObjectStreamTest( const XMultiServiceFactoryRef &r) : ODataStreamTest(r) {}
public: // refcounting
    BOOL                        queryInterface( Uik aUik, XInterfaceRef & rOut );

public:
    virtual void testInvariant(const UString& TestName, const XInterfaceRef& TestObject)
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );

    virtual INT32 test( const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) );


private:
    void OObjectStreamTest::testObject(     const XObjectOutputStreamRef &rOut,
                                            const XObjectInputStreamRef &rIn );

private:
};




BOOL OObjectStreamTest::queryInterface( Uik uik , XInterfaceRef &rOut )
{
    if( XSimpleTest::getSmartUik() == uik ) {
        rOut = (XSimpleTest *) this;
    }
    else {
        return ODataStreamTest::queryInterface( uik , rOut );
    }
    return TRUE;
}


void OObjectStreamTest::testInvariant( const UString& TestName, const XInterfaceRef& TestObject )
                                                                THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{


    if( L"com.sun.star.io.ObjectInputStream" == TestName ) {
        ODataStreamTest::testInvariant( TestName , TestObject );
        XObjectInputStreamRef dataInput( TestObject , USR_QUERY );
        XMarkableStreamRef markable( TestObject , USR_QUERY );
        ERROR_ASSERT( dataInput.is() , "XObjectInputStream cannot be queried" );
        ERROR_ASSERT(   markable.is() , "XMarkableStream cannot be queried" );
    }
    else if( L"com.sun.star.io.ObjectOutputStream" == TestName ) {
        ODataStreamTest::testInvariant( TestName , TestObject );
        XMarkableStreamRef markable( TestObject , USR_QUERY );
        XObjectOutputStreamRef dataOutput( TestObject , USR_QUERY );
        ERROR_ASSERT(   dataOutput.is(),    "XObjectOutputStream cannot be queried" );
        ERROR_ASSERT(   markable.is() , "XMarkableStream cannot be queried" );
    }

    XServiceInfoRef info( TestObject, USR_QUERY );
    ERROR_ASSERT( info.is() , "XServiceInfo not supported !" );
    if( info.is() )
    {
        ERROR_ASSERT( info->supportsService( TestName ), "XServiceInfo test failed" );
        ERROR_ASSERT( ! info->supportsService( L"bla bluzb" ) , "XServiceInfo test failed" );
    }

}

INT32 OObjectStreamTest::test(  const UString& TestName,
                        const XInterfaceRef& TestObject,
                        INT32 hTestHandle)                      THROWS( (   IllegalArgumentException,
                                                                            UsrSystemException) )
{
    if( L"com.sun.star.io.ObjectInputStream" == TestName ||
        L"com.sun.star.io.ObjectOutputStream" == TestName )  {

        TRY {
            if( 0 == hTestHandle ) {
                testInvariant( TestName , TestObject );
            }
            else if( DATASTREAM_TEST_MAX_HANDLE >= hTestHandle ) {
                INT32 hOldHandle = hTestHandle;
                hTestHandle = ODataStreamTest::test( L"com.sun.star.io.DataInputStream" ,
                                                        TestObject , hTestHandle );
                if( hTestHandle == -1 ){
                    hTestHandle = hOldHandle;
                }
            }
            else {

                XActiveDataSinkRef rSink( TestObject, USR_QUERY );
                XActiveDataSourceRef rSource( TestObject , USR_QUERY );

                XObjectInputStreamRef rInput( TestObject , USR_QUERY );
                XObjectOutputStreamRef rOutput( TestObject , USR_QUERY );


                XInterfaceRef x = m_rFactory->createInstance( L"com.sun.star.io.Pipe");

                XInputStreamRef  rPipeInput( x , USR_QUERY );
                XOutputStreamRef rPipeOutput( x , USR_QUERY );

                x = m_rFactory->createInstance( L"com.sun.star.io.MarkableInputStream" );
                XInputStreamRef     markableInput( x , USR_QUERY );
                XActiveDataSinkRef markableSink( x , USR_QUERY );

                x = m_rFactory->createInstance( L"com.sun.star.io.MarkableOutputStream" );
                XOutputStreamRef    markableOutput( x , USR_QUERY );
                XActiveDataSourceRef    markableSource( x , USR_QUERY );

                assert( markableInput.is()  );
                assert( markableOutput.is() );
                assert( markableSink.is()   );
                assert( markableSource.is() );

                markableSink->setInputStream( rPipeInput );
                markableSource->setOutputStream( rPipeOutput );

                if( ! rSink.is() ) {
                    x = m_rFactory->createInstance( L"com.sun.star.io.ObjectInputStream" );
                    x->queryInterface( XObjectInputStream::getSmartUik() , rInput );
                    x->queryInterface( XActiveDataSink::getSmartUik() , rSink );
                }
                else if ( !rSource.is() ) {
                    x = m_rFactory->createInstance( L"com.sun.star.io.ObjectOutputStream" );
                    x->queryInterface( XObjectOutputStream::getSmartUik() , rOutput );
                    x->queryInterface( XActiveDataSource::getSmartUik() , rSource );
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
        CATCH( Exception , e )  {
            BUILD_ERROR( 0 , UStringToString( e.getName() , CHARSET_SYSTEM ).GetCharStr() );
        }
        AND_CATCH_ALL() {
            BUILD_ERROR( 0 , "unknown exception (Exception is  not base class)" );
        }
        END_CATCH;

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


BOOL compareMyPropertySet( XPropertySetRef &r1 , XPropertySetRef &r2 )
{
    BOOL b = TRUE;

    if( r1->getPropertyValue( L"long" ).getReflection() == Void_getReflection() ||
        r2->getPropertyValue( L"long" ).getReflection() == Void_getReflection() ) {

        // one of the objects is not the correct propertyset !
        return FALSE;
    }

    b = b && (  r1->getPropertyValue( L"long" ).getINT32() ==
                r2->getPropertyValue( L"long" ).getINT32() );

    b = b && (  r1->getPropertyValue( L"float" ).getFloat() ==
                r2->getPropertyValue( L"float" ).getFloat() );

    b = b && (  r1->getPropertyValue( L"double" ).getDouble() ==
                r2->getPropertyValue( L"double" ).getDouble() );

    b = b && (  r1->getPropertyValue( L"bool" ).getBOOL() ==
                r2->getPropertyValue( L"bool" ).getBOOL() );

    b = b && (  r1->getPropertyValue( L"byte" ).getBYTE() ==
                r2->getPropertyValue( L"byte" ).getBYTE() );

    b = b && (  r1->getPropertyValue( L"char" ).getChar() ==
                r2->getPropertyValue( L"char" ).getChar() );

    b = b && (  r1->getPropertyValue( L"string" ).getString() ==
                r2->getPropertyValue( L"string" ).getString() );

    UsrAny o1 = r1->getPropertyValue( L"object" );
    UsrAny o2 = r2->getPropertyValue( L"object" );

    if( o1.getReflection() == XPersistObject_getReflection() ) {

        if( o2.getReflection() == XPersistObject_getReflection() ) {
            XPersistObjectRef rPersist1 = *((XPersistObjectRef*) o1.get());
            XPersistObjectRef rPersist2 = *((XPersistObjectRef*) o2.get());
            XPropertySetRef rProp1( rPersist1 , USR_QUERY );
            XPropertySetRef rProp2( rPersist2 , USR_QUERY );

            if( rProp1.is() && rProp2.is() && ! ( rProp1 == rProp2 )
                &&( rProp1 != r1 )) {
                    compareMyPropertySet( rProp1 , rProp2 );
            }
        }
        else {
            b = FALSE;
        }
    }
    else {
        if( o2.getReflection() == XPersistObject_getReflection() ) {
            b = FALSE;
        }
    }

    return b;
}

void OObjectStreamTest::testObject(     const XObjectOutputStreamRef &rOut,
                                        const XObjectInputStreamRef &rIn )
{
    ERROR_ASSERT( rOut.is() , "no objectOutputStream" );
    ERROR_ASSERT( rIn.is() , "no objectInputStream" );

    XMultiServiceFactoryRef xProv = getProcessServiceManager();
    ERROR_ASSERT( xProv.is() , "no global service manager" );


    // tests, if saving an object with an unknown service name allows
    // reading the data behind the object !
    {
        XInterfaceRef x = * new MyPersistObject( L"bla blubs" );

        XPersistObjectRef persistRef( x , USR_QUERY );
        ERROR_ASSERT( persistRef.is() , "couldn't instantiate PersistTest object" );

        rOut->writeObject( persistRef );
        rOut->writeLong( (INT32) 0xdeadbeef );

        ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );

        TRY
        {
            XPersistObjectRef xReadPersistRef = rIn->readObject();
            ERROR_ASSERT( 0 , "expected exception not thrown" );
        }
        CATCH( IOException , e )
        {
            // all is ok
        }

        ERROR_ASSERT( (INT32) 0xdeadbeef == rIn->readLong() ,
                      "wrong data after object with unknown service name." );
    }

    {
        XInterfaceRef x = xProv->createInstance(L"test.com.sun.star.io.PersistTest");
        XPersistObjectRef persistRef( x , USR_QUERY );

        ERROR_ASSERT( persistRef.is() , "couldn't instantiate PersistTest object" );

        XPropertySetRef rProp( persistRef , USR_QUERY );
        ERROR_ASSERT( rProp.is() , "test object is no property set " );

        UsrAny any;
        any.setINT32( 0x83482 );
        rProp->setPropertyValue( L"long" , any );

        any.setFloat( 42.23 );
        rProp->setPropertyValue( L"float" , any );

        any.setDouble(  233.321412 );
        rProp->setPropertyValue( L"double" , any );

        any.setBOOL( TRUE );
        rProp->setPropertyValue( L"bool" , any );

        any.setBYTE( 130 );
        rProp->setPropertyValue( L"byte" , any );

        any.setChar( 'h' );
        rProp->setPropertyValue( L"char" , any );

        any.setString( L"hi du !" );
        rProp->setPropertyValue( L"string" , any );

        any.set( &persistRef, XPersistObject_getReflection() ); // try a self reference
        rProp->setPropertyValue( L"object" , any );

        // do read and write
        rOut->writeObject( persistRef );
        ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );
        XPersistObjectRef xReadPersistRef   =   rIn->readObject( );

        XPropertySetRef rPropRead( xReadPersistRef , USR_QUERY );
        ERROR_ASSERT( compareMyPropertySet( rProp , rPropRead ) , "objects has not been read properly !" );

        // destroy selfreferences
        rProp->setPropertyValue( L"object", UsrAny() );
        rPropRead->setPropertyValue( L"object", UsrAny() );
    }

    {
        XMarkableStreamRef markableOut( rOut , USR_QUERY );
        ERROR_ASSERT( markableOut.is() , "markable stream cannot be queried" );

        // do the same thing multiple times to check if
        // buffering and marks work correctly
        for( int i = 0 ; i < 2000 ; i ++ ) {

            XInterfaceRef x = xProv->createInstance(L"test.com.sun.star.io.PersistTest");
            XPersistObjectRef persistRef( x , USR_QUERY );

            XPropertySetRef rProp( persistRef , USR_QUERY );
            ERROR_ASSERT( rProp.is() , "test object is no property set " );

            UsrAny any;
            any.setINT32( 0x83482 );
            rProp->setPropertyValue( L"long" , any );

            any.setFloat( 42.23 );
            rProp->setPropertyValue( L"float" , any );

            any.setDouble(  233.321412 );
            rProp->setPropertyValue( L"double" , any );

            any.setBOOL( TRUE );
            rProp->setPropertyValue( L"bool" , any );

            any.setBYTE( 130 );
            rProp->setPropertyValue( L"byte" , any );

            any.setChar( 'h' );
            rProp->setPropertyValue( L"char" , any );

            any.setString( L"hi du !" );
            rProp->setPropertyValue( L"string" , any );


            x = xProv->createInstance(L"test.com.sun.star.io.PersistTest");
            XPersistObjectRef persist2ndRef( x , USR_QUERY );


            // Note : persist2ndRef contains coincident values, but also coincident values must be
            // saved properly !
            any.set( &persist2ndRef, XPersistObject_getReflection() ); // try a self reference
            rProp->setPropertyValue( L"object" , any );

            // simply test, if markable operations and object operations do not interfere
            INT32 nMark = markableOut->createMark();

            // do read and write
            rOut->writeObject( persistRef );

            // further markable tests !
            INT32 nOffset = markableOut->offsetToMark( nMark );
            markableOut->jumpToMark( nMark );
            markableOut->deleteMark( nMark );
            markableOut->jumpToFurthest();





            ERROR_ASSERT( 0 != rIn->available() , "no data arrived at input" );
            XPersistObjectRef xReadPersistRef   =   rIn->readObject( );

            XPropertySetRef rProp1( persistRef , USR_QUERY );
            XPropertySetRef rProp2( xReadPersistRef , USR_QUERY );
            ERROR_ASSERT( compareMyPropertySet( rProp1, rProp2) , "objects has not been read properly !" );
        }
    }
}


XInterfaceRef OObjectStreamTest_CreateInstance( const XMultiServiceFactoryRef & rSMgr ) THROWS((Exception))
{
    OObjectStreamTest *p = new OObjectStreamTest( rSMgr );
    XInterfaceRef xService = *p;
    return xService;
}

Sequence<UString> OObjectStreamTest_getSupportedServiceNames( int i) THROWS( () )
{
    Sequence<UString> aRet(1);
    aRet.getArray()[0] = OObjectStreamTest_getImplementationName( i);
    return aRet;
}

UString     OObjectStreamTest_getServiceName( int i) THROWS( () )
{
    if( 1 == i ) {
        return L"test.com.sun.star.io.ObjectInputStream";
    }
    else {
        return L"test.com.sun.star.io.ObjectOutputStream";
    }
}

UString     OObjectStreamTest_getImplementationName( int i) THROWS( () )
{
    if( 1 == i ) {
        return L"test.com.sun.star.comp.extensions.stm.ObjectInputStream";
    }
    else {
        return L"test.com.sun.star.comp.extensions.stm.ObjectOutputStream";
    }
}


