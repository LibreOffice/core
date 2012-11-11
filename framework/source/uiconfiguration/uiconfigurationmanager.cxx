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

#include <uiconfiguration/uiconfigurationmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>
#include <uielement/rootitemcontainer.hxx>
#include <uielement/constitemcontainer.hxx>
#include <uielement/uielementtypenames.hxx>
#include <framework/menuconfiguration.hxx>
#include <framework/toolboxconfiguration.hxx>

#include <framework/statusbarconfiguration.hxx>

#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/ConfigurationEvent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>

#include <comphelper/componentcontext.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

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
DEFINE_XINTERFACE_7                    (    UIConfigurationManager                                                          ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( css::lang::XComponent                                         ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfiguration                  ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationManager           ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationPersistence       ),
                                            DIRECT_INTERFACE( ::com::sun::star::ui::XUIConfigurationStorage           )
                                        )

DEFINE_XTYPEPROVIDER_7                  (   UIConfigurationManager                                  ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            css::lang::XComponent                                   ,
                                            ::com::sun::star::ui::XUIConfiguration            ,
                                            ::com::sun::star::ui::XUIConfigurationManager     ,
                                            ::com::sun::star::ui::XUIConfigurationPersistence ,
                                            ::com::sun::star::ui::XUIConfigurationStorage
                                        )

DEFINE_XSERVICEINFO_MULTISERVICE        (   UIConfigurationManager                      ,
                                            ::cppu::OWeakObject                         ,
                                            SERVICENAME_UICONFIGURATIONMANAGER          ,
                                            IMPLEMENTATIONNAME_UICONFIGURATIONMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   UIConfigurationManager, {} )


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

static sal_Int16 RetrieveTypeFromResourceURL( const rtl::OUString& aResourceURL )
{

    if (( aResourceURL.indexOf( RESOURCEURL_PREFIX ) == 0 ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        rtl::OUString aTmpStr     = aResourceURL.copy( RESOURCEURL_PREFIX_SIZE );
        sal_Int32     nIndex      = aTmpStr.indexOf( '/' );
        if (( nIndex > 0 ) &&  ( aTmpStr.getLength() > nIndex ))
        {
            rtl::OUString aTypeStr( aTmpStr.copy( 0, nIndex ));
            for ( int i = 0; i < UIElementType::COUNT; i++ )
            {
                if ( aTypeStr.equalsAscii( UIELEMENTTYPENAMES[i] ))
                    return sal_Int16( i );
            }
        }
    }

    return UIElementType::UNKNOWN;
}

static rtl::OUString RetrieveNameFromResourceURL( const rtl::OUString& aResourceURL )
{
    if (( aResourceURL.indexOf( RESOURCEURL_PREFIX ) == 0 ) &&
        ( aResourceURL.getLength() > RESOURCEURL_PREFIX_SIZE ))
    {
        sal_Int32 nIndex = aResourceURL.lastIndexOf( '/' );
        if (( nIndex > 0 ) && (( nIndex+1 ) < aResourceURL.getLength()))
            return aResourceURL.copy( nIndex+1 );
    }

    return rtl::OUString();
}

void UIConfigurationManager::impl_fillSequenceWithElementTypeInfo( UIElementInfoHashMap& aUIElementInfoCollection, sal_Int16 nElementType )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( nElementType );

    UIElementDataHashMap& rUserElements = m_aUIElements[nElementType].aElementsHashMap;
    UIElementDataHashMap::const_iterator pUserIter = rUserElements.begin();

    while ( pUserIter != rUserElements.end() )
    {
        UIElementData* pDataSettings = impl_findUIElementData( pUserIter->second.aResourceURL, nElementType );
        if ( pDataSettings && !pDataSettings->bDefault )
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
        ++pUserIter;
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
            rtl::OUStringBuffer aBuf( RESOURCEURL_PREFIX_SIZE );
            aBuf.appendAscii( RESOURCEURL_PREFIX );
            aBuf.appendAscii( UIELEMENTTYPENAMES[ nElementType ] );
            aBuf.appendAscii( "/" );
            rtl::OUString aResURLPrefix( aBuf.makeStringAndClear() );

            UIElementDataHashMap& rHashMap = rElementTypeData.aElementsHashMap;
            Reference< XNameAccess > xNameAccess( xElementTypeStorage, UNO_QUERY );
            Sequence< rtl::OUString > aUIElementNames = xNameAccess->getElementNames();
            for ( sal_Int32 n = 0; n < aUIElementNames.getLength(); n++ )
            {
                UIElementData aUIElementData;

                // Resource name must be without ".xml"
                sal_Int32 nIndex = aUIElementNames[n].lastIndexOf( '.' );
                if (( nIndex > 0 ) && ( nIndex < aUIElementNames[n].getLength() ))
                {
                    rtl::OUString aExtension( aUIElementNames[n].copy( nIndex+1 ));
                    rtl::OUString aUIElementName( aUIElementNames[n].copy( 0, nIndex ));

                    if (!aUIElementName.isEmpty() &&
                        ( aExtension.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("xml"))))
                    {
                        aUIElementData.aResourceURL = aResURLPrefix + aUIElementName;
                        aUIElementData.aName        = aUIElementNames[n];
                        aUIElementData.bModified    = false;
                        aUIElementData.bDefault     = false;

                        // Create boost::unordered_map entries for all user interface elements inside the storage. We don't load the
                        // settings to speed up the process.
                        rHashMap.insert( UIElementDataHashMap::value_type( aUIElementData.aResourceURL, aUIElementData ));
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
    aUIElementData.xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer()), UNO_QUERY );
}

UIConfigurationManager::UIElementData* UIConfigurationManager::impl_findUIElementData( const rtl::OUString& aResourceURL, sal_Int16 nElementType, bool bLoad )
{
    // preload list of element types on demand
    impl_preloadUIElementTypeList( nElementType );

    // try to look into our document vector/boost::unordered_map combination
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
    return NULL;
}

void UIConfigurationManager::impl_storeElementTypeData( Reference< XStorage >& xStorage, UIElementType& rElementType, bool bResetModifyState )
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

void UIConfigurationManager::impl_resetElementTypeData(
    UIElementType& rDocElementType,
    ConfigEventNotifyContainer& rRemoveNotifyContainer )
{
    UIElementDataHashMap& rHashMap          = rDocElementType.aElementsHashMap;
    UIElementDataHashMap::iterator pIter    = rHashMap.begin();

    Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XInterface > xIfac( xThis, UNO_QUERY );

    // Make copies of the event structures to be thread-safe. We have to unlock our mutex before calling
    // our listeners!
    while ( pIter != rHashMap.end() )
    {
        UIElementData& rElement = pIter->second;
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

        ++pIter;
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
    UIElementDataHashMap::iterator pIter    = rHashMap.begin();
    Reference< XStorage > xElementStorage( rDocElementType.xStorage );
    Reference< XNameAccess > xElementNameAccess( xElementStorage, UNO_QUERY );

    Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< XInterface > xIfac( xThis, UNO_QUERY );
    sal_Int16 nType = rDocElementType.nElementType;

    while ( pIter != rHashMap.end() )
    {
        UIElementData& rElement = pIter->second;
        if ( rElement.bModified )
        {
            if ( xElementNameAccess->hasByName( rElement.aName ))
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
        ++pIter;
    }

    rDocElementType.bModified = sal_False;
}

void UIConfigurationManager::impl_Initialize()
{
    // Initialize the top-level structures with the storage data
    if ( m_xDocConfigStorage.is() )
    {
        long nModes = m_bReadOnly ? ElementModes::READ : ElementModes::READWRITE;

        // Try to access our module sub folder
        for ( sal_Int16 i = 1; i < ::com::sun::star::ui::UIElementType::COUNT;
              i++ )
        {
            Reference< XStorage > xElementTypeStorage;
            try
            {
                xElementTypeStorage = m_xDocConfigStorage->openStorageElement( rtl::OUString::createFromAscii( UIELEMENTTYPENAMES[i] ), nModes );
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

            m_aUIElements[i].nElementType = i;
            m_aUIElements[i].bModified = false;
            m_aUIElements[i].xStorage = xElementTypeStorage;
            m_aUIElements[i].bDefaultLayer = false;
        }
    }
    else
    {
        // We have no storage, just initialize ui element types with empty storage!
        for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
            m_aUIElements[i].xStorage = m_xDocConfigStorage;
    }
}

UIConfigurationManager::UIConfigurationManager( com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > xServiceManager ) :
    ThreadHelpBase( &Application::GetSolarMutex() )
    , m_xDocConfigStorage( 0 )
    , m_bReadOnly( true )
    , m_bModified( false )
    , m_bConfigRead( false )
    , m_bDisposed( false )
    , m_aXMLPostfix( ".xml" )
    , m_aPropUIName( "UIName" )
    , m_aPropResourceURL( "ResourceURL" )
    , m_xServiceManager( xServiceManager )
    , m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    // Make sure we have a default initialized entry for every layer and user interface element type!
    // The following code depends on this!
    m_aUIElements.resize( ::com::sun::star::ui::UIElementType::COUNT );
}

UIConfigurationManager::~UIConfigurationManager()
{
}

// XComponent
void SAL_CALL UIConfigurationManager::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    {
        ResetableGuard aGuard( m_aLock );
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
        m_bConfigRead = false;
        m_bModified = false;
        m_bDisposed = true;
    }
}

void SAL_CALL UIConfigurationManager::addEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL UIConfigurationManager::removeEventListener( const Reference< XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

// XUIConfigurationManager
void SAL_CALL UIConfigurationManager::addConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    {
        ResetableGuard aGuard( m_aLock );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        if ( m_bDisposed )
            throw DisposedException();
    }

    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XUIConfigurationListener >* ) NULL ), xListener );
}

void SAL_CALL UIConfigurationManager::removeConfigurationListener( const Reference< ::com::sun::star::ui::XUIConfigurationListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XUIConfigurationListener >* ) NULL ), xListener );
}


void SAL_CALL UIConfigurationManager::reset() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( m_bDisposed )
        throw DisposedException();

    if ( isReadOnly() )
        return;

    if ( m_xDocConfigStorage.is() )
    {
        try
        {
            // Remove all elements from our user-defined storage!
            bool bCommit( false );
            for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
            {
                UIElementType&        rElementType = m_aUIElements[i];
                Reference< XStorage > xSubStorage( rElementType.xStorage, UNO_QUERY );

                if ( xSubStorage.is() )
                {
                    bool bCommitSubStorage( false );
                    Reference< XNameAccess > xSubStorageNameAccess( xSubStorage, UNO_QUERY );
                    Sequence< rtl::OUString > aUIElementStreamNames = xSubStorageNameAccess->getElementNames();
                    for ( sal_Int32 j = 0; j < aUIElementStreamNames.getLength(); j++ )
                    {
                        xSubStorage->removeElement( aUIElementStreamNames[j] );
                        bCommitSubStorage = true;
                        bCommit = true;
                    }

                    if ( bCommitSubStorage )
                    {
                        Reference< XTransactedObject > xTransactedObject( xSubStorage, UNO_QUERY );
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
            for ( sal_Int16 j = 1; j < ::com::sun::star::ui::UIElementType::COUNT; j++ )
            {
                UIElementType& rDocElementType = m_aUIElements[j];

                impl_resetElementTypeData( rDocElementType, aRemoveEventNotifyContainer );
                rDocElementType.bModified = sal_False;
            }

            m_bModified = sal_False;

            // Unlock mutex before notify our listeners
            aGuard.unlock();

            // Notify our listeners
            for ( sal_uInt32 k = 0; k < aRemoveEventNotifyContainer.size(); k++ )
                implts_notifyContainerListener( aRemoveEventNotifyContainer[k], NotifyOp_Remove );
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

Sequence< Sequence< PropertyValue > > SAL_CALL UIConfigurationManager::getUIElementsInfo( sal_Int16 ElementType )
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

Reference< XIndexContainer > SAL_CALL UIConfigurationManager::createSettings() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    // Creates an empty item container which can be filled from outside
    return Reference< XIndexContainer >( static_cast< OWeakObject * >( new RootItemContainer()), UNO_QUERY );
}

sal_Bool SAL_CALL UIConfigurationManager::hasSettings( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    sal_Int16 nElementType = RetrieveTypeFromResourceURL( ResourceURL );

    if (( nElementType == ::com::sun::star::ui::UIElementType::UNKNOWN ) ||
        ( nElementType >= ::com::sun::star::ui::UIElementType::COUNT   ))
        throw IllegalArgumentException();
    else
    {
        UIElementData* pDataSettings = impl_findUIElementData( ResourceURL, nElementType, false );
        if ( pDataSettings && !pDataSettings->bDefault )
            return sal_True;
    }

    return sal_False;
}

Reference< XIndexAccess > SAL_CALL UIConfigurationManager::getSettings( const ::rtl::OUString& ResourceURL, sal_Bool bWriteable )
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
        if ( pDataSettings && !pDataSettings->bDefault )
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

void SAL_CALL UIConfigurationManager::replaceSettings( const ::rtl::OUString& ResourceURL, const Reference< ::com::sun::star::container::XIndexAccess >& aNewData )
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
        if ( pDataSettings && !pDataSettings->bDefault )
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
            UIElementType& rElementType = m_aUIElements[nElementType];
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
            throw NoSuchElementException();
    }
}

void SAL_CALL UIConfigurationManager::removeSettings( const ::rtl::OUString& ResourceURL )
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
                pDataSettings->bModified = true; // we have to remove this node from the user layer!
                pDataSettings->xSettings.clear();
                m_bModified = true; // user layer must be written

                // Modify type container
                UIElementType& rElementType = m_aUIElements[nElementType];
                rElementType.bModified = true;

                Reference< XUIConfigurationManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
                Reference< XInterface > xIfac( xThis, UNO_QUERY );

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
        else
            throw NoSuchElementException();
    }
}

void SAL_CALL UIConfigurationManager::insertSettings( const ::rtl::OUString& NewResourceURL, const Reference< XIndexAccess >& aNewData )
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
                pDataSettings->xSettings = Reference< XIndexAccess >( static_cast< OWeakObject * >( new ConstItemContainer( aNewData ) ), UNO_QUERY );
            else
                pDataSettings->xSettings = aNewData;

            m_bModified = true;

            UIElementType& rElementType = m_aUIElements[nElementType];
            rElementType.bModified = true;

            if ( bInsertData )
            {
                pDataSettings->aName        = RetrieveNameFromResourceURL( NewResourceURL ) + m_aXMLPostfix;
                pDataSettings->aResourceURL = NewResourceURL;

                UIElementDataHashMap& rElements = rElementType.aElementsHashMap;
                rElements.insert( UIElementDataHashMap::value_type( NewResourceURL, *pDataSettings ));
            }

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
    }
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getImageManager() throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_bDisposed )
        throw DisposedException();

    if ( !m_xImageManager.is() )
    {
        m_xImageManager = Reference< XComponent >( static_cast< cppu::OWeakObject *>( new ImageManager( m_xServiceManager )),
                                                         UNO_QUERY );
        Reference< XInitialization > xInit( m_xImageManager, UNO_QUERY );

        Sequence< Any > aPropSeq( 2 );
        PropertyValue aPropValue;
        aPropValue.Name  = rtl::OUString( "UserConfigStorage" );
        aPropValue.Value = makeAny( m_xDocConfigStorage );
        aPropSeq[0] = makeAny( aPropValue );
        aPropValue.Name  = rtl::OUString( "ModuleIdentifier" );
        aPropValue.Value = makeAny( m_aModuleIdentifier );
        aPropSeq[1] = makeAny( aPropValue );

        xInit->initialize( aPropSeq );
    }

    return Reference< XInterface >( m_xImageManager, UNO_QUERY );
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getShortCutManager() throw (::com::sun::star::uno::RuntimeException)
{
    // SAFE ->
    ResetableGuard aGuard( m_aLock );

    if (m_xAccConfig.is())
        return m_xAccConfig;

    Reference< XMultiServiceFactory > xSMGR         = m_xServiceManager;
    Reference< XStorage >             xDocumentRoot = m_xDocConfigStorage;

    aGuard.unlock();
    // <- SAFE

    Reference< XInterface >      xAccConfig = xSMGR->createInstance(SERVICENAME_DOCUMENTACCELERATORCONFIGURATION);
    Reference< XInitialization > xInit      (xAccConfig, UNO_QUERY_THROW);

    PropertyValue aProp;
    aProp.Name    = ::rtl::OUString("DocumentRoot");
    aProp.Value <<= xDocumentRoot;

    Sequence< Any > lArgs(1);
    lArgs[0] <<= aProp;

    xInit->initialize(lArgs);

    // SAFE ->
    aGuard.lock();
    m_xAccConfig = xAccConfig;
    aGuard.unlock();
    // <- SAFE

    return xAccConfig;
}

Reference< XInterface > SAL_CALL UIConfigurationManager::getEventsManager() throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XInterface >();
}

// XUIConfigurationStorage
void SAL_CALL UIConfigurationManager::setStorage( const Reference< XStorage >& Storage ) throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xDocConfigStorage.is() )
    {
        try
        {
            // Dispose old storage to be sure that it will be closed
            Reference< XComponent > xComponent( m_xDocConfigStorage, UNO_QUERY );
            if ( xComponent.is() )
                xComponent->dispose();
        }
        catch ( const Exception& )
        {
        }
    }

    // We store the new storage. Be careful it could be an empty reference!
    m_xDocConfigStorage = Storage;
    m_bReadOnly         = sal_True;

    Reference< XUIConfigurationStorage > xAccUpdate(m_xAccConfig, UNO_QUERY);
    if ( xAccUpdate.is() )
        xAccUpdate->setStorage( m_xDocConfigStorage );

    if ( m_xImageManager.is() )
    {
        ImageManager* pImageManager = (ImageManager*)m_xImageManager.get();
        if ( pImageManager )
            pImageManager->setStorage( m_xDocConfigStorage );
    }

    if ( m_xDocConfigStorage.is() )
    {
        Reference< XPropertySet > xPropSet( m_xDocConfigStorage, UNO_QUERY );
        if ( xPropSet.is() )
        {
            try
            {
                long nOpenMode = 0;
                Any a = xPropSet->getPropertyValue( rtl::OUString( "OpenMode" ));
                if ( a >>= nOpenMode )
                    m_bReadOnly = !( nOpenMode & ElementModes::WRITE );
            }
            catch ( const com::sun::star::beans::UnknownPropertyException& )
            {
            }
            catch ( const com::sun::star::lang::WrappedTargetException& )
            {
            }
        }
    }

    impl_Initialize();
}

sal_Bool SAL_CALL UIConfigurationManager::hasStorage() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    return ( m_xDocConfigStorage.is() );
}

// XUIConfigurationPersistence
void SAL_CALL UIConfigurationManager::reload() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xDocConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        ConfigEventNotifyContainer aRemoveNotifyContainer;
        ConfigEventNotifyContainer aReplaceNotifyContainer;
        for ( sal_Int16 i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
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

void SAL_CALL UIConfigurationManager::store() throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xDocConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
        {
            try
            {
                UIElementType& rElementType = m_aUIElements[i];
                Reference< XStorage > xStorage( rElementType.xStorage, UNO_QUERY );

                if ( rElementType.bModified && xStorage.is() )
                    impl_storeElementTypeData( xStorage, rElementType );
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
}

void SAL_CALL UIConfigurationManager::storeToStorage( const Reference< XStorage >& Storage ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xDocConfigStorage.is() && m_bModified && !m_bReadOnly )
    {
        // Try to access our module sub folder
        for ( int i = 1; i < ::com::sun::star::ui::UIElementType::COUNT; i++ )
        {
            try
            {
                Reference< XStorage > xElementTypeStorage( Storage->openStorageElement(
                                                           rtl::OUString::createFromAscii( UIELEMENTTYPENAMES[i] ), ElementModes::READWRITE ));
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
}

sal_Bool SAL_CALL UIConfigurationManager::isModified() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    return m_bModified;
}

sal_Bool SAL_CALL UIConfigurationManager::isReadOnly() throw (::com::sun::star::uno::RuntimeException)
{
    ResetableGuard aGuard( m_aLock );

    return m_bReadOnly;
}

void UIConfigurationManager::implts_notifyContainerListener( const ConfigurationEvent& aEvent, NotifyOp eOp )
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
