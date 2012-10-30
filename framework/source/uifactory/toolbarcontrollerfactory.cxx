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
                                            SERVICENAME_TOOLBARCONTROLLERFACTORY            ,
                                            IMPLEMENTATIONNAME_TOOLBARCONTROLLERFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   ToolbarControllerFactory, {} )

ToolbarControllerFactory::ToolbarControllerFactory( const Reference< XMultiServiceFactory >& xServiceManager ) :
    ThreadHelpBase(),
    m_bConfigRead( sal_False ),
    m_xServiceManager( xServiceManager )
{
    m_pConfigAccess = new ConfigurationAccess_ControllerFactory( comphelper::getComponentContext(m_xServiceManager), rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.UI.Controller/Registered/ToolBar" )) );
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
    if ( !aServiceName.isEmpty() )
        return m_xServiceManager->createInstance( aServiceName );
    else
        return Reference< XInterface >();
    // SAFE
}

Reference< XInterface > SAL_CALL ToolbarControllerFactory::createInstanceWithArgumentsAndContext(
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
    bool bHasValue = m_pConfigAccess->hasValue();
    aNewArgs.realloc( aNewArgs.getLength() + (bHasValue ? 2 : 1) );

    // Append the command URL to the Arguments sequence so that one controller can be
    // used for more than one command URL.
    aPropValue.Name     = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));
    aPropValue.Value  <<= ServiceSpecifier;
    aNewArgs[nAppendIndex] <<= aPropValue;

    if ( bHasValue )
    {
        // Append the optional value argument. It's an empty string if no additional info
        // is provided to the controller.
        rtl::OUString aValue = m_pConfigAccess->getValueFromCommandModule( ServiceSpecifier, aPropName );
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

        rtl::OUString aServiceName = m_pConfigAccess->getServiceFromCommandModule( ServiceSpecifier, aPropName );
        Reference< XMultiServiceFactory > xServiceManager( m_xServiceManager );

        aLock.unlock();
        // SAFE


        if ( !aServiceName.isEmpty() )
            return xServiceManager->createInstanceWithArguments( aServiceName, aNewArgs );
        else
            return Reference< XInterface >();
    }
}

Sequence< ::rtl::OUString > SAL_CALL ToolbarControllerFactory::getAvailableServiceNames()
throw (RuntimeException)
{
    return Sequence< ::rtl::OUString >();
}

// XUIControllerRegistration
sal_Bool SAL_CALL ToolbarControllerFactory::hasController(
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

    return ( !m_pConfigAccess->getServiceFromCommandModule( aCommandURL, aModuleName ).isEmpty() );
}

void SAL_CALL ToolbarControllerFactory::registerController(
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

void SAL_CALL ToolbarControllerFactory::deregisterController(
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
