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

#include <vector>

/*
 * PriorityHBox is a VclHBox which hides own childs if there is no sufficient space.
 * Hiding order can be modified using child's priorities. If a control have default
 * priority assigned (VCL_PRIORITY_DEFAULT), it is always shown.
 */

class SFX2_DLLPUBLIC PriorityHBox : public VclHBox
{
private:
    bool m_bInitialized;
    long m_nCurrentWidth;
    long m_nNeededWidth;

    std::vector<IPrioritable*> m_aSortedChilds;

public:
    PriorityHBox(vcl::Window *pParent)
        : VclHBox(pParent)
        , m_bInitialized(false)
        , m_nCurrentWidth(0)
        , m_nNeededWidth(0)
    {
    }

    virtual ~PriorityHBox() override
    {
        // TODO
        // GetSystemWindow()->RemoveEventListener( LINK( this, PriorityHBox, WindowEventListener ) );
    }

    virtual void Resize() override
    {
        long nWidth = GetSizePixel().Width();

        m_nCurrentWidth = m_nNeededWidth;

        // Hide lower priority controls
        auto pChild = m_aSortedChilds.begin();
        while (m_nCurrentWidth > nWidth && pChild != m_aSortedChilds.end())
        {
            VclContainer* pContainer = static_cast<VclContainer*>(*pChild);
            m_nCurrentWidth -= pContainer->GetSizePixel().Width() + get_spacing();
            pContainer->Hide();
            pChild++;
        }

        // Show higher priority controls if we already have enough space
        while (pChild != m_aSortedChilds.end())
        {
            static_cast<VclContainer*>(*pChild)->Show();
            pChild++;
        }

        VclHBox::Resize();
    }

    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) SAL_OVERRIDE
    {
        if (!m_bInitialized && SfxViewFrame::Current())
        {
            SystemWindow* pSystemWindow = SfxViewFrame::Current()->GetFrame().GetSystemWindow();
            pSystemWindow->AddEventListener(LINK(this, PriorityHBox, WindowEventListener));
            m_bInitialized = true;
        }

        VclHBox::setPosSizePixel(nX, nY, nWidth, nHeight, nFlags);
    }

private:
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );
};

bool lcl_comparePriority(const vcl::IPrioritable* a, const vcl::IPrioritable* b)
{
    return a->GetPriority() < b->GetPriority();
}

IMPL_LINK_TYPED( PriorityHBox, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if (rEvent.GetId() == VCLEVENT_WINDOW_RESIZE)
    {
        vcl::Window* pEventWindow = rEvent.GetWindow();

        OSL_ENSURE(pEventWindow, "PriorityHBox::WindowEventListener: no window!");

        long nWidth = pEventWindow->GetSizePixel().Width();
        SetSizePixel(LogicToPixel(Size(nWidth, GetSizePixel().Height())));

        if (m_nNeededWidth <= 0)
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
            m_nCurrentWidth = m_nNeededWidth;
        }
    }
}

VCL_BUILDER_FACTORY(PriorityHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
