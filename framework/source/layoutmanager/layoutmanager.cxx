/*************************************************************************
 *
 *  $RCSfile: layoutmanager.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-11 17:22:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef _FRAMEWORK_SERVICES_LAYOUTMANAGER_HXX_
#include <services/layoutmanager.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include <services.h>
#endif

#include <classes/sfxhelperfunctions.hxx>

#ifndef __FRAMEWORK_UIELEMENT_MENUBARWRAPPER_HXX_
#include <uielement/menubarwrapper.hxx>
#endif

#ifndef __FRAMEWORK_HELPER_MODULEIDENTIFIER_HXX_
#include <helper/moduleidentifier.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#include <classes/addonsoptions.hxx>
#endif

#ifndef __FRAMEWORK_UICONFIGURATION_WINDOWSTATECONFIGURATION_HXX_
#include <uiconfiguration/windowstateconfiguration.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMEACTION_HPP_
#include <com/sun/star/frame/FrameAction.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_FRAME_XUICONTROLLERREGISTRATION_HPP_
#include <drafts/com/sun/star/frame/XUIControllerRegistration.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTICOMPONENTFACTORY_HPP_
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTMENUPEER_HPP_
#include <com/sun/star/awt/XSystemDependentMenuPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XYSTEMDEPENDENT_HPP_
#include <com/sun/star/lang/SystemDependent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VCLWINDOWPEERATTRIBUTE_HPP_
#include <com/sun/star/awt/VclWindowPeerAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XSYSTEMDEPENDENTWINDOWPEER_HPP_
#include <com/sun/star/awt/XSystemDependentWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_UIELEMENTTYPE_HPP_
#include <drafts/com/sun/star/ui/UIElementType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEREPLACE_HPP_
#include <com/sun/star/container/XNameReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _SVTOOLS_IMGDEF_HXX
#include <svtools/imgdef.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXMENU_HXX_
#include <toolkit/awt/vclxmenu.hxx>
#endif

#include <algorithm>

// ______________________________________________
//  using namespace

using namespace rtl;
//using namespace com::sun::star::awt;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace drafts::com::sun::star::ui;
using namespace com::sun::star::frame;
using namespace drafts::com::sun::star::frame;

#define UIRESOURCE_PROTOCO_ASCII        "private:"
#define UIRESOURCE_RESOURCE_ASCII       "resource"
#define UIRESOURCE_URL_ASCII            "private:resource"
#define UIRESOURCE_URL                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( UIRESOURCE_URL_ASCII ))


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

bool LayoutManager::UIElement::operator< ( const UIElement& aUIElement ) const
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
                    bool bEqual = ( m_aDockedData.m_aPos.Y() == aUIElement.m_aDockedData.m_aPos.Y() );
                    if ( !bEqual )
                        return  ( m_aDockedData.m_aPos.Y() < aUIElement.m_aDockedData.m_aPos.Y() );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.X() == aUIElement.m_aDockedData.m_aPos.X() );
                        if ( bEqual )
                        {
                            if ( m_bUserActive && !aUIElement.m_bUserActive )
                                return sal_True;
                            else if ( !m_bUserActive && aUIElement.m_bUserActive )
                                return sal_False;
                            else
                                return sal_False;
                        }
                        else
                            return ( m_aDockedData.m_aPos.X() <= aUIElement.m_aDockedData.m_aPos.X() );
                    }
                }
                else
                {
                    bool bEqual = ( m_aDockedData.m_aPos.X() == aUIElement.m_aDockedData.m_aPos.X() );
                    if ( !bEqual )
                        return ( m_aDockedData.m_aPos.X() < aUIElement.m_aDockedData.m_aPos.X() );
                    else
                    {
                        bool bEqual = ( m_aDockedData.m_aPos.Y() == aUIElement.m_aDockedData.m_aPos.Y() );
                        if ( bEqual )
                        {
                            if ( m_bUserActive && !aUIElement.m_bUserActive )
                                return sal_True;
                            else if ( !m_bUserActive && aUIElement.m_bUserActive )
                                return sal_False;
                            else
                                return sal_False;
                        }
                        else
                            return ( m_aDockedData.m_aPos.Y() <= aUIElement.m_aDockedData.m_aPos.Y() );
                    }
                }
            }
        }
    }
}

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_13                    (   LayoutManager                                                                    ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::frame::XLayoutManager                 ),
                                            DIRECT_INTERFACE( css::awt::XWindowListener                                     ),
                                            DIRECT_INTERFACE( css::frame::XFrameActionListener                              ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::ui::XUIConfigurationListener          ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::frame::XInplaceLayout                 ),
                                            DIRECT_INTERFACE( css::awt::XDockableWindowListener                             ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::frame::XMenuBarMergingAcceptor        ),
                                            DERIVED_INTERFACE( css::lang::XEventListener, css::frame::XFrameActionListener  ),
                                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet                     ),
                                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet                         )
                                        )

DEFINE_XTYPEPROVIDER_13                 (   LayoutManager                                           ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            drafts::com::sun::star::frame::XLayoutManager           ,
                                            css::awt::XWindowListener                               ,
                                            css::frame::XFrameActionListener                        ,
                                            css::lang::XEventListener                               ,
                                            drafts::com::sun::star::ui::XUIConfigurationListener    ,
                                            drafts::com::sun::star::frame::XInplaceLayout           ,
                                            css::awt::XDockableWindowListener                       ,
                                            drafts::com::sun::star::frame::XMenuBarMergingAcceptor  ,
                                            ::com::sun::star::beans::XMultiPropertySet              ,
                                            ::com::sun::star::beans::XFastPropertySet               ,
                                            ::com::sun::star::beans::XPropertySet
                                        )

DEFINE_XSERVICEINFO_MULTISERVICE        (   LayoutManager                       ,
                                            ::cppu::OWeakObject                 ,
                                            SERVICENAME_LAYOUTMANAGER           ,
                                            IMPLEMENTATIONNAME_LAYOUTMANAGER
                                        )

DEFINE_INIT_SERVICE                     (   LayoutManager, {} )


LayoutManager::LayoutManager( const Reference< XMultiServiceFactory >& xServiceManager ) :
        ThreadHelpBase( &Application::GetSolarMutex() )
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex()         )
        ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        ,   ::cppu::OWeakObject         (                                                   )
        ,   m_xSMGR( xServiceManager )
        ,   m_xURLTransformer( Reference< XURLTransformer >( xServiceManager->createInstance(
                                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                             UNO_QUERY ))
        ,   m_nLockCount( 0 )
        ,   m_bActive( sal_False )
        ,   m_bInplaceMenuSet( sal_False )
        ,   m_bDockingInProgress( sal_False )
        ,   m_bMenuVisible( sal_True )
        ,   m_bComponentAttached( sal_False )
        ,   m_bDoLayout( sal_False )
        ,   m_bVisible( sal_True )
        ,   m_xModuleManager( Reference< XModuleManager >(
                xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ))
        ,   m_xUIElementFactoryManager( Reference< ::drafts::com::sun::star::ui::XUIElementFactory >(
                xServiceManager->createInstance( SERVICENAME_UIELEMENTFACTORYMANAGER ), UNO_QUERY ))
        ,   m_xPersistentWindowStateSupplier( Reference< XNameAccess >(
                xServiceManager->createInstance( SERVICENAME_WINDOWSTATECONFIGURATION ), UNO_QUERY ))
        ,   m_pAddonOptions( 0 )
        ,   m_aCustomTbxPrefix( RTL_CONSTASCII_USTRINGPARAM( "custom_" ))
{
    m_pMiscOptions = new SvtMiscOptions();

    m_pMiscOptions->AddListener( LINK( this, LayoutManager, OptionsChanged ) );
    Application::AddEventListener( LINK( this, LayoutManager, SettingsChanged ) );
    m_eSymbolSet = m_pMiscOptions->GetSymbolSet();

    m_aAsyncLayoutTimer.SetTimeout( 50 );
    m_aAsyncLayoutTimer.SetTimeoutHdl( LINK( this, LayoutManager, AsyncLayoutHdl ) );
}

LayoutManager::~LayoutManager()
{
}

// Internal helper function
void LayoutManager::impl_clearUpMenuBar()
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    // Clear up VCL menu bar to prepare shutdown
    if ( m_xContainerWindow.is() )
    {
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
                        Any a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "XMenuBar" )));
                        a >>= xMenuBar;
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
}

sal_Bool LayoutManager::impl_parseResourceURL( const rtl::OUString aResourceURL, rtl::OUString& aElementType, rtl::OUString& aElementName )
{
    URL         aURL;
    sal_Int32   nIndex = 0;

    aURL.Complete = aResourceURL;
    m_xURLTransformer->parseStrict( aURL );

    OUString aUIResource = aURL.Path.getToken( 0, (sal_Unicode)'/', nIndex );

    if (( aURL.Protocol.equalsIgnoreAsciiCaseAscii( UIRESOURCE_PROTOCO_ASCII )) &&
        ( aUIResource.equalsIgnoreAsciiCaseAscii( UIRESOURCE_RESOURCE_ASCII )))
    {
        aElementType = aURL.Path.getToken( 0, (sal_Unicode)'/', nIndex );
        aElementName = aURL.Path.getToken( 0, (sal_Unicode)'/', nIndex );
        return sal_True;
    }

    return sal_False;
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
    OUString aModuleIdentifier( m_aModuleIdentifier );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    Reference< XModel > xModel;
    if ( xFrame.is() )
    {
        if ( bAttached )
        {
            OUString aOldModuleIdentifier( aModuleIdentifier );
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

            Reference< XController > xController = xFrame->getController();
            if ( xController.is() )
                xModel = xController->getModel();

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
            // Release references to our configuration managers as we currently don't have
            // an attached module.
            xModuleCfgMgr.clear();
            xDocCfgMgr.clear();
            xPersistentWindowState.clear();
            aModuleIdentifier = OUString();
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
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_destroyElements();

        if ( bAttached )
        {
            implts_createCustomToolBars();
            implts_createAddonsToolBars();
        }

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
    }
}

void LayoutManager::implts_destroyElements()
{
    UIElementVector aUIElementVector;

    WriteGuard aWriteLock( m_aLock );
    aUIElementVector = m_aUIElements;
    m_aUIElements.clear();
    aWriteLock.unlock();

    UIElementVector::iterator pIter;
    for ( pIter = aUIElementVector.begin(); pIter != aUIElementVector.end(); pIter++ )
    {
        Reference< XComponent > xComponent( pIter->m_xUIElement, UNO_QUERY );
        if ( xComponent.is() )
            xComponent->dispose();
    }

    aWriteLock.lock();
    impl_clearUpMenuBar();
    aWriteLock.unlock();
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
                vos::OGuard aGuard( Application::GetSolarMutex() );

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
        OUString aTbxResName;
        OUString aTbxTitle;
        for ( sal_Int32 j = 0; j < rTbxSeq.getLength(); j++ )
        {
            if ( rTbxSeq[j].Name.equalsAscii( "ResourceURL" ))
                rTbxSeq[j].Value >>= aTbxResName;
            else if ( rTbxSeq[j].Name.equalsAscii( "UIName" ))
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
    Reference< XUIConfigurationManager > xModuleCfgMgr( m_xModuleCfgMgr, UNO_QUERY );
    Reference< XUIConfigurationManager > xDocCfgMgr( m_xDocCfgMgr, UNO_QUERY );
    aReadLock.unlock();

    if ( xFrame.is() )
    {
        sal_Int32 i( 0 );
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

void LayoutManager::implts_createAddonsToolBars()
{
    WriteGuard aWriteLock( m_aLock );
    if ( !m_bComponentAttached )
        return;
    if ( !m_pAddonOptions )
        m_pAddonOptions = new AddonsOptions;
    Reference< XUIElementFactory > xUIElementFactory( m_xUIElementFactoryManager );
    Reference< XFrame > xFrame( m_xFrame );
    aWriteLock.unlock();

    UIElementVector aUIElementVector;
    Sequence< Sequence< PropertyValue > > aAddonToolBarData;
    Reference< XUIElement >               xUIElement;

    sal_uInt32 nCount = m_pAddonOptions->GetAddonsToolBarCount();
    OUString aAddonsToolBarStaticName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/addon_" ));
    OUString aElementType( RTL_CONSTASCII_USTRINGPARAM( "toolbar" ));

    Sequence< PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropSeq[0].Value = makeAny( xFrame );
    aPropSeq[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationData" ));
    for ( sal_uInt32 i = 0; i < nCount; i++ )
    {
        OUString aAddonToolBarName( aAddonsToolBarStaticName + OUString::valueOf( sal_Int32( i )));
        aAddonToolBarData = m_pAddonOptions->GetAddonsToolBarPart( i );
        aPropSeq[1].Value = makeAny( aAddonToolBarData );

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

                WriteGuard aWriteLock( m_aLock );
                UIElement& rElement = impl_findElement( aAddonToolBarName );
                if ( rElement.m_aName.getLength() > 0 )
                {
                    // Reuse a local entry so we are able to use the latest
                    // UI changes for this document.
                    implts_setElementData( rElement, xDockWindow );
                    rElement.m_xUIElement = xUIElement;
                }
                else
                {
                    // Create new UI element and try to read its state data
                    UIElement aNewToolbar( aAddonToolBarName, aElementType, xUIElement );
                    aNewToolbar.m_bFloating = sal_True;
                    implts_readWindowStateData( aAddonToolBarName, aNewToolbar );
                    implts_setElementData( aNewToolbar, xDockWindow );
                    m_aUIElements.push_back( aNewToolbar );
                }
                aWriteLock.unlock();
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

void LayoutManager::implts_toggleFloatingUIElementsVisibility( sal_Bool bActive )
{
    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XWindow > xWindow( xDockWindow, UNO_QUERY );
            if ( xDockWindow.is() && xWindow.is() )
            {
                sal_Bool bVisible( sal_True );
                vos::OGuard aGuard( Application::GetSolarMutex() );
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
                            vos::OGuard aGuard( Application::GetSolarMutex() );
                            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
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
    sal_Int32 nIndex = 0;
    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            ReadGuard aReadLock( m_aLock );
            xUIElement = m_xMenuBar;
            return sal_True;
        }
        else
        {
            UIElementVector::const_iterator pIter;

            ReadGuard aReadLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
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
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
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
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
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
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_aName == aName )
            return *pIter;
    }

    return aEmptyElement;
}

void LayoutManager::implts_writeNewStateData( const rtl::OUString aName, const Reference< css::awt::XWindow >& xWindow )
{
    sal_Bool            bWinData( sal_False );
    Window*             pWindow( 0 );
    css::awt::Rectangle aPosSize;
    sal_Bool            bVisible( sal_False );
    sal_Bool            bFloating( sal_True );

    if ( xWindow.is() )
    {
        Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
        if ( xDockWindow.is() )
            bFloating = xDockWindow->isFloating();
        aPosSize = xWindow->getPosSize();

        vos::OGuard aGuard( Application::GetSolarMutex() );
        pWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pWindow )
            bVisible = pWindow->IsVisible();
    }

    WriteGuard aWriteLock( m_aLock );
    UIElement& rUIElement = impl_findElement( aName );
    if ( rUIElement.m_xUIElement.is() && xWindow.is() )
    {
        rUIElement.m_bVisible   = bVisible;
        rUIElement.m_bFloating  = bFloating;
        if ( bFloating )
        {
            rUIElement.m_aFloatingData.m_aPos  = ::Point( aPosSize.X, aPosSize.Y );
            rUIElement.m_aFloatingData.m_aSize = ::Size( aPosSize.Width, aPosSize.Height );
        }
    }

    implts_writeWindowStateData( aName, rUIElement );

    aWriteLock.unlock();
}

sal_Bool LayoutManager::implts_readWindowStateData( const rtl::OUString& aName, UIElement& rElementData )
{
    ReadGuard aReadLock( m_aLock );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aReadLock.unlock();

    if ( xPersistentWindowState.is() )
    {
        try
        {
            Any a;
            Sequence< PropertyValue > aWindowState;
            a = xPersistentWindowState->getByName( aName );
            if ( a >>= aWindowState )
            {
                sal_Bool bValue( sal_False );
                for ( sal_Int32 n = 0; n < aWindowState.getLength(); n++ )
                {
                    if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKED ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bFloating = !bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_VISIBLE ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_bVisible = bValue;
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKINGAREA ))
                    {
                        drafts::com::sun::star::ui::DockingArea eDockingArea;
                        if ( aWindowState[n].Value >>= eDockingArea )
                            rElementData.m_aDockedData.m_nDockedArea = sal_Int16( eDockingArea );
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_DOCKPOS ))
                    {
                        css::awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                        {
                            rElementData.m_aDockedData.m_aPos.X() = aPoint.X;
                            rElementData.m_aDockedData.m_aPos.Y() = aPoint.Y;
                        }
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_POS ))
                    {
                        css::awt::Point aPoint;
                        if ( aWindowState[n].Value >>= aPoint )
                        {
                            rElementData.m_aFloatingData.m_aPos.X() = aPoint.X;
                            rElementData.m_aFloatingData.m_aPos.Y() = aPoint.Y;
                        }
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_SIZE ))
                    {
                        css::awt::Size aSize;
                        if ( aWindowState[n].Value >>= aSize )
                        {
                            rElementData.m_aFloatingData.m_aSize.Width() = aSize.Width;
                            rElementData.m_aFloatingData.m_aSize.Height() = aSize.Height;
                        }
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_UINAME ))
                        aWindowState[n].Value >>= rElementData.m_aUIName;
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_STYLE ))
                    {
                        sal_Int32 nStyle;
                        if ( aWindowState[n].Value >>= nStyle )
                            rElementData.m_nStyle = sal_Int16( nStyle );
                    }
                    else if ( aWindowState[n].Name.equalsAscii( WINDOWSTATE_PROPERTY_LOCKED ))
                    {
                        if ( aWindowState[n].Value >>= bValue )
                            rElementData.m_aDockedData.m_bLocked = bValue;
                    }
                }

                return sal_True;
            }
        }
        catch ( NoSuchElementException& )
        {
        }
    }

    return sal_False;
}

void LayoutManager::implts_writeWindowStateData( const rtl::OUString& aName, const UIElement& rElementData )
{
    ReadGuard aReadLock( m_aLock );
    Reference< XNameAccess > xPersistentWindowState( m_xPersistentWindowState );
    aReadLock.unlock();

    sal_Bool bPersistent( sal_False );
    Reference< XPropertySet > xPropSet( rElementData.m_xUIElement, UNO_QUERY );
    if ( xPropSet.is() )
    {
        try
        {
            // Check persistent flag of the user interface element
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ))) >>= bPersistent;
        }
        catch ( com::sun::star::beans::UnknownPropertyException )
        {
        }
        catch ( com::sun::star::lang::WrappedTargetException )
        {
        }
    }

    if ( bPersistent && xPersistentWindowState.is() )
    {
        try
        {
            sal_Bool bValue( sal_False );
            Sequence< PropertyValue > aWindowState( 8 );

            aWindowState[0].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_DOCKED ));
            aWindowState[0].Value   = makeAny( sal_Bool( !rElementData.m_bFloating ));
            aWindowState[1].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_VISIBLE ));
            aWindowState[1].Value   = makeAny( sal_Bool( rElementData.m_bVisible ));

            aWindowState[2].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_DOCKINGAREA ));
            aWindowState[2].Value   = makeAny( static_cast< DockingArea >( rElementData.m_aDockedData.m_nDockedArea ) );

            css::awt::Point aPos;
            aPos.X = rElementData.m_aDockedData.m_aPos.X();
            aPos.Y = rElementData.m_aDockedData.m_aPos.Y();
            aWindowState[3].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_DOCKPOS ));
            aWindowState[3].Value   = makeAny( aPos );

            aPos.X = rElementData.m_aFloatingData.m_aPos.X();
            aPos.Y = rElementData.m_aFloatingData.m_aPos.Y();
            aWindowState[4].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_POS ));
            aWindowState[4].Value   = makeAny( aPos );

            css::awt::Size aSize;
            aSize.Width = rElementData.m_aFloatingData.m_aSize.Width();
            aSize.Height = rElementData.m_aFloatingData.m_aSize.Height();
            aWindowState[5].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_SIZE ));
            aWindowState[5].Value   = makeAny( aSize );
            aWindowState[6].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_STYLE ));
            aWindowState[6].Value   = makeAny( sal_Int32( rElementData.m_nStyle ));
            aWindowState[7].Name    = OUString( RTL_CONSTASCII_USTRINGPARAM( WINDOWSTATE_PROPERTY_LOCKED ));
            aWindowState[7].Value   = makeAny( sal_Bool( rElementData.m_aDockedData.m_bLocked ));

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
}

void LayoutManager::implts_setElementData( UIElement& rElement, const Reference< css::awt::XDockableWindow >& rDockWindow )
{
    Reference< css::awt::XDockableWindow > xDockWindow( rDockWindow );
    Reference< css::awt::XWindow >         xWindow( xDockWindow, UNO_QUERY );

    Window*     pWindow( 0 );
    ToolBox*    pToolBox( 0 );

    if ( xDockWindow.is() && xWindow.is() )
    {
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );
            pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow )
            {
                String aText = pWindow->GetText();
                if ( aText.Len() == 0 )
                    pWindow->SetText( rElement.m_aUIName );
                if ( pWindow->GetType() == WINDOW_TOOLBOX )
                    pToolBox = (ToolBox *)pWindow;
            }
            if ( pToolBox )
            {
                if (( rElement.m_nStyle < 0 ) ||
                    ( rElement.m_nStyle > BUTTON_SYMBOLTEXT ))
                    rElement.m_nStyle = BUTTON_SYMBOL;
                pToolBox->SetButtonType( (ButtonType)rElement.m_nStyle );
            }
        }

        if ( rElement.m_bFloating )
        {
            if ( pWindow )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                String aText = pWindow->GetText();
                if ( aText.Len() == 0 )
                    pWindow->SetText( rElement.m_aUIName );
            }

            ::Point  aPos( rElement.m_aFloatingData.m_aPos.X(),
                           rElement.m_aFloatingData.m_aPos.Y() );
            sal_Bool bWriteData( sal_False );
            sal_Bool bUndefPos = ( rElement.m_aFloatingData.m_aPos.X() == LONG_MAX ||
                                   rElement.m_aFloatingData.m_aPos.Y() == LONG_MAX );
            sal_Bool bSetSize = ( rElement.m_aFloatingData.m_aSize.Width() != 0 &&
                                  rElement.m_aFloatingData.m_aSize.Height() != 0 );
            xDockWindow->setFloatingMode( sal_True );
            if ( bUndefPos )
            {
                aPos = implts_findNextCascadeFloatingPos();
                rElement.m_aFloatingData.m_aPos = aPos; // set new cascaded position
                bWriteData = sal_True;
            }

            xWindow->setPosSize( aPos.X(),
                                aPos.Y(),
                                rElement.m_aFloatingData.m_aSize.Width(),
                                rElement.m_aFloatingData.m_aSize.Height(),
                                bSetSize ? css::awt::PosSize::POSSIZE : css::awt::PosSize::POS );

            if ( bWriteData )
                implts_writeWindowStateData( rElement.m_aName, rElement );
            if ( rElement.m_bVisible && pWindow )
            {
                {
                    vos::OGuard aGuard( Application::GetSolarMutex() );
                    pWindow->Show( sal_True, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                }
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
                vos::OGuard aGuard( Application::GetSolarMutex() );
                pToolBox->SetAlign( ImplConvertAlignment(rElement.m_aDockedData.m_nDockedArea )  );
                pToolBox->SetLineCount( 1 );
                if ( rElement.m_aDockedData.m_bLocked )
                    xDockWindow->lock();
                aSize = pToolBox->CalcWindowSizePixel();
                bSetSize = sal_True;

                if (( rElement.m_aDockedData.m_aPos.X() == LONG_MAX ) &&
                    ( rElement.m_aDockedData.m_aPos.Y() == LONG_MAX ))
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
                                 aSize.Width(),
                                 aSize.Height(),
                                 bSetSize ? css::awt::PosSize::POSSIZE : css::awt::PosSize::POS );
            if ( rElement.m_bVisible && pWindow )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
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
        vos::OGuard aGuard( Application::GetSolarMutex() );
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
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XDockableWindow > xDockWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XWindow > xWindow( xDockWindow, UNO_QUERY );
            if ( xDockWindow.is() && xWindow.is() && xDockWindow->isFloating() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if ( pWindow && pWindow->IsVisible() )
                {
                    css::awt::Rectangle aFloatRect = xWindow->getPosSize();
                    if ((( aFloatRect.X - nHotZoneX ) <= aStartPos.X() ) &&
                        ( aFloatRect.X >= aStartPos.X() ) &&
                        (( aFloatRect.Y - nHotZoneY ) <= aStartPos.Y() ) &&
                        ( aFloatRect.Y >= aStartPos.Y() ))
                    {
                        aCurrPos.X() = aFloatRect.X + nCascadeIndentX;
                        aCurrPos.Y() = aFloatRect.Y + nCascadeIndentY;
                        break;
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
        vos::OGuard aGuard( Application::GetSolarMutex() );
        pDockingWindow  = VCLUnoHelper::GetWindow( xDockingWindow );
        aDockingWinSize = pDockingWindow->GetOutputSizePixel();
    }

    sal_Int32 nFreeRowColPixelPos( 0 );
    sal_Int32 nMaxSpace( 0 );
    sal_Int32 nNeededSpace( 0 );
    sal_Int32 nTopDockingAreaSize( 0 );
    sal_Bool  bFoundRowColumn( sal_False );

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
    sal_Int32 nCurrRowCol( 0 );
    for ( sal_Int32 i = 0; i < sal_Int32( aRowColumnsWindowData.size()); i++ )
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
            for ( sal_Int32 j = 0; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size()); j++ )
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
    vos::OGuard aGuard( Application::GetSolarMutex() );
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
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_bUserActive )
            pIter->m_bUserActive = sal_False;
    }

    aWriteLock.unlock();
}

::Point LayoutManager::implts_convertVirtualToPhysicalPos( DockingArea eDockingArea, const ::Point& aPoint ) const
{
    return aPoint;
}

void LayoutManager::implts_getDockingAreaElementInfos( DockingArea eDockingArea, std::vector< SingleRowColumnWindowData >& rRowColumnsWindowData )
{
    std::vector< UIElement > aWindowVector;

    if (( eDockingArea < DockingArea_DOCKINGAREA_TOP ) ||
        ( eDockingArea > DockingArea_DOCKINGAREA_RIGHT ))
        eDockingArea = DockingArea_DOCKINGAREA_TOP;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    UIElementVector::iterator   pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_aDockedData.m_nDockedArea == eDockingArea )
        {
            Reference< XUIElement > xUIElement( pIter->m_xUIElement );
            if ( xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                if ( xWindow.is() )
                {
                    vos::OGuard aGuard( Application::GetSolarMutex() );
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                    if ( pWindow &&
                         pIter->m_bVisible &&
                         xDockWindow.is() &&
                         !pIter->m_bFloating )
                    {
                        // docked windows
                        const UIElement& rElement = *pIter;
                        aWindowVector.push_back( *pIter );
                    }
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

    for ( j = 0; j < sal_Int32( aWindowVector.size()); j++ )
    {
        const UIElement& rElement = aWindowVector[j];
        Reference< css::awt::XWindow > xWindow;
        Reference< XUIElement > xUIElement( rElement.m_xUIElement );
        css::awt::Rectangle aPosSize;
        if ( xUIElement.is() )
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );
            xWindow = Reference< css::awt::XWindow >( xUIElement->getRealInterface(), UNO_QUERY );
            aPosSize = xWindow->getPosSize();

            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
            if ( pWindow->GetType() == WINDOW_TOOLBOX )
            {
                ::Size aSize = ((ToolBox*)pWindow)->CalcWindowSizePixel( 1 );
                aPosSize.Width = aSize.Width();
                aPosSize.Height = aSize.Height();
            }
        }
        else
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
                ++nIndex;
                nLastPos = 0;
                nCurrPos = rElement.m_aDockedData.m_aPos.Y();
                SingleRowColumnWindowData aRowColumnWindowData;
                aRowColumnWindowData.nRowColumn = nCurrPos;
                rRowColumnsWindowData.push_back( aRowColumnWindowData );
            }

            // Calc space before an element and store it
            nSpace = ( rElement.m_aDockedData.m_aPos.X() - nLastPos );
            if ( rElement.m_aDockedData.m_aPos.X() > nLastPos )
                rRowColumnsWindowData[nIndex].nSpace += nSpace;
            else
                nSpace = 0;
            rRowColumnsWindowData[nIndex].aRowColumnSpace.push_back( nSpace );
            nLastPos = rElement.m_aDockedData.m_aPos.X() + aPosSize.Width;

            rRowColumnsWindowData[nIndex].aRowColumnWindows.push_back( xWindow );
            rRowColumnsWindowData[nIndex].aUIElementNames.push_back( rElement.m_aName );
            rRowColumnsWindowData[nIndex].aRowColumnWindowSizes.push_back(
                css::awt::Rectangle( rElement.m_aDockedData.m_aPos.X(),
                                     rElement.m_aDockedData.m_aPos.Y(),
                                     aPosSize.Width,
                                     aPosSize.Height ));
            if ( rRowColumnsWindowData[nIndex].nStaticSize < aPosSize.Height )
                rRowColumnsWindowData[nIndex].nStaticSize = aPosSize.Height;
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
                rRowColumnsWindowData[nIndex].nSpace += nSpace;
            else
                nSpace = 0;
            rRowColumnsWindowData[nIndex].aRowColumnSpace.push_back( nSpace );
            nLastPos = rElement.m_aDockedData.m_aPos.Y() + aPosSize.Height;

            rRowColumnsWindowData[nIndex].aRowColumnWindows.push_back( xWindow );
            rRowColumnsWindowData[nIndex].aUIElementNames.push_back( rElement.m_aName );
            rRowColumnsWindowData[nIndex].aRowColumnWindowSizes.push_back(
                css::awt::Rectangle( rElement.m_aDockedData.m_aPos.X(),
                                     rElement.m_aDockedData.m_aPos.Y(),
                                     aPosSize.Width,
                                     aPosSize.Height ));
            if ( rRowColumnsWindowData[nIndex].nStaticSize < aPosSize.Width )
                rRowColumnsWindowData[nIndex].nStaticSize = aPosSize.Width;
            rRowColumnsWindowData[nIndex].nVarSize += aPosSize.Height + nSpace;
        }
    }
}

::Rectangle LayoutManager::implts_calcDockingPosSize( UIElement&           rUIElement,
                                                      const ::Rectangle&   rTrackingRect,
                                                      const ::Point&       rMousePos )
{
    ReadGuard aReadLock( m_aLock );
    Reference< css::awt::XWindow > xContainerWindow = m_xContainerWindow;
    ::Size                         aContainerWinSize;
    Window* pContainerWindow( 0 );
    aReadLock.unlock();

    {
        // Retrieve output size from container Window
        vos::OGuard aGuard( Application::GetSolarMutex() );
        pContainerWindow  = VCLUnoHelper::GetWindow( xContainerWindow );
        aContainerWinSize = pContainerWindow->GetOutputSizePixel();
    }

    if ( !rUIElement.m_xUIElement.is() )
        return ::Rectangle();

    Window*                        pDockWindow( 0 );
    Window*                        pDockingAreaWindow( 0 );
    ToolBox*                       pToolBox( 0 );
    Reference< css::awt::XWindow > xWindow( rUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
    Reference< css::awt::XWindow > xDockingAreaWindow;
    ::Rectangle                    aTrackingRect( rTrackingRect );
    sal_Int16                      nDockedArea( rUIElement.m_aDockedData.m_nDockedArea );
    sal_Int32                      nTopDockingAreaSize( implts_getTopBottomDockingAreaSizes().Width() );
    sal_Int32                      nBottomDockingAreaSize( implts_getTopBottomDockingAreaSizes().Height() );
    sal_Bool                       bHorizontalDockArea( ( rUIElement.m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_TOP ) ||
                                                        ( rUIElement.m_aDockedData.m_nDockedArea == DockingArea_DOCKINGAREA_BOTTOM ));
    sal_Int32                      nMaxLeftRightDockAreaSize = aContainerWinSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize;

    aReadLock.lock();
    xDockingAreaWindow = m_xDockAreaWindows[nDockedArea];
    aReadLock.unlock();

    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        pDockingAreaWindow = VCLUnoHelper::GetWindow( xDockingAreaWindow );
        pDockWindow        = VCLUnoHelper::GetWindow( xWindow );
        if ( pDockWindow && pDockWindow->GetType() == WINDOW_TOOLBOX )
            pToolBox = (ToolBox *)pDockWindow;

        if ( pToolBox )
        {
            // docked toolbars always have one line
            ::Size aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( nDockedArea ) );
            aTrackingRect.SetSize( ::Size( aSize.Width(), aSize.Height() ));
        }
    }

    ::Rectangle aUIElementRect;
    sal_Int32 nDockLine( -1 );
    sal_Bool  bInsertBefore( sal_False );
    std::vector< sal_Int32 > aDockAreaLinePos;
    std::vector< sal_Int32 > aDockAreaLineSize;
    std::vector< sal_Int32 > aDockAreaLinePixelPos;
    UIElementVector::iterator pIter;

    sal_Int32 nCurrLinePos( -1 );
    aReadLock.lock();
    sal_Int32 j( 0 );

    sal_Int32 nRowColPixelPos( 0 );
    if ( nDockedArea == DockingArea_DOCKINGAREA_BOTTOM )
        nRowColPixelPos = aContainerWinSize.Height();
    else if ( nDockedArea == DockingArea_DOCKINGAREA_RIGHT )
        nRowColPixelPos = aContainerWinSize.Width();

    aReadLock.lock();
    UIElementVector::iterator pFoundIter = m_aUIElements.end();
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        sal_Int32 nCurrDockedArea( pIter->m_aDockedData.m_nDockedArea );

        if (( nCurrDockedArea == nDockedArea ) &&
              pIter->m_xUIElement.is() &&
              !pIter->m_bFloating &&
              pIter->m_bVisible )
        {
            css::awt::Rectangle             aWinRect;
            sal_Int32                       nDockedArea( rUIElement.m_aDockedData.m_nDockedArea );
            Reference< css::awt::XWindow >  xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );

            if ( xWindow.is() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );

                aWinRect = xWindow->getPosSize();
                ::Rectangle aRect( aWinRect.X, aWinRect.Y,
                                   aWinRect.X + aWinRect.Width, aWinRect.Y + aWinRect.Height );

                // Calc correct position of the window rectangle to be able to compare it with mouse pos/tracking rect
                aRect.SetPos( pContainerWindow->ScreenToOutputPixel( pDockingAreaWindow->OutputToScreenPixel( aRect.TopLeft() )));

                sal_Bool bIsInside( sal_False );
                bIsInside = aRect.IsInside( rMousePos );

                if ( bIsInside )
                {
                    pFoundIter = pIter;
                    sal_Int32  nMiddle( bHorizontalDockArea ? ( aRect.Left() + aRect.getWidth() / 2 ) :
                                                              ( aRect.Top() + aRect.getHeight() / 2 ));
                    bInsertBefore = bHorizontalDockArea ? ( rMousePos.X() < nMiddle ) : ( rMousePos.Y() < nMiddle );
                    if ( bInsertBefore )
                    {
                        aUIElementRect.SetPos( aRect.TopLeft() );
                        if ( bHorizontalDockArea )
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( aContainerWinSize.Width() -  aRect.Left() ),
                                                                                    sal_Int32( aTrackingRect.getWidth() )));
                            if ( nSize == 0 )
                                nSize = aRect.getWidth();
                            aUIElementRect.SetSize( ::Size( nSize, aRect.getHeight() ));

                            // Set virtual position
                            rUIElement.m_aDockedData.m_aPos.X() = aRect.Left();
                            rUIElement.m_aDockedData.m_aPos.Y() = pIter->m_aDockedData.m_aPos.Y();
                        }
                        else
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( nTopDockingAreaSize + nMaxLeftRightDockAreaSize - aRect.Top() ),
                                                                                    sal_Int32( aTrackingRect.getHeight() )));
                            if ( nSize == 0 )
                                nSize = aRect.getHeight();
                            aUIElementRect.SetSize( ::Size( aRect.getWidth(), nSize ));

                            // Set virtual position
                            sal_Int32 nPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( aRect.TopLeft() )).Y();
                            rUIElement.m_aDockedData.m_aPos.X() = pIter->m_aDockedData.m_aPos.X();
                            rUIElement.m_aDockedData.m_aPos.Y() = nPosY;
                        }
                    }
                    else
                    {
                        if ( bHorizontalDockArea )
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32(( aContainerWinSize.Width() ) - aRect.Right() ),
                                                                                    sal_Int32( aTrackingRect.getWidth() )));
                            if ( nSize == 0 )
                            {
                                aUIElementRect.SetPos( ::Point( aContainerWinSize.Width() - aTrackingRect.getWidth(), aRect.Top() ));
                                aUIElementRect.SetSize( ::Size( aTrackingRect.getWidth(), aRect.getHeight() ));
                                rUIElement.m_aDockedData.m_aPos.X() = aUIElementRect.Left();
                            }
                            else
                            {
                                aUIElementRect.SetPos( ::Point( aRect.Right(), aRect.Top() ));
                                aUIElementRect.SetSize( ::Size( nSize, aRect.getHeight() ));
                                rUIElement.m_aDockedData.m_aPos.X() = aRect.Right();
                            }

                            // Set virtual position
                            rUIElement.m_aDockedData.m_aPos.Y() = pIter->m_aDockedData.m_aPos.Y();
                        }
                        else
                        {
                            sal_Int32 nSize = ::std::max( sal_Int32( 0 ), std::min( sal_Int32( nTopDockingAreaSize + nMaxLeftRightDockAreaSize - aRect.Bottom() ),
                                                                                    sal_Int32( aTrackingRect.getHeight() )));
                            aUIElementRect.SetPos( ::Point( aRect.Left(), aRect.Bottom() ));
                            aUIElementRect.SetSize( ::Size( aRect.getWidth(), nSize ));

                            // Set virtual position
                            sal_Int32 nPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( aRect.BottomRight() )).Y();
                            rUIElement.m_aDockedData.m_aPos.X() = pIter->m_aDockedData.m_aPos.X();
                            rUIElement.m_aDockedData.m_aPos.Y() = nPosY;
                        }
                    }

                    break;
                }

                switch ( nCurrDockedArea )
                {
                    case DockingArea_DOCKINGAREA_BOTTOM:
                    case DockingArea_DOCKINGAREA_TOP:
                    {
                        if ( pIter->m_aDockedData.m_aPos.Y() > nCurrLinePos )
                        {
                            nCurrLinePos = pIter->m_aDockedData.m_aPos.Y();
                            aDockAreaLinePos.push_back( nCurrLinePos );
                            aDockAreaLineSize.push_back( aRect.getHeight() );

                            if ( nCurrDockedArea == DockingArea_DOCKINGAREA_BOTTOM )
                                nRowColPixelPos -= aRect.getHeight();
                            else
                                nRowColPixelPos += aRect.getHeight();
                            aDockAreaLinePixelPos.push_back( nRowColPixelPos );
                        }
                        break;
                    }

                    case DockingArea_DOCKINGAREA_RIGHT:
                    case DockingArea_DOCKINGAREA_LEFT:
                    {
                        if ( pIter->m_aDockedData.m_aPos.X() > nCurrLinePos )
                        {
                            nCurrLinePos = pIter->m_aDockedData.m_aPos.X();
                            aDockAreaLinePos.push_back( nCurrLinePos );
                            aDockAreaLineSize.push_back( aRect.getWidth() );

                            if ( nCurrDockedArea == DockingArea_DOCKINGAREA_RIGHT )
                                nRowColPixelPos -= aRect.getWidth();
                            else
                                nRowColPixelPos += aRect.getWidth();
                            aDockAreaLinePixelPos.push_back( nRowColPixelPos );
                        }
                        break;
                    }
                }
            }
            j++;
        }
    }
    aReadLock.unlock();

    if ( pFoundIter != m_aUIElements.end() )
        return aUIElementRect;
    else
    {
        ::Point  aWinPos    = aTrackingRect.TopLeft();
        ::Size   aWinSize   = aTrackingRect.GetSize();

        if ( aDockAreaLinePos.size() > 0 )
        {
            if (( nDockedArea == DockingArea_DOCKINGAREA_TOP ) ||
                ( nDockedArea == DockingArea_DOCKINGAREA_LEFT ))
            {
                sal_Int32 nRowColBorder( 0 );
                sal_Int32 nPixelPos( 0 );
                for ( sal_Int32 i = 0; i < sal_Int32( aDockAreaLinePos.size() ); i++ )
                {
                    nRowColBorder += aDockAreaLineSize[i];
                    switch ( nDockedArea )
                    {
                        case DockingArea_DOCKINGAREA_TOP:
                        {
                            if ( rMousePos.Y() <= nRowColBorder )
                            {
                                sal_Int32 nPosX( std::max( sal_Int32( aWinPos.X()), sal_Int32( 0 )));
                                if (( nPosX + aWinSize.Width()) > aContainerWinSize.Width() )
                                    nPosX = std::min( nPosX,
                                                    std::max( sal_Int32( aContainerWinSize.Width() - aWinSize.Width() ),
                                                                sal_Int32( 0 )));

                                sal_Int32 nSize = std::min( aContainerWinSize.Width(), aWinSize.Width() );

                                // Set virtual position
                                rUIElement.m_aDockedData.m_aPos.X() = nPosX;
                                rUIElement.m_aDockedData.m_aPos.Y() = aDockAreaLinePos[i];

                                return ::Rectangle( ::Point( nPosX, nPixelPos ),
                                                    ::Size( nSize, aDockAreaLineSize[i] ));
                            }
                            break;
                        }
                        case DockingArea_DOCKINGAREA_LEFT:
                        {
                            if ( rMousePos.X() <= nRowColBorder )
                            {
                                sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                                            sal_Int32( aContainerWinSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize ));

                                sal_Int32 nPosY( std::max( sal_Int32( aWinPos.Y()), sal_Int32( nTopDockingAreaSize )));
                                if (( nPosY + aWinSize.Height()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
                                    nPosY = std::min( nPosY,
                                                    std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aWinSize.Height() )),
                                                              sal_Int32( nTopDockingAreaSize )));

                                sal_Int32 nSize = std::min( nMaxDockingAreaHeight, aWinSize.Height() );

                                // Set virtual position
                                sal_Int32 nDockPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( ::Point( 0, nPosY ))).Y();
                                rUIElement.m_aDockedData.m_aPos.X() = aDockAreaLinePos[i];
                                rUIElement.m_aDockedData.m_aPos.Y() = nDockPosY;

                                return ::Rectangle( ::Point( nPixelPos, nPosY ),
                                                    ::Size( aDockAreaLineSize[i], nSize ));
                            }
                            break;
                        }
                    }
                    nPixelPos += aDockAreaLineSize[i];
                }
            }
            else
            {
                sal_Bool  bBottom( nDockedArea == DockingArea_DOCKINGAREA_BOTTOM );
                sal_Int32 nPixelPos( bBottom ? sal_Int32( aContainerWinSize.Height() ) :
                                               sal_Int32( aContainerWinSize.Width()  ));
                for ( sal_Int32 i = 0; i < sal_Int32( aDockAreaLinePos.size() ); i++ )
                {
                    nPixelPos -= aDockAreaLineSize[i];
                    switch ( nDockedArea )
                    {
                        case DockingArea_DOCKINGAREA_BOTTOM:
                        {
                            if ( rMousePos.Y() >= nPixelPos )
                            {
                                sal_Int32 nPosX( std::max( sal_Int32( aWinPos.X()), sal_Int32( 0 )));
                                if (( nPosX + aWinSize.Width()) > aContainerWinSize.Width() )
                                    nPosX = std::min( nPosX,
                                                    std::max( sal_Int32( aContainerWinSize.Width() - aWinSize.Width() ),
                                                                sal_Int32( 0 )));

                                sal_Int32 nSize = std::min( aContainerWinSize.Width(), aWinSize.Width() );

                                // Set virtual position
                                rUIElement.m_aDockedData.m_aPos.X() = nPosX;
                                rUIElement.m_aDockedData.m_aPos.Y() = aDockAreaLinePos[i];

                                return ::Rectangle( ::Point( nPosX, nPixelPos ),
                                                    ::Size( nSize, aDockAreaLineSize[i] ));
                            }
                            break;
                        }
                        case DockingArea_DOCKINGAREA_RIGHT:
                        {
                            if ( rMousePos.X() >= nPixelPos )
                            {
                                sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                                            sal_Int32( aContainerWinSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize ));

                                sal_Int32 nPosY( std::max( sal_Int32( aWinPos.Y()), sal_Int32( nTopDockingAreaSize )));
                                if (( nPosY + aWinSize.Height()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
                                    nPosY = std::min( nPosY,
                                                      std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aWinSize.Height() )),
                                                                sal_Int32( nTopDockingAreaSize )));

                                sal_Int32 nSize = std::min( nMaxDockingAreaHeight, aWinSize.Height() );

                                // Set virtual position
                                sal_Int32 nDockPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( ::Point( 0, nPosY ))).Y();
                                rUIElement.m_aDockedData.m_aPos.X() = aDockAreaLinePos[i];
                                rUIElement.m_aDockedData.m_aPos.Y() = nDockPosY;

                                return ::Rectangle( ::Point( nPixelPos, nPosY ),
                                                    ::Size( aDockAreaLineSize[i], nSize ));
                            }
                            break;
                        }
                    }
                }
            }
        }

        sal_Int32 nLastDockPixelPos( 0 );

        if ( aDockAreaLinePixelPos.size() > 0 )
        {
            sal_Int32 nLastIndex = aDockAreaLinePixelPos.size()-1;
            if (( nDockedArea == DockingArea_DOCKINGAREA_TOP ) ||
                ( nDockedArea == DockingArea_DOCKINGAREA_LEFT ))
                nLastDockPixelPos = aDockAreaLinePixelPos[nLastIndex];
            else
                nLastDockPixelPos = aDockAreaLinePixelPos[nLastIndex];
        }

        switch ( nDockedArea )
        {
            case DockingArea_DOCKINGAREA_TOP:
            {
                sal_Int32 nPosX( std::max( sal_Int32( aWinPos.X()), sal_Int32( 0 )));
                if (( nPosX + aWinSize.Width()) > aContainerWinSize.Width() )
                    nPosX = std::min( nPosX,
                                      std::max( sal_Int32( aContainerWinSize.Width() - aWinSize.Width() ),
                                                sal_Int32( 0 )));

                sal_Int32 nSize = std::min( aContainerWinSize.Width(), aWinSize.Width() );

                rUIElement.m_aDockedData.m_aPos.X() = nPosX;
                if ( aDockAreaLinePixelPos.size() == 0 )
                    rUIElement.m_aDockedData.m_aPos.Y() = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.Y() = aDockAreaLinePos[aDockAreaLinePixelPos.size()-1]+1;

                return ::Rectangle( ::Point( nPosX, nLastDockPixelPos ), ::Size( nSize, aWinSize.Height() ));
            }

            case DockingArea_DOCKINGAREA_BOTTOM:
            {
                sal_Int32 nPosX( std::max( sal_Int32( aWinPos.X()), sal_Int32( 0 )));
                if (( nPosX + aWinSize.Width()) > aContainerWinSize.Width() )
                    nPosX = std::min( nPosX,
                                      std::max( sal_Int32( aContainerWinSize.Width() - aWinSize.Width() ),
                                                sal_Int32( 0 )));

                sal_Int32 nSize = std::min( aContainerWinSize.Width(), aWinSize.Width() );
                if ( aDockAreaLinePos.size() == 0 )
                    nLastDockPixelPos = ( aContainerWinSize.Height() - aWinSize.Height() );
                else
                    nLastDockPixelPos = nLastDockPixelPos - aWinSize.Height();

                rUIElement.m_aDockedData.m_aPos.X()        = nPosX;
                if ( aDockAreaLinePixelPos.size() == 0 )
                    rUIElement.m_aDockedData.m_aPos.Y() = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.Y() = aDockAreaLinePos[aDockAreaLinePixelPos.size()-1]+1;

                return ::Rectangle( ::Point( nPosX, nLastDockPixelPos ), ::Size( nSize, aWinSize.Height() ));
            }

            case DockingArea_DOCKINGAREA_LEFT:
            {
                sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                            sal_Int32( aContainerWinSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize ));

                sal_Int32 nPosY( std::max( sal_Int32( aWinPos.Y()), sal_Int32( nTopDockingAreaSize )));
                if (( nPosY + aWinSize.Height()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
                    nPosY = std::min( nPosY,
                                      std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aWinSize.Height() )),
                                                sal_Int32( nTopDockingAreaSize )));

                sal_Int32 nSize = std::min( nMaxDockingAreaHeight, aWinSize.Height() );

                sal_Int32 nDockPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( ::Point( 0, nPosY ))).Y();
                if ( aDockAreaLinePixelPos.size() == 0 )
                    rUIElement.m_aDockedData.m_aPos.X() = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.X() = aDockAreaLinePos[aDockAreaLinePixelPos.size()-1]+1;
                rUIElement.m_aDockedData.m_aPos.Y()       = nDockPosY;

                return ::Rectangle( ::Point( nLastDockPixelPos, nPosY ), ::Size( aWinSize.Width(), nSize ));
            }

            case DockingArea_DOCKINGAREA_RIGHT:
            {
                sal_Int32 nMaxDockingAreaHeight = std::max( sal_Int32( 0 ),
                                                            sal_Int32( aContainerWinSize.Height() - nTopDockingAreaSize - nBottomDockingAreaSize ));

                sal_Int32 nPosY( std::max( sal_Int32( aWinPos.Y()), sal_Int32( nTopDockingAreaSize )));
                if (( nPosY + aWinSize.Height()) > ( nTopDockingAreaSize + nMaxDockingAreaHeight ))
                    nPosY = std::min( nPosY,
                                      std::max( sal_Int32( nTopDockingAreaSize + ( nMaxDockingAreaHeight - aWinSize.Height() )),
                                                sal_Int32( nTopDockingAreaSize )));

                sal_Int32 nSize = std::min( nMaxDockingAreaHeight, aWinSize.Height() );
                if ( aDockAreaLinePos.size() == 0 )
                    nLastDockPixelPos = ( aContainerWinSize.Width() - aWinSize.Width() );
                else
                    nLastDockPixelPos = nLastDockPixelPos - aWinSize.Width();

                sal_Int32 nDockPosY = pDockingAreaWindow->ScreenToOutputPixel( pContainerWindow->OutputToScreenPixel( ::Point( 0, nPosY ))).Y();
                if ( aDockAreaLinePixelPos.size() == 0 )
                    rUIElement.m_aDockedData.m_aPos.X() = 0;
                else
                    rUIElement.m_aDockedData.m_aPos.X() = aDockAreaLinePos[aDockAreaLinePixelPos.size()-1]+1;
                rUIElement.m_aDockedData.m_aPos.Y()       = nDockPosY;

                return ::Rectangle( ::Point( nLastDockPixelPos, nPosY ), ::Size( aWinSize.Width(), nSize ));
            }
        }
    }

    return aTrackingRect;
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

void LayoutManager::implts_sortActiveElement( const UIElement& rActiveUIElement )
{
    implts_sortUIElements();
}

Reference< XUIElement > LayoutManager::implts_createElement( const rtl::OUString& aName )
{
    Reference< drafts::com::sun::star::ui::XUIElement > xUIElement;

    ReadGuard   aReadLock( m_aLock );
    Sequence< PropertyValue > aPropSeq( 2 );
    aPropSeq[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    aPropSeq[0].Value <<= m_xFrame;
    aPropSeq[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
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
        aDescriptor.WindowServiceName   =   DECLARE_ASCII("window")                                                       ;
        aDescriptor.ParentIndex         =   -1                                                                            ;
        aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >( rParent, UNO_QUERY ) ;
        aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0)                                                  ;
        aDescriptor.WindowAttributes    =   0 | css::awt::VclWindowPeerAttribute::CLIPCHILDREN                            ;

        // create a new blank container window and get access to parent container to append new created task.
        xPeer = xToolkit->createWindow( aDescriptor );

        vos::OGuard aGuard( Application::GetSolarMutex() );
        ::Color aBackgroundColor = Application::GetSettings().GetStyleSettings().GetFaceColor();
        xPeer->setBackground( aBackgroundColor.GetColor() );

        Window* pWindow = VCLUnoHelper::GetWindow( xPeer );
        if( pWindow )
            // this will keep the correct face color after a settings change
            pWindow->SetStyle( pWindow->GetStyle() | WB_3DLOOK );

    }

    return xPeer;
}

void LayoutManager::implts_updateUIElementsVisibleState( sal_Bool bSetVisible )
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    std::vector< Reference< XUIElement > > aUIElementVector;

    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        pIter->m_bMasterHide = bSetVisible;
        if ( pIter->m_xUIElement.is() )
            aUIElementVector.push_back( pIter->m_xUIElement );
    }
    aWriteLock.unlock();

    try
    {
        for ( sal_uInt32 i = 0; i < aUIElementVector.size(); i++ )
        {
            Reference< css::awt::XWindow > xWindow( aUIElementVector[i]->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
            {
                // we need VCL here to pass special flags to Show()
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if( pWindow )
                    pWindow->Show( bSetVisible, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
            }
        }
    }
    catch ( DisposedException& )
    {
    }

    doLayout();
}

void SAL_CALL LayoutManager::attachFrame( const Reference< XFrame >& xFrame )
throw (::com::sun::star::uno::RuntimeException)
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_xFrame = xFrame;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::reset()
throw (RuntimeException)
{
    implts_reset( sal_True );
}

void SAL_CALL LayoutManager::setInplaceMenuBar( sal_Int64 pInplaceMenuBarPointer )
throw (::com::sun::star::uno::RuntimeException)
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

     if ( !m_bInplaceMenuSet )
     {
        vos::OGuard aGuard( Application::GetSolarMutex() );

        // Reset old inplace menubar!
        m_pInplaceMenuBar = 0;
        if ( m_xInplaceMenuBar.is() )
            m_xInplaceMenuBar->dispose();
        m_xInplaceMenuBar.clear();
        m_bInplaceMenuSet = sal_False;

        if ( m_xFrame.is() &&
             m_xContainerWindow.is() )
        {
            m_pInplaceMenuBar = new MenuBarManager( m_xSMGR, m_xFrame, (MenuBar *)(long)pInplaceMenuBarPointer, sal_False, sal_False );

            Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
            while ( pWindow && !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();

            if ( pWindow )
            {
                SystemWindow* pSysWindow = (SystemWindow *)pWindow;
                pSysWindow->SetMenuBar( (MenuBar *)(long)pInplaceMenuBarPointer );
            }

             m_bInplaceMenuSet = sal_True;
            m_xInplaceMenuBar = Reference< XComponent >( (OWeakObject *)m_pInplaceMenuBar, UNO_QUERY );
        }

        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        implts_updateMenuBarClose();
    }
}

void SAL_CALL LayoutManager::resetInplaceMenuBar()
throw (::com::sun::star::uno::RuntimeException)
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_bInplaceMenuSet = sal_False;

    if ( m_xMenuBar.is() &&
         m_xContainerWindow.is() )
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, m_xMenuBar.get() );
        Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
            pSysWindow->SetMenuBar( (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar() );
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

//---------------------------------------------------------------------------------------------------------
// XMenuBarMergingAcceptor
//---------------------------------------------------------------------------------------------------------
sal_Bool SAL_CALL LayoutManager::setMergeMenuBar(
    const Reference< XIndexAccess >& ContainerMenuBar,
    const Reference< XDispatchProvider >& ContainerDispatchProvider,
    const Reference< XIndexAccess >& EmbedObjectMenuBar,
    const Reference< XDispatchProvider >& EmbedObjectDispatchProvider )
throw (::com::sun::star::uno::RuntimeException)
{
    return sal_False;
}

void SAL_CALL LayoutManager::removeMergedMenuBar()
throw (::com::sun::star::uno::RuntimeException)
{
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

void SAL_CALL LayoutManager::setDockingAreaAcceptor( const Reference< ::drafts::com::sun::star::ui::XDockingAreaAcceptor >& xDockingAreaAcceptor )
throw ( RuntimeException )
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( m_xDockingAreaAcceptor == xDockingAreaAcceptor )
        return;

    // Remove listener from old docking area acceptor
    if ( m_xDockingAreaAcceptor.is() )
    {
        Reference< css::awt::XWindow > xWindow( m_xDockingAreaAcceptor->getContainerWindow() );
        if ( xWindow.is() )
            xWindow->removeWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject * >( this ), UNO_QUERY ));

        m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP]->dispose();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM]->dispose();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT]->dispose();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT]->dispose();

        m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP].clear();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM].clear();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT].clear();
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT].clear();
        m_aDockingArea = css::awt::Rectangle();
    }

    // Set new docking area acceptor and add ourself as window listener on the container window.
    // Create our docking area windows which are parents for all docked windows.
    m_xDockingAreaAcceptor = xDockingAreaAcceptor;
    if ( m_xDockingAreaAcceptor.is() )
    {
        m_aDockingArea     = css::awt::Rectangle();
        m_xContainerWindow = m_xDockingAreaAcceptor->getContainerWindow();
        m_xContainerWindow->addWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));

        css::uno::Reference< css::awt::XWindowPeer > xParent( m_xContainerWindow, UNO_QUERY );
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP]    = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT]   = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
        m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT]  = Reference< css::awt::XWindow >( implts_createToolkitWindow( xParent ), UNO_QUERY );
    }
    aWriteLock.unlock();
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    implts_destroyElements(); // remove all elements
    implts_createAddonsToolBars(); // create addon toolbars
    implts_createCustomToolBars(); // create custom toolbars
    implts_sortUIElements();
    implts_doLayout( sal_True );
}

void SAL_CALL LayoutManager::createElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );
    Reference< XFrame > xFrame = m_xFrame;
    Reference< XURLTransformer > xURLTransformer = m_xURLTransformer;
    sal_Bool    bInPlaceMenu = m_bInplaceMenuSet;
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( !xFrame.is() )
        return;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    sal_Bool                                            bFound = sal_False;
    OUString                                            aElementType;
    OUString                                            aElementName;
    Reference< drafts::com::sun::star::ui::XUIElement > xUIElement;

    implts_findElement( aName, aElementType, aElementName, xUIElement );
    bFound = xUIElement.is();

    if ( xFrame.is() &&
         m_xContainerWindow.is() )
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" )
                //&& xFrame->isTop()
                )
        {
            if ( !bFound  )
            {
                xUIElement = implts_createElement( aName );
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
                    }
                    else
                    {
                        // Create new UI element and try to read its state data
                        UIElement aNewToolbar( aName, aElementType, xUIElement );
                        implts_readWindowStateData( aName, aNewToolbar );
                        implts_setElementData( aNewToolbar, xDockWindow );
                        m_aUIElements.push_back( aNewToolbar );
                    }
                }
                aWriteLock.unlock();

                implts_sortUIElements();
                doLayout();
            }
        }
        else if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            if ( aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ) && !bInPlaceMenu )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                if ( !m_xMenuBar.is() )
                    m_xMenuBar = implts_createElement( aName );

                if ( m_xMenuBar.is() && xFrame->isTop() )
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
                                Any a = xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "XMenuBar" )));
                                a >>= xMenuBar;
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
                                    if ( m_bMenuVisible )
                                        pMenuBar->SetDisplayable( sal_True );
                                    else
                                        pMenuBar->SetDisplayable( sal_False );
                                    implts_updateMenuBarClose();
                                }
                            }
                        }
                    }
                }
            }
            aWriteLock.unlock();
        }
    }

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::destroyElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    sal_Bool    bFound( sal_False );
    sal_Bool    bMustLayout( sal_False );
    OUString    aElementType;
    OUString    aElementName;

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
            }
        }
        else
        {
            UIElementVector::iterator pIter;

            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->m_aName == aName )
                {
                    xComponent = Reference< XComponent >( pIter->m_xUIElement, UNO_QUERY );
                    Reference< XUIElement > xUIElement( pIter->m_xUIElement );
                    if ( xUIElement.is() )
                    {
                        Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                        Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

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

                        bMustLayout = sal_True;
                    }
                    pIter->m_xUIElement.clear();
                    break;
                }
            }
        }
    }
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( xComponent.is() )
        xComponent->dispose();

    if ( bMustLayout )
    {
        implts_sortUIElements();
        doLayout();
    }
}

::sal_Bool SAL_CALL LayoutManager::requestElement( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    UIElementVector::iterator pIter;

    WriteGuard aWriteLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if (( pIter->m_aName == ResourceURL ) &&
            ( pIter->m_xUIElement.is() ))
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

            if ( xWindow.is() &&
                 xDockWindow.is() &&
                 !pIter->m_bMasterHide &&
                 pIter->m_bVisible )
            {
                pIter->m_bVisible = sal_True;
                aWriteLock.unlock();

                // we need VCL here to pass special flags to Show()
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                if( pWindow )
                    pWindow->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                implts_writeNewStateData( ResourceURL, xWindow );

                if ( xDockWindow.is() && !xDockWindow->isFloating() )
                    doLayout();

                return sal_True;
            }
        }
    }

    return sal_False;
}

Reference< XUIElement > SAL_CALL LayoutManager::getElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    OUString                aElementType;
    OUString                aElementName;
    Reference< XUIElement > xElement;

    implts_findElement( aName, aElementType, aElementName, xElement );
    return xElement;
}

Sequence< Reference< ::drafts::com::sun::star::ui::XUIElement > > SAL_CALL LayoutManager::getElements()
throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard   aReadLock( m_aLock );

    sal_Bool  bMenuBar( sal_False );
    sal_Int32 nSize = m_aUIElements.size();

    if ( m_xMenuBar.is() )
    {
        ++nSize;
        bMenuBar = sal_True;
    }

    Sequence< Reference< ::drafts::com::sun::star::ui::XUIElement > > aSeq( nSize );

    sal_Int32 nIndex = 0;
    UIElementVector::const_iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_xUIElement.is() )
            aSeq[nIndex++] = pIter->m_xUIElement;
    }
    if ( bMenuBar )
        aSeq[nIndex++] = m_xMenuBar;

    // Resize sequence as we now know our correct size
    aSeq.realloc( nIndex );

    return aSeq;
}

sal_Bool SAL_CALL LayoutManager::showElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    OUString    aElementType;
    OUString    aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            WriteGuard aWriteLock( m_aLock );

            if ( m_xContainerWindow.is() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                m_bMenuVisible = sal_True;
                if ( pWindow )
                {
                    MenuBar* pSetMenuBar = 0;
                    if ( m_xInplaceMenuBar.is() )
                    {
                        pSetMenuBar = (MenuBar *)m_pInplaceMenuBar->GetMenuBar();
                        ((SystemWindow *)pWindow)->SetMenuBar( pSetMenuBar );
                        pSetMenuBar->SetDisplayable( sal_True );
                        return sal_True;
                    }
                    else
                    {
                        MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, m_xMenuBar.get() );
                        if ( pMenuBarWrapper )
                        {
                            pSetMenuBar = (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar();
                            ((SystemWindow *)pWindow)->SetMenuBar( pSetMenuBar );
                            pSetMenuBar->SetDisplayable( sal_True );
                            return sal_True;
                        }
                    }
                }
            }
        }
        else
        {
            UIElementVector::iterator pIter;

            WriteGuard aWriteLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
                    Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                    if ( xWindow.is() && xDockWindow.is() && !pIter->m_bMasterHide )
                    {
                        pIter->m_bVisible = sal_True;
                        implts_sortUIElements();
                        aWriteLock.unlock();

                        // we need VCL here to pass special flags to Show()
                        vos::OGuard aGuard( Application::GetSolarMutex() );
                        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                        if( pWindow )
                            pWindow->Show( TRUE, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
                        implts_writeNewStateData( aName, xWindow );

                        if ( xDockWindow.is() && !xDockWindow->isFloating() )
                            doLayout();

                        return sal_True;
                    }
                }
            }
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::hideElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    OUString            aElementType;
    OUString            aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            WriteGuard aWriteLock( m_aLock );

            if ( m_xContainerWindow.is() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
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
                        return sal_True;
                    }
                }
            }
        }
        else
        {
            UIElementVector::iterator pIter;

            WriteGuard aWriteLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
                    Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                    if ( xWindow.is() && xDockWindow.is() )
                    {
                        pIter->m_bVisible = sal_False;
                        aWriteLock.unlock();

                        xWindow->setVisible( sal_False );
                        implts_writeNewStateData( aName, xWindow );

                        if ( xDockWindow.is() && !xDockWindow->isFloating() )
                            doLayout();

                        return sal_True;
                    }
                }
            }
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::dockWindow( const ::rtl::OUString& aName, DockingArea DockingArea, const css::awt::Point& Pos )
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
                    if ( DockingArea != DockingArea_DOCKINGAREA_DEFAULT )
                        aUIElement.m_aDockedData.m_nDockedArea = DockingArea;

                    if (( Pos.X != LONG_MAX ) && ( Pos.Y != LONG_MAX ))
                        aUIElement.m_aDockedData.m_aPos = ::Point( Pos.X, Pos.Y );

                    if ( !xDockWindow->isFloating() )
                    {
                        if (( aUIElement.m_aDockedData.m_aPos.X() == LONG_MAX ) ||
                            ( aUIElement.m_aDockedData.m_aPos.Y() == LONG_MAX ))
                        {
                            // Docking on its default position without a preset position -
                            // we have to find a good place for it.
                            ::Size      aSize;
                            Window*     pWindow( 0 );
                            ToolBox*    pToolBox( 0 );

                            {
                                vos::OGuard aGuard( Application::GetSolarMutex() );
                                pWindow = VCLUnoHelper::GetWindow( xWindow );
                                if ( pWindow && pWindow->GetType() == WINDOW_TOOLBOX )
                                    pToolBox = (ToolBox *)pWindow;

                                if ( pToolBox )
                                    aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( aUIElement.m_aDockedData.m_nDockedArea ) );
                                else
                                    aSize = pWindow->GetSizePixel();
                            }

                            ::Point aPixelPos;
                            ::Point aDockPos;
                            implts_findNextDockingPos( (drafts::com::sun::star::ui::DockingArea)aUIElement.m_aDockedData.m_nDockedArea,
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
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::dockAllWindows( ::sal_Int16 nElementType ) throw (::com::sun::star::uno::RuntimeException)
{
    if ( nElementType == UIElementType::TOOLBAR )
    {
        std::vector< rtl::OUString > aToolBarNameVector;

        OUString                  aElementType;
        OUString                  aElementName;

        {
            ReadGuard aReadLock( m_aLock );
            UIElementVector::iterator pIter;
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->m_aType.equalsAscii( "toolbar" ) &&
                     pIter->m_xUIElement.is() &&
                     pIter->m_bFloating &&
                     pIter->m_bVisible )
                    aToolBarNameVector.push_back( pIter->m_aName );
            }
        }

        for ( sal_uInt32 i = 0; i < aToolBarNameVector.size(); i++ )
        {
            ::com::sun::star::awt::Point aPoint;
            aPoint.X = aPoint.Y = LONG_MAX;
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
                Reference< css::awt::XWindow > xWindow( aUIElement.m_xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                if ( xWindow.is() && xDockWindow.is() )
                {
                    if ( aUIElement.m_bFloating )
                    {
                        xWindow->setPosSize( 0, 0, aSize.Width, aSize.Height, css::awt::PosSize::SIZE );
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

    if ( implts_findElement( aName, aUIElement ))
    {
        if ( aUIElement.m_xUIElement.is() )
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
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );

                if ( xWindow.is() && xDockWindow.is() )
                {
                    if ( aUIElement.m_bFloating )
                    {
                        xWindow->setPosSize( aPos.X, aPos.Y, aSize.Width, aSize.Height, css::awt::PosSize::POSSIZE );
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
    OUString aElementType;
    OUString aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            ReadGuard aReadLock( m_aLock );
            if ( m_xContainerWindow.is() )
            {
                aReadLock.unlock();

                vos::OGuard aGuard( Application::GetSolarMutex() );
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
                    ReadGuard aReadLock( m_aLock );
                    return m_bMenuVisible;
                }
            }
        }
        else
        {
            UIElementVector::const_iterator pIter;

            ReadGuard aReadLock( m_aLock );
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
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
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementFloating( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
            {
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xDockWindow.is() )
                    return xDockWindow->isFloating();
            }
        }
    }

    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementDocked( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
            {
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xDockWindow.is() )
                    return !xDockWindow->isFloating();
            }
        }
    }

    return sal_False;
}

::sal_Bool SAL_CALL LayoutManager::isElementLocked( const ::rtl::OUString& ResourceURL )
throw (::com::sun::star::uno::RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if (( pIter->m_aName == ResourceURL ) &&
            ( pIter->m_xUIElement.is() ))
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
            {
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xDockWindow.is() )
                    return xDockWindow->isLocked();
            }
        }
    }

    return sal_False;
}

css::awt::Size SAL_CALL LayoutManager::getElementSize( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
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
                }
                else
                    break;
            }
        }
    }

    return css::awt::Size();
}

css::awt::Point SAL_CALL LayoutManager::getElementPos( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    UIElementVector::const_iterator pIter;

    ReadGuard aReadLock( m_aLock );
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        if ( pIter->m_aName == aName && pIter->m_xUIElement.is() )
        {
            Reference< css::awt::XWindow > xWindow( pIter->m_xUIElement->getRealInterface(), UNO_QUERY );
            Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
            if ( xWindow.is() && xDockWindow.is() )
            {
                css::awt::Point aPos;
                if ( pIter->m_bFloating )
                {
                    css::awt::Rectangle aRect = xWindow->getPosSize();
                    aPos.X = aRect.X;
                    aPos.Y = aRect.Y;
                }
                else
                {
                    ::Point aVirtualPos = pIter->m_aDockedData.m_aPos;
                    aPos.X = aVirtualPos.X();
                    aPos.Y = aVirtualPos.Y();
                }

                return aPos;
            }
        }
    }

    return css::awt::Point();
}

void SAL_CALL LayoutManager::lock()
throw (RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    ++m_nLockCount;
    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::unlock()
throw (RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    --m_nLockCount;
    if ( m_nLockCount < 0 )
        m_nLockCount = 0;

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::doLayout()
throw (RuntimeException)
{
    implts_doLayout( sal_False );
}

void LayoutManager::implts_doLayout( sal_Bool bForceRequestBorderSpace )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    sal_Bool bNoLock( sal_False );
    css::awt::Rectangle aCurrBorderSpace;
    Reference< css::awt::XWindow > xContainerWindow;
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor;

    ReadGuard aReadLock( m_aLock );
    bNoLock = ( m_nLockCount == 0 );
    xContainerWindow = m_xContainerWindow;
    xDockingAreaAcceptor = m_xDockingAreaAcceptor;
    aCurrBorderSpace = m_aDockingArea;
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    if ( bNoLock &&
         xDockingAreaAcceptor.is() &&
         xContainerWindow.is() )
    {
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteGuard( m_aLock );
        m_bDoLayout = sal_True;
        aWriteGuard.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        css::awt::Rectangle     aBorderSpace = implts_calcDockingAreaSizes();
        sal_Bool                bGotRequestedBorderSpace( sal_True );
        sal_Bool                bEqual = implts_compareRectangles( aBorderSpace, aCurrBorderSpace );

        if ( !bEqual || bForceRequestBorderSpace )
        {
            if ( bGotRequestedBorderSpace = xDockingAreaAcceptor->requestDockingAreaSpace( aBorderSpace ))
            {
                xDockingAreaAcceptor->setDockingAreaSpace( aBorderSpace );

                /* SAFE AREA ----------------------------------------------------------------------------------------------- */
                aWriteGuard.lock();
                m_aDockingArea = aBorderSpace;
                aWriteGuard.unlock();
                /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            }
        }

        if ( bGotRequestedBorderSpace )
        {
            std::vector< UIElement >    aWindowVector[DOCKINGAREAS_COUNT];
            ::Size                      aContainerSize;

            implts_setDockingAreaWindowSizes( aBorderSpace );
            aContainerSize = implts_getContainerWindowOutputSize();

            // Retrieve row/column dependent data from all docked user-interface elements
            for ( sal_Int32 i = 0; i < DOCKINGAREAS_COUNT; i++ )
            {
                std::vector< SingleRowColumnWindowData > aRowColumnsWindowData;

                implts_getDockingAreaElementInfos( (DockingArea)i, aRowColumnsWindowData );

                sal_Int32 nOffset( 0 );
                for ( sal_Int32 j = 0; j < sal_Int32( aRowColumnsWindowData.size() ); j++ )
                {
                    implts_calcWindowPosSizeOnSingleRowColumn( i, nOffset, aRowColumnsWindowData[j], aContainerSize );
                    nOffset += aRowColumnsWindowData[j].nStaticSize;
                }
            }

            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
            aWriteGuard.lock();
            m_bDoLayout = sal_False;
            aWriteGuard.unlock();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        }
    }
}

sal_Bool LayoutManager::implts_compareRectangles( const css::awt::Rectangle& rRect1,
                                                  const css::awt::Rectangle& rRect2 )
{
    return (( rRect1.X == rRect2.X ) &&
            ( rRect1.Y == rRect2.Y ) &&
            ( rRect1.Width == rRect2.Width ) &&
            ( rRect1.Height == rRect2.Height ));
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
        implts_updateUIElementsVisibleState( bVisible );
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
    sal_Int32   nSpace( rRowColumnWindowData.nSpace );
    sal_Int32   nTopDockingAreaSize;
    sal_Int32   nBottomDockingAreaSize;
    sal_Int32   nContainerClientSize;

    if ( rRowColumnWindowData.aRowColumnWindows.size() == 0 )
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

    if (( nDiff < 0 ) && ( nSpace > 0 ))
    {
        // First we try to reduce the size of blank space before/behind docked windows
        sal_Int32 i = rRowColumnWindowData.aRowColumnWindowSizes.size()-1;
        while ( i >= 0 )
        {
            sal_Int32            nSpace   = rRowColumnWindowData.aRowColumnSpace[i];
            css::awt::Rectangle& rWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
            if ( nSpace >= -nDiff )
            {
                if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                    ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_Int32 j = i; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X += nDiff;
                }
                else
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_Int32 j = i; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); j++ )
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
                    for ( sal_Int32 j = i; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].X -= nSpace;
                }
                else
                {
                    // Try to move this and all user elements behind with the calculated difference
                    for ( sal_Int32 j = i; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); j++ )
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
        sal_Int32 i = sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() - 1 );
        while ( i >= 0 )
        {
            css::awt::Rectangle& rWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
            ::Size               aMinSize;

            vos::OGuard aGuard( Application::GetSolarMutex() );
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
                    for ( sal_Int32 j = i; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); j++ )
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
                    for ( sal_Int32 j = i; j < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); j++ )
                        rRowColumnWindowData.aRowColumnWindowSizes[j].Y += nDiff;
                }
            }

            if ( nDiff >= 0 )
                break;

            --i;
        }

/*
        double    fFactor( 1.0 );
        sal_Int32 nPos = 0;

        if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
            ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
            fFactor = double( nContainerClientSize ) / double( rRowColumnWindowData.nVarSize - nSpace );
        else
            fFactor = double( nContainerClientSize ) / double( rRowColumnWindowData.nVarSize - nSpace );

        sal_Int32 nCurrPos( 0 );
        for ( sal_Int32 i = 0; i < sal_Int32( rRowColumnWindowData.aRowColumnWindowSizes.size() ); i++ )
        {
            css::awt::Rectangle& rWinRect = rRowColumnWindowData.aRowColumnWindowSizes[i];
            if (( nDockingArea == DockingArea_DOCKINGAREA_TOP ) ||
                ( nDockingArea == DockingArea_DOCKINGAREA_BOTTOM ))
            {
                rWinRect.X = nCurrPos;
                rWinRect.Width = sal_Int32( double( rWinRect.Width ) * fFactor );
                nCurrPos = rWinRect.X + rWinRect.Width;
            }
            else
            {
                rWinRect.Y = nCurrPos;
                rWinRect.Height = sal_Int32( double( rWinRect.Height ) * fFactor );
                nCurrPos = rWinRect.Y + rWinRect.Height;
            }
        }
*/
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

    vos::OGuard aGuard( Application::GetSolarMutex() );
    for ( sal_Int32 i = 0; i < sal_Int32( rRowColumnWindowData.aRowColumnWindows.size() ); i++ )
    {
        Reference< css::awt::XWindow > xWindow = rRowColumnWindowData.aRowColumnWindows[i];
        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
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

        for ( pConstIter = aWindowVector.begin(); pConstIter != aWindowVector.end(); pConstIter++ )
        {
            Reference< XUIElement > xUIElement( pConstIter->m_xUIElement, UNO_QUERY );
            if ( xUIElement.is() )
            {
                Reference< css::awt::XWindow > xWindow( xUIElement->getRealInterface(), UNO_QUERY );
                Reference< css::awt::XDockableWindow > xDockWindow( xWindow, UNO_QUERY );
                if ( xWindow.is() && xDockWindow.is() )
                {
                    vos::OGuard aGuard( Application::GetSolarMutex() );

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
        if ( aWindowVector.size() > 0 )
        {
            for ( sal_Int32 i = 0; i <= DockingArea_DOCKINGAREA_RIGHT; i++ )
            {
                sal_Int32 nSize( 0 );
                for ( sal_Int32 j = 0; j < sal_Int32( aRowColumnSizes[i].size() ); j++ )
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

    sal_Int32 nLeftRightDockingAreaHeight( aContainerClientSize.Height );
    if ( rBorderSpace.Y >= 0 )
    {
        // Top docking area window
        ReadGuard aReadLock( m_aLock );
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP] );
        aReadLock.unlock();

        xDockAreaWindow->setPosSize( 0, 0, aContainerClientSize.Width, rBorderSpace.Y, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
        nLeftRightDockingAreaHeight -= rBorderSpace.Y;
    }
    if ( rBorderSpace.Height >= 0 )
    {
        // Bottom docking area window
        sal_Int32 nBottomPos = std::max( sal_Int32( aContainerClientSize.Height - rBorderSpace.Height), sal_Int32( 0 ));
        sal_Int32 nHeight = ( nBottomPos == 0 ) ? 0 : rBorderSpace.Height;

        ReadGuard aReadLock( m_aLock );
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] );
        aReadLock.unlock();

        xDockAreaWindow->setPosSize( 0, nBottomPos, aContainerClientSize.Width, nHeight, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
        nLeftRightDockingAreaHeight -= nHeight;
    }
    if ( rBorderSpace.X >= 0 || nLeftRightDockingAreaHeight > 0 )
    {
        // Left docking area window
        ReadGuard aReadLock( m_aLock );
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT] );
        aReadLock.unlock();

        // We also have to change our right docking area window if the top or bottom area has changed. They have a higher priority!
        sal_Int32 nHeight = std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Height - rBorderSpace.Y - rBorderSpace.Height ));

        xDockAreaWindow->setPosSize( 0, rBorderSpace.Y, rBorderSpace.X, nHeight, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
    }
    if ( rBorderSpace.Width >= 0 || nLeftRightDockingAreaHeight > 0 )
    {
        // Right docking area window
        ReadGuard aReadLock( m_aLock );
        Reference< css::awt::XWindow > xDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT] );
        aReadLock.unlock();

        // We also have to change our right docking area window if the top or bottom area has changed. They have a higher priority!
        sal_Int32 nLeftPos  = std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Width - rBorderSpace.Width ));
        sal_Int32 nHeight   = std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Height - rBorderSpace.Y - rBorderSpace.Height ));
        sal_Int32 nWidth    = ( nLeftPos == 0 ) ? 0 : rBorderSpace.Width;

        xDockAreaWindow->setPosSize( nLeftPos, rBorderSpace.Y, nWidth, nHeight, css::awt::PosSize::POSSIZE );
        xDockAreaWindow->setVisible( sal_True );
    }
}

//---------------------------------------------------------------------------------------------------------
//  XMenuCloser
//---------------------------------------------------------------------------------------------------------
void LayoutManager::implts_updateMenuBarClose()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    Reference <XStatusListener > xListener( m_xMenuBarCloseListener );
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aWriteLock.unlock();

    if ( xContainerWindow.is() )
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );

        Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
            MenuBar* pMenuBar = pSysWindow->GetMenuBar();
            if ( pMenuBar )
            {
                if ( xListener.is() )
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


void LayoutManager::implts_setMenuBarCloser( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener >& StatusListener )
{
    Reference< XEventListener > xOldListener;
    Reference< XLayoutManager > xThis;

    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    xOldListener = Reference< XEventListener >( m_xMenuBarCloseListener, UNO_QUERY );
    m_xMenuBarCloseListener = StatusListener;
    xThis = Reference< XLayoutManager >( static_cast< OWeakObject* >( this ), UNO_QUERY );
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aWriteLock.unlock();

    if ( xContainerWindow.is() )
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );

        Window* pWindow = VCLUnoHelper::GetWindow( m_xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
            MenuBar* pMenuBar = pSysWindow->GetMenuBar();
            if ( pMenuBar )
            {
                pMenuBar->ShowCloser( TRUE );
                pMenuBar->SetCloserHdl( LINK( this, LayoutManager, MenuBarClose ));
            }
        }
    }

    css::lang::EventObject aEvent;
    aEvent.Source = xThis;
    if ( xOldListener.is() )
        xOldListener->disposing( aEvent );
}

void LayoutManager::implts_clearMenuBarCloser()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_xMenuBarCloseListener = Reference< XStatusListener >();
    Reference< css::awt::XWindow > xContainerWindow( m_xContainerWindow );
    aWriteLock.unlock();

    if ( xContainerWindow.is() )
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            SystemWindow* pSysWindow = (SystemWindow *)pWindow;
            MenuBar* pMenuBar = pSysWindow->GetMenuBar();
            if ( pMenuBar )
            {
                pMenuBar->SetCloserHdl( Link() );
                pMenuBar->ShowCloser( FALSE );
            }
        }
    }
}

sal_Int16 LayoutManager::implts_getCurrentSymbolSet()
{
    sal_Int16 eOptSymbolSet( 0 );
    ReadGuard aReadLock( m_aLock );
    if ( m_pMiscOptions )
        eOptSymbolSet = m_pMiscOptions->GetSymbolSet();
    aReadLock.unlock();

    if ( eOptSymbolSet == SFX_SYMBOLS_AUTO )
    {
        // Use system settings, we have to retrieve the toolbar icon size from the
        // Application class
        vos::OGuard aGuard( Application::GetSolarMutex() );
        ULONG nStyleIconSize = Application::GetSettings().GetStyleSettings().GetToolbarIconSize();
        if ( nStyleIconSize == STYLE_TOOLBAR_ICONSIZE_LARGE )
            eOptSymbolSet = SFX_SYMBOLS_LARGE;
        else
            eOptSymbolSet = SFX_SYMBOLS_SMALL;
    }

    return eOptSymbolSet;
}

IMPL_LINK( LayoutManager, MenuBarClose, MenuBar *, pMenu )
{
    ReadGuard aReadLock( m_aLock );
    Reference< XStatusListener > xMenuBarCloseListener = m_xMenuBarCloseListener;
    aReadLock.unlock();

    if ( xMenuBarCloseListener.is() )
    {
        Reference< XLayoutManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );

        FeatureStateEvent aEvent;
        aEvent.Source = xThis;
        xMenuBarCloseListener->statusChanged( aEvent );
    }

    return 0;
}

IMPL_LINK( LayoutManager, OptionsChanged, void*, pVoid )
{
    sal_Int16 eOldSymbolSet( 0 );
    sal_Int16 eSymbolSet( implts_getCurrentSymbolSet() );

    ReadGuard aReadLock( m_aLock );
    eOldSymbolSet = m_eSymbolSet;
    aReadLock.unlock();

    if ( eSymbolSet != eOldSymbolSet )
    {
        WriteGuard aWriteLock( m_aLock );
        m_eSymbolSet = eSymbolSet;
        aWriteLock.unlock();

        std::vector< Reference< XUpdatable > > aToolBarVector;

        ReadGuard aReadLock( m_aLock );
        {
            UIElementVector::iterator pIter;
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->m_xUIElement.is() )
                    aToolBarVector.push_back( Reference< XUpdatable >( pIter->m_xUIElement, UNO_QUERY ));
            }
        }
        aReadLock.unlock();

        lock();
        {
            std::vector< Reference< XUpdatable > >::iterator pIter;
            for ( pIter = aToolBarVector.begin(); pIter != aToolBarVector.end(); pIter++ )
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

IMPL_LINK( LayoutManager, SettingsChanged, void*, pVoid )
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
    Reference< css::awt::XWindow > xWindow( e.Source, UNO_QUERY );
    aReadGuard.unlock();

    Window* pContainerWindow( 0 );
    Window* pWindow( 0 );
    ::Point aMousePos;
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
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
            aUIElement.m_aFloatingData.m_aPos    = ::Point( e.TrackingRectangle.X, e.TrackingRectangle.Y );
            aUIElement.m_aFloatingData.m_aSize   = ::Size( e.TrackingRectangle.Width, e.TrackingRectangle.Height );

            vos::OGuard aGuard( Application::GetSolarMutex() );
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
        }
    }

    if ( bDockingInProgress &&
         xDockWindow.is() &&
         xWindow.is() )
    {
        try
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );

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
            Window* pDockingAreaWindow( 0 );
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
                    pDockingAreaWindow = VCLUnoHelper::GetWindow( xTopDockingWindow );
                }
                else if ( eDockingArea == DockingArea_DOCKINGAREA_BOTTOM )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_BOTTOM;
                    aUIDockingElement.m_bFloating = sal_False;
                    pDockingAreaWindow = VCLUnoHelper::GetWindow( xBottomDockingWindow );
                }
                else if ( eDockingArea == DockingArea_DOCKINGAREA_LEFT )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_LEFT;
                    aUIDockingElement.m_bFloating = sal_False;
                    pDockingAreaWindow = VCLUnoHelper::GetWindow( xLeftDockingWindow );
                }
                else if ( eDockingArea == DockingArea_DOCKINGAREA_RIGHT )
                {
                    aUIDockingElement.m_aDockedData.m_nDockedArea = DockingArea_DOCKINGAREA_RIGHT;
                    aUIDockingElement.m_bFloating = sal_False;
                    pDockingAreaWindow = VCLUnoHelper::GetWindow( xRightDockingWindow );
                }

                ::Point aOutputPos = pContainerWindow->ScreenToOutputPixel( aTrackingRect.TopLeft() );
                aTrackingRect.SetPos( aOutputPos );

                ::Rectangle aNewDockingRect = implts_calcDockingPosSize( aUIDockingElement, aTrackingRect, aMousePos );

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
                m_aDockUIElement.m_aDockedData   = aUIDockingElement.m_aDockedData;
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
    sal_Bool    bMustLayout( sal_False );
    sal_Bool    bStartDockFloated( sal_False );
    sal_Bool    bFloating( sal_False );
    UIElement   aUIDockingElement;

    WriteGuard aWriteLock( m_aLock );
    bDockingInProgress = m_bDockingInProgress;
    aUIDockingElement = m_aDockUIElement;
    bFloating = aUIDockingElement.m_bFloating;

    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        Reference< XUIElement > xUIElement( pIter->m_xUIElement, UNO_QUERY );
        if ( xUIElement.is() )
        {
            Reference< XInterface > xIfac( xUIElement->getRealInterface(), UNO_QUERY );
            if ( xIfac == e.Source )
            {
                if ( m_aDockUIElement.m_bFloating )
                {
                    // Write last position into position data
                    vos::OGuard aGuard( Application::GetSolarMutex() );
                    Window* pContainerWindow( VCLUnoHelper::GetWindow( m_xContainerWindow ) );
                    Reference< css::awt::XWindow > xWindow( xIfac, UNO_QUERY );
                    pIter->m_aFloatingData = m_aDockUIElement.m_aFloatingData;
                    css::awt::Rectangle aTmpRect = xWindow->getPosSize();
                    pIter->m_aFloatingData.m_aPos = ::Point( aTmpRect.X, aTmpRect.Y );
                }
                else
                {
                    pIter->m_aDockedData = m_aDockUIElement.m_aDockedData;
                    pIter->m_aFloatingData.m_aSize = m_aDockUIElement.m_aFloatingData.m_aSize;
                }

                bStartDockFloated = pIter->m_bFloating;
                pIter->m_bFloating = m_aDockUIElement.m_bFloating;
                pIter->m_bUserActive = sal_True;
                break;
            }
        }
    }
    m_aDockUIElement.m_xUIElement.clear();
    aWriteLock.unlock();

    implts_writeWindowStateData( aUIDockingElement.m_aName, aUIDockingElement );

    if ( bDockingInProgress )
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
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
                pToolBox->SetSizePixel( aSize );
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
                    vos::OGuard aGuard( Application::GetSolarMutex() );
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow )
                    {
                        if ( pWindow->GetType() == WINDOW_TOOLBOX )
                        {
                            ToolBox* pToolBox = (ToolBox *)pWindow;
                            aUIDockingElement.m_aFloatingData.m_aPos            = pToolBox->GetPosPixel();
                            aUIDockingElement.m_aFloatingData.m_aSize           = pToolBox->GetSizePixel();
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
    Reference< css::awt::XWindow > xWindow;

    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        xWindow = Reference< css::awt::XWindow >( e.Source, UNO_QUERY );
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
                vos::OGuard aGuard( Application::GetSolarMutex() );
                if ( pToolBox )
                {
                    pToolBox->SetLineCount( aUIDockingElement.m_aFloatingData.m_nLines );
                    if ( aUIDockingElement.m_aFloatingData.m_bIsHorizontal )
                        pToolBox->SetAlign( WINDOWALIGN_TOP );
                    else
                        pToolBox->SetAlign( WINDOWALIGN_LEFT );
                }

                sal_Bool bUndefPos = ( aUIDockingElement.m_aFloatingData.m_aPos.X() == LONG_MAX ||
                                       aUIDockingElement.m_aFloatingData.m_aPos.Y() == LONG_MAX );
                sal_Bool bSetSize = ( aUIDockingElement.m_aFloatingData.m_aSize.Width() != 0 &&
                                      aUIDockingElement.m_aFloatingData.m_aSize.Height() != 0 );

                if ( bUndefPos )
                    aUIDockingElement.m_aFloatingData.m_aPos = implts_findNextCascadeFloatingPos();

                if ( !bSetSize )
                {
                    if ( pToolBox )
                        aUIDockingElement.m_aFloatingData.m_aSize = pToolBox->CalcWindowSizePixel();
                    else
                        aUIDockingElement.m_aFloatingData.m_aSize = pWindow->GetSizePixel();
                }

                xWindow->setPosSize( aUIDockingElement.m_aFloatingData.m_aPos.X(),
                                     aUIDockingElement.m_aFloatingData.m_aPos.Y(),
                                     aUIDockingElement.m_aFloatingData.m_aSize.Width(),
                                     aUIDockingElement.m_aFloatingData.m_aSize.Height(),
                                     css::awt::PosSize::POSSIZE );
            }
            else
            {
                if (( aUIDockingElement.m_aDockedData.m_aPos.X() == LONG_MAX ) &&
                    ( aUIDockingElement.m_aDockedData.m_aPos.Y() == LONG_MAX ))
                {
                    // Docking on its default position without a preset position -
                    // we have to find a good place for it.
                    ::Point aPixelPos;
                    ::Point aDockPos;
                    ::Size  aSize;

                    {
                        vos::OGuard aGuard( Application::GetSolarMutex() );
                        if ( pToolBox )
                            aSize = pToolBox->CalcWindowSizePixel( 1, ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea ) );
                        else
                            aSize = pWindow->GetSizePixel();
                    }

                    implts_findNextDockingPos( (drafts::com::sun::star::ui::DockingArea)aUIDockingElement.m_aDockedData.m_nDockedArea,
                                                aSize,
                                                aDockPos,
                                                aPixelPos );
                    aUIDockingElement.m_aDockedData.m_aPos = aDockPos;
                }

                vos::OGuard aGuard( Application::GetSolarMutex() );
                if ( pToolBox )
                {
                    pToolBox->SetAlign( ImplConvertAlignment( aUIDockingElement.m_aDockedData.m_nDockedArea) );
                    ::Size aSize = pToolBox->CalcWindowSizePixel( 1 );
                    css::awt::Rectangle aRect = xWindow->getPosSize();
                    xWindow->setPosSize( aRect.X, aRect.Y, aSize.Width(), aSize.Height(), css::awt::PosSize::POSSIZE );
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
        vos::OGuard aGuard( Application::GetSolarMutex() );
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
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        Reference< XUIElement > xUIElement( pIter->m_xUIElement );
        if ( xUIElement.is() )
        {
            Reference< XInterface > xIfac( xUIElement->getRealInterface(), UNO_QUERY );
            if ( xIfac == e.Source )
            {
                aName = pIter->m_aName;
                pIter->m_bVisible = sal_False; // user close => make invisible!
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

void SAL_CALL LayoutManager::endPopupMode( const ::com::sun::star::awt::EndPopupModeEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{

}


//---------------------------------------------------------------------------------------------------------
//  XWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::windowResized( const css::awt::WindowEvent& aEvent )
throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Request to set docking area space again.
    css::awt::Rectangle               aDockingArea( m_aDockingArea );
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    Reference< css::awt::XWindow >    xContainerWindow( m_xContainerWindow );

    Reference< XInterface > xIfac( xContainerWindow, UNO_QUERY );
    if ( xIfac == aEvent.Source )
    {
        // We have to call our resize handler at least once synchronously, as some
        // application modules need this. So we have to check if this is the first
        // call after the async layout time expired.
        if ( !m_aAsyncLayoutTimer.IsActive() )
        {
            const Link& aLink = m_aAsyncLayoutTimer.GetTimeoutHdl();
            if ( aLink.IsSet() )
                aLink.Call( &m_aAsyncLayoutTimer );
        }
        m_aAsyncLayoutTimer.Start();
    }
    else
    {
        sal_Bool bLocked( m_bDockingInProgress );
        sal_Bool bDoLayout( m_bDoLayout );
        aReadLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        if ( !bLocked && !bDoLayout )
        {
            // Do not do anything if we are in the middle of a docking process. This would interfere all other
            // operations. We will store the new position and size in the docking handlers.
            // Do not do anything if we are in the middle of our layouting process. We will adopt the position
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

void SAL_CALL LayoutManager::windowMoved( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException )
{
}

void SAL_CALL LayoutManager::windowShown( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
}

void SAL_CALL LayoutManager::windowHidden( const css::lang::EventObject& aEvent ) throw( css::uno::RuntimeException )
{
}

IMPL_LINK( LayoutManager, AsyncLayoutHdl, Timer *, pTimer )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    // Request to set docking area space again.
    m_aAsyncLayoutTimer.Stop();

    if( !m_xContainerWindow.is() )
        return 0;

    css::awt::Rectangle               aDockingArea( m_aDockingArea );
    Reference< XDockingAreaAcceptor > xDockingAreaAcceptor( m_xDockingAreaAcceptor );
    Reference< css::awt::XWindow >    xContainerWindow( m_xContainerWindow );

    css::uno::Reference< css::awt::XDevice > xDevice( m_xContainerWindow, css::uno::UNO_QUERY );

    // Convert relativ size to output size.
    css::awt::Rectangle  aRectangle     = m_xContainerWindow->getPosSize();
    css::awt::DeviceInfo aInfo          = xDevice->getInfo();
    css::awt::Size aContainerClientSize = css::awt::Size( aRectangle.Width - aInfo.LeftInset - aInfo.RightInset  ,
                                                          aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

    // copy docking area windows
    Reference< css::awt::XWindow > xTopDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_TOP] );
    Reference< css::awt::XWindow > xBottomDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_BOTTOM] );
    Reference< css::awt::XWindow > xLeftDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_LEFT] );
    Reference< css::awt::XWindow > xRightDockAreaWindow( m_xDockAreaWindows[DockingArea_DOCKINGAREA_RIGHT] );
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    sal_Int32 nTopBottomDockAreaHeight( 0 );
    css::awt::Rectangle aTopRect = xTopDockAreaWindow->getPosSize();
    xTopDockAreaWindow->setPosSize( 0,
                                    0,
                                    aContainerClientSize.Width,
                                    aTopRect.Height,
                                    css::awt::PosSize::POSSIZE );
    nTopBottomDockAreaHeight += aTopRect.Height;

    css::awt::Rectangle aBottomRect = xBottomDockAreaWindow->getPosSize();
    xBottomDockAreaWindow->setPosSize( 0,
                                        std::max( sal_Int32( aContainerClientSize.Height - aBottomRect.Height), sal_Int32( 0 )),
                                        aContainerClientSize.Width,
                                        aBottomRect.Height,
                                        css::awt::PosSize::POSSIZE );
    nTopBottomDockAreaHeight += aBottomRect.Height;

    css::awt::Rectangle aRect = xLeftDockAreaWindow->getPosSize();
    xLeftDockAreaWindow->setPosSize( 0,
                                        aTopRect.Height,
                                        aRect.Width,
                                        std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Height - nTopBottomDockAreaHeight )),
                                        css::awt::PosSize::POSSIZE );

    aRect = xRightDockAreaWindow->getPosSize();
    xRightDockAreaWindow->setPosSize( std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Width - aRect.Width )),
                                        aTopRect.Height,
                                        aRect.Width,
                                        std::max( sal_Int32( 0 ), sal_Int32( aContainerClientSize.Height - nTopBottomDockAreaHeight )),
                                        css::awt::PosSize::POSSIZE );

    implts_doLayout( sal_True );
    return 0;
}

//---------------------------------------------------------------------------------------------------------
//  XFrameActionListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::frameAction( const FrameActionEvent& aEvent )
throw ( RuntimeException )
{
    if (( aEvent.Action == FrameAction_COMPONENT_ATTACHED   ) ||
        ( aEvent.Action == FrameAction_COMPONENT_REATTACHED ))
    {
        implts_reset( sal_True );
        implts_doLayout( sal_True );
    }
    else if (( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED      ) ||
             ( aEvent.Action == FrameAction_FRAME_UI_DEACTIVATING   ))
    {
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_bActive = ( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

        implts_toggleFloatingUIElementsVisibility( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
        doLayout();
    }
    else if ( aEvent.Action == FrameAction_COMPONENT_DETACHING )
    {
        implts_reset( sal_False );
    }
}

// ______________________________________________

void SAL_CALL LayoutManager::disposing( const css::lang::EventObject& aEvent )
throw( RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    if ( aEvent.Source == Reference< XInterface >( m_xFrame, UNO_QUERY ))
    {
        // Our frame gets disposed, release all our references that depends on a working
        // frame reference.
        Application::RemoveEventListener( LINK( this, LayoutManager, SettingsChanged ) );
        if ( m_pMiscOptions )
        {
            m_pMiscOptions->RemoveListener( LINK( this, LayoutManager, OptionsChanged ) );
            delete m_pMiscOptions;
            m_pMiscOptions = 0;
        }

        delete m_pAddonOptions;
        m_pAddonOptions = 0;

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
    }
    else if ( aEvent.Source == Reference< XInterface >( m_xContainerWindow, UNO_QUERY ))
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
    }
    else if ( aEvent.Source == Reference< XInterface >( m_xDocCfgMgr, UNO_QUERY ))
    {
        m_xDocCfgMgr.clear();
    }
    else if ( aEvent.Source == Reference< XInterface >( m_xModuleCfgMgr , UNO_QUERY ))
    {
        m_xModuleCfgMgr.clear();
    }

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::elementInserted( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );

    OUString                aElementType;
    OUString                aElementName;
    Reference< XUIElement > xUIElement;
    Reference< XFrame >     xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        implts_findElement( Event.ResourceURL, aElementType, aElementName, xUIElement );

        Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
        if ( xElementSettings.is() )
        {
            OUString aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
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
                    OUString                             aUIName;

                    try
                    {
                        xCfgMgr  = Reference< XUIConfigurationManager >( Event.Source, UNO_QUERY );
                        xPropSet = Reference< XPropertySet >( xCfgMgr->getSettings( Event.ResourceURL, sal_False ), UNO_QUERY );

                        if ( xPropSet.is() )
                            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "UIName" ))) >>= aUIName;
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
                        vos::OGuard aGuard( Application::GetSolarMutex() );
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

void SAL_CALL LayoutManager::elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    OUString                aElementType;
    OUString                aElementName;
    Reference< XUIElement > xUIElement;
    Reference< XFrame >     xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        implts_findElement( Event.ResourceURL, aElementType, aElementName, xUIElement );

        Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
        if ( xElementSettings.is() )
        {
            bool                        bNoSettings( false );
            OUString                    aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
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

void SAL_CALL LayoutManager::elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );

    OUString                aElementType;
    OUString                aElementName;
    Reference< XUIElement > xUIElement;
    Reference< XFrame >     xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        implts_findElement( Event.ResourceURL, aElementType, aElementName, xUIElement );

        Reference< XUIElementSettings > xElementSettings( xUIElement, UNO_QUERY );
        if ( xElementSettings.is() )
        {
            OUString                    aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
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
    //  Initialize state with FALSE !!!
    //  (Handle can be invalid)
    sal_Bool bReturn = sal_False;

    switch( nHandle )
    {
        case LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER :
            bReturn = PropHelper::willPropertyBeChanged(
                        com::sun::star::uno::makeAny(m_xMenuBarCloseListener),
                        aValue,
                        aOldValue,
                        aConvertedValue);
                break;
    }

    // Return state of operation.
    return bReturn ;
}

void SAL_CALL LayoutManager::setFastPropertyValue_NoBroadcast( sal_Int32                       nHandle ,
                                                               const com::sun::star::uno::Any& aValue  ) throw( com::sun::star::uno::Exception )
{
    switch( nHandle )
    {
        case LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER:
        {
            Reference< XStatusListener > xListener;
            aValue >>= xListener;
            if ( xListener.is() )
                implts_setMenuBarCloser( xListener );
            else
                implts_clearMenuBarCloser();
            break;
        }
    }
}

void SAL_CALL LayoutManager::getFastPropertyValue( com::sun::star::uno::Any& aValue  ,
                                                   sal_Int32                 nHandle    ) const
{
    switch( nHandle )
    {
        case LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER:
            aValue <<= m_xMenuBarCloseListener;
            break;
    }
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
            // "impl_getStaticPropertyDescriptor" is a non exported and static funtion, who will define a static propertytable.
            // "sal_True" say: Table is sorted by name.
            static ::cppu::OPropertyArrayHelper aInfoHelper( impl_getStaticPropertyDescriptor(), sal_True );
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

const com::sun::star::uno::Sequence< com::sun::star::beans::Property > LayoutManager::impl_getStaticPropertyDescriptor()
{
    // Create a new static property array to initialize sequence!
    // Table of all predefined properties of this class. Its used from OPropertySetHelper-class!
    // Don't forget to change the defines (see begin of this file), if you add, change or delete a property in this list!!!
    // It's necessary for methods of OPropertySetHelper.
    // ATTENTION:
    //      YOU MUST SORT FOLLOW TABLE BY NAME ALPHABETICAL !!!

    static const com::sun::star::beans::Property pProperties[] =
    {
        com::sun::star::beans::Property( LAYOUTMANAGER_PROPNAME_MENUBARCLOSER, LAYOUTMANAGER_PROPHANDLE_MENUBARCLOSER, ::getCppuType((const Reference< XStatusListener >*)NULL), com::sun::star::beans::PropertyAttribute::TRANSIENT  )
    };
    // Use it to initialize sequence!
    static const com::sun::star::uno::Sequence< com::sun::star::beans::Property > lPropertyDescriptor( pProperties, LAYOUTMANAGER_PROPCOUNT );
    // Return static "PropertyDescriptor"
    return lPropertyDescriptor;
}

}; // namespace framework
