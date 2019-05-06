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
#include <sfx2/sidebar/SidebarDockingWindow.hxx>
#include <sfx2/sidebar/SidebarChildWindow.hxx>
#include <sfx2/sidebar/SidebarController.hxx>

#include <sfx2/lokhelper.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/link.hxx>
#include <comphelper/lok.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

SidebarDockingWindow::SidebarDockingWindow(SfxBindings* pSfxBindings, SidebarChildWindow& rChildWindow,
                                           vcl::Window* pParentWindow, WinBits nBits)
    : SfxDockingWindow(pSfxBindings, &rChildWindow, pParentWindow, nBits)
    , mpSidebarController()
    , mbSidebarVisibleInLOK(rChildWindow.IsSidebarVisibleInLOK())
    , mpOldViewShell(SfxViewShell::Current())
{
    // Get the XFrame from the bindings.
    if (pSfxBindings==nullptr || pSfxBindings->GetDispatcher()==nullptr)
    {
        OSL_ASSERT(pSfxBindings!=nullptr);
        OSL_ASSERT(pSfxBindings->GetDispatcher()!=nullptr);
    }
    else
    {
        const SfxViewFrame* pViewFrame = pSfxBindings->GetDispatcher()->GetFrame();
        mpSidebarController.set(sfx2::sidebar::SidebarController::create(this, pViewFrame).get());
    }
}

SidebarDockingWindow::~SidebarDockingWindow()
{
    disposeOnce();
}

void SidebarDockingWindow::dispose()
{
    DoDispose();
    SfxDockingWindow::dispose();
}

void SidebarDockingWindow::DoDispose()
{
    if (comphelper::LibreOfficeKit::isActive())
    {
        if (const vcl::ILibreOfficeKitNotifier* pNotifier = GetLOKNotifier())
            pNotifier->notifyWindow(GetLOKWindowId(), "close");
    }

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

void SidebarDockingWindow::Resize()
{
    SfxDockingWindow::Resize();

    NotifyResize();
}

void SidebarDockingWindow::NotifyResize()
{
    if (comphelper::LibreOfficeKit::isActive() && SfxViewShell::Current() && mbSidebarVisibleInLOK)
    {
        // When a new view is attached, and Sidebar is created (SidebarDockingWindow is constructed),
        // unfortunately we still have the *old* ViewShell (and ViewFrame). This happens because
        // we get multiple NotifyResize are called while SfxBaseController::ConnectSfxFrame_Impl
        // goes through the motions of creating and attaching a new frame/view.
        // Problem is that once we SetLOKNotifier on a window, we can't change it. So we better
        // set the correct one. Worse, if we set the old one, we will change the sidebar of the
        // wrong view, messing things up badly for the users.
        // Knowing the above, we wait until the dust settles, by observing when the ViewShell is
        // changed from the time we were created.
        // Note: this means we *cannot* create a sidebar post attaching a new view because the
        // ViewShell will not change, and therefore we will never SetLOKNotifier. To avoid that
        // we hide sidebars instead of closing (see OnMenuItemSelected in SidebarController).
        if (mpSidebarController.is() && !GetLOKNotifier())
            SetLOKNotifier(SfxViewShell::Current());

        if (const vcl::ILibreOfficeKitNotifier* pNotifier = GetLOKNotifier())
        {
            std::vector<vcl::LOKPayloadItem> aItems;
            aItems.emplace_back("type", "deck");
            aItems.emplace_back(std::make_pair("position", Point(GetOutOffXPixel(), GetOutOffYPixel()).toString()));
            aItems.emplace_back(std::make_pair("size", GetSizePixel().toString()));
            pNotifier->notifyWindow(GetLOKWindowId(), "created", aItems);
        }
    }
}

SfxChildAlignment SidebarDockingWindow::CheckAlignment (
    SfxChildAlignment eCurrentAlignment,
    SfxChildAlignment eRequestedAlignment)
{
    switch (eRequestedAlignment)
    {
        case SfxChildAlignment::TOP:
        case SfxChildAlignment::HIGHESTTOP:
        case SfxChildAlignment::LOWESTTOP:
        case SfxChildAlignment::BOTTOM:
        case SfxChildAlignment::LOWESTBOTTOM:
        case SfxChildAlignment::HIGHESTBOTTOM:
            return eCurrentAlignment;

        case SfxChildAlignment::LEFT:
        case SfxChildAlignment::RIGHT:
        case SfxChildAlignment::FIRSTLEFT:
        case SfxChildAlignment::LASTLEFT:
        case SfxChildAlignment::FIRSTRIGHT:
        case SfxChildAlignment::LASTRIGHT:
            return eRequestedAlignment;

        default:
            return eRequestedAlignment;
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
