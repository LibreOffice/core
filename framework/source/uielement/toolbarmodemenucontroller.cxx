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

#include <uielement/toolbarmodemenucontroller.hxx>

#include "services.h"
#include <framework/sfxhelperfunctions.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/ui/UIElementType.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>

//  Defines

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::ui;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   ToolbarModeMenuController                  ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_TOOLBARMODEMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   ToolbarModeMenuController, {} )

ToolbarModeMenuController::ToolbarModeMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_xContext( xContext )
{
}

ToolbarModeMenuController::~ToolbarModeMenuController()
{
}

void ToolbarModeMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    if( SvtMiscOptions().DisableUICustomization() )
        return;

    SolarMutexGuard aSolarMutexGuard;
    resetPopupMenu( rPopupMenu );

    m_xPopupMenu->insertItem( 1, "Default", css::awt::MenuItemStyle::RADIOCHECK, m_xPopupMenu->getItemCount() );
    rPopupMenu->setCommand( 1, ".uno:ToolbarMode?Mode:string=Default" );
    m_xPopupMenu->insertItem( 2, "Single Toolbar", css::awt::MenuItemStyle::RADIOCHECK, m_xPopupMenu->getItemCount() );
    rPopupMenu->setCommand( 2, ".uno:ToolbarMode?Mode:string=Single" );
    m_xPopupMenu->insertItem( 3, "Sidebar", css::awt::MenuItemStyle::RADIOCHECK, m_xPopupMenu->getItemCount() );
    rPopupMenu->setCommand( 3, ".uno:ToolbarMode?Mode:string=Sidebar" );
    m_xPopupMenu->insertItem( 4, "Notebookbar", css::awt::MenuItemStyle::RADIOCHECK, m_xPopupMenu->getItemCount() );
    rPopupMenu->setCommand( 4, ".uno:ToolbarMode?Mode:string=Notebookbar" );
}

// XEventListener
void SAL_CALL ToolbarModeMenuController::disposing( const EventObject& ) throw ( RuntimeException, std::exception )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL ToolbarModeMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException, std::exception )
{
    OUString aFeatureURL( Event.FeatureURL.Complete );

    // All other status events will be processed here
    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< css::awt::XPopupMenu > xPopupMenu( m_xPopupMenu );
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        SolarMutexGuard aGuard;
        VCLXPopupMenu* pXPopupMenu = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( xPopupMenu ));
        PopupMenu*     pVCLPopupMenu = pXPopupMenu ? static_cast<PopupMenu *>(pXPopupMenu->GetMenu()) : nullptr;

        SAL_WARN_IF(!pVCLPopupMenu, "fwk.uielement", "worrying lack of popup menu");
        if (!pVCLPopupMenu)
            return;

        bool bSetCheckmark      = false;
        bool bCheckmark         = false;
        for ( sal_uInt16 i = 0; i < pVCLPopupMenu->GetItemCount(); i++ )
        {
            sal_uInt16 nId = pVCLPopupMenu->GetItemId( i );
            if ( nId == 0 )
                continue;

            OUString aCmd = pVCLPopupMenu->GetItemCommand( nId );
            if ( aCmd == aFeatureURL )
            {
                // Enable/disable item
                pVCLPopupMenu->EnableItem( nId, Event.IsEnabled );

                // Checkmark
                if ( Event.State >>= bCheckmark )
                    bSetCheckmark = true;

                if ( bSetCheckmark )
                    pVCLPopupMenu->CheckItem( nId, bCheckmark );
                else
                {
                    OUString aItemText;

                    if ( Event.State >>= aItemText )
                        pVCLPopupMenu->SetItemText( nId, aItemText );
                }
            }
        }
    }
}

// XMenuListener
void SAL_CALL ToolbarModeMenuController::itemSelected( const css::awt::MenuEvent& rEvent ) throw (RuntimeException, std::exception)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XURLTransformer >        xURLTransformer;
    Reference< XFrame >                 xFrame;
    Reference< XNameAccess >            xPersistentWindowState;

    osl::ClearableMutexGuard aLock( m_aMutex );
    xPopupMenu             = m_xPopupMenu;
    xURLTransformer        = m_xURLTransformer;
    xFrame                 = m_xFrame;
    aLock.clear();

    if ( xPopupMenu.is() )
    {
        VCLXPopupMenu* pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXPopupMenu::GetImplementation( xPopupMenu ));
        if ( pPopupMenu )
        {
            SolarMutexGuard aSolarMutexGuard;
            PopupMenu* pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

            OUString aCmd( pVCLPopupMenu->GetItemCommand( rEvent.MenuId ));
            URL aTargetURL;
            Sequence<PropertyValue> aArgs;

            aTargetURL.Complete = aCmd;
            xURLTransformer->parseStrict( aTargetURL );
            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( xDispatchProvider.is() )
            {
                Reference< XDispatch > xDispatch = xDispatchProvider->queryDispatch(
                                                        aTargetURL, OUString(), 0 );

                ExecuteInfo* pExecuteInfo = new ExecuteInfo;
                pExecuteInfo->xDispatch     = xDispatch;
                pExecuteInfo->aTargetURL    = aTargetURL;
                pExecuteInfo->aArgs         = aArgs;
                Application::PostUserEvent( LINK(nullptr, ToolbarModeMenuController, ExecuteHdl_Impl), pExecuteInfo );
            }
        }
    }
}

void SAL_CALL ToolbarModeMenuController::itemActivated( const css::awt::MenuEvent& ) throw (RuntimeException, std::exception)
{
    OUString aMode = officecfg::Office::UI::ToolbarMode::Active::get( m_xContext );

    m_xPopupMenu->checkItem( 1, (aMode.compareTo("Default") == 0) );
    m_xPopupMenu->checkItem( 2, (aMode.compareTo("Single") == 0) );
    m_xPopupMenu->checkItem( 3, (aMode.compareTo("Sidebar") == 0) );
    m_xPopupMenu->checkItem( 4, (aMode.compareTo("Notebookbar") == 0) );
}

// XPopupMenuController
void SAL_CALL ToolbarModeMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException, std::exception )
{
    osl::MutexGuard aLock( m_aMutex );

    throwIfDisposed();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        SolarMutexGuard aSolarMutexGuard;

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( static_cast<OWeakObject*>(this), UNO_QUERY ));
        fillPopupMenu( m_xPopupMenu );
    }
}

IMPL_STATIC_LINK_TYPED( ToolbarModeMenuController, ExecuteHdl_Impl, void*, p, void )
{
    ExecuteInfo* pExecuteInfo = static_cast<ExecuteInfo*>(p);
    try
    {
        // Asynchronous execution as this can lead to our own destruction!
        // Framework can recycle our current frame and the layout manager disposes all user interface
        // elements if a component gets detached from its frame!
        if ( pExecuteInfo->xDispatch.is() )
        {
            pExecuteInfo->xDispatch->dispatch( pExecuteInfo->aTargetURL, pExecuteInfo->aArgs );
        }
    }
    catch ( const Exception& )
    {
    }

    delete pExecuteInfo;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
