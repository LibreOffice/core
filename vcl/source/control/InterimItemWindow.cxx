/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/InterimItemWindow.hxx>
#include <vcl/layout.hxx>

InterimItemWindow::InterimItemWindow(vcl::Window* pParent, const OUString& rUIXMLDescription,
                                     const OString& rID, bool bAllowCycleFocusOut,
                                     sal_uInt64 nLOKWindowId)
    : Control(pParent, WB_TABSTOP)
    , m_pWidget(nullptr) // inheritors are expected to call InitControlBase
{
    m_aLayoutIdle.SetPriority(TaskPriority::RESIZE);
    m_aLayoutIdle.SetInvokeHandler(LINK(this, InterimItemWindow, DoLayout));
    m_aLayoutIdle.SetDebugName("InterimItemWindow m_aLayoutIdle");

    m_xVclContentArea = VclPtr<VclVBox>::Create(this);
    m_xVclContentArea->Show();
    m_xBuilder.reset(Application::CreateInterimBuilder(m_xVclContentArea, rUIXMLDescription,
                                                       bAllowCycleFocusOut, nLOKWindowId));
    m_xContainer = m_xBuilder->weld_container(rID);

    SetBackground();
    SetPaintTransparent(true);
}

void InterimItemWindow::StateChanged(StateChangedType nStateChange)
{
    if (nStateChange == StateChangedType::Enable)
        m_xContainer->set_sensitive(IsEnabled());
    Control::StateChanged(nStateChange);
}

InterimItemWindow::~InterimItemWindow() { disposeOnce(); }

void InterimItemWindow::dispose()
{
    m_pWidget = nullptr;

    m_xContainer.reset();
    m_xBuilder.reset();
    m_xVclContentArea.disposeAndClear();

    m_aLayoutIdle.Stop();

    Control::dispose();
}

void InterimItemWindow::StartIdleLayout()
{
    if (!m_xVclContentArea)
        return;
    if (m_aLayoutIdle.IsActive())
        return;
    m_aLayoutIdle.Start();
}

void InterimItemWindow::queue_resize(StateChangedType eReason)
{
    Control::queue_resize(eReason);
    StartIdleLayout();
}

void InterimItemWindow::Resize() { Layout(); }

IMPL_LINK_NOARG(InterimItemWindow, DoLayout, Timer*, void) { Layout(); }

void InterimItemWindow::Layout()
{
    m_aLayoutIdle.Stop();
    vcl::Window* pChild = GetWindow(GetWindowType::FirstChild);
    assert(pChild);
    VclContainer::setLayoutAllocation(*pChild, Point(0, 0), GetSizePixel());
    Control::Resize();
}

Size InterimItemWindow::GetOptimalSize() const
{
    return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));
}

bool InterimItemWindow::ControlHasFocus() const
{
    if (!m_pWidget)
        return false;
    return m_pWidget->has_focus();
}

void InterimItemWindow::InitControlBase(weld::Widget* pWidget) { m_pWidget = pWidget; }

void InterimItemWindow::GetFocus()
{
    if (m_pWidget)
        m_pWidget->grab_focus();

    /* let toolbox know this item window has focus so it updates its mnHighItemId to point
       to this toolitem in case tab means to move to another toolitem within
       the toolbox
    */
    vcl::Window* pToolBox = GetParent();
    NotifyEvent aNEvt(MouseNotifyEvent::GETFOCUS, this);
    pToolBox->EventNotify(aNEvt);
}

bool InterimItemWindow::ChildKeyInput(const KeyEvent& rKEvt)
{
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    if (nCode != KEY_TAB)
        return false;

    /* if the native widget has focus, then no vcl window has focus.

       We want to grab focus to this vcl widget so that pressing tab will traverse
       to the next vcl widget.

       But just using GrabFocus will, because no vcl widget has focus, trigger
       bringing the toplevel to front with the expectation that a suitable widget
       will be picked for focus when that happen, which is no use to us here.

       SetFakeFocus avoids the problem, allowing GrabFocus to do the expected thing
       then sending the Tab to our parent will do the right traversal
    */
    SetFakeFocus(true);
    GrabFocus();

    /* now give focus to our toolbox parent */
    vcl::Window* pToolBox = GetParent();
    pToolBox->GrabFocus();

    /* let toolbox know this item window has focus so it updates its mnHighItemId to point
       to this toolitem in case tab means to move to another toolitem within
       the toolbox
    */
    NotifyEvent aNEvt(MouseNotifyEvent::GETFOCUS, this);
    pToolBox->EventNotify(aNEvt);

    /* send parent the tab */
    pToolBox->KeyInput(rKEvt);

    return true;
}

void InterimItemWindow::Draw(OutputDevice* pDevice, const Point& rPos, DrawFlags /*nFlags*/)
{
    if (!m_pWidget)
        return;
    m_pWidget->draw(*pDevice, tools::Rectangle(rPos, GetSizePixel()));
}

void InterimItemWindow::ImplPaintToDevice(::OutputDevice* pTargetOutDev, const Point& rPos)
{
    Draw(pTargetOutDev, rPos, DrawFlags::NONE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
