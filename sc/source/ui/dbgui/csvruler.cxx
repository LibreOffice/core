/*************************************************************************
 *
 *  $RCSfile: csvruler.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 18:05:52 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif
#pragma hdrstop

// ============================================================================

#ifndef _SC_CSVRULER_HXX
#include "csvruler.hxx"
#endif

#ifndef _SC_ACCESSIBLECSVCONTROL_HXX
#include "AccessibleCsvControl.hxx"
#endif


// ============================================================================

ScCsvRuler::ScCsvRuler( ScCsvControl& rParent ) :
    ScCsvControl( rParent ),
    mnPosCursorLast( 1 )
{
    EnableRTL( false ); // #107812# RTL
    InitColors();
    InitSizeData();
    maBackgrDev.SetFont( GetFont() );
    maRulerDev.SetFont( GetFont() );
}


// common ruler handling ------------------------------------------------------

void ScCsvRuler::SetPosSizePixel(
        sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, sal_uInt16 nFlags )
{
    if( nFlags & WINDOW_POSSIZE_HEIGHT )
        nHeight = GetTextHeight() + mnSplitSize + 2;
    ScCsvControl::SetPosSizePixel( nX, nY, nWidth, nHeight, nFlags );
}

void ScCsvRuler::ApplyLayout( const ScCsvLayoutData& rOldData )
{
    ScCsvDiff nDiff = GetLayoutData().GetDiff( rOldData ) & (CSV_DIFF_HORIZONTAL | CSV_DIFF_RULERCURSOR);
    if( nDiff == CSV_DIFF_EQUAL ) return;

    DisableRepaint();
    if( nDiff & CSV_DIFF_HORIZONTAL )
    {
        InitSizeData();
        if( GetRulerCursorPos() >= GetPosCount() )
            MoveCursor( GetPosCount() - 1 );
    }
    if( nDiff & CSV_DIFF_RULERCURSOR )
    {
        ImplInvertCursor( rOldData.mnPosCursor );
        ImplInvertCursor( GetRulerCursorPos() );
    }
    EnableRepaint();

    if( nDiff & CSV_DIFF_POSOFFSET )
        AccSendVisibleEvent();
}

void ScCsvRuler::InitColors()
{
    const StyleSettings& rSett = GetSettings().GetStyleSettings();
    maBackColor = rSett.GetFaceColor();
    maActiveColor = rSett.GetWindowColor();
    maTextColor = rSett.GetLabelTextColor();
    maSplitColor = maBackColor.IsDark() ? maTextColor : Color( COL_LIGHTRED );
    InvalidateGfx();
}

void ScCsvRuler::InitSizeData()
{
    maWinSize = GetSizePixel();

    mnSplitSize = (GetCharWidth() * 3 / 5) | 1; // make an odd number

    sal_Int32 nActiveWidth = Min( GetWidth() - GetHdrWidth(), GetPosCount() * GetCharWidth() );
    sal_Int32 nActiveHeight = GetTextHeight();

    maActiveRect.SetPos( Point( GetFirstX(), (GetHeight() - nActiveHeight - 1) / 2 ) );
    maActiveRect.SetSize( Size( nActiveWidth, nActiveHeight ) );

    maBackgrDev.SetOutputSizePixel( maWinSize );
    maRulerDev.SetOutputSizePixel( maWinSize );

    InvalidateGfx();
}

void ScCsvRuler::MoveCursor( sal_Int32 nPos, bool bScroll )
{
    DisableRepaint();
    if( bScroll )
        Execute( CSVCMD_MAKEPOSVISIBLE, nPos );
    Execute( CSVCMD_MOVERULERCURSOR, IsVisibleSplitPos( nPos ) ? nPos : CSV_POS_INVALID );
    EnableRepaint();
    AccSendCaretEvent();
}

void ScCsvRuler::MoveCursorRel( ScMoveMode eDir )
{
    if( GetRulerCursorPos() != CSV_POS_INVALID )
    {
        switch( eDir )
        {
            case MOVE_FIRST:
                MoveCursor( 1 );
            break;
            case MOVE_LAST:
                MoveCursor( GetPosCount() - 1 );
            break;
            case MOVE_PREV:
                if( GetRulerCursorPos() > 1 )
                    MoveCursor( GetRulerCursorPos() - 1 );
            break;
            case MOVE_NEXT:
                if( GetRulerCursorPos() < GetPosCount() - 1 )
                    MoveCursor( GetRulerCursorPos() + 1 );
            break;
        }
    }
}

void ScCsvRuler::MoveCursorToSplit( ScMoveMode eDir )
{
    if( GetRulerCursorPos() != CSV_POS_INVALID )
    {
        sal_uInt32 nIndex = CSV_VEC_NOTFOUND;
        switch( eDir )
        {
            case MOVE_FIRST:    nIndex = maSplits.LowerBound( 0 );                          break;
            case MOVE_LAST:     nIndex = maSplits.UpperBound( GetPosCount() );              break;
            case MOVE_PREV:     nIndex = maSplits.UpperBound( GetRulerCursorPos() - 1 );    break;
            case MOVE_NEXT:     nIndex = maSplits.LowerBound( GetRulerCursorPos() + 1 );    break;
        }
        sal_Int32 nPos = maSplits[ nIndex ];
        if( nPos != CSV_POS_INVALID )
            MoveCursor( nPos );
    }
}

void ScCsvRuler::ScrollVertRel( ScMoveMode eDir )
{
    sal_Int32 nLine = GetFirstVisLine();
    switch( eDir )
    {
        case MOVE_PREV:     --nLine;                        break;
        case MOVE_NEXT:     ++nLine;                        break;
        case MOVE_PREVPAGE: nLine -= GetVisLineCount() - 1; break;
        case MOVE_NEXTPAGE: nLine += GetVisLineCount() - 1; break;
    }
    Execute( CSVCMD_SETLINEOFFSET, nLine );
}


// split handling -------------------------------------------------------------

sal_Int32 ScCsvRuler::GetNoScrollPos( sal_Int32 nPos ) const
{
    sal_Int32 nNewPos = nPos;
    if( nNewPos != CSV_POS_INVALID )
    {
        if( nNewPos < GetFirstVisPos() + CSV_SCROLL_DIST )
        {
            sal_Int32 nScroll = (GetFirstVisPos() > 0) ? CSV_SCROLL_DIST : 0;
            nNewPos = Max( nPos, GetFirstVisPos() + nScroll );
        }
        else if( nNewPos > GetLastVisPos() - CSV_SCROLL_DIST - 1L )
        {
            sal_Int32 nScroll = (GetFirstVisPos() < GetMaxPosOffset()) ? CSV_SCROLL_DIST : 0;
            nNewPos = Min( nNewPos, GetLastVisPos() - nScroll - 1L );
        }
    }
    return nNewPos;
}

void ScCsvRuler::InsertSplit( sal_Int32 nPos )
{
    if( maSplits.Insert( nPos ) )
    {
        ImplDrawSplit( nPos );
        Repaint();
    }
}

void ScCsvRuler::RemoveSplit( sal_Int32 nPos )
{
    if( maSplits.Remove( nPos ) )
    {
        ImplEraseSplit( nPos );
        Repaint();
    }
}

void ScCsvRuler::MoveSplit( sal_Int32 nPos, sal_Int32 nNewPos )
{
    bool bRemove = maSplits.Remove( nPos );
    bool bInsert = maSplits.Insert( nNewPos );
    if( bRemove || bInsert )
    {
        ImplEraseSplit( nPos );
        ImplDrawSplit( nNewPos );
        Repaint();
    }
}

void ScCsvRuler::RemoveAllSplits()
{
    maSplits.Clear();
    Repaint( true );
}

sal_Int32 ScCsvRuler::FindEmptyPos( sal_Int32 nPos, ScMoveMode eDir ) const
{
    sal_Int32 nNewPos = nPos;
    if( nNewPos != CSV_POS_INVALID )
    {
        switch( eDir )
        {
            case MOVE_FIRST:
                nNewPos = Min( nPos, FindEmptyPos( 0, MOVE_NEXT ) );
            break;
            case MOVE_LAST:
                nNewPos = Max( nPos, FindEmptyPos( GetPosCount(), MOVE_PREV ) );
            break;
            case MOVE_PREV:
                while( HasSplit( --nNewPos ) );
            break;
            case MOVE_NEXT:
                while( HasSplit( ++nNewPos ) );
            break;
        }
    }
    return IsValidSplitPos( nNewPos ) ? nNewPos : CSV_POS_INVALID;
}

void ScCsvRuler::MoveCurrSplit( sal_Int32 nNewPos )
{
    DisableRepaint();
    Execute( CSVCMD_MOVESPLIT, GetRulerCursorPos(), nNewPos );
    MoveCursor( nNewPos );
    EnableRepaint();
}

void ScCsvRuler::MoveCurrSplitRel( ScMoveMode eDir )
{
    if( HasSplit( GetRulerCursorPos() ) )
    {
        sal_Int32 nNewPos = FindEmptyPos( GetRulerCursorPos(), eDir );
        if( nNewPos != CSV_POS_INVALID )
            MoveCurrSplit( nNewPos );
    }
}


// event handling -------------------------------------------------------------

void ScCsvRuler::Resize()
{
    ScCsvControl::Resize();
    InitSizeData();
    Repaint();
}

void ScCsvRuler::GetFocus()
{
    ScCsvControl::GetFocus();
    DisableRepaint();
    if( GetRulerCursorPos() == CSV_POS_INVALID )
        MoveCursor( GetNoScrollPos( mnPosCursorLast ) );
    EnableRepaint();
}

void ScCsvRuler::LoseFocus()
{
    ScCsvControl::LoseFocus();
    mnPosCursorLast = GetRulerCursorPos();
    MoveCursor( CSV_POS_INVALID );
}

void ScCsvRuler::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        InitColors();
        Repaint();
    }
    ScCsvControl::DataChanged( rDCEvt );
}

void ScCsvRuler::MouseButtonDown( const MouseEvent& rMEvt )
{
    DisableRepaint();
    if( !HasFocus() )
        GrabFocus();
    if( rMEvt.IsLeft() )
    {
        sal_Int32 nPos = GetPosFromX( rMEvt.GetPosPixel().X() );
        if( IsVisibleSplitPos( nPos ) )
            StartMouseTracking( nPos );
        ImplSetMousePointer( nPos );
    }
    EnableRepaint();
}

void ScCsvRuler::MouseMove( const MouseEvent& rMEvt )
{
    if( !rMEvt.IsModifierChanged() )
    {
        sal_Int32 nPos = GetPosFromX( rMEvt.GetPosPixel().X() );
        if( IsTracking() )
        {
            // on mouse tracking: keep position valid
            nPos = Max( Min( nPos, GetPosCount() - 1L ), 1L );
            MoveMouseTracking( nPos );
        }
        else
        {
            Point aPoint;
            Rectangle aRect( aPoint, maWinSize );
            if( !IsVisibleSplitPos( nPos ) || !aRect.IsInside( rMEvt.GetPosPixel() ) )
                // if focused, keep old cursor position for key input
                nPos = HasFocus() ? GetRulerCursorPos() : CSV_POS_INVALID;
            MoveCursor( nPos, false );
        }
        ImplSetMousePointer( nPos );
    }
}

void ScCsvRuler::Tracking( const TrackingEvent& rTEvt )
{
    if( rTEvt.IsTrackingEnded() || rTEvt.IsTrackingRepeat() )
        MouseMove( rTEvt.GetMouseEvent() );
    if( rTEvt.IsTrackingEnded() )
        EndMouseTracking( !rTEvt.IsTrackingCanceled() );
}

void ScCsvRuler::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = rKCode.GetCode();
    bool bNoMod = !rKCode.GetModifier();
    bool bShift = (rKCode.GetModifier() == KEY_SHIFT);
    bool bJump = (rKCode.GetModifier() == KEY_MOD1);
    bool bMove = (rKCode.GetModifier() == (KEY_MOD1 | KEY_SHIFT));

    ScMoveMode eHDir = GetHorzDirection( nCode, true );
    ScMoveMode eVDir = GetVertDirection( nCode, false );

    if( bNoMod )
    {
        if( eHDir != MOVE_NONE )
            MoveCursorRel( eHDir );
        else if( eVDir != MOVE_NONE )
            ScrollVertRel( eVDir );
        else switch( nCode )
        {
            case KEY_SPACE:     Execute( CSVCMD_TOGGLESPLIT, GetRulerCursorPos() ); break;
            case KEY_INSERT:    Execute( CSVCMD_INSERTSPLIT, GetRulerCursorPos() ); break;
            case KEY_DELETE:    Execute( CSVCMD_REMOVESPLIT, GetRulerCursorPos() ); break;
        }
    }
    else if( bJump && (eHDir != MOVE_NONE) )
        MoveCursorToSplit( eHDir );
    else if( bMove && (eHDir != MOVE_NONE) )
        MoveCurrSplitRel( eHDir );
    else if( bShift && (nCode == KEY_DELETE) )
        Execute( CSVCMD_REMOVEALLSPLITS );

    if( rKCode.GetGroup() != KEYGROUP_CURSOR )
        ScCsvControl::KeyInput( rKEvt );
}

void ScCsvRuler::StartMouseTracking( sal_Int32 nPos )
{
    mnPosMTStart = mnPosMTCurr = nPos;
    mbPosMTMoved = false;
    maOldSplits = maSplits;
    Execute( CSVCMD_INSERTSPLIT, nPos );
    if( HasSplit( nPos ) )
        StartTracking( STARTTRACK_BUTTONREPEAT );
}

void ScCsvRuler::MoveMouseTracking( sal_Int32 nPos )
{
    if( mnPosMTCurr != nPos )
    {
        DisableRepaint();
        MoveCursor( nPos );
        if( (mnPosMTCurr != mnPosMTStart) && maOldSplits.HasSplit( mnPosMTCurr ) )
            Execute( CSVCMD_INSERTSPLIT, nPos );
        else
            Execute( CSVCMD_MOVESPLIT, mnPosMTCurr, nPos );
        mnPosMTCurr = nPos;
        mbPosMTMoved = true;
        EnableRepaint();
    }
}

void ScCsvRuler::EndMouseTracking( bool bApply )
{
    if( bApply )    // tracking finished successfully
    {
        // remove on simple click on an existing split
        if( (mnPosMTCurr == mnPosMTStart) && maOldSplits.HasSplit( mnPosMTCurr ) && !mbPosMTMoved )
            Execute( CSVCMD_REMOVESPLIT, mnPosMTCurr );
    }
    else            // tracking cancelled
    {
        MoveCursor( mnPosMTStart );
        // move split to origin
        if( maOldSplits.HasSplit( mnPosMTStart ) )
            MoveMouseTracking( mnPosMTStart );
        // remove temporarily inserted split
        else if( !maOldSplits.HasSplit( mnPosMTCurr ) )
            Execute( CSVCMD_REMOVESPLIT, mnPosMTCurr );
    }
    mnPosMTStart = CSV_POS_INVALID;
}


// painting -------------------------------------------------------------------

void ScCsvRuler::Paint( const Rectangle& )
{
    Repaint();
}

void ScCsvRuler::ImplRedraw()
{
    if( IsVisible() )
    {
        if( !IsValidGfx() )
        {
            ValidateGfx();
            ImplDrawBackgrDev();
            ImplDrawRulerDev();
        }
        DrawOutDev( Point(), maWinSize, Point(), maWinSize, maRulerDev );
        ImplDrawTrackingRect();
    }
}

void ScCsvRuler::ImplDrawArea( sal_Int32 nPosX, sal_Int32 nWidth )
{
    maBackgrDev.SetLineColor();
    Rectangle aRect( Point( nPosX, 0 ), Size( nWidth, GetHeight() ) );
    maBackgrDev.SetFillColor( maBackColor );
    maBackgrDev.DrawRect( aRect );

    aRect = maActiveRect;
    aRect.Left() = Max( GetFirstX(), nPosX );
    aRect.Right() = Min( Min( GetX( GetPosCount() ), GetLastX() ), nPosX + nWidth - 1L );
    if( aRect.Left() <= aRect.Right() )
    {
        maBackgrDev.SetFillColor( maActiveColor );
        maBackgrDev.DrawRect( aRect );
    }

    maBackgrDev.SetLineColor( maTextColor );
    sal_Int32 nY = GetHeight() - 1;
    maBackgrDev.DrawLine( Point( nPosX, nY ), Point( nPosX + nWidth - 1, nY ) );
}

void ScCsvRuler::ImplDrawBackgrDev()
{
    ImplDrawArea( 0, GetWidth() );

    // scale
    maBackgrDev.SetLineColor( maTextColor );
    maBackgrDev.SetFillColor();
    sal_Int32 nPos;

    sal_Int32 nFirstPos = Max( GetPosFromX( 0 ) - 1L, 0L );
    sal_Int32 nLastPos = GetPosFromX( GetWidth() );
    sal_Int32 nY = (maActiveRect.Top() + maActiveRect.Bottom()) / 2;
    for( nPos = nFirstPos; nPos <= nLastPos; ++nPos )
    {
        sal_Int32 nX = GetX( nPos );
        if( nPos % 5 )
            maBackgrDev.DrawPixel( Point( nX, nY ) );
        else
            maBackgrDev.DrawLine( Point( nX, nY - 1 ), Point( nX, nY + 1 ) );
    }

    // texts
    maBackgrDev.SetTextColor( maTextColor );
    maBackgrDev.SetTextFillColor();
    for( nPos = ((nFirstPos + 9) / 10) * 10; nPos <= nLastPos; nPos += 10 )
    {
        String aText( String::CreateFromInt32( nPos ) );
        sal_Int32 nTextWidth = maBackgrDev.GetTextWidth( aText );
        sal_Int32 nTextX = GetX( nPos ) - nTextWidth / 2;
        ImplDrawArea( nTextX - 1, nTextWidth + 2 );
        maBackgrDev.DrawText( Point( nTextX, maActiveRect.Top() ), aText );
    }
}

void ScCsvRuler::ImplDrawSplit( sal_Int32 nPos )
{
    if( IsVisibleSplitPos( nPos ) )
    {
        Point aPos( GetX( nPos ) - mnSplitSize / 2, GetHeight() - mnSplitSize - 2 );
        Size aSize( mnSplitSize, mnSplitSize );
        maRulerDev.SetLineColor( maTextColor );
        maRulerDev.SetFillColor( maSplitColor );
        maRulerDev.DrawEllipse( Rectangle( aPos, aSize ) );
        maRulerDev.DrawPixel( Point( GetX( nPos ), GetHeight() - 2 ) );
    }
}

void ScCsvRuler::ImplEraseSplit( sal_Int32 nPos )
{
    if( IsVisibleSplitPos( nPos ) )
    {
        ImplInvertCursor( GetRulerCursorPos() );
        Point aPos( GetX( nPos ) - mnSplitSize / 2, 0 );
        Size aSize( mnSplitSize, GetHeight() );
        maRulerDev.DrawOutDev( aPos, aSize, aPos, aSize, maBackgrDev );
        ImplInvertCursor( GetRulerCursorPos() );
    }
}

void ScCsvRuler::ImplDrawRulerDev()
{
    maRulerDev.DrawOutDev( Point(), maWinSize, Point(), maWinSize, maBackgrDev );
    ImplInvertCursor( GetRulerCursorPos() );

    sal_uInt32 nFirst = maSplits.LowerBound( GetFirstVisPos() );
    sal_uInt32 nLast = maSplits.UpperBound( GetLastVisPos() );
    if( (nFirst != CSV_POS_INVALID) && (nLast != CSV_POS_INVALID) )
        for( sal_uInt32 nIndex = nFirst; nIndex <= nLast; ++nIndex )
            ImplDrawSplit( GetSplitPos( nIndex ) );
}

void ScCsvRuler::ImplInvertCursor( sal_Int32 nPos )
{
    if( IsVisibleSplitPos( nPos ) )
    {
        ImplInvertRect( maRulerDev, Rectangle( Point( GetX( nPos ) - 1, 0 ), Size( 3, GetHeight() - 1 ) ) );
        if( HasSplit( nPos ) )
            ImplDrawSplit( nPos );
    }
}

void ScCsvRuler::ImplDrawTrackingRect()
{
    if( HasFocus() )
        InvertTracking( Rectangle( 0, 0, GetWidth() - 1, GetHeight() - 2 ),
            SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
}

void ScCsvRuler::ImplSetMousePointer( sal_Int32 nPos )
{
    SetPointer( Pointer( HasSplit( nPos ) ? POINTER_HSPLIT : POINTER_ARROW ) );
}


// accessibility ==============================================================

ScAccessibleCsvControl* ScCsvRuler::ImplCreateAccessible()
{
    return new ScAccessibleCsvRuler( *this );
}


// ============================================================================

