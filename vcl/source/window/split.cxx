/*************************************************************************
 *
 *  $RCSfile: split.cxx,v $
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

#define _SV_SPLIT_CXX

#ifndef _SV_RC_H
#include <rc.h>
#endif
#ifndef _SV_EVENT_HXX
#include <event.hxx>
#endif
#ifndef _SV_SPLIT_HXX
#include <split.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif

#pragma hdrstop

// =======================================================================

void Splitter::ImplInitData()
{
    mpRefWin          = NULL;
    mnSplitPos        = 0;
    mnLastSplitPos    = 0;
    mnStartSplitPos   = 0;
    mbDragFull        = FALSE;
}

// -----------------------------------------------------------------------

void Splitter::ImplInit( Window* pParent, WinBits nWinStyle )
{
    Window::ImplInit( pParent, nWinStyle, NULL );

    mpRefWin = pParent;

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    long nA = rSettings.GetScrollBarSize();
    long nB = rSettings.GetSplitSize();

    PointerStyle ePointerStyle;

    if ( nWinStyle & WB_HSCROLL )
    {
        ePointerStyle = POINTER_HSPLIT;
        mbHorzSplit = TRUE;
        SetSizePixel( Size( nB, nA ) );
    }
    else
    {
        ePointerStyle = POINTER_VSPLIT;
        mbHorzSplit = FALSE;
        SetSizePixel( Size( nA, nB ) );
    }

    SetPointer( Pointer( ePointerStyle ) );
    SetBackground( Wallpaper( Color( COL_BLACK ) ) );
}

// -----------------------------------------------------------------------

void Splitter::ImplSplitMousePos( Point& rPos )
{
    if ( mbHorzSplit )
    {
        if ( rPos.X() > maDragRect.Right()-1 )
            rPos.X() = maDragRect.Right()-1;
        if ( rPos.X() < maDragRect.Left()+1 )
            rPos.X() = maDragRect.Left()+1;
    }
    else
    {
        if ( rPos.Y() > maDragRect.Bottom()-1 )
            rPos.Y() = maDragRect.Bottom()-1;
        if ( rPos.Y() < maDragRect.Top()+1 )
            rPos.Y() = maDragRect.Top()+1;
    }
}

// -----------------------------------------------------------------------

void Splitter::ImplDrawSplitter()
{
    Rectangle aInvRect( maDragRect );

    if ( mbHorzSplit )
    {
        aInvRect.Left()     = maDragPos.X() - 1;
        aInvRect.Right()    = maDragPos.X() + 1;
    }
    else
    {
        aInvRect.Top()      = maDragPos.Y() - 1;
        aInvRect.Bottom()   = maDragPos.Y() + 1;
    }

    mpRefWin->InvertTracking( aInvRect, SHOWTRACK_SPLIT );
}

// -----------------------------------------------------------------------

Splitter::Splitter( Window* pParent, WinBits nStyle ) :
    Window( WINDOW_SPLITTER )
{
    ImplInitData();
    ImplInit( pParent, nStyle );
}

// -----------------------------------------------------------------------

Splitter::Splitter( Window* pParent, const ResId& rResId ) :
    Window( WINDOW_SPLITTER )
{
    ImplInitData();
    rResId.SetRT( RSC_SPLITTER );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInit( pParent, nStyle );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

// -----------------------------------------------------------------------

Splitter::~Splitter()
{
}

// -----------------------------------------------------------------------

void Splitter::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( rMEvt.GetClicks() == 2 )
    {
        if ( mnLastSplitPos != mnSplitPos )
        {
            StartSplit();
            Point aPos = rMEvt.GetPosPixel();
            if ( mbHorzSplit )
                aPos.X() = mnLastSplitPos;
            else
                aPos.Y() = mnLastSplitPos;
            ImplSplitMousePos( aPos );
            Splitting( aPos );
            ImplSplitMousePos( aPos );
            long nTemp = mnSplitPos;
            if ( mbHorzSplit )
                SetSplitPosPixel( aPos.X() );
            else
                SetSplitPosPixel( aPos.Y() );
            mnLastSplitPos = nTemp;
            Split();
        }
    }
    else
        StartDrag();
}

// -----------------------------------------------------------------------

void Splitter::Tracking( const TrackingEvent& rTEvt )
{
    if ( rTEvt.IsTrackingEnded() )
    {
        if ( !mbDragFull )
            ImplDrawSplitter();

        if ( !rTEvt.IsTrackingCanceled() )
        {
            long nNewPos;
            if ( mbHorzSplit )
                nNewPos = maDragPos.X();
            else
                nNewPos = maDragPos.Y();
            if ( nNewPos != mnStartSplitPos )
            {
                SetSplitPosPixel( nNewPos );
                mnLastSplitPos = 0;
                Split();
            }
        }
        else if ( mbDragFull )
        {
            SetSplitPosPixel( mnStartSplitPos );
            Split();
        }
        mnStartSplitPos = 0;
    }
    else
    {
        Point aNewPos = mpRefWin->ScreenToOutputPixel( OutputToScreenPixel( rTEvt.GetMouseEvent().GetPosPixel() ) );
        ImplSplitMousePos( aNewPos );
        Splitting( aNewPos );
        ImplSplitMousePos( aNewPos );

        if ( mbHorzSplit )
        {
            if ( aNewPos.X() == maDragPos.X() )
                return;
        }
        else
        {
            if ( aNewPos.Y() == maDragPos.Y() )
                return;
        }

        if ( mbDragFull )
        {
            maDragPos = aNewPos;
            long nNewPos;
            if ( mbHorzSplit )
                nNewPos = maDragPos.X();
            else
                nNewPos = maDragPos.Y();
            if ( nNewPos != mnSplitPos )
            {
                SetSplitPosPixel( nNewPos );
                mnLastSplitPos = 0;
                Split();
            }

            GetParent()->Update();
        }
        else
        {
            ImplDrawSplitter();
            maDragPos = aNewPos;
            ImplDrawSplitter();
        }
    }
}

// -----------------------------------------------------------------------

void Splitter::StartSplit()
{
    maStartSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void Splitter::Split()
{
    maSplitHdl.Call( this );
}

// -----------------------------------------------------------------------

void Splitter::Splitting( Point& /* rSplitPos */ )
{
}

// -----------------------------------------------------------------------

void Splitter::SetDragRectPixel( const Rectangle& rDragRect, Window* _pRefWin )
{
    maDragRect = rDragRect;
    if ( !_pRefWin )
        mpRefWin = GetParent();
    else
        mpRefWin = _pRefWin;
}

// -----------------------------------------------------------------------

void Splitter::SetSplitPosPixel( long nNewPos )
{
    mnSplitPos = nNewPos;
}

// -----------------------------------------------------------------------

void Splitter::SetLastSplitPosPixel( long nNewPos )
{
    mnLastSplitPos = nNewPos;
}

// -----------------------------------------------------------------------

void Splitter::StartDrag()
{
    if ( IsTracking() )
        return;

    StartSplit();

    // Tracking starten
    StartTracking();

    // Start-Positon ermitteln
    maDragPos = mpRefWin->GetPointerPosPixel();
    ImplSplitMousePos( maDragPos );
    Splitting( maDragPos );
    ImplSplitMousePos( maDragPos );
    if ( mbHorzSplit )
        mnStartSplitPos = maDragPos.X();
    else
        mnStartSplitPos = maDragPos.Y();

    mbDragFull = (Application::GetSettings().GetStyleSettings().GetDragFullOptions() & DRAGFULL_OPTION_SPLIT) != 0;
    if ( !mbDragFull )
        ImplDrawSplitter();
}
