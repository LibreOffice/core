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

#include "uifactory/factoryconfiguration.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

namespace framework
{
rtl::OUString getHashKeyFromStrings( const rtl::OUString& aCommandURL, const rtl::OUString& aModuleName )
{
    rtl::OUStringBuffer aKey( aCommandURL );
    aKey.appendAscii( "-" );
    aKey.append( aModuleName );
    return aKey.makeStringAndClear();
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
ConfigurationAccess_ControllerFactory::ConfigurationAccess_ControllerFactory( const Reference< XComponentContext >& rxContext, const ::rtl::OUString& _sRoot,bool _bAskValue ) :
    ThreadHelpBase(),
    m_aPropCommand( RTL_CONSTASCII_USTRINGPARAM( "Command" )),
    m_aPropModule( RTL_CONSTASCII_USTRINGPARAM( "Module" )),
    m_aPropController( RTL_CONSTASCII_USTRINGPARAM( "Controller" )),
    m_aPropValue( RTL_CONSTASCII_USTRINGPARAM( "Value" )),
    m_sRoot(_sRoot),
    m_bConfigAccessInitialized( sal_False ),
    m_bAskValue(_bAskValue)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::ConfigurationAccess_ControllerFactory" );
    m_xConfigProvider = configuration::theDefaultProvider::get( rxContext );
}

ConfigurationAccess_ControllerFactory::~ConfigurationAccess_ControllerFactory()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigAccessListener);
}

rtl::OUString ConfigurationAccess_ControllerFactory::getServiceFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::getServiceFromCommandModule" );
    // SAFE
    ResetableGuard aLock( m_aLock );
    MenuControllerMap::const_iterator pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aMenuControllerMap.end() )
        return pIter->second.m_aImplementationName;
    else if ( !rModule.isEmpty() )
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rtl::OUString() ));

        if ( pIter != m_aMenuControllerMap.end() )
            return pIter->second.m_aImplementationName;
    }

    return rtl::OUString();
}
rtl::OUString ConfigurationAccess_ControllerFactory::getValueFromCommandModule( const rtl::OUString& rCommandURL, const rtl::OUString& rModule ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::getValueFromCommandModule" );
    // SAFE
    ResetableGuard aLock( m_aLock );

    MenuControllerMap::const_iterator pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aMenuControllerMap.end() )
        return pIter->second.m_aValue;
    else if ( !rModule.isEmpty() )
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rtl::OUString() ));

        if ( pIter != m_aMenuControllerMap.end() )
            return pIter->second.m_aValue;
    }

    return rtl::OUString();
}


void ConfigurationAccess_ControllerFactory::addServiceToCommandModule(
    const rtl::OUString& rCommandURL,
    const rtl::OUString& rModule,
    const rtl::OUString& rServiceSpecifier )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::addServiceToCommandModule" );
    // SAFE
    ResetableGuard aLock( m_aLock );

    rtl::OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aMenuControllerMap.insert( MenuControllerMap::value_type( aHashKey,ControllerInfo(rServiceSpecifier,::rtl::OUString()) ));
}

void ConfigurationAccess_ControllerFactory::removeServiceFromCommandModule(
    const rtl::OUString& rCommandURL,
    const rtl::OUString& rModule )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::removeServiceFromCommandModule" );
    // SAFE
    ResetableGuard aLock( m_aLock );

    rtl::OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aMenuControllerMap.erase( aHashKey );
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_ControllerFactory::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::elementInserted" );
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
        ControllerInfo& rControllerInfo = m_aMenuControllerMap[ aHashKey ];
        rControllerInfo.m_aImplementationName = aService;
        rControllerInfo.m_aValue = aValue;
    }
}

void SAL_CALL ConfigurationAccess_ControllerFactory::elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::elementRemoved" );
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
        m_aMenuControllerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_ControllerFactory::elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::elementReplaced" );
    elementInserted(aEvent);
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_ControllerFactory::disposing( const EventObject& ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::disposing" );
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();
}

void ConfigurationAccess_ControllerFactory::readConfigurationData()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::readConfigurationData" );
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        Sequence< Any > aArgs( 1 );
        PropertyValue   aPropValue;

        aPropValue.Name  = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ));
        aPropValue.Value <<= m_sRoot;
        aArgs[0] <<= aPropValue;

        try
        {
            m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,aArgs ), UNO_QUERY );
        }
        catch ( const WrappedTargetException& )
        {
        }

        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
    {
        // Read and update configuration data
        updateConfigurationData();

        uno::Reference< container::XContainer > xContainer( m_xConfigAccess, uno::UNO_QUERY );
        // UNSAFE
        aLock.unlock();

        if ( xContainer.is() )
        {
            m_xConfigAccessListener = new WeakContainerListener(this);
            xContainer->addContainerListener(m_xConfigAccessListener);
        }
    }
}

void ConfigurationAccess_ControllerFactory::updateConfigurationData()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::updateConfigurationData" );
    // SAFE
    ResetableGuard aLock( m_aLock );
    if ( m_xConfigAccess.is() )
    {
        Sequence< rtl::OUString >   aPopupMenuControllers = m_xConfigAccess->getElementNames();

        rtl::OUString aCommand;
        rtl::OUString aModule;
        rtl::OUString aService;
        rtl::OUString aHashKey;
        rtl::OUString aValue;

        m_aMenuControllerMap.clear();
        for ( sal_Int32 i = 0; i < aPopupMenuControllers.getLength(); i++ )
        {
            try
            {
                if ( impl_getElementProps( m_xConfigAccess->getByName( aPopupMenuControllers[i] ), aCommand, aModule, aService,aValue ))
                {
                    // Create hash key from command and module as they are together a primary key to
                    // the UNO service that implements the popup menu controller.
                    aHashKey = getHashKeyFromStrings( aCommand, aModule );
                    m_aMenuControllerMap.insert( MenuControllerMap::value_type( aHashKey, ControllerInfo(aService,aValue) ));
                }
            }
            catch ( const NoSuchElementException& )
            {
            }
            catch ( const WrappedTargetException& )
            {
            }
        }
    }
}

sal_Bool ConfigurationAccess_ControllerFactory::impl_getElementProps( const Any& aElement, rtl::OUString& aCommand, rtl::OUString& aModule, rtl::OUString& aServiceSpecifier,rtl::OUString& aValue  ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "ConfigurationAccess_ControllerFactory::impl_getElementProps" );
    Reference< XPropertySet > xPropertySet;
    aElement >>= xPropertySet;

    if ( xPropertySet.is() )
    {
        try
        {
            xPropertySet->getPropertyValue( m_aPropCommand ) >>= aCommand;
            xPropertySet->getPropertyValue( m_aPropModule ) >>= aModule;
            xPropertySet->getPropertyValue( m_aPropController ) >>= aServiceSpecifier;
            if ( m_bAskValue )
                xPropertySet->getPropertyValue( m_aPropValue ) >>= aValue;
        }
        catch ( const com::sun::star::beans::UnknownPropertyException& )
        {
            return sal_False;
        }
        catch ( const com::sun::star::lang::WrappedTargetException& )
        {
            return sal_False;
        }
    }

    return sal_True;
}
} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
