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
#include <node.hxx>
#include <vcl/button.hxx>
#include <vcl/event.hxx>
#include <strings.hrc>
#include <svx/svdview.hxx>
#include <bitmaps.hlst>

#define BUTTON_WIDTH 18
#define BUTTON_HEIGHT 20

SwOutlineContentVisibilityWin::SwOutlineContentVisibilityWin(SwEditWin* pEditWin,
                                                             const SwFrame* pFrame)
    : PushButton(pEditWin, 0)
    , m_pEditWin(pEditWin)
    , m_pFrame(pFrame)
    , m_bIsAppearing(false)
    , m_nDelayAppearing(0)
    , m_bDestroyed(false)
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
    const SwTextNode* pTextNode = pTextFrame->GetTextNodeFirst();
    SwWrtShell& rSh = GetEditWin()->GetView().GetWrtShell();
    const SwOutlineNodes& rOutlineNodes = rSh.GetNodes().GetOutLineNds();
    rOutlineNodes.Seek_Entry(static_cast<SwNode*>(const_cast<SwTextNode*>(pTextNode)),
                             &m_nOutlinePos);
    assert(m_nOutlinePos != SwOutlineNodes::npos);

    // don't set if no content
    SwNode* pSttNd = rOutlineNodes[m_nOutlinePos];
    SwNode* pEndNd = &rSh.GetNodes().GetEndOfContent();
    if (rOutlineNodes.size() > m_nOutlinePos + 1)
        pEndNd = rOutlineNodes[m_nOutlinePos + 1];
    SwNodeIndex aIdx(*pSttNd, 1);
    if (&aIdx.GetNode() == pEndNd)
        return;

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
    Point aPxPt(GetEditWin()->GetOutDev()->LogicToPixel(aSwRect.BottomLeft()));
    aPxPt.AdjustX(-GetSizePixel().getWidth() - 2);
    aPxPt.AdjustY(-GetSizePixel().getHeight());
    SetPosPixel(aPxPt);

    // show expand button immediatly
    if (GetSymbol() == SymbolType::ARROW_RIGHT)
        Show();
}

void SwOutlineContentVisibilityWin::MouseMove(const MouseEvent& rMEvt)
{
    // used to hide collapse button if there is no chance of MouseMove event seen by edit window
    if (rMEvt.IsLeaveWindow() && GetSymbol() != SymbolType::ARROW_RIGHT)
        Hide();
}

void SwOutlineContentVisibilityWin::ShowAll(bool bShow)
{
    m_bIsAppearing = bShow;
    if (bShow)
        m_nDelayAppearing = 0;

    if (!m_bDestroyed && m_aDelayTimer.IsActive())
        m_aDelayTimer.Stop();
    if (!m_bDestroyed)
        m_aDelayTimer.Start();
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
    if (bSubs)
    {
        // toggle including sub levels
        SwOutlineNodes::size_type nPos = m_nOutlinePos;
        SwOutlineNodes::size_type nOutlineNodesCount
            = rSh.getIDocumentOutlineNodesAccess()->getOutlineNodesCount();
        int nLevel = rSh.getIDocumentOutlineNodesAccess()->getOutlineLevel(nPos);
        bool bFold = rSh.IsOutlineContentFolded(nPos);
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
    rSh.GotoOutline(m_nOutlinePos);
    rSh.LockView(false);
}

void SwOutlineContentVisibilityWin::MouseButtonDown(const MouseEvent& rMEvt)
{
    ToggleOutlineContentVisibility(rMEvt.IsRight() || rMEvt.IsMod1());
}

IMPL_LINK_NOARG(SwOutlineContentVisibilityWin, DelayHandler, Timer*, void)
{
    const int TICKS_BEFORE_WE_APPEAR = 10;
    if (m_bIsAppearing && m_nDelayAppearing < TICKS_BEFORE_WE_APPEAR)
    {
        ++m_nDelayAppearing;
        m_aDelayTimer.Start();
        return;
    }
    if (m_bIsAppearing)
        Show();
    else
        Hide();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
