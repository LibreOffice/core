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

#include <string.h>
#include <stdlib.h>
#include <osl/time.h>
#include <uno/threadpool.h>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>
#include <test/XTestFactory.hpp>
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/test/performance/XPerformanceTest.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace ::test;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::bridge;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::test::performance;

#include "testcomp.h"


void parseCommandLine( char *argv[] ,
                       OUString *pConnection , OUString *pProtocol ,
                       sal_Bool *pbLatency , sal_Bool *pbReverse)
{
    sal_Int32 nArgIndex = 1;
    if( ! strcmp( argv[1] , "-r" ) )
    {
        nArgIndex = 2;
        *pbReverse = sal_True;
    }
    else if( ! strcmp( argv[1] , "-latency" ) )
    {
        *pbLatency = sal_True;
        nArgIndex = 2;
    }

    OUString sTemp = OUString::createFromAscii( argv[nArgIndex] );
    sal_Int32 nIndex = sTemp.indexOf( ';' );
    if( -1 == nIndex )
    {
        *pConnection = sTemp;
        *pProtocol    = "iiop";
    }
    else
    {
        *pConnection = sTemp.copy( 0 , nIndex );
        *pProtocol = sTemp.copy( nIndex+1, sTemp.getLength() - (nIndex+1) );
    }
}

Any OInstanceProvider::queryInterface( const  Type & aType ) throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface( aType ,
            (static_cast< XInstanceProvider *  >(this)) );
    if( a.hasValue() )
    {
        return a;
    }
    return OWeakObject::queryInterface( aType );
}

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    OInstanceProvider::getInstance( const OUString& sObjectName )
        throw(::com::sun::star::container::NoSuchElementException,
              ::com::sun::star::uno::RuntimeException)
{
    // Tries to get the PerformanceTestObject
    if( sObjectName == "TestRemoteObject" )
    {
        return m_rSMgr->createInstance("com.sun.star.test.performance.PerformanceTestObject");
    }
    return Reference < XInterface > ( (::cppu::OWeakObject * ) new OTestFactory() );
}

class ServiceImpl
    : public XServiceInfo
    , public XPerformanceTest
{
    OUString _aDummyString;
    Any _aDummyAny;
    Sequence< Reference< XInterface > > _aDummySequence;
    ComplexTypes _aDummyStruct;
    RuntimeException _aDummyRE;

    sal_Int32 _nRef;

public:
    ServiceImpl()
        : _nRef( 0 )
        {}
    ServiceImpl( const Reference< XMultiServiceFactory > & /* xMgr */)
        : _nRef( 0 )
        {}

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException)
    {
        // execution time remains appr. constant any time
        Any aRet;
        if (aType == cppu::UnoType<XInterface>::get())
        {
            void * p = (XInterface *)(XPerformanceTest *)this;
            aRet.setValue( &p, cppu::UnoType<XInterface>::get() );
        }
        if (aType == cppu::UnoType<XPerformanceTest>::get())
        {
            void * p = (XPerformanceTest *)this;
            aRet.setValue( &p, cppu::UnoType<XPerformanceTest>::get() );
        }
        if (! aRet.hasValue())
        {
            void * p = (XPerformanceTest *)this;
            Any aDummy( &p, cppu::UnoType<XPerformanceTest>::get() );
        }
        return aRet;
    }
    virtual void SAL_CALL acquire() throw()
        { osl_atomic_increment( &_nRef ); }
    virtual void SAL_CALL release() throw()
        { if (! osl_atomic_decrement( &_nRef )) delete this; }

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

    // Attributes
    virtual sal_Int32 SAL_CALL getLong_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setLong_attr( sal_Int32 /* _attributelong */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual sal_Int64 SAL_CALL getHyper_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setHyper_attr( sal_Int64 /* _attributehyper */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual float SAL_CALL getFloat_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0.0; }
    virtual void SAL_CALL setFloat_attr( float /* _attributefloat */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual double SAL_CALL getDouble_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0.0; }
    virtual void SAL_CALL setDouble_attr( double /* _attributedouble */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual OUString SAL_CALL getString_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyString; }
    virtual void SAL_CALL setString_attr( const OUString& /* _attributestring */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Reference< XInterface > SAL_CALL getInterface_attr() throw(::com::sun::star::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void SAL_CALL setInterface_attr( const Reference< XInterface >& /* _attributeinterface */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Any SAL_CALL getAny_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void SAL_CALL setAny_attr( const Any& /* _attributeany */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > SAL_CALL getSequence_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void SAL_CALL setSequence_attr( const Sequence< Reference< XInterface > >& /* _attributesequence */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL getStruct_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void SAL_CALL setStruct_attr( const ::com::sun::star::test::performance::ComplexTypes& /* _attributestruct */) throw(::com::sun::star::uno::RuntimeException)
        {}

    // Methods
    virtual sal_Int32 SAL_CALL getLong() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setLong( sal_Int32 /* _long */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual sal_Int64 SAL_CALL getHyper() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setHyper( sal_Int64 /* _hyper */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual float SAL_CALL getFloat() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setFloat( float /* _float */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual double SAL_CALL getDouble() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setDouble( double /* _double */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyString; }
    virtual void SAL_CALL setString( const OUString& /* _string */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Reference< XInterface > SAL_CALL getInterface() throw(::com::sun::star::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void SAL_CALL setInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& /* _interface */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Any SAL_CALL getAny() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void SAL_CALL setAny( const ::com::sun::star::uno::Any& /* _any */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > SAL_CALL getSequence() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void SAL_CALL setSequence( const Sequence< Reference< XInterface > >& /*_sequence */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL getStruct() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void SAL_CALL setStruct( const ::com::sun::star::test::performance::ComplexTypes& /* c */) throw(::com::sun::star::uno::RuntimeException)
        {}

    virtual void SAL_CALL async() throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL sync() throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL complex_in( const ::com::sun::star::test::performance::ComplexTypes& aVal ) throw(::com::sun::star::uno::RuntimeException)
        { return aVal; }
    virtual ComplexTypes SAL_CALL complex_inout( ::com::sun::star::test::performance::ComplexTypes& aVal ) throw(::com::sun::star::uno::RuntimeException)
        { return aVal; }
    virtual void SAL_CALL complex_oneway( const ::com::sun::star::test::performance::ComplexTypes& /* aVal */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual void SAL_CALL complex_noreturn( const ::com::sun::star::test::performance::ComplexTypes& /* aVal */) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Reference< XPerformanceTest > SAL_CALL createObject() throw(::com::sun::star::uno::RuntimeException)
        { return new ServiceImpl(); }
    virtual void SAL_CALL raiseRuntimeException(  ) throw(::com::sun::star::uno::RuntimeException)
        { throw _aDummyRE; }
};

void ServiceImpl::async() throw(::com::sun::star::uno::RuntimeException)
{}

// XServiceInfo
OUString ServiceImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( );
}

sal_Bool ServiceImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > ServiceImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return Sequence< OUString > ();
}

Any OCallMe::queryInterface( const  Type & aType )  throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface( aType,
            (static_cast< XCallMe *  >(this)) );

    if( a.hasValue() )
    {
        return a;
    }

    return OWeakObject::queryInterface( aType );
}

void OCallMe::call( const OUString& s, sal_Int32 nToDo )
    throw( RuntimeException, ::test::TestBridgeException)
{
    if( nToDo < 0 )
    {
        throw TestBridgeException();
    }

    OUString sDummy;
    if( ! nToDo ) {
        OString o = OUStringToOString( s,RTL_TEXTENCODING_ASCII_US);
        printf( "%s\n" , o.pData->buffer );
    }
    for( sal_Int32 i = 0 ; i < nToDo ; i ++ )
    {
        sDummy += s;
    }
}

void SAL_CALL OCallMe::drawLine( sal_Int32 /* x1 */, sal_Int32 /* y1 */, sal_Int32 /* x2 */, sal_Int32 /* y2 */)
        throw(::com::sun::star::uno::RuntimeException)
{
    // do nothings
}

void OCallMe::callOneway( const OUString& /* s */, sal_Int32 nToDo )
    throw(RuntimeException)
{
    OUString sDummy;
    m_nLastToDos = nToDo;


    if( nToDo )
    {
        printf( "+" );
        fflush( stdout );

        TimeValue val = { nToDo , 0  };
        osl_waitThread( &val );
        printf( "-\n" );
    }

}

::test::TestTypes SAL_CALL OCallMe::transport( const ::test::TestTypes& types )
        throw(::com::sun::star::uno::RuntimeException)
{
    return types;
}

OUString OCallMe::getsAttribute() throw(RuntimeException)
{
    return m_sAttribute;
}
void OCallMe::setsAttribute( const OUString& _sattribute )
     throw(RuntimeException)
{
    m_sAttribute = _sattribute;
}
void OCallMe::callAgain( const Reference< ::test::XCallMe >& callAgainArg,
                         sal_Int32 nToCall ) throw(RuntimeException)
{
    ::osl::MutexGuard guard( m_mutex );
    if( nToCall %2 )
    {
        printf( "Deadlocktest pong %" SAL_PRIdINT32 "\n", nToCall );
    }
    else
    {
        printf( "Deadlocktest ping %" SAL_PRIdINT32 "\n", nToCall );
    }
    if( nToCall )
    {
        callAgainArg->callAgain( Reference< XCallMe > ( (XCallMe *) this ) , nToCall -1 );
    }
}

Any OInterfaceTest::queryInterface( const Type & aType )  throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface( aType,
            (static_cast< XInterfaceTest *  >(this)) );
    if( a.hasValue() )
    {
        return a;
    }
    return OWeakObject::queryInterface( aType );
}

void OInterfaceTest::setIn(
    const Reference< ::test::XCallMe >& callback )
            throw(RuntimeException)
{
    m_rCallMe = callback;
    call();
}

void OInterfaceTest::setInOut( Reference< ::test::XCallMe >& callback )
    throw(RuntimeException)
{
    Reference< XCallMe > r = m_rCallMe;
    m_rCallMe = callback;
    callback = r;
    call();
}

void OInterfaceTest::getOut( Reference< ::test::XCallMe >& callback )
    throw(RuntimeException)
{
    callback = m_rCallMe;
}

Reference< ::test::XCallMe > OInterfaceTest::get(  )
    throw(RuntimeException)
{
    call();
    return m_rCallMe;
}

void OInterfaceTest::call()
{
    if( m_rCallMe.is() )
    {
        m_rCallMe->call( OUString("This is my String during a callback!") , 5);
    }
}

Any OTestFactory::queryInterface( const Type & aType )  throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface( aType,
            (static_cast< XTestFactory *  >(this)) );

    if( a.hasValue() )
    {
        return a;
    }

    return OWeakObject::queryInterface( aType );
}

Reference< ::test::XCallMe > OTestFactory::createCallMe(  )
        throw(RuntimeException)
{
    return Reference< XCallMe > ( (XCallMe * ) new OCallMe() );
}

Reference< ::test::XInterfaceTest > SAL_CALL OTestFactory::createInterfaceTest(  )
        throw(RuntimeException)
{
    return Reference < XInterfaceTest > ( (XInterfaceTest * ) new OInterfaceTest() );
}

//  class OInstanceProvider :
//      public ::cppu::OWeakObject,
//      public XInstanceProvider
//  {
//  public:
//      OInstanceProvider( ){}
//      ~OInstanceProvider(){ printf( "instance provider dies\n" );}
//  public:
//      // XInterface
//      Any         SAL_CALL queryInterface( const Type & aType);
//      void        SAL_CALL acquire()                       { OWeakObject::acquire(); }
//      void        SAL_CALL release()                       { OWeakObject::release(); }

//  public:
//      virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
//             getInstance( const OUString& sObjectName )
//                   throw( ::com::sun::star::container::NoSuchElementException,
//                          ::com::sun::star::uno::RuntimeException);
//  };

double getCallsPerSec( const Reference < XCallMe > &rCall , int nLoops, int nToDo )
{
    TimeValue aStartTime, aEndTime;
    osl_getSystemTime( &aStartTime );
    for( sal_Int32 i = 0; i < nLoops; i ++ )
    {
        rCall->call( OUString("Performance test string") , nToDo );
    }
    osl_getSystemTime( &aEndTime );

    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);
    return fEnd-fStart;
}

double getCallsPerSecOneway( const Reference < XCallMe > &rCall ,
                             int nLoops,
                             int nToDo,
                             double *pdAfterExecution
                             )
{
    TimeValue aStartTime, aEndTime, aAfterExecution;
    osl_getSystemTime( &aStartTime );
    for( sal_Int32 i = 0; i < nLoops; i ++ )
    {
//          rCall->callOneway( OUString("Performance test string" ), 0 );
          rCall->drawLine( 0 , 0 , 500 , 123 );
    }
    osl_getSystemTime( &aEndTime );

    rCall->call( OUString("Performance test string") , nToDo );
    osl_getSystemTime( &aAfterExecution );

    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);
    *pdAfterExecution =   (double)aAfterExecution.Seconds +
                         ((double)aAfterExecution.Nanosec / 1000000000.0) - fStart;
    return fEnd-fStart;
}

void testOnewayPerformanceOnTwoInterfaces(
    const Reference < XCallMe > &rRemote1, const Reference < XCallMe > &rRemote2 )
{
      printf( "Doing oneway performance test on two interfaces ...\n" );
    const sal_Int32 nLoops = 10000;
    TimeValue aStartTime, aEndTime;
    osl_getSystemTime( &aStartTime );
    for( sal_Int32 i = 0; i < nLoops ; i ++ )
    {
          rRemote1->drawLine( 0 , 0 , 500 , 123 );
          rRemote2->drawLine( 0 , 0 , 500 , 123 );
    }
    osl_getSystemTime( &aEndTime );
    double fStart = (double)aStartTime.Seconds + ((double)aStartTime.Nanosec / 1000000000.0);
    double fEnd = (double)aEndTime.Seconds + ((double)aEndTime.Nanosec / 1000000000.0);

      printf( "Overhead per Call [ms] %g\n" , ((fEnd-fStart)/((double)nLoops/1000 ))/2.  );
}

void testPerformance( const Reference < XCallMe > &rRemote,
                      const Reference < XCallMe > &rLocal )
{
    sal_Int32 nDoSomething = 1;
    sal_Int32 nCalls = 80000;
    double dRemote, dLocal,dAfterExecution;

      printf( "performance test oneway...\n" );
      dLocal  = getCallsPerSecOneway( rLocal  , nCalls , nDoSomething , &dAfterExecution);
       dRemote = getCallsPerSecOneway( rRemote , nCalls , nDoSomething , &dAfterExecution);
      printf( "Local=%g s,"
              "Remote : %g s\n" , dLocal, dRemote );
      if( dLocal > 0. )
      {
          printf( "Remote/Local : %g\n", dRemote/dLocal );
      }

      printf( "Overhead per Call [ms] %g\n" , (dRemote - dLocal)/((double)nCalls/1000 ) );
      printf( "Overhead per Call after completion [ms] %g\n" , (dAfterExecution - dLocal)/((double)nCalls/1000 ) );

       nCalls = 2000;

      printf( "Doing performance test ...\n" );
       dRemote = getCallsPerSec( rRemote , nCalls , nDoSomething );
      dLocal  = getCallsPerSec( rLocal  , nCalls , nDoSomething );
      printf( "Local=%g s,\n"
              "Remote=%g s\n" , dLocal, dRemote );
      if( dLocal > 0. )
      {
          printf( "Remote/Local : %g\n", dRemote/dLocal );
      }
      printf( "Overhead per synchron Call [ms] %g\n" , ((dRemote - dLocal)/((double)nCalls/1000 )) );
}

void testException( const Reference < XCallMe > &r )
{
    try {
        r->call( OUString("dummy") , -1 );
        OSL_ASSERT( ! "no exception flown !" );
    }
    catch( TestBridgeException  & e )
    {
        // Exception flew successfully !
    }
    catch( Exception & e )
    {
        OSL_ASSERT( ! "only base class of exception could be catched!" );
    }
    catch(...)
    {
        OSL_ASSERT(! "wrong unknown exception !" );
    }
}

void testSequenceOfCalls( const Reference< XCallMe > & rRCallMe )
{
    printf( "Testing sequence of calls\n" );
    for( sal_Int32 i = 0 ; i < 800 ; i ++ )
    {
        rRCallMe->callOneway( OUString("hifuj" ), 0 );
    }
}

void testAllTypes( const Reference < XCallMe > & rRCallMe )
{
    printf( "Testing all types\n" );

    for( sal_Int32 i = 0; i < 32 ; i ++ )
    {

        TestTypes types;
        types.Bool = sal_True;
        types.Char = L'i';
        types.Byte = -12;
        types.Short = -32000;
        types.UShort = (sal_uInt16 ) (1 << i);
        types.Long = -123;
        types.ULong = 1 << i;
        types.Hyper = 50;
        types.UHyper = 1 << i*2;
        types.Float = (float)123.239;
        types.Double = 1279.12490012;
        types.String = OUString("abcdefghijklmnopqrstuvwxyz");
        types.Interface.set( rRCallMe , UNO_QUERY);
        types.Any <<= types.Double;

        TestTypes retTypes = rRCallMe->transport( types );

        OSL_ASSERT( ( types.Bool && retTypes.Bool  ) || ( ! types.Bool && ! retTypes.Bool ) );
        OSL_ASSERT( types.Char == retTypes.Char );
        OSL_ASSERT( types.Byte == retTypes.Byte );
        OSL_ASSERT( types.Short == retTypes.Short );
        OSL_ASSERT( types.UShort == retTypes.UShort );
        OSL_ASSERT( types.Long == retTypes.Long );
        OSL_ASSERT( types.ULong == retTypes.ULong );
        OSL_ASSERT( types.Hyper == retTypes.Hyper );
        OSL_ASSERT( types.UHyper == retTypes.UHyper );
        OSL_ASSERT( types.Float == retTypes.Float );
        OSL_ASSERT( types.Double == retTypes.Double );
        OSL_ASSERT( types.String == retTypes.String );
        OSL_ASSERT( types.Interface == retTypes.Interface );
        OSL_ASSERT( types.Any == retTypes.Any );
    }
}

void testRemote( const Reference< XInterface > &rRemote )
{
    cppu::UnoType<sal_Int8>::get();

    Reference< XTestFactory > rRFact( rRemote , UNO_QUERY );
    if( ! rRFact.is() )
    {
        printf( "remote object doesn't support XTestFactory\n" );
        return;
    }
    OSL_ASSERT( rRFact.is() );
    Reference< XCallMe > rLCallMe = (XCallMe * ) new OCallMe();
    Reference< XCallMe > rRCallMe = rRFact->createCallMe();

      testAllTypes( rLCallMe );
      testAllTypes( rRCallMe );

      printf( "Testing exception local ...\n" );
      testException( rLCallMe );
    printf( "Testing exception remote ...\n" );
    testException( rRCallMe );

      // Test attributes
       OUString ow( "dum didel dum dideldei" );
       rLCallMe->setsAttribute( ow );
       OSL_ASSERT( rLCallMe->getsAttribute() == ow );

         rRCallMe->setsAttribute( ow );
         OSL_ASSERT( rRCallMe->getsAttribute() == ow );

    // Performance test
    testPerformance( rRCallMe , rLCallMe );
     testOnewayPerformanceOnTwoInterfaces( rRFact->createCallMe(), rRCallMe );

     // Test sequence
       testSequenceOfCalls( rRCallMe );


     // test triple to check if transporting the same interface multiple
     // times causes any problems
       Reference< XInterfaceTest > rRTest = rRFact->createInterfaceTest();
       Reference< XInterfaceTest > rRTest2 = rRFact->createInterfaceTest();
       Reference< XInterfaceTest > rRTest3 = rRFact->createInterfaceTest();

     rRTest->setIn( rRCallMe );
     rRTest2->setIn( rRCallMe );
     rRTest3->setIn( rRCallMe );

     OSL_ASSERT( rRTest->get() == rRCallMe );
     OSL_ASSERT( rRTest2->get() == rRCallMe );
     OSL_ASSERT( rRTest3->get() == rRCallMe );

       rRTest->setIn( rLCallMe );
       rRTest2->setIn( rLCallMe );
       rRTest3->setIn( rLCallMe );

     {
         Reference< XCallMe > rLCallMe1 = (XCallMe * ) new OCallMe();
         Reference< XCallMe > rLCallMe2 = (XCallMe * ) new OCallMe();
         Reference< XCallMe > rLCallMe3 = (XCallMe * ) new OCallMe();
         rRTest->setIn( rLCallMe1 );
         rRTest2->setIn( rLCallMe2 );
         rRTest3->setIn( rLCallMe3 );
         OSL_ASSERT( rRTest->get() == rLCallMe1 );
         OSL_ASSERT( rRTest2->get() == rLCallMe2 );
         OSL_ASSERT( rRTest3->get() == rLCallMe3 );

         rRTest->setIn( rLCallMe );
         rRTest2->setIn( rLCallMe );
         rRTest3->setIn( rLCallMe );

         OSL_ASSERT( rRTest->get() == rLCallMe );
         OSL_ASSERT( rRTest2->get() == rLCallMe );
         OSL_ASSERT( rRTest3->get() == rLCallMe );
     }

     Reference < XCallMe > r = rRCallMe;
     rRTest->setInOut( r );
     OSL_ASSERT( r == rLCallMe );
     OSL_ASSERT( ! ( r == rRCallMe ) );

     // test empty references
     rRTest->setIn( Reference < XCallMe > () );

       // test thread deadlocking
      rLCallMe->callAgain( rRCallMe, 20 );

}

Reference <XInterface > createComponent( const OUString &sService ,
                                         const OUString &sDllName,
                                         const Reference < XMultiServiceFactory > &rSMgr )
{
    Reference< XInterface > rInterface;
    rInterface = rSMgr->createInstance( sService );

    if( ! rInterface.is() )
    {
        // erst registrieren
        Reference < XImplementationRegistration > rReg (
            rSMgr->createInstance("com.sun.star.registry.ImplementationRegistration"),
            UNO_QUERY );

        OSL_ASSERT( rReg.is() );
        OUString aDllName = sDllName;

        try
        {
            rReg->registerImplementation(
                OUString( "com.sun.star.loader.SharedLibrary" ),
                aDllName,
                Reference< XSimpleRegistry > () );
            rInterface = rSMgr->createInstance( sService );
        }
        catch( Exception & )
        {
            printf( "couldn't register dll %s\n" ,
                    OUStringToOString( aDllName, RTL_TEXTENCODING_ASCII_US ).getStr()  );
        }
    }
    return rInterface;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
