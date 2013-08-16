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

#include "PaneChildWindows.hxx"
#include "PaneDockingWindow.hrc"
#include "PaneDockingWindow.hxx"
#include "ViewShellBase.hxx"
#include "framework/FrameworkHelper.hxx"
#include "taskpane/ToolPanelViewShell.hxx"
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"

#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>

#include <sfx2/app.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/diagnose_ex.h>

namespace sd {

using ::com::sun::star::uno::Reference;
using ::com::sun::star::drawing::framework::XResourceId;
using ::com::sun::star::drawing::framework::XConfigurationController;
using ::com::sun::star::drawing::framework::ResourceActivationMode_ADD;
using ::com::sun::star::drawing::framework::ResourceActivationMode_REPLACE;

SFX_IMPL_DOCKINGWINDOW_WITHID(LeftPaneImpressChildWindow, SID_LEFT_PANE_IMPRESS)
SFX_IMPL_DOCKINGWINDOW_WITHID(LeftPaneDrawChildWindow, SID_LEFT_PANE_DRAW)
SFX_IMPL_DOCKINGWINDOW_WITHID(ToolPanelChildWindow, SID_TASKPANE)

//===== PaneChildWindow =======================================================

PaneChildWindow::PaneChildWindow (
    ::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo,
    const sal_uInt16 nDockWinTitleResId,
    const sal_uInt16 nTitleBarResId,
    SfxChildAlignment eAlignment)
    : SfxChildWindow (pParentWindow, nId)
{
    pWindow = new PaneDockingWindow (
        pBindings,
        this,
        pParentWindow,
        SdResId( nDockWinTitleResId ),
        SD_RESSTR( nTitleBarResId ) );
    eChildAlignment = eAlignment;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize(pInfo);
    SetHideNotDelete(sal_True);

    ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pBindings->GetDispatcher()->GetFrame());
    if (pBase != NULL)
    {
        framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
    }
}




PaneChildWindow::~PaneChildWindow (void)
{
    ViewShellBase* pBase = NULL;
    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(pWindow);
    if (pDockingWindow != NULL)
        pBase = ViewShellBase::GetViewShellBase(
            pDockingWindow->GetBindings().GetDispatcher()->GetFrame());
    if (pBase != NULL)
        framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
}






//===== LeftPaneImpressChildWindow ============================================

LeftPaneImpressChildWindow::LeftPaneImpressChildWindow (
    ::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        FLT_LEFT_PANE_IMPRESS_DOCKING_WINDOW,
        STR_LEFT_PANE_IMPRESS_TITLE,
        SFX_ALIGN_LEFT)
{
}




//===== LeftPaneDrawChildWindow ===============================================

LeftPaneDrawChildWindow::LeftPaneDrawChildWindow (
    ::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        FLT_LEFT_PANE_DRAW_DOCKING_WINDOW,
        STR_LEFT_PANE_DRAW_TITLE,
        SFX_ALIGN_LEFT)
{
}


ToolPanelChildWindow::ToolPanelChildWindow( ::Window* i_pParentWindow, sal_uInt16 i_nId, SfxBindings* i_pBindings,
        SfxChildWinInfo* i_pChildWindowInfo )
    :PaneChildWindow( i_pParentWindow, i_nId, i_pBindings, i_pChildWindowInfo,
        FLT_TOOL_PANEL_DOCKING_WINDOW, STR_RIGHT_PANE_TITLE, SFX_ALIGN_RIGHT )
{
    // just in case this window has been created by SFX, instead our resource framework: Ensure that the resource framework
    // activates the task pane, so it is really filled with content (in opposite to the other SFX applications, the
    // child window registered for SID_TASKPANE is not responsible for its content, but here in SD, it's the ToolPanelViewShell
    // which has this responsibility. And this view shell is created implicitly via the resource framework.)
    // #i113788#
    SfxDockingWindow* pDockingWindow = dynamic_cast< SfxDockingWindow* >( GetWindow() );
    ViewShellBase* pViewShellBase = ViewShellBase::GetViewShellBase( pDockingWindow->GetBindings().GetDispatcher()->GetFrame() );
    ENSURE_OR_RETURN_VOID( pViewShellBase != NULL, "ToolPanelChildWindow::ToolPanelChildWindow: no view shell access!" );

    const ::boost::shared_ptr< framework::FrameworkHelper > pFrameworkHelper( framework::FrameworkHelper::Instance( *pViewShellBase ) );
    ENSURE_OR_RETURN_VOID( pFrameworkHelper.get(), "ToolPanelChildWindow::ToolPanelChildWindow: no framework helper for the view shell!" );
    Reference<XConfigurationController> xConfigController( pFrameworkHelper->GetConfigurationController() );
    ENSURE_OR_RETURN_VOID( xConfigController.is(), "ToolPanelChildWindow::ToolPanelChildWindow: no config controller!" );
    xConfigController->requestResourceActivation(
        framework::FrameworkHelper::CreateResourceId( framework::FrameworkHelper::msRightPaneURL ),
        ResourceActivationMode_ADD );
    xConfigController->requestResourceActivation(
        framework::FrameworkHelper::CreateResourceId( framework::FrameworkHelper::msTaskPaneURL, framework::FrameworkHelper::msRightPaneURL ),
        ResourceActivationMode_REPLACE
    );
}

//----------------------------------------------------------------------------------------------------------------------
struct DelayedToolPanelActivation
{
    DelayedToolPanelActivation( ToolPanelChildWindow& i_rToolPanelWindow, const OUString& i_rPanelURL )
        :m_rToolPanelWindow( i_rToolPanelWindow )
        ,m_sPanelURL( i_rPanelURL )
    {
    }

    void operator() (bool)
    {
        m_rToolPanelWindow.ActivateToolPanel( m_sPanelURL );
    }

private:
    ToolPanelChildWindow&   m_rToolPanelWindow;
    const OUString   m_sPanelURL;
};

//----------------------------------------------------------------------------------------------------------------------
void ToolPanelChildWindow::ActivateToolPanel( const OUString& i_rPanelURL )
{
    SfxDockingWindow* pDockingWindow = dynamic_cast< SfxDockingWindow* >( GetWindow() );
    ViewShellBase* pViewShellBase = ViewShellBase::GetViewShellBase( pDockingWindow->GetBindings().GetDispatcher()->GetFrame() );
    ENSURE_OR_RETURN_VOID( pViewShellBase != NULL, "ToolPanelChildWindow::ActivateToolPanel: no view shell access!" );

    const ::boost::shared_ptr< framework::FrameworkHelper > pFrameworkHelper( framework::FrameworkHelper::Instance( *pViewShellBase ) );

    if ( i_rPanelURL.indexOf( framework::FrameworkHelper::msTaskPanelURLPrefix ) == 0 )
    {
        // it's one of our standard panels known to the drawing framework
        pFrameworkHelper->RequestSidebarPanel( i_rPanelURL );
    }
    else
    {
        // TODO: it would be nice if the drawing framework were able to handle non-standard panels, installed by
        // extensions, too. As long as this is not the case, we need to take the direct way ...
        ::boost::shared_ptr< ViewShell > pViewShell = pFrameworkHelper->GetViewShell( framework::FrameworkHelper::msRightPaneURL );
        toolpanel::ToolPanelViewShell* pToolPanelViewShell = dynamic_cast< toolpanel::ToolPanelViewShell* >( pViewShell.get() );
        if ( pToolPanelViewShell )
        {
            pToolPanelViewShell->ActivatePanel( i_rPanelURL );
        }
        else
        {
            Reference< XResourceId > xTaskPaneResource = pFrameworkHelper->RequestView(
                framework::FrameworkHelper::msTaskPaneURL, framework::FrameworkHelper::msRightPaneURL );
            pFrameworkHelper->RunOnResourceActivation( xTaskPaneResource, DelayedToolPanelActivation( *this, i_rPanelURL ) );
        }
    }
}

void ToolPanelChildWindow::RegisterChildWindowIfEnabled (sal_Bool bVisible,
                                                         SfxModule *pMod,
                                                         sal_uInt16 nFlags)
{
    if ( ! SfxViewFrame::IsSidebarEnabled() )
        RegisterChildWindow( bVisible, pMod, nFlags );
}

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
