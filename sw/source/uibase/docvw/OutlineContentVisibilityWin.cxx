/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <edtwin.hxx>
#include <OutlineContentVisibilityWin.hxx>
#include <view.hxx>
#include <wrtsh.hxx>

#include <IDocumentOutlineNodes.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <strings.hrc>
#include <svx/svdview.hxx>

#include <viewopt.hxx>

SwOutlineContentVisibilityWin::SwOutlineContentVisibilityWin(SwEditWin* pEditWin,
                                                             const SwFrame* pFrame)
    : InterimItemWindow(pEditWin, "modules/swriter/ui/outlinebutton.ui", "OutlineButton")
    , m_xRightBtn(m_xBuilder->weld_button("right"))
    , m_xDownBtn(m_xBuilder->weld_button("down"))
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
    , m_nDelayAppearing(0)
    , m_bDestroyed(false)
    , m_nOutlinePos(SwOutlineNodes::npos)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    SetPaintTransparent(false);
    SetBackground(rStyleSettings.GetFaceColor());

    Size aBtnsSize(m_xRightBtn->get_preferred_size());
    auto nDim = std::max(aBtnsSize.Width(), aBtnsSize.Height());
    m_xRightBtn->set_size_request(nDim, nDim);
    m_xDownBtn->set_size_request(nDim, nDim);

    SetSizePixel(get_preferred_size());
    SetSymbol(SymbolType::DONTKNOW);

    m_xRightBtn->connect_mouse_press(LINK(this, SwOutlineContentVisibilityWin, MousePressHdl));
    m_xDownBtn->connect_mouse_press(LINK(this, SwOutlineContentVisibilityWin, MousePressHdl));

    m_xRightBtn->connect_key_press(LINK(this, SwOutlineContentVisibilityWin, KeyInputHdl));
    m_xDownBtn->connect_key_press(LINK(this, SwOutlineContentVisibilityWin, KeyInputHdl));

    m_aDelayTimer.SetTimeout(25);
    m_aDelayTimer.SetInvokeHandler(LINK(this, SwOutlineContentVisibilityWin, DelayAppearHandler));
}

void SwOutlineContentVisibilityWin::dispose()
{
    m_bDestroyed = true;
    m_aDelayTimer.Stop();

    m_pEditWin.clear();
    m_pFrame = nullptr;

    m_xDownBtn.reset();
    m_xRightBtn.reset();

    InterimItemWindow::dispose();
}

SymbolType SwOutlineContentVisibilityWin::GetSymbol() const
{
    if (m_xRightBtn->get_visible())
        return SymbolType::ARROW_RIGHT;
    if (m_xDownBtn->get_visible())
        return SymbolType::ARROW_DOWN;
    return SymbolType::DONTKNOW;
}

void SwOutlineContentVisibilityWin::SetSymbol(SymbolType eStyle)
{
    if (GetSymbol() == eStyle)
        return;

    bool bRight = eStyle == SymbolType::ARROW_RIGHT;
    bool bDown = eStyle == SymbolType::ARROW_DOWN;

    bool bControlHasFocus = ControlHasFocus();

    // disable mouse move for the hidden button so we don't get mouse
    // leave events we don't care about when we swap buttons
    m_xRightBtn->connect_mouse_move(Link<const MouseEvent&, bool>());
    m_xDownBtn->connect_mouse_move(Link<const MouseEvent&, bool>());

    m_xRightBtn->set_visible(bRight);
    m_xDownBtn->set_visible(bDown);

    weld::Button* pButton = nullptr;
    if (bRight)
        pButton = m_xRightBtn.get();
    else if (bDown)
        pButton = m_xDownBtn.get();
    InitControlBase(pButton);
    if (pButton)
    {
        pButton->connect_mouse_move(LINK(this, SwOutlineContentVisibilityWin, MouseMoveHdl));
        if (bControlHasFocus)
            pButton->grab_focus();
    }
}

void SwOutlineContentVisibilityWin::Set()
{
    const SwTextFrame* pTextFrame = static_cast<const SwTextFrame*>(GetFrame());

    // outline node frame containing folded outline node content might be folded so need to hide it
    if (!pTextFrame || pTextFrame->IsInDtor())
    {
        SetSymbol(SymbolType::DONTKNOW);
        Hide();
        return;
    }
    const SwTextNode* pTextNode = pTextFrame->GetTextNodeFirst();
    SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();
    const SwOutlineNodes& rOutlineNodes = rSh.GetNodes().GetOutLineNds();
    if (!pTextNode
        || !rOutlineNodes.Seek_Entry(static_cast<SwNode*>(const_cast<SwTextNode*>(pTextNode)),
                                     &m_nOutlinePos)
        || m_nOutlinePos == SwOutlineNodes::npos)
    {
        assert(false); // should never get here
        return;
    }

    // set symbol displayed on button
    SetSymbol(rSh.IsOutlineContentVisible(m_nOutlinePos) ? SymbolType::ARROW_DOWN
                                                         : SymbolType::ARROW_RIGHT);

    // set quick help
    SwOutlineNodes::size_type nOutlineNodesCount
        = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
    int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos);
    OUString sQuickHelp(SwResId(STR_OUTLINE_CONTENT_TOGGLE_VISIBILITY));
    if (!rSh.GetViewOptions()->IsTreatSubOutlineLevelsAsContent()
        && m_nOutlinePos + 1 < nOutlineNodesCount
        && rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos + 1) > nLevel)
        sQuickHelp += " (" + SwResId(STR_OUTLINE_CONTENT_TOGGLE_VISIBILITY_EXT) + ")";
    SetQuickHelpText(sQuickHelp);

    // Set the position of the window
    SwRect aSwRect = GetFrame()->getFrameArea(); // not far in margin
    //SwRect aSwRect = GetFrame()->GetPaintArea(); // far in margin
    aSwRect.AddTop(GetFrame()->GetTopMargin());
    Point aPxPt(GetEditWin()->GetOutDev()->LogicToPixel(
        aSwRect.TopLeft() - (aSwRect.TopLeft() - aSwRect.BottomLeft()) / 2));
    aPxPt.AdjustX(-GetSizePixel().getWidth() + 1);
    aPxPt.AdjustY(-GetSizePixel().getHeight() / 2);
    SetPosPixel(aPxPt);
}

void SwOutlineContentVisibilityWin::ShowAll(bool bShow)
{
    if (bShow)
    {
        m_nDelayAppearing = 0;
        if (!m_bDestroyed && m_aDelayTimer.IsActive())
            m_aDelayTimer.Stop();
        if (!m_bDestroyed)
            m_aDelayTimer.Start();
    }
    else
        Hide();
}

bool SwOutlineContentVisibilityWin::Contains(const Point& rDocPt) const
{
    ::tools::Rectangle aRect(GetPosPixel(), GetSizePixel());
    if (aRect.IsInside(rDocPt))
        return true;
    return false;
}

void SwOutlineContentVisibilityWin::ToggleOutlineContentVisibility(const bool bSubs)
{
    SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();
    rSh.LockView(true);
    if (GetEditWin()->GetView().GetDrawView()->IsTextEdit())
        rSh.EndTextEdit();
    if (GetEditWin()->GetView().IsDrawMode())
        GetEditWin()->GetView().LeaveDrawCreate();
    rSh.EnterStdMode();
    // set cursor position here so Navigator tracks outline
    // when doc changed broadcast message is sent in toggle function
    rSh.GotoOutline(m_nOutlinePos);
    if (rSh.GetViewOptions()->IsTreatSubOutlineLevelsAsContent())
        rSh.ToggleOutlineContentVisibility(m_nOutlinePos);
    else if (bSubs)
    {
        // toggle including sub levels
        SwOutlineNodes::size_type nPos = m_nOutlinePos;
        SwOutlineNodes::size_type nOutlineNodesCount
            = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
        int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos);
        bool bVisible = rSh.IsOutlineContentVisible(m_nOutlinePos);
        do
        {
            if (rSh.IsOutlineContentVisible(nPos) == bVisible)
                rSh.ToggleOutlineContentVisibility(nPos);
        } while (++nPos < nOutlineNodesCount
                 && rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nLevel);
    }
    else
        rSh.ToggleOutlineContentVisibility(m_nOutlinePos);
    if (!m_bDestroyed)
    {
        SetSymbol(rSh.IsOutlineContentVisible(m_nOutlinePos) ? SymbolType::ARROW_DOWN
                                                             : SymbolType::ARROW_RIGHT);
    }
    rSh.LockView(false);
}

IMPL_LINK(SwOutlineContentVisibilityWin, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bConsumed = false;

    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    if (!aKeyCode.GetModifier()
        && (aKeyCode.GetCode() == KEY_RETURN || aKeyCode.GetCode() == KEY_SPACE))
    {
        ToggleOutlineContentVisibility(aKeyCode.GetCode() == KEY_RETURN);
        bConsumed = true;
    }
    else if (aKeyCode.GetCode() == KEY_ESCAPE)
    {
        Hide();
        GrabFocusToDocument();
        bConsumed = true;
    }

    return bConsumed;
}

IMPL_LINK(SwOutlineContentVisibilityWin, MouseMoveHdl, const MouseEvent&, rMEvt, bool)
{
    if (rMEvt.IsLeaveWindow())
    {
        // MouseMove event may not be seen by edit window
        // hide collapse button and grab focus to document
        if (GetSymbol() != SymbolType::ARROW_RIGHT)
            Hide();
        GrabFocusToDocument();
    }
    else if (rMEvt.IsEnterWindow())
    {
        if (!m_bDestroyed && m_aDelayTimer.IsActive())
            m_aDelayTimer.Stop();
        // bring button to top and grab focus
        SetZOrder(this, ZOrderFlags::First);
        if (!ControlHasFocus())
            GrabFocus();
    }
    GetEditWin()->SetSavedOutlineFrame(const_cast<SwFrame*>(GetFrame()));
    return false;
}

IMPL_LINK(SwOutlineContentVisibilityWin, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    ToggleOutlineContentVisibility(rMEvt.IsRight());
    return false;
}

IMPL_LINK_NOARG(SwOutlineContentVisibilityWin, DelayAppearHandler, Timer*, void)
{
    const int TICKS_BEFORE_WE_APPEAR = 5;
    if (m_nDelayAppearing < TICKS_BEFORE_WE_APPEAR)
    {
        ++m_nDelayAppearing;
        m_aDelayTimer.Start();
        return;
    }
    if (GetEditWin()->GetSavedOutlineFrame() == GetFrame())
    {
        Show();
        GrabFocus();
    }
    m_aDelayTimer.Stop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
