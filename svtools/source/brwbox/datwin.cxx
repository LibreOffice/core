/*************************************************************************
 *
 *  $RCSfile: datwin.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-30 13:01:06 $
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

#include "datwin.hxx"

#pragma hdrstop

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif

#include <tools/debug.hxx>

DECLARE_LIST( BrowserColumns, BrowserColumn* );

//===================================================================

static String FitInWidth( OutputDevice& rWin, String aVal, ULONG nWidth, BOOL bAbbr )
{
    if ( nWidth < 8 )
        return String();
    ULONG nValWidth = rWin.GetTextWidth( aVal );
    if ( nValWidth > nWidth )
    {
        String aDots( "...", RTL_TEXTENCODING_IBM_850 );
        ULONG nDotsWidth = 2;
        if ( bAbbr )
            nDotsWidth = rWin.GetTextWidth( aDots );
        if ( nDotsWidth > nWidth )
            aVal.Erase();
        else
        {
            aVal.Erase( aVal.Len() - 1 );
            while ( aVal.Len() && rWin.GetTextWidth( aVal ) + nDotsWidth > nWidth )
                aVal.Erase( aVal.Len() - 1 );
            if ( bAbbr )
                aVal += aDots;
        }
    }

    return aVal;
}

//-------------------------------------------------------------------

void ButtonFrame::Draw( OutputDevice& rDev )
{
    Color aOldFillColor = rDev.GetFillColor();
    Color aOldLineColor = rDev.GetLineColor();

    const StyleSettings &rSettings = rDev.GetSettings().GetStyleSettings();
    Color aColLight( rSettings.GetLightColor() );
    Color aColShadow( rSettings.GetShadowColor() );
    Color aColFace( rSettings.GetFaceColor() );

    rDev.SetLineColor( bPressed ? aColShadow : aColLight );
    rDev.DrawLine( aRect.TopLeft(), Point( aRect.Right(), aRect.Top() ) );
    rDev.DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom() - 1 ) );
    rDev.SetLineColor( bPressed ? aColLight : aColShadow );
    rDev.DrawLine( aRect.BottomRight(), Point( aRect.Right(), aRect.Top() ) );
    rDev.DrawLine( aRect.BottomRight(), Point( aRect.Left(), aRect.Bottom() ) );

    rDev.SetLineColor( aColFace );
    rDev.SetFillColor( aColFace );
    rDev.DrawRect( aInnerRect );

    if ( aText.Len() )
    {
        String aVal( FitInWidth( rDev, aText, aInnerRect.GetWidth() - 2*MIN_COLUMNWIDTH, bAbbr ) );
        Font aFont( rDev.GetFont() );
        BOOL bOldTransp = aFont.IsTransparent();
        if ( !bOldTransp )
        {
            aFont.SetTransparent( TRUE );
            rDev.SetFont( aFont );
        }

        Color aOldColor = rDev.GetTextColor();
        if (m_bDrawDisabled)
            rDev.SetTextColor(rSettings.GetDisableColor());

        rDev.DrawText( Point(
            ( aInnerRect.Left() + aInnerRect.Right() ) / 2 - ( rDev.GetTextWidth(aVal) / 2 ),
            aInnerRect.Top() ), aVal );

        // restore settings
        if ( !bOldTransp )
        {
            aFont.SetTransparent(FALSE);
            rDev.SetFont( aFont );
        }
        if (m_bDrawDisabled)
            rDev.SetTextColor(aOldColor);
    }

    if ( bCurs )
    {
        rDev.SetLineColor( Color( COL_BLACK ) );
        rDev.SetFillColor();
        rDev.DrawRect( Rectangle(
            Point( aRect.Left(), aRect.Top() ), Point( aRect.Right(), aRect.Bottom() ) ) );
    }

    rDev.SetLineColor( aOldLineColor );
    rDev.SetFillColor( aOldFillColor );
}

//-------------------------------------------------------------------

BrowserColumn::BrowserColumn( USHORT nItemId, const class Image &rImage,
                              const String& rTitle, ULONG nWidthPixel, const Fraction& rCurrentZoom,
                              HeaderBarItemBits nFlags )
:   _nId( nItemId ),
    _nWidth( nWidthPixel ),
    _aImage( rImage ),
    _aTitle( rTitle ),
    _bFrozen( FALSE ),
    _nFlags( nFlags )
{
    double n = (double)_nWidth;
    n *= (double)rCurrentZoom.GetDenominator();
    n /= (double)rCurrentZoom.GetNumerator();
    _nOriginalWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}

//-------------------------------------------------------------------

void BrowserColumn::SetWidth(ULONG nNewWidthPixel, const Fraction& rCurrentZoom)
{
    _nWidth = nNewWidthPixel;
    double n = (double)_nWidth;
    n *= (double)rCurrentZoom.GetDenominator();
    n /= (double)rCurrentZoom.GetNumerator();
    _nOriginalWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}

//-------------------------------------------------------------------

void BrowserColumn::Draw( BrowseBox& rBox, OutputDevice& rDev, const Point& rPos, BOOL bCurs  )
{
    if ( _nId == 0 )
    {
        // paint handle column
        ButtonFrame( rPos, Size( Width()-1, rBox.GetDataRowHeight()-1 ),
                     String(), FALSE, bCurs,
                     0 != (BROWSER_COLUMN_TITLEABBREVATION&_nFlags) ).Draw( rDev );
        Color aOldLineColor = rDev.GetLineColor();
        rDev.SetLineColor( Color( COL_BLACK ) );
        rDev.DrawLine(
            Point( rPos.X(), rPos.Y()+rBox.GetDataRowHeight()-1 ),
            Point( rPos.X() + Width() - 1, rPos.Y()+rBox.GetDataRowHeight()-1 ) );
        rDev.DrawLine(
            Point( rPos.X() + Width() - 1, rPos.Y() ),
            Point( rPos.X() + Width() - 1, rPos.Y()+rBox.GetDataRowHeight()-1 ) );
        rDev.SetLineColor( aOldLineColor );

        if ( rBox.bHasBitmapHandle )
            rBox.PaintField( rDev,
                Rectangle(
                    Point( rPos.X() + 2, rPos.Y() + 2 ),
                    Size( Width()-1, rBox.GetDataRowHeight()-1 ) ),
                GetId() );
    }
    else
    {
        // paint data column
        long nWidth = Width() == LONG_MAX ? rBox.GetDataWindow().GetSizePixel().Width() : Width();

        rBox.PaintField( rDev,
            Rectangle(
                Point( rPos.X() + MIN_COLUMNWIDTH, rPos.Y() ),
                Size( nWidth-2*MIN_COLUMNWIDTH, rBox.GetDataRowHeight()-1 ) ),
            GetId() );
    }
}

//-------------------------------------------------------------------

void BrowserColumn::ZoomChanged(const Fraction& rNewZoom)
{
    double n = (double)_nOriginalWidth;
    n *= (double)rNewZoom.GetNumerator();
    n /= (double)rNewZoom.GetDenominator();

    _nWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}

//-------------------------------------------------------------------

BrowserDataWin::BrowserDataWin( BrowseBox* pParent ) :
    Control( pParent, WinBits(WB_CLIPCHILDREN) ),
    DragSourceHelper( this ),
    DropTargetHelper( this ),
    pHeaderBar( 0 ),
    pEventWin( pParent ),
    pCornerWin( 0 ),
    bInPaint( FALSE ),
    bInCommand( FALSE ),
    bNoScrollBack( FALSE ),
    bUpdateMode( TRUE ),
    bResizeOnPaint( FALSE ),
    bUpdateOnUnlock( FALSE ),
    bInUpdateScrollbars( FALSE ),
    bHadRecursion( FALSE ),
    bOwnDataChangedHdl( FALSE ),
    nUpdateLock( 0 ),
    nCursorHidden( 0 ),
    pDtorNotify( 0 ),
    bCallingDropCallback( FALSE )
{
    aMouseTimer.SetTimeoutHdl( LINK( this, BrowserDataWin, RepeatedMouseMove ) );
    aMouseTimer.SetTimeout( 100 );
}

//-------------------------------------------------------------------
BrowserDataWin::~BrowserDataWin()
{
    if( pDtorNotify )
        *pDtorNotify = TRUE;
    if ( IsMouseCaptured() )
        ReleaseMouse();
}

//-------------------------------------------------------------------
void BrowserDataWin::LeaveUpdateLock()
{
    if ( !--nUpdateLock )
    {
        DoOutstandingInvalidations();
        if (bUpdateOnUnlock )
        {
            Control::Update();
            bUpdateOnUnlock = FALSE;
        }
    }
}

//-------------------------------------------------------------------
void InitSettings_Impl( Window *pWin,
                     BOOL bFont, BOOL bForeground, BOOL bBackground )
{
    const StyleSettings& rStyleSettings =
            pWin->GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetAppFont();
        if ( pWin->IsControlFont() )
            aFont.Merge( pWin->GetControlFont() );
        pWin->SetPointFont( aFont );
    }

    if ( bFont || bForeground )
    {
        Color aTextColor = rStyleSettings.GetWindowTextColor();
        if ( pWin->IsControlForeground() )
            aTextColor = pWin->GetControlForeground();
        pWin->SetTextColor( aTextColor );
    }

    if ( bBackground )
    {
        if( pWin->IsControlBackground() )
            pWin->SetBackground( pWin->GetControlBackground() );
        else
            pWin->SetBackground( rStyleSettings.GetWindowColor() );
    }
}

//-------------------------------------------------------------------
void BrowserDataWin::Update()
{
    if ( !nUpdateLock )
        Control::Update();
    else
        bUpdateOnUnlock = TRUE;
}

//-------------------------------------------------------------------
void BrowserDataWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        if( !bOwnDataChangedHdl )
        {
            InitSettings_Impl( this, TRUE, TRUE, TRUE );
            Invalidate();
            InitSettings_Impl( GetParent(), TRUE, TRUE, TRUE );
            GetParent()->Invalidate();
            GetParent()->Resize();
        }
    }
    else
        Control::DataChanged( rDCEvt );
}

//-------------------------------------------------------------------
void BrowserDataWin::Paint( const Rectangle& rRect )
{
    if ( !nUpdateLock && GetUpdateMode() )
    {
        bInPaint = TRUE;
        ( (BrowseBox*) GetParent() )->PaintData( *this, rRect );
        bInPaint = FALSE;
    }
    else
        aInvalidRegion.Insert( new Rectangle( rRect ) );
}

//-------------------------------------------------------------------

BrowseEvent BrowserDataWin::CreateBrowseEvent( const Point& rPosPixel )
{
    BrowseBox *pBox = GetParent();

    // seek to row under mouse
    short nRelRow = rPosPixel.Y() < 0
            ? -1
            : rPosPixel.Y() / pBox->GetDataRowHeight();
    long nRow = nRelRow < 0 ? -1 : nRelRow + pBox->nTopRow;

    // find column under mouse
    long nMouseX = rPosPixel.X();
    long nColX = 0;
    USHORT nCol;
    for ( nCol = 0;
          nCol < pBox->pCols->Count() && nColX < GetSizePixel().Width();
          ++nCol )
        if ( pBox->pCols->GetObject(nCol)->IsFrozen() || nCol >= pBox->nFirstCol )
        {
            nColX += pBox->pCols->GetObject(nCol)->Width();
            if ( nMouseX < nColX )
                break;
        }
    USHORT nColId = BROWSER_INVALIDID;
    if ( nCol < pBox->pCols->Count() )
        nColId = pBox->pCols->GetObject(nCol)->GetId();

    // compute the field rectangle and field relative MouseEvent
    Rectangle aFieldRect;
    MouseEvent aRelEvt;
    if ( nCol < pBox->pCols->Count() )
    {
        nColX -= pBox->pCols->GetObject(nCol)->Width();
        aFieldRect = Rectangle(
            Point( nColX, nRelRow * pBox->GetDataRowHeight() ),
            Size( pBox->pCols->GetObject(nCol)->Width(),
                  pBox->GetDataRowHeight() ) );
    }

    // assemble and return the BrowseEvent
    return BrowseEvent( this, nRow, nCol, nColId, aFieldRect );
}

//-------------------------------------------------------------------
sal_Int8 BrowserDataWin::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    bCallingDropCallback = sal_True;
    sal_Int8 nReturn = DND_ACTION_NONE;
#if SUPD>627 || FS_PRIV_DEBUG
    nReturn = GetParent()->AcceptDrop( BrowserAcceptDropEvent( this, _rEvt ) );
#endif
    bCallingDropCallback = sal_False;
    return nReturn;
}

//-------------------------------------------------------------------
sal_Int8 BrowserDataWin::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    bCallingDropCallback = sal_True;
    sal_Int8 nReturn = DND_ACTION_NONE;
#if SUPD>627 || FS_PRIV_DEBUG
    return GetParent()->ExecuteDrop( BrowserExecuteDropEvent( this, _rEvt ) );
#endif
    bCallingDropCallback = sal_False;
    return nReturn;
}

//-------------------------------------------------------------------
void BrowserDataWin::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    Point aEventPos( _rPosPixel );
    aEventPos.Y() += GetParent()->GetTitleHeight();
#if SUPD>626 || FS_PRIV_DEBUG
    GetParent()->StartDrag( _nAction, aEventPos );
#endif
}

//-------------------------------------------------------------------
void BrowserDataWin::Command( const CommandEvent& rEvt )
{
    // Scrollmaus-Event?
    BrowseBox *pBox = GetParent();
    if ( ( (rEvt.GetCommand() == COMMAND_WHEEL) ||
           (rEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
           (rEvt.GetCommand() == COMMAND_AUTOSCROLL) ) &&
         ( HandleScrollCommand( rEvt, &pBox->aHScroll, pBox->pVScroll ) ) )
      return;

    Point aEventPos( rEvt.GetMousePosPixel() );
    long nRow = pBox->GetRowAtYPosPixel( aEventPos.Y(), FALSE);
    MouseEvent aMouseEvt( aEventPos, 1, MOUSE_SELECT, MOUSE_LEFT );
    if ( COMMAND_CONTEXTMENU == rEvt.GetCommand() && rEvt.IsMouseEvent() &&
         nRow < pBox->GetRowCount() && !pBox->IsRowSelected(nRow) )
    {
        BOOL bDeleted = FALSE;
        pDtorNotify = &bDeleted;
        bInCommand = TRUE;
        MouseButtonDown( aMouseEvt );
        if( bDeleted )
            return;
        MouseButtonUp( aMouseEvt );
        if( bDeleted )
            return;
        pDtorNotify = 0;
        bInCommand = FALSE;
    }

    aEventPos.Y() += GetParent()->GetTitleHeight();
    CommandEvent aEvt( aEventPos, rEvt.GetCommand(),
                        rEvt.IsMouseEvent(), rEvt.GetData() );
    bInCommand = TRUE;
    BOOL bDeleted = FALSE;
    pDtorNotify = &bDeleted;
    GetParent()->Command( aEvt );
    if( bDeleted )
        return;
    pDtorNotify = 0;
    bInCommand = FALSE;

    if ( COMMAND_STARTDRAG == rEvt.GetCommand() )
        MouseButtonUp( aMouseEvt );

    Control::Command( rEvt );
}

//-------------------------------------------------------------------

void BrowserDataWin::MouseButtonDown( const MouseEvent& rEvt )
{
    aLastMousePos = OutputToScreenPixel( rEvt.GetPosPixel() );
    CaptureMouse();
    GetParent()->MouseButtonDown( BrowserMouseEvent( this, rEvt ) );
}

//-------------------------------------------------------------------

void BrowserDataWin::MouseMove( const MouseEvent& rEvt )
{
    // Pseudo MouseMoves verhindern
    Point aNewPos = OutputToScreenPixel( rEvt.GetPosPixel() );
    if ( aNewPos == aLastMousePos )
        return;
    aLastMousePos = aNewPos;

    // Paint-Probleme abfangen
    if ( !IsMouseCaptured() )
        return;

    // transform to a BrowseEvent
    GetParent()->MouseMove( BrowserMouseEvent( this, rEvt ) );

    // dragging out of the visible area?
    if ( rEvt.IsLeft() &&
         ( rEvt.GetPosPixel().Y() > GetSizePixel().Height() ||
           rEvt.GetPosPixel().Y() < 0 ) )
    {
        // repeat the event
        aRepeatEvt = rEvt;
        aMouseTimer.Start();
    }
    else
        // killing old repeat-event
        if ( aMouseTimer.IsActive() )
            aMouseTimer.Stop();
}

//-------------------------------------------------------------------

IMPL_LINK_INLINE_START( BrowserDataWin, RepeatedMouseMove, void *, pvoid )
{
    GetParent()->MouseMove( BrowserMouseEvent( this, aRepeatEvt ) );
    return 0;
}
IMPL_LINK_INLINE_END( BrowserDataWin, RepeatedMouseMove, void *, pvoid )

//-------------------------------------------------------------------

void BrowserDataWin::MouseButtonUp( const MouseEvent& rEvt )
{
    // Pseudo MouseMoves verhindern
    Point aNewPos = OutputToScreenPixel( rEvt.GetPosPixel() );
    aLastMousePos = aNewPos;

    // Paint-Probleme abfangen
    if ( !IsMouseCaptured() )
        return;

    // Move an die aktuelle Position simulieren
    MouseMove( rEvt );

    // eigentliches Up-Handling
    ReleaseMouse();
    if ( aMouseTimer.IsActive() )
        aMouseTimer.Stop();
    GetParent()->MouseButtonUp( BrowserMouseEvent( this, rEvt ) );
}

//-------------------------------------------------------------------

void BrowserDataWin::KeyInput( const KeyEvent& rEvt )
{
    // pass to parent window
    if ( !GetParent()->ProcessKey( rEvt ) )
        Control::KeyInput( rEvt );
}

//-------------------------------------------------------------------

void BrowserDataWin::RequestHelp( const HelpEvent& rHEvt )
{
    pEventWin = this;
    GetParent()->RequestHelp( rHEvt );
    pEventWin = GetParent();
}

//-------------------------------------------------------------------

BOOL BrowserDataWin::Drop( const DropEvent& rEvt )
{
    return GetParent()->Drop( BrowserDropEvent( this, rEvt ) );
}

//-------------------------------------------------------------------

BOOL BrowserDataWin::QueryDrop( DropEvent& rEvt )
{
    BrowserDropEvent aBrwDEvt( this, rEvt );
    BOOL bRet = GetParent()->QueryDrop( aBrwDEvt );
    rEvt = aBrwDEvt;
    return bRet;
}

//===================================================================

BrowseEvent::BrowseEvent( Window* pWindow,
                          long nAbsRow, USHORT nColumn, USHORT nColumnId,
                          const Rectangle& rRect ):
    pWin(pWindow),
    nRow(nAbsRow),
    nCol(nColumn),
    nColId(nColumnId),
    aRect(rRect)
{
}

//===================================================================

BrowserMouseEvent::BrowserMouseEvent( BrowserDataWin *pWin,
                          const MouseEvent& rEvt ):
    MouseEvent(rEvt),
    BrowseEvent( pWin->CreateBrowseEvent( rEvt.GetPosPixel() ) )
{
}

//-------------------------------------------------------------------

BrowserMouseEvent::BrowserMouseEvent( Window *pWin, const MouseEvent& rEvt,
                          long nAbsRow, USHORT nColumn, USHORT nColumnId,
                          const Rectangle& rRect ):
    MouseEvent(rEvt),
    BrowseEvent( pWin, nAbsRow, nColumn, nColumnId, rRect )
{
}

//===================================================================

BrowserDropEvent::BrowserDropEvent( BrowserDataWin *pWin, const DropEvent& rEvt )
    :DropEvent(rEvt)
    ,BrowseEvent( pWin->CreateBrowseEvent( rEvt.GetPosPixel() ) )
{
}

//===================================================================

BrowserAcceptDropEvent::BrowserAcceptDropEvent( BrowserDataWin *pWin, const AcceptDropEvent& rEvt )
    :AcceptDropEvent(rEvt)
    ,BrowseEvent( pWin->CreateBrowseEvent( rEvt.maPosPixel ) )
{
}

//===================================================================

BrowserExecuteDropEvent::BrowserExecuteDropEvent( BrowserDataWin *pWin, const ExecuteDropEvent& rEvt )
    :ExecuteDropEvent(rEvt)
    ,BrowseEvent( pWin->CreateBrowseEvent( rEvt.maPosPixel ) )
{
}

//===================================================================

//-------------------------------------------------------------------

void BrowserDataWin::SetUpdateMode( BOOL bMode )
{
    DBG_ASSERT( !bUpdateMode || aInvalidRegion.Count() == 0,
                "invalid region not empty" );
    if ( bMode == bUpdateMode )
        return;

    bUpdateMode = bMode;
    if ( bMode )
        DoOutstandingInvalidations();
}

//-------------------------------------------------------------------
void BrowserDataWin::DoOutstandingInvalidations()
{
    for ( Rectangle* pRect = aInvalidRegion.First();
          pRect;
          pRect = aInvalidRegion.Next() )
    {
        Window::Invalidate( *pRect, INVALIDATE_NOCHILDREN /*OV*/);
        delete pRect;
    }
    aInvalidRegion.Clear();
}

//-------------------------------------------------------------------

void BrowserDataWin::Invalidate()
{
    if ( !GetUpdateMode() )
    {
        for ( Rectangle* pRect = aInvalidRegion.First();
              pRect;
              pRect = aInvalidRegion.Next() )
            delete pRect;
        aInvalidRegion.Clear();
        aInvalidRegion.Insert(
            new Rectangle( Point( 0, 0 ), GetOutputSizePixel() ) );
    }
    else
        Window::Invalidate(INVALIDATE_NOCHILDREN /*OV*/ );
}

//-------------------------------------------------------------------

void BrowserDataWin::Invalidate( const Rectangle& rRect )
{
    if ( !GetUpdateMode() )
        aInvalidRegion.Insert( new Rectangle( rRect ) );
    else
        Window::Invalidate( rRect, INVALIDATE_NOCHILDREN /*OV*/ );
}

//===================================================================

void BrowserScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    ULONG nPos = GetThumbPos();
    if ( nPos != _nLastPos )
    {
        if ( _nTip )
            Help::HideTip( _nTip );

        String aTip( String::CreateFromInt32(nPos) );
        aTip += '/';
        if ( _pDataWin->GetRealRowCount().Len() )
            aTip += _pDataWin->GetRealRowCount();
        else
            aTip += String::CreateFromInt32(GetRangeMax());
        Rectangle aRect( GetPointerPosPixel(), Size( GetTextHeight(), GetTextWidth( aTip ) ) );
        _nTip = Help::ShowTip( this, aRect, aTip );
        _nLastPos = nPos;
    }

    ScrollBar::Tracking( rTEvt );
}

//-------------------------------------------------------------------

void BrowserScrollBar::EndScroll()
{
    if ( _nTip )
        Help::HideTip( _nTip );
    _nTip = 0;
    ScrollBar::EndScroll();
}


