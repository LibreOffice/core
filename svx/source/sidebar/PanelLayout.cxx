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
#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>

PanelLayout::PanelLayout(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const com::sun::star::uno::Reference<com::sun::star::frame::XFrame> &rFrame)
    : Control(pParent)
    , m_bInClose(false)
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame);
    m_aPanelLayoutTimer.SetTimeout(50);
    m_aPanelLayoutTimer.SetTimeoutHdl( LINK( this, PanelLayout, ImplHandlePanelLayoutTimerHdl ) );
}

PanelLayout::~PanelLayout()
{
    m_bInClose = true;
    m_aPanelLayoutTimer.Stop();
}

Size PanelLayout::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(WINDOW_FIRSTCHILD));

    return Control::GetOptimalSize();
}

bool PanelLayout::hasPanelPendingLayout() const
{
    return m_aPanelLayoutTimer.IsActive();
}

void PanelLayout::queue_resize()
{
    if (m_bInClose)
        return;
    if (hasPanelPendingLayout())
        return;
    if (!isLayoutEnabled(this))
        return;
    m_aPanelLayoutTimer.Start();
}

IMPL_LINK( PanelLayout, ImplHandlePanelLayoutTimerHdl, void*, EMPTYARG )
{
    Window *pChild = GetWindow(WINDOW_FIRSTCHILD);
    assert(pChild);
    VclContainer::setLayoutAllocation(*pChild, Point(0, 0), GetSizePixel());
    return 0;
}

void PanelLayout::setPosSizePixel(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags)
{
    bool bCanHandleSmallerWidth = false;
    bool bCanHandleSmallerHeight = false;

    bool bIsLayoutEnabled = isLayoutEnabled(this);
    Window *pChild = GetWindow(WINDOW_FIRSTCHILD);

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

    if (bIsLayoutEnabled && (nFlags & WINDOW_POSSIZE_SIZE))
        VclContainer::setLayoutAllocation(*pChild, Point(0, 0), Size(nWidth, nHeight));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
