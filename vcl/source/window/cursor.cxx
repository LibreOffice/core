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

#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/cursor.hxx>

#include <window.h>

#include <tools/poly.hxx>

struct ImplCursorData
{
    AutoTimer       maTimer;            // Timer
    Point           maPixPos;           // Pixel-Position
    Point           maPixRotOff;        // Pixel-Offset-Position
    Size            maPixSize;          // Pixel-Size
    long            mnPixSlant;         // Pixel-Slant
    short           mnOrientation;      // Pixel-Orientation
    CursorDirection mnDirection;        // indicates writing direction
    sal_uInt16      mnStyle;            // Cursor-Style
    bool            mbCurVisible;       // Ist Cursor aktuell sichtbar
    VclPtr<vcl::Window> mpWindow;           // Zugeordnetes Windows
};

static void ImplCursorInvert( ImplCursorData* pData )
{
    vcl::Window* pWindow  = pData->mpWindow;
    std::unique_ptr<PaintBufferGuard> pGuard;
    const bool bDoubleBuffering = pWindow->SupportsDoubleBuffering();
    if (bDoubleBuffering)
        pGuard.reset(new PaintBufferGuard(pWindow->ImplGetWindowImpl()->mpFrameData, pWindow));
    vcl::RenderContext* pRenderContext = bDoubleBuffering ? pGuard->GetRenderContext() : pWindow;
    Rectangle aPaintRect;
    bool    bMapMode = pRenderContext->IsMapModeEnabled();
    pRenderContext->EnableMapMode( false );
    sal_uInt16 nInvertStyle;
    if ( pData->mnStyle & CURSOR_SHADOW )
        nInvertStyle = INVERT_50;
    else
        nInvertStyle = 0;

    Rectangle aRect( pData->maPixPos, pData->maPixSize );
    if ( pData->mnDirection != CursorDirection::NONE || pData->mnOrientation || pData->mnPixSlant )
    {
        Polygon aPoly( aRect );
        if( aPoly.GetSize() == 5 )
        {
            aPoly[1].X() += 1;  // include the right border
            aPoly[2].X() += 1;
            if ( pData->mnPixSlant )
            {
                Point aPoint = aPoly.GetPoint( 0 );
                aPoint.X() += pData->mnPixSlant;
                aPoly.SetPoint( aPoint, 0 );
                aPoly.SetPoint( aPoint, 4 );
                aPoint = aPoly.GetPoint( 1 );
                aPoint.X() += pData->mnPixSlant;
                aPoly.SetPoint( aPoint, 1 );
            }

            // apply direction flag after slant to use the correct shape
            if ( pData->mnDirection != CursorDirection::NONE)
            {
                Point pAry[7];
                int delta = 3*aRect.getWidth()+1;
                if( pData->mnDirection == CursorDirection::LTR )
                {
                    // left-to-right
                    pAry[0] = aPoly.GetPoint( 0 );
                    pAry[1] = aPoly.GetPoint( 1 );
                    pAry[2] = pAry[1];
                    pAry[2].X() += delta;
                    pAry[3] =  pAry[1];
                    pAry[3].Y() += delta;
                    pAry[4] = aPoly.GetPoint( 2 );
                    pAry[5] = aPoly.GetPoint( 3 );
                    pAry[6] = aPoly.GetPoint( 4 );
                }
                else if( pData->mnDirection == CursorDirection::RTL )
                {
                    // right-to-left
                    pAry[0] = aPoly.GetPoint( 0 );
                    pAry[1] = aPoly.GetPoint( 1 );
                    pAry[2] = aPoly.GetPoint( 2 );
                    pAry[3] = aPoly.GetPoint( 3 );
                    pAry[4] = pAry[0];
                    pAry[4].Y() += delta;
                    pAry[5] =  pAry[0];
                    pAry[5].X() -= delta;
                    pAry[6] = aPoly.GetPoint( 4 );
                }
                aPoly = Polygon( 7, pAry);
            }

            if ( pData->mnOrientation )
                aPoly.Rotate( pData->maPixRotOff, pData->mnOrientation );
            pRenderContext->Invert( aPoly, nInvertStyle );
            if (bDoubleBuffering)
                aPaintRect = aPoly.GetBoundRect();
        }
    }
    else
    {
        pRenderContext->Invert( aRect, nInvertStyle );
        if (bDoubleBuffering)
            aPaintRect = aRect;
    }
    pRenderContext->EnableMapMode( bMapMode );
    if (bDoubleBuffering)
        pGuard->SetPaintRect(pRenderContext->PixelToLogic(aPaintRect));
}

void vcl::Cursor::ImplDraw()
{
    if ( mpData && mpData->mpWindow && !mpData->mbCurVisible )
    {
        vcl::Window* pWindow         = mpData->mpWindow;
        mpData->maPixPos        = pWindow->LogicToPixel( maPos );
        mpData->maPixSize       = pWindow->LogicToPixel( maSize );
        mpData->mnPixSlant      = pWindow->LogicToPixel( Size( mnSlant, 0 ) ).Width();
        mpData->mnOrientation   = mnOrientation;
        mpData->mnDirection     = mnDirection;

        // correct the position with the offset
        mpData->maPixRotOff = mpData->maPixPos;

        // use width (as set in Settings) if size is 0,
        if ( !mpData->maPixSize.Width() )
            mpData->maPixSize.Width() = pWindow->GetSettings().GetStyleSettings().GetCursorSize();

        // calculate output area and display
        ImplCursorInvert( mpData );
        mpData->mbCurVisible = true;
    }
}

void vcl::Cursor::ImplRestore()
{
    if ( mpData && mpData->mbCurVisible )
    {
        ImplCursorInvert( mpData );
        mpData->mbCurVisible = false;
    }
}

void vcl::Cursor::ImplDoShow( bool bDrawDirect, bool bRestore )
{
    if ( mbVisible )
    {
        vcl::Window* pWindow;
        if ( mpWindow )
            pWindow = mpWindow;
        else
        {
            // show the cursor, if there is an active window and the cursor
            // has been selected in this window
            pWindow = Application::GetFocusWindow();
            if ( !pWindow || (pWindow->mpWindowImpl->mpCursor != this) || pWindow->mpWindowImpl->mbInPaint
                || !pWindow->mpWindowImpl->mpFrameData->mbHasFocus )
                pWindow = NULL;
        }

        if ( pWindow )
        {
            if ( !mpData )
            {
                mpData = new ImplCursorData;
                mpData->mbCurVisible = false;
                mpData->maTimer.SetTimeoutHdl( LINK( this, Cursor, ImplTimerHdl ) );
            }

            mpData->mpWindow    = pWindow;
            mpData->mnStyle     = mnStyle;
            if ( bDrawDirect || bRestore )
                ImplDraw();

            if ( !mpWindow && ! ( ! bDrawDirect && mpData->maTimer.IsActive()) )
            {
                mpData->maTimer.SetTimeout( pWindow->GetSettings().GetStyleSettings().GetCursorBlinkTime() );
                if ( mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
                    mpData->maTimer.Start();
                else if ( !mpData->mbCurVisible )
                    ImplDraw();
            }
        }
    }
}

bool vcl::Cursor::ImplDoHide( bool bSuspend )
{
    bool bWasCurVisible = false;
    if ( mpData && mpData->mpWindow )
    {
        bWasCurVisible = mpData->mbCurVisible;
        if ( mpData->mbCurVisible )
            ImplRestore();

        if ( !bSuspend )
        {
            mpData->maTimer.Stop();
            mpData->mpWindow = NULL;
        }
    }
    return bWasCurVisible;
}

void vcl::Cursor::ImplShow( bool bDrawDirect )
{
    ImplDoShow( bDrawDirect, false );
}

void vcl::Cursor::ImplHide( bool i_bStopTimer )
{
    assert( i_bStopTimer );
    ImplDoHide( !i_bStopTimer );
}

void vcl::Cursor::ImplResume( bool bRestore )
{
    ImplDoShow( false, bRestore );
}

bool vcl::Cursor::ImplSuspend()
{
    return ImplDoHide( true );
}

void vcl::Cursor::ImplNew()
{
    if ( mbVisible && mpData && mpData->mpWindow )
    {
        if ( mpData->mbCurVisible )
            ImplRestore();

        ImplDraw();
        if ( !mpWindow )
        {
            if ( mpData->maTimer.GetTimeout() != STYLE_CURSOR_NOBLINKTIME )
                mpData->maTimer.Start();
        }
    }
}

IMPL_LINK_NOARG_TYPED(vcl::Cursor, ImplTimerHdl, Timer *, void)
{
    if ( mpData->mbCurVisible )
        ImplRestore();
    else
        ImplDraw();
}

vcl::Cursor::Cursor()
{
    mpData          = NULL;
    mpWindow        = NULL;
    mnSlant         = 0;
    mnOrientation   = 0;
    mnDirection     = CursorDirection::NONE;
    mnStyle         = 0;
    mbVisible       = false;
}

vcl::Cursor::Cursor( const Cursor& rCursor ) :
    maSize( rCursor.maSize ),
    maPos( rCursor.maPos )
{
    mpData          = NULL;
    mpWindow        = NULL;
    mnSlant         = rCursor.mnSlant;
    mnOrientation   = rCursor.mnOrientation;
    mnDirection     = rCursor.mnDirection;
    mnStyle         = 0;
    mbVisible       = rCursor.mbVisible;
}

vcl::Cursor::~Cursor()
{
    if ( mpData )
    {
        if ( mpData->mbCurVisible )
            ImplRestore();

        delete mpData;
    }
}

void vcl::Cursor::SetStyle( sal_uInt16 nStyle )
{
    if ( mnStyle != nStyle )
    {
        mnStyle = nStyle;
        ImplNew();
    }
}

void vcl::Cursor::Show()
{
    if ( !mbVisible )
    {
        mbVisible = true;
        ImplShow();
    }
}

void vcl::Cursor::Hide()
{
    if ( mbVisible )
    {
        mbVisible = false;
        ImplHide( true );
    }
}

void vcl::Cursor::SetWindow( vcl::Window* pWindow )
{
    if ( mpWindow.get() != pWindow )
    {
        mpWindow = pWindow;
        ImplNew();
    }
}

void vcl::Cursor::SetPos( const Point& rPoint )
{
    if ( maPos != rPoint )
    {
        maPos = rPoint;
        ImplNew();
    }
}

void vcl::Cursor::SetSize( const Size& rSize )
{
    if ( maSize != rSize )
    {
        maSize = rSize;
        ImplNew();
    }
}

void vcl::Cursor::SetWidth( long nNewWidth )
{
    if ( maSize.Width() != nNewWidth )
    {
        maSize.Width() = nNewWidth;
        ImplNew();
    }
}

void vcl::Cursor::SetOrientation( short nNewOrientation )
{
    if ( mnOrientation != nNewOrientation )
    {
        mnOrientation = nNewOrientation;
        ImplNew();
    }
}

void vcl::Cursor::SetDirection( CursorDirection nNewDirection )
{
    if ( mnDirection != nNewDirection )
    {
        mnDirection = nNewDirection;
        ImplNew();
    }
}

vcl::Cursor& vcl::Cursor::operator=( const vcl::Cursor& rCursor )
{
    maPos           = rCursor.maPos;
    maSize          = rCursor.maSize;
    mnSlant         = rCursor.mnSlant;
    mnOrientation   = rCursor.mnOrientation;
    mnDirection     = rCursor.mnDirection;
    mbVisible       = rCursor.mbVisible;
    ImplNew();

    return *this;
}

bool vcl::Cursor::operator==( const vcl::Cursor& rCursor ) const
{
    return
        ((maPos         == rCursor.maPos)           &&
         (maSize        == rCursor.maSize)          &&
         (mnSlant       == rCursor.mnSlant)         &&
         (mnOrientation == rCursor.mnOrientation)   &&
         (mnDirection   == rCursor.mnDirection)     &&
         (mbVisible     == rCursor.mbVisible))
        ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
