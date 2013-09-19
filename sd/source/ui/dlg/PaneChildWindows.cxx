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

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
