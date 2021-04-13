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

#include <FrameControlsManager.hxx>

SwOutlineContentVisibilityWin::SwOutlineContentVisibilityWin(SwEditWin* pEditWin,
                                                             const SwFrame* pFrame)
    : InterimItemWindow(pEditWin, "modules/swriter/ui/outlinebutton.ui", "OutlineButton")
    , m_xShowBtn(m_xBuilder->weld_button("show"))
    , m_xHideBtn(m_xBuilder->weld_button("hide"))
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
    , m_nDelayAppearing(0)
    , m_bDestroyed(false)
    , m_nOutlinePos(SwOutlineNodes::npos)
{
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    SetPaintTransparent(false);
    SetBackground(rStyleSettings.GetFaceColor());

    Size aBtnsSize(m_xShowBtn->get_preferred_size());
    auto nDim = std::max(aBtnsSize.Width(), aBtnsSize.Height());
    m_xShowBtn->set_size_request(nDim, nDim);
    m_xHideBtn->set_size_request(nDim, nDim);

    SetSizePixel(get_preferred_size());
    SetSymbol(ButtonSymbol::NONE);

    m_xShowBtn->connect_mouse_press(LINK(this, SwOutlineContentVisibilityWin, MousePressHdl));
    m_xHideBtn->connect_mouse_press(LINK(this, SwOutlineContentVisibilityWin, MousePressHdl));

    m_aDelayTimer.SetTimeout(25);
    m_aDelayTimer.SetInvokeHandler(LINK(this, SwOutlineContentVisibilityWin, DelayAppearHandler));
}

void SwOutlineContentVisibilityWin::dispose()
{
    m_bDestroyed = true;
    m_aDelayTimer.Stop();

    m_pEditWin.clear();
    m_pFrame = nullptr;

    m_xHideBtn.reset();
    m_xShowBtn.reset();

    InterimItemWindow::dispose();
}

ButtonSymbol SwOutlineContentVisibilityWin::GetSymbol() const
{
    if (m_xShowBtn->get_visible())
        return ButtonSymbol::SHOW;
    if (m_xHideBtn->get_visible())
        return ButtonSymbol::HIDE;
    return ButtonSymbol::NONE;
}

void SwOutlineContentVisibilityWin::SetSymbol(ButtonSymbol eStyle)
{
    if (GetSymbol() == eStyle)
        return;

    bool bShow = eStyle == ButtonSymbol::SHOW;
    bool bHide = eStyle == ButtonSymbol::HIDE;

    // disable mouse move for the hidden button so we don't get mouse
    // leave events we don't care about when we swap buttons
    m_xShowBtn->connect_mouse_move(Link<const MouseEvent&, bool>());
    m_xHideBtn->connect_mouse_move(Link<const MouseEvent&, bool>());

    m_xShowBtn->set_visible(bShow);
    m_xHideBtn->set_visible(bHide);

    weld::Button* pButton = nullptr;
    if (bShow)
        pButton = m_xShowBtn.get();
    else if (bHide)
        pButton = m_xHideBtn.get();
    InitControlBase(pButton);
    if (pButton)
        pButton->connect_mouse_move(LINK(this, SwOutlineContentVisibilityWin, MouseMoveHdl));
}

void SwOutlineContentVisibilityWin::Set()
{
    const SwTextFrame* pTextFrame = static_cast<const SwTextFrame*>(GetFrame());

    // outline node frame containing folded outline node content might be folded so need to hide it
    if (!pTextFrame || pTextFrame->IsInDtor())
    {
        SetSymbol(ButtonSymbol::NONE);
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
    SetSymbol(rSh.IsOutlineContentVisible(m_nOutlinePos) ? ButtonSymbol::HIDE : ButtonSymbol::SHOW);

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
    SwRect aFrameAreaRect = GetFrame()->getFrameArea();
    aFrameAreaRect.AddTop(GetFrame()->GetTopMargin());
    SwRect aCharRect;
    GetFrame()->GetCharRect(aCharRect, SwPosition(*pTextNode));
    Point aPxPt(GetEditWin()->GetOutDev()->LogicToPixel(
        Point(aCharRect.Right(), aFrameAreaRect.Center().getY())));
    if (GetFrame()->IsRightToLeft())
        aPxPt.AdjustX(5);
    else
        aPxPt.AdjustX(-(GetSizePixel().getWidth() + 5));
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
    // set cursor position to the toggled outline node
    rSh.GotoOutline(m_nOutlinePos);
    rSh.LockView(false);
}

IMPL_LINK(SwOutlineContentVisibilityWin, MouseMoveHdl, const MouseEvent&, rMEvt, bool)
{
    if (rMEvt.IsLeaveWindow())
    {
        if (GetSymbol() == ButtonSymbol::HIDE)
        {
            // MouseMove event may not be seen by the edit window for example when move is to
            // a show button or when move is outside of the edit window.
            // Only hide when mouse leave results in leaving the frame.
            tools::Rectangle aFrameAreaPxRect
                = GetEditWin()->LogicToPixel(GetFrame()->getFrameArea().SVRect());
            auto nY = GetPosPixel().getY() + rMEvt.GetPosPixel().getY();
            if (nY <= 0 || nY <= aFrameAreaPxRect.Top() || nY >= aFrameAreaPxRect.Bottom()
                || nY >= GetEditWin()->GetSizePixel().Height())
            {
                GetEditWin()->SetSavedOutlineFrame(nullptr);
                GetEditWin()->GetFrameControlsManager().RemoveControlsByType(
                    FrameControlType::Outline, GetFrame());
                // warning: "this" is disposed now
            }
        }
    }
    else if (rMEvt.IsEnterWindow())
    {
        // Leave window event might not have resulted in removing hide button from saved frame
        // and the edit win might not receive mouse event between leaving saved frame button and
        // entering this button.
        if (GetFrame() != GetEditWin()->GetSavedOutlineFrame())
        {
            SwFrameControlPtr pFrameControl = GetEditWin()->GetFrameControlsManager().GetControl(
                FrameControlType::Outline, GetEditWin()->GetSavedOutlineFrame());
            if (pFrameControl)
            {
                SwOutlineContentVisibilityWin* pControl
                    = dynamic_cast<SwOutlineContentVisibilityWin*>(pFrameControl->GetIFacePtr());
                if (pControl && pControl->GetSymbol() == ButtonSymbol::HIDE)
                {
                    GetEditWin()->GetFrameControlsManager().RemoveControlsByType(
                        FrameControlType::Outline, GetEditWin()->GetSavedOutlineFrame());
                    // The outline content visibility window frame control (hide button)
                    // for saved outline frame is now disposed.
                }
            }
            GetEditWin()->SetSavedOutlineFrame(const_cast<SwFrame*>(GetFrame()));
        }
        if (!m_bDestroyed && m_aDelayTimer.IsActive())
            m_aDelayTimer.Stop();
        // bring button to top
        SetZOrder(this, ZOrderFlags::First);
    }
    return false;
}

IMPL_LINK(SwOutlineContentVisibilityWin, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    Hide();
    // Crash occurs if control does not have focus when toggling from hidden to shown.
    // Seems to happen due to the control being disposed in the toggle process.
    // Does NOT crash in debugger with GrabFocus and GrabFocusToDocument commented out.
    // DOES crash in debugger on GrabFocusToDocument when GrabFocus is commented out.
    // Until light is shed on why this happens, prevent crash by doing the following:
    //  1) grab focus to the control
    //  2) toggle content visibility
    //  3) grab focus to the document
    if (!ControlHasFocus())
        GrabFocus();
    ToggleOutlineContentVisibility(rMEvt.IsRight());
    GrabFocusToDocument();
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
        Show();
    m_aDelayTimer.Stop();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
