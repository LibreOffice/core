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

#include <uielement/notebookbarmenucontroller.hxx>

#include "services.h"
#include <framework/sfxhelperfunctions.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <officecfg/Office/UI/Notebookbar.hxx>
#include <officecfg/Office/UI/ToolbarMode.hxx>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <svtools/menuoptions.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/confignode.hxx>

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

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   NotebookbarMenuController                  ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_NOTEBOOKBARMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   NotebookbarMenuController, {} )

NotebookbarMenuController::NotebookbarMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext ),
    m_xContext( xContext )
{
}

NotebookbarMenuController::~NotebookbarMenuController()
{
}

void NotebookbarMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    if ( SvtMiscOptions().DisableUICustomization() )
        return;

    SolarMutexGuard aSolarMutexGuard;
    resetPopupMenu( rPopupMenu );

    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( m_xContext );
    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(m_xFrame));

    OUStringBuffer aPath("org.openoffice.Office.UI.Notebookbar/Applications/");
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            aPath.append("Writer");
            break;
        case vcl::EnumContext::Application::Calc:
            aPath.append("Calc");
            break;
        case vcl::EnumContext::Application::Impress:
            aPath.append("Impress");
            break;
        default:
            break;
    }
    aPath.append("/Implementations");

    const utl::OConfigurationTreeRoot aImplementationsNode(
                                        m_xContext,
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aImplementationsNode.isValid() )
        return;

    const Sequence<OUString> aImplNodeNames (aImplementationsNode.getNodeNames());
    const sal_Int32 nCount(aImplNodeNames.getLength());

    for ( sal_Int32 nReadIndex = 0; nReadIndex < nCount; ++nReadIndex )
    {
        const utl::OConfigurationNode aImplNode(aImplementationsNode.openNode(aImplNodeNames[nReadIndex]));
        if ( !aImplNode.isValid() )
            continue;

        OUString aLabel = comphelper::getString( aImplNode.getNodeValue( "Label" ) );
        OUString aFile = comphelper::getString( aImplNode.getNodeValue( "File" ) );
        m_xPopupMenu->insertItem( nReadIndex+1, aLabel, css::awt::MenuItemStyle::RADIOCHECK, m_xPopupMenu->getItemCount() );
        rPopupMenu->setCommand( nReadIndex+1, aFile );
    }
}

// XEventListener
void SAL_CALL NotebookbarMenuController::disposing( const EventObject& )
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
void SAL_CALL NotebookbarMenuController::statusChanged( const FeatureStateEvent& Event )
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
void SAL_CALL NotebookbarMenuController::itemSelected( const css::awt::MenuEvent& rEvent )
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XURLTransformer >        xURLTransformer;
    Reference< XFrame >                 xFrame;

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

            OUStringBuffer aBuf(".uno:Notebookbar?File:string=");
            aBuf.append( aCmd );
            aTargetURL.Complete = aBuf.makeStringAndClear();

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
                Application::PostUserEvent( LINK(nullptr, NotebookbarMenuController, ExecuteHdl_Impl), pExecuteInfo );
            }
        }
    }
}

void SAL_CALL NotebookbarMenuController::itemActivated( const css::awt::MenuEvent& )
{
    OUString aActive;

    const Reference<frame::XModuleManager> xModuleManager  = frame::ModuleManager::create( m_xContext );
    vcl::EnumContext::Application eApp = vcl::EnumContext::GetApplicationEnum(xModuleManager->identify(m_xFrame));

    OUStringBuffer aPath("org.openoffice.Office.UI.ToolbarMode/Applications/");
    switch ( eApp )
    {
        case vcl::EnumContext::Application::Writer:
            aPath.append("Writer");
            aActive = officecfg::Office::UI::Notebookbar::ActiveWriter::get( m_xContext );
            break;
        case vcl::EnumContext::Application::Calc:
            aPath.append("Calc");
            aActive = officecfg::Office::UI::Notebookbar::ActiveCalc::get( m_xContext );
            break;
        case vcl::EnumContext::Application::Impress:
            aPath.append("Impress");
            aActive = officecfg::Office::UI::Notebookbar::ActiveImpress::get( m_xContext );
            break;
        default:
            break;
    }

    const utl::OConfigurationTreeRoot aModesNode(
                                        m_xContext,
                                        aPath.makeStringAndClear(),
                                        false);
    if ( !aModesNode.isValid() )
        return;

    // Entries are enabled only when Notebookbar mode is active
    bool bActive = comphelper::getString( aModesNode.getNodeValue( "Active" ) ) == "Notebookbar";

    for ( int i = 0; i < m_xPopupMenu->getItemCount(); ++i )
    {
        m_xPopupMenu->checkItem( i+1, aActive == m_xPopupMenu->getCommand( i+1 ) );
        m_xPopupMenu->enableItem( i+1, bActive );
    }
}

// XPopupMenuController
void SAL_CALL NotebookbarMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu )
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

IMPL_STATIC_LINK( NotebookbarMenuController, ExecuteHdl_Impl, void*, p, void )
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
