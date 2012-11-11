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

#include <uiconfiguration/moduleuiconfigurationmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>
#include <uielement/constitemcontainer.hxx>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/uielementtypenames.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/toolboxconfiguration.hxx>

#include <framework/statusbarconfiguration.hxx>

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XStream.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/sequenceashashmap.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using rtl::OUString;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;
using namespace com::sun::star::embed;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_8                    (    ModuleUIConfigurationManager                                                    ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                         ),
                                            DIRECT_INTERFACE( css::lang::XInitialization                                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfiguration                  ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationManager           ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XModuleUIConfigurationManager     ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationPersistence       )
                                        )

DEFINE_XTYPEPROVIDER_8                  (   ModuleUIConfigurationManager                                ,
                                            css::lang::XTypeProvider                                    ,
                                            css::lang::XServiceInfo                                     ,
                                            css::lang::XComponent                                       ,
                                            css::lang::XInitialization                                  ,
                                            ::com::sun::star::ui::XUIConfiguration                ,
                                            ::com::sun::star::ui::XUIConfigurationManager         ,
                                            ::com::sun::star::ui::XModuleUIConfigurationManager   ,
                                            ::com::sun::star::ui::XUIConfigurationPersistence
                                        )

DEFINE_XSERVICEINFO_MULTISERVICE        (   ModuleUIConfigurationManager                    ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_MODULEUICONFIGURATIONMANAGER        ,
                                            IMPLEMENTATIONNAME_MODULEUICONFIGURATIONMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   ModuleUIConfigurationManager, {} )


// important: The order and position of the elements must match the constant
// definition of "::com::sun::star::ui::UIElementType"
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
static const sal_Int32  RESOURCEURL_PREFIX_SIZE = 17;
static const char       RESOURCEURL_CUSTOM_ELEMENT[] = "custom_";

static sal_Int16 RetrieveTypeFromResourceURL( const rtl::OUString& aResourceURL )
{

    if (( aResourceURL.indexOf( RESOURCEURL_PREFIX ) == 0 ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        OUString    aTmpStr     = aResourceURL.copy( RESOURCEURL_PREFIX_SIZE );
        sal_Int32   nIndex      = aTmpStr.indexOf( '/' );
        if (( nIndex > 0 ) &&  ( aTmpStr.getLength() > nIndex ))
        {
            OUString aTypeStr( aTmpStr.copy( 0, nIndex ));
            for ( int i = 0; i < UIElementType::COUNT; i++ )
            {
                if ( aTypeStr.equalsAscii( UIELEMENTTYPENAMES[i] ))
                    return sal_Int16( i );
            }
        }
    }

    return UIElementType::UNKNOWN;
}

static OUString RetrieveNameFromResourceURL( const rtl::OUString& aResourceURL )
{
    if (( aResourceURL.indexOf( RESOURCEURL_PREFIX ) == 0 ) &&
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

    OUString aCustomUrlPrefix( RESOURCEURL_CUSTOM_ELEMENT );
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
                rtl::OUString aUIName;
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
                    rtl::OUString aUIName;
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
            rtl::OUStringBuffer aBuf( RESOURCEURL_PREFIX_SIZE );
            aBuf.appendAscii( RESOURCEURL_PREFIX );
            aBuf.appendAscii( UIELEMENTTYPENAMES[ nElementType ] );
            aBuf.appendAscii( "/" );
            OUString aResURLPrefix( aBuf.makeStringAndClear() );

            UIElementDataHashMap& rHashMap = rElementTypeData.aElementsHashMap;
            Reference< XNameAccess > xNameAccess( xElementTypeStorage, UNO_QUERY );
            Sequence< OUString > aUIElementNames = xNameAccess->getElementNames();
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
                        ( aExtension.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xml"))))
                    {
                        aUIElementData.aResourceURL = aResURLPrefix + aUIElementName;
                        aUIElementData.aName        = aUIElementNames[n];

                        if ( eLayer == LAYER_USERDEFINED )
                        {
                            aUIElementData.bModified    = false;
                            aUIElementData.bDefault     = false;
                            aUIElementData.bDefaultNode = false;
                        }

                        // Create boost::unordered_map entries for all user interface elements inside the storage. We don't load the
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
                    case ::com::sun::star::ui::UIElementType::UNKNOWN:
                    break;

                    case ::com::sun::star::ui::UIElementType::MENUBAR:
                    {
                        try
                        {
                            MenuConfiguration aMenuCfg( m_xServiceManager );
                            Reference< XIndexAccess > xContainer( aMenuCfg.CreateMenuBarConfigurationFromXML( xInputStream ));
                            RootItemContainer* pRootItemContainer = RootItemContainer::GetImplementation( xContainer );
                            if ( pRootItemContainer )
                                aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( pRootItemContainer, sal_True ) ), UNO_QUERY );
                            else
                                aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( xContainer, sal_True ) ), UNO_QUERY );
                            return;
                        }
                        catch ( const ::com::sun::star::lang::WrappedTargetException& )
                        {
                        }
                    }
                    break;

                    case ::com::sun::star::ui::UIElementType::POPUPMENU:
                    {
                        break;
                    }

                    case ::com::sun::star::ui::UIElementType::TOOLBAR:
                    {
                        try
                        {
                            Reference< XIndexContainer > xIndexContainer( static_cast< OWeakObject * >( new RootItemContainer() ), UNO_QUERY );
                            ToolBoxConfiguration::LoadToolBox( comphelper::getComponentContext(m_xServiceManager), xInputStream, xIndexContainer );
                            RootItemContainer* pRootItemContainer = RootItemContainer::GetImplementation( xIndexContainer );
                            aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( pRootItemContainer, sal_True ) ), UNO_QUERY );
                            return;
                        }
                        catch ( const ::com::sun::star::lang::WrappedTargetException& )
                        {
                        }

                        break;
                    }

                    case ::com::sun::star::ui::UIElementType::STATUSBAR:
                    {
                        try
                        {
                            Reference< XIndexContainer > xIndexContainer( static_cast< OWeakObject * >( new RootItemContainer() ), UNO_QUERY );
                            StatusBarConfiguration::LoadStatusBar( comphelper::getComponentContext(m_xServiceManager), xInputStream, xIndexContainer );
                            RootItemContainer* pRootItemContainer = RootItemContainer::GetImplementation( xIndexContainer );
                            aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( pRootItemContainer, sal_True ) ), UNO_QUERY );
                            return;
                        }
                        catch ( const ::com::sun::star::lang::WrappedTargetException& )
                        {
                        }

                        break;
                    }

                    case ::com::sun::star::ui::UIElementType::FLOATINGWINDOW:
                    {
                        break;
                    }
                }
            }
        }
        catch ( const ::com::sun::star::embed::InvalidStorageException& )
        {
        }
        catch ( const ::com::sun::star::lang::IllegalArgumentException& )
        {
        }
        catch ( const ::com::sun::star::io::IOException& )
        {
        }
        catch ( const ::com::sun::star::embed::StorageWrappedTargetException& )
        {
        }
    }

    // At least we provide an empty settings container!
    aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer() ), UNO_QUERY );
}

ModuleUIConfigurationManager::UIElementData*  ModuleUIConfigurationManager::impl_findUIElementData( const rtl::OUString& aResourceURL, sal_Int16 nElementType, bool bLoad )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( LAYER_USERDEFINED, nElementType );
    impl_preloadUIElementTypeList( LAYER_DEFAULT, nElementType );

    // first try to look into our user-defined vector/boost::unordered_map combination
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

    // Not successfull, we have to look into our default vector/boost::unordered_map combination
    UIElementDataHashMap& rDefaultHashMap = m_aUIElements[LAYER_DEFAULT][nElementType].aElementsHashMap;
    pIter = rDefaultHashMap.find( aResourceURL );
    if ( pIter != rDefaultHashMap.end() )
    {
        if ( !pIter->second.xSettings.is() && bLoad )
            impl_requestUIElementData( nElementType, LAYER_DEFAULT, pIter->second );
        return &(pIter->second);
    }

    // Nothing has been found!
    return NULL;
}

void ModuleUIConfigurationManager::impl_storeElementTypeData( Reference< XStorage > xStorage, UIElementType& rElementType, bool bResetModifyState )
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
                rElement.bModified = sal_False; // mark as not modified
            }
            else
            {
                Reference< XStream > xStream( xStorage->openStreamElement( rElement.aName, ElementModes::WRITE|ElementModes::TRUNCATE ), UNO_QUERY );
                Reference< XOutputStream > xOutputStream( xStream->getOutputStream() );

                if ( xOutputStream.is() )
                {
                    switch( rElementType.nElementType )
                    {
                        case ::com::sun::star::ui::UIElementType::MENUBAR:
                        {
                            try
                            {
                                MenuConfiguration aMenuCfg( m_xServiceManager );
                                aMenuCfg.StoreMenuBarConfigurationToXML( rElement.xSettings, xOutputStream );
                            }
                            catch ( const ::com::sun::star::lang::WrappedTargetException& )
                            {
                            }
                        }
                        break;

                        case ::com::sun::star::ui::UIElementType::TOOLBAR:
                        {
                            try
                            {
                                ToolBoxConfiguration::StoreToolBox( comphelper::getComponentContext(m_xServiceManager), xOutputStream, rElement.xSettings );
                            }
                            catch ( const ::com::sun::star::lang::WrappedTargetException& )
                            {
                            }
                        }
                        break;

                        case ::com::sun::star::ui::UIElementType::STATUSBAR:
                        {
                            try
                            {
                                StatusBarConfiguration::StoreStatusBar( comphelper::getComponentContext(m_xServiceManager), xOutputStream, rElement.xSettings );
                            }
                            catch ( const ::com::sun::star::lang::WrappedTargetException& )
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
                    rElement.bModified = sal_False;
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
        rElementType.bModified = sal_False;
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
    Reference< XInterface > xIfac( xThis, UNO_QUERY );
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

                ConfigurationEvent aReplaceEvent;
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
                ConfigurationEvent aEvent;
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

                ConfigurationEvent aReplaceEvent;

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

                ConfigurationEvent aReplaceEvent;

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
                ConfigurationEvent aRemoveEvent;

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

    rUserElementType.bModified = sal_False;
}

void ModuleUIConfigurationManager::impl_Initialize()
{
    // Initialize the top-level structures with the storage data
    if ( m_xUserConfigStorage.is() )
    {
        // Try to access our module sub folder
        for ( sal_Int16 i = 1; i < ::com::sun::star::ui::UIElementType::COUNT;
              i++ )
        {
            Reference< XStorage > xElementTypeStorage;
            try
            {
                if ( m_pStorageHandler[i] )
                    xElementTypeStorage = m_pStorageHandler[i]->getWorkingStorageUser();
            }
            catch ( const com::sun::star::container::NoSuchElementException& )
            {
            }
            catch ( const ::com::sun::star::embed::InvalidStorageException& )
            {
            }
            catch ( const ::com::sun::star::lang::IllegalArgumentException& )
            {
            }
            catch ( const ::com::sun::star::io::IOException& )
            {
            }
            catch ( const ::com::sun::star::embed::StorageWrappedTargetException& )
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
        for ( sal_Int16 i = 1; i < ::com::sun::star::ui::UIElementType::COUNT;
              i++ )
        {
            Reference< XStorage > xElementTypeStorage;
            try
            {
                const OUString sName( OUString::createFromAscii( UIELEMENTTYPENAMES[i] ) );
                if( xNameAccess->hasByName( sName ) )
                    xNameAccess->getByName( sName ) >>= xElementTypeStorage;
            }
            catch ( const com::sun::star::container::NoSuchElementException& )
            {
            }

            m_aUIElements[LAYER_DEFAULT][i].nElementType = i;
            m_aUIElements[LAYER_DEFAULT][i].bModified = false;
            m_aUIElements[LAYER_DEFAULT][i].xStorage = xElementTypeStorage;
            m_aUIElements[LAYER_DEFAULT][i].bDefaultLayer = true;
        }
    }
}

ModuleUIConfigurationManager::ModuleUIConfigurationManager( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xDefaultConfigStorage( 0 )
    , m_xUserConfigStorage( 0 )
    , m_bReadOnly( true )
    , m_bInitialized( false )
    , m_bModified( false )
    , m_bConfigRead( false )
    , m_bDisposed( false )
    , m_aXMLPostfix( ".xml" )
    , m_aPropUIName( "UIName" )
    , m_aPropResourceURL( "ResourceURL" )
    , m_xServiceManager( xServiceManager )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    for ( int i = 0; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
        m_pStorageHandler[i] = 0;

    // Make sure we have a default initialized entry for every layer and user interface element type!
    // The following code depends on this!
    m_aUIElements[LAYER_DEFAULT].resize( ::com::sun::star::ui::UIElementType::COUNT );
    m_aUIElements[LAYER_USERDEFINED].resize( ::com::sun::star::ui::UIElementType::COUNT );
}

ModuleUIConfigurationManager::~ModuleUIConfigurationManager()
{
    for ( int i = 0; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
        delete m_pStorageHandler[i];
}

// XComponent
void SAL_CALL ModuleUIConfigurationManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ResetableGuard aGuard( m_aLock );
    Reference< XComponent > xModuleImageManager( m_xModuleImageManager );
    m_xModuleImageManager.clear();
    m_xModuleAcceleratorManager.clear();
    m_aUIElements[LAYER_USERDEFINED].clear();
    m_aUIElements[LAYER_DEFAULT].clear();
    m_xDefaultConfigStorage.clear();
    m_xUserConfigStorage.clear();
    m_xUserRootCommit.clear();
    m_bConfigRead = false;
    m_bModified = false;
    m_bDisposed = true;
    aGuard.unlock();
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

void SAL_CALL ModuleUIConfigurationManager::addEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL ModuleUIConfigurationManager::removeEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

// XInitialization
void SAL_CALL ModuleUIConfigurationManager::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    ResetableGuard aLock( m_aLock );

    if ( !m_bInitialized )
    {
        ::comphelper::SequenceAsHashMap lArgs(aArguments);
        m_aModuleIdentifier = lArgs.getUnpackedValueOrDefault(::rtl::OUString("ModuleIdentifier"), ::rtl::OUString());
        m_aModuleShortName  = lArgs.getUnpackedValueOrDefault(::rtl::OUString("ModuleShortName"), ::rtl::OUString());

        for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
        {
            rtl::OUString aResourceType;
            if ( i == ::com::sun::star::ui::UIElementType::MENUBAR )
                aResourceType = PresetHandler::RESOURCETYPE_MENUBAR();
            else if ( i == ::com::sun::star::ui::UIElementType::TOOLBAR )
                aResourceType = PresetHandler::RESOURCETYPE_TOOLBAR();
            else if ( i == ::com::sun::star::ui::UIElementType::STATUSBAR )
                aResourceType = PresetHandler::RESOURCETYPE_STATUSBAR();

            if ( !aResourceType.isEmpty() )
            {
                m_pStorageHandler[i] = new PresetHandler( m_xServiceManager );
                m_pStorageHandler[i]->connectToResource( PresetHandler::E_MODULES,
                                                         aResourceType, // this path wont be used later ... seee next lines!
                                                         m_aModuleShortName,
                                                         css::uno::Reference< css::embed::XStorage >()); // no document root used here!
            }
        }

        // initialize root storages for all resource types
        m_xUserRootCommit       = css::uno::Reference< css::embed::XTransactedObject >(
                                    m_pStorageHandler[::com::sun::star::ui::UIElementType::MENUBAR]->getOrCreateRootStorageUser(), css::uno::UNO_QUERY); // can be empty
        m_xDefaultConfigStorage = m_pStorageHandler[::com::sun::star::ui::UIElementType::MENUBAR]->getParentStorageShare(
                                    m_pStorageHandler[::com::sun::star::ui::UIElementType::MENUBAR]->getWorkingStorageShare());
        m_xUserConfigStorage    = m_pStorageHandler[::com::sun::star::ui::UIElementType::MENUBAR]->getParentStorageUser(
                                    m_pStorageHandler[::com::sun::star::ui::UIElementType::MENUBAR]->getWorkingStorageUser());

        if ( m_xUserConfigStorage.is() )
        {
            Reference< XPropertySet > xPropSet( m_xUserConfigStorage, UNO_QUERY );
            if ( xPropSet.is() )
            {
                long nOpenMode = 0;
                Any a = xPropSet->getPropertyValue( rtl::OUString( "OpenMode" ));
                if ( a >>= nOpenMode )
                    m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
            }
        }

        impl_Initialize();

        m_bInitialized = true;
    }
}

// XUIConfiguration
void SAL_CALL ModuleUIConfigurationManager::addConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XUIConfigurationListener >* ) NULL ), xListener );
}

void SAL_CALL ModuleUIConfigurationManager::removeConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XUIConfigurationListener >* ) NULL ), xListener );
}


// XUIConfigurationManager
void SAL_CALL ModuleUIConfigurationManager::reset() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if ( !isReadOnly() )
    {
        // Remove all elements from our user-defined storage!
        try
        {
            for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
            {
                UIElementType&        rElementType = m_aUIElements[LAYER_USERDEFINED][i];
                Reference< XStorage > xSubStorage( rElementType.xStorage, UNO_QUERY );

                if ( xSubStorage.is() )
                {
                    bool bCommitSubStorage( false );
                    Reference< XNameAccess > xSubStorageNameAccess( xSubStorage, UNO_QUERY );
                    Sequence< OUString > aUIElementStreamNames = xSubStorageNameAccess->getElementNames();
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
            for ( sal_Int16 j = 1; j < ::com::sun::star::ui::UIElementType::COUNT; j++ )
            {
                try
                {
                    UIElementType& rUserElementType     = m_aUIElements[LAYER_USERDEFINED][j];
                    UIElementType& rDefaultElementType  = m_aUIElements[LAYER_DEFAULT][j];

                    impl_resetElementTypeData( rUserElementType, rDefaultElementType, aRemoveEventNotifyContainer, aReplaceEventNotifyContainer );
                    rUserElementType.bModified = sal_False;
                }
                catch ( const Exception& )
                {
                    throw IOException();
                }
            }

            m_bModified = sal_False;

            // Unlock mutex before notify our listeners
            aGuard.unlock();

            // Notify our listeners
            sal_uInt32 k = 0;
            for ( k = 0; k < aRemoveEventNotifyContainer.size(); k++ )
                implts_notifyContainerListener( aRemoveEventNotifyContainer[k], NotifyOp_Remove );
            for ( k = 0; k < aReplaceEventNotifyContainer.size(); k++ )
                implts_notifyContainerListener( aReplaceEventNotifyContainer[k], NotifyOp_Replace );
        }
        catch ( const ::com::sun::star::lang::IllegalArgumentException& )
        {
        }
        catch ( const ::com::sun::star::container::NoSuchElementException& )
        {
        }
        catch ( const ::com::sun::star::embed::InvalidStorageException& )
        {
        }
        catch ( const ::com::sun::star::embed::StorageWrappedTargetException& )
        {
        }
    }
}

Sequence< Sequence< PropertyValue > > SAL_CALL ModuleUIConfigurationManager::getUIElementsInfo( sal_Int16 ElementType )
throw ( IllegalArgumentException, RuntimeException )
{
    if (( ElementType < 0 ) || ( ElementType >= ::com::sun::star::ui::UIElementType::COUNT ))
        throw IllegalArgumentException();

    ResetableGuard aGuard( m_aLock );
    if ( m_bDisposed )
        throw DisposedException();

    Sequence< Sequence< PropertyValue > > aElementInfoSeq;
    UIElementInfoHashMap aUIElementInfoCollection;

    if ( ElementType == ::com::sun::star::ui::UIElementType::UNKNOWN )
    {
        for ( sal_Int16 i = 0; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
            impl_fillSequenceWithElementTypeInfo( aUIElementInfoCollection, sal_Int16( i ) );
    }
    else
        impl_fillSequenceWithElementTypeInfo( aUIElementInfoCollection, ElementType );

    Sequence< PropertyValue > aUIElementInfo( 2 );
    aUIElementInfo[0].Name = m_aPropResourceURL;
    aUIElementInfo[1].Name = m_aPropUIName;

    aElementInfoSeq.realloc( aUIElementInfoCollection.size() );
    UIElementInfoHashMap::const_iterator pIter = aUIElementInfoCollection.begin();

    sal_Int32 n = 0;
    while ( pIter != aUIElementInfoCollection.end() )
    {
        aUIElementInfo[0].Value <<= pIter->second.aResourceURL;
        aUIElementInfo[1].Value <<= pIter->second.aUIName;
        aElementInfoSeq[n++] = aUIElementInfo;
        ++pIter;
    }

    return aElementInfoSeq;
}

Reference< XIndexContainer > SAL_CALL ModuleUIConfigurationManager::createSettings() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    // Creates an empty item container which can be filled from outside
    return Reference< XIndexContainer >( static_cast< OWeakObject * >( new RootItemContainer() ), UNO_QUERY );
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::hasSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            throw DisposedException();

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType, false );
        if ( pDataSettings )
            return sal_True;
    }

    return sal_False;
}

Reference< XIndexAccess > SAL_CALL ModuleUIConfigurationManager::getSettings( const ::rtl::OUString& ResourceURL, sal_Bool bWriteable )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        ResetableGuard aGuard( m_aLock );

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

void SAL_CALL ModuleUIConfigurationManager::replaceSettings( const ::rtl::OUString& ResourceURL, const Reference< ::com::sun::star::container::XIndexAccess >& aNewData )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IllegalAccessException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else if ( m_bReadOnly )
        throw IllegalAccessException();
    else
    {
        ResetableGuard aGuard( m_aLock );

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
                    pDataSettings->xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
                else
                    pDataSettings->xSettings = aNewData;
                pDataSettings->bDefault  = false;
                pDataSettings->bModified = true;
                m_bModified = true;

                // Modify type container
                UIElementType& rElementType = m_aUIElements[LAYER_USERDEFINED][nElementType];
                rElementType.bModified = true;

                Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );

                // Create event to notify listener about replaced element settings
                ConfigurationEvent aEvent;
                Reference< XInterface > xIfac( xThis, UNO_QUERY );

                aEvent.ResourceURL = ResourceURL;
                aEvent.Accessor <<= xThis;
                aEvent.Source = xIfac;
                aEvent.ReplacedElement <<= xOldSettings;
                aEvent.Element <<= pDataSettings->xSettings;

                aGuard.unlock();

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
                    aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
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
                ConfigurationEvent aEvent;

                aEvent.ResourceURL = ResourceURL;
                aEvent.Accessor <<= xThis;
                aEvent.Source = xIfac;
                aEvent.ReplacedElement <<= pDataSettings->xSettings;
                aEvent.Element <<= aUIElementData.xSettings;

                aGuard.unlock();

                implts_notifyContainerListener( aEvent, NotifyOp_Replace );
            }
        }
        else
            throw NoSuchElementException();
    }
}

void SAL_CALL ModuleUIConfigurationManager::removeSettings( const ::rtl::OUString& ResourceURL )
throw ( NoSuchElementException, IllegalArgumentException, IllegalAccessException, RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else if ( m_bReadOnly )
        throw IllegalAccessException();
    else
    {
        ResetableGuard aGuard( m_aLock );

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
                    ConfigurationEvent aEvent;

                    aEvent.ResourceURL = ResourceURL;
                    aEvent.Accessor <<= xThis;
                    aEvent.Source = xIfac;
                    aEvent.Element <<= xRemovedSettings;
                    aEvent.ReplacedElement <<= pDefaultDataSettings->xSettings;

                    aGuard.unlock();

                    implts_notifyContainerListener( aEvent, NotifyOp_Replace );
                }
                else
                {
                    // Create event to notify listener about removed element settings
                    ConfigurationEvent aEvent;

                    aEvent.ResourceURL = ResourceURL;
                    aEvent.Accessor <<= xThis;
                    aEvent.Source = xIfac;
                    aEvent.Element <<= xRemovedSettings;

                    aGuard.unlock();

                    implts_notifyContainerListener( aEvent, NotifyOp_Remove );
                }
            }
        }
        else
            throw NoSuchElementException();
    }
}

void SAL_CALL ModuleUIConfigurationManager::insertSettings( const ::rtl::OUString& NewResourceURL, const Reference< XIndexAccess >& aNewData )
throw ( ElementExistException, IllegalArgumentException, IllegalAccessException, RuntimeException )
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( NewResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else if ( m_bReadOnly )
        throw IllegalAccessException();
    else
    {
        ResetableGuard aGuard( m_aLock );

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
                aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
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
            Reference< XInterface > xIfac( xThis, UNO_QUERY );

            // Create event to notify listener about removed element settings
            ConfigurationEvent aEvent;

            aEvent.ResourceURL = NewResourceURL;
            aEvent.Accessor <<= xThis;
            aEvent.Source = xIfac;
            aEvent.Element <<= xInsertSettings;

            aGuard.unlock();

            implts_notifyContainerListener( aEvent, NotifyOp_Insert );
        }
        else
            throw ElementExistException();
    }
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getImageManager() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xModuleImageManager.is() )
    {
        m_xModuleImageManager = Reference< XComponent >( static_cast< cppu::OWeakObject *>( new ModuleImageManager( m_xServiceManager )),
                                                         UNO_QUERY );
        Reference< XInitialization > xInit( m_xModuleImageManager, UNO_QUERY );

        Sequence< Any > aPropSeq( 3 );
        PropertyValue aPropValue;
        aPropValue.Name  = rtl::OUString( "UserConfigStorage" );
        aPropValue.Value = makeAny( m_xUserConfigStorage );
        aPropSeq[0] = makeAny( aPropValue );
        aPropValue.Name  = rtl::OUString( "ModuleIdentifier" );
        aPropValue.Value = makeAny( m_aModuleIdentifier );
        aPropSeq[1] = makeAny( aPropValue );
        aPropValue.Name  = rtl::OUString( "UserRootCommit" );
        aPropValue.Value = makeAny( m_xUserRootCommit );
        aPropSeq[2] = makeAny( aPropValue );

        xInit->initialize( aPropSeq );
    }

    return Reference< XInterface >( m_xModuleImageManager, UNO_QUERY );
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getShortCutManager() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    Reference< XMultiServiceFactory > xSMGR   = m_xServiceManager;
    ::rtl::OUString                   aModule = m_aModuleIdentifier;

    if ( !m_xModuleAcceleratorManager.is() )
    {
        Reference< XInterface >      xManager = xSMGR->createInstance(SERVICENAME_MODULEACCELERATORCONFIGURATION);
        Reference< XInitialization > xInit    (xManager, UNO_QUERY_THROW);

        PropertyValue aProp;
        aProp.Name    = ::rtl::OUString("ModuleIdentifier");
        aProp.Value <<= aModule;

        Sequence< Any > lArgs(1);
        lArgs[0] <<= aProp;

        xInit->initialize(lArgs);
        m_xModuleAcceleratorManager = Reference< XInterface >( xManager, UNO_QUERY );
    }

    return m_xModuleAcceleratorManager;
}

Reference< XInterface > SAL_CALL ModuleUIConfigurationManager::getEventsManager() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XInterface >();
}

// XModuleUIConfigurationManager
sal_Bool SAL_CALL ModuleUIConfigurationManager::isDefaultSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            throw DisposedException();

        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType, false );
        if ( pDataSettings && pDataSettings->bDefaultNode )
            return sal_True;
    }

    return sal_False;
}

Reference< XIndexAccess > SAL_CALL ModuleUIConfigurationManager::getDefaultSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        ResetableGuard aGuard( m_aLock );

        if ( m_bDisposed )
            throw DisposedException();

        // preload list of element types on demand
        impl_preloadUIElementTypeList( LAYER_DEFAULT, nElementType );

        // Look into our default vector/boost::unordered_map combination
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
void SAL_CALL ModuleUIConfigurationManager::reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xUserConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        ConfigEventNotifyContainer aRemoveNotifyContainer;
        ConfigEventNotifyContainer aReplaceNotifyContainer;
        for ( sal_Int16 i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
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

        m_bModified = sal_False;

        // Unlock mutex before notify our listeners
        aGuard.unlock();

        // Notify our listeners
        for ( sal_uInt32 j = 0; j < aRemoveNotifyContainer.size(); j++ )
            implts_notifyContainerListener( aRemoveNotifyContainer[j], NotifyOp_Remove );
        for ( sal_uInt32 k = 0; k < aReplaceNotifyContainer.size(); k++ )
            implts_notifyContainerListener( aReplaceNotifyContainer[k], NotifyOp_Replace );
    }
}

void SAL_CALL ModuleUIConfigurationManager::store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xUserConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
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

void SAL_CALL ModuleUIConfigurationManager::storeToStorage( const Reference< XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xUserConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
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

sal_Bool SAL_CALL ModuleUIConfigurationManager::isModified() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    return m_bModified;
}

sal_Bool SAL_CALL ModuleUIConfigurationManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    return m_bReadOnly;
}

void ModuleUIConfigurationManager::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
{
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< ::com::sun::star::ui::XUIConfigurationListener >*) NULL ) );
    if ( pContainer != NULL )
    {
        ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
        while ( pIterator.hasMoreElements() )
        {
            try
            {
                switch ( eOp )
                {
                    case NotifyOp_Replace:
                        ((::com::sun::star::ui::XUIConfigurationListener*)pIterator.next())->elementReplaced( aEvent );
                        break;
                    case NotifyOp_Insert:
                        ((::com::sun::star::ui::XUIConfigurationListener*)pIterator.next())->elementInserted( aEvent );
                        break;
                    case NotifyOp_Remove:
                        ((::com::sun::star::ui::XUIConfigurationListener*)pIterator.next())->elementRemoved( aEvent );
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

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
