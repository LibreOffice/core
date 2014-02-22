/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/config.h>

#include <uifactory/configurationaccessfactorymanager.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/ui/XUIElementFactoryManager.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;
using namespace framework;

namespace framework
{


static OUString getHashKeyFromStrings( const OUString& aType, const OUString& aName, const OUString& aModuleName )
{
    OUStringBuffer aKey( aType );
    aKey.appendAscii( "^" );
    aKey.append( aName );
    aKey.appendAscii( "^" );
    aKey.append( aModuleName );
    return aKey.makeStringAndClear();
}


ConfigurationAccess_FactoryManager::ConfigurationAccess_FactoryManager( const Reference< XComponentContext >& rxContext, const OUString& _sRoot ) :
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
    
    osl::MutexGuard g(m_aMutex);

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

OUString ConfigurationAccess_FactoryManager::getFactorySpecifierFromTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule ) const
{
    
    osl::MutexGuard g(m_aMutex);

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
    
    osl::MutexGuard g(m_aMutex);

    OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.find( aHashKey );

    if ( pIter != m_aFactoryManagerMap.end() )
        throw ElementExistException();
    else
        m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, rServiceSpecifier ));
}


void ConfigurationAccess_FactoryManager::removeFactorySpecifierFromTypeNameModule( const OUString& rType, const OUString& rName, const OUString& rModule )
{
    
    osl::MutexGuard g(m_aMutex);

    OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.find( aHashKey );

    if ( pIter == m_aFactoryManagerMap.end() )
        throw NoSuchElementException();
    else
        m_aFactoryManagerMap.erase( aHashKey );
}

Sequence< Sequence< PropertyValue > > ConfigurationAccess_FactoryManager::getFactoriesDescription() const
{
    
    osl::MutexGuard g(m_aMutex);

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


void SAL_CALL ConfigurationAccess_FactoryManager::elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException)
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    
    osl::MutexGuard g(m_aMutex);

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        
        
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

    
    osl::MutexGuard g(m_aMutex);

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        
        
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

    
    osl::MutexGuard g(m_aMutex);

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        
        
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.erase( aHashKey );
        m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, aService ));
    }
}


void SAL_CALL ConfigurationAccess_FactoryManager::disposing( const EventObject& ) throw(RuntimeException)
{
    
    
    osl::MutexGuard g(m_aMutex);
    m_xConfigAccess.clear();
}

void ConfigurationAccess_FactoryManager::readConfigurationData()
{
    
    osl::MutexGuard g(m_aMutex);

    if ( !m_bConfigAccessInitialized )
    {
        Sequence< Any > aArgs( 1 );
        PropertyValue   aPropValue;

        aPropValue.Name  = "nodepath";
        aPropValue.Value <<= m_sRoot;
        aArgs[0] <<= aPropValue;

        try
        {
            m_xConfigAccess.set( m_xConfigProvider->createInstanceWithArguments(
                        "com.sun.star.configuration.ConfigurationAccess", aArgs ), UNO_QUERY );
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
                
                
                aHashKey = getHashKeyFromStrings( aType, aName, aModule );
                m_aFactoryManagerMap.insert( FactoryManagerMap::value_type( aHashKey, aService ));
            }
        }

        Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
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

} 

namespace {

typedef ::cppu::WeakComponentImplHelper2<
    css::lang::XServiceInfo,
    css::ui::XUIElementFactoryManager> UIElementFactoryManager_BASE;

class UIElementFactoryManager : private osl::Mutex,
                                public UIElementFactoryManager_BASE
{
    virtual void SAL_CALL disposing() SAL_OVERRIDE;
public:
    UIElementFactoryManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UIElementFactoryManager();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    {
        return OUString("com.sun.star.comp.framework.UIElementFactoryManager");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException)
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException)
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = OUString("com.sun.star.ui.UIElementFactoryManager");
        return aSeq;
    }

    
    virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL createUIElement( const OUString& ResourceURL, const css::uno::Sequence< css::beans::PropertyValue >& Args ) throw (css::container::NoSuchElementException, css::lang::IllegalArgumentException, css::uno::RuntimeException);

    
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getRegisteredFactories(  ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::ui::XUIElementFactory > SAL_CALL getFactory( const OUString& ResourceURL, const OUString& ModuleIdentifier ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL registerFactory( const OUString& aType, const OUString& aName, const OUString& aModuleIdentifier, const OUString& aFactoryImplementationName ) throw (css::container::ElementExistException, css::uno::RuntimeException);
    virtual void SAL_CALL deregisterFactory( const OUString& aType, const OUString& aName, const OUString& aModuleIdentifier ) throw (css::container::NoSuchElementException, css::uno::RuntimeException);

private:
    sal_Bool                                                  m_bConfigRead;
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    ConfigurationAccess_FactoryManager*                       m_pConfigAccess;
};

UIElementFactoryManager::UIElementFactoryManager( const Reference< XComponentContext >& rxContext ) :
    UIElementFactoryManager_BASE(*static_cast<osl::Mutex *>(this)),
    m_bConfigRead( sal_False ),
    m_xContext(rxContext)
{
    m_pConfigAccess = new ConfigurationAccess_FactoryManager(rxContext,
            "/org.openoffice.Office.UI.Factories/Registered/UIElementFactories");
    m_pConfigAccess->acquire();
}

UIElementFactoryManager::~UIElementFactoryManager()
{
    disposing();
}

void SAL_CALL UIElementFactoryManager::disposing()
{
    osl::MutexGuard g(rBHelper.rMutex);
    if (m_pConfigAccess)
    {
        
        m_pConfigAccess->release();
        m_pConfigAccess = 0;
    }
}


Reference< XUIElement > SAL_CALL UIElementFactoryManager::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    Reference< XFrame > xFrame;
    { 
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    
    
    
    for ( int i = 0; i < Args.getLength(); i++ )
    {
        if ( Args[i].Name == "Frame")
            Args[i].Value >>= xFrame;
    }
    } 

    Reference< XModuleManager2 > xManager = ModuleManager::create( m_xContext );

    
    try
    {
        OUString aModuleId;
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


Sequence< Sequence< PropertyValue > > SAL_CALL UIElementFactoryManager::getRegisteredFactories()
throw ( RuntimeException )
{
    
    osl::MutexGuard g(rBHelper.rMutex);

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
    OUString aServiceSpecifier;
    { 
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    OUString aType;
    OUString aName;

    RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );

    aServiceSpecifier = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    } 

    if ( !aServiceSpecifier.isEmpty() ) try
    {
        return Reference< XUIElementFactory >(m_xContext->getServiceManager()->
                createInstanceWithContext(aServiceSpecifier, m_xContext), UNO_QUERY);
    }
    catch ( const css::loader::CannotActivateFactoryException& )
    {
        SAL_WARN("fwk.uielement", aServiceSpecifier <<
                " not available. This should happen only on mobile platforms.");
    }
    return Reference< XUIElementFactory >();
}

void SAL_CALL UIElementFactoryManager::registerFactory( const OUString& aType, const OUString& aName, const OUString& aModuleId, const OUString& aFactoryImplementationName )
throw ( ElementExistException, RuntimeException )
{
    
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addFactorySpecifierToTypeNameModule( aType, aName, aModuleId, aFactoryImplementationName );
    
}

void SAL_CALL UIElementFactoryManager::deregisterFactory( const OUString& aType, const OUString& aName, const OUString& aModuleId )
throw ( NoSuchElementException, RuntimeException )
{
    
    osl::MutexGuard g(rBHelper.rMutex);

    if ( !m_bConfigRead )
    {
        m_bConfigRead = sal_True;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new UIElementFactoryManager(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_UIElementFactoryManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
            Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
