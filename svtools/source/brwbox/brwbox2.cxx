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

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <svtools/brwbox.hxx>
#include "datwin.hxx"
#include <svtools/colorcfg.hxx>
#include <vcl/salgtype.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/ptrstyle.hxx>

#include <tools/multisel.hxx>
#include <tools/fract.hxx>
#include <algorithm>
#include <memory>

using namespace ::com::sun::star::datatransfer;


void BrowseBox::StartDrag( sal_Int8 /* _nAction */, const Point& /* _rPosPixel */ )
{
    // not interested in this event
}


sal_Int8 BrowseBox::AcceptDrop( const AcceptDropEvent& _rEvt )
{
    AcceptDropEvent aTransformed( _rEvt );
    aTransformed.maPosPixel = pDataWin->ScreenToOutputPixel( OutputToScreenPixel( _rEvt.maPosPixel ) );
    return pDataWin->AcceptDrop( aTransformed );
}


sal_Int8 BrowseBox::ExecuteDrop( const ExecuteDropEvent& _rEvt )
{
    ExecuteDropEvent aTransformed( _rEvt );
    aTransformed.maPosPixel = pDataWin->ScreenToOutputPixel( OutputToScreenPixel( _rEvt.maPosPixel ) );
    return pDataWin->ExecuteDrop( aTransformed );
}


sal_Int8 BrowseBox::AcceptDrop( const BrowserAcceptDropEvent& )
{
    // not interested in this event
    return DND_ACTION_NONE;
}


sal_Int8 BrowseBox::ExecuteDrop( const BrowserExecuteDropEvent& )
{
    // not interested in this event
    return DND_ACTION_NONE;
}


const DataFlavorExVector& BrowseBox::GetDataFlavors() const
{
    if (pDataWin->bCallingDropCallback)
        return pDataWin->GetDataFlavorExVector();
    return GetDataFlavorExVector();
}


bool BrowseBox::IsDropFormatSupported( SotClipboardFormatId _nFormat )
{
    if ( pDataWin->bCallingDropCallback )
        return pDataWin->IsDropFormatSupported( _nFormat );

    return DropTargetHelper::IsDropFormatSupported( _nFormat );
}


void BrowseBox::Command( const CommandEvent& rEvt )
{
    if ( !pDataWin->bInCommand )
        Control::Command( rEvt );
}


void BrowseBox::StateChanged( StateChangedType nStateChange )
{
    Control::StateChanged( nStateChange );

    if ( StateChangedType::Mirroring == nStateChange )
    {
        pDataWin->EnableRTL( IsRTLEnabled() );

        HeaderBar* pHeaderBar = pDataWin->pHeaderBar;
        if ( pHeaderBar )
            pHeaderBar->EnableRTL( IsRTLEnabled() );
        aHScroll->EnableRTL( IsRTLEnabled() );
        if( pVScroll )
            pVScroll->EnableRTL( IsRTLEnabled() );
        Resize();
    }
    else if ( StateChangedType::InitShow == nStateChange )
    {
        bBootstrapped = true; // must be set first!

        Resize();
        if ( bMultiSelection )
            uRow.pSel->SetTotalRange( Range( 0, nRowCount - 1 ) );
        if ( nRowCount == 0 )
            nCurRow = BROWSER_ENDOFSELECTION;
        else if ( nCurRow == BROWSER_ENDOFSELECTION )
            nCurRow = 0;


        if ( HasFocus() )
        {
            bSelectionIsVisible = true;
            bHasFocus = true;
        }
        UpdateScrollbars();
        AutoSizeLastColumn();
        CursorMoved();
    }
    else if (StateChangedType::Zoom == nStateChange)
    {
        pDataWin->SetZoom(GetZoom());
        HeaderBar* pHeaderBar = pDataWin->pHeaderBar;
        if (pHeaderBar)
            pHeaderBar->SetZoom(GetZoom());

        // let the columns calculate their new widths and adjust the header bar
        for (auto & pCol : mvCols)
        {
            pCol->ZoomChanged(GetZoom());
            if ( pHeaderBar )
                pHeaderBar->SetItemSize( pCol->GetId(), pCol->Width() );
        }

        // all our controls have to be repositioned
        Resize();
    }
    else if (StateChangedType::Enable == nStateChange)
    {
        // do we have a handle column?
        bool bHandleCol = !mvCols.empty() && (0 == mvCols[ 0 ]->GetId());
        // do we have a header bar?
        bool bHeaderBar = (nullptr != pDataWin->pHeaderBar.get());

        if  (   nTitleLines
            &&  (   !bHeaderBar
                ||  bHandleCol
                )
            )
            // we draw the text in our header bar in a color dependent on the enabled state. So if this state changed
            // -> redraw
            Invalidate(tools::Rectangle(Point(0, 0), Size(GetOutputSizePixel().Width(), GetTitleHeight() - 1)));
    }
}


void BrowseBox::Select()
{
}


void BrowseBox::DoubleClick( const BrowserMouseEvent & )
{
}


long BrowseBox::QueryMinimumRowHeight()
{
    return CalcZoom( 5 );
}


void BrowseBox::ImplStartTracking()
{
}


void BrowseBox::ImplEndTracking()
{
}


void BrowseBox::RowHeightChanged()
{
}


void BrowseBox::ColumnResized( sal_uInt16 )
{
}


void BrowseBox::ColumnMoved( sal_uInt16 )
{
}


void BrowseBox::StartScroll()
{
    DoHideCursor( "StartScroll" );
}


void BrowseBox::EndScroll()
{
    UpdateScrollbars();
    AutoSizeLastColumn();
    DoShowCursor( "EndScroll" );
}


void BrowseBox::ToggleSelection()
{

    // selection highlight-toggling allowed?
    if ( bHideSelect )
        return;
    if ( bNotToggleSel || !IsUpdateMode() || !bSelectionIsVisible )
        return;

    // only highlight painted areas!
    bNotToggleSel = true;

    // accumulate areas of rows to highlight
    std::vector<tools::Rectangle> aHighlightList;
    long nLastRowInRect = 0; // for the CFront

    // don't highlight handle column
    BrowserColumn *pFirstCol = mvCols.empty() ? nullptr : mvCols[ 0 ].get();
    long nOfsX = (!pFirstCol || pFirstCol->GetId()) ? 0 : pFirstCol->Width();

    // accumulate old row selection
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

        tools::Rectangle aAddRect(
            Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
            Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
        if ( !aHighlightList.empty() && nLastRowInRect == ( nRow - 1 ) )
            aHighlightList[ 0 ].Union( aAddRect );
        else
            aHighlightList.emplace( aHighlightList.begin(), aAddRect );
        nLastRowInRect = nRow;
    }

    // unhighlight the old selection (if any)
    while ( !aHighlightList.empty() )
    {
        pDataWin->Invalidate( aHighlightList.back() );
        aHighlightList.pop_back();
    }

    // unhighlight old column selection (if any)
    for ( long nColId = pColSel ? pColSel->FirstSelected() : BROWSER_ENDOFSELECTION;
          nColId != BROWSER_ENDOFSELECTION;
          nColId = pColSel->NextSelected() )
    {
        tools::Rectangle aRect( GetFieldRectPixel(nCurRow,
                                           mvCols[ nColId ]->GetId(),
                                           false ) );
        aRect.AdjustLeft( -(MIN_COLUMNWIDTH) );
        aRect.AdjustRight(MIN_COLUMNWIDTH );
        aRect.SetTop( 0 );
        aRect.SetBottom( pDataWin->GetOutputSizePixel().Height() );
        pDataWin->Invalidate( aRect );
    }

    bNotToggleSel = false;
}


void BrowseBox::DrawCursor()
{
    bool bReallyHide = false;
    if ( bHideCursor == TRISTATE_INDET )
    {
        if ( !GetSelectRowCount() && !GetSelectColumnCount() )
            bReallyHide = true;
    }
    else if ( bHideCursor == TRISTATE_TRUE )
    {
        bReallyHide = true;
    }

    bReallyHide |= !bSelectionIsVisible || !IsUpdateMode() || bScrolling || nCurRow < 0;

    if (PaintCursorIfHiddenOnce())
        bReallyHide |= ( GetCursorHideCount() > 1 );
    else
        bReallyHide |= ( GetCursorHideCount() > 0 );

    // no cursor on handle column
    if ( nCurColId == HandleColumnId )
        nCurColId = GetColumnId(1);

    // calculate cursor rectangle
    tools::Rectangle aCursor;
    if ( bColumnCursor )
    {
        aCursor = GetFieldRectPixel( nCurRow, nCurColId, false );
        aCursor.AdjustLeft( -(MIN_COLUMNWIDTH) );
        aCursor.AdjustRight(1 );
        aCursor.AdjustBottom(1 );
    }
    else
        aCursor = tools::Rectangle(
            Point( ( !mvCols.empty() && mvCols[ 0 ]->GetId() == 0 ) ?
                        mvCols[ 0 ]->Width() : 0,
                        (nCurRow - nTopRow) * GetDataRowHeight() + 1 ),
            Size( pDataWin->GetOutputSizePixel().Width() + 1,
                  GetDataRowHeight() - 2 ) );
    if ( bHLines )
    {
        if ( !bMultiSelection )
            aCursor.AdjustTop( -1 );
        aCursor.AdjustBottom( -1 );
    }

    if (m_aCursorColor == COL_TRANSPARENT)
    {
        // on these platforms, the StarView focus works correctly
        if ( bReallyHide )
            static_cast<Control*>(pDataWin.get())->HideFocus();
        else
            static_cast<Control*>(pDataWin.get())->ShowFocus( aCursor );
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

    sal_uInt16 nItemPos = GetColumnPos( nId );
    if ( nItemPos >= mvCols.size() )
        return 0;
    return mvCols[ nItemPos ]->Width();
}


sal_uInt16 BrowseBox::GetColumnId( sal_uInt16 nPos ) const
{

    if ( nPos >= mvCols.size() )
        return BROWSER_INVALIDID;
    return mvCols[ nPos ]->GetId();
}


sal_uInt16 BrowseBox::GetColumnPos( sal_uInt16 nId ) const
{
    for ( size_t nPos = 0; nPos < mvCols.size(); ++nPos )
        if ( mvCols[ nPos ]->GetId() == nId )
            return nPos;
    return BROWSER_INVALIDID;
}


bool BrowseBox::IsFrozen( sal_uInt16 nColumnId ) const
{
    for (auto const & pCol : mvCols)
        if ( pCol->GetId() == nColumnId )
            return pCol->IsFrozen();
    return false;
}


void BrowseBox::ExpandRowSelection( const BrowserMouseEvent& rEvt )
{

    DoHideCursor( "ExpandRowSelection" );

    // expand the last selection
    if ( bMultiSelection )
    {
        Range aJustifiedRange( aSelRange );
        aJustifiedRange.Justify();

        bool bSelectThis = ( bSelect != aJustifiedRange.IsInside( rEvt.GetRow() ) );

        if ( aJustifiedRange.IsInside( rEvt.GetRow() ) )
        {
            // down and up
            while ( rEvt.GetRow() < aSelRange.Max() )
            {   // ZTC/Mac bug - don't put these statements together!
                SelectRow( aSelRange.Max(), bSelectThis );
                --aSelRange.Max();
            }
            while ( rEvt.GetRow() > aSelRange.Max() )
            {   // ZTC/Mac bug - don't put these statements together!
                SelectRow( aSelRange.Max(), bSelectThis );
                ++aSelRange.Max();
            }
        }
        else
        {
            // up and down
            bool bOldSelecting = bSelecting;
            bSelecting = true;
            while ( rEvt.GetRow() < aSelRange.Max() )
            {   // ZTC/Mac bug - don't put these statements together!
                --aSelRange.Max();
                if ( !IsRowSelected( aSelRange.Max() ) )
                {
                    SelectRow( aSelRange.Max(), bSelectThis );
                    bSelect = true;
                }
            }
            while ( rEvt.GetRow() > aSelRange.Max() )
            {   // ZTC/Mac bug - don't put these statements together!
                ++aSelRange.Max();
                if ( !IsRowSelected( aSelRange.Max() ) )
                {
                    SelectRow( aSelRange.Max(), bSelectThis );
                    bSelect = true;
                }
            }
            bSelecting = bOldSelecting;
            if ( bSelect )
                Select();
        }
    }
    else
        if (!IsRowSelected(rEvt.GetRow()))
            SelectRow( rEvt.GetRow() );

    GoToRow( rEvt.GetRow(), false );
    DoShowCursor( "ExpandRowSelection" );
}


void BrowseBox::Resize()
{
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( StateChangedType::InitShow );
    if ( mvCols.empty() )
    {
        pDataWin->bResizeOnPaint = true;
        return;
    }
    pDataWin->bResizeOnPaint = false;

    // calc the size of the scrollbars
    sal_uLong nSBSize = GetBarHeight();
    if (IsZoom())
        nSBSize = static_cast<sal_uLong>(nSBSize * static_cast<double>(GetZoom()));

    DoHideCursor( "Resize" );
    sal_uInt16 nOldVisibleRows = 0;
    //fdo#42694, post #i111125# GetDataRowHeight() can be 0
    if (GetDataRowHeight())
        nOldVisibleRows = static_cast<sal_uInt16>(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    // did we need a horizontal scroll bar or is there a Control Area?
    if ( !pDataWin->bNoHScroll &&
         ( ( mvCols.size() - FrozenColCount() ) > 1 ) )
        aHScroll->Show();
    else
        aHScroll->Hide();

    // calculate the size of the data window
    long nDataHeight = GetOutputSizePixel().Height() - GetTitleHeight();
    if ( aHScroll->IsVisible() || ( nControlAreaWidth != USHRT_MAX ) )
        nDataHeight -= nSBSize;

    long nDataWidth = GetOutputSizePixel().Width();
    if ( pVScroll->IsVisible() )
        nDataWidth -= nSBSize;

    // adjust position and size of data window
    pDataWin->SetPosSizePixel(
        Point( 0, GetTitleHeight() ),
        Size( nDataWidth, nDataHeight ) );

    sal_uInt16 nVisibleRows = 0;

    if (GetDataRowHeight())
        nVisibleRows = static_cast<sal_uInt16>(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    // TopRow is unchanged, but the number of visible lines has changed.
    if ( nVisibleRows != nOldVisibleRows )
        VisibleRowsChanged(nTopRow, nVisibleRows);

    UpdateScrollbars();

    // Control-Area
    tools::Rectangle aInvalidArea( GetControlArea() );
    aInvalidArea.SetRight( GetOutputSizePixel().Width() );
    aInvalidArea.SetLeft( 0 );
    Invalidate( aInvalidArea );

    // external header-bar
    HeaderBar* pHeaderBar = pDataWin->pHeaderBar;
    if ( pHeaderBar )
    {
        // take the handle column into account
        BrowserColumn *pFirstCol = mvCols[ 0 ].get();
        long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();
        pHeaderBar->SetPosSizePixel( Point( nOfsX, 0 ), Size( GetOutputSizePixel().Width() - nOfsX, GetTitleHeight() ) );
    }

    AutoSizeLastColumn(); // adjust last column width
    DoShowCursor( "Resize" );
}


void BrowseBox::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{

    // initializations
    if (!bBootstrapped && IsReallyVisible())
        BrowseBox::StateChanged(StateChangedType::InitShow);
    if (mvCols.empty())
        return;

    BrowserColumn *pFirstCol = mvCols[ 0 ].get();
    bool bHandleCol = pFirstCol && pFirstCol->GetId() == 0;
    bool bHeaderBar = pDataWin->pHeaderBar.get() != nullptr;

    // draw delimitational lines
    if (!pDataWin->bNoHScroll)
        rRenderContext.DrawLine(Point(0, aHScroll->GetPosPixel().Y()),
                                Point(GetOutputSizePixel().Width(),
                                      aHScroll->GetPosPixel().Y()));

    if (nTitleLines)
    {
        if (!bHeaderBar)
        {
            rRenderContext.DrawLine(Point(0, GetTitleHeight() - 1),
                                    Point(GetOutputSizePixel().Width(), GetTitleHeight() - 1));
        }
        else if (bHandleCol)
        {
            rRenderContext.DrawLine(Point(0, GetTitleHeight() - 1),
                                    Point(pFirstCol->Width(), GetTitleHeight() - 1));
        }
    }

    // Title Bar
    // If there is a handle column and if the  header bar is available, only
    // take the HandleColumn into account
    if (!(nTitleLines && (!bHeaderBar || bHandleCol)))
        return;

    // iterate through columns to redraw
    long nX = 0;
    size_t nCol;
    for (nCol = 0; nCol < mvCols.size() && nX < rRect.Right(); ++nCol)
    {
        // skip invisible columns between frozen and scrollable area
        if (nCol < nFirstCol && !mvCols[nCol]->IsFrozen())
            nCol = nFirstCol;

        // only the handle column?
        if (bHeaderBar && bHandleCol && nCol > 0)
            break;

        BrowserColumn* pCol = mvCols[nCol].get();

        // draw the column and increment position
        if ( pCol->Width() > 4 )
        {
            ButtonFrame aButtonFrame( Point( nX, 0 ),
                Size( pCol->Width()-1, GetTitleHeight()-1 ),
                pCol->Title(), !IsEnabled());
            aButtonFrame.Draw(rRenderContext);
            rRenderContext.DrawLine(Point(nX + pCol->Width() - 1, 0),
                                    Point(nX + pCol->Width() - 1, GetTitleHeight() - 1));
        }
        else
        {
            rRenderContext.Push(PushFlags::FILLCOLOR);
            rRenderContext.SetFillColor(COL_BLACK);
            rRenderContext.DrawRect(tools::Rectangle(Point(nX, 0), Size(pCol->Width(), GetTitleHeight() - 1)));
            rRenderContext.Pop();
        }

        // skip column
        nX += pCol->Width();
    }

    // retouching
    if ( !bHeaderBar && nCol == mvCols.size() )
    {
        const StyleSettings &rSettings = rRenderContext.GetSettings().GetStyleSettings();
        Color aColFace(rSettings.GetFaceColor());
        rRenderContext.Push(PushFlags::FILLCOLOR | PushFlags::LINECOLOR);
        rRenderContext.SetFillColor(aColFace);
        rRenderContext.SetLineColor(aColFace);
        rRenderContext.DrawRect(tools::Rectangle(Point(nX, 0),
                                          Point(rRect.Right(), GetTitleHeight() - 2 )));
        rRenderContext.Pop();
    }
}


void BrowseBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags )
{
    // we need pixel coordinates
    Size aRealSize = pDev->LogicToPixel(rSize);
    Point aRealPos = pDev->LogicToPixel(rPos);

    if ((rSize.Width() < 3) || (rSize.Height() < 3))
        // we want to have two pixels frame ...
        return;

    vcl::Font aFont = pDataWin->GetDrawPixelFont( pDev );
        // the 'normal' painting uses always the data window as device to output to, so we have to calc the new font
        // relative to the data wins current settings

    pDev->Push();
    pDev->SetMapMode();
    pDev->SetFont( aFont );

    // draw a frame
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

    HeaderBar* pBar = pDataWin->pHeaderBar;

    // we're drawing onto a foreign device, so we have to fake the DataRowHeight for the subsequent ImplPaintData
    // (as it is based on the settings of our data window, not the foreign device)
    if (!nDataRowHeight)
        ImpGetDataRowHeight();
    long nHeightLogic = PixelToLogic(Size(0, nDataRowHeight), MapMode(MapUnit::Map10thMM)).Height();
    long nForeignHeightPixel = pDev->LogicToPixel(Size(0, nHeightLogic), MapMode(MapUnit::Map10thMM)).Height();

    long nOriginalHeight = nDataRowHeight;
    nDataRowHeight = nForeignHeightPixel;

    // this counts for the column widths, too
    size_t nPos;
    for ( nPos = 0; nPos < mvCols.size(); ++nPos )
    {
        BrowserColumn* pCurrent = mvCols[ nPos ].get();

        long nWidthLogic = PixelToLogic(Size(pCurrent->Width(), 0), MapMode(MapUnit::Map10thMM)).Width();
        long nForeignWidthPixel = pDev->LogicToPixel(Size(nWidthLogic, 0), MapMode(MapUnit::Map10thMM)).Width();

        pCurrent->SetWidth(nForeignWidthPixel, GetZoom());
        if ( pBar )
            pBar->SetItemSize( pCurrent->GetId(), pCurrent->Width() );
    }

    // a smaller area for the content
    aRealPos.AdjustX( 1 );
    aRealPos.AdjustY( 1 );
    aRealSize.AdjustWidth( -2 );
    aRealSize.AdjustHeight( -2 );

    // let the header bar draw itself
    if ( pBar )
    {
        // the title height with respect to the font set for the given device
        long nTitleHeight = PixelToLogic(Size(0, GetTitleHeight()), MapMode(MapUnit::Map10thMM)).Height();
        nTitleHeight = pDev->LogicToPixel(Size(0, nTitleHeight), MapMode(MapUnit::Map10thMM)).Height();

        BrowserColumn* pFirstCol = !mvCols.empty() ? mvCols[ 0 ].get() : nullptr;

        Point aHeaderPos(pFirstCol && (pFirstCol->GetId() == 0) ? pFirstCol->Width() : 0, 0);
        Size aHeaderSize(aRealSize.Width() - aHeaderPos.X(), nTitleHeight);

        aHeaderPos += aRealPos;
            // do this before converting to logics !

        // the header's draw expects logic coordinates, again
        aHeaderPos = pDev->PixelToLogic(aHeaderPos);
        aHeaderSize = pDev->PixelToLogic(aHeaderSize);

        pBar->Draw(pDev, aHeaderPos, aHeaderSize, nFlags);

        // draw the "upper left cell" (the intersection between the header bar and the handle column)
        if (pFirstCol && (pFirstCol->GetId() == 0) && (pFirstCol->Width() > 4))
        {
            ButtonFrame aButtonFrame( aRealPos,
                Size( pFirstCol->Width()-1, nTitleHeight-1 ),
                pFirstCol->Title(), !IsEnabled());
            aButtonFrame.Draw( *pDev );

            pDev->Push( PushFlags::LINECOLOR );
            pDev->SetLineColor( COL_BLACK );

            pDev->DrawLine( Point( aRealPos.X(), aRealPos.Y() + nTitleHeight-1 ),
               Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() + nTitleHeight-1 ) );
            pDev->DrawLine( Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() ),
               Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() + nTitleHeight-1 ) );

            pDev->Pop();
        }

        aRealPos.AdjustY(aHeaderSize.Height() );
        aRealSize.AdjustHeight( -(aHeaderSize.Height()) );
    }

    // draw our own content (with clipping)
    vcl::Region aRegion(tools::Rectangle(aRealPos, aRealSize));
    pDev->SetClipRegion( pDev->PixelToLogic( aRegion ) );

    // do we have to paint the background
    bool bBackground = pDataWin->IsControlBackground();
    if ( bBackground )
    {
        tools::Rectangle aRect( aRealPos, aRealSize );
        pDev->SetFillColor( pDataWin->GetControlBackground() );
        pDev->DrawRect( aRect );
    }

    ImplPaintData( *pDev, tools::Rectangle( aRealPos, aRealSize ), true );

    // restore the column widths/data row height
    nDataRowHeight = nOriginalHeight;
    for ( nPos = 0; nPos < mvCols.size(); ++nPos )
    {
        BrowserColumn* pCurrent = mvCols[ nPos ].get();

        long nForeignWidthLogic = pDev->PixelToLogic(Size(pCurrent->Width(), 0), MapMode(MapUnit::Map10thMM)).Width();
        long nWidthPixel = LogicToPixel(Size(nForeignWidthLogic, 0), MapMode(MapUnit::Map10thMM)).Width();

        pCurrent->SetWidth(nWidthPixel, GetZoom());
        if ( pBar )
            pBar->SetItemSize( pCurrent->GetId(), pCurrent->Width() );
    }

    pDev->Pop();
}


void BrowseBox::ImplPaintData(OutputDevice& _rOut, const tools::Rectangle& _rRect, bool _bForeignDevice)
{
    Point aOverallAreaPos = _bForeignDevice ? _rRect.TopLeft() : Point(0,0);
    Size aOverallAreaSize = _bForeignDevice ? _rRect.GetSize() : pDataWin->GetOutputSizePixel();
    Point aOverallAreaBRPos = _bForeignDevice ? _rRect.BottomRight() : Point( aOverallAreaSize.Width(), aOverallAreaSize.Height() );

    long nDataRowHeigt = GetDataRowHeight();

    // compute relative rows to redraw
    sal_uLong nRelTopRow = 0;
    sal_uLong nRelBottomRow = aOverallAreaSize.Height();
    if (!_bForeignDevice && nDataRowHeigt)
    {
        nRelTopRow = (static_cast<sal_uLong>(_rRect.Top()) / nDataRowHeigt);
        nRelBottomRow = static_cast<sal_uLong>(_rRect.Bottom()) / nDataRowHeigt;
    }

    // cache frequently used values
    Point aPos( aOverallAreaPos.X(), nRelTopRow * nDataRowHeigt + aOverallAreaPos.Y() );
    _rOut.SetLineColor( COL_WHITE );
    const AllSettings& rAllSets = _rOut.GetSettings();
    const StyleSettings &rSettings = rAllSets.GetStyleSettings();
    const Color &rHighlightTextColor = rSettings.GetHighlightTextColor();
    const Color &rHighlightFillColor = rSettings.GetHighlightColor();
    Color aOldTextColor = _rOut.GetTextColor();
    Color aOldFillColor = _rOut.GetFillColor();
    Color aOldLineColor = _rOut.GetLineColor();
    long nHLineX = 0 == mvCols[ 0 ]->GetId() ? mvCols[ 0 ]->Width() : 0;
    nHLineX += aOverallAreaPos.X();

    Color aDelimiterLineColor( ::svtools::ColorConfig().GetColorValue( ::svtools::CALCGRID ).nColor );

    // redraw the invalid fields
    for ( sal_uLong nRelRow = nRelTopRow;
          nRelRow <= nRelBottomRow && static_cast<sal_uLong>(nTopRow)+nRelRow < static_cast<sal_uLong>(nRowCount);
          ++nRelRow, aPos.AdjustY(nDataRowHeigt ) )
    {
        // get row
        // check valid area, to be on the safe side:
        DBG_ASSERT( static_cast<sal_uInt16>(nTopRow+nRelRow) < nRowCount, "BrowseBox::ImplPaintData: invalid seek" );
        if ( (nTopRow+long(nRelRow)) < 0 || static_cast<sal_uInt16>(nTopRow+nRelRow) >= nRowCount )
            continue;

        // prepare row
        sal_uLong nRow = nTopRow+nRelRow;
        if ( !SeekRow( nRow) ) {
            OSL_FAIL("BrowseBox::ImplPaintData: SeekRow failed");
        }
        _rOut.SetClipRegion();
        aPos.setX( aOverallAreaPos.X() );


        // #73325# don't paint the row outside the painting rectangle (DG)
        // prepare auto-highlight
        tools::Rectangle aRowRect( Point( _rRect.TopLeft().X(), aPos.Y() ),
                Size( _rRect.GetSize().Width(), nDataRowHeigt ) );

        bool bRowSelected   =   !bHideSelect
                            &&  IsRowSelected( nRow );
        if ( bRowSelected )
        {
            _rOut.SetTextColor( rHighlightTextColor );
            _rOut.SetFillColor( rHighlightFillColor );
            _rOut.SetLineColor();
            _rOut.DrawRect( aRowRect );
        }

        // iterate through columns to redraw
        size_t nCol;
        for ( nCol = 0; nCol < mvCols.size(); ++nCol )
        {
            // get column
            BrowserColumn *pCol = mvCols[ nCol ].get();

            // at end of invalid area
            if ( aPos.X() >= _rRect.Right() )
                break;

            // skip invisible columns between frozen and scrollable area
            if ( nCol < nFirstCol && !pCol->IsFrozen() )
            {
                nCol = nFirstCol;
                pCol = (nCol < mvCols.size() ) ? mvCols[ nCol ].get() : nullptr;
                if (!pCol)
                {   // FS - 21.05.99 - 66325
                    // actually this has been fixed elsewhere (in the right place),
                    // but let's make sure...
                    OSL_FAIL("BrowseBox::PaintData : nFirstCol is probably invalid !");
                    break;
                }
            }

            // prepare Column-AutoHighlight
            bool bColAutoHighlight  =   bColumnCursor
                                    &&  IsColumnSelected( pCol->GetId() );
            if ( bColAutoHighlight )
            {
                _rOut.SetClipRegion();
                _rOut.SetTextColor( rHighlightTextColor );
                _rOut.SetFillColor( rHighlightFillColor );
                _rOut.SetLineColor();
                tools::Rectangle aFieldRect( aPos,
                        Size( pCol->Width(), nDataRowHeigt ) );
                _rOut.DrawRect( aFieldRect );
            }

            if (!m_bFocusOnlyCursor && (pCol->GetId() == GetCurColumnId()) && (nRow == static_cast<sal_uLong>(GetCurRow())))
                DrawCursor();

            // draw a single field.
            // else something is drawn to, e.g. handle column
            if (pCol->Width())
            {
                // clip the column's output to the field area
                if (_bForeignDevice)
                {   // (not necessary if painting onto the data window)
                    Size aFieldSize(pCol->Width(), nDataRowHeigt);

                    if (aPos.X() + aFieldSize.Width() > aOverallAreaBRPos.X())
                        aFieldSize.setWidth( aOverallAreaBRPos.X() - aPos.X() );

                    if (aPos.Y() + aFieldSize.Height() > aOverallAreaBRPos.Y() + 1)
                    {
                        // for non-handle cols we don't clip vertically : we just don't draw the cell if the line isn't completely visible
                        if (pCol->GetId() != 0)
                            continue;
                        aFieldSize.setHeight( aOverallAreaBRPos.Y() + 1 - aPos.Y() );
                    }

                    vcl::Region aClipToField(tools::Rectangle(aPos, aFieldSize));
                    _rOut.SetClipRegion(aClipToField);
                }
                pCol->Draw( *this, _rOut, aPos );
                if (_bForeignDevice)
                    _rOut.SetClipRegion();
            }

            // reset Column-auto-highlight
            if ( bColAutoHighlight )
            {
                _rOut.SetTextColor( aOldTextColor );
                _rOut.SetFillColor( aOldFillColor );
                _rOut.SetLineColor( aOldLineColor );
            }

            // skip column
            aPos.AdjustX(pCol->Width() );
        }

        // reset auto-highlight
        if ( bRowSelected )
        {
            _rOut.SetTextColor( aOldTextColor );
            _rOut.SetFillColor( aOldFillColor );
            _rOut.SetLineColor( aOldLineColor );
        }

        if ( bHLines )
        {
            // draw horizontal delimitation lines
            _rOut.SetClipRegion();
            _rOut.Push( PushFlags::LINECOLOR );
            _rOut.SetLineColor( aDelimiterLineColor );
            long nY = aPos.Y() + nDataRowHeigt - 1;
            if (nY <= aOverallAreaBRPos.Y())
                _rOut.DrawLine( Point( nHLineX, nY ),
                                Point( bVLines
                                        ? std::min(long(aPos.X() - 1), aOverallAreaBRPos.X())
                                        : aOverallAreaBRPos.X(),
                                      nY ) );
            _rOut.Pop();
        }
    }

    if (aPos.Y() > aOverallAreaBRPos.Y() + 1)
        aPos.setY( aOverallAreaBRPos.Y() + 1 );
        // needed for some of the following drawing

    // retouching
    _rOut.SetClipRegion();
    aOldLineColor = _rOut.GetLineColor();
    aOldFillColor = _rOut.GetFillColor();
    _rOut.SetFillColor( rSettings.GetFaceColor() );
    if ( !mvCols.empty() && ( mvCols[ 0 ]->GetId() == 0 ) && ( aPos.Y() <= _rRect.Bottom() ) )
    {
        // fill rectangle gray below handle column
        // DG: fill it only until the end of the drawing rect and not to the end, as this may overpaint handle columns
        _rOut.SetLineColor( COL_BLACK );
        _rOut.DrawRect( tools::Rectangle(
            Point( aOverallAreaPos.X() - 1, aPos.Y() - 1 ),
            Point( aOverallAreaPos.X() + mvCols[ 0 ]->Width() - 1,
                   _rRect.Bottom() + 1) ) );
    }
    _rOut.SetFillColor( aOldFillColor );

    // draw vertical delimitational line between frozen and scrollable cols
    _rOut.SetLineColor( COL_BLACK );
    long nFrozenWidth = GetFrozenWidth()-1;
    _rOut.DrawLine( Point( aOverallAreaPos.X() + nFrozenWidth, aPos.Y() ),
                   Point( aOverallAreaPos.X() + nFrozenWidth, bHLines
                            ? aPos.Y() - 1
                            : aOverallAreaBRPos.Y() ) );

    // draw vertical delimitational lines?
    if ( bVLines )
    {
        _rOut.SetLineColor( aDelimiterLineColor );
        Point aVertPos( aOverallAreaPos.X() - 1, aOverallAreaPos.Y() );
        long nDeltaY = aOverallAreaBRPos.Y();
        for ( size_t nCol = 0; nCol < mvCols.size(); ++nCol )
        {
            // get column
            BrowserColumn *pCol = mvCols[ nCol ].get();

            // skip invisible columns between frozen and scrollable area
            if ( nCol < nFirstCol && !pCol->IsFrozen() )
            {
                nCol = nFirstCol;
                pCol = mvCols[ nCol ].get();
            }

            // skip column
            aVertPos.AdjustX(pCol->Width() );

            // at end of invalid area
            // invalid area is first reached when X > Right
            // and not >=
            if ( aVertPos.X() > _rRect.Right() )
                break;

            // draw a single line
            if ( pCol->GetId() != 0 )
                _rOut.DrawLine( aVertPos, Point( aVertPos.X(),
                               bHLines
                                ? aPos.Y() - 1
                                : aPos.Y() + nDeltaY ) );
        }
    }

    _rOut.SetLineColor( aOldLineColor );
}

void BrowseBox::PaintData( vcl::Window const & rWin, vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect )
{
    if (!bBootstrapped && IsReallyVisible())
        BrowseBox::StateChanged(StateChangedType::InitShow);

    // initializations
    if (mvCols.empty() || !rWin.IsUpdateMode())
        return;
    if (pDataWin->bResizeOnPaint)
        Resize();
    // MI: who was that? Window::Update();

    ImplPaintData(rRenderContext, rRect, false);
}

long BrowseBox::GetBarHeight() const
{
    // tdf#115941 because some platforms have things like overlay scrollbars, take a max
    // of a statusbar height and a scrollbar height as the control area height

    // (we can't ask the scrollbars for their size cause if we're zoomed they still have to be
    // resized - which is done in UpdateScrollbars)

    return std::max(aStatusBar->GetSizePixel().Height(), GetSettings().GetStyleSettings().GetScrollBarSize());
}

void BrowseBox::UpdateScrollbars()
{

    if ( !bBootstrapped || !IsUpdateMode() )
        return;

    // protect against recursion
    if ( pDataWin->bInUpdateScrollbars )
    {
        pDataWin->bHadRecursion = true;
        return;
    }
    pDataWin->bInUpdateScrollbars = true;

    // the size of the corner window (and the width of the VSB/height of the HSB)
    sal_uLong nCornerSize = GetBarHeight();
    if (IsZoom())
        nCornerSize = static_cast<sal_uLong>(nCornerSize * static_cast<double>(GetZoom()));

    bool bNeedsVScroll = false;
    long nMaxRows = 0;
    if (GetDataRowHeight())
    {
        // needs VScroll?
        nMaxRows = (pDataWin->GetSizePixel().Height()) / GetDataRowHeight();
        bNeedsVScroll =    pDataWin->bAutoVScroll
                            ?   nTopRow || ( nRowCount > nMaxRows )
                            :   !pDataWin->bNoVScroll;
    }
    Size aDataWinSize = pDataWin->GetSizePixel();
    if ( !bNeedsVScroll )
    {
        if ( pVScroll->IsVisible() )
        {
            pVScroll->Hide();
            Size aNewSize( aDataWinSize );
            aNewSize.setWidth( GetOutputSizePixel().Width() );
            aDataWinSize = aNewSize;
        }
    }
    else if ( !pVScroll->IsVisible() )
    {
        Size aNewSize( aDataWinSize );
        aNewSize.setWidth( GetOutputSizePixel().Width() - nCornerSize );
        aDataWinSize = aNewSize;
    }

    // needs HScroll?
    sal_uLong nLastCol = GetColumnAtXPosPixel( aDataWinSize.Width() - 1 );

    sal_uInt16 nFrozenCols = FrozenColCount();
    bool bNeedsHScroll =    pDataWin->bAutoHScroll
                        ?   ( nFirstCol > nFrozenCols ) || ( nLastCol <= mvCols.size() )
                        :   !pDataWin->bNoHScroll;
    if ( !bNeedsHScroll )
    {
        if ( aHScroll->IsVisible() )
        {
            aHScroll->Hide();
        }
        aDataWinSize.setHeight( GetOutputSizePixel().Height() - GetTitleHeight() );
        if ( nControlAreaWidth != USHRT_MAX )
            aDataWinSize.AdjustHeight( -sal_Int32(nCornerSize) );
    }
    else if ( !aHScroll->IsVisible() )
    {
        Size aNewSize( aDataWinSize );
        aNewSize.setHeight( GetOutputSizePixel().Height() - GetTitleHeight() - nCornerSize );
        aDataWinSize = aNewSize;
    }

    // adjust position and Width of horizontal scrollbar
    sal_uLong nHScrX = nControlAreaWidth == USHRT_MAX
        ? 0
        : nControlAreaWidth;

    aHScroll->SetPosSizePixel(
        Point( nHScrX, GetOutputSizePixel().Height() - nCornerSize ),
        Size( aDataWinSize.Width() - nHScrX, nCornerSize ) );

    // total scrollable columns
    short nScrollCols = short(mvCols.size()) - static_cast<short>(nFrozenCols);

    // visible columns
    short nVisibleHSize = nLastCol == BROWSER_INVALIDID
        ? static_cast<short>( mvCols.size() - nFirstCol )
        : static_cast<short>( nLastCol - nFirstCol );

    short nRange = std::max( nScrollCols, short(0) );
    aHScroll->SetVisibleSize( nVisibleHSize );
    aHScroll->SetRange( Range( 0, nRange ));
    if ( bNeedsHScroll && !aHScroll->IsVisible() )
        aHScroll->Show();

    // adjust position and height of vertical scrollbar
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

    // needs corner-window?
    // (do that AFTER positioning BOTH scrollbars)
    sal_uLong nActualCorderWidth = 0;
    if (aHScroll->IsVisible() && pVScroll && pVScroll->IsVisible() )
    {
        // if we have both scrollbars, the corner window fills the point of intersection of these two
        nActualCorderWidth = nCornerSize;
    }
    else if ( !aHScroll->IsVisible() && ( nControlAreaWidth != USHRT_MAX ) )
    {
        // if we have no horizontal scrollbar, but a control area, we need the corner window to
        // fill the space between the control are and the right border
        nActualCorderWidth = GetOutputSizePixel().Width() - nControlAreaWidth;
    }
    if ( nActualCorderWidth )
    {
        if ( !pDataWin->pCornerWin )
            pDataWin->pCornerWin = VclPtr<ScrollBarBox>::Create( this, 0 );
        pDataWin->pCornerWin->SetPosSizePixel(
            Point( GetOutputSizePixel().Width() - nActualCorderWidth, aHScroll->GetPosPixel().Y() ),
            Size( nActualCorderWidth, nCornerSize ) );
        pDataWin->pCornerWin->Show();
    }
    else
        pDataWin->pCornerWin.disposeAndClear();

    // scroll headerbar, if necessary
    if ( pDataWin->pHeaderBar )
    {
        long nWidth = 0;
        for ( size_t nCol = 0;
              nCol < mvCols.size() && nCol < nFirstCol;
              ++nCol )
        {
            // not the handle column
            if ( mvCols[ nCol ]->GetId() )
                nWidth += mvCols[ nCol ]->Width();
        }

        pDataWin->pHeaderBar->SetOffset( nWidth );
    }

    pDataWin->bInUpdateScrollbars = false;
    if ( pDataWin->bHadRecursion )
    {
        pDataWin->bHadRecursion = false;
        UpdateScrollbars();
    }
}


void BrowseBox::SetUpdateMode( bool bUpdate )
{

    bool bWasUpdate = IsUpdateMode();
    if ( bWasUpdate == bUpdate )
        return;

    Control::SetUpdateMode( bUpdate );
    // If WB_CLIPCHILDREN is st at the BrowseBox (to minimize flicker),
    // the data window is not invalidated by SetUpdateMode.
    if( bUpdate )
        pDataWin->Invalidate();
    pDataWin->SetUpdateMode( bUpdate );


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


bool BrowseBox::GetUpdateMode() const
{

    return pDataWin->IsUpdateMode();
}


long BrowseBox::GetFrozenWidth() const
{

    long nWidth = 0;
    for ( size_t nCol = 0;
          nCol < mvCols.size() && mvCols[ nCol ]->IsFrozen();
          ++nCol )
        nWidth += mvCols[ nCol ]->Width();
    return nWidth;
}


void BrowseBox::ColumnInserted( sal_uInt16 nPos )
{
    if ( pColSel )
        pColSel->Insert( nPos );
    UpdateScrollbars();
}


sal_uInt16 BrowseBox::FrozenColCount() const
{
    std::size_t nCol;
    for ( nCol = 0;
          nCol < mvCols.size() && mvCols[ nCol ]->IsFrozen();
          ++nCol )
        /* empty loop */;
    return nCol; //TODO: BrowserColumns::size_type -> sal_uInt16!
}


IMPL_LINK(BrowseBox, ScrollHdl, ScrollBar*, pBar, void)
{
    if ( pBar->GetDelta() == 0 )
        return;

    if ( pBar == aHScroll.get() )
        ScrollColumns( aHScroll->GetDelta() );
    if ( pBar == pVScroll )
        ScrollRows( pVScroll->GetDelta() );
}


IMPL_LINK( BrowseBox, StartDragHdl, HeaderBar*, pBar, void )
{
    pBar->SetDragSize( pDataWin->GetOutputSizePixel().Height() );
}


// usually only the first column was resized

void BrowseBox::MouseButtonDown( const MouseEvent& rEvt )
{

    GrabFocus();

    // only mouse events in the title-line are supported
    const Point &rEvtPos = rEvt.GetPosPixel();
    if ( rEvtPos.Y() >= GetTitleHeight() )
        return;

    long nX = 0;
    long nWidth = GetOutputSizePixel().Width();
    for ( size_t nCol = 0; nCol < mvCols.size() && nX < nWidth; ++nCol )
    {
        // is this column visible?
        BrowserColumn *pCol = mvCols[ nCol ].get();
        if ( pCol->IsFrozen() || nCol >= nFirstCol )
        {
            // compute right end of column
            long nR = nX + pCol->Width() - 1;

            // at the end of a column (and not handle column)?
            if ( pCol->GetId() && std::abs( nR - rEvtPos.X() ) < 2 )
            {
                // start resizing the column
                bResizing = true;
                nResizeCol = nCol;
                nDragX = nResizeX = rEvtPos.X();
                SetPointer( PointerStyle::HSplit );
                CaptureMouse();
                pDataWin->DrawLine( Point( nDragX, 0 ),
                    Point( nDragX, pDataWin->GetSizePixel().Height() ) );
                nMinResizeX = nX + MIN_COLUMNWIDTH;
                return;
            }
            else if ( nX < rEvtPos.X() && nR > rEvtPos.X() )
            {
                MouseButtonDown( BrowserMouseEvent(
                    this, rEvt, -1, nCol, pCol->GetId(), tools::Rectangle() ) );
                return;
            }
            nX = nR + 1;
        }
    }

    // event occurred out of data area
    if ( rEvt.IsRight() )
        pDataWin->Command(
            CommandEvent( Point( 1, LONG_MAX ), CommandEventId::ContextMenu, true ) );
    else
        SetNoSelection();
}


void BrowseBox::MouseMove( const MouseEvent& rEvt )
{
    SAL_INFO("svtools", "BrowseBox::MouseMove( MouseEvent )" );

    PointerStyle aNewPointer = PointerStyle::Arrow;

    sal_uInt16 nX = 0;
    for ( size_t nCol = 0;
          nCol < mvCols.size() &&
            ( nX + mvCols[ nCol ]->Width() ) < sal_uInt16(GetOutputSizePixel().Width());
          ++nCol )
        // is this column visible?
        if ( mvCols[ nCol ]->IsFrozen() || nCol >= nFirstCol )
        {
            // compute right end of column
            BrowserColumn *pCol = mvCols[ nCol ].get();
            sal_uInt16 nR = static_cast<sal_uInt16>(nX + pCol->Width() - 1);

            // show resize-pointer?
            if ( bResizing || ( pCol->GetId() &&
                 std::abs( static_cast<long>(nR) - rEvt.GetPosPixel().X() ) < MIN_COLUMNWIDTH ) )
            {
                aNewPointer = PointerStyle::HSplit;
                if ( bResizing )
                {
                    // delete old auxiliary line
                    pDataWin->HideTracking() ;

                    // check allowed width and new delta
                    nDragX = std::max( rEvt.GetPosPixel().X(), nMinResizeX );
                    long nDeltaX = nDragX - nResizeX;
                    sal_uInt16 nId = GetColumnId(nResizeCol);
                    sal_uLong nOldWidth = GetColumnWidth(nId);
                    nDragX = nOldWidth + nDeltaX + nResizeX - nOldWidth;

                    // draw new auxiliary line
                    pDataWin->ShowTracking( tools::Rectangle( Point( nDragX, 0 ),
                            Size( 1, pDataWin->GetSizePixel().Height() ) ),
                            ShowTrackFlags::Split|ShowTrackFlags::TrackWindow );
                }

            }

            nX = nR + 1;
        }

    SetPointer( aNewPointer );
}


void BrowseBox::MouseButtonUp( const MouseEvent & rEvt )
{

    if ( bResizing )
    {
        // delete auxiliary line
        pDataWin->HideTracking();

        // width changed?
        nDragX = std::max( rEvt.GetPosPixel().X(), nMinResizeX );
        if ( (nDragX - nResizeX) != static_cast<long>(mvCols[ nResizeCol ]->Width()) )
        {
            // resize column
            long nMaxX = pDataWin->GetSizePixel().Width();
            nDragX = std::min( nDragX, nMaxX );
            long nDeltaX = nDragX - nResizeX;
            sal_uInt16 nId = GetColumnId(nResizeCol);
            SetColumnWidth( GetColumnId(nResizeCol), GetColumnWidth(nId) + nDeltaX );
            ColumnResized( nId );
        }

        // end action
        SetPointer( PointerStyle::Arrow );
        ReleaseMouse();
        bResizing = false;
    }
    else
        MouseButtonUp( BrowserMouseEvent( pDataWin,
                MouseEvent( Point( rEvt.GetPosPixel().X(),
                        rEvt.GetPosPixel().Y() - pDataWin->GetPosPixel().Y() ),
                    rEvt.GetClicks(), rEvt.GetMode(), rEvt.GetButtons(),
                    rEvt.GetModifier() ) ) );
}


static bool bExtendedMode = false;
static bool bFieldMode = false;

void BrowseBox::MouseButtonDown( const BrowserMouseEvent& rEvt )
{

    GrabFocus();

    // adjust selection while and after double-click
    if ( rEvt.GetClicks() == 2 )
    {
        SetNoSelection();
        if ( rEvt.GetRow() >= 0 )
        {
            GoToRow( rEvt.GetRow() );
            SelectRow( rEvt.GetRow(), true, false );
        }
        else
        {
            if ( bColumnCursor && rEvt.GetColumn() != 0 )
            {
                if ( rEvt.GetColumn() < mvCols.size() )
                    SelectColumnPos( rEvt.GetColumn(), true, false);
            }
        }
        DoubleClick( rEvt );
    }
    // selections
    else if ( ( rEvt.GetMode() & ( MouseEventModifiers::SELECT | MouseEventModifiers::SIMPLECLICK ) ) &&
         ( bColumnCursor || rEvt.GetRow() >= 0 ) )
    {
        if ( rEvt.GetClicks() == 1 )
        {
            // initialise flags
            bHit            = false;

            // selection out of range?
            if ( rEvt.GetRow() >= nRowCount ||
                 rEvt.GetColumnId() == BROWSER_INVALIDID )
            {
                SetNoSelection();
                return;
            }

            // while selecting, no cursor
            bSelecting = true;
            DoHideCursor( "MouseButtonDown" );

            // DataRow?
            if ( rEvt.GetRow() >= 0 )
            {
                // line selection?
                if ( rEvt.GetColumnId() == HandleColumnId || !bColumnCursor )
                {
                    if ( bMultiSelection )
                    {
                        // remove column-selection, if exists
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

                        // expanding mode?
                        if ( rEvt.GetMode() & MouseEventModifiers::RANGESELECT )
                        {
                            // select the further touched rows too
                            bSelect = true;
                            ExpandRowSelection( rEvt );
                            return;
                        }

                        // click in the selected area?
                        else if ( IsRowSelected( rEvt.GetRow() ) )
                        {
                            // wait for Drag&Drop
                            bHit = true;
                            bExtendedMode = bool( rEvt.GetMode() & MouseEventModifiers::MULTISELECT );
                            return;
                        }

                        // extension mode?
                        else if ( rEvt.GetMode() & MouseEventModifiers::MULTISELECT )
                        {
                            // determine the new selection range
                            // and selection/deselection
                            aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
                            SelectRow( rEvt.GetRow(),
                                    !uRow.pSel->IsSelected( rEvt.GetRow() ) );
                            bSelect = true;
                            return;
                        }
                    }

                    // select directly
                    SetNoSelection();
                    GoToRow( rEvt.GetRow() );
                    SelectRow( rEvt.GetRow() );
                    aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
                    bSelect = true;
                }
                else // Column/Field-Selection
                {
                    // click in selected column
                    if ( IsColumnSelected( rEvt.GetColumn() ) ||
                         IsRowSelected( rEvt.GetRow() ) )
                    {
                        bHit = true;
                        bFieldMode = true;
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
                    // toggle all-selection
                    if ( uRow.pSel->GetSelectCount() > ( GetRowCount() / 2 ) )
                        SetNoSelection();
                    else
                        SelectAll();
                }
                else
                    SelectColumnPos( GetColumnPos(rEvt.GetColumnId()), true, false);
            }

            // turn cursor on again, if necessary
            bSelecting = false;
            DoShowCursor( "MouseButtonDown" );
            if ( bSelect )
                Select();
        }
    }
}


void BrowseBox::MouseButtonUp( const BrowserMouseEvent &rEvt )
{

    // D&D was possible, but did not occur
    if ( bHit )
    {
        aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
        if ( bExtendedMode )
            SelectRow( rEvt.GetRow(), false );
        else
        {
            SetNoSelection();
            if ( bFieldMode )
                GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
            else
            {
                GoToRow( rEvt.GetRow() );
                SelectRow( rEvt.GetRow() );
            }
        }
        bSelect = true;
        bExtendedMode = false;
        bFieldMode = false;
        bHit = false;
    }

    // activate cursor
    if ( bSelecting )
    {
        bSelecting = false;
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


bool BrowseBox::ProcessKey( const KeyEvent& rEvt )
{

    sal_uInt16 nCode = rEvt.GetKeyCode().GetCode();
    bool       bShift = rEvt.GetKeyCode().IsShift();
    bool       bCtrl = rEvt.GetKeyCode().IsMod1();
    bool       bAlt = rEvt.GetKeyCode().IsMod2();

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
                [[fallthrough]];
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

    long nRowsOnPage = pDataWin->GetSizePixel().Height() / GetDataRowHeight();

    switch ( nId )
    {
        case BROWSER_SELECTCOLUMN:
            if ( ColCount() )
                SelectColumnId( GetCurColumnId() );
            break;

        case BROWSER_CURSORDOWN:
            if ( ( GetCurRow() + 1 ) < nRowCount )
                GoToRow( GetCurRow() + 1, false );
            break;
        case BROWSER_CURSORUP:
            if ( GetCurRow() > 0 )
                GoToRow( GetCurRow() - 1, false );
            break;
        case BROWSER_SELECTHOME:
            if ( GetRowCount() )
            {
                DoHideCursor( "BROWSER_SELECTHOME" );
                for ( long nRow = GetCurRow(); nRow >= 0; --nRow )
                    SelectRow( nRow );
                GoToRow( 0, true );
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
                GoToRow( GetRowCount() - 1, true );
                DoShowCursor( "BROWSER_SELECTEND" );
            }
            break;
        case BROWSER_SELECTDOWN:
        {
            if ( GetRowCount() && ( GetCurRow() + 1 ) < nRowCount )
            {
                // deselect the current row, if it isn't the first
                // and there is no other selected row above
                long nRow = GetCurRow();
                bool bLocalSelect = ( !IsRowSelected( nRow ) ||
                                 GetSelectRowCount() == 1 || IsRowSelected( nRow - 1 ) );
                SelectRow( nRow, bLocalSelect );
                bool bDone = GoToRow( GetCurRow() + 1, false );
                if ( bDone )
                    SelectRow( GetCurRow() );
            }
            else
                ScrollRows( 1 );
            break;
        }
        case BROWSER_SELECTUP:
            if ( GetRowCount() )
            {
                // deselect the current row, if it isn't the first
                // and there is no other selected row under
                long nRow = GetCurRow();
                bool bLocalSelect = ( !IsRowSelected( nRow ) ||
                                 GetSelectRowCount() == 1 || IsRowSelected( nRow + 1 ) );
                SelectRow( nCurRow, bLocalSelect );
                bool bDone = GoToRow( nRow - 1, false );
                if ( bDone )
                    SelectRow( GetCurRow() );
            }
            break;
        case BROWSER_CURSORPAGEDOWN:
            ScrollRows( nRowsOnPage );
            break;
        case BROWSER_CURSORPAGEUP:
            ScrollRows( -nRowsOnPage );
            break;
        case BROWSER_CURSOREND:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewId = GetColumnId(ColCount() -1);
                nNewId != HandleColumnId && GoToColumnId( nNewId );
                break;
            }
            [[fallthrough]];
        case BROWSER_CURSORENDOFFILE:
            GoToRow( nRowCount - 1, false );
            break;
        case BROWSER_CURSORRIGHT:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewPos = GetColumnPos( GetCurColumnId() ) + 1;
                sal_uInt16 nNewId = GetColumnId( nNewPos );
                if (nNewId != BROWSER_INVALIDID)    // At end of row ?
                    GoToColumnId( nNewId );
                else
                {
                    sal_uInt16 nColId = GetColumnId(0);
                    if ( nColId == BROWSER_INVALIDID || nColId == HandleColumnId )
                        nColId = GetColumnId(1);
                    if ( GetRowCount() )
                    {
                        if ( nCurRow < GetRowCount() - 1 )
                        {
                            GoToRowColumnId( nCurRow + 1, nColId );
                        }
                    }
                    else if ( ColCount() )
                        GoToColumnId( nColId );
                }
            }
            else
                ScrollColumns( 1 );
            break;
        case BROWSER_CURSORHOME:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewId = GetColumnId(1);
                if (nNewId != HandleColumnId)
                {
                    GoToColumnId( nNewId );
                }
                break;
            }
            [[fallthrough]];
        case BROWSER_CURSORTOPOFFILE:
            GoToRow( 0, false );
            break;
        case BROWSER_CURSORLEFT:
            if ( bColumnCursor )
            {
                sal_uInt16 nNewPos = GetColumnPos( GetCurColumnId() ) - 1;
                sal_uInt16 nNewId = GetColumnId( nNewPos );
                if (nNewId != HandleColumnId)
                    GoToColumnId( nNewId );
                else
                {
                    if ( GetRowCount() )
                    {
                        if (nCurRow > 0)
                        {
                            GoToRowColumnId(nCurRow - 1, GetColumnId(ColCount() -1));
                        }
                    }
                    else if ( ColCount() )
                        GoToColumnId( GetColumnId(ColCount() -1) );
                }
            }
            else
                ScrollColumns( -1 );
            break;
        case BROWSER_ENHANCESELECTION:
            if ( GetRowCount() )
                SelectRow( GetCurRow(), !IsRowSelected( GetCurRow() ) );
            break;
        case BROWSER_SELECT:
            if ( GetRowCount() )
                SelectRow( GetCurRow(), !IsRowSelected( GetCurRow() ), false );
            break;
        case BROWSER_MOVECOLUMNLEFT:
        case BROWSER_MOVECOLUMNRIGHT:
            { // check if column moving is allowed
                BrowserHeader* pHeaderBar = pDataWin->pHeaderBar;
                if ( pHeaderBar && pHeaderBar->IsDragable() )
                {
                    sal_uInt16 nColId = GetCurColumnId();
                    bool bColumnSelected = IsColumnSelected(nColId);
                    sal_uInt16 nNewPos = GetColumnPos(nColId);
                    bool bMoveAllowed = false;
                    if ( BROWSER_MOVECOLUMNLEFT == nId && nNewPos > 1 )
                    {
                        --nNewPos;
                        bMoveAllowed = true;
                    }
                    else if ( BROWSER_MOVECOLUMNRIGHT == nId && nNewPos < (ColCount()-1) )
                    {
                        ++nNewPos;
                        bMoveAllowed = true;
                    }

                    if ( bMoveAllowed )
                    {
                        SetColumnPos( nColId, nNewPos );
                        ColumnMoved( nColId );
                        MakeFieldVisible(GetCurRow(), nColId);
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

    // ensure the cursor is hidden
    DoHideCursor("SetCursorColor");
    if (!m_bFocusOnlyCursor)
        DoHideCursor("SetCursorColor - force");

    m_aCursorColor = _rCol;

    if (!m_bFocusOnlyCursor)
        DoShowCursor("SetCursorColor - force");
    DoShowCursor("SetCursorColor");
}

tools::Rectangle BrowseBox::calcHeaderRect(bool _bIsColumnBar, bool _bOnScreen)
{
    vcl::Window* pParent = nullptr;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    Point aTopLeft;
    long nWidth;
    long nHeight;
    if ( _bIsColumnBar )
    {
        nWidth = pDataWin->GetOutputSizePixel().Width();
        nHeight = GetDataRowHeight();
    }
    else
    {
        aTopLeft.setY( GetDataRowHeight() );
        nWidth = GetColumnWidth(0);
        nHeight = GetWindowExtentsRelative( pParent ).GetHeight() - aTopLeft.Y() - GetControlArea().GetSize().Height();
    }
    aTopLeft += GetWindowExtentsRelative( pParent ).TopLeft();
    return tools::Rectangle(aTopLeft,Size(nWidth,nHeight));
}

tools::Rectangle BrowseBox::calcTableRect(bool _bOnScreen)
{
    vcl::Window* pParent = nullptr;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    tools::Rectangle aRect( GetWindowExtentsRelative( pParent ) );
    tools::Rectangle aRowBar = calcHeaderRect(false, pParent == nullptr);

    long nX = aRowBar.Right() - aRect.Left();
    long nY = aRowBar.Top() - aRect.Top();
    Size aSize(aRect.GetSize());

    return tools::Rectangle(aRowBar.TopRight(), Size(aSize.Width() - nX, aSize.Height() - nY - GetBarHeight()) );
}

tools::Rectangle BrowseBox::GetFieldRectPixelAbs( sal_Int32 _nRowId, sal_uInt16 _nColId, bool /*_bIsHeader*/, bool _bOnScreen )
{
    vcl::Window* pParent = nullptr;
    if ( !_bOnScreen )
        pParent = GetAccessibleParentWindow();

    tools::Rectangle aRect = GetFieldRectPixel(_nRowId,_nColId,_bOnScreen);

    Point aTopLeft = aRect.TopLeft();
    aTopLeft += GetWindowExtentsRelative( pParent ).TopLeft();

    return tools::Rectangle(aTopLeft,aRect.GetSize());
}

// ------------------------------------------------------------------------- EOF

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
