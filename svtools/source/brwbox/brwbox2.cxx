/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/debug.hxx>
#include <svtools/brwbox.hxx>
#include "datwin.hxx"
#include <svtools/colorcfg.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/settings.hxx>

#include <tools/multisel.hxx>
#include <algorithm>

using namespace ::com::sun::star::datatransfer;

#define getDataWindow() ((BrowserDataWin*)pDataWin)




DBG_NAMEEX(BrowseBox)



extern const char* BrowseBoxCheckInvariants( const void * pVoid );



void BrowseBox::StartDrag( sal_Int8 /* _nAction */, const Point& /* _rPosPixel */ )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    
}



sal_Int8 BrowseBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    BrowserDataWin* pDataWindow = static_cast<BrowserDataWin*>(pDataWin);
    AcceptDropEvent aTransformed( _rEvt );
    aTransformed.maPosPixel = pDataWindow->ScreenToOutputPixel( OutputToScreenPixel( _rEvt.maPosPixel ) );
    return pDataWindow->AcceptDrop( aTransformed );
}



sal_Int8 BrowseBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    BrowserDataWin* pDataWindow = static_cast<BrowserDataWin*>(pDataWin);
    ExecuteDropEvent aTransformed( _rEvt );
    aTransformed.maPosPixel = pDataWindow->ScreenToOutputPixel( OutputToScreenPixel( _rEvt.maPosPixel ) );
    return pDataWindow->ExecuteDrop( aTransformed );
}



sal_Int8 BrowseBox::AcceptDrop( const BrowserAcceptDropEvent& )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    
    return DND_ACTION_NONE;
}



sal_Int8 BrowseBox::ExecuteDrop( const BrowserExecuteDropEvent& )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    
    return DND_ACTION_NONE;
}



void* BrowseBox::implGetDataFlavors() const
{
    if (static_cast<BrowserDataWin*>(pDataWin)->bCallingDropCallback)
        return &static_cast<BrowserDataWin*>(pDataWin)->GetDataFlavorExVector();
    return &GetDataFlavorExVector();
}



sal_Bool BrowseBox::IsDropFormatSupported( SotFormatStringId _nFormat )
{
    if ( static_cast< BrowserDataWin* >( pDataWin )->bCallingDropCallback )
        return static_cast< BrowserDataWin* >( pDataWin )->IsDropFormatSupported( _nFormat );

    return DropTargetHelper::IsDropFormatSupported( _nFormat );
}



sal_Bool BrowseBox::IsDropFormatSupported( SotFormatStringId _nFormat ) const
{
    return const_cast< BrowseBox* >( this )->IsDropFormatSupported( _nFormat );
}



sal_Bool BrowseBox::IsDropFormatSupported( const DataFlavor& _rFlavor )
{
    if ( static_cast< BrowserDataWin* >( pDataWin )->bCallingDropCallback )
        return static_cast< BrowserDataWin* >( pDataWin )->IsDropFormatSupported( _rFlavor );

    return DropTargetHelper::IsDropFormatSupported( _rFlavor );
}



sal_Bool BrowseBox::IsDropFormatSupported( const DataFlavor& _rFlavor ) const
{
    return const_cast< BrowseBox* >( this )->IsDropFormatSupported( _rFlavor );
}



void BrowseBox::Command( const CommandEvent& rEvt )
{
    if ( !getDataWindow()->bInCommand )
        Control::Command( rEvt );
}



void BrowseBox::StateChanged( StateChangedType nStateChange )
{
    Control::StateChanged( nStateChange );

    if ( STATE_CHANGE_MIRRORING == nStateChange )
    {
        getDataWindow()->EnableRTL( IsRTLEnabled() );

        HeaderBar* pHeaderBar = getDataWindow()->pHeaderBar;
        if ( pHeaderBar )
            pHeaderBar->EnableRTL( IsRTLEnabled() );
        aHScroll.EnableRTL( IsRTLEnabled() );
        if( pVScroll )
            pVScroll->EnableRTL( IsRTLEnabled() );
        Resize();
    }
    else if ( STATE_CHANGE_INITSHOW == nStateChange )
    {
        bBootstrapped = sal_True; 

        Resize();
        if ( bMultiSelection )
            uRow.pSel->SetTotalRange( Range( 0, nRowCount - 1 ) );
        if ( nRowCount == 0 )
            nCurRow = BROWSER_ENDOFSELECTION;
        else if ( nCurRow == BROWSER_ENDOFSELECTION )
            nCurRow = 0;


        if ( HasFocus() )
        {
            bSelectionIsVisible = sal_True;
            bHasFocus = sal_True;
        }
        UpdateScrollbars();
        AutoSizeLastColumn();
        CursorMoved();
    }
    else if (STATE_CHANGE_ZOOM == nStateChange)
    {
        pDataWin->SetZoom(GetZoom());
        HeaderBar* pHeaderBar = getDataWindow()->pHeaderBar;
        if (pHeaderBar)
            pHeaderBar->SetZoom(GetZoom());

        
        for ( size_t nPos = 0; nPos < pCols->size(); ++nPos )
        {
            (*pCols)[ nPos ]->ZoomChanged(GetZoom());
            if ( pHeaderBar )
                pHeaderBar->SetItemSize( (*pCols)[ nPos ]->GetId(), (*pCols)[ nPos ]->Width() );
        }

        
        Resize();
    }
    else if (STATE_CHANGE_ENABLE == nStateChange)
    {
        
        sal_Bool bHandleCol = !pCols->empty() && (0 == (*pCols)[ 0 ]->GetId());
        
        sal_Bool bHeaderBar = (NULL != static_cast<BrowserDataWin&>(GetDataWindow()).pHeaderBar);

        if  (   nTitleLines
            &&  (   !bHeaderBar
                ||  bHandleCol
                )
            )
            
            
            Invalidate(Rectangle(Point(0, 0), Size(GetOutputSizePixel().Width(), GetTitleHeight() - 1)));
    }
}



void BrowseBox::Select()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}



void BrowseBox::DoubleClick( const BrowserMouseEvent & )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}



long BrowseBox::QueryMinimumRowHeight()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    return CalcZoom( 5 );
}



void BrowseBox::ImplStartTracking()
{
    DBG_CHKTHIS( BrowseBox, BrowseBoxCheckInvariants );
}



void BrowseBox::ImplTracking()
{
    DBG_CHKTHIS( BrowseBox, BrowseBoxCheckInvariants );
}



void BrowseBox::ImplEndTracking()
{
    DBG_CHKTHIS( BrowseBox, BrowseBoxCheckInvariants );
}



void BrowseBox::RowHeightChanged()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}



long BrowseBox::QueryColumnResize( sal_uInt16, long nWidth )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    return nWidth;
}



void BrowseBox::ColumnResized( sal_uInt16 )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}



void BrowseBox::ColumnMoved( sal_uInt16 )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}



void BrowseBox::StartScroll()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    DoHideCursor( "StartScroll" );
}



void BrowseBox::EndScroll()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    UpdateScrollbars();
    AutoSizeLastColumn();
    DoShowCursor( "EndScroll" );
}



void BrowseBox::ToggleSelection( sal_Bool bForce )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    
    if ( bHideSelect )
        return;
    if ( !bForce &&
         ( bNotToggleSel || !IsUpdateMode() || !bSelectionIsVisible ) )
        return;

    
    bNotToggleSel = sal_True;
    if ( false && !getDataWindow()->bInPaint )
        pDataWin->Update();

    
    RectangleList aHighlightList;
    long nLastRowInRect = 0; 

    
    BrowserColumn *pFirstCol = pCols->empty() ? NULL : (*pCols)[ 0 ];
    long nOfsX = (!pFirstCol || pFirstCol->GetId()) ? 0 : pFirstCol->Width();

    
    long nBottomRow = nTopRow +
        pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight();
    if ( nBottomRow > GetRowCount() && GetRowCount() )
        nBottomRow = GetRowCount();
    for ( long nRow = bMultiSelection ? uRow.pSel->FirstSelected() : uRow.nSel;
          nRow != BROWSER_ENDOFSELECTION && nRow <= nBottomRow;
          nRow = bMultiSelection ? uRow.pSel->NextSelected() : BROWSER_ENDOFSELECTION )
    {
        if ( nRow < nTopRow )
            continue;

        Rectangle aAddRect(
            Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
            Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
        if ( aHighlightList.size() && nLastRowInRect == ( nRow - 1 ) )
            aHighlightList[ 0 ]->Union( aAddRect );
        else
            aHighlightList.insert( aHighlightList.begin(), new Rectangle( aAddRect ) );
        nLastRowInRect = nRow;
    }

    
    for ( size_t i = aHighlightList.size(); i > 0; )
    {
        Rectangle *pRect = aHighlightList[ --i ];
        pDataWin->Invalidate( *pRect );
        delete pRect;
    }
    aHighlightList.clear();

    
    for ( long nColId = pColSel ? pColSel->FirstSelected() : BROWSER_ENDOFSELECTION;
          nColId != BROWSER_ENDOFSELECTION;
          nColId = pColSel->NextSelected() )
    {
        Rectangle aRect( GetFieldRectPixel(nCurRow,
                                           (*pCols)[ nColId ]->GetId(),
                                           sal_False ) );
        aRect.Left() -= MIN_COLUMNWIDTH;
        aRect.Right() += MIN_COLUMNWIDTH;
        aRect.Top() = 0;
        aRect.Bottom() = pDataWin->GetOutputSizePixel().Height();
        pDataWin->Invalidate( aRect );
    }

    bNotToggleSel = sal_False;
}



void BrowseBox::DrawCursor()
{
    bool bReallyHide = false;
    if ( SMART_CURSOR_HIDE == bHideCursor )
    {
        if ( !GetSelectRowCount() && !GetSelectColumnCount() )
            bReallyHide = true;
    }
    else if ( HARD_CURSOR_HIDE == bHideCursor )
    {
        bReallyHide = true;
    }

    bReallyHide |= !bSelectionIsVisible || !IsUpdateMode() || bScrolling || nCurRow < 0;

    if (PaintCursorIfHiddenOnce())
        bReallyHide |= ( GetCursorHideCount() > 1 );
    else
        bReallyHide |= ( GetCursorHideCount() > 0 );

    
    if ( nCurColId == HandleColumnId )
        nCurColId = GetColumnId(1);

    
    Rectangle aCursor;
    if ( bColumnCursor )
    {
        aCursor = GetFieldRectPixel( nCurRow, nCurColId, sal_False );
        aCursor.Left() -= MIN_COLUMNWIDTH;
        aCursor.Right() += 1;
        aCursor.Bottom() += 1;
    }
    else
        aCursor = Rectangle(
            Point( ( !pCols->empty() && (*pCols)[ 0 ]->GetId() == 0 ) ?
                        (*pCols)[ 0 ]->Width() : 0,
                        (nCurRow - nTopRow) * GetDataRowHeight() + 1 ),
            Size( pDataWin->GetOutputSizePixel().Width() + 1,
                  GetDataRowHeight() - 2 ) );
    if ( bHLines )
    {
        if ( !bMultiSelection )
            --aCursor.Top();
        --aCursor.Bottom();
    }

    if (m_aCursorColor == COL_TRANSPARENT)
    {
        
        if ( bReallyHide )
            ((Control*)pDataWin)->HideFocus();
        else
            ((Control*)pDataWin)->ShowFocus( aCursor );
    }
    else
    {
        Color rCol = bReallyHide ? pDataWin->GetFillColor() : m_aCursorColor;
        Color aOldFillColor = pDataWin->GetFillColor();
        Color aOldLineColor = pDataWin->GetLineColor();
        pDataWin->SetFillColor();
        pDataWin->SetLineColor( rCol );
        pDataWin->DrawRect( aCursor );
        pDataWin->SetLineColor( aOldLineColor );
        pDataWin->SetFillColor( aOldFillColor );
    }
}



sal_uLong BrowseBox::GetColumnWidth( sal_uInt16 nId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    sal_uInt16 nItemPos = GetColumnPos( nId );
    if ( nItemPos >= pCols->size() )
        return 0;
    return (*pCols)[ nItemPos ]->Width();
}



sal_uInt16 BrowseBox::GetColumnId( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( nPos >= pCols->size() )
        return BROWSER_INVALIDID;
    return (*pCols)[ nPos ]->GetId();
}



sal_uInt16 BrowseBox::GetColumnPos( sal_uInt16 nId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    for ( sal_uInt16 nPos = 0; nPos < pCols->size(); ++nPos )
        if ( (*pCols)[ nPos ]->GetId() == nId )
            return nPos;
    return BROWSER_INVALIDID;
}



sal_Bool BrowseBox::IsFrozen( sal_uInt16 nColumnId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    for ( size_t nPos = 0; nPos < pCols->size(); ++nPos )
        if ( (*pCols)[ nPos ]->GetId() == nColumnId )
            return (*pCols)[ nPos ]->IsFrozen();
    return sal_False;
}



void BrowseBox::ExpandRowSelection( const BrowserMouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    DoHideCursor( "ExpandRowSelection" );

    
    if ( bMultiSelection )
    {
        Range aJustifiedRange( aSelRange );
        aJustifiedRange.Justify();

        sal_Bool bSelectThis = ( bSelect != aJustifiedRange.IsInside( rEvt.GetRow() ) );

        if ( aJustifiedRange.IsInside( rEvt.GetRow() ) )
        {
            
            while ( rEvt.GetRow() < aSelRange.Max() )
            {   
                SelectRow( aSelRange.Max(), bSelectThis, sal_True );
                --aSelRange.Max();
            }
            while ( rEvt.GetRow() > aSelRange.Max() )
            {   
                SelectRow( aSelRange.Max(), bSelectThis, sal_True );
                ++aSelRange.Max();
            }
        }
        else
        {
            
            sal_Bool bOldSelecting = bSelecting;
            bSelecting = sal_True;
            while ( rEvt.GetRow() < aSelRange.Max() )
            {   
                --aSelRange.Max();
                if ( !IsRowSelected( aSelRange.Max() ) )
                {
                    SelectRow( aSelRange.Max(), bSelectThis, sal_True );
                    bSelect = true;
                }
            }
            while ( rEvt.GetRow() > aSelRange.Max() )
            {   
                ++aSelRange.Max();
                if ( !IsRowSelected( aSelRange.Max() ) )
                {
                    SelectRow( aSelRange.Max(), bSelectThis, sal_True );
                    bSelect = true;
                }
            }
            bSelecting = bOldSelecting;
            if ( bSelect )
                Select();
        }
    }
    else
        if ( !bMultiSelection || !IsRowSelected( rEvt.GetRow() ) )
            SelectRow( rEvt.GetRow(), sal_True );

    GoToRow( rEvt.GetRow(), sal_False );
    DoShowCursor( "ExpandRowSelection" );
}



void BrowseBox::Resize()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( STATE_CHANGE_INITSHOW );
    if ( pCols->empty() )
    {
        getDataWindow()->bResizeOnPaint = true;
        return;
    }
    getDataWindow()->bResizeOnPaint = false;

    
    
    
    sal_uLong nSBSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if (IsZoom())
        nSBSize = (sal_uLong)(nSBSize * (double)GetZoom());

    DoHideCursor( "Resize" );
    sal_uInt16 nOldVisibleRows = 0;
    
    if (GetDataRowHeight())
        nOldVisibleRows = (sal_uInt16)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    
    if ( !getDataWindow()->bNoHScroll &&
         ( ( pCols->size() - FrozenColCount() ) > 1 ) )
        aHScroll.Show();
    else
        aHScroll.Hide();

    
    long nDataHeight = GetOutputSizePixel().Height() - GetTitleHeight();
    if ( aHScroll.IsVisible() || ( nControlAreaWidth != USHRT_MAX ) )
        nDataHeight -= nSBSize;

    long nDataWidth = GetOutputSizePixel().Width();
    if ( pVScroll->IsVisible() )
        nDataWidth -= nSBSize;

    
    pDataWin->SetPosSizePixel(
        Point( 0, GetTitleHeight() ),
        Size( nDataWidth, nDataHeight ) );

    sal_uInt16 nVisibleRows = 0;

    if (GetDataRowHeight())
        nVisibleRows = (sal_uInt16)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    
    if ( nVisibleRows != nOldVisibleRows )
        VisibleRowsChanged(nTopRow, nVisibleRows);

    UpdateScrollbars();

    
    Rectangle aInvalidArea( GetControlArea() );
    aInvalidArea.Right() = GetOutputSizePixel().Width();
    aInvalidArea.Left() = 0;
    Invalidate( aInvalidArea );

    
    HeaderBar* pHeaderBar = getDataWindow()->pHeaderBar;
    if ( pHeaderBar )
    {
        
        BrowserColumn *pFirstCol = (*pCols)[ 0 ];
        long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();
        pHeaderBar->SetPosSizePixel( Point( nOfsX, 0 ), Size( GetOutputSizePixel().Width() - nOfsX, GetTitleHeight() ) );
    }

    AutoSizeLastColumn(); 
    DoShowCursor( "Resize" );
}



void BrowseBox::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( STATE_CHANGE_INITSHOW );
    if ( pCols->empty() )
        return;

    BrowserColumn *pFirstCol = (*pCols)[ 0 ];
    sal_Bool bHandleCol = pFirstCol && pFirstCol->GetId() == 0;
    sal_Bool bHeaderBar = getDataWindow()->pHeaderBar != NULL;

    
    if ( !getDataWindow()->bNoHScroll )
        DrawLine( Point( 0, aHScroll.GetPosPixel().Y() ),
                  Point( GetOutputSizePixel().Width(),
                         aHScroll.GetPosPixel().Y() ) );

    if ( nTitleLines )
    {
        if ( !bHeaderBar )
            DrawLine( Point( 0, GetTitleHeight() - 1 ),
                      Point( GetOutputSizePixel().Width(),
                             GetTitleHeight() - 1 ) );
        else if ( bHandleCol )
            DrawLine( Point( 0, GetTitleHeight() - 1 ),
                      Point( pFirstCol->Width(), GetTitleHeight() - 1 ) );
    }

    
    
    
    if ( nTitleLines && (!bHeaderBar || bHandleCol) )
    {
        
        long nX = 0;
        size_t nCol;
        for ( nCol = 0;
              nCol < pCols->size() && nX < rRect.Right();
              ++nCol )
        {
            
            if ( nCol < nFirstCol && !(*pCols)[ nCol ]->IsFrozen() )
                nCol = nFirstCol;

            
            if (bHeaderBar && bHandleCol && nCol > 0)
                break;

            BrowserColumn *pCol = (*pCols)[ nCol ];

            
            if ( pCol->Width() > 4 )
            {
                ButtonFrame aButtonFrame( Point( nX, 0 ),
                    Size( pCol->Width()-1, GetTitleHeight()-1 ),
                    pCol->Title(), sal_False, sal_False, !IsEnabled());
                aButtonFrame.Draw( *this );
                DrawLine( Point( nX + pCol->Width() - 1, 0 ),
                   Point( nX + pCol->Width() - 1, GetTitleHeight()-1 ) );
            }
            else
            {
                Color aOldFillColor = GetFillColor();
                SetFillColor( Color( COL_BLACK ) );
                DrawRect( Rectangle( Point( nX, 0 ), Size( pCol->Width(), GetTitleHeight() - 1 ) ) );
                SetFillColor( aOldFillColor );
            }

            
            nX += pCol->Width();
        }

        
        if ( !bHeaderBar && nCol == pCols->size() )
        {
            const StyleSettings &rSettings = GetSettings().GetStyleSettings();
            Color aColFace( rSettings.GetFaceColor() );
            Color aOldFillColor = GetFillColor();
            Color aOldLineColor = GetLineColor();
            SetFillColor( aColFace );
            SetLineColor( aColFace );
            DrawRect( Rectangle(
                Point( nX, 0 ),
                Point( rRect.Right(), GetTitleHeight() - 2 ) ) );
            SetFillColor( aOldFillColor);
            SetLineColor( aOldLineColor);
        }
    }
}



void BrowseBox::PaintRow( OutputDevice&, const Rectangle& )
{
}



void BrowseBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags )
{
    sal_Bool bDrawSelection = (nFlags & WINDOW_DRAW_NOSELECTION) == 0;

    
    Size aRealSize = pDev->LogicToPixel(rSize);
    Point aRealPos = pDev->LogicToPixel(rPos);

    if ((rSize.Width() < 3) || (rSize.Height() < 3))
        
        return;

    Font aFont = GetDataWindow().GetDrawPixelFont( pDev );
        
        

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );

    
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    pDev->SetLineColor(rStyleSettings.GetDarkShadowColor());
    pDev->DrawLine(Point(aRealPos.X(), aRealPos.Y()),
                   Point(aRealPos.X(), aRealPos.Y() + aRealSize.Height() - 1));
    pDev->DrawLine(Point(aRealPos.X(), aRealPos.Y()),
                   Point(aRealPos.X() + aRealSize.Width() - 1, aRealPos.Y()));
    pDev->SetLineColor(rStyleSettings.GetShadowColor());
    pDev->DrawLine(Point(aRealPos.X() + aRealSize.Width() - 1, aRealPos.Y() + 1),
                   Point(aRealPos.X() + aRealSize.Width() - 1, aRealPos.Y() + aRealSize.Height() - 1));
    pDev->DrawLine(Point(aRealPos.X() + aRealSize.Width() - 1, aRealPos.Y() + aRealSize.Height() - 1),
                   Point(aRealPos.X() + 1, aRealPos.Y() + aRealSize.Height() - 1));

    HeaderBar* pBar = getDataWindow()->pHeaderBar;

    
    
    if (!nDataRowHeight)
        ImpGetDataRowHeight();
    long nHeightLogic = PixelToLogic(Size(0, nDataRowHeight), MAP_10TH_MM).Height();
    long nForeignHeightPixel = pDev->LogicToPixel(Size(0, nHeightLogic), MAP_10TH_MM).Height();

    long nOriginalHeight = nDataRowHeight;
    nDataRowHeight = nForeignHeightPixel;

    
    size_t nPos;
    for ( nPos = 0; nPos < pCols->size(); ++nPos )
    {
        BrowserColumn* pCurrent = (*pCols)[ nPos ];

        long nWidthLogic = PixelToLogic(Size(pCurrent->Width(), 0), MAP_10TH_MM).Width();
        long nForeignWidthPixel = pDev->LogicToPixel(Size(nWidthLogic, 0), MAP_10TH_MM).Width();

        pCurrent->SetWidth(nForeignWidthPixel, GetZoom());
        if ( pBar )
            pBar->SetItemSize( pCurrent->GetId(), pCurrent->Width() );
    }

    
    ++aRealPos.X();
    ++aRealPos.Y();
    aRealSize.Width() -= 2;
    aRealSize.Height() -= 2;

    
    if ( pBar )
    {
        
        long nTitleHeight = PixelToLogic(Size(0, GetTitleHeight()), MAP_10TH_MM).Height();
        nTitleHeight = pDev->LogicToPixel(Size(0, nTitleHeight), MAP_10TH_MM).Height();

        BrowserColumn* pFirstCol = !pCols->empty() ? (*pCols)[ 0 ] : NULL;

        Point aHeaderPos(pFirstCol && (pFirstCol->GetId() == 0) ? pFirstCol->Width() : 0, 0);
        Size aHeaderSize(aRealSize.Width() - aHeaderPos.X(), nTitleHeight);

        aHeaderPos += aRealPos;
            

        
        aHeaderPos = pDev->PixelToLogic(aHeaderPos);
        aHeaderSize = pDev->PixelToLogic(aHeaderSize);

        pBar->Draw(pDev, aHeaderPos, aHeaderSize, nFlags);

        
        if (pFirstCol && (pFirstCol->GetId() == 0) && (pFirstCol->Width() > 4))
        {
            ButtonFrame aButtonFrame( aRealPos,
                Size( pFirstCol->Width()-1, nTitleHeight-1 ),
                pFirstCol->Title(), sal_False, sal_False, !IsEnabled());
            aButtonFrame.Draw( *pDev );

            pDev->Push( PUSH_LINECOLOR );
            pDev->SetLineColor( Color( COL_BLACK ) );

            pDev->DrawLine( Point( aRealPos.X(), aRealPos.Y() + nTitleHeight-1 ),
               Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() + nTitleHeight-1 ) );
            pDev->DrawLine( Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() ),
               Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() + nTitleHeight-1 ) );

            pDev->Pop();
        }

        aRealPos.Y() += aHeaderSize.Height();
        aRealSize.Height() -= aHeaderSize.Height();
    }

    
    Region aRegion(Rectangle(aRealPos, aRealSize));
    pDev->SetClipRegion( pDev->PixelToLogic( aRegion ) );

    
    sal_Bool bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && GetDataWindow().IsControlBackground();
    if ( bBackground )
    {
        Rectangle aRect( aRealPos, aRealSize );
        pDev->SetFillColor( GetDataWindow().GetControlBackground() );
        pDev->DrawRect( aRect );
    }

    ImplPaintData( *pDev, Rectangle( aRealPos, aRealSize ), sal_True, bDrawSelection );

    
    nDataRowHeight = nOriginalHeight;
    for ( nPos = 0; nPos < pCols->size(); ++nPos )
    {
        BrowserColumn* pCurrent = (*pCols)[ nPos ];

        long nForeignWidthLogic = pDev->PixelToLogic(Size(pCurrent->Width(), 0), MAP_10TH_MM).Width();
        long nWidthPixel = LogicToPixel(Size(nForeignWidthLogic, 0), MAP_10TH_MM).Width();

        pCurrent->SetWidth(nWidthPixel, GetZoom());
        if ( pBar )
            pBar->SetItemSize( pCurrent->GetId(), pCurrent->Width() );
    }

    pDev->Pop();
}



void BrowseBox::ImplPaintData(OutputDevice& _rOut, const Rectangle& _rRect, sal_Bool _bForeignDevice, sal_Bool _bDrawSelections)
{
    Point aOverallAreaPos = _bForeignDevice ? _rRect.TopLeft() : Point(0,0);
    Size aOverallAreaSize = _bForeignDevice ? _rRect.GetSize() : GetDataWindow().GetOutputSizePixel();
    Point aOverallAreaBRPos = _bForeignDevice ? _rRect.BottomRight() : Point( aOverallAreaSize.Width(), aOverallAreaSize.Height() );

    long nDataRowHeigt = GetDataRowHeight();

    
    sal_uLong nRelTopRow = 0;
    sal_uLong nRelBottomRow = aOverallAreaSize.Height();
    if (!_bForeignDevice && nDataRowHeigt)
    {
        nRelTopRow = ((sal_uLong)_rRect.Top() / nDataRowHeigt);
        nRelBottomRow = (sal_uLong)(_rRect.Bottom()) / nDataRowHeigt;
    }

    
    Point aPos( aOverallAreaPos.X(), nRelTopRow * nDataRowHeigt + aOverallAreaPos.Y() );
    _rOut.SetLineColor( Color( COL_WHITE ) );
    const AllSettings& rAllSets = _rOut.GetSettings();
    const StyleSettings &rSettings = rAllSets.GetStyleSettings();
    const Color &rHighlightTextColor = rSettings.GetHighlightTextColor();
    const Color &rHighlightFillColor = rSettings.GetHighlightColor();
    Color aOldTextColor = _rOut.GetTextColor();
    Color aOldFillColor = _rOut.GetFillColor();
    Color aOldLineColor = _rOut.GetLineColor();
    long nHLineX = 0 == (*pCols)[ 0 ]->GetId() ? (*pCols)[ 0 ]->Width() : 0;
    nHLineX += aOverallAreaPos.X();

    Color aDelimiterLineColor( ::svtools::ColorConfig().GetColorValue( ::svtools::CALCGRID ).nColor );

    
    for ( sal_uLong nRelRow = nRelTopRow;
          nRelRow <= nRelBottomRow && (sal_uLong)nTopRow+nRelRow < (sal_uLong)nRowCount;
          ++nRelRow, aPos.Y() += nDataRowHeigt )
    {
        
        
        DBG_ASSERT( (sal_uInt16)(nTopRow+nRelRow) < nRowCount, "BrowseBox::ImplPaintData: invalid seek" );
        if ( (nTopRow+long(nRelRow)) < 0 || (sal_uInt16)(nTopRow+nRelRow) >= nRowCount )
            continue;

        
        sal_uLong nRow = nTopRow+nRelRow;
        if ( !SeekRow( nRow) ) {
            OSL_FAIL("BrowseBox::ImplPaintData: SeekRow failed");
        }
        _rOut.SetClipRegion();
        aPos.X() = aOverallAreaPos.X();


        
        
        Rectangle aRowRect( Point( _rRect.TopLeft().X(), aPos.Y() ),
                Size( _rRect.GetSize().Width(), nDataRowHeigt ) );
        PaintRow( _rOut, aRowRect );

        sal_Bool bRowSelected   =   _bDrawSelections
                            &&  !bHideSelect
                            &&  IsRowSelected( nRow );
        if ( bRowSelected )
        {
            _rOut.SetTextColor( rHighlightTextColor );
            _rOut.SetFillColor( rHighlightFillColor );
            _rOut.SetLineColor();
            _rOut.DrawRect( aRowRect );
        }

        
        size_t nCol;
        for ( nCol = 0; nCol < pCols->size(); ++nCol )
        {
            
            BrowserColumn *pCol = (*pCols)[ nCol ];

            
            if ( aPos.X() >= _rRect.Right() )
                break;

            
            if ( nCol < nFirstCol && !pCol->IsFrozen() )
            {
                nCol = nFirstCol;
                pCol = (nCol < pCols->size() ) ? (*pCols)[ nCol ] : NULL;
                if (!pCol)
                {   
                    
                    
                    OSL_FAIL("BrowseBox::PaintData : nFirstCol is probably invalid !");
                    break;
                }
            }

            
            sal_Bool bColAutoHighlight  =   _bDrawSelections
                                    &&  bColumnCursor
                                    &&  IsColumnSelected( pCol->GetId() );
            if ( bColAutoHighlight )
            {
                _rOut.SetClipRegion();
                _rOut.SetTextColor( rHighlightTextColor );
                _rOut.SetFillColor( rHighlightFillColor );
                _rOut.SetLineColor();
                Rectangle aFieldRect( aPos,
                        Size( pCol->Width(), nDataRowHeigt ) );
                _rOut.DrawRect( aFieldRect );
            }

            if (!m_bFocusOnlyCursor && (pCol->GetId() == GetCurColumnId()) && (nRow == (sal_uLong)GetCurRow()))
                DrawCursor();

            
            
            if (pCol->Width())
            {
                
                if (_bForeignDevice)
                {   
                    Size aFieldSize(pCol->Width(), nDataRowHeigt);

                    if (aPos.X() + aFieldSize.Width() > aOverallAreaBRPos.X())
                        aFieldSize.Width() = aOverallAreaBRPos.X() - aPos.X();

                    if (aPos.Y() + aFieldSize.Height() > aOverallAreaBRPos.Y() + 1)
                    {
                        
                        if (pCol->GetId() != 0)
                            continue;
                        aFieldSize.Height() = aOverallAreaBRPos.Y() + 1 - aPos.Y();
                    }

                    Region aClipToField(Rectangle(aPos, aFieldSize));
                    _rOut.SetClipRegion(aClipToField);
                }
                pCol->Draw( *this, _rOut, aPos, sal_False );
                if (_bForeignDevice)
                    _rOut.SetClipRegion();
            }

            
            if ( bColAutoHighlight )
            {
                _rOut.SetTextColor( aOldTextColor );
                _rOut.SetFillColor( aOldFillColor );
                _rOut.SetLineColor( aOldLineColor );
            }

            
            aPos.X() += pCol->Width();
        }

        
        if ( bRowSelected )
        {
            _rOut.SetTextColor( aOldTextColor );
            _rOut.SetFillColor( aOldFillColor );
            _rOut.SetLineColor( aOldLineColor );
        }

        if ( bHLines )
        {
            
            _rOut.SetClipRegion();
            _rOut.Push( PUSH_LINECOLOR );
            _rOut.SetLineColor( aDelimiterLineColor );
            long nY = aPos.Y() + nDataRowHeigt - 1;
            if (nY <= aOverallAreaBRPos.Y())
                _rOut.DrawLine( Point( nHLineX, nY ),
                                Point( bVLines
                                        ? std::min(long(long(aPos.X()) - 1), aOverallAreaBRPos.X())
                                        : aOverallAreaBRPos.X(),
                                      nY ) );
            _rOut.Pop();
        }
    }

    if (aPos.Y() > aOverallAreaBRPos.Y() + 1)
        aPos.Y() = aOverallAreaBRPos.Y() + 1;
        

    
    _rOut.SetClipRegion();
    aOldLineColor = _rOut.GetLineColor();
    aOldFillColor = _rOut.GetFillColor();
    _rOut.SetFillColor( rSettings.GetFaceColor() );
    if ( !pCols->empty() && ( (*pCols)[ 0 ]->GetId() == 0 ) && ( aPos.Y() <= _rRect.Bottom() ) )
    {
        
        
        _rOut.SetLineColor( Color( COL_BLACK ) );
        _rOut.DrawRect( Rectangle(
            Point( aOverallAreaPos.X() - 1, aPos.Y() - 1 ),
            Point( aOverallAreaPos.X() + (*pCols)[ 0 ]->Width() - 1,
                   _rRect.Bottom() + 1) ) );
    }
    _rOut.SetFillColor( aOldFillColor );

    
    _rOut.SetLineColor( COL_BLACK );
    long nFrozenWidth = GetFrozenWidth()-1;
    _rOut.DrawLine( Point( aOverallAreaPos.X() + nFrozenWidth, aPos.Y() ),
                   Point( aOverallAreaPos.X() + nFrozenWidth, bHLines
                            ? aPos.Y() - 1
                            : aOverallAreaBRPos.Y() ) );

    
    if ( bVLines )
    {
        _rOut.SetLineColor( aDelimiterLineColor );
        Point aVertPos( aOverallAreaPos.X() - 1, aOverallAreaPos.Y() );
        long nDeltaY = aOverallAreaBRPos.Y();
        for ( size_t nCol = 0; nCol < pCols->size(); ++nCol )
        {
            
            BrowserColumn *pCol = (*pCols)[ nCol ];

            
            if ( nCol < nFirstCol && !pCol->IsFrozen() )
            {
                nCol = nFirstCol;
                pCol = (*pCols)[ nCol ];
            }

            
            aVertPos.X() += pCol->Width();

            
            
            
            if ( aVertPos.X() > _rRect.Right() )
                break;

            
            if ( pCol->GetId() != 0 )
                _rOut.DrawLine( aVertPos, Point( aVertPos.X(),
                               bHLines
                                ? aPos.Y() - 1
                                : aPos.Y() + nDeltaY ) );
        }
    }

    _rOut.SetLineColor( aOldLineColor );
}



void BrowseBox::PaintData( Window& rWin, const Rectangle& rRect )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( STATE_CHANGE_INITSHOW );

    
    if ( !pCols || pCols->empty() || !rWin.IsUpdateMode() )
        return;
    if ( getDataWindow()->bResizeOnPaint )
        Resize();
    

    ImplPaintData(rWin, rRect, sal_False, sal_True);
}



void BrowseBox::UpdateScrollbars()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( !bBootstrapped || !IsUpdateMode() )
        return;

    
    BrowserDataWin *pBDW = (BrowserDataWin*) pDataWin;
    if ( pBDW->bInUpdateScrollbars )
    {
        pBDW->bHadRecursion = true;
        return;
    }
    pBDW->bInUpdateScrollbars = true;

    
    sal_uLong nCornerSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if (IsZoom())
        nCornerSize = (sal_uLong)(nCornerSize * (double)GetZoom());

    sal_Bool bNeedsVScroll = sal_False;
    long nMaxRows = 0;
    if (GetDataRowHeight())
    {
        
        nMaxRows = (pDataWin->GetSizePixel().Height()) / GetDataRowHeight();
        bNeedsVScroll =    getDataWindow()->bAutoVScroll
                            ?   nTopRow || ( nRowCount > nMaxRows )
                            :   !getDataWindow()->bNoVScroll;
    }
    Size aDataWinSize = pDataWin->GetSizePixel();
    if ( !bNeedsVScroll )
    {
        if ( pVScroll->IsVisible() )
        {
            pVScroll->Hide();
            Size aNewSize( aDataWinSize );
            aNewSize.Width() = GetOutputSizePixel().Width();
            aDataWinSize = aNewSize;
        }
    }
    else if ( !pVScroll->IsVisible() )
    {
        Size aNewSize( aDataWinSize );
        aNewSize.Width() = GetOutputSizePixel().Width() - nCornerSize;
        aDataWinSize = aNewSize;
    }

    
    sal_uLong nLastCol = GetColumnAtXPosPixel( aDataWinSize.Width() - 1 );

    sal_uInt16 nFrozenCols = FrozenColCount();
    sal_Bool bNeedsHScroll =    getDataWindow()->bAutoHScroll
                        ?   ( nFirstCol > nFrozenCols ) || ( nLastCol <= pCols->size() )
                        :   !getDataWindow()->bNoHScroll;
    if ( !bNeedsHScroll )
    {
        if ( aHScroll.IsVisible() )
        {
            aHScroll.Hide();
        }
        aDataWinSize.Height() = GetOutputSizePixel().Height() - GetTitleHeight();
        if ( nControlAreaWidth != USHRT_MAX )
            aDataWinSize.Height() -= nCornerSize;
    }
    else if ( !aHScroll.IsVisible() )
    {
        Size aNewSize( aDataWinSize );
        aNewSize.Height() = GetOutputSizePixel().Height() - GetTitleHeight() - nCornerSize;
        aDataWinSize = aNewSize;
    }

    
    sal_uLong nHScrX = nControlAreaWidth == USHRT_MAX
        ? 0
        : nControlAreaWidth;

    aHScroll.SetPosSizePixel(
        Point( nHScrX, GetOutputSizePixel().Height() - nCornerSize ),
        Size( aDataWinSize.Width() - nHScrX, nCornerSize ) );

    
    short nScrollCols = short(pCols->size()) - (short)nFrozenCols;

    
    short nVisibleHSize = nLastCol == BROWSER_INVALIDID
        ? (short)( pCols->size() - nFirstCol )
        : (short)( nLastCol - nFirstCol );

    short nRange = std::max( nScrollCols, (short)0 );
    aHScroll.SetVisibleSize( nVisibleHSize );
    aHScroll.SetRange( Range( 0, nRange ));
    if ( bNeedsHScroll && !aHScroll.IsVisible() )
        aHScroll.Show();

    
    pVScroll->SetPageSize( nMaxRows );

    if ( nTopRow > nRowCount )
    {
        nTopRow = nRowCount - 1;
        OSL_FAIL("BrowseBox: nTopRow > nRowCount");
    }

    if ( pVScroll->GetThumbPos() != nTopRow )
        pVScroll->SetThumbPos( nTopRow );
    long nVisibleSize = std::min( std::min( nRowCount, nMaxRows ), long(nRowCount-nTopRow) );
    pVScroll->SetVisibleSize( nVisibleSize ? nVisibleSize : 1 );
    pVScroll->SetRange( Range( 0, nRowCount ) );
    pVScroll->SetPosSizePixel(
        Point( aDataWinSize.Width(), GetTitleHeight() ),
        Size( nCornerSize, aDataWinSize.Height()) );
    long nLclDataRowHeight = GetDataRowHeight();
    if ( nLclDataRowHeight > 0 && nRowCount < long( aDataWinSize.Height() / nLclDataRowHeight ) )
        ScrollRows( -nTopRow );
    if ( bNeedsVScroll && !pVScroll->IsVisible() )
        pVScroll->Show();

    pDataWin->SetPosSizePixel(
        Point( 0, GetTitleHeight() ),
        aDataWinSize );

    
    
    sal_uLong nActualCorderWidth = 0;
    if (aHScroll.IsVisible() && pVScroll && pVScroll->IsVisible() )
    {
        
        nActualCorderWidth = nCornerSize;
    }
    else if ( !aHScroll.IsVisible() && ( nControlAreaWidth != USHRT_MAX ) )
    {
        
        
        nActualCorderWidth = GetOutputSizePixel().Width() - nControlAreaWidth;
    }
    if ( nActualCorderWidth )
    {
        if ( !getDataWindow()->pCornerWin )
            getDataWindow()->pCornerWin = new ScrollBarBox( this, 0 );
        getDataWindow()->pCornerWin->SetPosSizePixel(
            Point( GetOutputSizePixel().Width() - nActualCorderWidth, aHScroll.GetPosPixel().Y() ),
            Size( nActualCorderWidth, nCornerSize ) );
        getDataWindow()->pCornerWin->Show();
    }
    else
        DELETEZ( getDataWindow()->pCornerWin );

    
    if ( getDataWindow()->pHeaderBar )
    {
        long nWidth = 0;
        for ( size_t nCol = 0;
              nCol < pCols->size() && nCol < nFirstCol;
              ++nCol )
        {
            
            if ( (*pCols)[ nCol ]->GetId() )
                nWidth += (*pCols)[ nCol ]->Width();
        }

        getDataWindow()->pHeaderBar->SetOffset( nWidth );
    }

    pBDW->bInUpdateScrollbars = false;
    if ( pBDW->bHadRecursion )
    {
        pBDW->bHadRecursion = false;
        UpdateScrollbars();
    }
}



void BrowseBox::SetUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    sal_Bool bWasUpdate = IsUpdateMode();
    if ( bWasUpdate == bUpdate )
        return;

    Control::SetUpdateMode( bUpdate );
    
    
    if( bUpdate )
        getDataWindow()->Invalidate();
    getDataWindow()->SetUpdateMode( bUpdate );


    if ( bUpdate )
    {
        if ( bBootstrapped )
        {
            UpdateScrollbars();
            AutoSizeLastColumn();
        }
        DoShowCursor( "SetUpdateMode" );
    }
    else
        DoHideCursor( "SetUpdateMode" );
}



sal_Bool BrowseBox::GetUpdateMode() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return getDataWindow()->IsUpdateMode();
}



long BrowseBox::GetFrozenWidth() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    long nWidth = 0;
    for ( size_t nCol = 0;
          nCol < pCols->size() && (*pCols)[ nCol ]->IsFrozen();
          ++nCol )
        nWidth += (*pCols)[ nCol ]->Width();
    return nWidth;
}



void BrowseBox::ColumnInserted( sal_uInt16 nPos )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( pColSel )
        pColSel->Insert( nPos );
    UpdateScrollbars();
}



sal_uInt16 BrowseBox::FrozenColCount() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    sal_uInt16 nCol;
    for ( nCol = 0;
          nCol < pCols->size() && (*pCols)[ nCol ]->IsFrozen();
          ++nCol )
        /* empty loop */;
    return nCol;
}



IMPL_LINK(BrowseBox,ScrollHdl,ScrollBar*,pBar)
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( pBar->GetDelta() == 0 )
        return 0;

    if ( pBar->GetDelta() < 0 && getDataWindow()->bNoScrollBack )
    {
        UpdateScrollbars();
        return 0;
    }

    if ( pBar == &aHScroll )
        ScrollColumns( aHScroll.GetDelta() );
    if ( pBar == pVScroll )
        ScrollRows( pVScroll->GetDelta() );

    return 0;
}



IMPL_LINK_NOARG(BrowseBox, EndScrollHdl)
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( getDataWindow()->bNoScrollBack )
    {
        EndScroll();
        return 0;
    }

    return 0;
}



IMPL_LINK( BrowseBox, StartDragHdl, HeaderBar*, pBar )
{
    pBar->SetDragSize( pDataWin->GetOutputSizePixel().Height() );
    return 0;
}




void BrowseBox::MouseButtonDown( const MouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    GrabFocus();

    
    const Point &rEvtPos = rEvt.GetPosPixel();
    if ( rEvtPos.Y() >= GetTitleHeight() )
        return;

    long nX = 0;
    long nWidth = GetOutputSizePixel().Width();
    for ( size_t nCol = 0; nCol < pCols->size() && nX < nWidth; ++nCol )
    {
        
        BrowserColumn *pCol = (*pCols)[ nCol ];
        if ( pCol->IsFrozen() || nCol >= nFirstCol )
        {
            
            long nR = nX + pCol->Width() - 1;

            
            if ( pCol->GetId() && std::abs( nR - rEvtPos.X() ) < 2 )
            {
                
                bResizing = sal_True;
                nResizeCol = nCol;
                nDragX = nResizeX = rEvtPos.X();
                SetPointer( Pointer( POINTER_HSPLIT ) );
                CaptureMouse();
                pDataWin->DrawLine( Point( nDragX, 0 ),
                    Point( nDragX, pDataWin->GetSizePixel().Height() ) );
                nMinResizeX = nX + MIN_COLUMNWIDTH;
                return;
            }
            else if ( nX < rEvtPos.X() && nR > rEvtPos.X() )
            {
                MouseButtonDown( BrowserMouseEvent(
                    this, rEvt, -1, nCol, pCol->GetId(), Rectangle() ) );
                return;
            }
            nX = nR + 1;
        }
    }

    
    if ( rEvt.IsRight() )
        pDataWin->Command(
            CommandEvent( Point( 1, LONG_MAX ), COMMAND_CONTEXTMENU, true ) );
    else
        SetNoSelection();
}



void BrowseBox::MouseMove( const MouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    OSL_TRACE( "BrowseBox::MouseMove( MouseEvent )" );

    Pointer aNewPointer;

    sal_uInt16 nX = 0;
    for ( size_t nCol = 0;
          nCol < pCols->size() &&
            ( nX + (*pCols)[ nCol ]->Width() ) < sal_uInt16(GetOutputSizePixel().Width());
          ++nCol )
        
        if ( (*pCols)[ nCol ]->IsFrozen() || nCol >= nFirstCol )
        {
            
            BrowserColumn *pCol = (*pCols)[ nCol ];
            sal_uInt16 nR = (sal_uInt16)(nX + pCol->Width() - 1);

            
            if ( bResizing || ( pCol->GetId() &&
                 std::abs( ((long) nR ) - rEvt.GetPosPixel().X() ) < MIN_COLUMNWIDTH ) )
            {
                aNewPointer = Pointer( POINTER_HSPLIT );
                if ( bResizing )
                {
                    
                    pDataWin->HideTracking() ;

                    
                    nDragX = std::max( rEvt.GetPosPixel().X(), nMinResizeX );
                    long nDeltaX = nDragX - nResizeX;
                    sal_uInt16 nId = GetColumnId(nResizeCol);
                    sal_uLong nOldWidth = GetColumnWidth(nId);
                    nDragX = QueryColumnResize( GetColumnId(nResizeCol),
                                    nOldWidth + nDeltaX )
                             + nResizeX - nOldWidth;

                    
                    pDataWin->ShowTracking( Rectangle( Point( nDragX, 0 ),
                            Size( 1, pDataWin->GetSizePixel().Height() ) ),
                            SHOWTRACK_SPLIT|SHOWTRACK_WINDOW );
                }

            }

            nX = nR + 1;
        }

    SetPointer( aNewPointer );
}



void BrowseBox::MouseButtonUp( const MouseEvent & rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( bResizing )
    {
        
        pDataWin->HideTracking();

        
        nDragX = std::max( rEvt.GetPosPixel().X(), nMinResizeX );
        if ( (nDragX - nResizeX) != (long)(*pCols)[ nResizeCol ]->Width() )
        {
            
            long nMaxX = pDataWin->GetSizePixel().Width();
            nDragX = std::min( nDragX, nMaxX );
            long nDeltaX = nDragX - nResizeX;
            sal_uInt16 nId = GetColumnId(nResizeCol);
            SetColumnWidth( GetColumnId(nResizeCol), GetColumnWidth(nId) + nDeltaX );
            ColumnResized( nId );
        }

        
        SetPointer( Pointer() );
        ReleaseMouse();
        bResizing = sal_False;
    }
    else
        MouseButtonUp( BrowserMouseEvent( (BrowserDataWin*)pDataWin,
                MouseEvent( Point( rEvt.GetPosPixel().X(),
                        rEvt.GetPosPixel().Y() - pDataWin->GetPosPixel().Y() ),
                    rEvt.GetClicks(), rEvt.GetMode(), rEvt.GetButtons(),
                    rEvt.GetModifier() ) ) );
}



sal_Bool bExtendedMode = sal_False;
sal_Bool bFieldMode = sal_False;

void BrowseBox::MouseButtonDown( const BrowserMouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    GrabFocus();

    
    if ( rEvt.GetClicks() == 2 )
    {
        SetNoSelection();
        if ( rEvt.GetRow() >= 0 )
        {
            GoToRow( rEvt.GetRow() );
            SelectRow( rEvt.GetRow(), sal_True, sal_False );
        }
        else
        {
            if ( bColumnCursor && rEvt.GetColumn() != 0 )
            {
                if ( rEvt.GetColumn() < pCols->size() )
                    SelectColumnPos( rEvt.GetColumn(), sal_True, sal_False);
            }
        }
        DoubleClick( rEvt );
    }
    
    else if ( ( rEvt.GetMode() & ( MOUSE_SELECT | MOUSE_SIMPLECLICK ) ) &&
         ( bColumnCursor || rEvt.GetRow() >= 0 ) )
    {
        if ( rEvt.GetClicks() == 1 )
        {
            
            bHit            = sal_False;
            a1stPoint       =
            a2ndPoint       = PixelToLogic( rEvt.GetPosPixel() );

            
            if ( rEvt.GetRow() >= nRowCount ||
                 rEvt.GetColumnId() == BROWSER_INVALIDID )
            {
                SetNoSelection();
                return;
            }

            
            bSelecting = sal_True;
            DoHideCursor( "MouseButtonDown" );

            
            if ( rEvt.GetRow() >= 0 )
            {
                
                if ( rEvt.GetColumnId() == HandleColumnId || !bColumnCursor )
                {
                    if ( bMultiSelection )
                    {
                        
                        if ( pColSel && pColSel->GetSelectCount() )
                        {
                            ToggleSelection();
                            if ( bMultiSelection )
                                uRow.pSel->SelectAll(false);
                            else
                                uRow.nSel = BROWSER_ENDOFSELECTION;
                            if ( pColSel )
                                pColSel->SelectAll(false);
                            bSelect = true;
                        }

                        
                        if ( rEvt.GetMode() & MOUSE_RANGESELECT )
                        {
                            
                            bSelect = true;
                            ExpandRowSelection( rEvt );
                            return;
                        }

                        
                        else if ( IsRowSelected( rEvt.GetRow() ) )
                        {
                            
                            bHit = sal_True;
                            bExtendedMode = MOUSE_MULTISELECT ==
                                    ( rEvt.GetMode() & MOUSE_MULTISELECT );
                            return;
                        }

                        
                        else if ( rEvt.GetMode() & MOUSE_MULTISELECT )
                        {
                            
                            
                            aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
                            SelectRow( rEvt.GetRow(),
                                    !uRow.pSel->IsSelected( rEvt.GetRow() ) );
                            bSelect = true;
                            return;
                        }
                    }

                    
                    SetNoSelection();
                    GoToRow( rEvt.GetRow() );
                    SelectRow( rEvt.GetRow(), sal_True );
                    aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
                    bSelect = true;
                }
                else 
                {
                    
                    if ( IsColumnSelected( rEvt.GetColumn() ) ||
                         IsRowSelected( rEvt.GetRow() ) )
                    {
                        bHit = sal_True;
                        bFieldMode = sal_True;
                        return;
                    }

                    SetNoSelection();
                    GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
                    bSelect = true;
                }
            }
            else
            {
                if ( bMultiSelection && rEvt.GetColumnId() == HandleColumnId )
                {
                    
                    if ( uRow.pSel->GetSelectCount() > ( GetRowCount() / 2 ) )
                        SetNoSelection();
                    else
                        SelectAll();
                }
                else
                    SelectColumnId( rEvt.GetColumnId(), sal_True, sal_False );
            }

            
            bSelecting = sal_False;
            DoShowCursor( "MouseButtonDown" );
            if ( bSelect )
                Select();
        }
    }
}



void BrowseBox::MouseMove( const BrowserMouseEvent& )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}



void BrowseBox::MouseButtonUp( const BrowserMouseEvent &rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    
    if ( bHit )
    {
        aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
        if ( bExtendedMode )
            SelectRow( rEvt.GetRow(), sal_False );
        else
        {
            SetNoSelection();
            if ( bFieldMode )
                GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
            else
            {
                GoToRow( rEvt.GetRow() );
                SelectRow( rEvt.GetRow(), sal_True );
            }
        }
        bSelect = true;
        bExtendedMode = sal_False;
        bFieldMode = sal_False;
        bHit = sal_False;
    }

    
    if ( bSelecting )
    {
        bSelecting = sal_False;
        DoShowCursor( "MouseButtonUp" );
        if ( bSelect )
            Select();
    }
}



void BrowseBox::KeyInput( const KeyEvent& rEvt )
{
    if ( !ProcessKey( rEvt ) )
        Control::KeyInput( rEvt );
}



sal_Bool BrowseBox::ProcessKey( const KeyEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    sal_uInt16 nCode = rEvt.GetKeyCode().GetCode();
    sal_Bool   bShift = rEvt.GetKeyCode().IsShift();
    sal_Bool   bCtrl = rEvt.GetKeyCode().IsMod1();
    sal_Bool   bAlt = rEvt.GetKeyCode().IsMod2();

    sal_uInt16 nId = BROWSER_NONE;

    if ( !bAlt && !bCtrl && !bShift )
    {
        switch ( nCode )
        {
            case KEY_DOWN:          nId = BROWSER_CURSORDOWN; break;
            case KEY_UP:            nId = BROWSER_CURSORUP; break;
            case KEY_HOME:          nId = BROWSER_CURSORHOME; break;
            case KEY_END:           nId = BROWSER_CURSOREND; break;
            case KEY_TAB:
                if ( !bColumnCursor )
                    break;
            case KEY_RIGHT:         nId = BROWSER_CURSORRIGHT; break;
            case KEY_LEFT:          nId = BROWSER_CURSORLEFT; break;
            case KEY_SPACE:         nId = BROWSER_SELECT; break;
        }
        if ( BROWSER_NONE != nId )
            SetNoSelection();

        switch ( nCode )
        {
            case KEY_PAGEDOWN:      nId = BROWSER_CURSORPAGEDOWN; break;
            case KEY_PAGEUP:        nId = BROWSER_CURSORPAGEUP; break;
        }
    }

    if ( !bAlt && !bCtrl && bShift )
        switch ( nCode )
        {
            case KEY_DOWN:          nId = BROWSER_SELECTDOWN; break;
            case KEY_UP:            nId = BROWSER_SELECTUP; break;
            case KEY_TAB:
                if ( !bColumnCursor )
                    break;
                                    nId = BROWSER_CURSORLEFT; break;
            case KEY_HOME:          nId = BROWSER_SELECTHOME; break;
            case KEY_END:           nId = BROWSER_SELECTEND; break;
        }


    if ( !bAlt && bCtrl && !bShift )
        switch ( nCode )
        {
            case KEY_DOWN:          nId = BROWSER_CURSORDOWN; break;
            case KEY_UP:            nId = BROWSER_CURSORUP; break;
            case KEY_PAGEDOWN:      nId = BROWSER_CURSORENDOFFILE; break;
            case KEY_PAGEUP:        nId = BROWSER_CURSORTOPOFFILE; break;
            case KEY_HOME:          nId = BROWSER_CURSORTOPOFSCREEN; break;
            case KEY_END:           nId = BROWSER_CURSORENDOFSCREEN; break;
            case KEY_SPACE:         nId = BROWSER_ENHANCESELECTION; break;
            case KEY_LEFT:          nId = BROWSER_MOVECOLUMNLEFT; break;
            case KEY_RIGHT:         nId = BROWSER_MOVECOLUMNRIGHT; break;
        }

    if ( nId != BROWSER_NONE )
        Dispatch( nId );
    return nId != BROWSER_NONE;
}



void BrowseBox::Dispatch( sal_uInt16 nId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    long nRowsOnPage = pDataWin->GetSizePixel().Height() / GetDataRowHeight();
    sal_Bool bDone = sal_False;

    switch ( nId )
    {
        case BROWSER_SELECTCOLUMN:
            if ( ColCount() )
                SelectColumnId( GetCurColumnId() );
            break;

        case BROWSER_CURSORDOWN:
            if ( ( GetCurRow() + 1 ) < nRowCount )
                bDone = GoToRow( GetCurRow() + 1, sal_False );
            break;
        case BROWSER_CURSORUP:
            if ( GetCurRow() > 0 )
                bDone = GoToRow( GetCurRow() - 1, sal_False );
            break;
        case BROWSER_SELECTHOME:
            if ( GetRowCount() )
            {
                DoHideCursor( "BROWSER_SELECTHOME" );
                for ( long nRow = GetCurRow(); nRow >= 0; --nRow )
                    SelectRow( nRow );
                GoToRow( 0, sal_True );
                DoShowCursor( "BROWSER_SELECTHOME" );
            }
            break;
        case BROWSER_SELECTEND:
            if ( GetRowCount() )
            {
                DoHideCursor( "BROWSER_SELECTEND" );
                long nRows = GetRowCount();
                for ( long nRow = GetCurRow(); nRow < nRows; ++nRow )
                    SelectRow( nRow );
                GoToRow( GetRowCount() - 1, sal_True );
                DoShowCursor( "BROWSER_SELECTEND" );
            }
            break;
        case BROWSER_SELECTDOWN:
        {
            if ( GetRowCount() && ( GetCurRow() + 1 ) < nRowCount )
            {
                
                
                long nRow = GetCurRow();
                sal_Bool bLocalSelect = ( !IsRowSelected( nRow ) ||
                                 GetSelectRowCount() == 1 || IsRowSelected( nRow - 1 ) );
                SelectRow( nRow, bLocalSelect, sal_True );
                bDone = GoToRow( GetCurRow() + 1 , sal_False );
                if ( bDone )
                    SelectRow( GetCurRow(), sal_True, sal_True );
            }
            else
                bDone = ScrollRows( 1 ) != 0;
            break;
        }
        case BROWSER_SELECTUP:
            if ( GetRowCount() )
            {
                
                
                long nRow = GetCurRow();
                sal_Bool bLocalSelect = ( !IsRowSelected( nRow ) ||
                                 GetSelectRowCount() == 1 || IsRowSelected( nRow + 1 ) );
                SelectRow( nCurRow, bLocalSelect, sal_True );
                bDone = GoToRow( nRow - 1 , sal_False );
                if ( bDone )
                    SelectRow( GetCurRow(), sal_True, sal_True );
            }
            break;
        case BROWSER_CURSORPAGEDOWN:
            bDone = (sal_Bool)ScrollRows( nRowsOnPage );
            break;
        case BROWSER_CURSORPAGEUP:
            bDone = (sal_Bool)ScrollRows( -nRowsOnPage );
            break;
        case BROWSER_CURSOREND:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewId = GetColumnId(ColCount() -1);
                bDone = nNewId != HandleColumnId && GoToColumnId( nNewId );
                break;
            }
        case BROWSER_CURSORENDOFFILE:
            bDone = GoToRow( nRowCount - 1, sal_False );
            break;
        case BROWSER_CURSORRIGHT:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewPos = GetColumnPos( GetCurColumnId() ) + 1;
                sal_uInt16 nNewId = GetColumnId( nNewPos );
                if (nNewId != BROWSER_INVALIDID)    
                    bDone = GoToColumnId( nNewId );
                else
                {
                    sal_uInt16 nColId = GetColumnId(0);
                    if ( nColId == BROWSER_INVALIDID || nColId == HandleColumnId )
                        nColId = GetColumnId(1);
                    if ( GetRowCount() )
                        bDone = ( nCurRow < GetRowCount() - 1 ) && GoToRowColumnId( nCurRow + 1, nColId );
                    else if ( ColCount() )
                        GoToColumnId( nColId );
                }
            }
            else
                bDone = ScrollColumns( 1 ) != 0;
            break;
        case BROWSER_CURSORHOME:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewId = GetColumnId(1);
                bDone = (nNewId != HandleColumnId) && GoToColumnId( nNewId );
                break;
            }
        case BROWSER_CURSORTOPOFFILE:
            bDone = GoToRow( 0, sal_False );
            break;
        case BROWSER_CURSORLEFT:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewPos = GetColumnPos( GetCurColumnId() ) - 1;
                sal_uInt16 nNewId = GetColumnId( nNewPos );
                if (nNewId != HandleColumnId)
                    bDone = GoToColumnId( nNewId );
                else
                {
                    if ( GetRowCount() )
                        bDone = (nCurRow > 0) && GoToRowColumnId(nCurRow - 1, GetColumnId(ColCount() -1));
                    else if ( ColCount() )
                        GoToColumnId( GetColumnId(ColCount() -1) );
                }
            }
            else
                bDone = ScrollColumns( -1 ) != 0;
            break;
        case BROWSER_ENHANCESELECTION:
            if ( GetRowCount() )
                SelectRow( GetCurRow(), !IsRowSelected( GetCurRow() ), sal_True );
            bDone = sal_True;
            break;
        case BROWSER_SELECT:
            if ( GetRowCount() )
                SelectRow( GetCurRow(), !IsRowSelected( GetCurRow() ), sal_False );
            bDone = sal_True;
            break;
        case BROWSER_MOVECOLUMNLEFT:
        case BROWSER_MOVECOLUMNRIGHT:
            { 
                BrowserHeader* pHeaderBar = getDataWindow()->pHeaderBar;
                if ( pHeaderBar && pHeaderBar->IsDragable() )
                {
                    sal_uInt16 nColId = GetCurColumnId();
                    sal_Bool bColumnSelected = IsColumnSelected(nColId);
                    sal_uInt16 nNewPos = GetColumnPos(nColId);
                    sal_Bool bMoveAllowed = sal_False;
                    if ( BROWSER_MOVECOLUMNLEFT == nId && nNewPos > 1 )
                        --nNewPos,bMoveAllowed = sal_True;
                    else if ( BROWSER_MOVECOLUMNRIGHT == nId && nNewPos < (ColCount()-1) )
                        ++nNewPos,bMoveAllowed = sal_True;

                    if ( bMoveAllowed )
                    {
                        SetColumnPos( nColId, nNewPos );
                        ColumnMoved( nColId );
                        MakeFieldVisible(GetCurRow(),nColId,sal_True);
                        if ( bColumnSelected )
                            SelectColumnId(nColId);
                    }
                }
            }
            break;
    }

    
}



void BrowseBox::SetCursorColor(const Color& _rCol)
{
    if (_rCol == m_aCursorColor)
        return;

    
    DoHideCursor("SetCursorColor");
    if (!m_bFocusOnlyCursor)
        DoHideCursor("SetCursorColor - force");

    m_aCursorColor = _rCol;

    if (!m_bFocusOnlyCursor)
        DoShowCursor("SetCursorColor - force");
    DoShowCursor("SetCursorColor");
}

Rectangle BrowseBox::calcHeaderRect(sal_Bool _bIsColumnBar,sal_Bool _bOnScreen)
{
    Window* pParent = NULL;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    Point aTopLeft;
    long nWidth;
    long nHeight;
    if ( _bIsColumnBar )
    {
        nWidth = GetDataWindow().GetOutputSizePixel().Width();
        nHeight = GetDataRowHeight();
    }
    else
    {
        aTopLeft.Y() = GetDataRowHeight();
        nWidth = GetColumnWidth(0);
        nHeight = GetWindowExtentsRelative( pParent ).GetHeight() - aTopLeft.Y() - GetControlArea().GetSize().B();
    }
    aTopLeft += GetWindowExtentsRelative( pParent ).TopLeft();
    return Rectangle(aTopLeft,Size(nWidth,nHeight));
}

Rectangle BrowseBox::calcTableRect(sal_Bool _bOnScreen)
{
    Window* pParent = NULL;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    Rectangle aRect( GetWindowExtentsRelative( pParent ) );
    Rectangle aRowBar = calcHeaderRect(sal_False,pParent == NULL);

    long nX = aRowBar.Right() - aRect.Left();
    long nY = aRowBar.Top() - aRect.Top();
    Size aSize(aRect.GetSize());

    return Rectangle(aRowBar.TopRight(), Size(aSize.A() - nX, aSize.B() - nY - aHScroll.GetSizePixel().Height()) );
}

Rectangle BrowseBox::GetFieldRectPixelAbs( sal_Int32 _nRowId,sal_uInt16 _nColId, sal_Bool /*_bIsHeader*/, sal_Bool _bOnScreen )
{
    Window* pParent = NULL;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    Rectangle aRect = GetFieldRectPixel(_nRowId,_nColId,_bOnScreen);

    Point aTopLeft = aRect.TopLeft();
    aTopLeft += GetWindowExtentsRelative( pParent ).TopLeft();

    return Rectangle(aTopLeft,aRect.GetSize());
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
