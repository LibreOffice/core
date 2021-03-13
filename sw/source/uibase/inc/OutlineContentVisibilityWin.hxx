/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/InterimItemWindow.hxx>
#include "edtwin.hxx"
#include "FrameControl.hxx"

enum class ButtonSymbol
{
    SHOW,
    HIDE,
    NONE
};

class SwOutlineContentVisibilityWin : public InterimItemWindow, public ISwFrameControl
{
private:
    std::unique_ptr<weld::Button> m_xShowBtn;
    std::unique_ptr<weld::Button> m_xHideBtn;

    VclPtr<SwEditWin> m_pEditWin;
    const SwFrame* m_pFrame;
    int m_nDelayAppearing; ///< Before we show the control, wait a few timer ticks to avoid appearing with every mouse over.
    Timer m_aDelayTimer;
    bool m_bDestroyed;
    size_t m_nOutlinePos;

    void ToggleOutlineContentVisibility(const bool bSubs);

public:
    SwOutlineContentVisibilityWin(SwEditWin* pEditWin, const SwFrame* pFrame);
    virtual ~SwOutlineContentVisibilityWin() override { disposeOnce(); }
    virtual void dispose() override;

    virtual void ShowAll(bool bShow) override;
    virtual bool Contains(const Point& rDocPt) const override;
    virtual void SetReadonly(bool /*bReadonly*/) override {}
    virtual bool IsFocused() const override { return ControlHasFocus(); }
    virtual const SwFrame* GetFrame() override { return m_pFrame; }
    virtual SwEditWin* GetEditWin() override { return m_pEditWin; }

    void Set();

    void SetSymbol(ButtonSymbol eTyle);
    ButtonSymbol GetSymbol() const;

private:
    DECL_LINK(DelayAppearHandler, Timer*, void);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
