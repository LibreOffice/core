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
#include <cppuhelper/supportsservice.hxx>

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


inline static Sequence< OUString > getSupportedServiceNames()
{
    OUString aName( SERVICENAME);
    return Sequence< OUString >( &aName, 1 );
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
    explicit ServiceImpl( const Reference< XMultiServiceFactory > & xMgr )
        : _nRef( 0 )
        {}

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException)
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
    virtual sal_Int32 SAL_CALL getLong_attr() throw(css::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setLong_attr( sal_Int32 _attributelong ) throw(css::uno::RuntimeException)
        {}
    virtual sal_Int64 SAL_CALL getHyper_attr() throw(css::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setHyper_attr( sal_Int64 _attributehyper ) throw(css::uno::RuntimeException)
        {}
    virtual float SAL_CALL getFloat_attr() throw(css::uno::RuntimeException)
        { return 0.0; }
    virtual void SAL_CALL setFloat_attr( float _attributefloat ) throw(css::uno::RuntimeException)
        {}
    virtual double SAL_CALL getDouble_attr() throw(css::uno::RuntimeException)
        { return 0.0; }
    virtual void SAL_CALL setDouble_attr( double _attributedouble ) throw(css::uno::RuntimeException)
        {}
    virtual OUString SAL_CALL getString_attr() throw(css::uno::RuntimeException)
        { return _aDummyString; }
    virtual void SAL_CALL setString_attr( const OUString& _attributestring ) throw(css::uno::RuntimeException)
        {}
    virtual Reference< XInterface > SAL_CALL getInterface_attr() throw(css::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void SAL_CALL setInterface_attr( const Reference< XInterface >& _attributeinterface ) throw(css::uno::RuntimeException)
        {}
    virtual Any SAL_CALL getAny_attr() throw(css::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void SAL_CALL setAny_attr( const Any& _attributeany ) throw(css::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > SAL_CALL getSequence_attr() throw(css::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void SAL_CALL setSequence_attr( const Sequence< Reference< XInterface > >& _attributesequence ) throw(css::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL getStruct_attr() throw(css::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void SAL_CALL setStruct_attr( const css::test::performance::ComplexTypes& _attributestruct ) throw(css::uno::RuntimeException)
        {}

    // Methods
    virtual sal_Int32 SAL_CALL getLong() throw(css::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setLong( sal_Int32 _long ) throw(css::uno::RuntimeException)
        {}
    virtual sal_Int64 SAL_CALL getHyper() throw(css::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setHyper( sal_Int64 _hyper ) throw(css::uno::RuntimeException)
        {}
    virtual float SAL_CALL getFloat() throw(css::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setFloat( float _float ) throw(css::uno::RuntimeException)
        {}
    virtual double SAL_CALL getDouble() throw(css::uno::RuntimeException)
        { return 0; }
    virtual void SAL_CALL setDouble( double _double ) throw(css::uno::RuntimeException)
        {}
    virtual OUString SAL_CALL getString() throw(css::uno::RuntimeException)
        { return _aDummyString; }
    virtual void SAL_CALL setString( const OUString& _string ) throw(css::uno::RuntimeException)
        {}
    virtual Reference< XInterface > SAL_CALL getInterface() throw(css::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void SAL_CALL setInterface( const css::uno::Reference< css::uno::XInterface >& _interface ) throw(css::uno::RuntimeException)
        {}
    virtual Any SAL_CALL getAny() throw(css::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void SAL_CALL setAny( const css::uno::Any& _any ) throw(css::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > SAL_CALL getSequence() throw(css::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void SAL_CALL setSequence( const Sequence< Reference< XInterface > >& _sequence ) throw(css::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL getStruct() throw(css::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void SAL_CALL setStruct( const css::test::performance::ComplexTypes& c ) throw(css::uno::RuntimeException)
        {}

    virtual void SAL_CALL async() throw(css::uno::RuntimeException)
        {}
    virtual void SAL_CALL sync() throw(css::uno::RuntimeException)
        {}
    virtual ComplexTypes SAL_CALL complex_in( const css::test::performance::ComplexTypes& aVal ) throw(css::uno::RuntimeException)
        { return aVal; }
    virtual ComplexTypes SAL_CALL complex_inout( css::test::performance::ComplexTypes& aVal ) throw(css::uno::RuntimeException)
        { return aVal; }
    virtual void SAL_CALL complex_oneway( const css::test::performance::ComplexTypes& aVal ) throw(css::uno::RuntimeException)
        {}
    virtual void SAL_CALL complex_noreturn( const css::test::performance::ComplexTypes& aVal ) throw(css::uno::RuntimeException)
        {}
    virtual Reference< XPerformanceTest > SAL_CALL createObject() throw(css::uno::RuntimeException)
        { return new ServiceImpl(); }
    virtual void SAL_CALL raiseRuntimeException(  ) throw(css::uno::RuntimeException)
        { throw _aDummyRE; }
};



// XServiceInfo

OUString ServiceImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( IMPLNAME );
}

sal_Bool ServiceImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > ServiceImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return benchmark_object::getSupportedServiceNames();
}




static Reference< XInterface > SAL_CALL ServiceImpl_create( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( (XPerformanceTest *)new ServiceImpl( xSMgr ) );
}

}







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
                    OUString( ("/" IMPLNAME "/UNO/SERVICES") ) ) );
            xNewKey->createKey( OUString( SERVICENAME ) );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}

SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if (pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
    {
        Reference< XSingleServiceFactory > xFactory( createSingleFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            OUString( IMPLNAME ),
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
