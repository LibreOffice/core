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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <services/layoutmanager.hxx>
#include <threadhelp/resetableguard.hxx>
#include <services.h>

#include <classes/sfxhelperfunctions.hxx>
#include <uielement/menubarwrapper.hxx>
#include <classes/addonsoptions.hxx>
#include <uiconfiguration/windowstateconfiguration.hxx>
#include <classes/fwkresid.hxx>

#include <classes/resource.hrc>
#include <toolkit/helper/convert.hxx>
#include <uielement/progressbarwrapper.hxx>
#include <uiconfiguration/globalsettings.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/FrameAction.hpp>
#include <com/sun/star/frame/XUIControllerRegistration.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#include <com/sun/star/lang/SystemDependent.hpp>
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/ui/XUIFunctionListener.hpp>
#include <com/sun/star/frame/LayoutManagerEvents.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <svtools/imgdef.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/window.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dockingarea.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/wall.hxx>
#include <toolkit/unohlp.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <comphelper/uno3.hxx>
#include <rtl/logfile.hxx>
#include <unotools/cmdoptions.hxx>

#include <algorithm>
#include <boost/bind.hpp>
// ______________________________________________
//  using namespace

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace ::com::sun::star::ui;
using namespace com::sun::star::frame;
using namespace ::com::sun::star::frame;

#define UIRESOURCE_PROTOCO_ASCII        "private:"
#define UIRESOURCE_RESOURCE_ASCII       "resource"
#define UIRESOURCE_URL_ASCII            "private:resource"
#define UIRESOURCE_URL                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UIRESOURCE_URL_ASCII ))

// ATTENTION!
// This value is directly copied from the sfx2 project.
// You have to change BOTH values, see sfx2/inc/sfx2/sfxsids.hrc (SID_DOCKWIN_START)
static const sal_Int32 DOCKWIN_ID_BASE = 9800;

bool lcl_checkUIElement(const Reference< XUIElement >& xUIElement,css::awt::Rectangle& _rPosSize,Reference< css::awt::XWindow >& _xWindow)
{
    bool bRet = xUIElement.is();
    if ( bRet )
    {
        SolarMutexGuard aGuard;
        _xWindow.set( xUIElement->getRealInterface(), UNO_QUERY );
        _rPosSize = _xWindow->getPosSize();

        Window* pWindow = VCLUnoHelper::GetWindow( _xWindow );
        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
        {
            ::Size aSize = ((ToolBox*)pWindow)->CalcWindowSizePixel( 1 );
            _rPosSize.Width = aSize.Width();
            _rPosSize.Height = aSize.Height();
        }
    }
    return bRet;
}

// convert alignment constant to vcl's WindowAlign type
static WindowAlign ImplConvertAlignment( sal_Int16 aAlignment )
{
    if ( aAlignment == DockingArea_DOCKINGAREA_LEFT )
        return WINDOWALIGN_LEFT;
    else if ( aAlignment == DockingArea_DOCKINGAREA_RIGHT )
        return WINDOWALIGN_RIGHT;
    else if ( aAlignment == DockingArea_DOCKINGAREA_TOP )
        return WINDOWALIGN_TOP;
    else
        return WINDOWALIGN_BOTTOM;
}

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

namespace framework
{

struct UIElementVisibility
{
    rtl::OUString aName;
    bool          bVisible;
};

bool LayoutManager::UIElement::operator< ( const LayoutManager::UIElement& aUIElement ) const
{
    if ( !m_xUIElement.is() && aUIElement.m_xUIElement.is() )
        return false;
    else if ( m_xUIElement.is() && !aUIElement.m_xUIElement.is() )
        return true;
    else if ( !m_bVisible && aUIElement.m_bVisible )
        return false;
    else if ( m_bVisible && !aUIElement.m_bVisible )
        return true;
    else if ( !m_bFloating && aUIElement.m_bFloating )
        return true;
    else if ( m_bFloating && !aUIElement.m_bFloating )
        return false;
    else
    {
        if ( m_bFloating )
        {
            bool bEqual = ( m_aFloatingData.m_aPos.Y() == aUIElement.m_aFloatingData.m_aPos.Y() );
            if ( bEqual )
                return ( m_aFloatingData.m_aPos.X() < aUIElement.m_aFloatingData.m_aPos.X() );
            else
                return ( m_aFloatingData.m_aPos.Y() < aUIElement.m_aFloatingData.m_aPos.Y() );
        }
        else
        {
            if ( m_aDockedData.m_nDockedArea < aUIElement.m_aDockedData.m_nDockedArea )
                return true;
            else if ( m_aDockedData.m_nDockedArea > aUIElement.m_aDockedData.m_nDockedArea )
                return false;
            else
            {
                if ( m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_TOP ||
                     m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_BOTTOM )
                {
                    if ( !( m_aDockedData.m_aPos.Y() == aUIElement.m_aDockedData.m_aPos.Y() ) )
                        return  ( m_aDockedData.m_aPos.Y() < aUIElement.m_aDockedData.m_aPos.Y() );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.X() == aUIElement.m_aDockedData.m_aPos.X() );
                        if ( bEqual )
                        {
                            return m_bUserActive && !aUIElement.m_bUserActive;
                        }
                        else
                            return ( m_aDockedData.m_aPos.X() < aUIElement.m_aDockedData.m_aPos.X() );
                    }
                }
                else
                {
                    if ( !( m_aDockedData.m_aPos.X() == aUIElement.m_aDockedData.m_aPos.X() ) )
                        return ( m_aDockedData.m_aPos.X() < aUIElement.m_aDockedData.m_aPos.X() );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.Y() == aUIElement.m_aDockedData.m_aPos.Y() );
                        if ( bEqual )
                        {
                            return m_bUserActive && !aUIElement.m_bUserActive;
                        }
                        else
                            return ( m_aDockedData.m_aPos.Y() < aUIElement.m_aDockedData.m_aPos.Y() );
                    }
                }
            }
        }
    }
}

LayoutManager::UIElement& LayoutManager::UIElement::operator= ( const LayoutManager::UIElement& rUIElement )
{
    if (this == &rUIElement) { return *this; }
    m_aType             = rUIElement.m_aType;
    m_aName             = rUIElement.m_aName;
    m_aUIName           = rUIElement.m_aUIName;
    m_xUIElement        = rUIElement.m_xUIElement;
    m_bFloating         = rUIElement.m_bFloating;
    m_bVisible          = rUIElement.m_bVisible;
    m_bUserActive       = rUIElement.m_bUserActive;
    m_bCreateNewRowCol0 = rUIElement.m_bCreateNewRowCol0;
    m_bDeactiveHide     = rUIElement.m_bDeactiveHide;
    m_bMasterHide       = rUIElement.m_bMasterHide;
    m_bContextSensitive = rUIElement.m_bContextSensitive;
    m_bContextActive    = rUIElement.m_bContextActive;
    m_bNoClose          = rUIElement.m_bNoClose;
    m_bSoftClose        = rUIElement.m_bSoftClose;
    m_bStateRead        = rUIElement.m_bStateRead;
    m_nStyle            = rUIElement.m_nStyle;
    m_aDockedData       = rUIElement.m_aDockedData;
    m_aFloatingData     = rUIElement.m_aFloatingData;
    return *this;
}

static Reference< XModel > impl_getModelFromFrame( const Reference< XFrame >& rFrame )
{
    // Query for the model to get check the context information
    Reference< XModel > xModel;
    if ( rFrame.is() )
    {
        Reference< XController > xController( rFrame->getController(), UNO_QUERY );
        if ( xController.is() )
            xModel = xController->getModel();
    }

    return xModel;
}

static sal_Bool implts_isPreviewModel( const Reference< XModel >& xModel )
{
    if ( xModel.is() )
    {
        ::comphelper::MediaDescriptor aDesc( xModel->getArgs() );
        return aDesc.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_PREVIEW(), (sal_Bool)sal_False);
    }
    else
        return sal_False;
}

static sal_Bool implts_isFrameOrWindowTop( const css::uno::Reference< css::frame::XFrame >& xFrame )
{
    if (xFrame->isTop())
        return sal_True;

    css::uno::Reference< css::awt::XTopWindow > xWindowCheck(xFrame->getContainerWindow(), css::uno::UNO_QUERY); // dont use _THROW here ... its a check only
    if (xWindowCheck.is())
    {
        // Top and system window is required (#i76867#)
        SolarMutexGuard aGuard;
        css::uno::Reference< css::awt::XWindow > xWindow( xWindowCheck, UNO_QUERY );
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        return ( pWindow && pWindow->IsSystemWindow() );
    }

    return sal_False;
}

static void impl_setDockingWindowVisibility( const css::uno::Reference< css::lang::XMultiServiceFactory>& rSMGR, const css::uno::Reference< css::frame::XFrame >& rFrame, const ::rtl::OUString& rDockingWindowName, bool bVisible )
{
    const ::rtl::OUString aDockWinPrefixCommand( RTL_CONSTASCII_USTRINGPARAM( "DockingWindow" ));
    css::uno::WeakReference< css::frame::XDispatchHelper > xDispatchHelper;

    sal_Int32 nID    = rDockingWindowName.toInt32();
    sal_Int32 nIndex = nID - DOCKWIN_ID_BASE;

    css::uno::Reference< css::frame::XDispatchProvider > xProvider(rFrame, css::uno::UNO_QUERY);
    if ( nIndex >= 0 && xProvider.is() )
    {
        ::rtl::OUString aDockWinCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));
        ::rtl::OUString aDockWinArgName( aDockWinPrefixCommand );

        aDockWinArgName += ::rtl::OUString::valueOf( nIndex );

        css::uno::Sequence< css::beans::PropertyValue > aArgs(1);
        aArgs[0].Name  = aDockWinArgName;
        aArgs[0].Value = css::uno::makeAny( bVisible );

        css::uno::Reference< css::frame::XDispatchHelper > xDispatcher( xDispatchHelper );
        if ( !xDispatcher.is())
        {
            xDispatcher = css::uno::Reference< css::frame::XDispatchHelper >(
                rSMGR->createInstance(SERVICENAME_DISPATCHHELPER), css::uno::UNO_QUERY_THROW);
        }

        aDockWinCommand = aDockWinCommand + aDockWinArgName;
        xDispatcher->executeDispatch(
            xProvider,
            aDockWinCommand,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
            0,
            aArgs);
    }
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
IMPLEMENT_FORWARD_XTYPEPROVIDER2( LayoutManager, LayoutManager_Base, LayoutManager_PBase )
IMPLEMENT_FORWARD_XINTERFACE2( LayoutManager, LayoutManager_Base, LayoutManager_PBase )

DEFINE_XSERVICEINFO_MULTISERVICE        (   LayoutManager                       ,
                                            ::cppu::OWeakObject                 ,
                                            SERVICENAME_LAYOUTMANAGER           ,
                                            IMPLEMENTATIONNAME_LAYOUTMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   LayoutManager, {} )


LayoutManager::LayoutManager( const Reference< XMultiServiceFactory >& xServiceManager )
        :   LayoutManager_Base          (                                                   )
        ,   ThreadHelpBase              ( &Application::GetSolarMutex()                     )
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex()         )
        ,   LayoutManager_PBase         ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        ,   m_xSMGR( xServiceManager )
        ,   m_xURLTransformer( xServiceManager->createInstance( SERVICENAME_URLTRANSFORMER ), UNO_QUERY )
        ,   m_xDisplayAccess( xServiceManager->createInstance( SERVICENAME_DISPLAYACCESS ), UNO_QUERY )
        ,   m_nLockCount( 0 )
        ,   m_bActive( sal_False )
        ,   m_bInplaceMenuSet( sal_False )
        ,   m_bDockingInProgress( sal_False )
        ,   m_bMenuVisible( sal_True )
        ,   m_bComponentAttached( sal_False )
        ,   m_bDoLayout( sal_False )
        ,   m_bVisible( sal_True )
        ,   m_bParentWindowVisible( sal_False )
        ,   m_bMustDoLayout( sal_True )
        ,   m_bAutomaticToolbars( sal_True )
        ,   m_bStoreWindowState( sal_False )
        ,   m_bHideCurrentUI( false )
        ,   m_bGlobalSettings( sal_False )
        ,   m_bPreserveContentSize( false )
        ,   m_eDockOperation( DOCKOP_ON_COLROW )
        ,   m_pInplaceMenuBar( NULL )
        ,   m_xModuleManager( Reference< XModuleManager >(
                xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ))
        ,   m_xUIElementFactoryManager( Reference< ::com::sun::star::ui::XUIElementFactory >(
                xServiceManager->createInstance( SERVICENAME_UIELEMENTFACTORYMANAGER ), UNO_QUERY ))
        ,   m_bMenuBarCloser( sal_False )
        ,   m_xPersistentWindowStateSupplier( Reference< XNameAccess >(
                xServiceManager->createInstance( SERVICENAME_WINDOWSTATECONFIGURATION ), UNO_QUERY ))
        ,   m_pGlobalSettings( 0 )
        ,   m_aCustomTbxPrefix( RTL_CONSTASCII_USTRINGPARAM( "custom_" ))
        ,   m_aFullCustomTbxPrefix( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/custom_" ))
        ,   m_aFullAddonTbxPrefix( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/addon_" ))
        ,   m_aStatusBarAlias( RTL_CONSTASCII_USTRINGPARAM( "private:resource/statusbar/statusbar" ))
        ,   m_aProgressBarAlias( RTL_CONSTASCII_USTRINGPARAM( "private:resource/progressbar/progressbar" ))
        ,   m_aPropDocked( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_DOCKED ))
        ,   m_aPropVisible( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_VISIBLE ))
        ,   m_aPropDockingArea( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_DOCKINGAREA ))
        ,   m_aPropDockPos( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_DOCKPOS ))
        ,   m_aPropPos( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_POS ))
        ,   m_aPropSize( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_SIZE ))
        ,   m_aPropUIName( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_UINAME ))
        ,   m_aPropStyle( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_STYLE ))
        ,   m_aPropLocked( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_LOCKED ))
        ,   m_aCustomizeCmd( RTL_CONSTASCII_USTRINGPARAM( "ConfigureDialog" ))
        ,   m_pAddonOptions( 0 )
        ,   m_aListenerContainer( m_aLock.getShareableOslMutex() )
{
    // Initialize statusbar member
    m_aStatusBarElement.m_aType = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "statusbar" ));
    m_aStatusBarElement.m_aName = m_aStatusBarAlias;

    m_pMiscOptions = new SvtMiscOptions();

    m_pMiscOptions->AddListenerLink( LINK( this, LayoutManager, OptionsChanged ) );
    Application::AddEventListener( LINK( this, LayoutManager, SettingsChanged ) );
    m_eSymbolsSize = m_pMiscOptions->GetSymbolsSize();
    m_eSymbolsStyle = m_pMiscOptions->GetCurrentSymbolsStyle();

    m_aAsyncLayoutTimer.SetTimeout( 50 );
    m_aAsyncLayoutTimer.SetTimeoutHdl( LINK( this, LayoutManager, AsyncLayoutHdl ) );


    registerProperty( LAYOUTMANAGER_PROPNAME_AUTOMATICTOOLBARS, LAYOUTMANAGER_PROPHANDLE_AUTOMATICTOOLBARS, css::beans::PropertyAttribute::TRANSIENT, &m_bAutomaticToolbars, ::getCppuType( &m_bAutomaticToolbars ) );
    registerProperty( LAYOUTMANAGER_PROPNAME_HIDECURRENTUI, LAYOUTMANAGER_PROPHANDLE_HIDECURRENTUI, css::beans::PropertyAttribute::TRANSIENT, &m_bHideCurrentUI, ::getCppuType( &m_bHideCurrentUI ) );
    registerProperty( LAYOUTMANAGER_PROPNAME_LOCKCOUNT, LAYOUTMANAGER_PROPHANDLE_LOCKCOUNT, css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY, &m_nLockCount, getCppuType( &m_nLockCount )  );
    registerProperty( LAYOUTMANAGER_PROPNAME_MENUBARCLOSER, LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER, css::beans::PropertyAttribute::TRANSIENT, &m_bMenuBarCloser, ::getCppuType( &m_bMenuBarCloser ) );
    const sal_Bool bRefreshVisibility = sal_False;
    registerPropertyNoMember( LAYOUTMANAGER_PROPNAME_REFRESHVISIBILITY, LAYOUTMANAGER_PROPHANDLE_REFRESHVISIBILITY, css::beans::PropertyAttribute::TRANSIENT, ::getCppuType( &bRefreshVisibility ), &bRefreshVisibility );
    registerProperty( LAYOUTMANAGER_PROPNAME_PRESERVE_CONTENT_SIZE, LAYOUTMANAGER_PROPHANDLE_PRESERVE_CONTENT_SIZE, css::beans::PropertyAttribute::TRANSIENT, &m_bPreserveContentSize, ::getCppuType( &m_bPreserveContentSize ) );
}

LayoutManager::~LayoutManager()
{
    Application::RemoveEventListener( LINK( this, LayoutManager, SettingsChanged ) );
    if ( m_pMiscOptions )
    {
        m_pMiscOptions->RemoveListenerLink( LINK( this, LayoutManager, OptionsChanged ) );
        delete m_pMiscOptions;
        m_pMiscOptions = 0;
    }
    m_aAsyncLayoutTimer.Stop();
}

// Internal helper function
void LayoutManager::impl_clearUpMenuBar()
{
    implts_lock();

    // Clear up VCL menu bar to prepare shutdown
    if ( m_xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            MenuBar* pSetMenuBar = 0;
            if ( m_xInplaceMenuBar.is() )
                pSetMenuBar = (MenuBar *)m_pInplaceMenuBar->GetMenuBar();
            else
            {
                Reference< css::awt::XMenuBar > xMenuBar;

                Reference< XPropertySet > xPropSet( m_xMenuBar, UNO_QUERY );
                if ( xPropSet.is() )
                {
                    try
                    {
                        xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMenuBar" ))) >>= xMenuBar;
                    }
                    catch ( com::sun::star::beans::UnknownPropertyException )
                    {
                    }
                    catch ( com::sun::star::lang::WrappedTargetException )
                    {
                    }
                }

                VCLXMenu* pAwtMenuBar = VCLXMenu::GetImplementation( xMenuBar );
                if ( pAwtMenuBar )
                    pSetMenuBar = (MenuBar*)pAwtMenuBar->GetMenu();
            }

            MenuBar* pTopMenuBar = ((SystemWindow *)pWindow)->GetMenuBar();
            if ( pSetMenuBar == pTopMenuBar )
                ((SystemWindow *)pWindow)->SetMenuBar( 0 );
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

sal_Bool LayoutManager::impl_parseResourceURL( const rtl::OUString aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName )
{
    URL         aURL;
    sal_Int32   nIndex = 0;

    aURL.Complete = aResourceURL;
    m_xURLTransformer->parseStrict( aURL );

    ::rtl::OUString aUIResource = aURL.Path.getToken( 0, (sal_Unicode)'/', nIndex );

    if (( aURL.Protocol.equalsIgnoreAsciiCaseAscii( UIRESOURCE_PROTOCO_ASCII )) &&
        ( aUIResource.equalsIgnoreAsciiCaseAscii( UIRESOURCE_RESOURCE_ASCII )))
    {
        aElementType = aURL.Path.getToken( 0, (sal_Unicode)'/', nIndex );
        aElementName = aURL.Path.getToken( 0, (sal_Unicode)'/', nIndex );
        return sal_True;
    }

    return sal_False;
}

void LayoutManager::implts_lock()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    ++m_nLockCount;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

sal_Bool LayoutManager::implts_unlock()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    --m_nLockCount;
    if ( m_nLockCount < 0 )
        m_nLockCount = 0;
    return ( m_nLockCount == 0 );
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void LayoutManager::implts_reset( sal_Bool bAttached )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< css::awt::XWindow > xTopDockingWindow    = m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP];
    Reference< css::awt::XWindow > xLeftDockingWindow   = m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT];
    Reference< css::awt::XWindow > xRightDockingWindow  = m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT];
    Reference< css::awt::XWindow > xBottomDockingWindow = m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM];
    Reference< XUIConfiguration > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
    Reference< XUIConfiguration > xDocCfgMgr( m_xDocCfgMgr, UNO_QUERY );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    Reference< XMultiServiceFactory > xServiceManager( m_xSMGR );
    Reference< XNameAccess > xPersistentWindowStateSupplier( m_xPersistentWindowStateSupplier );
    ::rtl::OUString aModuleIdentifier( m_aModuleIdentifier );
    sal_Bool bAutomaticToolbars( m_bAutomaticToolbars );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_lock();

    Reference< XModel > xModel;
    if ( xFrame.is() )
    {
        if ( bAttached )
        {
            ::rtl::OUString aOldModuleIdentifier( aModuleIdentifier );
            try
            {
                aModuleIdentifier = m_xModuleManager->identify( Reference< XInterface >( xFrame, UNO_QUERY ) );
            }
            catch( Exception& )
            {
            }

            if ( aModuleIdentifier.getLength() && aOldModuleIdentifier != aModuleIdentifier )
            {
                Reference< XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier;
                if ( xServiceManager.is() )
                    xModuleCfgSupplier = Reference< XModuleUIConfigurationManagerSupplier >(
                        xServiceManager->createInstance( SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ), UNO_QUERY );

                if ( xModuleCfgMgr.is() )
                {
                    try
                    {
                        // Remove listener to old module ui configuration manager
                        xModuleCfgMgr->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                    }
                    catch ( Exception& )
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
                catch ( Exception& )
                {
                }

                try
                {
                    // Retrieve persistent window state reference for our new module
                    if ( xPersistentWindowStateSupplier.is() )
                        xPersistentWindowStateSupplier->getByName( aModuleIdentifier ) >>= xPersistentWindowState;
                }
                catch ( NoSuchElementException& )
                {
                }
                catch ( WrappedTargetException& )
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
                        catch ( Exception& )
                        {
                        }
                    }

                    try
                    {
                        xDocCfgMgr = Reference< XUIConfiguration >( xUIConfigurationManagerSupplier->getUIConfigurationManager(), UNO_QUERY );
                        if ( xDocCfgMgr.is() )
                            xDocCfgMgr->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                    }
                    catch ( Exception& )
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
                catch ( Exception& )
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
                catch ( Exception& )
                {
                }
            }

            // Release references to our configuration managers as we currently don't have
            // an attached module.
            xModuleCfgMgr.clear();
            xDocCfgMgr.clear();
            xPersistentWindowState.clear();
            aModuleIdentifier = ::rtl::OUString();
        }

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_xModel = xModel;
        m_aDockingArea = css::awt::Rectangle();
        m_bComponentAttached = bAttached;
        m_aModuleIdentifier = aModuleIdentifier;
        m_xModuleCfgMgr = Reference< XUIConfigurationManager >( xModuleCfgMgr, UNO_QUERY );
        m_xDocCfgMgr = Reference< XUIConfigurationManager >( xDocCfgMgr, UNO_QUERY );
        m_xPersistentWindowState = xPersistentWindowState;
        m_aStatusBarElement.m_bStateRead = sal_False; // reset state to read data again!
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        if ( bAttached )
        {
            // reset docking area windows back to zero size
            try
            {
                if ( xTopDockingWindow.is() )
                    xTopDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
                if ( xLeftDockingWindow.is() )
                    xLeftDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
                if ( xRightDockingWindow.is() )
                    xRightDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
                if ( xBottomDockingWindow.is() )
                    xBottomDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
            }
            catch ( Exception& )
            {
            }

            if ( bAutomaticToolbars )
            {
                implts_createCustomToolBars();
                implts_createAddonsToolBars();
                implts_createNonContextSensitiveToolBars();
            }
            implts_sortUIElements();
        }
        else
            implts_destroyElements();
    }

    implts_unlock();
}

sal_Bool LayoutManager::implts_isEmbeddedLayoutManager() const
{
    // check if this layout manager is currently using the embedded feature
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    Reference< css::awt::XWindow > xFrameContainerWindow = xFrame->getContainerWindow();
    if ( xFrameContainerWindow == xContainerWindow )
        return sal_False;
    else
        return sal_True;
}

void LayoutManager::implts_destroyElements()
{
    UIElementVector aUIElementVector;

    WriteGuard aWriteLock( m_aLock );
    aUIElementVector = m_aUIElements;
    m_aUIElements.clear();
    aWriteLock.unlock();

    UIElementVector::iterator pIter;
    for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
    {
        Reference< XComponent > xComponent( pIter->m_xUIElement, UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }

    implts_destroyStatusBar();

    aWriteLock.lock();
    impl_clearUpMenuBar();
    aWriteLock.unlock();
}

void LayoutManager::implts_destroyDockingAreaWindows()
{
    std::vector< Reference< css::awt::XWindow > > oldDockingAreaWindows;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP] );
    oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] );
    oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT] );
    oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT] );

    m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP].clear();
    m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM].clear();
    m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT].clear();
    m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT].clear();
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    const sal_uInt32 nCount = oldDockingAreaWindows.size();
    for ( sal_uInt32 i=0; i < nCount; i++ )
    {
        if ( oldDockingAreaWindows[i].is() )
        {
            try
            {
                oldDockingAreaWindows[i]->dispose();
            }
            catch ( Exception& )
            {
            }
        }
    }
}

void LayoutManager::implts_createCustomToolBar( const rtl::OUString& aTbxResName, const rtl::OUString& aTitle )
{
    if ( aTbxResName.getLength() > 0 )
    {
        createElement( aTbxResName );
        if ( aTitle )
        {
            Reference< XUIElement > xUIElement = getElement( aTbxResName );
            if ( xUIElement.is() )
            {
                SolarMutexGuard aGuard;

                Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow  )
                    pWindow->SetText( aTitle );
            }
        }
    }
}

void LayoutManager::implts_createCustomToolBars(
    const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& aTbxSeqSeq )
{
    const Sequence< PropertyValue >* pTbxSeq = aTbxSeqSeq.getConstArray();
    for ( sal_Int32 i = 0; i < aTbxSeqSeq.getLength(); i++ )
    {
        const Sequence< PropertyValue >& rTbxSeq = pTbxSeq[i];
        ::rtl::OUString aTbxResName;
        ::rtl::OUString aTbxTitle;
        for ( sal_Int32 j = 0; j < rTbxSeq.getLength(); j++ )
        {
            if ( rTbxSeq[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ResourceURL")) )
                rTbxSeq[j].Value >>= aTbxResName;
            else if ( rTbxSeq[j].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("UIName")) )
                rTbxSeq[j].Value >>= aTbxTitle;
        }

        // Only create custom toolbars. Their name have to start with "custom_"!
        if ( aTbxResName.getLength() > 0 && aTbxResName.indexOf( m_aCustomTbxPrefix ) != -1 )
            implts_createCustomToolBar( aTbxResName, aTbxTitle );
    }
}

void LayoutManager::implts_createCustomToolBars()
{
    ReadGuard aReadLock( m_aLock );
    if ( !m_bComponentAttached )
        return;

    Reference< XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    Reference< XFrame > xFrame( m_xFrame );
    Reference< XModel > xModel;
    Reference< XUIConfigurationManager > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
    Reference< XUIConfigurationManager > xDocCfgMgr( m_xDocCfgMgr, UNO_QUERY );
    aReadLock.unlock();

    if ( xFrame.is() )
    {
        xModel = impl_getModelFromFrame( xFrame );
        if ( implts_isPreviewModel( xModel ))
            return; // no custom toolbars for preview frame!

        Sequence< Sequence< PropertyValue > > aTbxSeq;
        if ( xDocCfgMgr.is() )
        {
            aTbxSeq = xDocCfgMgr->getUIElementsInfo( UIElementType::TOOLBAR );
            implts_createCustomToolBars( aTbxSeq ); // first create all document based toolbars
        }
        if ( xModuleCfgMgr.is() )
        {
            aTbxSeq = xModuleCfgMgr->getUIElementsInfo( UIElementType::TOOLBAR );
            implts_createCustomToolBars( aTbxSeq ); // second create module based toolbars
        }
    }
}

rtl::OUString LayoutManager::implts_generateGenericAddonToolbarTitle( sal_Int32 nNumber ) const
{
    String aAddonGenericTitle;

    aAddonGenericTitle = String( FwkResId( STR_TOOLBAR_TITLE_ADDON ));
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();

    String aNumStr = rI18nHelper.GetNum( nNumber, 0, FALSE, FALSE );
    aAddonGenericTitle.SearchAndReplaceAscii( "%num%", aNumStr );

    return rtl::OUString( aAddonGenericTitle );
}

void LayoutManager::implts_createAddonsToolBars()
{
    WriteGuard aWriteLock( m_aLock );
    if ( !m_bComponentAttached )
        return;

    Reference< XModel > xModel;
    Reference< XFrame > xFrame( m_xFrame );
    if ( !xFrame.is() )
        return;

    if ( !m_pAddonOptions )
        m_pAddonOptions = new AddonsOptions;

    Reference< XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    aWriteLock.unlock();

    xModel = impl_getModelFromFrame( xFrame );
    if ( implts_isPreviewModel( xModel ))
        return; // no addon toolbars for preview frame!

    UIElementVector aUIElementVector;
    Sequence< Sequence< PropertyValue > > aAddonToolBarData;
    Reference< XUIElement >               xUIElement;

    sal_uInt32 nCount = m_pAddonOptions->GetAddonsToolBarCount();
    ::rtl::OUString aAddonsToolBarStaticName( m_aFullAddonTbxPrefix );
    ::rtl::OUString aElementType( RTL_CONSTASCII_USTRINGPARAM( "toolbar" ));

    Sequence< PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropSeq[0].Value <<= xFrame;
    aPropSeq[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationData" ));
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        ::rtl::OUString aAddonToolBarName( aAddonsToolBarStaticName + m_pAddonOptions->GetAddonsToolbarResourceName(i) );
        aAddonToolBarData = m_pAddonOptions->GetAddonsToolBarPart( i );
        aPropSeq[1].Value <<= aAddonToolBarData;

        aWriteLock.lock();
        UIElement aElement = impl_findElement( aAddonToolBarName );
        aWriteLock.unlock();

        // #i79828
        // It's now possible that we are called more than once. Be sure to not create
        // add-on toolbars more than once!
        if ( aElement.m_xUIElement.is() )
            continue;

        try
        {
            xUIElement = xUIElementFactory->createUIElement( aAddonToolBarName, aPropSeq );
            if ( xUIElement.is() )
            {
                Reference< css::awt::XDockableWindow > xDockWindow( xUIElement->getRealInterface(), UNO_QUERY );
                if ( xDockWindow.is() )
                {
                    try
                    {
                        xDockWindow->addDockableWindowListener( Reference< css::awt::XDockableWindowListener >( static_cast< OWeakObject * >( this ), UNO_QUERY ));
                        xDockWindow->enableDocking( sal_True );
                        Reference< css::awt::XWindow > xWindow( xDockWindow, UNO_QUERY );
                        if ( xWindow.is() )
                            xWindow->addWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject * >( this ), UNO_QUERY ));
                    }
                    catch ( Exception& )
                    {
                    }
                }

                ::rtl::OUString aGenericAddonTitle = implts_generateGenericAddonToolbarTitle( i+1 );

                if ( aElement.m_aName.getLength() > 0 )
                {
                    // Reuse a local entry so we are able to use the latest
                    // UI changes for this document.
                    implts_setElementData( aElement, xDockWindow );
                    aElement.m_xUIElement = xUIElement;
                    if ( aElement.m_aUIName.getLength() == 0 )
                    {
                        aElement.m_aUIName = aGenericAddonTitle;
                        implts_writeWindowStateData( aElement.m_aName, aElement );
                    }
                }
                else
                {
                    // Create new UI element and try to read its state data
                    UIElement aNewToolbar( aAddonToolBarName, aElementType, xUIElement );
                    aNewToolbar.m_bFloating = sal_True;
                    implts_readWindowStateData( aAddonToolBarName, aNewToolbar );
                    implts_setElementData( aNewToolbar, xDockWindow );
                    if ( aNewToolbar.m_aUIName.getLength() == 0 )
                    {
                        aNewToolbar.m_aUIName = aGenericAddonTitle;
                        implts_writeWindowStateData( aNewToolbar.m_aName, aNewToolbar );
                    }
                    implts_insertUIElement( aNewToolbar );
                }

                Reference< css::awt::XWindow > xWindow( xDockWindow, UNO_QUERY );
                if ( xWindow.is() )
                {
                    // Set generic title for add-on toolbar
                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow && pWindow->GetText().Len() == 0 )
                        pWindow->SetText( aGenericAddonTitle );
                    if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                    {
                        ToolBox* pToolbar = (ToolBox *)pWindow;
                        pToolbar->SetMenuType();
                    }
                }
            }
        }
        catch ( NoSuchElementException& )
        {
        }
        catch ( IllegalArgumentException& )
        {
        }
    }
}

void LayoutManager::implts_createNonContextSensitiveToolBars()
{
    ReadGuard aReadLock( m_aLock );

    if ( !m_xPersistentWindowState.is() ||
         !m_xFrame.is() ||
         !m_bComponentAttached )
        return;

    Reference< XFrame > xFrame( m_xFrame );

    Reference< XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aReadLock.unlock();

    if ( implts_isPreviewModel( impl_getModelFromFrame( xFrame )))
        return;

    std::vector< rtl::OUString > aMakeVisibleToolbars;

    try
    {
        Sequence< rtl::OUString > aToolbarNames = xPersistentWindowState->getElementNames();

        if ( aToolbarNames.getLength() > 0 )
        {
            rtl::OUString aElementType;
            rtl::OUString aElementName;
            rtl::OUString aName;

            Reference< ::com::sun::star::ui::XUIElement > xUIElement;
            aMakeVisibleToolbars.reserve(aToolbarNames.getLength());
            WriteGuard aWriteLock( m_aLock );

            const rtl::OUString* pTbNames = aToolbarNames.getConstArray();
            for ( sal_Int32 i = 0; i < aToolbarNames.getLength(); i++ )
            {
                aName = pTbNames[i];
                if ( impl_parseResourceURL( aName, aElementType, aElementName ))
                {
                    // Check that we only create:
                    // - Toolbars (the statusbar is also member of the persistent window state)
                    // - Not custom toolbars, there are created with their own method (implts_createCustomToolbars)
                    if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ) &&
                         aElementName.indexOf( m_aCustomTbxPrefix ) == -1 )
                    {
                        UIElement aNewToolbar( aName, aElementType, xUIElement );
                        bool bFound = implts_findElement( aName, aNewToolbar );
                        if ( !bFound )
                            implts_readWindowStateData( aName, aNewToolbar );

                        if ( aNewToolbar.m_bVisible &&
                             !aNewToolbar.m_bContextSensitive )
                        {
                            if ( !bFound )
                                implts_insertUIElement( aNewToolbar );
                            aMakeVisibleToolbars.push_back( aName );
                        }
                    }
                }
            }
        }
    }
    catch ( RuntimeException& e )
    {
        throw e;
    }
    catch ( Exception& )
    {
    }

    if ( !aMakeVisibleToolbars.empty() )
    {
        implts_lock();
        ::std::for_each( aMakeVisibleToolbars.begin(), aMakeVisibleToolbars.end(),::boost::bind( &LayoutManager::requestElement, this,_1 ));
        implts_unlock();
    }
}

void LayoutManager::implts_toggleFloatingUIElementsVisibility( sal_Bool bActive )
{
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XWindow > xWindow( xDockWindow, UNO_QUERY );
            if ( xDockWindow.is() && xWindow.is() )
            {
                sal_Bool bVisible( sal_True );
                SolarMutexGuard aGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow )
                    bVisible = pWindow->IsVisible();

                if ( xDockWindow->isFloating() )
                {
                    if ( bActive )
                    {
                        if ( !bVisible && pIter->m_bDeactiveHide )
                        {
                            pIter->m_bDeactiveHide = sal_False;
                            // we need VCL here to pass special flags to Show()
                            if( pWindow )
                                pWindow->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                            //xWindow->setVisible( sal_True );
                        }
                    }
                    else
                    {
                        if ( bVisible )
                        {
                            pIter->m_bDeactiveHide = sal_True;
                            xWindow->setVisible( sal_False );
                        }
                    }
                }
            }
        }
    }
}

sal_Bool LayoutManager::implts_findElement( const rtl::OUString& aName, rtl::OUString& aElementType, rtl::OUString& aElementName, Reference< XUIElement >& xUIElement )
{
    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            ReadGuard aReadLock( m_aLock );
            xUIElement = m_xMenuBar;
            return sal_True;
        }
        else if (( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) &&
                   aElementName.equalsIgnoreAsciiCaseAscii( "statusbar" )) ||
                 ( m_aStatusBarElement.m_aName == aName ))
        {
            ReadGuard aReadLock( m_aLock );
            xUIElement = m_aStatusBarElement.m_xUIElement;
            return sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" ))
        {
            ReadGuard aReadLock( m_aLock );
            xUIElement = m_aProgressBarElement.m_xUIElement;
            return sal_True;
        }
        else
        {
            UIElementVector::const_iterator pIter;

            ReadGuard aReadLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_aName == aName )
                {
                    xUIElement = pIter->m_xUIElement;
                    return sal_True;
                }
            }
        }
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_findElement( const Reference< XInterface >& xUIElement, UIElement& aElementData )
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
        {
            Reference< XInterface > xIfac( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            if ( xIfac == xUIElement )
            {
                aElementData = *pIter;
                return sal_True;
            }
        }
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_findElement( const rtl::OUString& aName, UIElement& aElementData )
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == aName )
        {
            aElementData = *pIter;
            return sal_True;
        }
    }

    return sal_False;
}

LayoutManager::UIElement& LayoutManager::impl_findElement( const rtl::OUString& aName )
{
    static UIElement aEmptyElement;

    UIElementVector::iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == aName )
            return *pIter;
    }

    return aEmptyElement;
}

sal_Bool LayoutManager::implts_insertUIElement( const UIElement& rUIElement )
{
    UIElement aTempData;
    bool bFound = implts_findElement( rUIElement.m_aName, aTempData );

#ifdef DBG_UTIL
    if ( bFound )
    {
        char aBuffer[256];
        const sal_Int32 MAX_NAME_LENGTH = 128;
        ::rtl::OString aName = ::rtl::OUStringToOString( rUIElement.m_aName, RTL_TEXTENCODING_ASCII_US );
        aName = aName.copy( ::std::min( MAX_NAME_LENGTH, aName.getLength() ));
        sprintf( aBuffer, "Try to insert an already existing user interface element (%s) into the list\n", aName.getStr() );
        DBG_ASSERT( bFound, aBuffer );
    }
#endif

    bool bResult( false );
    if ( !bFound )
    {
        WriteGuard aWriteLock( m_aLock );
        m_aUIElements.push_back( rUIElement );
        bResult = true;
    }
    return bResult;
}

void LayoutManager::implts_writeNewStateData( const rtl::OUString aName, const Reference< css::awt::XWindow >& xWindow )
{
    css::awt::Rectangle aPos;
    css::awt::Size      aSize;
    sal_Bool            bVisible( sal_False );
    sal_Bool            bFloating( sal_True );

    if ( xWindow.is() )
    {
        Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
        if ( xDockWindow.is() )
            bFloating = xDockWindow->isFloating();

        Reference< css::awt::XWindow2 > xWindow2( xWindow, UNO_QUERY );
        if( xWindow2.is() )
        {
            aPos     = xWindow2->getPosSize();
            aSize    = xWindow2->getOutputSize();   // always use output size for consistency
            bVisible = xWindow2->isVisible();
        }
    }

    WriteGuard aWriteLock( m_aLock );
    UIElement& rUIElement = impl_findElement( aName );
    if ( rUIElement.m_xUIElement.is() && xWindow.is() )
    {
        rUIElement.m_bVisible   = bVisible;
        rUIElement.m_bFloating  = bFloating;
        if ( bFloating )
        {
            rUIElement.m_aFloatingData.m_aPos  = ::Point( aPos.X, aPos.Y );
            rUIElement.m_aFloatingData.m_aSize = ::Size( aSize.Width, aSize.Height );
        }
    }

    implts_writeWindowStateData( aName, rUIElement );

    aWriteLock.unlock();
}

void LayoutManager::implts_refreshContextToolbarsVisibility()
{
    std::vector< UIElementVisibility > aToolbarVisibleVector;

    ReadGuard aReadLock( m_aLock );
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( !m_bVisible || !m_bAutomaticToolbars )
        return;

    UIElementVisibility aUIElementVisible;

    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("toolbar")) )
        {
            aUIElementVisible.aName    = pIter->m_aName;
            aUIElementVisible.bVisible = pIter->m_bVisible;
            aToolbarVisibleVector.push_back( aUIElementVisible );
        }
    }
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    aReadLock.unlock();

    UIElement aUIElement;
    const sal_uInt32 nCount = aToolbarVisibleVector.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        UIElementVisibility& rToolbar = aToolbarVisibleVector[i];

        sal_Bool bVisible = rToolbar.bVisible;
        if ( implts_readWindowStateData( rToolbar.aName, aUIElement ) &&
             aUIElement.m_bVisible != bVisible )
        {
            WriteGuard aWriteLock( m_aLock );
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            UIElement& rUIElement = impl_findElement( rToolbar.aName );

            if ( rUIElement.m_aName == rToolbar.aName )
                rUIElement.m_bVisible = aUIElement.m_bVisible;
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            aWriteLock.unlock();
        }
    }
}

sal_Bool LayoutManager::implts_readWindowStateData( const rtl::OUString& aName, UIElement& rElementData )
{
    sal_Bool bGetSettingsState( sal_False );

    WriteGuard aWriteLock( m_aLock );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aWriteLock.unlock();

    if ( xPersistentWindowState.is() )
    {
        aWriteLock.lock();
        sal_Bool bGlobalSettings( m_bGlobalSettings );
        GlobalSettings* pGlobalSettings( 0 );
        if ( m_pGlobalSettings == 0 )
        {
            m_pGlobalSettings = new GlobalSettings( m_xSMGR );
            bGetSettingsState = sal_True;
        }
        pGlobalSettings = m_pGlobalSettings;
        aWriteLock.unlock();

        try
        {
            Sequence< PropertyValue > aWindowState;
            if ( xPersistentWindowState->getByName( aName ) >>= aWindowState )
            {
                sal_Bool bValue( sal_False );
                for ( sal_Int32 n = 0; n < aWindowState.getLength(); n++ )
                {
                    if ( aWindowState[n].Name == m_aPropDocked )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                    else if ( aWindowState[n].Name == m_aPropVisible )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bVisible = bValue;
                    }
                    else if ( aWindowState[n].Name == m_aPropDockingArea )
                    {
                        ::com::sun::star::ui::DockingArea eDockingArea;
                        if ( aWindowState[n].Value >>= eDockingArea )
                            rElementData.m_aDockedData.m_nDockedArea = sal_Int16( eDockingArea );
                    }
                    else if ( aWindowState[n].Name == m_aPropDockPos )
                    {
                        css::awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                        {
                            rElementData.m_aDockedData.m_aPos.X() = aPoint.X;
                            rElementData.m_aDockedData.m_aPos.Y() = aPoint.Y;
                        }
                    }
                    else if ( aWindowState[n].Name == m_aPropPos )
                    {
                        css::awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                        {
                            rElementData.m_aFloatingData.m_aPos.X() = aPoint.X;
                            rElementData.m_aFloatingData.m_aPos.Y() = aPoint.Y;
                        }
                    }
                    else if ( aWindowState[n].Name == m_aPropSize )
                    {
                        css::awt::Size aSize;
                        if ( aWindowState[n].Value >>= aSize )
                        {
                            rElementData.m_aFloatingData.m_aSize.Width() = aSize.Width;
                            rElementData.m_aFloatingData.m_aSize.Height() = aSize.Height;
                        }
                    }
                    else if ( aWindowState[n].Name == m_aPropUIName )
                        aWindowState[n].Value >>= rElementData.m_aUIName;
                    else if ( aWindowState[n].Name == m_aPropStyle )
                    {
                        sal_Int32 nStyle = 0;
                        if ( aWindowState[n].Value >>= nStyle )
                            rElementData.m_nStyle = sal_Int16( nStyle );
                    }
                    else if ( aWindowState[n].Name == m_aPropLocked )
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_aDockedData.m_bLocked = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(WINDOWSTATE_PROPERTY_CONTEXT)))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bContextSensitive = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(WINDOWSTATE_PROPERTY_NOCLOSE)))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bNoClose = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(WINDOWSTATE_PROPERTY_CONTEXTACTIVE)))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bContextActive = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(WINDOWSTATE_PROPERTY_SOFTCLOSE)))
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
                    WriteGuard aWriteLock2( m_aLock );
                    m_bGlobalSettings = sal_True;
                    aWriteLock2.unlock();

                    css::uno::Any aValue;
                    sal_Bool      bValue = sal_Bool();
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
        catch ( NoSuchElementException& )
        {
        }
    }

    return sal_False;
}

void LayoutManager::implts_writeWindowStateData( const rtl::OUString& aName, const UIElement& rElementData )
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
            xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ))) >>= bPersistent;
        }
        catch ( com::sun::star::beans::UnknownPropertyException )
        {
            // Non-configurable elements should at least store their dimension/position
            bPersistent = sal_True;
        }
        catch ( com::sun::star::lang::WrappedTargetException )
        {
        }
    }

    if ( bPersistent && xPersistentWindowState.is() )
    {
        try
        {
            Sequence< PropertyValue > aWindowState( 8 );

            aWindowState[0].Name    = m_aPropDocked;
            aWindowState[0].Value   = makeAny( sal_Bool( !rElementData.m_bFloating ));
            aWindowState[1].Name    = m_aPropVisible;
            aWindowState[1].Value   = makeAny( sal_Bool( rElementData.m_bVisible ));

            aWindowState[2].Name    = m_aPropDockingArea;
            aWindowState[2].Value   = makeAny( static_cast< DockingArea >( rElementData.m_aDockedData.m_nDockedArea ) );

            css::awt::Point aPos;
            aPos.X = rElementData.m_aDockedData.m_aPos.X();
            aPos.Y = rElementData.m_aDockedData.m_aPos.Y();
            aWindowState[3].Name    = m_aPropDockPos;
            aWindowState[3].Value   <<= aPos;

            aPos.X = rElementData.m_aFloatingData.m_aPos.X();
            aPos.Y = rElementData.m_aFloatingData.m_aPos.Y();
            aWindowState[4].Name    = m_aPropPos;
            aWindowState[4].Value   <<= aPos;

            css::awt::Size aSize;
            aSize.Width = rElementData.m_aFloatingData.m_aSize.Width();
            aSize.Height = rElementData.m_aFloatingData.m_aSize.Height();
            aWindowState[5].Name    = m_aPropSize;
            aWindowState[5].Value   <<= aSize;
            aWindowState[6].Name    = m_aPropUIName;
            aWindowState[6].Value   = makeAny( rElementData.m_aUIName );
            aWindowState[7].Name    = m_aPropLocked;
            aWindowState[7].Value   = makeAny( rElementData.m_aDockedData.m_bLocked );

            if ( xPersistentWindowState->hasByName( aName ))
            {
                Reference< XNameReplace > xReplace( xPersistentWindowState, UNO_QUERY );
                xReplace->replaceByName( aName, makeAny( aWindowState ));
            }
            else
            {
                Reference< XNameContainer > xInsert( xPersistentWindowState, UNO_QUERY );
                xInsert->insertByName( aName, makeAny( aWindowState ));
            }
        }
        catch ( Exception& )
        {
        }
    }

    // Reset flag
    aWriteLock.lock();
    m_bStoreWindowState = sal_False;
    aWriteLock.unlock();
}

void LayoutManager::implts_setElementData( UIElement& rElement, const Reference< css::awt::XDockableWindow >& rDockWindow )
{
    ReadGuard aReadLock( m_aLock );
    sal_Bool bShowElement( rElement.m_bVisible && !rElement.m_bMasterHide && m_bParentWindowVisible );
    aReadLock.unlock();

    Reference< css::awt::XDockableWindow > xDockWindow( rDockWindow );
    Reference< css::awt::XWindow2 >        xWindow( xDockWindow, UNO_QUERY );

    Window*     pWindow( 0 );
    ToolBox*    pToolBox( 0 );

    if ( xDockWindow.is() && xWindow.is() )
    {
        {
            SolarMutexGuard aGuard;
            pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow )
            {
                String aText = pWindow->GetText();
                if ( aText.Len() == 0 )
                    pWindow->SetText( rElement.m_aUIName );
                if ( rElement.m_bNoClose )
                    pWindow->SetStyle( pWindow->GetStyle() & ~WB_CLOSEABLE );
                if ( pWindow->GetType() == WINDOW_TOOLBOX )
                    pToolBox = (ToolBox *)pWindow;
            }
            if ( pToolBox )
            {
                if (( rElement.m_nStyle < 0 ) ||
                    ( rElement.m_nStyle > BUTTON_SYMBOLTEXT ))
                    rElement.m_nStyle = BUTTON_SYMBOL;
                    pToolBox->SetButtonType( (ButtonType)rElement.m_nStyle );
                if ( rElement.m_bNoClose )
                    pToolBox->SetFloatStyle( pToolBox->GetFloatStyle() & ~WB_CLOSEABLE );
            }
        }

        if ( rElement.m_bFloating )
        {
            if ( pWindow )
            {
                SolarMutexGuard aGuard;
                String aText = pWindow->GetText();
                if ( aText.Len() == 0 )
                    pWindow->SetText( rElement.m_aUIName );
            }

            ::Point  aPos( rElement.m_aFloatingData.m_aPos.X(),
                           rElement.m_aFloatingData.m_aPos.Y() );
            sal_Bool bWriteData( sal_False );
            sal_Bool bUndefPos = ( rElement.m_aFloatingData.m_aPos.X() == SAL_MAX_INT32 ||
                                   rElement.m_aFloatingData.m_aPos.Y() == SAL_MAX_INT32 );
            sal_Bool bSetSize = ( rElement.m_aFloatingData.m_aSize.Width() != 0 &&
                                  rElement.m_aFloatingData.m_aSize.Height() != 0 );
            xDockWindow->setFloatingMode( sal_True );
            if ( bUndefPos )
            {
                aPos = implts_findNextCascadeFloatingPos();
                rElement.m_aFloatingData.m_aPos = aPos; // set new cascaded position
                bWriteData = sal_True;
            }

            if( bSetSize )
                xWindow->setOutputSize( AWTSize( rElement.m_aFloatingData.m_aSize ) );
            else
            {
                if( pToolBox )
                {
                    // set an optimal initial floating size
                    SolarMutexGuard aGuard;
                    ::Size aSize( pToolBox->CalcFloatingWindowSizePixel() );
                    pToolBox->SetOutputSizePixel( aSize );
                }
            }

            // #i60882# IMPORTANT: Set position after size as it is
            // possible that we position some part of the toolbar
            // outside of the desktop. A default constructed toolbar
            // always has one line. Now VCL automatically
            // position the toolbar back into the desktop. Therefore
            // we resize the toolbar with the new (wrong) position.
            // To fix this problem we have to set the size BEFORE the
            // position.
            xWindow->setPosSize( aPos.X(), aPos.Y(), 0, 0,
                                 css::awt::PosSize::POS );

            if ( bWriteData )
                implts_writeWindowStateData( rElement.m_aName, rElement );
            if ( bShowElement && pWindow )
            {
                SolarMutexGuard aGuard;
                pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
        }
        else
        {
            ::Point  aDockPos;
            ::Point  aPixelPos;
            sal_Bool bSetSize( sal_False );
            ::Size   aSize;

            if ( pToolBox )
            {
                SolarMutexGuard aGuard;
                pToolBox->SetAlign( ImplConvertAlignment(rElement.m_aDockedData.m_nDockedArea )  );
                pToolBox->SetLineCount( 1 );
                if ( rElement.m_aDockedData.m_bLocked )
                    xDockWindow->lock();
                aSize = pToolBox->CalcWindowSizePixel();
                bSetSize = sal_True;

                if (( rElement.m_aDockedData.m_aPos.X() == SAL_MAX_INT32 ) &&
                    ( rElement.m_aDockedData.m_aPos.Y() == SAL_MAX_INT32 ))
                {
                    implts_findNextDockingPos( (DockingArea)rElement.m_aDockedData.m_nDockedArea,
                                               aSize,
                                               aDockPos,
                                               aPixelPos );
                    rElement.m_aDockedData.m_aPos = aDockPos;
                }
            }

            xWindow->setPosSize( aPixelPos.X(),
                                 aPixelPos.Y(),
                                 0, 0,
                                 css::awt::PosSize::POS );
            if( bSetSize )
                xWindow->setOutputSize( AWTSize( aSize) );

            if ( bShowElement && pWindow )
            {
                SolarMutexGuard aGuard;
                pWindow->Show( sal_True );
            }
        }
    }
}

::Point LayoutManager::implts_findNextCascadeFloatingPos()
{
    const sal_Int32 nHotZoneX       = 50;
    const sal_Int32 nHotZoneY       = 50;
    const sal_Int32 nCascadeIndentX = 15;
    const sal_Int32 nCascadeIndentY = 15;

    ReadGuard aReadLock( m_aLock );
    Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
    Reference< css::awt::XWindow > xTopDockingWindow = m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP];
    Reference< css::awt::XWindow > xLeftDockingWindow = m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT];
    aReadLock.unlock();

    ::Point aStartPos( nCascadeIndentX, nCascadeIndentY );
    ::Point aCurrPos( aStartPos );
    css::awt::Rectangle aRect;

    Window* pContainerWindow( 0 );
    if ( xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;
        pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        if ( pContainerWindow )
            aStartPos = pContainerWindow->OutputToScreenPixel( aStartPos );
    }

    // Determine size of top and left docking area
    css::awt::Rectangle aTopRect  = xTopDockingWindow->getPosSize();
    css::awt::Rectangle aLeftRect = xLeftDockingWindow->getPosSize();

    aStartPos.X() += aLeftRect.Width + nCascadeIndentX;
    aStartPos.Y() += aTopRect.Height + nCascadeIndentY;
    aCurrPos = aStartPos;

    // Try to find a cascaded position for the new floating window
    UIElementVector::const_iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XWindow > xWindow( xDockWindow, UNO_QUERY );
            if ( xDockWindow.is() && xDockWindow->isFloating() )
            {
                SolarMutexGuard aGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->IsVisible() )
                {
                    css::awt::Rectangle aFloatRect = xWindow->getPosSize();
                    if ((( aFloatRect.X - nHotZoneX ) <= aCurrPos.X() ) &&
                        ( aFloatRect.X >= aCurrPos.X() ) &&
                        (( aFloatRect.Y - nHotZoneY ) <= aCurrPos.Y() ) &&
                        ( aFloatRect.Y >= aCurrPos.Y() ))
                    {
                        aCurrPos.X() = aFloatRect.X + nCascadeIndentX;
                        aCurrPos.Y() = aFloatRect.Y + nCascadeIndentY;
                    }
                }
            }
        }
    }

    return aCurrPos;
}

void LayoutManager::implts_findNextDockingPos( DockingArea DockingArea, const ::Size& aUIElementSize, ::Point& rVirtualPos, ::Point& rPixelPos )
{
    ReadGuard aReadLock( m_aLock );
    Reference< css::awt::XWindow > xDockingWindow = m_xDockAreaWindows[DockingArea];
    ::Size                         aDockingWinSize;
    Window*                        pDockingWindow( 0 );
    aReadLock.unlock();

    if (( DockingArea < DockingArea_DOCKINGAREA_TOP ) ||
        ( DockingArea > DockingArea_DOCKINGAREA_RIGHT ))
        DockingArea = DockingArea_DOCKINGAREA_TOP;

    {
        // Retrieve output size from container Window
        SolarMutexGuard aGuard;
        pDockingWindow  = VCLUnoHelper::GetWindow( xDockingWindow );
        if ( pDockingWindow )
            aDockingWinSize = pDockingWindow->GetOutputSizePixel();
    }

    sal_Int32 nFreeRowColPixelPos( 0 );
    sal_Int32 nMaxSpace( 0 );
    sal_Int32 nNeededSpace( 0 );
    sal_Int32 nTopDockingAreaSize( 0 );

    if (( DockingArea == DockingArea_DOCKINGAREA_TOP ) ||
        ( DockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
    {
        nMaxSpace    = aDockingWinSize.Width();
        nNeededSpace = aUIElementSize.Width();
    }
    else
    {
        nMaxSpace              = aDockingWinSize.Height();
        nNeededSpace           = aUIElementSize.Height();
        nTopDockingAreaSize    = implts_getTopBottomDockingAreaSizes().Width();
    }

    std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

    implts_getDockingAreaElementInfos( DockingArea, aRowColumnsWindowData );
    sal_Int32 nPixelPos( 0 );
    const sal_uInt32 nCount = aRowColumnsWindowData.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        SingleRowColumnWindowData& rRowColumnWindowData = aRowColumnsWindowData[i];

        if (( DockingArea == DockingArea_DOCKINGAREA_BOTTOM ) ||
            ( DockingArea == DockingArea_DOCKINGAREA_RIGHT  ))
            nPixelPos += rRowColumnWindowData.nStaticSize;

        if ((( nMaxSpace - rRowColumnWindowData.nVarSize ) >= nNeededSpace ) ||
            ( rRowColumnWindowData.nSpace >= nNeededSpace ))
        {
            // Check current row where we can find the needed space
            sal_Int32 nCurrPos( 0 );
            const sal_uInt32 nWindowSizesCount = rRowColumnWindowData.aRowColumnWindowSizes.size();
            for ( sal_uInt32 j = 0; j < nWindowSizesCount; j++ )
            {
                css::awt::Rectangle rRect   = rRowColumnWindowData.aRowColumnWindowSizes[j];
                sal_Int32&          rSpace  = rRowColumnWindowData.aRowColumnSpace[j];
                if (( DockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                    ( DockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
                {
                    if ( rSpace >= nNeededSpace )
                    {
                        rVirtualPos = ::Point( nCurrPos, rRowColumnWindowData.nRowColumn );
                        if ( DockingArea == DockingArea_DOCKINGAREA_TOP )
                            rPixelPos   = ::Point( nCurrPos, nPixelPos );
                        else
                            rPixelPos   = ::Point( nCurrPos, aDockingWinSize.Height() - nPixelPos );
                        return;
                    }
                    nCurrPos = rRect.X + rRect.Width;
                }
                else
                {
                    if ( rSpace >= nNeededSpace )
                    {
                        rVirtualPos = ::Point( rRowColumnWindowData.nRowColumn, nCurrPos );
                        if ( DockingArea == DockingArea_DOCKINGAREA_LEFT )
                            rPixelPos   = ::Point( nPixelPos, nTopDockingAreaSize + nCurrPos );
                        else
                            rPixelPos   = ::Point( aDockingWinSize.Width() - nPixelPos , nTopDockingAreaSize + nCurrPos );
                        return;
                    }
                    nCurrPos = rRect.Y + rRect.Height;
                }
            }

            if (( nCurrPos + nNeededSpace ) <= nMaxSpace )
            {
                if (( DockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                    ( DockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
                {
                    rVirtualPos = ::Point( nCurrPos, rRowColumnWindowData.nRowColumn );
                    if ( DockingArea == DockingArea_DOCKINGAREA_TOP )
                        rPixelPos   = ::Point( nCurrPos, nPixelPos );
                    else
                        rPixelPos   = ::Point( nCurrPos, aDockingWinSize.Height() - nPixelPos );
                    return;
                }
                else
                {
                    rVirtualPos = ::Point( rRowColumnWindowData.nRowColumn, nCurrPos );
                    if ( DockingArea == DockingArea_DOCKINGAREA_LEFT )
                        rPixelPos   = ::Point( nPixelPos, nTopDockingAreaSize + nCurrPos );
                    else
                        rPixelPos   = ::Point( aDockingWinSize.Width() - nPixelPos , nTopDockingAreaSize + nCurrPos );
                    return;
                }
            }
        }

        if (( DockingArea == DockingArea_DOCKINGAREA_TOP ) ||
            ( DockingArea == DockingArea_DOCKINGAREA_LEFT  ))
            nPixelPos += rRowColumnWindowData.nStaticSize;
    }

    sal_Int32 nNextFreeRowCol( 0 );
    sal_Int32 nRowColumnsCount = aRowColumnsWindowData.size();
    if ( nRowColumnsCount > 0 )
        nNextFreeRowCol = aRowColumnsWindowData[nRowColumnsCount-1].nRowColumn+1;
    else
        nNextFreeRowCol = 0;

    if ( nNextFreeRowCol == 0 )
    {
        if ( DockingArea == DockingArea_DOCKINGAREA_BOTTOM )
            nFreeRowColPixelPos = aDockingWinSize.Height() - aUIElementSize.Height();
        else if ( DockingArea == DockingArea_DOCKINGAREA_RIGHT  )
            nFreeRowColPixelPos = aDockingWinSize.Width() - aUIElementSize.Width();
    }

    if (( DockingArea == DockingArea_DOCKINGAREA_TOP ) ||
        ( DockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
    {
        rVirtualPos = ::Point( 0, nNextFreeRowCol );
        if ( DockingArea == DockingArea_DOCKINGAREA_TOP )
            rPixelPos = ::Point( 0, nFreeRowColPixelPos );
        else
            rPixelPos = ::Point( 0, aDockingWinSize.Height() - nFreeRowColPixelPos );
    }
    else
    {
        rVirtualPos = ::Point( nNextFreeRowCol, 0 );
        rPixelPos   = ::Point( aDockingWinSize.Width() - nFreeRowColPixelPos, 0 );
    }
}

::Size LayoutManager::implts_getContainerWindowOutputSize()
{
    ReadGuard aReadLock( m_aLock );
    Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
    ::Size                         aContainerWinSize;
    Window* pContainerWindow( 0 );
    aReadLock.unlock();

    // Retrieve output size from container Window
    SolarMutexGuard aGuard;
    pContainerWindow  = VCLUnoHelper::GetWindow( xContainerWindow );
    if ( pContainerWindow )
        return pContainerWindow->GetOutputSizePixel();
    else
        return ::Size();
}

void LayoutManager::implts_sortUIElements()
{
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIterStart = m_aUIElements.begin();
    UIElementVector::iterator pIterEnd   = m_aUIElements.end();

    std::stable_sort( pIterStart, pIterEnd ); // first created element should first

    // We have to reset our temporary flags.
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        // why check, just set it to false
        //if ( pIter->m_bUserActive )
            pIter->m_bUserActive = sal_False;
    }

#ifdef DBG_UTIL
    implts_checkElementContainer();
#endif
    aWriteLock.unlock();
}

void LayoutManager::implts_getDockingAreaElementInfos( DockingArea eDockingArea, std::vector< SingleRowColumnWindowData >& rRowColumnsWindowData )
{
    std::vector< UIElement > aWindowVector;

    if (( eDockingArea < DockingArea_DOCKINGAREA_TOP ) ||
        ( eDockingArea > DockingArea_DOCKINGAREA_RIGHT ))
        eDockingArea = DockingArea_DOCKINGAREA_TOP;

    Reference< css::awt::XWindow > xDockAreaWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    aWindowVector.reserve(m_aUIElements.size());
    xDockAreaWindow = m_xDockAreaWindows[eDockingArea];
    UIElementVector::iterator   pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aDockedData.m_nDockedArea == eDockingArea && pIter->m_bVisible && !pIter->m_bFloating )
        {
            Reference< XUIElement > xUIElement( pIter->m_xUIElement );
            if ( xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xDockWindow.is() )
                {
                    // docked windows
                    aWindowVector.push_back( *pIter );
                }
            }
        }
    }
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    rRowColumnsWindowData.clear();

    // Collect data from windows that are on the same row/column
    sal_Int32 j;
    sal_Int32 nIndex( 0 );
    sal_Int32 nLastPos( 0 );
    sal_Int32 nCurrPos( -1 );
    sal_Int32 nLastRowColPixelPos( 0 );
    css::awt::Rectangle aDockAreaRect;

    if ( xDockAreaWindow.is() )
        aDockAreaRect = xDockAreaWindow->getPosSize();

    if ( eDockingArea == DockingArea_DOCKINGAREA_TOP )
        nLastRowColPixelPos = 0;
    else if ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM )
        nLastRowColPixelPos = aDockAreaRect.Height;
    else if ( eDockingArea == DockingArea_DOCKINGAREA_LEFT )
        nLastRowColPixelPos = 0;
    else
        nLastRowColPixelPos = aDockAreaRect.Width;

    const sal_uInt32 nCount = aWindowVector.size();
    for ( j = 0; j < sal_Int32( nCount); j++ )
    {
        const UIElement& rElement = aWindowVector[j];
        Reference< css::awt::XWindow > xWindow;
        Reference< XUIElement > xUIElement( rElement.m_xUIElement );
        css::awt::Rectangle aPosSize;
        if ( !lcl_checkUIElement(xUIElement,aPosSize,xWindow) )
            continue;
        if (( eDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
            ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
        {
            if ( nCurrPos == -1 )
            {
                nCurrPos = rElement.m_aDockedData.m_aPos.Y();
                nLastPos = 0;

                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            sal_Int32 nSpace( 0 );
            if ( rElement.m_aDockedData.m_aPos.Y() != nCurrPos )
            {
                if ( eDockingArea == DockingArea_DOCKINGAREA_TOP )
                    nLastRowColPixelPos += rRowColumnsWindowData[nIndex].nStaticSize;
                else
                    nLastRowColPixelPos -= rRowColumnsWindowData[nIndex].nStaticSize;
                ++nIndex;
                nLastPos = 0;
                nCurrPos = rElement.m_aDockedData.m_aPos.Y();
                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.X() - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.X() >= nLastPos )
            {
                rRowColumnsWindowData[nIndex].nSpace += nSpace;
                nLastPos = rElement.m_aDockedData.m_aPos.X() + aPosSize.Width;
            }
            else
            {
                nSpace = 0;
                nLastPos += aPosSize.Width;
            }
            rRowColumnsWindowData[nIndex].aRowColumnSpace.push_back( nSpace );

            rRowColumnsWindowData[nIndex].aRowColumnWindows.push_back( xWindow );
            rRowColumnsWindowData[nIndex].aUIElementNames.push_back( rElement.m_aName );
            rRowColumnsWindowData[nIndex].aRowColumnWindowSizes.push_back(
                css::awt::Rectangle( rElement.m_aDockedData.m_aPos.X(),
                                     rElement.m_aDockedData.m_aPos.Y(),
                                     aPosSize.Width,
                                     aPosSize.Height ));
            if ( rRowColumnsWindowData[nIndex].nStaticSize < aPosSize.Height )
                rRowColumnsWindowData[nIndex].nStaticSize = aPosSize.Height;
            if ( eDockingArea == DockingArea_DOCKINGAREA_TOP )
                rRowColumnsWindowData[nIndex].aRowColumnRect = css::awt::Rectangle( 0, nLastRowColPixelPos,
                                                                                    aDockAreaRect.Width, aPosSize.Height );
            else
                rRowColumnsWindowData[nIndex].aRowColumnRect = css::awt::Rectangle( 0, ( nLastRowColPixelPos - aPosSize.Height ),
                                                                                    aDockAreaRect.Width, aPosSize.Height );
            rRowColumnsWindowData[nIndex].nVarSize += aPosSize.Width + nSpace;
        }
        else
        {
            if ( nCurrPos == -1 )
            {
                nCurrPos = rElement.m_aDockedData.m_aPos.X();
                nLastPos = 0;

                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            sal_Int32 nSpace( 0 );
            if ( rElement.m_aDockedData.m_aPos.X() != nCurrPos )
            {
                if ( eDockingArea == DockingArea_DOCKINGAREA_LEFT )
                    nLastRowColPixelPos += rRowColumnsWindowData[nIndex].nStaticSize;
                else
                    nLastRowColPixelPos -= rRowColumnsWindowData[nIndex].nStaticSize;
                ++nIndex;
                nLastPos = 0;
                nCurrPos = rElement.m_aDockedData.m_aPos.X();
                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.Y() - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.Y() > nLastPos )
            {
                rRowColumnsWindowData[nIndex].nSpace += nSpace;
                nLastPos = rElement.m_aDockedData.m_aPos.Y() + aPosSize.Height;
            }
            else
            {
                nSpace = 0;
                nLastPos += aPosSize.Height;
            }
            rRowColumnsWindowData[nIndex].aRowColumnSpace.push_back( nSpace );

            rRowColumnsWindowData[nIndex].aRowColumnWindows.push_back( xWindow );
            rRowColumnsWindowData[nIndex].aUIElementNames.push_back( rElement.m_aName );
            rRowColumnsWindowData[nIndex].aRowColumnWindowSizes.push_back(
                css::awt::Rectangle( rElement.m_aDockedData.m_aPos.X(),
                                     rElement.m_aDockedData.m_aPos.Y(),
                                     aPosSize.Width,
                                     aPosSize.Height ));
            if ( rRowColumnsWindowData[nIndex].nStaticSize < aPosSize.Width )
                rRowColumnsWindowData[nIndex].nStaticSize = aPosSize.Width;
            if ( eDockingArea == DockingArea_DOCKINGAREA_LEFT )
                rRowColumnsWindowData[nIndex].aRowColumnRect = css::awt::Rectangle( nLastRowColPixelPos, 0,
                                                                                    aPosSize.Width, aDockAreaRect.Height );
            else
                rRowColumnsWindowData[nIndex].aRowColumnRect = css::awt::Rectangle( ( nLastRowColPixelPos - aPosSize.Width ), 0,
                                                                                    aPosSize.Width, aDockAreaRect.Height );
            rRowColumnsWindowData[nIndex].nVarSize += aPosSize.Height + nSpace;
        }
    }
}

void LayoutManager::implts_getDockingAreaElementInfoOnSingleRowCol( DockingArea eDockingArea, sal_Int32 nRowCol, SingleRowColumnWindowData& rRowColumnWindowData )
{
    std::vector< UIElement > aWindowVector;

    if (( eDockingArea < DockingArea_DOCKINGAREA_TOP ) ||
        ( eDockingArea > DockingArea_DOCKINGAREA_RIGHT ))
        eDockingArea = DockingArea_DOCKINGAREA_TOP;

    sal_Bool bHorzDockArea = (( eDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                              ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ));

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    UIElementVector::iterator   pIter;
    UIElementVector::iterator   pEnd = m_aUIElements.end();
    for ( pIter = m_aUIElements.begin(); pIter != pEnd; ++pIter )
    {
        if ( pIter->m_aDockedData.m_nDockedArea == eDockingArea )
        {
            sal_Bool                bSameRowCol = bHorzDockArea ?
                                                   ( pIter->m_aDockedData.m_aPos.Y() == nRowCol ) :
                                                   ( pIter->m_aDockedData.m_aPos.X() == nRowCol );
            Reference< XUIElement > xUIElement( pIter->m_xUIElement );

            if ( bSameRowCol && xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                if ( xWindow.is() )
                {
                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                    if ( pWindow && pIter->m_bVisible && xDockWindow.is() && !pIter->m_bFloating )
                    {
                        // docked windows
                        aWindowVector.push_back( *pIter );
                    }
                }
            }
        }
    }
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // Initialize structure
    rRowColumnWindowData.aUIElementNames.clear();
    rRowColumnWindowData.aRowColumnWindows.clear();
    rRowColumnWindowData.aRowColumnWindowSizes.clear();
    rRowColumnWindowData.aRowColumnSpace.clear();
    rRowColumnWindowData.nVarSize = 0;
    rRowColumnWindowData.nStaticSize = 0;
    rRowColumnWindowData.nSpace = 0;
    rRowColumnWindowData.nRowColumn = nRowCol;

    // Collect data from windows that are on the same row/column
    sal_Int32 j;
    sal_Int32 nLastPos( 0 );

    const sal_uInt32 nCount = aWindowVector.size();
    for ( j = 0; j < sal_Int32( nCount); j++ )
    {
        const UIElement& rElement = aWindowVector[j];
        Reference< css::awt::XWindow > xWindow;
        Reference< XUIElement > xUIElement( rElement.m_xUIElement );
        css::awt::Rectangle aPosSize;
        if ( !lcl_checkUIElement(xUIElement,aPosSize,xWindow) )
            continue;

        sal_Int32 nSpace;
        if (( eDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
            ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
        {
            nSpace = ( rElement.m_aDockedData.m_aPos.X() - nLastPos );

            // Calc space before an element and store it
            if ( rElement.m_aDockedData.m_aPos.X() > nLastPos )
                rRowColumnWindowData.nSpace += nSpace;
            else
                nSpace = 0;

            nLastPos = rElement.m_aDockedData.m_aPos.X() + aPosSize.Width;


            rRowColumnWindowData.aRowColumnWindowSizes.push_back(
                css::awt::Rectangle( rElement.m_aDockedData.m_aPos.X(),
                                     rElement.m_aDockedData.m_aPos.Y(),
                                     aPosSize.Width,
                                     aPosSize.Height ));
            if ( rRowColumnWindowData.nStaticSize < aPosSize.Height )
                rRowColumnWindowData.nStaticSize = aPosSize.Height;
            rRowColumnWindowData.nVarSize += aPosSize.Width;
        }
        else
        {
            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.Y() - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.Y() > nLastPos )
                rRowColumnWindowData.nSpace += nSpace;
            else
                nSpace = 0;

            nLastPos = rElement.m_aDockedData.m_aPos.Y() + aPosSize.Height;

            rRowColumnWindowData.aRowColumnWindowSizes.push_back(
                css::awt::Rectangle( rElement.m_aDockedData.m_aPos.X(),
                                     rElement.m_aDockedData.m_aPos.Y(),
                                     aPosSize.Width,
                                     aPosSize.Height ));
            if ( rRowColumnWindowData.nStaticSize < aPosSize.Width )
                rRowColumnWindowData.nStaticSize = aPosSize.Width;
            rRowColumnWindowData.nVarSize += aPosSize.Height;
        }

        rRowColumnWindowData.aUIElementNames.push_back( rElement.m_aName );
        rRowColumnWindowData.aRowColumnWindows.push_back( xWindow );
        rRowColumnWindowData.aRowColumnSpace.push_back( nSpace );
        rRowColumnWindowData.nVarSize += nSpace;
    }
}

::Rectangle LayoutManager::implts_determineFrontDockingRect(
    DockingArea            eDockingArea,
    sal_Int32              nRowCol,
    const ::Rectangle&     rDockedElementRect,
    const ::rtl::OUString& rMovedElementName,
    const ::Rectangle&     rMovedElementRect )
{
    SingleRowColumnWindowData aRowColumnWindowData;

    sal_Bool bHorzDockArea = (( eDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                              ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ));

    implts_getDockingAreaElementInfoOnSingleRowCol( eDockingArea, nRowCol, aRowColumnWindowData );
    if ( aRowColumnWindowData.aRowColumnWindows.empty() )
        return rMovedElementRect;
    else
    {
        sal_Int32 nSpace( 0 );
        ::Rectangle aFrontDockingRect( rMovedElementRect );
        const sal_uInt32 nCount = aRowColumnWindowData.aRowColumnWindows.size();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            if ( bHorzDockArea )
            {
                if ( aRowColumnWindowData.aRowColumnWindowSizes[i].X >= rDockedElementRect.Left() )
                {
                    nSpace += aRowColumnWindowData.aRowColumnSpace[i];
                    break;
                }
                else if ( aRowColumnWindowData.aUIElementNames[i] == rMovedElementName )
                    nSpace += aRowColumnWindowData.aRowColumnWindowSizes[i].Width +
                              aRowColumnWindowData.aRowColumnSpace[i];
                else
                    nSpace = 0;
            }
            else
            {
                if ( aRowColumnWindowData.aRowColumnWindowSizes[i].Y >= rDockedElementRect.Top() )
                {
                    nSpace += aRowColumnWindowData.aRowColumnSpace[i];
                    break;
                }
                else if ( aRowColumnWindowData.aUIElementNames[i] == rMovedElementName )
                    nSpace += aRowColumnWindowData.aRowColumnWindowSizes[i].Height +
                              aRowColumnWindowData.aRowColumnSpace[i];
                else
                    nSpace = 0;
            }
        }

        if ( nSpace > 0 )
        {
            sal_Int32 nMove = std::min( nSpace, static_cast<sal_Int32>(aFrontDockingRect.getWidth()) );
            if ( bHorzDockArea )
                aFrontDockingRect.Move( -nMove, 0 );
            else
                aFrontDockingRect.Move( 0, -nMove );
        }

        return aFrontDockingRect;
    }
}

::Rectangle LayoutManager::implts_getWindowRectFromRowColumn(
    ::com::sun::star::ui::DockingArea DockingArea,
    const SingleRowColumnWindowData& rRowColumnWindowData,
    const ::Point& rMousePos,
    const rtl::OUString& rExcludeElementName )
{
    ::Rectangle aWinRect;

    if (( DockingArea < DockingArea_DOCKINGAREA_TOP ) ||
        ( DockingArea > DockingArea_DOCKINGAREA_RIGHT ))
        DockingArea = DockingArea_DOCKINGAREA_TOP;

    if ( rRowColumnWindowData.aRowColumnWindows.empty() )
        return aWinRect;
    else
    {
        ReadGuard aReadLock( m_aLock );
        Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
        Reference< css::awt::XWindow > xDockingAreaWindow = m_xDockAreaWindows[DockingArea];
        aReadLock.unlock();

        // Calc correct position of the column/row rectangle to be able to compare it with mouse pos/tracking rect
        SolarMutexGuard aGuard;

        // Retrieve output size from container Window
        Window* pContainerWindow( VCLUnoHelper::GetWindow( xContainerWindow ));
        Window* pDockingAreaWindow( VCLUnoHelper::GetWindow( xDockingAreaWindow ));
        if ( pDockingAreaWindow && pContainerWindow )
        {
            const sal_uInt32 nCount = rRowColumnWindowData.aRowColumnWindows.size();
            for ( sal_uInt32 i = 0; i < nCount; i++ )
            {
                css::awt::Rectangle aWindowRect = rRowColumnWindowData.aRowColumnWindows[i]->getPosSize();
                ::Rectangle aRect( aWindowRect.X, aWindowRect.Y, aWindowRect.X+aWindowRect.Width, aWindowRect.Y+aWindowRect.Height );
                aRect.SetPos( pContainerWindow->ScreenToOutputPixel( pDockingAreaWindow->OutputToScreenPixel( aRect.TopLeft() )));
                if ( aRect.IsInside( rMousePos ))
                {
                    // Check if we have found the excluded element. If yes, we have to provide an empty rectangle.
                    // We prevent that a toolbar cannot be moved when the mouse pointer is inside its own rectangle!
                    if ( rExcludeElementName != rRowColumnWindowData.aUIElementNames[i] )
                        return aRect;
                    else
                        break;
                }
            }
        }
    }

    return aWinRect;
}

framework::LayoutManager::DockingOperation
LayoutManager::implts_determineDockingOperation(
    ::com::sun::star::ui::DockingArea DockingArea,
    const ::Rectangle&                        rRowColRect,
    const Point&                              rMousePos )
{
    const sal_Int32 nHorzVerticalRegionSize        = 6;
    const sal_Int32 nHorzVerticalMoveRegion        = 4;

    if ( rRowColRect.IsInside( rMousePos ))
    {
        if (( DockingArea == DockingArea_DOCKINGAREA_TOP ) ||
            ( DockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
        {
            sal_Int32 nRegion = rRowColRect.getHeight() / nHorzVerticalRegionSize;
            sal_Int32 nPosY   = rRowColRect.Top() + nRegion;

            if ( rMousePos.Y() < nPosY )
                return ( DockingArea == DockingArea_DOCKINGAREA_TOP ) ? DOCKOP_BEFORE_COLROW : DOCKOP_AFTER_COLROW;
            else if ( rMousePos.Y() < ( nPosY + nRegion*nHorzVerticalMoveRegion ))
                return DOCKOP_ON_COLROW;
            else
                return ( DockingArea == DockingArea_DOCKINGAREA_TOP ) ? DOCKOP_AFTER_COLROW : DOCKOP_BEFORE_COLROW;
        }
        else
        {
            sal_Int32 nRegion = rRowColRect.getWidth() / nHorzVerticalRegionSize;
            sal_Int32 nPosX   = rRowColRect.Left() + nRegion;

            if ( rMousePos.X() < nPosX )
                return ( DockingArea == DockingArea_DOCKINGAREA_LEFT ) ? DOCKOP_BEFORE_COLROW : DOCKOP_AFTER_COLROW;
            else if ( rMousePos.X() < ( nPosX + nRegion*nHorzVerticalMoveRegion ))
                return DOCKOP_ON_COLROW;
            else
                return ( DockingArea == DockingArea_DOCKINGAREA_LEFT ) ? DOCKOP_AFTER_COLROW : DOCKOP_BEFORE_COLROW;
        }
    }
    else
        return DOCKOP_ON_COLROW;
}

::Rectangle LayoutManager::implts_calcTrackingAndElementRect(
    ::com::sun::star::ui::DockingArea eDockingArea,
    sal_Int32 nRowCol,
    UIElement& rUIElement,
    const ::Rectangle& rTrackingRect,
    const ::Rectangle& rRowColumnRect,
    const ::Size& rContainerWinSize )
{
    sal_Bool bHorizontalDockArea( ( eDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                                  ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ));

    sal_Int32 nTopDockingAreaSize( implts_getTopBottomDockingAreaSizes().Width() );
    sal_Int32 nBottomDockingAreaSize( implts_getTopBottomDockingAreaSizes().Height() );

    ::Size  aStatusBarSize( implts_getStatusBarSize() );
    sal_Int32 nMaxLeftRightDockAreaSize = rContainerWinSize.Height() -
                                          nTopDockingAreaSize -
                                          nBottomDockingAreaSize -
                                          aStatusBarSize.Height();

    ::Rectangle aTrackingRect( rTrackingRect );
    if ( bHorizontalDockArea )
    {
        sal_Int32 nPosX( std::max( sal_Int32( rTrackingRect.Left()), sal_Int32( 0 )));
        if (( nPosX + rTrackingRect.getWidth()) > rContainerWinSize.Width() )
            nPosX = std::min( nPosX,
                                std::max( sal_Int32( rContainerWinSize.Width() - rTrackingRect.getWidth() ),
                                          sal_Int32( 0 )));

        sal_Int32 nSize = std::min( rContainerWinSize.Width(), rTrackingRect.getWidth() );

        aTrackingRect.SetPos( ::Point( nPosX, rRowColumnRect.Top() ));
        aTrackingRect.setWidth( nSize );
        aTrackingRect.setHeight( rRowColumnRect.getHeight() );

        // Set virtual position
        rUIElement.m_aDockedData.m_aPos.X() = nPosX;
        rUIElement.m_aDockedData.m_aPos.Y() = nRowCol;
    }
    else
    {
        sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                    sal_Int32( nMaxLeftRightDockAreaSize ));

        sal_Int32 nPosY( std::max( sal_Int32( aTrackingRect.Top()), sal_Int32( nTopDockingAreaSize )));
        if (( nPosY + aTrackingRect.getHeight()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
            nPosY = std::min( nPosY,
                                std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aTrackingRect.getHeight() )),
                                        sal_Int32( nTopDockingAreaSize )));

        sal_Int32 nSize = std::min( nMaxDockingAreaHeight, static_cast<sal_Int32>(aTrackingRect.getHeight()) );

        aTrackingRect.SetPos( ::Point( rRowColumnRect.Left(), nPosY ));
        aTrackingRect.setWidth( rRowColumnRect.getWidth() );
        aTrackingRect.setHeight( nSize );

        ReadGuard aReadLock( m_aLock );
        Reference< css::awt::XWindow > xDockingAreaWindow = m_xDockAreaWindows[eDockingArea];
        Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
        aReadLock.unlock();

        sal_Int32 nDockPosY( 0 );
        Window* pDockingAreaWindow( 0 );
        Window* pContainerWindow( 0 );
        {
            SolarMutexGuard aGuard;
            pDockingAreaWindow = VCLUnoHelper::GetWindow( xDockingAreaWindow );
            pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            nDockPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                pContainerWindow->OutputToScreenPixel( ::Point( 0, nPosY ))).Y();
        }

        // Set virtual position
        rUIElement.m_aDockedData.m_aPos.X() = nRowCol;
        rUIElement.m_aDockedData.m_aPos.Y() = nDockPosY;
    }

    return aTrackingRect;
}

void implts_setTrackingRect( DockingArea eDockingArea, const Point& rMousePos, ::Rectangle& rTrackingRect )
{
    sal_Bool bHorizontalDockArea( ( eDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                                  ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ));

    ::Point aPoint = rTrackingRect.TopLeft();
    if ( bHorizontalDockArea )
        aPoint.X() = rMousePos.X();
    else
        aPoint.Y() = rMousePos.Y();
    rTrackingRect.SetPos( aPoint );
}

void LayoutManager::implts_calcDockingPosSize(
    UIElement&          rUIElement,
    DockingOperation&   rDockingOperation,
    ::Rectangle&        rTrackingRect,
    const Point&        rMousePos )
{
    ReadGuard aReadLock( m_aLock );
    Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
    ::Size                         aContainerWinSize;
    Window* pContainerWindow( 0 );
    aReadLock.unlock();

    {
        // Retrieve output size from container Window
        SolarMutexGuard aGuard;
        pContainerWindow  = VCLUnoHelper::GetWindow( xContainerWindow );
        aContainerWinSize = pContainerWindow->GetOutputSizePixel();
    }

    if ( !rUIElement.m_xUIElement.is() )
    {
        rTrackingRect = ::Rectangle();
        return;
    }

    Window*                                 pDockWindow( 0 );
    Window*                                 pDockingAreaWindow( 0 );
    ToolBox*                                pToolBox( 0 );
    Reference< css::awt::XWindow >          xWindow( rUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
    Reference< css::awt::XWindow >          xDockingAreaWindow;
    ::Rectangle                             aTrackingRect( rTrackingRect );
    ::com::sun::star::ui::DockingArea eDockedArea( (::com::sun::star::ui::DockingArea)rUIElement.m_aDockedData.m_nDockedArea );
    sal_Int32                               nTopDockingAreaSize( implts_getTopBottomDockingAreaSizes().Width() );
    sal_Int32                               nBottomDockingAreaSize( implts_getTopBottomDockingAreaSizes().Height() );
    sal_Bool                                bHorizontalDockArea( ( eDockedArea == DockingArea_DOCKINGAREA_TOP ) ||
                                                                 ( eDockedArea == DockingArea_DOCKINGAREA_BOTTOM ));
    ::Size                                  aStatusBarSize( implts_getStatusBarSize() );
    sal_Int32                               nMaxLeftRightDockAreaSize = aContainerWinSize.Height() -
                                                                        nTopDockingAreaSize -
                                                                        nBottomDockingAreaSize -
                                                                        aStatusBarSize.Height();
    ::Rectangle                             aDockingAreaRect;

    aReadLock.lock();
    xDockingAreaWindow = m_xDockAreaWindows[eDockedArea];
    aReadLock.unlock();

    {
        SolarMutexGuard aGuard;
        pDockingAreaWindow = VCLUnoHelper::GetWindow( xDockingAreaWindow );
        pDockWindow        = VCLUnoHelper::GetWindow( xWindow );
        if ( pDockWindow && pDockWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pDockWindow;

        aDockingAreaRect = ::Rectangle( pDockingAreaWindow->GetPosPixel(), pDockingAreaWindow->GetSizePixel() );
        if ( pToolBox )
        {
            // docked toolbars always have one line
            ::Size aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( sal_Int16( eDockedArea )) );
            aTrackingRect.SetSize( ::Size( aSize.Width(), aSize.Height() ));
        }
    }

    // default docking operation, dock on the given row/column
    sal_Bool                                 bOpOutsideOfDockingArea( !aDockingAreaRect.IsInside( rMousePos ));
    std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

    rDockingOperation = DOCKOP_ON_COLROW;
    implts_getDockingAreaElementInfos( eDockedArea, aRowColumnsWindowData );

    // determine current first row/column and last row/column
    sal_Int32 nMaxRowCol( -1 );
    sal_Int32 nMinRowCol( SAL_MAX_INT32 );
    const sal_uInt32 nCount = aRowColumnsWindowData.size();
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        if ( aRowColumnsWindowData[i].nRowColumn > nMaxRowCol )
            nMaxRowCol = aRowColumnsWindowData[i].nRowColumn;
        if ( aRowColumnsWindowData[i].nRowColumn < nMinRowCol )
            nMinRowCol = aRowColumnsWindowData[i].nRowColumn;
    }

    if ( !bOpOutsideOfDockingArea )
    {
        // docking inside our docking area
        sal_Int32   nIndex( -1 );
        sal_Int32   nRowCol( -1 );
        ::Rectangle aWindowRect;
        ::Rectangle aRowColumnRect;

        const sal_uInt32 nWindowDataCount = aRowColumnsWindowData.size();
        for ( sal_uInt32 i = 0; i < nWindowDataCount; i++ )
        {
            ::Rectangle aRect( aRowColumnsWindowData[i].aRowColumnRect.X,
                               aRowColumnsWindowData[i].aRowColumnRect.Y,
                               aRowColumnsWindowData[i].aRowColumnRect.X + aRowColumnsWindowData[i].aRowColumnRect.Width,
                               aRowColumnsWindowData[i].aRowColumnRect.Y + aRowColumnsWindowData[i].aRowColumnRect.Height );

            {
                // Calc correct position of the column/row rectangle to be able to compare it with mouse pos/tracking rect
                SolarMutexGuard aGuard;
                aRect.SetPos( pContainerWindow->ScreenToOutputPixel( pDockingAreaWindow->OutputToScreenPixel( aRect.TopLeft() )));
            }

            sal_Bool bIsInsideRowCol( aRect.IsInside( rMousePos ) );
            if ( bIsInsideRowCol )
            {
                nIndex            = i;
                nRowCol           = aRowColumnsWindowData[i].nRowColumn;
                rDockingOperation = implts_determineDockingOperation( eDockedArea, aRect, rMousePos );
                aWindowRect       = implts_getWindowRectFromRowColumn( eDockedArea, aRowColumnsWindowData[i], rMousePos, rUIElement.m_aName );
                aRowColumnRect    = aRect;
                break;
            }
        }

        OSL_ENSURE( ( nIndex >= 0 ) && ( nRowCol >= 0 ), "Impossible case - no row/column found but mouse pointer is inside our docking area" );
        if (( nIndex >= 0 ) && ( nRowCol >= 0 ))
        {
            if ( rDockingOperation == DOCKOP_ON_COLROW )
            {
                if ( !aWindowRect.IsEmpty())
                {
                    // Tracking rect is on a row/column and mouse is over a docked toolbar.
                    // Determine if the tracking rect must be located before/after the docked toolbar.

                    ::Rectangle aUIElementRect( aWindowRect );
                    sal_Int32   nMiddle( bHorizontalDockArea ? ( aWindowRect.Left() + aWindowRect.getWidth() / 2 ) :
                                                               ( aWindowRect.Top() + aWindowRect.getHeight() / 2 ));
                    sal_Bool    bInsertBefore( bHorizontalDockArea ? ( rMousePos.X() < nMiddle ) : ( rMousePos.Y() < nMiddle ));
                    if ( bInsertBefore )
                    {
                        if ( bHorizontalDockArea )
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( aContainerWinSize.Width() -  aWindowRect.Left() ),
                                                                                    sal_Int32( aTrackingRect.getWidth() )));
                            if ( nSize == 0 )
                                nSize = aWindowRect.getWidth();

                            aUIElementRect.SetSize( ::Size( nSize, aWindowRect.getHeight() ));
                            aWindowRect = implts_determineFrontDockingRect( eDockedArea, nRowCol, aWindowRect,rUIElement.m_aName, aUIElementRect );

                            // Set virtual position
                            rUIElement.m_aDockedData.m_aPos.X() = aWindowRect.Left();
                            rUIElement.m_aDockedData.m_aPos.Y() = nRowCol;
                        }
                        else
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32(
                                                    nTopDockingAreaSize + nMaxLeftRightDockAreaSize - aWindowRect.Top() ),
                                                    sal_Int32( aTrackingRect.getHeight() )));
                            if ( nSize == 0 )
                                nSize = aWindowRect.getHeight();

                            aUIElementRect.SetSize( ::Size( aWindowRect.getWidth(), nSize ));
                            aWindowRect = implts_determineFrontDockingRect( eDockedArea, nRowCol, aWindowRect, rUIElement.m_aName, aUIElementRect );

                            // Set virtual position
                            sal_Int32 nPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                                pContainerWindow->OutputToScreenPixel( aWindowRect.TopLeft() )).Y();
                            rUIElement.m_aDockedData.m_aPos.X() = nRowCol;
                            rUIElement.m_aDockedData.m_aPos.Y() = nPosY;
                        }

                        rTrackingRect = aWindowRect;
                        return;
                    }
                    else
                    {
                        if ( bHorizontalDockArea )
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32(( aContainerWinSize.Width() ) - aWindowRect.Right() ),
                                                                                    sal_Int32( aTrackingRect.getWidth() )));
                            if ( nSize == 0 )
                            {
                                aUIElementRect.SetPos( ::Point( aContainerWinSize.Width() - aTrackingRect.getWidth(), aWindowRect.Top() ));
                                aUIElementRect.SetSize( ::Size( aTrackingRect.getWidth(), aWindowRect.getHeight() ));
                                rUIElement.m_aDockedData.m_aPos.X() = aUIElementRect.Left();
                            }
                            else
                            {
                                aUIElementRect.SetPos( ::Point( aWindowRect.Right(), aWindowRect.Top() ));
                                aUIElementRect.SetSize( ::Size( nSize, aWindowRect.getHeight() ));
                                rUIElement.m_aDockedData.m_aPos.X() = aWindowRect.Right();
                            }

                            // Set virtual position
                            rUIElement.m_aDockedData.m_aPos.Y() = nRowCol;
                        }
                        else
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( nTopDockingAreaSize + nMaxLeftRightDockAreaSize - aWindowRect.Bottom() ),
                                                                                    sal_Int32( aTrackingRect.getHeight() )));
                            aUIElementRect.SetPos( ::Point( aWindowRect.Left(), aWindowRect.Bottom() ));
                            aUIElementRect.SetSize( ::Size( aWindowRect.getWidth(), nSize ));

                            // Set virtual position
                            sal_Int32 nPosY( 0 );
                            {
                                SolarMutexGuard aGuard;
                                nPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                                    pContainerWindow->OutputToScreenPixel( aWindowRect.BottomRight() )).Y();
                            }
                            rUIElement.m_aDockedData.m_aPos.X() = nRowCol;
                            rUIElement.m_aDockedData.m_aPos.Y() = nPosY;
                        }

                        rTrackingRect = aUIElementRect;
                        return;
                    }
                }
                else
                {
                    implts_setTrackingRect( eDockedArea, rMousePos, aTrackingRect );
                    rTrackingRect = implts_calcTrackingAndElementRect(
                                        eDockedArea, nRowCol, rUIElement,
                                        aTrackingRect, aRowColumnRect, aContainerWinSize );
                    return;
                }
            }
            else
            {
                if ((( nRowCol == nMinRowCol ) && ( rDockingOperation == DOCKOP_BEFORE_COLROW )) ||
                    (( nRowCol == nMaxRowCol ) && ( rDockingOperation == DOCKOP_AFTER_COLROW  )))
                    bOpOutsideOfDockingArea = sal_True;
                else
                {
                    // handle docking before/after a row
                    implts_setTrackingRect( eDockedArea, rMousePos, aTrackingRect );
                    rTrackingRect = implts_calcTrackingAndElementRect(
                                        eDockedArea, nRowCol, rUIElement,
                                        aTrackingRect, aRowColumnRect, aContainerWinSize );

                    sal_Int32 nOffsetX( 0 );
                    sal_Int32 nOffsetY( 0 );
                    if ( bHorizontalDockArea )
                        nOffsetY = sal_Int32( floor( aRowColumnRect.getHeight() / 2 + 0.5 ));
                    else
                        nOffsetX = sal_Int32( floor( aRowColumnRect.getWidth() / 2 + 0.5 ));

                    if ( rDockingOperation == DOCKOP_BEFORE_COLROW )
                    {
                        if (( eDockedArea == DockingArea_DOCKINGAREA_TOP ) ||
                            ( eDockedArea == DockingArea_DOCKINGAREA_LEFT ))
                        {
                            // Docking before/after means move track rectangle half column/row.
                            // As left and top are ordered 0...n instead of right and bottom
                            // which uses n...0, we have to use negative values for top/left.
                            nOffsetX *= -1;
                            nOffsetY *= -1;
                        }
                    }
                    else
                    {
                        if (( eDockedArea == DockingArea_DOCKINGAREA_BOTTOM ) ||
                            ( eDockedArea == DockingArea_DOCKINGAREA_RIGHT ))
                        {
                            // Docking before/after means move track rectangle half column/row.
                            // As left and top are ordered 0...n instead of right and bottom
                            // which uses n...0, we have to use negative values for top/left.
                            nOffsetX *= -1;
                            nOffsetY *= -1;
                        }
                        nRowCol++;
                    }

                    if ( bHorizontalDockArea )
                        rUIElement.m_aDockedData.m_aPos.Y() = nRowCol;
                    else
                        rUIElement.m_aDockedData.m_aPos.X() = nRowCol;

                    rTrackingRect.Move( nOffsetX, nOffsetY );
                    rTrackingRect.SetSize( aTrackingRect.GetSize() );
                }
            }
        }
    }

    // Docking outside of our docking window area =>
    // Users want to dock before/after first/last docked element or to an empty docking area
    if ( bOpOutsideOfDockingArea )
    {
        // set correct size for docking
        implts_setTrackingRect( eDockedArea, rMousePos, aTrackingRect );
        rTrackingRect = aTrackingRect;

        if ( bHorizontalDockArea )
        {
            sal_Int32 nPosX( std::max( sal_Int32( rTrackingRect.Left()), sal_Int32( 0 )));
            if (( nPosX + rTrackingRect.getWidth()) > aContainerWinSize.Width() )
                nPosX = std::min( nPosX,
                                std::max( sal_Int32( aContainerWinSize.Width() - rTrackingRect.getWidth() ),
                                          sal_Int32( 0 )));

            sal_Int32 nSize = std::min( aContainerWinSize.Width(), rTrackingRect.getWidth() );
            sal_Int32 nDockHeight = std::max( static_cast<sal_Int32>(aDockingAreaRect.getHeight()), sal_Int32( 0 ));
            if ( nDockHeight == 0 )
            {
                sal_Int32 nPosY( std::max( aDockingAreaRect.Top(), aDockingAreaRect.Bottom() ));
                if ( eDockedArea == DockingArea_DOCKINGAREA_BOTTOM )
                    nPosY -= rTrackingRect.getHeight();
                rTrackingRect.SetPos( Point( nPosX, nPosY ));
                rUIElement.m_aDockedData.m_aPos.Y() = 0;
            }
            else if ( rMousePos.Y() < ( aDockingAreaRect.Top() + ( nDockHeight / 2 )))
            {
                rTrackingRect.SetPos( Point( nPosX, aDockingAreaRect.Top() - rTrackingRect.getHeight() ));
                if ( eDockedArea == DockingArea_DOCKINGAREA_TOP )
                    rUIElement.m_aDockedData.m_aPos.Y() = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.Y() = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                rDockingOperation = DOCKOP_BEFORE_COLROW;
            }
            else
            {
                rTrackingRect.SetPos( Point( nPosX, aDockingAreaRect.Bottom() ));
                if ( eDockedArea == DockingArea_DOCKINGAREA_TOP )
                    rUIElement.m_aDockedData.m_aPos.Y() = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                else
                    rUIElement.m_aDockedData.m_aPos.Y() = 0;
                rDockingOperation = DOCKOP_AFTER_COLROW;
            }
            rTrackingRect.setWidth( nSize );

            {
                SolarMutexGuard aGuard;
                nPosX = pDockingAreaWindow->ScreenToOutputPixel(
                                    pContainerWindow->OutputToScreenPixel( rTrackingRect.TopLeft() )).X();
            }
            rUIElement.m_aDockedData.m_aPos.X() = nPosX;
        }
        else
        {
            sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                        sal_Int32( nMaxLeftRightDockAreaSize ));

            sal_Int32 nPosY( std::max( sal_Int32( aTrackingRect.Top()), sal_Int32( nTopDockingAreaSize )));
            if (( nPosY + aTrackingRect.getHeight()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
                nPosY = std::min( nPosY,
                                std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aTrackingRect.getHeight() )),
                                        sal_Int32( nTopDockingAreaSize )));

            sal_Int32 nSize = std::min( nMaxDockingAreaHeight, static_cast<sal_Int32>(aTrackingRect.getHeight()) );
            sal_Int32 nDockWidth = std::max( static_cast<sal_Int32>(aDockingAreaRect.getWidth()), sal_Int32( 0 ));
            if ( nDockWidth == 0 )
            {
                sal_Int32 nPosX( std::max( aDockingAreaRect.Left(), aDockingAreaRect.Right() ));
                if ( eDockedArea == DockingArea_DOCKINGAREA_RIGHT )
                    nPosX -= rTrackingRect.getWidth();
                rTrackingRect.SetPos( Point( nPosX, nPosY ));
                rUIElement.m_aDockedData.m_aPos.X() = 0;
            }
            else if ( rMousePos.X() < ( aDockingAreaRect.Left() + ( nDockWidth / 2 )))
            {
                rTrackingRect.SetPos( Point( aDockingAreaRect.Left() - rTrackingRect.getWidth(), nPosY ));
                if ( eDockedArea == DockingArea_DOCKINGAREA_LEFT )
                    rUIElement.m_aDockedData.m_aPos.X() = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.X() = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                rDockingOperation = DOCKOP_BEFORE_COLROW;
            }
            else
            {
                rTrackingRect.SetPos( Point( aDockingAreaRect.Right(), nPosY ));
                if ( eDockedArea == DockingArea_DOCKINGAREA_LEFT )
                    rUIElement.m_aDockedData.m_aPos.X() = ( nMaxRowCol >= 0 ) ? nMaxRowCol+1 : 0;
                else
                    rUIElement.m_aDockedData.m_aPos.X() = 0;
                rDockingOperation = DOCKOP_AFTER_COLROW;
            }
            rTrackingRect.setHeight( nSize );

            {
                SolarMutexGuard aGuard;
                nPosY = pDockingAreaWindow->ScreenToOutputPixel(
                                    pContainerWindow->OutputToScreenPixel( rTrackingRect.TopLeft() )).Y();
            }
            rUIElement.m_aDockedData.m_aPos.Y() = nPosY;
        }
    }
}

void LayoutManager::implts_renumberRowColumnData(
    ::com::sun::star::ui::DockingArea eDockingArea,
    DockingOperation /*eDockingOperation*/,
    const UIElement& rUIElement )
{
    ReadGuard aReadLock( m_aLock );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aReadLock.unlock();

    sal_Bool  bHorzDockingArea(( eDockingArea == DockingArea_DOCKINGAREA_TOP    ) ||
                               ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM ));
    sal_Int32 nRowCol( bHorzDockingArea ? rUIElement.m_aDockedData.m_aPos.Y() :
                                          rUIElement.m_aDockedData.m_aPos.X() );

    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if (( pIter->m_aDockedData.m_nDockedArea == sal_Int16( eDockingArea )) &&
            ( pIter->m_aName != rUIElement.m_aName ))
        {
            // Don't change toolbars without a valid docking position!
            if (( pIter->m_aDockedData.m_aPos.X() == SAL_MAX_INT32 ) &&
                ( pIter->m_aDockedData.m_aPos.Y() == SAL_MAX_INT32 ))
                continue;

            sal_Int32 nWindowRowCol = ( bHorzDockingArea ) ?
                pIter->m_aDockedData.m_aPos.Y() : pIter->m_aDockedData.m_aPos.X();
            if ( nWindowRowCol >= nRowCol )
            {
                if ( bHorzDockingArea )
                    pIter->m_aDockedData.m_aPos.Y() += 1;
                else
                    pIter->m_aDockedData.m_aPos.X() += 1;
            }
        }
    }
    aWriteLock.unlock();

    // We have to change the persistent window state part
    if ( xPersistentWindowState.is() )
    {
        try
        {
            Sequence< rtl::OUString > aWindowElements = xPersistentWindowState->getElementNames();
            for ( sal_Int32 i = 0; i < aWindowElements.getLength(); i++ )
            {
                if ( rUIElement.m_aName != aWindowElements[i] )
                {
                    try
                    {
                        Sequence< PropertyValue > aPropValueSeq;
                        css::awt::Point           aDockedPos;
                        DockingArea               nDockedArea( DockingArea_DOCKINGAREA_DEFAULT );

                        xPersistentWindowState->getByName( aWindowElements[i] ) >>= aPropValueSeq;
                        for ( sal_Int32 j = 0; j < aPropValueSeq.getLength(); j++ )
                        {
                            if ( aPropValueSeq[j].Name == m_aPropDockingArea )
                                aPropValueSeq[j].Value >>= nDockedArea;
                            else if ( aPropValueSeq[j].Name == m_aPropDockPos )
                                aPropValueSeq[j].Value >>= aDockedPos;
                        }

                        // Don't change toolbars without a valid docking position!
                        if (( aDockedPos.X == SAL_MAX_INT32 ) && ( aDockedPos.Y == SAL_MAX_INT32 ))
                            continue;

                        sal_Int32 nWindowRowCol = ( bHorzDockingArea ) ? aDockedPos.Y : aDockedPos.X;
                        if (( nDockedArea == eDockingArea ) && ( nWindowRowCol >= nRowCol ))
                        {
                            if ( bHorzDockingArea )
                                aDockedPos.Y += 1;
                            else
                                aDockedPos.X += 1;

                            Reference< XNameReplace > xReplace( xPersistentWindowState, UNO_QUERY );
                            xReplace->replaceByName( aWindowElements[i], makeAny( aPropValueSeq ));
                        }
                    }
                    catch ( Exception& )
                    {
                    }
                }
            }
        }
        catch ( Exception& )
        {
        }
    }
}

::Size LayoutManager::implts_getTopBottomDockingAreaSizes()
{
    ::Size                         aSize;
    Reference< css::awt::XWindow > xTopDockingAreaWindow;
    Reference< css::awt::XWindow > xBottomDockingAreaWindow;

    ReadGuard aReadLock( m_aLock );
    xTopDockingAreaWindow       = m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP];
    xBottomDockingAreaWindow    = m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM];
    aReadLock.unlock();

    if ( xTopDockingAreaWindow.is() )
        aSize.Width() = xTopDockingAreaWindow->getPosSize().Height;
    if ( xBottomDockingAreaWindow.is() )
        aSize.Height() = xBottomDockingAreaWindow->getPosSize().Height;

    return aSize;
}

Reference< XUIElement > LayoutManager::implts_createElement( const rtl::OUString& aName )
{
    Reference< ::com::sun::star::ui::XUIElement > xUIElement;

    ReadGuard   aReadLock( m_aLock );
    Sequence< PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropSeq[0].Value <<= m_xFrame;
    aPropSeq[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
    aPropSeq[1].Value <<= sal_True;

    try
    {
        xUIElement = m_xUIElementFactoryManager->createUIElement( aName, aPropSeq );
    }
    catch ( NoSuchElementException& )
    {
    }
    catch ( IllegalArgumentException& )
    {
    }

    return xUIElement;
}

Reference< css::awt::XWindowPeer > LayoutManager::implts_createToolkitWindow( const Reference< css::awt::XWindowPeer >& rParent )
{
    Reference< css::awt::XWindowPeer > xPeer;
    css::uno::Reference< css::awt::XToolkit > xToolkit( m_xSMGR->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY );
    if ( xToolkit.is() )
    {
        // describe window properties.
        css::awt::WindowDescriptor aDescriptor;
        aDescriptor.Type                =   css::awt::WindowClass_SIMPLE                                                  ;
        aDescriptor.WindowServiceName   =   DECLARE_ASCII("dockingarea")                                                  ;
        aDescriptor.ParentIndex         =   -1                                                                            ;
        aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >( rParent, UNO_QUERY )            ;
        aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0)                                                  ;
        aDescriptor.WindowAttributes    =   0                                                                             ;

        // create a docking area window
        xPeer = xToolkit->createWindow( aDescriptor );
    }

    return xPeer;
}

void LayoutManager::implts_setVisibleState( sal_Bool bShow )
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
        pIter->m_bMasterHide = !bShow;
    m_aStatusBarElement.m_bMasterHide = !bShow;
    aWriteLock.unlock();
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_updateUIElementsVisibleState( bShow );
    //implts_doLayout( sal_False );
}

void LayoutManager::implts_updateUIElementsVisibleState( sal_Bool bSetVisible )
{
    // notify listeners
    css::uno::Any a;
    if ( bSetVisible )
        implts_notifyListeners( css::frame::LayoutManagerEvents::VISIBLE, a );
    else
        implts_notifyListeners( css::frame::LayoutManagerEvents::INVISIBLE, a );
    std::vector< Reference< css::awt::XWindow > > aWinVector;
    sal_Bool bOld;

    {
        WriteGuard aWriteLock( m_aLock );
        m_bDoLayout = sal_True;
        bOld = m_bDoLayout;
    }

    ReadGuard aReadLock( m_aLock );
    aWinVector.reserve(m_aUIElements.size());
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
            {
                if ( bSetVisible )
                {
                    if ( pIter->m_bVisible && !pIter->m_bMasterHide )
                        aWinVector.push_back( xWindow );
                }
                else
                    aWinVector.push_back( xWindow );
            }
        }
    }

    aReadLock.unlock();

    try
    {
        SolarMutexGuard aGuard;
        const sal_uInt32 nCount = aWinVector.size();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            Reference< css::awt::XWindow > xWindow( aWinVector[i] );
            if ( xWindow.is() )
            {
                // we need VCL here to pass special flags to Show()
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if( pWindow )
                    pWindow->Show( bSetVisible, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
        }
    }
    catch ( DisposedException& )
    {
    }

    // Hide/show menubar according to bSetVisible
    aReadLock.lock();
    Reference< XUIElement > xMenuBar( m_xMenuBar, UNO_QUERY );
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< XComponent > xInplaceMenuBar( m_xInplaceMenuBar );
    MenuBarManager* pInplaceMenuBar( m_pInplaceMenuBar );
    aReadLock.unlock();

    if (( xMenuBar.is() || xInplaceMenuBar.is() ) && xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        MenuBar* pMenuBar( 0 );
        if ( xInplaceMenuBar.is() )
            pMenuBar = (MenuBar *)pInplaceMenuBar->GetMenuBar();
        else
        {
            MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, xMenuBar.get() );
            pMenuBar = (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar();
        }

        Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
            if ( bSetVisible )
                pSysWindow->SetMenuBar( pMenuBar );
            else
                pSysWindow->SetMenuBar( 0 );
        }
    }

    // Hide/show the statusbar according to bSetVisible
    if ( bSetVisible )
        implts_showStatusBar();
    else
        implts_hideStatusBar();

    if ( !bOld )
    {
        WriteGuard aWriteLock( m_aLock );
        m_bDoLayout = sal_False;
    }

    if ( bSetVisible )
    {
        implts_createNonContextSensitiveToolBars();
        implts_doLayout_notify( sal_False );
    }
    else
    {
        // Set docking area window size to zero
        ReadGuard aReadLock2( m_aLock );
        Reference< css::awt::XWindow > xTopDockingWindow    = m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP];
        Reference< css::awt::XWindow > xLeftDockingWindow   = m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT];
        Reference< css::awt::XWindow > xRightDockingWindow  = m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT];
        Reference< css::awt::XWindow > xBottomDockingWindow = m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM];
        aReadLock2.unlock();

        try
        {
            if ( xTopDockingWindow.is() )
                xTopDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
            if ( xLeftDockingWindow.is() )
                xLeftDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
            if ( xRightDockingWindow.is() )
                xRightDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );
            if ( xBottomDockingWindow.is() )
                xBottomDockingWindow->setPosSize( 0, 0, 0, 0, css::awt::PosSize::POSSIZE );

            WriteGuard aWriteLock( m_aLock );
            m_aDockingArea = css::awt::Rectangle();
            m_bMustDoLayout = sal_True;
            aWriteLock.unlock();
        }
        catch ( Exception& )
        {
        }
    }
}

void LayoutManager::implts_setCurrentUIVisibility( sal_Bool bShow )
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( !bShow && pIter->m_bVisible && pIter->m_xUIElement.is() )
            pIter->m_bMasterHide = true;
        else if ( bShow && pIter->m_bMasterHide )
            pIter->m_bMasterHide = false;
    }

    if ( !bShow && m_aStatusBarElement.m_bVisible && m_aStatusBarElement.m_xUIElement.is() )
        m_aStatusBarElement.m_bMasterHide = true;
    else if ( bShow && m_aStatusBarElement.m_bVisible )
        m_aStatusBarElement.m_bMasterHide = false;
    aWriteLock.unlock();
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_updateUIElementsVisibleState( bShow );
}

void LayoutManager::implts_destroyStatusBar()
{
     Reference< XComponent > xCompStatusBar;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_aStatusBarElement.m_aName = rtl::OUString();
    xCompStatusBar = Reference< XComponent >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    m_aStatusBarElement.m_xUIElement.clear();
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xCompStatusBar.is() )
        xCompStatusBar->dispose();

    implts_destroyProgressBar();
}

void LayoutManager::implts_createStatusBar( const rtl::OUString& aStatusBarName )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    if ( !m_aStatusBarElement.m_xUIElement.is() )
    {
        implts_readStatusBarState( aStatusBarName );
        m_aStatusBarElement.m_aName      = aStatusBarName;
        m_aStatusBarElement.m_xUIElement = implts_createElement( aStatusBarName );
    }

    implts_createProgressBar();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void LayoutManager::implts_readStatusBarState( const rtl::OUString& rStatusBarName )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    if ( !m_aStatusBarElement.m_bStateRead )
    {
        // Read persistent data for status bar if not yet read!
        if ( implts_readWindowStateData( rStatusBarName, m_aStatusBarElement ))
            m_aStatusBarElement.m_bStateRead = sal_True;
    }
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void LayoutManager::implts_createProgressBar()
{
     Reference< XUIElement > xStatusBar;
    Reference< XUIElement > xProgressBar;
    Reference< XUIElement > xProgressBarBackup;
    Reference< css::awt::XWindow > xContainerWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    xStatusBar = Reference< XUIElement >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    xProgressBarBackup = m_xProgressBarBackup;
    m_xProgressBarBackup.clear();
    xContainerWindow = m_xContainerWindow;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

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
        Reference< css::awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );
        pWrapper->setStatusBar( xWindow );
    }
    else
    {
        Reference< css::awt::XWindow > xStatusBarWindow = pWrapper->getStatusBar();

        SolarMutexGuard aGuard;
        Window* pStatusBarWnd = VCLUnoHelper::GetWindow( xStatusBarWindow );
        if ( !pStatusBarWnd )
        {
            Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            if ( pWindow )
            {
                StatusBar* pStatusBar = new StatusBar( pWindow, WinBits( WB_LEFT | WB_3DLOOK ) );
                Reference< css::awt::XWindow > xStatusBarWindow2( VCLUnoHelper::GetInterface( pStatusBar ));
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
            pWrapper->setStatusBar( Reference< css::awt::XWindow >(), sal_False );
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
    Reference< css::awt::XWindow > xContainerWindow;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    xStatusBar = Reference< XUIElement >( m_aStatusBarElement.m_xUIElement, UNO_QUERY );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );
    xContainerWindow = m_xContainerWindow;

    Reference< css::awt::XWindow > xWindow;
    if ( xStatusBar.is() )
        xWindow = Reference< css::awt::XWindow >( xStatusBar->getRealInterface(), UNO_QUERY );
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
    Reference< css::awt::XWindow > xWindow;

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
            xWindow = Reference< css::awt::XWindow >( xStatusBar->getRealInterface(), UNO_QUERY );
        }
        else if ( xProgressBar.is() )
        {
            ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
            if ( pWrapper )
                xWindow = pWrapper->getStatusBar();
        }
    }
    aWriteLock.unlock();

    SolarMutexGuard aGuard;
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( pWindow )
    {
        if ( !pWindow->IsVisible() )
        {
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
    Reference< css::awt::XWindow > xWindow;
    sal_Bool bHideStatusBar( sal_False );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    xProgressBar = Reference< XUIElement >( m_aProgressBarElement.m_xUIElement, UNO_QUERY );

    sal_Bool bInternalStatusBar( sal_False );
    if ( xProgressBar.is() )
    {
        Reference< css::awt::XWindow > xStatusBar;
        ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
        Reference< css::ui::XUIElement > xStatusBarElement = m_aStatusBarElement.m_xUIElement;
        if ( xStatusBarElement.is() )
            xStatusBar = Reference< css::awt::XWindow >( xStatusBarElement->getRealInterface(), UNO_QUERY );
        bInternalStatusBar = xStatusBar != xWindow;
    }
    m_aProgressBarElement.m_bVisible = sal_False;
    implts_readStatusBarState( m_aStatusBarAlias );
    bHideStatusBar = !m_aStatusBarElement.m_bVisible;
    aWriteLock.unlock();

    SolarMutexGuard aGuard;
    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
    if ( pWindow && pWindow->IsVisible() &&
         ( bHideStatusBar || bInternalStatusBar ))
    {
        pWindow->Hide();
        implts_doLayout_notify( sal_False );
        return sal_True;
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_showStatusBar( sal_Bool bStoreState )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    Reference< css::ui::XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    if ( bStoreState )
        m_aStatusBarElement.m_bVisible = sal_True;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xStatusBar.is() )
    {
        Reference< css::awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );

        SolarMutexGuard aGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && !pWindow->IsVisible() )
        {
            pWindow->Show();
            return sal_True;
        }
    }

    return sal_False;
}

sal_Bool LayoutManager::implts_hideStatusBar( sal_Bool bStoreState )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    Reference< css::ui::XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    if ( bStoreState )
        m_aStatusBarElement.m_bVisible = sal_False;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xStatusBar.is() )
    {
        Reference< css::awt::XWindow > xWindow( xStatusBar->getRealInterface(), UNO_QUERY );

        SolarMutexGuard aGuard;
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow && pWindow->IsVisible() )
        {
            pWindow->Hide();
            return sal_True;
        }
    }

    return sal_False;
}

void LayoutManager::implts_setInplaceMenuBar( const Reference< XIndexAccess >& xMergedMenuBar )
throw (::com::sun::star::uno::RuntimeException)
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

        if ( m_xFrame.is() &&
             m_xContainerWindow.is() )
        {
            rtl::OUString aModuleIdentifier;
            Reference< XDispatchProvider > xDispatchProvider;

            MenuBar* pMenuBar = new MenuBar;
            m_pInplaceMenuBar = new MenuBarManager( m_xSMGR, m_xFrame, m_xURLTransformer,xDispatchProvider, aModuleIdentifier, pMenuBar, sal_True, sal_True );
            m_pInplaceMenuBar->SetItemContainer( xMergedMenuBar );

            Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
            while ( pWindow && !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();

            if ( pWindow )
            {
                SystemWindow* pSysWindow = (SystemWindow *)pWindow;
                pSysWindow->SetMenuBar( pMenuBar );
            }

             m_bInplaceMenuSet = sal_True;
            m_xInplaceMenuBar = Reference< XComponent >( (OWeakObject *)m_pInplaceMenuBar, UNO_QUERY );
        }

        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        implts_updateMenuBarClose();
    }

}

void LayoutManager::implts_resetInplaceMenuBar()
throw (::com::sun::star::uno::RuntimeException)
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_bInplaceMenuSet = sal_False;

    // if ( m_xMenuBar.is() &&
    if ( m_xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;
        MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, m_xMenuBar.get() );
        Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
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

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::attachFrame( const Reference< XFrame >& xFrame )
throw (::com::sun::star::uno::RuntimeException)
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_xFrame = xFrame;
    aWriteLock.unlock();
}

void SAL_CALL LayoutManager::reset()
throw (RuntimeException)
{
    implts_reset( sal_True );
}

void SAL_CALL LayoutManager::setInplaceMenuBar( sal_Int64 )
throw (::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( sal_False, "This method is obsolete and should not be used!\n" );
}

void SAL_CALL LayoutManager::resetInplaceMenuBar()
throw (::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( sal_False, "This method is obsolete and should not be used!\n" );
}

//---------------------------------------------------------------------------------------------------------
// XMenuBarMergingAcceptor
//---------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL LayoutManager::setMergedMenuBar(
    const Reference< XIndexAccess >& xMergedMenuBar )
throw (::com::sun::star::uno::RuntimeException)
{
    implts_setInplaceMenuBar( xMergedMenuBar );

    css::uno::Any a;
    implts_notifyListeners( css::frame::LayoutManagerEvents::MERGEDMENUBAR, a );
    return sal_True;
}

void SAL_CALL LayoutManager::removeMergedMenuBar()
throw (::com::sun::star::uno::RuntimeException)
{
    implts_resetInplaceMenuBar();
}

::com::sun::star::awt::Rectangle SAL_CALL LayoutManager::getCurrentDockingArea()
throw ( RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    return m_aDockingArea;
}

Reference< XDockingAreaAcceptor > SAL_CALL LayoutManager::getDockingAreaAcceptor()
throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    return m_xDockingAreaAcceptor;
}

void SAL_CALL LayoutManager::setDockingAreaAcceptor( const Reference< ::com::sun::star::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor )
throw ( RuntimeException )
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if (( m_xDockingAreaAcceptor == xDockingAreaAcceptor ) ||
          !m_xFrame.is() )
        return;

    // IMPORTANT: Be sure to stop layout timer if don't have a docking area acceptor!
    if ( !xDockingAreaAcceptor.is() )
        m_aAsyncLayoutTimer.Stop();

    sal_Bool bAutomaticToolbars( m_bAutomaticToolbars );
    std::vector< Reference< css::awt::XWindow > > oldDockingAreaWindows;

    if ( !xDockingAreaAcceptor.is() )
        m_aAsyncLayoutTimer.Stop();

    // Remove listener from old docking area acceptor
    if ( m_xDockingAreaAcceptor.is() )
    {
        Reference< css::awt::XWindow > xWindow( m_xDockingAreaAcceptor->getContainerWindow() );
        if ( xWindow.is() && ( m_xFrame->getContainerWindow() != m_xContainerWindow || !xDockingAreaAcceptor.is() ) )
            xWindow->removeWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject * >( this ), UNO_QUERY ));

        oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP] );
        oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] );
        oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT] );
        oldDockingAreaWindows.push_back( m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT] );

        m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP].clear();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM].clear();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT].clear();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT].clear();
        m_aDockingArea = css::awt::Rectangle();

        Window* pContainerWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pContainerWindow )
            pContainerWindow->RemoveChildEventListener( LINK( this, LayoutManager, WindowEventListener ) );
    }

    // Set new docking area acceptor and add ourself as window listener on the container window.
    // Create our docking area windows which are parents for all docked windows.
    css::uno::Reference< css::awt::XWindow > xTopDockWindow;
    css::uno::Reference< css::awt::XWindow > xBottomDockWindow;
    css::uno::Reference< css::awt::XWindow > xLeftDockWindow;
    css::uno::Reference< css::awt::XWindow > xRightDockWindow;

    Reference< ::com::sun::star::ui::XDockingAreaAcceptor > xOldDockingAreaAcceptor( m_xDockingAreaAcceptor );
    m_xDockingAreaAcceptor = xDockingAreaAcceptor;
    if ( m_xDockingAreaAcceptor.is() )
    {
        m_aDockingArea     = css::awt::Rectangle();
        m_xContainerWindow = m_xDockingAreaAcceptor->getContainerWindow();
        m_xContainerTopWindow.set( m_xContainerWindow, UNO_QUERY );
        m_xContainerWindow->addWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));

        // we always must keep a connection to the window of our frame for resize events
        if ( m_xContainerWindow != m_xFrame->getContainerWindow() )
            m_xFrame->getContainerWindow()->addWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));

        // #i37884# set initial visibility state - in the plugin case the container window is already shown
        // and we get no notification anymore
        {
            SolarMutexGuard aGuard;
            Window* pContainerWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
            if( pContainerWindow )
                m_bParentWindowVisible = pContainerWindow->IsVisible();
        }

        css::uno::Reference< css::awt::XWindowPeer > xParent( m_xContainerWindow, UNO_QUERY );
        xTopDockWindow = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        xBottomDockWindow = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        xLeftDockWindow = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        xRightDockWindow = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP]    = xTopDockWindow;
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] = xBottomDockWindow;
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT]   = xLeftDockWindow;
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT]  = xRightDockWindow;
    }

    aWriteLock.unlock();
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xDockingAreaAcceptor.is() )
    {
        SolarMutexGuard aGuard;
        ::DockingAreaWindow* pWindow;
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xTopDockWindow ) );
        if( pWindow ) pWindow->SetAlign( WINDOWALIGN_TOP );
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xBottomDockWindow ) );
        if( pWindow ) pWindow->SetAlign( WINDOWALIGN_BOTTOM );
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xLeftDockWindow ) );
        if( pWindow ) pWindow->SetAlign( WINDOWALIGN_LEFT );
        pWindow = dynamic_cast< ::DockingAreaWindow* >(VCLUnoHelper::GetWindow( xRightDockWindow ) );
        if( pWindow ) pWindow->SetAlign( WINDOWALIGN_RIGHT );

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
        const sal_uInt32 nCount = oldDockingAreaWindows.size();
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            if ( oldDockingAreaWindows[i].is() )
            {
                try
                {
                    oldDockingAreaWindows[i]->dispose();
                }
                catch ( Exception& )
                {
                }
            }
        }

        // Reset docking area size for our old docking area acceptor
        css::awt::Rectangle aEmptyRect;
        xOldDockingAreaAcceptor->setDockingAreaSpace( aEmptyRect );
    }

    if ( xDockingAreaAcceptor.is() )
    {
        if ( bAutomaticToolbars )
        {
            implts_createAddonsToolBars(); // create addon toolbars
            implts_createCustomToolBars(); // create custom toolbars
            implts_createNonContextSensitiveToolBars();
        }
        implts_sortUIElements();
        implts_doLayout( sal_True, sal_False );
    }
}

void LayoutManager::implts_reparentChildWindows()
{
    UIElementVector aUIElementVector;
    UIElement       aStatusBarElement;
    css::uno::Reference< css::awt::XWindow > xTopDockWindow;
    css::uno::Reference< css::awt::XWindow > xBottomDockWindow;
    css::uno::Reference< css::awt::XWindow > xLeftDockWindow;
    css::uno::Reference< css::awt::XWindow > xRightDockWindow;
    css::uno::Reference< css::awt::XWindow > xContainerWindow;
    css::uno::Reference< css::awt::XWindow > xStatusBarWindow;

    WriteGuard aWriteLock( m_aLock );
    aUIElementVector    = m_aUIElements;
    xTopDockWindow      = m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP];
    xBottomDockWindow   = m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM];
    xLeftDockWindow     = m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT];
    xRightDockWindow    = m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT];
    xContainerWindow    = m_xContainerWindow;
    aStatusBarElement   = m_aStatusBarElement;
    aWriteLock.unlock();

    if ( aStatusBarElement.m_xUIElement.is() )
    {
        try
        {
            xStatusBarWindow = Reference< css::awt::XWindow >(
                                    aStatusBarElement.m_xUIElement->getRealInterface(),
                               UNO_QUERY );
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
        }
    }

    SolarMutexGuard aGuard;
    Window* pContainerWindow    = VCLUnoHelper::GetWindow( xContainerWindow );
    Window* pTopDockWindow      = VCLUnoHelper::GetWindow( xTopDockWindow );
    Window* pBottomDockWindow   = VCLUnoHelper::GetWindow( xBottomDockWindow );
    Window* pLeftDockWindow     = VCLUnoHelper::GetWindow( xLeftDockWindow );
    Window* pRightDockWindow    = VCLUnoHelper::GetWindow( xRightDockWindow );
    if ( pContainerWindow )
    {
        UIElementVector::iterator pIter;
        for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); ++pIter )
        {
            Reference< XUIElement > xUIElement( pIter->m_xUIElement );
            if ( xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow;
                try
                {
                    // We have to retreive the window reference with try/catch as it is
                    // possible that all elements has been disposed!
                    xWindow = Reference< css::awt::XWindow >( xUIElement->getRealInterface(), UNO_QUERY );
                }
                catch ( RuntimeException& )
                {
                    throw;
                }
                catch ( Exception& )
                {
                }

                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow )
                {
                    // Reparent our child windows acording to their current state.
                    if ( pIter->m_bFloating )
                        pWindow->SetParent( pContainerWindow );
                    else
                    {
                        if ( pIter->m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_TOP )
                            pWindow->SetParent( pTopDockWindow );
                        else if ( pIter->m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_BOTTOM )
                            pWindow->SetParent( pBottomDockWindow );
                        else if ( pIter->m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_LEFT )
                            pWindow->SetParent( pLeftDockWindow );
                        else
                            pWindow->SetParent( pRightDockWindow );
                    }
                }
            }
        }

        if ( xStatusBarWindow.is() )
        {
            Window* pWindow = VCLUnoHelper::GetWindow( xStatusBarWindow );
            if ( pWindow )
                pWindow->SetParent( pContainerWindow );
        }

        implts_resetMenuBar();
    }
}

IMPL_LINK( LayoutManager, WindowEventListener, VclSimpleEvent*, pEvent )
{
    // To enable toolbar controllers to change their image when a sub-toolbar function
    // is activated, we need this mechanism. We have NO connection between these toolbars
    // anymore!
    if ( pEvent && pEvent->ISA( VclWindowEvent ))
    {
        if ( pEvent->GetId() == VCLEVENT_TOOLBOX_SELECT )
        {
            Window*         pWindow( ((VclWindowEvent*)pEvent)->GetWindow() );
            ToolBox*        pToolBox( 0 );
            rtl::OUString   aToolbarName;
            rtl::OUString   aCommand;

            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            {
                pToolBox = (ToolBox *)pWindow;
                aToolbarName = pToolBox->GetSmartHelpId().GetStr();
                sal_Int32 i = aToolbarName.lastIndexOf( ':' );
                if (( aToolbarName.getLength() > 0 ) &&
                    ( i > 0 ) && (( i+ 1 ) < aToolbarName.getLength() ))
                {
                    // Remove ".HelpId:" protocol from toolbar name
                    aToolbarName = aToolbarName.copy( i+1 );

                    USHORT nId = pToolBox->GetCurItemId();
                    if ( nId > 0 )
                        aCommand = pToolBox->GetItemCommand( nId );
                }
            }

            if (( aToolbarName.getLength() > 0 ) && ( aCommand.getLength() > 0 ))
            {
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */
                ReadGuard aReadLock( m_aLock );
                std::vector< css::uno::Reference< css::ui::XUIFunctionListener > > aListenerArray;
                UIElementVector::iterator pIter;

                for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
                {
                    if ( pIter->m_aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("toolbar")) &&
                        pIter->m_xUIElement.is() )
                    {
                        css::uno::Reference< css::ui::XUIFunctionListener > xListener( pIter->m_xUIElement, UNO_QUERY );
                        if ( xListener.is() )
                            aListenerArray.push_back( xListener );
                    }
                }
                aReadLock.unlock();
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */

                const sal_uInt32 nCount = aListenerArray.size();
                for ( sal_uInt32 i = 0; i < nCount; ++i )
                {
                    try
                    {
                        aListenerArray[i]->functionExecute( aToolbarName, aCommand );
                    }
                    catch ( RuntimeException& e )
                    {
                        throw e;
                    }
                    catch ( Exception& ) {}
                }
            }
        }
        else if ( pEvent->GetId() == VCLEVENT_TOOLBOX_FORMATCHANGED )
        {
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            ReadGuard aReadLock( m_aLock );

            Window*         pWindow( ((VclWindowEvent*)pEvent)->GetWindow() );
            ToolBox*        pToolBox( 0 );
            rtl::OUString   aToolbarName;

            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            {
                pToolBox = (ToolBox *)pWindow;
                aToolbarName = pToolBox->GetSmartHelpId().GetStr();
                if (( aToolbarName.getLength() > 0 ) && ( m_nLockCount == 0 ))
                    m_aAsyncLayoutTimer.Start();
            }
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        }
    }

    return 1;
}

void SAL_CALL LayoutManager::createElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::createElement" );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< XURLTransformer > xURLTransformer = m_xURLTransformer;
    sal_Bool    bInPlaceMenu = m_bInplaceMenuSet;
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( !xFrame.is() )
        return;

    Reference< XModel >  xModel( impl_getModelFromFrame( xFrame ) );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    sal_Bool                                      bFound( sal_False );
    sal_Bool                                      bNotify( sal_False );
    ::rtl::OUString                                      aElementType;
    ::rtl::OUString                                      aElementName;
    Reference< ::com::sun::star::ui::XUIElement > xUIElement;

    implts_findElement( aName, aElementType, aElementName, xUIElement );
    bFound = xUIElement.is();

    if ( /*xFrame.is() && */m_xContainerWindow.is() && !implts_isPreviewModel( xModel ) ) // no bars on preview mode
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            if ( !bFound  )
            {
                SvtCommandOptions aCmdOptions;

                xUIElement = implts_createElement( aName );
                sal_Bool bVisible( sal_False );
                if ( xUIElement.is() )
                {
                    Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                    Reference< css::awt::XDockableWindow > xDockWindow( xUIElement->getRealInterface(), UNO_QUERY );
                    if ( xDockWindow.is() && xWindow.is() )
                    {
                        try
                        {
                            xDockWindow->addDockableWindowListener( Reference< css::awt::XDockableWindowListener >(
                                static_cast< OWeakObject * >( this ), UNO_QUERY ));
                            xWindow->addWindowListener( Reference< css::awt::XWindowListener >(
                                static_cast< OWeakObject * >( this ), UNO_QUERY ));
                            xDockWindow->enableDocking( sal_True );
                        }
                        catch ( Exception& )
                        {
                        }
                    }

                    UIElement& rElement = impl_findElement( aName );
                    if ( rElement.m_aName.getLength() > 0 )
                    {
                        // Reuse a local entry so we are able to use the latest
                        // UI changes for this document.
                        implts_setElementData( rElement, xDockWindow );
                        rElement.m_xUIElement = xUIElement;
                        bVisible = rElement.m_bVisible;
                    }
                    else
                    {
                        // Create new UI element and try to read its state data
                        UIElement aNewToolbar( aName, aElementType, xUIElement );
                        implts_readWindowStateData( aName, aNewToolbar );
                        implts_setElementData( aNewToolbar, xDockWindow );
                        implts_insertUIElement( aNewToolbar );
                        bVisible = aNewToolbar.m_bVisible;
                    }

                    // set toolbar menu style according to customize command state
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                    {
                        ToolBox* pToolbar = (ToolBox *)pWindow;
                        USHORT nMenuType = pToolbar->GetMenuType();
                        if ( aCmdOptions.Lookup( SvtCommandOptions::CMDOPTION_DISABLED, m_aCustomizeCmd ))
                            pToolbar->SetMenuType( nMenuType & ~TOOLBOX_MENUTYPE_CUSTOMIZE );
                        else
                            pToolbar->SetMenuType( nMenuType | TOOLBOX_MENUTYPE_CUSTOMIZE );
                    }
                }
                aWriteLock.unlock();

                implts_sortUIElements();

                if ( bVisible )
                {
                    doLayout();
                    bNotify = sal_True;
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            if ( aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ) && !bInPlaceMenu )
            {
                SolarMutexGuard aGuard;
                // PB 2004-12-15 #i38743# don't create a menubar if frame isn't top
                if ( !m_xMenuBar.is() && implts_isFrameOrWindowTop(xFrame) )
                    m_xMenuBar = implts_createElement( aName );

                if ( m_xMenuBar.is() && implts_isFrameOrWindowTop(xFrame) )
                {
                    Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
                    while ( pWindow && !pWindow->IsSystemWindow() )
                        pWindow = pWindow->GetParent();

                    if ( pWindow )
                    {
                        SystemWindow* pSysWindow = (SystemWindow *)pWindow;
                        Reference< css::awt::XMenuBar > xMenuBar;

                        Reference< XPropertySet > xPropSet( m_xMenuBar, UNO_QUERY );
                        if ( xPropSet.is() )
                        {
                            try
                            {
                                xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "XMenuBar" ))) >>= xMenuBar;
                            }
                            catch ( com::sun::star::beans::UnknownPropertyException )
                            {
                            }
                            catch ( com::sun::star::lang::WrappedTargetException )
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
                                    {
                                        bNotify = sal_True;
                                    }
                                    implts_updateMenuBarClose();
                                }
                            }
                        }
                    }
                }
            }
            aWriteLock.unlock();
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) && ( implts_isFrameOrWindowTop(xFrame) || implts_isEmbeddedLayoutManager() ))
        {
            implts_createStatusBar( aName );
            bNotify = sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  implts_isFrameOrWindowTop(xFrame) )
        {
            implts_createProgressBar();
            bNotify = sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "dockingwindow" ))
        {
            aWriteLock.unlock();

            // The docking window is created by a factory method located in the sfx2 library.
            CreateDockingWindow( xFrame, aElementName );
        }
    }

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    if ( bNotify )
    {
        // UI element is invisible - provide information to listeners
        implts_notifyListeners( css::frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( aName ) );
    }
}

void SAL_CALL LayoutManager::destroyElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::destroyElement" );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    sal_Bool    bMustLayouted( sal_False );
    sal_Bool    bMustBeDestroyed( sal_False );
    sal_Bool    bMustBeSorted( sal_False );
    sal_Bool    bNotify( sal_False );
    ::rtl::OUString    aElementType;
    ::rtl::OUString    aElementName;

    Reference< XComponent > xComponent;
    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            if ( !m_bInplaceMenuSet )
            {
                impl_clearUpMenuBar();
                m_xMenuBar.clear();
                bNotify = sal_True;
            }
        }
        else if (( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) &&
                   aElementName.equalsIgnoreAsciiCaseAscii( "statusbar" )) ||
                 ( m_aStatusBarElement.m_aName == aName ))
        {
            aWriteLock.unlock();
            implts_destroyStatusBar();
            bMustLayouted = sal_True;
            bNotify = sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" ) )
        {
            aWriteLock.unlock();
            implts_createProgressBar();
            bMustLayouted = sal_True;
            bNotify = sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            UIElementVector::iterator pIter;

            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_aName == aName )
                {
                    xComponent.set( pIter->m_xUIElement, UNO_QUERY );
                    Reference< XUIElement > xUIElement( pIter->m_xUIElement );
                    if ( xUIElement.is() )
                    {
                        Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                        Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                        rtl::OUString aAddonTbResourceName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/addon_" ));
                        if ( aName.indexOf( aAddonTbResourceName ) != 0 )
                        {
                            try
                            {
                                if ( xWindow.is() )
                                    xWindow->removeWindowListener( Reference< css::awt::XWindowListener >(
                                        static_cast< OWeakObject * >( this ), UNO_QUERY ));
                            }
                            catch( Exception& )
                            {
                            }

                            try
                            {
                                if ( xDockWindow.is() )
                                    xDockWindow->removeDockableWindowListener( Reference< css::awt::XDockableWindowListener >(
                                        static_cast< OWeakObject * >( this ), UNO_QUERY ));
                            }
                            catch ( Exception& )
                            {
                            }

                            bMustBeDestroyed = sal_True;
                        }
                        else
                        {
                            pIter->m_bVisible = sal_False;
                            xWindow->setVisible( sal_False );
                            bNotify = sal_True;
                        }

                        if ( !xDockWindow->isFloating() )
                            bMustLayouted = sal_True;
                        if ( bMustBeDestroyed )
                            pIter->m_xUIElement.clear();

                        bMustBeSorted = sal_True;
                    }

                    break;
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "dockingwindow" ))
        {
            css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame );
            css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR( m_xSMGR );
            aWriteLock.unlock();

            impl_setDockingWindowVisibility( xSMGR, xFrame, aElementName, false );
            bMustLayouted = sal_False;
            bNotify = sal_False;
        }
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( bMustBeDestroyed )
    {
        if ( xComponent.is() )
            xComponent->dispose();
        bNotify = sal_True;
    }

    if ( bMustBeSorted )
    {
        implts_sortUIElements();
        if ( bMustLayouted )
            doLayout();
    }

    if ( bNotify )
    {
        // UI element is invisible - provide information to listeners
        implts_notifyListeners( css::frame::LayoutManagerEvents::UIELEMENT_INVISIBLE, uno::makeAny( aName ) );
    }
}

::sal_Bool SAL_CALL LayoutManager::requestElement( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::requestElement" );

    ::rtl::OUString                  aElementType;
    ::rtl::OUString                  aElementName;
    UIElementVector::iterator pIter;
    sal_Bool                  bResult( sal_False );
    sal_Bool                  bNotify( sal_False );

    WriteGuard aWriteLock( m_aLock );
    if ( impl_parseResourceURL( ResourceURL, aElementType, aElementName ))
    {
        ::rtl::OString aResName = rtl::OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
        RTL_LOGFILE_CONTEXT_TRACE1( aLog, "framework (cd100003) Element %s", aResName.getStr() );

        if (( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) &&
              aElementName.equalsIgnoreAsciiCaseAscii( "statusbar" )) ||
            ( m_aStatusBarElement.m_aName == ResourceURL ))
        {
            implts_readStatusBarState( ResourceURL );
            if ( m_aStatusBarElement.m_bVisible && !m_aStatusBarElement.m_bMasterHide )
            {
                createElement( ResourceURL );

                // There are some situation where we are not able to create an element.
                // Therefore we have to check the reference before further action.
                // See #i70019#
                css::uno::Reference< css::ui::XUIElement > xUIElement( m_aStatusBarElement.m_xUIElement );
                if ( xUIElement.is() )
                {
                    // we need VCL here to pass special flags to Show()
                    SolarMutexGuard aGuard;
                    Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow )
                    {
                        pWindow->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                        doLayout();
                        bResult = sal_True;
                        bNotify = sal_True;
                    }
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" ) )
        {
            aWriteLock.unlock();
            implts_showProgressBar();
            doLayout();
            bResult = sal_True;
            bNotify = sal_True;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            if ( m_bVisible )
            {
                bool bFound( false );
                bool bShowElement( false );

                for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
                {
                    if ( pIter->m_aName == ResourceURL )
                    {
                        bFound       = sal_True;
                        bShowElement = ( pIter->m_bVisible && !pIter->m_bMasterHide && m_bParentWindowVisible );

                        Reference< css::awt::XWindow2 > xContainerWindow( m_xContainerWindow, UNO_QUERY );
                        if ( xContainerWindow.is() && pIter->m_bFloating )
                            bShowElement = ( bShowElement && xContainerWindow->isActive() );

                        if ( pIter->m_xUIElement.is() )
                        {
                            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
                            Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                            if ( xDockWindow.is() && xDockWindow->isFloating() )
                                bShowElement = ( bShowElement && xContainerWindow->isActive() );

                            if ( xDockWindow.is() && bShowElement )
                            {
                                pIter->m_bVisible = sal_True;
                                aWriteLock.unlock();

                                // we need VCL here to pass special flags to Show()
                                SolarMutexGuard aGuard;
                                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                                if( pWindow && !pWindow->IsReallyVisible() )
                                {
                                    pWindow->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                                    implts_writeNewStateData( ResourceURL, xWindow );

                                    if ( xDockWindow.is() && !xDockWindow->isFloating() )
                                        doLayout();
                                    bResult = sal_True;
                                    bNotify = sal_True;
                                }

                                bResult = sal_False;
                            }
                        } // if ( pIter->m_xUIElement.is() )
                        break;
                    }
                }

                // Create toolbar on demand when it's visible
                if ( !bResult )
                {
                    Reference< ::com::sun::star::ui::XUIElement > xUIElement;
                    if ( !bFound )
                    {
                        UIElement aNewToolbar( aElementName, aElementType, xUIElement );
                        aNewToolbar.m_aName = ResourceURL;
                        implts_readWindowStateData( ResourceURL, aNewToolbar );
                        implts_insertUIElement( aNewToolbar );
                        aWriteLock.unlock();

                        implts_sortUIElements();
                        if ( aNewToolbar.m_bVisible )
                            createElement( ResourceURL );
                        bResult = sal_True;
                        bNotify = sal_True;
                    }
                    else if ( bShowElement )
                    {
                        aWriteLock.unlock();

                        createElement( ResourceURL );
                        bResult = sal_True;
                        bNotify = sal_True;
                    }
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "dockingwindow" ))
        {
            css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame );
            aWriteLock.unlock();

            CreateDockingWindow( xFrame, aElementName );
        }
    }

    if ( bNotify )
    {
        // UI element is visible - provide information to listeners
        implts_notifyListeners( css::frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( ResourceURL ) );
    }

    return bResult;
}

Reference< XUIElement > SAL_CALL LayoutManager::getElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    ::rtl::OUString                aElementType;
    ::rtl::OUString                aElementName;
    Reference< XUIElement > xElement;

    implts_findElement( aName, aElementType, aElementName, xElement );
    return xElement;
}

Sequence< Reference< ::com::sun::star::ui::XUIElement > > SAL_CALL LayoutManager::getElements()
throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard   aReadLock( m_aLock );

    sal_Bool  bMenuBar( sal_False );
    sal_Bool  bStatusBar( sal_False );
    sal_Int32 nSize = m_aUIElements.size();

    if ( m_xMenuBar.is() )
    {
        ++nSize;
        bMenuBar = sal_True;
    }
    if ( m_aStatusBarElement.m_xUIElement.is() )
    {
        ++nSize;
        bStatusBar = sal_True;
    }

    Sequence< Reference< ::com::sun::star::ui::XUIElement > > aSeq( nSize );

    sal_Int32 nIndex = 0;
    UIElementVector::const_iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_xUIElement.is() )
            aSeq[nIndex++] = pIter->m_xUIElement;
    }
    if ( bMenuBar )
        aSeq[nIndex++] = m_xMenuBar;
    if ( bStatusBar )
        aSeq[nIndex++] = m_aStatusBarElement.m_xUIElement;

    // Resize sequence as we now know our correct size
    aSeq.realloc( nIndex );

    return aSeq;
}

sal_Bool SAL_CALL LayoutManager::showElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::showElement" );

    sal_Bool    bResult( sal_False );
    sal_Bool    bNotify( sal_False );
    ::rtl::OUString    aElementType;
    ::rtl::OUString    aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        ::rtl::OString aResName = rtl::OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
        RTL_LOGFILE_CONTEXT_TRACE1( aLog, "framework (cd100003) Element %s", aResName.getStr() );

        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            WriteGuard aWriteLock( m_aLock );
            m_bMenuVisible = sal_True;
            aWriteLock.unlock();

            bResult = implts_resetMenuBar();
            bNotify = bResult;
        }
        else if (( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) &&
                   aElementName.equalsIgnoreAsciiCaseAscii( "statusbar" )) ||
                 ( m_aStatusBarElement.m_aName == aName ))
        {
            WriteGuard aWriteLock( m_aLock );
            if ( m_aStatusBarElement.m_xUIElement.is() &&
                 !m_aStatusBarElement.m_bMasterHide )
            {
                if ( implts_showStatusBar( sal_True ))
                {
                    implts_writeWindowStateData( m_aStatusBarAlias, m_aStatusBarElement );
                    doLayout();
                    bResult = sal_True;
                    bNotify = sal_True;
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" ))
        {
            bNotify = bResult = implts_showProgressBar();
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            UIElementVector::iterator pIter;

            WriteGuard aWriteLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
                {
                    UIElement aUIElement = *pIter;
                    Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
                    Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                    sal_Bool bShowElement( !pIter->m_bMasterHide && m_bParentWindowVisible );

                    pIter->m_bVisible = sal_True;
                    aWriteLock.unlock();

                    implts_writeWindowStateData( aUIElement.m_aName, aUIElement );
                    implts_sortUIElements();

                    if ( xDockWindow.is() && bShowElement )
                    {
                        // we need VCL here to pass special flags to Show()
                        SolarMutexGuard aGuard;
                        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                        if( pWindow )
                        {
                            pWindow->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                            bNotify = sal_True;
                        }
                        implts_writeNewStateData( aName, xWindow );

                        if ( xDockWindow.is() && !xDockWindow->isFloating() )
                            doLayout();

                        bResult = sal_True;
                    } // if ( xDockWindow.is() && bShowElement )
                    break;
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "dockingwindow" ))
        {
            ReadGuard aReadGuard( m_aLock );
            css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame );
            css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR( m_xSMGR );
            aReadGuard.unlock();

            impl_setDockingWindowVisibility( xSMGR, xFrame, aElementName, true );
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolpanel" ))
        {
            ReadGuard aReadGuard( m_aLock );
            css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame );
            aReadGuard.unlock();

            ActivateToolPanel( m_xFrame, aName );
        }
    }

    if ( bNotify )
    {
        // UI element is visible - provide information to listeners
        implts_notifyListeners( css::frame::LayoutManagerEvents::UIELEMENT_VISIBLE, uno::makeAny( aName ) );
    }

    return bResult;
}

sal_Bool SAL_CALL LayoutManager::hideElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::hideElement" );


    sal_Bool            bNotify( sal_False );
    ::rtl::OUString            aElementType;
    ::rtl::OUString            aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        ::rtl::OString aResName = rtl::OUStringToOString( aElementName, RTL_TEXTENCODING_ASCII_US );
        RTL_LOGFILE_CONTEXT_TRACE1( aLog, "framework (cd100003) Element %s", aResName.getStr() );

        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            WriteGuard aWriteLock( m_aLock );

            if ( m_xContainerWindow.is() )
            {
                SolarMutexGuard aGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                m_bMenuVisible = sal_False;
                if ( pWindow )
                {
                    MenuBar* pMenuBar = ((SystemWindow *)pWindow)->GetMenuBar();
                    if ( pMenuBar )
                    {
                        pMenuBar->SetDisplayable( sal_False );
                        bNotify = sal_True;
                    }
                }
            }
        }
        else if (( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) &&
                   aElementName.equalsIgnoreAsciiCaseAscii( "statusbar" )) ||
                 ( m_aStatusBarElement.m_aName == aName ))
        {
            WriteGuard aWriteLock( m_aLock );
            if ( m_aStatusBarElement.m_xUIElement.is() &&
                 !m_aStatusBarElement.m_bMasterHide )
            {
                if ( implts_hideStatusBar( sal_True ))
                {
                    implts_writeWindowStateData( m_aStatusBarAlias, m_aStatusBarElement );
                    doLayout();
                    bNotify = sal_True;
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                  aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" ))
        {
            bNotify = implts_hideProgressBar();
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            UIElementVector::iterator pIter;

            WriteGuard aWriteLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
                    Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                    if ( xDockWindow.is() )
                    {
                        pIter->m_bVisible = sal_False;
                        aWriteLock.unlock();

                        xWindow->setVisible( sal_False );
                        implts_writeNewStateData( aName, xWindow );

                        if ( xDockWindow.is() && !xDockWindow->isFloating() )
                            doLayout();

                        bNotify = sal_True;
                    } // if ( xDockWindow.is() )
                    break;
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "dockingwindow" ))
        {
            ReadGuard aReadGuard( m_aLock );
            css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame );
            css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR( m_xSMGR );
            aReadGuard.unlock();

            impl_setDockingWindowVisibility( xSMGR, xFrame, aElementName, false );
        }
    }

    if ( bNotify )
    {
        // UI element is visible - provide information to listeners
        implts_notifyListeners( css::frame::LayoutManagerEvents::UIELEMENT_INVISIBLE, uno::makeAny( aName ) );
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::dockWindow( const ::rtl::OUString& aName, DockingArea DockingArea, const css::awt::Point& Pos )
throw (RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( aName, aUIElement ) && aUIElement.m_xUIElement.is() )
    {
        try
        {
            Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
            if ( xDockWindow.is() )
            {
                if ( DockingArea != DockingArea_DOCKINGAREA_DEFAULT )
                    aUIElement.m_aDockedData.m_nDockedArea = sal_Int16( DockingArea );

                if (( Pos.X != SAL_MAX_INT32 ) && ( Pos.Y != SAL_MAX_INT32 ))
                    aUIElement.m_aDockedData.m_aPos = ::Point( Pos.X, Pos.Y );

                if ( !xDockWindow->isFloating() )
                {
                    Window*     pWindow( 0 );
                    ToolBox*    pToolBox( 0 );

                    {
                        SolarMutexGuard aGuard;
                        pWindow = VCLUnoHelper::GetWindow( xWindow );
                        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                        {
                            pToolBox = (ToolBox *)pWindow;

                            // We have to set the alignment of the toolbox. It's possible that the toolbox is moved from a
                            // horizontal to a vertical docking area!
                            pToolBox->SetAlign( ImplConvertAlignment( aUIElement.m_aDockedData.m_nDockedArea ));
                        }
                    }

                    if (( aUIElement.m_aDockedData.m_aPos.X() == SAL_MAX_INT32 ) ||
                        ( aUIElement.m_aDockedData.m_aPos.Y() == SAL_MAX_INT32 ))
                    {
                        // Docking on its default position without a preset position -
                        // we have to find a good place for it.
                        ::Size      aSize;

                        SolarMutexGuard aGuard;
                        {
                            if ( pToolBox )
                                aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( aUIElement.m_aDockedData.m_nDockedArea ) );
                            else
                                aSize = pWindow->GetSizePixel();
                        }

                        ::Point aPixelPos;
                        ::Point aDockPos;
                        implts_findNextDockingPos( (::com::sun::star::ui::DockingArea)aUIElement.m_aDockedData.m_nDockedArea,
                                                aSize,
                                                aDockPos,
                                                aPixelPos );
                        aUIElement.m_aDockedData.m_aPos = aDockPos;
                    }
                }

                WriteGuard aWriteLock( m_aLock );
                UIElement& rUIElement = LayoutManager::impl_findElement( aUIElement.m_aName );
                if ( rUIElement.m_aName == aName )
                {
                    rUIElement.m_aDockedData.m_nDockedArea = aUIElement.m_aDockedData.m_nDockedArea;
                    rUIElement.m_aDockedData.m_aPos        = aUIElement.m_aDockedData.m_aPos;
                }
                aWriteLock.unlock();

                if ( xDockWindow->isFloating() )
                {
                    // Will call toggle floating mode which will do the rest!
                    xWindow->setVisible( sal_False );
                    xDockWindow->setFloatingMode( sal_False );
                    xWindow->setVisible( sal_True );
                }
                else
                {
                    implts_writeWindowStateData( aName, aUIElement );
                    implts_sortUIElements();

                    if ( aUIElement.m_bVisible )
                        doLayout();
                }

                return sal_True;
            }
        }
        catch ( DisposedException& )
        {
        }
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::dockAllWindows( ::sal_Int16 nElementType ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( nElementType == UIElementType::TOOLBAR )
    {
        std::vector< rtl::OUString > aToolBarNameVector;

        ::rtl::OUString                  aElementType;
        ::rtl::OUString                  aElementName;

        {
            ReadGuard aReadLock( m_aLock );
            UIElementVector::iterator pIter;
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_aType.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("toolbar")) &&
                     pIter->m_xUIElement.is() &&
                     pIter->m_bFloating &&
                     pIter->m_bVisible )
                    aToolBarNameVector.push_back( pIter->m_aName );
            }
        }

        const sal_uInt32 nCount = aToolBarNameVector.size();
        for ( sal_uInt32 i = 0; i < nCount; ++i )
        {
            ::com::sun::star::awt::Point aPoint;
            aPoint.X = aPoint.Y = SAL_MAX_INT32;
            dockWindow( aToolBarNameVector[i], DockingArea_DOCKINGAREA_DEFAULT, aPoint );
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::floatWindow( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( aName, aUIElement ))
    {
        if ( aUIElement.m_xUIElement.is() )
        {
            try
            {
                Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xWindow.is() &&  xDockWindow.is() )
                {
                    if ( !xDockWindow->isFloating() )
                    {
                        xDockWindow->setFloatingMode( sal_True );
                        return sal_True;
                    }
                }
            }
            catch ( DisposedException& )
            {
            }
        }
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::lockWindow( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( ResourceURL, aUIElement ))
    {
        if ( aUIElement.m_xUIElement.is() )
        {
            try
            {
                Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow &&
                     pWindow->IsVisible() &&
                     xDockWindow.is() &&
                     !xDockWindow->isFloating() )
                {
                    aUIElement.m_aDockedData.m_bLocked = sal_True;
                    implts_writeWindowStateData( ResourceURL, aUIElement );
                    xDockWindow->lock();

                    // Write back lock state
                    WriteGuard aWriteLock( m_aLock );
                    UIElement& rUIElement = LayoutManager::impl_findElement( aUIElement.m_aName );
                    if ( rUIElement.m_aName == aUIElement.m_aName )
                        rUIElement.m_aDockedData.m_bLocked = aUIElement.m_aDockedData.m_bLocked;
                    aWriteLock.unlock();

                    doLayout();
                    return sal_True;
                }
            }
            catch ( DisposedException& )
            {
            }
        }
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::unlockWindow( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( ResourceURL, aUIElement ))
    {
        if ( aUIElement.m_xUIElement.is() )
        {
            try
            {
                Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow &&
                     pWindow->IsVisible() &&
                     xDockWindow.is() &&
                     !xDockWindow->isFloating() )
                {
                    aUIElement.m_aDockedData.m_bLocked = sal_False;
                    implts_writeWindowStateData( ResourceURL, aUIElement );
                    xDockWindow->unlock();

                    // Write back lock state
                    WriteGuard aWriteLock( m_aLock );
                    UIElement& rUIElement = LayoutManager::impl_findElement( aUIElement.m_aName );
                    if ( rUIElement.m_aName == aUIElement.m_aName )
                        rUIElement.m_aDockedData.m_bLocked = aUIElement.m_aDockedData.m_bLocked;
                    aWriteLock.unlock();

                    doLayout();
                    return sal_True;
                }
            }
            catch ( DisposedException& )
            {
            }
        }
    }

    return sal_False;
}

void SAL_CALL LayoutManager::setElementSize( const ::rtl::OUString& aName, const css::awt::Size& aSize )
throw (RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( aName, aUIElement ))
    {
        if ( aUIElement.m_xUIElement.is() )
        {
            try
            {
                Reference< css::awt::XWindow >  xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XWindow2 > xWindow2( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                if ( xWindow.is() && xWindow2.is() && xDockWindow.is() )
                {
                    if ( aUIElement.m_bFloating )
                    {
                        xWindow2->setOutputSize( aSize );
                        implts_writeNewStateData( aName, xWindow );
                    }
                }
            }
            catch ( DisposedException& )
            {
            }
        }
    }
}

void SAL_CALL LayoutManager::setElementPos( const ::rtl::OUString& aName, const css::awt::Point& aPos )
throw (RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( aName, aUIElement ) && aUIElement.m_xUIElement.is() )
    {
        try
        {
            Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

            if ( xWindow.is() && xDockWindow.is() )
            {
                if ( aUIElement.m_bFloating )
                {
                    xWindow->setPosSize( aPos.X, aPos.Y, 0, 0, css::awt::PosSize::POS );
                    implts_writeNewStateData( aName, xWindow );
                }
                else
                {
                    WriteGuard aWriteLock( m_aLock );
                    UIElement& rUIElement = LayoutManager::impl_findElement( aUIElement.m_aName );
                    if ( rUIElement.m_aName == aName )
                        rUIElement.m_aDockedData.m_aPos = ::Point( aPos.X, aPos.Y );
                    aWriteLock.unlock();

                    aUIElement.m_aDockedData.m_aPos = ::Point( aPos.X, aPos.Y );
                    implts_writeWindowStateData( aName, aUIElement );
                    implts_sortUIElements();

                    if ( aUIElement.m_bVisible )
                        doLayout();
                }
            }
        }
        catch ( DisposedException& )
        {
        }
    }
}

void SAL_CALL LayoutManager::setElementPosSize( const ::rtl::OUString& aName, const css::awt::Point& aPos, const css::awt::Size& aSize )
throw (RuntimeException)
{
    UIElement aUIElement;

    if ( implts_findElement( aName, aUIElement ))
    {
        if ( aUIElement.m_xUIElement.is() )
        {
            try
            {
                Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XWindow2 > xWindow2( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                if ( xWindow.is() && xWindow2.is() && xDockWindow.is() )
                {
                    if ( aUIElement.m_bFloating )
                    {
                        xWindow2->setPosSize( aPos.X, aPos.Y, 0, 0, css::awt::PosSize::POS );
                        xWindow2->setOutputSize( aSize );
                        implts_writeNewStateData( aName, xWindow );
                    }
                    else
                    {
                        WriteGuard aWriteLock( m_aLock );
                        UIElement& rUIElement = LayoutManager::impl_findElement( aUIElement.m_aName );
                        if ( rUIElement.m_aName == aName )
                            rUIElement.m_aDockedData.m_aPos = ::Point( aPos.X, aPos.Y );
                        aWriteLock.unlock();

                        aUIElement.m_aDockedData.m_aPos = ::Point( aPos.X, aPos.Y );
                        implts_writeWindowStateData( aName, rUIElement );
                        implts_sortUIElements();

                        if ( aUIElement.m_bVisible )
                            doLayout();
                    }
                }
            }
            catch ( DisposedException& )
            {
            }
        }
    }
}

sal_Bool SAL_CALL LayoutManager::isElementVisible( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    ::rtl::OUString aElementType;
    ::rtl::OUString aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            ReadGuard aReadLock( m_aLock );
            if ( m_xContainerWindow.is() )
            {
                aReadLock.unlock();

                SolarMutexGuard aGuard;
                Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                if ( pWindow )
                {
                    MenuBar* pMenuBar = ((SystemWindow *)pWindow)->GetMenuBar();
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
        else if (( aElementType.equalsIgnoreAsciiCaseAscii( "statusbar" ) &&
                   aElementName.equalsIgnoreAsciiCaseAscii( "statusbar" )) ||
                 ( m_aStatusBarElement.m_aName == aName ))
        {
            if ( m_aStatusBarElement.m_xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow(
                    m_aStatusBarElement.m_xUIElement->getRealInterface(), UNO_QUERY );
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
        else if (( aElementType.equalsIgnoreAsciiCaseAscii( "progressbar" ) &&
                   aElementName.equalsIgnoreAsciiCaseAscii( "progressbar" )))
        {
            if ( m_aProgressBarElement.m_xUIElement.is() )
                return m_aProgressBarElement.m_bVisible;
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            UIElementVector::const_iterator pIter;

            ReadGuard aReadLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
                    if ( xWindow.is() )
                    {
                        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                        return pWindow && pWindow->IsVisible();
                    }
                }
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "dockingwindow" ))
        {
            ReadGuard aReadGuard( m_aLock );
            css::uno::Reference< css::frame::XFrame > xFrame( m_xFrame );
            aReadGuard.unlock();

            return IsDockingWindowVisible( xFrame, aElementName );
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementFloating( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            return xDockWindow.is() && xDockWindow->isFloating();
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementDocked( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            return xDockWindow.is() && !xDockWindow->isFloating();
        }
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::isElementLocked( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        if (( pIter->m_aName == ResourceURL ) && ( pIter->m_xUIElement.is() ))
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            return xDockWindow.is() && !xDockWindow->isLocked();
        }
    }

    return sal_False;
}

css::awt::Size SAL_CALL LayoutManager::getElementSize( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    UIElement aElementData;
    if ( implts_findElement( aName,aElementData  ) && aElementData.m_xUIElement.is() )
    {
        Reference< css::awt::XWindow > xWindow( aElementData.m_xUIElement->getRealInterface(), UNO_QUERY );
        if ( xWindow.is() )
        {
            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow )
            {
                ::Size aSize = pWindow->GetSizePixel();
                css::awt::Size aElementSize;
                aElementSize.Width  = aSize.Width();
                aElementSize.Height = aSize.Height();
                return aElementSize;
            } // if ( pWindow )
        }
    }
    return css::awt::Size();
}

css::awt::Point SAL_CALL LayoutManager::getElementPos( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    UIElement aElementData;
    if ( implts_findElement( aName,aElementData  ) && aElementData.m_xUIElement.is() )
    {
        Reference< css::awt::XWindow > xWindow( aElementData.m_xUIElement->getRealInterface(), UNO_QUERY );
        Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
        if ( xDockWindow.is() )
        {
            css::awt::Point aPos;
            if ( aElementData.m_bFloating )
            {
                css::awt::Rectangle aRect = xWindow->getPosSize();
                aPos.X = aRect.X;
                aPos.Y = aRect.Y;
            }
            else
            {
                ::Point aVirtualPos = aElementData.m_aDockedData.m_aPos;
                aPos.X = aVirtualPos.X();
                aPos.Y = aVirtualPos.Y();
            }

            return aPos;
        }
    }

    return css::awt::Point();
}

void SAL_CALL LayoutManager::lock()
throw (RuntimeException)
{
    implts_lock();

    ReadGuard aReadLock( m_aLock );
    sal_Int32 nLockCount( m_nLockCount );
    aReadLock.unlock();

    RTL_LOGFILE_TRACE1( "framework (cd100003) ::LayoutManager::lock lockCount=%d", nLockCount );
#ifdef DBG_UTIL
    ByteString aStr("LayoutManager::lock ");
    aStr += ByteString::CreateFromInt32((long)this);
    aStr += " - ";
    aStr += ByteString::CreateFromInt32(nLockCount);
    DBG_TRACE( aStr.GetBuffer() );
#endif

    Any a( nLockCount );
    implts_notifyListeners( css::frame::LayoutManagerEvents::LOCK, a );
}

void SAL_CALL LayoutManager::unlock()
throw (RuntimeException)
{
    sal_Bool bDoLayout( implts_unlock() );

    ReadGuard aReadLock( m_aLock );
    sal_Int32 nLockCount( m_nLockCount );
    aReadLock.unlock();

    RTL_LOGFILE_TRACE1( "framework (cd100003) ::LayoutManager::unlock lockCount=%d", nLockCount );
#ifdef DBG_UTIL
    ByteString aStr("LayoutManager::unlock ");
    aStr += ByteString::CreateFromInt32((long)this);
    aStr += " - ";
    aStr += ByteString::CreateFromInt32(nLockCount);
    DBG_TRACE( aStr.GetBuffer() );
#endif
    // conform to documentation: unlock with lock count == 0 means force a layout

    WriteGuard aWriteLock( m_aLock );
    if ( bDoLayout )
        m_aAsyncLayoutTimer.Stop();
    aWriteLock.unlock();

    Any a( nLockCount );
    implts_notifyListeners( css::frame::LayoutManagerEvents::UNLOCK, a );

    if ( bDoLayout )
      implts_doLayout_notify( sal_True );
}

void SAL_CALL LayoutManager::doLayout()
throw (RuntimeException)
{
    implts_doLayout_notify( sal_True );
}

void LayoutManager::implts_doLayout_notify( sal_Bool bOuterResize )
{
    sal_Bool bLayouted = implts_doLayout( sal_False, bOuterResize );
    if ( bLayouted )
        implts_notifyListeners( css::frame::LayoutManagerEvents::LAYOUT, Any() );
}

sal_Bool LayoutManager::implts_doLayout( sal_Bool bForceRequestBorderSpace, sal_Bool bOuterResize )
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::implts_doLayout" );

    sal_Bool bNoLock( sal_False );
    css::awt::Rectangle aCurrBorderSpace;
    Reference< css::awt::XWindow > xContainerWindow;
    Reference< css::awt::XTopWindow2 > xContainerTopWindow;
    Reference< css::awt::XWindow > xComponentWindow;
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor;
    bool bPreserveContentSize( false );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    sal_Bool bMustDoLayout( m_bMustDoLayout );
    if ( !m_bParentWindowVisible )
        return sal_False;

    bNoLock = ( m_nLockCount == 0 );
    xContainerWindow = m_xContainerWindow;
    xContainerTopWindow = m_xContainerTopWindow;
    xComponentWindow = m_xFrame->getComponentWindow();
    xDockingAreaAcceptor = m_xDockingAreaAcceptor;
    aCurrBorderSpace = m_aDockingArea;
    bPreserveContentSize = m_bPreserveContentSize;
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    sal_Bool bLayouted( sal_False );

    if ( bNoLock &&
         xDockingAreaAcceptor.is() &&
         xContainerWindow.is() &&
         xComponentWindow.is() )
    {
        bLayouted = sal_True;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteGuard( m_aLock );
        m_bDoLayout = sal_True;
        aWriteGuard.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        css::awt::Rectangle     aBorderSpace = implts_calcDockingAreaSizes();
        sal_Bool                bGotRequestedBorderSpace( sal_True );
        sal_Bool                bEqual = implts_compareRectangles( aBorderSpace, aCurrBorderSpace );

        if ( !bEqual || bForceRequestBorderSpace || bMustDoLayout )
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
            css::awt::Rectangle aComponentRect = xComponentWindow->getPosSize();
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
            {
                bGotRequestedBorderSpace = xDockingAreaAcceptor->requestDockingAreaSpace( aBorderSpace );
                if ( bGotRequestedBorderSpace )
                    xDockingAreaAcceptor->setDockingAreaSpace( aBorderSpace );
            }

            if ( bGotRequestedBorderSpace )
            {
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */
                aWriteGuard.lock();
                m_aDockingArea = aBorderSpace;
                m_bMustDoLayout = sal_False;
                aWriteGuard.unlock();
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            }
        }

        if ( bGotRequestedBorderSpace )
        {
            ::Size                      aContainerSize;
            ::Size                      aStatusBarSize;

            aStatusBarSize = implts_getStatusBarSize();
            aBorderSpace.Height -= aStatusBarSize.Height();
            implts_setDockingAreaWindowSizes( aBorderSpace );

            // Subtract status bar size from our container output size. Docking area windows
            // don't contain the status bar!
            aContainerSize = implts_getContainerWindowOutputSize();
            aContainerSize.Height() -= aStatusBarSize.Height();

            // Retrieve row/column dependent data from all docked user-interface elements
            for ( sal_Int32 i = 0; i < DOCKINGAREAS_COUNT; i++ )
            {
                std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

                implts_getDockingAreaElementInfos( (DockingArea)i, aRowColumnsWindowData );

                sal_Int32 nOffset( 0 );
                const sal_uInt32 nCount = aRowColumnsWindowData.size();
                for ( sal_uInt32 j = 0; j < nCount; ++j )
                {
                    implts_calcWindowPosSizeOnSingleRowColumn( i, nOffset, aRowColumnsWindowData[j], aContainerSize );
                    nOffset += aRowColumnsWindowData[j].nStaticSize;
                }
            }

            // Position the status bar
            if ( aStatusBarSize.Height() > 0 )
            {
                implts_setStatusBarPosSize( ::Point( 0, std::max(( aContainerSize.Height() ), long( 0 ))),
                                            ::Size( aContainerSize.Width(),aStatusBarSize.Height() ));
            }

            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            aWriteGuard.lock();
            m_bDoLayout = sal_False;
            aWriteGuard.unlock();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        }
    }

    return bLayouted;
}

sal_Bool LayoutManager::implts_compareRectangles( const css::awt::Rectangle& rRect1,
                                                  const css::awt::Rectangle& rRect2 )
{
    return (( rRect1.X == rRect2.X ) &&
            ( rRect1.Y == rRect2.Y ) &&
            ( rRect1.Width == rRect2.Width ) &&
            ( rRect1.Height == rRect2.Height ));
}

sal_Bool LayoutManager::implts_resizeContainerWindow( const awt::Size& rContainerSize,
                                                      const awt::Point& rComponentPos )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Reference< awt::XWindow >               xContainerWindow    = m_xContainerWindow;
    Reference< awt::XTopWindow2 >           xContainerTopWindow = m_xContainerTopWindow;
    Reference< awt::XWindow >               xComponentWindow    = m_xFrame->getComponentWindow();
    Reference< container::XIndexAccess >    xDisplayAccess      = m_xDisplayAccess;
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // calculate the maximum size we have for the container window
    awt::Rectangle aWorkArea;
    try
    {
        sal_Int32 nDisplay = xContainerTopWindow->getDisplay();
        Reference< beans::XPropertySet > xDisplayInfo( xDisplayAccess->getByIndex( nDisplay ), UNO_QUERY_THROW );
        OSL_VERIFY( xDisplayInfo->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "WorkArea" ) ) ) >>= aWorkArea );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    if ( ( aWorkArea.Width > 0 ) && ( aWorkArea.Height > 0 ) )
    {
        if  (   ( rContainerSize.Width > aWorkArea.Width )
            ||  ( rContainerSize.Height > aWorkArea.Height )
            )
            return sal_False;
        // Strictly, this is not correct. If we have a multi-screen display (css.awt.DisplayAccess.MultiDisplay == true),
        // the the "effective work area" would be much larger than the work area of a single display, since we could in theory
        // position the container window across multiple screens.
        // However, this should suffice as a heuristics here ... (nobody really wants to check whethere the different screens are
        // stacked horizontally or vertically, whether their work areas can really be combined, or are separated by non-work-areas,
        // and the like ... right?)
    }

    // resize our container window
    xContainerWindow->setPosSize( 0, 0, rContainerSize.Width, rContainerSize.Height, css::awt::PosSize::SIZE );
    // position the component window
    xComponentWindow->setPosSize( rComponentPos.X, rComponentPos.Y, 0, 0, css::awt::PosSize::POS );
    return sal_True;
}

void SAL_CALL LayoutManager::setVisible( sal_Bool bVisible )
throw (::com::sun::star::uno::RuntimeException)
{
    sal_Bool bWasVisible( sal_True );

    WriteGuard aWriteLock( m_aLock );
    bWasVisible = m_bVisible;
    m_bVisible = bVisible;
    aWriteLock.unlock();

    if ( bWasVisible != bVisible )
        implts_setVisibleState( bVisible );
}

sal_Bool SAL_CALL LayoutManager::isVisible()
throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );
    return m_bVisible;
}

void LayoutManager::implts_calcWindowPosSizeOnSingleRowColumn( sal_Int32 nDockingArea,
                                                               sal_Int32 nOffset,
                                                               SingleRowColumnWindowData& rRowColumnWindowData,
                                                               const ::Size& rContainerSize )
{
    sal_Int32   nDiff( 0 );
    sal_Int32   nRCSpace( rRowColumnWindowData.nSpace );
    sal_Int32   nTopDockingAreaSize;
    sal_Int32   nBottomDockingAreaSize;
    sal_Int32   nContainerClientSize;

    if ( rRowColumnWindowData.aRowColumnWindows.empty() )
        return;

    if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
        ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
    {
        nContainerClientSize = rContainerSize.Width();
        nDiff = nContainerClientSize - rRowColumnWindowData.nVarSize;
    }
    else
    {
        nTopDockingAreaSize    = implts_getTopBottomDockingAreaSizes().Width();
        nBottomDockingAreaSize = implts_getTopBottomDockingAreaSizes().Height();
        nContainerClientSize   = ( rContainerSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize );
        nDiff = nContainerClientSize - rRowColumnWindowData.nVarSize;
    }

    const sal_uInt32 nCount = rRowColumnWindowData.aRowColumnWindowSizes.size();
    if (( nDiff < 0 ) && ( nRCSpace > 0 ))
    {
        // First we try to reduce the size of blank space before/behind docked windows
        sal_Int32 i = nCount - 1;
        while ( i >= 0 )
        {
            sal_Int32 nSpace = rRowColumnWindowData.aRowColumnSpace[i];
            if ( nSpace >= -nDiff )
            {

                if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                    ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount ; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X += nDiff;
                }
                else
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount ; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y += nDiff;
                }
                nDiff = 0;

                break;
            }
            else if ( nSpace > 0 )
            {
                if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                    ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X -= nSpace;
                }
                else
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y -= nSpace;
                }
                nDiff += nSpace;
            }
            --i;
        }
    }

    // Check if we have to reduce further
    if ( nDiff < 0 )
    {
        // Now we have to reduce the size of certain docked windows
        sal_Int32 i = sal_Int32( nCount - 1 );
        while ( i >= 0 )
        {
            css::awt::Rectangle& rWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
            ::Size               aMinSize;

            SolarMutexGuard aGuard;
            {
                Reference< css::awt::XWindow > xWindow = rRowColumnWindowData.aRowColumnWindows[i];
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                    aMinSize = ((ToolBox *)pWindow)->CalcMinimumWindowSizePixel();
            }

            if (( aMinSize.Width() > 0 ) && ( aMinSize.Height() > 0 ))
            {
                if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                    ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
                {
                    sal_Int32 nMaxReducation = ( rWinRect.Width - aMinSize.Width() );
                    if ( nMaxReducation >= -nDiff )
                    {
                        rWinRect.Width = rWinRect.Width + nDiff;
                        nDiff = 0;
                    }
                    else
                    {
                        rWinRect.Width = aMinSize.Width();
                        nDiff += nMaxReducation;
                    }

                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X += nDiff;
                }
                else
                {
                    sal_Int32 nMaxReducation = ( rWinRect.Height - aMinSize.Height() );
                    if ( nMaxReducation >= -nDiff )
                    {
                        rWinRect.Height = rWinRect.Height + nDiff;
                        nDiff = 0;
                    }
                    else
                    {
                        rWinRect.Height = aMinSize.Height();
                        nDiff += nMaxReducation;
                    }

                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_uInt32 j = i; j < nCount; j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y += nDiff;
                }
            }

            if ( nDiff >= 0 )
                break;

            --i;
        }
    }

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Window* pDockAreaWindow = VCLUnoHelper::GetWindow( m_xDockAreaWindows[nDockingArea] );
    aReadLock.unlock();

    sal_Int32 nCurrPos( 0 );
    sal_Int32 nStartOffset( 0 );

    if ( nDockingArea == DockingArea_DOCKINGAREA_RIGHT )
        nStartOffset = pDockAreaWindow->GetSizePixel().Width() - rRowColumnWindowData.nStaticSize;
    else if ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM )
        nStartOffset = pDockAreaWindow->GetSizePixel().Height() - rRowColumnWindowData.nStaticSize;

    SolarMutexGuard aGuard;
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        Reference< css::awt::XWindow > xWindow = rRowColumnWindowData.aRowColumnWindows[i];
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
        Window* pOldParentWindow = pWindow->GetParent();

        if ( pDockAreaWindow != pOldParentWindow )
            pWindow->SetParent( pDockAreaWindow );

        css::awt::Rectangle aWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
        if ( nDockingArea == DockingArea_DOCKINGAREA_TOP )
        {
            if ( aWinRect.X < nCurrPos )
                aWinRect.X = nCurrPos;
            pWindow->SetPosSizePixel( ::Point( aWinRect.X, nOffset ),
                                      ::Size( aWinRect.Width, rRowColumnWindowData.nStaticSize ));
            nCurrPos += ( aWinRect.X - nCurrPos ) + aWinRect.Width;
        }
        else if ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM )
        {
            if ( aWinRect.X < nCurrPos )
                aWinRect.X = nCurrPos;
            pWindow->SetPosSizePixel( ::Point( aWinRect.X, nStartOffset - nOffset ),
                                      ::Size( aWinRect.Width, rRowColumnWindowData.nStaticSize ));
            nCurrPos += ( aWinRect.X - nCurrPos ) + aWinRect.Width;
        }
        else if ( nDockingArea == DockingArea_DOCKINGAREA_LEFT )
        {
            if ( aWinRect.Y < nCurrPos )
                aWinRect.Y = nCurrPos;
            pWindow->SetPosSizePixel( ::Point( nOffset, aWinRect.Y ),
                                      ::Size( rRowColumnWindowData.nStaticSize, aWinRect.Height ));
            nCurrPos += ( aWinRect.Y - nCurrPos ) + aWinRect.Height;
        }
        else if ( nDockingArea == DockingArea_DOCKINGAREA_RIGHT )
        {
            if ( aWinRect.Y < nCurrPos )
                aWinRect.Y = nCurrPos;
            pWindow->SetPosSizePixel( ::Point( nStartOffset - nOffset, aWinRect.Y ),
                                      ::Size( rRowColumnWindowData.nStaticSize, aWinRect.Height ));
            nCurrPos += ( aWinRect.Y - nCurrPos ) + aWinRect.Height;
        }
    }
}

::Size LayoutManager::implts_getStatusBarSize()
{
    ReadGuard aReadLock( m_aLock );
    sal_Bool bStatusBarVisible( isElementVisible( m_aStatusBarAlias ));
    sal_Bool bProgressBarVisible( isElementVisible( m_aProgressBarAlias ));
    sal_Bool bVisible = m_bVisible;
    Reference< XUIElement > xStatusBar = m_aStatusBarElement.m_xUIElement;
    Reference< XUIElement > xProgressBar = m_aProgressBarElement.m_xUIElement;

    Reference< css::awt::XWindow > xWindow;
    if ( bStatusBarVisible && bVisible && xStatusBar.is() )
        xWindow = Reference< css::awt::XWindow >( xStatusBar->getRealInterface(), UNO_QUERY );
    else if ( xProgressBar.is() && !xStatusBar.is() && bProgressBarVisible )
    {
        ProgressBarWrapper* pWrapper = (ProgressBarWrapper*)xProgressBar.get();
        if ( pWrapper )
            xWindow = pWrapper->getStatusBar();
    }
    aReadLock.unlock();

    if ( xWindow.is() )
    {
        css::awt::Rectangle aPosSize = xWindow->getPosSize();
        return ::Size( aPosSize.Width, aPosSize.Height );
    }
    else
        return ::Size();
}

css::awt::Rectangle LayoutManager::implts_calcDockingAreaSizes()
{
    Reference< css::awt::XWindow > xContainerWindow;
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor;

    ReadGuard aReadLock( m_aLock );
    xContainerWindow = m_xContainerWindow;
    xDockingAreaAcceptor = m_xDockingAreaAcceptor;
    UIElementVector aWindowVector( m_aUIElements );
    aReadLock.unlock();

    css::awt::Rectangle aBorderSpace;
    if ( xDockingAreaAcceptor.is() && xContainerWindow.is() )
    {
        sal_Int32                       nCurrRowColumn( 0 );
        sal_Int32                       nCurrPos( 0 );
        sal_Int32                       nCurrDockingArea( DockingArea_DOCKINGAREA_TOP );
        std::vector< sal_Int32 >        aRowColumnSizes[DOCKINGAREAS_COUNT];
        UIElementVector::const_iterator pConstIter;

        aRowColumnSizes[nCurrDockingArea].clear();
        aRowColumnSizes[nCurrDockingArea].push_back( 0 );

        for ( pConstIter = aWindowVector.begin(); pConstIter != aWindowVector.end(); ++pConstIter )
        {
            Reference< XUIElement > xUIElement( pConstIter->m_xUIElement, UNO_QUERY );
            if ( xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xWindow.is() && xDockWindow.is() )
                {
                    SolarMutexGuard aGuard;

                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow && pWindow->IsVisible() && !xDockWindow->isFloating() )
                    {
                        css::awt::Rectangle aPosSize = xWindow->getPosSize();
                        if ( pConstIter->m_aDockedData.m_nDockedArea != nCurrDockingArea )
                        {
                            nCurrDockingArea    = pConstIter->m_aDockedData.m_nDockedArea;
                            nCurrRowColumn      = 0;
                            nCurrPos            = 0;
                            aRowColumnSizes[nCurrDockingArea].clear();
                            aRowColumnSizes[nCurrDockingArea].push_back( 0 );
                        }

                        if ( pConstIter->m_aDockedData.m_nDockedArea == nCurrDockingArea )
                        {
                            if (( pConstIter->m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_TOP ) ||
                                ( pConstIter->m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_BOTTOM ))
                            {
                                if ( pConstIter->m_aDockedData.m_aPos.Y() > nCurrPos )
                                {
                                    ++nCurrRowColumn;
                                    nCurrPos = pConstIter->m_aDockedData.m_aPos.Y();
                                    aRowColumnSizes[nCurrDockingArea].push_back( 0 );
                                }

                                if ( aPosSize.Height > aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] )
                                    aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] = aPosSize.Height;
                            }
                            else
                            {
                                if ( pConstIter->m_aDockedData.m_aPos.X() > nCurrPos )
                                {
                                    ++nCurrRowColumn;
                                    nCurrPos = pConstIter->m_aDockedData.m_aPos.X();
                                    aRowColumnSizes[nCurrDockingArea].push_back( 0 );
                                }

                                if ( aPosSize.Width > aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] )
                                    aRowColumnSizes[nCurrDockingArea][nCurrRowColumn] = aPosSize.Width;
                            }
                        }
                    }
                }
            }
        }

        // Sum up max heights from every row/column
        if ( !aWindowVector.empty() )
        {
            for ( sal_Int32 i = 0; i <= DockingArea_DOCKINGAREA_RIGHT; i++ )
            {
                sal_Int32 nSize( 0 );
                const sal_uInt32 nCount = aRowColumnSizes[i].size();
                for ( sal_uInt32 j = 0; j < nCount; j++ )
                    nSize += aRowColumnSizes[i][j];

                if ( i == DockingArea_DOCKINGAREA_TOP )
                    aBorderSpace.Y = nSize;
                else if ( i == DockingArea_DOCKINGAREA_BOTTOM )
                    aBorderSpace.Height = nSize;
                else if ( i == DockingArea_DOCKINGAREA_LEFT )
                    aBorderSpace.X = nSize;
                else
                    aBorderSpace.Width = nSize;
            }
        }

        // We have to add the height of a possible status bar
        aBorderSpace.Height += implts_getStatusBarSize().Height();
    }

    return aBorderSpace;
}

void LayoutManager::implts_setDockingAreaWindowSizes( const css::awt::Rectangle& rBorderSpace )
{
    Reference< css::awt::XWindow > xContainerWindow;

    ReadGuard aReadLock( m_aLock );
    xContainerWindow = m_xContainerWindow;
    aReadLock.unlock();

    css::uno::Reference< css::awt::XDevice > xDevice( xContainerWindow, css::uno::UNO_QUERY );
    // Convert relativ size to output size.
    css::awt::Rectangle  aRectangle             = xContainerWindow->getPosSize();
    css::awt::DeviceInfo aInfo                  = xDevice->getInfo();
    css::awt::Size       aContainerClientSize   = css::awt::Size( aRectangle.Width - aInfo.LeftInset - aInfo.RightInset  ,
                                                                  aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );
    ::Size               aStatusBarSize         = implts_getStatusBarSize();

    sal_Int32 nLeftRightDockingAreaHeight( aContainerClientSize.Height );
    if ( rBorderSpace.Y >= 0 )
    {
        // Top docking area window
        aReadLock.lock();
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP] );
        aReadLock.unlock();

        xDockAreaWindow->setPosSize( 0, 0, aContainerClientSize.Width, rBorderSpace.Y, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
        nLeftRightDockingAreaHeight -= rBorderSpace.Y;
    }

    if ( rBorderSpace.Height >= 0 )
    {
        // Bottom docking area window
        sal_Int32 nBottomPos = std::max( sal_Int32( aContainerClientSize.Height - rBorderSpace.Height - aStatusBarSize.Height() ), sal_Int32( 0 ));
        sal_Int32 nHeight = ( nBottomPos == 0 ) ? 0 : rBorderSpace.Height;

        aReadLock.lock();
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] );
        aReadLock.unlock();

        xDockAreaWindow->setPosSize( 0, nBottomPos, aContainerClientSize.Width, nHeight, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
        nLeftRightDockingAreaHeight -= nHeight;
    }

    nLeftRightDockingAreaHeight -= aStatusBarSize.Height();
    if ( rBorderSpace.X >= 0 || nLeftRightDockingAreaHeight > 0 )
    {
        // Left docking area window
        aReadLock.lock();
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT] );
        aReadLock.unlock();

        // We also have to change our right docking area window if the top or bottom area has changed. They have a higher priority!
        sal_Int32 nHeight = std::max( sal_Int32( 0 ), sal_Int32( nLeftRightDockingAreaHeight ));

        xDockAreaWindow->setPosSize( 0, rBorderSpace.Y, rBorderSpace.X, nHeight, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
    }
    if ( rBorderSpace.Width >= 0 || nLeftRightDockingAreaHeight > 0 )
    {
        // Right docking area window
        aReadLock.lock();
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT] );
        aReadLock.unlock();

        // We also have to change our right docking area window if the top or bottom area has changed. They have a higher priority!
        sal_Int32 nLeftPos  = std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Width - rBorderSpace.Width ));
        sal_Int32 nHeight   = std::max( sal_Int32( 0 ), sal_Int32( nLeftRightDockingAreaHeight ));
        sal_Int32 nWidth    = ( nLeftPos == 0 ) ? 0 : rBorderSpace.Width;

        xDockAreaWindow->setPosSize( nLeftPos, rBorderSpace.Y, nWidth, nHeight, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
    }

    // Position the status bar
    if ( aStatusBarSize.Height() > 0 )
    {
        implts_setStatusBarPosSize( ::Point( 0, std::max(( aContainerClientSize.Height - aStatusBarSize.Height() ), long( 0 ))),
                                    ::Size( aContainerClientSize.Width, aStatusBarSize.Height() ));
    }
}

//---------------------------------------------------------------------------------------------------------
//  XMenuCloser
//---------------------------------------------------------------------------------------------------------
void LayoutManager::implts_updateMenuBarClose()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    sal_Bool bShowCloser = m_bMenuBarCloser;
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aWriteLock.unlock();

    if ( xContainerWindow.is() )
    {
        SolarMutexGuard aGuard;

        Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
            MenuBar* pMenuBar = pSysWindow->GetMenuBar();
            if ( pMenuBar )
            {
                // TODO remove link on FALSE ?!
                if ( bShowCloser )
                {
                    pMenuBar->ShowCloser( TRUE );
                    pMenuBar->SetCloserHdl( LINK( this, LayoutManager, MenuBarClose ));
                }
                else
                {
                    pMenuBar->ShowCloser( FALSE );
                    pMenuBar->SetCloserHdl( LINK( this, LayoutManager, MenuBarClose ));
                }
            }
        }
    }
}

sal_Bool LayoutManager::implts_resetMenuBar()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    sal_Bool bMenuVisible( m_bMenuVisible );
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );

    MenuBar* pSetMenuBar = 0;
    if ( m_xInplaceMenuBar.is() )
        pSetMenuBar = (MenuBar *)m_pInplaceMenuBar->GetMenuBar();
    else
    {
        MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, m_xMenuBar.get() );
        if ( pMenuBarWrapper )
            pSetMenuBar = (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar();
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    SolarMutexGuard aGuard;
    Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
    while ( pWindow && !pWindow->IsSystemWindow() )
        pWindow = pWindow->GetParent();

    if ( pWindow && bMenuVisible && pSetMenuBar )
    {
        ((SystemWindow *)pWindow)->SetMenuBar( pSetMenuBar );
        pSetMenuBar->SetDisplayable( sal_True );
        return sal_True;
    }

    return sal_False;
}

sal_Int16 LayoutManager::implts_getCurrentSymbolsSize()
{
    sal_Int16 eOptSymbolsSize( 0 );

    {
        ReadGuard aReadLock( m_aLock );
        SolarMutexGuard aGuard;
        if ( m_pMiscOptions )
            eOptSymbolsSize = m_pMiscOptions->GetCurrentSymbolsSize();
    }

    return eOptSymbolsSize;
}

sal_Int16 LayoutManager::implts_getCurrentSymbolsStyle()
{
    sal_Int16 eOptSymbolsStyle( 0 );

    {
        ReadGuard aReadLock( m_aLock );
        SolarMutexGuard aGuard;
        if ( m_pMiscOptions )
            eOptSymbolsStyle = m_pMiscOptions->GetCurrentSymbolsStyle();
    }

    return eOptSymbolsStyle;
}

IMPL_LINK( LayoutManager, MenuBarClose, MenuBar *, EMPTYARG )
{
    ReadGuard aReadLock( m_aLock );
    css::uno::Reference< css::frame::XDispatchProvider >   xProvider(m_xFrame, css::uno::UNO_QUERY);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR    = m_xSMGR;
    aReadLock.unlock();

    if (! xProvider.is())
        return 0;

    css::uno::Reference< css::frame::XDispatchHelper > xDispatcher(
        xSMGR->createInstance(SERVICENAME_DISPATCHHELPER), css::uno::UNO_QUERY_THROW);

    xDispatcher->executeDispatch(
        xProvider,
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:CloseWin")),
        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_self")),
        0,
        css::uno::Sequence< css::beans::PropertyValue >());

    return 0;
}

IMPL_LINK( LayoutManager, OptionsChanged, void*, EMPTYARG )
{
    sal_Int16 eSymbolsSize( implts_getCurrentSymbolsSize() );
    sal_Int16 eSymbolsStyle( implts_getCurrentSymbolsStyle() );

    ReadGuard aReadLock( m_aLock );
    sal_Int16 eOldSymbolsSize = m_eSymbolsSize;
    sal_Int16 eOldSymbolsStyle = m_eSymbolsStyle;
    aReadLock.unlock();

    if ( eSymbolsSize != eOldSymbolsSize || eSymbolsStyle != eOldSymbolsStyle )
    {
        WriteGuard aWriteLock( m_aLock );
        m_eSymbolsSize = eSymbolsSize;
        m_eSymbolsStyle = eSymbolsStyle;
        aWriteLock.unlock();

        std::vector< Reference< XUpdatable > > aToolBarVector;

        aReadLock.lock();
        {
            UIElementVector::iterator pIter;
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
            {
                if ( pIter->m_xUIElement.is() )
                    aToolBarVector.push_back( Reference< XUpdatable >( pIter->m_xUIElement, UNO_QUERY ));
            }
        }
        aReadLock.unlock();

        lock();
        {
            std::vector< Reference< XUpdatable > >::iterator pIter;
            for ( pIter = aToolBarVector.begin(); pIter != aToolBarVector.end(); ++pIter )
            {
                if ( (*pIter).is() )
                    (*pIter)->update();
            }
        }
        unlock();
        doLayout();
    }

    return 1;
}

IMPL_LINK( LayoutManager, SettingsChanged, void*, EMPTYARG )
{
    return 1;
}

//---------------------------------------------------------------------------------------------------------
//  XDockableWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::startDocking( const ::com::sun::star::awt::DockingEvent& e ) throw (::com::sun::star::uno::RuntimeException)
{
    sal_Bool    bWinFound( sal_False );
    UIElement   aUIElement;

    ReadGuard aReadGuard( m_aLock );
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    Reference< css::awt::XWindow2 > xWindow( e.Source, UNO_QUERY );
    aReadGuard.unlock();

    Window* pContainerWindow( 0 );
    Window* pWindow( 0 );
    ::Point aMousePos;
    {
        SolarMutexGuard aGuard;
        pContainerWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        aMousePos = pContainerWindow->ScreenToOutputPixel( ::Point( e.MousePos.X, e.MousePos.Y ));
    }

    bWinFound = implts_findElement( e.Source, aUIElement );

    if ( bWinFound && xWindow.is() )
    {
        css::awt::Rectangle aRect;
        Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
        if ( xDockWindow->isFloating() )
        {
            css::awt::Rectangle aPos  = xWindow->getPosSize();
            css::awt::Size      aSize = xWindow->getOutputSize();

            aUIElement.m_aFloatingData.m_aPos    = ::Point( aPos.X, aPos.Y );
            aUIElement.m_aFloatingData.m_aSize   = ::Size( aSize.Width, aSize.Height );

            SolarMutexGuard aGuard;
            pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            {
                ToolBox* pToolBox = (ToolBox *)pWindow;
                aUIElement.m_aFloatingData.m_nLines          = pToolBox->GetFloatingLines();
                aUIElement.m_aFloatingData.m_bIsHorizontal   = (( pToolBox->GetAlign() == WINDOWALIGN_TOP ) ||
                                                                ( pToolBox->GetAlign() == WINDOWALIGN_BOTTOM ));
            }
        }
    }

    WriteGuard aWriteLock( m_aLock );
    m_bDockingInProgress = bWinFound;
    m_aDockUIElement = aUIElement;
    m_aDockUIElement.m_bUserActive = sal_True;
    m_aStartDockMousePos = aMousePos;
    aWriteLock.unlock();
}

::Rectangle LayoutManager::implts_calcHotZoneRect( const ::Rectangle& rRect, sal_Int32 nHotZoneOffset )
{
    ::Rectangle aRect( rRect );

    aRect.Left() -= nHotZoneOffset;
    aRect.Top() -= nHotZoneOffset;
    aRect.Right() += nHotZoneOffset;
    aRect.Bottom() += nHotZoneOffset;
    return aRect;
}

css::awt::DockingData SAL_CALL LayoutManager::docking( const ::com::sun::star::awt::DockingEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    const sal_Int32 MAGNETIC_DISTANCE_UNDOCK = 25;
    const sal_Int32 MAGNETIC_DISTANCE_DOCK   = 20;

    css::awt::DockingData                  aDockingData;
    Reference< css::awt::XDockableWindow > xDockWindow( e.Source, UNO_QUERY );
    Reference< css::awt::XWindow >         xWindow( e.Source, UNO_QUERY );
    Reference< css::awt::XWindow >         xTopDockingWindow;
    Reference< css::awt::XWindow >         xLeftDockingWindow;
    Reference< css::awt::XWindow >         xRightDockingWindow;
    Reference< css::awt::XWindow >         xBottomDockingWindow;
    Reference< css::awt::XWindow >         xContainerWindow;
    UIElement                              aUIDockingElement;
    DockingOperation                       eDockingOperation( DOCKOP_ON_COLROW );
    ::Size                                 aStatusBarSize;

    aDockingData.TrackingRectangle = e.TrackingRectangle;
    sal_Bool bDockingInProgress;

    {
        ReadGuard aReadLock( m_aLock );
        bDockingInProgress = m_bDockingInProgress;
        if ( bDockingInProgress )
        {
            xContainerWindow        = m_xContainerWindow;
            xTopDockingWindow       = m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP];
            xLeftDockingWindow      = m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT];
            xRightDockingWindow     = m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT];
            xBottomDockingWindow    = m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM];
            aUIDockingElement       = m_aDockUIElement;
            aStatusBarSize          = implts_getStatusBarSize();
        }
    }

    if ( bDockingInProgress &&
         xDockWindow.is() &&
         xWindow.is() )
    {
        try
        {
            SolarMutexGuard aGuard;

            sal_Int16 eDockingArea( -1 ); // none
            sal_Int32 nMagneticZone( aUIDockingElement.m_bFloating ? MAGNETIC_DISTANCE_DOCK : MAGNETIC_DISTANCE_UNDOCK );
            css::awt::Rectangle aNewTrackingRect;
            ::Rectangle aTrackingRect( e.TrackingRectangle.X,
                                       e.TrackingRectangle.Y,
                                       ( e.TrackingRectangle.X + e.TrackingRectangle.Width ),
                                       ( e.TrackingRectangle.Y + e.TrackingRectangle.Height ));

            css::awt::Rectangle aTmpRect = xTopDockingWindow->getPosSize();
            ::Rectangle aTopDockRect( aTmpRect.X, aTmpRect.Y, aTmpRect.Width, aTmpRect.Height );
            ::Rectangle aHotZoneTopDockRect( implts_calcHotZoneRect( aTopDockRect, nMagneticZone ));

            aTmpRect = xBottomDockingWindow->getPosSize();
            ::Rectangle aBottomDockRect( aTmpRect.X, aTmpRect.Y, ( aTmpRect.X + aTmpRect.Width), ( aTmpRect.Y + aTmpRect.Height ));
            ::Rectangle aHotZoneBottomDockRect( implts_calcHotZoneRect( aBottomDockRect, nMagneticZone ));

            aTmpRect = xLeftDockingWindow->getPosSize();
            ::Rectangle aLeftDockRect( aTmpRect.X, aTmpRect.Y, ( aTmpRect.X + aTmpRect.Width ), ( aTmpRect.Y + aTmpRect.Height ));
            ::Rectangle aHotZoneLeftDockRect( implts_calcHotZoneRect( aLeftDockRect, nMagneticZone ));

            aTmpRect = xRightDockingWindow->getPosSize();
            ::Rectangle aRightDockRect( aTmpRect.X, aTmpRect.Y, ( aTmpRect.X + aTmpRect.Width ), ( aTmpRect.Y + aTmpRect.Height ));
            ::Rectangle aHotZoneRightDockRect( implts_calcHotZoneRect( aRightDockRect, nMagneticZone ));

            Window* pContainerWindow( VCLUnoHelper::GetWindow( xContainerWindow ) );
            ::Point aMousePos( pContainerWindow->ScreenToOutputPixel( ::Point( e.MousePos.X, e.MousePos.Y )));

            if ( aHotZoneTopDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_TOP;
            else if ( aHotZoneBottomDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_BOTTOM;
            else if ( aHotZoneLeftDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_LEFT;
            else if ( aHotZoneRightDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_RIGHT;

            // Higher priority for movements inside the real docking area
            if ( aTopDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_TOP;
            else if ( aBottomDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_BOTTOM;
            else if ( aLeftDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_LEFT;
            else if ( aRightDockRect.IsInside( aMousePos ))
                eDockingArea = DockingArea_DOCKINGAREA_RIGHT;

            // Determine if we have a toolbar and set alignment according to the docking area!
            Window*     pWindow = VCLUnoHelper::GetWindow( xWindow );
            ToolBox*    pToolBox = 0;
            if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                pToolBox = (ToolBox *)pWindow;

            if ( eDockingArea != -1 )
            {
                if ( eDockingArea == DockingArea_DOCKINGAREA_TOP )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_TOP;
                    aUIDockingElement.m_bFloating = sal_False;
                    VCLUnoHelper::GetWindow( xTopDockingWindow );
                }
                else if ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_BOTTOM;
                    aUIDockingElement.m_bFloating = sal_False;
                    VCLUnoHelper::GetWindow( xBottomDockingWindow );
                }
                else if ( eDockingArea == DockingArea_DOCKINGAREA_LEFT )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_LEFT;
                    aUIDockingElement.m_bFloating = sal_False;
                    VCLUnoHelper::GetWindow( xLeftDockingWindow );
                }
                else if ( eDockingArea == DockingArea_DOCKINGAREA_RIGHT )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_RIGHT;
                    aUIDockingElement.m_bFloating = sal_False;
                    VCLUnoHelper::GetWindow( xRightDockingWindow );
                }

                ::Point aOutputPos = pContainerWindow->ScreenToOutputPixel( aTrackingRect.TopLeft() );
                aTrackingRect.SetPos( aOutputPos );

                ::Rectangle         aNewDockingRect( aTrackingRect );
                implts_calcDockingPosSize( aUIDockingElement, eDockingOperation, aNewDockingRect, aMousePos );

                ::Point aScreenPos = pContainerWindow->OutputToScreenPixel( aNewDockingRect.TopLeft() );
                aNewTrackingRect = css::awt::Rectangle( aScreenPos.X(),
                                                        aScreenPos.Y(),
                                                        aNewDockingRect.getWidth(),
                                                        aNewDockingRect.getHeight() );
                aDockingData.TrackingRectangle = aNewTrackingRect;
            }
            else if ( pToolBox && bDockingInProgress )
            {
                sal_Bool bIsHorizontal = (( pToolBox->GetAlign() == WINDOWALIGN_TOP    ) ||
                                          ( pToolBox->GetAlign() == WINDOWALIGN_BOTTOM ));
                ::Size aFloatSize = aUIDockingElement.m_aFloatingData.m_aSize;
                if ( aFloatSize.Width() > 0 && aFloatSize.Height() > 0 )
                {
                    aUIDockingElement.m_aFloatingData.m_aPos = pContainerWindow->ScreenToOutputPixel(
                                                                ::Point( e.MousePos.X, e.MousePos.Y ));
                    aDockingData.TrackingRectangle.Height = aFloatSize.Height();
                    aDockingData.TrackingRectangle.Width  = aFloatSize.Width();
                }
                else
                {
                    aFloatSize = pToolBox->CalcWindowSizePixel();
                    if ( !bIsHorizontal )
                    {
                        // Floating toolbars are always horizontal aligned! We have to swap
                        // width/height if we have a vertical aligned toolbar.
                        sal_Int32 nTemp = aFloatSize.Height();
                        aFloatSize.Height() = aFloatSize.Width();
                        aFloatSize.Width()  = nTemp;
                    }

                    aDockingData.TrackingRectangle.Height = aFloatSize.Height();
                    aDockingData.TrackingRectangle.Width  = aFloatSize.Width();

                    // For the first time we don't have any data about the floating size of a toolbar.
                    // We calculate it and store it for later use.
                    aUIDockingElement.m_aFloatingData.m_aPos = pContainerWindow->ScreenToOutputPixel(
                                                                ::Point( e.MousePos.X, e.MousePos.Y ));
                    aUIDockingElement.m_aFloatingData.m_aSize           = aFloatSize;
                    aUIDockingElement.m_aFloatingData.m_nLines          = pToolBox->GetFloatingLines();
                    aUIDockingElement.m_aFloatingData.m_bIsHorizontal   = (( pToolBox->GetAlign() == WINDOWALIGN_TOP ) ||
                                                                           ( pToolBox->GetAlign() == WINDOWALIGN_BOTTOM ));
                }
                aDockingData.TrackingRectangle.X = e.MousePos.X;
                aDockingData.TrackingRectangle.Y = e.MousePos.Y;
            }

            aDockingData.bFloating = ( eDockingArea == -1 );

            // Write current data to the member docking progress data
            WriteGuard aWriteLock( m_aLock );
            m_aDockUIElement.m_bFloating = aDockingData.bFloating;
            if ( !aDockingData.bFloating )
            {
                m_aDockUIElement.m_aDockedData   = aUIDockingElement.m_aDockedData;
                m_eDockOperation                 = eDockingOperation;
            }
            else
                m_aDockUIElement.m_aFloatingData = aUIDockingElement.m_aFloatingData;
            aWriteLock.unlock();
        }
        catch ( Exception& )
        {
        }
    }

    return aDockingData;
}

void SAL_CALL LayoutManager::endDocking( const ::com::sun::star::awt::EndDockingEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    sal_Bool    bDockingInProgress( sal_False );
    sal_Bool    bStartDockFloated( sal_False );
    sal_Bool    bFloating( sal_False );
    UIElement   aUIDockingElement;

    WriteGuard aWriteLock( m_aLock );
    bDockingInProgress = m_bDockingInProgress;
    aUIDockingElement = m_aDockUIElement;
    bFloating = aUIDockingElement.m_bFloating;

    UIElement& rUIElement = impl_findElement( aUIDockingElement.m_aName );
    if ( rUIElement.m_aName == aUIDockingElement.m_aName )
    {
        if ( aUIDockingElement.m_bFloating )
        {
            // Write last position into position data
            Reference< css::awt::XWindow > xWindow( aUIDockingElement.m_xUIElement->getRealInterface(), UNO_QUERY );
            rUIElement.m_aFloatingData = aUIDockingElement.m_aFloatingData;
            css::awt::Rectangle aTmpRect = xWindow->getPosSize();
            rUIElement.m_aFloatingData.m_aPos = ::Point( aTmpRect.X, aTmpRect.Y );
            // make changes also for our local data as we use it to make data persistent
            aUIDockingElement.m_aFloatingData = rUIElement.m_aFloatingData;
        }
        else
        {
            rUIElement.m_aDockedData = aUIDockingElement.m_aDockedData;
            rUIElement.m_aFloatingData.m_aSize = aUIDockingElement.m_aFloatingData.m_aSize;

            if ( m_eDockOperation != DOCKOP_ON_COLROW )
            {
                // we have to renumber our row/column data to insert a new row/column
                implts_renumberRowColumnData( (::com::sun::star::ui::DockingArea)aUIDockingElement.m_aDockedData.m_nDockedArea,
                                              m_eDockOperation,
                                              aUIDockingElement );
            }
        }

        bStartDockFloated = rUIElement.m_bFloating;
        rUIElement.m_bFloating = m_aDockUIElement.m_bFloating;
        rUIElement.m_bUserActive = sal_True;
    }

    // reset member for next docking operation
    m_aDockUIElement.m_xUIElement.clear();
    m_eDockOperation = DOCKOP_ON_COLROW;
    aWriteLock.unlock();

    implts_writeWindowStateData( aUIDockingElement.m_aName, aUIDockingElement );

    if ( bDockingInProgress )
    {
        SolarMutexGuard aGuard;
        Window*  pWindow = VCLUnoHelper::GetWindow( Reference< css::awt::XWindow >( e.Source, UNO_QUERY ));
        ToolBox* pToolBox = 0;
        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pWindow;

        if ( pToolBox )
        {
            if( e.bFloating )
            {
                if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                    pToolBox->SetAlign( WINDOWALIGN_TOP );
                else
                    pToolBox->SetAlign( WINDOWALIGN_LEFT );
            }
            else
            {
                ::Size aSize;

                pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );

                // Docked toolbars have always one line
                aSize = pToolBox->CalcWindowSizePixel( 1 );

                // Lock layouting updates as our listener would be called due to SetSizePixel
                pToolBox->SetOutputSizePixel( aSize );
            }
        }
    }

    aWriteLock.lock();
    m_bDockingInProgress = sal_False;
    aWriteLock.unlock();

    implts_sortUIElements();
    if ( !bStartDockFloated || !bFloating )
    {
        // Optimization: Don't layout if we started floating and now floating again.
        // This would not change anything for the docked user-interface elements.
        doLayout();
    }
}

sal_Bool SAL_CALL LayoutManager::prepareToggleFloatingMode( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    sal_Bool    bDockingInProgress( sal_False );

    ReadGuard aReadLock( m_aLock );
    bDockingInProgress = m_bDockingInProgress;
    aReadLock.unlock();

    UIElement                         aUIDockingElement;
    sal_Bool                          bWinFound( implts_findElement( e.Source, aUIDockingElement ) );
    Reference< css::awt::XWindow >    xWindow( e.Source, UNO_QUERY );

    if ( bWinFound && xWindow.is() )
    {
        if ( !bDockingInProgress )
        {
            css::awt::Rectangle aRect;
            Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
            if ( xDockWindow->isFloating() )
            {
                {
                    SolarMutexGuard aGuard;
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow )
                    {
                        if ( pWindow->GetType() == WINDOW_TOOLBOX )
                        {
                            ToolBox* pToolBox = (ToolBox *)pWindow;
                            aUIDockingElement.m_aFloatingData.m_aPos            = pToolBox->GetPosPixel();
                            aUIDockingElement.m_aFloatingData.m_aSize           = pToolBox->GetOutputSizePixel();
                            aUIDockingElement.m_aFloatingData.m_nLines          = pToolBox->GetFloatingLines();
                            aUIDockingElement.m_aFloatingData.m_bIsHorizontal   = (( pToolBox->GetAlign() == WINDOWALIGN_TOP ) ||
                                                                                   ( pToolBox->GetAlign() == WINDOWALIGN_BOTTOM ));
                        }
                    }
                }

                WriteGuard aWriteLock( m_aLock );
                UIElement& rUIElement = LayoutManager::impl_findElement( aUIDockingElement.m_aName );
                if ( rUIElement.m_aName == aUIDockingElement.m_aName )
                    rUIElement = aUIDockingElement;
                aWriteLock.unlock();
            }
        }
    }

    return sal_True;
}

void SAL_CALL LayoutManager::toggleFloatingMode( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    sal_Bool  bDockingInProgress( sal_False );
    UIElement aUIDockingElement;

    ReadGuard aReadLock( m_aLock );
    bDockingInProgress = m_bDockingInProgress;
    if ( bDockingInProgress )
        aUIDockingElement = m_aDockUIElement;
    aReadLock.unlock();

    Window*  pWindow( 0 );
    ToolBox* pToolBox( 0 );
    Reference< css::awt::XWindow2 > xWindow;

    {
        SolarMutexGuard aGuard;
        xWindow = Reference< css::awt::XWindow2 >( e.Source, UNO_QUERY );
        pWindow = VCLUnoHelper::GetWindow( xWindow );

        if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pWindow;
    }

    if ( !bDockingInProgress )
    {
        sal_Bool bWinFound( implts_findElement( e.Source, aUIDockingElement ) );
        if ( bWinFound && xWindow.is() )
        {
            aUIDockingElement.m_bFloating   = !aUIDockingElement.m_bFloating;
            aUIDockingElement.m_bUserActive = sal_True;

            WriteGuard aWriteLock( m_aLock );
            m_bDoLayout = sal_True;
            aWriteLock.unlock();

            if ( aUIDockingElement.m_bFloating )
            {
                SolarMutexGuard aGuard;
                if ( pToolBox )
                {
                    pToolBox->SetLineCount( aUIDockingElement.m_aFloatingData.m_nLines );
                    if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                        pToolBox->SetAlign( WINDOWALIGN_TOP );
                    else
                        pToolBox->SetAlign( WINDOWALIGN_LEFT );
                }

                sal_Bool bUndefPos = ( aUIDockingElement.m_aFloatingData.m_aPos.X() == SAL_MAX_INT32 ||
                                       aUIDockingElement.m_aFloatingData.m_aPos.Y() == SAL_MAX_INT32 );
                sal_Bool bSetSize = ( aUIDockingElement.m_aFloatingData.m_aSize.Width() != 0 &&
                                      aUIDockingElement.m_aFloatingData.m_aSize.Height() != 0 );

                if ( bUndefPos )
                    aUIDockingElement.m_aFloatingData.m_aPos = implts_findNextCascadeFloatingPos();

                if ( !bSetSize )
                {
                    if ( pToolBox )
                        aUIDockingElement.m_aFloatingData.m_aSize = pToolBox->CalcFloatingWindowSizePixel();
                    else
                        aUIDockingElement.m_aFloatingData.m_aSize = pWindow->GetOutputSizePixel();
                }

                xWindow->setPosSize( aUIDockingElement.m_aFloatingData.m_aPos.X(),
                                     aUIDockingElement.m_aFloatingData.m_aPos.Y(),
                                     0, 0, css::awt::PosSize::POS );
                xWindow->setOutputSize( AWTSize( aUIDockingElement.m_aFloatingData.m_aSize ) );
            }
            else
            {
                if (( aUIDockingElement.m_aDockedData.m_aPos.X() == SAL_MAX_INT32 ) &&
                    ( aUIDockingElement.m_aDockedData.m_aPos.Y() == SAL_MAX_INT32 ))
                {
                    // Docking on its default position without a preset position -
                    // we have to find a good place for it.
                    ::Point aPixelPos;
                    ::Point aDockPos;
                    ::Size  aSize;

                    {
                        SolarMutexGuard aGuard;
                        if ( pToolBox )
                            aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea ) );
                        else
                            aSize = pWindow->GetSizePixel();
                    }

                    implts_findNextDockingPos( (::com::sun::star::ui::DockingArea)aUIDockingElement.m_aDockedData.m_nDockedArea,
                                                aSize,
                                                aDockPos,
                                                aPixelPos );
                    aUIDockingElement.m_aDockedData.m_aPos = aDockPos;
                }

                SolarMutexGuard aGuard;
                if ( pToolBox )
                {
                    pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );
                    ::Size aSize = pToolBox->CalcWindowSizePixel( 1 );
                    css::awt::Rectangle aRect = xWindow->getPosSize();
                    xWindow->setPosSize( aRect.X, aRect.Y, 0, 0, css::awt::PosSize::POS );
                    xWindow->setOutputSize( AWTSize( aSize ) );
                }
            }

            aWriteLock.lock();
            m_bDoLayout = sal_False;
            UIElement& rUIElement = LayoutManager::impl_findElement( aUIDockingElement.m_aName );
            if ( rUIElement.m_aName == aUIDockingElement.m_aName )
                rUIElement = aUIDockingElement;
            aWriteLock.unlock();

            implts_writeWindowStateData( aUIDockingElement.m_aName, aUIDockingElement );

            implts_sortUIElements();
            doLayout();
        }
    }
    else
    {
        SolarMutexGuard aGuard;
        if ( pToolBox )
        {
            if ( aUIDockingElement.m_bFloating )
            {
                if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                    pToolBox->SetAlign( WINDOWALIGN_TOP );
                else
                    pToolBox->SetAlign( WINDOWALIGN_LEFT );
            }
            else
            {
                pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );
            }
        }
    }
}

void SAL_CALL LayoutManager::closed( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    rtl::OUString aName;
    UIElement     aUIElement;
    UIElementVector::iterator pIter;

    WriteGuard aWriteLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
    {
        Reference< XUIElement > xUIElement( pIter->m_xUIElement );
        if ( xUIElement.is() )
        {
            Reference< XInterface > xIfac( xUIElement->getRealInterface(), UNO_QUERY );
            if ( xIfac == e.Source )
            {
                aName = pIter->m_aName;

                // user closes a toolbar =>
                // context sensitive toolbar: only destroy toolbar and store state.
                // context sensitive toolbar: make it invisible, store state and destroy it.
                if ( !pIter->m_bContextSensitive )
                    pIter->m_bVisible = sal_False;

                aUIElement = *pIter;
                break;
            }
        }
    }
    aWriteLock.unlock();


    // destroy element
    if ( aName.getLength() > 0 )
    {
        implts_writeWindowStateData( aName, aUIElement );
        destroyElement( aName );
    }
}

void SAL_CALL LayoutManager::endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& )
throw (::com::sun::star::uno::RuntimeException)
{

}

//---------------------------------------------------------------------------------------------------------
//  XLayoutManagerEventBroadcaster
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::addLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.addInterface( ::getCppuType( (const css::uno::Reference< css::frame::XLayoutManagerListener >*)NULL ), xListener );
}

void SAL_CALL LayoutManager::removeLayoutManagerEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManagerListener >& xListener )
throw (::com::sun::star::uno::RuntimeException)
{
    m_aListenerContainer.removeInterface( ::getCppuType( (const css::uno::Reference< css::frame::XLayoutManagerListener >*)NULL ), xListener );
}

void LayoutManager::implts_notifyListeners( short nEvent, ::com::sun::star::uno::Any aInfoParam )
{
    css::lang::EventObject             aSource    (static_cast< ::cppu::OWeakObject*>(this));
    ::cppu::OInterfaceContainerHelper* pContainer = m_aListenerContainer.getContainer( ::getCppuType( ( const css::uno::Reference< css::frame::XLayoutManagerListener >*) NULL ) );
    if (pContainer!=NULL)
    {
        ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
        while (pIterator.hasMoreElements())
        {
            try
            {
                ((css::frame::XLayoutManagerListener*)pIterator.next())->layoutEvent( aSource, nEvent, aInfoParam );
            }
            catch( css::uno::RuntimeException& )
            {
                pIterator.remove();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------
//  XWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::windowResized( const css::awt::WindowEvent& aEvent )
throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( !m_xDockingAreaAcceptor.is() )
        return;

    // Request to set docking area space again.
    css::awt::Rectangle               aDockingArea( m_aDockingArea );
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    Reference< css::awt::XWindow >    xContainerWindow( m_xContainerWindow );

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
        Reference< css::awt::XWindow > xComponentWindow( m_xFrame->getComponentWindow() );
        if( xComponentWindow.is() == sal_True )
        {
            css::uno::Reference< css::awt::XDevice > xDevice( m_xFrame->getContainerWindow(), css::uno::UNO_QUERY );

            // Convert relativ size to output size.
            css::awt::Rectangle  aRectangle  = m_xFrame->getContainerWindow()->getPosSize();
            css::awt::DeviceInfo aInfo       = xDevice->getInfo();
            css::awt::Size       aSize       (  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

            // Resize our component window.
            xComponentWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );
        }
    }
    else
    {
        // resize event for one of the UIElements
        sal_Bool bLocked( m_bDockingInProgress );
        sal_Bool bDoLayout( m_bDoLayout );
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        if ( !bLocked && !bDoLayout )
        {
            // Do not do anything if we are in the middle of a docking process. This would interfere all other
            // operations. We will store the new position and size in the docking handlers.
            // Do not do anything if we are in the middle of our layouting process. We will adapt the position
            // and size of the user interface elements.
            UIElement aUIElement;
            if ( implts_findElement( aEvent.Source, aUIElement ))
            {
                if ( aUIElement.m_bFloating )
                    implts_writeNewStateData( aUIElement.m_aName,
                                            Reference< css::awt::XWindow >( aEvent.Source, UNO_QUERY ));
                else
                    doLayout();
            }
        }
    }
}

void SAL_CALL LayoutManager::windowMoved( const css::awt::WindowEvent& ) throw( css::uno::RuntimeException )
{
}

void SAL_CALL LayoutManager::windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Request to set docking area space again.
    Reference< css::awt::XWindow >  xContainerWindow( m_xContainerWindow );
    bool                        bParentWindowVisible( m_bParentWindowVisible );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        bool bSetVisible = false;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_bParentWindowVisible = true;
        bSetVisible = ( m_bParentWindowVisible != bParentWindowVisible );
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        if ( bSetVisible )
        {
            implts_updateUIElementsVisibleState( sal_True );
            //implts_doLayout( sal_False );
        }
    }
}

void SAL_CALL LayoutManager::windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Request to set docking area space again.
    Reference< css::awt::XWindow >  xContainerWindow( m_xContainerWindow );
    bool                        bParentWindowVisible( m_bParentWindowVisible );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        bool bSetInvisible = false;

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_bParentWindowVisible = false;
        bSetInvisible = ( m_bParentWindowVisible != bParentWindowVisible );
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        if ( bSetInvisible )
            implts_updateUIElementsVisibleState( sal_False );
    }
}

IMPL_LINK( LayoutManager, AsyncLayoutHdl, Timer *, EMPTYARG )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Request to set docking area space again.
    m_aAsyncLayoutTimer.Stop();

    if( !m_xContainerWindow.is() )
        return 0;

    css::awt::Rectangle aDockingArea( m_aDockingArea );
    ::Size              aStatusBarSize( implts_getStatusBarSize() );

    // Subtract status bar height
    aDockingArea.Height -= aStatusBarSize.Height();
    aReadLock.unlock();

    implts_setDockingAreaWindowSizes( aDockingArea );
    implts_doLayout( sal_True, sal_False );

    return 0;
}

#ifdef DBG_UTIL
void LayoutManager::implts_checkElementContainer()
{
#ifdef DBG_UTIL
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    BaseHash< sal_Int32 > aUIElementHash;

    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); ++pIter )
        aUIElementHash[pIter->m_aName]++;


    BaseHash< sal_Int32 >::const_iterator pCheckIter = aUIElementHash.begin();
    for ( ; pCheckIter != aUIElementHash.end(); ++pCheckIter )
    {
        if ( pCheckIter->second > 1 )
        {
            ::rtl::OString aName = ::rtl::OUStringToOString( pCheckIter->first, RTL_TEXTENCODING_ASCII_US );
            DBG_ASSERT( "More than one element (%s) with the same name found!", aName.getStr() );
        }
    } // for ( ; pCheckIter != aUIElementHash.end(); pCheckIter++ )
#endif
}
#endif

//---------------------------------------------------------------------------------------------------------
//  XFrameActionListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::frameAction( const FrameActionEvent& aEvent )
throw ( RuntimeException )
{
    if (( aEvent.Action == FrameAction_COMPONENT_ATTACHED   ) ||
        ( aEvent.Action == FrameAction_COMPONENT_REATTACHED ))
    {
        RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::frameAction (COMPONENT_ATTACHED|REATTACHED)" );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_bComponentAttached = sal_True;
        m_bMustDoLayout = sal_True;
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_reset( sal_True );
        implts_doLayout( sal_True, sal_False );
        implts_doLayout( sal_True, sal_True );
    }
    else if (( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED      ) ||
             ( aEvent.Action == FrameAction_FRAME_UI_DEACTIVATING   ))
    {
        RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::frameAction (FRAME_UI_ACTIVATED|DEACTIVATING)" );

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_bActive = ( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_toggleFloatingUIElementsVisibility( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
//        doLayout();
    }
    else if ( aEvent.Action == FrameAction_COMPONENT_DETACHING )
    {
        RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::LayoutManager::frameAction (COMPONENT_DETACHING)" );

        // SAFE AREA -----------------------------------------------------------------------------------------------
        WriteGuard aWriteLock( m_aLock );
        m_bComponentAttached = sal_False;
        aWriteLock.unlock();
        // SAFE AREA -----------------------------------------------------------------------------------------------

        implts_reset( sal_False );
    }
}

// ______________________________________________

void SAL_CALL LayoutManager::disposing( const css::lang::EventObject& rEvent )
throw( RuntimeException )
{
    sal_Bool bDisposeAndClear( sal_False );

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( rEvent.Source == Reference< XInterface >( m_xFrame, UNO_QUERY ))
    {
        // Our frame gets disposed, release all our references that depends on a working
        // frame reference.
        Application::RemoveEventListener( LINK( this, LayoutManager, SettingsChanged ) );
        if ( m_pMiscOptions )
        {
            m_pMiscOptions->RemoveListenerLink( LINK( this, LayoutManager, OptionsChanged ) );
            delete m_pMiscOptions;
            m_pMiscOptions = 0;
        }

        delete m_pAddonOptions;
        m_pAddonOptions = 0;

        // destroy all elements, it's possible that dettaching is NOT called!
        implts_destroyElements();

        m_aUIElements.clear();
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
        implts_destroyDockingAreaWindows();

        if ( m_xModuleCfgMgr.is() )
        {
            try
            {
                Reference< XUIConfiguration > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
                xModuleCfgMgr->removeConfigurationListener(
                    Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
            }
            catch ( Exception& )
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
            catch ( Exception& )
            {
            }
        }

        m_xDocCfgMgr.clear();
        m_xModuleCfgMgr.clear();
        m_xFrame.clear();
        delete m_pGlobalSettings;
        m_pGlobalSettings = 0;
        m_xDockingAreaAcceptor = Reference< ::com::sun::star::ui::XDockingAreaAcceptor >();

        bDisposeAndClear = sal_True;
    }
    else if ( rEvent.Source == Reference< XInterface >( m_xContainerWindow, UNO_QUERY ))
    {
        // Our container window gets disposed. Remove all user interface elements.
        m_aUIElements.clear();
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
    {
        m_xDocCfgMgr.clear();
    }
    else if ( rEvent.Source == Reference< XInterface >( m_xModuleCfgMgr , UNO_QUERY ))
    {
        m_xModuleCfgMgr.clear();
    }

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    // Send disposing to our listener when we have lost our frame.
    if ( bDisposeAndClear )
    {
        // Send message to all listener and forget her references.
        css::uno::Reference< css::frame::XLayoutManager > xThis( static_cast< ::cppu::OWeakObject* >(this), css::uno::UNO_QUERY );
        css::lang::EventObject aEvent( xThis );
        m_aListenerContainer.disposeAndClear( aEvent );
    }
}

void SAL_CALL LayoutManager::elementInserted( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );

    ::rtl::OUString                aElementType;
    ::rtl::OUString                aElementName;
    Reference< XUIElement > xElement;
    Reference< XFrame >     xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        implts_findElement( Event.ResourceURL, aElementType, aElementName, xElement );

        Reference< XUIElementSettings > xElementSettings( xElement, UNO_QUERY );
        if ( xElementSettings.is() )
        {
            ::rtl::OUString aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
            Reference< XPropertySet > xPropSet( xElementSettings, UNO_QUERY );
            if ( xPropSet.is() )
            {
                if ( Event.Source == Reference< XInterface >( m_xDocCfgMgr, UNO_QUERY ))
                    xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( m_xDocCfgMgr ));
            }
            xElementSettings->updateSettings();
        }
        else
        {
            if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ) &&
                 ( aElementName.indexOf( m_aCustomTbxPrefix ) != -1 ))
            {
                // custom toolbar must be directly created, shown and layouted!
                createElement( Event.ResourceURL );
                Reference< XUIElement > xUIElement = getElement( Event.ResourceURL );
                if ( xUIElement.is() )
                {
                    Reference< XUIConfigurationManager > xCfgMgr;
                    Reference< XPropertySet >            xPropSet;
                    ::rtl::OUString                             aUIName;

                    try
                    {
                        xCfgMgr  = Reference< XUIConfigurationManager >( Event.Source, UNO_QUERY );
                        xPropSet = Reference< XPropertySet >( xCfgMgr->getSettings( Event.ResourceURL, sal_False ), UNO_QUERY );

                        if ( xPropSet.is() )
                            xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= aUIName;
                    }
                    catch ( com::sun::star::container::NoSuchElementException& )
                    {
                    }
                    catch ( com::sun::star::beans::UnknownPropertyException& )
                    {
                    }
                    catch ( com::sun::star::lang::WrappedTargetException& )
                    {
                    }

                    {
                        SolarMutexGuard aGuard;
                        Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                        if ( pWindow  )
                            pWindow->SetText( aUIName );
                    }

                    showElement( Event.ResourceURL );
                }
            }
        }
    }
}

void SAL_CALL LayoutManager::elementRemoved( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    ::rtl::OUString                aElementType;
    ::rtl::OUString                aElementName;
    Reference< XUIElement > xUIElement;
    Reference< XFrame >     xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        implts_findElement( Event.ResourceURL, aElementType, aElementName, xUIElement );

        Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
        if ( xElementSettings.is() )
        {
            bool                        bNoSettings( false );
            ::rtl::OUString                    aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
            Reference< XInterface >     xElementCfgMgr;
            Reference< XPropertySet >   xPropSet( xElementSettings, UNO_QUERY );

            if ( xPropSet.is() )
                xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

            if ( !xElementCfgMgr.is() )
                return;

            // Check if the same UI configuration manager has changed => check further
            if ( Event.Source == xElementCfgMgr )
            {
                // Same UI configuration manager where our element has its settings
                if ( Event.Source == Reference< XInterface >( m_xDocCfgMgr, UNO_QUERY ))
                {
                    // document settings removed
                    if ( m_xModuleCfgMgr->hasSettings( Event.ResourceURL ))
                    {
                        xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( m_xModuleCfgMgr ));
                        xElementSettings->updateSettings();
                        return;
                    }
                }

                bNoSettings = true;
            }

            // No settings anymore, element must be destroyed
            if ( m_xContainerWindow.is() && bNoSettings )
            {
                if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
                     aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
                {
                    Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
                    while ( pWindow && !pWindow->IsSystemWindow() )
                        pWindow = pWindow->GetParent();

                    if ( pWindow && !m_bInplaceMenuSet )
                        ((SystemWindow *)pWindow)->SetMenuBar( 0 );

                    Reference< XComponent > xComp( m_xMenuBar, UNO_QUERY );
                    if ( xComp.is() )
                        xComp->dispose();
                    m_xMenuBar.clear();
                }
                else if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
                {
                    destroyElement( Event.ResourceURL );
                }
            }
        }
    }
}

void SAL_CALL LayoutManager::elementReplaced( const ::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );

    ::rtl::OUString                aElementType;
    ::rtl::OUString                aElementName;
    Reference< XUIElement > xUIElement;
    Reference< XFrame >     xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        implts_findElement( Event.ResourceURL, aElementType, aElementName, xUIElement );

        Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
        if ( xElementSettings.is() )
        {
            ::rtl::OUString                    aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
            Reference< XInterface >     xElementCfgMgr;
            Reference< XPropertySet >   xPropSet( xElementSettings, UNO_QUERY );

            if ( xPropSet.is() )
                xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

            if ( !xElementCfgMgr.is() )
                return;

            // Check if the same UI configuration manager has changed => update settings
            if ( Event.Source == xElementCfgMgr )
            {
                xElementSettings->updateSettings();
                if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
                    doLayout();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------
//  OPropertySetHelper
//---------------------------------------------------------------------------------------------------------
// XPropertySet helper
sal_Bool SAL_CALL LayoutManager::convertFastPropertyValue( Any&       aConvertedValue ,
                                                           Any&       aOldValue       ,
                                                           sal_Int32  nHandle         ,
                                                           const Any& aValue             ) throw( com::sun::star::lang::IllegalArgumentException )
{
    return LayoutManager_PBase::convertFastPropertyValue( aConvertedValue, aOldValue, nHandle, aValue );
}

void SAL_CALL LayoutManager::setFastPropertyValue_NoBroadcast( sal_Int32                       nHandle ,
                                                               const com::sun::star::uno::Any& aValue  ) throw( com::sun::star::uno::Exception )
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
            sal_Bool bValue = sal_Bool();
            if (( aValue >>= bValue ) && bValue )
                implts_refreshContextToolbarsVisibility();
            break;
        }

        case LAYOUTMANAGER_PROPHANDLE_HIDECURRENTUI:
            implts_setCurrentUIVisibility( !m_bHideCurrentUI );
            break;
    }
}

void SAL_CALL LayoutManager::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                   sal_Int32                 nHandle    ) const
{
    LayoutManager_PBase::getFastPropertyValue( aValue, nHandle );
}

::cppu::IPropertyArrayHelper& SAL_CALL LayoutManager::getInfoHelper()
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfoHelper is NULL - for the second call pInfoHelper is different from NULL!
    static ::cppu::OPropertyArrayHelper* pInfoHelper = NULL;

    if( pInfoHelper == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        // Control this pointer again, another instance can be faster then these!
        if( pInfoHelper == NULL )
        {
            // Define static member to give structure of properties to baseclass "OPropertySetHelper".
            uno::Sequence< beans::Property > aProperties;
            describeProperties( aProperties );
            static ::cppu::OPropertyArrayHelper aInfoHelper( aProperties, sal_True );
            pInfoHelper = &aInfoHelper;
        }
    }

    return(*pInfoHelper);
}

com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > SAL_CALL LayoutManager::getPropertySetInfo() throw (::com::sun::star::uno::RuntimeException)
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pInfo is NULL - for the second call pInfo is different from NULL!
    static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >* pInfo = NULL;

    if( pInfo == NULL )
    {
        // Ready for multithreading
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;
        // Control this pointer again, another instance can be faster then these!
        if( pInfo == NULL )
        {
            // Create structure of propertysetinfo for baseclass "OPropertySetHelper".
            // (Use method "getInfoHelper()".)
            static com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
            pInfo = &xInfo;
        }
    }

    return (*pInfo);
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
