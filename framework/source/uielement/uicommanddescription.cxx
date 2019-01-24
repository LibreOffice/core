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

#include <properties.h>

#include <helper/mischelper.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>

#include <rtl/ustrbuf.hxx>
#include <cppuhelper/implbase.hxx>
#include <unotools/configmgr.hxx>

#include <vcl/mnemonic.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::configuration;
using namespace com::sun::star::container;
using namespace ::com::sun::star::frame;

//  Namespace

static const char CONFIGURATION_ROOT_ACCESS[]           = "/org.openoffice.Office.UI.";

// Special resource URLs to retrieve additional information
static const char PRIVATE_RESOURCE_URL[]                = "private:";

const sal_Int32   COMMAND_PROPERTY_IMAGE                = 1;
const sal_Int32   COMMAND_PROPERTY_ROTATE               = 2;
const sal_Int32   COMMAND_PROPERTY_MIRROR               = 4;

namespace framework
{

//  Configuration access class for PopupMenuControllerFactory implementation

class ConfigurationAccess_UICommand : // Order is necessary for right initialization!
                                        public  ::cppu::WeakImplHelper<XNameAccess,XContainerListener>
{
    osl::Mutex m_aMutex;
    public:
                                  ConfigurationAccess_UICommand( const OUString& aModuleName, const Reference< XNameAccess >& xGenericUICommands, const Reference< XComponentContext >& rxContext );
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
        void                      fillInfoFromResult( CmdToInfoMap& rCmdInfo, const OUString& aLabel );
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

//  XInterface, XTypeProvider

ConfigurationAccess_UICommand::ConfigurationAccess_UICommand( const OUString& aModuleName, const Reference< XNameAccess >& rGenericUICommands, const Reference< XComponentContext>& rxContext ) :
    m_aConfigCmdAccess( CONFIGURATION_ROOT_ACCESS ),
    m_aConfigPopupAccess( CONFIGURATION_ROOT_ACCESS ),
    m_aPropProperties( "Properties" ),
    m_xGenericUICommands( rGenericUICommands ),
    m_bConfigAccessInitialized( false ),
    m_bCacheFilled( false ),
    m_bGenericDataRetrieved( false )
{
    // Create configuration hierarchical access name
    m_aConfigCmdAccess += aModuleName + "/UserInterface/Commands";

    m_xConfigProvider = theDefaultProvider::get( rxContext );

    m_aConfigPopupAccess += aModuleName + "/UserInterface/Popups";
}

ConfigurationAccess_UICommand::~ConfigurationAccess_UICommand()
{
    // SAFE
    osl::MutexGuard g(m_aMutex);
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
    osl::MutexGuard g(m_aMutex);
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
            return makeAny( m_aCommandImageList );
        else if ( rCommandURL.equalsIgnoreAsciiCase( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDROTATEIMAGELIST ))
            return makeAny( m_aCommandRotateImageList );
        else if ( rCommandURL.equalsIgnoreAsciiCase( UICOMMANDDESCRIPTION_NAMEACCESS_COMMANDMIRRORIMAGELIST ))
            return makeAny( m_aCommandMirrorImageList );
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

        Sequence< PropertyValue > aPropSeq( 8 );
        aPropSeq[0].Name  = "Label";
        aPropSeq[0].Value = !pIter->second.aContextLabel.isEmpty() ?
                makeAny( pIter->second.aContextLabel ): makeAny( pIter->second.aLabel );
        aPropSeq[1].Name  = "Name";
        aPropSeq[1].Value <<= pIter->second.aCommandName;
        aPropSeq[2].Name  = "Popup";
        aPropSeq[2].Value <<= pIter->second.bPopup;
        aPropSeq[3].Name  = m_aPropProperties;
        aPropSeq[3].Value <<= pIter->second.nProperties;
        aPropSeq[4].Name  = "PopupLabel";
        aPropSeq[4].Value <<= pIter->second.aPopupLabel;
        aPropSeq[5].Name  = "TooltipLabel";
        aPropSeq[5].Value <<= pIter->second.aTooltipLabel;
        aPropSeq[6].Name  = "TargetURL";
        aPropSeq[6].Value <<= pIter->second.aTargetURL;
        aPropSeq[7].Name = "IsExperimental";
        aPropSeq[7].Value <<= pIter->second.bIsExperimental;
        return makeAny( aPropSeq );
    }

    return Any();
}
void ConfigurationAccess_UICommand::impl_fill(const Reference< XNameAccess >& _xConfigAccess,bool _bPopup,
                                                std::vector< OUString >& aImageCommandVector,
                                                std::vector< OUString >& aImageRotateVector,
                                                std::vector< OUString >& aImageMirrorVector)
{
    if ( _xConfigAccess.is() )
    {
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
    if ( m_xGenericUICommands.is() && !m_bGenericDataRetrieved )
    {
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
    osl::MutexGuard g(m_aMutex);

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
    osl::MutexGuard g(m_aMutex);
    m_bCacheFilled = false;
    fillCache();
}

void SAL_CALL ConfigurationAccess_UICommand::elementRemoved( const ContainerEvent& )
{
    osl::MutexGuard g(m_aMutex);
    m_bCacheFilled = false;
    fillCache();
}

void SAL_CALL ConfigurationAccess_UICommand::elementReplaced( const ContainerEvent& )
{
    osl::MutexGuard g(m_aMutex);
    m_bCacheFilled = false;
    fillCache();
}

// lang.XEventListener
void SAL_CALL ConfigurationAccess_UICommand::disposing( const EventObject& aEvent )
{
    // SAFE
    // remove our reference to the config access
    osl::MutexGuard g(m_aMutex);

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

UICommandDescription::UICommandDescription(const Reference< XComponentContext >& rxContext)
    : UICommandDescription_BASE(m_aMutex)
    , m_aPrivateResourceURL(PRIVATE_RESOURCE_URL)
    , m_xContext(rxContext)
{
    Reference< XNameAccess > xEmpty;
    OUString aGenericUICommand( "GenericCommands" );
    m_xGenericUICommands = new ConfigurationAccess_UICommand( aGenericUICommand, xEmpty, m_xContext );

    impl_fillElements("ooSetupFactoryCommandConfigRef");

    // insert generic commands
    UICommandsHashMap::iterator pIter = m_aUICommandsHashMap.find( aGenericUICommand );
    if ( pIter != m_aUICommandsHashMap.end() )
        pIter->second = m_xGenericUICommands;
}

UICommandDescription::UICommandDescription(const Reference< XComponentContext >& rxContext, bool)
    : UICommandDescription_BASE(m_aMutex)
    , m_xContext(rxContext)
{
}

UICommandDescription::~UICommandDescription()
{
    osl::MutexGuard g(rBHelper.rMutex);
    m_aModuleToCommandFileMap.clear();
    m_aUICommandsHashMap.clear();
    m_xGenericUICommands.clear();
}
void UICommandDescription::impl_fillElements(const sal_Char* _pName)
{
    m_xModuleManager.set( ModuleManager::create( m_xContext ) );
    Sequence< OUString > aElementNames = m_xModuleManager->getElementNames();
    Sequence< PropertyValue > aSeq;
    OUString                  aModuleIdentifier;

    for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
    {
        aModuleIdentifier = aElementNames[i];
        if ( m_xModuleManager->getByName( aModuleIdentifier ) >>= aSeq )
        {
            OUString aCommandStr;
            for ( sal_Int32 y = 0; y < aSeq.getLength(); y++ )
            {
                if ( aSeq[y].Name.equalsAscii(_pName) )
                {
                    aSeq[y].Value >>= aCommandStr;
                    break;
                }
            }

            // Create first mapping ModuleIdentifier ==> Command File
            m_aModuleToCommandFileMap.emplace( aModuleIdentifier, aCommandStr );

            // Create second mapping Command File ==> commands instance
            UICommandsHashMap::iterator pIter = m_aUICommandsHashMap.find( aCommandStr );
            if ( pIter == m_aUICommandsHashMap.end() )
                m_aUICommandsHashMap.emplace( aCommandStr, Reference< XNameAccess >() );
        }
    } // for ( sal_Int32 i = 0; i < aElementNames.getLength(); i++ )
}

Any SAL_CALL UICommandDescription::getByName( const OUString& aName )
{
    Any a;

    osl::MutexGuard g(rBHelper.rMutex);

    ModuleToCommandFileMap::const_iterator pM2CIter = m_aModuleToCommandFileMap.find( aName );
    if ( pM2CIter != m_aModuleToCommandFileMap.end() )
    {
        OUString aCommandFile( pM2CIter->second );
        UICommandsHashMap::iterator pIter = m_aUICommandsHashMap.find( aCommandFile );
        if ( pIter != m_aUICommandsHashMap.end() )
        {
            if ( pIter->second.is() )
                a <<= pIter->second;
            else
            {
                Reference< XNameAccess > xUICommands;
                ConfigurationAccess_UICommand* pUICommands = new ConfigurationAccess_UICommand( aCommandFile,
                                                                                               m_xGenericUICommands,
                                                                                               m_xContext );
                xUICommands.set( static_cast< cppu::OWeakObject* >( pUICommands ),UNO_QUERY );
                pIter->second = xUICommands;
                a <<= xUICommands;
            }
        }
    }
    else if ( !m_aPrivateResourceURL.isEmpty() && aName.startsWith( m_aPrivateResourceURL ) )
    {
        // special keys to retrieve information about a set of commands
        return m_xGenericUICommands->getByName( aName );
    }
    else
    {
        throw NoSuchElementException();
    }

    return a;
}

Sequence< OUString > SAL_CALL UICommandDescription::getElementNames()
{
    osl::MutexGuard g(rBHelper.rMutex);

    return comphelper::mapKeysToSequence( m_aModuleToCommandFileMap );
}

sal_Bool SAL_CALL UICommandDescription::hasByName( const OUString& aName )
{
    osl::MutexGuard g(rBHelper.rMutex);

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

namespace {

struct Instance {
    explicit Instance(
        css::uno::Reference<css::uno::XComponentContext> const & context):
        instance(static_cast<cppu::OWeakObject *>(
                    new framework::UICommandDescription(context)))
    {
    }

    css::uno::Reference<css::uno::XInterface> instance;
};

struct Singleton:
    public rtl::StaticWithArg<
        Instance, css::uno::Reference<css::uno::XComponentContext>, Singleton>
{};

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_UICommandDescription_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(static_cast<cppu::OWeakObject *>(
                Singleton::get(context).instance.get()));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
