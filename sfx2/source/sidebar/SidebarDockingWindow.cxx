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

#include <comphelper/processfactory.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/event.hxx>
#include <comphelper/lok.hxx>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

SidebarDockingWindow::SidebarDockingWindow(SfxBindings* pSfxBindings, SidebarChildWindow& rChildWindow,
                                           vcl::Window* pParentWindow, WinBits nBits)
    : SfxDockingWindow(pSfxBindings, &rChildWindow, pParentWindow, nBits)
    , mpSidebarController()
    , mbIsReadyToDrag(false)
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
        const SfxFrame& rFrame = pViewFrame->GetFrame();
        mpSidebarController.set(sfx2::sidebar::SidebarController::create(this, rFrame.GetFrameInterface()).get());
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
    {
        mpSidebarController->RequestOpenDeck();
        mpSidebarController->GetFocusManager().GrabFocus();
    }
    else
        SfxDockingWindow::GetFocus();
}

bool SidebarDockingWindow::Close()
{
    mpSidebarController->SetFloatingDeckClosed( true );
    return SfxDockingWindow::Close();
}

void SidebarDockingWindow::Resize()
{
    SfxDockingWindow::Resize();

    NotifyResize();
}

void SidebarDockingWindow::NotifyResize()
{
    if (comphelper::LibreOfficeKit::isActive() && SfxViewShell::Current())
    {
        if (!GetLOKNotifier())
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

bool SidebarDockingWindow::EventNotify(NotifyEvent& rEvent)
{
    MouseNotifyEvent nType = rEvent.GetType();
    if (MouseNotifyEvent::KEYINPUT == nType)
    {
        const vcl::KeyCode& rKeyCode = rEvent.GetKeyEvent()->GetKeyCode();
        if (!(rKeyCode.GetCode() == KEY_F10 && rKeyCode.GetModifier() &&
            rKeyCode.IsShift() && rKeyCode.IsMod1()))
        {
            if (!mpAccel)
            {
                mpAccel = svt::AcceleratorExecute::createAcceleratorHelper();
                mpAccel->init(comphelper::getProcessComponentContext(), mpSidebarController->getXFrame());
            }
            const OUString aCommand(mpAccel->findCommand(svt::AcceleratorExecute::st_VCLKey2AWTKey(rKeyCode)));
            if (".uno:DesignerDialog" == aCommand)
            {
                std::shared_ptr<PanelDescriptor> xPanelDescriptor =
                    mpSidebarController->GetResourceManager()->GetPanelDescriptor( "StyleListPanel" );
                if ( xPanelDescriptor && mpSidebarController->IsDeckVisible( xPanelDescriptor->msDeckId ) )
                    Close();
                return true;
            }
            if (".uno:Sidebar" != aCommand)
                return true;
        }
    }
    else if (MouseNotifyEvent::MOUSEBUTTONDOWN == nType)
    {
        const MouseEvent *mEvt = rEvent.GetMouseEvent();
        if (mEvt->IsLeft())
        {
            tools::Rectangle aGrip = mpSidebarController->GetDeckDragArea();
            if ( aGrip.IsInside( mEvt->GetPosPixel() ) )
                SetReadyToDrag( true );
        }
    }
    else if (MouseNotifyEvent::MOUSEMOVE == nType)
    {
        const MouseEvent *mEvt = rEvent.GetMouseEvent();
        tools::Rectangle aGrip = mpSidebarController->GetDeckDragArea();
        if (mEvt->IsLeft() && aGrip.IsInside( mEvt->GetPosPixel() ) && IsReadyToDrag() )
        {
            Point aPos = mEvt->GetPosPixel();
            vcl::Window* pWindow = rEvent.GetWindow();
            if ( pWindow != this )
            {
                aPos = pWindow->OutputToScreenPixel( aPos );
                aPos = ScreenToOutputPixel( aPos );
            }
            ImplStartDocking( aPos );
        }
    }

    return SfxDockingWindow::EventNotify(rEvent);
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
