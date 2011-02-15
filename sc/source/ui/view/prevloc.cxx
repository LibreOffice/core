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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <vcl/outdev.hxx>
#include <tools/debug.hxx>

#include "prevloc.hxx"
#include "document.hxx"

//==================================================================

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

//------------------------------------------------------------------

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
    void* pEntry = aEntries.First();
    while ( pEntry )
    {
        delete (ScPreviewLocationEntry*) pEntry;
        pEntry = aEntries.Next();
    }
    aEntries.Clear();

    nDrawRanges = 0;
}

void ScPreviewLocationData::AddCellRange( const Rectangle& rRect, const ScRange& rRange, sal_Bool bRepCol, sal_Bool bRepRow,
                                            const MapMode& rDrawMap )
{
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_CELLRANGE, aPixelRect, rRange, bRepCol, bRepRow ) );

    DBG_ASSERT( nDrawRanges < SC_PREVIEW_MAXRANGES, "too many ranges" );
    if ( nDrawRanges < SC_PREVIEW_MAXRANGES )
    {
        aDrawRectangle[nDrawRanges] = aPixelRect;
        aDrawMapMode[nDrawRanges] = rDrawMap;
            if (bRepCol)
                if (bRepRow)
                    aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_EDGE;
                else
                    aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_REPCOL;
            else
                if (bRepRow)
                    aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_REPROW;
                else
                    aDrawRangeId[nDrawRanges] = SC_PREVIEW_RANGE_TAB;
        ++nDrawRanges;
    }
}

void ScPreviewLocationData::AddColHeaders( const Rectangle& rRect, SCCOL nStartCol, SCCOL nEndCol, sal_Bool bRepCol )
{
    SCTAB nTab = 0; //! ?
    ScRange aRange( nStartCol, 0, nTab, nEndCol, 0, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_COLHEADER, aPixelRect, aRange, bRepCol, sal_False ) );
}

void ScPreviewLocationData::AddRowHeaders( const Rectangle& rRect, SCROW nStartRow, SCROW nEndRow, sal_Bool bRepRow )
{
    SCTAB nTab = 0; //! ?
    ScRange aRange( 0, nStartRow, nTab, 0, nEndRow, nTab );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_ROWHEADER, aPixelRect, aRange, sal_False, bRepRow ) );
}

void ScPreviewLocationData::AddHeaderFooter( const Rectangle& rRect, sal_Bool bHeader, sal_Bool bLeft )
{
    ScRange aRange;     //! ?
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    ScPreviewLocationType eType = bHeader ?
                ( bLeft ? SC_PLOC_LEFTHEADER : SC_PLOC_RIGHTHEADER ) :
                ( bLeft ? SC_PLOC_LEFTFOOTER : SC_PLOC_RIGHTFOOTER );
    aEntries.Insert( new ScPreviewLocationEntry( eType, aPixelRect, aRange, sal_False, sal_False ) );
}

void ScPreviewLocationData::AddNoteMark( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_NOTEMARK, aPixelRect, aRange, sal_False, sal_False ) );
}

void ScPreviewLocationData::AddNoteText( const Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    aEntries.Insert( new ScPreviewLocationEntry( SC_PLOC_NOTETEXT, aPixelRect, aRange, sal_False, sal_False ) );
}

//------------------------------------------------------------------

void ScPreviewLocationData::GetDrawRange( sal_uInt16 nPos, Rectangle& rPixelRect, MapMode& rMapMode, sal_uInt8& rRangeId ) const
{
    DBG_ASSERT( nPos < nDrawRanges, "wrong position" );
    if ( nPos < nDrawRanges )
    {
        rPixelRect = aDrawRectangle[nPos];
        rMapMode = aDrawMapMode[nPos];
        rRangeId = aDrawRangeId[nPos];
    }
}

ScPreviewLocationEntry* lcl_GetEntryByAddress( const List& rEntries, const ScAddress& rPos, ScPreviewLocationType eType )
{
    sal_uLong nCount = rEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)rEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aCellRange.In( rPos ) )
            return pEntry;
    }
    return NULL;
}

//UNUSED2008-05  ScAddress ScPreviewLocationData::GetCellFromRange( const Size& rOffsetPixel, const ScRange& rRange ) const
//UNUSED2008-05  {
//UNUSED2008-05      const double nScaleX = HMM_PER_TWIPS;
//UNUSED2008-05      const double nScaleY = HMM_PER_TWIPS;
//UNUSED2008-05
//UNUSED2008-05      Size aOffsetLogic = pWindow->PixelToLogic( rOffsetPixel, aCellMapMode );
//UNUSED2008-05      SCTAB nTab = rRange.aStart.Tab();
//UNUSED2008-05
//UNUSED2008-05      long nPosX = 0;
//UNUSED2008-05      SCCOL nCol = rRange.aStart.Col();
//UNUSED2008-05      SCCOL nEndCol = rRange.aEnd.Col();
//UNUSED2008-05      while ( nCol <= nEndCol && nPosX < aOffsetLogic.Width() )
//UNUSED2008-05      {
//UNUSED2008-05          sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nTab );
//UNUSED2008-05          if (nDocW)
//UNUSED2008-05              nPosX += (long) (nDocW * nScaleX);
//UNUSED2008-05          ++nCol;
//UNUSED2008-05      }
//UNUSED2008-05      if ( nCol > rRange.aStart.Col() )
//UNUSED2008-05          --nCol;
//UNUSED2008-05
//UNUSED2008-05      long nPosY = 0;
//UNUSED2008-05      ScCoupledCompressedArrayIterator< SCROW, sal_uInt8, sal_uInt16> aIter(
//UNUSED2008-05              pDoc->GetRowFlagsArray( nTab), rRange.aStart.Row(),
//UNUSED2008-05              rRange.aEnd.Row(), CR_HIDDEN, 0, pDoc->GetRowHeightArray( nTab));
//UNUSED2008-05      while ( aIter && nPosY < aOffsetLogic.Height() )
//UNUSED2008-05      {
//UNUSED2008-05          sal_uInt16 nDocH = *aIter;
//UNUSED2008-05          if (nDocH)
//UNUSED2008-05              nPosY += (long) (nDocH * nScaleY);
//UNUSED2008-05          ++aIter;
//UNUSED2008-05      }
//UNUSED2008-05      SCROW nRow = aIter.GetPos();
//UNUSED2008-05      if ( nRow > rRange.aStart.Row() )
//UNUSED2008-05          --nRow;
//UNUSED2008-05
//UNUSED2008-05      return ScAddress( nCol, nRow, nTab );
//UNUSED2008-05  }

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
    return sal_False;
}

sal_Bool ScPreviewLocationData::HasCellsInRange( const Rectangle& rVisiblePixel ) const
{
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        ScPreviewLocationType eType = pEntry->eType;
        if ( eType == SC_PLOC_CELLRANGE || eType == SC_PLOC_COLHEADER || eType == SC_PLOC_ROWHEADER )
            if ( pEntry->aPixelRect.IsOver( rVisiblePixel ) )
                return sal_True;
    }
    return sal_False;
}

sal_Bool ScPreviewLocationData::GetHeaderPosition( Rectangle& rRect ) const
{
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_LEFTHEADER || pEntry->eType == SC_PLOC_RIGHTHEADER )
        {
            rRect = pEntry->aPixelRect;
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool ScPreviewLocationData::GetFooterPosition( Rectangle& rRect ) const
{
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_LEFTFOOTER || pEntry->eType == SC_PLOC_RIGHTFOOTER )
        {
            rRect = pEntry->aPixelRect;
            return sal_True;
        }
    }
    return sal_False;
}

sal_Bool ScPreviewLocationData::IsHeaderLeft() const
{
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_LEFTHEADER )
            return sal_True;
        if ( pEntry->eType == SC_PLOC_RIGHTHEADER )
            return sal_False;
    }
    return sal_False;
}

sal_Bool ScPreviewLocationData::IsFooterLeft() const
{
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_LEFTFOOTER )
            return sal_True;
        if ( pEntry->eType == SC_PLOC_RIGHTFOOTER )
            return sal_False;
    }
    return sal_False;
}

long ScPreviewLocationData::GetNoteCountInRange( const Rectangle& rVisiblePixel, sal_Bool bNoteMarks ) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nRet = 0;
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aPixelRect.IsOver( rVisiblePixel ) )
            ++nRet;
    }
    return nRet;
}

sal_Bool ScPreviewLocationData::GetNoteInRange( const Rectangle& rVisiblePixel, long nIndex, sal_Bool bNoteMarks,
                                            ScAddress& rCellPos, Rectangle& rNoteRect ) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nPos = 0;
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aPixelRect.IsOver( rVisiblePixel ) )
        {
            if ( nPos == sal::static_int_cast<sal_uLong>(nIndex) )
            {
                rCellPos = pEntry->aCellRange.aStart;
                rNoteRect = pEntry->aPixelRect;
                return sal_True;
            }
            ++nPos;
        }
    }
    return sal_False;
}

Rectangle ScPreviewLocationData::GetNoteInRangeOutputRect(const Rectangle& rVisiblePixel, sal_Bool bNoteMarks, const ScAddress& aCellPos) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nPos = 0;
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == eType && pEntry->aPixelRect.IsOver( rVisiblePixel ) )
        {
            if ( aCellPos == pEntry->aCellRange.aStart )
                return pEntry->aPixelRect;
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
    sal_Bool bHasHeaderCol = sal_False;
    sal_Bool bHasRepCols   = sal_False;
    sal_Bool bHasMainCols  = sal_False;
    SCCOL nRepeatColStart = 0;
    SCCOL nRepeatColEnd   = 0;
    SCCOL nMainColStart   = 0;
    SCCOL nMainColEnd     = 0;

    // from top to bottom:
    sal_Bool bHasHeaderRow = sal_False;
    sal_Bool bHasRepRows   = sal_False;
    sal_Bool bHasMainRows  = sal_False;
    SCROW nRepeatRowStart = 0;
    SCROW nRepeatRowEnd   = 0;
    SCROW nMainRowStart   = 0;
    SCROW nMainRowEnd     = 0;

    Rectangle aHeaderRect, aRepeatRect, aMainRect;
    SCTAB nTab = 0;

    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_CELLRANGE )
        {
            if ( pEntry->bRepeatCol )
            {
                bHasRepCols = sal_True;
                nRepeatColStart = pEntry->aCellRange.aStart.Col();
                nRepeatColEnd = pEntry->aCellRange.aEnd.Col();
                aRepeatRect.Left() = pEntry->aPixelRect.Left();
                aRepeatRect.Right() = pEntry->aPixelRect.Right();
            }
            else
            {
                bHasMainCols = sal_True;
                nMainColStart = pEntry->aCellRange.aStart.Col();
                nMainColEnd = pEntry->aCellRange.aEnd.Col();
                aMainRect.Left() = pEntry->aPixelRect.Left();
                aMainRect.Right() = pEntry->aPixelRect.Right();
            }
            if ( pEntry->bRepeatRow )
            {
                bHasRepRows = sal_True;
                nRepeatRowStart = pEntry->aCellRange.aStart.Row();
                nRepeatRowEnd = pEntry->aCellRange.aEnd.Row();
                aRepeatRect.Top() = pEntry->aPixelRect.Top();
                aRepeatRect.Bottom() = pEntry->aPixelRect.Bottom();
            }
            else
            {
                bHasMainRows = sal_True;
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
            bHasHeaderCol = sal_True;
            aHeaderRect.Left() = pEntry->aPixelRect.Left();
            aHeaderRect.Right() = pEntry->aPixelRect.Right();
        }
        else if ( pEntry->eType == SC_PLOC_COLHEADER )
        {
            // column headers result in an additional row
            bHasHeaderRow = sal_True;
            aHeaderRect.Top() = pEntry->aPixelRect.Top();
            aHeaderRect.Bottom() = pEntry->aPixelRect.Bottom();
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
                    pColInfo[nColPos].Set( sal_False, nCol,
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
                    pColInfo[nColPos].Set( sal_False, nCol,
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
                pRowInfo[nRowPos].Set( sal_False, nRow,
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
                pRowInfo[nRowPos].Set( sal_False, nRow,
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
    sal_uLong nCount = aEntries.Count();
    for (sal_uLong nListPos=0; nListPos<nCount; nListPos++)
    {
        ScPreviewLocationEntry* pEntry = (ScPreviewLocationEntry*)aEntries.GetObject(nListPos);
        if ( pEntry->eType == SC_PLOC_CELLRANGE && !pEntry->bRepeatCol && !pEntry->bRepeatRow )
        {
            rRange = pEntry->aCellRange;
            rPixRect = pEntry->aPixelRect;
            return sal_True;
        }
    }
    return sal_False;       // not found
}

