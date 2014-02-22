/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */





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
#include <vcl/settings.hxx>
#include "misc.hrc"

#define CONTROL_BORDER_WIDTH    1

#define CONTROL_LEFT_OFFSET     6
#define CONTROL_RIGHT_OFFSET    3
#define CONTROL_TOP_OFFSET      4

#define CONTROL_TRIANGLE_WIDTH  4
#define CONTROL_TRIANGLE_PAD    3


SwCommentRuler::SwCommentRuler( SwViewShell* pViewSh, Window* pParent, SwEditWin* pWin, sal_uInt16 nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
: SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle | WB_HSCROLL)
, mpViewShell(pViewSh)
, mpSwWin(pWin)
, mbIsHighlighted(false)
, mnFadeRate(0)
, maVirDev( *this )
{
    
    maFadeTimer.SetTimeout(40);
    maFadeTimer.SetTimeoutHdl( LINK( this, SwCommentRuler, FadeHandler ) );
}


SwCommentRuler::~SwCommentRuler()
{
}

void SwCommentRuler::Paint( const Rectangle& rRect )
{
    SvxRuler::Paint( rRect );
    
    if ( mpViewShell->GetPostItMgr()
         && mpViewShell->GetPostItMgr()->HasNotes() )
        DrawCommentControl();
}


void SwCommentRuler::DrawCommentControl()
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    bool bIsCollapsed = ! mpViewShell->GetPostItMgr()->ShowNotes();

    Rectangle aControlRect = GetCommentControlRegion();
    maVirDev.SetOutputSizePixel( aControlRect.GetSize() );

    
    
    Color aBgColor = GetFadedColor( rStyleSettings.GetDarkShadowColor(), rStyleSettings.GetWorkspaceColor() );
    maVirDev.SetFillColor( aBgColor );

    if ( mbIsHighlighted || !bIsCollapsed )
    {
        
        maVirDev.SetLineColor( rStyleSettings.GetShadowColor() );
    }
    else
    {
        
        maVirDev.SetLineColor();
    }

    maVirDev.DrawRect( Rectangle( Point(), aControlRect.GetSize() ) );

    
    OUString aLabel( SW_RESSTR ( STR_COMMENTS_LABEL ) );
    
    Point aLabelPos;
    Point aArrowPos;
    bool  bArrowToRight;
    
    aLabelPos.Y() = 0;
    aArrowPos.Y() = CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET;
    if ( !Application::GetSettings().GetLayoutRTL() )
    {
        
        if ( bIsCollapsed )
        {
            
            aLabelPos.X() = CONTROL_LEFT_OFFSET + CONTROL_TRIANGLE_WIDTH + CONTROL_TRIANGLE_PAD;
            aArrowPos.X() = CONTROL_LEFT_OFFSET;
        }
        else
        {
            
            aLabelPos.X() = CONTROL_LEFT_OFFSET;
            aArrowPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH;
        }
        bArrowToRight = bIsCollapsed;
    }
    else
    {
        
        long nLabelWidth = GetTextWidth( aLabel );
        if ( bIsCollapsed )
        {
            
            aArrowPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH;
            aLabelPos.X() = aArrowPos.X() - CONTROL_TRIANGLE_PAD - nLabelWidth;
        }
        else
        {
            
            aLabelPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - nLabelWidth;
            aArrowPos.X() = CONTROL_LEFT_OFFSET;
        }
        bArrowToRight = !bIsCollapsed;
    }

    
    Color aTextColor = GetFadedColor( rStyleSettings.GetButtonTextColor(), rStyleSettings.GetDarkShadowColor() );
    maVirDev.SetTextColor( aTextColor );
    
    maVirDev.DrawText( aLabelPos, aLabel );

    
    
    Color aArrowColor = GetFadedColor( Color( COL_BLACK ), rStyleSettings.GetShadowColor() );
    ImplDrawArrow ( aArrowPos.X(), aArrowPos.Y(), aArrowColor, bArrowToRight );

    
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


void SwCommentRuler::Command( const CommandEvent& rCEvt )
{
    Point aMousePos = rCEvt.GetMousePosPixel();
    
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

    Point aMousePos = rMEvt.GetPosPixel();
    bool  bWasHighlighted = mbIsHighlighted;
    mbIsHighlighted = GetCommentControlRegion().IsInside( aMousePos );
    if ( mbIsHighlighted != bWasHighlighted )
    {
        
        if ( mbIsHighlighted )
        {
            
            UpdateCommentHelpText();
        }
        else
        {
            
            
            SetQuickHelpText( OUString() );
        }
        
        maFadeTimer.Start();
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

    
    SwView &rView = mpSwWin->GetView();
    SfxRequest aRequest( rView.GetViewFrame(), FN_VIEW_NOTES );
    rView.ExecViewOptions( aRequest );

    
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


Rectangle SwCommentRuler::GetCommentControlRegion()
{
    long nLeft = 0;
    SwPostItMgr *pPostItMgr = mpViewShell->GetPostItMgr();

    
    
    
    if (!pPostItMgr)
        return Rectangle();

    unsigned long nSidebarWidth = pPostItMgr->GetSidebarWidth(true);
    
    if (GetTextRTL())
       nLeft = GetPageOffset() - nSidebarWidth + GetBorderOffset();
    else
       nLeft = GetWinOffset() + GetPageOffset() + mpSwWin->LogicToPixel(Size(GetPageWidth(), 0)).Width();
    long nTop    = 0 + 4; 
    
    long nRight  = nLeft + nSidebarWidth + pPostItMgr->GetSidebarBorderWidth(true);
    long nBottom = nTop + GetRulerVirHeight() - 3;

    Rectangle aRect(nLeft, nTop, nRight, nBottom);
    return aRect;
}

Color SwCommentRuler::GetFadedColor(const Color &rHighColor, const Color &rLowColor)
{
    if ( ! maFadeTimer.IsActive() )
        return mbIsHighlighted ? rHighColor : rLowColor;

    Color aColor = rHighColor;
    aColor.Merge( rLowColor, mnFadeRate * 255/100.f );
    return aColor;
}

IMPL_LINK_NOARG(SwCommentRuler, FadeHandler)
{
    const int nStep = 25;
    if ( mbIsHighlighted && mnFadeRate < 100 )
        mnFadeRate += nStep;
    else if ( !mbIsHighlighted && mnFadeRate > 0 )
        mnFadeRate -= nStep;
    else
        return 0;

    Invalidate();

    if ( mnFadeRate != 0 && mnFadeRate != 100)
        maFadeTimer.Start();
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
