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

#include <viewopt.hxx>

#include <FrameControlsManager.hxx>

SwOutlineContentVisibilityWin::SwOutlineContentVisibilityWin(SwEditWin* pEditWin,
                                                             const SwFrame* pFrame)
    : InterimItemWindow(pEditWin, u"modules/swriter/ui/outlinebutton.ui"_ustr,
                        u"OutlineButton"_ustr)
    , m_xShowBtn(m_xBuilder->weld_button(u"show"_ustr))
    , m_xHideBtn(m_xBuilder->weld_button(u"hide"_ustr))
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
    , m_nDelayAppearing(0)
    , m_aDelayTimer("SwOutlineContentVisibilityWin m_aDelayTimer")
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
    const SwTextNode* pTextNode = pTextFrame->GetTextNodeFirst();
    SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();
    const SwOutlineNodes& rOutlineNodes = rSh.GetNodes().GetOutLineNds();

    (void)rOutlineNodes.Seek_Entry(pTextNode, &m_nOutlinePos);

    // set symbol displayed on button
    bool bVisible = true;
    const_cast<SwTextNode*>(pTextNode)->GetAttrOutlineContentVisible(bVisible);
    SetSymbol(bVisible ? ButtonSymbol::HIDE : ButtonSymbol::SHOW);

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
    SwRect aFrameAreaRect = pTextFrame->getFrameArea();
    aFrameAreaRect.AddTop(pTextFrame->GetTopMargin());
    SwSpecialPos aSpecialPos;
    aSpecialPos.nExtendRange = pTextNode->HasVisibleNumberingOrBullet() ? SwSPExtendRange::BEFORE
                                                                        : SwSPExtendRange::NONE;
    SwCursorMoveState aMoveState;
    aMoveState.m_pSpecialPos = &aSpecialPos;
    SwRect aCharRect;
    pTextFrame->GetCharRect(aCharRect, SwPosition(*(pTextFrame->GetTextNodeForParaProps())),
                            &aMoveState);
    Point aPxPt(GetEditWin()->GetOutDev()->LogicToPixel(
        Point(aCharRect.Left(), aFrameAreaRect.Center().getY())));
    if (pTextFrame->IsRightToLeft())
        aPxPt.AdjustX(2);
    else
        aPxPt.AdjustX(-(GetSizePixel().getWidth() + 2));
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
    if (aRect.Contains(rDocPt))
        return true;
    return false;
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
            GetEditWin()->SetSavedOutlineFrame(
                static_cast<SwTextFrame*>(const_cast<SwFrame*>(GetFrame())));
        }
        if (!m_bDestroyed && m_aDelayTimer.IsActive())
            m_aDelayTimer.Stop();
        // bring button to top
        SetZOrder(this, ZOrderFlags::First);
    }
    return false;
}

// Toggle the outline content visibility on mouse press
IMPL_LINK(SwOutlineContentVisibilityWin, MousePressHdl, const MouseEvent&, rMEvt, bool)
{
    Hide();
    GetEditWin()->ToggleOutlineContentVisibility(m_nOutlinePos, rMEvt.IsRight());
    return false;
}

IMPL_LINK_NOARG(SwOutlineContentVisibilityWin, DelayAppearHandler, Timer*, void)
{
    const int TICKS_BEFORE_WE_APPEAR = 3;
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
