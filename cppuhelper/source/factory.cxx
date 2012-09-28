/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/component.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/shlib.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <rtl/instance.hxx>
#include <rtl/unload.h>

#include "cppuhelper/propshlp.hxx"

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>
#include "com/sun/star/beans/PropertyAttribute.hpp"

#include <memory>

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )


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
        SAL_THROW(())
        : xSMgr( rServiceManager )
        , pCreateFunction( pCreateFunction_ )
        , m_fptr( fptr )
        , aImplementationName( rImplementationName_ )
        {
            if( pServiceNames_ )
                aServiceNames = *pServiceNames_;
        }

    // old function, only for backward compatibility
    OSingleFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_ )
        SAL_THROW(())
        : xSMgr( rServiceManager )
        , pCreateFunction( NULL )
        , m_fptr( 0 )
        , aImplementationName( rImplementationName_ )
        {}

    virtual ~OSingleFactoryHelper();

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XSingleComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException);
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException);

protected:
    /**
     * Create an instance specified by the factory. The one instance logic is implemented
     * in the createInstance and createInstanceWithArguments methods.
     * @return the newly created instance. Do not return a previous (one instance) instance.
     */
    virtual Reference<XInterface >  createInstanceEveryTime(
        Reference< XComponentContext > const & xContext )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    Reference<XMultiServiceFactory > xSMgr;
    ComponentInstantiation           pCreateFunction;
    ComponentFactoryFunc             m_fptr;
    Sequence< OUString >             aServiceNames;
    OUString                         aImplementationName;
};
OSingleFactoryHelper::~OSingleFactoryHelper()
{
}


//-----------------------------------------------------------------------------
Any OSingleFactoryHelper::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
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
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
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
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceWithContext( Reference< XComponentContext >() );
}

// XSingleServiceFactory
Reference<XInterface > OSingleFactoryHelper::createInstanceWithArguments(
    const Sequence<Any>& Arguments )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceWithArgumentsAndContext(
        Arguments, Reference< XComponentContext >() );
}

// XSingleComponentFactory
//__________________________________________________________________________________________________
Reference< XInterface > OSingleFactoryHelper::createInstanceWithContext(
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
{
    return createInstanceEveryTime( xContext );
}
//__________________________________________________________________________________________________
Reference< XInterface > OSingleFactoryHelper::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
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
                OUString( RTL_CONSTASCII_USTRINGPARAM("cannot pass arguments to component => no XInitialization implemented!") ),
                Reference< XInterface >(), 0 );
        }
    }

    return xRet;
}

// XServiceInfo
OUString OSingleFactoryHelper::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    return aImplementationName;
}

// XServiceInfo
sal_Bool OSingleFactoryHelper::supportsService(
    const OUString& ServiceName )
    throw(::com::sun::star::uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

// XServiceInfo
Sequence< OUString > OSingleFactoryHelper::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
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
        sal_Bool bOneInstance_ = sal_False )
        SAL_THROW(())
        : OComponentHelper( aMutex )
        , OSingleFactoryHelper( rServiceManager, rImplementationName_, pCreateFunction_, fptr, pServiceNames_ )
        , bOneInstance( bOneInstance_ )
        , pModuleCount(0)
        {
        }

    // Used by the createXXXFactory functions. The argument pModCount is used to  prevent the unloading of the module
    // which contains pCreateFunction_
    OFactoryComponentHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        ComponentInstantiation pCreateFunction_,
        ComponentFactoryFunc fptr,
        const Sequence< OUString > * pServiceNames_,
        rtl_ModuleCount * pModCount,
        sal_Bool bOneInstance_ = sal_False )
        SAL_THROW(())
        : OComponentHelper( aMutex )
        , OSingleFactoryHelper( rServiceManager, rImplementationName_, pCreateFunction_, fptr, pServiceNames_ )
        , bOneInstance( bOneInstance_ )
        , pModuleCount(pModCount)
        {
            if(pModuleCount)
                pModuleCount->acquire( pModuleCount);
        }

    // old function, only for backward compatibility
    OFactoryComponentHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        sal_Bool bOneInstance_ = sal_False )
        SAL_THROW(())
        : OComponentHelper( aMutex )
        , OSingleFactoryHelper( rServiceManager, rImplementationName_ )
        , bOneInstance( bOneInstance_ )
        , pModuleCount(0)
        {
        }

    ~OFactoryComponentHelper()
    {
        if(pModuleCount)
            pModuleCount->release( pModuleCount);
    }

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType )
        throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL acquire() throw()
        { OComponentHelper::acquire(); }
    void SAL_CALL release() throw()
        { OComponentHelper::release(); }

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    Reference<XInterface > SAL_CALL createInstanceWithArguments( const Sequence<Any>& Arguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XSingleComponentFactory
    virtual Reference< XInterface > SAL_CALL createInstanceWithContext(
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XAggregation
    Any SAL_CALL queryAggregation( const Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    // XUnloadingPreference
    virtual sal_Bool SAL_CALL releaseOnNotification()
        throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

private:
    Reference<XInterface >  xTheInstance;
    sal_Bool                bOneInstance;
    rtl_ModuleCount *       pModuleCount;
protected:
    // needed for implementing XUnloadingPreference in inheriting classes
    sal_Bool isOneInstance() {return bOneInstance;}
    sal_Bool isInstance() {return xTheInstance.is();}
};


Any SAL_CALL OFactoryComponentHelper::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    if( rType == ::getCppuType( (Reference<XUnloadingPreference>*)0))
    {
        return makeAny(
            Reference< XUnloadingPreference >(
                static_cast< XUnloadingPreference * >(this) ) );
    }
    return OComponentHelper::queryInterface( rType );
}

// XAggregation
Any OFactoryComponentHelper::queryAggregation( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    Any aRet( OComponentHelper::queryAggregation( rType ) );
    return (aRet.hasValue() ? aRet : OSingleFactoryHelper::queryInterface( rType ));
}

// XTypeProvider
Sequence< Type > OFactoryComponentHelper::getTypes()
    throw (::com::sun::star::uno::RuntimeException)
{
    Type ar[ 4 ];
    ar[ 0 ] = ::getCppuType( (const Reference< XSingleServiceFactory > *)0 );
    ar[ 1 ] = ::getCppuType( (const Reference< XServiceInfo > *)0 );
    ar[ 2 ] = ::getCppuType( (const Reference< XUnloadingPreference > *)0 );

    if (m_fptr)
        ar[ 3 ] = ::getCppuType( (const Reference< XSingleComponentFactory > *)0 );

    return Sequence< Type >( ar, m_fptr ? 4 : 3 );
}

namespace
{
    class theOFactoryComponentHelperImplementationId :
        public rtl::Static<OImplementationId, theOFactoryComponentHelperImplementationId>{};
}

Sequence< sal_Int8 > OFactoryComponentHelper::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    return theOFactoryComponentHelperImplementationId::get().getImplementationId();
}

// XSingleServiceFactory
Reference<XInterface > OFactoryComponentHelper::createInstance()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
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
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
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
//__________________________________________________________________________________________________
Reference< XInterface > OFactoryComponentHelper::createInstanceWithContext(
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
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
//__________________________________________________________________________________________________
Reference< XInterface > OFactoryComponentHelper::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
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
    throw(::com::sun::star::uno::RuntimeException)
{
    OComponentHelper::dispose();

    Reference<XInterface > x;
    {
        // do not delete in the guard section
        MutexGuard aGuard( aMutex );
        x = xTheInstance;
        xTheInstance = Reference<XInterface >();
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
sal_Bool SAL_CALL OFactoryComponentHelper::releaseOnNotification() throw(::com::sun::star::uno::RuntimeException)
{
    if( bOneInstance)
        return sal_False;
    return sal_True;
}

class ORegistryFactoryHelper : public OFactoryComponentHelper,
                               public OPropertySetHelper

{
public:
    ORegistryFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        const Reference<XRegistryKey > & xImplementationKey_,
        sal_Bool bOneInstance_ = sal_False ) SAL_THROW(())
            : OFactoryComponentHelper(
                rServiceManager, rImplementationName_, 0, 0, 0, bOneInstance_ ),
              OPropertySetHelper( OComponentHelper::rBHelper ),
              xImplementationKey( xImplementationKey_ )
        {}

    // XInterface
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();
    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes()
        throw (RuntimeException);
    // XPropertySet
    virtual Reference< beans::XPropertySetInfo > SAL_CALL getPropertySetInfo()
        throw (RuntimeException);

    // OPropertySetHelper
    virtual IPropertyArrayHelper & SAL_CALL getInfoHelper();
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        Any & rConvertedValue, Any & rOldValue,
        sal_Int32 nHandle, Any const & rValue )
        throw (lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle, Any const & rValue )
        throw (Exception);
    using OPropertySetHelper::getFastPropertyValue;
    virtual void SAL_CALL getFastPropertyValue(
        Any & rValue, sal_Int32 nHandle ) const;

    // OSingleFactoryHelper
    Reference<XInterface > createInstanceEveryTime(
        Reference< XComponentContext > const & xContext )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    // XSingleComponentFactory
    Reference< XInterface > SAL_CALL createInstanceWithArgumentsAndContext(
        Sequence< Any > const & rArguments,
        Reference< XComponentContext > const & xContext )
        throw (Exception, RuntimeException);

    // XServiceInfo
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException);
    // XUnloadingPreference
    sal_Bool SAL_CALL releaseOnNotification()
        throw( RuntimeException);


private:
    Reference< XInterface > createModuleFactory()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    /** The registry key of the implementation section */
    Reference<XRegistryKey >    xImplementationKey;
    /** The factory created with the loader. */
    Reference<XSingleComponentFactory > xModuleFactory;
    Reference<XSingleServiceFactory >   xModuleFactoryDepr;
    Reference< beans::XPropertySetInfo > m_xInfo;
    ::std::auto_ptr< IPropertyArrayHelper > m_property_array_helper;
protected:
    using OPropertySetHelper::getTypes;
};

// XInterface
//______________________________________________________________________________
Any SAL_CALL ORegistryFactoryHelper::queryInterface(
    Type const & type ) throw (RuntimeException)
{
    Any ret( OFactoryComponentHelper::queryInterface( type ) );
    if (ret.hasValue())
        return ret;
    else
        return OPropertySetHelper::queryInterface( type );
}

//______________________________________________________________________________
void ORegistryFactoryHelper::acquire() throw ()
{
    OFactoryComponentHelper::acquire();
}

//______________________________________________________________________________
void ORegistryFactoryHelper::release() throw ()
{
    OFactoryComponentHelper::release();
}

// XTypeProvider
//______________________________________________________________________________
Sequence< Type > ORegistryFactoryHelper::getTypes() throw (RuntimeException)
{
    Sequence< Type > types( OFactoryComponentHelper::getTypes() );
    sal_Int32 pos = types.getLength();
    types.realloc( pos + 3 );
    Type * p = types.getArray();
    p[ pos++ ] = ::getCppuType(
        reinterpret_cast< Reference< beans::XMultiPropertySet > const * >(0) );
    p[ pos++ ] = ::getCppuType(
        reinterpret_cast< Reference< beans::XFastPropertySet > const * >(0) );
    p[ pos++ ] = ::getCppuType(
        reinterpret_cast< Reference< beans::XPropertySet > const * >(0) );
    return types;
}

// XPropertySet
//______________________________________________________________________________
Reference< beans::XPropertySetInfo >
ORegistryFactoryHelper::getPropertySetInfo() throw (RuntimeException)
{
    ::osl::MutexGuard guard( aMutex );
    if (! m_xInfo.is())
        m_xInfo = createPropertySetInfo( getInfoHelper() );
    return m_xInfo;
}

// OPropertySetHelper
//______________________________________________________________________________
IPropertyArrayHelper & ORegistryFactoryHelper::getInfoHelper()
{
    ::osl::MutexGuard guard( aMutex );
    if (m_property_array_helper.get() == 0)
    {
        beans::Property prop(
            OUSTR("ImplementationKey") /* name */,
            0 /* handle */,
            ::getCppuType( &xImplementationKey ),
            beans::PropertyAttribute::READONLY |
            beans::PropertyAttribute::OPTIONAL );
        m_property_array_helper.reset(
            new ::cppu::OPropertyArrayHelper( &prop, 1 ) );
    }
    return *m_property_array_helper.get();
}

//______________________________________________________________________________
sal_Bool ORegistryFactoryHelper::convertFastPropertyValue(
    Any &, Any &, sal_Int32, Any const & )
    throw (lang::IllegalArgumentException)
{
    OSL_FAIL( "unexpected!" );
    return false;
}

//______________________________________________________________________________
void ORegistryFactoryHelper::setFastPropertyValue_NoBroadcast(
    sal_Int32, Any const & )
    throw (Exception)
{
    throw beans::PropertyVetoException(
        OUSTR("unexpected: only readonly properties!"),
        static_cast< OWeakObject * >(this) );
}

//______________________________________________________________________________
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
            OUSTR("unknown property!"), static_cast< OWeakObject * >(
                const_cast< ORegistryFactoryHelper * >(this) ) );
    }
}

Reference<XInterface > ORegistryFactoryHelper::createInstanceEveryTime(
    Reference< XComponentContext > const & xContext )
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
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
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
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
#if OSL_DEBUG_LEVEL > 1
        OSL_TRACE( "### no context ORegistryFactoryHelper::createInstanceWithArgumentsAndContext()!" );
#endif
        return xModuleFactory->createInstanceWithArgumentsAndContext( Arguments, Reference< XComponentContext >() );
    }

    return Reference<XInterface >();
}

Reference< XInterface > ORegistryFactoryHelper::createInstanceWithArgumentsAndContext(
    Sequence< Any > const & rArguments,
    Reference< XComponentContext > const & xContext )
    throw (Exception, RuntimeException)
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
#if OSL_DEBUG_LEVEL > 1
        if (xContext.is())
        {
            OSL_TRACE( "### ignoring context calling ORegistryFactoryHelper::createInstanceWithArgumentsAndContext()!" );
        }
#endif
        return xModuleFactoryDepr->createInstanceWithArguments( rArguments );
    }

    return Reference<XInterface >();
}


// OSingleFactoryHelper
Reference< XInterface > ORegistryFactoryHelper::createModuleFactory()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    OUString aActivatorUrl;
    OUString aActivatorName;
    OUString aLocation;

    Reference<XRegistryKey > xActivatorKey = xImplementationKey->openKey(
        OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/ACTIVATOR") ) );
    if( xActivatorKey.is() && xActivatorKey->getValueType() == RegistryValueType_ASCII )
    {
        aActivatorUrl = xActivatorKey->getAsciiValue();

        OUString tmpActivator(aActivatorUrl.getStr());
        sal_Int32 nIndex = 0;
        aActivatorName = tmpActivator.getToken(0, ':', nIndex );

        Reference<XRegistryKey > xLocationKey = xImplementationKey->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/LOCATION") ) );
        if( xLocationKey.is() && xLocationKey->getValueType() == RegistryValueType_ASCII )
            aLocation = xLocationKey->getAsciiValue();
    }
    else
    {
        // old style"url"
        // the location of the program code of the implementation
        Reference<XRegistryKey > xLocationKey = xImplementationKey->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/URL") ) );
        // is the the key of the right type ?
        if( xLocationKey.is() && xLocationKey->getValueType() == RegistryValueType_ASCII )
        {
            // one implementation found -> try to activate
            aLocation = xLocationKey->getAsciiValue();

            // search protocol delimiter
            sal_Int32 nPos = aLocation.indexOf(
                OUString( RTL_CONSTASCII_USTRINGPARAM("://") ) );
            if( nPos != -1 )
            {
                aActivatorName = aLocation.copy( 0, nPos );
                if( aActivatorName.compareToAscii( "java" ) == 0 )
                    aActivatorName = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.Java") );
                else if( aActivatorName.compareToAscii( "module" ) == 0 )
                    aActivatorName = OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.loader.SharedLibrary") );
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
Sequence< OUString > ORegistryFactoryHelper::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
{
    MutexGuard aGuard( aMutex );
    if( aServiceNames.getLength() == 0 )
    {
        // not yet loaded
        try
        {
            Reference<XRegistryKey > xKey = xImplementationKey->openKey(
                OUString( RTL_CONSTASCII_USTRINGPARAM("UNO/SERVICES") ) );

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

sal_Bool SAL_CALL ORegistryFactoryHelper::releaseOnNotification() throw(::com::sun::star::uno::RuntimeException)
{
    sal_Bool retVal= sal_True;
    if( isOneInstance() && isInstance())
    {
        retVal= sal_False;
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

class OFactoryProxyHelper : public WeakImplHelper3< XServiceInfo, XSingleServiceFactory,
                                                    XUnloadingPreference >
{
    Reference<XSingleServiceFactory >   xFactory;

public:

    OFactoryProxyHelper( const Reference<XSingleServiceFactory > & rFactory )
        SAL_THROW(())
        : xFactory( rFactory )
        {}

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName)
        throw(::com::sun::star::uno::RuntimeException);
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException);
    //XUnloadingPreference
    sal_Bool SAL_CALL releaseOnNotification()
        throw(::com::sun::star::uno::RuntimeException);

};

// XSingleServiceFactory
Reference<XInterface > OFactoryProxyHelper::createInstance()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return xFactory->createInstance();
}

// XSingleServiceFactory
Reference<XInterface > OFactoryProxyHelper::createInstanceWithArguments
(
    const Sequence<Any>& Arguments
)
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return xFactory->createInstanceWithArguments( Arguments );
}

// XServiceInfo
OUString OFactoryProxyHelper::getImplementationName()
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XServiceInfo > xInfo( xFactory, UNO_QUERY  );
    if( xInfo.is() )
        return xInfo->getImplementationName();
    return OUString();
}

// XServiceInfo
sal_Bool OFactoryProxyHelper::supportsService(const OUString& ServiceName)
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XServiceInfo > xInfo( xFactory, UNO_QUERY  );
    return xInfo.is() && xInfo->supportsService( ServiceName );
}

// XServiceInfo
Sequence< OUString > OFactoryProxyHelper::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XServiceInfo > xInfo( xFactory, UNO_QUERY  );
    if( xInfo.is() )
        return xInfo->getSupportedServiceNames();
    return Sequence< OUString >();
}

sal_Bool SAL_CALL OFactoryProxyHelper::releaseOnNotification() throw(::com::sun::star::uno::RuntimeException)
{

    Reference<XUnloadingPreference> pref( xFactory, UNO_QUERY);
    if( pref.is())
        return pref->releaseOnNotification();
    return sal_True;
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames,
    rtl_ModuleCount *pModCount )
    SAL_THROW(())
{
    return new OFactoryComponentHelper(
        rServiceManager, rImplementationName, pCreateFunction, 0, &rServiceNames, pModCount, sal_False );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createFactoryProxy(
    SAL_UNUSED_PARAMETER const Reference<XMultiServiceFactory > &,
    const Reference<XSingleServiceFactory > & rFactory )
    SAL_THROW(())
{
    return new OFactoryProxyHelper( rFactory );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames,
    rtl_ModuleCount *pModCount )
    SAL_THROW(())
{
    return new OFactoryComponentHelper(
        rServiceManager, rImplementationName, pCreateFunction, 0, &rServiceNames, pModCount, sal_True );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
    SAL_THROW(())
{
    return new ORegistryFactoryHelper(
        rServiceManager, rImplementationName, rImplementationKey, sal_False );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
    SAL_THROW(())
{
    return new ORegistryFactoryHelper(
        rServiceManager, rImplementationName, rImplementationKey, sal_True );
}

//##################################################################################################
Reference< lang::XSingleComponentFactory > SAL_CALL createSingleComponentFactory(
    ComponentFactoryFunc fptr,
    OUString const & rImplementationName,
    Sequence< OUString > const & rServiceNames,
    rtl_ModuleCount * pModCount)
    SAL_THROW(())
{
    return new OFactoryComponentHelper(
        Reference< XMultiServiceFactory >(), rImplementationName, 0, fptr, &rServiceNames, pModCount, sal_False );
}

Reference< lang::XSingleComponentFactory > SAL_CALL createOneInstanceComponentFactory(
    ComponentFactoryFunc fptr,
    OUString const & rImplementationName,
    Sequence< OUString > const & rServiceNames,
    rtl_ModuleCount * pModCount)
    SAL_THROW(())
{
    return new OFactoryComponentHelper(
        Reference< XMultiServiceFactory >(), rImplementationName, 0, fptr, &rServiceNames, pModCount, sal_True );
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
