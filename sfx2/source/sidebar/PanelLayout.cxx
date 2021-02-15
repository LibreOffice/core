/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/TabBar.hxx>
#include <sfx2/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>
#include <vcl/accel.hxx>

using namespace sfx2::sidebar;

PanelLayout::PanelLayout(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
                         const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Control(pParent)
    , m_pInitialFocusWidget(nullptr)
    , m_bInClose(false)
    , mxFrame(rFrame)
{
    m_aPanelLayoutIdle.SetPriority(TaskPriority::RESIZE);
    m_aPanelLayoutIdle.SetInvokeHandler( LINK( this, PanelLayout, ImplHandlePanelLayoutTimerHdl ) );
    m_aPanelLayoutIdle.SetDebugName("sfx2::PanelLayout m_aPanelLayoutIdle");

    SetStyle(GetStyle() | WB_DIALOGCONTROL);

    // Builder will trigger resize and start Idle
    m_xVclContentArea = VclPtr<VclVBox>::Create(this);
    m_xVclContentArea->Show();
    m_xBuilder.reset(Application::CreateInterimBuilder(m_xVclContentArea, rUIXMLDescription, true));
    m_xContainer = m_xBuilder->weld_container(rID);
}

void PanelLayout::GetFocus()
{
    Control::GetFocus();
    if (m_pInitialFocusWidget)
        m_pInitialFocusWidget->grab_focus();
}

PanelLayout::~PanelLayout()
{
    disposeOnce();
}

void PanelLayout::dispose()
{
    m_bInClose = true;
    m_pInitialFocusWidget = nullptr;
    m_aPanelLayoutIdle.Stop();
    m_xContainer.reset();
    m_xBuilder.reset();
    m_xVclContentArea.disposeAndClear();
    Control::dispose();
}

Size PanelLayout::GetOptimalSize() const
{
    Size aSize = m_xContainer->get_preferred_size();

    if (mxFrame)
    {
        SidebarController* pController
            = SidebarController::GetSidebarControllerForFrame(mxFrame);
        if (pController)
            aSize.setWidth(std::min<tools::Long>(
                aSize.Width(), (pController->getMaximumWidth() - TabBar::GetDefaultWidth())
                                   * GetDPIScaleFactor()));
    }

    return aSize;
}

void PanelLayout::queue_resize(StateChangedType /*eReason*/)
{
    if (m_bInClose)
        return;
    if (m_aPanelLayoutIdle.IsActive())
        return;
    InvalidateSizeCache();
    m_aPanelLayoutIdle.Start();
}

IMPL_LINK_NOARG( PanelLayout, ImplHandlePanelLayoutTimerHdl, Timer*, void )
{
    vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
    assert(pChild);
    VclContainer::setLayoutAllocation(*pChild, Point(0, 0), GetSizePixel());
}

void PanelLayout::setPosSizePixel(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags)
{
    bool bCanHandleSmallerWidth = false;
    bool bCanHandleSmallerHeight = false;

    vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);

    if (pChild->GetType() == WindowType::SCROLLWINDOW)
    {
        WinBits nStyle = pChild->GetStyle();
        if (nStyle & (WB_AUTOHSCROLL | WB_HSCROLL))
            bCanHandleSmallerWidth = true;
        if (nStyle & (WB_AUTOVSCROLL | WB_VSCROLL))
            bCanHandleSmallerHeight = true;
    }

    Size aSize(GetOptimalSize());
    if (!bCanHandleSmallerWidth)
        nWidth = std::max(nWidth,aSize.Width());
    if (!bCanHandleSmallerHeight)
        nHeight = std::max(nHeight,aSize.Height());

    Control::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);

    if (nFlags & PosSizeFlags::Size)
        VclContainer::setLayoutAllocation(*pChild, Point(0, 0), Size(nWidth, nHeight));
}

bool PanelLayout::EventNotify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
        Accelerator::ToggleMnemonicsOnHierarchy(*rNEvt.GetCommandEvent(), this);
    return Control::EventNotify( rNEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
