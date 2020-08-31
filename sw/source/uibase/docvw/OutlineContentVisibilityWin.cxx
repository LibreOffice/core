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
#include <viewopt.hxx>
#include <wrtsh.hxx>

#include <memory>

#include <IDocumentOutlineNodes.hxx>
#include <txtfrm.hxx>
#include <ndtxt.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <strings.hrc>
#include <svx/svdview.hxx>

#define BUTTON_WIDTH 18
#define BUTTON_HEIGHT 20

SwOutlineContentVisibilityWin::SwOutlineContentVisibilityWin(SwEditWin* pEditWin,
                                                             const SwFrame* pFrame)
    : PushButton(pEditWin, 0)
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
    , m_nDelayAppearing(0)
    , m_bDestroyed(false)
    , m_nOutlinePos(SwOutlineNodes::npos)
{
    SetSizePixel(Size(BUTTON_WIDTH, BUTTON_HEIGHT));

    m_aDelayTimer.SetTimeout(50);
    m_aDelayTimer.SetInvokeHandler(LINK(this, SwOutlineContentVisibilityWin, DelayHandler));
}

void SwOutlineContentVisibilityWin::dispose()
{
    m_bDestroyed = true;
    m_aDelayTimer.Stop();

    m_pEditWin.clear();
    m_pFrame = nullptr;

    PushButton::dispose();
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

    // don't set if no content and no subs with content
    auto nPos = m_nOutlinePos;
    SwNode* pSttNd = rOutlineNodes[nPos];
    SwNode* pEndNd;
    SwNodeIndex aIdx(*pSttNd);
    while (true)
    {
        if (rOutlineNodes.size() > ++nPos)
            pEndNd = rOutlineNodes[nPos];
        else
            pEndNd = &rSh.GetNodes().GetEndOfContent();
        if (!pSttNd->IsEndNode())
            aIdx.Assign(*pSttNd, +1);
        if (pSttNd->IsEndNode()
            || ((&aIdx.GetNode() == pEndNd && pEndNd->IsEndNode())
                || (&aIdx.GetNode() == pEndNd && pSttNd->IsTextNode() && pEndNd->IsTextNode()
                    && pSttNd->GetTextNode()->GetAttrOutlineLevel()
                           >= pEndNd->GetTextNode()->GetAttrOutlineLevel())))
        {
            SetSymbol(SymbolType::DONTKNOW);
            Hide();
            return;
        }
        if (&aIdx.GetNode() != pEndNd)
            break;
        pSttNd = pEndNd;
    }

    // set symbol displayed on button
    SetSymbol(rSh.IsOutlineContentFolded(m_nOutlinePos) ? SymbolType::ARROW_RIGHT
                                                        : SymbolType::ARROW_DOWN);

    // set quick help
    SwOutlineNodes::size_type nOutlineNodesCount
        = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
    int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos);
    OUString sQuickHelp(SwResId(STR_OUTLINE_CONTENT_TOGGLE_VISIBILITY));
    if (m_nOutlinePos + 1 < nOutlineNodesCount
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
    if (bSubs)
    {
        // toggle including sub levels
        SwOutlineNodes::size_type nPos = m_nOutlinePos;
        SwOutlineNodes::size_type nOutlineNodesCount
            = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
        int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(m_nOutlinePos);
        bool bFold = rSh.IsOutlineContentFolded(m_nOutlinePos);
        do
        {
            if (rSh.IsOutlineContentFolded(nPos) == bFold)
                rSh.ToggleOutlineContentVisibility(nPos);
        } while (++nPos < nOutlineNodesCount
                 && rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos) > nLevel);
    }
    else
        rSh.ToggleOutlineContentVisibility(m_nOutlinePos);
    SetSymbol(rSh.IsOutlineContentFolded(m_nOutlinePos) ? SymbolType::ARROW_RIGHT
                                                        : SymbolType::ARROW_DOWN);
    rSh.LockView(false);
}

void SwOutlineContentVisibilityWin::KeyInput(const KeyEvent& rKEvt)
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    if (!aKeyCode.GetModifier()
        && (aKeyCode.GetCode() == KEY_RETURN || aKeyCode.GetCode() == KEY_SPACE))
    {
        ToggleOutlineContentVisibility(aKeyCode.GetCode() == KEY_RETURN);
    }
    else if (aKeyCode.GetCode() == KEY_ESCAPE)
    {
        Hide();
        GrabFocusToDocument();
    }
}

void SwOutlineContentVisibilityWin::MouseMove(const MouseEvent& rMEvt)
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
        GrabFocus();
    }
    GetEditWin()->SetSavedOutlineFrame(const_cast<SwFrame*>(GetFrame()));
}

void SwOutlineContentVisibilityWin::MouseButtonDown(const MouseEvent& rMEvt)
{
    ToggleOutlineContentVisibility(rMEvt.IsRight() || rMEvt.IsMod1());
}

IMPL_LINK_NOARG(SwOutlineContentVisibilityWin, DelayHandler, Timer*, void)
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
