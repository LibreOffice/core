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


#include <osl/diagnose.h>
#include <osl/interlck.h>

#include <cppuhelper/factory.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>

#include <com/sun/star/test/performance/XPerformanceTest.hpp>

using namespace osl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::test::performance;


#define SERVICENAME     "com.sun.star.test.performance.PerformanceTestObject"
#define IMPLNAME        "com.sun.star.comp.performance.PerformanceTestObject"

namespace benchmark_object
{

//--------------------------------------------------------------------------------------------------
inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) );
    return Sequence< OUString >( &aName, 1 );
}

//==================================================================================================
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
    ServiceImpl( const Reference< XMultiServiceFactory > & xMgr )
        : _nRef( 0 )
        {}

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException)
    {
        // execution time remains appr. constant any time
        Any aRet;
        if (aType == ::getCppuType( (const Reference< XInterface > *)0 ))
        {
            void * p = (XInterface *)(XPerformanceTest *)this;
            aRet.setValue( &p, ::getCppuType( (const Reference< XInterface > *)0 ) );
        }
        if (aType == ::getCppuType( (const Reference< XPerformanceTest > *)0 ))
        {
            void * p = (XPerformanceTest *)this;
            aRet.setValue( &p, ::getCppuType( (const Reference< XPerformanceTest > *)0 ) );
        }
        if (! aRet.hasValue())
        {
            void * p = (XPerformanceTest *)this;
            Any aDummy( &p, ::getCppuType( (const Reference< XPerformanceTest > *)0 ) );
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
    virtual void SAL_CALL setLong_attr( sal_Int32 _attributelong ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual sal_Int64 SAL_CALL getHyper_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setHyper_attr( sal_Int64 _attributehyper ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual float SAL_CALL getFloat_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0.0; }
    virtual void SAL_CALL setFloat_attr( float _attributefloat ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual double SAL_CALL getDouble_attr() throw(::com::sun::star::uno::RuntimeException)
        { return 0.0; }
    virtual void SAL_CALL setDouble_attr( double _attributedouble ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual OUString SAL_CALL getString_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyString; }
    virtual void SAL_CALL setString_attr( const OUString& _attributestring ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Reference< XInterface > SAL_CALL getInterface_attr() throw(::com::sun::star::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void SAL_CALL setInterface_attr( const Reference< XInterface >& _attributeinterface ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Any SAL_CALL getAny_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void SAL_CALL setAny_attr( const Any& _attributeany ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > SAL_CALL getSequence_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void SAL_CALL setSequence_attr( const Sequence< Reference< XInterface > >& _attributesequence ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL getStruct_attr() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void SAL_CALL setStruct_attr( const ::com::sun::star::test::performance::ComplexTypes& _attributestruct ) throw(::com::sun::star::uno::RuntimeException)
        {}

    // Methods
    virtual sal_Int32 SAL_CALL getLong() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setLong( sal_Int32 _long ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual sal_Int64 SAL_CALL getHyper() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setHyper( sal_Int64 _hyper ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual float SAL_CALL getFloat() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setFloat( float _float ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual double SAL_CALL getDouble() throw(::com::sun::star::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setDouble( double _double ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyString; }
    virtual void SAL_CALL setString( const OUString& _string ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Reference< XInterface > SAL_CALL getInterface() throw(::com::sun::star::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void SAL_CALL setInterface( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _interface ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Any SAL_CALL getAny() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void SAL_CALL setAny( const ::com::sun::star::uno::Any& _any ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > SAL_CALL getSequence() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void SAL_CALL setSequence( const Sequence< Reference< XInterface > >& _sequence ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL getStruct() throw(::com::sun::star::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void SAL_CALL setStruct( const ::com::sun::star::test::performance::ComplexTypes& c ) throw(::com::sun::star::uno::RuntimeException)
        {}

    virtual void SAL_CALL async() throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual void SAL_CALL sync() throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL complex_in( const ::com::sun::star::test::performance::ComplexTypes& aVal ) throw(::com::sun::star::uno::RuntimeException)
        { return aVal; }
    virtual ComplexTypes SAL_CALL complex_inout( ::com::sun::star::test::performance::ComplexTypes& aVal ) throw(::com::sun::star::uno::RuntimeException)
        { return aVal; }
    virtual void SAL_CALL complex_oneway( const ::com::sun::star::test::performance::ComplexTypes& aVal ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual void SAL_CALL complex_noreturn( const ::com::sun::star::test::performance::ComplexTypes& aVal ) throw(::com::sun::star::uno::RuntimeException)
        {}
    virtual Reference< XPerformanceTest > SAL_CALL createObject() throw(::com::sun::star::uno::RuntimeException)
        { return new ServiceImpl(); }
    virtual void SAL_CALL raiseRuntimeException(  ) throw(::com::sun::star::uno::RuntimeException)
        { throw _aDummyRE; }
};

//##################################################################################################

// XServiceInfo
//__________________________________________________________________________________________________
OUString ServiceImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}
//__________________________________________________________________________________________________
sal_Bool ServiceImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    const Sequence< OUString > & rSNL = getSupportedServiceNames();
    const OUString * pArray = rSNL.getConstArray();
    for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
    {
        if (pArray[nPos] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
Sequence< OUString > ServiceImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return benchmark_object::getSupportedServiceNames();
}

// ...

//==================================================================================================
static Reference< XInterface > SAL_CALL ServiceImpl_create( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( (XPerformanceTest *)new ServiceImpl( xSMgr ) );
}

}


//##################################################################################################
//##################################################################################################
//##################################################################################################


extern "C"
{
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );
            xNewKey->createKey( OUString( RTL_CONSTASCII_USTRINGPARAM(SERVICENAME) ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) ),
            benchmark_object::ServiceImpl_create,
            benchmark_object::getSupportedServiceNames() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
