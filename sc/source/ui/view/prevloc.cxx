/*************************************************************************
 *
 *  $RCSfile: prevloc.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: nn $ $Date: 2002-03-11 19:28:00 $
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

// INCLUDE ---------------------------------------------------------------

#include <vcl/window.hxx>
#include <tools/debug.hxx>

#include "prevloc.hxx"
#include "document.hxx"

//==================================================================

enum ScPreviewLocationType
{
    SC_PLOC_CELLRANGE,
    SC_PLOC_COLHEADER,
    SC_PLOC_ROWHEADER,
    SC_PLOC_HEADER,
    SC_PLOC_FOOTER,
    SC_PLOC_NOTEMARK,
    SC_PLOC_NOTETEXT
};

struct ScPreviewLocationEntry
{
    ScPreviewLocationType   eType;
    Rectangle               aPixelRect;
    ScRange                 aCellRange;
    BOOL                    bRepeatCol;
    BOOL                    bRepeatRow;

    ScPreviewLocationEntry( ScPreviewLocationType eNewType, const Rectangle& rPixel, const ScRange& rRange,
                            BOOL bRepCol, BOOL bRepRow ) :
        eType( eNewType ),
        aPixelRect( rPixel ),
        aCellRange( rRange ),
        bRepeatCol( bRepCol ),
        bRepeatRow( bRepRow )
    {
    }
};

//==================================================================

ScPreviewTableInfo::ScPreviewTableInfo() :
    nTab(0),
    nCols(0),
    nRows(0),
    pColInfo(NULL),
    pRowInfo(NULL)
{
}

ScPreviewTableInfo::~ScPreviewTableInfo()
{
    delete[] pColInfo;
    delete[] pRowInfo;
}

void ScPreviewTableInfo::SetTab( USHORT nNewTab )
{
    nTab = nNewTab;
}

void ScPreviewTableInfo::SetColInfo( USHORT nCount, ScPreviewColRowInfo* pNewInfo )
{
    delete[] pColInfo;
    pColInfo = pNewInfo;
    nCols = nCount;
}

void ScPreviewTableInfo::SetRowInfo( USHORT nCount, ScPreviewColRowInfo* pNewInfo )
{
    delete[] pRowInfo;
    pRowInfo = pNewInfo;
    nRows = nCount;
}

void ScPreviewTableInfo::LimitToArea( const Rectangle& rPixelArea )
{
    if ( pColInfo )
    {
        //  cells completely left of the visible area
        USHORT nStart = 0;
        while ( nStart < nCols && pColInfo[nStart].nPixelEnd < rPixelArea.Left() )
            ++nStart;

        //  cells completely right of the visible area
        USHORT nEnd = nCols;
        while ( nEnd > 0 && pColInfo[nEnd-1].nPixelStart > rPixelArea.Right() )
            --nEnd;

        if ( nStart > 0 || nEnd < nCols )
        {
            if ( nEnd > nStart )
            {
                USHORT nNewCount = nEnd - nStart;
                ScPreviewColRowInfo* pNewInfo = new ScPreviewColRowInfo[nNewCount];
                for (USHORT i=0; i<nNewCount; i++)
                    pNewInfo[i] = pColInfo[nStart + i];
                SetColInfo( nNewCount, pNewInfo );
            }
            else
                SetColInfo( 0, NULL );      // all invisible
        }
    }

    if ( pRowInfo )
    {
        //  cells completely above the visible area
        USHORT nStart = 0;
        while ( nStart < nRows && pRowInfo[nStart].nPixelEnd < rPixelArea.Top() )
            ++nStart;

        //  cells completely below the visible area
        USHORT nEnd = nRows;
        while ( nEnd > 0 && pRowInfo[nEnd-1].nPixelStart > rPixelArea.Bottom() )
            --nEnd;

        if ( nStart > 0 || nEnd < nRows )
        {
            if ( nEnd > nStart )
            {
                USHORT nNewCount = nEnd - nStart;
                ScPreviewColRowInfo* pNewInfo = new ScPreviewColRowInfo[nNewCount];
                for (USHORT i=0; i<nNewCount; i++)
                    pNewInfo[i] = pRowInfo[nStart + i];
                SetRowInfo( nNewCount, pNewInfo );
            }
            else
                SetRowInfo( 0, NULL );      // all invisible
        }
    }
}

//------------------------------------------------------------------

ScPreviewLocationData::ScPreviewLocationData( ScDocument* pDocument, Window* pWin ) :
    pDoc( pDocument ),
    pWindow( pWin )
{
}

ScPreviewLocationData::~ScPreviewLocationData()
{
    Clear();
}

void ScPreviewLocationData::SetCellMapMode( const MapMode& rMapMode )
{
    aCellMapMode = rMapMode;
}

void ScPreviewLocationData::Clear()
{
    void* pEntry = aEntries.First();
    while ( pEntry )
    {
        delete (ScPreviewLocationEntry*) pEntry;
        pEntry = aEntries.Next();
    }
    aEntries.Clear();
}

void ScPreviewLocationData::AddCellRange( const Rectangle& rRect, const ScRange& rRange, BOOL bRepCol, BOOL bRepRow )
{
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_CELLRANGE, aPixelRect, rRange, bRepCol, bRepRow ) );
}

void ScPreviewLocationData::AddColHeaders( const Rectangle& rRect, USHORT nStartCol, USHORT nEndCol, BOOL bRepCol )
{
    USHORT nTab = 0;    //! ?
    ScRange aRange( nStartCol, 0, nTab, nEndCol, 0, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_COLHEADER, aPixelRect, aRange, bRepCol, FALSE ) );
}

void ScPreviewLocationData::AddRowHeaders( const Rectangle& rRect, USHORT nStartRow, USHORT nEndRow, BOOL bRepRow )
{
    USHORT nTab = 0;    //! ?
    ScRange aRange( 0, nStartRow, nTab, 0, nEndRow, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_ROWHEADER, aPixelRect, aRange, FALSE, bRepRow ) );
}

void ScPreviewLocationData::AddHeaderFooter( const Rectangle& rRect, BOOL bHeader )
{
    ScRange aRange;     //! ?
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    ScPreviewLocationType eType = bHeader ? SC_PLOC_HEADER : SC_PLOC_FOOTER;
    aEntries.Insert( new ScPreviewLocationEntry( eType, aPixelRect, aRange, FALSE, FALSE ) );
}

void ScPreviewLocationData::AddNoteMark( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_NOTEMARK, aPixelRect, aRange, FALSE, FALSE ) );
}

void ScPreviewLocationData::AddNoteText( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_NOTETEXT, aPixelRect, aRange, FALSE, FALSE ) );
}

//------------------------------------------------------------------

ScPreviewLocationEntry* lcl_GetEntryByPosition( const List& rEntries, const Point& rPos, ScPreviewLocationType eType )
{
    ULONG nCount = rEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)rEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aPixelRect.IsInside( rPos ) )
            return pEntry;
    }
    return NULL;
}

ScPreviewLocationEntry* lcl_GetEntryByAddress( const List& rEntries, const ScAddress& rPos, ScPreviewLocationType eType )
{
    ULONG nCount = rEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)rEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aCellRange.In( rPos ) )
            return pEntry;
    }
    return NULL;
}

ScAddress ScPreviewLocationData::GetCellFromRange( const Size& rOffsetPixel, const ScRange& rRange ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;

    Size aOffsetLogic = pWindow->PixelToLogic( rOffsetPixel, aCellMapMode );
    USHORT nTab = rRange.aStart.Tab();

    long nPosX = 0;
    USHORT nCol = rRange.aStart.Col();
    USHORT nEndCol = rRange.aEnd.Col();
    while ( nCol <= nEndCol && nPosX < aOffsetLogic.Width() )
    {
        USHORT nDocW = pDoc->GetColWidth( nCol, nTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
        ++nCol;
    }
    if ( nCol > rRange.aStart.Col() )
        --nCol;

    long nPosY = 0;
    USHORT nRow = rRange.aStart.Row();
    USHORT nEndRow = rRange.aEnd.Row();
    while ( nRow <= nEndRow && nPosY < aOffsetLogic.Height() )
    {
        USHORT nDocH = pDoc->FastGetRowHeight( nRow, nTab );
        if (nDocH)
            nPosY += (long) (nDocH * nScaleY);
        ++nRow;
    }
    if ( nRow > rRange.aStart.Row() )
        --nRow;

    return ScAddress( nCol, nRow, nTab );
}

Rectangle ScPreviewLocationData::GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;
    USHORT nTab = rRange.aStart.Tab();

    long nPosX = 0;
    USHORT nEndCol = rCellPos.Col();
    for (USHORT nCol = rRange.aStart.Col(); nCol < nEndCol; nCol++)
    {
        USHORT nDocW = pDoc->GetColWidth( nCol, nTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
    }
    long nSizeX = (long) ( pDoc->GetColWidth( nEndCol, nTab ) * nScaleX );

    long nPosY = 0;
    USHORT nEndRow = rCellPos.Row();
    for (USHORT nRow = rRange.aStart.Row(); nRow < nEndRow; nRow++)
    {
        USHORT nDocH = pDoc->FastGetRowHeight( nRow, nTab );
        if (nDocH)
            nPosY += (long) (nDocH * nScaleY);
    }
    long nSizeY = (long) ( pDoc->FastGetRowHeight( nEndRow, nTab ) * nScaleY );

    Size aOffsetLogic( nPosX, nPosY );
    Size aSizeLogic( nSizeX, nSizeY );
    Size aOffsetPixel = pWindow->LogicToPixel( aOffsetLogic, aCellMapMode );
    Size aSizePixel = pWindow->LogicToPixel( aSizeLogic, aCellMapMode );

    return Rectangle( Point( aOffsetPixel.Width(), aOffsetPixel.Height() ), aSizePixel );
}

BOOL ScPreviewLocationData::GetCell( const Point& rPos, ScAddress& rCellPos, Rectangle& rCellRect ) const
{
    ScPreviewLocationEntry* pEntry = lcl_GetEntryByPosition( aEntries, rPos, SC_PLOC_CELLRANGE );
    if ( pEntry )
    {
        Size aOffsetPixel( rPos.X() - pEntry->aPixelRect.Left(), rPos.Y() - pEntry->aPixelRect.Top() );
        rCellPos = GetCellFromRange( aOffsetPixel, pEntry->aCellRange );

        Rectangle aOffsetRect = GetOffsetPixel( rCellPos, pEntry->aCellRange );
        rCellRect = Rectangle( aOffsetRect.Left() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Top() + pEntry->aPixelRect.Top(),
                               aOffsetRect.Right() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Bottom() + pEntry->aPixelRect.Top() );
        return TRUE;
    }
    return FALSE;
}

BOOL ScPreviewLocationData::GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const
{
    ScPreviewLocationEntry* pEntry = lcl_GetEntryByAddress( aEntries, rCellPos, SC_PLOC_CELLRANGE );
    if ( pEntry )
    {
        Rectangle aOffsetRect = GetOffsetPixel( rCellPos, pEntry->aCellRange );
        rCellRect = Rectangle( aOffsetRect.Left() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Top() + pEntry->aPixelRect.Top(),
                               aOffsetRect.Right() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Bottom() + pEntry->aPixelRect.Top() );
        return TRUE;
    }
    return FALSE;
}

BOOL ScPreviewLocationData::HasCellsInRange( const Rectangle& rVisiblePixel ) const
{
    ULONG nCount = aEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        ScPreviewLocationType eType = pEntry->eType;
        if ( eType == SC_PLOC_CELLRANGE || eType == SC_PLOC_COLHEADER || eType == SC_PLOC_ROWHEADER )
            if ( pEntry->aPixelRect.IsOver( rVisiblePixel ) )
                return TRUE;
    }
    return FALSE;
}

BOOL ScPreviewLocationData::GetHeaderPosition( Rectangle& rRect ) const
{
    ULONG nCount = aEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_HEADER )
        {
            rRect = pEntry->aPixelRect;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL ScPreviewLocationData::GetFooterPosition( Rectangle& rRect ) const
{
    ULONG nCount = aEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_FOOTER )
        {
            rRect = pEntry->aPixelRect;
            return TRUE;
        }
    }
    return FALSE;
}

void ScPreviewLocationData::GetTableInfo( const Rectangle& rVisiblePixel, ScPreviewTableInfo& rInfo ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;

    // from left to right:
    BOOL bHasHeaderCol = FALSE;
    BOOL bHasRepCols   = FALSE;
    BOOL bHasMainCols  = FALSE;
    USHORT nRepeatColStart = 0;
    USHORT nRepeatColEnd   = 0;
    USHORT nMainColStart   = 0;
    USHORT nMainColEnd     = 0;

    // from top to bottom:
    BOOL bHasHeaderRow = FALSE;
    BOOL bHasRepRows   = FALSE;
    BOOL bHasMainRows  = FALSE;
    USHORT nRepeatRowStart = 0;
    USHORT nRepeatRowEnd   = 0;
    USHORT nMainRowStart   = 0;
    USHORT nMainRowEnd     = 0;

    Rectangle aHeaderRect, aRepeatRect, aMainRect;
    USHORT nTab = 0;

    ULONG nCount = aEntries.Count();
    for (ULONG nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_CELLRANGE )
        {
            if ( pEntry->bRepeatCol )
            {
                bHasRepCols = TRUE;
                nRepeatColStart = pEntry->aCellRange.aStart.Col();
                nRepeatColEnd = pEntry->aCellRange.aEnd.Col();
                aRepeatRect.Left() = pEntry->aPixelRect.Left();
                aRepeatRect.Right() = pEntry->aPixelRect.Right();
            }
            else
            {
                bHasMainCols = TRUE;
                nMainColStart = pEntry->aCellRange.aStart.Col();
                nMainColEnd = pEntry->aCellRange.aEnd.Col();
                aMainRect.Left() = pEntry->aPixelRect.Left();
                aMainRect.Right() = pEntry->aPixelRect.Right();
            }
            if ( pEntry->bRepeatRow )
            {
                bHasRepRows = TRUE;
                nRepeatRowStart = pEntry->aCellRange.aStart.Row();
                nRepeatRowEnd = pEntry->aCellRange.aEnd.Row();
                aRepeatRect.Top() = pEntry->aPixelRect.Top();
                aRepeatRect.Bottom() = pEntry->aPixelRect.Bottom();
            }
            else
            {
                bHasMainRows = TRUE;
                nMainRowStart = pEntry->aCellRange.aStart.Row();
                nMainRowEnd = pEntry->aCellRange.aEnd.Row();
                aMainRect.Top() = pEntry->aPixelRect.Top();
                aMainRect.Bottom() = pEntry->aPixelRect.Bottom();
            }
            nTab = pEntry->aCellRange.aStart.Tab();     //! store separately?
        }
        else if ( pEntry->eType == SC_PLOC_ROWHEADER )
        {
            // row headers result in an additional column
            bHasHeaderCol = TRUE;
            aHeaderRect.Left() = pEntry->aPixelRect.Left();
            aHeaderRect.Right() = pEntry->aPixelRect.Right();
        }
        else if ( pEntry->eType == SC_PLOC_COLHEADER )
        {
            // column headers result in an additional row
            bHasHeaderRow = TRUE;
            aHeaderRect.Top() = pEntry->aPixelRect.Top();
            aHeaderRect.Bottom() = pEntry->aPixelRect.Bottom();
        }
    }

    //
    //  get column info
    //

    USHORT nColCount = 0;
    USHORT nCol;
    if ( bHasHeaderCol )
        ++nColCount;
    if ( bHasRepCols )
        for ( nCol=nRepeatColStart; nCol<=nRepeatColEnd; nCol++ )
            if ( ( pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN ) == 0 )
                ++nColCount;
    if ( bHasMainCols )
        for ( nCol=nMainColStart; nCol<=nMainColEnd; nCol++ )
            if ( ( pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN ) == 0 )
                ++nColCount;

    if ( nColCount > 0 )
    {
        ScPreviewColRowInfo* pColInfo = new ScPreviewColRowInfo[ nColCount ];
        USHORT nColPos = 0;

        if ( bHasHeaderCol )
        {
            pColInfo[nColPos].Set( TRUE, 0, aHeaderRect.Left(), aHeaderRect.Right() );
            ++nColPos;
        }
        if ( bHasRepCols )
        {
            long nPosX = 0;
            for ( nCol=nRepeatColStart; nCol<=nRepeatColEnd; nCol++ )
                if ( ( pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN ) == 0 )
                {
                    USHORT nDocW = pDoc->GetColWidth( nCol, nTab );
                    long nNextX = nPosX + (long) (nDocW * nScaleX);

                    long nPixelStart = pWindow->LogicToPixel( Size( nPosX, 0 ), aCellMapMode ).Width();
                    long nPixelEnd = pWindow->LogicToPixel( Size( nNextX, 0 ), aCellMapMode ).Width() - 1;
                    pColInfo[nColPos].Set( FALSE, nCol,
                                                aRepeatRect.Left() + nPixelStart,
                                                aRepeatRect.Left() + nPixelEnd );

                    nPosX = nNextX;
                    ++nColPos;
                }
        }
        if ( bHasMainCols )
        {
            long nPosX = 0;
            for ( nCol=nMainColStart; nCol<=nMainColEnd; nCol++ )
                if ( ( pDoc->GetColFlags( nCol, nTab ) & CR_HIDDEN ) == 0 )
                {
                    USHORT nDocW = pDoc->GetColWidth( nCol, nTab );
                    long nNextX = nPosX + (long) (nDocW * nScaleX);

                    long nPixelStart = pWindow->LogicToPixel( Size( nPosX, 0 ), aCellMapMode ).Width();
                    long nPixelEnd = pWindow->LogicToPixel( Size( nNextX, 0 ), aCellMapMode ).Width() - 1;
                    pColInfo[nColPos].Set( FALSE, nCol,
                                                aMainRect.Left() + nPixelStart,
                                                aMainRect.Left() + nPixelEnd );

                    nPosX = nNextX;
                    ++nColPos;
                }
        }
        rInfo.SetColInfo( nColCount, pColInfo );
    }
    else
        rInfo.SetColInfo( 0, NULL );

    //
    //  get row info
    //

    USHORT nRowCount = 0;
    USHORT nRow;
    if ( bHasHeaderRow )
        ++nRowCount;
    if ( bHasRepRows )
        for ( nRow=nRepeatRowStart; nRow<=nRepeatRowEnd; nRow++ )
            if ( ( pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN ) == 0 )
                ++nRowCount;
    if ( bHasMainRows )
        for ( nRow=nMainRowStart; nRow<=nMainRowEnd; nRow++ )
            if ( ( pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN ) == 0 )
                ++nRowCount;

    if ( nRowCount > 0 )
    {
        ScPreviewColRowInfo* pRowInfo = new ScPreviewColRowInfo[ nRowCount ];
        USHORT nRowPos = 0;

        if ( bHasHeaderRow )
        {
            pRowInfo[nRowPos].Set( TRUE, 0, aHeaderRect.Top(), aHeaderRect.Bottom() );
            ++nRowPos;
        }
        if ( bHasRepRows )
        {
            long nPosY = 0;
            for ( nRow=nRepeatRowStart; nRow<=nRepeatRowEnd; nRow++ )
                if ( ( pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN ) == 0 )
                {
                    USHORT nDocH = pDoc->FastGetRowHeight( nRow, nTab );
                    long nNextY = nPosY + (long) (nDocH * nScaleY);

                    long nPixelStart = pWindow->LogicToPixel( Size( 0, nPosY ), aCellMapMode ).Height();
                    long nPixelEnd = pWindow->LogicToPixel( Size( 0, nNextY ), aCellMapMode ).Height() - 1;
                    pRowInfo[nRowPos].Set( FALSE, nRow,
                                                aRepeatRect.Top() + nPixelStart,
                                                aRepeatRect.Top() + nPixelEnd );

                    nPosY = nNextY;
                    ++nRowPos;
                }
        }
        if ( bHasMainRows )
        {
            long nPosY = 0;
            for ( nRow=nMainRowStart; nRow<=nMainRowEnd; nRow++ )
                if ( ( pDoc->GetRowFlags( nRow, nTab ) & CR_HIDDEN ) == 0 )
                {
                    USHORT nDocH = pDoc->FastGetRowHeight( nRow, nTab );
                    long nNextY = nPosY + (long) (nDocH * nScaleY);

                    long nPixelStart = pWindow->LogicToPixel( Size( 0, nPosY ), aCellMapMode ).Height();
                    long nPixelEnd = pWindow->LogicToPixel( Size( 0, nNextY ), aCellMapMode ).Height() - 1;
                    pRowInfo[nRowPos].Set( FALSE, nRow,
                                                aMainRect.Top() + nPixelStart,
                                                aMainRect.Top() + nPixelEnd );

                    nPosY = nNextY;
                    ++nRowPos;
                }
        }
        rInfo.SetRowInfo( nRowCount, pRowInfo );
    }
    else
        rInfo.SetRowInfo( 0, NULL );

    //
    //  limit to visible area
    //

    rInfo.SetTab( nTab );
    rInfo.LimitToArea( rVisiblePixel );
}


