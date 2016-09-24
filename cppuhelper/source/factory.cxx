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
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/instance.hxx>
#include <rtl/unload.h>

#include <cppuhelper/propshlp.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <memory>


using namespace osl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::loader;
using namespace com::sun::star::registry;

using ::rtl::OUString;

namespace cppu
{

class OSingleFactoryHelper
    : public XServiceInfo
    , public XSingleServiceFactory
    , public lang::XSingleComponentFactory
    , public XUnloadingPreference
{
public:
    OSingleFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        ComponentInstantiation pCreateFunction_,
        ComponentFactoryFunc fptr,
        const Sequence< OUString > * pServiceNames_ )
        : xSMgr( rServiceManager )
        , pCreateFunction( pCreateFunction_ )
        , m_fptr( fptr )
        , aImplementationName( rImplementationName_ )
        {
            if( pServiceNames_ )
                aServiceNames = *pServiceNames_;
        }

    virtual ~OSingleFactoryHelper();

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType )
        throw(css::uno::RuntimeException, std::exception) override;

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    // XSingleComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException, std::exception) override;
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(css::uno::RuntimeException, std::exception) override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException, std::exception) override;

protected:
    /**
     * Create an instance specified by the factory. The one instance logic is implemented
     * in the createInstance and createInstanceWithArguments methods.
     * @return the newly created instance. Do not return a previous (one instance) instance.
     */
    virtual Reference<XInterface >  createInstanceEveryTime(
        Reference< XComponentContext > const & xContext )
        throw(css::uno::Exception, css::uno::RuntimeException);

    Reference<XMultiServiceFactory > xSMgr;
    ComponentInstantiation           pCreateFunction;
    ComponentFactoryFunc             m_fptr;
    Sequence< OUString >             aServiceNames;
    OUString                         aImplementationName;
};
OSingleFactoryHelper::~OSingleFactoryHelper()
{
}


Any OSingleFactoryHelper::queryInterface( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    return ::cppu::queryInterface(
        rType,
        static_cast< XSingleComponentFactory * >( this ),
        static_cast< XSingleServiceFactory * >( this ),
        static_cast< XServiceInfo * >( this ) ,
        static_cast< XUnloadingPreference * >( this ));
}

// OSingleFactoryHelper
Reference<XInterface > OSingleFactoryHelper::createInstanceEveryTime(
    Reference< XComponentContext > const & xContext )
    throw(css::uno::Exception, css::uno::RuntimeException)
{
    if (m_fptr)
    {
        return (*m_fptr)( xContext );
    }
    else if( pCreateFunction )
    {
        if (xContext.is())
        {
            Reference< lang::XMultiServiceFactory > xContextMgr(
                xContext->getServiceManager(), UNO_QUERY );
            if (xContextMgr.is())
                return (*pCreateFunction)( xContextMgr );
        }
        return (*pCreateFunction)( xSMgr );
    }
    else
    {
        return Reference< XInterface >();
    }
}

// XSingleServiceFactory
Reference<XInterface > OSingleFactoryHelper::createInstance()
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    return createInstanceWithContext( Reference< XComponentContext >() );
}

// XSingleServiceFactory
Reference<XInterface > OSingleFactoryHelper::createInstanceWithArguments(
    const Sequence<Any>& Arguments )
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    return createInstanceWithArgumentsAndContext(
        Arguments, Reference< XComponentContext >() );
}

// XSingleComponentFactory

Reference< XInterface > OSingleFactoryHelper::createInstanceWithContext(
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException, std::exception)
{
    return createInstanceEveryTime( xContext );
}

Reference< XInterface > OSingleFactoryHelper::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException, std::exception)
{
    Reference< XInterface > xRet( createInstanceWithContext( xContext ) );

    Reference< lang::XInitialization > xInit( xRet, UNO_QUERY );
    // always call initialize, even if there are no arguments.
    // #i63511# / 2006-03-27 / frank.schoenheit@sun.com
    if (xInit.is())
    {
        xInit->initialize( rArguments );
    }
    else
    {
        if ( rArguments.getLength() )
        {
            // dispose the here created UNO object before throwing out exception
            // to avoid risk of memory leaks #i113722#
            Reference<XComponent> xComp( xRet, UNO_QUERY );
            if (xComp.is())
                xComp->dispose();

            throw lang::IllegalArgumentException(
                OUString("cannot pass arguments to component => no XInitialization implemented!"),
                Reference< XInterface >(), 0 );
        }
    }

    return xRet;
}

// XServiceInfo
OUString OSingleFactoryHelper::getImplementationName()
    throw(css::uno::RuntimeException, std::exception)
{
    return aImplementationName;
}

// XServiceInfo
sal_Bool OSingleFactoryHelper::supportsService(
    const OUString& ServiceName )
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OSingleFactoryHelper::getSupportedServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    return aServiceNames;
}

struct OFactoryComponentHelper_Mutex
{
    Mutex   aMutex;
};

class OFactoryComponentHelper
    : public OFactoryComponentHelper_Mutex
    , public OComponentHelper
    , public OSingleFactoryHelper
{
public:
    OFactoryComponentHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        ComponentInstantiation pCreateFunction_,
        ComponentFactoryFunc fptr,
        const Sequence< OUString > * pServiceNames_,
        bool bOneInstance_ = false )
        : OComponentHelper( aMutex )
        , OSingleFactoryHelper( rServiceManager, rImplementationName_, pCreateFunction_, fptr, pServiceNames_ )
        , bOneInstance( bOneInstance_ )
        {
        }

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType )
        throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL acquire() throw() override
        { OComponentHelper::acquire(); }
    void SAL_CALL release() throw() override
        { OComponentHelper::release(); }

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    Reference<XInterface > SAL_CALL createInstanceWithArguments( const Sequence<Any>& Arguments )
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    // XSingleComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException, std::exception) override;
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException, std::exception) override;

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (css::uno::RuntimeException, std::exception) override;
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(css::uno::RuntimeException, std::exception) override;

    // XAggregation
    Any SAL_CALL queryAggregation( const Type & rType )
        throw(css::uno::RuntimeException, std::exception) override;

    // XUnloadingPreference
    virtual sal_Bool SAL_CALL releaseOnNotification()
        throw(css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

private:
    Reference<XInterface >  xTheInstance;
    bool                bOneInstance;
protected:
    // needed for implementing XUnloadingPreference in inheriting classes
    bool isOneInstance() {return bOneInstance;}
    bool isInstance() {return xTheInstance.is();}
};


Any SAL_CALL OFactoryComponentHelper::queryInterface( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    if( rType == cppu::UnoType<XUnloadingPreference>::get() )
    {
        return makeAny(
            Reference< XUnloadingPreference >(
                static_cast< XUnloadingPreference * >(this) ) );
    }
    return OComponentHelper::queryInterface( rType );
}

// XAggregation
Any OFactoryComponentHelper::queryAggregation( const Type & rType )
    throw(css::uno::RuntimeException, std::exception)
{
    Any aRet( OComponentHelper::queryAggregation( rType ) );
    return (aRet.hasValue() ? aRet : OSingleFactoryHelper::queryInterface( rType ));
}

// XTypeProvider
Sequence< Type > OFactoryComponentHelper::getTypes()
    throw (css::uno::RuntimeException, std::exception)
{
    Type ar[ 4 ];
    ar[ 0 ] = cppu::UnoType<XSingleServiceFactory>::get();
    ar[ 1 ] = cppu::UnoType<XServiceInfo>::get();
    ar[ 2 ] = cppu::UnoType<XUnloadingPreference>::get();

    if (m_fptr)
        ar[ 3 ] = cppu::UnoType<XSingleComponentFactory>::get();

    return Sequence< Type >( ar, m_fptr ? 4 : 3 );
}

Sequence< sal_Int8 > OFactoryComponentHelper::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XSingleServiceFactory
Reference<XInterface > OFactoryComponentHelper::createInstance()
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    if( bOneInstance )
    {
        if( !xTheInstance.is() )
        {
            MutexGuard aGuard( aMutex );
            if( !xTheInstance.is() )
                xTheInstance = OSingleFactoryHelper::createInstance();
        }
        return xTheInstance;
    }
    return OSingleFactoryHelper::createInstance();
}

Reference<XInterface > OFactoryComponentHelper::createInstanceWithArguments(
    const Sequence<Any>& Arguments )
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    if( bOneInstance )
    {
        if( !xTheInstance.is() )
        {
            MutexGuard aGuard( aMutex );
//          OSL_ENSURE( !xTheInstance.is(), "### arguments will be ignored!" );
            if( !xTheInstance.is() )
                xTheInstance = OSingleFactoryHelper::createInstanceWithArguments( Arguments );
        }
        return xTheInstance;
    }
    return OSingleFactoryHelper::createInstanceWithArguments( Arguments );
}

// XSingleComponentFactory

Reference< XInterface > OFactoryComponentHelper::createInstanceWithContext(
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException, std::exception)
{
    if( bOneInstance )
    {
        if( !xTheInstance.is() )
        {
            MutexGuard aGuard( aMutex );
//          OSL_ENSURE( !xTheInstance.is(), "### context will be ignored!" );
            if( !xTheInstance.is() )
                xTheInstance = OSingleFactoryHelper::createInstanceWithContext( xContext );
        }
        return xTheInstance;
    }
    return OSingleFactoryHelper::createInstanceWithContext( xContext );
}

Reference< XInterface > OFactoryComponentHelper::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException, std::exception)
{
    if( bOneInstance )
    {
        if( !xTheInstance.is() )
        {
            MutexGuard aGuard( aMutex );
//          OSL_ENSURE( !xTheInstance.is(), "### context and arguments will be ignored!" );
            if( !xTheInstance.is() )
                xTheInstance = OSingleFactoryHelper::createInstanceWithArgumentsAndContext( rArguments, xContext );
        }
        return xTheInstance;
    }
    return OSingleFactoryHelper::createInstanceWithArgumentsAndContext( rArguments, xContext );
}


// OComponentHelper
void OFactoryComponentHelper::dispose()
    throw(css::uno::RuntimeException, std::exception)
{
    OComponentHelper::dispose();

    Reference<XInterface > x;
    {
        // do not delete in the guard section
        MutexGuard aGuard( aMutex );
        x = xTheInstance;
        xTheInstance.clear();
    }
    // if it is a component call dispose at the component
    Reference<XComponent > xComp( x, UNO_QUERY );
    if( xComp.is() )
        xComp->dispose();
}

// XUnloadingPreference
// This class is used for single factories, component factories and
// one-instance factories. Depending on the usage this function has
// to return different values.
// one-instance factory: sal_False
// single factory: sal_True
// component factory: sal_True
sal_Bool SAL_CALL OFactoryComponentHelper::releaseOnNotification() throw(css::uno::RuntimeException, std::exception)
{
    if( bOneInstance)
        return false;
    return true;
}

class ORegistryFactoryHelper : public OFactoryComponentHelper,
                               public OPropertySetHelper

{
public:
    ORegistryFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        const Reference<XRegistryKey > & xImplementationKey_,
        bool bOneInstance_ = false )
            : OFactoryComponentHelper(
                rServiceManager, rImplementationName_, nullptr, nullptr, nullptr, bOneInstance_ ),
              OPropertySetHelper( OComponentHelper::rBHelper ),
              xImplementationKey( xImplementationKey_ )
        {}

    // XInterface
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;
    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes()
        throw (RuntimeException, std::exception) override;
    // XPropertySet
    virtual Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (RuntimeException, std::exception) override;

    // OPropertySetHelper
    virtual IPropertyArrayHelper & SAL_CALL getInfoHelper() override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        Any & rConvertedValue, Any & rOldValue,
        sal_Int32 nHandle, Any const & rValue )
        throw (lang::IllegalArgumentException) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle, Any const & rValue )
        throw (Exception, std::exception) override;
    using OPropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue(
        Any & rValue, sal_Int32 nHandle ) const override;

    // OSingleFactoryHelper
    Reference<XInterface > createInstanceEveryTime(
        Reference< XComponentContext > const & xContext )
        throw(css::uno::Exception, css::uno::RuntimeException) override;

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    // XSingleComponentFactory
    Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException, std::exception) override;

    // XServiceInfo
    Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException, std::exception) override;
    // XUnloadingPreference
    sal_Bool SAL_CALL releaseOnNotification()
        throw( RuntimeException, std::exception) override;


private:
    Reference< XInterface > createModuleFactory()
        throw(css::uno::Exception, css::uno::RuntimeException);

    /** The registry key of the implementation section */
    Reference<XRegistryKey >    xImplementationKey;
    /** The factory created with the loader. */
    Reference<XSingleComponentFactory > xModuleFactory;
    Reference<XSingleServiceFactory >   xModuleFactoryDepr;
    Reference< beans::XPropertySetInfo > m_xInfo;
    ::std::unique_ptr< IPropertyArrayHelper > m_property_array_helper;
protected:
    using OPropertySetHelper::getTypes;
};

// XInterface

Any SAL_CALL ORegistryFactoryHelper::queryInterface(
    Type const & type ) throw (RuntimeException, std::exception)
{
    Any ret( OFactoryComponentHelper::queryInterface( type ) );
    if (ret.hasValue())
        return ret;
    else
        return OPropertySetHelper::queryInterface( type );
}


void ORegistryFactoryHelper::acquire() throw ()
{
    OFactoryComponentHelper::acquire();
}


void ORegistryFactoryHelper::release() throw ()
{
    OFactoryComponentHelper::release();
}

// XTypeProvider

Sequence< Type > ORegistryFactoryHelper::getTypes() throw (RuntimeException, std::exception)
{
    Sequence< Type > types( OFactoryComponentHelper::getTypes() );
    sal_Int32 pos = types.getLength();
    types.realloc( pos + 3 );
    Type * p = types.getArray();
    p[ pos++ ] = cppu::UnoType<beans::XMultiPropertySet>::get();
    p[ pos++ ] = cppu::UnoType<beans::XFastPropertySet>::get();
    p[ pos++ ] = cppu::UnoType<beans::XPropertySet>::get();
    return types;
}

// XPropertySet

Reference< beans::XPropertySetInfo >
ORegistryFactoryHelper::getPropertySetInfo() throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard guard( aMutex );
    if (! m_xInfo.is())
        m_xInfo = createPropertySetInfo( getInfoHelper() );
    return m_xInfo;
}

// OPropertySetHelper

IPropertyArrayHelper & ORegistryFactoryHelper::getInfoHelper()
{
    ::osl::MutexGuard guard( aMutex );
    if (m_property_array_helper.get() == nullptr)
    {
        beans::Property prop(
            "ImplementationKey" /* name */,
            0 /* handle */,
            cppu::UnoType<decltype(xImplementationKey)>::get(),
            beans::PropertyAttribute::READONLY |
            beans::PropertyAttribute::OPTIONAL );
        m_property_array_helper.reset(
            new ::cppu::OPropertyArrayHelper( &prop, 1 ) );
    }
    return *m_property_array_helper.get();
}


sal_Bool ORegistryFactoryHelper::convertFastPropertyValue(
    Any &, Any &, sal_Int32, Any const & )
    throw (lang::IllegalArgumentException)
{
    OSL_FAIL( "unexpected!" );
    return false;
}


void ORegistryFactoryHelper::setFastPropertyValue_NoBroadcast(
    sal_Int32, Any const & )
    throw (Exception, std::exception)
{
    throw beans::PropertyVetoException(
        "unexpected: only readonly properties!",
        static_cast< OWeakObject * >(this) );
}


void ORegistryFactoryHelper::getFastPropertyValue(
    Any & rValue, sal_Int32 nHandle ) const
{
    if (nHandle == 0)
    {
        rValue <<= xImplementationKey;
    }
    else
    {
        rValue.clear();
        throw beans::UnknownPropertyException(
            "unknown property!", static_cast< OWeakObject * >(
                const_cast< ORegistryFactoryHelper * >(this) ) );
    }
}

Reference<XInterface > ORegistryFactoryHelper::createInstanceEveryTime(
    Reference< XComponentContext > const & xContext )
    throw(css::uno::Exception, css::uno::RuntimeException)
{
    if( !xModuleFactory.is() && !xModuleFactoryDepr.is() )
    {
        Reference< XInterface > x( createModuleFactory() );
        if (x.is())
        {
            MutexGuard aGuard( aMutex );
            if( !xModuleFactory.is() && !xModuleFactoryDepr.is() )
            {
                xModuleFactory.set( x, UNO_QUERY );
                xModuleFactoryDepr.set( x, UNO_QUERY );
            }
        }
    }
    if( xModuleFactory.is() )
    {
        return xModuleFactory->createInstanceWithContext( xContext );
    }
    else if( xModuleFactoryDepr.is() )
    {
        return xModuleFactoryDepr->createInstance();
    }

    return Reference<XInterface >();
}

Reference<XInterface > SAL_CALL ORegistryFactoryHelper::createInstanceWithArguments(
    const Sequence<Any>& Arguments )
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    if( !xModuleFactory.is() && !xModuleFactoryDepr.is() )
    {
        Reference< XInterface > x( createModuleFactory() );
        if (x.is())
        {
            MutexGuard aGuard( aMutex );
            if( !xModuleFactory.is() && !xModuleFactoryDepr.is() )
            {
                xModuleFactory.set( x, UNO_QUERY );
                xModuleFactoryDepr.set( x, UNO_QUERY );
            }
        }
    }
    if( xModuleFactoryDepr.is() )
    {
        return xModuleFactoryDepr->createInstanceWithArguments( Arguments );
    }
    else if( xModuleFactory.is() )
    {
        SAL_INFO("cppuhelper", "no context ORegistryFactoryHelper::createInstanceWithArgumentsAndContext()!");
        return xModuleFactory->createInstanceWithArgumentsAndContext( Arguments, Reference< XComponentContext >() );
    }

    return Reference<XInterface >();
}

Reference< XInterface > ORegistryFactoryHelper::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException, std::exception)
{
    if( !xModuleFactory.is() && !xModuleFactoryDepr.is() )
    {
        Reference< XInterface > x( createModuleFactory() );
        if (x.is())
        {
            MutexGuard aGuard( aMutex );
            if( !xModuleFactory.is() && !xModuleFactoryDepr.is() )
            {
                xModuleFactory.set( x, UNO_QUERY );
                xModuleFactoryDepr.set( x, UNO_QUERY );
            }
        }
    }
    if( xModuleFactory.is() )
    {
        return xModuleFactory->createInstanceWithArgumentsAndContext( rArguments, xContext );
    }
    else if( xModuleFactoryDepr.is() )
    {
        SAL_INFO_IF(xContext.is(), "cppuhelper", "ignoring context calling ORegistryFactoryHelper::createInstaceWithArgumentsAndContext()!");
        return xModuleFactoryDepr->createInstanceWithArguments( rArguments );
    }

    return Reference<XInterface >();
}


// OSingleFactoryHelper
Reference< XInterface > ORegistryFactoryHelper::createModuleFactory()
    throw(css::uno::Exception, css::uno::RuntimeException)
{
    OUString aActivatorUrl;
    OUString aActivatorName;
    OUString aLocation;

    Reference<XRegistryKey > xActivatorKey = xImplementationKey->openKey(
        "/UNO/ACTIVATOR" );
    if( xActivatorKey.is() && xActivatorKey->getValueType() == RegistryValueType_ASCII )
    {
        aActivatorUrl = xActivatorKey->getAsciiValue();

        OUString tmpActivator(aActivatorUrl.getStr());
        sal_Int32 nIndex = 0;
        aActivatorName = tmpActivator.getToken(0, ':', nIndex );

        Reference<XRegistryKey > xLocationKey = xImplementationKey->openKey(
            "/UNO/LOCATION" );
        if( xLocationKey.is() && xLocationKey->getValueType() == RegistryValueType_ASCII )
            aLocation = xLocationKey->getAsciiValue();
    }
    else
    {
        // old style"url"
        // the location of the program code of the implementation
        Reference<XRegistryKey > xLocationKey = xImplementationKey->openKey(
            "/UNO/URL" );
        // is the key of the right type ?
        if( xLocationKey.is() && xLocationKey->getValueType() == RegistryValueType_ASCII )
        {
            // one implementation found -> try to activate
            aLocation = xLocationKey->getAsciiValue();

            // search protocol delimiter
            sal_Int32 nPos = aLocation.indexOf("://");
            if( nPos != -1 )
            {
                aActivatorName = aLocation.copy( 0, nPos );
                if( aActivatorName == "java" )
                    aActivatorName = "com.sun.star.loader.Java";
                else if( aActivatorName == "module" )
                    aActivatorName = "com.sun.star.loader.SharedLibrary";
                aLocation = aLocation.copy( nPos + 3 );
            }
        }
    }

    Reference< XInterface > xFactory;
    if( !aActivatorName.isEmpty() )
    {
        Reference<XInterface > x = xSMgr->createInstance( aActivatorName );
        Reference<XImplementationLoader > xLoader( x, UNO_QUERY );
        Reference<XInterface > xMF;
        if (xLoader.is())
        {
            xFactory = xLoader->activate( aImplementationName, aActivatorUrl, aLocation, xImplementationKey );
        }
    }
    return xFactory;
}

// XServiceInfo
Sequence< OUString > ORegistryFactoryHelper::getSupportedServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    MutexGuard aGuard( aMutex );
    if( aServiceNames.getLength() == 0 )
    {
        // not yet loaded
        try
        {
            Reference<XRegistryKey > xKey = xImplementationKey->openKey( "UNO/SERVICES" );

            if (xKey.is())
            {
                // length of prefix. +1 for the '/' at the end
                sal_Int32 nPrefixLen = xKey->getKeyName().getLength() + 1;

                // Full qualified names like "IMPLEMENTATIONS/TEST/UNO/SERVICES/com.sun.star..."
                Sequence<OUString> seqKeys = xKey->getKeyNames();
                OUString* pKeys = seqKeys.getArray();
                for( sal_Int32 i = 0; i < seqKeys.getLength(); i++ )
                    pKeys[i] = pKeys[i].copy(nPrefixLen);

                aServiceNames = seqKeys;
            }
        }
        catch (InvalidRegistryException &)
        {
        }
    }
    return aServiceNames;
}

sal_Bool SAL_CALL ORegistryFactoryHelper::releaseOnNotification() throw(css::uno::RuntimeException, std::exception)
{
    bool retVal= true;
    if( isOneInstance() && isInstance())
    {
        retVal= false;
    }
    else if( ! isOneInstance())
    {
        // try to delegate
        if( xModuleFactory.is())
        {
            Reference<XUnloadingPreference> xunloading( xModuleFactory, UNO_QUERY);
            if( xunloading.is())
                retVal= xunloading->releaseOnNotification();
        }
        else if( xModuleFactoryDepr.is())
        {
            Reference<XUnloadingPreference> xunloading( xModuleFactoryDepr, UNO_QUERY);
            if( xunloading.is())
                retVal= xunloading->releaseOnNotification();
        }
    }
    return retVal;
}

class OFactoryProxyHelper : public WeakImplHelper< XServiceInfo, XSingleServiceFactory,
                                                    XUnloadingPreference >
{
    Reference<XSingleServiceFactory >   xFactory;

public:

    explicit OFactoryProxyHelper( const Reference<XSingleServiceFactory > & rFactory )
        : xFactory( rFactory )
        {}

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(css::uno::RuntimeException, std::exception) override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw(css::uno::RuntimeException, std::exception) override;
    //XUnloadingPreference
    sal_Bool SAL_CALL releaseOnNotification()
        throw(css::uno::RuntimeException, std::exception) override;

};

// XSingleServiceFactory
Reference<XInterface > OFactoryProxyHelper::createInstance()
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    return xFactory->createInstance();
}

// XSingleServiceFactory
Reference<XInterface > OFactoryProxyHelper::createInstanceWithArguments
(
    const Sequence<Any>& Arguments
)
    throw(css::uno::Exception, css::uno::RuntimeException, std::exception)
{
    return xFactory->createInstanceWithArguments( Arguments );
}

// XServiceInfo
OUString OFactoryProxyHelper::getImplementationName()
    throw(css::uno::RuntimeException, std::exception)
{
    Reference<XServiceInfo > xInfo( xFactory, UNO_QUERY  );
    if( xInfo.is() )
        return xInfo->getImplementationName();
    return OUString();
}

// XServiceInfo
sal_Bool OFactoryProxyHelper::supportsService(const OUString& ServiceName)
    throw(css::uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OFactoryProxyHelper::getSupportedServiceNames()
    throw(css::uno::RuntimeException, std::exception)
{
    Reference<XServiceInfo > xInfo( xFactory, UNO_QUERY  );
    if( xInfo.is() )
        return xInfo->getSupportedServiceNames();
    return Sequence< OUString >();
}

sal_Bool SAL_CALL OFactoryProxyHelper::releaseOnNotification() throw(css::uno::RuntimeException, std::exception)
{

    Reference<XUnloadingPreference> pref( xFactory, UNO_QUERY);
    if( pref.is())
        return pref->releaseOnNotification();
    return true;
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames,
    rtl_ModuleCount * )
{
    return new OFactoryComponentHelper(
        rServiceManager, rImplementationName, pCreateFunction, nullptr, &rServiceNames, false );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createFactoryProxy(
    SAL_UNUSED_PARAMETER const Reference<XMultiServiceFactory > &,
    const Reference<XSingleServiceFactory > & rFactory )
{
    return new OFactoryProxyHelper( rFactory );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames,
    rtl_ModuleCount * )
{
    return new OFactoryComponentHelper(
        rServiceManager, rImplementationName, pCreateFunction, nullptr, &rServiceNames, true );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
{
    return new ORegistryFactoryHelper(
        rServiceManager, rImplementationName, rImplementationKey, false );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
{
    return new ORegistryFactoryHelper(
        rServiceManager, rImplementationName, rImplementationKey, true );
}


Reference< lang::XSingleComponentFactory > SAL_CALL createSingleComponentFactory(
    ComponentFactoryFunc fptr,
    OUString const & rImplementationName,
    Sequence< OUString > const & rServiceNames,
    rtl_ModuleCount *)
{
    return new OFactoryComponentHelper(
        Reference< XMultiServiceFactory >(), rImplementationName, nullptr, fptr, &rServiceNames, false );
}

Reference< lang::XSingleComponentFactory > SAL_CALL createOneInstanceComponentFactory(
    ComponentFactoryFunc fptr,
    OUString const & rImplementationName,
    Sequence< OUString > const & rServiceNames,
    rtl_ModuleCount *)
{
    return new OFactoryComponentHelper(
        Reference< XMultiServiceFactory >(), rImplementationName, nullptr, fptr, &rServiceNames, true );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
