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
#include "SidebarDockingWindow.hxx"
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include "SidebarController.hxx"

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/link.hxx>

using namespace css;
using namespace css::uno;


namespace sfx2 { namespace sidebar {


SidebarDockingWindow::SidebarDockingWindow(
    SfxBindings* pSfxBindings,
    SidebarChildWindow& rChildWindow,
    vcl::Window* pParentWindow,
    WinBits nBits)
    : SfxDockingWindow(pSfxBindings, &rChildWindow, pParentWindow, nBits),
      mpSidebarController()
{
    // Get the XFrame from the bindings.
    if (pSfxBindings==NULL || pSfxBindings->GetDispatcher()==NULL)
    {
        OSL_ASSERT(pSfxBindings!=NULL);
        OSL_ASSERT(pSfxBindings->GetDispatcher()!=NULL);
    }
    else
    {
        const SfxViewFrame* pViewFrame = pSfxBindings->GetDispatcher()->GetFrame();
        const SfxFrame& rFrame = pViewFrame->GetFrame();
        mpSidebarController.set(new sfx2::sidebar::SidebarController(this, rFrame.GetFrameInterface()));
    }
}




SidebarDockingWindow::~SidebarDockingWindow (void)
{
    DoDispose();
}




void SidebarDockingWindow::DoDispose (void)
{
    Reference<lang::XComponent> xComponent (static_cast<XWeak*>(mpSidebarController.get()), UNO_QUERY);
    mpSidebarController.clear();
    if (xComponent.is())
    {
        xComponent->dispose();
    }
}




void SidebarDockingWindow::GetFocus()
{
    if (mpSidebarController.is())
        mpSidebarController->GetFocusManager().GrabFocus();
    else
        SfxDockingWindow::GetFocus();
}




// fdo#87217
bool SidebarDockingWindow::Close (void)
{
    return SfxDockingWindow::Close();
}




SfxChildAlignment SidebarDockingWindow::CheckAlignment (
    SfxChildAlignment eCurrentAlignment,
    SfxChildAlignment eRequestedAlignment)
{
    switch (eRequestedAlignment)
    {
        case SFX_ALIGN_TOP:
        case SFX_ALIGN_HIGHESTTOP:
        case SFX_ALIGN_LOWESTTOP:
        case SFX_ALIGN_BOTTOM:
        case SFX_ALIGN_LOWESTBOTTOM:
        case SFX_ALIGN_HIGHESTBOTTOM:
            return eCurrentAlignment;

        case SFX_ALIGN_LEFT:
        case SFX_ALIGN_RIGHT:
        case SFX_ALIGN_FIRSTLEFT:
        case SFX_ALIGN_LASTLEFT:
        case SFX_ALIGN_FIRSTRIGHT:
        case SFX_ALIGN_LASTRIGHT:
            return eRequestedAlignment;

        default:
            return eRequestedAlignment;
    }
}


} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
