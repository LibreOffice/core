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

#include <sal/config.h>

#include <string_view>

#include <uifactory/factoryconfiguration.hxx>
#include <services.h>

#include <helper/mischelper.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <comphelper/propertysequence.hxx>
#include <utility>

//  Defines

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

//  Namespace

namespace framework
{
static OUString getHashKeyFromStrings(
    std::u16string_view aCommandURL, std::u16string_view aModuleName )
{
    return OUString::Concat(aCommandURL) + "-" + aModuleName;
}

//  XInterface, XTypeProvider

ConfigurationAccess_ControllerFactory::ConfigurationAccess_ControllerFactory( const Reference< XComponentContext >& rxContext, OUString _sRoot ) :
    m_aPropCommand( u"Command"_ustr ),
    m_aPropModule( u"Module"_ustr ),
    m_aPropController( u"Controller"_ustr ),
    m_aPropValue( u"Value"_ustr ),
    m_sRoot(std::move(_sRoot)),
    m_bConfigAccessInitialized( false )
{
    m_xConfigProvider = configuration::theDefaultProvider::get( rxContext );
}

ConfigurationAccess_ControllerFactory::~ConfigurationAccess_ControllerFactory()
{
    std::unique_lock g(m_mutex);

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigAccessListener);
}

OUString ConfigurationAccess_ControllerFactory::getServiceFromCommandModule( std::u16string_view rCommandURL, std::u16string_view rModule ) const
{
    std::unique_lock g(m_mutex);
    MenuControllerMap::const_iterator pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aMenuControllerMap.end() )
        return pIter->second.m_aImplementationName;
    else if ( !rModule.empty() )
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aMenuControllerMap.find(
            getHashKeyFromStrings( rCommandURL, std::u16string_view() ));

        if ( pIter != m_aMenuControllerMap.end() )
            return pIter->second.m_aImplementationName;
    }

    return OUString();
}
OUString ConfigurationAccess_ControllerFactory::getValueFromCommandModule( std::u16string_view rCommandURL, std::u16string_view rModule ) const
{
    std::unique_lock g(m_mutex);

    MenuControllerMap::const_iterator pIter = m_aMenuControllerMap.find( getHashKeyFromStrings( rCommandURL, rModule ));

    if ( pIter != m_aMenuControllerMap.end() )
        return pIter->second.m_aValue;
    else if ( !rModule.empty() )
    {
        // Try to detect if we have a generic popup menu controller
        pIter = m_aMenuControllerMap.find(
            getHashKeyFromStrings( rCommandURL, std::u16string_view() ));

        if ( pIter != m_aMenuControllerMap.end() )
            return pIter->second.m_aValue;
    }

    return OUString();
}

void ConfigurationAccess_ControllerFactory::addServiceToCommandModule(
    std::u16string_view rCommandURL,
    std::u16string_view rModule,
    const OUString& rServiceSpecifier )
{
    std::unique_lock g(m_mutex);

    OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aMenuControllerMap.emplace( aHashKey,ControllerInfo(rServiceSpecifier,OUString()) );
}

void ConfigurationAccess_ControllerFactory::removeServiceFromCommandModule(
    std::u16string_view rCommandURL,
    std::u16string_view rModule )
{
    std::unique_lock g(m_mutex);

    OUString aHashKey = getHashKeyFromStrings( rCommandURL, rModule );
    m_aMenuControllerMap.erase( aHashKey );
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_ControllerFactory::elementInserted( const ContainerEvent& aEvent )
{
    OUString   aCommand;
    OUString   aModule;
    OUString   aService;
    OUString   aValue;

    std::unique_lock g(m_mutex);

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

void SAL_CALL ConfigurationAccess_ControllerFactory::elementRemoved ( const ContainerEvent& aEvent )
{
    OUString   aCommand;
    OUString   aModule;
    OUString   aService;
    OUString   aValue;

    std::unique_lock g(m_mutex);

    if ( impl_getElementProps( aEvent.Element, aCommand, aModule, aService, aValue ))
    {
        // Create hash key from command and module as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aCommand, aModule ));
        m_aMenuControllerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_ControllerFactory::elementReplaced( const ContainerEvent& aEvent )
{
    elementInserted(aEvent);
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_ControllerFactory::disposing( const EventObject& )
{
    // remove our reference to the config access
    std::unique_lock g(m_mutex);
    m_xConfigAccess.clear();
}

void ConfigurationAccess_ControllerFactory::readConfigurationData()
{
    // SAFE
    std::unique_lock aLock( m_mutex );

    if ( !m_bConfigAccessInitialized )
    {
        uno::Sequence<uno::Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", uno::Any(m_sRoot)}
        }));
        try
        {
            m_xConfigAccess.set( m_xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,aArgs ), UNO_QUERY );
        }
        catch ( const WrappedTargetException& )
        {
        }

        m_bConfigAccessInitialized = true;
    }

    if ( !m_xConfigAccess.is() )
        return;

    // Read and update configuration data
    updateConfigurationDataImpl();

    uno::Reference< container::XContainer > xContainer( m_xConfigAccess, uno::UNO_QUERY );
    // UNSAFE
    aLock.unlock();

    if ( xContainer.is() )
    {
        m_xConfigAccessListener = new WeakContainerListener(this);
        xContainer->addContainerListener(m_xConfigAccessListener);
    }
}

void ConfigurationAccess_ControllerFactory::updateConfigurationDataImpl()
{
    const Sequence< OUString > aPopupMenuControllers = m_xConfigAccess->getElementNames();

    OUString aCommand;
    OUString aModule;
    OUString aService;
    OUString aHashKey;
    OUString aValue;

    m_aMenuControllerMap.clear();
    for ( OUString const & name : aPopupMenuControllers )
    {
        try
        {
            if ( impl_getElementProps( m_xConfigAccess->getByName( name ), aCommand, aModule, aService,aValue ))
            {
                // Create hash key from command and module as they are together a primary key to
                // the UNO service that implements the popup menu controller.
                aHashKey = getHashKeyFromStrings( aCommand, aModule );
                m_aMenuControllerMap.emplace( aHashKey, ControllerInfo(aService,aValue) );
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

bool ConfigurationAccess_ControllerFactory::impl_getElementProps( const Any& aElement, OUString& aCommand, OUString& aModule, OUString& aServiceSpecifier,OUString& aValue  ) const
{
    Reference< XPropertySet > xPropertySet;
    aElement >>= xPropertySet;

    if ( !xPropertySet.is() )
        return true;

    try
    {
        xPropertySet->getPropertyValue( m_aPropCommand ) >>= aCommand;
        xPropertySet->getPropertyValue( m_aPropModule ) >>= aModule;
        xPropertySet->getPropertyValue( m_aPropController ) >>= aServiceSpecifier;
        xPropertySet->getPropertyValue( m_aPropValue ) >>= aValue;
    }
    catch ( const css::beans::UnknownPropertyException& )
    {
        return false;
    }
    catch ( const css::lang::WrappedTargetException& )
    {
        return false;
    }

    return true;
}
} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
