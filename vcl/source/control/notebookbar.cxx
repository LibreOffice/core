/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/layout.hxx>
#include <vcl/notebookbar.hxx>

NotebookBar::NotebookBar(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame)
    : Control(pParent)
{
    SetStyle(GetStyle() | WB_DIALOGCONTROL);
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID, rFrame);
}

NotebookBar::~NotebookBar()
{
    disposeOnce();
}

void NotebookBar::dispose()
{
    disposeBuilder();
    Control::dispose();
}

Size NotebookBar::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(GetWindowType::FirstChild));

    return Control::GetOptimalSize();
}

void NotebookBar::setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags)
{
    bool bCanHandleSmallerWidth = false;
    bool bCanHandleSmallerHeight = false;

    bool bIsLayoutEnabled = isLayoutEnabled(this);
    Window *pChild = GetWindow(GetWindowType::FirstChild);

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
        nWidth = std::max(nWidth, aSize.Width());
    if (!bCanHandleSmallerHeight)
        nHeight = std::max(nHeight, aSize.Height());

    Control::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);

    if (bIsLayoutEnabled && (nFlags & PosSizeFlags::Size))
        VclContainer::setLayoutAllocation(*pChild, Point(0, 0), Size(nWidth, nHeight));
}

void NotebookBar::StateChanged(StateChangedType nType)
{
    if (nType == StateChangedType::Visible)
    {
        // visibility changed, update the container
        GetParent()->Resize();
    }

    Control::StateChanged(nType);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
