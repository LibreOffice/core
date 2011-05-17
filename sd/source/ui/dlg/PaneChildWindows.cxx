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
#include "precompiled_sd.hxx"
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

SFX_IMPL_DOCKINGWINDOW(LeftPaneImpressChildWindow, SID_LEFT_PANE_IMPRESS)
SFX_IMPL_DOCKINGWINDOW(LeftPaneDrawChildWindow, SID_LEFT_PANE_DRAW)
SFX_IMPL_DOCKINGWINDOW( ToolPanelChildWindow, SID_TASKPANE)

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
        String( SdResId( nTitleBarResId ) ) );
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




//======================================================================================================================
//= ToolPanelChildWindow
//======================================================================================================================
//----------------------------------------------------------------------------------------------------------------------
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
    DelayedToolPanelActivation( ToolPanelChildWindow& i_rToolPanelWindow, const ::rtl::OUString& i_rPanelURL )
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
    const ::rtl::OUString   m_sPanelURL;
};

//----------------------------------------------------------------------------------------------------------------------
void ToolPanelChildWindow::ActivateToolPanel( const ::rtl::OUString& i_rPanelURL )
{
    SfxDockingWindow* pDockingWindow = dynamic_cast< SfxDockingWindow* >( GetWindow() );
    ViewShellBase* pViewShellBase = ViewShellBase::GetViewShellBase( pDockingWindow->GetBindings().GetDispatcher()->GetFrame() );
    ENSURE_OR_RETURN_VOID( pViewShellBase != NULL, "ToolPanelChildWindow::ActivateToolPanel: no view shell access!" );

    const ::boost::shared_ptr< framework::FrameworkHelper > pFrameworkHelper( framework::FrameworkHelper::Instance( *pViewShellBase ) );

    if ( i_rPanelURL.indexOf( framework::FrameworkHelper::msTaskPanelURLPrefix ) == 0 )
    {
        // it's one of our standard panels known to the drawing framework
        pFrameworkHelper->RequestTaskPanel( i_rPanelURL );
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

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
