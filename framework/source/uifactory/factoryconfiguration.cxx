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

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

namespace framework
{
OUString getHashKeyFromStrings( const OUString& aCommandURL, const OUString& aModuleName )
{
    OUStringBuffer aKey( aCommandURL );
    aKey.appendAscii( "-" );
    aKey.append( aModuleName );
    return aKey.makeStringAndClear();
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
ConfigurationAccess_ControllerFactory::ConfigurationAccess_ControllerFactory( const Reference< XComponentContext >& rxContext, const OUString& _sRoot,bool _bAskValue ) :
    ThreadHelpBase(),
    m_aPropCommand( "Command" ),
    m_aPropModule( "Module" ),
    m_aPropController( "Controller" ),
    m_aPropValue( "Value" ),
    m_sRoot(_sRoot),
    m_bConfigAccessInitialized( sal_False ),
    m_bAskValue(_bAskValue)
{
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

OUString ConfigurationAccess_ControllerFactory::getServiceFromCommandModule( const OUString& rCommandURL, const OUString& rModule ) const
{
    // SAFE
    ResetableGuard aLock( m_aLock );
    MenuControllerMap::const_iterator pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aMenuControllerMap.end() )
        return pIter->second.m_aImplementationName;
    else if ( !rModule.isEmpty() )
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, OUString() ));

        if ( pIter != m_aMenuControllerMap.end() )
            return pIter->second.m_aImplementationName;
    }

    return OUString();
}
OUString ConfigurationAccess_ControllerFactory::getValueFromCommandModule( const OUString& rCommandURL, const OUString& rModule ) const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    MenuControllerMap::const_iterator pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aMenuControllerMap.end() )
        return pIter->second.m_aValue;
    else if ( !rModule.isEmpty() )
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, OUString() ));

        if ( pIter != m_aMenuControllerMap.end() )
            return pIter->second.m_aValue;
    }

    return OUString();
}


void ConfigurationAccess_ControllerFactory::addServiceToCommandModule(
    const OUString& rCommandURL,
    const OUString& rModule,
    const OUString& rServiceSpecifier )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aMenuControllerMap.insert( MenuControllerMap::value_type( aHashKey,ControllerInfo(rServiceSpecifier,OUString()) ));
}

void ConfigurationAccess_ControllerFactory::removeServiceFromCommandModule(
    const OUString& rCommandURL,
    const OUString& rModule )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aMenuControllerMap.erase( aHashKey );
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_ControllerFactory::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    OUString   aCommand;
    OUString   aModule;
    OUString   aService;
    OUString   aValue;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aCommand, aModule, aService, aValue ))
    {
        // Create hash key from command and module as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aCommand, aModule ));
        ControllerInfo& rControllerInfo = m_aMenuControllerMap[ aHashKey ];
        rControllerInfo.m_aImplementationName = aService;
        rControllerInfo.m_aValue = aValue;
    }
}

void SAL_CALL ConfigurationAccess_ControllerFactory::elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    OUString   aCommand;
    OUString   aModule;
    OUString   aService;
    OUString   aValue;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aCommand, aModule, aService, aValue ))
    {
        // Create hash key from command and module as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aCommand, aModule ));
        m_aMenuControllerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_ControllerFactory::elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    elementInserted(aEvent);
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_ControllerFactory::disposing( const EventObject& ) throw(RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();
}

void ConfigurationAccess_ControllerFactory::readConfigurationData()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigAccessInitialized )
    {
        Sequence< Any > aArgs( 1 );
        PropertyValue   aPropValue;

        aPropValue.Name  = OUString( "nodepath" );
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
    // SAFE
    ResetableGuard aLock( m_aLock );
    if ( m_xConfigAccess.is() )
    {
        Sequence< OUString >   aPopupMenuControllers = m_xConfigAccess->getElementNames();

        OUString aCommand;
        OUString aModule;
        OUString aService;
        OUString aHashKey;
        OUString aValue;

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

sal_Bool ConfigurationAccess_ControllerFactory::impl_getElementProps( const Any& aElement, OUString& aCommand, OUString& aModule, OUString& aServiceSpecifier,OUString& aValue  ) const
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
