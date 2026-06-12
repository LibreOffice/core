/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "svimpbox.hxx"
#include "svtabbx.hxx"

class LclTabListBox final : public SvTabListBox
{
    Link<SvTreeListBox*, bool> m_aStartDragHdl;
    Link<SvTreeListBox*, void> m_aEndDragHdl;

public:
    LclTabListBox(vcl::Window* pParent, WinBits nWinStyle)
        : SvTabListBox(pParent, nWinStyle)
    {
    }

    void SetStartDragHdl(const Link<SvTreeListBox*, bool>& rLink) { m_aStartDragHdl = rLink; }
    void SetEndDragHdl(const Link<SvTreeListBox*, void>& rLink) { m_aEndDragHdl = rLink; }

    virtual void StartDrag(sal_Int8 nAction, const Point& rPosPixel) override
    {
        if (m_aStartDragHdl.Call(this))
            return;
        SvTabListBox::StartDrag(nAction, rPosPixel);
    }

    virtual void DragFinished(sal_Int8 nDropAction) override
    {
        SvTabListBox::DragFinished(nDropAction);
        m_aEndDragHdl.Call(this);
    }

    SvTreeListEntry* GetTargetAtPoint(const Point& rPos, bool bHighLightTarget, bool bScroll = true)
    {
        SvTreeListEntry* pOldTargetEntry = m_pTargetEntry;
        m_pTargetEntry = PosOverBody(rPos) ? m_pImpl->GetEntry(rPos) : nullptr;
        if (pOldTargetEntry != m_pTargetEntry)
            ImplShowTargetEmphasis(pOldTargetEntry, false);

        if (bScroll)
        {
            // scroll
            if (rPos.Y() < 12)
            {
                ImplShowTargetEmphasis(m_pTargetEntry, false);
                ScrollOutputArea(+1);
            }
            else
            {
                Size aSize(m_pImpl->GetOutputSize());
                if (rPos.Y() > aSize.Height() - 12)
                {
                    ImplShowTargetEmphasis(m_pTargetEntry, false);
                    ScrollOutputArea(-1);
                }
            }
        }

        if (m_pTargetEntry && bHighLightTarget)
            ImplShowTargetEmphasis(m_pTargetEntry, true);
        return m_pTargetEntry;
    }

    virtual SvTreeListEntry* GetDropTarget(const Point& rPos) override
    {
        return GetTargetAtPoint(rPos, true);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
