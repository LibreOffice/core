/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/sidebar/PanelLayout.hxx>
#include <vcl/layout.hxx>

PanelLayout::PanelLayout(Window* pParent, const OString& rID, const OUString& rUIXMLDescription)
    : Control(pParent)
{
    m_pUIBuilder = new VclBuilder(this, getUIRootDir(), rUIXMLDescription, rID);
}

Size PanelLayout::GetOptimalSize() const
{
    if (isLayoutEnabled(this))
        return VclContainer::getLayoutRequisition(*GetWindow(WINDOW_FIRSTCHILD));

    return Control::GetOptimalSize();
}

void PanelLayout::setPosSizePixel(long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags)
{
    Control::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);

    if (isLayoutEnabled(this) && (nFlags & WINDOW_POSSIZE_SIZE))
        VclContainer::setLayoutAllocation(*GetWindow(WINDOW_FIRSTCHILD), Point(0, 0), Size(nWidth, nHeight));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
