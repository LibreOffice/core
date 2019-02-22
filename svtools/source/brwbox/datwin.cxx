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
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <vcl/image.hxx>
#include <vcl/settings.hxx>
#include <vcl/ptrstyle.hxx>
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

    if( rDev.GetOutDevType() != OUTDEV_WINDOW )
    {
        rDev.SetLineColor( aColLight );
        rDev.DrawLine( aRect.TopLeft(), Point( aRect.Right(), aRect.Top() ) );
        rDev.DrawLine( aRect.TopLeft(), Point( aRect.Left(), aRect.Bottom() - 1 ) );
        rDev.SetLineColor( aColShadow );
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

    rDev.SetLineColor( aOldLineColor );
    rDev.SetFillColor( aOldFillColor );
}

BrowserColumn::BrowserColumn( sal_uInt16 nItemId,
                              const OUString& rTitle, sal_uLong nWidthPixel, const Fraction& rCurrentZoom )
:   _nId( nItemId ),
    _nWidth( nWidthPixel ),
    _aTitle( rTitle ),
    _bFrozen( false )
{
    double n = static_cast<double>(_nWidth);
    n *= static_cast<double>(rCurrentZoom.GetDenominator());
    if (!rCurrentZoom.GetNumerator())
        throw o3tl::divide_by_zero();
    n /= static_cast<double>(rCurrentZoom.GetNumerator());
    _nOriginalWidth = n>0 ? static_cast<long>(n+0.5) : -static_cast<long>(-n+0.5);
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
        double n = static_cast<double>(_nWidth);
        n *= static_cast<double>(rCurrentZoom.GetDenominator());
        if (!rCurrentZoom.GetNumerator())
            throw o3tl::divide_by_zero();
        n /= static_cast<double>(rCurrentZoom.GetNumerator());
        _nOriginalWidth = n>0 ? static_cast<long>(n+0.5) : -static_cast<long>(-n+0.5);
    }
}

void BrowserColumn::Draw( BrowseBox const & rBox, OutputDevice& rDev, const Point& rPos  )
{
    if ( _nId == 0 )
    {
        // paint handle column
        ButtonFrame( rPos, Size( Width()-1, rBox.GetDataRowHeight()-1 ),
                     "", false ).Draw( rDev );
        Color aOldLineColor = rDev.GetLineColor();
        rDev.SetLineColor( COL_BLACK );
        rDev.DrawLine(
            Point( rPos.X(), rPos.Y()+rBox.GetDataRowHeight()-1 ),
            Point( rPos.X() + Width() - 1, rPos.Y()+rBox.GetDataRowHeight()-1 ) );
        rDev.DrawLine(
            Point( rPos.X() + Width() - 1, rPos.Y() ),
            Point( rPos.X() + Width() - 1, rPos.Y()+rBox.GetDataRowHeight()-1 ) );
        rDev.SetLineColor( aOldLineColor );

        rBox.DoPaintField( rDev,
            tools::Rectangle(
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
            tools::Rectangle(
                Point( rPos.X() + MIN_COLUMNWIDTH, rPos.Y() ),
                Size( nWidth-2*MIN_COLUMNWIDTH, rBox.GetDataRowHeight()-1 ) ),
            GetId(),
            BrowseBox::BrowserColumnAccess() );
    }
}


void BrowserColumn::ZoomChanged(const Fraction& rNewZoom)
{
    double n(_nOriginalWidth * rNewZoom);
    _nWidth = n>0 ? static_cast<long>(n+0.5) : -static_cast<long>(-n+0.5);
}


BrowserDataWin::BrowserDataWin( BrowseBox* pParent )
    :Control( pParent, WB_CLIPCHILDREN )
    ,DragSourceHelper( this )
    ,DropTargetHelper( this )
    ,pHeaderBar( nullptr )
    ,pCornerWin( nullptr )
    ,bInDtor( false )
    ,bInPaint( false )
    ,bInCommand( false )
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
    ,bCallingDropCallback( false )
    ,nUpdateLock( 0 )
    ,nCursorHidden( 0 )
    ,m_nDragRowDividerLimit( 0 )
    ,m_nDragRowDividerOffset( 0 )
{
    aMouseTimer.SetInvokeHandler( LINK( this, BrowserDataWin, RepeatedMouseMove ) );
    aMouseTimer.SetTimeout( 100 );
}


BrowserDataWin::~BrowserDataWin()
{
    disposeOnce();
}

void BrowserDataWin::dispose()
{
    bInDtor = true;

    aInvalidRegion.clear();
    pHeaderBar.clear();
    pCornerWin.clear();
    DragSourceHelper::dispose();
    DropTargetHelper::dispose();
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

void InitSettings_Impl(vcl::Window* pWin)
{
    const StyleSettings& rStyleSettings = pWin->GetSettings().GetStyleSettings();

    pWin->ApplyControlFont(*pWin, rStyleSettings.GetFieldFont());
    pWin->ApplyControlForeground(*pWin, rStyleSettings.GetWindowTextColor());
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
        InitSettings_Impl(this);
        Invalidate();
        InitSettings_Impl(GetParent());
        GetParent()->Invalidate();
        GetParent()->Resize();
    }
    else
        Control::DataChanged( rDCEvt );
}


void BrowserDataWin::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!nUpdateLock && GetUpdateMode())
    {
        if (bInPaint)
        {
            aInvalidRegion.emplace_back(rRect);
            return;
        }
        bInPaint = true;
        GetParent()->PaintData(*this, rRenderContext, rRect);
        bInPaint = false;
        DoOutstandingInvalidations();
    }
    else
    {
        aInvalidRegion.emplace_back(rRect);
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
          nCol < pBox->mvCols.size() && nColX < GetSizePixel().Width();
          ++nCol )
        if ( pBox->mvCols[ nCol ]->IsFrozen() || nCol >= pBox->nFirstCol )
        {
            nColX += pBox->mvCols[ nCol ]->Width();
            if ( nMouseX < nColX )
                break;
        }
    sal_uInt16 nColId = BROWSER_INVALIDID;
    if ( nCol < pBox->mvCols.size() )
        nColId = pBox->mvCols[ nCol ]->GetId();

    // compute the field rectangle and field relative MouseEvent
    tools::Rectangle aFieldRect;
    if ( nCol < pBox->mvCols.size() )
    {
        nColX -= pBox->mvCols[ nCol ]->Width();
        aFieldRect = tools::Rectangle(
            Point( nColX, nRelRow * pBox->GetDataRowHeight() ),
            Size( pBox->mvCols[ nCol ]->Width(),
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
        aEventPos.AdjustY(GetParent()->GetTitleHeight() );
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

    aEventPos.AdjustY(GetParent()->GetTitleHeight() );
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
    if ( aNewPos == aLastMousePos )
        return;
    aLastMousePos = aNewPos;

    // transform to a BrowseEvent
    BrowserMouseEvent aBrowserEvent( this, rEvt );
    GetParent()->MouseMove( aBrowserEvent );

    // pointer shape
    PointerStyle ePointerStyle = PointerStyle::Arrow;
    if ( ImplRowDividerHitTest( aBrowserEvent ) )
        ePointerStyle = PointerStyle::VSizeBar;
    SetPointer( ePointerStyle );

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


IMPL_LINK_NOARG(BrowserDataWin, RepeatedMouseMove, Timer *, void)
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

    tools::Rectangle aDragSplitRect( 0, m_nDragRowDividerLimit, GetOutputSizePixel().Width(), nDragRowDividerCurrentPos );
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
        aMousePos.setY( GetOutputSizePixel().Height() );

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
        long nDragRowDividerCurrentPos = aMousePos.Y() + m_nDragRowDividerOffset;

        // care for minimum row height
        if ( nDragRowDividerCurrentPos < m_nDragRowDividerLimit + GetParent()->QueryMinimumRowHeight() )
            nDragRowDividerCurrentPos = m_nDragRowDividerLimit + GetParent()->QueryMinimumRowHeight();

        tools::Rectangle aDragSplitRect( 0, m_nDragRowDividerLimit, GetOutputSizePixel().Width(), nDragRowDividerCurrentPos );
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
    GetParent()->RequestHelp( rHEvt );
}


BrowseEvent::BrowseEvent( vcl::Window* pWindow,
                          long nAbsRow, sal_uInt16 nColumn, sal_uInt16 nColumnId,
                          const tools::Rectangle& rRect ):
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
                          const tools::Rectangle& rRect ):
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
    if ( bMode == bUpdateMode )
        return;

    bUpdateMode = bMode;
    if ( bMode )
        DoOutstandingInvalidations();
}


void BrowserDataWin::DoOutstandingInvalidations()
{
    for (auto& rRect : aInvalidRegion)
        Control::Invalidate( rRect );
    aInvalidRegion.clear();
}


void BrowserDataWin::Invalidate( InvalidateFlags nFlags )
{
    if ( !GetUpdateMode() )
    {
        aInvalidRegion.clear();
        aInvalidRegion.emplace_back( Point( 0, 0 ), GetOutputSizePixel() );
    }
    else
        Window::Invalidate( nFlags );
}


void BrowserDataWin::Invalidate( const tools::Rectangle& rRect, InvalidateFlags nFlags )
{
    if ( !GetUpdateMode() )
        aInvalidRegion.emplace_back( rRect );
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

        tools::Rectangle aRect(GetPointerPosPixel(), Size(GetTextWidth(aTip), GetTextHeight()));
        Help::ShowQuickHelp(this, aRect, aTip);
        _nLastPos = nPos;
    }

    ScrollBar::Tracking( rTEvt );
}

void BrowserScrollBar::EndScroll()
{
    Help::HideBalloonAndQuickHelp();
    ScrollBar::EndScroll();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
