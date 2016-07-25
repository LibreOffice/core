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
 * PriorityHBox is a VclHBox which hides own childs if there is no sufficient space.
 * Hiding order can be modified using child's priorities. If a control have default
 * priority assigned (VCL_PRIORITY_DEFAULT), it is always shown.
 */

class SFX2_DLLPUBLIC PriorityHBox : public VclHBox
{
private:
    bool m_bInitialized;
    long m_nNeededWidth;

    std::vector<IPrioritable*> m_aSortedChilds;

public:
    explicit PriorityHBox(vcl::Window *pParent)
        : VclHBox(pParent)
        , m_bInitialized(false)
        , m_nNeededWidth(0)
    {
    }

    virtual ~PriorityHBox() override
    {
        disposeOnce();
    }

    virtual void dispose() override
    {
        if (m_bInitialized && SfxViewFrame::Current())
        {
            SystemWindow* pSystemWindow = SfxViewFrame::Current()->GetFrame().GetSystemWindow();
            pSystemWindow->RemoveEventListener(LINK(this, PriorityHBox, WindowEventListener));
        }
        VclHBox::dispose();
    }

    virtual void Resize() override
    {
        long nWidth = GetSizePixel().Width();
        long nCurrentWidth = m_nNeededWidth;

        // Hide lower priority controls
        auto pChild = m_aSortedChilds.begin();
        while (nCurrentWidth > nWidth && pChild != m_aSortedChilds.end())
        {
            DropdownBox* pContainer = static_cast<DropdownBox*>(*pChild);
            nCurrentWidth -= pContainer->GetSizePixel().Width() + get_spacing();
            pContainer->HideContent();
            nCurrentWidth += pContainer->GetSizePixel().Width() + get_spacing();
            pChild++;
        }

        // Show higher priority controls if we already have enough space
        while (pChild != m_aSortedChilds.end())
        {
            static_cast<DropdownBox*>(*pChild)->ShowContent();
            pChild++;
        }

        VclHBox::Resize();
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override
    {
        if (!m_bInitialized && SfxViewFrame::Current())
        {
            m_bInitialized = true;

            SystemWindow* pSystemWindow = SfxViewFrame::Current()->GetFrame().GetSystemWindow();
            pSystemWindow->AddEventListener(LINK(this, PriorityHBox, WindowEventListener));

            CalcNeededWidth();

            long nWidth = pSystemWindow->GetSizePixel().Width();
            SetSizePixel(Size(nWidth, GetSizePixel().Height()));
        }

        VclHBox::Paint(rRenderContext, rRect);
    }

    void CalcNeededWidth()
    {
        int spacing = get_spacing();

        for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
        {
            vcl::Window* pChild = GetChild(i);
            m_nNeededWidth += pChild->GetSizePixel().Width() + spacing;

            // Add only containers which have explicitly assigned priority.
            if (pChild->GetType() == WINDOW_CONTAINER)
            {
                IPrioritable* pPrioritable = dynamic_cast<IPrioritable*>(pChild);
                if (pPrioritable->GetPriority() != VCL_PRIORITY_DEFAULT)
                    m_aSortedChilds.push_back(pPrioritable);
            }
        }

        std::sort(m_aSortedChilds.begin(), m_aSortedChilds.end(), lcl_comparePriority);
    }

private:
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );
};

IMPL_LINK_TYPED( PriorityHBox, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if (rEvent.GetId() == VCLEVENT_WINDOW_RESIZE)
    {
        vcl::Window* pEventWindow = rEvent.GetWindow();

        OSL_ENSURE(pEventWindow, "PriorityHBox::WindowEventListener: no window!");

        long nWidth = pEventWindow->GetSizePixel().Width();
        SetSizePixel(Size(nWidth, GetSizePixel().Height()));
    }
}

VCL_BUILDER_FACTORY(PriorityHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
