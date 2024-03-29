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

#include <helper/mischelper.hxx>

#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <sal/log.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/syslocale.hxx>

#include <comphelper/propertysequence.hxx>

#include <string_view>
#include <unordered_map>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace framework;

namespace {

class ConfigurationAccess_UICategory : public ::cppu::WeakImplHelper<XNameAccess,XContainerListener>
{
    std::mutex aMutex;
    public:
                                  ConfigurationAccess_UICategory( std::u16string_view aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XComponentContext >& rxContext );
        virtual                   ~ConfigurationAccess_UICategory() override;

        // XNameAccess
        virtual css::uno::Any SAL_CALL getByName( const OUString& aName ) override;

        virtual css::uno::Sequence< OUString > SAL_CALL getElementNames() override;

        virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override;

        // XElementAccess
        virtual css::uno::Type SAL_CALL getElementType() override;

        virtual sal_Bool SAL_CALL hasElements() override;

        // container.XContainerListener
        virtual void SAL_CALL     elementInserted( const ContainerEvent& aEvent ) override;
        virtual void SAL_CALL     elementRemoved ( const ContainerEvent& aEvent ) override;
        virtual void SAL_CALL     elementReplaced( const ContainerEvent& aEvent ) override;

        // lang.XEventListener
        virtual void SAL_CALL disposing( const EventObject& aEvent ) override;

    protected:
        Any                       getUINameFromID( const OUString& rId );
        Any                       getUINameFromCache( const OUString& rId );
        Sequence< OUString > getAllIds();
        void                  fillCache();

    private:
        typedef std::unordered_map< OUString,
                                    OUString > IdToInfoCache;

        void initializeConfigAccess();

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

ConfigurationAccess_UICategory::ConfigurationAccess_UICategory( std::u16string_view aModuleName, const Reference< XNameAccess >& rGenericUICategories, const Reference< XComponentContext >& rxContext ) :
    // Create configuration hierarchical access name
    m_aConfigCategoryAccess(
        OUString::Concat("/org.openoffice.Office.UI.") + aModuleName + "/Commands/Categories"),
    m_aPropUIName( "Name" ),
    m_xGenericUICategories( rGenericUICategories ),
    m_xConfigProvider(theDefaultProvider::get( rxContext )),
    m_bConfigAccessInitialized( false ),
    m_bCacheFilled( false )
{
}

ConfigurationAccess_UICategory::~ConfigurationAccess_UICategory()
{
    // SAFE
    std::unique_lock g(aMutex);
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
}

// XNameAccess
Any SAL_CALL ConfigurationAccess_UICategory::getByName( const OUString& rId )
{
    std::unique_lock g(aMutex);
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
{
    return getAllIds();
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasByName( const OUString& rId )
{
    return getByName( rId ).hasValue();
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICategory::getElementType()
{
    return cppu::UnoType<OUString>::get();
}

sal_Bool SAL_CALL ConfigurationAccess_UICategory::hasElements()
{
    // There must be global categories!
    return true;
}

void ConfigurationAccess_UICategory::fillCache()
{
    SAL_INFO( "fwk", "framework (cd100003) ::ConfigurationAccess_UICategory::fillCache" );

    if ( m_bCacheFilled )
        return;

    OUString        aUIName;
    const Sequence< OUString > aNameSeq = m_xConfigAccess->getElementNames();

    for ( OUString const & rName : aNameSeq )
    {
        try
        {
            Reference< XNameAccess > xNameAccess(m_xConfigAccess->getByName( rName ),UNO_QUERY);
            if ( xNameAccess.is() )
            {
                xNameAccess->getByName( m_aPropUIName ) >>= aUIName;

                m_aIdCache.emplace( rName, aUIName );
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
    std::unique_lock g(aMutex);

    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = true;
        fillCache();
    }

    if ( m_xConfigAccess.is() )
    {
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

void ConfigurationAccess_UICategory::initializeConfigAccess()
{
    try
    {
        Sequence<Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(m_aConfigCategoryAccess)}
        }));

        m_xConfigAccess.set( m_xConfigProvider->createInstanceWithArguments(
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
    }
    catch ( const WrappedTargetException& )
    {
    }
    catch ( const Exception& )
    {
    }
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UICategory::elementInserted( const ContainerEvent& )
{
}

void SAL_CALL ConfigurationAccess_UICategory::elementRemoved ( const ContainerEvent& )
{
}

void SAL_CALL ConfigurationAccess_UICategory::elementReplaced( const ContainerEvent& )
{
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICategory::disposing( const EventObject& aEvent )
{
    // SAFE
    // remove our reference to the config access
    std::unique_lock g(aMutex);

    Reference< XInterface > xIfac1( aEvent.Source, UNO_QUERY );
    Reference< XInterface > xIfac2( m_xConfigAccess, UNO_QUERY );
    if ( xIfac1 == xIfac2 )
        m_xConfigAccess.clear();
}

class UICategoryDescription :  public UICommandDescription
{
public:
    explicit UICategoryDescription( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual OUString SAL_CALL getImplementationName() override
    {
        return "com.sun.star.comp.framework.UICategoryDescription";
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.ui.UICategoryDescription"};
    }

};

UICategoryDescription::UICategoryDescription( const Reference< XComponentContext >& rxContext ) :
    UICommandDescription(rxContext,true)
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    Reference< XNameAccess > xEmpty;
    OUString aGenericCategories( "GenericCategories" );
    m_xGenericUICommands[rCurrentLanguage] = new ConfigurationAccess_UICategory( aGenericCategories, xEmpty, rxContext );

    // insert generic categories mappings
    m_aModuleToCommandFileMap.emplace( OUString("generic"), aGenericCategories );

    auto& rMap = m_aUICommandsHashMap[rCurrentLanguage];
    UICommandsHashMap::iterator pCatIter = rMap.find( aGenericCategories );
    if ( pCatIter != rMap.end() )
        pCatIter->second = m_xGenericUICommands[rCurrentLanguage];

    impl_fillElements("ooSetupFactoryCmdCategoryConfigRef");
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_UICategoryDescription_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UICategoryDescription(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
