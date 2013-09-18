/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// FIX fdo#38246 https://bugs.freedesktop.org/show_bug.cgi?id=38246
// Design proposal: https://wiki.documentfoundation.org/Design/Whiteboards/Comments_Ruler_Control
// TODO Alpha blend border when it doesn't fit in window
// TODO Delayed highlight fading when user moves mouse over and out the control

#include "swruler.hxx"

#include "viewsh.hxx"
#include "edtwin.hxx"
#include "PostItMgr.hxx"
#include "viewopt.hxx"
#include <view.hxx>
#include "cmdid.h"
#include <sfx2/request.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include "misc.hrc"

#define CONTROL_BORDER_WIDTH    1

#define CONTROL_LEFT_OFFSET     6
#define CONTROL_RIGHT_OFFSET    3
#define CONTROL_TOP_OFFSET      4

#define CONTROL_TRIANGLE_WIDTH  4
#define CONTROL_TRIANGLE_PAD    3

// Constructor
SwCommentRuler::SwCommentRuler( ViewShell* pViewSh, Window* pParent, SwEditWin* pWin, sal_uInt16 nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
: SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle | WB_HSCROLL)
, mpViewShell(pViewSh)
, mpSwWin(pWin)
, mbIsHighlighted(false)
, maVirDev( *this )
{
}

// Destructor
SwCommentRuler::~SwCommentRuler()
{
}

void SwCommentRuler::Paint( const Rectangle& rRect )
{
    SvxRuler::Paint( rRect );
    // Don't draw if there is not any note
    if ( mpViewShell->GetPostItMgr()->HasNotes() )
        DrawCommentControl();
}

void SwCommentRuler::DrawCommentControl()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    bool bIsCollapsed = ! mpViewShell->GetPostItMgr()->ShowNotes();

    Rectangle aControlRect = GetCommentControlRegion();
    maVirDev.SetOutputSizePixel( aControlRect.GetSize() );

    // Paint comment control background
    // TODO Check if these are best colors to be used
    if ( mbIsHighlighted )
        maVirDev.SetFillColor( rStyleSettings.GetDarkShadowColor() );
    else
        maVirDev.SetFillColor( rStyleSettings.GetWorkspaceColor() );

    if ( mbIsHighlighted || !bIsCollapsed )
    {
        // Draw borders
        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
    }
    else
    {
        // No borders
        maVirDev.SetLineColor();
    }

    maVirDev.DrawRect( Rectangle( Point(), aControlRect.GetSize() ) );

    String aLabel( SW_RESSTR ( STR_COMMENTS_LABEL ) );
    // Get label and arrow coordinates
    Point aLabelPos;
    Point aArrowPos;
    bool  bArrowToRight;
    // TODO Discover why it should be 0 instead of CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET
    aLabelPos.Y() = 0;
    aArrowPos.Y() = CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET;
    if ( !Application::GetSettings().GetLayoutRTL() )
    {
        // LTR
        if ( bIsCollapsed )
        {
            // It should draw something like | > Comments  |
            aLabelPos.X() = CONTROL_LEFT_OFFSET + CONTROL_TRIANGLE_WIDTH + CONTROL_TRIANGLE_PAD;
            aArrowPos.X() = CONTROL_LEFT_OFFSET;
        }
        else
        {
            // It should draw something like | Comments  < |
            aLabelPos.X() = CONTROL_LEFT_OFFSET;
            aArrowPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH;
        }
        bArrowToRight = bIsCollapsed;
    }
    else
    {
        // RTL
        long nLabelWidth = GetTextWidth( aLabel );
        if ( bIsCollapsed )
        {
            // It should draw something like |  Comments < |
            aArrowPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH;
            aLabelPos.X() = aArrowPos.X() - CONTROL_TRIANGLE_PAD - nLabelWidth;
        }
        else
        {
            // It should draw something like | >  Comments |
            aLabelPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - nLabelWidth;
            aArrowPos.X() = CONTROL_LEFT_OFFSET;
        }
        bArrowToRight = !bIsCollapsed;
    }

    // Draw label
    const Color &rTextColor = mbIsHighlighted ? rStyleSettings.GetButtonTextColor() : rStyleSettings.GetDeactiveTextColor();
    maVirDev.SetTextColor( rTextColor );
    // FIXME Expected font size?
    maVirDev.DrawText( aLabelPos, aLabel );

    // Draw arrow
    // FIXME consistence of button colors. http://opengrok.libreoffice.org/xref/core/vcl/source/control/button.cxx#785
    const Color &rArrowColor = mbIsHighlighted ? Color( COL_BLACK ) : rStyleSettings.GetShadowColor();
    ImplDrawArrow ( aArrowPos.X(), aArrowPos.Y(), rArrowColor, bArrowToRight );

    // Blit comment control
    DrawOutDev( aControlRect.TopLeft(), aControlRect.GetSize(), Point(), aControlRect.GetSize(), maVirDev );
}

void SwCommentRuler::ImplDrawArrow(long nX, long nY, const Color& rColor, bool bPointRight)
{
    maVirDev.SetLineColor();
    maVirDev.SetFillColor( rColor );
    if ( bPointRight )
    {
        maVirDev.DrawRect( Rectangle( nX+0, nY+0, nX+0, nY+6 ) );
        maVirDev.DrawRect( Rectangle( nX+1, nY+1, nX+1, nY+5 ) );
        maVirDev.DrawRect( Rectangle( nX+2, nY+2, nX+2, nY+4 ) );
        maVirDev.DrawRect( Rectangle( nX+3, nY+3, nX+3, nY+3 ) );
    }
    else
    {
        maVirDev.DrawRect( Rectangle( nX+0, nY+3, nX+0, nY+3 ) );
        maVirDev.DrawRect( Rectangle( nX+1, nY+2, nX+1, nY+4 ) );
        maVirDev.DrawRect( Rectangle( nX+2, nY+1, nX+2, nY+5 ) );
        maVirDev.DrawRect( Rectangle( nX+3, nY+0, nX+3, nY+6 ) );
    }
}

// Just accept double-click outside comment control
void SwCommentRuler::Command( const CommandEvent& rCEvt )
{
    Point aMousePos = rCEvt.GetMousePosPixel();
    // Ignore command request if it is inside Comment Control
    if ( !mpViewShell->GetPostItMgr()
          || !mpViewShell->GetPostItMgr()->HasNotes()
          || !GetCommentControlRegion().IsInside( aMousePos ) )
        SvxRuler::Command( rCEvt );
}

void SwCommentRuler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
    if ( ! mpViewShell->GetPostItMgr() || ! mpViewShell->GetPostItMgr()->HasNotes() )
        return;
    // TODO Delay 0.1s to highlight and 0.2s to "lowlight"
    Point aMousePos = rMEvt.GetPosPixel();
    bool  bWasHighlighted = mbIsHighlighted;
    mbIsHighlighted = GetCommentControlRegion().IsInside( aMousePos );
    if ( mbIsHighlighted != bWasHighlighted )
    {
        // Repaint for highlight changes make effect
        Invalidate();
        // Set proper help text
        if ( mbIsHighlighted )
        {
            // Mouse over comment control
            UpdateCommentHelpText();
        }
        else
        {
            // Mouse out of comment control
            // FIXME Should remember previous tooltip text?
            SetQuickHelpText( String() );
        }
    }
}

void SwCommentRuler::MouseButtonDown( const MouseEvent& rMEvt )
{
    Point   aMousePos = rMEvt.GetPosPixel();
    if ( !rMEvt.IsLeft() || IsTracking() || !GetCommentControlRegion().IsInside( aMousePos ) )
    {
        SvxRuler::MouseButtonDown(rMEvt);
        return;
    }

    // Toggle notes visibility
    SwView &rView = mpSwWin->GetView();
    SfxRequest aRequest( rView.GetViewFrame(), FN_VIEW_NOTES );
    rView.ExecViewOptions( aRequest );

    // It is inside comment control, so update help text
    UpdateCommentHelpText();

    Invalidate();
}

void SwCommentRuler::Update()
{
    Rectangle aPreviousControlRect = GetCommentControlRegion();
    SvxRuler::Update();
    if (aPreviousControlRect != GetCommentControlRegion())
        Invalidate();
}

void SwCommentRuler::UpdateCommentHelpText()
{
    int nTooltipResId;
    if ( mpViewShell->GetPostItMgr()->ShowNotes() )
        nTooltipResId = STR_HIDE_COMMENTS;
    else
        nTooltipResId = STR_SHOW_COMMENTS;
    SetQuickHelpText( OUString( SW_RESSTR( nTooltipResId ) ) );
}

// NOTE: If sometime ahead SwSidebar moves from page right side due RTL layout,
//   change how this rectangle is used by callers
// TODO Make Ruler return its central rectangle instead of margins.
Rectangle SwCommentRuler::GetCommentControlRegion()
{
    long nLeft = 0;
    SwPostItMgr *pPostItMgr = mpViewShell->GetPostItMgr();

    //rhbz#1006850 When the SwPostItMgr ctor is called from SwView::SwView it
    //triggers an update of the uiview, but the result of the ctor hasn't been
    //set into the mpViewShell yet, so GetPostItMgr is temporarily still NULL
    if (!pPostItMgr)
        return Rectangle();

    //FIXME When the page width is larger then screen, the ruler is misplaced by one pixel
    if (GetTextRTL())
       nLeft = GetPageOffset() - pPostItMgr->GetSidebarWidth(true) + GetBorderOffset();
    else
       nLeft = GetWinOffset() + GetPageOffset() + mpSwWin->LogicToPixel(Size(GetPageWidth(), 0)).Width();
    long nTop    = 0 + 4; // Ruler::ImplDraw uses RULER_OFF (value: 3px) as offset, and Ruler::ImplFormat adds one extra pixel
    // Somehow pPostItMgr->GetSidebarBorderWidth() returns border width already doubled
    long nRight  = nLeft+ pPostItMgr->GetSidebarWidth(true) + pPostItMgr->GetSidebarBorderWidth(true);
    long nBottom = nTop + GetRulerVirHeight() - 3;

    Rectangle aRect(nLeft, nTop, nRight, nBottom);
    return aRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
