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

#include <vcl/builderfactory.hxx>
#include <vcl/layout.hxx>
#include <sfx2/viewfrm.hxx>
#include "PriorityHBox.hxx"
#include <comphelper/lok.hxx>

namespace
{
    bool lcl_comparePriority(const vcl::IPrioritable* a, const vcl::IPrioritable* b)
    {
        return a->GetPriority() < b->GetPriority();
    }
}

PriorityHBox::PriorityHBox(vcl::Window *pParent)
    : VclHBox(pParent)
    , m_bInitialized(false)
{
}

PriorityHBox::~PriorityHBox()
{
    disposeOnce();
}

void PriorityHBox::Initialize()
{
    m_bInitialized = true;

    GetChildrenWithPriorities();
    SetSizeFromParent();
}

int PriorityHBox::GetHiddenCount() const
{
    int nCount = 0;

    for (auto pWindow : m_aSortedChildren)
        if (pWindow->IsHidden())
            nCount++;

    return nCount;
}

void PriorityHBox::SetSizeFromParent()
{
    vcl::Window* pParent = GetParent();
    if (pParent)
    {
        Size aParentSize = pParent->GetSizePixel();
        SetSizePixel(Size(aParentSize.getWidth(), aParentSize.getHeight()));
    }
}

Size PriorityHBox::calculateRequisition() const
{
    if (!m_bInitialized)
    {
        return VclHBox::calculateRequisition();
    }

    sal_uInt16 nVisibleChildren = 0;

    Size aSize;
    for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
    {
        if (!pChild->IsVisible())
            continue;
        ++nVisibleChildren;
        Size aChildSize = getLayoutRequisition(*pChild);

        bool bAlwaysExpanded = true;

        vcl::IPrioritable* pPrioritable = dynamic_cast<vcl::IPrioritable*>(pChild);
        if (pPrioritable && pPrioritable->GetPriority() != VCL_PRIORITY_DEFAULT)
            bAlwaysExpanded = false;

        if (bAlwaysExpanded)
        {
            long nPrimaryDimension = getPrimaryDimension(aChildSize);
            nPrimaryDimension += pChild->get_padding() * 2;
            setPrimaryDimension(aChildSize, nPrimaryDimension);
        }
        else
            setPrimaryDimension(aChildSize, 0);

        accumulateMaxes(aChildSize, aSize);
    }

    return finalizeMaxes(aSize, nVisibleChildren);
}

void PriorityHBox::Resize()
{
    if (!m_bInitialized && SfxViewFrame::Current())
        Initialize();

    if (!m_bInitialized || comphelper::LibreOfficeKit::isActive())
    {
        return VclHBox::Resize();
    }

    long nWidth = GetSizePixel().Width();
    long nCurrentWidth = VclHBox::calculateRequisition().getWidth();

    // Hide lower priority controls
    for (vcl::IPrioritable* pPrioritable : m_aSortedChildren)
    {
        if (nCurrentWidth <= nWidth)
            break;

        vcl::Window* pWindow = dynamic_cast<vcl::Window*>(pPrioritable);

        if (pWindow && pWindow->GetParent() == this)
        {
            nCurrentWidth -= pWindow->GetOutputWidthPixel() + get_spacing();
            pWindow->Show();
            pPrioritable->HideContent();
            nCurrentWidth += pWindow->GetOutputWidthPixel() + get_spacing();
        }
    }

    auto pChildR = m_aSortedChildren.rbegin();
    // Show higher priority controls if we already have enough space
    while (pChildR != m_aSortedChildren.rend())
    {
        vcl::Window* pWindow = dynamic_cast<vcl::Window*>(*pChildR);
        vcl::IPrioritable* pPrioritable = *pChildR;

        if(pWindow->GetParent() != this)
        {
            pChildR++;
            continue;
        }

        if (pWindow)
        {
            nCurrentWidth -= pWindow->GetOutputWidthPixel() + get_spacing();
            pWindow->Show();
            pPrioritable->ShowContent();
            nCurrentWidth += getLayoutRequisition(*pWindow).Width() + get_spacing();

            if (nCurrentWidth > nWidth)
            {
                pPrioritable->HideContent();
                break;
            }
        }

        pChildR++;
    }

    VclHBox::Resize();
}

void PriorityHBox::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!m_bInitialized && SfxViewFrame::Current())
        Initialize();

    VclHBox::Paint(rRenderContext, rRect);
}

void PriorityHBox::GetChildrenWithPriorities()
{
    for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
    {
        vcl::Window* pChild = GetChild(i);

        // Add only containers which have explicitly assigned priority.
        vcl::IPrioritable* pPrioritable = dynamic_cast<vcl::IPrioritable*>(pChild);
        if (pPrioritable && pPrioritable->GetPriority() != VCL_PRIORITY_DEFAULT)
            m_aSortedChildren.push_back(pPrioritable);
    }

    if (m_aSortedChildren.empty())
        m_bInitialized = false;

    std::sort(m_aSortedChildren.begin(), m_aSortedChildren.end(), lcl_comparePriority);
}

VCL_BUILDER_FACTORY(PriorityHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
