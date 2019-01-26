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

#include <prevloc.hxx>
#include <document.hxx>

#include <osl/diagnose.h>
#include <vcl/outdev.hxx>

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
    ScPreviewLocationType const   eType;
    tools::Rectangle const        aPixelRect;
    ScRange const                 aCellRange;
    bool const                    bRepeatCol;
    bool const                    bRepeatRow;

    ScPreviewLocationEntry( ScPreviewLocationType eNewType, const tools::Rectangle& rPixel, const ScRange& rRange,
                            bool bRepCol, bool bRepRow ) :
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
    nRows(0)
{
}

ScPreviewTableInfo::~ScPreviewTableInfo()
{
}

void ScPreviewTableInfo::SetTab( SCTAB nNewTab )
{
    nTab = nNewTab;
}

void ScPreviewTableInfo::SetColInfo( SCCOL nCount, ScPreviewColRowInfo* pNewInfo )
{
    pColInfo.reset(pNewInfo);
    nCols = nCount;
}

void ScPreviewTableInfo::SetRowInfo( SCROW nCount, ScPreviewColRowInfo* pNewInfo )
{
    pRowInfo.reset(pNewInfo);
    nRows = nCount;
}

void ScPreviewTableInfo::LimitToArea( const tools::Rectangle& rPixelArea )
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
                SetColInfo( 0, nullptr );      // all invisible
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
                SetRowInfo( 0, nullptr );      // all invisible
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
    m_Entries.clear();

    nDrawRanges = 0;
}

void ScPreviewLocationData::AddCellRange( const tools::Rectangle& rRect, const ScRange& rRange, bool bRepCol, bool bRepRow,
                                            const MapMode& rDrawMap )
{
    tools::Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );
    m_Entries.push_front( std::make_unique<ScPreviewLocationEntry>(SC_PLOC_CELLRANGE, aPixelRect, rRange, bRepCol, bRepRow) );

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

void ScPreviewLocationData::AddColHeaders( const tools::Rectangle& rRect, SCCOL nStartCol, SCCOL nEndCol, bool bRepCol )
{
    SCTAB nTab = 0; //! ?
    ScRange aRange( nStartCol, 0, nTab, nEndCol, 0, nTab );
    tools::Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    m_Entries.push_front( std::make_unique<ScPreviewLocationEntry>(SC_PLOC_COLHEADER, aPixelRect, aRange, bRepCol, false) );
}

void ScPreviewLocationData::AddRowHeaders( const tools::Rectangle& rRect, SCROW nStartRow, SCROW nEndRow, bool bRepRow )
{
    SCTAB nTab = 0; //! ?
    ScRange aRange( 0, nStartRow, nTab, 0, nEndRow, nTab );
    tools::Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    m_Entries.push_front( std::make_unique<ScPreviewLocationEntry>(SC_PLOC_ROWHEADER, aPixelRect, aRange, false, bRepRow) );
}

void ScPreviewLocationData::AddHeaderFooter( const tools::Rectangle& rRect, bool bHeader, bool bLeft )
{
    ScRange aRange;     //! ?
    tools::Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    ScPreviewLocationType eType = bHeader ?
                ( bLeft ? SC_PLOC_LEFTHEADER : SC_PLOC_RIGHTHEADER ) :
                ( bLeft ? SC_PLOC_LEFTFOOTER : SC_PLOC_RIGHTFOOTER );

    m_Entries.push_front( std::make_unique<ScPreviewLocationEntry>(eType, aPixelRect, aRange, false, false) );
}

void ScPreviewLocationData::AddNoteMark( const tools::Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    tools::Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    m_Entries.push_front( std::make_unique<ScPreviewLocationEntry>(SC_PLOC_NOTEMARK, aPixelRect, aRange, false, false) );
}

void ScPreviewLocationData::AddNoteText( const tools::Rectangle& rRect, const ScAddress& rPos )
{
    ScRange aRange( rPos );
    tools::Rectangle aPixelRect( pWindow->LogicToPixel( rRect ) );

    m_Entries.push_front( std::make_unique<ScPreviewLocationEntry>(SC_PLOC_NOTETEXT, aPixelRect, aRange, false, false) );
}

void ScPreviewLocationData::GetDrawRange( sal_uInt16 nPos, tools::Rectangle& rPixelRect, MapMode& rMapMode, sal_uInt8& rRangeId ) const
{
    OSL_ENSURE( nPos < nDrawRanges, "wrong position" );
    if ( nPos < nDrawRanges )
    {
        rPixelRect = aDrawRectangle[nPos];
        rMapMode = aDrawMapMode[nPos];
        rRangeId = aDrawRangeId[nPos];
    }
}

static ScPreviewLocationEntry* lcl_GetEntryByAddress(
        ScPreviewLocationData::Entries_t const& rEntries,
        const ScAddress& rPos, ScPreviewLocationType const eType)
{
    for (auto const& it : rEntries)
    {
        if ( it->eType == eType && it->aCellRange.In( rPos ) )
            return it.get();
    }

    return nullptr;
}

tools::Rectangle ScPreviewLocationData::GetOffsetPixel( const ScAddress& rCellPos, const ScRange& rRange ) const
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
            nPosX += static_cast<long>(nDocW * nScaleX);
    }
    long nSizeX = static_cast<long>( pDoc->GetColWidth( nEndCol, nTab ) * nScaleX );

    SCROW nEndRow = rCellPos.Row();
    long nPosY = static_cast<long>(pDoc->GetScaledRowHeight( rRange.aStart.Row(),
            nEndRow, nTab, nScaleY));
    long nSizeY = static_cast<long>( pDoc->GetRowHeight( nEndRow, nTab ) * nScaleY );

    Size aOffsetLogic( nPosX, nPosY );
    Size aSizeLogic( nSizeX, nSizeY );
    Size aOffsetPixel = pWindow->LogicToPixel( aOffsetLogic, aCellMapMode );
    Size aSizePixel = pWindow->LogicToPixel( aSizeLogic, aCellMapMode );

    return tools::Rectangle( Point( aOffsetPixel.Width(), aOffsetPixel.Height() ), aSizePixel );
}

void ScPreviewLocationData::GetCellPosition( const ScAddress& rCellPos, tools::Rectangle& rCellRect ) const
{
    ScPreviewLocationEntry* pEntry = lcl_GetEntryByAddress( m_Entries, rCellPos, SC_PLOC_CELLRANGE );
    if ( pEntry )
    {
        tools::Rectangle aOffsetRect = GetOffsetPixel( rCellPos, pEntry->aCellRange );
        rCellRect = tools::Rectangle( aOffsetRect.Left() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Top() + pEntry->aPixelRect.Top(),
                               aOffsetRect.Right() + pEntry->aPixelRect.Left(),
                               aOffsetRect.Bottom() + pEntry->aPixelRect.Top() );
    }
}

bool ScPreviewLocationData::HasCellsInRange( const tools::Rectangle& rVisiblePixel ) const
{
    for (auto const& it : m_Entries)
    {
        if ( it->eType == SC_PLOC_CELLRANGE || it->eType == SC_PLOC_COLHEADER || it->eType == SC_PLOC_ROWHEADER )
            if ( it->aPixelRect.IsOver( rVisiblePixel ) )
                return true;
    }

    return false;
}

bool ScPreviewLocationData::GetHeaderPosition( tools::Rectangle& rRect ) const
{
    for (auto const& it : m_Entries)
    {
        if ( it->eType == SC_PLOC_LEFTHEADER || it->eType == SC_PLOC_RIGHTHEADER )
        {
            rRect = it->aPixelRect;
            return true;
        }
    }

    return false;
}

bool ScPreviewLocationData::GetFooterPosition( tools::Rectangle& rRect ) const
{
    for (auto const& it : m_Entries)
    {
        if ( it->eType == SC_PLOC_LEFTFOOTER || it->eType == SC_PLOC_RIGHTFOOTER )
        {
            rRect = it->aPixelRect;
            return true;
        }
    }

    return false;
}

bool ScPreviewLocationData::IsHeaderLeft() const
{
    for (auto const& it : m_Entries)
    {
        if ( it->eType == SC_PLOC_LEFTHEADER )
            return true;

        if ( it->eType == SC_PLOC_RIGHTHEADER )
            return false;
    }

    return false;
}

bool ScPreviewLocationData::IsFooterLeft() const
{
    for (auto const& it : m_Entries)
    {
        if ( it->eType == SC_PLOC_LEFTFOOTER )
            return true;

        if ( it->eType == SC_PLOC_RIGHTFOOTER )
            return false;
    }

    return false;
}

long ScPreviewLocationData::GetNoteCountInRange( const tools::Rectangle& rVisiblePixel, bool bNoteMarks ) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nRet = 0;
    for (auto const& it : m_Entries)
    {
        if ( it->eType == eType && it->aPixelRect.IsOver( rVisiblePixel ) )
            ++nRet;
    }

    return nRet;
}

bool ScPreviewLocationData::GetNoteInRange( const tools::Rectangle& rVisiblePixel, long nIndex, bool bNoteMarks,
                                            ScAddress& rCellPos, tools::Rectangle& rNoteRect ) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nPos = 0;
    for (auto const& it : m_Entries)
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

tools::Rectangle ScPreviewLocationData::GetNoteInRangeOutputRect(const tools::Rectangle& rVisiblePixel, bool bNoteMarks, const ScAddress& aCellPos) const
{
    ScPreviewLocationType eType = bNoteMarks ? SC_PLOC_NOTEMARK : SC_PLOC_NOTETEXT;

    sal_uLong nPos = 0;
    for (auto const& it : m_Entries)
    {
        if ( it->eType == eType && it->aPixelRect.IsOver( rVisiblePixel ) )
        {
            if ( aCellPos == it->aCellRange.aStart )
                return it->aPixelRect;
            ++nPos;
        }
    }

    return tools::Rectangle();
}

void ScPreviewLocationData::GetTableInfo( const tools::Rectangle& rVisiblePixel, ScPreviewTableInfo& rInfo ) const
{
    const double nScaleX = HMM_PER_TWIPS;
    const double nScaleY = HMM_PER_TWIPS;

    // from left to right:
    bool bHasHeaderCol = false;
    bool bHasRepCols   = false;
    bool bHasMainCols  = false;
    SCCOL nRepeatColStart = 0;
    SCCOL nRepeatColEnd   = 0;
    SCCOL nMainColStart   = 0;
    SCCOL nMainColEnd     = 0;

    // from top to bottom:
    bool bHasHeaderRow = false;
    bool bHasRepRows   = false;
    bool bHasMainRows  = false;
    SCROW nRepeatRowStart = 0;
    SCROW nRepeatRowEnd   = 0;
    SCROW nMainRowStart   = 0;
    SCROW nMainRowEnd     = 0;

    tools::Rectangle aHeaderRect, aRepeatRect, aMainRect;
    SCTAB nTab = 0;

    for (auto const& it : m_Entries)
    {
        if ( it->eType == SC_PLOC_CELLRANGE )
        {
            if ( it->bRepeatCol )
            {
                bHasRepCols = true;
                nRepeatColStart = it->aCellRange.aStart.Col();
                nRepeatColEnd = it->aCellRange.aEnd.Col();
                aRepeatRect.SetLeft( it->aPixelRect.Left() );
                aRepeatRect.SetRight( it->aPixelRect.Right() );
            }
            else
            {
                bHasMainCols = true;
                nMainColStart = it->aCellRange.aStart.Col();
                nMainColEnd = it->aCellRange.aEnd.Col();
                aMainRect.SetLeft( it->aPixelRect.Left() );
                aMainRect.SetRight( it->aPixelRect.Right() );
            }
            if ( it->bRepeatRow )
            {
                bHasRepRows = true;
                nRepeatRowStart = it->aCellRange.aStart.Row();
                nRepeatRowEnd = it->aCellRange.aEnd.Row();
                aRepeatRect.SetTop( it->aPixelRect.Top() );
                aRepeatRect.SetBottom( it->aPixelRect.Bottom() );
            }
            else
            {
                bHasMainRows = true;
                nMainRowStart = it->aCellRange.aStart.Row();
                nMainRowEnd = it->aCellRange.aEnd.Row();
                aMainRect.SetTop( it->aPixelRect.Top() );
                aMainRect.SetBottom( it->aPixelRect.Bottom() );
            }
            nTab = it->aCellRange.aStart.Tab();     //! store separately?
        }
        else if ( it->eType == SC_PLOC_ROWHEADER )
        {
            // row headers result in an additional column
            bHasHeaderCol = true;
            aHeaderRect.SetLeft( it->aPixelRect.Left() );
            aHeaderRect.SetRight( it->aPixelRect.Right() );
        }
        else if ( it->eType == SC_PLOC_COLHEADER )
        {
            // column headers result in an additional row
            bHasHeaderRow = true;
            aHeaderRect.SetTop( it->aPixelRect.Top() );
            aHeaderRect.SetBottom( it->aPixelRect.Bottom() );
        }
    }

    //  get column info

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
            pColInfo[nColPos].Set( true, 0, aHeaderRect.Left(), aHeaderRect.Right() );
            ++nColPos;
        }
        if ( bHasRepCols )
        {
            long nPosX = 0;
            for ( nCol=nRepeatColStart; nCol<=nRepeatColEnd; nCol++ )
                if (!pDoc->ColHidden(nCol, nTab))
                {
                    sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nTab );
                    long nNextX = nPosX + static_cast<long>(nDocW * nScaleX);

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
                    long nNextX = nPosX + static_cast<long>(nDocW * nScaleX);

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
        rInfo.SetColInfo( 0, nullptr );

    //  get row info

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
            pRowInfo[nRowPos].Set( true, 0, aHeaderRect.Top(), aHeaderRect.Bottom() );
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
                long nNextY = nPosY + static_cast<long>(nDocH * nScaleY);

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
                long nNextY = nPosY + static_cast<long>(nDocH * nScaleY);

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
        rInfo.SetRowInfo( 0, nullptr );

    //  limit to visible area

    rInfo.SetTab( nTab );
    rInfo.LimitToArea( rVisiblePixel );
}

tools::Rectangle ScPreviewLocationData::GetHeaderCellOutputRect(const tools::Rectangle& rVisRect, const ScAddress& rCellPos, bool bColHeader) const
{
    // first a stupid implementation
    // NN says here should be done more
    tools::Rectangle aClipRect;
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
            aClipRect = tools::Rectangle( rColInfo.nPixelStart, rRowInfo.nPixelStart, rColInfo.nPixelEnd, rRowInfo.nPixelEnd );
    }
    return aClipRect;
}

tools::Rectangle ScPreviewLocationData::GetCellOutputRect(const ScAddress& rCellPos) const
{
    // first a stupid implementation
    // NN says here should be done more
    tools::Rectangle aRect;
    GetCellPosition(rCellPos, aRect);
    return aRect;
}

// GetMainCellRange is used for links in PDF export

bool ScPreviewLocationData::GetMainCellRange( ScRange& rRange, tools::Rectangle& rPixRect ) const
{
    for (auto const& it : m_Entries)
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
