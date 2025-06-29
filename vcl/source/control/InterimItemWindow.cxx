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
#include <salobj.hxx>
#include <svdata.hxx>
#include <window.h>

InterimItemWindow::InterimItemWindow(vcl::Window* pParent, const OUString& rUIXMLDescription,
                                     const OUString& rID, bool bAllowCycleFocusOut,
                                     sal_uInt64 nLOKWindowId)
    : Control(pParent, WB_TABSTOP)
    , m_pWidget(nullptr) // inheritors are expected to call InitControlBase
    , m_aLayoutIdle("InterimItemWindow m_aLayoutIdle")
{
    m_aLayoutIdle.SetPriority(TaskPriority::RESIZE);
    m_aLayoutIdle.SetInvokeHandler(LINK(this, InterimItemWindow, DoLayout));

    m_xVclContentArea = VclPtr<VclVBox>::Create(this);
    m_xVclContentArea->Show();
    m_xBuilder = Application::CreateInterimBuilder(m_xVclContentArea, rUIXMLDescription,
                                                   bAllowCycleFocusOut, nLOKWindowId);
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

void InterimItemWindow::UnclipVisibleSysObj()
{
    if (!IsVisible())
        return;
    vcl::Window* pChild = m_xVclContentArea->GetWindow(GetWindowType::FirstChild);
    if (!pChild)
        return;
    WindowImpl* pWindowImpl = pChild->ImplGetWindowImpl();
    if (!pWindowImpl)
        return;
    if (!pWindowImpl->mpSysObj)
        return;
    pWindowImpl->mpSysObj->Show(true);
    pWindowImpl->mpSysObj->ResetClipRegion();
    // flag that sysobj clip is dirty and needs to be recalculated on next use
    pWindowImpl->mbInitWinClipRegion = true;
}

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

void InterimItemWindow::InvalidateChildSizeCache()
{
    // find the bottom vcl::Window of the hierarchy and queue_resize on that
    // one will invalidate all the size caches upwards
    vcl::Window* pChild = GetWindow(GetWindowType::FirstChild);
    while (true)
    {
        vcl::Window* pSubChild = pChild->GetWindow(GetWindowType::FirstChild);
        if (!pSubChild)
            break;
        pChild = pSubChild;
    }
    pChild->queue_resize();
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
    // tdf#157738 Don't grab focus to the other widget hierarchy if the parent has
    // captured the mouse in order to avoid breaking the capture.
    ImplSVWinData* pWinData = ImplGetSVData()->mpWinData;
    const bool bParentHasCapturedMouse
        = pWinData->mpCaptureWin && pWinData->mpCaptureWin->ImplIsChild(this);
    if (m_pWidget && !bParentHasCapturedMouse)
        m_pWidget->grab_focus();

    /* let toolbox know this item window has focus so it updates its mnHighItemId to point
       to this toolitem in case tab means to move to another toolitem within
       the toolbox
    */
    vcl::Window* pToolBox = GetParent();
    NotifyEvent aNEvt(NotifyEventType::GETFOCUS, this);
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
    NotifyEvent aNEvt(NotifyEventType::GETFOCUS, this);
    pToolBox->EventNotify(aNEvt);

    /* send parent the tab */
    pToolBox->KeyInput(rKEvt);

    return true;
}

void InterimItemWindow::Draw(OutputDevice* pDevice, const Point& rPos,
                             SystemTextColorFlags /*nFlags*/)
{
    m_xContainer->draw(*pDevice, rPos, GetSizePixel());
}

void InterimItemWindow::SetPriority(TaskPriority nPriority)
{
    // Eliminate warning when changing timer's priority
    // Task::SetPriority() expects the timer to be stopped while
    // changing the timer's priority.
    bool bActive = m_aLayoutIdle.IsActive();
    if (bActive)
        m_aLayoutIdle.Stop();
    m_aLayoutIdle.SetPriority(nPriority);
    if (bActive)
        m_aLayoutIdle.Start();
}

void InterimItemWindow::ImplPaintToDevice(::OutputDevice* pTargetOutDev, const Point& rPos)
{
    Draw(pTargetOutDev, rPos, SystemTextColorFlags::NONE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
