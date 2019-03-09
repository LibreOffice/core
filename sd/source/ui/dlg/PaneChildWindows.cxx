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

#include <PaneChildWindows.hxx>
#include <PaneDockingWindow.hxx>
#include <ViewShellBase.hxx>
#include <framework/FrameworkHelper.hxx>
#include <app.hrc>
#include <strings.hrc>
#include <sdresid.hxx>

#include <sfx2/dockwin.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>

namespace sd {


SFX_IMPL_DOCKINGWINDOW_WITHID(LeftPaneImpressChildWindow, SID_LEFT_PANE_IMPRESS)
SFX_IMPL_DOCKINGWINDOW_WITHID(LeftPaneDrawChildWindow, SID_LEFT_PANE_DRAW)

//===== PaneChildWindow =======================================================
PaneChildWindow::PaneChildWindow (
    vcl::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo,
    const char* pTitleBarResId)
    : SfxChildWindow (pParentWindow, nId)
{
    SetWindow( VclPtr<PaneDockingWindow>::Create(
        pBindings,
        this,
        pParentWindow,
        SdResId(pTitleBarResId)));
    SetAlignment(SfxChildAlignment::LEFT);
    static_cast<SfxDockingWindow*>(GetWindow())->Initialize(pInfo);
    SetHideNotDelete(true);

    ViewShellBase* pBase = ViewShellBase::GetViewShellBase(pBindings->GetDispatcher()->GetFrame());
    if (pBase != nullptr)
    {
        framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
    }
}

PaneChildWindow::~PaneChildWindow()
{
    ViewShellBase* pBase = nullptr;
    PaneDockingWindow* pDockingWindow = dynamic_cast<PaneDockingWindow*>(GetWindow());
    if (pDockingWindow != nullptr)
        pBase = ViewShellBase::GetViewShellBase(
            pDockingWindow->GetBindings().GetDispatcher()->GetFrame());
    if (pBase != nullptr)
        framework::FrameworkHelper::Instance(*pBase)->UpdateConfiguration();
}

//===== LeftPaneImpressChildWindow ============================================
LeftPaneImpressChildWindow::LeftPaneImpressChildWindow (
    vcl::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        STR_LEFT_PANE_IMPRESS_TITLE)
{
}

//===== LeftPaneDrawChildWindow ===============================================
LeftPaneDrawChildWindow::LeftPaneDrawChildWindow (
    vcl::Window* pParentWindow,
    sal_uInt16 nId,
    SfxBindings* pBindings,
    SfxChildWinInfo* pInfo)
    : PaneChildWindow(
        pParentWindow,
        nId,
        pBindings,
        pInfo,
        STR_LEFT_PANE_DRAW_TITLE)
{
}

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
