/*************************************************************************
 *
 *  $RCSfile: factory.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:52:00 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _RTL_UNLOAD_H_
#include <rtl/unload.h>
#endif


#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XUnloadingPreference.hpp>

using namespace osl;
using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::loader;
using namespace com::sun::star::registry;

namespace cppu
{

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
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
        SAL_THROW( () )
        : xSMgr( rServiceManager )
        , aImplementationName( rImplementationName_ )
        , pCreateFunction( pCreateFunction_ )
        , m_fptr( fptr )
        {
            if( pServiceNames_ )
                aServiceNames = *pServiceNames_;
        }

    // old function, only for backward compatibility
    OSingleFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_ )
        SAL_THROW( () )
        : xSMgr( rServiceManager )
        , aImplementationName( rImplementationName_ )
        , pCreateFunction( NULL )
        , m_fptr( 0 )
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
#if OSL_DEBUG_LEVEL > 1
        if (xContext.is())
        {
           OSL_TRACE( "### ignoring context calling OSingleFactoryHelper::createInstanceEveryTime()!\n" );
        }
#endif
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

    if (rArguments.getLength())
    {
        Reference< lang::XInitialization > xInit( xRet, UNO_QUERY );
        if (xInit.is())
        {
            xInit->initialize( rArguments );
        }
        else
        {
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
    Sequence< OUString > seqServices = getSupportedServiceNames();
    const OUString * pServices = seqServices.getConstArray();
    for( sal_Int32 i = 0; i < seqServices.getLength(); i++ )
        if( pServices[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > OSingleFactoryHelper::getSupportedServiceNames(void)
    throw(::com::sun::star::uno::RuntimeException)
{
    return aServiceNames;
}


//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
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
        SAL_THROW( () )
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
        SAL_THROW( () )
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
        SAL_THROW( () )
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
        Any ret;
        Reference<XUnloadingPreference> xpref( static_cast<XUnloadingPreference*>(this));
        ret<<= xpref;
        return ret;
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

Sequence< sal_Int8 > OFactoryComponentHelper::getImplementationId()
    throw (::com::sun::star::uno::RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
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


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
class ORegistryFactoryHelper
    : public OFactoryComponentHelper
{
public:
    ORegistryFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        const Reference<XRegistryKey > & xImplementationKey_,
        sal_Bool bOneInstance_ = sal_False )
        SAL_THROW( () )
        : OFactoryComponentHelper( rServiceManager, rImplementationName_, 0, 0, 0, bOneInstance_ )
        , xImplementationKey( xImplementationKey_ )
        {}

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
};

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
        OSL_TRACE( "### no context ORegistryFactoryHelper::createInstanceWithArgumentsAndContext()!\n" );
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
            OSL_TRACE( "### ignoring context calling ORegistryFactoryHelper::createInstanceWithArgumentsAndContext()!\n" );
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
    Reference< XInterface > xFactory;
    try
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

                // search protocol delemitter
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

        if( aActivatorName.getLength() != 0 )
        {
            Reference<XInterface > x = xSMgr->createInstance( aActivatorName );
            Reference<XImplementationLoader > xLoader( x, UNO_QUERY );
            Reference<XInterface > xMF;
            if (xLoader.is())
            {
                // JSC: This exception must not be catched if a concept for exception handling
                // is specified, implemented and used.
                try
                {
                    xFactory = xLoader->activate( aImplementationName, aActivatorUrl, aLocation, xImplementationKey );
                }
                catch( CannotActivateFactoryException& e)
                {
                    OString msg( OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US) );
                    OSL_TRACE(msg.getStr() );
                }
            }
        }
    }
    catch (InvalidRegistryException & e)
    {
        OString msg( OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US) );
        OSL_TRACE(msg.getStr() );
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#if defined( MACOSX ) && ( __GNUC__ < 3 )
// Initialize static template data here to break circular reference to libstatic
ClassData3 WeakImplHelper3< XServiceInfo, XSingleServiceFactory, XUnloadingPreference >::s_aCD(3);
#endif

class OFactoryProxyHelper : public WeakImplHelper3< XServiceInfo, XSingleServiceFactory,
                                                    XUnloadingPreference >
{
    Reference<XSingleServiceFactory >   xFactory;

public:

    OFactoryProxyHelper(
        const Reference<XMultiServiceFactory > & /*rServiceManager*/,
        const Reference<XSingleServiceFactory > & rFactory )
        SAL_THROW( () )
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
    if( xInfo.is() )
        return xInfo->supportsService( ServiceName );
    return sal_False;
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


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames,
    rtl_ModuleCount *pModCount )
    SAL_THROW( () )
{
    return new OFactoryComponentHelper(
        rServiceManager, rImplementationName, pCreateFunction, 0, &rServiceNames, pModCount, sal_False );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createFactoryProxy(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const Reference<XSingleServiceFactory > & rFactory )
    SAL_THROW( () )
{
    return new OFactoryProxyHelper(
        rServiceManager, rFactory );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames,
    rtl_ModuleCount *pModCount )
    SAL_THROW( () )
{
    return new OFactoryComponentHelper(
        rServiceManager, rImplementationName, pCreateFunction, 0, &rServiceNames, pModCount, sal_True );
//  return new OFactoryUnloadableComponentHelper(
//      rServiceManager, rImplementationName, pCreateFunction, 0, &rServiceNames, pModCount, sal_True );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
    SAL_THROW( () )
{
    return new ORegistryFactoryHelper(
        rServiceManager, rImplementationName, rImplementationKey, sal_False );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
    SAL_THROW( () )
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
    SAL_THROW( () )
{
    return new OFactoryComponentHelper(
        Reference< XMultiServiceFactory >(), rImplementationName, 0, fptr, &rServiceNames, pModCount, sal_False );
}

}


