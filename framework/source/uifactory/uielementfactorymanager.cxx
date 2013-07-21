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

#include <uifactory/uielementfactorymanager.hxx>
#include <uifactory/windowcontentfactorymanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/svapp.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________

namespace framework
{

// global function needed by both implementations
OUString getHashKeyFromStrings( const OUString& aType, const OUString& aName, const OUString& aModuleName )
{
    OUStringBuffer aKey( aType );
    aKey.appendAscii( "^" );
    aKey.append( aName );
    aKey.appendAscii( "^" );
    aKey.append( aModuleName );
    return aKey.makeStringAndClear();
}


//*****************************************************************************************************************
//  Configuration access class for UIElementFactoryManager implementation
//*****************************************************************************************************************


ConfigurationAccess_FactoryManager::ConfigurationAccess_FactoryManager( const Reference< XComponentContext >& rxContext, const OUString& _sRoot ) :
    ThreadHelpBase(),
    m_aPropType( "Type" ),
    m_aPropName( "Name" ),
    m_aPropModule( "Module" ),
    m_aPropFactory( "FactoryImplementation" ),
    m_sRoot(_sRoot),
    m_bConfigAccessInitialized( sal_False )
{
    m_xConfigProvider = theDefaultProvider::get( rxContext );
}

ConfigurationAccess_FactoryManager::~ConfigurationAccess_FactoryManager()
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

OUString ConfigurationAccess_FactoryManager::getFactorySpecifierFromTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule ) const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    FactoryManagerMap::const_iterator pIter =
        m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, rModule ));
    if ( pIter != m_aFactoryManagerMap.end() )
        return pIter->second;
    else
    {
        pIter = m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, OUString() ));
        if ( pIter != m_aFactoryManagerMap.end() )
            return pIter->second;
        else
        {
            // Support factories which uses a defined prefix before the ui name.
            sal_Int32 nIndex = rName.indexOf( '_' );
            if ( nIndex > 0 )
            {
                OUString aName = rName.copy( 0, nIndex+1 );
                pIter = m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, aName, OUString() ));
                if ( pIter != m_aFactoryManagerMap.end() )
                    return pIter->second;
            }

            pIter = m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, OUString(), OUString() ));
            if ( pIter != m_aFactoryManagerMap.end() )
                return pIter->second;
        }
    }

    return OUString();
}

void ConfigurationAccess_FactoryManager::addFactorySpecifierToTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule, const OUString& rServiceSpecifier )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.find( aHashKey );

    if ( pIter != m_aFactoryManagerMap.end() )
        throw ElementExistException();
    else
        m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, rServiceSpecifier ));
}


void ConfigurationAccess_FactoryManager::removeFactorySpecifierFromTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.find( aHashKey );

    if ( pIter == m_aFactoryManagerMap.end() )
        throw NoSuchElementException();
    else
        m_aFactoryManagerMap.erase( aHashKey );
}

Sequence< Sequence< PropertyValue > > ConfigurationAccess_FactoryManager::getFactoriesDescription() const
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    Sequence< Sequence< PropertyValue > > aSeqSeq;

    sal_Int32 nIndex( 0 );
    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.begin();
    while ( pIter != m_aFactoryManagerMap.end() )
    {
        OUString aFactory = pIter->first;
        if ( !aFactory.isEmpty() )
        {
            sal_Int32                 nToken = 0;
            Sequence< PropertyValue > aSeq( 1 );

            aSeqSeq.realloc( aSeqSeq.getLength() + 1 );
            aSeq[0].Name  = m_aPropType;
            aSeq[0].Value = makeAny( aFactory.getToken( 0, '^', nToken ));
            if ( nToken > 0 )
            {
                aSeq.realloc( 2 );
                aSeq[1].Name  = m_aPropName;
                aSeq[1].Value = makeAny( aFactory.getToken( 0, '^', nToken ));
                if ( nToken > 0 )
                {
                    aSeq.realloc( 3 );
                    aSeq[2].Name  = m_aPropModule;
                    aSeq[2].Value = makeAny( aFactory.getToken( 0, '^', nToken ));
                }
            }

            aSeqSeq[nIndex++] = aSeq;
        }

        ++pIter;
    }

    return aSeqSeq;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_FactoryManager::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from type, name and module as they are together a primary key to
        // the UNO service that implements a user interface factory.
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, aService ));
    }
}

void SAL_CALL ConfigurationAccess_FactoryManager::elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from command and model as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_FactoryManager::elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from command and model as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.erase( aHashKey );
        m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, aService ));
    }
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_FactoryManager::disposing( const EventObject& ) throw(RuntimeException)
{
    // SAFE
    // remove our reference to the config access
    ResetableGuard aLock( m_aLock );
    m_xConfigAccess.clear();
}

void ConfigurationAccess_FactoryManager::readConfigurationData()
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
            m_xConfigAccess.set( m_xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS,aArgs ), UNO_QUERY );
        }
        catch ( const WrappedTargetException& )
        {
        }

        m_bConfigAccessInitialized = sal_True;
    }

    if ( m_xConfigAccess.is() )
    {
        Sequence< OUString >   aUIElementFactories = m_xConfigAccess->getElementNames();

        OUString             aType;
        OUString             aName;
        OUString             aModule;
        OUString             aService;
        OUString             aHashKey;
        Reference< XPropertySet > xPropertySet;
        for ( sal_Int32 i = 0; i < aUIElementFactories.getLength(); i++ )
        {
            if ( impl_getElementProps( m_xConfigAccess->getByName( aUIElementFactories[i] ), aType, aName, aModule, aService ))
            {
                // Create hash key from type, name and module as they are together a primary key to
                // the UNO service that implements the user interface element factory.
                aHashKey = getHashKeyFromStrings( aType, aName, aModule );
                m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, aService ));
            }
        }

        Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
        aLock.unlock();
        // UNSAFE
        if ( xContainer.is() )
        {
            m_xConfigListener = new WeakContainerListener(this);
            xContainer->addContainerListener(m_xConfigListener);
        }
    }
}

sal_Bool ConfigurationAccess_FactoryManager::impl_getElementProps( const Any& aElement, OUString& rType, OUString& rName, OUString& rModule, OUString& rServiceSpecifier ) const
{
    Reference< XPropertySet > xPropertySet;
    aElement >>= xPropertySet;

    if ( xPropertySet.is() )
    {
        try
        {
            xPropertySet->getPropertyValue( m_aPropType ) >>= rType;
            xPropertySet->getPropertyValue( m_aPropName ) >>= rName;
            xPropertySet->getPropertyValue( m_aPropModule ) >>= rModule;
            xPropertySet->getPropertyValue( m_aPropFactory ) >>= rServiceSpecifier;
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

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2  (   UIElementFactoryManager                         ,
                                            ::cppu::OWeakObject                             ,
                                            DECLARE_ASCII("com.sun.star.ui.UIElementFactoryManager"),
                                            IMPLEMENTATIONNAME_UIELEMENTFACTORYMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   UIElementFactoryManager, {} )

UIElementFactoryManager::UIElementFactoryManager( const Reference< XComponentContext >& rxContext ) :
    ThreadHelpBase( &Application::GetSolarMutex() ),
    m_bConfigRead( sal_False ),
    m_xContext(rxContext)
{
    m_pConfigAccess = new ConfigurationAccess_FactoryManager( rxContext, OUString( "/org.openoffice.Office.UI.Factories/Registered/UIElementFactories" ) );
    m_pConfigAccess->acquire();
    m_xModuleManager = ModuleManager::create( rxContext );
}

UIElementFactoryManager::~UIElementFactoryManager()
{
    ResetableGuard aLock( m_aLock );

    // reduce reference count
    m_pConfigAccess->release();
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL UIElementFactoryManager::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    const OUString aPropFrame( "Frame" );

    OUString   aModuleId;
    PropertyValue   aPropValue;
    Reference< XFrame > xFrame;

    // Retrieve the frame instance from the arguments to determine the module identifier. This must be provided
    // to the search function. An empty module identifier is provided if the frame is missing or the module id cannot
    // retrieve from it.
    for ( int i = 0; i < Args.getLength(); i++ )
    {
        if ( Args[i].Name.equals( aPropFrame ))
            Args[i].Value >>= xFrame;
    }

    Reference< XModuleManager2 > xManager( m_xModuleManager );
    aLock.unlock();

    // Determine the module identifier
    try
    {
        if ( xFrame.is() && xManager.is() )
            aModuleId = xManager->identify( xFrame );

        Reference< XUIElementFactory > xUIElementFactory = getFactory( ResourceURL, aModuleId );
        if ( xUIElementFactory.is() )
            return xUIElementFactory->createUIElement( ResourceURL, Args );
    }
    catch ( const UnknownModuleException& )
    {
    }

    throw NoSuchElementException();
}

// XUIElementFactoryRegistration
Sequence< Sequence< PropertyValue > > SAL_CALL UIElementFactoryManager::getRegisteredFactories()
throw ( RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    return m_pConfigAccess->getFactoriesDescription();
}

Reference< XUIElementFactory > SAL_CALL UIElementFactoryManager::getFactory( const OUString& aResourceURL, const OUString& aModuleId )
throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    OUString aType;
    OUString aName;

    WindowContentFactoryManager::RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );

    Reference< XComponentContext > xContext( m_xContext );

    OUString aServiceSpecifier = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );

    aLock.unlock();
    if ( !aServiceSpecifier.isEmpty() )
        return Reference< XUIElementFactory >( xContext->getServiceManager()->createInstanceWithContext(aServiceSpecifier, xContext), UNO_QUERY );
    else
        return Reference< XUIElementFactory >();
}

void SAL_CALL UIElementFactoryManager::registerFactory( const OUString& aType, const OUString& aName, const OUString& aModuleId, const OUString& aFactoryImplementationName )
throw ( ElementExistException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addFactorySpecifierToTypeNameModule( aType, aName, aModuleId, aFactoryImplementationName );
    // SAFE
}

void SAL_CALL UIElementFactoryManager::deregisterFactory( const OUString& aType, const OUString& aName, const OUString& aModuleId )
throw ( NoSuchElementException, RuntimeException )
{
    // SAFE
    ResetableGuard aLock( m_aLock );

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    // SAFE
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
