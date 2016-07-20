/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <sfx2/sidebar/SidebarController.hxx>
#include <sfx2/sidebar/TabBar.hxx>
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>

using namespace sfx2::sidebar;

PanelLayout::PanelLayout(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Control(pParent)
    , m_aPanelLayoutIdle("svx sidebar PanelLayoutIdle")
    , m_bInClose(false)
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame);

    m_aPanelLayoutIdle.SetPriority(SchedulerPriority::RESIZE);
    m_aPanelLayoutIdle.SetIdleHdl( LINK( this, PanelLayout, ImplHandlePanelLayoutTimerHdl ) );
    m_aPanelLayoutIdle.SetDebugName( "svx::PanelLayout  m_aPanelLayoutIdle" );
    if (GetSettings().GetStyleSettings().GetAutoMnemonic())
       Accelerator::GenerateAutoMnemonicsOnHierarchy(this);
}

PanelLayout::~PanelLayout()
{
    disposeOnce();
}

void PanelLayout::dispose()
{
    m_bInClose = true;
    m_aPanelLayoutIdle.Stop();
    disposeBuilder();
    Control::dispose();
}

Size PanelLayout::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
    {
        Size aSize = VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));
        aSize.Width() = std::min<long>(aSize.Width(),
            (SidebarController::gnMaximumSidebarWidth - TabBar::GetDefaultWidth()) * GetDPIScaleFactor());
        return aSize;
    }

    return Control::GetOptimalSize();
}

bool PanelLayout::hasPanelPendingLayout() const
{
    return m_aPanelLayoutIdle.IsActive();
}

void PanelLayout::queue_resize(StateChangedType /*eReason*/)
{
    if (m_bInClose)
        return;
    if (hasPanelPendingLayout())
        return;
    if (!isLayoutEnabled(this))
        return;
    InvalidateSizeCache();
    m_aPanelLayoutIdle.Start();
}

IMPL_LINK_NOARG_TYPED( PanelLayout, ImplHandlePanelLayoutTimerHdl, Idle*, void )
{
    vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);
    assert(pChild);
    VclContainer::setLayoutAllocation(*pChild, Point(0, 0), GetSizePixel());
}

void PanelLayout::setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags)
{
    bool bCanHandleSmallerWidth = false;
    bool bCanHandleSmallerHeight = false;

    bool bIsLayoutEnabled = isLayoutEnabled(this);
    vcl::Window *pChild = GetWindow(GetWindowType::FirstChild);

    if (bIsLayoutEnabled && pChild->GetType() == WINDOW_SCROLLWINDOW)
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

    if (bIsLayoutEnabled && (nFlags & PosSizeFlags::Size))
        VclContainer::setLayoutAllocation(*pChild, Point(0, 0), Size(nWidth, nHeight));
}

bool PanelLayout::Notify(NotifyEvent& rNEvt)
{
    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
        Accelerator::ToggleMnemonicsOnHierarchy(*rNEvt.GetCommandEvent(), this);
    return Control::Notify( rNEvt );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
