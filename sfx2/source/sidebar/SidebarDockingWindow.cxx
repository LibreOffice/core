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
#include <sfx2/sidebar/PanelDescriptor.hxx>

#include <comphelper/processfactory.hxx>
#include <sfx2/lokhelper.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <tools/link.hxx>
#include <tools/gen.hxx>
#include <vcl/event.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <boost/property_tree/json_parser.hpp>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

class SidebarNotifyIdle : public Idle
{
    SidebarDockingWindow& m_rSidebarDockingWin;
    std::string m_LastNotificationMessage;
    vcl::LOKWindowId m_LastLOKWindowId;

public:
    SidebarNotifyIdle(SidebarDockingWindow &rSidebarDockingWin) :
        Idle("Sidebar notify"),
        m_rSidebarDockingWin(rSidebarDockingWin),
        m_LastNotificationMessage(),
        m_LastLOKWindowId(0)
    {
        SetPriority(TaskPriority::POST_PAINT);
    }

    void Invoke() override
    {
        if (!comphelper::LibreOfficeKit::isActive())
            return;

        try
        {
            auto pNotifier = m_rSidebarDockingWin.GetLOKNotifier();
            if (!pNotifier)
                return;

            const SfxViewShell* pOwnerView = dynamic_cast<const SfxViewShell*>(pNotifier);
            if (pOwnerView && pOwnerView->isLOKMobilePhone())
            {
                // Mobile.
                std::stringstream aStream;
                boost::property_tree::ptree aTree = m_rSidebarDockingWin.DumpAsPropertyTree();
                aTree.put("id", m_rSidebarDockingWin.GetLOKWindowId());
                boost::property_tree::write_json(aStream, aTree);
                const std::string message = aStream.str();
                if (message != m_LastNotificationMessage)
                {
                    m_LastNotificationMessage = message;
                    pOwnerView->libreOfficeKitViewCallback(LOK_CALLBACK_JSDIALOG, message.c_str());
                }
            }

            // Notify the sidebar is created, and its LOKWindowId, which
            // is needed on mobile phones, tablets, and desktop.
            const Point pos(m_rSidebarDockingWin.GetOutOffXPixel(),
                            m_rSidebarDockingWin.GetOutOffYPixel());
            const OString posMessage = pos.toString();
            const OString sizeMessage = m_rSidebarDockingWin.GetSizePixel().toString();

            const std::string message = OString(posMessage + sizeMessage).getStr();
            const vcl::LOKWindowId lokWindowId = m_rSidebarDockingWin.GetLOKWindowId();

            if (lokWindowId != m_LastLOKWindowId || message != m_LastNotificationMessage)
            {
                m_LastLOKWindowId = lokWindowId;
                m_LastNotificationMessage = message;

                std::vector<vcl::LOKPayloadItem> aItems;
                aItems.emplace_back("type", "deck");
                aItems.emplace_back("position", posMessage);
                aItems.emplace_back("size", sizeMessage);
                pNotifier->notifyWindow(lokWindowId, "created", aItems);
            }
        }
        catch (boost::property_tree::json_parser::json_parser_error& rError)
        {
            SAL_WARN("sfx.sidebar", rError.message());
        }
    }
};

SidebarDockingWindow::SidebarDockingWindow(SfxBindings* pSfxBindings, SidebarChildWindow& rChildWindow,
                                           vcl::Window* pParentWindow, WinBits nBits)
    : SfxDockingWindow(pSfxBindings, &rChildWindow, pParentWindow, nBits)
    , mpSidebarController()
    , mbIsReadyToDrag(false)
    , mpIdleNotify(new SidebarNotifyIdle(*this))
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
    if (comphelper::LibreOfficeKit::isActive())
        LOKClose();

    Reference<lang::XComponent> xComponent (static_cast<XWeak*>(mpSidebarController.get()), UNO_QUERY);
    mpSidebarController.clear();
    if (xComponent.is())
        xComponent->dispose();

    SfxDockingWindow::dispose();
}

void SidebarDockingWindow::LOKClose()
{
    assert(comphelper::LibreOfficeKit::isActive());
    if (const vcl::ILibreOfficeKitNotifier* pNotifier = GetLOKNotifier())
    {
        mpIdleNotify->Stop();

        pNotifier->notifyWindow(GetLOKWindowId(), "close");
        ReleaseLOKNotifier();
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
    if (mpSidebarController.is())
        mpSidebarController->SetFloatingDeckClosed(true);

    return SfxDockingWindow::Close();
}

void SidebarDockingWindow::Resize()
{
    SfxDockingWindow::Resize();

    NotifyResize();
}

void SidebarDockingWindow::SyncUpdate()
{
    if (mpSidebarController.is())
        mpSidebarController->SyncUpdate();
}

void SidebarDockingWindow::NotifyResize()
{
    if (comphelper::LibreOfficeKit::isActive() && mpSidebarController.is() && SfxViewShell::Current())
    {
        const vcl::ILibreOfficeKitNotifier* pCurrentView = SfxViewShell::Current();
        if (GetLOKNotifier() != pCurrentView)
        {
            // ViewShell not yet set, or has changed. Reset it.
            // Note GetLOKWindowId will return a new value after resetting, so we must notify clients.
            LOKClose();

            SetLOKNotifier(pCurrentView);
        }

        mpIdleNotify->Start();
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
                    mpSidebarController->GetResourceManager()->GetPanelDescriptor( "StyleListPanel" );
            if ( xPanelDescriptor && mpSidebarController->IsDeckVisible( xPanelDescriptor->msDeckId ) )
                Close();
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
