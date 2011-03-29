/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/settings.hxx>
#include <vcl/window.hxx>
#include <vcl/window.h>
#include <tools/poly.hxx>
#include <vcl/cursor.hxx>


// =======================================================================

struct ImplCursorData
{
    AutoTimer       maTimer;            // Timer
    Point           maPixPos;           // Pixel-Position
    Point           maPixRotOff;        // Pixel-Offset-Position
    Size            maPixSize;          // Pixel-Size
    long            mnPixSlant;         // Pixel-Slant
    short           mnOrientation;      // Pixel-Orientation
    unsigned char   mnDirection;        // indicates writing direction
    sal_uInt16          mnStyle;            // Cursor-Style
    sal_Bool            mbCurVisible;       // Ist Cursor aktuell sichtbar
    Window*         mpWindow;           // Zugeordnetes Windows
};

// =======================================================================

static void ImplCursorInvert( ImplCursorData* pData )
{
    Window* pWindow  = pData->mpWindow;
    sal_Bool    bMapMode = pWindow->IsMapModeEnabled();
    pWindow->EnableMapMode( sal_False );
    sal_uInt16 nInvertStyle;
    if ( pData->mnStyle & CURSOR_SHADOW )
        nInvertStyle = INVERT_50;
    else
        nInvertStyle = 0;

    Rectangle aRect( pData->maPixPos, pData->maPixSize );
    if ( pData->mnDirection || pData->mnOrientation || pData->mnPixSlant )
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
            if ( pData->mnDirection )
            {
                Point pAry[7];
                int delta = 3*aRect.getWidth()+1;
                if( pData->mnDirection == CURSOR_DIRECTION_LTR )
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
                else if( pData->mnDirection == CURSOR_DIRECTION_RTL )
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
            pWindow->Invert( aPoly, nInvertStyle );
        }
    }
    else
        pWindow->Invert( aRect, nInvertStyle );
    pWindow->EnableMapMode( bMapMode );
}

// -----------------------------------------------------------------------

void Cursor::ImplDraw()
{
    if ( mpData && mpData->mpWindow && !mpData->mbCurVisible )
    {
        Window* pWindow         = mpData->mpWindow;
        mpData->maPixPos        = pWindow->LogicToPixel( maPos );
        mpData->maPixSize       = pWindow->LogicToPixel( maSize );
        mpData->mnPixSlant      = pWindow->LogicToPixel( Size( mnSlant, 0 ) ).Width();
        mpData->mnOrientation   = mnOrientation;
        mpData->mnDirection     = mnDirection;
        long nOffsetY           = pWindow->LogicToPixel( Size( 0, mnOffsetY ) ).Height();

        // Position um den Offset korrigieren
        mpData->maPixPos.Y() -= nOffsetY;
        mpData->maPixRotOff = mpData->maPixPos;
        mpData->maPixRotOff.Y() += nOffsetY;

        // Wenn groesse 0 ist, nehmen wir die breite, die in den
        // Settings eingestellt ist
        if ( !mpData->maPixSize.Width() )
            mpData->maPixSize.Width() = pWindow->GetSettings().GetStyleSettings().GetCursorSize();

        // Ausgabeflaeche berechnen und ausgeben
        ImplCursorInvert( mpData );
        mpData->mbCurVisible = sal_True;
    }
}

// -----------------------------------------------------------------------

void Cursor::ImplRestore()
{
    if ( mpData && mpData->mbCurVisible )
    {
        ImplCursorInvert( mpData );
        mpData->mbCurVisible = sal_False;
    }
}

// -----------------------------------------------------------------------

void Cursor::ImplShow( bool bDrawDirect, bool bRestore )
{
    if ( mbVisible )
    {
        Window* pWindow;
        if ( mpWindow )
            pWindow = mpWindow;
        else
        {
            // Gibt es ein aktives Fenster und ist der Cursor in dieses Fenster
            // selektiert, dann zeige den Cursor an
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
                mpData->mbCurVisible = sal_False;
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

// -----------------------------------------------------------------------

bool Cursor::ImplHide( bool i_bStopTimer )
{
    bool bWasCurVisible = false;
    if ( mpData && mpData->mpWindow )
    {
        bWasCurVisible = mpData->mbCurVisible;
        if ( mpData->mbCurVisible )
            ImplRestore();
    }

    if( mpData && i_bStopTimer )
    {
        mpData->maTimer.Stop();
        mpData->mpWindow = NULL;
    }

    return bWasCurVisible;
}

// -----------------------------------------------------------------------

void Cursor::ImplNew()
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

// -----------------------------------------------------------------------

IMPL_LINK( Cursor, ImplTimerHdl, AutoTimer*, EMPTYARG )
{
    if ( mpData->mbCurVisible )
        ImplRestore();
    else
        ImplDraw();
    return 0;
}

// =======================================================================

Cursor::Cursor()
{
    mpData          = NULL;
    mpWindow        = NULL;
    mnSlant         = 0;
    mnOffsetY       = 0;
    mnOrientation   = 0;
    mnDirection     = 0;
    mnStyle         = 0;
    mbVisible       = sal_False;
}

// -----------------------------------------------------------------------

Cursor::Cursor( const Cursor& rCursor ) :
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

// -----------------------------------------------------------------------

Cursor::~Cursor()
{
    if ( mpData )
    {
        if ( mpData->mbCurVisible )
            ImplRestore();

        delete mpData;
    }
}

// -----------------------------------------------------------------------

void Cursor::SetStyle( sal_uInt16 nStyle )
{
    if ( mnStyle != nStyle )
    {
        mnStyle = nStyle;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::Show()
{
    if ( !mbVisible )
    {
        mbVisible = sal_True;
        ImplShow();
    }
}

// -----------------------------------------------------------------------

void Cursor::Hide()
{
    if ( mbVisible )
    {
        mbVisible = sal_False;
        ImplHide( true );
    }
}

// -----------------------------------------------------------------------

void Cursor::SetWindow( Window* pWindow )
{
    if ( mpWindow != pWindow )
    {
        mpWindow = pWindow;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetPos( const Point& rPoint )
{
    if ( maPos != rPoint )
    {
        maPos = rPoint;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetOffsetY( long nNewOffsetY )
{
    if ( mnOffsetY != nNewOffsetY )
    {
        mnOffsetY = nNewOffsetY;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetSize( const Size& rSize )
{
    if ( maSize != rSize )
    {
        maSize = rSize;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetWidth( long nNewWidth )
{
    if ( maSize.Width() != nNewWidth )
    {
        maSize.Width() = nNewWidth;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetHeight( long nNewHeight )
{
    if ( maSize.Height() != nNewHeight )
    {
        maSize.Height() = nNewHeight;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetSlant( long nNewSlant )
{
    if ( mnSlant != nNewSlant )
    {
        mnSlant = nNewSlant;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetOrientation( short nNewOrientation )
{
    if ( mnOrientation != nNewOrientation )
    {
        mnOrientation = nNewOrientation;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

void Cursor::SetDirection( unsigned char nNewDirection )
{
    if ( mnDirection != nNewDirection )
    {
        mnDirection = nNewDirection;
        ImplNew();
    }
}

// -----------------------------------------------------------------------

Cursor& Cursor::operator=( const Cursor& rCursor )
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

// -----------------------------------------------------------------------

sal_Bool Cursor::operator==( const Cursor& rCursor ) const
{
    if ( (maPos         == rCursor.maPos)           &&
         (maSize        == rCursor.maSize)          &&
         (mnSlant       == rCursor.mnSlant)         &&
         (mnOrientation == rCursor.mnOrientation)   &&
         (mnDirection   == rCursor.mnDirection)     &&
         (mbVisible     == rCursor.mbVisible) )
        return sal_True;
    else
        return sal_False;
}
