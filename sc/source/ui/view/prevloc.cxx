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


#include <vcl/outdev.hxx>

#include "prevloc.hxx"
#include "document.hxx"

enum ScPreviewLocationType
{
    SC_PLOC_CELLRANGE,
    SC_PLOC_COLHEADER,
    SC_PLOC_ROWHEADER,
    SC_PLOC_LEFTHEADER,
    SC_PLOC_RIGHTHEADER,
    SC_PLOC_LEFTFOOTER,
    SC_PLOC_RIGHTFOOTER,
    SC_PLOC_NOTEMARK,
    SC_PLOC_NOTETEXT
};

struct ScPreviewLocationEntry
{
    ScPreviewLocationType   eType;
    Rectangle               aPixelRect;
    ScRange                 aCellRange;
    sal_Bool                    bRepeatCol;
    sal_Bool                    bRepeatRow;

    ScPreviewLocationEntry( ScPreviewLocationType eNewType, const Rectangle& rPixel, const ScRange& rRange,
                            sal_Bool bRepCol, sal_Bool bRepRow ) :
        eType( eNewType ),
        aPixelRect( rPixel ),
        aCellRange( rRange ),
        bRepeatCol( bRepCol ),
        bRepeatRow( bRepRow )
    {
    }
};

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

void ScPreviewTableInfo::SetTab( SCTAB nNewTab )
{
    nTab = nNewTab;
}

void ScPreviewTableInfo::SetColInfo( SCCOL nCount, ScPreviewColRowInfo* pNewInfo )
{
    delete[] pColInfo;
    pColInfo = pNewInfo;
    nCols = nCount;
}

void ScPreviewTableInfo::SetRowInfo( SCROW nCount, ScPreviewColRowInfo* pNewInfo )
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
        SCCOL nStart = 0;
        while ( nStart < nCols && pColInfo[nStart].nPixelEnd < rPixelArea.Left() )
            ++nStart;

        //  cells completely right of the visible area
        SCCOL nEnd = nCols;
        while ( nEnd > 0 && pColInfo[nEnd-1].nPixelStart > rPixelArea.Right() )
            --nEnd;

        if ( nStart > 0 || nEnd < nCols )
        {
            if ( nEnd > nStart )
            {
                SCCOL nNewCount = nEnd - nStart;
                ScPreviewColRowInfo* pNewInfo = new ScPreviewColRowInfo[nNewCount];
                for (SCCOL i=0; i<nNewCount; i++)
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
        SCROW nStart = 0;
        while ( nStart < nRows && pRowInfo[nStart].nPixelEnd < rPixelArea.Top() )
            ++nStart;

        //  cells completely below the visible area
        SCROW nEnd = nRows;
        while ( nEnd > 0 && pRowInfo[nEnd-1].nPixelStart > rPixelArea.Bottom() )
            --nEnd;

        if ( nStart > 0 || nEnd < nRows )
        {
            if ( nEnd > nStart )
            {
                SCROW nNewCount = nEnd - nStart;
                ScPreviewColRowInfo* pNewInfo = new ScPreviewColRowInfo[nNewCount];
                for (SCROW i=0; i<nNewCount; i++)
                    pNewInfo[i] = pRowInfo[nStart + i];
                SetRowInfo( nNewCount, pNewInfo );
            }
            else
                SetRowInfo( 0, NULL );      // all invisible
        }
    }
}

ScPreviewLocationData::ScPreviewLocationData( ScDocument* pDocument, OutputDevice* pWin ) :
    pWindow( pWin ),
    pDoc( pDocument ),
    nDrawRanges( 0 ),
    nPrintTab( 0 )
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

void ScPreviewLocationData::SetPrintTab( SCTAB nNew )
{
    nPrintTab = nNew;
}

void ScPreviewLocationData::Clear()
{
    aEntries.clear();

    nDrawRanges = 0;
}

void ScPreviewLocationData::AddCellRange( const Rectangle& rRect, const ScRange& rRange, sal_Bool bRepCol, sal_Bool bRepRow,
                                            const MapMode& rDrawMap )
{
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.push_front( new ScPreviewLocationEntry( SC_PLOC_CELLRANGE, aPixelRect, rRange, bRepCol, bRepRow ) );

    OSL_ENSURE( nDrawRanges < SC_PREVIEW_MAXRANGES, "too many ranges" );

    if ( nDrawRanges < SC_PREVIEW_MAXRANGES )
    {
        aDrawRectangle[nDrawRanges] = aPixelRect;
        aDrawMapMode[nDrawRanges] = rDrawMap;

        if (bRepCol)
        {
            if (bRepRow)
                aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_EDGE;
            else
                aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_REPCOL;
        }
        else
        {
            if (bRepRow)
                aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_REPROW;
            else
                aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_TAB;
        }

        ++nDrawRanges;
    }
}

void ScPreviewLocationData::AddColHeaders( const Rectangle& rRect, SCCOL nStartCol, SCCOL nEndCol, sal_Bool bRepCol )
{
    SCTAB nTab = 0; //! ?
    ScRange aRange( nStartCol, 0, nTab, nEndCol, 0, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    aEntries.push_front( new ScPreviewLocationEntry( SC_PLOC_COLHEADER, aPixelRect, aRange, bRepCol, false ) );
}

void ScPreviewLocationData::AddRowHeaders( const Rectangle& rRect, SCROW nStartRow, SCROW nEndRow, sal_Bool bRepRow )
{
    SCTAB nTab = 0; //! ?
    ScRange aRange( 0, nStartRow, nTab, 0, nEndRow, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    aEntries.push_front( new ScPreviewLocationEntry( SC_PLOC_ROWHEADER, aPixelRect, aRange, false, bRepRow ) );
}

void ScPreviewLocationData::AddHeaderFooter( const Rectangle& rRect, sal_Bool bHeader, sal_Bool bLeft )
{
    ScRange aRange;     //! ?
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    ScPreviewLocationType eType = bHeader ?
                ( bLeft ? SC_PLOC_LEFTHEADER : SC_PLOC_RIGHTHEADER ) :
                ( bLeft ? SC_PLOC_LEFTFOOTER : SC_PLOC_RIGHTFOOTER );

    aEntries.push_front( new ScPreviewLocationEntry( eType, aPixelRect, aRange, false, false ) );
}

void ScPreviewLocationData::AddNoteMark( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    aEntries.push_front( new ScPreviewLocationEntry( SC_PLOC_NOTEMARK, aPixelRect, aRange, false, false ) );
}

void ScPreviewLocationData::AddNoteText( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    aEntries.push_front( new ScPreviewLocationEntry( SC_PLOC_NOTETEXT, aPixelRect, aRange, false, false ) );
}

void ScPreviewLocationData::GetDrawRange( sal_uInt16 nPos, Rectangle& rPixelRect, MapMode& rMapMode, sal_uInt8& rRangeId ) const
{
    OSL_ENSURE( nPos < nDrawRanges, "wrong position" );
    if ( nPos < nDrawRanges )
    {
        rPixelRect = aDrawRectangle[nPos];
        rMapMode = aDrawMapMode[nPos];
        rRangeId = aDrawRangeId[nPos];
    }
}

static ScPreviewLocationEntry* lcl_GetEntryByAddress( const boost::ptr_list<ScPreviewLocationEntry> &rEntries,
                                               const ScAddress& rPos, ScPreviewLocationType eType )
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = rEntries.begin(); it != rEntries.end(); ++it)
    {
        if ( it->eType == eType && it->aCellRange.In( rPos ) )
            return const_cast<ScPreviewLocationEntry*>(&(*it));
    }

    return NULL;
}


Rectangle ScPreviewLocationData::GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;
    SCTAB nTab = rRange.aStart.Tab();

    long nPosX = 0;
    SCCOL nEndCol = rCellPos.Col();
    for (SCCOL nCol = rRange.aStart.Col(); nCol < nEndCol; nCol++)
    {
        sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
    }
    long nSizeX = (long) ( pDoc->GetColWidth( nEndCol, nTab ) * nScaleX );

    SCROW nEndRow = rCellPos.Row();
    long nPosY = (long) pDoc->GetScaledRowHeight( rRange.aStart.Row(),
            nEndRow, nTab, nScaleY);
    long nSizeY = (long) ( pDoc->GetRowHeight( nEndRow, nTab ) * nScaleY );

    Size aOffsetLogic( nPosX, nPosY );
    Size aSizeLogic( nSizeX, nSizeY );
    Size aOffsetPixel = pWindow->LogicToPixel( aOffsetLogic, aCellMapMode );
    Size aSizePixel = pWindow->LogicToPixel( aSizeLogic, aCellMapMode );

    return Rectangle( Point( aOffsetPixel.Width(), aOffsetPixel.Height() ), aSizePixel );
}

sal_Bool ScPreviewLocationData::GetCellPosition( const ScAddress& rCellPos, Rectangle& rCellRect ) const
{
    ScPreviewLocationEntry* pEntry = lcl_GetEntryByAddress( aEntries, rCellPos, SC_PLOC_CELLRANGE );
    if ( pEntry )
    {
        Rectangle aOffsetRect = GetOffsetPixel( rCellPos, pEntry->aCellRange );
        rCellRect = Rectangle( aOffsetRect.Left() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Top() + pEntry->aPixelRect.Top(),
                               aOffsetRect.Right() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Bottom() + pEntry->aPixelRect.Top() );
        return sal_True;
    }
    return false;
}

sal_Bool ScPreviewLocationData::HasCellsInRange( const Rectangle& rVisiblePixel ) const
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_CELLRANGE || it->eType == SC_PLOC_COLHEADER || it->eType == SC_PLOC_ROWHEADER )
            if ( it->aPixelRect.IsOver( rVisiblePixel ) )
                return true;
    }

    return false;
}

sal_Bool ScPreviewLocationData::GetHeaderPosition( Rectangle& rRect ) const
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_LEFTHEADER || it->eType == SC_PLOC_RIGHTHEADER )
        {
            rRect = it->aPixelRect;
            return true;
        }
    }

    return false;
}

sal_Bool ScPreviewLocationData::GetFooterPosition( Rectangle& rRect ) const
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_LEFTFOOTER || it->eType == SC_PLOC_RIGHTFOOTER )
        {
            rRect = it->aPixelRect;
            return true;
        }
    }

    return false;
}

sal_Bool ScPreviewLocationData::IsHeaderLeft() const
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_LEFTHEADER )
            return true;

        if ( it->eType == SC_PLOC_RIGHTHEADER )
            return false;
    }

    return false;
}

sal_Bool ScPreviewLocationData::IsFooterLeft() const
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_LEFTFOOTER )
            return true;

        if ( it->eType == SC_PLOC_RIGHTFOOTER )
            return false;
    }

    return false;
}

long ScPreviewLocationData::GetNoteCountInRange( const Rectangle& rVisiblePixel, sal_Bool bNoteMarks ) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nRet = 0;
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == eType && it->aPixelRect.IsOver( rVisiblePixel ) )
            ++nRet;
    }

    return nRet;
}

sal_Bool ScPreviewLocationData::GetNoteInRange( const Rectangle& rVisiblePixel, long nIndex, sal_Bool bNoteMarks,
                                            ScAddress& rCellPos, Rectangle& rNoteRect ) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nPos = 0;
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == eType && it->aPixelRect.IsOver( rVisiblePixel ) )
        {
            if ( nPos == sal::static_int_cast<sal_uLong>(nIndex) )
            {
                rCellPos = it->aCellRange.aStart;
                rNoteRect = it->aPixelRect;
                return true;
            }
            ++nPos;
        }
    }

    return false;
}

Rectangle ScPreviewLocationData::GetNoteInRangeOutputRect(const Rectangle& rVisiblePixel, sal_Bool bNoteMarks, const ScAddress& aCellPos) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nPos = 0;
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == eType && it->aPixelRect.IsOver( rVisiblePixel ) )
        {
            if ( aCellPos == it->aCellRange.aStart )
                return it->aPixelRect;
            ++nPos;
        }
    }

    return Rectangle();
}

void ScPreviewLocationData::GetTableInfo( const Rectangle& rVisiblePixel, ScPreviewTableInfo& rInfo ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;

    // from left to right:
    sal_Bool bHasHeaderCol = false;
    sal_Bool bHasRepCols   = false;
    sal_Bool bHasMainCols  = false;
    SCCOL nRepeatColStart = 0;
    SCCOL nRepeatColEnd   = 0;
    SCCOL nMainColStart   = 0;
    SCCOL nMainColEnd     = 0;

    // from top to bottom:
    sal_Bool bHasHeaderRow = false;
    sal_Bool bHasRepRows   = false;
    sal_Bool bHasMainRows  = false;
    SCROW nRepeatRowStart = 0;
    SCROW nRepeatRowEnd   = 0;
    SCROW nMainRowStart   = 0;
    SCROW nMainRowEnd     = 0;

    Rectangle aHeaderRect, aRepeatRect, aMainRect;
    SCTAB nTab = 0;

    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_CELLRANGE )
        {
            if ( it->bRepeatCol )
            {
                bHasRepCols = true;
                nRepeatColStart = it->aCellRange.aStart.Col();
                nRepeatColEnd = it->aCellRange.aEnd.Col();
                aRepeatRect.Left() = it->aPixelRect.Left();
                aRepeatRect.Right() = it->aPixelRect.Right();
            }
            else
            {
                bHasMainCols = true;
                nMainColStart = it->aCellRange.aStart.Col();
                nMainColEnd = it->aCellRange.aEnd.Col();
                aMainRect.Left() = it->aPixelRect.Left();
                aMainRect.Right() = it->aPixelRect.Right();
            }
            if ( it->bRepeatRow )
            {
                bHasRepRows = true;
                nRepeatRowStart = it->aCellRange.aStart.Row();
                nRepeatRowEnd = it->aCellRange.aEnd.Row();
                aRepeatRect.Top() = it->aPixelRect.Top();
                aRepeatRect.Bottom() = it->aPixelRect.Bottom();
            }
            else
            {
                bHasMainRows = true;
                nMainRowStart = it->aCellRange.aStart.Row();
                nMainRowEnd = it->aCellRange.aEnd.Row();
                aMainRect.Top() = it->aPixelRect.Top();
                aMainRect.Bottom() = it->aPixelRect.Bottom();
            }
            nTab = it->aCellRange.aStart.Tab();     //! store separately?
        }
        else if ( it->eType == SC_PLOC_ROWHEADER )
        {
            // row headers result in an additional column
            bHasHeaderCol = true;
            aHeaderRect.Left() = it->aPixelRect.Left();
            aHeaderRect.Right() = it->aPixelRect.Right();
        }
        else if ( it->eType == SC_PLOC_COLHEADER )
        {
            // column headers result in an additional row
            bHasHeaderRow = true;
            aHeaderRect.Top() = it->aPixelRect.Top();
            aHeaderRect.Bottom() = it->aPixelRect.Bottom();
        }
    }

    //
    //  get column info
    //

    SCCOL nColCount = 0;
    SCCOL nCol;
    if ( bHasHeaderCol )
        ++nColCount;
    if ( bHasRepCols )
        for ( nCol=nRepeatColStart; nCol<=nRepeatColEnd; nCol++ )
            if (!pDoc->ColHidden(nCol, nTab))
                ++nColCount;
    if ( bHasMainCols )
        for ( nCol=nMainColStart; nCol<=nMainColEnd; nCol++ )
            if (!pDoc->ColHidden(nCol, nTab))
                ++nColCount;

    if ( nColCount > 0 )
    {
        ScPreviewColRowInfo* pColInfo = new ScPreviewColRowInfo[ nColCount ];
        SCCOL nColPos = 0;

        if ( bHasHeaderCol )
        {
            pColInfo[nColPos].Set( sal_True, 0, aHeaderRect.Left(), aHeaderRect.Right() );
            ++nColPos;
        }
        if ( bHasRepCols )
        {
            long nPosX = 0;
            for ( nCol=nRepeatColStart; nCol<=nRepeatColEnd; nCol++ )
                if (!pDoc->ColHidden(nCol, nTab))
                {
                    sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nTab );
                    long nNextX = nPosX + (long) (nDocW * nScaleX);

                    long nPixelStart = pWindow->LogicToPixel( Size( nPosX, 0 ), aCellMapMode ).Width();
                    long nPixelEnd = pWindow->LogicToPixel( Size( nNextX, 0 ), aCellMapMode ).Width() - 1;
                    pColInfo[nColPos].Set( false, nCol,
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
                if (!pDoc->ColHidden(nCol, nTab))
                {
                    sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nTab );
                    long nNextX = nPosX + (long) (nDocW * nScaleX);

                    long nPixelStart = pWindow->LogicToPixel( Size( nPosX, 0 ), aCellMapMode ).Width();
                    long nPixelEnd = pWindow->LogicToPixel( Size( nNextX, 0 ), aCellMapMode ).Width() - 1;
                    pColInfo[nColPos].Set( false, nCol,
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

    SCROW nRowCount = 0;
    if ( bHasHeaderRow )
        ++nRowCount;
    if ( bHasRepRows )
        nRowCount += pDoc->CountVisibleRows(nRepeatRowStart, nRepeatRowEnd, nTab);
    if ( bHasMainRows )
        nRowCount += pDoc->CountVisibleRows(nMainRowStart, nMainRowEnd, nTab);

    if ( nRowCount > 0 )
    {
        ScPreviewColRowInfo* pRowInfo = new ScPreviewColRowInfo[ nRowCount ];
        SCROW nRowPos = 0;

        if ( bHasHeaderRow )
        {
            pRowInfo[nRowPos].Set( sal_True, 0, aHeaderRect.Top(), aHeaderRect.Bottom() );
            ++nRowPos;
        }
        if ( bHasRepRows )
        {
            long nPosY = 0;
            for (SCROW nRow = nRepeatRowStart; nRow <= nRepeatRowEnd; ++nRow)
            {
                if (pDoc->RowHidden(nRow, nTab))
                    continue;

                sal_uInt16 nDocH = pDoc->GetOriginalHeight( nRow, nTab );
                long nNextY = nPosY + (long) (nDocH * nScaleY);

                long nPixelStart = pWindow->LogicToPixel( Size( 0, nPosY ), aCellMapMode ).Height();
                long nPixelEnd = pWindow->LogicToPixel( Size( 0, nNextY ), aCellMapMode ).Height() - 1;
                pRowInfo[nRowPos].Set( false, nRow,
                        aRepeatRect.Top() + nPixelStart,
                        aRepeatRect.Top() + nPixelEnd );

                nPosY = nNextY;
                ++nRowPos;
            }
        }
        if ( bHasMainRows )
        {
            long nPosY = 0;
            for (SCROW nRow = nMainRowStart; nRow <= nMainRowEnd; ++nRow)
            {
                if (pDoc->RowHidden(nRow, nTab))
                    continue;

                sal_uInt16 nDocH = pDoc->GetOriginalHeight( nRow, nTab );
                long nNextY = nPosY + (long) (nDocH * nScaleY);

                long nPixelStart = pWindow->LogicToPixel( Size( 0, nPosY ), aCellMapMode ).Height();
                long nPixelEnd = pWindow->LogicToPixel( Size( 0, nNextY ), aCellMapMode ).Height() - 1;
                pRowInfo[nRowPos].Set( false, nRow,
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

Rectangle ScPreviewLocationData::GetHeaderCellOutputRect(const Rectangle& rVisRect, const ScAddress& rCellPos, sal_Bool bColHeader) const
{
    // first a stupid implementation
    // NN says here should be done more
    Rectangle aClipRect;
    ScPreviewTableInfo aTableInfo;
    GetTableInfo( rVisRect, aTableInfo );

    if ( (rCellPos.Col() >= 0) &&
        (rCellPos.Row() >= 0) && (rCellPos.Col() < aTableInfo.GetCols()) &&
        (rCellPos.Row() < aTableInfo.GetRows()) )
    {
        SCCOL nCol(0);
        SCROW nRow(0);
        if (bColHeader)
            nCol = rCellPos.Col();
        else
            nRow = rCellPos.Row();
        const ScPreviewColRowInfo& rColInfo = aTableInfo.GetColInfo()[nCol];
        const ScPreviewColRowInfo& rRowInfo = aTableInfo.GetRowInfo()[nRow];

        if ( rColInfo.bIsHeader || rRowInfo.bIsHeader )
            aClipRect = Rectangle( rColInfo.nPixelStart, rRowInfo.nPixelStart, rColInfo.nPixelEnd, rRowInfo.nPixelEnd );
    }
    return aClipRect;
}

Rectangle ScPreviewLocationData::GetCellOutputRect(const ScAddress& rCellPos) const
{
    // first a stupid implementation
    // NN says here should be done more
    Rectangle aRect;
    GetCellPosition(rCellPos, aRect);
    return aRect;
}

// GetMainCellRange is used for links in PDF export

sal_Bool ScPreviewLocationData::GetMainCellRange( ScRange& rRange, Rectangle& rPixRect ) const
{
    boost::ptr_list<ScPreviewLocationEntry>::const_iterator it;
    for (it = aEntries.begin(); it != aEntries.end(); ++it)
    {
        if ( it->eType == SC_PLOC_CELLRANGE && !it->bRepeatCol && !it->bRepeatRow )
        {
            rRange = it->aCellRange;
            rPixRect = it->aPixelRect;
            return true;
        }
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
