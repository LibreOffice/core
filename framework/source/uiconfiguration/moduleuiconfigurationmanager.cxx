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

#include <accelerators/presethandler.hxx>
#include <uiconfiguration/moduleimagemanager.hxx>
#include <stdtypes.h>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/uielementtypenames.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/toolboxconfiguration.hxx>

#include <framework/statusbarconfiguration.hxx>

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/ui/ModuleAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager2.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalAccessException.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/InvalidStorageException.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>

using namespace css;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::embed;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace framework;

#define RESOURCETYPE_MENUBAR   "menubar"
#define RESOURCETYPE_TOOLBAR   "toolbar"
#define RESOURCETYPE_STATUSBAR "statusbar"
#define RESOURCETYPE_POPUPMENU "popupmenu"

namespace {

class ModuleUIConfigurationManager : public cppu::WeakImplHelper<
                                       css::lang::XServiceInfo,
                                       css::lang::XComponent,
                                       css::ui::XModuleUIConfigurationManager2 >
{
public:
    ModuleUIConfigurationManager(
            const css::uno::Reference< css::uno::XComponentContext >& xServiceManager,
            const css::uno::Sequence< css::uno::Any >& aArguments);

    virtual ~ModuleUIConfigurationManager() override;

    virtual OUString SAL_CALL getImplementationName() override
    {
        return OUString("com.sun.star.comp.framework.ModuleUIConfigurationManager");
    }

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return {"com.sun.star.ui.ModuleUIConfigurationManager"};
    }

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
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getEventsManager() override;

    // XModuleUIConfigurationManager
    virtual sal_Bool SAL_CALL isDefaultSettings( const OUString& ResourceURL ) override;
    virtual css::uno::Reference< css::container::XIndexAccess > SAL_CALL getDefaultSettings( const OUString& ResourceURL ) override;

    // XUIConfigurationPersistence
    virtual void SAL_CALL reload() override;
    virtual void SAL_CALL store() override;
    virtual void SAL_CALL storeToStorage( const css::uno::Reference< css::embed::XStorage >& Storage ) override;
    virtual sal_Bool SAL_CALL isModified() override;
    virtual sal_Bool SAL_CALL isReadOnly() override;

private:
    // private data types
    enum Layer
    {
        LAYER_DEFAULT,
        LAYER_USERDEFINED,
        LAYER_COUNT
    };

    enum NotifyOp
    {
        NotifyOp_Remove,
        NotifyOp_Insert,
        NotifyOp_Replace
    };

    struct UIElementInfo
    {
        UIElementInfo( const OUString& rResourceURL, const OUString& rUIName ) :
            aResourceURL( rResourceURL), aUIName( rUIName ) {}
        OUString   aResourceURL;
        OUString   aUIName;
    };

    struct UIElementData
    {
        UIElementData() : bModified( false ), bDefault( true ), bDefaultNode( true ) {};

        OUString aResourceURL;
        OUString aName;
        bool          bModified;        // has been changed since last storing
        bool          bDefault;         // default settings
        bool          bDefaultNode;     // this is a default layer element data
        css::uno::Reference< css::container::XIndexAccess > xSettings;
    };

    typedef std::unordered_map< OUString, UIElementData, OUStringHash > UIElementDataHashMap;

    struct UIElementType
    {
        UIElementType() : bModified( false ),
                          bLoaded( false ),
                          bDefaultLayer( false ),
                          nElementType( css::ui::UIElementType::UNKNOWN ) {}

        bool                                                              bModified;
        bool                                                              bLoaded;
        bool                                                              bDefaultLayer;
        sal_Int16                                                         nElementType;
        UIElementDataHashMap                                              aElementsHashMap;
        css::uno::Reference< css::embed::XStorage > xStorage;
    };

    typedef std::vector< UIElementType > UIElementTypesVector;
    typedef std::vector< css::ui::ConfigurationEvent > ConfigEventNotifyContainer;
    typedef std::unordered_map< OUString, UIElementInfo, OUStringHash > UIElementInfoHashMap;

    void            impl_Initialize();
    void            implts_notifyContainerListener( const css::ui::ConfigurationEvent& aEvent, NotifyOp eOp );
    void            impl_fillSequenceWithElementTypeInfo( UIElementInfoHashMap& aUIElementInfoCollection, sal_Int16 nElementType );
    void            impl_preloadUIElementTypeList( Layer eLayer, sal_Int16 nElementType );
    UIElementData*  impl_findUIElementData( const OUString& aResourceURL, sal_Int16 nElementType, bool bLoad = true );
    void            impl_requestUIElementData( sal_Int16 nElementType, Layer eLayer, UIElementData& aUIElementData );
    void            impl_storeElementTypeData( const css::uno::Reference< css::embed::XStorage >& xStorage, UIElementType& rElementType, bool bResetModifyState = true );
    void            impl_resetElementTypeData( UIElementType& rUserElementType, UIElementType& rDefaultElementType, ConfigEventNotifyContainer& rRemoveNotifyContainer, ConfigEventNotifyContainer& rReplaceNotifyContainer );
    void            impl_reloadElementTypeData( UIElementType& rUserElementType, UIElementType& rDefaultElementType, ConfigEventNotifyContainer& rRemoveNotifyContainer, ConfigEventNotifyContainer& rReplaceNotifyContainer );

    UIElementTypesVector                                      m_aUIElements[LAYER_COUNT];
    PresetHandler*                                            m_pStorageHandler[css::ui::UIElementType::COUNT];
    css::uno::Reference< css::embed::XStorage >               m_xDefaultConfigStorage;
    css::uno::Reference< css::embed::XStorage >               m_xUserConfigStorage;
    bool                                                      m_bReadOnly;
    bool                                                      m_bModified;
    bool                                                      m_bDisposed;
    OUString                                                  m_aXMLPostfix;
    OUString                                                  m_aPropUIName;
    OUString                                                  m_aPropResourceURL;
    OUString                                                  m_aModuleIdentifier;
    OUString                                                  m_aModuleShortName;
    css::uno::Reference< css::embed::XTransactedObject >      m_xUserRootCommit;
    css::uno::Reference< css::uno::XComponentContext >        m_xContext;
    osl::Mutex                                                m_mutex;
    ::cppu::OMultiTypeInterfaceContainerHelper                m_aListenerContainer;   /// container for ALL Listener
    css::uno::Reference< css::lang::XComponent >              m_xModuleImageManager;
    css::uno::Reference< css::ui::XAcceleratorConfiguration > m_xModuleAcceleratorManager;
};

// important: The order and position of the elements must match the constant
// definition of "css::ui::UIElementType"
static const char* UIELEMENTTYPENAMES[] =
{
    "",  // Dummy value for unknown!
    UIELEMENTTYPE_MENUBAR_NAME,
    UIELEMENTTYPE_POPUPMENU_NAME,
    UIELEMENTTYPE_TOOLBAR_NAME,
    UIELEMENTTYPE_STATUSBAR_NAME,
    UIELEMENTTYPE_FLOATINGWINDOW_NAME,
    UIELEMENTTYPE_PROGRESSBAR_NAME,
    UIELEMENTTYPE_TOOLPANEL_NAME
};

static const char       RESOURCEURL_PREFIX[] = "private:resource/";
static const sal_Int32  RESOURCEURL_PREFIX_SIZE = strlen(RESOURCEURL_PREFIX);

sal_Int16 RetrieveTypeFromResourceURL( const OUString& aResourceURL )
{

    if (( aResourceURL.startsWith( RESOURCEURL_PREFIX ) ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        OUString    aTmpStr     = aResourceURL.copy( RESOURCEURL_PREFIX_SIZE );
        sal_Int32   nIndex      = aTmpStr.indexOf( '/' );
        if (( nIndex > 0 ) &&  ( aTmpStr.getLength() > nIndex ))
        {
            OUString aTypeStr( aTmpStr.copy( 0, nIndex ));
            for ( int i = 0; i < ui::UIElementType::COUNT; i++ )
            {
                if ( aTypeStr.equalsAscii( UIELEMENTTYPENAMES[i] ))
                    return sal_Int16( i );
            }
        }
    }

    return ui::UIElementType::UNKNOWN;
}

OUString RetrieveNameFromResourceURL( const OUString& aResourceURL )
{
    if (( aResourceURL.startsWith( RESOURCEURL_PREFIX ) ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        sal_Int32 nIndex = aResourceURL.lastIndexOf( '/' );
        if (( nIndex > 0 ) && (( nIndex+1 ) < aResourceURL.getLength()))
            return aResourceURL.copy( nIndex+1 );
    }

    return OUString();
}

void ModuleUIConfigurationManager::impl_fillSequenceWithElementTypeInfo( UIElementInfoHashMap& aUIElementInfoCollection, sal_Int16 nElementType )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( LAYER_USERDEFINED, nElementType );
    impl_preloadUIElementTypeList( LAYER_DEFAULT, nElementType );

    UIElementDataHashMap& rUserElements = m_aUIElements[LAYER_USERDEFINED][nElementType].aElementsHashMap;
    UIElementDataHashMap::const_iterator pUserIter = rUserElements.begin();

    OUString aCustomUrlPrefix( "custom_" );
    while ( pUserIter != rUserElements.end() )
    {
        sal_Int32 nIndex = pUserIter->second.aResourceURL.indexOf( aCustomUrlPrefix, RESOURCEURL_PREFIX_SIZE );
        if ( nIndex > RESOURCEURL_PREFIX_SIZE )
        {
            // Performance: Retrieve user interface name only for custom user interface elements.
            // It's only used by them!
            UIElementData* pDataSettings = impl_findUIElementData( pUserIter->second.aResourceURL, nElementType );
            if ( pDataSettings )
            {
                // Retrieve user interface name from XPropertySet interface
                OUString aUIName;
                Reference< XPropertySet > xPropSet( pDataSettings->xSettings, UNO_QUERY );
                if ( xPropSet.is() )
                {
                    Any a = xPropSet->getPropertyValue( m_aPropUIName );
                    a >>= aUIName;
                }

                UIElementInfo aInfo( pUserIter->second.aResourceURL, aUIName );
                aUIElementInfoCollection.insert( UIElementInfoHashMap::value_type( pUserIter->second.aResourceURL, aInfo ));
            }
        }
        else
        {
            // The user interface name for standard user interface elements is stored in the WindowState.xcu file
            UIElementInfo aInfo( pUserIter->second.aResourceURL, OUString() );
            aUIElementInfoCollection.insert( UIElementInfoHashMap::value_type( pUserIter->second.aResourceURL, aInfo ));
        }
        ++pUserIter;
    }

    UIElementDataHashMap& rDefaultElements = m_aUIElements[LAYER_DEFAULT][nElementType].aElementsHashMap;
    UIElementDataHashMap::const_iterator pDefIter = rDefaultElements.begin();

    while ( pDefIter != rDefaultElements.end() )
    {
        UIElementInfoHashMap::const_iterator pIterInfo = aUIElementInfoCollection.find( pDefIter->second.aResourceURL );
        if ( pIterInfo == aUIElementInfoCollection.end() )
        {
            sal_Int32 nIndex = pDefIter->second.aResourceURL.indexOf( aCustomUrlPrefix, RESOURCEURL_PREFIX_SIZE );
            if ( nIndex > RESOURCEURL_PREFIX_SIZE )
            {
                // Performance: Retrieve user interface name only for custom user interface elements.
                // It's only used by them!
                UIElementData* pDataSettings = impl_findUIElementData( pDefIter->second.aResourceURL, nElementType );
                if ( pDataSettings )
                {
                    // Retrieve user interface name from XPropertySet interface
                    OUString aUIName;
                    Reference< XPropertySet > xPropSet( pDataSettings->xSettings, UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        Any a = xPropSet->getPropertyValue( m_aPropUIName );
                        a >>= aUIName;
                    }

                    UIElementInfo aInfo( pDefIter->second.aResourceURL, aUIName );
                    aUIElementInfoCollection.insert( UIElementInfoHashMap::value_type( pDefIter->second.aResourceURL, aInfo ));
                }
            }
            else
            {
                // The user interface name for standard user interface elements is stored in the WindowState.xcu file
                UIElementInfo aInfo( pDefIter->second.aResourceURL, OUString() );
                aUIElementInfoCollection.insert( UIElementInfoHashMap::value_type( pDefIter->second.aResourceURL, aInfo ));
            }
        }

        ++pDefIter;
    }
}

void ModuleUIConfigurationManager::impl_preloadUIElementTypeList( Layer eLayer, sal_Int16 nElementType )
{
    UIElementType& rElementTypeData = m_aUIElements[eLayer][nElementType];

    if ( !rElementTypeData.bLoaded )
    {
        Reference< XStorage > xElementTypeStorage = rElementTypeData.xStorage;
        if ( xElementTypeStorage.is() )
        {
            OUStringBuffer aBuf( RESOURCEURL_PREFIX_SIZE );
            aBuf.append( RESOURCEURL_PREFIX );
            aBuf.appendAscii( UIELEMENTTYPENAMES[ nElementType ] );
            aBuf.append( "/" );
            OUString aResURLPrefix( aBuf.makeStringAndClear() );

            UIElementDataHashMap& rHashMap = rElementTypeData.aElementsHashMap;
            Sequence< OUString > aUIElementNames = xElementTypeStorage->getElementNames();
            for ( sal_Int32 n = 0; n < aUIElementNames.getLength(); n++ )
            {
                UIElementData aUIElementData;

                // Resource name must be without ".xml"
                sal_Int32 nIndex = aUIElementNames[n].lastIndexOf( '.' );
                if (( nIndex > 0 ) && ( nIndex < aUIElementNames[n].getLength() ))
                {
                    OUString aExtension( aUIElementNames[n].copy( nIndex+1 ));
                    OUString aUIElementName( aUIElementNames[n].copy( 0, nIndex ));

                    if (!aUIElementName.isEmpty() &&
                        ( aExtension.equalsIgnoreAsciiCase("xml")))
                    {
                        aUIElementData.aResourceURL = aResURLPrefix + aUIElementName;
                        aUIElementData.aName        = aUIElementNames[n];

                        if ( eLayer == LAYER_USERDEFINED )
                        {
                            aUIElementData.bModified    = false;
                            aUIElementData.bDefault     = false;
                            aUIElementData.bDefaultNode = false;
                        }

                        // Create std::unordered_map entries for all user interface elements inside the storage. We don't load the
                        // settings to speed up the process.
                        rHashMap.insert( UIElementDataHashMap::value_type( aUIElementData.aResourceURL, aUIElementData ));
                    }
                }
                rElementTypeData.bLoaded = true;
            }
        }
    }

}

void ModuleUIConfigurationManager::impl_requestUIElementData( sal_Int16 nElementType, Layer eLayer, UIElementData& aUIElementData )
{
    UIElementType& rElementTypeData = m_aUIElements[eLayer][nElementType];

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
                            RootItemContainer* pRootItemContainer = RootItemContainer::GetImplementation( xContainer );
                            if ( pRootItemContainer )
                                aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( pRootItemContainer, true ) ), UNO_QUERY );
                            else
                                aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( xContainer, true ) ), UNO_QUERY );
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
                            Reference< XIndexContainer > xIndexContainer( static_cast< OWeakObject * >( new RootItemContainer() ), UNO_QUERY );
                            ToolBoxConfiguration::LoadToolBox( m_xContext, xInputStream, xIndexContainer );
                            RootItemContainer* pRootItemContainer = RootItemContainer::GetImplementation( xIndexContainer );
                            aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( pRootItemContainer, true ) ), UNO_QUERY );
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
                            Reference< XIndexContainer > xIndexContainer( static_cast< OWeakObject * >( new RootItemContainer() ), UNO_QUERY );
                            StatusBarConfiguration::LoadStatusBar( m_xContext, xInputStream, xIndexContainer );
                            RootItemContainer* pRootItemContainer = RootItemContainer::GetImplementation( xIndexContainer );
                            aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( pRootItemContainer, true ) ), UNO_QUERY );
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
    aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer() ), UNO_QUERY );
}

ModuleUIConfigurationManager::UIElementData*  ModuleUIConfigurationManager::impl_findUIElementData( const OUString& aResourceURL, sal_Int16 nElementType, bool bLoad )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( LAYER_USERDEFINED, nElementType );
    impl_preloadUIElementTypeList( LAYER_DEFAULT, nElementType );

    // first try to look into our user-defined vector/unordered_map combination
    UIElementDataHashMap& rUserHashMap = m_aUIElements[LAYER_USERDEFINED][nElementType].aElementsHashMap;
    UIElementDataHashMap::iterator pIter = rUserHashMap.find( aResourceURL );
    if ( pIter != rUserHashMap.end() )
    {
        // Default data settings data must be retrieved from the default layer!
        if ( !pIter->second.bDefault )
        {
            if ( !pIter->second.xSettings.is() && bLoad )
                impl_requestUIElementData( nElementType, LAYER_USERDEFINED, pIter->second );
            return &(pIter->second);
        }
    }

    // Not successful, we have to look into our default vector/unordered_map combination
    UIElementDataHashMap& rDefaultHashMap = m_aUIElements[LAYER_DEFAULT][nElementType].aElementsHashMap;
    pIter = rDefaultHashMap.find( aResourceURL );
    if ( pIter != rDefaultHashMap.end() )
    {
        if ( !pIter->second.xSettings.is() && bLoad )
            impl_requestUIElementData( nElementType, LAYER_DEFAULT, pIter->second );
        return &(pIter->second);
    }

    // Nothing has been found!
    return nullptr;
}

void ModuleUIConfigurationManager::impl_storeElementTypeData( const Reference< XStorage >& xStorage, UIElementType& rElementType, bool bResetModifyState )
{
    UIElementDataHashMap& rHashMap          = rElementType.aElementsHashMap;
    UIElementDataHashMap::iterator pIter    = rHashMap.begin();

    while ( pIter != rHashMap.end() )
    {
        UIElementData& rElement = pIter->second;
        if ( rElement.bModified )
        {
            if ( rElement.bDefault )
            {
                xStorage->removeElement( rElement.aName );
                rElement.bModified = false; // mark as not modified
            }
            else
            {
                Reference< XStream > xStream( xStorage->openStreamElement( rElement.aName, ElementModes::WRITE|ElementModes::TRUNCATE ), UNO_QUERY );
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

        ++pIter;
    }

    // commit element type storage
    Reference< XTransactedObject > xTransactedObject( xStorage, UNO_QUERY );
    if ( xTransactedObject.is() )
        xTransactedObject->commit();

    // mark UIElementType as not modified if we store to our own storage
    if ( bResetModifyState )
        rElementType.bModified = false;
}

// This is only allowed to be called on the LAYER_USER_DEFINED!
void ModuleUIConfigurationManager::impl_resetElementTypeData(
    UIElementType& rUserElementType,
    UIElementType& rDefaultElementType,
    ConfigEventNotifyContainer& rRemoveNotifyContainer,
    ConfigEventNotifyContainer& rReplaceNotifyContainer )
{
    UIElementDataHashMap& rHashMap          = rUserElementType.aElementsHashMap;
    UIElementDataHashMap::iterator pIter    = rHashMap.begin();

    Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XInterface >  xIfac( xThis, UNO_QUERY );
    Reference< XNameAccess > xDefaultNameAccess( rDefaultElementType.xStorage, UNO_QUERY );
    sal_Int16 nType = rUserElementType.nElementType;

    // Make copies of the event structures to be thread-safe. We have to unlock our mutex before calling
    // our listeners!
    while ( pIter != rHashMap.end() )
    {
        UIElementData& rElement = pIter->second;
        if ( !rElement.bDefault )
        {
            if ( xDefaultNameAccess->hasByName( rElement.aName ))
            {
                // Replace settings with data from default layer
                Reference< XIndexAccess > xOldSettings( rElement.xSettings );
                impl_requestUIElementData( nType, LAYER_DEFAULT, rElement );

                ui::ConfigurationEvent aReplaceEvent;
                aReplaceEvent.ResourceURL = rElement.aResourceURL;
                aReplaceEvent.Accessor <<= xThis;
                aReplaceEvent.Source = xIfac;
                aReplaceEvent.ReplacedElement <<= xOldSettings;
                aReplaceEvent.Element <<= rElement.xSettings;

                rReplaceNotifyContainer.push_back( aReplaceEvent );

                // Mark element as default and not modified. That means "not active"
                // in the user layer anymore.
                rElement.bModified = false;
                rElement.bDefault  = true;
            }
            else
            {
                // Remove user-defined settings from user layer
                ui::ConfigurationEvent aEvent;
                aEvent.ResourceURL = rElement.aResourceURL;
                aEvent.Accessor <<= xThis;
                aEvent.Source = xIfac;
                aEvent.Element <<= rElement.xSettings;

                rRemoveNotifyContainer.push_back( aEvent );

                // Mark element as default and not modified. That means "not active"
                // in the user layer anymore.
                rElement.bModified = false;
                rElement.bDefault  = true;
            }
        }

        ++pIter;
    }

    // Remove all settings from our user interface elements
    rHashMap.clear();
}

void ModuleUIConfigurationManager::impl_reloadElementTypeData(
    UIElementType&              rUserElementType,
    UIElementType&              rDefaultElementType,
    ConfigEventNotifyContainer& rRemoveNotifyContainer,
    ConfigEventNotifyContainer& rReplaceNotifyContainer )
{
    UIElementDataHashMap& rHashMap          = rUserElementType.aElementsHashMap;
    UIElementDataHashMap::iterator pIter    = rHashMap.begin();
    Reference< XStorage > xUserStorage( rUserElementType.xStorage );
    Reference< XStorage > xDefaultStorage( rDefaultElementType.xStorage );
    Reference< XNameAccess > xUserNameAccess( rUserElementType.xStorage, UNO_QUERY );
    Reference< XNameAccess > xDefaultNameAccess( rDefaultElementType.xStorage, UNO_QUERY );

    Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XInterface > xIfac( xThis, UNO_QUERY );
    sal_Int16 nType = rUserElementType.nElementType;

    while ( pIter != rHashMap.end() )
    {
        UIElementData& rElement = pIter->second;
        if ( rElement.bModified )
        {
            if ( xUserNameAccess->hasByName( rElement.aName ))
            {
                // Replace settings with data from user layer
                Reference< XIndexAccess > xOldSettings( rElement.xSettings );

                impl_requestUIElementData( nType, LAYER_USERDEFINED, rElement );

                ui::ConfigurationEvent aReplaceEvent;

                aReplaceEvent.ResourceURL = rElement.aResourceURL;
                aReplaceEvent.Accessor <<= xThis;
                aReplaceEvent.Source = xIfac;
                aReplaceEvent.ReplacedElement <<= xOldSettings;
                aReplaceEvent.Element <<= rElement.xSettings;
                rReplaceNotifyContainer.push_back( aReplaceEvent );

                rElement.bModified = false;
            }
            else if ( xDefaultNameAccess->hasByName( rElement.aName ))
            {
                // Replace settings with data from default layer
                Reference< XIndexAccess > xOldSettings( rElement.xSettings );

                impl_requestUIElementData( nType, LAYER_DEFAULT, rElement );

                ui::ConfigurationEvent aReplaceEvent;

                aReplaceEvent.ResourceURL = rElement.aResourceURL;
                aReplaceEvent.Accessor <<= xThis;
                aReplaceEvent.Source = xIfac;
                aReplaceEvent.ReplacedElement <<= xOldSettings;
                aReplaceEvent.Element <<= rElement.xSettings;
                rReplaceNotifyContainer.push_back( aReplaceEvent );

                // Mark element as default and not modified. That means "not active"
                // in the user layer anymore.
                rElement.bModified = false;
                rElement.bDefault  = true;
            }
            else
            {
                // Element settings are not in any storage => remove
                ui::ConfigurationEvent aRemoveEvent;

                aRemoveEvent.ResourceURL = rElement.aResourceURL;
                aRemoveEvent.Accessor <<= xThis;
                aRemoveEvent.Source = xIfac;
                aRemoveEvent.Element <<= rElement.xSettings;

                rRemoveNotifyContainer.push_back( aRemoveEvent );

                // Mark element as default and not modified. That means "not active"
                // in the user layer anymore.
                rElement.bModified = false;
                rElement.bDefault  = true;
            }
        }
        ++pIter;
    }

    rUserElementType.bModified = false;
}

void ModuleUIConfigurationManager::impl_Initialize()
{
    // Initialize the top-level structures with the storage data
    if ( m_xUserConfigStorage.is() )
    {
        // Try to access our module sub folder
        for ( sal_Int16 i = 1; i < css::ui::UIElementType::COUNT;
              i++ )
        {
            Reference< XStorage > xElementTypeStorage;
            try
            {
                if ( m_pStorageHandler[i] )
                    xElementTypeStorage = m_pStorageHandler[i]->getWorkingStorageUser();
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

            m_aUIElements[LAYER_USERDEFINED][i].nElementType = i;
            m_aUIElements[LAYER_USERDEFINED][i].bModified = false;
            m_aUIElements[LAYER_USERDEFINED][i].xStorage = xElementTypeStorage;
            m_aUIElements[LAYER_USERDEFINED][i].bDefaultLayer = false;
        }
    }

    if ( m_xDefaultConfigStorage.is() )
    {
        Reference< XNameAccess > xNameAccess( m_xDefaultConfigStorage, UNO_QUERY_THROW );

        // Try to access our module sub folder
        for ( sal_Int16 i = 1; i < css::ui::UIElementType::COUNT;
              i++ )
        {
            Reference< XStorage > xElementTypeStorage;
            try
            {
                const OUString sName( OUString::createFromAscii( UIELEMENTTYPENAMES[i] ) );
                if( xNameAccess->hasByName( sName ) )
                    xNameAccess->getByName( sName ) >>= xElementTypeStorage;
            }
            catch ( const css::container::NoSuchElementException& )
            {
            }

            m_aUIElements[LAYER_DEFAULT][i].nElementType = i;
            m_aUIElements[LAYER_DEFAULT][i].bModified = false;
            m_aUIElements[LAYER_DEFAULT][i].xStorage = xElementTypeStorage;
            m_aUIElements[LAYER_DEFAULT][i].bDefaultLayer = true;
        }
    }
}

ModuleUIConfigurationManager::ModuleUIConfigurationManager(
        const Reference< XComponentContext >& xContext,
        const css::uno::Sequence< css::uno::Any >& aArguments)
    : m_xDefaultConfigStorage( nullptr )
    , m_xUserConfigStorage( nullptr )
    , m_bReadOnly( true )
    , m_bModified( false )
    , m_bDisposed( false )
    , m_aXMLPostfix( ".xml" )
    , m_aPropUIName( "UIName" )
    , m_aPropResourceURL( "ResourceURL" )
    , m_xContext( xContext )
    , m_aListenerContainer( m_mutex )
{
    for (PresetHandler* & i : m_pStorageHandler)
        i = nullptr;

    // Make sure we have a default initialized entry for every layer and user interface element type!
    // The following code depends on this!
    m_aUIElements[LAYER_DEFAULT].resize( css::ui::UIElementType::COUNT );
    m_aUIElements[LAYER_USERDEFINED].resize( css::ui::UIElementType::COUNT );

    SolarMutexGuard g;

    if( aArguments.getLength() == 2 && (aArguments[0] >>= m_aModuleShortName) && (aArguments[1] >>= m_aModuleIdentifier))
    {
    }
    else
    {
        ::comphelper::SequenceAsHashMap lArgs(aArguments);
        m_aModuleShortName  = lArgs.getUnpackedValueOrDefault("ModuleShortName", OUString());
        m_aModuleIdentifier = lArgs.getUnpackedValueOrDefault("ModuleIdentifier", OUString());
    }

    for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
    {
        OUString aResourceType;
        if ( i == css::ui::UIElementType::MENUBAR )
            aResourceType = RESOURCETYPE_MENUBAR;
        else if ( i == css::ui::UIElementType::TOOLBAR )
            aResourceType = RESOURCETYPE_TOOLBAR;
        else if ( i == css::ui::UIElementType::STATUSBAR )
            aResourceType = RESOURCETYPE_STATUSBAR;
        else if ( i == css::ui::UIElementType::POPUPMENU )
            aResourceType = RESOURCETYPE_POPUPMENU;

        if ( !aResourceType.isEmpty() )
        {
            m_pStorageHandler[i] = new PresetHandler( m_xContext );
            m_pStorageHandler[i]->connectToResource( PresetHandler::E_MODULES,
                                                     aResourceType, // this path won't be used later... see next lines!
                                                     m_aModuleShortName,
                                                     css::uno::Reference< css::embed::XStorage >()); // no document root used here!
        }
    }

    // initialize root storages for all resource types
    m_xUserRootCommit.set( m_pStorageHandler[css::ui::UIElementType::MENUBAR]->getOrCreateRootStorageUser(), css::uno::UNO_QUERY); // can be empty
    m_xDefaultConfigStorage = m_pStorageHandler[css::ui::UIElementType::MENUBAR]->getParentStorageShare(
                                m_pStorageHandler[css::ui::UIElementType::MENUBAR]->getWorkingStorageShare());
    m_xUserConfigStorage    = m_pStorageHandler[css::ui::UIElementType::MENUBAR]->getParentStorageUser(
                                m_pStorageHandler[css::ui::UIElementType::MENUBAR]->getWorkingStorageUser());

    if ( m_xUserConfigStorage.is() )
    {
        Reference< XPropertySet > xPropSet( m_xUserConfigStorage, UNO_QUERY );
        if ( xPropSet.is() )
        {
            long nOpenMode = 0;
            Any a = xPropSet->getPropertyValue("OpenMode");
            if ( a >>= nOpenMode )
                m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
        }
    }

    impl_Initialize();
}

ModuleUIConfigurationManager::~ModuleUIConfigurationManager()
{
    for (PresetHandler* i : m_pStorageHandler)
        delete i;
}

// XComponent
void SAL_CALL ModuleUIConfigurationManager::dispose()
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aGuard;
    Reference< XComponent > xModuleImageManager( m_xModuleImageManager );
    m_xModuleImageManager.clear();
    Reference< XComponent > xCompMAM( m_xModuleAcceleratorManager, UNO_QUERY );
    if ( xCompMAM.is() )
        xCompMAM->dispose();
    m_xModuleAcceleratorManager.clear();
    m_aUIElements[LAYER_USERDEFINED].clear();
    m_aUIElements[LAYER_DEFAULT].clear();
    m_xDefaultConfigStorage.clear();
    m_xUserConfigStorage.clear();
    m_xUserRootCommit.clear();
    m_bModified = false;
    m_bDisposed = true;
    aGuard.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    try
    {
        if ( xModuleImageManager.is() )
            xModuleImageManager->dispose();
    }
    catch ( const Exception& )
    {
    }
}

void SAL_CALL ModuleUIConfigurationManager::addEventListener( const Reference< XEventListener >& xListener )
{
    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL ModuleUIConfigurationManager::removeEventListener( const Reference< XEventListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

// XUIConfiguration
void SAL_CALL ModuleUIConfigurationManager::addConfigurationListener( const Reference< css::ui::XUIConfigurationListener >& xListener )
{
    {
        SolarMutexGuard g;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( cppu::UnoType<ui::XUIConfigurationListener>::get(), xListener );
}

void SAL_CALL ModuleUIConfigurationManager::removeConfigurationListener( const Reference< css::ui::XUIConfigurationListener >& xListener )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( cppu::UnoType<ui::XUIConfigurationListener>::get(), xListener );
}

// XUIConfigurationManager
void SAL_CALL ModuleUIConfigurationManager::reset()
{
    SolarMutexClearableGuard aGuard;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if ( !isReadOnly() )
    {
        // Remove all elements from our user-defined storage!
        try
        {
            for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
            {
                UIElementType&        rElementType = m_aUIElements[LAYER_USERDEFINED][i];
                Reference< XStorage > xSubStorage( rElementType.xStorage, UNO_QUERY );

                if ( xSubStorage.is() )
                {
                    bool bCommitSubStorage( false );
                    Sequence< OUString > aUIElementStreamNames = xSubStorage->getElementNames();
                    for ( sal_Int32 j = 0; j < aUIElementStreamNames.getLength(); j++ )
                    {
                        xSubStorage->removeElement( aUIElementStreamNames[j] );
                        bCommitSubStorage = true;
                    }

                    if ( bCommitSubStorage )
                    {
                        Reference< XTransactedObject > xTransactedObject( xSubStorage, UNO_QUERY );
                        if ( xTransactedObject.is() )
                            xTransactedObject->commit();
                        m_pStorageHandler[i]->commitUserChanges();
                    }
                }
            }

            // remove settings from user defined layer and notify listener about removed settings data!
            ConfigEventNotifyContainer aRemoveEventNotifyContainer;
            ConfigEventNotifyContainer aReplaceEventNotifyContainer;
            for ( sal_Int16 j = 1; j < css::ui::UIElementType::COUNT; j++ )
            {
                try
                {
                    UIElementType& rUserElementType     = m_aUIElements[LAYER_USERDEFINED][j];
                    UIElementType& rDefaultElementType  = m_aUIElements[LAYER_DEFAULT][j];

                    impl_resetElementTypeData( rUserElementType, rDefaultElementType, aRemoveEventNotifyContainer, aReplaceEventNotifyContainer );
                    rUserElementType.bModified = false;
                }
                catch (const Exception& e)
                {
                    css::uno::Any a(e);
                    throw css::lang::WrappedTargetRuntimeException(
                            "ModuleUIConfigurationManager::reset exception",
                            css::uno::Reference<css::uno::XInterface>(*this), a);
                }
            }

            m_bModified = false;

            // Unlock mutex before notify our listeners
            aGuard.clear();

            // Notify our listeners
            sal_uInt32 k = 0;
            for ( k = 0; k < aRemoveEventNotifyContainer.size(); k++ )
                implts_notifyContainerListener( aRemoveEventNotifyContainer[k], NotifyOp_Remove );
            for ( k = 0; k < aReplaceEventNotifyContainer.size(); k++ )
                implts_notifyContainerListener( aReplaceEventNotifyContainer[k], NotifyOp_Replace );
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
}

Sequence< Sequence< PropertyValue > > SAL_CALL ModuleUIConfigurationManager::getUIElementsInfo( sal_Int16 ElementType )
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
            impl_fillSequenceWithElementTypeInfo( aUIElementInfoCollection, sal_Int16( i ) );
    }
    else
        impl_fillSequenceWithElementTypeInfo( aUIElementInfoCollection, ElementType );

    Sequence< PropertyValue > aUIElementInfo( 2 );
    aUIElementInfo[0].Name = m_aPropResourceURL;
    aUIElementInfo[1].Name = m_aPropUIName;

    aElementInfoSeq.resize( aUIElementInfoCollection.size() );
    UIElementInfoHashMap::const_iterator pIter = aUIElementInfoCollection.begin();

    sal_Int32 n = 0;
    while ( pIter != aUIElementInfoCollection.end() )
    {
        aUIElementInfo[0].Value <<= pIter->second.aResourceURL;
        aUIElementInfo[1].Value <<= pIter->second.aUIName;
        aElementInfoSeq[n++] = aUIElementInfo;
        ++pIter;
    }

    return comphelper::containerToSequence(aElementInfoSeq);
}

Reference< XIndexContainer > SAL_CALL ModuleUIConfigurationManager::createSettings()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    // Creates an empty item container which can be filled from outside
    return Reference< XIndexContainer >( static_cast< OWeakObject * >( new RootItemContainer() ), UNO_QUERY );
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::hasSettings( const OUString& ResourceURL )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            throw DisposedException();

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType, false );
        if ( pDataSettings )
            return true;
    }

    return false;
}

Reference< XIndexAccess > SAL_CALL ModuleUIConfigurationManager::getSettings( const OUString& ResourceURL, sal_Bool bWriteable )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            throw DisposedException();

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType );
        if ( pDataSettings )
        {
            // Create a copy of our data if someone wants to change the data.
            if ( bWriteable )
                return Reference< XIndexAccess >( static_cast< OWeakObject * >( new RootItemContainer( pDataSettings->xSettings ) ), UNO_QUERY );
            else
                return pDataSettings->xSettings;
        }
    }

    throw NoSuchElementException();
}

void SAL_CALL ModuleUIConfigurationManager::replaceSettings( const OUString& ResourceURL, const Reference< css::container::XIndexAccess >& aNewData )
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
        if ( pDataSettings )
        {
            if ( !pDataSettings->bDefaultNode )
            {
                // we have a settings entry in our user-defined layer - replace
                Reference< XIndexAccess > xOldSettings = pDataSettings->xSettings;

                // Create a copy of the data if the container is not const
                Reference< XIndexReplace > xReplace( aNewData, UNO_QUERY );
                if ( xReplace.is() )
                    pDataSettings->xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
                else
                    pDataSettings->xSettings = aNewData;
                pDataSettings->bDefault  = false;
                pDataSettings->bModified = true;
                m_bModified = true;

                // Modify type container
                UIElementType& rElementType = m_aUIElements[LAYER_USERDEFINED][nElementType];
                rElementType.bModified = true;

                Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
                Reference< XInterface > xIfac( xThis, UNO_QUERY );

                // Create event to notify listener about replaced element settings
                ui::ConfigurationEvent aEvent;
                aEvent.ResourceURL = ResourceURL;
                aEvent.Accessor <<= xThis;
                aEvent.Source = xIfac;
                aEvent.ReplacedElement <<= xOldSettings;
                aEvent.Element <<= pDataSettings->xSettings;

                aGuard.clear();

                implts_notifyContainerListener( aEvent, NotifyOp_Replace );
            }
            else
            {
                // we have no settings in our user-defined layer - insert
                UIElementData aUIElementData;

                aUIElementData.bDefault     = false;
                aUIElementData.bDefaultNode = false;
                aUIElementData.bModified    = true;

                // Create a copy of the data if the container is not const
                Reference< XIndexReplace > xReplace( aNewData, UNO_QUERY );
                if ( xReplace.is() )
                    aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
                else
                    aUIElementData.xSettings = aNewData;
                aUIElementData.aName        = RetrieveNameFromResourceURL( ResourceURL ) + m_aXMLPostfix;
                aUIElementData.aResourceURL = ResourceURL;
                m_bModified = true;

                // Modify type container
                UIElementType& rElementType = m_aUIElements[LAYER_USERDEFINED][nElementType];
                rElementType.bModified = true;

                UIElementDataHashMap& rElements = rElementType.aElementsHashMap;

                // Check our user element settings hash map as it can already contain settings that have been set to default!
                // If no node can be found, we have to insert it.
                UIElementDataHashMap::iterator pIter = rElements.find( ResourceURL );
                if ( pIter != rElements.end() )
                    pIter->second = aUIElementData;
                else
                    rElements.insert( UIElementDataHashMap::value_type( ResourceURL, aUIElementData ));

                Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
                Reference< XInterface > xIfac( xThis, UNO_QUERY );

                // Create event to notify listener about replaced element settings
                ui::ConfigurationEvent aEvent;

                aEvent.ResourceURL = ResourceURL;
                aEvent.Accessor <<= xThis;
                aEvent.Source = xIfac;
                aEvent.ReplacedElement <<= pDataSettings->xSettings;
                aEvent.Element <<= aUIElementData.xSettings;

                aGuard.clear();

                implts_notifyContainerListener( aEvent, NotifyOp_Replace );
            }
        }
        else
            throw NoSuchElementException();
    }
}

void SAL_CALL ModuleUIConfigurationManager::removeSettings( const OUString& ResourceURL )
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
        if ( pDataSettings )
        {
            // If element settings are default, we don't need to change anything!
            if ( pDataSettings->bDefault )
                return;
            else
            {
                Reference< XIndexAccess > xRemovedSettings = pDataSettings->xSettings;
                pDataSettings->bDefault = true;

                // check if this is a default layer node
                if ( !pDataSettings->bDefaultNode )
                    pDataSettings->bModified = true; // we have to remove this node from the user layer!
                pDataSettings->xSettings.clear();
                m_bModified = true; // user layer must be written

                // Modify type container
                UIElementType& rElementType = m_aUIElements[LAYER_USERDEFINED][nElementType];
                rElementType.bModified = true;

                Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
                Reference< XInterface > xIfac( xThis, UNO_QUERY );

                // Check if we have settings in the default layer which replaces the user-defined one!
                UIElementData* pDefaultDataSettings = impl_findUIElementData( ResourceURL, nElementType );
                if ( pDefaultDataSettings )
                {
                    // Create event to notify listener about replaced element settings
                    ui::ConfigurationEvent aEvent;

                    aEvent.ResourceURL = ResourceURL;
                    aEvent.Accessor <<= xThis;
                    aEvent.Source = xIfac;
                    aEvent.Element <<= xRemovedSettings;
                    aEvent.ReplacedElement <<= pDefaultDataSettings->xSettings;

                    aGuard.clear();

                    implts_notifyContainerListener( aEvent, NotifyOp_Replace );
                }
                else
                {
                    // Create event to notify listener about removed element settings
                    ui::ConfigurationEvent aEvent;

                    aEvent.ResourceURL = ResourceURL;
                    aEvent.Accessor <<= xThis;
                    aEvent.Source = xIfac;
                    aEvent.Element <<= xRemovedSettings;

                    aGuard.clear();

                    implts_notifyContainerListener( aEvent, NotifyOp_Remove );
                }
            }
        }
        else
            throw NoSuchElementException();
    }
}

void SAL_CALL ModuleUIConfigurationManager::insertSettings( const OUString& NewResourceURL, const Reference< XIndexAccess >& aNewData )
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

        UIElementData* pDataSettings = impl_findUIElementData( NewResourceURL, nElementType );
        if ( !pDataSettings )
        {
            UIElementData aUIElementData;

            aUIElementData.bDefault     = false;
            aUIElementData.bDefaultNode = false;
            aUIElementData.bModified    = true;

            // Create a copy of the data if the container is not const
            Reference< XIndexReplace > xReplace( aNewData, UNO_QUERY );
            if ( xReplace.is() )
                aUIElementData.xSettings.set( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
            else
                aUIElementData.xSettings = aNewData;
            aUIElementData.aName        = RetrieveNameFromResourceURL( NewResourceURL ) + m_aXMLPostfix;
            aUIElementData.aResourceURL = NewResourceURL;
            m_bModified = true;

            UIElementType& rElementType = m_aUIElements[LAYER_USERDEFINED][nElementType];
            rElementType.bModified = true;

            UIElementDataHashMap& rElements = rElementType.aElementsHashMap;
            rElements.insert( UIElementDataHashMap::value_type( NewResourceURL, aUIElementData ));

            Reference< XIndexAccess > xInsertSettings( aUIElementData.xSettings );
            Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );

            // Create event to notify listener about removed element settings
            ui::ConfigurationEvent aEvent;

            aEvent.ResourceURL = NewResourceURL;
            aEvent.Accessor <<= xThis;
            aEvent.Source = xThis;
            aEvent.Element <<= xInsertSettings;

            aGuard.clear();

            implts_notifyContainerListener( aEvent, NotifyOp_Insert );
        }
        else
            throw ElementExistException();
    }
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getImageManager()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xModuleImageManager.is() )
    {
        m_xModuleImageManager.set( static_cast< cppu::OWeakObject *>( new ModuleImageManager( m_xContext )),
                                   UNO_QUERY );
        Reference< XInitialization > xInit( m_xModuleImageManager, UNO_QUERY );

        Sequence< Any > aPropSeq( 3 );
        PropertyValue aPropValue;
        aPropValue.Name  = "UserConfigStorage";
        aPropValue.Value = makeAny( m_xUserConfigStorage );
        aPropSeq[0] = makeAny( aPropValue );
        aPropValue.Name  = "ModuleIdentifier";
        aPropValue.Value = makeAny( m_aModuleIdentifier );
        aPropSeq[1] = makeAny( aPropValue );
        aPropValue.Name  = "UserRootCommit";
        aPropValue.Value = makeAny( m_xUserRootCommit );
        aPropSeq[2] = makeAny( aPropValue );

        xInit->initialize( aPropSeq );
    }

    return Reference< XInterface >( m_xModuleImageManager, UNO_QUERY );
}

Reference< ui::XAcceleratorConfiguration > SAL_CALL ModuleUIConfigurationManager::getShortCutManager()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xModuleAcceleratorManager.is() ) try
    {
        m_xModuleAcceleratorManager = ui::ModuleAcceleratorConfiguration::
            createWithModuleIdentifier(m_xContext, m_aModuleIdentifier);
    }
    catch ( const css::uno::DeploymentException& )
    {
        SAL_WARN("fwk.uiconfiguration", "ModuleAcceleratorConfiguration"
                " not available. This should happen only on mobile platforms.");
    }

    return m_xModuleAcceleratorManager;
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getEventsManager()
{
    return Reference< XInterface >();
}

// XModuleUIConfigurationManager
sal_Bool SAL_CALL ModuleUIConfigurationManager::isDefaultSettings( const OUString& ResourceURL )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            throw DisposedException();

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType, false );
        if ( pDataSettings && pDataSettings->bDefaultNode )
            return true;
    }

    return false;
}

Reference< XIndexAccess > SAL_CALL ModuleUIConfigurationManager::getDefaultSettings( const OUString& ResourceURL )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == css::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= css::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        SolarMutexGuard g;

        if ( m_bDisposed )
            throw DisposedException();

        // preload list of element types on demand
        impl_preloadUIElementTypeList( LAYER_DEFAULT, nElementType );

        // Look into our default vector/unordered_map combination
        UIElementDataHashMap& rDefaultHashMap = m_aUIElements[LAYER_DEFAULT][nElementType].aElementsHashMap;
        UIElementDataHashMap::iterator pIter = rDefaultHashMap.find( ResourceURL );
        if ( pIter != rDefaultHashMap.end() )
        {
            if ( !pIter->second.xSettings.is() )
                impl_requestUIElementData( nElementType, LAYER_DEFAULT, pIter->second );
            return pIter->second.xSettings;
        }
    }

    // Nothing has been found!
    throw NoSuchElementException();
}

// XUIConfigurationPersistence
void SAL_CALL ModuleUIConfigurationManager::reload()
{
    SolarMutexClearableGuard aGuard;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xUserConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        ConfigEventNotifyContainer aRemoveNotifyContainer;
        ConfigEventNotifyContainer aReplaceNotifyContainer;
        for ( sal_Int16 i = 1; i < css::ui::UIElementType::COUNT; i++ )
        {
            try
            {
                UIElementType& rUserElementType    = m_aUIElements[LAYER_USERDEFINED][i];
                UIElementType& rDefaultElementType = m_aUIElements[LAYER_DEFAULT][i];

                if ( rUserElementType.bModified )
                    impl_reloadElementTypeData( rUserElementType, rDefaultElementType, aRemoveNotifyContainer, aReplaceNotifyContainer );
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
        for (ui::ConfigurationEvent & j : aRemoveNotifyContainer)
            implts_notifyContainerListener( j, NotifyOp_Remove );
        for (ui::ConfigurationEvent & k : aReplaceNotifyContainer)
            implts_notifyContainerListener( k, NotifyOp_Replace );
    }
}

void SAL_CALL ModuleUIConfigurationManager::store()
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xUserConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
        {
            try
            {
                UIElementType&        rElementType = m_aUIElements[LAYER_USERDEFINED][i];
                Reference< XStorage > xStorage( rElementType.xStorage, UNO_QUERY );

                if ( rElementType.bModified && xStorage.is() )
                {
                    impl_storeElementTypeData( xStorage, rElementType );
                    m_pStorageHandler[i]->commitUserChanges();
                }
            }
            catch ( const Exception& )
            {
                throw IOException();
            }
        }

        m_bModified = false;
    }
}

void SAL_CALL ModuleUIConfigurationManager::storeToStorage( const Reference< XStorage >& Storage )
{
    SolarMutexGuard g;

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xUserConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        for ( int i = 1; i < css::ui::UIElementType::COUNT; i++ )
        {
            try
            {
                Reference< XStorage > xElementTypeStorage( Storage->openStorageElement(
                                                            OUString::createFromAscii( UIELEMENTTYPENAMES[i] ), ElementModes::READWRITE ));
                UIElementType&        rElementType = m_aUIElements[LAYER_USERDEFINED][i];

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
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::isModified()
{
    SolarMutexGuard g;

    return m_bModified;
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::isReadOnly()
{
    SolarMutexGuard g;

    return m_bReadOnly;
}

void ModuleUIConfigurationManager::implts_notifyContainerListener( const ui::ConfigurationEvent& aEvent, NotifyOp eOp )
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( cppu::UnoType<css::ui::XUIConfigurationListener>::get());
    if ( pContainer != nullptr )
    {
        ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
        while ( pIterator.hasMoreElements() )
        {
            try
            {
                switch ( eOp )
                {
                    case NotifyOp_Replace:
                        static_cast< css::ui::XUIConfigurationListener*>(pIterator.next())->elementReplaced( aEvent );
                        break;
                    case NotifyOp_Insert:
                        static_cast< css::ui::XUIConfigurationListener*>(pIterator.next())->elementInserted( aEvent );
                        break;
                    case NotifyOp_Remove:
                        static_cast< css::ui::XUIConfigurationListener*>(pIterator.next())->elementRemoved( aEvent );
                        break;
                }
            }
            catch( const css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ModuleUIConfigurationManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &arguments)
{
    return cppu::acquire(new ModuleUIConfigurationManager(context, arguments));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
