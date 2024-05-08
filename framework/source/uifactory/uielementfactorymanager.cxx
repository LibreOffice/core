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

#include <uifactory/configurationaccessfactorymanager.hxx>
#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/loader/CannotActivateFactoryException.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/UnknownModuleException.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/ui/XUIElementFactoryManager.hpp>

#include <rtl/ref.hxx>
#include <sal/log.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <utility>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::frame;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace framework;

namespace framework
{

// global function needed by both implementations
static OUString getHashKeyFromStrings( std::u16string_view aType, std::u16string_view aName, std::u16string_view aModuleName )
{
    return OUString::Concat(aType) + "^" + aName + "^" + aModuleName;
}

ConfigurationAccess_FactoryManager::ConfigurationAccess_FactoryManager( const Reference< XComponentContext >& rxContext, OUString  _sRoot ) :
    m_aPropType( u"Type"_ustr ),
    m_aPropName( u"Name"_ustr ),
    m_aPropModule( u"Module"_ustr ),
    m_aPropFactory( u"FactoryImplementation"_ustr ),
    m_sRoot(std::move(_sRoot)),
    m_bConfigAccessInitialized( false )
{
    m_xConfigProvider = theDefaultProvider::get( rxContext );
}

ConfigurationAccess_FactoryManager::~ConfigurationAccess_FactoryManager()
{
    // SAFE
    std::unique_lock g(m_aMutex);

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

OUString ConfigurationAccess_FactoryManager::getFactorySpecifierFromTypeNameModule( std::u16string_view rType, std::u16string_view rName, std::u16string_view rModule ) const
{
    // SAFE
    std::unique_lock g(m_aMutex);

    FactoryManagerMap::const_iterator pIter =
        m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, rName, rModule ));
    if ( pIter != m_aFactoryManagerMap.end() )
        return pIter->second;
    else
    {
        pIter = m_aFactoryManagerMap.find(
            getHashKeyFromStrings( rType, rName, std::u16string_view() ));
        if ( pIter != m_aFactoryManagerMap.end() )
            return pIter->second;
        else
        {
            // Support factories which uses a defined prefix before the ui name.
            size_t nIndex = rName.find( '_' );
            if ( nIndex > 0 && nIndex != std::u16string_view::npos)
            {
                std::u16string_view aName = rName.substr( 0, nIndex+1 );
                pIter = m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, aName, std::u16string_view() ));
                if ( pIter != m_aFactoryManagerMap.end() )
                    return pIter->second;
            }

            pIter = m_aFactoryManagerMap.find( getHashKeyFromStrings( rType, std::u16string_view(), std::u16string_view() ));
            if ( pIter != m_aFactoryManagerMap.end() )
                return pIter->second;
        }
    }

    return OUString();
}

void ConfigurationAccess_FactoryManager::addFactorySpecifierToTypeNameModule( std::u16string_view rType, std::u16string_view rName, std::u16string_view rModule, const OUString& rServiceSpecifier )
{
    // SAFE
    std::unique_lock g(m_aMutex);

    OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.find( aHashKey );

    if ( pIter != m_aFactoryManagerMap.end() )
        throw ElementExistException();
    m_aFactoryManagerMap.emplace( aHashKey, rServiceSpecifier );
}

void ConfigurationAccess_FactoryManager::removeFactorySpecifierFromTypeNameModule( std::u16string_view rType, std::u16string_view rName, std::u16string_view rModule )
{
    // SAFE
    std::unique_lock g(m_aMutex);

    OUString aHashKey = getHashKeyFromStrings( rType, rName, rModule );

    FactoryManagerMap::const_iterator pIter = m_aFactoryManagerMap.find( aHashKey );

    if ( pIter == m_aFactoryManagerMap.end() )
        throw NoSuchElementException();
    m_aFactoryManagerMap.erase( aHashKey );
}

Sequence< Sequence< PropertyValue > > ConfigurationAccess_FactoryManager::getFactoriesDescription() const
{
    // SAFE
    std::unique_lock g(m_aMutex);

    Sequence< Sequence< PropertyValue > > aSeqSeq;

    sal_Int32 nIndex( 0 );
    for ( const auto& rEntry : m_aFactoryManagerMap )
    {
        OUString aFactory = rEntry.first;
        if ( !aFactory.isEmpty() )
        {
            sal_Int32                 nToken = 0;

            aSeqSeq.realloc( aSeqSeq.getLength() + 1 );
            Sequence< PropertyValue > aSeq{ comphelper::makePropertyValue(
                m_aPropType, aFactory.getToken( 0, '^', nToken )) };
            if ( nToken > 0 )
            {
                aSeq.realloc( 2 );
                aSeq.getArray()[1]
                    = comphelper::makePropertyValue(m_aPropName,
                                                    aFactory.getToken( 0, '^', nToken ));
                if ( nToken > 0 )
                {
                    aSeq.realloc( 3 );
                    aSeq.getArray()[2]
                        = comphelper::makePropertyValue(m_aPropModule,
                                                        aFactory.getToken( 0, '^', nToken ));
                }
            }

            aSeqSeq.getArray()[nIndex++] = aSeq;
        }
    }

    return aSeqSeq;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_FactoryManager::elementInserted( const ContainerEvent& aEvent )
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    // SAFE
    std::unique_lock g(m_aMutex);

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from type, name and module as they are together a primary key to
        // the UNO service that implements a user interface factory.
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.emplace( aHashKey, aService );
    }
}

void SAL_CALL ConfigurationAccess_FactoryManager::elementRemoved ( const ContainerEvent& aEvent )
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    // SAFE
    std::unique_lock g(m_aMutex);

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from command and model as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.erase( aHashKey );
    }
}

void SAL_CALL ConfigurationAccess_FactoryManager::elementReplaced( const ContainerEvent& aEvent )
{
    OUString   aType;
    OUString   aName;
    OUString   aModule;
    OUString   aService;

    // SAFE
    std::unique_lock g(m_aMutex);

    if ( impl_getElementProps( aEvent.Element, aType, aName, aModule, aService ))
    {
        // Create hash key from command and model as they are together a primary key to
        // the UNO service that implements the popup menu controller.
        OUString aHashKey( getHashKeyFromStrings( aType, aName, aModule ));
        m_aFactoryManagerMap.erase( aHashKey );
        m_aFactoryManagerMap.emplace( aHashKey, aService );
    }
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_FactoryManager::disposing( const EventObject& )
{
    // SAFE
    // remove our reference to the config access
    std::unique_lock g(m_aMutex);
    m_xConfigAccess.clear();
}

void ConfigurationAccess_FactoryManager::readConfigurationData()
{
    // SAFE
    std::unique_lock g(m_aMutex);

    if ( !m_bConfigAccessInitialized )
    {
        Sequence<Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(m_sRoot)}
        }));

        try
        {
            m_xConfigAccess.set( m_xConfigProvider->createInstanceWithArguments(
                        u"com.sun.star.configuration.ConfigurationAccess"_ustr, aArgs ), UNO_QUERY );
        }
        catch ( const WrappedTargetException& )
        {
        }

        m_bConfigAccessInitialized = true;
    }

    if ( !m_xConfigAccess.is() )
        return;

    const Sequence< OUString > aUIElementFactories = m_xConfigAccess->getElementNames();

    OUString             aType;
    OUString             aName;
    OUString             aModule;
    OUString             aService;
    OUString             aHashKey;
    for ( OUString const & factoryName : aUIElementFactories )
    {
        if ( impl_getElementProps( m_xConfigAccess->getByName( factoryName ), aType, aName, aModule, aService ))
        {
            // Create hash key from type, name and module as they are together a primary key to
            // the UNO service that implements the user interface element factory.
            aHashKey = getHashKeyFromStrings( aType, aName, aModule );
            m_aFactoryManagerMap.emplace( aHashKey, aService );
        }
    }

    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
    {
        m_xConfigListener = new WeakContainerListener(this);
        xContainer->addContainerListener(m_xConfigListener);
    }
}

bool ConfigurationAccess_FactoryManager::impl_getElementProps( const Any& aElement, OUString& rType, OUString& rName, OUString& rModule, OUString& rServiceSpecifier ) const
{
    Reference< XPropertySet > xPropertySet;
    aElement >>= xPropertySet;

    if ( !xPropertySet.is() )
        return true;

    try
    {
        xPropertySet->getPropertyValue( m_aPropType ) >>= rType;
        xPropertySet->getPropertyValue( m_aPropName ) >>= rName;
        xPropertySet->getPropertyValue( m_aPropModule ) >>= rModule;
        xPropertySet->getPropertyValue( m_aPropFactory ) >>= rServiceSpecifier;
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

} // framework

namespace {

typedef comphelper::WeakComponentImplHelper<
    css::lang::XServiceInfo,
    css::ui::XUIElementFactoryManager> UIElementFactoryManager_BASE;

class UIElementFactoryManager : public UIElementFactoryManager_BASE
{
    virtual void disposing(std::unique_lock<std::mutex>&) override;
public:
    explicit UIElementFactoryManager( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.framework.UIElementFactoryManager"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.ui.UIElementFactoryManager"_ustr};
    }

    // XUIElementFactory
    virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL createUIElement( const OUString& ResourceURL, const css::uno::Sequence< css::beans::PropertyValue >& Args ) override;

    // XUIElementFactoryRegistration
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getRegisteredFactories(  ) override;
    virtual css::uno::Reference< css::ui::XUIElementFactory > SAL_CALL getFactory( const OUString& ResourceURL, const OUString& ModuleIdentifier ) override;
    virtual void SAL_CALL registerFactory( const OUString& aType, const OUString& aName, const OUString& aModuleIdentifier, const OUString& aFactoryImplementationName ) override;
    virtual void SAL_CALL deregisterFactory( const OUString& aType, const OUString& aName, const OUString& aModuleIdentifier ) override;

private:
    bool                                                  m_bConfigRead;
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    rtl::Reference<ConfigurationAccess_FactoryManager> m_pConfigAccess;
};

UIElementFactoryManager::UIElementFactoryManager( const Reference< XComponentContext >& rxContext ) :
    m_bConfigRead( false ),
    m_xContext(rxContext),
    m_pConfigAccess(
        new ConfigurationAccess_FactoryManager(
            rxContext,
            u"/org.openoffice.Office.UI.Factories/Registered/UIElementFactories"_ustr))
{}

void UIElementFactoryManager::disposing(std::unique_lock<std::mutex>&)
{
    m_pConfigAccess.clear();
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL UIElementFactoryManager::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
{
    Reference< XFrame > xFrame;
    OUString aModuleId;
    { // SAFE
        std::unique_lock g(m_aMutex);
        if (m_bDisposed) {
            throw css::lang::DisposedException(
                u"disposed"_ustr, static_cast<OWeakObject *>(this));
        }

        if ( !m_bConfigRead )
        {
            m_bConfigRead = true;
            m_pConfigAccess->readConfigurationData();
        }

        // Retrieve the frame instance from the arguments to determine the module identifier. This must be provided
        // to the search function. An empty module identifier is provided if the frame is missing or the module id cannot
        // retrieve from it.
        for ( auto const & arg : Args )
        {
            if ( arg.Name == "Frame")
                arg.Value >>= xFrame;
            if (arg.Name == "Module")
                arg.Value >>= aModuleId;
        }
    } // SAFE

    Reference< XModuleManager2 > xManager = ModuleManager::create( m_xContext );

    // Determine the module identifier
    try
    {
        if ( aModuleId.isEmpty() && xFrame.is() && xManager.is() )
            aModuleId = xManager->identify( Reference<XInterface>( xFrame, UNO_QUERY ) );

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
{
    // SAFE
    std::unique_lock g(m_aMutex);
    if (m_bDisposed) {
        throw css::lang::DisposedException(
            u"disposed"_ustr, static_cast<OWeakObject *>(this));
    }

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    return m_pConfigAccess->getFactoriesDescription();
}

Reference< XUIElementFactory > SAL_CALL UIElementFactoryManager::getFactory( const OUString& aResourceURL, const OUString& aModuleId )
{
    OUString aServiceSpecifier;
    { // SAFE
        std::unique_lock g(m_aMutex);
        if (m_bDisposed) {
            throw css::lang::DisposedException(
                u"disposed"_ustr, static_cast<OWeakObject *>(this));
        }

        if ( !m_bConfigRead )
        {
            m_bConfigRead = true;
            m_pConfigAccess->readConfigurationData();
        }

        OUString aType;
        OUString aName;
        RetrieveTypeNameFromResourceURL( aResourceURL, aType, aName );
        aServiceSpecifier = m_pConfigAccess->getFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    } // SAFE

    if ( !aServiceSpecifier.isEmpty() ) try
    {
        Reference< XUIElementFactory > xFactory(m_xContext->getServiceManager()->
                createInstanceWithContext(aServiceSpecifier, m_xContext), UNO_QUERY);
        SAL_WARN_IF(!xFactory.is(), "fwk.uielement", "could not create factory: " << aServiceSpecifier);
        return xFactory;
    }
    catch ( const css::loader::CannotActivateFactoryException& )
    {
        SAL_WARN("fwk.uielement", aServiceSpecifier <<
                " not available. This should happen only on mobile platforms.");
    }
    return Reference< XUIElementFactory >();
}

void SAL_CALL UIElementFactoryManager::registerFactory( const OUString& aType, const OUString& aName, const OUString& aModuleId, const OUString& aFactoryImplementationName )
{
    // SAFE
    std::unique_lock g(m_aMutex);
    if (m_bDisposed) {
        throw css::lang::DisposedException(
            u"disposed"_ustr, static_cast<OWeakObject *>(this));
    }

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->addFactorySpecifierToTypeNameModule( aType, aName, aModuleId, aFactoryImplementationName );
    // SAFE
}

void SAL_CALL UIElementFactoryManager::deregisterFactory( const OUString& aType, const OUString& aName, const OUString& aModuleId )
{
    // SAFE
    std::unique_lock g(m_aMutex);
    if (m_bDisposed) {
        throw css::lang::DisposedException(
            u"disposed"_ustr, static_cast<OWeakObject *>(this));
    }

    if ( !m_bConfigRead )
    {
        m_bConfigRead = true;
        m_pConfigAccess->readConfigurationData();
    }

    m_pConfigAccess->removeFactorySpecifierFromTypeNameModule( aType, aName, aModuleId );
    // SAFE
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_UIElementFactoryManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UIElementFactoryManager(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
