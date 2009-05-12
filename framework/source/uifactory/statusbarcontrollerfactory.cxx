/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: statusbarcontrollerfactory.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include "uifactory/statusbarcontrollerfactory.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

namespace framework
{

// global function needed by both implementations
static rtl::OUString getHashKeyFromStrings( const rtl::OUString& aCommandURL, const rtl::OUString& aModuleName )
{
    rtl::OUStringBuffer aKey( aCommandURL );
    aKey.appendAscii( "-" );
    aKey.append( aModuleName );
    return aKey.makeStringAndClear();
}


//*****************************************************************************************************************
//  Configuration access class for StatusbarControllerFactory implementation
//*****************************************************************************************************************

class ConfigurationAccess_StatusbarControllerFactory : // interfaces
                                                       public  XTypeProvider                            ,
                                                       public  XContainerListener                       ,
                                                       // baseclasses
                                                       // Order is neccessary for right initialization!
                                                       private ThreadHelpBase                           ,
                                                       public  ::cppu::OWeakObject
{
    public:
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

                      ConfigurationAccess_StatusbarControllerFactory( Reference< XMultiServiceFactory >& rServiceManager );
        virtual       ~ConfigurationAccess_StatusbarControllerFactory();

        void          readConfigurationData();

        rtl::OUString getServiceFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const;
        rtl::OUString getValueFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const;
        void          addServiceToCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule, const rtl::OUString& rServiceSpecifier );
        void          removeServiceFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule );

        // container.XContainerListener
        virtual void SAL_CALL elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException);
        virtual void SAL_CALL elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException);

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) throw(RuntimeException);

    private:
        struct StatusbarControllerInfo
        {
            rtl::OUString m_aImplementationName;
            rtl::OUString m_aValue;
        };

        class StatusbarControllerMap : public std::hash_map< rtl::OUString,
                                                             StatusbarControllerInfo,
                                                             OUStringHashCode,
                                                             ::std::equal_to< ::rtl::OUString > >
        {
            inline void free()
            {
                StatusbarControllerMap().swap( *this );
            }
        };

        sal_Bool impl_getElementProps( const Any& aElement,
                                       rtl::OUString& aCommand,
                                       rtl::OUString& aModule,
                                       rtl::OUString& aServiceSpecifier,
                                       rtl::OUString& aValue ) const;

        rtl::OUString                     m_aPropCommand;
        rtl::OUString                     m_aPropModule;
        rtl::OUString                     m_aPropController;
        rtl::OUString                     m_aPropValue;
        StatusbarControllerMap            m_aStatusbarControllerMap;
        Reference< XMultiServiceFactory > m_xServiceManager;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        sal_Bool                          m_bConfigAccessInitialized;
};

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_3     (   ConfigurationAccess_StatusbarControllerFactory                      ,
                            OWeakObject                                                         ,
                            DIRECT_INTERFACE ( XTypeProvider                                    ),
                            DIRECT_INTERFACE ( XContainerListener                               ),
                            DERIVED_INTERFACE( css::lang::XEventListener, XContainerListener    )
                        )

DEFINE_XTYPEPROVIDER_3  (   ConfigurationAccess_StatusbarControllerFactory  ,
                            XTypeProvider                                   ,
                            XContainerListener                              ,
                            css::lang::XEventListener
                        )

ConfigurationAccess_StatusbarControllerFactory::ConfigurationAccess_StatusbarControllerFactory( Reference< XMultiServiceFactory >& rServiceManager ) :
    ThreadHelpBase(),
    m_aPropCommand( RTL_CONSTASCII_USTRINGPARAM( "Command" )),
    m_aPropModule( RTL_CONSTASCII_USTRINGPARAM( "Module" )),
    m_aPropController( RTL_CONSTASCII_USTRINGPARAM( "Controller" )),
    m_aPropValue( RTL_CONSTASCII_USTRINGPARAM( "Value" )),
    m_xServiceManager( rServiceManager ),
    m_bConfigAccessInitialized( sal_False )
{
    m_xConfigProvider = Reference< XMultiServiceFactory >( rServiceManager->createInstance(
                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                    "com.sun.star.configuration.ConfigurationProvider" ))),
                                                           UNO_QUERY );
}

ConfigurationAccess_StatusbarControllerFactory::~ConfigurationAccess_StatusbarControllerFactory()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener( this );
}

rtl::OUString ConfigurationAccess_StatusbarControllerFactory::getServiceFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    StatusbarControllerMap::const_iterator pIter =
        m_aStatusbarControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aStatusbarControllerMap.end() )
        return pIter->second.m_aImplementationName;
    else
    {
        // Try to detect if we have a generic controller
        pIter = m_aStatusbarControllerMap.find( getHashKeyFromStrings( rCommandURL, rtl::OUString() ));

        if ( pIter != m_aStatusbarControllerMap.end() )
            return pIter->second.m_aImplementationName;
    }

    return rtl::OUString();
}

rtl::OUString ConfigurationAccess_StatusbarControllerFactory::getValueFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    StatusbarControllerMap::const_iterator pIter =
        m_aStatusbarControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aStatusbarControllerMap.end() )
        return pIter->second.m_aValue;
    else
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aStatusbarControllerMap.find( getHashKeyFromStrings( rCommandURL, rtl::OUString() ));

        if ( pIter != m_aStatusbarControllerMap.end() )
            return pIter->second.m_aValue;
    }

    return rtl::OUString();
}

void ConfigurationAccess_StatusbarControllerFactory::addServiceToCommandModule(
    const rtl::OUString& rCommandURL,
    const rtl::OUString& rModule,
    const rtl::OUString& rServiceSpecifier )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    rtl::OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );

    StatusbarControllerInfo& rControllerInfo = m_aStatusbarControllerMap[ aHashKey ];
    rControllerInfo.m_aImplementationName = rServiceSpecifier;
    rControllerInfo.m_aValue = rtl::OUString();
}

void ConfigurationAccess_StatusbarControllerFactory::removeServiceFromCommandModule(
    const rtl::OUString& rCommandURL,
    const rtl::OUString& rModule )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    rtl::OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aStatusbarControllerMap.erase( aHashKey );
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_StatusbarControllerFactory::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    rtl::OUString   aCommand;
    rtl::OUString   aModule;
    rtl::OUString   aService;
    rtl::OUString   aValue;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aCommand, aModule, aService, aValue ))
    {
        // Create hash key from command and module as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        rtl::OUString aHashKey( getHashKeyFromStrings( aCommand, aModule ));
        StatusbarControllerInfo& rControllerInfo = m_aStatusbarControllerMap[ aHashKey ];
        rControllerInfo.m_aImplementationName = aService;
        rControllerInfo.m_aValue = aValue;
    }
}

void SAL_CALL ConfigurationAccess_StatusbarControllerFactory::elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    rtl::OUString   aCommand;
    rtl::OUString   aModule;
    rtl::OUString   aService;
    rtl::OUString   aValue;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aCommand, aModule, aService, aValue ))
    {
        // Create hash key from command and module as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        rtl::OUString aHashKey( getHashKeyFromStrings( aCommand, aModule ));
        m_aStatusbarControllerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_StatusbarControllerFactory::elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    rtl::OUString   aCommand;
    rtl::OUString   aModule;
    rtl::OUString   aService;
    rtl::OUString   aValue;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aCommand, aModule, aService, aValue ))
    {
        // Create hash key from command and module as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        rtl::OUString aHashKey( getHashKeyFromStrings( aCommand, aModule ));
        StatusbarControllerInfo& rControllerInfo = m_aStatusbarControllerMap[ aHashKey ];
        rControllerInfo.m_aImplementationName = aService;
        rControllerInfo.m_aValue = aValue;
    }
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_StatusbarControllerFactory::disposing( const EventObject& ) throw(RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();
}

void ConfigurationAccess_StatusbarControllerFactory::readConfigurationData()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        Sequence< Any > aArgs( 1 );
        PropertyValue   aPropValue;

        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value <<= rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Controller/Registered/StatusBar" ));
        aArgs[0] <<= aPropValue;

        try
        {
            m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                                                                                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                                                                    "com.sun.star.configuration.ConfigurationAccess" )),
                                                                                aArgs ),
                                                                            UNO_QUERY );
        }
        catch ( WrappedTargetException& )
        {
        }

        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
    {
        Sequence< rtl::OUString >   aStatusbarControllers = m_xConfigAccess->getElementNames();

        Any a;
        rtl::OUString             aCommand;
        rtl::OUString             aModule;
        rtl::OUString             aService;
        rtl::OUString             aValue;
        rtl::OUString             aHashKey;
        Reference< XPropertySet > xPropertySet;
        for ( sal_Int32 i = 0; i < aStatusbarControllers.getLength(); i++ )
        {
            if ( impl_getElementProps( m_xConfigAccess->getByName( aStatusbarControllers[i] ), aCommand, aModule, aService, aValue ))
            {
                // Create hash key from command and module as they are together a primary key to
                // the UNO service that implements the popup menu controller.
                aHashKey = getHashKeyFromStrings( aCommand, aModule );
                StatusbarControllerInfo& rControllerInfo = m_aStatusbarControllerMap[ aHashKey ];
                rControllerInfo.m_aImplementationName = aService;
                rControllerInfo.m_aValue = aValue;
            }
        }

        // UNSAFE
        aLock.unlock();

        Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
        if ( xContainer.is() )
            xContainer->addContainerListener( this );
    }
}

sal_Bool ConfigurationAccess_StatusbarControllerFactory::impl_getElementProps(
    const Any& aElement,
    rtl::OUString& aCommand,
    rtl::OUString& aModule,
    rtl::OUString& aServiceSpecifier,
    rtl::OUString& aValue ) const
{
    Reference< XPropertySet > xPropertySet;
    aElement >>= xPropertySet;

    if ( xPropertySet.is() )
    {
        try
        {
            xPropertySet->getPropertyValue( m_aPropCommand ) >>= aCommand;
            xPropertySet->getPropertyValue( m_aPropModule ) >>= aModule;
            xPropertySet->getPropertyValue( m_aPropController ) >>= aServiceSpecifier;
            xPropertySet->getPropertyValue( m_aPropValue ) >>= aValue;
        }
        catch ( com::sun::star::beans::UnknownPropertyException& )
        {
            return sal_False;
        }
        catch ( com::sun::star::lang::WrappedTargetException& )
        {
            return sal_False;
        }
    }

    return sal_True;
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_4                    (    StatusbarControllerFactory                                                      ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::lang::XMultiComponentFactory                             ),
                                            DIRECT_INTERFACE( ::com::sun::star::frame::XUIControllerRegistration    )
                                        )

DEFINE_XTYPEPROVIDER_4                  (   StatusbarControllerFactory          ,
                                            css::lang::XTypeProvider            ,
                                            css::lang::XServiceInfo             ,
                                            css::lang::XMultiComponentFactory   ,
                                            ::com::sun::star::frame::XUIControllerRegistration
                                        )

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   StatusbarControllerFactory                      ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_STATUSBARCONTROLLERFACTORY          ,
                                            IMPLEMENTATIONNAME_STATUSBARCONTROLLERFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   StatusbarControllerFactory, {} )

StatusbarControllerFactory::StatusbarControllerFactory( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = new ConfigurationAccess_StatusbarControllerFactory( m_xServiceManager );
    m_pConfigAccess->acquire();
}

StatusbarControllerFactory::~StatusbarControllerFactory()
{
    ResetableGuard aLock( m_aLock );

    // reduce reference count
    m_pConfigAccess->release();
}

// XMultiComponentFactory
Reference< XInterface > SAL_CALL StatusbarControllerFactory::createInstanceWithContext(
    const ::rtl::OUString& aServiceSpecifier,
    const Reference< XComponentContext >& )
throw (Exception, RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    rtl::OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( aServiceSpecifier, rtl::OUString() );
    if ( aServiceName.getLength() > 0 )
        return m_xServiceManager->createInstance( aServiceName );
    else
        return Reference< XInterface >();
    // SAFE
}

Reference< XInterface > SAL_CALL StatusbarControllerFactory::createInstanceWithArgumentsAndContext(
    const ::rtl::OUString&                  ServiceSpecifier,
    const Sequence< Any >&                  Arguments,
    const Reference< XComponentContext >& )
throw (Exception, RuntimeException)
{
    const rtl::OUString aPropModuleName( RTL_CONSTASCII_USTRINGPARAM( "ModuleName" ));
    const rtl::OUString aPropValueName( RTL_CONSTASCII_USTRINGPARAM( "Value" ));

    rtl::OUString   aPropName;
    PropertyValue   aPropValue;

    // Retrieve the optional module name form the Arguments sequence. It is used as a part of
    // the hash map key to support different controller implementation for the same URL but different
    // module!!
    for ( int i = 0; i < Arguments.getLength(); i++ )
    {
        if (( Arguments[i] >>= aPropValue ) && ( aPropValue.Name.equals( aPropModuleName )))
        {
            aPropValue.Value >>= aPropName;
            break;
        }
    }

    Sequence< Any > aNewArgs( Arguments );

    sal_Int32 nAppendIndex = aNewArgs.getLength();
    aNewArgs.realloc( aNewArgs.getLength()+2 );

    // Append the command URL to the Arguments sequence so that one controller can be
    // used for more than one command URL.
    aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
    aPropValue.Value  <<= ServiceSpecifier;
    aNewArgs[nAppendIndex] <<= aPropValue;

    // Append the optional value argument. It's an empty string if no additional info
    // is provided to the controller.
    rtl::OUString aValue = m_pConfigAccess->getValueFromCommandModule( ServiceSpecifier, aPropName );
    aPropValue.Name = aPropValueName;
    aPropValue.Value <<= aValue;
    aNewArgs[nAppendIndex+1] <<= aPropValue;

    {
        // SAFE
        ResetableGuard aLock( m_aLock );

        if ( !m_bConfigRead )
        {
            m_bConfigRead = sal_True;
            m_pConfigAccess->readConfigurationData();
        }

        rtl::OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( ServiceSpecifier, aPropName );
        Reference< XMultiServiceFactory > xServiceManager( m_xServiceManager );

        aLock.unlock();
        // SAFE


        if ( aServiceName.getLength() > 0 )
            return xServiceManager->createInstanceWithArguments( aServiceName, aNewArgs );
        else
            return Reference< XInterface >();
    }
}

Sequence< ::rtl::OUString > SAL_CALL StatusbarControllerFactory::getAvailableServiceNames()
throw (RuntimeException)
{
    return Sequence< ::rtl::OUString >();
}

// XUIControllerRegistration
sal_Bool SAL_CALL StatusbarControllerFactory::hasController(
    const ::rtl::OUString& aCommandURL,
    const rtl::OUString& aModuleName )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    return ( m_pConfigAccess->getServiceFromCommandModule( aCommandURL, aModuleName ).getLength() > 0 );
}

void SAL_CALL StatusbarControllerFactory::registerController(
    const ::rtl::OUString& aCommandURL,
    const ::rtl::OUString& aModuleName,
    const ::rtl::OUString& aControllerImplementationName )
throw (RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addServiceToCommandModule( aCommandURL, aModuleName, aControllerImplementationName );
    // SAFE
}

void SAL_CALL StatusbarControllerFactory::deregisterController(
    const ::rtl::OUString& aCommandURL,
    const rtl::OUString& aModuleName )
throw (RuntimeException)
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeServiceFromCommandModule( aCommandURL, aModuleName );
    // SAFE
}

} // namespace framework
