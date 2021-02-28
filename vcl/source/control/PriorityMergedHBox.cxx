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

#include <vcl/toolkit/button.hxx>
#include <vcl/layout.hxx>
#include <bitmaps.hlst>
#include <NotebookbarPopup.hxx>
#include <PriorityHBox.hxx>
#include <PriorityMergedHBox.hxx>
#include <comphelper/lok.hxx>

#define DUMMY_WIDTH 50
#define BUTTON_WIDTH 30

/*
* PriorityMergedHBox is a VclHBox which hides its own children if there is no sufficient space.
*/

PriorityMergedHBox::PriorityMergedHBox(vcl::Window* pParent)
    : PriorityHBox(pParent)
{
    m_pButton = VclPtr<PushButton>::Create(this, WB_FLATBUTTON);
    m_pButton->SetClickHdl(LINK(this, PriorityMergedHBox, PBClickHdl));
    m_pButton->SetModeImage(Image(StockImage::Yes, CHEVRON));
    m_pButton->set_width_request(25);
    m_pButton->set_pack_type(VclPackType::End);
    m_pButton->Show();
}

void PriorityMergedHBox::Resize()
{
    if (comphelper::LibreOfficeKit::isActive())
        return VclHBox::Resize();

    if (!m_bInitialized)
        Initialize();

    if (!m_bInitialized)
    {
        return VclHBox::Resize();
    }

    tools::Long nWidth = GetSizePixel().Width();
    tools::Long nCurrentWidth = VclHBox::calculateRequisition().getWidth() + BUTTON_WIDTH;

    // Hide lower priority controls
    for (int i = GetChildCount() - 1; i >= 0; i--)
    {
        vcl::Window* pWindow = GetChild(i);

        if (nCurrentWidth <= nWidth)
            break;

        if (pWindow && pWindow->GetParent() == this && pWindow->IsVisible())
        {
            if (pWindow->GetOutputWidthPixel())
                nCurrentWidth -= pWindow->GetOutputWidthPixel();
            else
                nCurrentWidth -= DUMMY_WIDTH;
            pWindow->Hide();
        }
    }

    // Show higher priority controls if we already have enough space
    for (int i = 0; i < GetChildCount(); i++)
    {
        vcl::Window* pWindow = GetChild(i);

        if (pWindow->GetParent() != this)
        {
            continue;
        }

        if (pWindow && !pWindow->IsVisible())
        {
            pWindow->Show();
            nCurrentWidth += getLayoutRequisition(*pWindow).Width() + get_spacing();

            if (nCurrentWidth > nWidth)
            {
                pWindow->Hide();
                break;
            }
        }
    }

    VclHBox::Resize();

    if (GetHiddenCount())
        m_pButton->Show();
    else
        m_pButton->Hide();
}

void PriorityMergedHBox::dispose()
{
    m_pButton.disposeAndClear();
    if (m_pPopup)
        m_pPopup.disposeAndClear();
    PriorityHBox::dispose();
}

int PriorityMergedHBox::GetHiddenCount() const
{
    int nCount = 0;

    for (int i = GetChildCount() - 1; i >= 0; i--)
    {
        vcl::Window* pWindow = GetChild(i);
        if (pWindow && pWindow->GetParent() == this && !pWindow->IsVisible())
            nCount++;
    }

    return nCount;
}

Size PriorityMergedHBox::calculateRequisition() const
{
    if (!m_bInitialized)
    {
        return VclHBox::calculateRequisition();
    }

    sal_uInt16 nVisibleChildren = 0;

    Size aSize;
    for (vcl::Window* pChild = GetWindow(GetWindowType::FirstChild); pChild;
         pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = getLayoutRequisition(*pChild);

        tools::Long nPrimaryDimension = getPrimaryDimension(aChildSize);
        nPrimaryDimension += pChild->get_padding() * 2;
        setPrimaryDimension(aChildSize, nPrimaryDimension);

        accumulateMaxes(aChildSize, aSize);
    }

    setPrimaryDimension(aSize, 200);
    return finalizeMaxes(aSize, nVisibleChildren);
}

IMPL_LINK(PriorityMergedHBox, PBClickHdl, Button*, /*pButton*/, void)
{
    if (m_pPopup)
        m_pPopup.disposeAndClear();

    m_pPopup = VclPtr<NotebookbarPopup>::Create(this);

    for (int i = 0; i < GetChildCount(); i++)
    {
        vcl::Window* pWindow = GetChild(i);
        if (pWindow != m_pButton)
        {
            if (!pWindow->IsVisible())
            {
                pWindow->Show();
                pWindow->SetParent(m_pPopup->getBox());
                // count is decreased because we moved child
                i--;
            }
        }
    }

    m_pPopup->hideSeparators(true);

    tools::Long x = m_pButton->GetPosPixel().getX();
    tools::Long y = m_pButton->GetPosPixel().getY() + GetSizePixel().Height();
    tools::Rectangle aRect(x, y, x, y);

    m_pPopup->StartPopupMode(aRect, FloatWinPopupFlags::Down | FloatWinPopupFlags::GrabFocus
                                        | FloatWinPopupFlags::AllMouseButtonClose);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
