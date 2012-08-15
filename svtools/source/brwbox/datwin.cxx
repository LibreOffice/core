/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include "datwin.hxx"

#include <vcl/svapp.hxx>

#include <vcl/help.hxx>
#include <vcl/image.hxx>

#include <tools/debug.hxx>

//===================================================================
void ButtonFrame::Draw( OutputDevice& rDev )
{
    Color aOldFillColor = rDev.GetFillColor();
    Color aOldLineColor = rDev.GetLineColor();

    const StyleSettings &rSettings = rDev.GetSettings().GetStyleSettings();
    Color aColLight( rSettings.GetLightColor() );
    Color aColShadow( rSettings.GetShadowColor() );
    Color aColFace( rSettings.GetFaceColor() );

    rDev.SetLineColor( aColFace );
    rDev.SetFillColor( aColFace );
    rDev.DrawRect( aRect );

    if( rDev.GetOutDevType() == OUTDEV_WINDOW )
    {
        Window *pWin = (Window*) &rDev;
        if( bPressed )
            pWin->DrawSelectionBackground( aRect, 0, sal_True, sal_False, sal_False );
    }
    else
    {
        rDev.SetLineColor( bPressed ? aColShadow : aColLight );
        rDev.DrawLine( aRect.TopLeft(), Point( aRect.Right(), aRect.Top() ) );
        rDev.DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom() - 1 ) );
        rDev.SetLineColor( bPressed ? aColLight : aColShadow );
        rDev.DrawLine( aRect.BottomRight(), Point( aRect.Right(), aRect.Top() ) );
        rDev.DrawLine( aRect.BottomRight(), Point( aRect.Left(), aRect.Bottom() ) );
    }

    if ( aText.Len() )
    {
        String aVal = rDev.GetEllipsisString(aText,aInnerRect.GetWidth() - 2*MIN_COLUMNWIDTH);

        Font aFont( rDev.GetFont() );
        sal_Bool bOldTransp = aFont.IsTransparent();
        if ( !bOldTransp )
        {
            aFont.SetTransparent( sal_True );
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
            aFont.SetTransparent(sal_False);
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

BrowserColumn::BrowserColumn( sal_uInt16 nItemId, const class Image &rImage,
                              const String& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom )
:   _nId( nItemId ),
    _nWidth( nWidthPixel ),
    _aImage( rImage ),
    _aTitle( rTitle ),
    _bFrozen( sal_False )
{
    double n = (double)_nWidth;
    n *= (double)rCurrentZoom.GetDenominator();
    n /= (double)rCurrentZoom.GetNumerator();
    _nOriginalWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}

BrowserColumn::~BrowserColumn()
{
}

//-------------------------------------------------------------------

void BrowserColumn::SetWidth(sal_uLong nNewWidthPixel, const Fraction& rCurrentZoom)
{
    _nWidth = nNewWidthPixel;
    double n = (double)_nWidth;
    n *= (double)rCurrentZoom.GetDenominator();
    n /= (double)rCurrentZoom.GetNumerator();
    _nOriginalWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}

//-------------------------------------------------------------------

void BrowserColumn::Draw( BrowseBox& rBox, OutputDevice& rDev, const Point& rPos, sal_Bool bCurs  )
{
    if ( _nId == 0 )
    {
        // paint handle column
        ButtonFrame( rPos, Size( Width()-1, rBox.GetDataRowHeight()-1 ),
                     String(), sal_False, bCurs, false ).Draw( rDev );
        Color aOldLineColor = rDev.GetLineColor();
        rDev.SetLineColor( Color( COL_BLACK ) );
        rDev.DrawLine(
            Point( rPos.X(), rPos.Y()+rBox.GetDataRowHeight()-1 ),
            Point( rPos.X() + Width() - 1, rPos.Y()+rBox.GetDataRowHeight()-1 ) );
        rDev.DrawLine(
            Point( rPos.X() + Width() - 1, rPos.Y() ),
            Point( rPos.X() + Width() - 1, rPos.Y()+rBox.GetDataRowHeight()-1 ) );
        rDev.SetLineColor( aOldLineColor );

        rBox.DoPaintField( rDev,
            Rectangle(
                Point( rPos.X() + 2, rPos.Y() + 2 ),
                Size( Width()-1, rBox.GetDataRowHeight()-1 ) ),
            GetId(),
            BrowseBox::BrowserColumnAccess() );
    }
    else
    {
        // paint data column
        long nWidth = Width() == LONG_MAX ? rBox.GetDataWindow().GetSizePixel().Width() : Width();

        rBox.DoPaintField( rDev,
            Rectangle(
                Point( rPos.X() + MIN_COLUMNWIDTH, rPos.Y() ),
                Size( nWidth-2*MIN_COLUMNWIDTH, rBox.GetDataRowHeight()-1 ) ),
            GetId(),
            BrowseBox::BrowserColumnAccess() );
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

BrowserDataWin::BrowserDataWin( BrowseBox* pParent )
    :Control( pParent, WinBits(WB_CLIPCHILDREN) )
    ,DragSourceHelper( this )
    ,DropTargetHelper( this )
    ,pHeaderBar( 0 )
    ,pEventWin( pParent )
    ,pCornerWin( 0 )
    ,pDtorNotify( 0 )
    ,bInPaint( sal_False )
    ,bInCommand( sal_False )
    ,bNoScrollBack( sal_False )
    ,bNoHScroll( sal_False )
    ,bNoVScroll( sal_False )
    ,bUpdateMode( sal_True )
    ,bResizeOnPaint( sal_False )
    ,bUpdateOnUnlock( sal_False )
    ,bInUpdateScrollbars( sal_False )
    ,bHadRecursion( sal_False )
    ,bOwnDataChangedHdl( sal_False )
    ,bCallingDropCallback( sal_False )
    ,nUpdateLock( 0 )
    ,nCursorHidden( 0 )
    ,m_nDragRowDividerLimit( 0 )
    ,m_nDragRowDividerOffset( 0 )
{
    aMouseTimer.SetTimeoutHdl( LINK( this, BrowserDataWin, RepeatedMouseMove ) );
    aMouseTimer.SetTimeout( 100 );
}

//-------------------------------------------------------------------
BrowserDataWin::~BrowserDataWin()
{
    if( pDtorNotify )
        *pDtorNotify = sal_True;

    for ( size_t i = 0, n = aInvalidRegion.size(); i < n; ++i )
        delete aInvalidRegion[ i ];
    aInvalidRegion.clear();
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
            bUpdateOnUnlock = sal_False;
        }
    }
}

//-------------------------------------------------------------------
void InitSettings_Impl( Window *pWin,
                     sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground )
{
    const StyleSettings& rStyleSettings =
            pWin->GetSettings().GetStyleSettings();

    if ( bFont )
    {
        Font aFont = rStyleSettings.GetFieldFont();
        if ( pWin->IsControlFont() )
            aFont.Merge( pWin->GetControlFont() );
        pWin->SetZoomedPointFont( aFont );
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
        bUpdateOnUnlock = sal_True;
}

//-------------------------------------------------------------------
void BrowserDataWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        if( !bOwnDataChangedHdl )
        {
            InitSettings_Impl( this, sal_True, sal_True, sal_True );
            Invalidate();
            InitSettings_Impl( GetParent(), sal_True, sal_True, sal_True );
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
        if ( bInPaint )
        {
            aInvalidRegion.push_back( new Rectangle( rRect ) );
            return;
        }
        bInPaint = sal_True;
        ( (BrowseBox*) GetParent() )->PaintData( *this, rRect );
        bInPaint = sal_False;
        DoOutstandingInvalidations();
    }
    else
        aInvalidRegion.push_back( new Rectangle( rRect ) );
}

//-------------------------------------------------------------------

BrowseEvent BrowserDataWin::CreateBrowseEvent( const Point& rPosPixel )
{
    BrowseBox *pBox = GetParent();

    // seek to row under mouse
    long nRelRow = rPosPixel.Y() < 0
            ? -1
            : rPosPixel.Y() / pBox->GetDataRowHeight();
    long nRow = nRelRow < 0 ? -1 : nRelRow + pBox->nTopRow;

    // find column under mouse
    long nMouseX = rPosPixel.X();
    long nColX = 0;
    size_t nCol;
    for ( nCol = 0;
          nCol < pBox->pCols->size() && nColX < GetSizePixel().Width();
          ++nCol )
        if ( (*pBox->pCols)[ nCol ]->IsFrozen() || nCol >= pBox->nFirstCol )
        {
            nColX += (*pBox->pCols)[ nCol ]->Width();
            if ( nMouseX < nColX )
                break;
        }
    sal_uInt16 nColId = BROWSER_INVALIDID;
    if ( nCol < pBox->pCols->size() )
        nColId = (*pBox->pCols)[ nCol ]->GetId();

    // compute the field rectangle and field relative MouseEvent
    Rectangle aFieldRect;
    if ( nCol < pBox->pCols->size() )
    {
        nColX -= (*pBox->pCols)[ nCol ]->Width();
        aFieldRect = Rectangle(
            Point( nColX, nRelRow * pBox->GetDataRowHeight() ),
            Size( (*pBox->pCols)[ nCol ]->Width(),
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
    nReturn = GetParent()->AcceptDrop( BrowserAcceptDropEvent( this, _rEvt ) );
    bCallingDropCallback = sal_False;
    return nReturn;
}

//-------------------------------------------------------------------
sal_Int8 BrowserDataWin::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    bCallingDropCallback = sal_True;
    sal_Int8 nReturn = DND_ACTION_NONE;
    nReturn = GetParent()->ExecuteDrop( BrowserExecuteDropEvent( this, _rEvt ) );
    bCallingDropCallback = sal_False;
    return nReturn;
}

//-------------------------------------------------------------------
void BrowserDataWin::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    if ( !GetParent()->bRowDividerDrag )
    {
        Point aEventPos( _rPosPixel );
        aEventPos.Y() += GetParent()->GetTitleHeight();
        GetParent()->StartDrag( _nAction, aEventPos );
    }
}

//-------------------------------------------------------------------
void BrowserDataWin::Command( const CommandEvent& rEvt )
{
    // scroll mouse event?
    BrowseBox *pBox = GetParent();
    if ( ( (rEvt.GetCommand() == COMMAND_WHEEL) ||
           (rEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
           (rEvt.GetCommand() == COMMAND_AUTOSCROLL) ) &&
         ( HandleScrollCommand( rEvt, &pBox->aHScroll, pBox->pVScroll ) ) )
      return;

    Point aEventPos( rEvt.GetMousePosPixel() );
    long nRow = pBox->GetRowAtYPosPixel( aEventPos.Y(), sal_False);
    MouseEvent aMouseEvt( aEventPos, 1, MOUSE_SELECT, MOUSE_LEFT );
    if ( COMMAND_CONTEXTMENU == rEvt.GetCommand() && rEvt.IsMouseEvent() &&
         nRow < pBox->GetRowCount() && !pBox->IsRowSelected(nRow) )
    {
        sal_Bool bDeleted = sal_False;
        pDtorNotify = &bDeleted;
        bInCommand = sal_True;
        MouseButtonDown( aMouseEvt );
        if( bDeleted )
            return;
        MouseButtonUp( aMouseEvt );
        if( bDeleted )
            return;
        pDtorNotify = 0;
        bInCommand = sal_False;
    }

    aEventPos.Y() += GetParent()->GetTitleHeight();
    CommandEvent aEvt( aEventPos, rEvt.GetCommand(),
                        rEvt.IsMouseEvent(), rEvt.GetData() );
    bInCommand = sal_True;
    sal_Bool bDeleted = sal_False;
    pDtorNotify = &bDeleted;
    GetParent()->Command( aEvt );
    if( bDeleted )
        return;
    pDtorNotify = 0;
    bInCommand = sal_False;

    if ( COMMAND_STARTDRAG == rEvt.GetCommand() )
        MouseButtonUp( aMouseEvt );

    Control::Command( rEvt );
}

//-------------------------------------------------------------------

sal_Bool BrowserDataWin::ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent )
{
    if ( ! (  GetParent()->IsInteractiveRowHeightEnabled()
           && ( _rEvent.GetRow() >= 0 )
           && ( _rEvent.GetRow() < GetParent()->GetRowCount() )
           && ( _rEvent.GetColumnId() == BrowseBox::HandleColumnId )
           )
       )
       return sal_False;

    long nDividerDistance = GetParent()->GetDataRowHeight() - ( _rEvent.GetPosPixel().Y() % GetParent()->GetDataRowHeight() );
    return ( nDividerDistance <= 4 );
}

//-------------------------------------------------------------------

void BrowserDataWin::MouseButtonDown( const MouseEvent& rEvt )
{
    aLastMousePos = OutputToScreenPixel( rEvt.GetPosPixel() );

    BrowserMouseEvent aBrowserEvent( this, rEvt );
    if ( ( aBrowserEvent.GetClicks() == 1 ) && ImplRowDividerHitTest( aBrowserEvent ) )
    {
        StartRowDividerDrag( aBrowserEvent.GetPosPixel() );
        return;
    }

    GetParent()->MouseButtonDown( BrowserMouseEvent( this, rEvt ) );
}

//-------------------------------------------------------------------

void BrowserDataWin::MouseMove( const MouseEvent& rEvt )
{
    // avoid pseudo MouseMoves
    Point aNewPos = OutputToScreenPixel( rEvt.GetPosPixel() );
    if ( ( aNewPos == aLastMousePos ) )
        return;
    aLastMousePos = aNewPos;

    // transform to a BrowseEvent
    BrowserMouseEvent aBrowserEvent( this, rEvt );
    GetParent()->MouseMove( aBrowserEvent );

    // pointer shape
    PointerStyle ePointerStyle = POINTER_ARROW;
    if ( ImplRowDividerHitTest( aBrowserEvent ) )
        ePointerStyle = POINTER_VSIZEBAR;
    SetPointer( Pointer( ePointerStyle ) );

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

IMPL_LINK_NOARG_INLINE_START(BrowserDataWin, RepeatedMouseMove)
{
    GetParent()->MouseMove( BrowserMouseEvent( this, aRepeatEvt ) );
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(BrowserDataWin, RepeatedMouseMove)

//-------------------------------------------------------------------

void BrowserDataWin::MouseButtonUp( const MouseEvent& rEvt )
{
    // avoid pseudo MouseMoves
    Point aNewPos = OutputToScreenPixel( rEvt.GetPosPixel() );
    aLastMousePos = aNewPos;

    // simulate a move to the current position
    MouseMove( rEvt );

    // actual button up handling
    ReleaseMouse();
    if ( aMouseTimer.IsActive() )
        aMouseTimer.Stop();
    GetParent()->MouseButtonUp( BrowserMouseEvent( this, rEvt ) );
}

//-------------------------------------------------------------------

void BrowserDataWin::StartRowDividerDrag( const Point& _rStartPos )
{
    long nDataRowHeight = GetParent()->GetDataRowHeight();
    // the exact separation pos of the two rows
    long nDragRowDividerCurrentPos = _rStartPos.Y();
    if ( ( nDragRowDividerCurrentPos % nDataRowHeight ) > nDataRowHeight / 2 )
        nDragRowDividerCurrentPos += nDataRowHeight;
    nDragRowDividerCurrentPos /= nDataRowHeight;
    nDragRowDividerCurrentPos *= nDataRowHeight;

    m_nDragRowDividerOffset = nDragRowDividerCurrentPos - _rStartPos.Y();

    m_nDragRowDividerLimit = nDragRowDividerCurrentPos - nDataRowHeight;

    GetParent()->bRowDividerDrag = sal_True;
    GetParent()->ImplStartTracking();

    Rectangle aDragSplitRect( 0, m_nDragRowDividerLimit, GetOutputSizePixel().Width(), nDragRowDividerCurrentPos );
    ShowTracking( aDragSplitRect, SHOWTRACK_SMALL );

    StartTracking();
}

//-------------------------------------------------------------------

void BrowserDataWin::Tracking( const TrackingEvent& rTEvt )
{
    if ( !GetParent()->bRowDividerDrag )
        return;

    Point aMousePos = rTEvt.GetMouseEvent().GetPosPixel();
    // stop resizing at our bottom line
    if ( aMousePos.Y() > GetOutputSizePixel().Height() )
        aMousePos.Y() = GetOutputSizePixel().Height();

    if ( rTEvt.IsTrackingEnded() )
    {
        HideTracking();
        GetParent()->bRowDividerDrag = sal_False;
        GetParent()->ImplEndTracking();

        if ( !rTEvt.IsTrackingCanceled() )
        {
            long nNewRowHeight = aMousePos.Y() + m_nDragRowDividerOffset - m_nDragRowDividerLimit;

            // care for minimum row height
            if ( nNewRowHeight < GetParent()->QueryMinimumRowHeight() )
                nNewRowHeight = GetParent()->QueryMinimumRowHeight();

            GetParent()->SetDataRowHeight( nNewRowHeight );
            GetParent()->RowHeightChanged();
        }
    }
    else
    {
        GetParent()->ImplTracking();

        long nDragRowDividerCurrentPos = aMousePos.Y() + m_nDragRowDividerOffset;

        // care for minimum row height
        if ( nDragRowDividerCurrentPos < m_nDragRowDividerLimit + GetParent()->QueryMinimumRowHeight() )
            nDragRowDividerCurrentPos = m_nDragRowDividerLimit + GetParent()->QueryMinimumRowHeight();

        Rectangle aDragSplitRect( 0, m_nDragRowDividerLimit, GetOutputSizePixel().Width(), nDragRowDividerCurrentPos );
        ShowTracking( aDragSplitRect, SHOWTRACK_SMALL );
    }
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

//===================================================================

BrowseEvent::BrowseEvent( Window* pWindow,
                          long nAbsRow, sal_uInt16 nColumn, sal_uInt16 nColumnId,
                          const Rectangle& rRect ):
    pWin(pWindow),
    nRow(nAbsRow),
    aRect(rRect),
    nCol(nColumn),
    nColId(nColumnId)
{
}

//===================================================================
BrowserMouseEvent::BrowserMouseEvent( BrowserDataWin *pWindow,
                          const MouseEvent& rEvt ):
    MouseEvent(rEvt),
    BrowseEvent( pWindow->CreateBrowseEvent( rEvt.GetPosPixel() ) )
{
}

//-------------------------------------------------------------------

BrowserMouseEvent::BrowserMouseEvent( Window *pWindow, const MouseEvent& rEvt,
                          long nAbsRow, sal_uInt16 nColumn, sal_uInt16 nColumnId,
                          const Rectangle& rRect ):
    MouseEvent(rEvt),
    BrowseEvent( pWindow, nAbsRow, nColumn, nColumnId, rRect )
{
}

//===================================================================

BrowserAcceptDropEvent::BrowserAcceptDropEvent( BrowserDataWin *pWindow, const AcceptDropEvent& rEvt )
    :AcceptDropEvent(rEvt)
    ,BrowseEvent( pWindow->CreateBrowseEvent( rEvt.maPosPixel ) )
{
}

//===================================================================

BrowserExecuteDropEvent::BrowserExecuteDropEvent( BrowserDataWin *pWindow, const ExecuteDropEvent& rEvt )
    :ExecuteDropEvent(rEvt)
    ,BrowseEvent( pWindow->CreateBrowseEvent( rEvt.maPosPixel ) )
{
}

//===================================================================

//-------------------------------------------------------------------

void BrowserDataWin::SetUpdateMode( sal_Bool bMode )
{
    DBG_ASSERT( !bUpdateMode || aInvalidRegion.empty(), "invalid region not empty" );
    if ( (bool) bMode == bUpdateMode )
        return;

    bUpdateMode = bMode;
    if ( bMode )
        DoOutstandingInvalidations();
}

//-------------------------------------------------------------------
void BrowserDataWin::DoOutstandingInvalidations()
{
    for ( size_t i = 0, n = aInvalidRegion.size(); i < n; ++i ) {
        Control::Invalidate( *aInvalidRegion[ i ] );
        delete aInvalidRegion[ i ];
    }
    aInvalidRegion.clear();
}

//-------------------------------------------------------------------

void BrowserDataWin::Invalidate( sal_uInt16 nFlags )
{
    if ( !GetUpdateMode() )
    {
        for ( size_t i = 0, n = aInvalidRegion.size(); i < n; ++i )
            delete aInvalidRegion[ i ];
        aInvalidRegion.clear();
        aInvalidRegion.push_back( new Rectangle( Point( 0, 0 ), GetOutputSizePixel() ) );
    }
    else
        Window::Invalidate( nFlags );
}

//-------------------------------------------------------------------

void BrowserDataWin::Invalidate( const Rectangle& rRect, sal_uInt16 nFlags )
{
    if ( !GetUpdateMode() )
        aInvalidRegion.push_back( new Rectangle( rRect ) );
    else
        Window::Invalidate( rRect, nFlags );
}

//===================================================================

void BrowserScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    sal_uLong nPos = GetThumbPos();
    if ( nPos != _nLastPos )
    {
        String aTip( rtl::OUString::valueOf(static_cast<sal_Int32>(nPos)) );
        aTip += '/';
        if ( _pDataWin->GetRealRowCount().Len() )
            aTip += _pDataWin->GetRealRowCount();
        else
            aTip += rtl::OUString::valueOf(static_cast<sal_Int32>(GetRangeMax()));

        Rectangle aRect( GetPointerPosPixel(), Size( GetTextHeight(), GetTextWidth( aTip ) ) );
        if ( _nTip )
            Help::UpdateTip( _nTip, this, aRect, aTip );
        else
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
