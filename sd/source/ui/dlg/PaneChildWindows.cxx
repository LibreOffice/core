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
#include "app.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include <sfx2/app.hxx>
#include <sfx2/dockwin.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sd
{
    SFX_IMPL_DOCKINGWINDOW(LeftPaneImpressChildWindow, SID_LEFT_PANE_IMPRESS)
    SFX_IMPL_DOCKINGWINDOW(LeftPaneDrawChildWindow, SID_LEFT_PANE_DRAW)
    SFX_IMPL_DOCKINGWINDOW(RightPaneChildWindow, SID_RIGHT_PANE)
}


#include "PaneDockingWindow.hxx"
#include "ViewShellBase.hxx"
#include "framework/FrameworkHelper.hxx"

namespace sd {

//===== PaneChildWindow =======================================================

PaneChildWindow::PaneChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo,
    const ResId& rResId,
    const ::rtl::OUString& rsTitle,
    SfxChildAlignment eAlignment)
    : SfxChildWindow (pParentWindow, nId)
{
    pWindow = new PaneDockingWindow (
        pBindings,
        this,
        pParentWindow,
        rResId,
        framework::FrameworkHelper::msLeftImpressPaneURL,
        rsTitle);
    eChildAlignment = eAlignment;
    static_cast<SfxDockingWindow*>(pWindow)->Initialize(pInfo);
    SetHideNotDelete(TRUE);

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
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        SdResId(FLT_LEFT_PANE_IMPRESS_DOCKING_WINDOW),
        String(SdResId(STR_LEFT_PANE_IMPRESS_TITLE)),
        SFX_ALIGN_LEFT)
{
}




//===== LeftPaneDrawChildWindow ===============================================

LeftPaneDrawChildWindow::LeftPaneDrawChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        SdResId(FLT_LEFT_PANE_DRAW_DOCKING_WINDOW),
        String(SdResId(STR_LEFT_PANE_DRAW_TITLE)),
        SFX_ALIGN_LEFT)
{
}




//===== RightPaneChildWindow ==================================================

RightPaneChildWindow::RightPaneChildWindow (
    ::Window* pParentWindow,
    USHORT nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        SdResId(FLT_RIGHT_PANE_DOCKING_WINDOW),
        String(SdResId(STR_RIGHT_PANE_TITLE)),
        SFX_ALIGN_RIGHT)
{
}


} // end of namespace ::sd
