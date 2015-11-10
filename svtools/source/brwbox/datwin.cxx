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


#include "datwin.hxx"
#include <o3tl/numeric.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <rtl/string.hxx>
#include <tools/debug.hxx>
#include <tools/fract.hxx>


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
        vcl::Window *pWin = static_cast<vcl::Window*>( &rDev );
        if( bPressed )
            pWin->DrawSelectionBackground( aRect, 0, true, false, false );
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

    if ( !aText.isEmpty() )
    {
        OUString aVal = rDev.GetEllipsisString(aText,aInnerRect.GetWidth() - 2*MIN_COLUMNWIDTH);

        vcl::Font aFont( rDev.GetFont() );
        bool bOldTransp = aFont.IsTransparent();
        if ( !bOldTransp )
        {
            aFont.SetTransparent( true );
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
            aFont.SetTransparent(false);
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

BrowserColumn::BrowserColumn( sal_uInt16 nItemId, const class Image &rImage,
                              const OUString& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom )
:   _nId( nItemId ),
    _nWidth( nWidthPixel ),
    _aImage( rImage ),
    _aTitle( rTitle ),
    _bFrozen( false )
{
    double n = (double)_nWidth;
    n *= (double)rCurrentZoom.GetDenominator();
    if (!rCurrentZoom.GetNumerator())
        throw o3tl::divide_by_zero();
    n /= (double)rCurrentZoom.GetNumerator();
    _nOriginalWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}

BrowserColumn::~BrowserColumn()
{
}

void BrowserColumn::SetWidth(sal_uLong nNewWidthPixel, const Fraction& rCurrentZoom)
{
    _nWidth = nNewWidthPixel;
    // Avoid overflow when called with LONG_MAX from
    // BrowseBox::AutoSizeLastColumn:
    if (_nWidth == LONG_MAX)
    {
        _nOriginalWidth = _nWidth;
    }
    else
    {
        double n = (double)_nWidth;
        n *= (double)rCurrentZoom.GetDenominator();
        if (!rCurrentZoom.GetNumerator())
            throw o3tl::divide_by_zero();
        n /= (double)rCurrentZoom.GetNumerator();
        _nOriginalWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
    }
}

void BrowserColumn::Draw( BrowseBox& rBox, OutputDevice& rDev, const Point& rPos, bool bCurs  )
{
    if ( _nId == 0 )
    {
        // paint handle column
        ButtonFrame( rPos, Size( Width()-1, rBox.GetDataRowHeight()-1 ),
                     "", false, bCurs, false ).Draw( rDev );
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



void BrowserColumn::ZoomChanged(const Fraction& rNewZoom)
{
    double n = (double)_nOriginalWidth;
    n *= (double)rNewZoom.GetNumerator();
    n /= (double)rNewZoom.GetDenominator();

    _nWidth = n>0 ? (long)(n+0.5) : -(long)(-n+0.5);
}



BrowserDataWin::BrowserDataWin( BrowseBox* pParent )
    :Control( pParent, WinBits(WB_CLIPCHILDREN) )
    ,DragSourceHelper( this )
    ,DropTargetHelper( this )
    ,pHeaderBar( nullptr )
    ,pEventWin( pParent )
    ,pCornerWin( nullptr )
    ,bInDtor( false )
    ,bInPaint( false )
    ,bInCommand( false )
    ,bNoScrollBack( false )
    ,bNoHScroll( false )
    ,bNoVScroll( false )
    ,bAutoHScroll(false)
    ,bAutoVScroll(false)
    ,bUpdateMode( true )
    ,bAutoSizeLastCol(false)
    ,bResizeOnPaint( false )
    ,bUpdateOnUnlock( false )
    ,bInUpdateScrollbars( false )
    ,bHadRecursion( false )
    ,bOwnDataChangedHdl( false )
    ,bCallingDropCallback( false )
    ,nUpdateLock( 0 )
    ,nCursorHidden( 0 )
    ,m_nDragRowDividerLimit( 0 )
    ,m_nDragRowDividerOffset( 0 )
{
    aMouseTimer.SetTimeoutHdl( LINK( this, BrowserDataWin, RepeatedMouseMove ) );
    aMouseTimer.SetTimeout( 100 );
}


BrowserDataWin::~BrowserDataWin()
{
    disposeOnce();
}

void BrowserDataWin::dispose()
{
    bInDtor = true;

    for ( size_t i = 0, n = aInvalidRegion.size(); i < n; ++i )
        delete aInvalidRegion[ i ];
    aInvalidRegion.clear();
    pHeaderBar.clear();
    pEventWin.clear();
    pCornerWin.clear();
    Control::dispose();
}


void BrowserDataWin::LeaveUpdateLock()
{
    if ( !--nUpdateLock )
    {
        DoOutstandingInvalidations();
        if (bUpdateOnUnlock )
        {
            Control::Update();
            bUpdateOnUnlock = false;
        }
    }
}

void InitSettings_Impl(vcl::Window* pWin, bool bFont, bool bForeground, bool bBackground)
{
    const StyleSettings& rStyleSettings = pWin->GetSettings().GetStyleSettings();

    if (bFont)
        pWin->ApplyControlFont(*pWin, rStyleSettings.GetFieldFont());

    if (bFont || bForeground)
        pWin->ApplyControlForeground(*pWin, rStyleSettings.GetWindowTextColor());

    if (bBackground)
        pWin->ApplyControlBackground(*pWin, rStyleSettings.GetWindowColor());
}


void BrowserDataWin::Update()
{
    if ( !nUpdateLock )
        Control::Update();
    else
        bUpdateOnUnlock = true;
}


void BrowserDataWin::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) &&
         (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        if( !bOwnDataChangedHdl )
        {
            InitSettings_Impl(this);
            Invalidate();
            InitSettings_Impl(GetParent());
            GetParent()->Invalidate();
            GetParent()->Resize();
        }
    }
    else
        Control::DataChanged( rDCEvt );
}


void BrowserDataWin::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    if (!nUpdateLock && GetUpdateMode())
    {
        if (bInPaint)
        {
            aInvalidRegion.push_back(new Rectangle(rRect));
            return;
        }
        bInPaint = true;
        GetParent()->PaintData(*this, rRenderContext, rRect);
        bInPaint = false;
        DoOutstandingInvalidations();
    }
    else
    {
        aInvalidRegion.push_back(new Rectangle(rRect));
    }
}



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


sal_Int8 BrowserDataWin::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    bCallingDropCallback = true;
    sal_Int8 nReturn = GetParent()->AcceptDrop( BrowserAcceptDropEvent( this, _rEvt ) );
    bCallingDropCallback = false;
    return nReturn;
}


sal_Int8 BrowserDataWin::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    bCallingDropCallback = true;
    sal_Int8 nReturn = GetParent()->ExecuteDrop( BrowserExecuteDropEvent( this, _rEvt ) );
    bCallingDropCallback = false;
    return nReturn;
}


void BrowserDataWin::StartDrag( sal_Int8 _nAction, const Point& _rPosPixel )
{
    if ( !GetParent()->bRowDividerDrag )
    {
        Point aEventPos( _rPosPixel );
        aEventPos.Y() += GetParent()->GetTitleHeight();
        GetParent()->StartDrag( _nAction, aEventPos );
    }
}


void BrowserDataWin::Command( const CommandEvent& rEvt )
{
    // scroll mouse event?
    BrowseBox *pBox = GetParent();
    if ( ( (rEvt.GetCommand() == CommandEventId::Wheel) ||
           (rEvt.GetCommand() == CommandEventId::StartAutoScroll) ||
           (rEvt.GetCommand() == CommandEventId::AutoScroll) ) &&
         ( HandleScrollCommand( rEvt, pBox->aHScroll.get(), pBox->pVScroll ) ) )
      return;

    Point aEventPos( rEvt.GetMousePosPixel() );
    long nRow = pBox->GetRowAtYPosPixel( aEventPos.Y(), false);
    MouseEvent aMouseEvt( aEventPos, 1, MouseEventModifiers::SELECT, MOUSE_LEFT );
    if ( CommandEventId::ContextMenu == rEvt.GetCommand() && rEvt.IsMouseEvent() &&
         nRow < pBox->GetRowCount() && !pBox->IsRowSelected(nRow) )
    {
        bInCommand = true;
        MouseButtonDown( aMouseEvt );
        if( bInDtor )
            return;
        MouseButtonUp( aMouseEvt );
        if( bInDtor )
            return;
        bInCommand = false;
    }

    aEventPos.Y() += GetParent()->GetTitleHeight();
    CommandEvent aEvt( aEventPos, rEvt.GetCommand(),
                        rEvt.IsMouseEvent(), rEvt.GetEventData() );
    bInCommand = true;
    GetParent()->Command( aEvt );
    if( bInDtor )
        return;
    bInCommand = false;

    if ( CommandEventId::StartDrag == rEvt.GetCommand() )
        MouseButtonUp( aMouseEvt );

    Control::Command( rEvt );
}



bool BrowserDataWin::ImplRowDividerHitTest( const BrowserMouseEvent& _rEvent )
{
    if ( ! (  GetParent()->IsInteractiveRowHeightEnabled()
           && ( _rEvent.GetRow() >= 0 )
           && ( _rEvent.GetRow() < GetParent()->GetRowCount() )
           && ( _rEvent.GetColumnId() == BrowseBox::HandleColumnId )
           )
       )
       return false;

    long nDividerDistance = GetParent()->GetDataRowHeight() - ( _rEvent.GetPosPixel().Y() % GetParent()->GetDataRowHeight() );
    return ( nDividerDistance <= 4 );
}



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
    PointerStyle ePointerStyle = PointerStyle::Arrow;
    if ( ImplRowDividerHitTest( aBrowserEvent ) )
        ePointerStyle = PointerStyle::VSizeBar;
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



IMPL_LINK_NOARG_TYPED(BrowserDataWin, RepeatedMouseMove, Timer *, void)
{
    GetParent()->MouseMove( BrowserMouseEvent( this, aRepeatEvt ) );
}

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

    GetParent()->bRowDividerDrag = true;
    GetParent()->ImplStartTracking();

    Rectangle aDragSplitRect( 0, m_nDragRowDividerLimit, GetOutputSizePixel().Width(), nDragRowDividerCurrentPos );
    ShowTracking( aDragSplitRect );

    StartTracking();
}



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
        GetParent()->bRowDividerDrag = false;
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
        ShowTracking( aDragSplitRect );
    }
}



void BrowserDataWin::KeyInput( const KeyEvent& rEvt )
{
    // pass to parent window
    if ( !GetParent()->ProcessKey( rEvt ) )
        Control::KeyInput( rEvt );
}



void BrowserDataWin::RequestHelp( const HelpEvent& rHEvt )
{
    pEventWin = this;
    GetParent()->RequestHelp( rHEvt );
    pEventWin = GetParent();
}



BrowseEvent::BrowseEvent( vcl::Window* pWindow,
                          long nAbsRow, sal_uInt16 nColumn, sal_uInt16 nColumnId,
                          const Rectangle& rRect ):
    pWin(pWindow),
    nRow(nAbsRow),
    aRect(rRect),
    nCol(nColumn),
    nColId(nColumnId)
{
}


BrowserMouseEvent::BrowserMouseEvent( BrowserDataWin *pWindow,
                          const MouseEvent& rEvt ):
    MouseEvent(rEvt),
    BrowseEvent( pWindow->CreateBrowseEvent( rEvt.GetPosPixel() ) )
{
}



BrowserMouseEvent::BrowserMouseEvent( vcl::Window *pWindow, const MouseEvent& rEvt,
                          long nAbsRow, sal_uInt16 nColumn, sal_uInt16 nColumnId,
                          const Rectangle& rRect ):
    MouseEvent(rEvt),
    BrowseEvent( pWindow, nAbsRow, nColumn, nColumnId, rRect )
{
}



BrowserAcceptDropEvent::BrowserAcceptDropEvent( BrowserDataWin *pWindow, const AcceptDropEvent& rEvt )
    :AcceptDropEvent(rEvt)
    ,BrowseEvent( pWindow->CreateBrowseEvent( rEvt.maPosPixel ) )
{
}



BrowserExecuteDropEvent::BrowserExecuteDropEvent( BrowserDataWin *pWindow, const ExecuteDropEvent& rEvt )
    :ExecuteDropEvent(rEvt)
    ,BrowseEvent( pWindow->CreateBrowseEvent( rEvt.maPosPixel ) )
{
}





void BrowserDataWin::SetUpdateMode( bool bMode )
{
    DBG_ASSERT( !bUpdateMode || aInvalidRegion.empty(), "invalid region not empty" );
    if ( (bool) bMode == bUpdateMode )
        return;

    bUpdateMode = bMode;
    if ( bMode )
        DoOutstandingInvalidations();
}


void BrowserDataWin::DoOutstandingInvalidations()
{
    for ( size_t i = 0, n = aInvalidRegion.size(); i < n; ++i ) {
        Control::Invalidate( *aInvalidRegion[ i ] );
        delete aInvalidRegion[ i ];
    }
    aInvalidRegion.clear();
}



void BrowserDataWin::Invalidate( InvalidateFlags nFlags )
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



void BrowserDataWin::Invalidate( const Rectangle& rRect, InvalidateFlags nFlags )
{
    if ( !GetUpdateMode() )
        aInvalidRegion.push_back( new Rectangle( rRect ) );
    else
        Window::Invalidate( rRect, nFlags );
}

BrowserScrollBar::~BrowserScrollBar()
{
    disposeOnce();
}

void BrowserScrollBar::dispose()
{
    _pDataWin.clear();
    ScrollBar::dispose();
}

void BrowserScrollBar::Tracking( const TrackingEvent& rTEvt )
{
    sal_uLong nPos = GetThumbPos();
    if ( nPos != _nLastPos )
    {
        OUString aTip( OUString::number(nPos) );
        aTip += "/";
        if ( !_pDataWin->GetRealRowCount().isEmpty() )
            aTip += _pDataWin->GetRealRowCount();
        else
            aTip += OUString::number(GetRangeMax());

        Rectangle aRect(GetPointerPosPixel(), Size(GetTextWidth(aTip), GetTextHeight()));
        if ( _nTip )
            Help::UpdateTip( _nTip, this, aRect, aTip );
        else
            _nTip = Help::ShowTip( this, aRect, aTip );
        _nLastPos = nPos;
    }

    ScrollBar::Tracking( rTEvt );
}



void BrowserScrollBar::EndScroll()
{
    if ( _nTip )
        Help::HideTip( _nTip );
    _nTip = 0;
    ScrollBar::EndScroll();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
