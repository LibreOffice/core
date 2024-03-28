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
#include <sidebar/PanelDescriptor.hxx>

#include <comphelper/dispatchcommand.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <tools/gen.hxx>
#include <vcl/event.hxx>
#include <osl/diagnose.h>

#include <boost/property_tree/json_parser.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2::sidebar {

SidebarDockingWindow::SidebarDockingWindow(SfxBindings* pSfxBindings, SidebarChildWindow& rChildWindow,
                                           vcl::Window* pParentWindow, WinBits nBits)
    : SfxDockingWindow(pSfxBindings, &rChildWindow, pParentWindow, nBits)
    , mbIsReadyToDrag(false)
{
    // Get the XFrame from the bindings.
    if (pSfxBindings==nullptr || pSfxBindings->GetDispatcher()==nullptr)
    {
        OSL_ASSERT(pSfxBindings!=nullptr);
        OSL_ASSERT(pSfxBindings->GetDispatcher()!=nullptr);
    }
    else if (!comphelper::LibreOfficeKit::isActive())
    {
        GetOrCreateSidebarController();
    }
}

rtl::Reference<sfx2::sidebar::SidebarController>& SidebarDockingWindow::GetOrCreateSidebarController()
{
    if (!mpSidebarController)
    {
        const SfxViewFrame* pViewFrame = GetBindings().GetDispatcher()->GetFrame();
        mpSidebarController = sfx2::sidebar::SidebarController::create(this, pViewFrame);
    }

    return mpSidebarController;
}

SidebarDockingWindow::~SidebarDockingWindow()
{
    disposeOnce();
}

void SidebarDockingWindow::dispose()
{
    Reference<lang::XComponent> xComponent(mpSidebarController);
    mpSidebarController.clear();
    if (xComponent.is())
        xComponent->dispose();

    SfxDockingWindow::dispose();
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
    if (mpSidebarController.is())
        mpSidebarController->SetFloatingDeckClosed(true);

    return SfxDockingWindow::Close();
}

void SidebarDockingWindow::SyncUpdate()
{
    if (mpSidebarController.is())
        mpSidebarController->SyncUpdate();
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
    if (mpSidebarController)
    {
        NotifyEventType nType = rEvent.GetType();
        if (NotifyEventType::KEYINPUT == nType)
        {
            const vcl::KeyCode& rKeyCode = rEvent.GetKeyEvent()->GetKeyCode();
            switch (rKeyCode.GetCode())
            {
                case KEY_UP:
                case KEY_DOWN:
                case KEY_PAGEUP:
                case KEY_PAGEDOWN:
                case KEY_HOME:
                case KEY_END:
                case KEY_LEFT:
                case KEY_RIGHT:
                case KEY_BACKSPACE:
                case KEY_DELETE:
                case KEY_INSERT:
                case KEY_RETURN:
                case KEY_ESCAPE:
                {
                    return true;
                }
                default:
                break;
            }
            if (!mpAccel)
            {
                mpAccel = svt::AcceleratorExecute::createAcceleratorHelper();
                mpAccel->init(comphelper::getProcessComponentContext(), mpSidebarController->getXFrame());
            }
            const OUString aCommand(mpAccel->findCommand(svt::AcceleratorExecute::st_VCLKey2AWTKey(rKeyCode)));
            if (".uno:DesignerDialog" == aCommand)
            {
                std::shared_ptr<PanelDescriptor> xPanelDescriptor =
                        mpSidebarController->GetResourceManager()->GetPanelDescriptor( u"StyleListPanel" );
                if ( xPanelDescriptor && mpSidebarController->IsDeckVisible( xPanelDescriptor->msDeckId ) )
                    Close();
                return true;
            }
            if (".uno:Undo" == aCommand || ".uno:Redo" == aCommand)
            {
                comphelper::dispatchCommand(aCommand, {});
                return true;
            }
        }
        else if (NotifyEventType::MOUSEBUTTONDOWN == nType)
        {
            const MouseEvent *mEvt = rEvent.GetMouseEvent();
            if (mEvt->IsLeft())
            {
                tools::Rectangle aGrip = mpSidebarController->GetDeckDragArea();
                if ( aGrip.Contains( mEvt->GetPosPixel() ) )
                    mbIsReadyToDrag = true;
            }
        }
        else if (NotifyEventType::MOUSEMOVE == nType)
        {
            const MouseEvent *mEvt = rEvent.GetMouseEvent();
            tools::Rectangle aGrip = mpSidebarController->GetDeckDragArea();
            if (mEvt->IsLeft() && aGrip.Contains( mEvt->GetPosPixel() ) && mbIsReadyToDrag )
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
    }

    return SfxDockingWindow::EventNotify(rEvent);
}

} // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
