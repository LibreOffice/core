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

#include <memory>
#include <config_features.h>

#include <services/layoutmanager.hxx>
#include "helpers.hxx"

#include <framework/sfxhelperfunctions.hxx>
#include <uielement/menubarwrapper.hxx>
#include <framework/addonsoptions.hxx>
#include <toolkit/helper/convert.hxx>
#include <uielement/progressbarwrapper.hxx>
#include <uiconfiguration/globalsettings.hxx>
#include <uiconfiguration/windowstateproperties.hxx>
#include "toolbarlayoutmanager.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <com/sun/star/ui/theUIElementFactoryManager.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/lok.hxx>
#include <svtools/imgdef.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/status.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dockingarea.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/wall.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <comphelper/uno3.hxx>
#include <rtl/instance.hxx>
#include <unotools/cmdoptions.hxx>
#include <unotools/compatibilityviewoptions.hxx>

#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include <algorithm>

//      using namespace
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;

static const char STATUS_BAR_ALIAS[] = "private:resource/statusbar/statusbar";

namespace framework
{

IMPLEMENT_FORWARD_XTYPEPROVIDER2( LayoutManager, LayoutManager_Base, LayoutManager_PBase )
IMPLEMENT_FORWARD_XINTERFACE2( LayoutManager, LayoutManager_Base, LayoutManager_PBase )

LayoutManager::LayoutManager( const Reference< XComponentContext >& xContext ) : LayoutManager_Base()
        , ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >(m_aMutex)
        , LayoutManager_PBase( *static_cast< ::cppu::OBroadcastHelper* >(this) )
        , m_xContext( xContext )
        , m_xURLTransformer( URLTransformer::create(xContext) )
        , m_nLockCount( 0 )
        , m_bInplaceMenuSet( false )
        , m_bMenuVisible( true )
        , m_bVisible( true )
        , m_bParentWindowVisible( false )
        , m_bMustDoLayout( true )
#if HAVE_FEATURE_DESKTOP
        , m_bAutomaticToolbars( true )
#else
        , m_bAutomaticToolbars( false )
#endif
        , m_bHideCurrentUI( false )
        , m_bGlobalSettings( false )
        , m_bPreserveContentSize( false )
        , m_bMenuBarCloseButton( false )
        , m_xModuleManager( ModuleManager::create( xContext ))
        , m_xUIElementFactoryManager( ui::theUIElementFactoryManager::get(xContext) )
        , m_xPersistentWindowStateSupplier( ui::theWindowStateConfiguration::get( xContext ) )
        , m_aListenerContainer( m_aMutex )
{
    // Initialize statusbar member
    m_aStatusBarElement.m_aType = "statusbar";
    m_aStatusBarElement.m_aName = STATUS_BAR_ALIAS;

    if (!comphelper::LibreOfficeKit::isActive())
    {
        m_xToolbarManager = new ToolbarLayoutManager( xContext, Reference<XUIElementFactory>(m_xUIElementFactoryManager, UNO_QUERY_THROW), this );
    }

    m_aAsyncLayoutTimer.SetPriority( TaskPriority::HIGH_IDLE );
    m_aAsyncLayoutTimer.SetTimeout( 50 );
    m_aAsyncLayoutTimer.SetInvokeHandler( LINK( this, LayoutManager, AsyncLayoutHdl ) );
    m_aAsyncLayoutTimer.SetDebugName( "framework::LayoutManager m_aAsyncLayoutTimer" );

    registerProperty( LAYOUTMANAGER_PROPNAME_ASCII_AUTOMATICTOOLBARS, LAYOUTMANAGER_PROPHANDLE_AUTOMATICTOOLBARS, css::beans::PropertyAttribute::TRANSIENT, &m_bAutomaticToolbars, cppu::UnoType<decltype(m_bAutomaticToolbars)>::get() );
    registerProperty( LAYOUTMANAGER_PROPNAME_ASCII_HIDECURRENTUI, LAYOUTMANAGER_PROPHANDLE_HIDECURRENTUI, beans::PropertyAttribute::TRANSIENT, &m_bHideCurrentUI, cppu::UnoType<decltype(m_bHideCurrentUI)>::get() );
    registerProperty( LAYOUTMANAGER_PROPNAME_ASCII_LOCKCOUNT, LAYOUTMANAGER_PROPHANDLE_LOCKCOUNT, beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::READONLY, &m_nLockCount, cppu::UnoType<decltype(m_nLockCount)>::get()  );
    registerProperty( LAYOUTMANAGER_PROPNAME_MENUBARCLOSER, LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER, beans::PropertyAttribute::TRANSIENT, &m_bMenuBarCloseButton, cppu::UnoType<decltype(m_bMenuBarCloseButton)>::get() );
    registerPropertyNoMember( LAYOUTMANAGER_PROPNAME_ASCII_REFRESHVISIBILITY, LAYOUTMANAGER_PROPHANDLE_REFRESHVISIBILITY, beans::PropertyAttribute::TRANSIENT, cppu::UnoType<bool>::get(), css::uno::Any(false) );
    registerProperty( LAYOUTMANAGER_PROPNAME_ASCII_PRESERVE_CONTENT_SIZE, LAYOUTMANAGER_PROPHANDLE_PRESERVE_CONTENT_SIZE, beans::PropertyAttribute::TRANSIENT, &m_bPreserveContentSize, cppu::UnoType<decltype(m_bPreserveContentSize)>::get() );
}

LayoutManager::~LayoutManager()
{
    m_aAsyncLayoutTimer.Stop();
    setDockingAreaAcceptor(nullptr);
    m_pGlobalSettings.reset();
}

void LayoutManager::implts_createMenuBar(const OUString& rMenuBarName)
{
    SolarMutexClearableGuard aWriteLock;

    // Create a customized menu if compatibility mode is on
    SvtCompatibilityViewOptions aCompOptions;
    if( aCompOptions.HasMSOCompatibleFormsMenu() && m_aModuleIdentifier == "com.sun.star.text.TextDocument" )
    {
        implts_createMSCompatibleMenuBar(rMenuBarName);
    }

    // Create the default menubar otherwise
    if (!m_bInplaceMenuSet && !m_xMenuBar.is())
    {
        m_xMenuBar = implts_createElement( rMenuBarName );
        if ( m_xMenuBar.is() )
        {
            SolarMutexGuard aGuard;

            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
            {
                Reference< awt::XMenuBar > xMenuBar;

                Reference< XPropertySet > xPropSet( m_xMenuBar, UNO_QUERY );
                if ( xPropSet.is() )
                {
                    try
                    {
                        xPropSet->getPropertyValue("XMenuBar") >>= xMenuBar;
                    }
                    catch (const beans::UnknownPropertyException&)
                    {
                    }
                    catch (const lang::WrappedTargetException&)
                    {
                    }
                }

                if ( xMenuBar.is() )
                {
                    VCLXMenu* pAwtMenuBar = VCLXMenu::GetImplementation( xMenuBar );
                    if ( pAwtMenuBar )
                    {
                        MenuBar* pMenuBar = static_cast<MenuBar*>(pAwtMenuBar->GetMenu());
                        if ( pMenuBar )
                        {
                            pSysWindow->SetMenuBar(pMenuBar);
                            pMenuBar->SetDisplayable( m_bMenuVisible );
                            implts_updateMenuBarClose();
                        }
                    }
                }
            }
        }
    }
    aWriteLock.clear();
}

// Internal helper function
void LayoutManager::impl_clearUpMenuBar()
{
    implts_lock();

    // Clear up VCL menu bar to prepare shutdown
    if ( m_xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
        if ( pSysWindow )
        {
            MenuBar* pSetMenuBar = nullptr;
            if ( m_xInplaceMenuBar.is() )
                pSetMenuBar = static_cast<MenuBar *>(m_xInplaceMenuBar->GetMenuBar());
            else
            {
                Reference< awt::XMenuBar > xMenuBar;

                Reference< XPropertySet > xPropSet( m_xMenuBar, UNO_QUERY );
                if ( xPropSet.is() )
                {
                    try
                    {
                        xPropSet->getPropertyValue("XMenuBar") >>= xMenuBar;
                    }
                    catch (const beans::UnknownPropertyException&)
                    {
                    }
                    catch (const lang::WrappedTargetException&)
                    {
                    }
                }

                VCLXMenu* pAwtMenuBar = VCLXMenu::GetImplementation( xMenuBar );
                if ( pAwtMenuBar )
                    pSetMenuBar = static_cast<MenuBar*>(pAwtMenuBar->GetMenu());
            }

            MenuBar* pTopMenuBar = pSysWindow->GetMenuBar();
            if ( pSetMenuBar == pTopMenuBar )
                pSysWindow->SetMenuBar( nullptr );
        }
    }

    // reset inplace menubar manager
    VclPtr<Menu> pMenuBar;
    if (m_xInplaceMenuBar.is())
    {
        pMenuBar = m_xInplaceMenuBar->GetMenuBar();
        m_xInplaceMenuBar->dispose();
        m_xInplaceMenuBar.clear();
    }
    pMenuBar.disposeAndClear();
    m_bInplaceMenuSet = false;

    Reference< XComponent > xComp( m_xMenuBar, UNO_QUERY );
    if ( xComp.is() )
        xComp->dispose();
    m_xMenuBar.clear();
    implts_unlock();
}

void LayoutManager::implts_lock()
{
    SolarMutexGuard g;
    ++m_nLockCount;
}

bool LayoutManager::implts_unlock()
{
    SolarMutexGuard g;
    m_nLockCount = std::max( m_nLockCount-1, static_cast<sal_Int32>(0) );
    return ( m_nLockCount == 0 );
}

void LayoutManager::implts_reset( bool bAttached )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;
    Reference< XFrame > xFrame = m_xFrame;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< XUIConfiguration > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
    Reference< XUIConfiguration > xDocCfgMgr( m_xDocCfgMgr, UNO_QUERY );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    Reference< XComponentContext > xContext( m_xContext );
    Reference< XNameAccess > xPersistentWindowStateSupplier( m_xPersistentWindowStateSupplier );
    rtl::Reference<ToolbarLayoutManager> xToolbarManager( m_xToolbarManager );
    OUString aModuleIdentifier( m_aModuleIdentifier );
    bool bAutomaticToolbars( m_bAutomaticToolbars );
    aReadLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_lock();

    Reference< XModel > xModel;
    if ( xFrame.is() )
    {
        if ( bAttached )
        {
            OUString aOldModuleIdentifier( aModuleIdentifier );
            try
            {
                aModuleIdentifier = m_xModuleManager->identify( xFrame );
            }
            catch( const Exception& ) {}

            if ( !aModuleIdentifier.isEmpty() && aOldModuleIdentifier != aModuleIdentifier )
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier;
                if ( xContext.is() )
                    xModuleCfgSupplier = theModuleUIConfigurationManagerSupplier::get( xContext );

                if ( xModuleCfgMgr.is() )
                {
                    try
                    {
                        // Remove listener to old module ui configuration manager
                        xModuleCfgMgr->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                    }
                    catch (const Exception&)
                    {
                    }
                }

                try
                {
                    // Add listener to new module ui configuration manager
                    xModuleCfgMgr.set( xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier ), UNO_QUERY );
                    if ( xModuleCfgMgr.is() )
                        xModuleCfgMgr->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
                catch (const Exception&)
                {
                }

                try
                {
                    // Retrieve persistent window state reference for our new module
                    if ( xPersistentWindowStateSupplier.is() )
                        xPersistentWindowStateSupplier->getByName( aModuleIdentifier ) >>= xPersistentWindowState;
                }
                catch (const NoSuchElementException&)
                {
                }
                catch (const WrappedTargetException&)
                {
                }
            }

            xModel = impl_getModelFromFrame( xFrame );
            if ( xModel.is() )
            {
                Reference< XUIConfigurationManagerSupplier > xUIConfigurationManagerSupplier( xModel, UNO_QUERY );
                if ( xUIConfigurationManagerSupplier.is() )
                {
                    if ( xDocCfgMgr.is() )
                    {
                        try
                        {
                            // Remove listener to old ui configuration manager
                            xDocCfgMgr->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                        }
                        catch (const Exception&)
                        {
                        }
                    }

                    try
                    {
                        xDocCfgMgr.set( xUIConfigurationManagerSupplier->getUIConfigurationManager(), UNO_QUERY );
                        if ( xDocCfgMgr.is() )
                            xDocCfgMgr->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                    }
                    catch (const Exception&)
                    {
                    }
                }
            }
        }
        else
        {
            // Remove configuration listeners before we can release our references
            if ( xModuleCfgMgr.is() )
            {
                try
                {
                    xModuleCfgMgr->removeConfigurationListener(
                        Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
                catch (const Exception&)
                {
                }
            }

            if ( xDocCfgMgr.is() )
            {
                try
                {
                    xDocCfgMgr->removeConfigurationListener(
                        Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
                catch (const Exception&)
                {
                }
            }

            // Release references to our configuration managers as we currently don't have
            // an attached module.
            xModuleCfgMgr.clear();
            xDocCfgMgr.clear();
            xPersistentWindowState.clear();
            aModuleIdentifier.clear();
        }

        Reference< XUIConfigurationManager > xModCfgMgr( xModuleCfgMgr, UNO_QUERY );
        Reference< XUIConfigurationManager > xDokCfgMgr( xDocCfgMgr, UNO_QUERY );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        SolarMutexClearableGuard aWriteLock;
        m_aDockingArea = awt::Rectangle();
        m_aModuleIdentifier = aModuleIdentifier;
        m_xModuleCfgMgr = xModCfgMgr;
        m_xDocCfgMgr = xDokCfgMgr;
        m_xPersistentWindowState = xPersistentWindowState;
        m_aStatusBarElement.m_bStateRead = false; // reset state to read data again!
        aWriteLock.clear();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        // reset/notify toolbar layout manager
        if ( xToolbarManager.is() )
        {
            if ( bAttached )
            {
                xToolbarManager->attach( xFrame, xModCfgMgr, xDokCfgMgr, xPersistentWindowState );
                uno::Reference< awt::XWindowPeer > xParent( xContainerWindow, UNO_QUERY );
                xToolbarManager->setParentWindow( xParent );
                if ( bAutomaticToolbars )
                    xToolbarManager->createStaticToolbars();
            }
            else
            {
                xToolbarManager->reset();
                implts_destroyElements();
            }
        }
    }

    implts_unlock();
}

bool LayoutManager::implts_isEmbeddedLayoutManager() const
{
    SolarMutexClearableGuard aReadLock;
    Reference< XFrame > xFrame = m_xFrame;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    aReadLock.clear();

    Reference< awt::XWindow > xFrameContainerWindow = xFrame->getContainerWindow();
    return xFrameContainerWindow != xContainerWindow;
}

void LayoutManager::implts_destroyElements()
{
    SolarMutexResettableGuard aWriteLock;
    ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
    aWriteLock.clear();

    if ( pToolbarManager )
        pToolbarManager->destroyToolbars();

    implts_destroyStatusBar();

    aWriteLock.reset();
    impl_clearUpMenuBar();
    aWriteLock.clear();
}

void LayoutManager::implts_toggleFloatingUIElementsVisibility( bool bActive )
{
    SolarMutexClearableGuard aReadLock;
    ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
    aReadLock.clear();

    if ( pToolbarManager )
        pToolbarManager->setFloatingToolbarsVisibility( bActive );
}

uno::Reference< ui::XUIElement > LayoutManager::implts_findElement( const OUString& aName )
{
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
        return m_xMenuBar;
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
        return m_aStatusBarElement.m_xUIElement;
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
        return m_aProgressBarElement.m_xUIElement;

    return uno::Reference< ui::XUIElement >();
}

bool LayoutManager::implts_readWindowStateData( const OUString& aName, UIElement& rElementData )
{
    return readWindowStateData( aName, rElementData, m_xPersistentWindowState,
            m_pGlobalSettings, m_bGlobalSettings, m_xContext );
}

bool LayoutManager::readWindowStateData( const OUString& aName, UIElement& rElementData,
        const Reference< XNameAccess > &rPersistentWindowState,
        std::unique_ptr<GlobalSettings> &rGlobalSettings, bool &bInGlobalSettings,
        const Reference< XComponentContext > &rComponentContext )
{
    if ( rPersistentWindowState.is() )
    {
        bool bGetSettingsState( false );

        SolarMutexResettableGuard aWriteLock;
        bool bGlobalSettings( bInGlobalSettings );
        if ( rGlobalSettings == nullptr )
        {
            rGlobalSettings.reset( new GlobalSettings( rComponentContext ) );
            bGetSettingsState = true;
        }
        GlobalSettings* pGlobalSettings = rGlobalSettings.get();
        aWriteLock.clear();

        try
        {
            Sequence< PropertyValue > aWindowState;
            if ( rPersistentWindowState->hasByName( aName ) && (rPersistentWindowState->getByName( aName ) >>= aWindowState) )
            {
                bool bValue( false );
                for ( sal_Int32 n = 0; n < aWindowState.getLength(); n++ )
                {
                    if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_DOCKED )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_VISIBLE )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bVisible = bValue;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_DOCKINGAREA )
                    {
                        ui::DockingArea eDockingArea;
                        if ( aWindowState[n].Value >>= eDockingArea )
                            rElementData.m_aDockedData.m_nDockedArea = eDockingArea;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_DOCKPOS )
                    {
                        awt::Point aPoint;
                        if (aWindowState[n].Value >>= aPoint)
                        {
                            //tdf#90256 repair these broken Docking positions
                            if (aPoint.X < 0)
                                aPoint.X = SAL_MAX_INT32;
                            if (aPoint.Y < 0)
                                aPoint.Y = SAL_MAX_INT32;
                            rElementData.m_aDockedData.m_aPos = aPoint;
                        }
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_POS )
                    {
                        awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                            rElementData.m_aFloatingData.m_aPos = aPoint;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_SIZE )
                    {
                        awt::Size aSize;
                        if ( aWindowState[n].Value >>= aSize )
                            rElementData.m_aFloatingData.m_aSize = aSize;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_UINAME )
                        aWindowState[n].Value >>= rElementData.m_aUIName;
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_STYLE )
                    {
                        sal_Int32 nStyle = 0;
                        if ( aWindowState[n].Value >>= nStyle )
                            rElementData.m_nStyle = static_cast<ButtonType>( nStyle );
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_LOCKED )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_aDockedData.m_bLocked = bValue;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_CONTEXT )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bContextSensitive = bValue;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_NOCLOSE )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bNoClose = bValue;
                    }
                }
            }

            // oversteer values with global settings
            if (bGetSettingsState || bGlobalSettings)
            {
                if ( pGlobalSettings->HasToolbarStatesInfo())
                {
                    SolarMutexClearableGuard aWriteLock2;
                    bInGlobalSettings = true;
                    aWriteLock2.clear();

                    uno::Any aValue;
                    if ( pGlobalSettings->GetToolbarStateInfo(
                                                        GlobalSettings::STATEINFO_LOCKED,
                                                        aValue ))
                        aValue >>= rElementData.m_aDockedData.m_bLocked;
                    if ( pGlobalSettings->GetToolbarStateInfo(
                                                        GlobalSettings::STATEINFO_DOCKED,
                                                        aValue ))
                    {
                        bool bValue;
                        if ( aValue >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                }
            }

            const bool bDockingSupportCrippled = !StyleSettings::GetDockingFloatsSupported();
            if (bDockingSupportCrippled)
                rElementData.m_bFloating = false;

            return true;
        }
        catch (const NoSuchElementException&)
        {
        }
    }

    return false;
}

void LayoutManager::implts_writeWindowStateData( const OUString& aName, const UIElement& rElementData )
{
    SolarMutexResettableGuard aWriteLock;
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );

    aWriteLock.clear();

    bool bPersistent( false );
    Reference< XPropertySet > xPropSet( rElementData.m_xUIElement, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            // Check persistent flag of the user interface element
            xPropSet->getPropertyValue("Persistent") >>= bPersistent;
        }
        catch (const beans::UnknownPropertyException&)
        {
            // Non-configurable elements should at least store their dimension/position
            bPersistent = true;
        }
        catch (const lang::WrappedTargetException&)
        {
        }
    }

    if ( bPersistent && xPersistentWindowState.is() )
    {
        try
        {
            Sequence< PropertyValue > aWindowState( 8 );

            aWindowState[0].Name  = WINDOWSTATE_PROPERTY_DOCKED;
            aWindowState[0].Value <<= !rElementData.m_bFloating;
            aWindowState[1].Name  = WINDOWSTATE_PROPERTY_VISIBLE;
            aWindowState[1].Value <<= rElementData.m_bVisible;

            aWindowState[2].Name  = WINDOWSTATE_PROPERTY_DOCKINGAREA;
            aWindowState[2].Value <<= rElementData.m_aDockedData.m_nDockedArea;

            aWindowState[3].Name = WINDOWSTATE_PROPERTY_DOCKPOS;
            aWindowState[3].Value <<= rElementData.m_aDockedData.m_aPos;

            aWindowState[4].Name = WINDOWSTATE_PROPERTY_POS;
            aWindowState[4].Value <<= rElementData.m_aFloatingData.m_aPos;

            aWindowState[5].Name  = WINDOWSTATE_PROPERTY_SIZE;
            aWindowState[5].Value <<= rElementData.m_aFloatingData.m_aSize;
            aWindowState[6].Name  = WINDOWSTATE_PROPERTY_UINAME;
            aWindowState[6].Value <<= rElementData.m_aUIName;
            aWindowState[7].Name  = WINDOWSTATE_PROPERTY_LOCKED;
            aWindowState[7].Value <<= rElementData.m_aDockedData.m_bLocked;

            if ( xPersistentWindowState->hasByName( aName ))
            {
                Reference< XNameReplace > xReplace( xPersistentWindowState, uno::UNO_QUERY );
                xReplace->replaceByName( aName, makeAny( aWindowState ));
            }
            else
            {
                Reference< XNameContainer > xInsert( xPersistentWindowState, uno::UNO_QUERY );
                xInsert->insertByName( aName, makeAny( aWindowState ));
            }
        }
        catch (const Exception&)
        {
        }
    }

    // Reset flag
    aWriteLock.reset();
    aWriteLock.clear();
}

::Size LayoutManager::implts_getContainerWindowOutputSize()
{
    ::Size  aContainerWinSize;
    vcl::Window* pContainerWindow( nullptr );

    // Retrieve output size from container Window
    SolarMutexGuard aGuard;
    pContainerWindow  = VCLUnoHelper::GetWindow( m_xContainerWindow ).get();
    if ( pContainerWindow )
        aContainerWinSize = pContainerWindow->GetOutputSizePixel();

    return aContainerWinSize;
}

Reference< XUIElement > LayoutManager::implts_createElement( const OUString& aName )
{
    Reference< ui::XUIElement > xUIElement;

    SolarMutexGuard g;
    Sequence< PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = "Frame";
    aPropSeq[0].Value <<= m_xFrame;
    aPropSeq[1].Name = "Persistent";
    aPropSeq[1].Value <<= true;

    try
    {
        xUIElement = m_xUIElementFactoryManager->createUIElement( aName, aPropSeq );
    }
    catch (const NoSuchElementException&)
    {
    }
    catch (const IllegalArgumentException&)
    {
    }

    return xUIElement;
}

void LayoutManager::implts_setVisibleState( bool bShow )
{
    SolarMutexClearableGuard aWriteLock;
    m_aStatusBarElement.m_bMasterHide = !bShow;
    aWriteLock.clear();

    implts_updateUIElementsVisibleState( bShow );
}

void LayoutManager::implts_updateUIElementsVisibleState( bool bSetVisible )
{
    // notify listeners
    uno::Any a;
    if ( bSetVisible )
        implts_notifyListeners( frame::LayoutManagerEvents::VISIBLE, a );
    else
        implts_notifyListeners( frame::LayoutManagerEvents::INVISIBLE, a );

    SolarMutexResettableGuard aWriteLock;
    Reference< XUIElement >   xMenuBar( m_xMenuBar, UNO_QUERY );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    rtl::Reference< MenuBarManager > xInplaceMenuBar( m_xInplaceMenuBar );
    aWriteLock.clear();

    if (( xMenuBar.is() || xInplaceMenuBar.is() ) && xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        MenuBar* pMenuBar( nullptr );
        if ( xInplaceMenuBar.is() )
            pMenuBar = static_cast<MenuBar *>(xInplaceMenuBar->GetMenuBar());
        else
        {
            MenuBarWrapper* pMenuBarWrapper = static_cast< MenuBarWrapper* >(xMenuBar.get());
            pMenuBar = static_cast<MenuBar *>(pMenuBarWrapper->GetMenuBarManager()->GetMenuBar());
        }

        SystemWindow* pSysWindow = getTopSystemWindow( xContainerWindow );
        if ( pSysWindow )
        {
            if ( bSetVisible )
            {
                pSysWindow->SetMenuBar(pMenuBar);
            }
            else
                pSysWindow->SetMenuBar( nullptr );
        }
    }

    bool bMustDoLayout;
    // Hide/show the statusbar according to bSetVisible
    if ( bSetVisible )
        bMustDoLayout = !implts_showStatusBar();
    else
        bMustDoLayout = !implts_hideStatusBar();

    aWriteLock.reset();
    ToolbarLayoutManager* pToolbarManager( m_xToolbarManager.get() );
    aWriteLock.clear();

    if ( pToolbarManager )
    {
        pToolbarManager->setVisible( bSetVisible );
        bMustDoLayout = pToolbarManager->isLayoutDirty();
    }

    if ( bMustDoLayout )
        implts_doLayout_notify( false );
}

void LayoutManager::implts_setCurrentUIVisibility( bool bShow )
{
    SolarMutexClearableGuard aWriteLock;
    if ( !bShow && m_aStatusBarElement.m_bVisible && m_aStatusBarElement.m_xUIElement.is() )
        m_aStatusBarElement.m_bMasterHide = true;
    else if ( bShow && m_aStatusBarElement.m_bVisible )
        m_aStatusBarElement.m_bMasterHide = false;
    aWriteLock.clear();

    implts_updateUIElementsVisibleState( bShow );
}

void LayoutManager::implts_destroyStatusBar()
{
    Reference< XComponent > xCompStatusBar;

    SolarMutexClearableGuard aWriteLock;
    m_aStatusBarElement.m_aName.clear();
    xCompStatusBar.set( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    m_aStatusBarElement.m_xUIElement.clear();
    aWriteLock.clear();

    if ( xCompStatusBar.is() )
        xCompStatusBar->dispose();

    implts_destroyProgressBar();
}

void LayoutManager::implts_createStatusBar( const OUString& aStatusBarName )
{
    SolarMutexClearableGuard aWriteLock;
    if ( !m_aStatusBarElement.m_xUIElement.is() )
    {
        implts_readStatusBarState( aStatusBarName );
        m_aStatusBarElement.m_aName      = aStatusBarName;
        m_aStatusBarElement.m_xUIElement = implts_createElement( aStatusBarName );
    }
    aWriteLock.clear();

    implts_createProgressBar();
}

void LayoutManager::implts_readStatusBarState( const OUString& rStatusBarName )
{
    SolarMutexGuard g;
    if ( !m_aStatusBarElement.m_bStateRead )
    {
        // Read persistent data for status bar if not yet read!
        if ( implts_readWindowStateData( rStatusBarName, m_aStatusBarElement ))
            m_aStatusBarElement.m_bStateRead = true;
    }
}

void LayoutManager::implts_createProgressBar()
{
    Reference< XUIElement > xStatusBar;
    Reference< XUIElement > xProgressBar;
    Reference< XUIElement > xProgressBarBackup;
    Reference< awt::XWindow > xContainerWindow;

    SolarMutexResettableGuard aWriteLock;
    xStatusBar.set( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar.set( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    xProgressBarBackup = m_xProgressBarBackup;
    m_xProgressBarBackup.clear();
    xContainerWindow = m_xContainerWindow;
    aWriteLock.clear();

    bool                bRecycled = xProgressBarBackup.is();
    ProgressBarWrapper* pWrapper  = nullptr;
    if ( bRecycled )
        pWrapper = static_cast<ProgressBarWrapper*>(xProgressBarBackup.get());
    else if ( xProgressBar.is() )
        pWrapper = static_cast<ProgressBarWrapper*>(xProgressBar.get());
    else
        pWrapper = new ProgressBarWrapper();

    if ( xStatusBar.is() )
    {
        Reference< awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );
        pWrapper->setStatusBar( xWindow );
    }
    else
    {
        Reference< awt::XWindow > xStatusBarWindow = pWrapper->getStatusBar();

        SolarMutexGuard aGuard;
        VclPtr<vcl::Window> pStatusBarWnd = VCLUnoHelper::GetWindow( xStatusBarWindow );
        if ( !pStatusBarWnd )
        {
            VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            if ( pWindow )
            {
                VclPtrInstance<StatusBar> pStatusBar( pWindow, WinBits( WB_LEFT | WB_3DLOOK ) );
                Reference< awt::XWindow > xStatusBarWindow2( VCLUnoHelper::GetInterface( pStatusBar ));
                pWrapper->setStatusBar( xStatusBarWindow2, true );
            }
        }
    }

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    aWriteLock.reset();
    m_aProgressBarElement.m_xUIElement.set( static_cast< cppu::OWeakObject* >( pWrapper ), UNO_QUERY );
    aWriteLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( bRecycled )
        implts_showProgressBar();
}

void LayoutManager::implts_backupProgressBarWrapper()
{
    SolarMutexGuard g;

    if (m_xProgressBarBackup.is())
        return;

    // safe a backup copy of the current progress!
    // This copy will be used automatically inside createProgressBar() which is called
    // implicitly from implts_doLayout() .-)
    m_xProgressBarBackup = m_aProgressBarElement.m_xUIElement;

    // remove the relation between this old progress bar and our old status bar.
    // Otherwise we work on disposed items ...
    // The internal used ProgressBarWrapper can handle a NULL reference.
    if ( m_xProgressBarBackup.is() )
    {
        ProgressBarWrapper* pWrapper = static_cast<ProgressBarWrapper*>(m_xProgressBarBackup.get());
        if ( pWrapper )
            pWrapper->setStatusBar( Reference< awt::XWindow >() );
    }

    // prevent us from dispose() the m_aProgressBarElement.m_xUIElement inside implts_reset()
    m_aProgressBarElement.m_xUIElement.clear();
}

void LayoutManager::implts_destroyProgressBar()
{
    // don't remove the progressbar in general
    // We must reuse it if a new status bar is created later.
    // Of course there exists one backup only.
    // And further this backup will be released inside our dtor.
    implts_backupProgressBarWrapper();
}

void LayoutManager::implts_setStatusBarPosSize( const ::Point& rPos, const ::Size& rSize )
{
    Reference< XUIElement > xStatusBar;
    Reference< XUIElement > xProgressBar;
    Reference< awt::XWindow > xContainerWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;
    xStatusBar.set( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar.set( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    xContainerWindow = m_xContainerWindow;

    Reference< awt::XWindow > xWindow;
    if ( xStatusBar.is() )
        xWindow.set( xStatusBar->getRealInterface(), UNO_QUERY );
    else if ( xProgressBar.is() )
    {
        ProgressBarWrapper* pWrapper = static_cast<ProgressBarWrapper*>(xProgressBar.get());
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
    }
    aReadLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xWindow.is() )
    {
        SolarMutexGuard aGuard;
        VclPtr<vcl::Window> pParentWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pParentWindow && ( pWindow && pWindow->GetType() == WindowType::STATUSBAR ))
        {
            vcl::Window* pOldParentWindow = pWindow->GetParent();
            if ( pParentWindow != pOldParentWindow )
                pWindow->SetParent( pParentWindow );
            static_cast<StatusBar *>(pWindow.get())->SetPosSizePixel( rPos, rSize );
        }
    }
}

bool LayoutManager::implts_showProgressBar()
{
    Reference< XUIElement > xStatusBar;
    Reference< XUIElement > xProgressBar;
    Reference< awt::XWindow > xWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;
    xStatusBar.set( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar.set( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    bool bVisible( m_bVisible );

    m_aProgressBarElement.m_bVisible = true;
    if ( bVisible )
    {
        if ( xStatusBar.is() && !m_aStatusBarElement.m_bMasterHide )
        {
            xWindow.set( xStatusBar->getRealInterface(), UNO_QUERY );
        }
        else if ( xProgressBar.is() )
        {
            ProgressBarWrapper* pWrapper = static_cast<ProgressBarWrapper*>(xProgressBar.get());
            if ( pWrapper )
                xWindow = pWrapper->getStatusBar();
        }
    }
    aWriteLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    SolarMutexGuard aGuard;
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( pWindow )
    {
        if ( !pWindow->IsVisible() )
        {
            implts_setOffset( pWindow->GetSizePixel().Height() );
            pWindow->Show();
            implts_doLayout_notify( false );
        }
        return true;
    }

    return false;
}

bool LayoutManager::implts_hideProgressBar()
{
    Reference< XUIElement > xProgressBar;
    Reference< awt::XWindow > xWindow;
    bool bHideStatusBar( false );

    SolarMutexGuard g;
    xProgressBar.set( m_aProgressBarElement.m_xUIElement, UNO_QUERY );

    bool bInternalStatusBar( false );
    if ( xProgressBar.is() )
    {
        Reference< awt::XWindow > xStatusBar;
        ProgressBarWrapper* pWrapper = static_cast<ProgressBarWrapper*>(xProgressBar.get());
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
        Reference< ui::XUIElement > xStatusBarElement = m_aStatusBarElement.m_xUIElement;
        if ( xStatusBarElement.is() )
            xStatusBar.set( xStatusBarElement->getRealInterface(), UNO_QUERY );
        bInternalStatusBar = xStatusBar != xWindow;
    }
    m_aProgressBarElement.m_bVisible = false;
    implts_readStatusBarState( STATUS_BAR_ALIAS );
    bHideStatusBar = !m_aStatusBarElement.m_bVisible;

    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( pWindow && pWindow->IsVisible() && ( bHideStatusBar || bInternalStatusBar ))
    {
        implts_setOffset( 0 );
        pWindow->Hide();
        implts_doLayout_notify( false );
        return true;
    }

    return false;
}

bool LayoutManager::implts_showStatusBar( bool bStoreState )
{
    SolarMutexClearableGuard aWriteLock;
    Reference< ui::XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    if ( bStoreState )
        m_aStatusBarElement.m_bVisible = true;
    aWriteLock.clear();

    if ( xStatusBar.is() )
    {
        Reference< awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );

        SolarMutexGuard aGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && !pWindow->IsVisible() )
        {
            implts_setOffset( pWindow->GetSizePixel().Height() );
            pWindow->Show();
            implts_doLayout_notify( false );
            return true;
        }
    }

    return false;
}

bool LayoutManager::implts_hideStatusBar( bool bStoreState )
{
    SolarMutexClearableGuard aWriteLock;
    Reference< ui::XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    if ( bStoreState )
        m_aStatusBarElement.m_bVisible = false;
    aWriteLock.clear();

    if ( xStatusBar.is() )
    {
        Reference< awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );

        SolarMutexGuard aGuard;
        VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->IsVisible() )
        {
            implts_setOffset( 0 );
            pWindow->Hide();
            implts_doLayout_notify( false );
            return true;
        }
    }

    return false;
}

void LayoutManager::implts_setOffset( const sal_Int32 nBottomOffset )
{
    ::tools::Rectangle aOffsetRect;
    setZeroRectangle( aOffsetRect );
    aOffsetRect.setHeight( nBottomOffset );

    if ( m_xToolbarManager.is() )
        m_xToolbarManager->setDockingAreaOffsets( aOffsetRect );
}

void LayoutManager::implts_setInplaceMenuBar( const Reference< XIndexAccess >& xMergedMenuBar )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;

    if ( !m_bInplaceMenuSet )
    {
        SolarMutexGuard aGuard;

        // Reset old inplace menubar!
        VclPtr<Menu> pOldMenuBar;
        if (m_xInplaceMenuBar.is())
        {
            pOldMenuBar = m_xInplaceMenuBar->GetMenuBar();
            m_xInplaceMenuBar->dispose();
            m_xInplaceMenuBar.clear();
        }
        pOldMenuBar.disposeAndClear();

        m_bInplaceMenuSet = false;

        if ( m_xFrame.is() && m_xContainerWindow.is() )
        {
            Reference< XDispatchProvider > xDispatchProvider;

            VclPtr<MenuBar> pMenuBar = VclPtr<MenuBar>::Create();
            m_xInplaceMenuBar = new MenuBarManager( m_xContext, m_xFrame, m_xURLTransformer, xDispatchProvider, OUString(), pMenuBar, true );
            m_xInplaceMenuBar->SetItemContainer( xMergedMenuBar );

            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
                pSysWindow->SetMenuBar(pMenuBar);

            m_bInplaceMenuSet = true;
        }

        aWriteLock.clear();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_updateMenuBarClose();
    }
}

void LayoutManager::implts_resetInplaceMenuBar()
{
    SolarMutexGuard g;
    m_bInplaceMenuSet = false;

    if ( m_xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;
        MenuBarWrapper* pMenuBarWrapper = static_cast< MenuBarWrapper* >(m_xMenuBar.get());
        SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
        if ( pSysWindow )
        {
            if ( pMenuBarWrapper )
                pSysWindow->SetMenuBar(static_cast<MenuBar *>(pMenuBarWrapper->GetMenuBarManager()->GetMenuBar()));
            else
                pSysWindow->SetMenuBar(nullptr);
        }
    }

    // Remove inplace menu bar
    VclPtr<Menu> pMenuBar;
    if (m_xInplaceMenuBar.is())
    {
        pMenuBar = m_xInplaceMenuBar->GetMenuBar();
        m_xInplaceMenuBar->dispose();
        m_xInplaceMenuBar.clear();
    }
    pMenuBar.disposeAndClear();
}

void SAL_CALL LayoutManager::attachFrame( const Reference< XFrame >& xFrame )
{
    SolarMutexGuard g;
    m_xFrame = xFrame;
}

void SAL_CALL LayoutManager::reset()
{
    implts_reset( true );
}

// XMenuBarMergingAcceptor

sal_Bool SAL_CALL LayoutManager::setMergedMenuBar(
    const Reference< XIndexAccess >& xMergedMenuBar )
{
    implts_setInplaceMenuBar( xMergedMenuBar );

    uno::Any a;
    implts_notifyListeners( frame::LayoutManagerEvents::MERGEDMENUBAR, a );
    return true;
}

void SAL_CALL LayoutManager::removeMergedMenuBar()
{
    implts_resetInplaceMenuBar();
}

awt::Rectangle SAL_CALL LayoutManager::getCurrentDockingArea()
{
    SolarMutexGuard g;
    return m_aDockingArea;
}

Reference< XDockingAreaAcceptor > SAL_CALL LayoutManager::getDockingAreaAcceptor()
{
    SolarMutexGuard g;
    return m_xDockingAreaAcceptor;
}

void SAL_CALL LayoutManager::setDockingAreaAcceptor( const Reference< ui::XDockingAreaAcceptor >& xDockingAreaAcceptor )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;

    if (( m_xDockingAreaAcceptor == xDockingAreaAcceptor ) || !m_xFrame.is() )
        return;

    // IMPORTANT: Be sure to stop layout timer if don't have a docking area acceptor!
    if ( !xDockingAreaAcceptor.is() )
        m_aAsyncLayoutTimer.Stop();

    bool bAutomaticToolbars( m_bAutomaticToolbars );

    ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();

    if ( !xDockingAreaAcceptor.is() )
        m_aAsyncLayoutTimer.Stop();

    // Remove listener from old docking area acceptor
    if ( m_xDockingAreaAcceptor.is() )
    {
        Reference< awt::XWindow > xWindow( m_xDockingAreaAcceptor->getContainerWindow() );
        if ( xWindow.is() && ( m_xFrame->getContainerWindow() != m_xContainerWindow || !xDockingAreaAcceptor.is() ) )
            xWindow->removeWindowListener( Reference< awt::XWindowListener >( static_cast< OWeakObject * >( this ), UNO_QUERY ));

        m_aDockingArea = awt::Rectangle();
        if ( pToolbarManager )
            pToolbarManager->resetDockingArea();

        VclPtr<vcl::Window> pContainerWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pContainerWindow )
            pContainerWindow->RemoveChildEventListener( LINK( this, LayoutManager, WindowEventListener ) );
    }

    m_xDockingAreaAcceptor = xDockingAreaAcceptor;
    if ( m_xDockingAreaAcceptor.is() )
    {
        m_aDockingArea     = awt::Rectangle();
        m_xContainerWindow = m_xDockingAreaAcceptor->getContainerWindow();
        m_xContainerTopWindow.set( m_xContainerWindow, UNO_QUERY );
        m_xContainerWindow->addWindowListener( Reference< awt::XWindowListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));

        // we always must keep a connection to the window of our frame for resize events
        if ( m_xContainerWindow != m_xFrame->getContainerWindow() )
            m_xFrame->getContainerWindow()->addWindowListener( Reference< awt::XWindowListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));

        // #i37884# set initial visibility state - in the plugin case the container window is already shown
        // and we get no notification anymore
        {
            SolarMutexGuard aGuard;
            VclPtr<vcl::Window> pContainerWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
            if( pContainerWindow )
                m_bParentWindowVisible = pContainerWindow->IsVisible();
        }
    }

    aWriteLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xDockingAreaAcceptor.is() )
    {
        SolarMutexGuard aGuard;

        // Add layout manager as listener to get notifications about toolbar button activities
        VclPtr<vcl::Window> pContainerWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        if ( pContainerWindow )
            pContainerWindow->AddChildEventListener( LINK( this, LayoutManager, WindowEventListener ) );

        // We have now a new container window, reparent all child windows!
        implts_reparentChildWindows();
    }
    else
        implts_destroyElements(); // remove all elements

    if ( pToolbarManager && xDockingAreaAcceptor.is() )
    {
        if ( bAutomaticToolbars )
        {
            lock();
            pToolbarManager->createStaticToolbars();
            unlock();
        }
        implts_doLayout( true, false );
    }
}

void LayoutManager::implts_reparentChildWindows()
{
    SolarMutexResettableGuard aWriteLock;
    UIElement aStatusBarElement = m_aStatusBarElement;
    uno::Reference< awt::XWindow > xContainerWindow  = m_xContainerWindow;
    aWriteLock.clear();

    uno::Reference< awt::XWindow > xStatusBarWindow;
    if ( aStatusBarElement.m_xUIElement.is() )
    {
        try
        {
            xStatusBarWindow.set( aStatusBarElement.m_xUIElement->getRealInterface(), UNO_QUERY );
        }
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const Exception&)
        {
        }
    }

    if ( xStatusBarWindow.is() )
    {
        SolarMutexGuard aGuard;
        VclPtr<vcl::Window> pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        VclPtr<vcl::Window> pWindow          = VCLUnoHelper::GetWindow( xStatusBarWindow );
        if ( pWindow && pContainerWindow )
            pWindow->SetParent( pContainerWindow );
    }

    implts_resetMenuBar();

    aWriteLock.reset();
    ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
    if ( pToolbarManager )
        pToolbarManager->setParentWindow( uno::Reference< awt::XWindowPeer >( xContainerWindow, uno::UNO_QUERY ));
    aWriteLock.clear();
}

uno::Reference< ui::XUIElement > LayoutManager::implts_createDockingWindow( const OUString& aElementName )
{
    Reference< XUIElement > xUIElement = implts_createElement( aElementName );
    return xUIElement;
}

IMPL_LINK( LayoutManager, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    vcl::Window* pWindow = rEvent.GetWindow();
    if ( pWindow && pWindow->GetType() == WindowType::TOOLBOX )
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager( m_xToolbarManager.get() );
        aReadLock.clear();

        if ( pToolbarManager )
            pToolbarManager->childWindowEvent( &rEvent );
    }
}

void SAL_CALL LayoutManager::createElement( const OUString& aName )
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::createElement" );

    SolarMutexClearableGuard aReadLock;
    Reference< XFrame > xFrame = m_xFrame;
    aReadLock.clear();

    if ( !xFrame.is() )
        return;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;

    bool bMustBeLayouted( false );
    bool bNotify( false );

    bool bPreviewFrame;
    if (m_xToolbarManager.is())
        // Assumes that we created the ToolbarLayoutManager with our frame, if
        // not then we're somewhat fouled up ...
        bPreviewFrame = m_xToolbarManager->isPreviewFrame();
    else
    {
        Reference< XModel >  xModel( impl_getModelFromFrame( xFrame ) );
        bPreviewFrame = implts_isPreviewModel( xModel );
    }

    if ( m_xContainerWindow.is() && !bPreviewFrame ) // no UI elements on preview frames
    {
        OUString aElementType;
        OUString aElementName;

        parseResourceURL( aName, aElementType, aElementName );

        if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ) && m_xToolbarManager.is() )
        {
            bNotify         = m_xToolbarManager->createToolbar( aName );
            bMustBeLayouted = m_xToolbarManager->isLayoutDirty();
        }
        else if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
                  aElementName.equalsIgnoreAsciiCase("menubar") &&
                  implts_isFrameOrWindowTop(xFrame) )
        {
            implts_createMenuBar( aName );
            if (m_bMenuVisible)
                bNotify = true;

            aWriteLock.clear();
        }
        else if ( aElementType.equalsIgnoreAsciiCase("statusbar") &&
                  ( implts_isFrameOrWindowTop(xFrame) || implts_isEmbeddedLayoutManager() ))
        {
            implts_createStatusBar( aName );
            bNotify = true;
        }
        else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
                  aElementName.equalsIgnoreAsciiCase("progressbar") &&
                  implts_isFrameOrWindowTop(xFrame) )
        {
            implts_createProgressBar();
            bNotify = true;
        }
        else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
        {
            // Add layout manager as listener for docking and other window events
            uno::Reference< uno::XInterface > xThis( static_cast< OWeakObject* >(this), uno::UNO_QUERY );
            uno::Reference< ui::XUIElement > xUIElement( implts_createDockingWindow( aName ));

            if ( xUIElement.is() )
            {
                impl_addWindowListeners( xThis, xUIElement );
            }

            // The docking window is created by a factory method located in the sfx2 library.
//            CreateDockingWindow( xFrame, aElementName );
        }
    }

    if ( bMustBeLayouted )
        implts_doLayout_notify( true );

    if ( bNotify )
    {
        // UI element is invisible - provide information to listeners
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( aName ) );
    }
}

void SAL_CALL LayoutManager::destroyElement( const OUString& aName )
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::destroyElement" );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;

    bool            bMustBeLayouted( false );
    bool            bNotify( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );

    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        if ( !m_bInplaceMenuSet )
        {
            impl_clearUpMenuBar();
            m_xMenuBar.clear();
            bNotify = true;
        }
    }
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
    {
        aWriteLock.clear();
        implts_destroyStatusBar();
        bMustBeLayouted = true;
        bNotify         = true;
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        aWriteLock.clear();
        implts_createProgressBar();
        bMustBeLayouted = true;
        bNotify = true;
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ) && m_xToolbarManager.is() )
    {
        aWriteLock.clear();
        bNotify         = m_xToolbarManager->destroyToolbar( aName );
        bMustBeLayouted = m_xToolbarManager->isLayoutDirty();
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        uno::Reference< XComponentContext > xContext( m_xContext );
        aWriteLock.clear();

        impl_setDockingWindowVisibility( xContext, xFrame, aElementName, false );
        bMustBeLayouted = false;
        bNotify         = false;
    }
    aWriteLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( bMustBeLayouted )
        doLayout();

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_INVISIBLE, uno::makeAny( aName ) );
}

sal_Bool SAL_CALL LayoutManager::requestElement( const OUString& rResourceURL )
{
    bool            bResult( false );
    bool            bNotify( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( rResourceURL, aElementType, aElementName );

    SolarMutexClearableGuard aWriteLock;

    OString aResName = OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk", "framework (cd100003) Element " << aResName << " requested." );

    if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
          aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
        ( m_aStatusBarElement.m_aName == rResourceURL ))
    {
        implts_readStatusBarState( rResourceURL );
        if ( m_aStatusBarElement.m_bVisible && !m_aStatusBarElement.m_bMasterHide )
        {
            aWriteLock.clear();
            createElement( rResourceURL );

            // There are some situation where we are not able to create an element.
            // Therefore we have to check the reference before further action.
            // See #i70019#
            uno::Reference< ui::XUIElement > xUIElement( m_aStatusBarElement.m_xUIElement );
            if ( xUIElement.is() )
            {
                // we need VCL here to pass special flags to Show()
                SolarMutexGuard aGuard;
                Reference< awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow )
                {
                    pWindow->Show( true, ShowFlags::NoFocusChange | ShowFlags::NoActivate );
                    bResult   = true;
                    bNotify   = true;
                }
            }
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        aWriteLock.clear();
        implts_showProgressBar();
        bResult   = true;
        bNotify   = true;
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ) && m_bVisible )
    {
        bool bComponentAttached( !m_aModuleIdentifier.isEmpty() );
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aWriteLock.clear();

        if ( pToolbarManager && bComponentAttached )
        {
                bNotify   = pToolbarManager->requestToolbar( rResourceURL );
            }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        aWriteLock.clear();

        CreateDockingWindow( xFrame, aElementName );
    }

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( rResourceURL ) );

    return bResult;
}

Reference< XUIElement > SAL_CALL LayoutManager::getElement( const OUString& aName )
{
    Reference< XUIElement > xUIElement = implts_findElement( aName );
    if ( !xUIElement.is() )
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager*             pToolbarManager( m_xToolbarManager.get() );
        aReadLock.clear();

        if ( pToolbarManager )
            xUIElement = pToolbarManager->getToolbar( aName );
    }

    return xUIElement;
}

Sequence< Reference< ui::XUIElement > > SAL_CALL LayoutManager::getElements()
{
    SolarMutexClearableGuard aReadLock;
    uno::Reference< ui::XUIElement >  xMenuBar( m_xMenuBar );
    uno::Reference< ui::XUIElement >  xStatusBar( m_aStatusBarElement.m_xUIElement );
    ToolbarLayoutManager*             pToolbarManager( m_xToolbarManager.get() );
    aReadLock.clear();

    Sequence< Reference< ui::XUIElement > > aSeq;
    if ( pToolbarManager )
        aSeq = pToolbarManager->getToolbars();

    sal_Int32 nSize = aSeq.getLength();
    sal_Int32 nMenuBarIndex(-1);
    sal_Int32 nStatusBarIndex(-1);
    if ( xMenuBar.is() )
    {
        nMenuBarIndex = nSize;
        ++nSize;
    }
    if ( xStatusBar.is() )
    {
        nStatusBarIndex = nSize;
        ++nSize;
    }

    aSeq.realloc(nSize);
    if ( nMenuBarIndex >= 0 )
        aSeq[nMenuBarIndex] = xMenuBar;
    if ( nStatusBarIndex >= 0 )
        aSeq[nStatusBarIndex] = xStatusBar;

    return aSeq;
}

sal_Bool SAL_CALL LayoutManager::showElement( const OUString& aName )
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::showElement" );

    bool            bResult( false );
    bool            bNotify( false );
    bool            bMustLayout( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );

    OString aResName = OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk", "framework (cd100003) Element " << aResName );

    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        SolarMutexClearableGuard aWriteLock;
        m_bMenuVisible = true;
        aWriteLock.clear();

        bResult = implts_resetMenuBar();
        bNotify = bResult;
    }
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
    {
        SolarMutexClearableGuard aWriteLock;
        if ( m_aStatusBarElement.m_xUIElement.is() && !m_aStatusBarElement.m_bMasterHide &&
             implts_showStatusBar( true ))
        {
            aWriteLock.clear();

            implts_writeWindowStateData( STATUS_BAR_ALIAS, m_aStatusBarElement );
            bMustLayout = true;
            bResult     = true;
            bNotify     = true;
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        bNotify = bResult = implts_showProgressBar();
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            bNotify     = pToolbarManager->showToolbar( aName );
            bMustLayout = pToolbarManager->isLayoutDirty();
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        SolarMutexClearableGuard aReadGuard;
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        uno::Reference< XComponentContext > xContext( m_xContext );
        aReadGuard.clear();

        impl_setDockingWindowVisibility( xContext, xFrame, aElementName, true );
    }

    if ( bMustLayout )
        doLayout();

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( aName ) );

    return bResult;
}

sal_Bool SAL_CALL LayoutManager::hideElement( const OUString& aName )
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::hideElement" );

    bool            bNotify( false );
    bool            bMustLayout( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    OString aResName = OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk", "framework (cd100003) Element " << aResName );

    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        SolarMutexGuard g;

        if ( m_xContainerWindow.is() )
        {
            m_bMenuVisible = false;

            SolarMutexGuard aGuard;
            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
            {
                MenuBar* pMenuBar = pSysWindow->GetMenuBar();
                if ( pMenuBar )
                {
                    pMenuBar->SetDisplayable( false );
                    bNotify = true;
                }
            }
        }
    }
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
    {
        SolarMutexGuard g;
        if ( m_aStatusBarElement.m_xUIElement.is() && !m_aStatusBarElement.m_bMasterHide &&
             implts_hideStatusBar( true ))
        {
            implts_writeWindowStateData( STATUS_BAR_ALIAS, m_aStatusBarElement );
            bMustLayout = true;
            bNotify     = true;
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        bNotify = implts_hideProgressBar();
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            bNotify     = pToolbarManager->hideToolbar( aName );
            bMustLayout = pToolbarManager->isLayoutDirty();
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        SolarMutexClearableGuard aReadGuard;
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        uno::Reference< XComponentContext > xContext( m_xContext );
        aReadGuard.clear();

        impl_setDockingWindowVisibility( xContext, xFrame, aElementName, false );
    }

    if ( bMustLayout )
        doLayout();

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_INVISIBLE, uno::makeAny( aName ) );

    return false;
}

sal_Bool SAL_CALL LayoutManager::dockWindow( const OUString& aName, DockingArea DockingArea, const awt::Point& Pos )
{
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager*             pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            pToolbarManager->dockToolbar( aName, DockingArea, Pos );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return false;
}

sal_Bool SAL_CALL LayoutManager::dockAllWindows( ::sal_Int16 /*nElementType*/ )
{
    SolarMutexClearableGuard aReadLock;
    bool bResult( false );
    ToolbarLayoutManager*             pToolbarManager = m_xToolbarManager.get();
    aReadLock.clear();

    if ( pToolbarManager )
    {
        bResult = pToolbarManager->dockAllToolbars();
        if ( pToolbarManager->isLayoutDirty() )
            doLayout();
    }
    return bResult;
}

sal_Bool SAL_CALL LayoutManager::floatWindow( const OUString& aName )
{
    bool bResult( false );
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager*             pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            bResult = pToolbarManager->floatToolbar( aName );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return bResult;
}

sal_Bool SAL_CALL LayoutManager::lockWindow( const OUString& aName )
{
    bool bResult( false );
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager*             pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            bResult = pToolbarManager->lockToolbar( aName );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return bResult;
}

sal_Bool SAL_CALL LayoutManager::unlockWindow( const OUString& aName )
{
    bool bResult( false );
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager*             pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            bResult = pToolbarManager->unlockToolbar( aName );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return bResult;
}

void SAL_CALL LayoutManager::setElementSize( const OUString& aName, const awt::Size& aSize )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager*             pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
        {
            pToolbarManager->setToolbarSize( aName, aSize );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
}

void SAL_CALL LayoutManager::setElementPos( const OUString& aName, const awt::Point& aPos )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager( m_xToolbarManager.get() );
        aReadLock.clear();

        if ( pToolbarManager )
        {
            pToolbarManager->setToolbarPos( aName, aPos );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
}

void SAL_CALL LayoutManager::setElementPosSize( const OUString& aName, const awt::Point& aPos, const awt::Size& aSize )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager( m_xToolbarManager.get() );
        aReadLock.clear();

        if ( pToolbarManager )
        {
            pToolbarManager->setToolbarPosSize( aName, aPos, aSize );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
}

sal_Bool SAL_CALL LayoutManager::isElementVisible( const OUString& aName )
{
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        SolarMutexResettableGuard aReadLock;
        if ( m_xContainerWindow.is() )
        {
            aReadLock.clear();

            SolarMutexGuard aGuard;
            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
            {
                MenuBar* pMenuBar = pSysWindow->GetMenuBar();
                if ( pMenuBar && pMenuBar->IsDisplayable() )
                    return true;
            }
            else
            {
                aReadLock.reset();
                return m_bMenuVisible;
            }
        }
    }
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
    {
        if ( m_aStatusBarElement.m_xUIElement.is() )
        {
            Reference< awt::XWindow > xWindow( m_aStatusBarElement.m_xUIElement->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
            {
                SolarMutexGuard g;
                VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->IsVisible() )
                    return true;
                else
                    return false;
            }
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        if ( m_aProgressBarElement.m_xUIElement.is() )
            return m_aProgressBarElement.m_bVisible;
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarVisible( aName );
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        SolarMutexClearableGuard aReadGuard;
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        aReadGuard.clear();

        return IsDockingWindowVisible( xFrame, aElementName );
    }

    return false;
}

sal_Bool SAL_CALL LayoutManager::isElementFloating( const OUString& aName )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarFloating( aName );
    }

    return false;
}

sal_Bool SAL_CALL LayoutManager::isElementDocked( const OUString& aName )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarDocked( aName );
    }

    return false;
}

sal_Bool SAL_CALL LayoutManager::isElementLocked( const OUString& aName )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarLocked( aName );
    }

    return false;
}

awt::Size SAL_CALL LayoutManager::getElementSize( const OUString& aName )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
            return pToolbarManager->getToolbarSize( aName );
    }

    return awt::Size();
}

awt::Point SAL_CALL LayoutManager::getElementPos( const OUString& aName )
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        SolarMutexClearableGuard aReadLock;
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        aReadLock.clear();

        if ( pToolbarManager )
            return pToolbarManager->getToolbarPos( aName );
    }

    return awt::Point();
}

void SAL_CALL LayoutManager::lock()
{
    implts_lock();

    SolarMutexClearableGuard aReadLock;
    sal_Int32 nLockCount( m_nLockCount );
    aReadLock.clear();

    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::lock lockCount=" << nLockCount );
#ifdef DBG_UTIL
    SAL_INFO( "fwk", "LayoutManager::lock " << reinterpret_cast<sal_Int64>(this) << " - " << nLockCount );
#endif

    Any a( nLockCount );
    implts_notifyListeners( frame::LayoutManagerEvents::LOCK, a );
}

void SAL_CALL LayoutManager::unlock()
{
    bool bDoLayout( implts_unlock() );

    SolarMutexClearableGuard aReadLock;
    sal_Int32 nLockCount( m_nLockCount );
    aReadLock.clear();

    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::unlock lockCount=" << nLockCount );
#ifdef DBG_UTIL
    SAL_INFO( "fwk", "LayoutManager::unlock " << reinterpret_cast<sal_Int64>(this) << " - " << nLockCount);
#endif
    // conform to documentation: unlock with lock count == 0 means force a layout

    SolarMutexClearableGuard aWriteLock;
    if ( bDoLayout )
        m_aAsyncLayoutTimer.Stop();
    aWriteLock.clear();

    Any a( nLockCount );
    implts_notifyListeners( frame::LayoutManagerEvents::UNLOCK, a );

    if ( bDoLayout )
        implts_doLayout_notify( true );
}

void SAL_CALL LayoutManager::doLayout()
{
    implts_doLayout_notify( true );
}

//  ILayoutNotifications

void LayoutManager::requestLayout()
{
    doLayout();
}

void LayoutManager::implts_doLayout_notify( bool bOuterResize )
{
    bool bLayouted = implts_doLayout( false, bOuterResize );
    if ( bLayouted )
        implts_notifyListeners( frame::LayoutManagerEvents::LAYOUT, Any() );
}

bool LayoutManager::implts_doLayout( bool bForceRequestBorderSpace, bool bOuterResize )
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::implts_doLayout" );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aReadLock;

    if ( !m_xFrame.is() || !m_bParentWindowVisible )
        return false;

    bool bPreserveContentSize( m_bPreserveContentSize );
    bool bMustDoLayout( m_bMustDoLayout );
    bool bNoLock = ( m_nLockCount == 0 );
    awt::Rectangle aCurrBorderSpace( m_aDockingArea );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< awt::XTopWindow2 > xContainerTopWindow( m_xContainerTopWindow );
    Reference< awt::XWindow > xComponentWindow;
    try {
        xComponentWindow = m_xFrame->getComponentWindow();
    } catch (css::lang::DisposedException &) {
        // There can be a race between one thread calling Frame::dispose
        // (framework/source/services/frame.cxx) -> Frame::disableLayoutManager
        // -> LayoutManager::attachFrame(null) setting m_xFrame to null, and
        // the main thread firing the timer-triggered
        // LayoutManager::AsyncLayoutHdl -> LayoutManager::implts_doLayout and
        // calling into the in-dispose m_xFrame here, so silently ignore a
        // DisposedException here:
        return false;
    }
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    aReadLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    bool bLayouted( false );

    if ( bNoLock && xDockingAreaAcceptor.is() && xContainerWindow.is() && xComponentWindow.is() )
    {
        bLayouted = true;

        SolarMutexResettableGuard aWriteGuard;
        aWriteGuard.clear();

        awt::Rectangle aDockSpace( implts_calcDockingAreaSizes() );
        awt::Rectangle aBorderSpace( aDockSpace );
        bool       bGotRequestedBorderSpace( true );

        // We have to add the height of a possible status bar
        aBorderSpace.Height += implts_getStatusBarSize().Height();

        if ( !equalRectangles( aBorderSpace, aCurrBorderSpace ) || bForceRequestBorderSpace || bMustDoLayout )
        {
            // we always resize the content window (instead of the complete container window) if we're not set up
            // to (attempt to) preserve the content window's size
            if ( bOuterResize && !bPreserveContentSize )
                bOuterResize = false;

            // maximized windows can resized their content window only, not their container window
            if ( bOuterResize && xContainerTopWindow.is() && xContainerTopWindow->getIsMaximized() )
                bOuterResize = false;

            // if the component window does not have a size (yet), then we can't use it to calc the container
            // window size
            awt::Rectangle aComponentRect = xComponentWindow->getPosSize();
            if ( bOuterResize && ( aComponentRect.Width == 0 ) && ( aComponentRect.Height == 0 ) )
                bOuterResize = false;

            bGotRequestedBorderSpace = false;
            if ( bOuterResize )
            {
                Reference< awt::XDevice > xDevice( xContainerWindow, uno::UNO_QUERY );
                awt::DeviceInfo aContainerInfo  = xDevice->getInfo();

                awt::Size aRequestedSize( aComponentRect.Width + aContainerInfo.LeftInset + aContainerInfo.RightInset + aBorderSpace.X + aBorderSpace.Width,
                                          aComponentRect.Height + aContainerInfo.TopInset  + aContainerInfo.BottomInset + aBorderSpace.Y + aBorderSpace.Height );
                awt::Point aComponentPos( aBorderSpace.X, aBorderSpace.Y );

                bGotRequestedBorderSpace = implts_resizeContainerWindow( aRequestedSize, aComponentPos );
            }

            // if we did not do an container window resize, or it failed, then use the DockingAcceptor as usual
            if ( !bGotRequestedBorderSpace )
                bGotRequestedBorderSpace = xDockingAreaAcceptor->requestDockingAreaSpace( aBorderSpace );

            if ( bGotRequestedBorderSpace )
            {
                aWriteGuard.reset();
                m_aDockingArea = aBorderSpace;
                m_bMustDoLayout = false;
                aWriteGuard.clear();
            }
        }

        if ( bGotRequestedBorderSpace )
        {
            ::Size      aContainerSize;
            ::Size      aStatusBarSize;

            // Interim solution to let the layout method within the
            // toolbar layout manager.
            implts_setOffset( implts_getStatusBarSize().Height() );
            if ( m_xToolbarManager.is() )
                m_xToolbarManager->setDockingArea( aDockSpace );

            // Subtract status bar size from our container output size. Docking area windows
            // don't contain the status bar!
            aStatusBarSize = implts_getStatusBarSize();
            aContainerSize = implts_getContainerWindowOutputSize();
            aContainerSize.AdjustHeight( -(aStatusBarSize.Height()) );

            if ( m_xToolbarManager.is() )
                m_xToolbarManager->doLayout(aContainerSize);

            // Position the status bar
            if ( aStatusBarSize.Height() > 0 )
            {
                implts_setStatusBarPosSize( ::Point( 0, std::max(( aContainerSize.Height() ), long( 0 ))),
                                            ::Size( aContainerSize.Width(),aStatusBarSize.Height() ));
            }

            xDockingAreaAcceptor->setDockingAreaSpace( aBorderSpace );

            aWriteGuard.reset();
            aWriteGuard.clear();
        }
    }

    return bLayouted;
}

bool LayoutManager::implts_resizeContainerWindow( const awt::Size& rContainerSize,
                                                      const awt::Point& rComponentPos )
{
    SolarMutexClearableGuard aReadLock;
    Reference< awt::XWindow >               xContainerWindow    = m_xContainerWindow;
    Reference< awt::XTopWindow2 >           xContainerTopWindow = m_xContainerTopWindow;
    Reference< awt::XWindow >               xComponentWindow    = m_xFrame->getComponentWindow();
    aReadLock.clear();

    // calculate the maximum size we have for the container window
    sal_Int32 nDisplay = xContainerTopWindow->getDisplay();
    tools::Rectangle aWorkArea = Application::GetScreenPosSizePixel( nDisplay );

    if (( aWorkArea.GetWidth() > 0 ) && ( aWorkArea.GetHeight() > 0 ))
    {
        if (( rContainerSize.Width > aWorkArea.GetWidth() ) || ( rContainerSize.Height > aWorkArea.GetHeight() ))
            return false;
        // Strictly, this is not correct. If we have a multi-screen display (css.awt.DisplayAccess.MultiDisplay == true),
        // the "effective work area" would be much larger than the work area of a single display, since we could in theory
        // position the container window across multiple screens.
        // However, this should suffice as a heuristics here ... (nobody really wants to check whether the different screens are
        // stacked horizontally or vertically, whether their work areas can really be combined, or are separated by non-work-areas,
        // and the like ... right?)
    }

    // resize our container window
    xContainerWindow->setPosSize( 0, 0, rContainerSize.Width, rContainerSize.Height, awt::PosSize::SIZE );
    // position the component window
    xComponentWindow->setPosSize( rComponentPos.X, rComponentPos.Y, 0, 0, awt::PosSize::POS );
    return true;
}

void SAL_CALL LayoutManager::setVisible( sal_Bool bVisible )
{
    SolarMutexClearableGuard aWriteLock;
    bool bWasVisible( m_bVisible );
    m_bVisible = bVisible;
    aWriteLock.clear();

    if ( bWasVisible != bool(bVisible) )
        implts_setVisibleState( bVisible );
}

sal_Bool SAL_CALL LayoutManager::isVisible()
{
    SolarMutexGuard g;
    return m_bVisible;
}

::Size LayoutManager::implts_getStatusBarSize()
{
    SolarMutexClearableGuard aReadLock;
    bool bStatusBarVisible( isElementVisible( STATUS_BAR_ALIAS ));
    bool bProgressBarVisible( isElementVisible( "private:resource/progressbar/progressbar" ));
    bool bVisible( m_bVisible );
    Reference< XUIElement > xStatusBar( m_aStatusBarElement.m_xUIElement );
    Reference< XUIElement > xProgressBar( m_aProgressBarElement.m_xUIElement );

    Reference< awt::XWindow > xWindow;
    if ( bStatusBarVisible && bVisible && xStatusBar.is() )
        xWindow.set( xStatusBar->getRealInterface(), UNO_QUERY );
    else if ( xProgressBar.is() && !xStatusBar.is() && bProgressBarVisible )
    {
        ProgressBarWrapper* pWrapper = static_cast<ProgressBarWrapper*>(xProgressBar.get());
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
    }
    aReadLock.clear();

    if ( xWindow.is() )
    {
        awt::Rectangle aPosSize = xWindow->getPosSize();
        return ::Size( aPosSize.Width, aPosSize.Height );
    }
    else
        return ::Size();
}

awt::Rectangle LayoutManager::implts_calcDockingAreaSizes()
{
    SolarMutexClearableGuard aReadLock;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    aReadLock.clear();

    awt::Rectangle aBorderSpace;
    if ( m_xToolbarManager.is() && xDockingAreaAcceptor.is() && xContainerWindow.is() )
        aBorderSpace = m_xToolbarManager->getDockingArea();

    return aBorderSpace;
}

void LayoutManager::implts_setDockingAreaWindowSizes()
{
    SolarMutexClearableGuard aReadLock;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    aReadLock.clear();

    uno::Reference< awt::XDevice > xDevice( xContainerWindow, uno::UNO_QUERY );
    // Convert relative size to output size.
    awt::Rectangle  aRectangle           = xContainerWindow->getPosSize();
    awt::DeviceInfo aInfo                = xDevice->getInfo();
    awt::Size       aContainerClientSize = awt::Size( aRectangle.Width - aInfo.LeftInset - aInfo.RightInset,
                                                      aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );
    ::Size          aStatusBarSize       = implts_getStatusBarSize();

    // Position the status bar
    if ( aStatusBarSize.Height() > 0 )
    {
        implts_setStatusBarPosSize( ::Point( 0, std::max(( aContainerClientSize.Height - aStatusBarSize.Height() ), long( 0 ))),
                                    ::Size( aContainerClientSize.Width, aStatusBarSize.Height() ));
    }
}

void LayoutManager::implts_updateMenuBarClose()
{
    SolarMutexClearableGuard aWriteLock;
    bool                      bShowCloseButton( m_bMenuBarCloseButton );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    aWriteLock.clear();

    if ( xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        SystemWindow* pSysWindow = getTopSystemWindow( xContainerWindow );
        if ( pSysWindow )
        {
            MenuBar* pMenuBar = pSysWindow->GetMenuBar();
            if ( pMenuBar )
            {
                // TODO remove link on sal_False ?!
                pMenuBar->ShowCloseButton(bShowCloseButton);
                pMenuBar->SetCloseButtonClickHdl(LINK(this, LayoutManager, MenuBarClose));
            }
        }
    }
}

bool LayoutManager::implts_resetMenuBar()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;
    bool bMenuVisible( m_bMenuVisible );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );

    MenuBar* pSetMenuBar = nullptr;
    if ( m_xInplaceMenuBar.is() )
        pSetMenuBar = static_cast<MenuBar *>(m_xInplaceMenuBar->GetMenuBar());
    else
    {
        MenuBarWrapper* pMenuBarWrapper = static_cast< MenuBarWrapper* >( m_xMenuBar.get() );
        if ( pMenuBarWrapper )
            pSetMenuBar = static_cast<MenuBar*>(pMenuBarWrapper->GetMenuBarManager()->GetMenuBar());
    }
    aWriteLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    SolarMutexGuard aGuard;
    SystemWindow* pSysWindow = getTopSystemWindow( xContainerWindow );
    if ( pSysWindow && bMenuVisible && pSetMenuBar )
    {
        pSysWindow->SetMenuBar(pSetMenuBar);
        pSetMenuBar->SetDisplayable( true );
        return true;
    }

    return false;
}

void LayoutManager::implts_createMSCompatibleMenuBar( const OUString& aName )
{
    SolarMutexClearableGuard aWriteLock;

    // Find Forms menu in the original menubar
    m_xMenuBar = implts_createElement( aName );
    uno::Reference< XUIElementSettings > xMenuBarSettings(m_xMenuBar, UNO_QUERY);
    uno::Reference< container::XIndexReplace > xMenuIndex(xMenuBarSettings->getSettings(true), UNO_QUERY);

    sal_Int32 nFormsMenu = -1;
    for (sal_Int32 nIndex = 0; nIndex < xMenuIndex->getCount(); ++nIndex)
    {
        uno::Sequence< beans::PropertyValue > aProps;
        xMenuIndex->getByIndex( nIndex ) >>= aProps;
        OUString aCommand;
        for (sal_Int32 nSeqInd = 0; nSeqInd < aProps.getLength(); ++nSeqInd)
        {
            if (aProps[nSeqInd].Name == "CommandURL")
            {
                aProps[nSeqInd].Value >>= aCommand;
                break;
            }
        }

        if (aCommand == ".uno:FormatFormMenu")
            nFormsMenu = nIndex;
    }
    assert(nFormsMenu != -1);

    // Create the MS compatible Forms menu
    css::uno::Reference< css::ui::XUIElement > xFormsMenu = implts_createElement( "private:resource/menubar/mscompatibleformsmenu" );
    if(!xFormsMenu.is())
        return;

    // Merge the MS compatible Forms menu into the menubar
    uno::Reference< XUIElementSettings > xFormsMenuSettings(xFormsMenu, UNO_QUERY);
    uno::Reference< container::XIndexAccess > xFormsMenuIndex(xFormsMenuSettings->getSettings(true));

    assert(xFormsMenuIndex->getCount() >= 1);
    uno::Sequence< beans::PropertyValue > aNewFormsMenu;
    xFormsMenuIndex->getByIndex( 0 ) >>= aNewFormsMenu;
    xMenuIndex->replaceByIndex(nFormsMenu, uno::makeAny(aNewFormsMenu));

    setMergedMenuBar( xMenuIndex );

    // Clear up the temporal forms menubar
    Reference< XComponent > xFormsMenuComp( xFormsMenu, UNO_QUERY );
    if ( xFormsMenuComp.is() )
        xFormsMenuComp->dispose();
    xFormsMenu.clear();

    aWriteLock.clear();
}

IMPL_LINK_NOARG(LayoutManager, MenuBarClose, void*, void)
{
    SolarMutexClearableGuard aReadLock;
    uno::Reference< frame::XDispatchProvider >   xProvider(m_xFrame, uno::UNO_QUERY);
    uno::Reference< XComponentContext > xContext( m_xContext );
    aReadLock.clear();

    if ( !xProvider.is())
        return;

    uno::Reference< frame::XDispatchHelper > xDispatcher = frame::DispatchHelper::create( xContext );

    xDispatcher->executeDispatch(
        xProvider,
        ".uno:CloseWin",
        "_self",
        0,
        uno::Sequence< beans::PropertyValue >());
}

//  XLayoutManagerEventBroadcaster

void SAL_CALL LayoutManager::addLayoutManagerEventListener( const uno::Reference< frame::XLayoutManagerListener >& xListener )
{
    m_aListenerContainer.addInterface( cppu::UnoType<frame::XLayoutManagerListener>::get(), xListener );
}

void SAL_CALL LayoutManager::removeLayoutManagerEventListener( const uno::Reference< frame::XLayoutManagerListener >& xListener )
{
    m_aListenerContainer.removeInterface( cppu::UnoType<frame::XLayoutManagerListener>::get(), xListener );
}

void LayoutManager::implts_notifyListeners(short nEvent, const uno::Any& rInfoParam)
{
    lang::EventObject                  aSource( static_cast< ::cppu::OWeakObject*>(this) );
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( cppu::UnoType<frame::XLayoutManagerListener>::get());
    if (pContainer!=nullptr)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                static_cast<frame::XLayoutManagerListener*>(pIterator.next())->layoutEvent(aSource, nEvent, rInfoParam);
            }
            catch( const uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

//      XWindowListener

void SAL_CALL LayoutManager::windowResized( const awt::WindowEvent& aEvent )
{
    SolarMutexGuard g;
    Reference< awt::XWindow >         xContainerWindow( m_xContainerWindow );

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source && m_bVisible )
    {
        // We have to call our resize handler at least once synchronously, as some
        // application modules need this. So we have to check if this is the first
        // call after the async layout time expired.
        m_bMustDoLayout = true;
        if ( !m_aAsyncLayoutTimer.IsActive() )
        {
            m_aAsyncLayoutTimer.Invoke();
            if ( m_nLockCount == 0 )
                m_aAsyncLayoutTimer.Start();
        }
    }
    else if ( m_xFrame.is() && aEvent.Source == m_xFrame->getContainerWindow() )
    {
        // the container window of my DockingAreaAcceptor is not the same as of my frame
        // I still have to resize my frames' window as nobody else will do it
        Reference< awt::XWindow > xComponentWindow( m_xFrame->getComponentWindow() );
        if( xComponentWindow.is() )
        {
            uno::Reference< awt::XDevice > xDevice( m_xFrame->getContainerWindow(), uno::UNO_QUERY );

            // Convert relative size to output size.
            awt::Rectangle  aRectangle = m_xFrame->getContainerWindow()->getPosSize();
            awt::DeviceInfo aInfo      = xDevice->getInfo();
            awt::Size       aSize(  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                    aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            // Resize our component window.
            xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, awt::PosSize::POSSIZE );
        }
    }
}

void SAL_CALL LayoutManager::windowMoved( const awt::WindowEvent& )
{
}

void SAL_CALL LayoutManager::windowShown( const lang::EventObject& aEvent )
{
    SolarMutexClearableGuard aReadLock;
    Reference< awt::XWindow >  xContainerWindow( m_xContainerWindow );
    bool                       bParentWindowVisible( m_bParentWindowVisible );
    aReadLock.clear();

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        SolarMutexClearableGuard aWriteLock;
        m_bParentWindowVisible = true;
        bool bSetVisible = ( m_bParentWindowVisible != bParentWindowVisible );
        aWriteLock.clear();

        if ( bSetVisible )
            implts_updateUIElementsVisibleState( true );
    }
}

void SAL_CALL LayoutManager::windowHidden( const lang::EventObject& aEvent )
{
    SolarMutexClearableGuard aReadLock;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    bool                      bParentWindowVisible( m_bParentWindowVisible );
    aReadLock.clear();

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        SolarMutexClearableGuard aWriteLock;
        m_bParentWindowVisible = false;
        bool bSetInvisible = ( m_bParentWindowVisible != bParentWindowVisible );
        aWriteLock.clear();

        if ( bSetInvisible )
            implts_updateUIElementsVisibleState( false );
    }
}

IMPL_LINK_NOARG(LayoutManager, AsyncLayoutHdl, Timer *, void)
{
    SolarMutexClearableGuard aReadLock;

    if( !m_xContainerWindow.is() )
        return;

    aReadLock.clear();

    implts_setDockingAreaWindowSizes();
    implts_doLayout( true, false );
}

//      XFrameActionListener

void SAL_CALL LayoutManager::frameAction( const FrameActionEvent& aEvent )
{
    if (( aEvent.Action == FrameAction_COMPONENT_ATTACHED ) || ( aEvent.Action == FrameAction_COMPONENT_REATTACHED ))
    {
        SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::frameAction (COMPONENT_ATTACHED|REATTACHED)" );

        SolarMutexClearableGuard aWriteLock;
        m_bMustDoLayout = true;
        aWriteLock.clear();

        implts_reset( true );
        implts_doLayout( true, false );
        implts_doLayout( true, true );
    }
    else if (( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED ) || ( aEvent.Action == FrameAction_FRAME_UI_DEACTIVATING ))
    {
        SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::frameAction (FRAME_UI_ACTIVATED|DEACTIVATING)" );

        implts_toggleFloatingUIElementsVisibility( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
    }
    else if ( aEvent.Action == FrameAction_COMPONENT_DETACHING )
    {
        SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::frameAction (COMPONENT_DETACHING)" );

        implts_reset( false );
    }
}

void SAL_CALL LayoutManager::disposing( const lang::EventObject& rEvent )
{
    bool bDisposeAndClear( false );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    SolarMutexClearableGuard aWriteLock;

    if ( rEvent.Source == Reference< XInterface >( m_xFrame, UNO_QUERY ))
    {
        // Our frame gets disposed, release all our references that depends on a working frame reference.

        setDockingAreaAcceptor( Reference< ui::XDockingAreaAcceptor >() );

        // destroy all elements, it's possible that detaching is NOT called!
        implts_destroyElements();
        impl_clearUpMenuBar();
        m_xMenuBar.clear();
        VclPtr<Menu> pMenuBar;
        if (m_xInplaceMenuBar.is())
        {
            pMenuBar = m_xInplaceMenuBar->GetMenuBar();
            m_xInplaceMenuBar->dispose();
            m_xInplaceMenuBar.clear();
        }
        pMenuBar.disposeAndClear();
        m_xContainerWindow.clear();
        m_xContainerTopWindow.clear();

        // forward disposing call to toolbar manager
        if ( m_xToolbarManager.is() )
            m_xToolbarManager->disposing(rEvent);

        if ( m_xModuleCfgMgr.is() )
        {
            try
            {
                Reference< XUIConfiguration > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
                xModuleCfgMgr->removeConfigurationListener(
                    Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch (const Exception&)
            {
            }
        }

        if ( m_xDocCfgMgr.is() )
        {
            try
            {
                Reference< XUIConfiguration > xDocCfgMgr( m_xDocCfgMgr, UNO_QUERY );
                xDocCfgMgr->removeConfigurationListener(
                    Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch (const Exception&)
            {
            }
        }

        m_xDocCfgMgr.clear();
        m_xModuleCfgMgr.clear();
        m_xFrame.clear();
        m_pGlobalSettings.reset();

        bDisposeAndClear = true;
    }
    else if ( rEvent.Source == Reference< XInterface >( m_xContainerWindow, UNO_QUERY ))
    {
        // Our container window gets disposed. Remove all user interface elements.
        ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
        if ( pToolbarManager )
        {
            uno::Reference< awt::XWindowPeer > aEmptyWindowPeer;
            pToolbarManager->setParentWindow( aEmptyWindowPeer );
        }
        impl_clearUpMenuBar();
        m_xMenuBar.clear();
        VclPtr<Menu> pMenuBar;
        if (m_xInplaceMenuBar.is())
        {
            pMenuBar = m_xInplaceMenuBar->GetMenuBar();
            m_xInplaceMenuBar->dispose();
            m_xInplaceMenuBar.clear();
        }
        pMenuBar.disposeAndClear();
        m_xContainerWindow.clear();
        m_xContainerTopWindow.clear();
    }
    else if ( rEvent.Source == Reference< XInterface >( m_xDocCfgMgr, UNO_QUERY ))
        m_xDocCfgMgr.clear();
    else if ( rEvent.Source == Reference< XInterface >( m_xModuleCfgMgr , UNO_QUERY ))
        m_xModuleCfgMgr.clear();

    aWriteLock.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // Send disposing to our listener when we have lost our frame.
    if ( bDisposeAndClear )
    {
        // Send message to all listener and forget her references.
        uno::Reference< frame::XLayoutManager > xThis( static_cast< ::cppu::OWeakObject* >(this), uno::UNO_QUERY );
        lang::EventObject aEvent( xThis );
        m_aListenerContainer.disposeAndClear( aEvent );
    }
}

void SAL_CALL LayoutManager::elementInserted( const ui::ConfigurationEvent& Event )
{
    SolarMutexClearableGuard aReadLock;
    Reference< XFrame > xFrame( m_xFrame );
    rtl::Reference< ToolbarLayoutManager > xToolbarManager( m_xToolbarManager );
    aReadLock.clear();

    if ( xFrame.is() )
    {
        OUString aElementType;
        OUString aElementName;
        bool            bRefreshLayout(false);

        parseResourceURL( Event.ResourceURL, aElementType, aElementName );
        if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
        {
            if ( xToolbarManager.is() )
            {
                xToolbarManager->elementInserted( Event );
                bRefreshLayout = xToolbarManager->isLayoutDirty();
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_MENUBAR ))
        {
            Reference< XUIElement >         xUIElement = implts_findElement( Event.ResourceURL );
            Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
            if ( xElementSettings.is() )
            {
                uno::Reference< XPropertySet > xPropSet( xElementSettings, uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    if ( Event.Source == uno::Reference< uno::XInterface >( m_xDocCfgMgr, uno::UNO_QUERY ))
                        xPropSet->setPropertyValue( "ConfigurationSource", makeAny( m_xDocCfgMgr ));
                }
                xElementSettings->updateSettings();
            }
        }

        if ( bRefreshLayout )
            doLayout();
    }
}

void SAL_CALL LayoutManager::elementRemoved( const ui::ConfigurationEvent& Event )
{
    SolarMutexClearableGuard aReadLock;
    Reference< frame::XFrame >                xFrame( m_xFrame );
    rtl::Reference< ToolbarLayoutManager >    xToolbarManager( m_xToolbarManager );
    Reference< awt::XWindow >                 xContainerWindow( m_xContainerWindow );
    Reference< ui::XUIElement >               xMenuBar( m_xMenuBar );
    Reference< ui::XUIConfigurationManager >  xModuleCfgMgr( m_xModuleCfgMgr );
    Reference< ui::XUIConfigurationManager >  xDocCfgMgr( m_xDocCfgMgr );
    aReadLock.clear();

    if ( xFrame.is() )
    {
        OUString aElementType;
        OUString aElementName;
        bool            bRefreshLayout(false);

        parseResourceURL( Event.ResourceURL, aElementType, aElementName );
        if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
        {
            if ( xToolbarManager.is() )
            {
                xToolbarManager->elementRemoved( Event );
                bRefreshLayout = xToolbarManager->isLayoutDirty();
            }
        }
        else
        {
            Reference< XUIElement > xUIElement = implts_findElement( Event.ResourceURL );
            Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
            if ( xElementSettings.is() )
            {
                bool                      bNoSettings( false );
                OUString           aConfigSourcePropName( "ConfigurationSource" );
                Reference< XInterface >   xElementCfgMgr;
                Reference< XPropertySet > xPropSet( xElementSettings, UNO_QUERY );

                if ( xPropSet.is() )
                    xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

                if ( !xElementCfgMgr.is() )
                    return;

                // Check if the same UI configuration manager has changed => check further
                if ( Event.Source == xElementCfgMgr )
                {
                    // Same UI configuration manager where our element has its settings
                    if ( Event.Source == Reference< XInterface >( xDocCfgMgr, UNO_QUERY ))
                    {
                        // document settings removed
                        if ( xModuleCfgMgr->hasSettings( Event.ResourceURL ))
                        {
                            xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( m_xModuleCfgMgr ));
                            xElementSettings->updateSettings();
                            return;
                        }
                    }

                    bNoSettings = true;
                }

                // No settings anymore, element must be destroyed
                if ( xContainerWindow.is() && bNoSettings )
                {
                    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
                         aElementName.equalsIgnoreAsciiCase("menubar") )
                    {
                        SystemWindow* pSysWindow = getTopSystemWindow( xContainerWindow );
                        if ( pSysWindow && !m_bInplaceMenuSet )
                            pSysWindow->SetMenuBar( nullptr );

                        Reference< XComponent > xComp( xMenuBar, UNO_QUERY );
                        if ( xComp.is() )
                            xComp->dispose();

                        SolarMutexGuard g;
                        m_xMenuBar.clear();
                    }
                }
            }
        }

        if ( bRefreshLayout )
            doLayout();
    }
}

void SAL_CALL LayoutManager::elementReplaced( const ui::ConfigurationEvent& Event )
{
    SolarMutexClearableGuard aReadLock;
    Reference< XFrame >                       xFrame( m_xFrame );
    rtl::Reference< ToolbarLayoutManager >    xToolbarManager( m_xToolbarManager );
    aReadLock.clear();

    if ( xFrame.is() )
    {
        OUString aElementType;
        OUString aElementName;
        bool            bRefreshLayout(false);

        parseResourceURL( Event.ResourceURL, aElementType, aElementName );
        if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
        {
            if ( xToolbarManager.is() )
            {
                xToolbarManager->elementReplaced( Event );
                bRefreshLayout = xToolbarManager->isLayoutDirty();
            }
        }
        else
        {
            Reference< XUIElement >         xUIElement = implts_findElement( Event.ResourceURL );
            Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
            if ( xElementSettings.is() )
            {
                Reference< XInterface >   xElementCfgMgr;
                Reference< XPropertySet > xPropSet( xElementSettings, UNO_QUERY );

                if ( xPropSet.is() )
                    xPropSet->getPropertyValue( "ConfigurationSource" ) >>= xElementCfgMgr;

                if ( !xElementCfgMgr.is() )
                    return;

                // Check if the same UI configuration manager has changed => update settings
                if ( Event.Source == xElementCfgMgr )
                    xElementSettings->updateSettings();
            }
        }

        if ( bRefreshLayout )
            doLayout();
    }
}

void SAL_CALL LayoutManager::setFastPropertyValue_NoBroadcast( sal_Int32       nHandle,
                                                               const uno::Any& aValue  )
{
    if ( nHandle != LAYOUTMANAGER_PROPHANDLE_REFRESHVISIBILITY )
        LayoutManager_PBase::setFastPropertyValue_NoBroadcast( nHandle, aValue );

    switch( nHandle )
    {
        case LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER:
            implts_updateMenuBarClose();
            break;

        case LAYOUTMANAGER_PROPHANDLE_REFRESHVISIBILITY:
        {
            bool bValue(false);
            if (( aValue >>= bValue ) && bValue )
            {
                SolarMutexClearableGuard aReadLock;
                ToolbarLayoutManager* pToolbarManager = m_xToolbarManager.get();
                bool bAutomaticToolbars( m_bAutomaticToolbars );
                aReadLock.clear();

                if ( pToolbarManager )
                    pToolbarManager->refreshToolbarsVisibility( bAutomaticToolbars );
            }
            break;
        }

        case LAYOUTMANAGER_PROPHANDLE_HIDECURRENTUI:
            implts_setCurrentUIVisibility( !m_bHideCurrentUI );
            break;
        default: break;
    }
}

namespace detail
{
    class InfoHelperBuilder
    {
    private:
        std::unique_ptr<::cppu::OPropertyArrayHelper> m_pInfoHelper;
    public:
        explicit InfoHelperBuilder(const LayoutManager &rManager)
        {
            uno::Sequence< beans::Property > aProperties;
            rManager.describeProperties(aProperties);
            m_pInfoHelper.reset( new ::cppu::OPropertyArrayHelper(aProperties, true) );
        }
        InfoHelperBuilder(const InfoHelperBuilder&) = delete;
        InfoHelperBuilder& operator=(const InfoHelperBuilder&) = delete;

        ::cppu::OPropertyArrayHelper& getHelper() { return *m_pInfoHelper; }
    };
}
namespace
{
    struct theInfoHelper :
        public rtl::StaticWithArg< detail::InfoHelperBuilder, LayoutManager,
        theInfoHelper >
    {
    };
}

::cppu::IPropertyArrayHelper& SAL_CALL LayoutManager::getInfoHelper()
{
    return theInfoHelper::get(*this).getHelper();
}

uno::Reference< beans::XPropertySetInfo > SAL_CALL LayoutManager::getPropertySetInfo()
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );

    return xInfo;
}

} // namespace framework

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_framework_LayoutManager_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new framework::LayoutManager(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
