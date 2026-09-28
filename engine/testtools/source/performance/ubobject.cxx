/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
using namespace ::cpo::uno;
using namespace cpo::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::registry;
using namespace com::sun::star::test::performance;


#define SERVICENAME     "com.sun.star.test.performance.PerformanceTestObject"
#define IMPLNAME        "com.sun.star.comp.performance.PerformanceTestObject"

namespace benchmark_object
{


inline static Sequence< OUString > getSupportedServiceNames()
{
    return { SERVICENAME };
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
    virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) throw(cpo::uno::RuntimeException)
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
    virtual void acquire() throw()
        { osl_atomic_increment( &_nRef ); }
    virtual void release() throw()
        { if (! osl_atomic_decrement( &_nRef )) delete this; }

    // XServiceInfo
    virtual OUString getImplementationName() throw (RuntimeException);
    virtual bool supportsService( const OUString & rServiceName ) throw (RuntimeException);
    virtual Sequence< OUString > getSupportedServiceNames() throw (RuntimeException);

    // Attributes
    virtual sal_Int32 getLong_attr() throw(cpo::uno::RuntimeException)
        { return 0; }
    virtual void setLong_attr( sal_Int32 _attributelong ) throw(cpo::uno::RuntimeException)
        {}
    virtual sal_Int64 getHyper_attr() throw(cpo::uno::RuntimeException)
        { return 0; }
    virtual void setHyper_attr( sal_Int64 _attributehyper ) throw(cpo::uno::RuntimeException)
        {}
    virtual float getFloat_attr() throw(cpo::uno::RuntimeException)
        { return 0.0; }
    virtual void setFloat_attr( float _attributefloat ) throw(cpo::uno::RuntimeException)
        {}
    virtual double getDouble_attr() throw(cpo::uno::RuntimeException)
        { return 0.0; }
    virtual void setDouble_attr( double _attributedouble ) throw(cpo::uno::RuntimeException)
        {}
    virtual OUString getString_attr() throw(cpo::uno::RuntimeException)
        { return _aDummyString; }
    virtual void setString_attr( const OUString& _attributestring ) throw(cpo::uno::RuntimeException)
        {}
    virtual Reference< XInterface > getInterface_attr() throw(cpo::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void setInterface_attr( const Reference< XInterface >& _attributeinterface ) throw(cpo::uno::RuntimeException)
        {}
    virtual Any getAny_attr() throw(cpo::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void setAny_attr( const Any& _attributeany ) throw(cpo::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > getSequence_attr() throw(cpo::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void setSequence_attr( const Sequence< Reference< XInterface > >& _attributesequence ) throw(cpo::uno::RuntimeException)
        {}
    virtual ComplexTypes getStruct_attr() throw(cpo::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void setStruct_attr( const css::test::performance::ComplexTypes& _attributestruct ) throw(cpo::uno::RuntimeException)
        {}

    // Methods
    virtual sal_Int32 getLong() throw(cpo::uno::RuntimeException)
        { return 0; }
    virtual void setLong( sal_Int32 _long ) throw(cpo::uno::RuntimeException)
        {}
    virtual sal_Int64 getHyper() throw(cpo::uno::RuntimeException)
        { return 0; }
    virtual void setHyper( sal_Int64 _hyper ) throw(cpo::uno::RuntimeException)
        {}
    virtual float getFloat() throw(cpo::uno::RuntimeException)
        { return 0; }
    virtual void setFloat( float _float ) throw(cpo::uno::RuntimeException)
        {}
    virtual double getDouble() throw(cpo::uno::RuntimeException)
        { return 0; }
    virtual void setDouble( double _double ) throw(cpo::uno::RuntimeException)
        {}
    virtual OUString getString() throw(cpo::uno::RuntimeException)
        { return _aDummyString; }
    virtual void setString( const OUString& _string ) throw(cpo::uno::RuntimeException)
        {}
    virtual Reference< XInterface > getInterface() throw(cpo::uno::RuntimeException)
        { return Reference< XInterface >(); }
    virtual void setInterface( const cpo::uno::Reference< cpo::uno::XInterface >& _interface ) throw(cpo::uno::RuntimeException)
        {}
    virtual Any getAny() throw(cpo::uno::RuntimeException)
        { return _aDummyAny; }
    virtual void setAny( const cpo::uno::Any& _any ) throw(cpo::uno::RuntimeException)
        {}
    virtual Sequence< Reference< XInterface > > getSequence() throw(cpo::uno::RuntimeException)
        { return _aDummySequence; }
    virtual void setSequence( const Sequence< Reference< XInterface > >& _sequence ) throw(cpo::uno::RuntimeException)
        {}
    virtual ComplexTypes getStruct() throw(cpo::uno::RuntimeException)
        { return _aDummyStruct; }
    virtual void setStruct( const css::test::performance::ComplexTypes& c ) throw(cpo::uno::RuntimeException)
        {}

    virtual void async() throw(cpo::uno::RuntimeException)
        {}
    virtual void sync() throw(cpo::uno::RuntimeException)
        {}
    virtual ComplexTypes complexIn( const css::test::performance::ComplexTypes& aVal ) throw(cpo::uno::RuntimeException)
        { return aVal; }
    virtual ComplexTypes complexInout( css::test::performance::ComplexTypes& aVal ) throw(cpo::uno::RuntimeException)
        { return aVal; }
    virtual void complexOneway( const css::test::performance::ComplexTypes& aVal ) throw(cpo::uno::RuntimeException)
        {}
    virtual void complexNoreturn( const css::test::performance::ComplexTypes& aVal ) throw(cpo::uno::RuntimeException)
        {}
    virtual Reference< XPerformanceTest > createObject() throw(cpo::uno::RuntimeException)
        { return new ServiceImpl(); }
    virtual void raiseRuntimeException(  ) throw(cpo::uno::RuntimeException)
        { throw _aDummyRE; }
};


// XServiceInfo

OUString ServiceImpl::getImplementationName()
    throw (RuntimeException)
{
    return OUString( IMPLNAME );
}

bool ServiceImpl::supportsService( const OUString & rServiceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > ServiceImpl::getSupportedServiceNames()
    throw (RuntimeException)
{
    return benchmark_object::getSupportedServiceNames();
}


static Reference< XInterface > ServiceImpl_create( const Reference< XMultiServiceFactory > & xSMgr )
{
    return Reference< XInterface >( (XPerformanceTest *)new ServiceImpl( xSMgr ) );
}

}


extern "C"
{
bool component_writeInfo(
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

            return true;
        }
        catch (InvalidRegistryException &)
        {
            OSL_FAIL( "### InvalidRegistryException!" );
        }
    }
    return false;
}

SAL_DLLPUBLIC_EXPORT void * component_getFactory(
    const char * pImplName, void * pServiceManager, void * pRegistryKey )
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
