/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <memory>

#include <svx/devtools/DevelopmentToolDockingWindow.hxx>

#include <sfx2/dispatch.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svx/svxids.hrc>

SFX_IMPL_DOCKINGWINDOW_WITHID(DevelopmentToolChildWindow, SID_DEVELOPMENT_TOOLS_DOCKING_WINDOW);

DevelopmentToolChildWindow::DevelopmentToolChildWindow(vcl::Window* pParentWindow, sal_uInt16 nId,
                                                       SfxBindings* pBindings,
                                                       SfxChildWinInfo* pInfo)
    : SfxChildWindow(pParentWindow, nId)
{
    VclPtr<DevelopmentToolDockingWindow> pWin
        = VclPtr<DevelopmentToolDockingWindow>::Create(pBindings, this, pParentWindow);
    SetWindow(pWin);
    SetAlignment(SfxChildAlignment::BOTTOM);
    pWin->Initialize(pInfo);
}

DevelopmentToolChildWindow::~DevelopmentToolChildWindow() {}

DevelopmentToolDockingWindow::DevelopmentToolDockingWindow(SfxBindings* pInputBindings,
                                                           SfxChildWindow* pChildWindow,
                                                           vcl::Window* pParent)
    : SfxDockingWindow(pInputBindings, pChildWindow, pParent, "DevelopmentTool",
                       "svx/ui/developmenttool.ui")
{
}

DevelopmentToolDockingWindow::~DevelopmentToolDockingWindow() { disposeOnce(); }

void DevelopmentToolDockingWindow::ToggleFloatingMode()
{
    SfxDockingWindow::ToggleFloatingMode();

    if (GetFloatingWindow())
        GetFloatingWindow()->SetMinOutputSizePixel(Size(300, 300));

    Invalidate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
