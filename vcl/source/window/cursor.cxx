/*************************************************************************
 *
 *  $RCSfile: cursor.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:05:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_CURSOR_CXX

#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <settings.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <window.hxx>
#endif
#ifndef _SV_POLY_HXX
#include <poly.hxx>
#endif
#ifndef _SV_CURSOR_HXX
#include <cursor.hxx>
#endif

#pragma hdrstop

// =======================================================================

struct ImplCursorData
{
    AutoTimer       maTimer;            // Timer
    Point           maPixPos;           // Pixel-Position
    Point           maPixRotOff;        // Pixel-Offset-Position
    Size            maPixSize;          // Pixel-Size
    long            mnPixSlant;         // Pixel-Slant
    short           mnOrientation;      // Pixel-Orientation
    USHORT          mnStyle;            // Cursor-Style
    BOOL            mbCurVisible;       // Ist Cursor aktuell sichtbar
    Window*         mpWindow;           // Zugeordnetes Windows
};

// =======================================================================

static void ImplCursorInvert( ImplCursorData* pData )
{
    Window* pWindow  = pData->mpWindow;
    BOOL    bMapMode = pWindow->IsMapModeEnabled();
    pWindow->EnableMapMode( FALSE );
    USHORT nInvertStyle;
    if ( pData->mnStyle & CURSOR_SHADOW )
        nInvertStyle = INVERT_50;
    else
        nInvertStyle = 0;
    Rectangle aRect( pData->maPixPos, pData->maPixSize );
    if ( pData->mnOrientation || pData->mnPixSlant )
    {
        Polygon aPoly( aRect );
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
        if ( pData->mnOrientation )
            aPoly.Rotate( pData->maPixRotOff, pData->mnOrientation );
        pWindow->Invert( aPoly, nInvertStyle );
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
        mpData->mbCurVisible = TRUE;
    }
}

// -----------------------------------------------------------------------

void Cursor::ImplRestore()
{
    if ( mpData && mpData->mbCurVisible )
    {
        ImplCursorInvert( mpData );
        mpData->mbCurVisible = FALSE;
    }
}

// -----------------------------------------------------------------------

void Cursor::ImplShow( BOOL bDrawDirect )
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
            if ( !pWindow || (pWindow->mpCursor != this) || pWindow->mbInPaint )
                pWindow = NULL;
        }

        if ( pWindow )
        {
            if ( !mpData )
            {
                mpData = new ImplCursorData;
                mpData->mbCurVisible = FALSE;
                mpData->maTimer.SetTimeoutHdl( LINK( this, Cursor, ImplTimerHdl ) );
            }

            mpData->mpWindow    = pWindow;
            mpData->mnStyle     = mnStyle;
            if ( bDrawDirect )
                ImplDraw();

            if ( !mpWindow )
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

void Cursor::ImplHide()
{
    if ( mpData && mpData->mpWindow )
    {
        if ( mpData->mbCurVisible )
            ImplRestore();

        mpData->maTimer.Stop();
        mpData->mpWindow = NULL;
    }
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
    mnStyle         = 0;
    mbVisible       = FALSE;
}

// -----------------------------------------------------------------------

Cursor::Cursor( const Cursor& rCursor ) :
    maPos( rCursor.maPos ),
    maSize( rCursor.maSize )
{
    mpData          = NULL;
    mpWindow        = NULL;
    mnSlant         = rCursor.mnSlant;
    mnOrientation   = rCursor.mnOrientation;
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

void Cursor::SetStyle( USHORT nStyle )
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
        mbVisible = TRUE;
        ImplShow();
    }
}

// -----------------------------------------------------------------------

void Cursor::Hide()
{
    if ( mbVisible )
    {
        mbVisible = FALSE;
        ImplHide();
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

Cursor& Cursor::operator=( const Cursor& rCursor )
{
    maPos           = rCursor.maPos;
    maSize          = rCursor.maSize;
    mnSlant         = rCursor.mnSlant;
    mnOrientation   = rCursor.mnOrientation;
    mbVisible       = rCursor.mbVisible;
    ImplNew();

    return *this;
}

// -----------------------------------------------------------------------

BOOL Cursor::operator==( const Cursor& rCursor ) const
{
    if ( (maPos         == rCursor.maPos)           &&
         (maSize        == rCursor.maSize)          &&
         (mnSlant       == rCursor.mnSlant)         &&
         (mnOrientation == rCursor.mnOrientation)   &&
         (mbVisible     == rCursor.mbVisible) )
        return TRUE;
    else
        return FALSE;
}
