/*************************************************************************
 *
 *  $RCSfile: brwbox2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:56 $
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

#include <tools/debug.hxx>
#include <brwbox.hxx>
#include "datwin.hxx"

#ifndef _SV_SALGTYPE_HXX
#include <vcl/salgtype.hxx>
#endif

#pragma hdrstop

#ifndef _SV_MULTISEL_HXX
#include <tools/multisel.hxx>
#endif

#if SUPD<558
BOOL            BrowseBox::m_bFocusOnlyCursor;
Color           BrowseBox::m_aCursorColor;
BrowserMode     BrowseBox::m_nCurrentMode;
#endif


//===================================================================

DBG_NAMEEX(BrowseBox);

//===================================================================

extern const char* BrowseBoxCheckInvariants( const void * pVoid );

DECLARE_LIST( BrowserColumns, BrowserColumn* );

//===================================================================

void BrowseBox::Command( const CommandEvent& rEvt )
{
    if ( !((BrowserDataWin*)pDataWin)->bInCommand )
        Control::Command( rEvt );
}

//===================================================================

BOOL BrowseBox::IsInCommandEvent() const
{
    return ((BrowserDataWin*)pDataWin)->bInCommand;
}

//===================================================================

void BrowseBox::StateChanged( StateChangedType nStateChange )
{
    if ( STATE_CHANGE_INITSHOW == nStateChange )
    {
        bBootstrapped = TRUE; // muss zuerst gesetzt werden!

        Resize();
        if ( bMultiSelection )
            uRow.pSel->SetTotalRange( Range( 0, nRowCount - 1 ) );
        if ( nRowCount == 0 )
            nCurRow = BROWSER_ENDOFSELECTION;
        else if ( nCurRow == BROWSER_ENDOFSELECTION )
            nCurRow = 0;


        if ( HasFocus() )
        {
            bSelectionIsVisible = TRUE;
            bHasFocus = TRUE;
        }
        UpdateScrollbars();
        AutoSizeLastColumn();
        CursorMoved();
    }
    else if (STATE_CHANGE_ZOOM == nStateChange)
    {
        pDataWin->SetZoom(GetZoom());
        HeaderBar* pHeaderBar = ( (BrowserDataWin*)pDataWin )->pHeaderBar;
        if (pHeaderBar)
            pHeaderBar->SetZoom(GetZoom());

        // let the cols calc their new widths and adjust the header bar
        for ( USHORT nPos = 0; nPos < pCols->Count(); ++nPos )
        {
            pCols->GetObject(nPos)->ZoomChanged(GetZoom());
            if ( pHeaderBar )
                pHeaderBar->SetItemSize( pCols->GetObject(nPos)->GetId(), pCols->GetObject(nPos)->Width() );
        }

        // all our controls have to be repositioned
        Resize();
    }
}

//===================================================================

void BrowseBox::Select()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

void BrowseBox::DoubleClick( const BrowserMouseEvent & )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

void BrowseBox::CursorMoved()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

long BrowseBox::QueryColumnResize( USHORT nId, long nWidth )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    return nWidth;
}

//-------------------------------------------------------------------

void BrowseBox::ColumnResized( USHORT nId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

void BrowseBox::ColumnMoved( USHORT nId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

void BrowseBox::StartScroll()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    //((Control*)pDataWin)->HideFocus();
    DoHideCursor( "StartScroll" );
}

//-------------------------------------------------------------------

void BrowseBox::EndScroll()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    UpdateScrollbars();
    AutoSizeLastColumn();
    DoShowCursor( "EndScroll" );
}

//-------------------------------------------------------------------

#pragma optimize( "", off )

void BrowseBox::ToggleSelection( BOOL bForce )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // selection highlight-toggling allowed?
    if ( bHideSelect )
        return;
    if ( !bForce &&
         ( bNotToggleSel || !IsUpdateMode() || !bSelectionIsVisible ) )
        return;
//MI, 28.01.98
//  if ( !((BrowserDataWin*)pDataWin)->bHighlightToggle &&
//       !((BrowserDataWin*)pDataWin)->bHighlightAuto )
//      return;

    // only highlight painted areas!
    bNotToggleSel = TRUE;
    if ( FALSE && !((BrowserDataWin*)pDataWin)->bInPaint )
        pDataWin->Update();

    // accumulate areas of rows to highlight
    RectangleList aHighlightList;
    long nLastRowInRect = 0; // fuer den CFront

    // Handle-Column nicht highlighten
    BrowserColumn *pFirstCol = pCols->GetObject(0);
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

        Rectangle aAddRect(
            Point( nOfsX, (nRow-nTopRow)*GetDataRowHeight() ),
            Size( pDataWin->GetSizePixel().Width(), GetDataRowHeight() ) );
        if ( aHighlightList.Count() && nLastRowInRect == ( nRow - 1 ) )
            aHighlightList.First()->Union( aAddRect );
        else
            aHighlightList.Insert( new Rectangle( aAddRect ), (ULONG) 0 );
        nLastRowInRect = nRow;
    }

    // unhighlight the old selection (if any)
    while ( aHighlightList.Count() )
    {
        Rectangle *pRect = aHighlightList.Remove( aHighlightList.Count() - 1 );
        pDataWin->Invalidate( *pRect );
        delete pRect;
    }

    // unhighlight old column selection (if any)
    for ( long nColId = pColSel ? pColSel->FirstSelected() : BROWSER_ENDOFSELECTION;
          nColId != BROWSER_ENDOFSELECTION;
          nColId = pColSel->NextSelected() )
    {
        Rectangle aRect( GetFieldRectPixel(nCurRow,
                                           pCols->GetObject(nColId)->GetId(),
                                           FALSE ) );
        aRect.Left() -= MIN_COLUMNWIDTH;
        aRect.Right() += MIN_COLUMNWIDTH;
        aRect.Top() = 0;
        aRect.Bottom() = pDataWin->GetOutputSizePixel().Height();
        pDataWin->Invalidate( aRect );
    }

    bNotToggleSel = FALSE;
}

#pragma optimize( "", on )

//-------------------------------------------------------------------

void BrowseBox::DrawCursor()
{
    short nCursorHideCount = GetCursorHideCount();
    BOOL bHidden = bHideCursor || !bSelectionIsVisible ||
                   !IsUpdateMode() || bScrolling || nCurRow < 0;

    if (PaintCursorIfHiddenOnce())
        bHidden |= ( GetCursorHideCount() > 1 );
    else
        bHidden |= ( GetCursorHideCount() > 0 );
//  bHidden |= ( GetCursorHideCount() > 0 ) && !( ( !m_bFocusOnlyCursor && GetCursorHideCount() == 1 ) && !HasFocus() );
        // hidden if the hide count non-zero
        // exception : we hided the cursor exactly once in LoseFocus and we have to show the cursor

    // keine Cursor auf Handle-Column
    if ( nCurColId == 0 )
        nCurColId = GetColumnId(1);

    // Cursor-Rechteck berechnen
    Rectangle aCursor;
    if ( bColumnCursor )
    {
        aCursor = GetFieldRectPixel( nCurRow, nCurColId, FALSE );
        //! --aCursor.Bottom();
        aCursor.Left() -= MIN_COLUMNWIDTH;
        aCursor.Right() += 1;
        aCursor.Bottom() += 1;
    }
    else
        aCursor = Rectangle(
            Point( ( pCols->Count() && pCols->GetObject(0)->GetId() == 0 ) ?
                        pCols->GetObject(0)->Width() : 0,
                        (nCurRow - nTopRow) * GetDataRowHeight() + 1 ),
            Size( pDataWin->GetOutputSizePixel().Width() + 1,
                  GetDataRowHeight() - 2 ) );
    if ( bHLines )
    {
        if ( !bMultiSelection )
            --aCursor.Top();
        --aCursor.Bottom();
    }

    //!mi_mac pDataWin->Update();

    if (m_aCursorColor == COL_TRANSPARENT)
    {
        // auf diesem Plattformen funktioniert der StarView-Focus richtig
        if ( bHidden )
            ((Control*)pDataWin)->HideFocus();
        else
            ((Control*)pDataWin)->ShowFocus( aCursor );
    }
    else
    {
        Color rCol = bHidden ? pDataWin->GetFillColor() : m_aCursorColor;
        Color aOldFillColor = pDataWin->GetFillColor();
        Color aOldLineColor = pDataWin->GetLineColor();
        pDataWin->SetFillColor();
        pDataWin->SetLineColor( rCol );
        pDataWin->DrawRect( aCursor );
        pDataWin->SetLineColor( aOldLineColor );
        pDataWin->SetFillColor( aOldFillColor );
    }
}

//-------------------------------------------------------------------

ULONG BrowseBox::GetColumnWidth( USHORT nId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    USHORT nItemPos = GetColumnPos( nId );
    if ( nItemPos >= pCols->Count() )
        return 0;
    return pCols->GetObject(nItemPos)->Width();
}

//-------------------------------------------------------------------

USHORT BrowseBox::GetColumnId( USHORT nPos ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( nPos >= pCols->Count() )
        return 0;
    return pCols->GetObject(nPos)->GetId();
}

//-------------------------------------------------------------------

USHORT BrowseBox::GetColumnPos( USHORT nId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    for ( USHORT nPos = 0; nPos < pCols->Count(); ++nPos )
        if ( pCols->GetObject(nPos)->GetId() == nId )
            return nPos;
    return BROWSER_INVALIDID;
}

//-------------------------------------------------------------------

BOOL BrowseBox::IsFrozen( USHORT nColumnId ) const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    for ( USHORT nPos = 0; nPos < pCols->Count(); ++nPos )
        if ( pCols->GetObject(nPos)->GetId() == nColumnId )
            return pCols->GetObject(nPos)->IsFrozen();
    return FALSE;
}

//-------------------------------------------------------------------

void BrowseBox::ExpandRowSelection( const BrowserMouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    DoHideCursor( "ExpandRowSelection" );

    // expand the last selection
    if ( bMultiSelection )
    {
        Range aJustifiedRange( aSelRange );
        aJustifiedRange.Justify();

        BOOL bSelectThis =
            ( bSelect && !aJustifiedRange.IsInside( rEvt.GetRow() ) ) ||
            ( !bSelect && aJustifiedRange.IsInside( rEvt.GetRow() ) );

        if ( aJustifiedRange.IsInside( rEvt.GetRow() ) )
        {
            // down and up
            while ( rEvt.GetRow() < aSelRange.Max() )
            {   // ZTC/Mac bug - dont put these statemants together!
                SelectRow( aSelRange.Max(), bSelectThis, TRUE );
                --aSelRange.Max();
            }
            while ( rEvt.GetRow() > aSelRange.Max() )
            {   // ZTC/Mac bug - dont put these statemants together!
                SelectRow( aSelRange.Max(), bSelectThis, TRUE );
                ++aSelRange.Max();
            }
        }
        else
        {
            // up and down
            BOOL bOldSelecting = bSelecting;
            bSelecting = TRUE;
            while ( rEvt.GetRow() < aSelRange.Max() )
            {   // ZTC/Mac bug - dont put these statemants together!
                --aSelRange.Max();
                if ( !IsRowSelected( aSelRange.Max() ) )
                {
                    SelectRow( aSelRange.Max(), bSelectThis, TRUE );
                    bSelect = TRUE;
                }
            }
            while ( rEvt.GetRow() > aSelRange.Max() )
            {   // ZTC/Mac bug - dont put these statemants together!
                ++aSelRange.Max();
                if ( !IsRowSelected( aSelRange.Max() ) )
                {
                    SelectRow( aSelRange.Max(), bSelectThis, TRUE );
                    bSelect = TRUE;
                }
            }
            bSelecting = bOldSelecting;
            if ( bSelect )
                Select();
        }
    }
    else
        if ( !bMultiSelection || !IsRowSelected( rEvt.GetRow() ) )
            SelectRow( rEvt.GetRow(), TRUE );

    GoToRow( rEvt.GetRow(), FALSE );
    DoShowCursor( "ExpandRowSelection" );
}

//-------------------------------------------------------------------

void BrowseBox::Resize()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( STATE_CHANGE_INITSHOW );
    if ( !pCols->Count() )
    {
        ((BrowserDataWin*)pDataWin)->bResizeOnPaint = TRUE;
        return;
    }
    ((BrowserDataWin*)pDataWin)->bResizeOnPaint = FALSE;

    // calc the size of the scrollbars
    // (we can't ask the scrollbars for their widths cause if we're zoomed they still have to be
    // resized - which is done in UpdateScrollbars)
    ULONG nSBSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if (IsZoom())
        nSBSize = (ULONG)(nSBSize * (double)GetZoom());

    long nSize = pDataWin->GetPosPixel().Y();
    if( !((BrowserDataWin*)pDataWin)->bNoHScroll )
        nSize += aHScroll.GetSizePixel().Height();
    if ( GetOutputSizePixel().Height() < nSize ) return;

    DoHideCursor( "Resize" );
    USHORT nOldVisibleRows =
        (USHORT)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    // did we need a horiz. scroll bar oder gibt es eine Control Area?
    if ( !((BrowserDataWin*)pDataWin)->bNoHScroll &&
         ( ( pCols->Count() - FrozenColCount() ) > 1 || nControlAreaWidth > 0 ) )
        aHScroll.Show();
    else
        aHScroll.Hide();

    // calculate the size of the data window
    long nDataHeight = GetOutputSizePixel().Height() - GetTitleHeight();
    if ( aHScroll.IsVisible() )
        nDataHeight -= nSBSize;

    long nDataWidth = GetOutputSizePixel().Width();
    if ( pVScroll->IsVisible() )
        nDataWidth -= nSBSize;

    // adjust position and size of data window
    pDataWin->SetPosSizePixel(
        Point( 0, GetTitleHeight() ),
        Size( nDataWidth, nDataHeight ) );

    USHORT nVisibleRows =
        (USHORT)(pDataWin->GetOutputSizePixel().Height() / GetDataRowHeight() + 1);

    // TopRow ist unveraendert, aber die Anzahl sichtbarer Zeilen hat sich
    // geaendert
    if ( nVisibleRows != nOldVisibleRows )
        VisibleRowsChanged(nTopRow, nVisibleRows);

    UpdateScrollbars();

    // Control-Area
    Rectangle aInvalidArea( GetControlArea() );
    aInvalidArea.Right() = GetOutputSizePixel().Width();
    aInvalidArea.Left() = 0;
    Invalidate( aInvalidArea );

    // external header-bar
    HeaderBar* pHeaderBar = ( (BrowserDataWin*)pDataWin )->pHeaderBar;
    if ( pHeaderBar )
    {
        // Handle-Column beruecksichtigen
        BrowserColumn *pFirstCol = pCols->GetObject(0);
        long nOfsX = pFirstCol->GetId() ? 0 : pFirstCol->Width();
        pHeaderBar->SetPosPixel( Point( nOfsX, 0 ) );
        pHeaderBar->SetSizePixel( Size( GetOutputSizePixel().Width() - nOfsX, GetTitleHeight() ) );
    }

    AutoSizeLastColumn(); // adjust last column width
    DoShowCursor( "Resize" );
}

//-------------------------------------------------------------------

void BrowseBox::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // initializations
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( STATE_CHANGE_INITSHOW );
    if ( !pCols->Count() )
        return;

    BrowserColumn *pFirstCol = pCols->GetObject(0);
    BOOL bHandleCol = pFirstCol && pFirstCol->GetId() == 0;
    BOOL bHeaderBar = ((BrowserDataWin*)pDataWin)->pHeaderBar != NULL;

    // draw delimitational lines
    if ( !((BrowserDataWin*)pDataWin)->bNoHScroll )
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

    // Title Bar
    // Wenn es eine Handle Column gibt und die Headerbar verfuegbar ist, dann nur
    // die HandleColumn
    // Handle-Column beruecksichtigen
    if ( nTitleLines && (!bHeaderBar || bHandleCol) )
    {
        // iterate through columns to redraw
        long nX = 0;
        USHORT nCol;
        for ( nCol = 0;
              nCol < pCols->Count() && nX < rRect.Right();
              ++nCol )
        {
            // skip invisible colums between frozen and scrollable area
            if ( nCol < nFirstCol && !pCols->GetObject(nCol)->IsFrozen() )
                nCol = nFirstCol;

            // nur die HandleCol ?
            if (bHeaderBar && bHandleCol && nCol > 0)
                break;

            BrowserColumn *pCol = pCols->GetObject(nCol);

            // draw the column and increment position
            if ( pCol->Width() > 4 )
            {
                ButtonFrame aButtonFrame( Point( nX, 0 ),
                    Size( pCol->Width()-1, GetTitleHeight()-1 ),
                    pCol->Title(), FALSE, FALSE,
                    0 != (BROWSER_COLUMN_TITLEABBREVATION&pCol->Flags()) );
                aButtonFrame .Draw( *this );
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

            // skip column
            nX += pCol->Width();
        }

        // retouching
        if ( !bHeaderBar && nCol == pCols->Count() )
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
            SetFillColor( aOldLineColor );
            SetLineColor( aOldFillColor );
        }
    }
}

//-------------------------------------------------------------------

void BrowseBox::PaintRow( OutputDevice &rDev, const Rectangle &rRect )
{
}

//-------------------------------------------------------------------

void BrowseBox::Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags )
{
    BOOL bDrawSelection = (nFlags & WINDOW_DRAW_NOSELECTION) == 0;

    // we need pixel coordinates
    Size aRealSize = pDev->LogicToPixel(rSize);
    Point aRealPos = pDev->LogicToPixel(rPos);

    if ((rSize.Width() < 3) || (rSize.Height() < 3))
        // we want to have two pixels frame ...
        return;

    Font aFont = GetDataWindow().GetDrawPixelFont( pDev );
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

    HeaderBar* pBar = ((BrowserDataWin*)pDataWin)->pHeaderBar;

    // we're drawing onto a foreign device, so we have to fake the DataRowHeight for the subsequent ImplPaintData
    // (as it is based on the settings of our data window, not the foreign device)
    if (!nDataRowHeight)
        ImpGetDataRowHeight();
    long nHeightLogic = PixelToLogic(Size(0, nDataRowHeight), MAP_10TH_MM).Height();
    long nForeignHeightPixel = pDev->LogicToPixel(Size(0, nHeightLogic), MAP_10TH_MM).Height();

    long nOriginalHeight = nDataRowHeight;
    nDataRowHeight = nForeignHeightPixel;

    // this counts for the column widths, too
    USHORT nPos;
    for ( nPos = 0; nPos < pCols->Count(); ++nPos )
    {
        BrowserColumn* pCurrent = pCols->GetObject(nPos);

        long nWidthLogic = PixelToLogic(Size(pCurrent->Width(), 0), MAP_10TH_MM).Width();
        long nForeignWidthPixel = pDev->LogicToPixel(Size(nWidthLogic, 0), MAP_10TH_MM).Width();

        pCurrent->SetWidth(nForeignWidthPixel, GetZoom());
        if ( pBar )
            pBar->SetItemSize( pCurrent->GetId(), pCurrent->Width() );
    }

    // a smaller area for the content
    ++aRealPos.X();
    ++aRealPos.Y();
    aRealSize.Width() -= 2;
    aRealSize.Height() -= 2;

    // let the header bar draw itself
    if ( pBar )
    {
        // the title height with respect to the font set for the given device
        long nTitleHeight = PixelToLogic(Size(0, GetTitleHeight()), MAP_10TH_MM).Height();
        nTitleHeight = pDev->LogicToPixel(Size(0, nTitleHeight), MAP_10TH_MM).Height();

        BrowserColumn* pFirstCol = pCols->Count() ? pCols->GetObject(0) : NULL;

        Point aHeaderPos(pFirstCol && (pFirstCol->GetId() == 0) ? pFirstCol->Width() : 0, 0);
        Size aHeaderSize(aRealSize.Width() - aHeaderPos.X(), nTitleHeight);

        aHeaderPos += aRealPos;
            // do this before converting to logics !

        // the header's draw expects logic coordinates, again
        aHeaderPos = pDev->PixelToLogic(aHeaderPos);
        aHeaderSize = pDev->PixelToLogic(aHeaderSize);

        pBar->Draw(pDev, aHeaderPos, aHeaderSize, nFlags);

        // draw the "upper left cell" (the intersection between the header bar and the handle column)
        if (( pFirstCol->GetId() == 0 ) && ( pFirstCol->Width() > 4 ))
        {
            ButtonFrame aButtonFrame( aRealPos,
                Size( pFirstCol->Width()-1, nTitleHeight-1 ),
                pFirstCol->Title(), FALSE, FALSE, FALSE);
            aButtonFrame.Draw( *pDev );

            Color aOldColor = pDev->GetLineColor();
            pDev->SetLineColor( Color( COL_BLACK ) );

            pDev->DrawLine( Point( aRealPos.X(), aRealPos.Y() + nTitleHeight-1 ),
               Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() + nTitleHeight-1 ) );
            pDev->DrawLine( Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() ),
               Point( aRealPos.X() + pFirstCol->Width() - 1, aRealPos.Y() + nTitleHeight-1 ) );

            pDev->SetLineColor( aOldColor );
        }

        aRealPos.Y() += aHeaderSize.Height();
        aRealSize.Height() -= aHeaderSize.Height();
    }

    // draw our own content (with clipping)
    Region aRegion(Rectangle(aRealPos, aRealSize));
    pDev->SetClipRegion( pDev->PixelToLogic( aRegion ) );

    // do we have to paint the background
    BOOL bBackground = !(nFlags & WINDOW_DRAW_NOBACKGROUND) && GetDataWindow().IsControlBackground();
    if ( bBackground )
    {
        Rectangle aRect( aRealPos, aRealSize );
        pDev->SetFillColor( GetDataWindow().GetControlBackground() );
        pDev->DrawRect( aRect );
    }

    ImplPaintData( *pDev, Rectangle( aRealPos, aRealSize ), TRUE, bDrawSelection );

    // restore the column widths/data row height
    nDataRowHeight = nOriginalHeight;
    for ( nPos = 0; nPos < pCols->Count(); ++nPos )
    {
        BrowserColumn* pCurrent = pCols->GetObject(nPos);

        long nForeignWidthLogic = pDev->PixelToLogic(Size(pCurrent->Width(), 0), MAP_10TH_MM).Width();
        long nWidthPixel = LogicToPixel(Size(nForeignWidthLogic, 0), MAP_10TH_MM).Width();

        pCurrent->SetWidth(nWidthPixel, GetZoom());
        if ( pBar )
            pBar->SetItemSize( pCurrent->GetId(), pCurrent->Width() );
    }

    pDev->Pop();
}

//-------------------------------------------------------------------

void BrowseBox::ImplPaintData(OutputDevice& _rOut, const Rectangle& _rRect, BOOL _bForeignDevice, BOOL _bDrawSelections)
{
    Point aOverallAreaPos = _bForeignDevice ? _rRect.TopLeft() : Point(0,0);
    Size aOverallAreaSize = _bForeignDevice ? _rRect.GetSize() : GetDataWindow().GetOutputSizePixel();
    Point aOverallAreaBRPos = _bForeignDevice ? _rRect.BottomRight() : Point( aOverallAreaSize.Width(), aOverallAreaSize.Height() );

    long nDataRowHeigt = GetDataRowHeight();

    // compute relative rows to redraw
    ULONG nRelTopRow = _bForeignDevice ? 0 : ((ULONG)_rRect.Top() / nDataRowHeigt);
    ULONG nRelBottomRow = (ULONG)(_bForeignDevice ? aOverallAreaSize.Height() : _rRect.Bottom()) / nDataRowHeigt;

    // cache frequently used values
    Point aPos( aOverallAreaPos.X(), nRelTopRow * nDataRowHeigt + aOverallAreaPos.Y() );
    _rOut.SetLineColor( Color( COL_WHITE ) );
    const StyleSettings &rSettings = _rOut.GetSettings().GetStyleSettings();
    const Color &rHighlightTextColor = rSettings.GetHighlightTextColor();
    const Color &rHighlightFillColor = rSettings.GetHighlightColor();
    Color aOldTextColor = _rOut.GetTextColor();
    Color aOldFillColor = _rOut.GetFillColor();
    Color aOldLineColor = _rOut.GetLineColor();
    long nHLineX = 0 == pCols->GetObject(0)->GetId()
                    ? pCols->GetObject(0)->Width()
                    : 0;
    nHLineX += aOverallAreaPos.X();

    // redraw the invalid fields
    BOOL bRetouching = FALSE;
    for ( ULONG nRelRow = nRelTopRow;
          nRelRow <= nRelBottomRow && (ULONG)nTopRow+nRelRow < (ULONG)nRowCount;
          ++nRelRow, aPos.Y() += nDataRowHeigt )
    {
        // get row
        // Zur Sicherheit auf zul"assigen Bereich abfragen:
        DBG_ASSERT( (USHORT)(nTopRow+nRelRow) >= 0 && (USHORT)(nTopRow+nRelRow) < nRowCount,
                    "BrowseBox::ImplPaintData: invalid seek" );
        if ( (nTopRow+nRelRow) < 0 || (USHORT)(nTopRow+nRelRow) >= nRowCount )
            continue;

        // prepare row
        ULONG nCurRow = nTopRow+nRelRow;
        if ( !SeekRow( nCurRow) )
            DBG_ERROR("BrowseBox::ImplPaintData: SeekRow gescheitert");
        _rOut.SetClipRegion();
        aPos.X() = aOverallAreaPos.X();


        // #73325# don't paint the row outside the painting rectangle (DG)
        // prepare auto-highlight
        Rectangle aRowRect( Point( _rRect.TopLeft().X(), aPos.Y() ),
                Size( _rRect.GetSize().Width(), nDataRowHeigt ) );
        PaintRow( _rOut, aRowRect );

        BOOL bRowAutoHighlight  =   _bDrawSelections
                                &&  !bHideSelect
                                &&  ((BrowserDataWin&)GetDataWindow()).bHighlightAuto
                                &&  IsRowSelected( nCurRow );
        if ( bRowAutoHighlight )
        {
            _rOut.SetTextColor( rHighlightTextColor );
            _rOut.SetFillColor( rHighlightFillColor );
            _rOut.SetLineColor();
            _rOut.DrawRect( aRowRect );
        }

        // iterate through columns to redraw
        USHORT nCol;
        for ( nCol = 0; nCol < pCols->Count(); ++nCol )
        {
            // get column
            BrowserColumn *pCol = pCols->GetObject(nCol);

            // at end of invalid area
            if ( aPos.X() >= _rRect.Right() )
                break;

            // skip invisible colums between frozen and scrollable area
            if ( nCol < nFirstCol && !pCol->IsFrozen() )
            {
                nCol = nFirstCol;
                pCol = pCols->GetObject(nCol);
                if (!pCol)
                {   // FS - 21.05.99 - 66325
                    // ist zwar eigentlich woanders (an der richtigen Stelle) gefixt, aber sicher ist sicher ...
                    DBG_ERROR("BrowseBox::PaintData : nFirstCol is probably invalid !");
                    break;
                }
            }

            // prepare Column-AutoHighlight
            BOOL bColAutoHighlight  =   _bDrawSelections
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

            if (!m_bFocusOnlyCursor && (pCol->GetId() == GetCurColumnId()) && (nCurRow == GetCurRow()))
                DrawCursor();

            // draw a single field
            // #63864#, Sonst wird auch etwas gezeichnet, bsp Handle Column
            if (pCol->Width())
            {
                // clip the column's output to the field area
                if (_bForeignDevice)
                {   // (not neccessary if painting onto the data window)
                    Size aFieldSize(pCol->Width(), nDataRowHeigt);

                    if (aPos.X() + aFieldSize.Width() > aOverallAreaBRPos.X())
                        aFieldSize.Width() = aOverallAreaBRPos.X() - aPos.X();

                    if (aPos.Y() + aFieldSize.Height() > aOverallAreaBRPos.Y() + 1)
                    {
                        // for non-handle cols we don't clip vertically : we just don't draw the cell if the line isn't completely visible
                        if (pCol->GetId() != 0)
                            continue;
                        aFieldSize.Height() = aOverallAreaBRPos.Y() + 1 - aPos.Y();
                    }

                    Region aClipToField(Rectangle(aPos, aFieldSize));
                    _rOut.SetClipRegion(aClipToField);
                }
                pCol->Draw( *this, _rOut, aPos, FALSE );
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
            aPos.X() += pCol->Width();
        }

        if ( nCol == pCols->Count() )
            bRetouching = TRUE;

        // reset auto-highlight
        if ( bRowAutoHighlight )
        {
            _rOut.SetTextColor( aOldTextColor );
            _rOut.SetFillColor( aOldFillColor );
            _rOut.SetLineColor( aOldLineColor );
        }

        if ( bHLines )
        {
            // draw horizontal delimitation lines
            _rOut.SetClipRegion();
            Color aOldColor = _rOut.GetLineColor();
            _rOut.SetLineColor( aLineColor );
            long nY = aPos.Y() + nDataRowHeigt - 1;
            if (nY <= aOverallAreaBRPos.Y())
                _rOut.DrawLine( Point( nHLineX, nY ),
                                Point( bVLines
                                        ? min(long(long(aPos.X()) - 1), aOverallAreaBRPos.X())
                                        : aOverallAreaBRPos.X(),
                                      nY ) );
            _rOut.SetLineColor( aOldColor );
        }
    }

    if (aPos.Y() > aOverallAreaBRPos.Y() + 1)
        aPos.Y() = aOverallAreaBRPos.Y() + 1;
        // needed for some of the following drawing

    // retouching
    _rOut.SetClipRegion();
    aOldLineColor = _rOut.GetLineColor();
    aOldFillColor = _rOut.GetFillColor();
    _rOut.SetFillColor( rSettings.GetFaceColor() );
    if ( pCols->Count() && ( pCols->GetObject(0)->GetId() == 0 ) && ( aPos.Y() <= _rRect.Bottom() ) )
    {
        // fill rectangle gray below handle column
        // DG: fill it only until the end of the drawing rect and not to the end, as this may overpaint handle columns
        _rOut.SetLineColor( Color( COL_BLACK ) );
        _rOut.DrawRect( Rectangle(
            Point( aOverallAreaPos.X() - 1, aPos.Y() - 1 ),
            Point( aOverallAreaPos.X() + pCols->GetObject(0)->Width() - 1,
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
        _rOut.SetLineColor( aLineColor );
        Point aVertPos( aOverallAreaPos.X() - 1, aOverallAreaPos.Y() );
        long nDeltaY = aOverallAreaBRPos.Y();
        for ( USHORT nCol = 0; nCol < pCols->Count(); ++nCol )
        {
            // get column
            BrowserColumn *pCol = pCols->GetObject(nCol);

            // skip invisible colums between frozen and scrollable area
            if ( nCol < nFirstCol && !pCol->IsFrozen() )
            {
                nCol = nFirstCol;
                pCol = pCols->GetObject(nCol);
            }

            // skip column
            aVertPos.X() += pCol->Width();

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

//-------------------------------------------------------------------

void BrowseBox::PaintData( Window& rWin, const Rectangle& rRect )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    if ( !bBootstrapped && IsReallyVisible() )
        BrowseBox::StateChanged( STATE_CHANGE_INITSHOW );

    // initializations
    if ( !pCols || !pCols->Count() || !rWin.IsUpdateMode() )
        return;
    if ( ((BrowserDataWin*)pDataWin)->bResizeOnPaint )
        Resize();
    // MI: wer war das denn? Window::Update();

    ImplPaintData(rWin, rRect, FALSE, TRUE);
}

//-------------------------------------------------------------------

void BrowseBox::UpdateScrollbars()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( !bBootstrapped || !IsUpdateMode() )
        return;

    // Rekursionsschutz
    BrowserDataWin *pBDW = (BrowserDataWin*) pDataWin;
    if ( pBDW->bInUpdateScrollbars )
    {
        pBDW->bHadRecursion = TRUE;
        return;
    }
    pBDW->bInUpdateScrollbars = TRUE;

    // the size of the corner window (and the width of the VSB/height of the HSB)
    ULONG nCornerSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if (IsZoom())
        nCornerSize = (ULONG)(nCornerSize * (double)GetZoom());

    // needs VScroll?
    long nMaxRows = (pDataWin->GetSizePixel().Height()) / GetDataRowHeight();
    BOOL bNeedsVScroll = ((BrowserDataWin*)pDataWin)->bAutoVScroll
                            ? nTopRow || nRowCount > nMaxRows
                            : TRUE;
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

    // needs HScroll?
    ULONG nLastCol = GetColumnAtXPosPixel( aDataWinSize.Width() - 1 );

    USHORT nFrozenCols = FrozenColCount();
    BOOL bNeedsHScroll = ((BrowserDataWin*)pDataWin)->bAutoHScroll
        ? nFirstCol > nFrozenCols || nLastCol <= pCols->Count()
        : !((BrowserDataWin*)pDataWin)->bNoHScroll;
    if ( !bNeedsHScroll )
    {
        if ( aHScroll.IsVisible() )
        {
            aHScroll.Hide();
            Size aNewSize( aDataWinSize );
            aNewSize.Height() = GetOutputSizePixel().Height() - GetTitleHeight();
            aDataWinSize = aNewSize;
        }
    }
    else if ( !aHScroll.IsVisible() )
    {
        Size aNewSize( aDataWinSize );
        aNewSize.Height() = GetOutputSizePixel().Height() - GetTitleHeight() - nCornerSize;
        aDataWinSize = aNewSize;
    }

    // adjust position and Width of horizontal scrollbar
    ULONG nHScrX = nControlAreaWidth == USHRT_MAX
        ? GetFrozenWidth() - 1
        : nControlAreaWidth;

    aHScroll.SetPosSizePixel(
        Point( nHScrX, GetOutputSizePixel().Height() - nCornerSize ),
        Size( aDataWinSize.Width() - nHScrX, nCornerSize ) );

    // Scrollable Columns insgesamt
    short nScrollCols = short(pCols->Count()) - (short)nFrozenCols;
    /*short nVisibleHSize= max(nLastCol == BROWSER_INVALIDID
                                ? pCols->Count() - nFirstCol -1
                                : nLastCol - nFirstCol - 1, 0);

    aHScroll.SetVisibleSize( nVisibleHSize );
    aHScroll.SetRange( Range( 0, Max( min(nScrollCols, nVisibleHSize), (short)0 ) ) );
    if ( bNeedsHScroll && !aHScroll.IsVisible() )
        aHScroll.Show();*/

    // Sichtbare Columns
    short nVisibleHSize = nLastCol == BROWSER_INVALIDID
        ? (short)( pCols->Count() - nFirstCol )
        : (short)( nLastCol - nFirstCol );

    short nRange = Max( nScrollCols, (short)0 );
    aHScroll.SetVisibleSize( nVisibleHSize );
    aHScroll.SetRange( Range( 0, nRange ));
    if ( bNeedsHScroll && !aHScroll.IsVisible() )
        aHScroll.Show();

    // adjust position and height of vertical scrollbar
    pVScroll->SetPageSize( nMaxRows );

    if ( nTopRow > nRowCount )
    {
        nTopRow = nRowCount - 1;
        DBG_ERROR("BrowseBox: nTopRow > nRowCount");
    }

    if ( pVScroll->GetThumbPos() != nTopRow )
        pVScroll->SetThumbPos( nTopRow );
    long nVisibleSize = Min( Min( nRowCount, nMaxRows ), long(nRowCount-nTopRow) );
    pVScroll->SetVisibleSize( nVisibleSize ? nVisibleSize : 1 );
    pVScroll->SetRange( Range( 0, nRowCount ) );
    pVScroll->SetPosSizePixel(
        Point( aDataWinSize.Width(), GetTitleHeight() ),
        Size( nCornerSize, aDataWinSize.Height() ) );
    if ( nRowCount <
         long( aDataWinSize.Height() / GetDataRowHeight() ) )
        ScrollRows( -nTopRow );
    long nDelta = nCornerSize;
    if ( bNeedsVScroll && !pVScroll->IsVisible() )
        pVScroll->Show();

    pDataWin->SetSizePixel( aDataWinSize );

    // needs corner-window?
    // (do that AFTER positioning BOTH scrollbars)
    if ( aHScroll.IsVisible() && pVScroll && pVScroll->IsVisible() )
    {
        if ( !( (BrowserDataWin*)pDataWin )->pCornerWin )
            ( (BrowserDataWin*)pDataWin )->pCornerWin = new ScrollBarBox( this, WB_3DLOOK );
        ( (BrowserDataWin*)pDataWin )->pCornerWin->SetPosSizePixel(
            Point( pVScroll->GetPosPixel().X(), aHScroll.GetPosPixel().Y() ),
            Size( nCornerSize, nCornerSize ) );
        ( (BrowserDataWin*)pDataWin )->pCornerWin->Show();
    }
    else
        DELETEZ( ( (BrowserDataWin*)pDataWin )->pCornerWin );

    // ggf. Headerbar mitscrollen
    if ( ((BrowserDataWin*)pDataWin)->pHeaderBar )
    {
        long nWidth = 0;
        for ( USHORT nCol = 0;
              nCol < pCols->Count() && nCol < nFirstCol;
              ++nCol )
        {
            // HandleColumn nicht
            if ( pCols->GetObject(nCol)->GetId() )
                nWidth += pCols->GetObject(nCol)->Width();
        }

        ((BrowserDataWin*)pDataWin)->pHeaderBar->SetOffset( nWidth );
    }

    pBDW->bInUpdateScrollbars = FALSE;
    if ( pBDW->bHadRecursion )
    {
        pBDW->bHadRecursion = FALSE;
        UpdateScrollbars();
    }
}

//-------------------------------------------------------------------

void BrowseBox::Invalidate()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // readjust cursor and selection
    if ( bMultiSelection )
        uRow.pSel->SetTotalRange( Range( 0, nRowCount-1 ) );
    else
        uRow.nSel = Min( uRow.nSel, (long)(nRowCount-1) );
    nCurRow = Min( nCurRow, (long)(nRowCount-1 ));

    // BowseBox::Resize(); if Size not set, last Culumn will be cuttet ?!?
    Control::Invalidate(INVALIDATE_NOCHILDREN /*OV*/ );
    ((BrowserDataWin*)pDataWin)->Invalidate();
}

//-------------------------------------------------------------------

void BrowseBox::SetUpdateMode( BOOL bUpdate )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    BOOL bWasUpdate = IsUpdateMode();
    if ( bWasUpdate == bUpdate )
        return;

    Control::SetUpdateMode( bUpdate );
    // OV
    // Wenn an der BrowseBox WB_CLIPCHILDREN gesetzt ist (wg. Flackerminimierung),
    // wird das Datenfenster nicht von SetUpdateMode invalidiert.
    if( bUpdate )
        ((BrowserDataWin*)pDataWin)->Invalidate();
    ((BrowserDataWin*)pDataWin)->SetUpdateMode( bUpdate );


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

//-------------------------------------------------------------------

BOOL BrowseBox::GetUpdateMode() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return ((BrowserDataWin*)pDataWin)->IsUpdateMode();
}

//-------------------------------------------------------------------

long BrowseBox::GetFrozenWidth() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    long nWidth = 0;
    for ( USHORT nCol = 0;
          nCol < pCols->Count() && pCols->GetObject(nCol)->IsFrozen();
          ++nCol )
        nWidth += pCols->GetObject(nCol)->Width();
    return nWidth;
}

//-------------------------------------------------------------------

void BrowseBox::ColumnInserted( USHORT nPos )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( pColSel )
        pColSel->Insert( nPos );
    UpdateScrollbars();
}

//-------------------------------------------------------------------

USHORT BrowseBox::FrozenColCount() const
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    USHORT nCol;
    for ( nCol = 0;
          nCol < pCols->Count() && pCols->GetObject(nCol)->IsFrozen();
          ++nCol )
        /* empty loop */;
    return nCol;
}

//-------------------------------------------------------------------

IMPL_LINK(BrowseBox,ScrollHdl,ScrollBar*,pBar)
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( pBar->GetDelta() == 0 )
        return 0;

    if ( pBar->GetDelta() < 0 && ((BrowserDataWin*)pDataWin)->bNoScrollBack )
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

//-------------------------------------------------------------------

IMPL_LINK( BrowseBox,EndScrollHdl,ScrollBar*, pBar )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // kein Focus grabben!
    /// GrabFocus();

    if ( /*pBar->GetDelta() <= 0 &&*/ ((BrowserDataWin*)pDataWin)->bNoScrollBack )
    {
        // UpdateScrollbars();
        EndScroll();
        return 0;
    }

    return 0;
}

//-------------------------------------------------------------------

IMPL_LINK( BrowseBox, StartDragHdl, HeaderBar*, pBar )
{
    pBar->SetDragSize( pDataWin->GetOutputSizePixel().Height() );
    return 0;
}

//-------------------------------------------------------------------
// MI: es wurde immer nur die 1. Spalte resized
#pragma optimize("elg",off)

void BrowseBox::MouseButtonDown( const MouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    GrabFocus();

    // onl< mouse events in the title-line are supported
    const Point &rEvtPos = rEvt.GetPosPixel();
    if ( rEvtPos.Y() >= GetTitleHeight() )
        return;

    long nX = 0;
    long nWidth = GetOutputSizePixel().Width();
    for ( USHORT nCol = 0; nCol < pCols->Count() && nX < nWidth; ++nCol )
    {
        // is this column visible?
        BrowserColumn *pCol = pCols->GetObject(nCol);
        if ( pCol->IsFrozen() || nCol >= nFirstCol )
        {
            // compute right end of column
            long nR = nX + pCol->Width() - 1;

            // at the end of a column (and not handle column)?
            if ( pCol->GetId() && Abs( nR - rEvtPos.X() ) < 2 )
            {
                // start resizing the column
                bResizing = TRUE;
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

    // event occured out of data area
    if ( rEvt.IsRight() )
        pDataWin->Command(
            CommandEvent( Point( 1, LONG_MAX ), COMMAND_CONTEXTMENU, TRUE ) );
    else
        SetNoSelection();
}

#pragma optimize("",on)

//-------------------------------------------------------------------

void BrowseBox::MouseMove( const MouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    Pointer aNewPointer;

    USHORT nX = 0;
    for ( USHORT nCol = 0;
          nCol < USHORT(pCols->Count()) &&
            ( nX + pCols->GetObject(nCol)->Width() ) < USHORT(GetOutputSizePixel().Width());
          ++nCol )
        // is this column visible?
        if ( pCols->GetObject(nCol)->IsFrozen() || nCol >= nFirstCol )
        {
            // compute right end of column
            BrowserColumn *pCol = pCols->GetObject(nCol);
            USHORT nR = (USHORT)(nX + pCol->Width() - 1);

            // show resize-pointer?
            if ( bResizing || ( pCol->GetId() &&
                 Abs( ((long) nR ) - rEvt.GetPosPixel().X() ) < MIN_COLUMNWIDTH ) )
            {
                aNewPointer = Pointer( POINTER_HSPLIT );
                if ( bResizing )
                {
                    // alte Hilfslinie loeschen
                    pDataWin->HideTracking() ;

                    // erlaubte breite abholen und neues Delta
                    nDragX = Max( rEvt.GetPosPixel().X(), nMinResizeX );
                    long nDeltaX = nDragX - nResizeX;
                    USHORT nId = GetColumnId(nResizeCol);
                    ULONG nOldWidth = GetColumnWidth(nId);
                    nDragX = QueryColumnResize( GetColumnId(nResizeCol),
                                    nOldWidth + nDeltaX )
                             + nResizeX - nOldWidth;

                    // neue Hilfslinie zeichnen
                    pDataWin->ShowTracking( Rectangle( Point( nDragX, 0 ),
                            Size( 1, pDataWin->GetSizePixel().Height() ) ),
                            SHOWTRACK_SPLIT|SHOWTRACK_WINDOW );
                }

            }

            nX = nR + 1;
        }

    SetPointer( aNewPointer );
}

//-------------------------------------------------------------------

void BrowseBox::MouseButtonUp( const MouseEvent & rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    if ( bResizing )
    {
        // Hilfslinie loeschen
        pDataWin->HideTracking();

        // width changed?
        nDragX = Max( rEvt.GetPosPixel().X(), nMinResizeX );
        if ( (nDragX - nResizeX) != (long)pCols->GetObject(nResizeCol)->Width() )
        {
            // resize column
            long nMaxX = pDataWin->GetSizePixel().Width();
            nDragX = Min( nDragX, nMaxX );
            long nDeltaX = nDragX - nResizeX;
            USHORT nId = GetColumnId(nResizeCol);
            SetColumnWidth( GetColumnId(nResizeCol), GetColumnWidth(nId) + nDeltaX );
            ColumnResized( nId );
        }

        // end action
        SetPointer( Pointer() );
        ReleaseMouse();
        bResizing = FALSE;
    }
    else
        MouseButtonUp( BrowserMouseEvent( (BrowserDataWin*)pDataWin,
                MouseEvent( Point( rEvt.GetPosPixel().X(),
                        rEvt.GetPosPixel().Y() - pDataWin->GetPosPixel().Y() ),
                    rEvt.GetClicks(), rEvt.GetMode(), rEvt.GetButtons(),
                    rEvt.GetModifier() ) ) );
}

//-------------------------------------------------------------------

BOOL bExtendedMode = FALSE;
BOOL bFieldMode = FALSE;

void BrowseBox::MouseButtonDown( const BrowserMouseEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    GrabFocus();

    // adjust selection while and after double-click
    if ( rEvt.GetClicks() == 2 )
    {
        SetNoSelection();
        if ( rEvt.GetRow() >= 0 )
        {
            GoToRow( rEvt.GetRow() );
            SelectRow( rEvt.GetRow(), TRUE, FALSE );
        }
        else if ( bColumnCursor && rEvt.GetColumn() != 0 )
            SelectColumnPos( rEvt.GetColumn(), TRUE, FALSE);
        DoubleClick( rEvt );
    }
    // selections
    else if ( ( rEvt.GetMode() & ( MOUSE_SELECT | MOUSE_SIMPLECLICK ) ) &&
         ( bColumnCursor || rEvt.GetRow() >= 0 ) )
    {
        if ( rEvt.GetClicks() == 1 )
        {
            // initialise flags
            bDrag        = FALSE;
            bHit         = FALSE;
            bRubber      = FALSE;
            a1stPoint    =
            a2ndPoint    = PixelToLogic( rEvt.GetPosPixel() );

            // selection out of range?
            if ( rEvt.GetRow() >= nRowCount ||
                 rEvt.GetColumnId() == BROWSER_INVALIDID )
            {
                SetNoSelection();
                return;
            }

            // while selecting, no cursor
            bSelecting = TRUE;
            DoHideCursor( "MouseButtonDown" );

            // DataRow?
            if ( rEvt.GetRow() >= 0 )
            {
                // Zeilenselektion?
                if ( rEvt.GetColumnId() == 0 || !bColumnCursor )
                {
                    if ( bMultiSelection )
                    {
                        // remove column-selection, if exists
                        if ( pColSel && pColSel->GetSelectCount() )
                        {
                            ToggleSelection();
                            if ( bMultiSelection )
                                uRow.pSel->SelectAll(FALSE);
                            else
                                uRow.nSel = BROWSER_ENDOFSELECTION;
                            if ( pColSel )
                                pColSel->SelectAll(FALSE);
                            bSelect = TRUE;
                        }

                        // expanding mode?
                        if ( rEvt.GetMode() & MOUSE_RANGESELECT )
                        {
                            // select the further touched rows too
                            bSelect = TRUE;
                            ExpandRowSelection( rEvt );
                            return;
                        }

                        // click in the selected area?
                        else if ( IsRowSelected( rEvt.GetRow() ) )
                        {
                            // auf Drag&Drop warten
                            bHit = TRUE;
                            bExtendedMode = MOUSE_MULTISELECT ==
                                    ( rEvt.GetMode() & MOUSE_MULTISELECT );
                            return;
                        }

                        // extension mode?
                        else if ( rEvt.GetMode() & MOUSE_MULTISELECT )
                        {
                            // determine the new selection range
                            // and selection/deselection
                            aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
                            SelectRow( rEvt.GetRow(),
                                    !uRow.pSel->IsSelected( rEvt.GetRow() ) );
                            bSelect = TRUE;
                            return;
                        }
                    }

                    // select directly
                    SetNoSelection();
                    GoToRow( rEvt.GetRow() );
                    SelectRow( rEvt.GetRow(), TRUE );
                    aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
                    bSelect = TRUE;
                }
                else // Column/Field-Selection
                {
                    // click in selected column
                    if ( IsColumnSelected( rEvt.GetColumn() ) ||
                         IsRowSelected( rEvt.GetRow() ) )
                    {
                        bHit = TRUE;
                        bFieldMode = TRUE;
                        return;
                    }

                    SetNoSelection();
                    GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
                    bSelect = TRUE;
                }
            }
            else
            {
                if ( bMultiSelection && rEvt.GetColumnId() == 0 )
                {
                    // toggle all-selection
                    if ( uRow.pSel->GetSelectCount() > ( GetRowCount() / 2 ) )
                        SetNoSelection();
                    else
                        SelectAll();
                }
                else
                    SelectColumnId( rEvt.GetColumnId(), TRUE, FALSE );
            }

            // ggf. Cursor wieder an
            bSelecting = FALSE;
            DoShowCursor( "MouseButtonDown" );
            if ( bSelect )
                Select();
        }
    }
}

//-------------------------------------------------------------------

void BrowseBox::MouseMove( const BrowserMouseEvent &rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
}

//-------------------------------------------------------------------

void BrowseBox::MouseButtonUp( const BrowserMouseEvent &rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    // D&D was possible, but did not occur
    if ( bHit )
    {
        aSelRange = Range( rEvt.GetRow(), rEvt.GetRow() );
        if ( bExtendedMode )
            SelectRow( rEvt.GetRow(), FALSE );
        else
        {
            SetNoSelection();
            if ( bFieldMode )
                GoToRowColumnId( rEvt.GetRow(), rEvt.GetColumnId() );
            else
            {
                GoToRow( rEvt.GetRow() );
                SelectRow( rEvt.GetRow(), TRUE );
            }
        }
        bSelect = TRUE;
        bExtendedMode = FALSE;
        bFieldMode = FALSE;
        bHit = FALSE;
    }

    // activate cursor
    if ( bSelecting )
    {
        bSelecting = FALSE;
        DoShowCursor( "MouseButtonUp" );
        if ( bSelect )
            Select();
    }
}

//-------------------------------------------------------------------

BOOL BrowseBox::Drop( const BrowserDropEvent& )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return FALSE;
}

//-------------------------------------------------------------------

BOOL BrowseBox::QueryDrop( const BrowserDropEvent& )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    return FALSE;
}

//-------------------------------------------------------------------

void BrowseBox::KeyInput( const KeyEvent& rEvt )
{
    if ( !ProcessKey( rEvt ) )
        Control::KeyInput( rEvt );
}

//-------------------------------------------------------------------

BOOL BrowseBox::ProcessKey( const KeyEvent& rEvt )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    USHORT nCode = rEvt.GetKeyCode().GetCode();
    BOOL   bShift = rEvt.GetKeyCode().IsShift();
    BOOL   bCtrl = rEvt.GetKeyCode().IsMod1();
    BOOL   bAlt = rEvt.GetKeyCode().IsMod2();

    USHORT nId = BROWSER_NONE;

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
        }

    if ( nId != BROWSER_NONE )
        Dispatch( nId );
    return nId != BROWSER_NONE;
}

//-------------------------------------------------------------------

void BrowseBox::Dispatch( USHORT nId )
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);

    long nRowsOnPage = pDataWin->GetSizePixel().Height() / GetDataRowHeight();
    BOOL bDone = FALSE;

    switch ( nId )
    {
        case BROWSER_CURSORDOWN:
            if ( ( GetCurRow() + 1 ) < nRowCount )
                bDone = GoToRow( GetCurRow() + 1, FALSE );
            break;
        case BROWSER_CURSORUP:
            if ( GetCurRow() > 0 )
                bDone = GoToRow( GetCurRow() - 1, FALSE );
            break;
        case BROWSER_SELECTHOME:
        {
            DoHideCursor( "BROWSER_SELECTHOME" );
            for ( long nRow = GetCurRow(); nRow >= 0; --nRow )
                SelectRow( nRow );
            GoToRow( 0, TRUE );
            DoShowCursor( "BROWSER_SELECTHOME" );
            break;
        }
        case BROWSER_SELECTEND:
        {
            DoHideCursor( "BROWSER_SELECTEND" );
            long nRowCount = GetRowCount();
            for ( long nRow = GetCurRow(); nRow < nRowCount; ++nRow )
                SelectRow( nRow );
            GoToRow( GetRowCount() - 1, TRUE );
            DoShowCursor( "BROWSER_SELECTEND" );
            break;
        }
        case BROWSER_SELECTDOWN:
        {
            if ( ( GetCurRow() + 1 ) < nRowCount )
            {
                // deselect the current row, if it isn't the first
                // and there is no other selected row above
                long nCurRow = GetCurRow();
                BOOL bSelect = ( !IsRowSelected( nCurRow ) ||
                                 GetSelectRowCount() == 1 || IsRowSelected( nCurRow - 1 ) );
                SelectRow( nCurRow, bSelect, TRUE );
                if ( bDone = GoToRow( GetCurRow() + 1 , FALSE ) )
                    SelectRow( GetCurRow(), TRUE, TRUE );
            }
            else
                bDone = ScrollRows( 1 ) != 0;
            break;
        }
        case BROWSER_SELECTUP:
        {
            // deselect the current row, if it isn't the first
            // and there is no other selected row under
            long nCurRow = GetCurRow();
            BOOL bSelect = ( !IsRowSelected( nCurRow ) ||
                             GetSelectRowCount() == 1 || IsRowSelected( nCurRow + 1 ) );
            SelectRow( nCurRow, bSelect, TRUE );
            if ( bDone = GoToRow( nCurRow - 1 , FALSE ) )
                SelectRow( GetCurRow(), TRUE, TRUE );
            break;
        }
        case BROWSER_CURSORPAGEDOWN:
            bDone = (BOOL)ScrollRows( nRowsOnPage );
            break;
        case BROWSER_CURSORPAGEUP:
            bDone = (BOOL)ScrollRows( -nRowsOnPage );
            break;
        case BROWSER_CURSOREND:
            if ( bColumnCursor )
            {
                USHORT nNewId = GetColumnId(ColCount() -1);
                bDone = (nNewId != 0) && GoToColumnId( nNewId );
                break;
            }
        case BROWSER_CURSORENDOFFILE:
            bDone = GoToRow( nRowCount - 1, FALSE );
            break;
        case BROWSER_CURSORRIGHT:
            if ( bColumnCursor )
            {
                USHORT nNewPos = GetColumnPos( GetCurColumnId() ) + 1;
                USHORT nNewId = GetColumnId( nNewPos );
                if (nNewId != 0)    // Am Zeilenende ?
                    bDone = GoToColumnId( nNewId );
                else
                {
                    USHORT nColId = ( GetColumnId(0) == 0 ) ? GetColumnId(1) : GetColumnId(0);
                    bDone = ( nCurRow < GetRowCount() - 1 ) && GoToRowColumnId( nCurRow + 1, nColId );
                }
            }
            else
                bDone = ScrollColumns( 1 ) != 0;
            break;
        case BROWSER_CURSORHOME:
            if ( bColumnCursor )
            {
                USHORT nNewId = GetColumnId(1);
                bDone = (nNewId != 0) && GoToColumnId( nNewId );
                break;
            }
        case BROWSER_CURSORTOPOFFILE:
            bDone = GoToRow( 0, FALSE );
            break;
        case BROWSER_CURSORLEFT:
            if ( bColumnCursor )
            {
                USHORT nNewPos = GetColumnPos( GetCurColumnId() ) - 1;
                USHORT nNewId = GetColumnId( nNewPos );
                if (nNewId != 0)
                    bDone = GoToColumnId( nNewId );
                else
                    bDone = (nCurRow > 0) && GoToRowColumnId(nCurRow - 1, GetColumnId(ColCount() -1));
            }
            else
                bDone = ScrollColumns( -1 ) != 0;
            break;
        case BROWSER_ENHANCESELECTION:
            SelectRow( GetCurRow(), !IsRowSelected( GetCurRow() ), TRUE );
            bDone = TRUE;
            break;
        case BROWSER_SELECT:
            SelectRow( GetCurRow(), !IsRowSelected( GetCurRow() ), FALSE );
            bDone = TRUE;
            break;
    }

    //! return bDone;
}

//-------------------------------------------------------------------

void BrowseBox::LoseFocus()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    DBG_TRACE1( "BrowseBox: %p->LoseFocus", this );

    if ( bHasFocus )
    {
        DBG_TRACE1( "BrowseBox: %p->HideCursor", this );
        DoHideCursor( "LoseFocus" );

        if ( !bKeepHighlight )
        {
            ToggleSelection();
            bSelectionIsVisible = FALSE;
        }

        bHasFocus = FALSE;
    }
    Control::LoseFocus();
}

//-------------------------------------------------------------------

void BrowseBox::GetFocus()
{
    DBG_CHKTHIS(BrowseBox,BrowseBoxCheckInvariants);
    DBG_TRACE1( "BrowseBox: %p->GetFocus", this );

    if ( !bHasFocus )
    {
        if ( !bSelectionIsVisible )
        {
            bSelectionIsVisible = TRUE;
            if ( bBootstrapped )
                ToggleSelection();
        }

        bHasFocus = TRUE;
        DoShowCursor( "GetFocus" );
    }
    Control::GetFocus();
}

//-------------------------------------------------------------------

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


