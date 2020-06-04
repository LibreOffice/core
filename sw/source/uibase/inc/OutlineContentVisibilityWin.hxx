/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_OUTLINECONTENTVISIBILITYWIN_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_OUTLINECONTENTVISIBILITYWIN_HXX

#include "edtwin.hxx"
#include "FrameControl.hxx"
#include <vcl/timer.hxx>

class SwOutlineContentVisibilityWin : public PushButton, public ISwFrameControl
{
private:
    VclPtr<SwEditWin> m_pEditWin;
    const SwFrame* m_pFrame;
    bool m_bIsAppearing;
    int m_nDelayAppearing; ///< Before we show the control, wait a few timer ticks to avoid appearing with every mouse over.
    Timer m_aDelayTimer;
    bool m_bDestroyed;
    size_t m_nOutlinePos;

public:
    SwOutlineContentVisibilityWin(SwEditWin* pEditWin, const SwFrame* pFrame);
    virtual ~SwOutlineContentVisibilityWin() override { disposeOnce(); }
    virtual void dispose() override;

    virtual void MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void LoseFocus() override { ShowAll(false); }
    virtual void ShowAll(bool bShow) override;
    virtual bool Contains(const Point& rDocPt) const override;
    virtual void SetReadonly(bool bReadonly) override { ShowAll(!bReadonly); }
    virtual const SwFrame* GetFrame() override { return m_pFrame; }
    virtual SwEditWin* GetEditWin() override { return m_pEditWin; }

    void Set();

private:
    DECL_LINK(DelayHandler, Timer*, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
