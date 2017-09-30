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
#include <sfx2/dllapi.h>
#include <sfx2/viewfrm.hxx>
#include "DropdownBox.hxx"

#include <vector>

bool lcl_comparePriority(const vcl::IPrioritable* a, const vcl::IPrioritable* b)
{
    return a->GetPriority() < b->GetPriority();
}

/*
 * PriorityHBox is a VclHBox which hides its own children if there is no sufficient space.
 * Hiding order can be modified using child's priorities. If a control have default
 * priority assigned (VCL_PRIORITY_DEFAULT), it is always shown.
 */

class PriorityHBox : public VclHBox
{
private:
    bool m_bInitialized;

    std::vector<vcl::IPrioritable*> m_aSortedChilds;

public:
    explicit PriorityHBox(vcl::Window *pParent)
        : VclHBox(pParent)
        , m_bInitialized(false)
    {
    }

    virtual ~PriorityHBox() override
    {
        disposeOnce();
    }

    virtual Size calculateRequisition() const override
    {
        sal_uInt16 nVisibleChildren = 0;

        Size aSize;
        for (vcl::Window *pChild = GetWindow(GetWindowType::FirstChild); pChild; pChild = pChild->GetWindow(GetWindowType::Next))
        {
            if (!pChild->IsVisible())
                continue;
            ++nVisibleChildren;
            Size aChildSize = getLayoutRequisition(*pChild);

            bool bAllwaysExpanded = true;

            vcl::IPrioritable* pPrioritable = dynamic_cast<vcl::IPrioritable*>(pChild);
            if (pPrioritable && pPrioritable->GetPriority() != VCL_PRIORITY_DEFAULT)
                bAllwaysExpanded = false;

            if (bAllwaysExpanded)
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

    virtual void Resize() override
    {
        long nWidth = GetSizePixel().Width();
        long nCurrentWidth = VclHBox::calculateRequisition().getWidth();

        // Hide lower priority controls
        auto pChild = m_aSortedChilds.begin();
        while (nCurrentWidth > nWidth && pChild != m_aSortedChilds.end())
        {
            // ATM DropdownBox is the only one derived class from IPrioritable
            DropdownBox* pDropdownBox = static_cast<DropdownBox*>(*pChild);

            nCurrentWidth -= pDropdownBox->GetOutputWidthPixel() + get_spacing();
            pDropdownBox->HideContent();
            nCurrentWidth += pDropdownBox->GetOutputWidthPixel() + get_spacing();

            pChild++;
        }

        auto pChildR = m_aSortedChilds.rbegin();
        // Show higher priority controls if we already have enough space
        while (pChildR != m_aSortedChilds.rend())
        {
            DropdownBox* pBox = static_cast<DropdownBox*>(*pChildR);

            nCurrentWidth -= pBox->GetOutputWidthPixel() + get_spacing();
            pBox->ShowContent();
            nCurrentWidth += getLayoutRequisition(*pBox).Width() + get_spacing();

            if (nCurrentWidth > nWidth)
            {
                pBox->HideContent();
                break;
            }

            pChildR++;
        }

        VclHBox::Resize();
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override
    {
        if (!m_bInitialized && SfxViewFrame::Current())
        {
            m_bInitialized = true;

            GetChildrenWithPriorities();

            SystemWindow* pSystemWindow = SfxViewFrame::Current()->GetFrame().GetSystemWindow();
            if (pSystemWindow)
            {
                long nWidth = pSystemWindow->GetSizePixel().Width();
                SetSizePixel(Size(nWidth, GetSizePixel().Height()));
            }
        }

        VclHBox::Paint(rRenderContext, rRect);
    }

    void GetChildrenWithPriorities()
    {
        for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
        {
            vcl::Window* pChild = GetChild(i);

            // Add only containers which have explicitly assigned priority.
            vcl::IPrioritable* pPrioritable = dynamic_cast<vcl::IPrioritable*>(pChild);
            if (pPrioritable && pPrioritable->GetPriority() != VCL_PRIORITY_DEFAULT)
                m_aSortedChilds.push_back(pPrioritable);
        }

        std::sort(m_aSortedChilds.begin(), m_aSortedChilds.end(), lcl_comparePriority);
    }
};

VCL_BUILDER_FACTORY(PriorityHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
