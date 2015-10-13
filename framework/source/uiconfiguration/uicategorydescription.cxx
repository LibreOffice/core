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

#include <uielement/uicommanddescription.hxx>

#include "properties.h"

#include "helper/mischelper.hxx"

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/configmgr.hxx>

#include <vcl/mnemonic.hxx>
#include <comphelper/sequence.hxx>

#include <unordered_map>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace framework;

namespace {

struct ModuleToCategory
{
    const char* pModuleId;
    const char* pCommands;
};

static const char GENERIC_MODULE_NAME[]                     = "generic";
static const char CONFIGURATION_ROOT_ACCESS[]               = "/org.openoffice.Office.UI.";
static const char CONFIGURATION_CATEGORY_ELEMENT_ACCESS[]   = "/Commands/Categories";
static const char CONFIGURATION_PROPERTY_NAME[]             = "Name";

class ConfigurationAccess_UICategory : public ::cppu::WeakImplHelper<XNameAccess,XContainerListener>
{
    osl::Mutex aMutex;
    public:
                                  ConfigurationAccess_UICategory( const OUString& aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XComponentContext >& rxContext );
        virtual                   ~ConfigurationAccess_UICategory();

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName )
            throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName )
            throw (css::uno::RuntimeException, std::exception) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType()
            throw (css::uno::RuntimeException, std::exception) override;

        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException, std::exception) override;

        // container.XContainerListener
        virtual void SAL_CALL     elementInserted( const ContainerEvent& aEvent ) throw(RuntimeException, std::exception) override;
        virtual void SAL_CALL     elementRemoved ( const ContainerEvent& aEvent ) throw(RuntimeException, std::exception) override;
        virtual void SAL_CALL     elementReplaced( const ContainerEvent& aEvent ) throw(RuntimeException, std::exception) override;

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) throw(RuntimeException, std::exception) override;

    protected:
        Any                       getUINameFromID( const OUString& rId );
        Any                       getUINameFromCache( const OUString& rId );
        Sequence< OUString > getAllIds();
        bool                  fillCache();

    private:
        typedef std::unordered_map< OUString,
                                    OUString,
                                    OUStringHash,
                                    std::equal_to< OUString > > IdToInfoCache;

        bool initializeConfigAccess();

        OUString                          m_aConfigCategoryAccess;
        OUString                          m_aPropUIName;
        Reference< XNameAccess >          m_xGenericUICategories;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XContainerListener >   m_xConfigListener;
        bool                              m_bConfigAccessInitialized;
        bool                              m_bCacheFilled;
        IdToInfoCache                     m_aIdCache;
};

//  XInterface, XTypeProvider

ConfigurationAccess_UICategory::ConfigurationAccess_UICategory( const OUString& aModuleName, const Reference< XNameAccess >& rGenericUICategories, const Reference< XComponentContext >& rxContext ) :
    m_aConfigCategoryAccess( CONFIGURATION_ROOT_ACCESS ),
    m_aPropUIName( CONFIGURATION_PROPERTY_NAME ),
    m_xGenericUICategories( rGenericUICategories ),
    m_bConfigAccessInitialized( false ),
    m_bCacheFilled( false )
{
    // Create configuration hierarchical access name
    m_aConfigCategoryAccess += aModuleName;
    m_aConfigCategoryAccess += CONFIGURATION_CATEGORY_ELEMENT_ACCESS;

    m_xConfigProvider = theDefaultProvider::get( rxContext );
}

ConfigurationAccess_UICategory::~ConfigurationAccess_UICategory()
{
    // SAFE
    osl::MutexGuard g(aMutex);
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_UICategory::getByName( const OUString& rId )
throw ( NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    osl::MutexGuard g(aMutex);
    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = true;
        fillCache();
    }

    // SAFE
    Any a = getUINameFromID( rId );

    if ( !a.hasValue() )
        throw NoSuchElementException();

    return a;
}

Sequence< OUString > SAL_CALL ConfigurationAccess_UICategory::getElementNames()
throw ( RuntimeException, std::exception )
{
    return getAllIds();
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasByName( const OUString& rId )
throw (css::uno::RuntimeException, std::exception)
{
    return getByName( rId ).hasValue();
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICategory::getElementType()
throw ( RuntimeException, std::exception )
{
    return( cppu::UnoType<OUString>::get());
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasElements()
throw ( RuntimeException, std::exception )
{
    // There must be global categories!
    return sal_True;
}

bool ConfigurationAccess_UICategory::fillCache()
{
    SAL_INFO( "fwk", "framework (cd100003) ::ConfigurationAccess_UICategory::fillCache" );

    if ( m_bCacheFilled )
        return true;

    sal_Int32            i( 0 );
    OUString        aUIName;
    Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

    for ( i = 0; i < aNameSeq.getLength(); i++ )
    {
        try
        {
            Reference< XNameAccess > xNameAccess(m_xConfigAccess->getByName( aNameSeq[i] ),UNO_QUERY);
            if ( xNameAccess.is() )
            {
                xNameAccess->getByName( m_aPropUIName ) >>= aUIName;

                m_aIdCache.insert( IdToInfoCache::value_type( aNameSeq[i], aUIName ));
            }
        }
        catch ( const css::lang::WrappedTargetException& )
        {
        }
        catch ( const css::container::NoSuchElementException& )
        {
        }
    }

    m_bCacheFilled = true;

    return true;
}

Any ConfigurationAccess_UICategory::getUINameFromID( const OUString& rId )
{
    Any a;

    try
    {
        a = getUINameFromCache( rId );
        if ( !a.hasValue() )
        {
            // Try to ask our global commands configuration access
            if ( m_xGenericUICategories.is() )
            {
                try
                {
                    return m_xGenericUICategories->getByName( rId );
                }
                catch ( const css::lang::WrappedTargetException& )
                {
                }
                catch ( const css::container::NoSuchElementException& )
                {
                }
            }
        }
    }
    catch( const css::container::NoSuchElementException& )
    {
    }
    catch ( const css::lang::WrappedTargetException& )
    {
    }

    return a;
}

Any ConfigurationAccess_UICategory::getUINameFromCache( const OUString& rId )
{
    Any a;

    IdToInfoCache::const_iterator pIter = m_aIdCache.find( rId );
    if ( pIter != m_aIdCache.end() )
        a <<= pIter->second;

    return a;
}

Sequence< OUString > ConfigurationAccess_UICategory::getAllIds()
{
    // SAFE
    osl::MutexGuard g(aMutex);

    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = true;
        fillCache();
    }

    if ( m_xConfigAccess.is() )
    {
        Reference< XNameAccess > xNameAccess;

        try
        {
            Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

            if ( m_xGenericUICategories.is() )
            {
                // Create concat list of supported user interface commands of the module
                Sequence< OUString > aGenericNameSeq = m_xGenericUICategories->getElementNames();
                sal_uInt32 nCount1 = aNameSeq.getLength();
                sal_uInt32 nCount2 = aGenericNameSeq.getLength();

                aNameSeq.realloc( nCount1 + nCount2 );
                OUString* pNameSeq = aNameSeq.getArray();
                const OUString* pGenericSeq = aGenericNameSeq.getConstArray();
                for ( sal_uInt32 i = 0; i < nCount2; i++ )
                    pNameSeq[nCount1+i] = pGenericSeq[i];
            }

            return aNameSeq;
        }
        catch( const css::container::NoSuchElementException& )
        {
        }
        catch ( const css::lang::WrappedTargetException& )
        {
        }
    }

    return Sequence< OUString >();
}

bool ConfigurationAccess_UICategory::initializeConfigAccess()
{
    Sequence< Any > aArgs( 1 );
    PropertyValue   aPropValue;

    try
    {
        aPropValue.Name  = "nodepath";
        aPropValue.Value <<= m_aConfigCategoryAccess;
        aArgs[0] <<= aPropValue;

        m_xConfigAccess = Reference< XNameAccess >( m_xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess", aArgs ),UNO_QUERY );
        if ( m_xConfigAccess.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
            if ( xContainer.is() )
            {
                m_xConfigListener = new WeakContainerListener(this);
                xContainer->addContainerListener(m_xConfigListener);
            }
        }

        return true;
    }
    catch ( const WrappedTargetException& )
    {
    }
    catch ( const Exception& )
    {
    }

    return false;
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UICategory::elementInserted( const ContainerEvent& ) throw(RuntimeException, std::exception)
{
}

void SAL_CALL ConfigurationAccess_UICategory::elementRemoved ( const ContainerEvent& ) throw(RuntimeException, std::exception)
{
}

void SAL_CALL ConfigurationAccess_UICategory::elementReplaced( const ContainerEvent& ) throw(RuntimeException, std::exception)
{
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICategory::disposing( const EventObject& aEvent ) throw(RuntimeException, std::exception)
{
    // SAFE
    // remove our reference to the config access
    osl::MutexGuard g(aMutex);

    Reference< XInterface > xIfac1( aEvent.Source, UNO_QUERY );
    Reference< XInterface > xIfac2( m_xConfigAccess, UNO_QUERY );
    if ( xIfac1 == xIfac2 )
        m_xConfigAccess.clear();
}

class UICategoryDescription :  public UICommandDescription
{
public:
    UICategoryDescription( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UICategoryDescription();

    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    {
        return OUString("com.sun.star.comp.framework.UICategoryDescription");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        css::uno::Sequence< OUString > aSeq(1);
        aSeq[0] = "com.sun.star.ui.UICategoryDescription";
        return aSeq;
    }

};

UICategoryDescription::UICategoryDescription( const Reference< XComponentContext >& rxContext ) :
    UICommandDescription(rxContext,true)
{
    Reference< XNameAccess > xEmpty;
    OUString aGenericCategories( "GenericCategories" );
    m_xGenericUICommands = new ConfigurationAccess_UICategory( aGenericCategories, xEmpty, rxContext );

    // insert generic categories mappings
    m_aModuleToCommandFileMap.insert( ModuleToCommandFileMap::value_type(
        OUString(GENERIC_MODULE_NAME ), aGenericCategories ));

    UICommandsHashMap::iterator pCatIter = m_aUICommandsHashMap.find( aGenericCategories );
    if ( pCatIter != m_aUICommandsHashMap.end() )
        pCatIter->second = m_xGenericUICommands;

    impl_fillElements("ooSetupFactoryCmdCategoryConfigRef");
}

UICategoryDescription::~UICategoryDescription()
{
}

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new UICategoryDescription(context)))
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
com_sun_star_comp_framework_UICategoryDescription_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
