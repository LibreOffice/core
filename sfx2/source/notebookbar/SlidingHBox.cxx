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

/*
 * SlidingHBox is a VclHBox which hides own childs if there is no sufficient space.
 * Hiding order can be modified using child's priorities. If a control have default
 * priority assigned (VCL_PRIORITY_DEFAULT), it is always shown.
 */

class SFX2_DLLPUBLIC SlidingHBox : public VclHBox
{
private:
    bool m_bInitialized;
    long m_nNeededWidth;

    std::vector<IPrioritable*> m_aSortedChilds;

    VclPtr<PushButton> m_pLeft;
    VclPtr<PushButton> m_pRight;

public:
    explicit SlidingHBox(vcl::Window *pParent)
        : VclHBox(pParent)
        , m_bInitialized(false)
        , m_nNeededWidth(0)
    {
        m_pLeft = VclPtr<PushButton>::Create(this);
        m_pLeft->SetText("<");
        m_pLeft->SetClickHdl(LINK(this, SlidingHBox, Slide));
        m_pRight = VclPtr<PushButton>::Create(this);
        m_pRight->SetText(">");
        m_pRight->SetClickHdl(LINK(this, SlidingHBox, Slide));
    }

    virtual ~SlidingHBox() override
    {
        disposeOnce();
    }

    virtual void dispose() override
    {
        if (m_bInitialized && SfxViewFrame::Current())
        {
            SystemWindow* pSystemWindow = SfxViewFrame::Current()->GetFrame().GetSystemWindow();
            pSystemWindow->RemoveEventListener(LINK(this, SlidingHBox, WindowEventListener));
        }
        m_pLeft.disposeAndClear();
        m_pRight.disposeAndClear();
        VclHBox::dispose();
    }

    virtual void Resize() override
    {
        Point aPos(m_pRight->GetPosPixel());
        aPos.X() = GetSizePixel().Width() - m_pRight->GetSizePixel().Width() - 6;
        m_pRight->SetPosPixel(aPos);

        long nCurentWidth = GetSizePixel().Width();

        if( m_nNeededWidth > nCurentWidth )
        {
            m_pLeft->Show();
            m_pRight->Show();

            for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
            {
                vcl::Window* pChild = GetChild(i);
                if ( pChild != m_pLeft.get() && pChild != m_pRight.get() )
                {
                    Point aPos2 = pChild->GetPosPixel();
                    aPos2.X() -= m_pRight->GetSizePixel().Width();
                    pChild->SetPosPixel(aPos2);
                }
            }
        }
        else
        {
            m_pLeft->Hide();
            m_pRight->Hide();
        }

        m_pRight->Invalidate();
        m_pLeft->Invalidate();

        VclHBox::Resize();
    }

    virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect) override
    {
        if (!m_bInitialized && SfxViewFrame::Current())
        {
            m_bInitialized = true;

            SystemWindow* pSystemWindow = SfxViewFrame::Current()->GetFrame().GetSystemWindow();
            pSystemWindow->AddEventListener(LINK(this, SlidingHBox, WindowEventListener));

            CalcNeededWidth();

            long nWidth = pSystemWindow->GetSizePixel().Width();
            SetSizePixel(Size(nWidth, GetSizePixel().Height()));
            Resize();
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
        }
    }

private:
    DECL_LINK_TYPED( WindowEventListener, VclWindowEvent&, void );
    DECL_LINK_TYPED( Slide, Button*, void );
};

IMPL_LINK_TYPED( SlidingHBox, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    if (rEvent.GetId() == VCLEVENT_WINDOW_RESIZE)
    {
        vcl::Window* pEventWindow = rEvent.GetWindow();

        OSL_ENSURE(pEventWindow, "PriorityHBox::WindowEventListener: no window!");

        long nWidth = pEventWindow->GetSizePixel().Width();
        SetSizePixel(Size(nWidth, GetSizePixel().Height()));
        Resize();
    }
}

IMPL_LINK_TYPED( SlidingHBox, Slide, Button*, pButton, void )
{
    if ( pButton == m_pRight.get() )
    {
        for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
        {
            vcl::Window* pChild = GetChild(i);
            if ( pChild != m_pLeft.get() && pChild != m_pRight.get() )
            {
                Point aPos = pChild->GetPosPixel();
                aPos.X() -= 20;
                pChild->SetPosPixel(aPos);
            }
        }
    }
    else
    {
        for (sal_uInt16 i = 0; i < GetChildCount(); ++i)
        {
            vcl::Window* pChild = GetChild(i);
            if ( pChild != m_pLeft.get() && pChild != m_pRight.get() )
            {
                Point aPos = pChild->GetPosPixel();
                aPos.X() += 20;
                pChild->SetPosPixel(aPos);
            }
        }
    }
}

VCL_BUILDER_FACTORY(SlidingHBox)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
