/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

// FIXME https://bugs.freedesktop.org/show_bug.cgi?id=38246
// Design proposal: https://wiki.documentfoundation.org/Design/Whiteboards/Comments_Ruler_Control

#include "swruler.hxx"

#include "viewsh.hxx"
#include "vcl/window.hxx"
#include "edtwin.hxx"
#include "PostItMgr.hxx"
#include "viewopt.hxx"
#include <view.hxx>
#include "cmdid.h"
#include <sfx2/request.hxx>

#define CONTROL_BORDER_WIDTH    1

#define CONTROL_LEFT_OFFSET     6
#define CONTROL_RIGHT_OFFSET    3
#define CONTROL_TOP_OFFSET      4
#define CONTROL_BOTTOM_OFFSET   4

#define CONTROL_TRIANGLE_WIDTH  4
#define CONTROL_TRIANGLE_HEIGHT 7
#define CONTROL_TRIANGLE_PAD    3

// Constructor
SwCommentRuler::SwCommentRuler( ViewShell* pViewSh, Window* pParent, SwEditWin* pWin, sal_uInt16 nRulerFlags,  SfxBindings& rBindings, WinBits nWinStyle)
: SvxRuler(pParent, pWin, nRulerFlags, rBindings, nWinStyle | WB_HSCROLL)
, mpViewShell(pViewSh)
, mpSwWin(pWin)
, mbHighlighted(false)
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
    bool bCollapsed = ! mpViewShell->GetPostItMgr()->ShowNotes();

    // FIXME RTL
    Rectangle aControlRect = GetCommentControlRegion();
    maVirDev.SetOutputSizePixel( aControlRect.GetSize() );

    // Paint comment control background
    // TODO Check if these are best colors to be used
    if ( mbHighlighted )
        maVirDev.SetFillColor( rStyleSettings.GetDarkShadowColor() );
    else
        maVirDev.SetFillColor( rStyleSettings.GetWorkspaceColor() );

    if ( mbHighlighted || !bCollapsed )
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

    // Get label and arrow coordinates
    Point aLabelPos;
    Point aArrowPos;
    aLabelPos.Y() = CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET;
    aArrowPos.Y() = CONTROL_BORDER_WIDTH + CONTROL_TOP_OFFSET;
    if ( bCollapsed )
    {
        aLabelPos.X() = CONTROL_LEFT_OFFSET + CONTROL_TRIANGLE_WIDTH + CONTROL_TRIANGLE_PAD;
        aArrowPos.X() = CONTROL_LEFT_OFFSET;
    }
    else
    {
        aLabelPos.X() = CONTROL_LEFT_OFFSET;
        aArrowPos.X() = aControlRect.GetSize().Width() - 1 - CONTROL_RIGHT_OFFSET - CONTROL_BORDER_WIDTH - CONTROL_TRIANGLE_WIDTH;
    }

    // Draw label
    const Color &rTextColor = mbHighlighted ? rStyleSettings.GetButtonTextColor() : rStyleSettings.GetDeactiveTextColor();
    maVirDev.SetTextColor( rTextColor );
    // FIXME i18n
    // FIXME Vertical alignment
    // FIXME Expected font size?
    maVirDev.DrawText( aLabelPos, String("Comments") );

    // Draw arrow
    // FIXME colors consistence. http://opengrok.libreoffice.org/xref/core/vcl/source/control/button.cxx#785
    const Color &rArrowColor = mbHighlighted ? Color( COL_BLACK ) : rStyleSettings.GetShadowColor();
    ImplDrawArrow ( aArrowPos.X(), aArrowPos.Y(), rArrowColor, bCollapsed );

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
    if ( !mpViewShell->GetPostItMgr()
          || !mpViewShell->GetPostItMgr()->HasNotes()
          || !GetCommentControlRegion().IsInside( aMousePos ) )
        SvxRuler::Command( rCEvt );
}

void SwCommentRuler::MouseMove(const MouseEvent& rMEvt)
{
    SvxRuler::MouseMove(rMEvt);
    // TODO Delay 0.1s to highlight and 0.2s to "lowlight"
    Point aMousePos = rMEvt.GetPosPixel();
    bool  bWasHighlighted = mbHighlighted;
    mbHighlighted = GetCommentControlRegion().IsInside( aMousePos );
    if ( mbHighlighted != bWasHighlighted )
        Invalidate();
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

    Invalidate();
}

void SwCommentRuler::Update()
{
    Rectangle aPreviousControlRect = GetCommentControlRegion();
    SvxRuler::Update();
    if (aPreviousControlRect != GetCommentControlRegion())
        Invalidate();
}

Rectangle SwCommentRuler::GetCommentControlRegion()
{
    long nLeft   = GetWinOffset() + GetPageOffset() + GetRulerWidth();
    long nTop    = 0 + 4;
    // Right: need to add sidebar border width not included
    long nRight  = nLeft+ mpViewShell->GetPostItMgr()->GetSidebarWidth(true) + 2*1;
    long nBottom = nTop + GetRulerVirHeight() - 3;

    Rectangle aRect(nLeft, nTop, nRight, nBottom);
    return aRect;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
