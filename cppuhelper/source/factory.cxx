/*************************************************************************
 *
 *  $RCSfile: factory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-09 12:15:28 $
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
#ifndef _CPPUHELPER_IMPLBASE2_HXX
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/loader/XImplementationLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

using namespace osl;
using namespace rtl;
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
{
public:
    OSingleFactoryHelper(
        const Reference<XMultiServiceFactory > & rServiceManager,
        const OUString & rImplementationName_,
        ComponentInstantiation pCreateFunction_,
        const Sequence< OUString > * pServiceNames_ )
        SAL_THROW( () )
        : xSMgr( rServiceManager )
        , aImplementationName( rImplementationName_ )
        , pCreateFunction( pCreateFunction_ )
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
        {}

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    virtual Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

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
    virtual Reference<XInterface >  createInstanceEveryTime()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    Reference<XMultiServiceFactory > xSMgr;
    ComponentInstantiation           pCreateFunction;
    Sequence< OUString >             aServiceNames;
    OUString                         aImplementationName;
};

//-----------------------------------------------------------------------------
Any OSingleFactoryHelper::queryInterface( const Type & rType )
    throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::queryInterface(
        rType, static_cast< XSingleServiceFactory * >( this ), static_cast< XServiceInfo * >( this ) );
}

// OSingleFactoryHelper
Reference<XInterface > OSingleFactoryHelper::createInstanceEveryTime()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if( pCreateFunction )
        return pCreateFunction( xSMgr );
    else
        return Reference<XInterface >();
}

// XSingleServiceFactory
Reference<XInterface > OSingleFactoryHelper::createInstance()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    return createInstanceEveryTime();
}

// XSingleServiceFactory
Reference<XInterface > OSingleFactoryHelper::createInstanceWithArguments
(
    const Sequence<Any>& Arguments
)
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    Reference<XInterface > xRet = createInstanceEveryTime();

    if( Arguments.getLength() )
    {
        Reference<XInitialization > xInit( xRet, UNO_QUERY );
        if( xInit.is() )
            xInit->initialize( Arguments );
        else
            throw IllegalArgumentException();
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
sal_Bool OSingleFactoryHelper::supportsService(const OUString& ServiceName)
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
        const Sequence< OUString > * pServiceNames_,
        sal_Bool bOneInstance_ = sal_False )
        SAL_THROW( () )
        : OComponentHelper( aMutex )
        , OSingleFactoryHelper( rServiceManager, rImplementationName_, pCreateFunction_, pServiceNames_ )
        , bOneInstance( bOneInstance_ )
        {
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
        {
        }

    // XInterface
    Any SAL_CALL queryInterface( const Type & rType ) throw(::com::sun::star::uno::RuntimeException)
        { return OComponentHelper::queryInterface( rType ); }
    void SAL_CALL acquire() throw(::com::sun::star::uno::RuntimeException)
        { OComponentHelper::acquire(); }
    void SAL_CALL release() throw(::com::sun::star::uno::RuntimeException)
        { OComponentHelper::release(); }

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstance()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    Reference<XInterface > SAL_CALL createInstanceWithArguments( const Sequence<Any>& Arguments )
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XAggregation
    Any SAL_CALL queryAggregation( const Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

private:
    Reference<XInterface >  xTheInstance;
    sal_Bool                bOneInstance;
};

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
    static OTypeCollection * pTypes = 0;
    if (! pTypes)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pTypes)
        {
            static OTypeCollection aTypes(
                ::getCppuType( (const Reference< XSingleServiceFactory > *)0 ),
                ::getCppuType( (const Reference< XServiceInfo > *)0 ),
                OComponentHelper::getTypes() );
            pTypes = &aTypes;
        }
    }
    return pTypes->getTypes();
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
        MutexGuard aGuard( aMutex );
        if( !xTheInstance.is() )
            xTheInstance = OSingleFactoryHelper::createInstance();
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
        MutexGuard aGuard( aMutex );
        if( !xTheInstance.is() )
            xTheInstance = OSingleFactoryHelper::createInstanceWithArguments( Arguments );
        return xTheInstance;
    }
    return OSingleFactoryHelper::createInstanceWithArguments( Arguments );
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
        : OFactoryComponentHelper( rServiceManager, rImplementationName_, 0, 0, bOneInstance_ )
        , xImplementationKey( xImplementationKey_ )
        {}

    // OSingleFactoryHelper
    Reference<XInterface > createInstanceEveryTime()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XSingleServiceFactory
    Reference<XInterface > SAL_CALL createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    Sequence< OUString > SAL_CALL getSupportedServiceNames(void)
        throw(::com::sun::star::uno::RuntimeException);

private:
    Reference< XSingleServiceFactory > createModuleFactory()
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    /** The registry key of the implementation section */
    Reference<XRegistryKey >    xImplementationKey;
    /** The factory created with the loader. */
    Reference<XSingleServiceFactory >   xModuleFactory;
};

Reference<XInterface > ORegistryFactoryHelper::createInstanceEveryTime()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if( pCreateFunction )
        // the create function is set, do not use the implementation loader.
        return OFactoryComponentHelper::createInstanceEveryTime();

    if( !xModuleFactory.is() )
    {
        MutexGuard aGuard( aMutex );
        if( !xModuleFactory.is() )      // is xModuleFactory really not set (MultiThreaded!)
        {
            xModuleFactory = createModuleFactory();
        }
    }
    if( xModuleFactory.is() )
        return xModuleFactory->createInstance();

    return Reference<XInterface >();
}

// OSingleFactoryHelper
Reference< XSingleServiceFactory > ORegistryFactoryHelper::createModuleFactory()
    throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    Reference<XSingleServiceFactory >   xFactory;
    try
    {
        MutexGuard aGuard( aMutex );

        OUString aActivatorUrl;
        OUString aActivatorName;
        OUString aLocation;

        Reference<XRegistryKey > xActivatorKey = xImplementationKey->openKey(
            OUString( RTL_CONSTASCII_USTRINGPARAM("/UNO/ACTIVATOR") ) );
        if( xActivatorKey.is() && xActivatorKey->getValueType() == RegistryValueType_ASCII )
        {
            aActivatorUrl = xActivatorKey->getAsciiValue();

            OUString tmpActivator(aActivatorUrl.getStr());
            aActivatorName = tmpActivator.getToken(0, L':').getStr();

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
                sal_Int32 nPos = aLocation.search(
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

        if( aActivatorName.len() != 0 )
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
                    xMF = xLoader->activate( aImplementationName, aActivatorUrl, aLocation, xImplementationKey );
                }
                catch( CannotActivateFactoryException& e)
                {
                    OString msg( OUStringToOString(e.Message, RTL_TEXTENCODING_ASCII_US) );
                    OSL_ENSHURE( !msg.getLength(), msg.getStr() );
                }
            }
            xFactory = Reference<XSingleServiceFactory >( xMF, UNO_QUERY );
        }
    }
    catch (InvalidRegistryException &)
    {
    }

    return xFactory;
}

Reference<XInterface > SAL_CALL ORegistryFactoryHelper::createInstanceWithArguments(const Sequence<Any>& Arguments)
        throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    if( !xModuleFactory.is() )
    {
        MutexGuard aGuard( aMutex );
        if( !xModuleFactory.is() )   // MultiThreaded!
        {
            xModuleFactory = createModuleFactory();
        }
    }

    if( xModuleFactory.is() )
        return xModuleFactory->createInstanceWithArguments( Arguments );

    return Reference<XInterface >();
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
                sal_Int32 nPrefixLen = xKey->getKeyName().len() + 1;

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


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------


class OFactoryProxyHelper : public WeakImplHelper2< XServiceInfo, XSingleServiceFactory >
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

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames )
    SAL_THROW( () )
{
    return new OFactoryComponentHelper( rServiceManager, rImplementationName,
                                        pCreateFunction, &rServiceNames, sal_False );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createFactoryProxy(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const Reference<XSingleServiceFactory > & rFactory )
    SAL_THROW( () )
{
    return new OFactoryProxyHelper( rServiceManager, rFactory );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    ComponentInstantiation pCreateFunction,
    const Sequence< OUString > & rServiceNames )
    SAL_THROW( () )
{
    return new OFactoryComponentHelper( rServiceManager, rImplementationName,
                                        pCreateFunction, &rServiceNames, sal_True );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createSingleRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
    SAL_THROW( () )
{
    return new ORegistryFactoryHelper( rServiceManager, rImplementationName,
                                       rImplementationKey, sal_False );
}

// global function
Reference<XSingleServiceFactory > SAL_CALL createOneInstanceRegistryFactory(
    const Reference<XMultiServiceFactory > & rServiceManager,
    const OUString & rImplementationName,
    const Reference<XRegistryKey > & rImplementationKey )
    SAL_THROW( () )
{
    return new ORegistryFactoryHelper( rServiceManager, rImplementationName,
                                       rImplementationKey, sal_True );
}

}


