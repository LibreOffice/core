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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include "collect.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "column.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "userlist.hxx"
#include "stlsheet.hxx"
#include "global.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "stlsheet.hxx"
#include "brdcst.hxx"
#include "tabprotection.hxx"
#include "globstr.hrc"
#include "segmenttree.hxx"
#include <com/sun/star/sheet/TablePageBreakData.hpp>

#include <algorithm>
#include <limits>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::sheet::TablePageBreakData;
using ::std::set;

// STATIC DATA -----------------------------------------------------------

#define GET_SCALEVALUE(set,id)  ((const SfxUInt16Item&)(set.Get( id ))).GetValue()


void ScTable::UpdatePageBreaks( const ScRange* pUserArea )
{
    if ( pDocument->IsImportingXML() )
        return;

    // pUserArea != NULL -> print area is specified.  We need to force-update
    // the page breaks.

    if (!pUserArea)
    {
        if (!bPageSizeValid)
            return;

        if (mbPageBreaksValid)
            return;
    }

    SfxStyleSheetBase* pStyle = pDocument->GetStyleSheetPool()->
                                    Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );
    if ( !pStyle )
    {
        OSL_FAIL("UpdatePageBreaks: Style nicht gefunden");
        return;
    }
    SfxItemSet* pStyleSet = &pStyle->GetItemSet();
    const SfxPoolItem* pItem;

    SCCOL nX;
    SCCOL nStartCol = 0;
    SCROW nStartRow = 0;
    SCCOL nEndCol = MAXCOL;
    SCROW nEndRow = MAXROW;
    if (pUserArea)
    {
        nStartCol = pUserArea->aStart.Col();
        nStartRow = pUserArea->aStart.Row();
        nEndCol = pUserArea->aEnd.Col();
        nEndRow = pUserArea->aEnd.Row();
    }
    else
    {
        USHORT nAreaCount = GetPrintRangeCount();
        if ( nAreaCount > 1 )
        {
            //  bei mehreren Bereichen nichts anzeigen:

            for (nX=0; nX<MAXCOL; nX++)
                RemoveColBreak(nX, true, false);

            RemoveRowPageBreaks(0, MAXROW-1);

            return;
        }
        else if ( nAreaCount == 1 )
        {
            const ScRange* pArea = GetPrintRange( 0 );
            if (pArea)
            {
                nStartCol = pArea->aStart.Col();
                nStartRow = pArea->aStart.Row();
                nEndCol = pArea->aEnd.Col();
                nEndRow = pArea->aEnd.Row();
            }
        }           // sonst alles
    }

    // get bSkipColBreaks/bSkipRowBreaks flags:

    bool bSkipColBreaks = false;
    bool bSkipRowBreaks = false;
    bool bSkipBreaks = false;

    if ( pStyleSet->GetItemState( ATTR_PAGE_SCALETOPAGES, FALSE, &pItem ) == SFX_ITEM_SET )
    {
        DBG_ASSERT( pItem->ISA(SfxUInt16Item), "falsches Item" );
        bSkipColBreaks = bSkipRowBreaks = ( ((const SfxUInt16Item*)pItem)->GetValue() > 0 );
    }

    if ( !bSkipColBreaks && pStyleSet->GetItemState(ATTR_PAGE_SCALETO, FALSE, &pItem) == SFX_ITEM_SET )
    {
        // #i54993# when fitting to width or height, ignore only manual breaks in that direction
        const ScPageScaleToItem* pScaleToItem = static_cast<const ScPageScaleToItem*>(pItem);
        if ( pScaleToItem->GetWidth() > 0 )
            bSkipColBreaks = true;
        if ( pScaleToItem->GetHeight() > 0 )
            bSkipRowBreaks = true;
    }

    if (!bSkipBreaks && pStyleSet->GetItemState(ATTR_PAGE_SCALETO, false, &pItem) == SFX_ITEM_SET)
    {
        const ScPageScaleToItem& rScaleToItem = static_cast<const ScPageScaleToItem&>(
            pStyleSet->Get(ATTR_PAGE_SCALETO));
        if (rScaleToItem.GetWidth() > 0 || rScaleToItem.GetHeight() > 0)
            // when fitting to a fixed width x height, ignore manual breaks.
            bSkipBreaks = true;
    }

    //--------------------------------------------------------------------------

    long nPageSizeX = aPageSizeTwips.Width();
    long nPageSizeY = aPageSizeTwips.Height();

        //  Anfang: Breaks loeschen

    for (nX=0; nX<nStartCol; nX++)
        RemoveColBreak(nX, true, false);
    RemoveRowPageBreaks(0, nStartRow-1);

    if (nStartCol > 0)
        SetColBreak(nStartCol, true, false);  // AREABREAK
    if (nStartRow > 0)
        SetRowBreak(nStartRow, true, false);  // AREABREAK

        //  Mittelteil: Breaks verteilen

    BOOL bRepeatCol = ( nRepeatStartX != SCCOL_REPEAT_NONE );
    BOOL bColFound = FALSE;
    long nSizeX = 0;
    for (nX=nStartCol; nX<=nEndCol; nX++)
    {
        BOOL bStartOfPage = FALSE;
        long nThisX = ColHidden(nX) ? 0 : pColWidth[nX];
        bool bManualBreak = HasColManualBreak(nX);
        if ( (nSizeX+nThisX > nPageSizeX) || (bManualBreak && !bSkipColBreaks) )
        {
            SetColBreak(nX, true, false);
            nSizeX = 0;
            bStartOfPage = TRUE;
        }
        else if (nX != nStartCol)
            RemoveColBreak(nX, true, false);
        else
            bStartOfPage = TRUE;

        if ( bStartOfPage && bRepeatCol && nX>nRepeatStartX && !bColFound )
        {
            // subtract size of repeat columns from page size
            for (SCCOL i=nRepeatStartX; i<=nRepeatEndX; i++)
                nPageSizeX -= ColHidden(i) ? 0 : pColWidth[i];
            while (nX<=nRepeatEndX)
                RemoveColBreak(++nX, true, false);
            bColFound = TRUE;
        }

        nSizeX += nThisX;
    }

    // Remove all page breaks in range.
    RemoveRowPageBreaks(nStartRow+1, nEndRow);

    // And set new page breaks.
    BOOL bRepeatRow = ( nRepeatStartY != SCROW_REPEAT_NONE );
    BOOL bRowFound = FALSE;
    long nSizeY = 0;
    ScFlatBoolRowSegments::ForwardIterator aIterHidden(*mpHiddenRows);
    ScFlatUInt16RowSegments::ForwardIterator aIterHeights(*mpRowHeights);
    SCROW nNextManualBreak = GetNextManualBreak(nStartRow); // -1 => no more manual breaks
    for (SCROW nY = nStartRow; nY <= nEndRow; ++nY)
    {
        BOOL bStartOfPage = FALSE;
        bool bThisRowHidden = false;
        aIterHidden.getValue(nY, bThisRowHidden);
        long nThisY = 0;
        if (!bThisRowHidden)
        {
            sal_uInt16 nTmp;
            aIterHeights.getValue(nY, nTmp);
            nThisY = static_cast<long>(nTmp);
        }

        bool bManualBreak = false;
        if (nNextManualBreak >= 0)
        {
            bManualBreak = (nY == nNextManualBreak);
            if (nY >= nNextManualBreak)
                // Query the next menual break position.
                nNextManualBreak = GetNextManualBreak(nY+1);
        }

        if ( (nSizeY+nThisY > nPageSizeY) || (bManualBreak && !bSkipRowBreaks) )
        {
            SetRowBreak(nY, true, false);
            nSizeY = 0;
            bStartOfPage = TRUE;
        }
        else if (nY != nStartRow)
            ; // page break already removed
        else
            bStartOfPage = TRUE;

        if ( bStartOfPage && bRepeatRow && nY>nRepeatStartY && !bRowFound )
        {
            // subtract size of repeat rows from page size
            unsigned long nHeights = GetTotalRowHeight(nRepeatStartY, nRepeatEndY);
#ifdef DBG_UTIL
            if (nHeights == ::std::numeric_limits<unsigned long>::max())
                DBG_ERRORFILE("ScTable::UpdatePageBreaks: row heights overflow");
#endif
            nPageSizeY -= nHeights;
            if (nY <= nRepeatEndY)
                RemoveRowPageBreaks(nY, nRepeatEndY);
            bRowFound = TRUE;
        }

        if (bThisRowHidden)
        {
            // Hidden row range.  Skip them unless there is a manual break.
            SCROW nLastCommon = aIterHidden.getLastPos();
            if (nNextManualBreak >= 0)
                nLastCommon = ::std::min(nLastCommon, nNextManualBreak-1);
            nY = nLastCommon;
        }
        else
        {
            // Visible row range.

            SCROW nLastHidden = aIterHidden.getLastPos();
            SCROW nLastHeight = aIterHeights.getLastPos();
            SCROW nLastCommon = ::std::min(nLastHidden, nLastHeight);
            if (nNextManualBreak >= 0)
                nLastCommon = ::std::min(nLastCommon, nNextManualBreak-1);

            if (nLastCommon > nY)
            {
                long nMaxMultiple = static_cast<long>(nLastCommon - nY);
                long nMultiple = (nPageSizeY - nSizeY) / nThisY;
                if (nMultiple > nMaxMultiple)
                    nMultiple = nMaxMultiple;
                if (nMultiple > 1)
                {
                    nSizeY += nThisY * (nMultiple - 1);
                    nY += nMultiple - 1;
                }
            }
        }

        nSizeY += nThisY;
    }

        //  Ende: Breaks loeschen

    if (nEndCol < MAXCOL)
    {
        SetColBreak(nEndCol+1, true, false);  // AREABREAK
        for (nX=nEndCol+2; nX<=MAXCOL; nX++)
            RemoveColBreak(nX, true, false);
    }
    if (nEndRow < MAXROW)
    {
        SetRowBreak(nEndRow+1, true, false);  // AREABREAK
        if (nEndRow+2 <= MAXROW)
            RemoveRowPageBreaks(nEndRow+2, MAXROW);
    }
    mbPageBreaksValid = true;
}

void ScTable::RemoveManualBreaks()
{
    maRowManualBreaks.clear();
    maColManualBreaks.clear();
    InvalidatePageBreaks();

    if (IsStreamValid())
        SetStreamValid(FALSE);
}

BOOL ScTable::HasManualBreaks() const
{
    return !maRowManualBreaks.empty() || !maColManualBreaks.empty();
}

void ScTable::SetRowManualBreaks( const ::std::set<SCROW>& rBreaks )
{
    maRowManualBreaks = rBreaks;
    InvalidatePageBreaks();
    if (IsStreamValid())
        SetStreamValid(FALSE);
}

void ScTable::SetColManualBreaks( const ::std::set<SCCOL>& rBreaks )
{
    maColManualBreaks = rBreaks;
    InvalidatePageBreaks();
    if (IsStreamValid())
        SetStreamValid(FALSE);
}

void ScTable::GetAllRowBreaks(set<SCROW>& rBreaks, bool bPage, bool bManual) const
{
    if (bPage)
        rBreaks = maRowPageBreaks;

    if (bManual)
    {
        using namespace std;
        copy(maRowManualBreaks.begin(), maRowManualBreaks.end(), inserter(rBreaks, rBreaks.begin()));
    }
}

void ScTable::GetAllColBreaks(set<SCCOL>& rBreaks, bool bPage, bool bManual) const
{
    if (bPage)
        rBreaks = maColPageBreaks;

    if (bManual)
    {
        using namespace std;
        copy(maColManualBreaks.begin(), maColManualBreaks.end(), inserter(rBreaks, rBreaks.begin()));
    }
}

bool ScTable::HasRowPageBreak(SCROW nRow) const
{
    if (!ValidRow(nRow))
        return false;

    return maRowPageBreaks.find(nRow) != maRowPageBreaks.end();
}

bool ScTable::HasColPageBreak(SCCOL nCol) const
{
    if (!ValidCol(nCol))
        return false;

    return maColPageBreaks.find(nCol) != maColPageBreaks.end();
}

bool ScTable::HasRowManualBreak(SCROW nRow) const
{
    if (!ValidRow(nRow))
        return false;

    return maRowManualBreaks.find(nRow) != maRowManualBreaks.end();
}

bool ScTable::HasColManualBreak(SCCOL nCol) const
{
    if (!ValidCol(nCol))
        return false;

    return maColManualBreaks.find(nCol) != maColManualBreaks.end();
}

SCROW ScTable::GetNextManualBreak(SCROW nRow) const
{
    set<SCROW>::const_iterator itr = maRowManualBreaks.lower_bound(nRow);
    return itr == maRowManualBreaks.end() ? -1 : *itr;
}

void ScTable::RemoveRowPageBreaks(SCROW nStartRow, SCROW nEndRow)
{
    using namespace std;

    if (!ValidRow(nStartRow) || !ValidRow(nEndRow))
        return;

    set<SCROW>::iterator low  = maRowPageBreaks.lower_bound(nStartRow);
    set<SCROW>::iterator high = maRowPageBreaks.upper_bound(nEndRow);
    maRowPageBreaks.erase(low, high);
}

void ScTable::RemoveRowBreak(SCROW nRow, bool bPage, bool bManual)
{
    if (!ValidRow(nRow))
        return;

    if (bPage)
        maRowPageBreaks.erase(nRow);

    if (bManual)
    {
        maRowManualBreaks.erase(nRow);
        InvalidatePageBreaks();
    }
}

void ScTable::RemoveColBreak(SCCOL nCol, bool bPage, bool bManual)
{
    if (!ValidCol(nCol))
        return;

    if (bPage)
        maColPageBreaks.erase(nCol);

    if (bManual)
    {
        maColManualBreaks.erase(nCol);
        InvalidatePageBreaks();
    }
}

void ScTable::SetRowBreak(SCROW nRow, bool bPage, bool bManual)
{
    if (!ValidRow(nRow))
        return;

    if (bPage)
        maRowPageBreaks.insert(nRow);

    if (bManual)
    {
        maRowManualBreaks.insert(nRow);
        InvalidatePageBreaks();
    }
}

void ScTable::SetColBreak(SCCOL nCol, bool bPage, bool bManual)
{
    if (!ValidCol(nCol))
        return;

    if (bPage)
        maColPageBreaks.insert(nCol);

    if (bManual)
    {
        maColManualBreaks.insert(nCol);
        InvalidatePageBreaks();
    }
}

Sequence<TablePageBreakData> ScTable::GetRowBreakData() const
{
    using ::std::copy;
    using ::std::inserter;

    set<SCROW> aRowBreaks = maRowPageBreaks;
    copy(maRowManualBreaks.begin(), maRowManualBreaks.end(), inserter(aRowBreaks, aRowBreaks.begin()));

    set<SCROW>::const_iterator itr = aRowBreaks.begin(), itrEnd = aRowBreaks.end();
    Sequence<TablePageBreakData> aSeq(aRowBreaks.size());

    for (sal_Int32 i = 0; itr != itrEnd; ++itr, ++i)
    {
        SCROW nRow = *itr;
        TablePageBreakData aData;
        aData.Position = nRow;
        aData.ManualBreak = HasRowManualBreak(nRow);
        aSeq[i] = aData;
    }

    return aSeq;
}

bool ScTable::RowHidden(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!ValidRow(nRow))
    {
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mpHiddenRows->getRangeData(nRow, aData))
    {
        // search failed.
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    if (pFirstRow)
        *pFirstRow = aData.mnRow1;
    if (pLastRow)
        *pLastRow = aData.mnRow2;

    return aData.mbValue;
}

bool ScTable::RowHiddenLeaf(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!ValidRow(nRow))
    {
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mpHiddenRows->getRangeDataLeaf(nRow, aData))
    {
        // search failed.
        if (pFirstRow)
            *pFirstRow = nRow;
        if (pLastRow)
            *pLastRow = nRow;
        return true;
    }

    if (pFirstRow)
        *pFirstRow = aData.mnRow1;
    if (pLastRow)
        *pLastRow = aData.mnRow2;

    return aData.mbValue;
}

bool ScTable::HasHiddenRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = -1;
        bool bHidden = RowHidden(nRow, NULL, &nLastRow);
        if (bHidden)
            return true;

        nRow = nLastRow + 1;
    }
    return false;
}

bool ScTable::ColHidden(SCCOL nCol, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    if (!ValidCol(nCol))
        return true;

    ScFlatBoolColSegments::RangeData aData;
    if (!mpHiddenCols->getRangeData(nCol, aData))
        return true;

    if (pFirstCol)
        *pFirstCol = aData.mnCol1;
    if (pLastCol)
        *pLastCol = aData.mnCol2;

    return aData.mbValue;
}

bool ScTable::SetRowHidden(SCROW nStartRow, SCROW nEndRow, bool bHidden)
{
    bool bChanged = false;
    if (bHidden)
        bChanged = mpHiddenRows->setTrue(nStartRow, nEndRow);
    else
        bChanged = mpHiddenRows->setFalse(nStartRow, nEndRow);

    if (bChanged)
    {
        if (IsStreamValid())
            SetStreamValid(false);
    }

    return bChanged;
}

bool ScTable::SetColHidden(SCCOL nStartCol, SCCOL nEndCol, bool bHidden)
{
    bool bChanged = false;
    if (bHidden)
        bChanged = mpHiddenCols->setTrue(nStartCol, nEndCol);
    else
        bChanged = mpHiddenCols->setFalse(nStartCol, nEndCol);

    if (bChanged)
    {
        if (IsStreamValid())
            SetStreamValid(false);
    }

    return bChanged;
}

void ScTable::CopyColHidden(ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol)
{
    SCCOL nCol = nStartCol;
    while (nCol <= nEndCol)
    {
        SCCOL nLastCol;
        bool bHidden = rTable.ColHidden(nCol, NULL, &nLastCol);
        if (nLastCol > nEndCol)
            nLastCol = nEndCol;

        SetColHidden(nCol, nLastCol, bHidden);
        nCol = nLastCol + 1;
    }
}

void ScTable::CopyRowHidden(ScTable& rTable, SCROW nStartRow, SCROW nEndRow)
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = -1;
        bool bHidden = rTable.RowHidden(nRow, NULL, &nLastRow);
        if (nLastRow > nEndRow)
            nLastRow = nEndRow;
        SetRowHidden(nRow, nLastRow, bHidden);
        nRow = nLastRow + 1;
    }
}

void ScTable::CopyRowHeight(ScTable& rSrcTable, SCROW nStartRow, SCROW nEndRow, SCROW nSrcOffset)
{
    SCROW nRow = nStartRow;
    ScFlatUInt16RowSegments::RangeData aSrcData;
    while (nRow <= nEndRow)
    {
        if (!rSrcTable.mpRowHeights->getRangeData(nRow + nSrcOffset, aSrcData))
            // Something is wrong !
            return;

        SCROW nLastRow = aSrcData.mnRow2 - nSrcOffset;
        if (nLastRow > nEndRow)
            nLastRow = nEndRow;

        mpRowHeights->setValue(nRow, nLastRow, aSrcData.mnValue);
        nRow = nLastRow + 1;
    }
}

SCROW ScTable::FirstVisibleRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!ValidRow(nRow))
            break;

        if (!mpHiddenRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // visible row found
            return nRow;

        nRow = aData.mnRow2 + 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW ScTable::LastVisibleRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nEndRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow >= nStartRow)
    {
        if (!ValidRow(nRow))
            break;

        if (!mpHiddenRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // visible row found
            return nRow;

        nRow = aData.mnRow1 - 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW ScTable::CountVisibleRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nCount = 0;
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!mpHiddenRows->getRangeData(nRow, aData))
            break;

        if (aData.mnRow2 > nEndRow)
            aData.mnRow2 = nEndRow;

        if (!aData.mbValue)
            nCount += aData.mnRow2 - nRow + 1;

        nRow = aData.mnRow2 + 1;
    }
    return nCount;
}

sal_uInt32 ScTable::GetTotalRowHeight(SCROW nStartRow, SCROW nEndRow) const
{
    sal_uInt32 nHeight = 0;
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!mpHiddenRows->getRangeData(nRow, aData))
            break;

        if (aData.mnRow2 > nEndRow)
            aData.mnRow2 = nEndRow;

        if (!aData.mbValue)
            // visible row range.
            nHeight += mpRowHeights->getSumValue(nRow, aData.mnRow2);

        nRow = aData.mnRow2 + 1;
    }

    return nHeight;
}

SCCOLROW ScTable::LastHiddenColRow(SCCOLROW nPos, bool bCol) const
{
    if (bCol)
    {
        SCCOL nCol = static_cast<SCCOL>(nPos);
        if (ColHidden(nCol))
        {
            for (SCCOL i = nCol+1; i <= MAXCOL; ++i)
            {
                if (!ColHidden(nCol))
                    return nCol - 1;
            }
        }
    }
    else
    {
        SCROW nRow = static_cast<SCROW>(nPos);
        SCROW nLastRow;
        if (RowHidden(nRow, NULL, &nLastRow))
            return static_cast<SCCOLROW>(nLastRow);
    }
    return ::std::numeric_limits<SCCOLROW>::max();
}

bool ScTable::RowFiltered(SCROW nRow, SCROW* pFirstRow, SCROW* pLastRow) const
{
    if (!ValidRow(nRow))
        return false;

    ScFlatBoolRowSegments::RangeData aData;
    if (!mpFilteredRows->getRangeData(nRow, aData))
        // search failed.
        return false;

    if (pFirstRow)
        *pFirstRow = aData.mnRow1;
    if (pLastRow)
        *pLastRow = aData.mnRow2;

    return aData.mbValue;
}

bool ScTable::ColFiltered(SCCOL nCol, SCCOL* pFirstCol, SCCOL* pLastCol) const
{
    if (!ValidCol(nCol))
        return false;

    ScFlatBoolColSegments::RangeData aData;
    if (!mpFilteredCols->getRangeData(nCol, aData))
        // search failed.
        return false;

    if (pFirstCol)
        *pFirstCol = aData.mnCol1;
    if (pLastCol)
        *pLastCol = aData.mnCol2;

    return aData.mbValue;
}

bool ScTable::HasFilteredRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = nRow;
        bool bFiltered = RowFiltered(nRow, NULL, &nLastRow);
        if (bFiltered)
            return true;

        nRow = nLastRow + 1;
    }
    return false;
}

void ScTable::CopyColFiltered(ScTable& rTable, SCCOL nStartCol, SCCOL nEndCol)
{
    SCCOL nCol = nStartCol;
    while (nCol <= nEndCol)
    {
        SCCOL nLastCol;
        bool bFiltered = rTable.ColFiltered(nCol, NULL, &nLastCol);
        if (nLastCol > nEndCol)
            nLastCol = nEndCol;

        SetColFiltered(nCol, nLastCol, bFiltered);
        nCol = nLastCol + 1;
    }
}

void ScTable::CopyRowFiltered(ScTable& rTable, SCROW nStartRow, SCROW nEndRow)
{
    SCROW nRow = nStartRow;
    while (nRow <= nEndRow)
    {
        SCROW nLastRow = -1;
        bool bFiltered = rTable.RowFiltered(nRow, NULL, &nLastRow);
        if (nLastRow > nEndRow)
            nLastRow = nEndRow;
        SetRowFiltered(nRow, nLastRow, bFiltered);
        nRow = nLastRow + 1;
    }
}

void ScTable::SetRowFiltered(SCROW nStartRow, SCROW nEndRow, bool bFiltered)
{
    if (bFiltered)
        mpFilteredRows->setTrue(nStartRow, nEndRow);
    else
        mpFilteredRows->setFalse(nStartRow, nEndRow);
}

void ScTable::SetColFiltered(SCCOL nStartCol, SCCOL nEndCol, bool bFiltered)
{
    if (bFiltered)
        mpFilteredCols->setTrue(nStartCol, nEndCol);
    else
        mpFilteredCols->setFalse(nStartCol, nEndCol);
}

SCROW ScTable::FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!ValidRow(nRow))
            break;

        if (!mpFilteredRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // non-filtered row found
            return nRow;

        nRow = aData.mnRow2 + 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW ScTable::LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nRow = nEndRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow >= nStartRow)
    {
        if (!ValidRow(nRow))
            break;

        if (!mpFilteredRows->getRangeData(nRow, aData))
            // failed to get range data.
            break;

        if (!aData.mbValue)
            // non-filtered row found
            return nRow;

        nRow = aData.mnRow1 - 1;
    }

    return ::std::numeric_limits<SCROW>::max();
}

SCROW ScTable::CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow) const
{
    SCROW nCount = 0;
    SCROW nRow = nStartRow;
    ScFlatBoolRowSegments::RangeData aData;
    while (nRow <= nEndRow)
    {
        if (!mpFilteredRows->getRangeData(nRow, aData))
            break;

        if (aData.mnRow2 > nEndRow)
            aData.mnRow2 = nEndRow;

        if (!aData.mbValue)
            nCount += aData.mnRow2 - nRow + 1;

        nRow = aData.mnRow2 + 1;
    }
    return nCount;
}

namespace {

void lcl_syncFlags(ScFlatBoolColSegments& rColSegments, ScFlatBoolRowSegments& rRowSegments,
    BYTE* pColFlags, ScBitMaskCompressedArray< SCROW, BYTE>* pRowFlags, const BYTE nFlagMask)
{
    using ::sal::static_int_cast;

    pRowFlags->AndValue(0, MAXROW, static_int_cast<BYTE>(~nFlagMask));
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        pColFlags[i] &= static_int_cast<BYTE>(~nFlagMask);

    {
        // row hidden flags.

        SCROW nRow = 0;
        ScFlatBoolRowSegments::RangeData aData;
        while (nRow <= MAXROW)
        {
            if (!rRowSegments.getRangeData(nRow, aData))
                break;

            if (aData.mbValue)
                pRowFlags->OrValue(nRow, aData.mnRow2, static_int_cast<BYTE>(nFlagMask));

            nRow = aData.mnRow2 + 1;
        }
    }

    {
        // column hidden flags.

        SCCOL nCol = 0;
        ScFlatBoolColSegments::RangeData aData;
        while (nCol <= MAXCOL)
        {
            if (!rColSegments.getRangeData(nCol, aData))
                break;

            if (aData.mbValue)
            {
                for (SCCOL i = nCol; i <= aData.mnCol2; ++i)
                    pColFlags[i] |= nFlagMask;
            }

            nCol = aData.mnCol2 + 1;
        }
    }
}

}

void ScTable::SyncColRowFlags()
{
    using ::sal::static_int_cast;

    // Manual breaks.
    pRowFlags->AndValue(0, MAXROW, static_int_cast<BYTE>(~CR_MANUALBREAK));
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        pColFlags[i] &= static_int_cast<BYTE>(~CR_MANUALBREAK);

    if (!maRowManualBreaks.empty())
    {
        for (set<SCROW>::const_iterator itr = maRowManualBreaks.begin(), itrEnd = maRowManualBreaks.end();
              itr != itrEnd; ++itr)
            pRowFlags->OrValue(*itr, static_int_cast<BYTE>(CR_MANUALBREAK));
    }

    if (!maColManualBreaks.empty())
    {
        for (set<SCCOL>::const_iterator itr = maColManualBreaks.begin(), itrEnd = maColManualBreaks.end();
              itr != itrEnd; ++itr)
            pColFlags[*itr] |= CR_MANUALBREAK;
    }

    // Hidden flags.
    lcl_syncFlags(*mpHiddenCols, *mpHiddenRows, pColFlags, pRowFlags, CR_HIDDEN);
    lcl_syncFlags(*mpFilteredCols, *mpFilteredRows, pColFlags, pRowFlags, CR_FILTERED);
}

void ScTable::SetPageSize( const Size& rSize )
{
    if ( rSize.Width() != 0 && rSize.Height() != 0 )
    {
        if (aPageSizeTwips != rSize)
            InvalidatePageBreaks();

        bPageSizeValid = TRUE;
        aPageSizeTwips = rSize;
    }
    else
        bPageSizeValid = FALSE;
}

BOOL ScTable::IsProtected() const
{
    return pTabProtection.get() && pTabProtection->isProtected();
}

void ScTable::SetProtection(const ScTableProtection* pProtect)
{
    if (pProtect)
        pTabProtection.reset(new ScTableProtection(*pProtect));
    else
        pTabProtection.reset(NULL);

    if (IsStreamValid())
        SetStreamValid(FALSE);
}

ScTableProtection* ScTable::GetProtection()
{
    return pTabProtection.get();
}

Size ScTable::GetPageSize() const
{
    if ( bPageSizeValid )
        return aPageSizeTwips;
    else
        return Size();  // leer
}

void ScTable::SetRepeatArea( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow )
{
    nRepeatStartX = nStartCol;
    nRepeatEndX   = nEndCol;
    nRepeatStartY = nStartRow;
    nRepeatEndY   = nEndRow;
}

void ScTable::StartListening( const ScAddress& rAddress, SvtListener* pListener )
{
    aCol[rAddress.Col()].StartListening( *pListener, rAddress.Row() );
}

void ScTable::EndListening( const ScAddress& rAddress, SvtListener* pListener )
{
    aCol[rAddress.Col()].EndListening( *pListener, rAddress.Row() );
}

void ScTable::SetPageStyle( const String& rName )
{
    if ( aPageStyle != rName )
    {
        String                  aStrNew    = rName;
        SfxStyleSheetBasePool*  pStylePool = pDocument->GetStyleSheetPool();
        SfxStyleSheetBase*      pNewStyle  = pStylePool->Find( aStrNew, SFX_STYLE_FAMILY_PAGE );

        if ( !pNewStyle )
        {
            aStrNew = ScGlobal::GetRscString(STR_STYLENAME_STANDARD);
            pNewStyle = pStylePool->Find( aStrNew, SFX_STYLE_FAMILY_PAGE );
        }

        if ( aPageStyle != aStrNew )
        {
            SfxStyleSheetBase* pOldStyle = pStylePool->Find( aPageStyle, SFX_STYLE_FAMILY_PAGE );

            if ( pOldStyle && pNewStyle )
            {
                SfxItemSet&  rOldSet          = pOldStyle->GetItemSet();
                SfxItemSet&  rNewSet          = pNewStyle->GetItemSet();
                const USHORT nOldScale        = GET_SCALEVALUE(rOldSet,ATTR_PAGE_SCALE);
                const USHORT nOldScaleToPages = GET_SCALEVALUE(rOldSet,ATTR_PAGE_SCALETOPAGES);
                const USHORT nNewScale        = GET_SCALEVALUE(rNewSet,ATTR_PAGE_SCALE);
                const USHORT nNewScaleToPages = GET_SCALEVALUE(rNewSet,ATTR_PAGE_SCALETOPAGES);

                if ( (nOldScale != nNewScale) || (nOldScaleToPages != nNewScaleToPages) )
                    InvalidateTextWidth(NULL, NULL, FALSE, FALSE);
            }

            if ( pNewStyle )            // auch ohne den alten (fuer UpdateStdNames)
                aPageStyle = aStrNew;

            if (IsStreamValid())
                SetStreamValid(FALSE);
        }
    }
}

void ScTable::PageStyleModified( const String& rNewName )
{
    aPageStyle = rNewName;
    InvalidateTextWidth(NULL, NULL, FALSE, FALSE);      // don't know what was in the style before
}

void ScTable::InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo,
                                   BOOL bNumFormatChanged, BOOL bBroadcast )
{
    if ( pAdrFrom && !pAdrTo )
    {
        ScBaseCell* pCell = aCol[pAdrFrom->Col()].GetCell( pAdrFrom->Row() );
        if ( pCell )
        {
            pCell->SetTextWidth( TEXTWIDTH_DIRTY );
            if ( bNumFormatChanged )
                pCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
            if ( bBroadcast )
            {   // nur bei CalcAsShown
                switch ( pCell->GetCellType() )
                {
                    case CELLTYPE_VALUE :
                        pDocument->Broadcast( SC_HINT_DATACHANGED,
                            ScAddress( pAdrFrom->Col(), pAdrFrom->Row(), nTab ),
                            pCell );
                        break;
                    case CELLTYPE_FORMULA :
                        ((ScFormulaCell*)pCell)->SetDirty();
                        break;
                    default:
                    {
                        // added to avoid warnings
                    }
                }
            }
        }
    }
    else
    {
        const SCCOL nColStart = pAdrFrom ? pAdrFrom->Col() : 0;
        const SCROW nRowStart = pAdrFrom ? pAdrFrom->Row() : 0;
        const SCCOL nColEnd   = pAdrTo   ? pAdrTo->Col()   : MAXCOL;
        const SCROW nRowEnd   = pAdrTo   ? pAdrTo->Row()   : MAXROW;

        for ( SCCOL nCol=nColStart; nCol<=nColEnd; nCol++ )
        {
            ScColumnIterator aIter( &aCol[nCol], nRowStart, nRowEnd );
            ScBaseCell*      pCell = NULL;
            SCROW            nRow  = nRowStart;

            while ( aIter.Next( nRow, pCell ) )
            {
                pCell->SetTextWidth( TEXTWIDTH_DIRTY );
                if ( bNumFormatChanged )
                    pCell->SetScriptType( SC_SCRIPTTYPE_UNKNOWN );
                if ( bBroadcast )
                {   // nur bei CalcAsShown
                    switch ( pCell->GetCellType() )
                    {
                        case CELLTYPE_VALUE :
                            pDocument->Broadcast( SC_HINT_DATACHANGED,
                                ScAddress( nCol, nRow, nTab ), pCell );
                            break;
                        case CELLTYPE_FORMULA :
                            ((ScFormulaCell*)pCell)->SetDirty();
                            break;
                        default:
                        {
                            // added to avoid warnings
                        }
                    }
                }
            }
        }
    }
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
