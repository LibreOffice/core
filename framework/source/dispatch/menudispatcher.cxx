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

#include <dispatch/menudispatcher.hxx>
#include <general.h>
#include <framework/menuconfiguration.hxx>
#include <framework/addonmenu.hxx>
#include <services.h>

#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/window.hxx>
#include <vcl/syswin.hxx>
#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <tools/rcid.h>
#include <osl/mutex.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <ucbhelper/content.hxx>

namespace framework{

using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::cppu;

const sal_uInt16 SLOTID_MDIWINDOWLIST = 5610;

//  constructor

MenuDispatcher::MenuDispatcher(   const   uno::Reference< XComponentContext >&  xContext    ,
                                  const   uno::Reference< XFrame >&             xOwner      )
        :   m_xOwnerWeak            ( xOwner                         )
        ,   m_xContext              ( xContext                       )
        ,   m_aListenerContainer    ( m_mutex )
        ,   m_bAlreadyDisposed      ( false                      )
        ,   m_bActivateListener     ( false                      )
        ,   m_pMenuManager          ( NULL                           )
{
    // Safe impossible cases
    // We need valid information about our owner for work.
    SAL_WARN_IF( !( xContext.is() && xOwner.is() ), "fwk", "MenuDispatcher::MenuDispatcher()\nInvalid parameter detected!" );

    m_bActivateListener = true;
    xOwner->addFrameActionListener( uno::Reference< XFrameActionListener >( static_cast<OWeakObject *>(this), UNO_QUERY ));
}

//  destructor

MenuDispatcher::~MenuDispatcher()
{
    // Warn programmer if he forgot to dispose this instance.
    // We must release all our references ...
    // and a dtor isn't the best place to do that!
}

//  XDispatch

void SAL_CALL MenuDispatcher::dispatch(    const   URL&                        /*aURL*/            ,
                                            const   Sequence< PropertyValue >&  /*seqProperties*/   ) throw( RuntimeException, std::exception )
{
}

//  XDispatch

void SAL_CALL MenuDispatcher::addStatusListener(   const   uno::Reference< XStatusListener >&   xControl,
                                                    const   URL&                            aURL    ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard g;
    // Safe impossible cases
    // Method not defined for all incoming parameter
    SAL_WARN_IF( !xControl.is() || aURL.Complete.isEmpty(), "fwk", "MenuDispatcher::addStatusListener(): Invalid parameter detected." );
    // Add listener to container.
    m_aListenerContainer.addInterface( aURL.Complete, xControl );
}

//  XDispatch

void SAL_CALL MenuDispatcher::removeStatusListener(    const   uno::Reference< XStatusListener >&   xControl,
                                                        const   URL&                            aURL    ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard g;
    // Safe impossible cases
    // Method not defined for all incoming parameter
    SAL_WARN_IF( !xControl.is() || aURL.Complete.isEmpty(), "fwk", "MenuDispatcher::removeStatusListener(): Invalid parameter detected." );
    // Add listener to container.
    m_aListenerContainer.removeInterface( aURL.Complete, xControl );
}

//   XFrameActionListener

void SAL_CALL MenuDispatcher::frameAction( const FrameActionEvent& aEvent ) throw ( RuntimeException, std::exception )
{
    SolarMutexResettableGuard aGuard;

    if ( m_pMenuManager && aEvent.Action == FrameAction_FRAME_UI_ACTIVATED )
    {
        MenuBar* pMenuBar = static_cast<MenuBar *>(m_pMenuManager->GetMenu());
        uno::Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
        aGuard.clear();

        if ( xFrame.is() && pMenuBar )
        {
            uno::Reference< css::awt::XWindow >xContainerWindow = xFrame->getContainerWindow();

            aGuard.reset();
            {
                vcl::Window* pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
                while ( pWindow && !pWindow->IsSystemWindow() )
                    pWindow = pWindow->GetParent();

                if ( pWindow )
                {
                    SystemWindow* pSysWindow = static_cast<SystemWindow *>(pWindow);
                    pSysWindow->SetMenuBar( pMenuBar );
                }
            }
        }
    }
    else if ( m_pMenuManager && aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING )
    {
        if ( m_pMenuManager )
            impl_setMenuBar( NULL );
    }
}

//   XEventListener
void SAL_CALL MenuDispatcher::disposing( const EventObject& ) throw( RuntimeException, std::exception )
{
    SolarMutexGuard g;
    // Safe impossible cases
    SAL_WARN_IF( m_bAlreadyDisposed, "fwk", "MenuDispatcher::disposing(): Object already disposed .. don't call it again!" );

    if( !m_bAlreadyDisposed )
    {
        m_bAlreadyDisposed = true;

        if ( m_bActivateListener )
        {
            uno::Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
            if ( xFrame.is() )
            {
                xFrame->removeFrameActionListener( uno::Reference< XFrameActionListener >( static_cast<OWeakObject *>(this), UNO_QUERY ));
                m_bActivateListener = false;
                if ( m_pMenuManager )
                {
                    EventObject aEventObj;
                    aEventObj.Source = xFrame;
                    m_pMenuManager->disposing( aEventObj );
                }
            }
        }

        // Forget our factory.
        m_xContext = uno::Reference< XComponentContext >();

        // Remove our menu from system window if it is still there!
        if ( m_pMenuManager )
            impl_setMenuBar( NULL );
    }
}

void MenuDispatcher::impl_setAccelerators( Menu* pMenu, const Accelerator& aAccel )
{
    for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); ++nPos )
    {
        sal_uInt16     nId    = pMenu->GetItemId(nPos);
        ::PopupMenu* pPopup = pMenu->GetPopupMenu(nId);
        if ( pPopup )
            impl_setAccelerators( static_cast<Menu *>(pPopup), aAccel );
        else if ( nId && !pMenu->GetPopupMenu(nId))
        {
            vcl::KeyCode aCode = aAccel.GetKeyCode( nId );
            if ( aCode.GetCode() )
                pMenu->SetAccelKey( nId, aCode );
        }
    }
}

bool MenuDispatcher::impl_setMenuBar( MenuBar* pMenuBar, bool bMenuFromResource )
{
    uno::Reference< XFrame > xFrame( m_xOwnerWeak.get(), UNO_QUERY );
    if ( xFrame.is() )
    {
        uno::Reference< css::awt::XWindow >xContainerWindow = xFrame->getContainerWindow();
        vcl::Window* pWindow = NULL;

        SolarMutexGuard aSolarGuard;
        {
            pWindow = VCLUnoHelper::GetWindow( xContainerWindow );
            while ( pWindow && !pWindow->IsSystemWindow() )
                pWindow = pWindow->GetParent();
        }

        if ( pWindow )
        {
            SystemWindow* pSysWindow = static_cast<SystemWindow *>(pWindow);

            if ( m_pMenuManager )
            {
                // remove old menu from our system window if it was set before
                if ( m_pMenuManager->GetMenu() == static_cast<Menu *>(pSysWindow->GetMenuBar()) )
                    pSysWindow->SetMenuBar( NULL );

                // remove listener before we destruct ourself, so we cannot be called back afterwards
                m_pMenuManager->RemoveListener();

                (static_cast< css::uno::XInterface* >(static_cast<OWeakObject*>(m_pMenuManager)))->release();

                m_pMenuManager = 0;
            }

            if ( pMenuBar != NULL )
            {
                sal_uInt16 nPos = pMenuBar->GetItemPos( SLOTID_MDIWINDOWLIST );
                if ( nPos != MENU_ITEM_NOTFOUND )
                {
                    uno::Reference< XModel >            xModel;
                    uno::Reference< XController >   xController( xFrame->getController(), UNO_QUERY );

                    if ( xController.is() )
                        xModel = uno::Reference< XModel >( xController->getModel(), UNO_QUERY );

                    // retrieve addon popup menus and add them to our menu bar
                    AddonMenuManager::MergeAddonPopupMenus( xFrame, nPos, pMenuBar, m_xContext );

                    // retrieve addon help menu items and add them to our help menu
                    AddonMenuManager::MergeAddonHelpMenu( xFrame, pMenuBar, m_xContext );
                }

                // set new menu on our system window and create new menu manager
                if ( bMenuFromResource )
                {
                    m_pMenuManager = new MenuManager( m_xContext, xFrame, pMenuBar, true, false );
                }
                else
                {
                    m_pMenuManager = new MenuManager( m_xContext, xFrame, pMenuBar, true, true );
                }

                pSysWindow->SetMenuBar( pMenuBar );
            }

            return true;
        }
    }

    return false;
}

}       //  namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
