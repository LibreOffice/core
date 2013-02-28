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

#include "uifactory/toolbarcontrollerfactory.hxx"
#include "uifactory/factoryconfiguration.hxx"
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

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

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   ToolbarControllerFactory                        ,
                                            ::cppu::OWeakObject                             ,
                                            DECLARE_ASCII("com.sun.star.frame.ToolBarControllerFactory"),
                                            IMPLEMENTATIONNAME_TOOLBARCONTROLLERFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   ToolbarControllerFactory, {} )

ToolbarControllerFactory::ToolbarControllerFactory( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = new ConfigurationAccess_ControllerFactory( comphelper::getComponentContext(m_xServiceManager), OUString( "/org.openoffice.Office.UI.Controller/Registered/ToolBar" ) );
    m_pConfigAccess->acquire();
}

ToolbarControllerFactory::ToolbarControllerFactory( const Reference< XMultiServiceFactory >& xServiceManager,bool ) :
    ThreadHelpBase(),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = NULL;
}

ToolbarControllerFactory::~ToolbarControllerFactory()
{
    ResetableGuard aLock( m_aLock );

    // reduce reference count
    m_pConfigAccess->release();
}

// XMultiComponentFactory
Reference< XInterface > SAL_CALL ToolbarControllerFactory::createInstanceWithContext(
    const OUString& aServiceSpecifier,
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

    OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( aServiceSpecifier, OUString() );
    if ( !aServiceName.isEmpty() )
        return m_xServiceManager->createInstance( aServiceName );
    else
        return Reference< XInterface >();
    // SAFE
}

Reference< XInterface > SAL_CALL ToolbarControllerFactory::createInstanceWithArgumentsAndContext(
    const OUString&                  ServiceSpecifier,
    const Sequence< Any >&                  Arguments,
    const Reference< XComponentContext >& )
throw (Exception, RuntimeException)
{
    const OUString aPropModuleName( "ModuleName" );
    const OUString aPropValueName( "Value" );

    OUString   aPropName;
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
    bool bHasValue = m_pConfigAccess->hasValue();
    aNewArgs.realloc( aNewArgs.getLength() + (bHasValue ? 2 : 1) );

    // Append the command URL to the Arguments sequence so that one controller can be
    // used for more than one command URL.
    aPropValue.Name     = OUString( "CommandURL" );
    aPropValue.Value  <<= ServiceSpecifier;
    aNewArgs[nAppendIndex] <<= aPropValue;

    if ( bHasValue )
    {
        // Append the optional value argument. It's an empty string if no additional info
        // is provided to the controller.
        OUString aValue = m_pConfigAccess->getValueFromCommandModule( ServiceSpecifier, aPropName );
        aPropValue.Name = aPropValueName;
        aPropValue.Value <<= aValue;
        aNewArgs[nAppendIndex+1] <<= aPropValue;
    }

    {
        // SAFE
        ResetableGuard aLock( m_aLock );

        if ( !m_bConfigRead )
        {
            m_bConfigRead = sal_True;
            m_pConfigAccess->readConfigurationData();
        }

        OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( ServiceSpecifier, aPropName );
        Reference< XMultiServiceFactory > xServiceManager( m_xServiceManager );

        aLock.unlock();
        // SAFE


        if ( !aServiceName.isEmpty() )
            return xServiceManager->createInstanceWithArguments( aServiceName, aNewArgs );
        else
            return Reference< XInterface >();
    }
}

Sequence< OUString > SAL_CALL ToolbarControllerFactory::getAvailableServiceNames()
throw (RuntimeException)
{
    return Sequence< OUString >();
}

// XUIControllerRegistration
sal_Bool SAL_CALL ToolbarControllerFactory::hasController(
    const OUString& aCommandURL,
    const OUString& aModuleName )
throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    return ( !m_pConfigAccess->getServiceFromCommandModule( aCommandURL, aModuleName ).isEmpty() );
}

void SAL_CALL ToolbarControllerFactory::registerController(
    const OUString& aCommandURL,
    const OUString& aModuleName,
    const OUString& aControllerImplementationName )
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

void SAL_CALL ToolbarControllerFactory::deregisterController(
    const OUString& aCommandURL,
    const OUString& aModuleName )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
