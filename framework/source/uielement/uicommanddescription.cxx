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

#include <uielement/uicommanddescription.hxx>

#include <properties.h>

#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <cppuhelper/implbase.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/syslocale.hxx>

#include <vcl/mnemonic.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//  Namespace

const char CONFIGURATION_ROOT_ACCESS[]           = "/org.openoffice.Office.UI.";

// Special resource URLs to retrieve additional information
constexpr OUString PRIVATE_RESOURCE_URL       = u"private:"_ustr;

const sal_Int32   COMMAND_PROPERTY_IMAGE                = 1;
const sal_Int32   COMMAND_PROPERTY_ROTATE               = 2;
const sal_Int32   COMMAND_PROPERTY_MIRROR               = 4;

namespace framework
{

//  Configuration access class for PopupMenuControllerFactory implementation

namespace {

class ConfigurationAccess_UICommand : // Order is necessary for right initialization!
                                        public  ::cppu::WeakImplHelper<XNameAccess,XContainerListener>
{
    std::mutex m_aMutex;
    public:
                                  ConfigurationAccess_UICommand( std::u16string_view aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XComponentContext >& rxContext );
        virtual                   ~ConfigurationAccess_UICommand() override;

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
        css::uno::Any getByNameImpl( const OUString& aName );

        struct CmdToInfoMap
        {
            CmdToInfoMap() : bPopup( false ),
                             bCommandNameCreated( false ),
                             bIsExperimental( false ),
                             nProperties( 0 ) {}

            OUString            aLabel;
            OUString            aContextLabel;
            OUString            aCommandName;
            OUString            aPopupLabel;
            OUString            aTooltipLabel;
            OUString            aTargetURL;
            bool                bPopup : 1,
                                bCommandNameCreated : 1;
            bool                bIsExperimental;
            sal_Int32           nProperties;
        };

        Any                       getSequenceFromCache( const OUString& aCommandURL );
        Any                       getInfoFromCommand( const OUString& rCommandURL );
        static void               fillInfoFromResult( CmdToInfoMap& rCmdInfo, const OUString& aLabel );
        Sequence< OUString > getAllCommands();
        void                  fillCache();
        void                  addGenericInfoToCache();
        void                      impl_fill(const Reference< XNameAccess >& _xConfigAccess,bool _bPopup,
                                                std::vector< OUString >& aImageCommandVector,
                                                std::vector< OUString >& aImageRotateVector,
                                                std::vector< OUString >& aImageMirrorVector);

    private:
        typedef std::unordered_map< OUString,
                                    CmdToInfoMap > CommandToInfoCache;

        void initializeConfigAccess();

        OUString                     m_aConfigCmdAccess;
        OUString                     m_aConfigPopupAccess;
        OUString                     m_aPropProperties;
        Reference< XNameAccess >          m_xGenericUICommands;
        Reference< XMultiServiceFactory > m_xConfigProvider;
        Reference< XNameAccess >          m_xConfigAccess;
        Reference< XContainerListener >   m_xConfigListener;
        Reference< XNameAccess >          m_xConfigAccessPopups;
        Reference< XContainerListener >   m_xConfigAccessListener;
        Sequence< OUString >         m_aCommandImageList;
        Sequence< OUString >         m_aCommandRotateImageList;
        Sequence< OUString >         m_aCommandMirrorImageList;
        CommandToInfoCache                m_aCmdInfoCache;
        bool                          m_bConfigAccessInitialized;
        bool                          m_bCacheFilled;
        bool                          m_bGenericDataRetrieved;
};

}


//  XInterface, XTypeProvider

ConfigurationAccess_UICommand::ConfigurationAccess_UICommand( std::u16string_view aModuleName, const Reference< XNameAccess >& rGenericUICommands, const Reference< XComponentContext>& rxContext ) :
    // Create configuration hierarchical access name
    m_aConfigCmdAccess(
        OUString::Concat(CONFIGURATION_ROOT_ACCESS) + aModuleName + "/UserInterface/Commands"),
    m_aConfigPopupAccess(
        OUString::Concat(CONFIGURATION_ROOT_ACCESS) + aModuleName + "/UserInterface/Popups"),
    m_aPropProperties( "Properties" ),
    m_xGenericUICommands( rGenericUICommands ),
    m_xConfigProvider( theDefaultProvider::get( rxContext ) ),
    m_bConfigAccessInitialized( false ),
    m_bCacheFilled( false ),
    m_bGenericDataRetrieved( false )
{
}

ConfigurationAccess_UICommand::~ConfigurationAccess_UICommand()
{
    // SAFE
    std::unique_lock g(m_aMutex);
    Reference< XContainer > xContainer( m_xConfigAccess, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigListener);
    xContainer.set( m_xConfigAccessPopups, UNO_QUERY );
    if ( xContainer.is() )
        xContainer->removeContainerListener(m_xConfigAccessListener);
}

// XNameAccess
Any ConfigurationAccess_UICommand::getByNameImpl( const OUString& rCommandURL )
{
    std::unique_lock g(m_aMutex);
    if ( !m_bConfigAccessInitialized )
    {
        initializeConfigAccess();
        m_bConfigAccessInitialized = true;
        fillCache();
    }

    if ( rCommandURL.startsWith( PRIVATE_RESOURCE_URL ) )
    {
        // special keys to retrieve information about a set of commands
        // SAFE
        addGenericInfoToCache();

        if ( rCommandURL.equalsIgnoreAsciiCase( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDIMAGELIST ))
            return Any( m_aCommandImageList );
        else if ( rCommandURL.equalsIgnoreAsciiCase( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST ))
            return Any( m_aCommandRotateImageList );
        else if ( rCommandURL.equalsIgnoreAsciiCase( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST ))
            return Any( m_aCommandMirrorImageList );
        else
            return Any();
    }
    else
    {
        // SAFE
        return getInfoFromCommand( rCommandURL );
    }
}

Any SAL_CALL ConfigurationAccess_UICommand::getByName( const OUString& rCommandURL )
{
    Any aRet( getByNameImpl( rCommandURL ) );
    if( !aRet.hasValue() )
        throw NoSuchElementException();

    return aRet;
}

Sequence< OUString > SAL_CALL ConfigurationAccess_UICommand::getElementNames()
{
    return getAllCommands();
}

sal_Bool SAL_CALL ConfigurationAccess_UICommand::hasByName( const OUString& rCommandURL )
{
    return getByNameImpl( rCommandURL ).hasValue();
}

// XElementAccess
Type SAL_CALL ConfigurationAccess_UICommand::getElementType()
{
    return cppu::UnoType<Sequence< PropertyValue >>::get();
}

sal_Bool SAL_CALL ConfigurationAccess_UICommand::hasElements()
{
    // There must are global commands!
    return true;
}

// static
void ConfigurationAccess_UICommand::fillInfoFromResult( CmdToInfoMap& rCmdInfo, const OUString& aLabel )
{
    OUString aStr(aLabel.replaceAll("%PRODUCTNAME", utl::ConfigManager::getProductName()));
    rCmdInfo.aLabel = aStr;
    aStr = comphelper::string::stripEnd(aStr, '.'); // Remove "..." from string
    rCmdInfo.aCommandName = MnemonicGenerator::EraseAllMnemonicChars(aStr);
    rCmdInfo.bCommandNameCreated = true;
}

Any ConfigurationAccess_UICommand::getSequenceFromCache( const OUString& aCommandURL )
{
    CommandToInfoCache::iterator pIter = m_aCmdInfoCache.find( aCommandURL );
    if ( pIter != m_aCmdInfoCache.end() )
    {
        if ( !pIter->second.bCommandNameCreated )
            fillInfoFromResult( pIter->second, pIter->second.aLabel );

        static constexpr OUString sLabel = u"Label"_ustr;
        static constexpr OUString sName = u"Name"_ustr;
        static constexpr OUString sPopup = u"Popup"_ustr;
        static constexpr OUString sPopupLabel = u"PopupLabel"_ustr;
        static constexpr OUString sTooltipLabel = u"TooltipLabel"_ustr;
        static constexpr OUString sTargetURL = u"TargetURL"_ustr;
        static constexpr OUString sIsExperimental = u"IsExperimental"_ustr;
        Sequence< PropertyValue > aPropSeq{
            comphelper::makePropertyValue(sLabel, !pIter->second.aContextLabel.isEmpty()
                                                       ? Any(pIter->second.aContextLabel)
                                                       : Any(pIter->second.aLabel)),
            comphelper::makePropertyValue(sName, pIter->second.aCommandName),
            comphelper::makePropertyValue(sPopup, pIter->second.bPopup),
            comphelper::makePropertyValue(m_aPropProperties, pIter->second.nProperties),
            comphelper::makePropertyValue(sPopupLabel, pIter->second.aPopupLabel),
            comphelper::makePropertyValue(sTooltipLabel, pIter->second.aTooltipLabel),
            comphelper::makePropertyValue(sTargetURL, pIter->second.aTargetURL),
            comphelper::makePropertyValue(sIsExperimental, pIter->second.bIsExperimental)
        };
        return Any( aPropSeq );
    }

    return Any();
}
void ConfigurationAccess_UICommand::impl_fill(const Reference< XNameAccess >& _xConfigAccess,bool _bPopup,
                                                std::vector< OUString >& aImageCommandVector,
                                                std::vector< OUString >& aImageRotateVector,
                                                std::vector< OUString >& aImageMirrorVector)
{
    if ( !_xConfigAccess.is() )
        return;

    Sequence< OUString> aNameSeq = _xConfigAccess->getElementNames();
    const sal_Int32 nCount = aNameSeq.getLength();
    for ( sal_Int32 i = 0; i < nCount; i++ )
    {
        try
        {
            Reference< XNameAccess > xNameAccess(_xConfigAccess->getByName( aNameSeq[i] ),UNO_QUERY);
            if ( xNameAccess.is() )
            {
                CmdToInfoMap aCmdToInfo;

                aCmdToInfo.bPopup = _bPopup;
                xNameAccess->getByName( "Label" )           >>= aCmdToInfo.aLabel;
                xNameAccess->getByName( "ContextLabel" )    >>= aCmdToInfo.aContextLabel;
                xNameAccess->getByName( "PopupLabel" )      >>= aCmdToInfo.aPopupLabel;
                xNameAccess->getByName( "TooltipLabel" )    >>= aCmdToInfo.aTooltipLabel;
                xNameAccess->getByName( "TargetURL" )       >>= aCmdToInfo.aTargetURL;
                xNameAccess->getByName( "IsExperimental" )  >>= aCmdToInfo.bIsExperimental;
                xNameAccess->getByName( m_aPropProperties ) >>= aCmdToInfo.nProperties;

                m_aCmdInfoCache.emplace( aNameSeq[i], aCmdToInfo );

                if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_IMAGE )
                    aImageCommandVector.push_back( aNameSeq[i] );
                if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_ROTATE )
                    aImageRotateVector.push_back( aNameSeq[i] );
                if ( aCmdToInfo.nProperties & COMMAND_PROPERTY_MIRROR )
                    aImageMirrorVector.push_back( aNameSeq[i] );
            }
        }
        catch (const css::lang::WrappedTargetException&)
        {
        }
        catch (const css::container::NoSuchElementException&)
        {
        }
    }
}
void ConfigurationAccess_UICommand::fillCache()
{

    if ( m_bCacheFilled )
        return;

    std::vector< OUString > aImageCommandVector;
    std::vector< OUString > aImageRotateVector;
    std::vector< OUString > aImageMirrorVector;

    impl_fill(m_xConfigAccess,false,aImageCommandVector,aImageRotateVector,aImageMirrorVector);
    impl_fill(m_xConfigAccessPopups,true,aImageCommandVector,aImageRotateVector,aImageMirrorVector);
    // Create cached sequences for fast retrieving
    m_aCommandImageList       = comphelper::containerToSequence( aImageCommandVector );
    m_aCommandRotateImageList = comphelper::containerToSequence( aImageRotateVector );
    m_aCommandMirrorImageList = comphelper::containerToSequence( aImageMirrorVector );

    m_bCacheFilled = true;
}

void ConfigurationAccess_UICommand::addGenericInfoToCache()
{
    if ( !m_xGenericUICommands.is() || m_bGenericDataRetrieved )
        return;

    Sequence< OUString > aCommandNameSeq;
    try
    {
        if ( m_xGenericUICommands->getByName(
                UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST ) >>= aCommandNameSeq )
            m_aCommandRotateImageList = comphelper::concatSequences< OUString >( m_aCommandRotateImageList, aCommandNameSeq );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
    }

    try
    {
        if ( m_xGenericUICommands->getByName(
                UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST ) >>= aCommandNameSeq )
            m_aCommandMirrorImageList = comphelper::concatSequences< OUString >( m_aCommandMirrorImageList, aCommandNameSeq );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
    }

    m_bGenericDataRetrieved = true;
}

Any ConfigurationAccess_UICommand::getInfoFromCommand( const OUString& rCommandURL )
{
    Any a;

    try
    {
        a = getSequenceFromCache( rCommandURL );
        if ( !a.hasValue() )
        {
            // First try to ask our global commands configuration access. It also caches maybe
            // we find the entry in its cache first.
            if ( m_xGenericUICommands.is() && m_xGenericUICommands->hasByName( rCommandURL ) )
            {
                try
                {
                    return m_xGenericUICommands->getByName( rCommandURL );
                }
                catch (const css::lang::WrappedTargetException&)
                {
                }
                catch (const css::container::NoSuchElementException&)
                {
                }
            }
        }
    }
    catch (const css::container::NoSuchElementException&)
    {
    }
    catch (const css::lang::WrappedTargetException&)
    {
    }

    return a;
}

Sequence< OUString > ConfigurationAccess_UICommand::getAllCommands()
{
    // SAFE
    std::unique_lock g(m_aMutex);

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

            if ( m_xGenericUICommands.is() )
            {
                // Create concat list of supported user interface commands of the module
                Sequence< OUString > aGenericNameSeq = m_xGenericUICommands->getElementNames();
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
        catch (const css::container::NoSuchElementException&)
        {
        }
        catch (const css::lang::WrappedTargetException&)
        {
        }
    }

    return Sequence< OUString >();
}

void ConfigurationAccess_UICommand::initializeConfigAccess()
{
    try
    {
        Sequence<Any> aArgs(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(m_aConfigCmdAccess)}
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

        Sequence<Any> aArgs2(comphelper::InitAnyPropertySequence(
        {
            {"nodepath", Any(m_aConfigPopupAccess)}
        }));
        m_xConfigAccessPopups.set( m_xConfigProvider->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess", aArgs2 ),UNO_QUERY );
        if ( m_xConfigAccessPopups.is() )
        {
            // Add as container listener
            Reference< XContainer > xContainer( m_xConfigAccessPopups, UNO_QUERY );
            if ( xContainer.is() )
            {
                m_xConfigAccessListener = new WeakContainerListener(this);
                xContainer->addContainerListener(m_xConfigAccessListener);
            }
        }
    }
    catch (const WrappedTargetException&)
    {
    }
    catch (const Exception&)
    {
    }
}

// container.XContainerListener
void SAL_CALL ConfigurationAccess_UICommand::elementInserted( const ContainerEvent& )
{
    std::unique_lock g(m_aMutex);
    m_bCacheFilled = false;
    fillCache();
}

void SAL_CALL ConfigurationAccess_UICommand::elementRemoved( const ContainerEvent& )
{
    std::unique_lock g(m_aMutex);
    m_bCacheFilled = false;
    fillCache();
}

void SAL_CALL ConfigurationAccess_UICommand::elementReplaced( const ContainerEvent& )
{
    std::unique_lock g(m_aMutex);
    m_bCacheFilled = false;
    fillCache();
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICommand::disposing( const EventObject& aEvent )
{
    // SAFE
    // remove our reference to the config access
    std::unique_lock g(m_aMutex);

    Reference< XInterface > xIfac1( aEvent.Source, UNO_QUERY );
    Reference< XInterface > xIfac2( m_xConfigAccess, UNO_QUERY );
    if ( xIfac1 == xIfac2 )
        m_xConfigAccess.clear();
    else
    {
        xIfac1.set( m_xConfigAccessPopups, UNO_QUERY );
        if ( xIfac1 == xIfac2 )
            m_xConfigAccessPopups.clear();
    }
}

void UICommandDescription::ensureGenericUICommandsForLanguage(const LanguageTag& rLanguage)
{
    auto xGenericUICommands = m_xGenericUICommands.find(rLanguage);
    if (xGenericUICommands == m_xGenericUICommands.end())
    {
        Reference< XNameAccess > xEmpty;
        m_xGenericUICommands[rLanguage] = new ConfigurationAccess_UICommand( u"GenericCommands", xEmpty, m_xContext );
    }
}

UICommandDescription::UICommandDescription(const Reference< XComponentContext >& rxContext)
    : m_aPrivateResourceURL(PRIVATE_RESOURCE_URL)
    , m_xContext(rxContext)
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();

    ensureGenericUICommandsForLanguage(rCurrentLanguage);

    impl_fillElements("ooSetupFactoryCommandConfigRef");

    // insert generic commands
    auto& rMap = m_aUICommandsHashMap[rCurrentLanguage];
    UICommandsHashMap::iterator pIter = rMap.find( "GenericCommands" );
    if ( pIter != rMap.end() )
        pIter->second = m_xGenericUICommands[rCurrentLanguage];
}

UICommandDescription::UICommandDescription(const Reference< XComponentContext >& rxContext, bool)
    : m_xContext(rxContext)
{
}

UICommandDescription::~UICommandDescription()
{
    std::unique_lock g(m_aMutex);
    m_aModuleToCommandFileMap.clear();
    m_aUICommandsHashMap.clear();
    m_xGenericUICommands.clear();
}
void UICommandDescription::impl_fillElements(const char* _pName)
{
    m_xModuleManager.set( ModuleManager::create( m_xContext ) );
    const Sequence< OUString > aElementNames = m_xModuleManager->getElementNames();

    SvtSysLocale aSysLocale;

    for ( OUString const & aModuleIdentifier : aElementNames )
    {
        Sequence< PropertyValue > aSeq;
        if ( m_xModuleManager->getByName( aModuleIdentifier ) >>= aSeq )
        {
            OUString aCommandStr;
            for (PropertyValue const& prop : aSeq)
            {
                if ( prop.Name.equalsAscii(_pName) )
                {
                    prop.Value >>= aCommandStr;
                    break;
                }
            }

            // Create first mapping ModuleIdentifier ==> Command File
            m_aModuleToCommandFileMap.emplace( aModuleIdentifier, aCommandStr );

            // Create second mapping Command File ==> commands instance
            const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
            auto& rMap = m_aUICommandsHashMap[rCurrentLanguage];
            UICommandsHashMap::iterator pIter = rMap.find( aCommandStr );
            if ( pIter == rMap.end() )
                rMap.emplace( aCommandStr, Reference< XNameAccess >() );
        }
    } // for ( sal_Int32 i = 0; i < aElementNames.(); i++ )
}

Any SAL_CALL UICommandDescription::getByName( const OUString& aName )
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    Any a;

    std::unique_lock g(m_aMutex);

    ModuleToCommandFileMap::const_iterator pM2CIter = m_aModuleToCommandFileMap.find( aName );
    if ( pM2CIter != m_aModuleToCommandFileMap.end() )
    {
        OUString aCommandFile( pM2CIter->second );
        auto pMapIter = m_aUICommandsHashMap.find( rCurrentLanguage );
        if ( pMapIter == m_aUICommandsHashMap.end() )
            impl_fillElements("ooSetupFactoryCommandConfigRef");

        auto& rMap = m_aUICommandsHashMap[rCurrentLanguage];
        UICommandsHashMap::iterator pIter = rMap.find( aCommandFile );
        if ( pIter != rMap.end() )
        {
            if ( pIter->second.is() )
                a <<= pIter->second;
            else
            {
                ensureGenericUICommandsForLanguage(rCurrentLanguage);

                Reference< XNameAccess > xUICommands = new ConfigurationAccess_UICommand( aCommandFile,
                                                                                          m_xGenericUICommands[rCurrentLanguage],
                                                                                          m_xContext );
                pIter->second = xUICommands;
                a <<= xUICommands;
            }
        }
    }
    else if ( !m_aPrivateResourceURL.isEmpty() && aName.startsWith( m_aPrivateResourceURL ) )
    {
        ensureGenericUICommandsForLanguage(rCurrentLanguage);

        // special keys to retrieve information about a set of commands
        return m_xGenericUICommands[rCurrentLanguage]->getByName( aName );
    }
    else
    {
        throw NoSuchElementException();
    }

    return a;
}

Sequence< OUString > SAL_CALL UICommandDescription::getElementNames()
{
    std::unique_lock g(m_aMutex);

    return comphelper::mapKeysToSequence( m_aModuleToCommandFileMap );
}

sal_Bool SAL_CALL UICommandDescription::hasByName( const OUString& aName )
{
    std::unique_lock g(m_aMutex);

    ModuleToCommandFileMap::const_iterator pIter = m_aModuleToCommandFileMap.find( aName );
    return ( pIter != m_aModuleToCommandFileMap.end() );
}

// XElementAccess
Type SAL_CALL UICommandDescription::getElementType()
{
    return cppu::UnoType<XNameAccess>::get();
}

sal_Bool SAL_CALL UICommandDescription::hasElements()
{
    // generic UI commands are always available!
    return true;
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_UICommandDescription_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new framework::UICommandDescription(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
