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
#include "sfx2/sidebar/SidebarChildWindow.hxx"
#include "SidebarController.hxx"

#include "sfx2/bindings.hxx"
#include "sfx2/dispatch.hxx"
#include <tools/link.hxx>

using namespace css;
using namespace cssu;


namespace sfx2 { namespace sidebar {


SidebarDockingWindow::SidebarDockingWindow(
    SfxBindings* pSfxBindings,
    SidebarChildWindow& rChildWindow,
    Window* pParentWindow,
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
}




void SidebarDockingWindow::GetFocus()
{
    mpSidebarController->GetFocusManager().GrabFocus();
}




SfxChildWindow* SidebarDockingWindow::GetChildWindow (void)
{
    return GetChildWindow_Impl();
}




sal_Bool SidebarDockingWindow::Close (void)
{
    if (mpSidebarController.is())
    {
        // Do not close the floating window.
        // Dock it and close just the deck instead.
        mpSidebarController->RequestCloseDeck();
        SetFloatingMode(sal_False);
        mpSidebarController->NotifyResize();
        return sal_False;
    }
    else
        return SfxDockingWindow::Close();
}


} } // end of namespace sfx2::sidebar
