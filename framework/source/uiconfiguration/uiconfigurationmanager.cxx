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

#include <uiconfiguration/imagemanager.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/uielementtypenames.hxx>
#include <menuconfiguration.hxx>
#include <statusbarconfiguration.hxx>
#include <toolboxconfiguration.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/InvalidStorageException.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/ui/DocumentAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationManager2.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

#include <mutex>
#include <string_view>
#include <unordered_map>

using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::embed;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace com::sun::star::ui;
using namespace framework;

namespace {

class UIConfigurationManager :   public ::cppu::WeakImplHelper<
                                        css::lang::XServiceInfo  ,
                                        css::ui::XUIConfigurationManager2 >
{
public:
    virtual OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.framework.UIConfigurationManager"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {u"com.sun.star.ui.UIConfigurationManager"_ustr};
    }

    explicit UIConfigurationManager( css::uno::Reference< css::uno::XComponentContext > xContext );

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XUIConfiguration
    virtual void SAL_CALL addConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener ) override;
    virtual void SAL_CALL removeConfigurationListener( const css::uno::Reference< css::ui::XUIConfigurationListener >& Listener ) override;

    // XUIConfigurationManager
    virtual void SAL_CALL reset() override;
    virtual css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > SAL_CALL getUIElementsInfo( sal_Int16 ElementType ) override;
    virtual css::uno::Reference< css::container::XIndexContainer > SAL_CALL createSettings(  ) override;
    virtual sal_Bool SAL_CALL hasSettings( const OUString& ResourceURL ) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getSettings( const OUString& ResourceURL, sal_Bool bWriteable ) override;
    virtual void SAL_CALL replaceSettings( const OUString& ResourceURL, const css::uno::Reference< css::container::XIndexAccess >& aNewData ) override;
    virtual void SAL_CALL removeSettings( const OUString& ResourceURL ) override;
    virtual void SAL_CALL insertSettings( const OUString& NewResourceURL, const css::uno::Reference< css::container::XIndexAccess >& aNewData ) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getImageManager() override;
    virtual css::uno::Reference< css::ui::XAcceleratorConfiguration > SAL_CALL getShortCutManager() override;
    virtual css::uno::Reference< css::ui::XAcceleratorConfiguration > SAL_CALL createShortCutManager() override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getEventsManager() override;

    // XUIConfigurationPersistence
    virtual void SAL_CALL reload() override;
    virtual void SAL_CALL store() override;
    virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& Storage ) override;
    virtual sal_Bool SAL_CALL isModified() override;
    virtual sal_Bool SAL_CALL isReadOnly() override;

    // XUIConfigurationStorage
    virtual void SAL_CALL setStorage( const css::uno::Reference< css::embed::XStorage >& Storage ) override;
    virtual sal_Bool SAL_CALL hasStorage() override;

private:
    // private data types
    enum NotifyOp
    {
        NotifyOp_Remove,
        NotifyOp_Insert,
        NotifyOp_Replace
    };

    struct UIElementInfo
    {
        UIElementInfo( OUString _aResourceURL, OUString _aUIName ) :
            aResourceURL(std::move( _aResourceURL)), aUIName(std::move( _aUIName )) {}
        OUString   aResourceURL;
        OUString   aUIName;
    };

    struct UIElementData
    {
        UIElementData() : bModified( false ), bDefault( true ) {};

        OUString aResourceURL;
        OUString aName;
        bool          bModified;        // has been changed since last storing
        bool          bDefault;         // default settings
        css::uno::Reference< css::container::XIndexAccess > xSettings;
    };

    struct UIElementType;
    friend struct UIElementType;
    typedef std::unordered_map< OUString, UIElementData > UIElementDataHashMap;

    struct UIElementType
    {
        UIElementType() : bModified( false ),
                          bLoaded( false ),
                          nElementType( css::ui::UIElementType::UNKNOWN ) {}

        bool                                                              bModified;
        bool                                                              bLoaded;
        sal_Int16                                                         nElementType;
        UIElementDataHashMap                                              aElementsHashMap;
        css::uno::Reference< css::embed::XStorage > xStorage;
    };

    typedef std::vector< UIElementType > UIElementTypesVector;
    typedef std::vector< css::ui::ConfigurationEvent > ConfigEventNotifyContainer;
    typedef std::unordered_map< OUString, UIElementInfo > UIElementInfoHashMap;

    void            impl_Initialize();
    void            implts_notifyContainerListener( const css::ui::ConfigurationEvent& aEvent, NotifyOp eOp );
    void            impl_fillSequenceWithElementTypeInfo( UIElementInfoHashMap& aUIElementInfoCollection, sal_Int16 nElementType );
    void            impl_preloadUIElementTypeList( sal_Int16 nElementType );
    UIElementData*  impl_findUIElementData( const OUString& aResourceURL, sal_Int16 nElementType, bool bLoad = true );
    void            impl_requestUIElementData( sal_Int16 nElementType, UIElementData& aUIElementData );
    void            impl_storeElementTypeData( css::uno::Reference< css::embed::XStorage > const & xStorage, UIElementType& rElementType, bool bResetModifyState = true );
    void            impl_resetElementTypeData( UIElementType& rDocElementType, ConfigEventNotifyContainer& rRemoveNotifyContainer );
    void            impl_reloadElementTypeData( UIElementType& rDocElementType, ConfigEventNotifyContainer& rRemoveNotifyContainer, ConfigEventNotifyContainer& rReplaceNotifyContainer );

    UIElementTypesVector                                      m_aUIElements;
    css::uno::Reference< css::embed::XStorage >               m_xDocConfigStorage;
    bool                                                      m_bReadOnly;
    bool                                                      m_bModified;
    bool                                                      m_bDisposed;
    OUString                                                  m_aPropUIName;
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    std::mutex                                                m_mutex;
    comphelper::OInterfaceContainerHelper4<css::lang::XEventListener>               m_aEventListeners;
    comphelper::OInterfaceContainerHelper4<css::ui::XUIConfigurationListener>       m_aConfigListeners;
    rtl::Reference< ImageManager >                            m_xImageManager;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xAccConfig;
};

// important: The order and position of the elements must match the constant
// definition of "css::ui::UIElementType"
std::u16string_view UIELEMENTTYPENAMES[] =
{
    u"",  // Dummy value for unknown!
    u"" UIELEMENTTYPE_MENUBAR_NAME,
    u"" UIELEMENTTYPE_POPUPMENU_NAME,
    u"" UIELEMENTTYPE_TOOLBAR_NAME,
    u"" UIELEMENTTYPE_STATUSBAR_NAME,
    u"" UIELEMENTTYPE_FLOATINGWINDOW_NAME,
    u"" UIELEMENTTYPE_PROGRESSBAR_NAME,
    u"" UIELEMENTTYPE_TOOLPANEL_NAME
};

constexpr std::u16string_view RESOURCEURL_PREFIX = u"private:resource/";

sal_Int16 RetrieveTypeFromResourceURL( std::u16string_view aResourceURL )
{

    if (( o3tl::starts_with(aResourceURL, RESOURCEURL_PREFIX ) ) &&
        ( aResourceURL.size() > RESOURCEURL_PREFIX.size() ))
    {
        std::u16string_view aTmpStr = aResourceURL.substr( RESOURCEURL_PREFIX.size() );
        size_t nIndex = aTmpStr.find( '/' );
        if (( nIndex > 0 ) &&  ( aTmpStr.size() > nIndex ))
        {
            std::u16string_view aTypeStr( aTmpStr.substr( 0, nIndex ));
            for ( int i = 0; i < UIElementType::COUNT; i++ )
            {
                if ( aTypeStr == UIELEMENTTYPENAMES[i] )
                    return sal_Int16( i );
            }
        }
    }

    return UIElementType::UNKNOWN;
}

OUString RetrieveNameFromResourceURL( std::u16string_view aResourceURL )
{
    if (( o3tl::starts_with(aResourceURL, RESOURCEURL_PREFIX ) ) &&
        ( aResourceURL.size() > RESOURCEURL_PREFIX.size() ))
    {
        size_t nIndex = aResourceURL.rfind( '/' );
        if ( (nIndex > 0) && (nIndex != std::u16string_view::npos) && (( nIndex+1 ) < aResourceURL.size()) )
            return OUString(aResourceURL.substr( nIndex+1 ));
    }

    return OUString();
}

void UIConfigurationManager::impl_fillSequenceWithElementTypeInfo( UIElementInfoHashMap& aUIElementInfoCollection, sal_Int16 nElementType )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( nElementType );

    UIElementDataHashMap& rUserElements = m_aUIElements[nElementType].aElementsHashMap;

    for (auto const& elem : rUserElements)
    {
        UIElementData* pDataSettings = impl_findUIElementData( elem.second.aResourceURL, nElementType );
        if ( pDataSettings && !pDataSettings->bDefault )
        {
            // Retrieve user interface name from XPropertySet interface
            OUString aUIName;
            Reference< XPropertySet > xPropSet( pDataSettings->xSettings, UNO_QUERY );
            if ( xPropSet.is() )
            {
                Any a = xPropSet->getPropertyValue( m_aPropUIName );
                a >>= aUIName;
            }

            UIElementInfo aInfo( elem.second.aResourceURL, aUIName );
            aUIElementInfoCollection.emplace( elem.second.aResourceURL, aInfo );
        }
    }
}

void UIConfigurationManager::impl_preloadUIElementTypeList( sal_Int16 nElementType )
{
    UIElementType& rElementTypeData = m_aUIElements[nElementType];

    if ( !rElementTypeData.bLoaded )
    {
        Reference< XStorage > xElementTypeStorage = rElementTypeData.xStorage;
        if ( xElementTypeStorage.is() )
        {
            OUString aResURLPrefix =
                OUString::Concat(RESOURCEURL_PREFIX) +
                UIELEMENTTYPENAMES[ nElementType ] +
                "/";

            UIElementDataHashMap& rHashMap = rElementTypeData.aElementsHashMap;
            const Sequence< OUString > aUIElementNames = xElementTypeStorage->getElementNames();
            for ( OUString const & rElementName : aUIElementNames )
            {
                UIElementData aUIElementData;

                // Resource name must be without ".xml"
                sal_Int32 nIndex = rElementName.lastIndexOf( '.' );
                if (( nIndex > 0 ) && ( nIndex < rElementName.getLength() ))
                {
                    std::u16string_view aExtension( rElementName.subView( nIndex+1 ));
                    std::u16string_view aUIElementName( rElementName.subView( 0, nIndex ));

                    if (!aUIElementName.empty() &&
                        ( o3tl::equalsIgnoreAsciiCase(aExtension, u"xml")))
                    {
                        aUIElementData.aResourceURL = aResURLPrefix + aUIElementName;
                        aUIElementData.aName        = rElementName;
                        aUIElementData.bModified    = false;
                        aUIElementData.bDefault     = false;

                        // Create unordered_map entries for all user interface elements inside the storage. We don't load the
                        // settings to speed up the process.
                        rHashMap.emplace( aUIElementData.aResourceURL, aUIElementData );
                    }
                }
            }
        }
    }

    rElementTypeData.bLoaded = true;
}

void UIConfigurationManager::impl_requestUIElementData( sal_Int16 nElementType, UIElementData& aUIElementData )
{
    UIElementType& rElementTypeData = m_aUIElements[nElementType];

    Reference< XStorage > xElementTypeStorage = rElementTypeData.xStorage;
    if ( xElementTypeStorage.is() && !aUIElementData.aName.isEmpty() )
    {
        try
        {
            Reference< XStream > xStream = xElementTypeStorage->openStreamElement( aUIElementData.aName, ElementModes::READ );
            Reference< XInputStream > xInputStream = xStream->getInputStream();

            if ( xInputStream.is() )
            {
                switch ( nElementType )
                {
                    case css::ui::UIElementType::UNKNOWN:
                    break;

                    case css::ui::UIElementType::MENUBAR:
                    case css::ui::UIElementType::POPUPMENU:
                    {
                        try
                        {
                            MenuConfiguration aMenuCfg( m_xContext );
                            Reference< XIndexAccess > xContainer( aMenuCfg.CreateMenuBarConfigurationFromXML( xInputStream ));
                            auto pRootItemContainer = dynamic_cast<RootItemContainer*>( xContainer.get() );
                            if ( pRootItemContainer )
                                aUIElementData.xSettings = new ConstItemContainer( pRootItemContainer, true );
                            else
                                aUIElementData.xSettings = new ConstItemContainer( xContainer, true );
                            return;
                        }
                        catch ( const css::lang::WrappedTargetException& )
                        {
                        }
                    }
                    break;

                    case css::ui::UIElementType::TOOLBAR:
                    {
                        try
                        {
                            Reference< XIndexContainer > xIndexContainer( new RootItemContainer() );
                            ToolBoxConfiguration::LoadToolBox( m_xContext, xInputStream, xIndexContainer );
                            auto pRootItemContainer = dynamic_cast<RootItemContainer*>( xIndexContainer.get() );
                            aUIElementData.xSettings = new ConstItemContainer( pRootItemContainer, true );
                            return;
                        }
                        catch ( const css::lang::WrappedTargetException& )
                        {
                        }

                        break;
                    }

                    case css::ui::UIElementType::STATUSBAR:
                    {
                        try
                        {
                            Reference< XIndexContainer > xIndexContainer( new RootItemContainer() );
                            StatusBarConfiguration::LoadStatusBar( m_xContext, xInputStream, xIndexContainer );
                            auto pRootItemContainer = dynamic_cast<RootItemContainer*>( xIndexContainer.get() );
                            aUIElementData.xSettings = new ConstItemContainer( pRootItemContainer, true );
                            return;
                        }
                        catch ( const css::lang::WrappedTargetException& )
                        {
                        }

                        break;
                    }

                    case css::ui::UIElementType::FLOATINGWINDOW:
                    {
                        break;
                    }
                }
            }
        }
        catch ( const css::embed::InvalidStorageException& )
        {
        }
        catch ( const css::lang::IllegalArgumentException& )
        {
        }
        catch ( const css::io::IOException& )
        {
        }
        catch ( const css::embed::StorageWrappedTargetException& )
        {
        }
    }

    // At least we provide an empty settings container!
    aUIElementData.xSettings = new ConstItemContainer();
}

UIConfigurationManager::UIElementData* UIConfigurationManager::impl_findUIElementData( const OUString& aResourceURL, sal_Int16 nElementType, bool bLoad )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( nElementType );

    // try to look into our document vector/unordered_map combination
    UIElementDataHashMap& rUserHashMap = m_aUIElements[nElementType].aElementsHashMap;
    UIElementDataHashMap::iterator pIter = rUserHashMap.find( aResourceURL );
    if ( pIter != rUserHashMap.end() )
    {
        // Default data settings data means removed!
        if ( pIter->second.bDefault )
            return &(pIter->second);
        else
        {
            if ( !pIter->second.xSettings.is() && bLoad )
                impl_requestUIElementData( nElementType, pIter->second );
            return &(pIter->second);
        }
    }

    // Nothing has been found!
    return nullptr;
}

void UIConfigurationManager::impl_storeElementTypeData( Reference< XStorage > const & xStorage, UIElementType& rElementType, bool bResetModifyState )
{
    UIElementDataHashMap& rHashMap          = rElementType.aElementsHashMap;

    for (auto & elem : rHashMap)
    {
        UIElementData& rElement = elem.second;
        if ( rElement.bModified )
        {
            if ( rElement.bDefault )
            {
                xStorage->removeElement( rElement.aName );
                rElement.bModified = false; // mark as not modified
            }
            else
            {
                Reference< XStream > xStream = xStorage->openStreamElement( rElement.aName, ElementModes::WRITE|ElementModes::TRUNCATE );
                Reference< XOutputStream > xOutputStream( xStream->getOutputStream() );

                if ( xOutputStream.is() )
                {
                    switch( rElementType.nElementType )
                    {
                        case css::ui::UIElementType::MENUBAR:
                        case css::ui::UIElementType::POPUPMENU:
                        {
                            try
                            {
                                MenuConfiguration aMenuCfg( m_xContext );
                                aMenuCfg.StoreMenuBarConfigurationToXML(
                                    rElement.xSettings, xOutputStream, rElementType.nElementType == css::ui::UIElementType::MENUBAR );
                            }
                            catch ( const css::lang::WrappedTargetException& )
                            {
                            }
                        }
                        break;

                        case css::ui::UIElementType::TOOLBAR:
                        {
                            try
                            {
                                ToolBoxConfiguration::StoreToolBox( m_xContext, xOutputStream, rElement.xSettings );
                            }
                            catch ( const css::lang::WrappedTargetException& )
                            {
                            }
                        }
                        break;

                        case css::ui::UIElementType::STATUSBAR:
                        {
                            try
                            {
                                StatusBarConfiguration::StoreStatusBar( m_xContext, xOutputStream, rElement.xSettings );
                            }
                            catch ( const css::lang::WrappedTargetException& )
                            {
                            }
                        }
                        break;

                        default:
                        break;
                    }
                }

                // mark as not modified if we store to our own storage
                if ( bResetModifyState )
                    rElement.bModified = false;
            }
        }
    }

    // commit element type storage
    Reference< XTransactedObject > xTransactedObject( xStorage, UNO_QUERY );
    if ( xTransactedObject.is() )
        xTransactedObject->commit();

    // mark UIElementType as not modified if we store to our own storage
    if ( bResetModifyState )
        rElementType.bModified = false;
}

void UIConfigurationManager::impl_resetElementTypeData(
    UIElementType& rDocElementType,
    ConfigEventNotifyContainer& rRemoveNotifyContainer )
{
    UIElementDataHashMap& rHashMap          = rDocElementType.aElementsHashMap;

    Reference< XUIConfigurationManager > xThis(this);
    Reference< XInterface > xIfac( xThis, UNO_QUERY );

    // Make copies of the event structures to be thread-safe. We have to unlock our mutex before calling
    // our listeners!
    for (auto & elem : rHashMap)
    {
        UIElementData& rElement = elem.second;
        if ( !rElement.bDefault )
        {
            // Remove user-defined settings from document
            ConfigurationEvent aEvent;
            aEvent.ResourceURL = rElement.aResourceURL;
            aEvent.Accessor <<= xThis;
            aEvent.Source = xIfac;
            aEvent.Element <<= rElement.xSettings;

            rRemoveNotifyContainer.push_back( aEvent );

            // Mark element as default.
            rElement.bModified = false;
            rElement.bDefault  = true;
        }
        else
            rElement.bModified = false;
    }

    // Remove all settings from our user interface elements
    rHashMap.clear();
}

void UIConfigurationManager::impl_reloadElementTypeData(
    UIElementType&              rDocElementType,
    ConfigEventNotifyContainer& rRemoveNotifyContainer,
    ConfigEventNotifyContainer& rReplaceNotifyContainer )
{
    UIElementDataHashMap& rHashMap          = rDocElementType.aElementsHashMap;
    Reference< XStorage > xElementStorage( rDocElementType.xStorage );

    Reference< XUIConfigurationManager > xThis(this);
    Reference< XInterface > xIfac( xThis, UNO_QUERY );
    sal_Int16 nType = rDocElementType.nElementType;

    for (auto & elem : rHashMap)
    {
        UIElementData& rElement = elem.second;
        if ( rElement.bModified )
        {
            if ( xElementStorage->hasByName( rElement.aName ))
            {
                // Replace settings with data from user layer
                Reference< XIndexAccess > xOldSettings( rElement.xSettings );

                impl_requestUIElementData( nType, rElement );

                ConfigurationEvent aReplaceEvent;

                aReplaceEvent.ResourceURL = rElement.aResourceURL;
                aReplaceEvent.Accessor <<= xThis;
                aReplaceEvent.Source = xIfac;
                aReplaceEvent.ReplacedElement <<= xOldSettings;
                aReplaceEvent.Element <<= rElement.xSettings;
                rReplaceNotifyContainer.push_back( aReplaceEvent );

                rElement.bModified = false;
            }
            else
            {
                // Element settings are not in any storage => remove
                ConfigurationEvent aRemoveEvent;

                aRemoveEvent.ResourceURL = rElement.aResourceURL;
                aRemoveEvent.Accessor <<= xThis;
                aRemoveEvent.Source = xIfac;
                aRemoveEvent.Element <<= rElement.xSettings;

                rRemoveNotifyContainer.push_back( aRemoveEvent );

                // Mark element as default and not modified. That means "not active" in the document anymore
                rElement.bModified = false;
                rElement.bDefault  = true;
            }
        }
    }

    rDocElementType.bModified = false;
}

void UIConfigurationManager::impl_Initialize()
{
    // Initialize the top-level structures with the storage data
    if ( m_xDocConfigStorage.is() )
    {
        tools::Long nModes = m_bReadOnly ? ElementModes::READ : ElementModes::READWRITE;

        // Try to access our module sub folder
        for ( sal_Int16 i = 1; i < css::ui::UIElementType::COUNT;
              i++ )
        {
            Reference< XStorage > xElementTypeStorage;
            try
            {
                xElementTypeStorage = m_xDocConfigStorage->openStorageElement( OUString(UIELEMENTTYPENAMES[i]), nModes );
            }
            catch ( const css::container::NoSuchElementException& )
            {
            }
            catch ( const css::embed::InvalidStorageException& )
            {
            }
            catch ( const css::lang::IllegalArgumentException& )
            {
            }
            catch ( const css::io::IOException& )
            {
            }
            catch ( const css::embed::StorageWrappedTargetException& )
            {
            }

            m_aUIElements[i].nElementType = i;
            m_aUIElements[i].bModified = false;
            m_aUIElements[i].xStorage = xElementTypeStorage;
        }
    }
    else
    {
        // We have no storage, just initialize ui element types with empty storage!
        for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
            m_aUIElements[i].xStorage = m_xDocConfigStorage;
    }
}

UIConfigurationManager::UIConfigurationManager( css::uno::Reference< css::uno::XComponentContext > xContext ) :
      m_bReadOnly( true )
    , m_bModified( false )
    , m_bDisposed( false )
    , m_aPropUIName( u"UIName"_ustr )
    , m_xContext(std::move( xContext ))
{
    // Make sure we have a default initialized entry for every layer and user interface element type!
    // The following code depends on this!
    m_aUIElements.resize( css::ui::UIElementType::COUNT );
}

// XComponent
void SAL_CALL UIConfigurationManager::dispose()
{
    Reference< XComponent > xThis(this);

    css::lang::EventObject aEvent( xThis );
    {
        std::unique_lock aGuard(m_mutex);
        m_aEventListeners.disposeAndClear( aGuard, aEvent );
    }
    {
        std::unique_lock aGuard(m_mutex);
        m_aConfigListeners.disposeAndClear( aGuard, aEvent );
    }

    {
        SolarMutexGuard g;
        try
        {
            if ( m_xImageManager.is() )
                m_xImageManager->dispose();
        }
        catch ( const Exception& )
        {
        }

        m_xImageManager.clear();
        m_aUIElements.clear();
        m_xDocConfigStorage.clear();
        m_bModified = false;
        m_bDisposed = true;
    }
}

void SAL_CALL UIConfigurationManager::addEventListener( const Reference< XEventListener >& xListener )
{
    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    std::unique_lock aGuard(m_mutex);
    m_aEventListeners.addInterface( aGuard, xListener );
}

void SAL_CALL UIConfigurationManager::removeEventListener( const Reference< XEventListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    std::unique_lock aGuard(m_mutex);
    m_aEventListeners.removeInterface( aGuard, xListener );
}

// XUIConfigurationManager
void SAL_CALL UIConfigurationManager::addConfigurationListener( const Reference< css::ui::XUIConfigurationListener >& xListener )
{
    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    std::unique_lock aGuard(m_mutex);
    m_aConfigListeners.addInterface( aGuard, xListener );
}

void SAL_CALL UIConfigurationManager::removeConfigurationListener( const Reference< css::ui::XUIConfigurationListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    std::unique_lock aGuard(m_mutex);
    m_aConfigListeners.removeInterface( aGuard, xListener );
}

void SAL_CALL UIConfigurationManager::reset()
{
    SolarMutexClearableGuard aGuard;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if ( isReadOnly() )
        return;

    if ( !m_xDocConfigStorage.is() )
        return;

    try
    {
        // Remove all elements from our user-defined storage!
        bool bCommit( false );
        for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
        {
            UIElementType&        rElementType = m_aUIElements[i];

            if ( rElementType.xStorage.is() )
            {
                bool bCommitSubStorage( false );
                const Sequence< OUString > aUIElementStreamNames = rElementType.xStorage->getElementNames();
                for ( OUString const & rStreamName : aUIElementStreamNames )
                {
                    rElementType.xStorage->removeElement( rStreamName );
                    bCommitSubStorage = true;
                    bCommit = true;
                }

                if ( bCommitSubStorage )
                {
                    Reference< XTransactedObject > xTransactedObject( rElementType.xStorage, UNO_QUERY );
                    if ( xTransactedObject.is() )
                        xTransactedObject->commit();
                }
            }
        }

        // Commit changes
        if ( bCommit )
        {
            Reference< XTransactedObject > xTransactedObject( m_xDocConfigStorage, UNO_QUERY );
            if ( xTransactedObject.is() )
                xTransactedObject->commit();
        }

        // remove settings from user defined layer and notify listener about removed settings data!
        // Try to access our module sub folder
        ConfigEventNotifyContainer aRemoveEventNotifyContainer;
        for ( sal_Int16 j = 1; j < css::ui::UIElementType::COUNT; j++ )
        {
            UIElementType& rDocElementType = m_aUIElements[j];

            impl_resetElementTypeData( rDocElementType, aRemoveEventNotifyContainer );
            rDocElementType.bModified = false;
        }

        m_bModified = false;

        // Unlock mutex before notify our listeners
        aGuard.clear();

        // Notify our listeners
        for (const ConfigurationEvent & k : aRemoveEventNotifyContainer)
            implts_notifyContainerListener( k, NotifyOp_Remove );
    }
    catch ( const css::lang::IllegalArgumentException& )
    {
    }
    catch ( const css::container::NoSuchElementException& )
    {
    }
    catch ( const css::embed::InvalidStorageException& )
    {
    }
    catch ( const css::embed::StorageWrappedTargetException& )
    {
    }
}

Sequence< Sequence< PropertyValue > > SAL_CALL UIConfigurationManager::getUIElementsInfo( sal_Int16 ElementType )
{
    if (( ElementType < 0 ) || ( ElementType >= css::ui::UIElementType::COUNT ))
        throw IllegalArgumentException();

    SolarMutexGuard g;
    if ( m_bDisposed )
        throw DisposedException();

    std::vector< Sequence< PropertyValue > > aElementInfoSeq;
    UIElementInfoHashMap aUIElementInfoCollection;

    if ( ElementType == css::ui::UIElementType::UNKNOWN )
    {
        for ( sal_Int16 i = 0; i < css::ui::UIElementType::COUNT; i++ )
            impl_fillSequenceWithElementTypeInfo( aUIElementInfoCollection, i );
    }
    else
        impl_fillSequenceWithElementTypeInfo( aUIElementInfoCollection, ElementType );

    aElementInfoSeq.resize( aUIElementInfoCollection.size() );
    sal_Int32 n = 0;
    for (auto const& elem : aUIElementInfoCollection)
    {
        aElementInfoSeq[n++] =
        {
            comphelper::makePropertyValue(u"ResourceURL"_ustr, elem.second.aResourceURL),
            comphelper::makePropertyValue(m_aPropUIName, elem.second.aUIName)
        };
    }

    return comphelper::containerToSequence(aElementInfoSeq);
}

Reference< XIndexContainer > SAL_CALL UIConfigurationManager::createSettings()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    // Creates an empty item container which can be filled from outside
    return Reference< XIndexContainer >( new RootItemContainer() );
}

sal_Bool SAL_CALL UIConfigurationManager::hasSettings( const OUString& ResourceURL )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType, false );
    if ( pDataSettings && !pDataSettings->bDefault )
        return true;

    return false;
}

Reference< XIndexAccess > SAL_CALL UIConfigurationManager::getSettings( const OUString& ResourceURL, sal_Bool bWriteable )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();

    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType );
    if ( pDataSettings && !pDataSettings->bDefault )
    {
        // Create a copy of our data if someone wants to change the data.
        if ( bWriteable )
            return Reference< XIndexAccess >( new RootItemContainer( pDataSettings->xSettings ) );
        else
            return pDataSettings->xSettings;
    }

    throw NoSuchElementException();
}

void SAL_CALL UIConfigurationManager::replaceSettings( const OUString& ResourceURL, const Reference< css::container::XIndexAccess >& aNewData )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else if ( m_bReadOnly )
        throw IllegalAccessException();
    else
    {
        SolarMutexClearableGuard aGuard;

        if ( m_bDisposed )
            throw DisposedException();

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType );
        if ( !pDataSettings || pDataSettings->bDefault )
            throw NoSuchElementException();
        // we have a settings entry in our user-defined layer - replace
        Reference< XIndexAccess > xOldSettings = pDataSettings->xSettings;

        // Create a copy of the data if the container is not const
        Reference< XIndexReplace > xReplace( aNewData, UNO_QUERY );
        if ( xReplace.is() )
            pDataSettings->xSettings = new ConstItemContainer( aNewData );
        else
            pDataSettings->xSettings = aNewData;

        pDataSettings->bDefault  = false;
        pDataSettings->bModified = true;
        m_bModified = true;

        // Modify type container
        UIElementType& rElementType = m_aUIElements[nElementType];
        rElementType.bModified = true;

        Reference< XUIConfigurationManager > xThis(this);

        // Create event to notify listener about replaced element settings
        ConfigurationEvent aEvent;

        aEvent.ResourceURL = ResourceURL;
        aEvent.Accessor <<= xThis;
        aEvent.Source.set(xThis, UNO_QUERY);
        aEvent.ReplacedElement <<= xOldSettings;
        aEvent.Element <<= pDataSettings->xSettings;

        aGuard.clear();

        implts_notifyContainerListener( aEvent, NotifyOp_Replace );
    }
}

void SAL_CALL UIConfigurationManager::removeSettings( const OUString& ResourceURL )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException( "The ResourceURL is not valid or "
                                        "describes an unknown type. "
                                        "ResourceURL: " + ResourceURL, nullptr, 0 );
    else if ( m_bReadOnly )
        throw IllegalAccessException( "The configuration manager is read-only. "
                                      "ResourceURL: " + ResourceURL, nullptr );
    else
    {
        SolarMutexClearableGuard aGuard;

        if ( m_bDisposed )
            throw DisposedException( "The configuration manager has been disposed, "
                                     "and can't uphold its method specification anymore. "
                                     "ResourceURL: " + ResourceURL, nullptr );

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType );
        if ( !pDataSettings )
            throw NoSuchElementException( "The settings data cannot be found. "
                                          "ResourceURL: " + ResourceURL, nullptr);
        // If element settings are default, we don't need to change anything!
        if ( pDataSettings->bDefault )
            return;
        else
        {
            Reference< XIndexAccess > xRemovedSettings = pDataSettings->xSettings;
            pDataSettings->bDefault = true;

            // check if this is a default layer node
            pDataSettings->bModified = true; // we have to remove this node from the user layer!
            pDataSettings->xSettings.clear();
            m_bModified = true; // user layer must be written

            // Modify type container
            UIElementType& rElementType = m_aUIElements[nElementType];
            rElementType.bModified = true;

            Reference< XUIConfigurationManager > xThis(this);

            // Create event to notify listener about removed element settings
            ConfigurationEvent aEvent;

            aEvent.ResourceURL = ResourceURL;
            aEvent.Accessor <<= xThis;
            aEvent.Source.set(xThis, UNO_QUERY);

            aEvent.Element <<= xRemovedSettings;

            aGuard.clear();

            implts_notifyContainerListener( aEvent, NotifyOp_Remove );
        }
    }
}

void SAL_CALL UIConfigurationManager::insertSettings( const OUString& NewResourceURL, const Reference< XIndexAccess >& aNewData )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( NewResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else if ( m_bReadOnly )
        throw IllegalAccessException();
    else
    {
        SolarMutexClearableGuard aGuard;

        if ( m_bDisposed )
            throw DisposedException();

        bool           bInsertData( false );
        UIElementData  aUIElementData;
        UIElementData* pDataSettings = impl_findUIElementData( NewResourceURL, nElementType );

        if ( pDataSettings && !pDataSettings->bDefault )
            throw ElementExistException();

        if ( !pDataSettings )
        {
            pDataSettings = &aUIElementData;
            bInsertData   = true;
        }

        {
            pDataSettings->bDefault     = false;
            pDataSettings->bModified    = true;

            // Create a copy of the data if the container is not const
            Reference< XIndexReplace > xReplace( aNewData, UNO_QUERY );
            if ( xReplace.is() )
                pDataSettings->xSettings = new ConstItemContainer( aNewData );
            else
                pDataSettings->xSettings = aNewData;

            m_bModified = true;

            UIElementType& rElementType = m_aUIElements[nElementType];
            rElementType.bModified = true;

            if ( bInsertData )
            {
                pDataSettings->aName        = RetrieveNameFromResourceURL( NewResourceURL ) + ".xml";
                pDataSettings->aResourceURL = NewResourceURL;

                UIElementDataHashMap& rElements = rElementType.aElementsHashMap;
                rElements.emplace( NewResourceURL, *pDataSettings );
            }

            Reference< XIndexAccess > xInsertSettings( aUIElementData.xSettings );
            Reference< XUIConfigurationManager > xThis(this);

            // Create event to notify listener about removed element settings
            ConfigurationEvent aEvent;

            aEvent.ResourceURL = NewResourceURL;
            aEvent.Accessor <<= xThis;
            aEvent.Source.set(xThis, UNO_QUERY);
            aEvent.Element <<= xInsertSettings;

            aGuard.clear();

            implts_notifyContainerListener( aEvent, NotifyOp_Insert );
        }
    }
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getImageManager()
{
    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xImageManager.is() )
    {
        m_xImageManager = new ImageManager( m_xContext, /*bForModule*/false );

        Sequence<Any> aPropSeq(comphelper::InitAnyPropertySequence(
        {
            {"UserConfigStorage", Any(m_xDocConfigStorage)},
            {"ModuleIdentifier", Any(OUString())},
        }));

        m_xImageManager->initialize( aPropSeq );
    }

    return Reference< XInterface >( static_cast<cppu::OWeakObject*>(m_xImageManager.get()), UNO_QUERY );
}

Reference< XAcceleratorConfiguration > SAL_CALL UIConfigurationManager::createShortCutManager()
{
    return DocumentAcceleratorConfiguration::createWithDocumentRoot(m_xContext, m_xDocConfigStorage);
}

Reference< XAcceleratorConfiguration > SAL_CALL UIConfigurationManager::getShortCutManager()
{
    // SAFE ->
    SolarMutexGuard g;

    if (!m_xAccConfig.is()) try
    {
        m_xAccConfig = DocumentAcceleratorConfiguration::
            createWithDocumentRoot(m_xContext, m_xDocConfigStorage);
    }
    catch ( const css::uno::DeploymentException& )
    {
        SAL_WARN("fwk.uiconfiguration", "DocumentAcceleratorConfiguration"
                " not available. This should happen only on mobile platforms.");
    }

    return m_xAccConfig;
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getEventsManager()
{
    return Reference< XInterface >();
}

// XUIConfigurationStorage
void SAL_CALL UIConfigurationManager::setStorage( const Reference< XStorage >& Storage )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xDocConfigStorage.is() )
    {
        try
        {
            // Dispose old storage to be sure that it will be closed
            m_xDocConfigStorage->dispose();
        }
        catch ( const Exception& )
        {
        }
    }

    // We store the new storage. Be careful it could be an empty reference!
    m_xDocConfigStorage = Storage;
    m_bReadOnly         = true;

    if ( m_xAccConfig.is() )
        m_xAccConfig->setStorage( m_xDocConfigStorage );

    if ( m_xImageManager )
        m_xImageManager->setStorage( m_xDocConfigStorage );

    if ( m_xDocConfigStorage.is() )
    {
        Reference< XPropertySet > xPropSet( m_xDocConfigStorage, UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                tools::Long nOpenMode = 0;
                Any a = xPropSet->getPropertyValue(u"OpenMode"_ustr);
                if ( a >>= nOpenMode )
                    m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
            }
            catch ( const css::beans::UnknownPropertyException& )
            {
            }
            catch ( const css::lang::WrappedTargetException& )
            {
            }
        }
    }

    impl_Initialize();
}

sal_Bool SAL_CALL UIConfigurationManager::hasStorage()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    return m_xDocConfigStorage.is();
}

// XUIConfigurationPersistence
void SAL_CALL UIConfigurationManager::reload()
{
    SolarMutexClearableGuard aGuard;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xDocConfigStorage.is() || !m_bModified || m_bReadOnly )
        return;

    // Try to access our module sub folder
    ConfigEventNotifyContainer aRemoveNotifyContainer;
    ConfigEventNotifyContainer aReplaceNotifyContainer;
    for ( sal_Int16 i = 1; i < css::ui::UIElementType::COUNT; i++ )
    {
        try
        {
            UIElementType& rDocElementType = m_aUIElements[i];
            if ( rDocElementType.bModified )
                impl_reloadElementTypeData( rDocElementType, aRemoveNotifyContainer, aReplaceNotifyContainer );
        }
        catch ( const Exception& )
        {
            throw IOException();
        }
    }

    m_bModified = false;

    // Unlock mutex before notify our listeners
    aGuard.clear();

    // Notify our listeners
    for (const ConfigurationEvent & j : aRemoveNotifyContainer)
        implts_notifyContainerListener( j, NotifyOp_Remove );
    for (const ConfigurationEvent & k : aReplaceNotifyContainer)
        implts_notifyContainerListener( k, NotifyOp_Replace );
}

void SAL_CALL UIConfigurationManager::store()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xDocConfigStorage.is() || !m_bModified || m_bReadOnly )
        return;

    // Try to access our module sub folder
    for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
    {
        try
        {
            UIElementType& rElementType = m_aUIElements[i];

            if ( rElementType.bModified && rElementType.xStorage.is() )
                impl_storeElementTypeData( rElementType.xStorage, rElementType );
        }
        catch ( const Exception& )
        {
            throw IOException();
        }
    }

    m_bModified = false;
    Reference< XTransactedObject > xTransactedObject( m_xDocConfigStorage, UNO_QUERY );
    if ( xTransactedObject.is() )
        xTransactedObject->commit();
}

void SAL_CALL UIConfigurationManager::storeToStorage( const Reference< XStorage >& Storage )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xDocConfigStorage.is() || !m_bModified || m_bReadOnly )
        return;

    // Try to access our module sub folder
    for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
    {
        try
        {
            Reference< XStorage > xElementTypeStorage( Storage->openStorageElement(
                                                        OUString(UIELEMENTTYPENAMES[i]), ElementModes::READWRITE ));
            UIElementType& rElementType = m_aUIElements[i];

            if ( rElementType.bModified && xElementTypeStorage.is() )
                impl_storeElementTypeData( xElementTypeStorage, rElementType, false ); // store data to storage, but don't reset modify flag!
        }
        catch ( const Exception& )
        {
            throw IOException();
        }
    }

    Reference< XTransactedObject > xTransactedObject( Storage, UNO_QUERY );
    if ( xTransactedObject.is() )
        xTransactedObject->commit();
}

sal_Bool SAL_CALL UIConfigurationManager::isModified()
{
    SolarMutexGuard g;

    return m_bModified;
}

sal_Bool SAL_CALL UIConfigurationManager::isReadOnly()
{
    SolarMutexGuard g;

    return m_bReadOnly;
}

void UIConfigurationManager::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
{
    std::unique_lock aGuard(m_mutex);
    m_aConfigListeners.forEach(aGuard, [&eOp, &aEvent](const css::uno::Reference<XUIConfigurationListener>& l) {
        switch ( eOp )
        {
            case NotifyOp_Replace:
                l->elementReplaced( aEvent );
                break;
            case NotifyOp_Insert:
                l->elementInserted( aEvent );
                break;
            case NotifyOp_Remove:
                l->elementRemoved( aEvent );
                break;
        }
    });
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_UIConfigurationManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new UIConfigurationManager(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
