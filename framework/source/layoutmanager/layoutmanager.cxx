/*************************************************************************
 *
 *  $RCSfile: layoutmanager.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-10 13:21:34 $
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
#ifndef _DRAFTS_COM_SUN_STAR_UI_XMODULEUICONFIGURATIONMANAGER_HPP_
#include <drafts/com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_UIELEMENTTYPE_HPP_
#include <drafts/com/sun/star/ui/UIElementType.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

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

// ______________________________________________
//  using namespace

using namespace rtl;
using namespace com::sun::star::awt;
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

//_________________________________________________________________________________________________________________
//  Namespace
//_________________________________________________________________________________________________________________
//

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XINTERFACE_12                    (   LayoutManager                                                                    ,
                                            OWeakObject                                                                     ,
                                            DIRECT_INTERFACE( css::lang::XTypeProvider                                      ),
                                            DIRECT_INTERFACE( css::lang::XServiceInfo                                       ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::frame::XLayoutManager                 ),
                                            DIRECT_INTERFACE( css::awt::XWindowListener                                     ),
                                            DIRECT_INTERFACE( css::frame::XFrameActionListener                              ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::ui::XUIConfigurationListener          ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::frame::XInplaceLayout                 ),
                                            DIRECT_INTERFACE( drafts::com::sun::star::frame::XMenuBarMergingAcceptor        ),
                                            DERIVED_INTERFACE( css::lang::XEventListener, css::frame::XFrameActionListener  ),
                                            DIRECT_INTERFACE( ::com::sun::star::beans::XMultiPropertySet                    ),
                                            DIRECT_INTERFACE( ::com::sun::star::beans::XFastPropertySet                     ),
                                            DIRECT_INTERFACE( ::com::sun::star::beans::XPropertySet                         )
                                        )

DEFINE_XTYPEPROVIDER_12                 (   LayoutManager                                           ,
                                            css::lang::XTypeProvider                                ,
                                            css::lang::XServiceInfo                                 ,
                                            drafts::com::sun::star::frame::XLayoutManager           ,
                                            css::awt::XWindowListener                               ,
                                            css::frame::XFrameActionListener                        ,
                                            css::lang::XEventListener                               ,
                                            drafts::com::sun::star::ui::XUIConfigurationListener    ,
                                            drafts::com::sun::star::frame::XInplaceLayout           ,
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
            ThreadHelpBase()
        ,   ::cppu::OBroadcastHelperVar< ::cppu::OMultiTypeInterfaceContainerHelper, ::cppu::OMultiTypeInterfaceContainerHelper::keyType >( m_aLock.getShareableOslMutex()         )
        ,   ::cppu::OPropertySetHelper  ( *(static_cast< ::cppu::OBroadcastHelper* >(this)) )
        ,   ::cppu::OWeakObject         (                                                   )
        ,   m_xSMGR( xServiceManager )
        ,   m_xURLTransformer( Reference< XURLTransformer >( xServiceManager->createInstance(
                                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                             UNO_QUERY ))
        ,   m_nLockCount( 0 )
        ,   m_bActive( sal_False )
        ,   m_bVisible( sal_True )
        ,   m_bInplaceMenuSet( sal_False )
        ,   m_bMenuVisible( sal_True )
        ,   m_xModuleManager( Reference< XModuleManager >(
                xServiceManager->createInstance( SERVICENAME_MODULEMANAGER ), UNO_QUERY ))
        ,   m_xUIElementFactoryManager( Reference< ::drafts::com::sun::star::ui::XUIElementFactory >(
                xServiceManager->createInstance( SERVICENAME_UIELEMENTFACTORYMANAGER ), UNO_QUERY ))
{
}

LayoutManager::~LayoutManager()
{
    WriteGuard aWriteLock( m_aLock );
    impl_clearUpMenuBar();
}

// Internal helper function
void LayoutManager::impl_clearUpMenuBar()
{
    vos::OGuard aGuard( Application::GetSolarMutex() );

    // Clear up VCL menu bar to prepare shutdown
    if ( m_xFrame.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
        while ( pWindow && !pWindow->IsSystemWindow() )
            pWindow = pWindow->GetParent();

        if ( pWindow )
        {
            MenuBar* pSetMenuBar = 0;
            if ( m_xInplaceMenuBar.is() )
                pSetMenuBar = (MenuBar *)m_pInplaceMenuBar->GetMenuBar();
            else
            {
                Reference< XMenuBar > xMenuBar;

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
    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    Reference< XModel > xModel;
    if ( xFrame.is() )
    {
        if ( bAttached )
        {
            OUString aModuleIdentifier;
            try
            {
                aModuleIdentifier = m_xModuleManager->identify( Reference< XInterface >( xFrame, UNO_QUERY ) );
            }
            catch( Exception& )
            {
            }

            if ( aModuleIdentifier.getLength() )
            {
                Reference< ::drafts::com::sun::star::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(
                    m_xSMGR->createInstance( SERVICENAME_MODULEUICONFIGURATIONMANAGERSUPPLIER ), UNO_QUERY );

                WriteGuard aWriteLock( m_aLock );
                if ( m_xModuleCfgMgr.is() )
                {
                    // Remove listener to old ui configuration manager
                    try
                    {
                        Reference< XUIConfiguration > xModuleCfg( m_xModuleCfgMgr, UNO_QUERY );
                        xModuleCfg->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                    }
                    catch ( Exception& )
                    {
                    }
                }

                try
                {
                    m_xModuleCfgMgr = xModuleCfgSupplier->getUIConfigurationManager( aModuleIdentifier );
                    Reference< XUIConfiguration > xModuleCfg( m_xModuleCfgMgr, UNO_QUERY );
                    xModuleCfg->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                }
                catch ( Exception& )
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
                    WriteGuard aWriteLock( m_aLock );
                    if ( m_xDocCfgMgr.is() )
                    {
                        // Remove listener to old ui configuration manager
                        try
                        {
                            Reference< XUIConfiguration > xDocCfg( m_xDocCfgMgr, UNO_QUERY );
                            xDocCfg->removeConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                        }
                        catch ( Exception& )
                        {
                        }
                    }

                    try
                    {
                        m_xDocCfgMgr = xUIConfigurationManagerSupplier->getUIConfigurationManager();
                        Reference< XUIConfiguration > xDocCfg( m_xDocCfgMgr, UNO_QUERY );
                        xDocCfg->addConfigurationListener( Reference< XUIConfigurationListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));
                    }
                    catch ( Exception& )
                    {
                    }
                }
            }
        }

        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_xModel = xModel;
        m_aUIElements.clear();
        impl_clearUpMenuBar();
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    }
}

void LayoutManager::implts_updateUIElementsVisibleState( sal_Bool bSetVisivble )
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    std::vector< Reference< XUIElement > > aUIElementVector;

    WriteGuard aWriteLock( m_aLock );
    UIElementVector::iterator pIter;
    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
    {
        pIter->bMasterHide = bSetVisivble;
        aUIElementVector.push_back( pIter->xUIElement );
    }
    aWriteLock.unlock();

    try
    {
        for ( sal_uInt32 i = 0; i < aUIElementVector.size(); i++ )
        {
            Reference< XWindow > xWindow( aUIElementVector[i]->getRealInterface(), UNO_QUERY );
            if ( xWindow.is() )
                xWindow->setVisible( bSetVisivble );
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

        if ( m_xFrame.is() )
        {
            // #110897# m_pInplaceMenuBar = new MenuBarManager( m_xFrame, (MenuBar *)(long)pInplaceMenuBarPointer, sal_False, sal_False );
            m_pInplaceMenuBar = new MenuBarManager( m_xSMGR, m_xFrame, (MenuBar *)(long)pInplaceMenuBarPointer, sal_False, sal_False );

            Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
            while ( pWindow && !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();

            if ( pWindow )
            {
                SystemWindow* pSysWindow = (SystemWindow *)pWindow;
                pSysWindow->SetMenuBar( (MenuBar *)(long)pInplaceMenuBarPointer );
            }

             m_bInplaceMenuSet = sal_True;
            m_xInplaceMenuBar = Reference< XComponent >( (OWeakObject *)m_pInplaceMenuBar, UNO_QUERY );

            aWriteLock.unlock();
            /* SAFE AREA ----------------------------------------------------------------------------------------------- */

            implts_updateMenuBarClose();
        }
    }
}

void SAL_CALL LayoutManager::resetInplaceMenuBar()
throw (::com::sun::star::uno::RuntimeException)
{
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    m_bInplaceMenuSet = sal_False;

    if ( m_xMenuBar.is() &&
         m_xFrame.is() )
    {
        vos::OGuard aGuard( Application::GetSolarMutex() );
        MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, m_xMenuBar.get() );
        Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
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

    // Remove listener from old docking area acceptor
    if ( m_xDockingAreaAcceptor.is() )
    {
        Reference< css::awt::XWindow > xWindow( m_xDockingAreaAcceptor->getContainerWindow() );
        if ( xWindow.is() )
            xWindow->removeWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject * >( this ), UNO_QUERY ));
        m_xTopDockingWindow.clear();
        m_aUIElements.clear(); // remove all elements
    }

    // Set new docking area acceptor and add ourself as window listener on the container window.
    m_xDockingAreaAcceptor = xDockingAreaAcceptor;
    if ( m_xDockingAreaAcceptor.is() )
    {
        m_aDockingArea     = css::awt::Rectangle();
        m_xContainerWindow = m_xDockingAreaAcceptor->getContainerWindow();
        m_xContainerWindow->addWindowListener( Reference< css::awt::XWindowListener >( static_cast< OWeakObject* >( this ), UNO_QUERY ));

        css::uno::Reference< css::awt::XToolkit > xToolkit( m_xSMGR->createInstance( SERVICENAME_VCLTOOLKIT ), css::uno::UNO_QUERY );
        if ( xToolkit.is() )
        {
            // describe window properties.
            css::awt::WindowDescriptor aDescriptor;
            aDescriptor.Type                =   css::awt::WindowClass_SIMPLE                                                  ;
            aDescriptor.WindowServiceName   =   DECLARE_ASCII("window")                                                       ;
            aDescriptor.ParentIndex         =   -1                                                                            ;
            aDescriptor.Parent              =   css::uno::Reference< css::awt::XWindowPeer >( m_xContainerWindow, UNO_QUERY ) ;
            aDescriptor.Bounds              =   css::awt::Rectangle(0,0,0,0)                                                  ;
            aDescriptor.WindowAttributes    =   0 | css::awt::VclWindowPeerAttribute::CLIPCHILDREN                            ;

            // create a new blank container window and get access to parent container to append new created task.
            css::uno::Reference< css::awt::XWindowPeer > xPeer      = xToolkit->createWindow( aDescriptor );
            m_xTopDockingWindow = css::uno::Reference< css::awt::XWindow >( xPeer, css::uno::UNO_QUERY );
        }
    }
    aWriteLock.unlock();
     /* SAFE AREA ----------------------------------------------------------------------------------------------- */

    doLayout();
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

    sal_Bool                        bFound = sal_False;
    sal_Int32                       nIndex = 0;
    OUString                        aElementType;
    OUString                        aElementName;
    UIElementVector::const_iterator pIter;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
        {
            if ( pIter->aName == aElementName )
            {
                bFound = sal_True;
                break;
            }
        }
    }

/*
    if ( !bFound && m_xTopDockingWindow.is() )
    {
        // TODO: move some parts of this code to the UI element factory!!
        Reference< drafts::com::sun::star::ui::XUIElement > xUIElement;
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "toolbar" ))
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );
            xUIElement = CreateToolBox( xFrame, m_xTopDockingWindow, aElementName );    // Use sfx2 code to create toolbar
        }

        if ( xUIElement.is() )
        {
            // Create toolbar element
            Any             a;
            rtl::OUString   aElementName;
            rtl::OUString   aElementType;
            Reference< XPropertySet > xPropSet( xUIElement, UNO_QUERY );

            a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" )));
            a >>= aElementName;
            a = xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Type" )));
            a >>= aElementType;

            m_aUIElements.push_back( UIElement( aElementName, aElementType, xUIElement ));
        }
    }
*/

    if ( xFrame.is() )
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
            aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
            !bInPlaceMenu )
        {
            vos::OGuard aGuard( Application::GetSolarMutex() );
            if ( !m_xMenuBar.is() )
            {
                Sequence< PropertyValue > aPropSeq( 2 );
                aPropSeq[0].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
                aPropSeq[0].Value <<= xFrame;
                aPropSeq[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Persistent" ));
                aPropSeq[1].Value <<= sal_True;

                try
                {
                    m_xMenuBar = m_xUIElementFactoryManager->createUIElement( aName, aPropSeq );
                }
                catch ( NoSuchElementException& )
                {
                }
                catch ( IllegalArgumentException& )
                {
                }
            }

            if ( xFrame->isTop() )
            {
                Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                if ( pWindow )
                {
                    SystemWindow* pSysWindow = (SystemWindow *)pWindow;
                    Reference< XMenuBar > xMenuBar;

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
    }

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

void SAL_CALL LayoutManager::destroyElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    sal_Bool    bFound = sal_False;
    OUString    aElementType;
    OUString    aElementName;

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
                if ( pIter->aName == aElementName )
                {
                    Reference< XComponent > xComponent( pIter->xUIElement->getRealInterface(), UNO_QUERY );
                    m_aUIElements.erase( pIter );
                    bFound = sal_True;
                    break;
                }
            }
        }
    }

    aWriteLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
}

Reference< XUIElement > SAL_CALL LayoutManager::getElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    ReadGuard   aReadLock( m_aLock );

    sal_Int32   nIndex = 0;
    OUString    aElementType;
    OUString    aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            return m_xMenuBar;
        }
        else
        {
            UIElementVector::const_iterator pIter;

            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->aName == aElementName )
                    return pIter->xUIElement;
            }
        }
    }

    return Reference< XUIElement >();
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
        aSeq[nIndex++] = pIter->xUIElement;
    if ( bMenuBar )
        aSeq[nIndex] = m_xMenuBar;

    return aSeq;
}

sal_Bool SAL_CALL LayoutManager::showElement( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    OUString    aElementType;
    OUString    aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            if ( m_xFrame.is() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
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
                    }
                    else
                    {
                        MenuBarWrapper* pMenuBarWrapper = SAL_STATIC_CAST( MenuBarWrapper*, m_xMenuBar.get() );
                        if ( pMenuBarWrapper )
                        {
                            pSetMenuBar = (MenuBar *)pMenuBarWrapper->GetMenuBarManager()->GetMenuBar();
                            ((SystemWindow *)pWindow)->SetMenuBar( pSetMenuBar );
                            pSetMenuBar->SetDisplayable( sal_True );
                        }
                    }
                }
            }
        }
        else
        {
            UIElementVector::const_iterator pIter;

            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->aName == aName )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->xUIElement->getRealInterface(), UNO_QUERY );
                    if ( xWindow.is() && !pIter->bMasterHide )
                    {
                        xWindow->setVisible( sal_True );
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
    ReadGuard aReadLock( m_aLock );

    OUString            aElementType;
    OUString            aElementName;

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            if ( m_xFrame.is() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                m_bMenuVisible = sal_False;
                if ( pWindow )
                {
                    MenuBar* pMenuBar = ((SystemWindow *)pWindow)->GetMenuBar();
                    if ( pMenuBar )
                        pMenuBar->SetDisplayable( sal_False );
                }
            }
        }
        else
        {
            UIElementVector::const_iterator pIter;

            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->aName == aName )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->xUIElement->getRealInterface(), UNO_QUERY );
                    if ( xWindow.is() )
                    {
                        xWindow->setVisible( sal_False );
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
    return sal_True;
}

sal_Bool SAL_CALL LayoutManager::floatWindow( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    return sal_True;
}

void SAL_CALL LayoutManager::setElementSize( const ::rtl::OUString& aName, const css::awt::Size& aSize )
throw (RuntimeException)
{
}

void SAL_CALL LayoutManager::setElementPos( const ::rtl::OUString& aName, const css::awt::Point& aPos )
throw (RuntimeException)
{
}

void SAL_CALL LayoutManager::setElementPosSize( const ::rtl::OUString& aName, const css::awt::Point& aPos, const css::awt::Size& aSize )
throw (RuntimeException)
{
}

sal_Bool SAL_CALL LayoutManager::isElementVisible( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    ReadGuard aReadLock( m_aLock );

    OUString            aElementType;
    OUString            aElementName;
    Reference< XFrame > xFrame( m_xFrame );

    if ( impl_parseResourceURL( aName, aElementType, aElementName ))
    {
        if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
             aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
        {
            if ( m_xFrame.is() )
            {
                vos::OGuard aGuard( Application::GetSolarMutex() );
                Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                if ( pWindow )
                {
                    MenuBar* pMenuBar = ((SystemWindow *)pWindow)->GetMenuBar();
                    if ( pMenuBar && pMenuBar->IsDisplayable() )
                        return sal_True;
                }
                else
                    return m_bMenuVisible;
            }
        }
        else
        {
            UIElementVector::const_iterator pIter;

            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                if ( pIter->aName == aName )
                {
                    Reference< css::awt::XWindow > xWindow( pIter->xUIElement->getRealInterface(), UNO_QUERY );
                    if ( xWindow.is() )
                    {
                        Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                        if ( pWindow && pWindow->IsVisible() )
                            return sal_True;
                        else
                            return sal_True;
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
    return sal_False;
}

sal_Bool SAL_CALL LayoutManager::isElementDocked( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    return sal_False;
}

css::awt::Size SAL_CALL LayoutManager::getElementSize( const ::rtl::OUString& aName )
throw (RuntimeException)
{
    return css::awt::Size();
}

css::awt::Point SAL_CALL LayoutManager::getElementPos( const ::rtl::OUString& aName )
throw (RuntimeException)
{
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
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    ReadGuard aReadLock( m_aLock );

    if ( m_nLockCount == 0 )
    {
        if ( m_xDockingAreaAcceptor.is() &&
             m_xContainerWindow.is()        )
        {
            sal_Int32 nDockingHeight = 0;
            UIElementVector::const_iterator pIter;
            for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
            {
                Reference< css::awt::XWindow > xWindow( pIter->xUIElement->getRealInterface(), UNO_QUERY );
                if ( xWindow.is() )
                {
                    Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                    if ( pWindow && pWindow->IsVisible() )
                    {
                        css::awt::Rectangle aPosSize = xWindow->getPosSize();
                        nDockingHeight += aPosSize.Height;
                    }
                }
            }

            css::awt::Rectangle aBorderSpace;
            aBorderSpace.X      = 0;
            aBorderSpace.Y      = nDockingHeight;
            aBorderSpace.Width  = 0;
            aBorderSpace.Height = 0;

            if ( m_xDockingAreaAcceptor->requestDockingAreaSpace( aBorderSpace ))
            {
                m_xDockingAreaAcceptor->setDockingAreaSpace( aBorderSpace );
                m_aDockingArea = aBorderSpace;

                if ( m_xTopDockingWindow.is()       )
                {
                    css::awt::Size aTopDockingAreaSize;
                    if ( nDockingHeight > 0 )
                    {
                        css::uno::Reference< css::awt::XDevice > xDevice( m_xContainerWindow, css::uno::UNO_QUERY );
                        // Convert relativ size to output size.
                        css::awt::Rectangle  aRectangle  = m_xContainerWindow->getPosSize();
                        css::awt::DeviceInfo aInfo       = xDevice->getInfo();
                        aTopDockingAreaSize = css::awt::Size(  aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                            aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

                        m_xTopDockingWindow->setPosSize( 0, 0, aTopDockingAreaSize.Width, nDockingHeight, css::awt::PosSize::POSSIZE );
                        m_xTopDockingWindow->setVisible( sal_True );
                    }
                    else
                        m_xTopDockingWindow->setVisible( sal_False );

                    sal_Int32 nPosY = 0;
                    for ( pIter = m_aUIElements.begin(); pIter != m_aUIElements.end(); pIter++ )
                    {
                        Reference< css::awt::XWindow > xWindow( pIter->xUIElement->getRealInterface(), UNO_QUERY );
                        if ( xWindow.is() )
                        {
                            Window* pWindow = VCLUnoHelper::GetWindow( xWindow );
                            if ( pWindow && pWindow->IsVisible() )
                            {
                                css::awt::Rectangle aPosSize = xWindow->getPosSize();
                                xWindow->setPosSize( 0, nPosY, aTopDockingAreaSize.Width, aPosSize.Height, css::awt::PosSize::POSSIZE );
                                nPosY += aPosSize.Height;
                            }
                        }
                    }
                }
            }
        }
    }

    aReadLock.unlock();
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
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

//---------------------------------------------------------------------------------------------------------
//  XMenuCloser
//---------------------------------------------------------------------------------------------------------
void LayoutManager::implts_updateMenuBarClose()
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );
    Reference< XFrame > xFrame( m_xFrame );
    Reference <XStatusListener > xListener( m_xMenuBarCloseListener );
    aWriteLock.unlock();

    ReadGuard aReadLock( m_aLock );

    if ( xFrame.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
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
    aWriteLock.unlock();

    ReadGuard aReadLock( m_aLock );

    if ( m_xFrame.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
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
    aWriteLock.unlock();

    ReadGuard aReadLock( m_aLock );

    if ( m_xFrame.is() )
    {
        Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
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

IMPL_LINK( LayoutManager, MenuBarClose, MenuBar *, pMenu )
{
    ReadGuard aReadLock( m_aLock );
    if ( m_xMenuBarCloseListener.is() )
    {
        Reference< XLayoutManager > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );

        FeatureStateEvent aEvent;
        aEvent.Source = xThis;
        m_xMenuBarCloseListener->statusChanged( aEvent );
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------
//  XWindowListener
//---------------------------------------------------------------------------------------------------------
void SAL_CALL LayoutManager::windowResized( const css::awt::WindowEvent& aEvent ) throw( css::uno::RuntimeException )
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    // Request to set docking area space again.
    if ( m_xDockingAreaAcceptor.is() )
        m_xDockingAreaAcceptor->setDockingAreaSpace( m_aDockingArea );
    if ( m_xContainerWindow.is() && m_xTopDockingWindow.is() )
    {
        css::uno::Reference< css::awt::XDevice > xDevice( m_xContainerWindow, css::uno::UNO_QUERY );
        // Convert relativ size to output size.
        css::awt::Rectangle  aRectangle     = m_xContainerWindow->getPosSize();
        css::awt::Rectangle  aTopDockRect   = m_xTopDockingWindow->getPosSize();
        css::awt::DeviceInfo aInfo          = xDevice->getInfo();
        css::awt::Size aContainerClientSize = css::awt::Size( aRectangle.Width  - aInfo.LeftInset - aInfo.RightInset  ,
                                                              aRectangle.Height - aInfo.TopInset  - aInfo.BottomInset );

        m_xTopDockingWindow->setPosSize( 0, 0, aContainerClientSize.Width, aTopDockRect.Height, css::awt::PosSize::SIZE );
        aWriteLock.unlock();

        doLayout();
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
        doLayout();
    }
    else if (( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED      ) ||
             ( aEvent.Action == FrameAction_FRAME_UI_DEACTIVATING   ))
    {
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */
        WriteGuard aWriteLock( m_aLock );
        m_bActive = ( aEvent.Action == FrameAction_FRAME_UI_ACTIVATED );
        aWriteLock.unlock();
        /* SAFE AREA ----------------------------------------------------------------------------------------------- */

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

    OUString            aElementType;
    OUString            aElementName;
    Reference< XFrame > xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        if ( impl_parseResourceURL( Event.ResourceURL, aElementType, aElementName ))
        {
            if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
                 aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
            {
                Reference< XUIElementSettings > xElementSettings( m_xMenuBar, UNO_QUERY );
                if ( xElementSettings.is() )
                {
                    OUString aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
                    Reference< XPropertySet > xPropSet( m_xMenuBar, UNO_QUERY );
                    if ( xPropSet.is() )
                    {
                        if ( Event.Source == Reference< XInterface >( m_xDocCfgMgr, UNO_QUERY ))
                            xPropSet->setPropertyValue( aConfigSourcePropName, makeAny( m_xDocCfgMgr ));
                    }
                    xElementSettings->updateSettings();
                }
            }
        }
    }
}

void SAL_CALL LayoutManager::elementRemoved( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    /* SAFE AREA ----------------------------------------------------------------------------------------------- */
    WriteGuard aWriteLock( m_aLock );

    OUString            aElementType;
    OUString            aElementName;
    Reference< XFrame > xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        if ( impl_parseResourceURL( Event.ResourceURL, aElementType, aElementName ))
        {
            if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
                 aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
            {
                Reference< XUIElementSettings > xElementSettings( m_xMenuBar, UNO_QUERY );
                bool bNoSettings( false );

                if ( xElementSettings.is() )
                {
                    OUString                    aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
                    Reference< XInterface >     xElementCfgMgr;
                    Reference< XPropertySet >   xPropSet( m_xMenuBar, UNO_QUERY );

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

                    // Clear up VCL menu bar to prepare shutdown
                    if ( m_xFrame.is() && bNoSettings )
                    {
                        Window* pWindow = VCLUnoHelper::GetWindow( m_xFrame->getContainerWindow() );
                        while ( pWindow && !pWindow->IsSystemWindow() )
                            pWindow = pWindow->GetParent();

                        if ( pWindow && !m_bInplaceMenuSet )
                            ((SystemWindow *)pWindow)->SetMenuBar( 0 );

                        Reference< XComponent > xComp( m_xMenuBar, UNO_QUERY );
                        if ( xComp.is() )
                            xComp->dispose();
                        m_xMenuBar.clear();
                    }
                }
            }
        }
    }
}

void SAL_CALL LayoutManager::elementReplaced( const ::drafts::com::sun::star::ui::ConfigurationEvent& Event ) throw (::com::sun::star::uno::RuntimeException)
{
    ReadGuard aReadLock( m_aLock );

    OUString            aElementType;
    OUString            aElementName;
    Reference< XFrame > xFrame( m_xFrame );

    if ( m_xFrame.is() )
    {
        if ( impl_parseResourceURL( Event.ResourceURL, aElementType, aElementName ))
        {
            if ( aElementType.equalsIgnoreAsciiCaseAscii( "menubar" ) &&
                 aElementName.equalsIgnoreAsciiCaseAscii( "menubar" ))
            {
                Reference< XUIElementSettings > xElementSettings( m_xMenuBar, UNO_QUERY );
                if ( xElementSettings.is() )
                {
                    OUString                    aConfigSourcePropName( RTL_CONSTASCII_USTRINGPARAM( "ConfigurationSource" ));
                    Reference< XInterface >     xElementCfgMgr;
                    Reference< XPropertySet >   xPropSet( m_xMenuBar, UNO_QUERY );

                    if ( xPropSet.is() )
                        xPropSet->getPropertyValue( aConfigSourcePropName ) >>= xElementCfgMgr;

                    if ( !xElementCfgMgr.is() )
                        return;

                    // Check if the same UI configuration manager has changed => update settings
                    if ( Event.Source == xElementCfgMgr )
                        xElementSettings->updateSettings();
                }
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
