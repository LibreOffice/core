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

#include <services/layoutmanager.hxx>
#include <helpers.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>

#include <framework/sfxhelperfunctions.hxx>
#include <uielement/menubarwrapper.hxx>
#include <framework/addonsoptions.hxx>
#include <uiconfiguration/windowstateconfiguration.hxx>
#include <classes/fwkresid.hxx>
#include <classes/resource.hrc>
#include <toolkit/helper/convert.hxx>
#include <uielement/progressbarwrapper.hxx>
#include <uiconfiguration/globalsettings.hxx>
#include <toolbarlayoutmanager.hxx>

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
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/WindowStateConfiguration.hpp>
#include <com/sun/star/ui/UIElementFactoryManager.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/DispatchHelper.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/util/URLTransformer.hpp>

#include <comphelper/processfactory.hxx>
#include <svtools/imgdef.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dockingarea.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/wall.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/uno3.hxx>
#include <rtl/instance.hxx>
#include <unotools/cmdoptions.hxx>

#include <rtl/strbuf.hxx>

#include <algorithm>
#include <boost/utility.hpp>

//      using namespace
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace ::com::sun::star::frame;


// ATTENTION!
// This value is directly copied from the sfx2 project.
// You have to change BOTH values, see sfx2/inc/sfx2/sfxsids.hrc (SID_DOCKWIN_START)
static const sal_Int32 DOCKWIN_ID_BASE = 9800;

static const char STATUS_BAR_ALIAS[] = "private:resource/statusbar/statusbar";
static const char PROGRESS_BAR_ALIAS[] = "private:resource/progressbar/progressbar";

namespace framework
{

IMPLEMENT_FORWARD_XTYPEPROVIDER2( LayoutManager, LayoutManager_Base, LayoutManager_PBase )
IMPLEMENT_FORWARD_XINTERFACE2( LayoutManager, LayoutManager_Base, LayoutManager_PBase )
DEFINE_XSERVICEINFO_MULTISERVICE_2( LayoutManager, ::cppu::OWeakObject, "com.sun.star.frame.LayoutManager", OUString("com.sun.star.comp.framework.LayoutManager"))
DEFINE_INIT_SERVICE( LayoutManager, {} )

LayoutManager::LayoutManager( const Reference< XComponentContext >& xContext ) : LayoutManager_Base()
        , ThreadHelpBase( &Application::GetSolarMutex())
        , ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex())
        , LayoutManager_PBase( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        , m_xContext( xContext )
        , m_xURLTransformer( URLTransformer::create(xContext) )
        , m_nLockCount( 0 )
        , m_bActive( false )
        , m_bInplaceMenuSet( false )
        , m_bDockingInProgress( false )
        , m_bMenuVisible( true )
        , m_bComponentAttached( false )
        , m_bDoLayout( false )
        , m_bVisible( true )
        , m_bParentWindowVisible( false )
        , m_bMustDoLayout( true )
        , m_bAutomaticToolbars( true )
        , m_bStoreWindowState( false )
        , m_bHideCurrentUI( false )
        , m_bGlobalSettings( false )
        , m_bPreserveContentSize( false )
        , m_bMenuBarCloser( false )
        , m_pInplaceMenuBar( NULL )
        , m_xModuleManager( ModuleManager::create( xContext ))
        , m_xUIElementFactoryManager( ui::UIElementFactoryManager::create(xContext) )
        , m_xPersistentWindowStateSupplier( ui::WindowStateConfiguration::create( xContext ) )
        , m_pGlobalSettings( 0 )
        , m_aListenerContainer( m_aLock.getShareableOslMutex() )
        , m_pToolbarManager( 0 )
        , m_xToolbarManager( 0 )
{
    // Initialize statusbar member
    const sal_Bool bRefreshVisibility = sal_False;
    m_aStatusBarElement.m_aType = "statusbar";
    m_aStatusBarElement.m_aName = STATUS_BAR_ALIAS;

    m_pToolbarManager = new ToolbarLayoutManager( xContext, Reference<XUIElementFactory>(m_xUIElementFactoryManager, UNO_QUERY_THROW), this );
    m_xToolbarManager = uno::Reference< ui::XUIConfigurationListener >( static_cast< OWeakObject* >( m_pToolbarManager ), uno::UNO_QUERY );

    Application::AddEventListener( LINK( this, LayoutManager, SettingsChanged ) );

    m_aAsyncLayoutTimer.SetTimeout( 50 );
    m_aAsyncLayoutTimer.SetTimeoutHdl( LINK( this, LayoutManager, AsyncLayoutHdl ) );

    registerProperty( LAYOUTMANAGER_PROPNAME_AUTOMATICTOOLBARS, LAYOUTMANAGER_PROPHANDLE_AUTOMATICTOOLBARS, css::beans::PropertyAttribute::TRANSIENT, &m_bAutomaticToolbars, ::getCppuType( &m_bAutomaticToolbars ) );
    registerProperty( LAYOUTMANAGER_PROPNAME_HIDECURRENTUI, LAYOUTMANAGER_PROPHANDLE_HIDECURRENTUI, beans::PropertyAttribute::TRANSIENT, &m_bHideCurrentUI, ::getCppuType( &m_bHideCurrentUI ) );
    registerProperty( LAYOUTMANAGER_PROPNAME_LOCKCOUNT, LAYOUTMANAGER_PROPHANDLE_LOCKCOUNT, beans::PropertyAttribute::TRANSIENT | beans::PropertyAttribute::READONLY, &m_nLockCount, getCppuType( &m_nLockCount )  );
    registerProperty( LAYOUTMANAGER_PROPNAME_MENUBARCLOSER, LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER, beans::PropertyAttribute::TRANSIENT, &m_bMenuBarCloser, ::getCppuType( &m_bMenuBarCloser ) );
    registerPropertyNoMember( LAYOUTMANAGER_PROPNAME_REFRESHVISIBILITY, LAYOUTMANAGER_PROPHANDLE_REFRESHVISIBILITY, beans::PropertyAttribute::TRANSIENT, ::getCppuType( &bRefreshVisibility ), &bRefreshVisibility );
    registerProperty( LAYOUTMANAGER_PROPNAME_PRESERVE_CONTENT_SIZE, LAYOUTMANAGER_PROPHANDLE_PRESERVE_CONTENT_SIZE, beans::PropertyAttribute::TRANSIENT, &m_bPreserveContentSize, ::getCppuType( &m_bPreserveContentSize ) );
}

LayoutManager::~LayoutManager()
{
    Application::RemoveEventListener( LINK( this, LayoutManager, SettingsChanged ) );
    m_aAsyncLayoutTimer.Stop();
    delete m_pGlobalSettings;
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
            MenuBar* pSetMenuBar = 0;
            if ( m_xInplaceMenuBar.is() )
                pSetMenuBar = (MenuBar *)m_pInplaceMenuBar->GetMenuBar();
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
                    pSetMenuBar = (MenuBar*)pAwtMenuBar->GetMenu();
            }

            MenuBar* pTopMenuBar = pSysWindow->GetMenuBar();
            if ( pSetMenuBar == pTopMenuBar )
                pSysWindow->SetMenuBar( 0 );
        }
    }

    // reset inplace menubar manager
    m_pInplaceMenuBar = 0;
    if ( m_xInplaceMenuBar.is() )
    {
        m_xInplaceMenuBar->dispose();
        m_xInplaceMenuBar.clear();
    }

    Reference< XComponent > xComp( m_xMenuBar, UNO_QUERY );
    if ( xComp.is() )
        xComp->dispose();
    m_xMenuBar.clear();
    implts_unlock();
}

void LayoutManager::implts_lock()
{
    WriteGuard aWriteLock( m_aLock );
    ++m_nLockCount;
}

sal_Bool LayoutManager::implts_unlock()
{
    WriteGuard aWriteLock( m_aLock );
    m_nLockCount = std::max( m_nLockCount-1, static_cast<sal_Int32>(0) );
    return ( m_nLockCount == 0 );
}

void LayoutManager::implts_reset( sal_Bool bAttached )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< XUIConfiguration > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
    Reference< XUIConfiguration > xDocCfgMgr( m_xDocCfgMgr, UNO_QUERY );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    Reference< XComponentContext > xContext( m_xContext );
    Reference< XNameAccess > xPersistentWindowStateSupplier( m_xPersistentWindowStateSupplier );
    ToolbarLayoutManager* pToolbarManager( m_pToolbarManager );
    OUString aModuleIdentifier( m_aModuleIdentifier );
    bool bAutomaticToolbars( m_bAutomaticToolbars );
    aReadLock.unlock();
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
                    xModuleCfgSupplier = ModuleUIConfigurationManagerSupplier::create( xContext );

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
                    xModuleCfgMgr = Reference< XUIConfiguration >( xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier ), UNO_QUERY );
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
                        xDocCfgMgr = Reference< XUIConfiguration >( xUIConfigurationManagerSupplier->getUIConfigurationManager(), UNO_QUERY );
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
            aModuleIdentifier = OUString();
        }

        Reference< XUIConfigurationManager > xModCfgMgr( xModuleCfgMgr, UNO_QUERY );
        Reference< XUIConfigurationManager > xDokCfgMgr( xDocCfgMgr, UNO_QUERY );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_xModel = xModel;
        m_aDockingArea = awt::Rectangle();
        m_bComponentAttached = bAttached;
        m_aModuleIdentifier = aModuleIdentifier;
        m_xModuleCfgMgr = xModCfgMgr;
        m_xDocCfgMgr = xDokCfgMgr;
        m_xPersistentWindowState = xPersistentWindowState;
        m_aStatusBarElement.m_bStateRead = sal_False; // reset state to read data again!
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        // reset/notify toolbar layout manager
        if ( pToolbarManager )
        {
            if ( bAttached )
            {
                pToolbarManager->attach( xFrame, xModCfgMgr, xDokCfgMgr, xPersistentWindowState );
                uno::Reference< awt::XWindowPeer > xParent( xContainerWindow, UNO_QUERY );
                pToolbarManager->setParentWindow( xParent );
                if ( bAutomaticToolbars )
                    pToolbarManager->createStaticToolbars();
            }
            else
            {
                pToolbarManager->reset();
                implts_destroyElements();
            }
        }
    }

    implts_unlock();
}

sal_Bool LayoutManager::implts_isEmbeddedLayoutManager() const
{
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    aReadLock.unlock();

    Reference< awt::XWindow > xFrameContainerWindow = xFrame->getContainerWindow();
    if ( xFrameContainerWindow == xContainerWindow )
        return sal_False;
    else
        return sal_True;
}

void LayoutManager::implts_destroyElements()
{
    WriteGuard aWriteLock( m_aLock );
    ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
    aWriteLock.unlock();

    if ( pToolbarManager )
        pToolbarManager->destroyToolbars();

    implts_destroyStatusBar();

    aWriteLock.lock();
    impl_clearUpMenuBar();
    aWriteLock.unlock();
}

void LayoutManager::implts_toggleFloatingUIElementsVisibility( sal_Bool bActive )
{
    ReadGuard aReadLock( m_aLock );
    ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
    aReadLock.unlock();

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

sal_Bool LayoutManager::implts_readWindowStateData( const OUString& aName, UIElement& rElementData )
{
    return readWindowStateData( aName, rElementData, m_aLock, m_xPersistentWindowState,
            m_pGlobalSettings, m_bGlobalSettings, m_xContext );
}

sal_Bool LayoutManager::readWindowStateData( const OUString& aName, UIElement& rElementData,
        LockHelper &rLock, const Reference< XNameAccess > &rPersistentWindowState,
        GlobalSettings* &rGlobalSettings, bool &bInGlobalSettings,
        const Reference< XComponentContext > &rComponentContext )
{
    bool bGetSettingsState( false );

    WriteGuard aWriteLock( rLock );
    Reference< XNameAccess > xPersistentWindowState( rPersistentWindowState );
    aWriteLock.unlock();

    if ( xPersistentWindowState.is() )
    {
        aWriteLock.lock();
        bool bGlobalSettings( bInGlobalSettings );
        GlobalSettings* pGlobalSettings( 0 );
        if ( rGlobalSettings == 0 )
        {
            rGlobalSettings = new GlobalSettings( rComponentContext );
            bGetSettingsState = true;
        }
        pGlobalSettings = rGlobalSettings;
        aWriteLock.unlock();

        try
        {
            Sequence< PropertyValue > aWindowState;
            if ( xPersistentWindowState->hasByName( aName ) && (xPersistentWindowState->getByName( aName ) >>= aWindowState) )
            {
                sal_Bool bValue( sal_False );
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
                            rElementData.m_aDockedData.m_nDockedArea = sal_Int16( eDockingArea );
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_DOCKPOS )
                    {
                        awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                            rElementData.m_aDockedData.m_aPos = aPoint;
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
                            rElementData.m_nStyle = sal_Int16( nStyle );
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
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_CONTEXTACTIVE )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bContextActive = bValue;
                    }
                    else if ( aWindowState[n].Name == WINDOWSTATE_PROPERTY_SOFTCLOSE )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bSoftClose = bValue;
                    }
                }
            }

            // oversteer values with global settings
            if ( pGlobalSettings && ( bGetSettingsState || bGlobalSettings ))
            {
                if ( pGlobalSettings->HasStatesInfo( GlobalSettings::UIELEMENT_TYPE_TOOLBAR ))
                {
                    WriteGuard aWriteLock2( rLock );
                    bInGlobalSettings = true;
                    aWriteLock2.unlock();

                    uno::Any aValue;
                    sal_Bool bValue = sal_Bool();
                    if ( pGlobalSettings->GetStateInfo( GlobalSettings::UIELEMENT_TYPE_TOOLBAR,
                                                        GlobalSettings::STATEINFO_LOCKED,
                                                        aValue ))
                        aValue >>= rElementData.m_aDockedData.m_bLocked;
                    if ( pGlobalSettings->GetStateInfo( GlobalSettings::UIELEMENT_TYPE_TOOLBAR,
                                                        GlobalSettings::STATEINFO_DOCKED,
                                                        aValue ))
                    {
                        if ( aValue >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                }
            }

            return sal_True;
        }
        catch (const NoSuchElementException&)
        {
        }
    }

    return sal_False;
}

void LayoutManager::implts_writeWindowStateData( const OUString& aName, const UIElement& rElementData )
{
    WriteGuard aWriteLock( m_aLock );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );

    // set flag to determine that we triggered the notification
    m_bStoreWindowState = sal_True;
    aWriteLock.unlock();

    sal_Bool bPersistent( sal_False );
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
            bPersistent = sal_True;
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
            aWindowState[0].Value = makeAny( sal_Bool( !rElementData.m_bFloating ));
            aWindowState[1].Name  = WINDOWSTATE_PROPERTY_VISIBLE;
            aWindowState[1].Value = makeAny( sal_Bool( rElementData.m_bVisible ));

            aWindowState[2].Name  = WINDOWSTATE_PROPERTY_DOCKINGAREA;
            aWindowState[2].Value = makeAny( static_cast< DockingArea >( rElementData.m_aDockedData.m_nDockedArea ) );

            aWindowState[3].Name = WINDOWSTATE_PROPERTY_DOCKPOS;
            aWindowState[3].Value <<= rElementData.m_aDockedData.m_aPos;

            aWindowState[4].Name = WINDOWSTATE_PROPERTY_POS;
            aWindowState[4].Value <<= rElementData.m_aFloatingData.m_aPos;

            aWindowState[5].Name  = WINDOWSTATE_PROPERTY_SIZE;
            aWindowState[5].Value <<= rElementData.m_aFloatingData.m_aSize;
            aWindowState[6].Name  = WINDOWSTATE_PROPERTY_UINAME;
            aWindowState[6].Value = makeAny( rElementData.m_aUIName );
            aWindowState[7].Name  = WINDOWSTATE_PROPERTY_LOCKED;
            aWindowState[7].Value = makeAny( rElementData.m_aDockedData.m_bLocked );

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
    aWriteLock.lock();
    m_bStoreWindowState = sal_False;
    aWriteLock.unlock();
}

::Size LayoutManager::implts_getContainerWindowOutputSize()
{
    ::Size  aContainerWinSize;
    Window* pContainerWindow( 0 );

    // Retrieve output size from container Window
    SolarMutexGuard aGuard;
    pContainerWindow  = VCLUnoHelper::GetWindow( m_xContainerWindow );
    if ( pContainerWindow )
        aContainerWinSize = pContainerWindow->GetOutputSizePixel();

    return aContainerWinSize;
}

Reference< XUIElement > LayoutManager::implts_createElement( const OUString& aName )
{
    Reference< ui::XUIElement > xUIElement;

    ReadGuard   aReadLock( m_aLock );
    Sequence< PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = OUString( "Frame" );
    aPropSeq[0].Value <<= m_xFrame;
    aPropSeq[1].Name = OUString( "Persistent" );
    aPropSeq[1].Value <<= sal_True;

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

void LayoutManager::implts_setVisibleState( sal_Bool bShow )
{
    WriteGuard aWriteLock( m_aLock );
    m_aStatusBarElement.m_bMasterHide = !bShow;
    aWriteLock.unlock();

    implts_updateUIElementsVisibleState( bShow );
}

void LayoutManager::implts_updateUIElementsVisibleState( sal_Bool bSetVisible )
{
    // notify listeners
    uno::Any a;
    if ( bSetVisible )
        implts_notifyListeners( frame::LayoutManagerEvents::VISIBLE, a );
    else
        implts_notifyListeners( frame::LayoutManagerEvents::INVISIBLE, a );

    WriteGuard aWriteLock( m_aLock );
    Reference< XUIElement >   xMenuBar( m_xMenuBar, UNO_QUERY );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< XComponent >   xInplaceMenuBar( m_xInplaceMenuBar );
    MenuBarManager*           pInplaceMenuBar( m_pInplaceMenuBar );
    aWriteLock.unlock();

    bool bMustDoLayout(false);
    if (( xMenuBar.is() || xInplaceMenuBar.is() ) && xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        MenuBar* pMenuBar( 0 );
        if ( xInplaceMenuBar.is() )
            pMenuBar = (MenuBar *)pInplaceMenuBar->GetMenuBar();
        else
        {
            MenuBarWrapper* pMenuBarWrapper = (static_cast< MenuBarWrapper* >(xMenuBar.get()) );
            pMenuBar = (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar();
        }

        SystemWindow* pSysWindow = getTopSystemWindow( xContainerWindow );
        if ( pSysWindow )
        {
            if ( bSetVisible )
                pSysWindow->SetMenuBar( pMenuBar );
            else
                pSysWindow->SetMenuBar( 0 );
            bMustDoLayout = true;
        }
    }

    // Hide/show the statusbar according to bSetVisible
    if ( bSetVisible )
        bMustDoLayout = !implts_showStatusBar();
    else
        bMustDoLayout = !implts_hideStatusBar();

    aWriteLock.lock();
    ToolbarLayoutManager* pToolbarManager( m_pToolbarManager );
    aWriteLock.unlock();

    if ( pToolbarManager )
    {
        pToolbarManager->setVisible( bSetVisible );
        bMustDoLayout = pToolbarManager->isLayoutDirty();
    }

    if ( bMustDoLayout )
        implts_doLayout_notify( sal_False );
}

void LayoutManager::implts_setCurrentUIVisibility( sal_Bool bShow )
{
    WriteGuard aWriteLock( m_aLock );
    if ( !bShow && m_aStatusBarElement.m_bVisible && m_aStatusBarElement.m_xUIElement.is() )
        m_aStatusBarElement.m_bMasterHide = true;
    else if ( bShow && m_aStatusBarElement.m_bVisible )
        m_aStatusBarElement.m_bMasterHide = false;
    aWriteLock.unlock();

    implts_updateUIElementsVisibleState( bShow );
}

void LayoutManager::implts_destroyStatusBar()
{
    Reference< XComponent > xCompStatusBar;

    WriteGuard aWriteLock( m_aLock );
    m_aStatusBarElement.m_aName = OUString();
    xCompStatusBar = Reference< XComponent >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    m_aStatusBarElement.m_xUIElement.clear();
    aWriteLock.unlock();

    if ( xCompStatusBar.is() )
        xCompStatusBar->dispose();

    implts_destroyProgressBar();
}

void LayoutManager::implts_createStatusBar( const OUString& aStatusBarName )
{
    WriteGuard aWriteLock( m_aLock );
    if ( !m_aStatusBarElement.m_xUIElement.is() )
    {
        implts_readStatusBarState( aStatusBarName );
        m_aStatusBarElement.m_aName      = aStatusBarName;
        m_aStatusBarElement.m_xUIElement = implts_createElement( aStatusBarName );
    }
    aWriteLock.unlock();

    implts_createProgressBar();
}

void LayoutManager::implts_readStatusBarState( const OUString& rStatusBarName )
{
    WriteGuard aWriteLock( m_aLock );
    if ( !m_aStatusBarElement.m_bStateRead )
    {
        // Read persistent data for status bar if not yet read!
        if ( implts_readWindowStateData( rStatusBarName, m_aStatusBarElement ))
            m_aStatusBarElement.m_bStateRead = sal_True;
    }
}

void LayoutManager::implts_createProgressBar()
{
    Reference< XUIElement > xStatusBar;
    Reference< XUIElement > xProgressBar;
    Reference< XUIElement > xProgressBarBackup;
    Reference< awt::XWindow > xContainerWindow;

    WriteGuard aWriteLock( m_aLock );
    xStatusBar = Reference< XUIElement >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    xProgressBarBackup = m_xProgressBarBackup;
    m_xProgressBarBackup.clear();
    xContainerWindow = m_xContainerWindow;
    aWriteLock.unlock();

    sal_Bool            bRecycled = xProgressBarBackup.is();
    ProgressBarWrapper* pWrapper  = 0;
    if ( bRecycled )
        pWrapper = (ProgressBarWrapper*)xProgressBarBackup.get();
    else if ( xProgressBar.is() )
        pWrapper = (ProgressBarWrapper*)xProgressBar.get();
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
        Window* pStatusBarWnd = VCLUnoHelper::GetWindow( xStatusBarWindow );
        if ( !pStatusBarWnd )
        {
            Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            if ( pWindow )
            {
                StatusBar* pStatusBar = new StatusBar( pWindow, WinBits( WB_LEFT | WB_3DLOOK ) );
                Reference< awt::XWindow > xStatusBarWindow2( VCLUnoHelper::GetInterface( pStatusBar ));
                pWrapper->setStatusBar( xStatusBarWindow2, sal_True );
            }
        }
    }

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    aWriteLock.lock();
    m_aProgressBarElement.m_xUIElement = Reference< XUIElement >(
        static_cast< cppu::OWeakObject* >( pWrapper ), UNO_QUERY );
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( bRecycled )
        implts_showProgressBar();
}

void LayoutManager::implts_backupProgressBarWrapper()
{
    // SAFE -> ----------------------------------
    WriteGuard aWriteLock(m_aLock);

    if (m_xProgressBarBackup.is())
        return;

    // safe a backup copy of the current progress!
    // This copy will be used automaticly inside createProgressBar() which is called
    // implictly from implts_doLayout() .-)
    m_xProgressBarBackup = m_aProgressBarElement.m_xUIElement;

    // remove the relation between this old progress bar and our old status bar.
    // Otherwhise we work on disposed items ...
    // The internal used ProgressBarWrapper can handle a NULL reference.
    if ( m_xProgressBarBackup.is() )
    {
        ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)m_xProgressBarBackup.get();
        if ( pWrapper )
            pWrapper->setStatusBar( Reference< awt::XWindow >(), sal_False );
    }

    // prevent us from dispose() the m_aProgressBarElement.m_xUIElement inside implts_reset()
    m_aProgressBarElement.m_xUIElement.clear();

    aWriteLock.unlock();
    // <- SAFE ----------------------------------
}

void LayoutManager::implts_destroyProgressBar()
{
    // dont remove the progressbar in general
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
    ReadGuard aReadLock( m_aLock );
    xStatusBar = Reference< XUIElement >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    xContainerWindow = m_xContainerWindow;

    Reference< awt::XWindow > xWindow;
    if ( xStatusBar.is() )
        xWindow = Reference< awt::XWindow >( xStatusBar->getRealInterface(), UNO_QUERY );
    else if ( xProgressBar.is() )
    {
        ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
    }
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xWindow.is() )
    {
        SolarMutexGuard aGuard;
        Window* pParentWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pParentWindow && ( pWindow && pWindow->GetType() == WINDOW_STATUSBAR ))
        {
            Window* pOldParentWindow = pWindow->GetParent();
            if ( pParentWindow != pOldParentWindow )
                pWindow->SetParent( pParentWindow );
            ((StatusBar *)pWindow)->SetPosSizePixel( rPos, rSize );
        }
    }
}

sal_Bool LayoutManager::implts_showProgressBar()
{
    Reference< XUIElement > xStatusBar;
    Reference< XUIElement > xProgressBar;
    Reference< awt::XWindow > xWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    xStatusBar = Reference< XUIElement >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    sal_Bool bVisible( m_bVisible );

    m_aProgressBarElement.m_bVisible = sal_True;
    if ( bVisible )
    {
        if ( xStatusBar.is() && !m_aStatusBarElement.m_bMasterHide )
        {
            xWindow = Reference< awt::XWindow >( xStatusBar->getRealInterface(), UNO_QUERY );
        }
        else if ( xProgressBar.is() )
        {
            ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
            if ( pWrapper )
                xWindow = pWrapper->getStatusBar();
        }
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    SolarMutexGuard aGuard;
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( pWindow )
    {
        if ( !pWindow->IsVisible() )
        {
            implts_setOffset( pWindow->GetSizePixel().Height() );
            pWindow->Show();
            implts_doLayout_notify( sal_False );
        }
        return sal_True;
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_hideProgressBar()
{
    Reference< XUIElement > xProgressBar;
    Reference< awt::XWindow > xWindow;
    sal_Bool bHideStatusBar( sal_False );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );

    sal_Bool bInternalStatusBar( sal_False );
    if ( xProgressBar.is() )
    {
        Reference< awt::XWindow > xStatusBar;
        ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
        Reference< ui::XUIElement > xStatusBarElement = m_aStatusBarElement.m_xUIElement;
        if ( xStatusBarElement.is() )
            xStatusBar = Reference< awt::XWindow >( xStatusBarElement->getRealInterface(), UNO_QUERY );
        bInternalStatusBar = xStatusBar != xWindow;
    }
    m_aProgressBarElement.m_bVisible = sal_False;
    implts_readStatusBarState( STATUS_BAR_ALIAS );
    bHideStatusBar = !m_aStatusBarElement.m_bVisible;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    SolarMutexGuard aGuard;
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( pWindow && pWindow->IsVisible() && ( bHideStatusBar || bInternalStatusBar ))
    {
        implts_setOffset( 0 );
        pWindow->Hide();
        implts_doLayout_notify( sal_False );
        return sal_True;
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_showStatusBar( sal_Bool bStoreState )
{
    WriteGuard aWriteLock( m_aLock );
    Reference< ui::XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    if ( bStoreState )
        m_aStatusBarElement.m_bVisible = sal_True;
    aWriteLock.unlock();

    if ( xStatusBar.is() )
    {
        Reference< awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );

        SolarMutexGuard aGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && !pWindow->IsVisible() )
        {
            implts_setOffset( pWindow->GetSizePixel().Height() );
            pWindow->Show();
            implts_doLayout_notify( sal_False );
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_hideStatusBar( sal_Bool bStoreState )
{
    WriteGuard aWriteLock( m_aLock );
    Reference< ui::XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    if ( bStoreState )
        m_aStatusBarElement.m_bVisible = sal_False;
    aWriteLock.unlock();

    if ( xStatusBar.is() )
    {
        Reference< awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );

        SolarMutexGuard aGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->IsVisible() )
        {
            implts_setOffset( 0 );
            pWindow->Hide();
            implts_doLayout_notify( sal_False );
            return sal_True;
        }
    }

    return sal_False;
}

void LayoutManager::implts_setOffset( const sal_Int32 nBottomOffset )
{
    ::Rectangle aOffsetRect;
    setZeroRectangle( aOffsetRect );
    aOffsetRect.setHeight( nBottomOffset );

    if ( m_pToolbarManager )
        m_pToolbarManager->setDockingAreaOffsets( aOffsetRect );
}

void LayoutManager::implts_setInplaceMenuBar( const Reference< XIndexAccess >& xMergedMenuBar )
throw (uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( !m_bInplaceMenuSet )
    {
        SolarMutexGuard aGuard;

        // Reset old inplace menubar!
        m_pInplaceMenuBar = 0;
        if ( m_xInplaceMenuBar.is() )
            m_xInplaceMenuBar->dispose();
        m_xInplaceMenuBar.clear();
        m_bInplaceMenuSet = sal_False;

        if ( m_xFrame.is() && m_xContainerWindow.is() )
        {
            OUString aModuleIdentifier;
            Reference< XDispatchProvider > xDispatchProvider;

            MenuBar* pMenuBar = new MenuBar;
            m_pInplaceMenuBar = new MenuBarManager( m_xContext, m_xFrame, m_xURLTransformer, xDispatchProvider, aModuleIdentifier, pMenuBar, sal_True, sal_True );
            m_pInplaceMenuBar->SetItemContainer( xMergedMenuBar );

            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
                            pSysWindow->SetMenuBar( pMenuBar );

                m_bInplaceMenuSet = sal_True;
            m_xInplaceMenuBar = Reference< XComponent >( (OWeakObject *)m_pInplaceMenuBar, UNO_QUERY );
        }

        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_updateMenuBarClose();
    }
}

void LayoutManager::implts_resetInplaceMenuBar()
throw (uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_bInplaceMenuSet = sal_False;

    if ( m_xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;
        MenuBarWrapper* pMenuBarWrapper = (static_cast< MenuBarWrapper* >(m_xMenuBar.get()) );
        SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
        if ( pSysWindow )
        {
            if ( pMenuBarWrapper )
                                pSysWindow->SetMenuBar( (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar() );
                        else
                                pSysWindow->SetMenuBar( 0 );
        }
    }

    // Remove inplace menu bar
    m_pInplaceMenuBar = 0;
    if ( m_xInplaceMenuBar.is() )
        m_xInplaceMenuBar->dispose();
    m_xInplaceMenuBar.clear();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::attachFrame( const Reference< XFrame >& xFrame )
throw (uno::RuntimeException)
{
    WriteGuard aWriteLock( m_aLock );
    m_xFrame = xFrame;
}

void SAL_CALL LayoutManager::reset()
throw (RuntimeException)
{
    implts_reset( sal_True );
}

//---------------------------------------------------------------------------------------------------------
// XMenuBarMergingAcceptor
//---------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL LayoutManager::setMergedMenuBar(
    const Reference< XIndexAccess >& xMergedMenuBar )
throw (uno::RuntimeException)
{
    implts_setInplaceMenuBar( xMergedMenuBar );

    uno::Any a;
    implts_notifyListeners( frame::LayoutManagerEvents::MERGEDMENUBAR, a );
    return sal_True;
}

void SAL_CALL LayoutManager::removeMergedMenuBar()
throw (uno::RuntimeException)
{
    implts_resetInplaceMenuBar();
}

awt::Rectangle SAL_CALL LayoutManager::getCurrentDockingArea()
throw ( RuntimeException )
{
    ReadGuard aReadLock( m_aLock );
    return m_aDockingArea;
}

Reference< XDockingAreaAcceptor > SAL_CALL LayoutManager::getDockingAreaAcceptor()
throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    return m_xDockingAreaAcceptor;
}

void SAL_CALL LayoutManager::setDockingAreaAcceptor( const Reference< ui::XDockingAreaAcceptor >& xDockingAreaAcceptor )
throw ( RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if (( m_xDockingAreaAcceptor == xDockingAreaAcceptor ) || !m_xFrame.is() )
        return;

    // IMPORTANT: Be sure to stop layout timer if don't have a docking area acceptor!
    if ( !xDockingAreaAcceptor.is() )
        m_aAsyncLayoutTimer.Stop();

    sal_Bool bAutomaticToolbars( m_bAutomaticToolbars );
    std::vector< Reference< awt::XWindow > > oldDockingAreaWindows;

    ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;

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

        Window* pContainerWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pContainerWindow )
            pContainerWindow->RemoveChildEventListener( LINK( this, LayoutManager, WindowEventListener ) );
    }

    Reference< ui::XDockingAreaAcceptor > xOldDockingAreaAcceptor( m_xDockingAreaAcceptor );
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
            Window* pContainerWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
            if( pContainerWindow )
                m_bParentWindowVisible = pContainerWindow->IsVisible();
        }

        uno::Reference< awt::XWindowPeer > xParent( m_xContainerWindow, UNO_QUERY );
    }

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xDockingAreaAcceptor.is() )
    {
        SolarMutexGuard aGuard;

        // Add layout manager as listener to get notifications about toolbar button activties
        Window* pContainerWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        if ( pContainerWindow )
            pContainerWindow->AddChildEventListener( LINK( this, LayoutManager, WindowEventListener ) );

        // We have now a new container window, reparent all child windows!
        implts_reparentChildWindows();
    }
    else
        implts_destroyElements(); // remove all elements

    if ( !oldDockingAreaWindows.empty() )
    {
        // Reset docking area size for our old docking area acceptor
        awt::Rectangle aEmptyRect;
        xOldDockingAreaAcceptor->setDockingAreaSpace( aEmptyRect );
    }

    if ( pToolbarManager && xDockingAreaAcceptor.is() )
    {
        if ( bAutomaticToolbars )
        {
            lock();
            pToolbarManager->createStaticToolbars();
            unlock();
        }
        implts_doLayout( sal_True, sal_False );
    }
}

void LayoutManager::implts_reparentChildWindows()
{
    WriteGuard aWriteLock( m_aLock );
    UIElement aStatusBarElement = m_aStatusBarElement;
    uno::Reference< awt::XWindow > xContainerWindow  = m_xContainerWindow;
    aWriteLock.unlock();

    uno::Reference< awt::XWindow > xStatusBarWindow;
    if ( aStatusBarElement.m_xUIElement.is() )
    {
        try
        {
            xStatusBarWindow = Reference< awt::XWindow >( aStatusBarElement.m_xUIElement->getRealInterface(), UNO_QUERY );
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
        Window* pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        Window* pWindow          = VCLUnoHelper::GetWindow( xStatusBarWindow );
        if ( pWindow && pContainerWindow )
            pWindow->SetParent( pContainerWindow );
    }

    implts_resetMenuBar();

    aWriteLock.lock();
    ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
    if ( pToolbarManager )
        pToolbarManager->setParentWindow( uno::Reference< awt::XWindowPeer >( xContainerWindow, uno::UNO_QUERY ));
    aWriteLock.unlock();
}

uno::Reference< ui::XUIElement > LayoutManager::implts_createDockingWindow( const OUString& aElementName )
{
    Reference< XUIElement > xUIElement = implts_createElement( aElementName );
    return xUIElement;
}

IMPL_LINK( LayoutManager, WindowEventListener, VclSimpleEvent*, pEvent )
{
    long nResult( 1 );

    if ( pEvent && pEvent->ISA( VclWindowEvent ))
    {
        Window* pWindow = static_cast< VclWindowEvent* >(pEvent)->GetWindow();
        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
        {
            ReadGuard aReadLock( m_aLock );
            ToolbarLayoutManager* pToolbarManager( m_pToolbarManager );
            aReadLock.unlock();

            if ( pToolbarManager )
                nResult = pToolbarManager->childWindowEvent( pEvent );
        }
    }

    return nResult;
}

void SAL_CALL LayoutManager::createElement( const OUString& aName )
throw (RuntimeException)
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::createElement" );

    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< XURLTransformer > xURLTransformer = m_xURLTransformer;
    sal_Bool    bInPlaceMenu = m_bInplaceMenuSet;
    aReadLock.unlock();

    if ( !xFrame.is() )
        return;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    bool bMustBeLayouted( false );
    bool bNotify( false );

    bool bPreviewFrame;
    if (m_pToolbarManager)
        // Assumes that we created the ToolbarLayoutManager with our frame, if
        // not then we're somewhat fouled up ...
        bPreviewFrame = m_pToolbarManager->isPreviewFrame();
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

        if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ) && m_pToolbarManager != NULL )
        {
            bNotify         = m_pToolbarManager->createToolbar( aName );
            bMustBeLayouted = m_pToolbarManager->isLayoutDirty();
        }
        else if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
                  aElementName.equalsIgnoreAsciiCase("menubar") )
        {
            // #i38743# don't create a menubar if frame isn't top
            if ( !bInPlaceMenu && !m_xMenuBar.is() && implts_isFrameOrWindowTop( xFrame ))
                {
                m_xMenuBar = implts_createElement( aName );
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
                                MenuBar* pMenuBar = (MenuBar*)pAwtMenuBar->GetMenu();
                                if ( pMenuBar )
                                {
                                    pSysWindow->SetMenuBar( pMenuBar );
                                    pMenuBar->SetDisplayable( m_bMenuVisible );
                                    if ( m_bMenuVisible )
                                        bNotify = sal_True;
                                    implts_updateMenuBarClose();
                                }
                            }
                        }
                    }
                }
            }
            aWriteLock.unlock();
        }
        else if ( aElementType.equalsIgnoreAsciiCase("statusbar") &&
                  ( implts_isFrameOrWindowTop(xFrame) || implts_isEmbeddedLayoutManager() ))
        {
            implts_createStatusBar( aName );
            bNotify = sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
                  aElementName.equalsIgnoreAsciiCase("progressbar") &&
                  implts_isFrameOrWindowTop(xFrame) )
        {
            implts_createProgressBar();
            bNotify = sal_True;
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
        implts_doLayout_notify( sal_True );

    if ( bNotify )
    {
        // UI element is invisible - provide information to listeners
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( aName ) );
    }
}

void SAL_CALL LayoutManager::destroyElement( const OUString& aName )
throw (RuntimeException)
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::destroyElement" );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    bool            bMustBeLayouted( sal_False );
    bool            bMustBeDestroyed( sal_False );
    bool            bNotify( sal_False );
    OUString aElementType;
    OUString aElementName;

    Reference< XComponent > xComponent;
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
        aWriteLock.unlock();
        implts_destroyStatusBar();
        bMustBeLayouted = true;
        bNotify         = true;
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        aWriteLock.unlock();
        implts_createProgressBar();
        bMustBeLayouted = true;
        bNotify = sal_True;
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ) && m_pToolbarManager != NULL )
    {
        aWriteLock.unlock();
        bNotify         = m_pToolbarManager->destroyToolbar( aName );
        bMustBeLayouted = m_pToolbarManager->isLayoutDirty();
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        uno::Reference< XComponentContext > xContext( m_xContext );
        aWriteLock.unlock();

        impl_setDockingWindowVisibility( xContext, xFrame, aElementName, false );
        bMustBeLayouted = false;
        bNotify         = false;
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( bMustBeDestroyed )
    {
        if ( xComponent.is() )
            xComponent->dispose();
        bNotify = true;
    }

    if ( bMustBeLayouted )
        doLayout();

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_INVISIBLE, uno::makeAny( aName ) );
}

::sal_Bool SAL_CALL LayoutManager::requestElement( const OUString& rResourceURL )
throw (uno::RuntimeException)
{
    bool            bResult( false );
    bool            bNotify( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( rResourceURL, aElementType, aElementName );

    WriteGuard aWriteLock( m_aLock );

    OString aResName = OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk", "framework (cd100003) Element " << aResName.getStr() << " requested." );

    if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
          aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
        ( m_aStatusBarElement.m_aName == rResourceURL ))
    {
        implts_readStatusBarState( rResourceURL );
        if ( m_aStatusBarElement.m_bVisible && !m_aStatusBarElement.m_bMasterHide )
        {
            aWriteLock.unlock();
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
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow )
                {
                    pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                    bResult   = true;
                    bNotify   = true;
                }
            }
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        aWriteLock.unlock();
        implts_showProgressBar();
        bResult   = true;
        bNotify   = true;
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ) && m_bVisible )
    {
        bool bComponentAttached( !m_aModuleIdentifier.isEmpty() );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aWriteLock.unlock();

        if ( pToolbarManager && bComponentAttached )
        {
                bNotify   = pToolbarManager->requestToolbar( rResourceURL );
            }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        aWriteLock.unlock();

        CreateDockingWindow( xFrame, aElementName );
    }

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( rResourceURL ) );

    return bResult;
}

Reference< XUIElement > SAL_CALL LayoutManager::getElement( const OUString& aName )
throw (RuntimeException)
{
    Reference< XUIElement > xUIElement = implts_findElement( aName );
    if ( !xUIElement.is() )
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager*             pToolbarManager( m_pToolbarManager );
        aReadLock.unlock();

        if ( pToolbarManager )
            xUIElement = pToolbarManager->getToolbar( aName );
    }

    return xUIElement;
}

Sequence< Reference< ui::XUIElement > > SAL_CALL LayoutManager::getElements()
throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< ui::XUIElement >  xMenuBar( m_xMenuBar );
    uno::Reference< ui::XUIElement >  xStatusBar( m_aStatusBarElement.m_xUIElement );
    ToolbarLayoutManager*             pToolbarManager( m_pToolbarManager );
    aReadLock.unlock();

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
throw (RuntimeException)
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::showElement" );

    bool            bResult( false );
    bool            bNotify( false );
    bool            bMustLayout( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );

    OString aResName = OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk", "framework (cd100003) Element " << aResName.getStr() );

    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        WriteGuard aWriteLock( m_aLock );
        m_bMenuVisible = sal_True;
        aWriteLock.unlock();

        bResult = implts_resetMenuBar();
        bNotify = bResult;
    }
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
    {
        WriteGuard aWriteLock( m_aLock );
        if ( m_aStatusBarElement.m_xUIElement.is() && !m_aStatusBarElement.m_bMasterHide &&
             implts_showStatusBar( sal_True ))
        {
            aWriteLock.unlock();

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
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            bNotify     = pToolbarManager->showToolbar( aName );
            bMustLayout = pToolbarManager->isLayoutDirty();
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        ReadGuard aReadGuard( m_aLock );
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        uno::Reference< XComponentContext > xContext( m_xContext );
        aReadGuard.unlock();

        impl_setDockingWindowVisibility( xContext, xFrame, aElementName, true );
    }
    else if ( aElementType.equalsIgnoreAsciiCase("toolpanel"))
    {
        ReadGuard aReadGuard( m_aLock );
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        aReadGuard.unlock();
        ActivateToolPanel( m_xFrame, aName );
    }

    if ( bMustLayout )
        doLayout();

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( aName ) );

    return bResult;
}

sal_Bool SAL_CALL LayoutManager::hideElement( const OUString& aName )
throw (RuntimeException)
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::hideElement" );

    bool            bNotify( false );
    bool            bMustLayout( false );
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    OString aResName = OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
    SAL_INFO( "fwk", "framework (cd100003) Element " << aResName.getStr() );

    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        WriteGuard aWriteLock( m_aLock );

        if ( m_xContainerWindow.is() )
        {
            m_bMenuVisible = sal_False;

            SolarMutexGuard aGuard;
            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
            {
                MenuBar* pMenuBar = pSysWindow->GetMenuBar();
                if ( pMenuBar )
                {
                    pMenuBar->SetDisplayable( sal_False );
                    bNotify = true;
                }
            }
        }
    }
    else if (( aElementType.equalsIgnoreAsciiCase("statusbar") &&
               aElementName.equalsIgnoreAsciiCase("statusbar") ) ||
             ( m_aStatusBarElement.m_aName == aName ))
    {
        WriteGuard aWriteLock( m_aLock );
        if ( m_aStatusBarElement.m_xUIElement.is() && !m_aStatusBarElement.m_bMasterHide &&
             implts_hideStatusBar( sal_True ))
        {
            implts_writeWindowStateData( STATUS_BAR_ALIAS, m_aStatusBarElement );
            bMustLayout = sal_True;
            bNotify     = sal_True;
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("progressbar") &&
              aElementName.equalsIgnoreAsciiCase("progressbar") )
    {
        bNotify = implts_hideProgressBar();
    }
    else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            bNotify     = pToolbarManager->hideToolbar( aName );
            bMustLayout = pToolbarManager->isLayoutDirty();
        }
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        ReadGuard aReadGuard( m_aLock );
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        uno::Reference< XComponentContext > xContext( m_xContext );
        aReadGuard.unlock();

        impl_setDockingWindowVisibility( xContext, xFrame, aElementName, false );
    }

    if ( bMustLayout )
        doLayout();

    if ( bNotify )
        implts_notifyListeners( frame::LayoutManagerEvents::UIELEMENT_INVISIBLE, uno::makeAny( aName ) );

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::dockWindow( const OUString& aName, DockingArea DockingArea, const awt::Point& Pos )
throw (RuntimeException)
{
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager*             pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            pToolbarManager->dockToolbar( aName, DockingArea, Pos );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::dockAllWindows( ::sal_Int16 /*nElementType*/ ) throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    bool bResult( false );
    ToolbarLayoutManager*             pToolbarManager = m_pToolbarManager;
    aReadLock.unlock();

    if ( pToolbarManager )
    {
        bResult = pToolbarManager->dockAllToolbars();
        if ( pToolbarManager->isLayoutDirty() )
            doLayout();
    }
    return bResult;
}

sal_Bool SAL_CALL LayoutManager::floatWindow( const OUString& aName )
throw (RuntimeException)
{
    bool bResult( false );
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager*             pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            bResult = pToolbarManager->floatToolbar( aName );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return bResult;
}

::sal_Bool SAL_CALL LayoutManager::lockWindow( const OUString& aName )
throw (uno::RuntimeException)
{
    bool bResult( false );
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager*             pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            bResult = pToolbarManager->lockToolbar( aName );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
    return bResult;
}

::sal_Bool SAL_CALL LayoutManager::unlockWindow( const OUString& aName )
throw (uno::RuntimeException)
{
    bool bResult( false );
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager*             pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

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
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager*             pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            pToolbarManager->setToolbarSize( aName, aSize );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
}

void SAL_CALL LayoutManager::setElementPos( const OUString& aName, const awt::Point& aPos )
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager( m_pToolbarManager );
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            pToolbarManager->setToolbarPos( aName, aPos );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
}

void SAL_CALL LayoutManager::setElementPosSize( const OUString& aName, const awt::Point& aPos, const awt::Size& aSize )
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager( m_pToolbarManager );
        aReadLock.unlock();

        if ( pToolbarManager )
        {
            pToolbarManager->setToolbarPosSize( aName, aPos, aSize );
            if ( pToolbarManager->isLayoutDirty() )
                doLayout();
        }
    }
}

sal_Bool SAL_CALL LayoutManager::isElementVisible( const OUString& aName )
throw (RuntimeException)
{
    OUString aElementType;
    OUString aElementName;

    parseResourceURL( aName, aElementType, aElementName );
    if ( aElementType.equalsIgnoreAsciiCase("menubar") &&
         aElementName.equalsIgnoreAsciiCase("menubar") )
    {
        ReadGuard aReadLock( m_aLock );
        if ( m_xContainerWindow.is() )
        {
            aReadLock.unlock();

            SolarMutexGuard aGuard;
            SystemWindow* pSysWindow = getTopSystemWindow( m_xContainerWindow );
            if ( pSysWindow )
            {
                MenuBar* pMenuBar = pSysWindow->GetMenuBar();
                if ( pMenuBar && pMenuBar->IsDisplayable() )
                    return sal_True;
            }
            else
            {
                aReadLock.lock();
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
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->IsVisible() )
                    return sal_True;
                else
                    return sal_False;
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
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarVisible( aName );
    }
    else if ( aElementType.equalsIgnoreAsciiCase("dockingwindow"))
    {
        ReadGuard aReadGuard( m_aLock );
        uno::Reference< frame::XFrame > xFrame( m_xFrame );
        aReadGuard.unlock();

        return IsDockingWindowVisible( xFrame, aElementName );
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementFloating( const OUString& aName )
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarFloating( aName );
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementDocked( const OUString& aName )
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarDocked( aName );
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::isElementLocked( const OUString& aName )
throw (uno::RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
            return pToolbarManager->isToolbarLocked( aName );
    }

    return sal_False;
}

awt::Size SAL_CALL LayoutManager::getElementSize( const OUString& aName )
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
            return pToolbarManager->getToolbarSize( aName );
    }

    return awt::Size();
}

awt::Point SAL_CALL LayoutManager::getElementPos( const OUString& aName )
throw (RuntimeException)
{
    if ( getElementTypeFromResourceURL( aName ).equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
    {
        ReadGuard aReadLock( m_aLock );
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        aReadLock.unlock();

        if ( pToolbarManager )
            return pToolbarManager->getToolbarPos( aName );
    }

    return awt::Point();
}

void SAL_CALL LayoutManager::lock()
throw (RuntimeException)
{
    implts_lock();

    ReadGuard aReadLock( m_aLock );
    sal_Int32 nLockCount( m_nLockCount );
    aReadLock.unlock();

    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::lock lockCount=" << nLockCount );
#ifdef DBG_UTIL
    OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("LayoutManager::lock "));
    aStr.append(reinterpret_cast<sal_Int64>(this));
    aStr.append(RTL_CONSTASCII_STRINGPARAM(" - "));
    aStr.append(nLockCount);
    SAL_INFO( "fwk", "" << aStr.getStr());
#endif

    Any a( nLockCount );
    implts_notifyListeners( frame::LayoutManagerEvents::LOCK, a );
}

void SAL_CALL LayoutManager::unlock()
throw (RuntimeException)
{
    sal_Bool bDoLayout( implts_unlock() );

    ReadGuard aReadLock( m_aLock );
    sal_Int32 nLockCount( m_nLockCount );
    aReadLock.unlock();

    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::unlock lockCount=" << nLockCount );
#ifdef DBG_UTIL
    OStringBuffer aStr(RTL_CONSTASCII_STRINGPARAM("LayoutManager::unlock "));
    aStr.append(reinterpret_cast<sal_Int64>(this));
    aStr.append(RTL_CONSTASCII_STRINGPARAM(" - "));
    aStr.append(nLockCount);
    SAL_INFO( "fwk", "" << aStr.getStr());
#endif
    // conform to documentation: unlock with lock count == 0 means force a layout

    WriteGuard aWriteLock( m_aLock );
        if ( bDoLayout )
                m_aAsyncLayoutTimer.Stop();
        aWriteLock.unlock();

    Any a( nLockCount );
    implts_notifyListeners( frame::LayoutManagerEvents::UNLOCK, a );

    if ( bDoLayout )
        implts_doLayout_notify( sal_True );
}

void SAL_CALL LayoutManager::doLayout()
throw (RuntimeException)
{
    implts_doLayout_notify( sal_True );
}

//---------------------------------------------------------------------------------------------------------
//  ILayoutNotifications
//---------------------------------------------------------------------------------------------------------
void LayoutManager::requestLayout( Hint eHint )
{
    if ( eHint == HINT_TOOLBARSPACE_HAS_CHANGED )
        doLayout();
}

void LayoutManager::implts_doLayout_notify( sal_Bool bOuterResize )
{
    bool bLayouted = implts_doLayout( false, bOuterResize );
    if ( bLayouted )
        implts_notifyListeners( frame::LayoutManagerEvents::LAYOUT, Any() );
}

sal_Bool LayoutManager::implts_doLayout( sal_Bool bForceRequestBorderSpace, sal_Bool bOuterResize )
{
    SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::implts_doLayout" );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    if ( !m_xFrame.is() || !m_bParentWindowVisible )
        return sal_False;

    bool bPreserveContentSize( m_bPreserveContentSize );
    bool bMustDoLayout( m_bMustDoLayout );
    bool bNoLock = ( m_nLockCount == 0 );
    awt::Rectangle aCurrBorderSpace( m_aDockingArea );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< awt::XTopWindow2 > xContainerTopWindow( m_xContainerTopWindow );
    Reference< awt::XWindow > xComponentWindow( m_xFrame->getComponentWindow() );
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    sal_Bool bLayouted( sal_False );

    if ( bNoLock && xDockingAreaAcceptor.is() && xContainerWindow.is() && xComponentWindow.is() )
    {
        bLayouted = sal_True;

        WriteGuard aWriteGuard( m_aLock );
        m_bDoLayout = sal_True;
        aWriteGuard.unlock();

        awt::Rectangle aDockSpace( implts_calcDockingAreaSizes() );
        awt::Rectangle aBorderSpace( aDockSpace );
        sal_Bool       bGotRequestedBorderSpace( sal_True );

        // We have to add the height of a possible status bar
        aBorderSpace.Height += implts_getStatusBarSize().Height();

        if ( !equalRectangles( aBorderSpace, aCurrBorderSpace ) || bForceRequestBorderSpace || bMustDoLayout )
        {
            // we always resize the content window (instead of the complete container window) if we're not set up
            // to (attempt to) preserve the content window's size
            if ( bOuterResize && !bPreserveContentSize )
                bOuterResize = sal_False;

            // maximized windows can resized their content window only, not their container window
            if ( bOuterResize && xContainerTopWindow.is() && xContainerTopWindow->getIsMaximized() )
                bOuterResize = sal_False;

            // if the component window does not have a size (yet), then we can't use it to calc the container
            // window size
            awt::Rectangle aComponentRect = xComponentWindow->getPosSize();
            if ( bOuterResize && ( aComponentRect.Width == 0 ) && ( aComponentRect.Height == 0 ) )
                bOuterResize = sal_False;

            bGotRequestedBorderSpace = sal_False;
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
                aWriteGuard.lock();
                m_aDockingArea = aBorderSpace;
                m_bMustDoLayout = sal_False;
                aWriteGuard.unlock();
            }
        }

        if ( bGotRequestedBorderSpace )
        {
            ::Size      aContainerSize;
            ::Size      aStatusBarSize;

            // Interim solution to let the layout method within the
            // toolbar layout manager.
            implts_setOffset( implts_getStatusBarSize().Height() );
            if ( m_pToolbarManager )
                m_pToolbarManager->setDockingArea( aDockSpace );

            // Subtract status bar size from our container output size. Docking area windows
            // don't contain the status bar!
            aStatusBarSize = implts_getStatusBarSize();
            aContainerSize = implts_getContainerWindowOutputSize();
            aContainerSize.Height() -= aStatusBarSize.Height();

            if ( m_pToolbarManager )
                m_pToolbarManager->doLayout(aContainerSize);

            // Position the status bar
            if ( aStatusBarSize.Height() > 0 )
            {
                implts_setStatusBarPosSize( ::Point( 0, std::max(( aContainerSize.Height() ), long( 0 ))),
                                            ::Size( aContainerSize.Width(),aStatusBarSize.Height() ));
            }

            xDockingAreaAcceptor->setDockingAreaSpace( aBorderSpace );

            aWriteGuard.lock();
            m_bDoLayout = sal_False;
            aWriteGuard.unlock();
        }
    }

    return bLayouted;
}

sal_Bool LayoutManager::implts_resizeContainerWindow( const awt::Size& rContainerSize,
                                                      const awt::Point& rComponentPos )
{
    ReadGuard aReadLock( m_aLock );
    Reference< awt::XWindow >               xContainerWindow    = m_xContainerWindow;
    Reference< awt::XTopWindow2 >           xContainerTopWindow = m_xContainerTopWindow;
    Reference< awt::XWindow >               xComponentWindow    = m_xFrame->getComponentWindow();
    aReadLock.unlock();

    // calculate the maximum size we have for the container window
    sal_Int32 nDisplay = xContainerTopWindow->getDisplay();
    Rectangle aWorkArea = Application::GetScreenPosSizePixel( nDisplay );

    if (( aWorkArea.GetWidth() > 0 ) && ( aWorkArea.GetHeight() > 0 ))
    {
        if (( rContainerSize.Width > aWorkArea.GetWidth() ) || ( rContainerSize.Height > aWorkArea.GetHeight() ))
            return sal_False;
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
    return sal_True;
}

void SAL_CALL LayoutManager::setVisible( sal_Bool bVisible )
throw (uno::RuntimeException)
{
    WriteGuard aWriteLock( m_aLock );
    sal_Bool bWasVisible( m_bVisible );
    m_bVisible = bVisible;
    aWriteLock.unlock();

    if ( bWasVisible != bVisible )
        implts_setVisibleState( bVisible );
}

sal_Bool SAL_CALL LayoutManager::isVisible()
throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    return m_bVisible;
}

::Size LayoutManager::implts_getStatusBarSize()
{
    ReadGuard aReadLock( m_aLock );
    bool bStatusBarVisible( isElementVisible( STATUS_BAR_ALIAS ));
    bool bProgressBarVisible( isElementVisible( PROGRESS_BAR_ALIAS ));
    bool bVisible( m_bVisible );
    Reference< XUIElement > xStatusBar( m_aStatusBarElement.m_xUIElement );
    Reference< XUIElement > xProgressBar( m_aProgressBarElement.m_xUIElement );

    Reference< awt::XWindow > xWindow;
    if ( bStatusBarVisible && bVisible && xStatusBar.is() )
        xWindow = Reference< awt::XWindow >( xStatusBar->getRealInterface(), UNO_QUERY );
    else if ( xProgressBar.is() && !xStatusBar.is() && bProgressBarVisible )
    {
        ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
    }
    aReadLock.unlock();

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
    ReadGuard aReadLock( m_aLock );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    aReadLock.unlock();

    awt::Rectangle aBorderSpace;
    if ( m_pToolbarManager && xDockingAreaAcceptor.is() && xContainerWindow.is() )
        aBorderSpace = m_pToolbarManager->getDockingArea();

    return aBorderSpace;
}

void LayoutManager::implts_setDockingAreaWindowSizes( const awt::Rectangle& /*rBorderSpace*/ )
{
    ReadGuard aReadLock( m_aLock );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    aReadLock.unlock();

    uno::Reference< awt::XDevice > xDevice( xContainerWindow, uno::UNO_QUERY );
    // Convert relativ size to output size.
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

//---------------------------------------------------------------------------------------------------------
//      XMenuCloser
//---------------------------------------------------------------------------------------------------------
void LayoutManager::implts_updateMenuBarClose()
{
    WriteGuard aWriteLock( m_aLock );
    bool                      bShowCloser( m_bMenuBarCloser );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    aWriteLock.unlock();

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
                pMenuBar->ShowCloser( bShowCloser );
                pMenuBar->SetCloserHdl( LINK( this, LayoutManager, MenuBarClose ));
            }
        }
    }
}

sal_Bool LayoutManager::implts_resetMenuBar()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    sal_Bool bMenuVisible( m_bMenuVisible );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );

    MenuBar* pSetMenuBar = 0;
    if ( m_xInplaceMenuBar.is() )
        pSetMenuBar = (MenuBar *)m_pInplaceMenuBar->GetMenuBar();
    else
    {
        MenuBarWrapper* pMenuBarWrapper = static_cast< MenuBarWrapper* >( m_xMenuBar.get() );
        if ( pMenuBarWrapper )
            pSetMenuBar = (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar();
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    SolarMutexGuard aGuard;
    SystemWindow* pSysWindow = getTopSystemWindow( xContainerWindow );
    if ( pSysWindow && bMenuVisible && pSetMenuBar )
    {
        pSysWindow->SetMenuBar( pSetMenuBar );
        pSetMenuBar->SetDisplayable( sal_True );
        return sal_True;
    }

    return sal_False;
}

IMPL_LINK_NOARG(LayoutManager, MenuBarClose)
{
    ReadGuard aReadLock( m_aLock );
    uno::Reference< frame::XDispatchProvider >   xProvider(m_xFrame, uno::UNO_QUERY);
    uno::Reference< XComponentContext > xContext( m_xContext );
    aReadLock.unlock();

    if ( !xProvider.is())
        return 0;

    uno::Reference< frame::XDispatchHelper > xDispatcher = frame::DispatchHelper::create( xContext );

    xDispatcher->executeDispatch(
        xProvider,
        OUString(".uno:CloseWin"),
        OUString("_self"),
        0,
        uno::Sequence< beans::PropertyValue >());

    return 0;
}

IMPL_LINK_NOARG(LayoutManager, SettingsChanged)
{
    return 1;
}

//---------------------------------------------------------------------------------------------------------
//  XLayoutManagerEventBroadcaster
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::addLayoutManagerEventListener( const uno::Reference< frame::XLayoutManagerListener >& xListener )
throw (uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( (const uno::Reference< frame::XLayoutManagerListener >*)NULL ), xListener );
}

void SAL_CALL LayoutManager::removeLayoutManagerEventListener( const uno::Reference< frame::XLayoutManagerListener >& xListener )
throw (uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( (const uno::Reference< frame::XLayoutManagerListener >*)NULL ), xListener );
}

void LayoutManager::implts_notifyListeners( short nEvent, uno::Any aInfoParam )
{
    lang::EventObject                  aSource( static_cast< ::cppu::OWeakObject*>(this) );
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const uno::Reference< frame::XLayoutManagerListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((frame::XLayoutManagerListener*)pIterator.next())->layoutEvent( aSource, nEvent, aInfoParam );
            }
            catch( const uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------
//      XWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::windowResized( const awt::WindowEvent& aEvent )
throw( uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( !m_xDockingAreaAcceptor.is() )
        return;

    // Request to set docking area space again.
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    Reference< awt::XWindow >         xContainerWindow( m_xContainerWindow );

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source && m_bVisible )
    {
        // We have to call our resize handler at least once synchronously, as some
        // application modules need this. So we have to check if this is the first
        // call after the async layout time expired.
        m_bMustDoLayout = sal_True;
        if ( !m_aAsyncLayoutTimer.IsActive() )
        {
            const Link& aLink = m_aAsyncLayoutTimer.GetTimeoutHdl();
            if ( aLink.IsSet() )
                aLink.Call( &m_aAsyncLayoutTimer );
        }
        if ( m_nLockCount == 0 )
            m_aAsyncLayoutTimer.Start();
    }
    else if ( m_xFrame.is() && aEvent.Source == m_xFrame->getContainerWindow() )
    {
        // the container window of my DockingAreaAcceptor is not the same as of my frame
        // I still have to resize my frames' window as nobody else will do it
        Reference< awt::XWindow > xComponentWindow( m_xFrame->getComponentWindow() );
        if( xComponentWindow.is() == sal_True )
        {
            uno::Reference< awt::XDevice > xDevice( m_xFrame->getContainerWindow(), uno::UNO_QUERY );

            // Convert relativ size to output size.
            awt::Rectangle  aRectangle = m_xFrame->getContainerWindow()->getPosSize();
            awt::DeviceInfo aInfo      = xDevice->getInfo();
            awt::Size       aSize(  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                    aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            // Resize our component window.
            xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, awt::PosSize::POSSIZE );
        }
    }
}

void SAL_CALL LayoutManager::windowMoved( const awt::WindowEvent& ) throw( uno::RuntimeException )
{
}

void SAL_CALL LayoutManager::windowShown( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    ReadGuard aReadLock( m_aLock );
    Reference< awt::XWindow >  xContainerWindow( m_xContainerWindow );
    bool                       bParentWindowVisible( m_bParentWindowVisible );
    aReadLock.unlock();

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        WriteGuard aWriteLock( m_aLock );
        m_bParentWindowVisible = true;
        bool bSetVisible = ( m_bParentWindowVisible != bParentWindowVisible );
        aWriteLock.unlock();

        if ( bSetVisible )
            implts_updateUIElementsVisibleState( sal_True );
    }
}

void SAL_CALL LayoutManager::windowHidden( const lang::EventObject& aEvent ) throw( uno::RuntimeException )
{
    ReadGuard aReadLock( m_aLock );
    Reference< awt::XWindow > xContainerWindow( m_xContainerWindow );
    bool                      bParentWindowVisible( m_bParentWindowVisible );
    aReadLock.unlock();

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        WriteGuard aWriteLock( m_aLock );
        m_bParentWindowVisible = false;
        bool bSetInvisible = ( m_bParentWindowVisible != bParentWindowVisible );
        aWriteLock.unlock();

        if ( bSetInvisible )
            implts_updateUIElementsVisibleState( sal_False );
    }
}

IMPL_LINK_NOARG(LayoutManager, AsyncLayoutHdl)
{
    ReadGuard aReadLock( m_aLock );
    m_aAsyncLayoutTimer.Stop();

    if( !m_xContainerWindow.is() )
        return 0;

    awt::Rectangle aDockingArea( m_aDockingArea );
    ::Size         aStatusBarSize( implts_getStatusBarSize() );

    // Subtract status bar height
    aDockingArea.Height -= aStatusBarSize.Height();
    aReadLock.unlock();

    implts_setDockingAreaWindowSizes( aDockingArea );
    implts_doLayout( sal_True, sal_False );

    return 0;
}

//---------------------------------------------------------------------------------------------------------
//      XFrameActionListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::frameAction( const FrameActionEvent& aEvent )
throw ( RuntimeException )
{
    if (( aEvent.Action == FrameAction_COMPONENT_ATTACHED ) || ( aEvent.Action == FrameAction_COMPONENT_REATTACHED ))
    {
        SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::frameAction (COMPONENT_ATTACHED|REATTACHED)" );

        WriteGuard aWriteLock( m_aLock );
        m_bComponentAttached = sal_True;
        m_bMustDoLayout = sal_True;
        aWriteLock.unlock();

        implts_reset( sal_True );
        implts_doLayout( sal_True, sal_False );
        implts_doLayout( sal_True, sal_True );
    }
    else if (( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED ) || ( aEvent.Action == FrameAction_FRAME_UI_DEACTIVATING ))
    {
        SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::frameAction (FRAME_UI_ACTIVATED|DEACTIVATING)" );

        WriteGuard aWriteLock( m_aLock );
        m_bActive = ( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
        aWriteLock.unlock();

        implts_toggleFloatingUIElementsVisibility( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
    }
    else if ( aEvent.Action == FrameAction_COMPONENT_DETACHING )
    {
        SAL_INFO( "fwk", "framework (cd100003) ::LayoutManager::frameAction (COMPONENT_DETACHING)" );

        WriteGuard aWriteLock( m_aLock );
        m_bComponentAttached = sal_False;
        aWriteLock.unlock();

        implts_reset( sal_False );
    }
}

// ______________________________________________

void SAL_CALL LayoutManager::disposing( const lang::EventObject& rEvent )
throw( RuntimeException )
{
    sal_Bool bDisposeAndClear( sal_False );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( rEvent.Source == Reference< XInterface >( m_xFrame, UNO_QUERY ))
    {
        // Our frame gets disposed, release all our references that depends on a working frame reference.
        Application::RemoveEventListener( LINK( this, LayoutManager, SettingsChanged ) );

        // destroy all elements, it's possible that dettaching is NOT called!
        implts_destroyElements();
        impl_clearUpMenuBar();
        m_xMenuBar.clear();
        if ( m_xInplaceMenuBar.is() )
        {
            m_pInplaceMenuBar = 0;
            m_xInplaceMenuBar->dispose();
        }
        m_xInplaceMenuBar.clear();
        m_xContainerWindow.clear();
        m_xContainerTopWindow.clear();

        // forward disposing call to toolbar manager
        if ( m_pToolbarManager != NULL )
            m_pToolbarManager->disposing(rEvent);

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
        delete m_pGlobalSettings;
        m_pGlobalSettings = 0;
        m_xDockingAreaAcceptor = Reference< ui::XDockingAreaAcceptor >();

        bDisposeAndClear = sal_True;
    }
    else if ( rEvent.Source == Reference< XInterface >( m_xContainerWindow, UNO_QUERY ))
    {
        // Our container window gets disposed. Remove all user interface elements.
        ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
        if ( pToolbarManager )
        {
            uno::Reference< awt::XWindowPeer > aEmptyWindowPeer;
            pToolbarManager->setParentWindow( aEmptyWindowPeer );
        }
        impl_clearUpMenuBar();
        m_xMenuBar.clear();
        if ( m_xInplaceMenuBar.is() )
        {
            m_pInplaceMenuBar = 0;
            m_xInplaceMenuBar->dispose();
        }
        m_xInplaceMenuBar.clear();
        m_xContainerWindow.clear();
        m_xContainerTopWindow.clear();
    }
    else if ( rEvent.Source == Reference< XInterface >( m_xDocCfgMgr, UNO_QUERY ))
        m_xDocCfgMgr.clear();
    else if ( rEvent.Source == Reference< XInterface >( m_xModuleCfgMgr , UNO_QUERY ))
        m_xModuleCfgMgr.clear();

    aWriteLock.unlock();
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

void SAL_CALL LayoutManager::elementInserted( const ui::ConfigurationEvent& Event ) throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame( m_xFrame );
    Reference< ui::XUIConfigurationListener > xUICfgListener( m_xToolbarManager );
    ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
    aReadLock.unlock();

    if ( xFrame.is() )
    {
        OUString aElementType;
        OUString aElementName;
        bool            bRefreshLayout(false);

        parseResourceURL( Event.ResourceURL, aElementType, aElementName );
        if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_TOOLBAR ))
        {
            if ( xUICfgListener.is() )
            {
                xUICfgListener->elementInserted( Event );
                bRefreshLayout = pToolbarManager->isLayoutDirty();
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCase( UIRESOURCETYPE_MENUBAR ))
        {
            Reference< XUIElement >         xUIElement = implts_findElement( Event.ResourceURL );
            Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
            if ( xElementSettings.is() )
            {
                OUString aConfigSourcePropName( "ConfigurationSource" );
                uno::Reference< XPropertySet > xPropSet( xElementSettings, uno::UNO_QUERY );
                if ( xPropSet.is() )
                {
                    if ( Event.Source == uno::Reference< uno::XInterface >( m_xDocCfgMgr, uno::UNO_QUERY ))
                        xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( m_xDocCfgMgr ));
                }
                xElementSettings->updateSettings();
            }
        }

        if ( bRefreshLayout )
            doLayout();
    }
}

void SAL_CALL LayoutManager::elementRemoved( const ui::ConfigurationEvent& Event ) throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    Reference< frame::XFrame >                xFrame( m_xFrame );
    Reference< ui::XUIConfigurationListener > xToolbarManager( m_xToolbarManager );
    Reference< awt::XWindow >                 xContainerWindow( m_xContainerWindow );
    Reference< ui::XUIElement >               xMenuBar( m_xMenuBar );
    Reference< ui::XUIConfigurationManager >  xModuleCfgMgr( m_xModuleCfgMgr );
    Reference< ui::XUIConfigurationManager >  xDocCfgMgr( m_xDocCfgMgr );
    ToolbarLayoutManager*                     pToolbarManager = m_pToolbarManager;
    aReadLock.unlock();

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
                bRefreshLayout = pToolbarManager->isLayoutDirty();
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
                            pSysWindow->SetMenuBar( 0 );

                        Reference< XComponent > xComp( xMenuBar, UNO_QUERY );
                        if ( xComp.is() )
                            xComp->dispose();

                        WriteGuard aWriteLock( m_aLock );
                        m_xMenuBar.clear();
                    }
                }
            }
        }

        if ( bRefreshLayout )
            doLayout();
    }
}

void SAL_CALL LayoutManager::elementReplaced( const ui::ConfigurationEvent& Event ) throw (uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame >                       xFrame( m_xFrame );
    Reference< ui::XUIConfigurationListener > xToolbarManager( m_xToolbarManager );
    ToolbarLayoutManager*                     pToolbarManager = m_pToolbarManager;
    aReadLock.unlock();

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
                bRefreshLayout = pToolbarManager->isLayoutDirty();
            }
        }
        else
        {
            Reference< XUIElement >         xUIElement = implts_findElement( Event.ResourceURL );
            Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
            if ( xElementSettings.is() )
            {
                OUString           aConfigSourcePropName( "ConfigurationSource" );
                Reference< XInterface >   xElementCfgMgr;
                Reference< XPropertySet > xPropSet( xElementSettings, UNO_QUERY );

                if ( xPropSet.is() )
                    xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

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

//---------------------------------------------------------------------------------------------------------
//      OPropertySetHelper
//---------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL LayoutManager::convertFastPropertyValue( Any&       aConvertedValue,
                                                           Any&       aOldValue,
                                                           sal_Int32  nHandle,
                                                           const Any& aValue ) throw( lang::IllegalArgumentException )
{
    return LayoutManager_PBase::convertFastPropertyValue( aConvertedValue, aOldValue, nHandle, aValue );
}

void SAL_CALL LayoutManager::setFastPropertyValue_NoBroadcast( sal_Int32       nHandle,
                                                               const uno::Any& aValue  ) throw( uno::Exception )
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
            sal_Bool bValue(sal_False);
            if (( aValue >>= bValue ) && bValue )
            {
                ReadGuard aReadLock( m_aLock );
                ToolbarLayoutManager* pToolbarManager = m_pToolbarManager;
                bool bAutomaticToolbars( m_bAutomaticToolbars );
                aReadLock.unlock();

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

void SAL_CALL LayoutManager::getFastPropertyValue( uno::Any& aValue, sal_Int32 nHandle ) const
{
    LayoutManager_PBase::getFastPropertyValue( aValue, nHandle );
}

namespace detail
{
    class InfoHelperBuilder : private ::boost::noncopyable
    {
    private:
        ::cppu::OPropertyArrayHelper *m_pInfoHelper;
    public:
        InfoHelperBuilder(const LayoutManager &rManager)
        {
            uno::Sequence< beans::Property > aProperties;
            rManager.describeProperties(aProperties);
            m_pInfoHelper = new ::cppu::OPropertyArrayHelper(aProperties, sal_True);
        }
        ~InfoHelperBuilder()
        {
            delete m_pInfoHelper;
        }

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

uno::Reference< beans::XPropertySetInfo > SAL_CALL LayoutManager::getPropertySetInfo() throw (uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        if( pInfo == NULL )
        {
            static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
