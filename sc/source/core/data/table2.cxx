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

#include <algorithm>
#include <memory>
#include <table.hxx>
#include <patattr.hxx>
#include <docpool.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <olinetab.hxx>
#include <rechead.hxx>
#include <stlpool.hxx>
#include <attarray.hxx>
#include <markdata.hxx>
#include <progress.hxx>
#include <dociter.hxx>
#include <conditio.hxx>
#include <chartlis.hxx>
#include <fillinfo.hxx>
#include <bcaslot.hxx>
#include <postit.hxx>
#include <sheetevents.hxx>
#include <segmenttree.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <dbdata.hxx>
#include <colorscale.hxx>
#include <tokenarray.hxx>
#include <clipcontext.hxx>
#include <types.hxx>
#include <editutil.hxx>
#include <mtvcellfunc.hxx>
#include <refupdatecontext.hxx>
#include <scopetools.hxx>
#include <tabprotection.hxx>
#include <columnspanset.hxx>
#include <rowheightcontext.hxx>
#include <refhint.hxx>
#include <listenercontext.hxx>
#include <token.hxx>
#include <compressedarray.hxx>

#include <scitems.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/editobj.hxx>
#include <svl/poolcach.hxx>
#include <unotools/charclass.hxx>
#include <math.h>
#include <svl/PasswordHelper.hxx>
#include <unotools/transliterationwrapper.hxx>

namespace {

class ColumnRegroupFormulaCells
{
    ScColContainer& mrCols;
    std::vector<ScAddress>* mpGroupPos;

public:
    ColumnRegroupFormulaCells( ScColContainer& rCols, std::vector<ScAddress>* pGroupPos ) :
        mrCols(rCols), mpGroupPos(pGroupPos) {}

    void operator() (SCCOL nCol)
    {
        mrCols[nCol].RegroupFormulaCells(mpGroupPos);
    }
};

}

sal_uInt16 ScTable::GetTextWidth(SCCOL nCol, SCROW nRow) const
{
    return aCol[nCol].GetTextWidth(nRow);
}

bool ScTable::SetOutlineTable( const ScOutlineTable* pNewOutline )
{
    sal_uInt16 nOldSizeX = 0;
    sal_uInt16 nOldSizeY = 0;
    sal_uInt16 nNewSizeX = 0;
    sal_uInt16 nNewSizeY = 0;

    if (pOutlineTable)
    {
        nOldSizeX = pOutlineTable->GetColArray().GetDepth();
        nOldSizeY = pOutlineTable->GetRowArray().GetDepth();
        pOutlineTable.reset();
    }

    if (pNewOutline)
    {
        pOutlineTable.reset(new ScOutlineTable( *pNewOutline ));
        nNewSizeX = pOutlineTable->GetColArray().GetDepth();
        nNewSizeY = pOutlineTable->GetRowArray().GetDepth();
    }

    return ( nNewSizeX != nOldSizeX || nNewSizeY != nOldSizeY );        // changed size?
}

void ScTable::StartOutlineTable()
{
    if (!pOutlineTable)
        pOutlineTable.reset(new ScOutlineTable);
}

void ScTable::SetSheetEvents( std::unique_ptr<ScSheetEvents> pNew )
{
    pSheetEvents = std::move(pNew);

    SetCalcNotification( false );       // discard notifications before the events were set

    SetStreamValid(false);
}

void ScTable::SetCalcNotification( bool bSet )
{
    bCalcNotification = bSet;
}

bool ScTable::TestInsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize ) const
{
    bool bTest = true;

    if ( nStartCol==0 && nEndCol==MAXCOL && pOutlineTable )
        bTest = pOutlineTable->TestInsertRow(nSize);

    for (SCCOL i=nStartCol; (i<=nEndCol) && bTest; i++)
        bTest = aCol[i].TestInsertRow(nStartRow, nSize);

    return bTest;
}

void ScTable::InsertRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize )
{
    if (nStartCol==0 && nEndCol==MAXCOL)
    {
        if (mpRowHeights && pRowFlags)
        {
            mpRowHeights->insertSegment(nStartRow, nSize);
            CRFlags nNewFlags = pRowFlags->Insert( nStartRow, nSize);
            // only copy manual size flag, clear all others
            if (nNewFlags != CRFlags::NONE && (nNewFlags != CRFlags::ManualSize))
                pRowFlags->SetValue( nStartRow, nStartRow + nSize - 1,
                        nNewFlags & CRFlags::ManualSize);
        }

        if (pOutlineTable)
            pOutlineTable->InsertRow( nStartRow, nSize );

        mpFilteredRows->insertSegment(nStartRow, nSize);
        mpHiddenRows->insertSegment(nStartRow, nSize);

        if (!maRowManualBreaks.empty())
        {
            // Copy all breaks up to nStartRow (non-inclusive).
            ::std::set<SCROW>::iterator itr1 = maRowManualBreaks.lower_bound(nStartRow);
            ::std::set<SCROW> aNewBreaks(maRowManualBreaks.begin(), itr1);

            // Copy all breaks from nStartRow (inclusive) to the last element,
            // but add nSize to each value.
            ::std::set<SCROW>::iterator itr2 = maRowManualBreaks.end();
            for (; itr1 != itr2; ++itr1)
                aNewBreaks.insert(static_cast<SCROW>(*itr1 + nSize));

            maRowManualBreaks.swap(aNewBreaks);
        }
    }

    for (SCCOL j=nStartCol; j<=nEndCol; j++)
        aCol[j].InsertRow( nStartRow, nSize );

    mpCondFormatList->InsertRow(nTab, nStartCol, nEndCol, nStartRow, nSize);

    InvalidatePageBreaks();

    // TODO: In the future we may want to check if the table has been
    // really modified before setting the stream invalid.
    SetStreamValid(false);
}

void ScTable::DeleteRow(
    const sc::ColumnSet& rRegroupCols, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
    bool* pUndoOutline, std::vector<ScAddress>* pGroupPos )
{
    if (nStartCol==0 && nEndCol==MAXCOL)
    {
        if (pRowFlags)
            pRowFlags->Remove( nStartRow, nSize);

        if (mpRowHeights)
            mpRowHeights->removeSegment(nStartRow, nStartRow+nSize);

        if (pOutlineTable)
            if (pOutlineTable->DeleteRow( nStartRow, nSize ))
                if (pUndoOutline)
                    *pUndoOutline = true;

        mpFilteredRows->removeSegment(nStartRow, nStartRow+nSize);
        mpHiddenRows->removeSegment(nStartRow, nStartRow+nSize);

        if (!maRowManualBreaks.empty())
        {
            // Erase all manual breaks between nStartRow and nStartRow + nSize - 1 (inclusive).
            std::set<SCROW>::iterator itr1 = maRowManualBreaks.lower_bound(nStartRow);
            std::set<SCROW>::iterator itr2 = maRowManualBreaks.upper_bound(static_cast<SCROW>(nStartRow + nSize - 1));
            maRowManualBreaks.erase(itr1, itr2);

            // Copy all breaks from the 1st element up to nStartRow to the new container.
            itr1 = maRowManualBreaks.lower_bound(nStartRow);
            ::std::set<SCROW> aNewBreaks(maRowManualBreaks.begin(), itr1);

            // Copy all breaks from nStartRow to the last element, but subtract each value by nSize.
            itr2 = maRowManualBreaks.end();
            for (; itr1 != itr2; ++itr1)
                aNewBreaks.insert(static_cast<SCROW>(*itr1 - nSize));

            maRowManualBreaks.swap(aNewBreaks);
        }
    }

    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM(), SfxHintId::ScDataChanged);
        for (SCCOL j=nStartCol; j<=nEndCol; j++)
            aCol[j].DeleteRow(nStartRow, nSize, pGroupPos);
    }

    std::vector<SCCOL> aRegroupCols;
    rRegroupCols.getColumns(nTab, aRegroupCols);
    std::for_each(
        aRegroupCols.begin(), aRegroupCols.end(), ColumnRegroupFormulaCells(aCol, pGroupPos));

    InvalidatePageBreaks();

    // TODO: In the future we may want to check if the table has been
    // really modified before setting the stream invalid.
    SetStreamValid(false);
}

bool ScTable::TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize ) const
{
    if ( nSize > static_cast<SCSIZE>(MAXCOL) )
        return false;

    if ( nStartRow==0 && nEndRow==MAXROW && pOutlineTable
        && ! pOutlineTable->TestInsertCol(nSize) )
            return false;

    auto range = GetColumnsRange( MAXCOL - static_cast<SCCOL>(nSize) + 1, MAXCOL );
    for (auto it = range.rbegin(); it != range.rend(); ++it )
        if (! aCol[*it].TestInsertCol(nStartRow, nEndRow))
            return false;

    return true;
}

void ScTable::InsertCol(
    const sc::ColumnSet& rRegroupCols, SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize )
{
    if (nStartRow==0 && nEndRow==MAXROW)
    {
        if (mpColWidth && mpColFlags)
        {
            mpColWidth->InsertPreservingSize(nStartCol, nSize, STD_COL_WIDTH);
            // The inserted columns have the same widths as the columns, which were selected for insert.
            for (SCSIZE i=0; i < std::min(MAXCOL-nSize-nStartCol, nSize); ++i)
                mpColWidth->SetValue(nStartCol + i, mpColWidth->GetValue(nStartCol+i+nSize));
            mpColFlags->InsertPreservingSize(nStartCol, nSize, CRFlags::NONE);
        }
        if (pOutlineTable)
            pOutlineTable->InsertCol( nStartCol, nSize );

        mpHiddenCols->insertSegment(nStartCol, static_cast<SCCOL>(nSize));
        mpFilteredCols->insertSegment(nStartCol, static_cast<SCCOL>(nSize));

        if (!maColManualBreaks.empty())
        {
            // Copy all breaks up to nStartCol (non-inclusive).
            ::std::set<SCCOL>::iterator itr1 = maColManualBreaks.lower_bound(nStartCol);
            ::std::set<SCCOL> aNewBreaks(maColManualBreaks.begin(), itr1);

            // Copy all breaks from nStartCol (inclusive) to the last element,
            // but add nSize to each value.
            ::std::set<SCCOL>::iterator itr2 = maColManualBreaks.end();
            for (; itr1 != itr2; ++itr1)
                aNewBreaks.insert(static_cast<SCCOL>(*itr1 + nSize));

            maColManualBreaks.swap(aNewBreaks);
        }
    }

    if ((nStartRow == 0) && (nEndRow == MAXROW))
    {
        for (SCSIZE i=0; i < nSize; i++)
            for (SCCOL nCol = aCol.size() - 1; nCol > nStartCol; nCol--)
                aCol[nCol].SwapCol(aCol[nCol-1]);
    }
    else
    {
        for (SCSIZE i=0; static_cast<SCCOL>(i+nSize)+nStartCol < aCol.size(); i++)
            aCol[aCol.size() - 1 - nSize - i].MoveTo(nStartRow, nEndRow, aCol[aCol.size() - 1 - i]);
    }

    std::vector<SCCOL> aRegroupCols;
    rRegroupCols.getColumns(nTab, aRegroupCols);
    std::for_each(aRegroupCols.begin(), aRegroupCols.end(), ColumnRegroupFormulaCells(aCol, nullptr));

    if (nStartCol>0)                        // copy old attributes
    {
        sal_uInt16 nWhichArray[2];
        nWhichArray[0] = ATTR_MERGE;
        nWhichArray[1] = 0;

        sc::CopyToDocContext aCxt(*pDocument);
        for (SCSIZE i=0; i<nSize; i++)
        {
            aCol[nStartCol-1].CopyToColumn(aCxt, nStartRow, nEndRow, InsertDeleteFlags::ATTRIB,
                                                false, aCol[nStartCol+i] );
            aCol[nStartCol+i].RemoveFlags( nStartRow, nEndRow,
                                                ScMF::Hor | ScMF::Ver | ScMF::Auto );
            aCol[nStartCol+i].ClearItems( nStartRow, nEndRow, nWhichArray );
        }
    }

    mpCondFormatList->InsertCol(nTab, nStartRow, nEndRow, nStartCol, nSize);

    InvalidatePageBreaks();

    // TODO: In the future we may want to check if the table has been
    // really modified before setting the stream invalid.
    SetStreamValid(false);
}

void ScTable::DeleteCol(
    const sc::ColumnSet& rRegroupCols, SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize, bool* pUndoOutline )
{
    if (nStartRow==0 && nEndRow==MAXROW)
    {
        if (mpColWidth && mpColFlags)
        {
            assert( nStartCol + nSize <= MAXCOL+1 );    // moving 0 if ==MAXCOL+1 is correct
            mpColWidth->RemovePreservingSize(nStartCol, nSize, STD_COL_WIDTH);
            mpColFlags->RemovePreservingSize(nStartCol, nSize, CRFlags::NONE);
        }
        if (pOutlineTable)
            if (pOutlineTable->DeleteCol( nStartCol, nSize ))
                if (pUndoOutline)
                    *pUndoOutline = true;

        SCCOL nRmSize = nStartCol + static_cast<SCCOL>(nSize);
        mpHiddenCols->removeSegment(nStartCol, nRmSize);
        mpFilteredCols->removeSegment(nStartCol, nRmSize);

        if (!maColManualBreaks.empty())
        {
            // Erase all manual breaks between nStartCol and nStartCol + nSize - 1 (inclusive).
            std::set<SCCOL>::iterator itr1 = maColManualBreaks.lower_bound(nStartCol);
            std::set<SCCOL>::iterator itr2 = maColManualBreaks.upper_bound(static_cast<SCCOL>(nStartCol + nSize - 1));
            maColManualBreaks.erase(itr1, itr2);

            // Copy all breaks from the 1st element up to nStartCol to the new container.
            itr1 = maColManualBreaks.lower_bound(nStartCol);
            ::std::set<SCCOL> aNewBreaks(maColManualBreaks.begin(), itr1);

            // Copy all breaks from nStartCol to the last element, but subtract each value by nSize.
            itr2 = maColManualBreaks.end();
            for (; itr1 != itr2; ++itr1)
                aNewBreaks.insert(static_cast<SCCOL>(*itr1 - nSize));

            maColManualBreaks.swap(aNewBreaks);
        }
    }

    for (SCSIZE i = 0; i < nSize; i++)
        aCol[nStartCol + i].DeleteArea(nStartRow, nEndRow, InsertDeleteFlags::ALL, false);

    if ((nStartRow == 0) && (nEndRow == MAXROW))
    {
        for (SCSIZE i=0; i < nSize; i++)
            for (SCCOL nCol = nStartCol; nCol < aCol.size() - 1; nCol++)
                aCol[nCol].SwapCol(aCol[nCol+1]);
    }
    else
    {
        for (SCSIZE i=0; static_cast<SCCOL>(i+nSize)+nStartCol < aCol.size(); i++)
            aCol[nStartCol + nSize + i].MoveTo(nStartRow, nEndRow, aCol[nStartCol + i]);
    }

    std::vector<SCCOL> aRegroupCols;
    rRegroupCols.getColumns(nTab, aRegroupCols);
    std::for_each(aRegroupCols.begin(), aRegroupCols.end(), ColumnRegroupFormulaCells(aCol, nullptr));

    InvalidatePageBreaks();

    // TODO: In the future we may want to check if the table has been
    // really modified before setting the stream invalid.
    SetStreamValid(false);
}

void ScTable::DeleteArea(
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, InsertDeleteFlags nDelFlag,
    bool bBroadcast, sc::ColumnSpanSet* pBroadcastSpans )
{
    if ( nCol2 >= aCol.size() ) nCol2 = aCol.size() - 1;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        {   // scope for bulk broadcast
            ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM(), SfxHintId::ScDataChanged);
            for (SCCOL i = nCol1; i <= nCol2; i++)
                aCol[i].DeleteArea(nRow1, nRow2, nDelFlag, bBroadcast, pBroadcastSpans);
        }

            // Do not set protected cell in a protected table

        if ( IsProtected() && (nDelFlag & InsertDeleteFlags::ATTRIB) )
        {
            ScPatternAttr aPattern(pDocument->GetPool());
            aPattern.GetItemSet().Put( ScProtectionAttr( false ) );
            ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
        }

        if( nDelFlag & InsertDeleteFlags::ATTRIB )
            mpCondFormatList->DeleteArea( nCol1, nRow1, nCol2, nRow2 );
    }

    // TODO: In the future we may want to check if the table has been
    // really modified before setting the stream invalid.
    SetStreamValid(false);
}

void ScTable::DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast )
{
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM(), SfxHintId::ScDataChanged);
        for (SCCOL i=0; i < aCol.size(); i++)
            aCol[i].DeleteSelection(nDelFlag, rMark, bBroadcast);
    }

    ScRangeList aRangeList;
    rMark.FillRangeListWithMarks(&aRangeList, false);

    for (size_t i = 0; i < aRangeList.size(); ++i)
    {
        const ScRange & rRange = aRangeList[i];

        if((nDelFlag & InsertDeleteFlags::ATTRIB) && rRange.aStart.Tab() == nTab)
            mpCondFormatList->DeleteArea( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row() );
    }

        // Do not set protected cell in a protected sheet

    if ( IsProtected() && (nDelFlag & InsertDeleteFlags::ATTRIB) )
    {
        ScDocumentPool* pPool = pDocument->GetPool();
        SfxItemSet aSet( *pPool, svl::Items<ATTR_PATTERN_START, ATTR_PATTERN_END>{} );
        aSet.Put( ScProtectionAttr( false ) );
        SfxItemPoolCache aCache( pPool, &aSet );
        ApplySelectionCache( &aCache, rMark );
    }

    // TODO: In the future we may want to check if the table has been
    // really modified before setting the stream invalid.
    SetStreamValid(false);
}

// pTable = Clipboard
void ScTable::CopyToClip(
    sc::CopyToClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    ScTable* pTable )
{
    if (!ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2))
        return;

    //  copy content
    //local range names need to be copied first for formula cells
    if (!pTable->mpRangeName && mpRangeName)
        pTable->mpRangeName.reset( new ScRangeName(*mpRangeName) );

    SCCOL i;

    for ( i = nCol1; i <= nCol2; i++)
        aCol[i].CopyToClip(rCxt, nRow1, nRow2, pTable->aCol[i]);  // notes are handled at column level

    //  copy widths/heights, and only "hidden", "filtered" and "manual" flags
    //  also for all preceding columns/rows, to have valid positions for drawing objects

    if (mpColWidth && pTable->mpColWidth)
        pTable->mpColWidth->CopyFrom(*mpColWidth, 0, nCol2);

    pTable->CopyColHidden(*this, 0, nCol2);
    pTable->CopyColFiltered(*this, 0, nCol2);
    if (pDBDataNoName)
        pTable->SetAnonymousDBData(std::unique_ptr<ScDBData>(new ScDBData(*pDBDataNoName)));

    if (pRowFlags && pTable->pRowFlags && mpRowHeights && pTable->mpRowHeights)
    {
        pTable->pRowFlags->CopyFromAnded( *pRowFlags, 0, nRow2, CRFlags::ManualSize);
        pTable->CopyRowHeight(*this, 0, nRow2, 0);
    }

    pTable->CopyRowHidden(*this, 0, nRow2);
    pTable->CopyRowFiltered(*this, 0, nRow2);

    // If necessary replace formulas with values

    if ( IsProtected() )
        for (i = nCol1; i <= nCol2; i++)
            pTable->aCol[i].RemoveProtected(nRow1, nRow2);

    pTable->mpCondFormatList.reset(new ScConditionalFormatList(pTable->pDocument, *mpCondFormatList));
}

void ScTable::CopyToClip(
    sc::CopyToClipContext& rCxt, const ScRangeList& rRanges, ScTable* pTable )
{
    for ( size_t i = 0, nListSize = rRanges.size(); i < nListSize; ++i )
    {
        const ScRange & r = rRanges[ i ];
        CopyToClip( rCxt, r.aStart.Col(), r.aStart.Row(), r.aEnd.Col(), r.aEnd.Row(), pTable);
    }
}

void ScTable::CopyStaticToDocument(
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, const SvNumberFormatterMergeMap& rMap, ScTable* pDestTab )
{
    if (nCol1 > nCol2)
        return;

    for (SCCOL i = nCol1; i <= nCol2; ++i)
    {
        ScColumn& rSrcCol = aCol[i];
        ScColumn& rDestCol = pDestTab->aCol[i];
        rSrcCol.CopyStaticToDocument(nRow1, nRow2, rMap, rDestCol);
    }
}

void ScTable::CopyCellToDocument(SCCOL nSrcCol, SCROW nSrcRow, SCCOL nDestCol, SCROW nDestRow, ScTable& rDestTab )
{
    if (!ValidColRow(nSrcCol, nSrcRow) || !ValidColRow(nDestCol, nDestRow))
        return;

    ScColumn& rSrcCol = aCol[nSrcCol];
    ScColumn& rDestCol = rDestTab.aCol[nDestCol];
    rSrcCol.CopyCellToDocument(nSrcRow, nDestRow, rDestCol);
}

namespace {

bool CheckAndDeduplicateCondFormat(ScDocument* pDocument, ScConditionalFormat* pOldFormat, const ScConditionalFormat* pNewFormat, SCTAB nTab)
{
    if (!pOldFormat)
        return false;

    if (pOldFormat->EqualEntries(*pNewFormat, true))
    {
        const ScRangeList& rNewRangeList = pNewFormat->GetRange();
        ScRangeList& rDstRangeList = pOldFormat->GetRangeList();
        for (size_t i = 0; i < rNewRangeList.size(); ++i)
        {
            rDstRangeList.Join(rNewRangeList[i]);
        }
        pDocument->AddCondFormatData(rNewRangeList, nTab, pOldFormat->GetKey());
        return true;
    }

    return false;
}

}

void ScTable::CopyConditionalFormat( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        SCCOL nDx, SCROW nDy, const ScTable* pTable)
{
    ScRange aOldRange( nCol1 - nDx, nRow1 - nDy, pTable->nTab, nCol2 - nDx, nRow2 - nDy, pTable->nTab);
    ScRange aNewRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
    bool bSameDoc = pDocument->GetStyleSheetPool() == pTable->pDocument->GetStyleSheetPool();

    for(ScConditionalFormatList::const_iterator itr = pTable->mpCondFormatList->begin(),
            itrEnd = pTable->mpCondFormatList->end(); itr != itrEnd; ++itr)
    {
        const ScRangeList& rCondFormatRange = (*itr)->GetRange();
        if(!rCondFormatRange.Intersects( aOldRange ))
            continue;

        ScRangeList aIntersectedRange = rCondFormatRange.GetIntersectedRange(aOldRange);
        ScConditionalFormat* pNewFormat = (*itr)->Clone(pDocument);

        pNewFormat->SetRange(aIntersectedRange);
        sc::RefUpdateContext aRefCxt(*pDocument);
        aRefCxt.meMode = URM_COPY;
        aRefCxt.maRange = aNewRange;
        aRefCxt.mnColDelta = nDx;
        aRefCxt.mnRowDelta = nDy;
        aRefCxt.mnTabDelta = nTab - pTable->nTab;
        pNewFormat->UpdateReference(aRefCxt, true);

        if (bSameDoc && pTable->nTab == nTab && CheckAndDeduplicateCondFormat(pDocument, mpCondFormatList->GetFormat((*itr)->GetKey()), pNewFormat, nTab))
        {
            delete pNewFormat;
            continue;
        }
        sal_uLong nMax = 0;
        bool bDuplicate = false;
        for(ScConditionalFormatList::const_iterator itrCond = mpCondFormatList->begin();
                itrCond != mpCondFormatList->end(); ++itrCond)
        {
            // Check if there is the same format in the destination
            // If there is, then simply expand its range
            if (CheckAndDeduplicateCondFormat(pDocument, (*itrCond).get(), pNewFormat, nTab))
            {
                bDuplicate = true;
                break;
            }

            if ((*itrCond)->GetKey() > nMax)
                nMax = (*itrCond)->GetKey();
        }
        // Do not add duplicate entries
        if (bDuplicate)
        {
            delete pNewFormat;
            continue;
        }

        pNewFormat->SetKey(nMax + 1);
        mpCondFormatList->InsertNew(pNewFormat);

        if(!bSameDoc)
        {
            for(size_t i = 0, n = pNewFormat->size();
                    i < n; ++i)
            {
                OUString aStyleName;
                const ScFormatEntry* pEntry = pNewFormat->GetEntry(i);
                if(pEntry->GetType() == ScFormatEntry::Type::Condition)
                    aStyleName = static_cast<const ScCondFormatEntry*>(pEntry)->GetStyle();
                else if(pEntry->GetType() == ScFormatEntry::Type::Date)
                    aStyleName = static_cast<const ScCondDateFormatEntry*>(pEntry)->GetStyleName();

                if(!aStyleName.isEmpty())
                {
                    if(pDocument->GetStyleSheetPool()->Find(aStyleName, SfxStyleFamily::Para))
                        continue;

                    pDocument->GetStyleSheetPool()->CopyStyleFrom(
                            pTable->pDocument->GetStyleSheetPool(), aStyleName, SfxStyleFamily::Para );
                }
            }
        }

        pDocument->AddCondFormatData( pNewFormat->GetRange(), nTab, pNewFormat->GetKey() );
    }
}

bool ScTable::InitColumnBlockPosition( sc::ColumnBlockPosition& rBlockPos, SCCOL nCol )
{
    if (!ValidCol(nCol))
        return false;

    return aCol[nCol].InitBlockPosition(rBlockPos);
}

void ScTable::CopyFromClip(
    sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    SCCOL nDx, SCROW nDy, ScTable* pTable )
{
    if (nCol2 > aCol.size() - 1)
        nCol2 = aCol.size() - 1;
    if (nRow2 > MAXROW)
        nRow2 = MAXROW;

    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        for ( SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].CopyFromClip(rCxt, nRow1, nRow2, nDy, pTable->aCol[i - nDx]); // notes are handles at column level

        if (rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB)
        {
            // make sure that there are no old references to the cond formats
            sal_uInt16 nWhichArray[2];
            nWhichArray[0] = ATTR_CONDITIONAL;
            nWhichArray[1] = 0;
            for ( SCCOL i = nCol1; i <= nCol2; ++i)
                aCol[i].ClearItems(nRow1, nRow2, nWhichArray);
        }

        if ((rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB) != InsertDeleteFlags::NONE)
        {
            if (nRow1==0 && nRow2==MAXROW && mpColWidth && pTable->mpColWidth)
                mpColWidth->CopyFrom(*pTable->mpColWidth, nCol1, nCol2, nCol1 - nDx);

            if (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pTable->mpRowHeights &&
                                             pRowFlags && pTable->pRowFlags)
            {
                CopyRowHeight(*pTable, nRow1, nRow2, -nDy);
                // Must copy CRFlags::ManualSize bit too, otherwise pRowHeight doesn't make sense
                for (SCROW j=nRow1; j<=nRow2; j++)
                {
                    if ( pTable->pRowFlags->GetValue(j-nDy) & CRFlags::ManualSize )
                        pRowFlags->OrValue( j, CRFlags::ManualSize);
                    else
                        pRowFlags->AndValue( j, ~CRFlags::ManualSize);
                }
            }

            // Do not set protected cell in a protected sheet
            if (IsProtected() && (rCxt.getInsertFlag() & InsertDeleteFlags::ATTRIB))
            {
                ScPatternAttr aPattern(pDocument->GetPool());
                aPattern.GetItemSet().Put( ScProtectionAttr( false ) );
                ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
            }

            // create deep copies for conditional formatting
            CopyConditionalFormat( nCol1, nRow1, nCol2, nRow2, nDx, nDy, pTable);
        }
    }
}

void ScTable::MixData(
    sc::MixDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    ScPasteFunc nFunction, bool bSkipEmpty, const ScTable* pSrcTab )
{
    for (SCCOL i=nCol1; i<=nCol2; i++)
        aCol[i].MixData(rCxt, nRow1, nRow2, nFunction, bSkipEmpty, pSrcTab->aCol[i]);
}

// Selection form this document
void ScTable::MixMarked(
    sc::MixDocContext& rCxt, const ScMarkData& rMark, ScPasteFunc nFunction,
    bool bSkipEmpty, const ScTable* pSrcTab )
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].MixMarked(rCxt, rMark, nFunction, bSkipEmpty, pSrcTab->aCol[i]);
}

namespace {

class TransClipHandler
{
    ScTable& mrClipTab;
    SCTAB mnSrcTab;
    SCCOL mnSrcCol;
    size_t mnTopRow;
    SCROW mnTransRow;
    bool mbAsLink;
    bool mbWasCut;

    ScAddress getDestPos(size_t nRow) const
    {
        return ScAddress(static_cast<SCCOL>(nRow-mnTopRow), mnTransRow, mrClipTab.GetTab());
    }

    ScFormulaCell* createRefCell(size_t nSrcRow, const ScAddress& rDestPos) const
    {
        ScAddress aSrcPos(mnSrcCol, nSrcRow, mnSrcTab);
        ScSingleRefData aRef;
        aRef.InitAddress(aSrcPos); // Absolute reference.
        aRef.SetFlag3D(true);

        ScTokenArray aArr;
        aArr.AddSingleReference(aRef);
        return new ScFormulaCell(&mrClipTab.GetDoc(), rDestPos, aArr);
    }

    void setLink(size_t nRow)
    {
        SCCOL nTransCol = nRow - mnTopRow;
        mrClipTab.SetFormulaCell(
            nTransCol, mnTransRow, createRefCell(nRow, getDestPos(nRow)));
    }

public:
    TransClipHandler(ScTable& rClipTab, SCTAB nSrcTab, SCCOL nSrcCol, size_t nTopRow, SCROW nTransRow, bool bAsLink, bool bWasCut) :
        mrClipTab(rClipTab), mnSrcTab(nSrcTab), mnSrcCol(nSrcCol),
        mnTopRow(nTopRow), mnTransRow(nTransRow), mbAsLink(bAsLink), mbWasCut(bWasCut) {}

    void operator() (size_t nRow, double fVal)
    {
        if (mbAsLink)
        {
            setLink(nRow);
            return;
        }

        SCCOL nTransCol = nRow - mnTopRow;
        mrClipTab.SetValue(nTransCol, mnTransRow, fVal);
    }

    void operator() (size_t nRow, const svl::SharedString& rStr)
    {
        if (mbAsLink)
        {
            setLink(nRow);
            return;
        }

        SCCOL nTransCol = nRow - mnTopRow;
        mrClipTab.SetRawString(nTransCol, mnTransRow, rStr);
    }

    void operator() (size_t nRow, const EditTextObject* p)
    {
        if (mbAsLink)
        {
            setLink(nRow);
            return;
        }

        SCCOL nTransCol = nRow - mnTopRow;
        mrClipTab.SetEditText(nTransCol, mnTransRow, ScEditUtil::Clone(*p, mrClipTab.GetDoc()));
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        if (mbAsLink)
        {
            setLink(nRow);
            return;
        }

        ScFormulaCell* pNew = new ScFormulaCell(
            *p, mrClipTab.GetDoc(), getDestPos(nRow), ScCloneFlags::StartListening);

        //  rotate reference
        //  for Cut, the references are later adjusted through UpdateTranspose

        if (!mbWasCut)
            pNew->TransposeReference();

        SCCOL nTransCol = nRow - mnTopRow;
        mrClipTab.SetFormulaCell(nTransCol, mnTransRow, pNew);
    }
};

}

void ScTable::TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ScTable* pTransClip, InsertDeleteFlags nFlags, bool bAsLink )
{
    bool bWasCut = pDocument->IsCutMode();

    ScDocument* pDestDoc = pTransClip->pDocument;

    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
    {
        SCROW nRow;
        if ( bAsLink && nFlags == InsertDeleteFlags::ALL )
        {
            //  with InsertDeleteFlags::ALL, also create links (formulas) for empty cells

            for ( nRow=nRow1; nRow<=nRow2; nRow++ )
            {
                //  create simple formula, as in ScColumn::CreateRefCell

                ScAddress aDestPos( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
                ScSingleRefData aRef;
                aRef.InitAddress(ScAddress(nCol,nRow,nTab));
                aRef.SetFlag3D(true);
                ScTokenArray aArr;
                aArr.AddSingleReference( aRef );

                pTransClip->SetFormulaCell(
                    static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1),
                    new ScFormulaCell(pDestDoc, aDestPos, aArr));
            }
        }
        else
        {
            TransClipHandler aFunc(*pTransClip, nTab, nCol, nRow1, static_cast<SCROW>(nCol-nCol1), bAsLink, bWasCut);
            const sc::CellStoreType& rCells = aCol[nCol].maCells;
            sc::ParseAllNonEmpty(rCells.begin(), rCells, nRow1, nRow2, aFunc);
        }

        //  Attribute

        SCROW nAttrRow1;
        SCROW nAttrRow2;
        const ScPatternAttr* pPattern;
        std::unique_ptr<ScAttrIterator> pAttrIter(aCol[nCol].CreateAttrIterator( nRow1, nRow2 ));
        while ( (pPattern = pAttrIter->Next( nAttrRow1, nAttrRow2 )) != nullptr )
        {
            if ( !IsDefaultItem( pPattern ) )
            {
                const SfxItemSet& rSet = pPattern->GetItemSet();
                if ( rSet.GetItemState( ATTR_MERGE, false ) == SfxItemState::DEFAULT &&
                     rSet.GetItemState( ATTR_MERGE_FLAG, false ) == SfxItemState::DEFAULT &&
                     rSet.GetItemState( ATTR_BORDER, false ) == SfxItemState::DEFAULT )
                {
                    // no borders or merge items involved - use pattern as-is
                    for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), *pPattern );
                }
                else
                {
                    // transpose borders and merge values, remove merge flags (refreshed after pasting)
                    ScPatternAttr aNewPattern( *pPattern );
                    SfxItemSet& rNewSet = aNewPattern.GetItemSet();

                    const SvxBoxItem& rOldBox = rSet.Get(ATTR_BORDER);
                    if ( rOldBox.GetTop() || rOldBox.GetBottom() || rOldBox.GetLeft() || rOldBox.GetRight() )
                    {
                        SvxBoxItem aNew( ATTR_BORDER );
                        aNew.SetLine( rOldBox.GetLine( SvxBoxItemLine::TOP ), SvxBoxItemLine::LEFT );
                        aNew.SetLine( rOldBox.GetLine( SvxBoxItemLine::LEFT ), SvxBoxItemLine::TOP );
                        aNew.SetLine( rOldBox.GetLine( SvxBoxItemLine::BOTTOM ), SvxBoxItemLine::RIGHT );
                        aNew.SetLine( rOldBox.GetLine( SvxBoxItemLine::RIGHT ), SvxBoxItemLine::BOTTOM );
                        aNew.SetDistance( rOldBox.GetDistance( SvxBoxItemLine::TOP ), SvxBoxItemLine::LEFT );
                        aNew.SetDistance( rOldBox.GetDistance( SvxBoxItemLine::LEFT ), SvxBoxItemLine::TOP );
                        aNew.SetDistance( rOldBox.GetDistance( SvxBoxItemLine::BOTTOM ), SvxBoxItemLine::RIGHT );
                        aNew.SetDistance( rOldBox.GetDistance( SvxBoxItemLine::RIGHT ), SvxBoxItemLine::BOTTOM );
                        rNewSet.Put( aNew );
                    }

                    const ScMergeAttr& rOldMerge = rSet.Get(ATTR_MERGE);
                    if (rOldMerge.IsMerged())
                        rNewSet.Put( ScMergeAttr( std::min(
                                        static_cast<SCCOL>(rOldMerge.GetRowMerge()),
                                        static_cast<SCCOL>(MAXCOL+1 - (nAttrRow2-nRow1))),
                                    std::min(
                                        static_cast<SCROW>(rOldMerge.GetColMerge()),
                                        static_cast<SCROW>(MAXROW+1 - (nCol-nCol1)))));
                    const ScMergeFlagAttr& rOldFlag = rSet.Get(ATTR_MERGE_FLAG);
                    if (rOldFlag.IsOverlapped())
                    {
                        ScMF nNewFlags = rOldFlag.GetValue() & ~ScMF( ScMF::Hor | ScMF::Ver );
                        if ( nNewFlags != ScMF::NONE )
                            rNewSet.Put( ScMergeFlagAttr( nNewFlags ) );
                        else
                            rNewSet.ClearItem( ATTR_MERGE_FLAG );
                    }

                    for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( static_cast<SCCOL>(nRow-nRow1),
                                static_cast<SCROW>(nCol-nCol1), aNewPattern);
                }
            }
        }

        // Cell Notes - fdo#68381 paste cell notes on Transpose
        if ( pDocument->HasColNotes(nCol, nTab) )
            TransposeColNotes(pTransClip, nCol1, nCol, nRow1, nRow2);
    }
}

void ScTable::TransposeColNotes(ScTable* pTransClip, SCCOL nCol1, SCCOL nCol, SCROW nRow1, SCROW nRow2)
{
    sc::CellNoteStoreType::const_iterator itBlk = aCol[nCol].maCellNotes.begin(), itBlkEnd = aCol[nCol].maCellNotes.end();

    // Locate the top row position.
    size_t nOffsetInBlock = 0;
    size_t nBlockStart = 0, nBlockEnd = 0, nRowPos = static_cast<size_t>(nRow1);
    for (; itBlk != itBlkEnd; ++itBlk, nBlockStart = nBlockEnd)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
        {
            // Found.
            nOffsetInBlock = nRowPos - nBlockStart;
            break;
        }
    }

    if (itBlk != itBlkEnd)
        // Specified range found
    {
        nRowPos = static_cast<size_t>(nRow2); // End row position.

        // Keep processing until we hit the end row position.
        sc::cellnote_block::const_iterator itData, itDataEnd;
        for (; itBlk != itBlkEnd; ++itBlk, nBlockStart = nBlockEnd, nOffsetInBlock = 0)
        {
            nBlockEnd = nBlockStart + itBlk->size;

            if (itBlk->data)
            {
                itData = sc::cellnote_block::begin(*itBlk->data);
                std::advance(itData, nOffsetInBlock);

                if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
                {
                    // This block contains the end row. Only process partially.
                    size_t nOffsetEnd = nRowPos - nBlockStart + 1;
                    itDataEnd = sc::cellnote_block::begin(*itBlk->data);
                    std::advance(itDataEnd, nOffsetEnd);
                    size_t curRow = nBlockStart + nOffsetInBlock;
                    for (; itData != itDataEnd; ++itData, ++curRow)
                    {
                        ScAddress aDestPos( static_cast<SCCOL>(curRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
                        pTransClip->pDocument->ReleaseNote(aDestPos);
                        ScPostIt* pNote = *itData;
                        if (pNote)
                        {
                            ScPostIt* pClonedNote = pNote->Clone( ScAddress(nCol, curRow, nTab), *pTransClip->pDocument, aDestPos, true );
                            pTransClip->pDocument->SetNote(aDestPos, pClonedNote);
                        }
                    }
                    break; // we reached the last valid block
                }
                else
                {
                    itDataEnd = sc::cellnote_block::end(*itBlk->data);
                    size_t curRow = nBlockStart + nOffsetInBlock;
                    for (; itData != itDataEnd; ++itData, ++curRow)
                    {
                        ScAddress aDestPos( static_cast<SCCOL>(curRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
                        pTransClip->pDocument->ReleaseNote(aDestPos);
                        ScPostIt* pNote = *itData;
                        if (pNote)
                        {
                            ScPostIt* pClonedNote = pNote->Clone( ScAddress(nCol, curRow, nTab), *pTransClip->pDocument, aDestPos, true );
                            pTransClip->pDocument->SetNote(aDestPos, pClonedNote);
                        }
                    }
                }
            }
            else
            {
                size_t curRow;
                for ( curRow = nBlockStart + nOffsetInBlock; curRow <= nBlockEnd && curRow <= nRowPos; ++curRow)
                {
                    ScAddress aDestPos( static_cast<SCCOL>(curRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
                    pTransClip->pDocument->ReleaseNote(aDestPos);
                }
                if (curRow == nRowPos)
                    break;
            }
        }
    }
}

ScColumn* ScTable::FetchColumn( SCCOL nCol )
{
    if (!ValidCol(nCol))
        return nullptr;

    return &aCol[nCol];
}

const ScColumn* ScTable::FetchColumn( SCCOL nCol ) const
{
    if (!ValidCol(nCol))
        return nullptr;

    return &aCol[nCol];
}

void ScTable::StartListeners( sc::StartListeningContext& rCxt, bool bAll )
{
    std::shared_ptr<const sc::ColumnSet> pColSet = rCxt.getColumnSet();
    if (!pColSet)
    {
        for (SCCOL i=0; i < aCol.size(); i++)
            aCol[i].StartListeners(rCxt, bAll);
    }
    else if (pColSet->hasTab( nTab))
    {
        std::vector<SCCOL> aColumns;
        pColSet->getColumns( nTab, aColumns);
        for (auto i : aColumns)
        {
            if (0 <= i && i < aCol.size())
                aCol[i].StartListeners(rCxt, bAll);
        }
    }
}

void ScTable::AttachFormulaCells(
    sc::StartListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].AttachFormulaCells(rCxt, nRow1, nRow2);
}

void ScTable::DetachFormulaCells(
    sc::EndListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].DetachFormulaCells(rCxt, nRow1, nRow2);
}

void ScTable::SetDirtyFromClip(
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sc::ColumnSpanSet& rBroadcastSpans )
{
    if ( nCol2 >= aCol.size() ) nCol2 = aCol.size() - 1;
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
        for (SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].SetDirtyFromClip(nRow1, nRow2, rBroadcastSpans);
}

void ScTable::StartListeningFormulaCells(
    sc::StartListeningContext& rStartCxt, sc::EndListeningContext& rEndCxt,
    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if ( nCol2 >= aCol.size() ) nCol2 = aCol.size() - 1;
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
        for (SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].StartListeningFormulaCells(rStartCxt, rEndCxt, nRow1, nRow2);
}

void ScTable::CopyToTable(
    sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    InsertDeleteFlags nFlags, bool bMarked, ScTable* pDestTab, const ScMarkData* pMarkData,
    bool bAsLink, bool bColRowFlags, bool bGlobalNamesToLocal, bool bCopyCaptions )
{
    if (!ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2))
        return;

    bool bIsUndoDoc = pDestTab->pDocument->IsUndo();
    if (nFlags != InsertDeleteFlags::NONE)
    {
        InsertDeleteFlags nTempFlags( nFlags &
                ~InsertDeleteFlags( InsertDeleteFlags::NOTE | InsertDeleteFlags::ADDNOTES));
        for (SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].CopyToColumn(rCxt, nRow1, nRow2, bIsUndoDoc ? nFlags : nTempFlags, bMarked,
                                pDestTab->aCol[i], pMarkData, bAsLink, bGlobalNamesToLocal);
    }

    if (!bColRowFlags)      // Column widths/Row heights/Flags
        return;

    if(bIsUndoDoc && (nFlags & InsertDeleteFlags::ATTRIB))
    {
        pDestTab->mpCondFormatList.reset(new ScConditionalFormatList(pDestTab->pDocument, *mpCondFormatList));
    }

    if (pDBDataNoName)
    {
        std::unique_ptr<ScDBData> pNewDBData(new ScDBData(*pDBDataNoName));
        SCCOL aCol1, aCol2;
        SCROW aRow1, aRow2;
        SCTAB aTab;
        pNewDBData->GetArea(aTab, aCol1, aRow1, aCol2, aRow2);
        pNewDBData->MoveTo(pDestTab->nTab, aCol1, aRow1, aCol2, aRow2);
        pDestTab->SetAnonymousDBData(std::move(pNewDBData));
    }
    //  Charts have to be adjusted when hide/show
    ScChartListenerCollection* pCharts = pDestTab->pDocument->GetChartListenerCollection();

    bool bFlagChange = false;

    bool bWidth  = (nRow1==0 && nRow2==MAXROW && mpColWidth && pDestTab->mpColWidth);
    bool bHeight = (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pDestTab->mpRowHeights);

    if (bWidth || bHeight)
    {
        if (bWidth)
        {
            auto destTabColWidthIt = pDestTab->mpColWidth->begin() + nCol1;
            auto thisTabColWidthIt = mpColWidth->begin() + nCol1;
            pDestTab->mpColWidth->CopyFrom(*mpColWidth, nCol1, nCol2);
            pDestTab->mpColFlags->CopyFrom(*mpColFlags, nCol1, nCol2);
            for (SCCOL i = nCol1; i <= nCol2; ++i)
            {
                bool bThisHidden = ColHidden(i);
                bool bHiddenChange = (pDestTab->ColHidden(i) != bThisHidden);
                bool bChange = bHiddenChange || (*destTabColWidthIt != *thisTabColWidthIt);
                pDestTab->SetColHidden(i, i, bThisHidden);
                //TODO: collect changes?
                if (bHiddenChange && pCharts)
                    pCharts->SetRangeDirty(ScRange( i, 0, nTab, i, MAXROW, nTab ));

                if (bChange)
                    bFlagChange = true;

                ++destTabColWidthIt;
                ++thisTabColWidthIt;
            }
            pDestTab->SetColManualBreaks( maColManualBreaks);
        }

        if (bHeight)
        {
            bool bChange = pDestTab->GetRowHeight(nRow1, nRow2) != GetRowHeight(nRow1, nRow2);

            if (bChange)
                bFlagChange = true;

            pDestTab->CopyRowHeight(*this, nRow1, nRow2, 0);
            pDestTab->pRowFlags->CopyFrom(*pRowFlags, nRow1, nRow2);

            // Hidden flags.
            for (SCROW i = nRow1; i <= nRow2; ++i)
            {
                SCROW nLastRow;
                bool bHidden = RowHidden(i, nullptr, &nLastRow);
                if (nLastRow >= nRow2)
                    // the last row shouldn't exceed the upper bound the caller specified.
                    nLastRow = nRow2;

                bool bHiddenChanged = pDestTab->SetRowHidden(i, nLastRow, bHidden);
                if (bHiddenChanged && pCharts)
                    // Hidden flags differ.
                    pCharts->SetRangeDirty(ScRange(0, i, nTab, MAXCOL, nLastRow, nTab));

                if (bHiddenChanged)
                    bFlagChange = true;

                // Jump to the last row of the identical flag segment.
                i = nLastRow;
            }

            // Filtered flags.
            for (SCROW i = nRow1; i <= nRow2; ++i)
            {
                SCROW nLastRow;
                bool bFiltered = RowFiltered(i, nullptr, &nLastRow);
                if (nLastRow >= nRow2)
                    // the last row shouldn't exceed the upper bound the caller specified.
                    nLastRow = nRow2;
                pDestTab->SetRowFiltered(i, nLastRow, bFiltered);
                i = nLastRow;
            }
            pDestTab->SetRowManualBreaks( maRowManualBreaks);
        }
    }

    if (bFlagChange)
        pDestTab->InvalidatePageBreaks();

    if(nFlags & InsertDeleteFlags::ATTRIB)
    {
        pDestTab->mpCondFormatList->DeleteArea(nCol1, nRow1, nCol2, nRow2);
        pDestTab->CopyConditionalFormat(nCol1, nRow1, nCol2, nRow2, 0, 0, this);
    }

    if(nFlags & InsertDeleteFlags::OUTLINE) // also only when bColRowFlags
        pDestTab->SetOutlineTable( pOutlineTable.get() );

    if (!bIsUndoDoc && bCopyCaptions && (nFlags & (InsertDeleteFlags::NOTE | InsertDeleteFlags::ADDNOTES)))
    {
        bool bCloneCaption = (nFlags & InsertDeleteFlags::NOCAPTIONS) == InsertDeleteFlags::NONE;
        CopyCaptionsToTable( nCol1, nRow1, nCol2, nRow2, pDestTab, bCloneCaption);
    }
}

void ScTable::CopyCaptionsToTable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pDestTab,
        bool bCloneCaption )
{
    if (!ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2))
        return;

    for (SCCOL i = nCol1; i <= nCol2; i++)
    {
        aCol[i].CopyCellNotesToDocument(nRow1, nRow2, pDestTab->aCol[i], bCloneCaption);
        pDestTab->aCol[i].UpdateNoteCaptions(nRow1, nRow2);
    }
}

void ScTable::UndoToTable(
    sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    InsertDeleteFlags nFlags, bool bMarked, ScTable* pDestTab )
{
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        bool bWidth  = (nRow1==0 && nRow2==MAXROW && mpColWidth && pDestTab->mpColWidth);
        bool bHeight = (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pDestTab->mpRowHeights);

        for ( SCCOL i = 0; i < aCol.size(); i++)
        {
            if ( i >= nCol1 && i <= nCol2 )
                aCol[i].UndoToColumn(rCxt, nRow1, nRow2, nFlags, bMarked, pDestTab->aCol[i]);
            else
                aCol[i].CopyToColumn(rCxt, 0, MAXROW, InsertDeleteFlags::FORMULA, false, pDestTab->aCol[i]);
        }

        if (nFlags & InsertDeleteFlags::ATTRIB)
            pDestTab->mpCondFormatList.reset(new ScConditionalFormatList(pDestTab->pDocument, *mpCondFormatList));

        if (bWidth||bHeight)
        {
            if (bWidth)
            {
                pDestTab->mpColWidth->CopyFrom(*mpColWidth, nCol1, nCol2);
                pDestTab->SetColManualBreaks( maColManualBreaks);
            }
            if (bHeight)
            {
                pDestTab->CopyRowHeight(*this, nRow1, nRow2, 0);
                pDestTab->SetRowManualBreaks( maRowManualBreaks);
            }
        }
    }
}

void ScTable::CopyUpdated( const ScTable* pPosTab, ScTable* pDestTab ) const
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].CopyUpdated( pPosTab->aCol[i], pDestTab->aCol[i] );
}

void ScTable::InvalidateTableArea()
{
    bTableAreaValid = false;
}

void ScTable::InvalidatePageBreaks()
{
    mbPageBreaksValid = false;
}

void ScTable::CopyScenarioTo( ScTable* pDestTab ) const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].CopyScenarioTo( pDestTab->aCol[i] );
}

void ScTable::CopyScenarioFrom( const ScTable* pSrcTab )
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].CopyScenarioFrom( pSrcTab->aCol[i] );
}

void ScTable::MarkScenarioIn( ScMarkData& rDestMark, ScScenarioFlags nNeededBits ) const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    if ( ( nScenarioFlags & nNeededBits ) != nNeededBits )  // Are all Bits set?
        return;

    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].MarkScenarioIn( rDestMark );
}

bool ScTable::HasScenarioRange( const ScRange& rRange ) const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    ScRange aTabRange = rRange;
    aTabRange.aStart.SetTab( nTab );
    aTabRange.aEnd.SetTab( nTab );

    const ScRangeList* pList = GetScenarioRanges();

    if (pList)
    {
        for ( size_t j = 0, n = pList->size(); j < n; j++ )
        {
            const ScRange & rR = (*pList)[j];
            if ( rR.Intersects( aTabRange ) )
                return true;
        }
    }

    return false;
}

void ScTable::InvalidateScenarioRanges()
{
    pScenarioRanges.reset();
}

const ScRangeList* ScTable::GetScenarioRanges() const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    if (!pScenarioRanges)
    {
        const_cast<ScTable*>(this)->pScenarioRanges.reset(new ScRangeList);
        ScMarkData aMark;
        MarkScenarioIn( aMark, ScScenarioFlags::NONE );     // always
        aMark.FillRangeListWithMarks( pScenarioRanges.get(), false );
    }
    return pScenarioRanges.get();
}

bool ScTable::TestCopyScenarioTo( const ScTable* pDestTab ) const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    if (!pDestTab->IsProtected())
        return true;

    bool bOk = true;
    for (SCCOL i=0; i < aCol.size() && bOk; i++)
        bOk = aCol[i].TestCopyScenarioTo( pDestTab->aCol[i] );
    return bOk;
}

bool ScTable::SetString( SCCOL nCol, SCROW nRow, SCTAB nTabP, const OUString& rString,
                         const ScSetStringParam * pParam )
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].SetString(
            nRow, nTabP, rString, pDocument->GetAddressConvention(), pParam );
    else
        return false;
}

bool ScTable::SetEditText( SCCOL nCol, SCROW nRow, std::unique_ptr<EditTextObject> pEditText )
{
    if (!ValidColRow(nCol, nRow))
    {
        return false;
    }

    aCol[nCol].SetEditText(nRow, std::move(pEditText));
    return true;
}

void ScTable::SetEditText( SCCOL nCol, SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool )
{
    if (!ValidColRow(nCol, nRow))
        return;

    aCol[nCol].SetEditText(nRow, rEditText, pEditPool);
}

SCROW ScTable::GetFirstEditTextRow( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2) || nCol2 < nCol1)
        return -1;

    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow2 < nRow1)
        return -1;

    SCROW nFirst = MAXROW+1;
    for (SCCOL i = nCol1; i <= nCol2; ++i)
    {
        const ScColumn& rCol = aCol[i];
        SCROW nThisFirst = -1;
        if (const_cast<ScColumn&>(rCol).HasEditCells(nRow1, nRow2, nThisFirst))
        {
            if (nThisFirst == nRow1)
                return nRow1;

            if (nThisFirst < nFirst)
                nFirst = nThisFirst;
        }
    }

    return nFirst == (MAXROW+1) ? -1 : nFirst;
}

void ScTable::SetEmptyCell( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol, nRow))
        return;

    aCol[nCol].Delete(nRow);
}

void ScTable::SetFormula(
    SCCOL nCol, SCROW nRow, const ScTokenArray& rArray, formula::FormulaGrammar::Grammar eGram )
{
    if (!ValidColRow(nCol, nRow))
        return;

    aCol[nCol].SetFormula(nRow, rArray, eGram);
}

void ScTable::SetFormula(
    SCCOL nCol, SCROW nRow, const OUString& rFormula, formula::FormulaGrammar::Grammar eGram )
{
    if (!ValidColRow(nCol, nRow))
        return;

    aCol[nCol].SetFormula(nRow, rFormula, eGram);
}

ScFormulaCell* ScTable::SetFormulaCell( SCCOL nCol, SCROW nRow, ScFormulaCell* pCell )
{
    if (!ValidColRow(nCol, nRow))
    {
        delete pCell;
        return nullptr;
    }

    return aCol[nCol].SetFormulaCell(nRow, pCell, sc::ConvertToGroupListening);
}

bool ScTable::SetFormulaCells( SCCOL nCol, SCROW nRow, std::vector<ScFormulaCell*>& rCells )
{
    if (!ValidCol(nCol))
        return false;

    return aCol[nCol].SetFormulaCells(nRow, rCells);
}

svl::SharedString ScTable::GetSharedString( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return svl::SharedString();

    return aCol[nCol].GetSharedString(nRow);
}

void ScTable::SetValue( SCCOL nCol, SCROW nRow, const double& rVal )
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetValue( nRow, rVal );
}

void ScTable::SetRawString( SCCOL nCol, SCROW nRow, const svl::SharedString& rStr )
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetRawString(nRow, rStr);
}

void ScTable::GetString( SCCOL nCol, SCROW nRow, OUString& rString, const ScInterpreterContext* pContext ) const
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetString( nRow, rString, pContext );
    else
        rString.clear();
}

double* ScTable::GetValueCell( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol,nRow))
        return nullptr;

    return aCol[nCol].GetValueCell(nRow);
}

void ScTable::GetInputString( SCCOL nCol, SCROW nRow, OUString& rString ) const
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetInputString( nRow, rString );
    else
        rString.clear();
}

double ScTable::GetValue( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow( nCol, nRow ))
        return aCol[nCol].GetValue( nRow );
    return 0.0;
}

const EditTextObject* ScTable::GetEditText( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return nullptr;

    return aCol[nCol].GetEditText(nRow);
}

void ScTable::RemoveEditTextCharAttribs( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr )
{
    if (!ValidColRow(nCol, nRow))
        return;

    return aCol[nCol].RemoveEditTextCharAttribs(nRow, rAttr);
}

void ScTable::GetFormula( SCCOL nCol, SCROW nRow, OUString& rFormula ) const
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetFormula( nRow, rFormula );
    else
        rFormula.clear();
}

const ScFormulaCell* ScTable::GetFormulaCell( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return nullptr;

    return aCol[nCol].GetFormulaCell(nRow);
}

ScFormulaCell* ScTable::GetFormulaCell( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol, nRow))
        return nullptr;

    return aCol[nCol].GetFormulaCell(nRow);
}

ScPostIt* ScTable::ReleaseNote( SCCOL nCol, SCROW nRow )
{
    if (!ValidCol(nCol))
        return nullptr;

    return aCol[nCol].ReleaseNote(nRow);
}

size_t ScTable::GetNoteCount( SCCOL nCol ) const
{
    if (!ValidCol(nCol))
        return 0;

    return aCol[nCol].GetNoteCount();
}

SCROW ScTable::GetNotePosition( SCCOL nCol, size_t nIndex ) const
{
    if (!ValidCol(nCol))
        return -1;

    return aCol[nCol].GetNotePosition(nIndex);
}

void ScTable::CreateAllNoteCaptions()
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].CreateAllNoteCaptions();
}

void ScTable::ForgetNoteCaptions( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bPreserveData )
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2))
        return;
    if ( nCol2 >= aCol.size() ) nCol2 = aCol.size() - 1;
    for (SCCOL i = nCol1; i <= nCol2; ++i)
        aCol[i].ForgetNoteCaptions(nRow1, nRow2, bPreserveData);
}

void ScTable::GetAllNoteEntries( std::vector<sc::NoteEntry>& rNotes ) const
{
    for (SCCOL nCol = 0; nCol < aCol.size(); ++nCol)
        aCol[nCol].GetAllNoteEntries(rNotes);
}

void ScTable::GetNotesInRange( const ScRange& rRange, std::vector<sc::NoteEntry>& rNotes ) const
{
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow = rRange.aEnd.Row();
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        aCol[nCol].GetNotesInRange(nStartRow, nEndRow, rNotes);
    }
}

CommentCaptionState ScTable::GetAllNoteCaptionsState(const ScRange& rRange, std::vector<sc::NoteEntry>& rNotes )
{
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow = rRange.aEnd.Row();
    bool bIsFirstNoteShownState = true; // because of error: -Werror=maybe-uninitialized
    bool bFirstControl = true;

    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        if (bFirstControl && pDocument->HasColNotes(nCol, nTab)) // detect status of first note caption
        {
            aCol[nCol].GetNotesInRange(nStartRow, nEndRow, rNotes);
            bIsFirstNoteShownState = rNotes.begin()->mpNote->IsCaptionShown();
            bFirstControl = false;
        }

        if (pDocument->HasColNotes(nCol, nTab))
        {
            aCol[nCol].GetNotesInRange(nStartRow, nEndRow, rNotes);

            for(std::vector<sc::NoteEntry>::const_iterator itr = rNotes.begin(),
                         itrEnd = rNotes.end(); itr != itrEnd; ++itr)
            {
                if ( bIsFirstNoteShownState != itr->mpNote->IsCaptionShown())  // compare the first note caption with others
                {
                    return CommentCaptionState::MIXED;
                }
            }
        }
    }
    return bIsFirstNoteShownState ? CommentCaptionState::ALLSHOWN : CommentCaptionState::ALLHIDDEN;
}

void ScTable::GetUnprotectedCells( ScRangeList& rRangeList ) const
{
    for (auto const & pCol : aCol)
        pCol->GetUnprotectedCells(0, MAXROW, rRangeList);
}

bool ScTable::ContainsNotesInRange( const ScRange& rRange ) const
{
    SCROW nStartRow = rRange.aStart.Row();
    SCROW nEndRow = rRange.aEnd.Row();
    for (SCCOL nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
    {
        bool bContainsNote = !aCol[nCol].IsNotesEmptyBlock(nStartRow, nEndRow);
        if(bContainsNote)
            return true;
    }

    return false;
}

CellType ScTable::GetCellType( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow( nCol, nRow ))
        return aCol[nCol].GetCellType( nRow );
    return CELLTYPE_NONE;
}

ScRefCellValue ScTable::GetCellValue( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return ScRefCellValue();

    return aCol[nCol].GetCellValue(nRow);
}

void ScTable::GetFirstDataPos(SCCOL& rCol, SCROW& rRow) const
{
    rCol = 0;
    rRow = MAXROW+1;
    while (rCol < (aCol.size() - 1) && aCol[rCol].IsEmptyData() )
        ++rCol;
    SCCOL nCol = rCol;
    while (nCol < aCol.size() && rRow > 0)
    {
        if (!aCol[nCol].IsEmptyData())
            rRow = ::std::min( rRow, aCol[nCol].GetFirstDataPos());
        ++nCol;
    }
}

void ScTable::GetLastDataPos(SCCOL& rCol, SCROW& rRow) const
{
    rCol = aCol.size() - 1;
    rRow = 0;
    while (aCol[rCol].IsEmptyData() && (rCol > 0))
        rCol--;
    SCCOL nCol = rCol;
    while (nCol >= 0 && rRow < MAXROW)
        rRow = ::std::max( rRow, aCol[nCol--].GetLastDataPos());
}

bool ScTable::HasData( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].HasDataAt( nRow );
    else
        return false;
}

bool ScTable::HasStringData( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].HasStringData( nRow );
    else
        return false;
}

bool ScTable::HasValueData( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].HasValueData( nRow );
    else
        return false;
}

bool ScTable::HasStringCells( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow ) const
{
    if ( ValidCol(nEndCol) )
        for ( SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++ )
            if (aCol[nCol].HasStringCells(nStartRow, nEndRow))
                return true;

    return false;
}

void ScTable::SetDirtyVar()
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].SetDirtyVar();
}

void ScTable::CheckVectorizationState()
{
    sc::AutoCalcSwitch aACSwitch(*pDocument, false);

    for (SCCOL i = 0; i < aCol.size(); i++)
        aCol[i].CheckVectorizationState();
}

void ScTable::SetAllFormulasDirty( const sc::SetFormulaDirtyContext& rCxt )
{
    sc::AutoCalcSwitch aACSwitch(*pDocument, false);

    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].SetAllFormulasDirty(rCxt);
}

void ScTable::SetDirty( const ScRange& rRange, ScColumn::BroadcastMode eMode )
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // avoid multiple recalculations
    SCCOL nCol2 = rRange.aEnd.Col();
    for (SCCOL i=rRange.aStart.Col(); i<=nCol2; i++)
        aCol[i].SetDirty(rRange.aStart.Row(), rRange.aEnd.Row(), eMode);
    pDocument->SetAutoCalc( bOldAutoCalc );
}

void ScTable::SetTableOpDirty( const ScRange& rRange )
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // no multiple recalculation
    SCCOL nCol2 = rRange.aEnd.Col();
    for (SCCOL i=rRange.aStart.Col(); i<=nCol2; i++)
        aCol[i].SetTableOpDirty( rRange );
    pDocument->SetAutoCalc( bOldAutoCalc );
}

void ScTable::SetDirtyAfterLoad()
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // avoid multiple recalculations
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].SetDirtyAfterLoad();
    pDocument->SetAutoCalc( bOldAutoCalc );
}

void ScTable::SetDirtyIfPostponed()
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // avoid multiple recalculations
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].SetDirtyIfPostponed();
    pDocument->SetAutoCalc( bOldAutoCalc );
}

void ScTable::BroadcastRecalcOnRefMove()
{
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].BroadcastRecalcOnRefMove();
}

bool ScTable::BroadcastBroadcasters( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScHint& rHint )
{
    bool bBroadcasted = false;
    sc::AutoCalcSwitch aSwitch(*pDocument, false);
    rHint.GetAddress().SetTab(nTab);
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        bBroadcasted |= aCol[nCol].BroadcastBroadcasters( nRow1, nRow2, rHint);
    return bBroadcasted;
}

void ScTable::TransferListeners(
    ScTable& rDestTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    SCCOL nColDelta, SCROW nRowDelta )
{
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        ScColumn& rSrcCol = aCol[nCol];
        ScColumn& rDestCol = rDestTab.aCol[nCol+nColDelta];
        rSrcCol.TransferListeners(rDestCol, nRow1, nRow2, nRowDelta);
    }
}

void ScTable::SetLoadingMedium(bool bLoading)
{
    mpRowHeights->enableTreeSearch(!bLoading);
}

void ScTable::CalcAll()
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].CalcAll();

    mpCondFormatList->CalcAll();
}

void ScTable::CompileAll( sc::CompileFormulaContext& rCxt )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].CompileAll(rCxt);

    if(mpCondFormatList)
        mpCondFormatList->CompileAll();
}

void ScTable::CompileXML( sc::CompileFormulaContext& rCxt, ScProgress& rProgress )
{
    if (mpRangeName)
        mpRangeName->CompileUnresolvedXML(rCxt);

    for (SCCOL i=0; i < aCol.size(); i++)
    {
        aCol[i].CompileXML(rCxt, rProgress);
    }

    if(mpCondFormatList)
        mpCondFormatList->CompileXML();
}

bool ScTable::CompileErrorCells( sc::CompileFormulaContext& rCxt, FormulaError nErrCode )
{
    bool bCompiled = false;
    for (SCCOL i = 0; i < aCol.size(); ++i)
    {
        if (aCol[i].CompileErrorCells(rCxt, nErrCode))
            bCompiled = true;
    }

    return bCompiled;
}

void ScTable::CalcAfterLoad( sc::CompileFormulaContext& rCxt, bool bStartListening )
{
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].CalcAfterLoad(rCxt, bStartListening);
}

void ScTable::ResetChanged( const ScRange& rRange )
{
    SCCOL nStartCol = rRange.aStart.Col();
    SCROW nStartRow = rRange.aStart.Row();
    SCCOL nEndCol = rRange.aEnd.Col();
    SCROW nEndRow = rRange.aEnd.Row();

    for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
        aCol[nCol].ResetChanged(nStartRow, nEndRow);
}

//  Attribute

const SfxPoolItem* ScTable::GetAttr( SCCOL nCol, SCROW nRow, sal_uInt16 nWhich ) const
{
    if (ValidColRow(nCol,nRow))
        return &aCol[nCol].GetAttr( nRow, nWhich );
    else
        return nullptr;
}

sal_uInt32 ScTable::GetNumberFormat( const ScInterpreterContext& rContext, const ScAddress& rPos ) const
{
    return ValidColRow(rPos.Col(),rPos.Row()) ?
        aCol[rPos.Col()].GetNumberFormat( rContext, rPos.Row() ) :
        0;
}

sal_uInt32 ScTable::GetNumberFormat( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].GetNumberFormat( pDocument->GetNonThreadedContext(), nRow );
    else
        return 0;
}

sal_uInt32 ScTable::GetNumberFormat( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (!ValidCol(nCol) || !ValidRow(nStartRow) || !ValidRow(nEndRow))
        return 0;

    return aCol[nCol].GetNumberFormat(nStartRow, nEndRow);
}

void ScTable::SetNumberFormat( SCCOL nCol, SCROW nRow, sal_uInt32 nNumberFormat )
{
    if (!ValidColRow(nCol, nRow))
        return;

    aCol[nCol].SetNumberFormat(nRow, nNumberFormat);
}

const ScPatternAttr* ScTable::GetPattern( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].GetPattern( nRow );
    else
    {
        OSL_FAIL("wrong column or row");
        return pDocument->GetDefPattern();      // for safety
    }
}

const ScPatternAttr* ScTable::GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if ( ValidColRow( nCol, nStartRow ) && ValidRow( nEndRow ) && (nStartRow <= nEndRow) )
        return aCol[nCol].GetMostUsedPattern( nStartRow, nEndRow );
    else
        return nullptr;
}

bool ScTable::HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, HasAttrFlags nMask ) const
{
    if ( nCol1 >= aCol.size() )
         return false;
    if ( nCol2 >= aCol.size() )
         nCol2 = aCol.size() - 1; // Rows above range, doesn't contains flags

    bool bFound = false;
    for (SCCOL i=nCol1; i<=nCol2 && !bFound; i++)
        bFound |= aCol[i].HasAttrib( nRow1, nRow2, nMask );
    return bFound;
}

bool ScTable::HasAttribSelection( const ScMarkData& rMark, HasAttrFlags nMask ) const
{
    std::vector<sc::ColRowSpan> aSpans = rMark.GetMarkedColSpans();

    for (sc::ColRowSpan & aSpan : aSpans)
    {
        for (SCCOLROW j = aSpan.mnStart; j <= aSpan.mnEnd; ++j)
        {
            if (aCol[j].HasAttribSelection(rMark, nMask))
              return true;
        }
    }
    return false;
}

bool ScTable::ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                           SCCOL& rEndCol, SCROW& rEndRow,
                           bool bRefresh )
{
    if (!(ValidCol(nStartCol) && ValidCol(rEndCol)))
    {
        OSL_FAIL("ScTable::ExtendMerge: invalid column number");
        return false;
    }
    if ( nStartCol >= aCol.size() )
    {
        OSL_FAIL("ScTable::ExtendMerge: invalid nStartCol");
        return false;
    }
    bool bFound = false;
    SCCOL nOldEndX = std::min( rEndCol, static_cast<SCCOL>(aCol.size()-1) );
    SCROW nOldEndY = rEndRow;
    for (SCCOL i=nStartCol; i<=nOldEndX; i++)
        bFound |= aCol[i].ExtendMerge( i, nStartRow, nOldEndY, rEndCol, rEndRow, bRefresh );
    return bFound;
}

void ScTable::SetMergedCells( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    ScMergeAttr aAttr(nCol2-nCol1+1, nRow2-nRow1+1);
    ApplyAttr(nCol1, nRow1, aAttr);

    if (nCol1 < nCol2)
        ApplyFlags(nCol1+1, nRow1, nCol2, nRow2, ScMF::Hor);

    if (nRow1 < nRow2)
        ApplyFlags(nCol1, nRow1+1, nCol1, nRow2, ScMF::Ver);

    if (nCol1 < nCol2 && nRow1 < nRow2)
        ApplyFlags(nCol1+1, nRow1+1, nCol2, nRow2, ScMF::Hor | ScMF::Ver);
}

bool ScTable::IsBlockEmpty( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, bool bIgnoreNotes ) const
{
    if (!(ValidCol(nCol1) && ValidCol(nCol2)))
    {
        OSL_FAIL("ScTable::IsBlockEmpty: invalid column number");
        return false;
    }
    bool bEmpty = true;
    for (SCCOL i=nCol1; i<=nCol2 && bEmpty; i++)
    {
        bEmpty = aCol[i].IsEmptyBlock( nRow1, nRow2 );
        if (!bIgnoreNotes && bEmpty)
        {
            bEmpty = aCol[i].IsNotesEmptyBlock(nRow1, nRow2);
        }
    }
    return bEmpty;
}

SCSIZE ScTable::FillMaxRot( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2,
                            SCCOL nCol, SCROW nAttrRow1, SCROW nAttrRow2, SCSIZE nArrY,
                            const ScPatternAttr* pPattern, const SfxItemSet* pCondSet )
{
    //  Return value = new nArrY

    ScRotateDir nRotDir = pPattern->GetRotateDir( pCondSet );
    if ( nRotDir != ScRotateDir::NONE )
    {
        bool bHit = true;
        if ( nCol+1 < nX1 )                             // column to the left
            bHit = ( nRotDir != ScRotateDir::Left );
        else if ( nCol > nX2+1 )                        // column to the right
            bHit = ( nRotDir != ScRotateDir::Right );      // ScRotateDir::Standard may now also be extended to the left

        if ( bHit )
        {
            double nFactor = 0.0;
            if ( nCol > nX2+1 )
            {
                long nRotVal = pPattern->
                        GetItem( ATTR_ROTATE_VALUE, pCondSet ).GetValue();
                double nRealOrient = nRotVal * F_PI18000;   // 1/100 degree
                double nCos = cos( nRealOrient );
                double nSin = sin( nRealOrient );
                //TODO: limit !!!
                //TODO: additional factor for varying PPT X/Y !!!

                // for ScRotateDir::Left this gives a negative value,
                // if the mode is considered
                nFactor = -fabs( nCos / nSin );
            }

            for ( SCROW nRow = nAttrRow1; nRow <= nAttrRow2; nRow++ )
            {
                if (!RowHidden(nRow))
                {
                    bool bHitOne = true;
                    if ( nCol > nX2+1 )
                    {
                        // Does the rotated cell extend into the visible range?

                        SCCOL nTouchedCol = nCol;
                        long nWidth = static_cast<long>(mpRowHeights->getValue(nRow) * nFactor);
                        OSL_ENSURE(nWidth <= 0, "Wrong direction");
                        while ( nWidth < 0 && nTouchedCol > 0 )
                        {
                            --nTouchedCol;
                            nWidth += GetColWidth( nTouchedCol );
                        }
                        if ( nTouchedCol > nX2 )
                            bHitOne = false;
                    }

                    if (bHitOne)
                    {
                        while ( nArrY<nArrCount && pRowInfo[nArrY].nRowNo < nRow )
                            ++nArrY;
                        if ( nArrY<nArrCount && pRowInfo[nArrY].nRowNo == nRow )
                            pRowInfo[nArrY].nRotMaxCol = nCol;
                    }
                }
            }
        }
    }

    return nArrY;
}

void ScTable::FindMaxRotCol( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2 )
{
    if ( !mpColWidth || !mpRowHeights || !mpColFlags || !pRowFlags )
    {
        OSL_FAIL( "Row/column info missing" );
        return;
    }

    //  nRotMaxCol is initialized to SC_ROTMAX_NONE, nRowNo is already set

    SCROW nY1 = pRowInfo[0].nRowNo;
    SCROW nY2 = pRowInfo[nArrCount-1].nRowNo;

    for (SCCOL nCol : GetColumnsRange(0, MAXCOL))
    {
        if (!ColHidden(nCol))
        {
            SCSIZE nArrY = 0;
            ScDocAttrIterator aIter( pDocument, nTab, nCol, nY1, nCol, nY2 );
            SCCOL nAttrCol;
            SCROW nAttrRow1, nAttrRow2;
            const ScPatternAttr* pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
            while ( pPattern )
            {
                const SfxPoolItem* pCondItem;
                if ( pPattern->GetItemSet().GetItemState( ATTR_CONDITIONAL, true, &pCondItem )
                        == SfxItemState::SET )
                {
                    //  Run through all formats, so that each cell does not have to be
                    //  handled individually

                    const std::vector<sal_uInt32>& rCondFormatData = static_cast<const ScCondFormatItem*>(pCondItem)->GetCondFormatData();
                    ScStyleSheetPool* pStylePool = pDocument->GetStyleSheetPool();
                    if (mpCondFormatList && pStylePool && !rCondFormatData.empty())
                    {
                        for(std::vector<sal_uInt32>::const_iterator itr = rCondFormatData.begin(), itrEnd = rCondFormatData.end();
                                itr != itrEnd; ++itr)
                        {
                            const ScConditionalFormat* pFormat = mpCondFormatList->GetFormat(*itr);
                            if ( pFormat )
                            {
                                size_t nEntryCount = pFormat->size();
                                for (size_t nEntry=0; nEntry<nEntryCount; nEntry++)
                                {
                                    const ScFormatEntry* pEntry = pFormat->GetEntry(nEntry);
                                    if(pEntry->GetType() != ScFormatEntry::Type::Condition)
                                        continue;

                                    OUString  aStyleName = static_cast<const ScCondFormatEntry*>(pEntry)->GetStyle();
                                    if (!aStyleName.isEmpty())
                                    {
                                        SfxStyleSheetBase* pStyleSheet =
                                            pStylePool->Find( aStyleName, SfxStyleFamily::Para );
                                        if ( pStyleSheet )
                                        {
                                            FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
                                                    nCol, nAttrRow1, nAttrRow2,
                                                    nArrY, pPattern, &pStyleSheet->GetItemSet() );
                                            //  not changing nArrY
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                nArrY = FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
                                    nCol, nAttrRow1, nAttrRow2,
                                    nArrY, pPattern, nullptr );

                pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
            }
        }
    }
}

bool ScTable::HasBlockMatrixFragment( const SCCOL nCol1, SCROW nRow1, const SCCOL nCol2, SCROW nRow2 ) const
{
    using namespace sc;

    if ( !IsColValid( nCol1 ) )
        return false;

    const SCCOL nMaxCol2 = std::min<SCCOL>( nCol2, aCol.size() - 1 );

    MatrixEdge nEdges = MatrixEdge::Nothing;

    if ( nCol1 == nMaxCol2 )
    {   // left and right column
        const MatrixEdge n = MatrixEdge::Left | MatrixEdge::Right;
        nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, n );
        if ((nEdges != MatrixEdge::Nothing) && (((nEdges & n)!=n) || (nEdges & (MatrixEdge::Inside|MatrixEdge::Open))))
            return true;        // left or right edge is missing or open
    }
    else
    {   // left column
        nEdges = aCol[nCol1].GetBlockMatrixEdges(nRow1, nRow2, MatrixEdge::Left);
        if ((nEdges != MatrixEdge::Nothing) && ((!(nEdges & MatrixEdge::Left)) || (nEdges & (MatrixEdge::Inside|MatrixEdge::Open))))
            return true;        // left edge missing or open
        // right column
        nEdges = aCol[nMaxCol2].GetBlockMatrixEdges(nRow1, nRow2, MatrixEdge::Right);
        if ((nEdges != MatrixEdge::Nothing) && ((!(nEdges & MatrixEdge::Right)) || (nEdges & (MatrixEdge::Inside|MatrixEdge::Open))))
            return true;        // right edge is missing or open
    }

    if ( nRow1 == nRow2 )
    {   // Row on top and on bottom
        bool bOpen = false;
        const MatrixEdge n = MatrixEdge::Bottom | MatrixEdge::Top;
        for ( SCCOL i=nCol1; i<=nMaxCol2; i++)
        {
            nEdges = aCol[i].GetBlockMatrixEdges( nRow1, nRow1, n );
            if (nEdges != MatrixEdge::Nothing)
            {
                if ( (nEdges & n) != n )
                    return true;        // Top or bottom edge missing
                if (nEdges & MatrixEdge::Left)
                    bOpen = true;       // left edge open, continue
                else if ( !bOpen )
                    return true;        // Something exist that has not been opened
                if (nEdges & MatrixEdge::Right)
                    bOpen = false;      // Close right edge
            }
        }
        if ( bOpen )
            return true;
    }
    else
    {
        int j;
        MatrixEdge n;
        SCROW nR;
        // first top row, then bottom row
        for ( j=0, n = MatrixEdge::Top,    nR=nRow1; j<2;
              j++, n = MatrixEdge::Bottom, nR=nRow2)
        {
            bool bOpen = false;
            for ( SCCOL i=nCol1; i<=nMaxCol2; i++)
            {
                nEdges = aCol[i].GetBlockMatrixEdges( nR, nR, n );
                if ( nEdges != MatrixEdge::Nothing)
                {
                    // in top row no top edge respectively
                    // in bottom row no bottom edge
                    if ( (nEdges & n) != n )
                        return true;
                    if (nEdges & MatrixEdge::Left)
                        bOpen = true;       // open left edge, continue
                    else if ( !bOpen )
                        return true;        // Something exist that has not been opened
                    if (nEdges & MatrixEdge::Right)
                        bOpen = false;      // Close right edge
                }
            }
            if ( bOpen )
                return true;
        }
    }
    return false;
}

bool ScTable::HasSelectionMatrixFragment( const ScMarkData& rMark ) const
{
    std::vector<sc::ColRowSpan> aSpans = rMark.GetMarkedColSpans();

    for (sc::ColRowSpan & aSpan : aSpans)
    {
        for ( SCCOLROW j=aSpan.mnStart; j<=aSpan.mnEnd; j++ )
        {
            if ( aCol[j].HasSelectionMatrixFragment(rMark) )
                return true;
        }
    }
    return false;
}

bool ScTable::IsBlockEditable( SCCOL nCol1, SCROW nRow1, SCCOL nCol2,
            SCROW nRow2, bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    if ( !ValidColRow( nCol2, nRow2 ) )
    {
        OSL_FAIL("IsBlockEditable: invalid column or row");
        if (pOnlyNotBecauseOfMatrix)
            *pOnlyNotBecauseOfMatrix = false;
        return false;
    }

    bool bIsEditable = true;
    if ( nLockCount )
        bIsEditable = false;
    else if ( IsProtected() && !pDocument->IsScenario(nTab) )
    {
        bIsEditable = !HasAttrib( nCol1, nRow1, nCol2, nRow2, HasAttrFlags::Protected );
        if (!bIsEditable)
        {
            // An enhanced protection permission may override the attribute.
            if (pTabProtection)
                bIsEditable = pTabProtection->isBlockEditable( ScRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab));
        }
        if (bIsEditable)
        {
            // If Sheet is protected and cells are not protected then
            // check the active scenario protect flag if this range is
            // on the active scenario range. Note the 'copy back' must also
            // be set to apply protection.
            sal_uInt16 nScenTab = nTab+1;
            while(pDocument->IsScenario(nScenTab))
            {
                ScRange aEditRange(nCol1, nRow1, nScenTab, nCol2, nRow2, nScenTab);
                if(pDocument->IsActiveScenario(nScenTab) && pDocument->HasScenarioRange(nScenTab, aEditRange))
                {
                    ScScenarioFlags nFlags;
                    pDocument->GetScenarioFlags(nScenTab,nFlags);
                    bIsEditable = !((nFlags & ScScenarioFlags::Protected) && (nFlags & ScScenarioFlags::TwoWay));
                    break;
                }
                nScenTab++;
            }
        }
    }
    else if (pDocument->IsScenario(nTab))
    {
        // Determine if the preceding sheet is protected
        SCTAB nActualTab = nTab;
        do
        {
            nActualTab--;
        }
        while(pDocument->IsScenario(nActualTab));

        if(pDocument->IsTabProtected(nActualTab))
        {
            ScRange aEditRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
            if(pDocument->HasScenarioRange(nTab, aEditRange))
            {
                ScScenarioFlags nFlags;
                pDocument->GetScenarioFlags(nTab,nFlags);
                bIsEditable = !(nFlags & ScScenarioFlags::Protected);
            }
        }
    }
    if ( bIsEditable )
    {
        if ( HasBlockMatrixFragment( nCol1, nRow1, nCol2, nRow2 ) )
        {
            bIsEditable = false;
            if ( pOnlyNotBecauseOfMatrix )
                *pOnlyNotBecauseOfMatrix = true;
        }
        else if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = false;
    }
    else if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = false;
    return bIsEditable;
}

bool ScTable::IsSelectionEditable( const ScMarkData& rMark,
            bool* pOnlyNotBecauseOfMatrix /* = NULL */ ) const
{
    bool bIsEditable = true;
    if ( nLockCount )
        bIsEditable = false;
    else if ( IsProtected() && !pDocument->IsScenario(nTab) )
    {
        ScRangeList aRanges;
        rMark.FillRangeListWithMarks( &aRanges, false );
        bIsEditable = !HasAttribSelection( rMark, HasAttrFlags::Protected );
        if (!bIsEditable)
        {
            // An enhanced protection permission may override the attribute.
            if (pTabProtection)
                bIsEditable = pTabProtection->isSelectionEditable( aRanges);
        }
        if (bIsEditable)
        {
            // If Sheet is protected and cells are not protected then
            // check the active scenario protect flag if this area is
            // in the active scenario range.
            SCTAB nScenTab = nTab+1;
            while(pDocument->IsScenario(nScenTab) && bIsEditable)
            {
                if(pDocument->IsActiveScenario(nScenTab))
                {
                    for (size_t i=0, nRange = aRanges.size(); (i < nRange) && bIsEditable; i++ )
                    {
                        const ScRange & rRange = aRanges[ i ];
                        if(pDocument->HasScenarioRange(nScenTab, rRange))
                        {
                            ScScenarioFlags nFlags;
                            pDocument->GetScenarioFlags(nScenTab,nFlags);
                            bIsEditable = !((nFlags & ScScenarioFlags::Protected) && (nFlags & ScScenarioFlags::TwoWay));
                        }
                    }
                }
                nScenTab++;
            }
        }
    }
    else if (pDocument->IsScenario(nTab))
    {
        // Determine if the preceding sheet is protected
        SCTAB nActualTab = nTab;
        do
        {
            nActualTab--;
        }
        while(pDocument->IsScenario(nActualTab));

        if(pDocument->IsTabProtected(nActualTab))
        {
            ScRangeList aRanges;
            rMark.FillRangeListWithMarks( &aRanges, false );
            for (size_t i = 0, nRange = aRanges.size(); (i < nRange) && bIsEditable; i++)
            {
                const ScRange & rRange = aRanges[ i ];
                if(pDocument->HasScenarioRange(nTab, rRange))
                {
                    ScScenarioFlags nFlags;
                    pDocument->GetScenarioFlags(nTab,nFlags);
                    bIsEditable = !(nFlags & ScScenarioFlags::Protected);
                }
            }
        }
    }
    if ( bIsEditable )
    {
        if ( HasSelectionMatrixFragment( rMark ) )
        {
            bIsEditable = false;
            if ( pOnlyNotBecauseOfMatrix )
                *pOnlyNotBecauseOfMatrix = true;
        }
        else if ( pOnlyNotBecauseOfMatrix )
            *pOnlyNotBecauseOfMatrix = false;
    }
    else if ( pOnlyNotBecauseOfMatrix )
        *pOnlyNotBecauseOfMatrix = false;
    return bIsEditable;
}

void ScTable::LockTable()
{
    ++nLockCount;
}

void ScTable::UnlockTable()
{
    if (nLockCount)
        --nLockCount;
    else
    {
        OSL_FAIL("UnlockTable without LockTable");
    }
}

void ScTable::MergeSelectionPattern( ScMergePatternState& rState, const ScMarkData& rMark, bool bDeep ) const
{
    std::vector<sc::ColRowSpan> aSpans = rMark.GetMarkedColSpans();

    for (const sc::ColRowSpan & rSpan : aSpans)
    {
        for (SCCOLROW i = rSpan.mnStart; i <= rSpan.mnEnd; ++i)
        {
            aCol[i].MergeSelectionPattern( rState, rMark, bDeep );
        }
    }
}

void ScTable::MergePatternArea( ScMergePatternState& rState, SCCOL nCol1, SCROW nRow1,
                                                    SCCOL nCol2, SCROW nRow2, bool bDeep ) const
{
    for (SCCOL i=nCol1; i<=nCol2; i++)
        aCol[i].MergePatternArea( rState, nRow1, nRow2, bDeep );
}

void ScTable::MergeBlockFrame( SvxBoxItem* pLineOuter, SvxBoxInfoItem* pLineInner, ScLineFlags& rFlags,
                    SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow ) const
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (SCCOL i=nStartCol; i<=nEndCol; i++)
            aCol[i].MergeBlockFrame( pLineOuter, pLineInner, rFlags,
                                    nStartRow, nEndRow, (i==nStartCol), nEndCol-i );
    }
}

void ScTable::ApplyBlockFrame(const SvxBoxItem& rLineOuter, const SvxBoxInfoItem* pLineInner,
                              SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow)
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (SCCOL i=nStartCol; i<=nEndCol; i++)
            aCol[i].ApplyBlockFrame(rLineOuter, pLineInner,
                                    nStartRow, nEndRow, (i==nStartCol), nEndCol-i);
    }
}

void ScTable::ApplyPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyPattern( nRow, rAttr );
}

void ScTable::ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                     const ScPatternAttr& rAttr, ScEditDataArray* pDataArray,
                                     bool* const pIsChanged )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            aCol[i].ApplyPatternArea(nStartRow, nEndRow, rAttr, pDataArray, pIsChanged);
    }
}

void ScTable::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScPatternAttr& rPattern, SvNumFormatType nNewType )
{
    SCCOL nEndCol = rRange.aEnd.Col();
    for ( SCCOL nCol = rRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        aCol[nCol].ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
    }
}

void ScTable::AddCondFormatData( const ScRangeList& rRangeList, sal_uInt32 nIndex )
{
    size_t n = rRangeList.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange & rRange = rRangeList[i];
        SCCOL nColStart = rRange.aStart.Col();
        SCCOL nColEnd = rRange.aEnd.Col();
        SCROW nRowStart = rRange.aStart.Row();
        SCROW nRowEnd = rRange.aEnd.Row();
        for(SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        {
            aCol[nCol].AddCondFormat(nRowStart, nRowEnd, nIndex);
        }
    }
}

void ScTable::RemoveCondFormatData( const ScRangeList& rRangeList, sal_uInt32 nIndex )
{
    size_t n = rRangeList.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange & rRange = rRangeList[i];
        SCCOL nColStart = rRange.aStart.Col();
        SCCOL nColEnd = rRange.aEnd.Col();
        SCROW nRowStart = rRange.aStart.Row();
        SCROW nRowEnd = rRange.aEnd.Row();
        for(SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        {
            aCol[nCol].RemoveCondFormat(nRowStart, nRowEnd, nIndex);
        }
    }
}

void ScTable::ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet* rStyle )
{
    if (ValidColRow(nCol,nRow))
        // If column not exists then we need to create it
        CreateColumnIfNotExists( nCol ).ApplyStyle( nRow, rStyle );
}

void ScTable::ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        if ( nEndCol == MAXCOL )
        {
            if ( nStartCol < aCol.size() )
            {
                // If we would like set all columns to specific style, then change only default style for not existing columns
                nEndCol = aCol.size() - 1;
                for (SCCOL i = nStartCol; i <= nEndCol; i++)
                    aCol[i].ApplyStyleArea(nStartRow, nEndRow, rStyle);
                aDefaultColAttrArray.ApplyStyleArea(nStartRow, nEndRow, rStyle );
            }
            else
            {
                CreateColumnIfNotExists( nStartCol - 1 );
                aDefaultColAttrArray.ApplyStyleArea(nStartRow, nEndRow, rStyle );
            }
        }
        else
        {
            CreateColumnIfNotExists( nEndCol );
            for (SCCOL i = nStartCol; i <= nEndCol; i++)
                aCol[i].ApplyStyleArea(nStartRow, nEndRow, rStyle);
        }
    }
}

void ScTable::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].ApplySelectionStyle( rStyle, rMark );
}

void ScTable::ApplySelectionLineStyle( const ScMarkData& rMark,
                            const ::editeng::SvxBorderLine* pLine, bool bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}

const ScStyleSheet* ScTable::GetStyle( SCCOL nCol, SCROW nRow ) const
{
    if ( !ValidColRow( nCol, nRow ) )
        return nullptr;
    if ( nCol < aCol.size() )
        return aCol[nCol].GetStyle( nRow );
    else
        return aDefaultColAttrArray.GetPattern( nRow )->GetStyleSheet();
}

const ScStyleSheet* ScTable::GetSelectionStyle( const ScMarkData& rMark, bool& rFound ) const
{
    rFound = false;

    bool    bEqual = true;
    bool    bColFound;

    const ScStyleSheet* pStyle = nullptr;
    const ScStyleSheet* pNewStyle;

    for (SCCOL i=0; i < aCol.size() && bEqual; i++)
        if (rMark.HasMultiMarks(i))
        {
            pNewStyle = aCol[i].GetSelectionStyle( rMark, bColFound );
            if (bColFound)
            {
                rFound = true;
                if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                    bEqual = false;
                pStyle = pNewStyle;
            }
        }

    return bEqual ? pStyle : nullptr;
}

const ScStyleSheet* ScTable::GetAreaStyle( bool& rFound, SCCOL nCol1, SCROW nRow1,
                                           SCCOL nCol2, SCROW nRow2 ) const
{
    rFound = false;

    bool    bEqual = true;
    bool    bColFound;

    const ScStyleSheet* pStyle = nullptr;
    const ScStyleSheet* pNewStyle;

    for (SCCOL i=nCol1; i<=nCol2 && bEqual; i++)
    {
        pNewStyle = aCol[i].GetAreaStyle(bColFound, nRow1, nRow2);
        if (bColFound)
        {
            rFound = true;
            if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                bEqual = false;
            pStyle = pNewStyle;
        }
    }

    return bEqual ? pStyle : nullptr;
}

bool ScTable::IsStyleSheetUsed( const ScStyleSheet& rStyle ) const
{
    bool bIsUsed = false;

    for ( SCCOL i=0; i < aCol.size(); i++ )
    {
        if ( aCol[i].IsStyleSheetUsed( rStyle ) )
        {
            bIsUsed = true;
        }
    }

    return bIsUsed;
}

void ScTable::StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, bool bRemoved,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY )
{
    ScFlatBoolRowSegments aUsedRows;
    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].FindStyleSheet(pStyleSheet, aUsedRows, bRemoved);

    sc::RowHeightContext aCxt(nPPTX, nPPTY, rZoomX, rZoomY, pDev);
    SCROW nRow = 0;
    while (nRow <= MAXROW)
    {
        ScFlatBoolRowSegments::RangeData aData;
        if (!aUsedRows.getRangeData(nRow, aData))
            // search failed!
            return;

        SCROW nEndRow = aData.mnRow2;
        if (aData.mbValue)
            SetOptimalHeight(aCxt, nRow, nEndRow);

        nRow = nEndRow + 1;
    }
}

bool ScTable::ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                          ScMF nFlags )
{
    bool bChanged = false;
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            bChanged |= aCol[i].ApplyFlags(nStartRow, nEndRow, nFlags);
    return bChanged;
}

bool ScTable::RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                           ScMF nFlags )
{
    bool bChanged = false;
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            bChanged |= aCol[i].RemoveFlags(nStartRow, nEndRow, nFlags);
    return bChanged;
}

void ScTable::SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].SetPattern( nRow, rAttr );
}

void ScTable::ApplyAttr( SCCOL nCol, SCROW nRow, const SfxPoolItem& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyAttr( nRow, rAttr );
}

void ScTable::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark,
                                   ScEditDataArray* pDataArray, bool* const pIsChanged )
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].ApplySelectionCache( pCache, rMark, pDataArray, pIsChanged );
}

void ScTable::ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark )
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].ChangeSelectionIndent( bIncrement, rMark );
}

void ScTable::ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark )
{
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].ClearSelectionItems( pWhich, rMark );
}

//  Column widths / Row heights

void ScTable::SetColWidth( SCCOL nCol, sal_uInt16 nNewWidth )
{
    if (ValidCol(nCol) && mpColWidth)
    {
        if (!nNewWidth)
        {
            nNewWidth = STD_COL_WIDTH;
        }

        if ( nNewWidth != mpColWidth->GetValue(nCol) )
        {
            mpColWidth->SetValue(nCol, nNewWidth);
            InvalidatePageBreaks();
        }
    }
    else
    {
        OSL_FAIL("Invalid column number or no widths");
    }
}

void ScTable::SetColWidthOnly( SCCOL nCol, sal_uInt16 nNewWidth )
{
    if (!ValidCol(nCol) || !mpColWidth)
        return;

    if (!nNewWidth)
        nNewWidth = STD_COL_WIDTH;

    if (nNewWidth != mpColWidth->GetValue(nCol))
        mpColWidth->SetValue(nCol, nNewWidth);
}

void ScTable::SetRowHeight( SCROW nRow, sal_uInt16 nNewHeight )
{
    if (ValidRow(nRow) && mpRowHeights)
    {
        if (!nNewHeight)
        {
            OSL_FAIL("SetRowHeight: Row height zero");
            nNewHeight = ScGlobal::nStdRowHeight;
        }

        sal_uInt16 nOldHeight = mpRowHeights->getValue(nRow);
        if ( nNewHeight != nOldHeight )
        {
            mpRowHeights->setValue(nRow, nRow, nNewHeight);
            InvalidatePageBreaks();
        }
    }
    else
    {
        OSL_FAIL("Invalid row number or no heights");
    }
}

namespace {

/**
 * Check if the new pixel size is different from the old size between
 * specified ranges.
 */
bool lcl_pixelSizeChanged(
    ScFlatUInt16RowSegments& rRowHeights, SCROW nStartRow, SCROW nEndRow,
    sal_uInt16 nNewHeight, double nPPTY)
{
    long nNewPix = static_cast<long>(nNewHeight * nPPTY);

    ScFlatUInt16RowSegments::ForwardIterator aFwdIter(rRowHeights);
    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        sal_uInt16 nHeight;
        if (!aFwdIter.getValue(nRow, nHeight))
            break;

        if (nHeight != nNewHeight)
        {
            bool bChanged = (nNewPix != static_cast<long>(nHeight * nPPTY));
            if (bChanged)
                return true;
        }

        // Skip ahead to the last position of the current range.
        nRow = aFwdIter.getLastPos();
    }
    return false;
}

}

bool ScTable::SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight,
                                    double nPPTY )
{
    bool bChanged = false;
    if (ValidRow(nStartRow) && ValidRow(nEndRow) && mpRowHeights)
    {
        if (!nNewHeight)
        {
            OSL_FAIL("SetRowHeight: Row height zero");
            nNewHeight = ScGlobal::nStdRowHeight;
        }

        bool bSingle = false;   // true = process every row for its own
        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if (pDrawLayer)
            if (pDrawLayer->HasObjectsInRows( nTab, nStartRow, nEndRow ))
                bSingle = true;

        if (bSingle)
        {
            ScFlatUInt16RowSegments::RangeData aData;
            if (mpRowHeights->getRangeData(nStartRow, aData) &&
                nNewHeight == aData.mnValue && nEndRow <= aData.mnRow2)
            {
                bSingle = false;    // no difference in this range
            }
        }
        if (bSingle)
        {
            if (nEndRow-nStartRow < 20)
            {
                if (!bChanged)
                    bChanged = lcl_pixelSizeChanged(*mpRowHeights, nStartRow, nEndRow, nNewHeight, nPPTY);

                mpRowHeights->setValue(nStartRow, nEndRow, nNewHeight);
            }
            else
            {
                SCROW nMid = (nStartRow+nEndRow) / 2;
                if (SetRowHeightRange( nStartRow, nMid, nNewHeight, 1.0 ))
                    bChanged = true;
                if (SetRowHeightRange( nMid+1, nEndRow, nNewHeight, 1.0 ))
                    bChanged = true;
            }
        }
        else
        {
            if (!bChanged)
                bChanged = lcl_pixelSizeChanged(*mpRowHeights, nStartRow, nEndRow, nNewHeight, nPPTY);

            mpRowHeights->setValue(nStartRow, nEndRow, nNewHeight);
        }

        if (bChanged)
            InvalidatePageBreaks();
    }
    else
    {
        OSL_FAIL("Invalid row number or no heights");
    }

    return bChanged;
}

void ScTable::SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nNewHeight )
{
    if (!ValidRow(nStartRow) || !ValidRow(nEndRow) || !mpRowHeights)
        return;

    if (!nNewHeight)
        nNewHeight = ScGlobal::nStdRowHeight;

    mpRowHeights->setValue(nStartRow, nEndRow, nNewHeight);
}

void ScTable::SetManualHeight( SCROW nStartRow, SCROW nEndRow, bool bManual )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow) && pRowFlags)
    {
        if (bManual)
            pRowFlags->OrValue( nStartRow, nEndRow, CRFlags::ManualSize);
        else
            pRowFlags->AndValue( nStartRow, nEndRow, ~CRFlags::ManualSize);
    }
    else
    {
        OSL_FAIL("Invalid row number or no column flags");
    }
}

sal_uInt16 ScTable::GetColWidth( SCCOL nCol, bool bHiddenAsZero ) const
{
    OSL_ENSURE(ValidCol(nCol),"wrong column number");

    if (ValidCol(nCol) && mpColFlags && mpColWidth)
    {
        if (bHiddenAsZero && ColHidden(nCol))
            return 0;
        else
            return mpColWidth->GetValue(nCol);
    }
    else
        return sal_uInt16(STD_COL_WIDTH);
}

sal_uLong ScTable::GetColWidth( SCCOL nStartCol, SCCOL nEndCol ) const
{
    if (!ValidCol(nStartCol) || !ValidCol(nEndCol) || nStartCol > nEndCol)
        return 0;

    sal_uLong nW = 0;
    bool bHidden = false;
    SCCOL nLastHiddenCol = -1;
    auto colWidthIt = mpColWidth->begin() + nStartCol;
    for (SCCOL nCol = nStartCol; nCol <= nEndCol; (++nCol <= nEndCol) ? ++colWidthIt : (void)false)
    {
        if (nCol > nLastHiddenCol)
            bHidden = ColHidden(nCol, nullptr, &nLastHiddenCol);

        if (bHidden)
            continue;

        nW += *colWidthIt;
    }
    return nW;
}

sal_uInt16 ScTable::GetOriginalWidth( SCCOL nCol ) const        // always the set value
{
    OSL_ENSURE(ValidCol(nCol),"wrong column number");

    if (ValidCol(nCol) && mpColWidth)
        return mpColWidth->GetValue(nCol);
    else
        return sal_uInt16(STD_COL_WIDTH);
}

sal_uInt16 ScTable::GetCommonWidth( SCCOL nEndCol ) const
{
    //  get the width that is used in the largest continuous column range (up to nEndCol)

    if ( !ValidCol(nEndCol) )
    {
        OSL_FAIL("wrong column");
        nEndCol = MAXCOL;
    }

    sal_uInt16 nMaxWidth = 0;
    sal_uInt16 nMaxCount = 0;
    SCCOL nRangeStart = 0;
    while ( nRangeStart <= nEndCol )
    {
        //  skip hidden columns
        while ( nRangeStart <= nEndCol && ColHidden(nRangeStart) )
            ++nRangeStart;
        if ( nRangeStart <= nEndCol )
        {
            sal_uInt16 nThisCount = 0;
            auto colWidthIt = mpColWidth->begin() + nRangeStart;
            sal_uInt16 nThisWidth = *colWidthIt;
            SCCOL nRangeEnd = nRangeStart;
            while ( nRangeEnd <= nEndCol && *colWidthIt == nThisWidth )
            {
                ++nThisCount;
                ++nRangeEnd;
                ++colWidthIt;

                //  skip hidden columns
                while ( nRangeEnd <= nEndCol && ColHidden(nRangeEnd) )
                {
                    ++nRangeEnd;
                    ++colWidthIt;
                }
            }

            if ( nThisCount > nMaxCount )
            {
                nMaxCount = nThisCount;
                nMaxWidth = nThisWidth;
            }

            nRangeStart = nRangeEnd;        // next range
        }
    }

    return nMaxWidth;
}

sal_uInt16 ScTable::GetRowHeight( SCROW nRow, SCROW* pStartRow, SCROW* pEndRow, bool bHiddenAsZero ) const
{
    OSL_ENSURE(ValidRow(nRow),"Invalid row number");

    if (ValidRow(nRow) && mpRowHeights)
    {
        if (bHiddenAsZero && RowHidden( nRow, pStartRow, pEndRow))
            return 0;
        else
        {
            ScFlatUInt16RowSegments::RangeData aData;
            if (!mpRowHeights->getRangeData(nRow, aData))
            {
                if (pStartRow)
                    *pStartRow = nRow;
                if (pEndRow)
                    *pEndRow = nRow;
                // TODO: What should we return in case the search fails?
                return 0;
            }

            // If bHiddenAsZero, pStartRow and pEndRow were initialized to
            // boundaries of a non-hidden segment. Assume that the previous and
            // next segment are hidden then and limit the current height
            // segment.
            if (pStartRow)
                *pStartRow = (bHiddenAsZero ? std::max( *pStartRow, aData.mnRow1) : aData.mnRow1);
            if (pEndRow)
                *pEndRow = (bHiddenAsZero ? std::min( *pEndRow, aData.mnRow2) : aData.mnRow2);
            return aData.mnValue;
        }
    }
    else
    {
        if (pStartRow)
            *pStartRow = nRow;
        if (pEndRow)
            *pEndRow = nRow;
        return ScGlobal::nStdRowHeight;
    }
}

sal_uLong ScTable::GetRowHeight( SCROW nStartRow, SCROW nEndRow, bool bHiddenAsZero ) const
{
    OSL_ENSURE(ValidRow(nStartRow) && ValidRow(nEndRow),"wrong row number");

    if (ValidRow(nStartRow) && ValidRow(nEndRow) && mpRowHeights)
    {
        sal_uLong nHeight = 0;
        SCROW nRow = nStartRow;
        while (nRow <= nEndRow)
        {
            SCROW nLastRow = -1;
            if (!( ( RowHidden(nRow, nullptr, &nLastRow) ) && bHiddenAsZero ) )
            {
                if (nLastRow > nEndRow)
                    nLastRow = nEndRow;
                nHeight += mpRowHeights->getSumValue(nRow, nLastRow);
            }
            nRow = nLastRow + 1;
        }
        return nHeight;
    }
    else
        return (nEndRow - nStartRow + 1) * static_cast<sal_uLong>(ScGlobal::nStdRowHeight);
}

sal_uLong ScTable::GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, double fScale ) const
{
    OSL_ENSURE(ValidRow(nStartRow) && ValidRow(nEndRow),"wrong row number");

    if (ValidRow(nStartRow) && ValidRow(nEndRow) && mpRowHeights)
    {
        sal_uLong nHeight = 0;
        SCROW nRow = nStartRow;
        while (nRow <= nEndRow)
        {
            SCROW nLastRow = -1;
            if (!RowHidden(nRow, nullptr, &nLastRow))
            {
                if (nLastRow > nEndRow)
                    nLastRow = nEndRow;

                // #i117315# can't use getSumValue, because individual values must be rounded
                while (nRow <= nLastRow)
                {
                    ScFlatUInt16RowSegments::RangeData aData;
                    if (!mpRowHeights->getRangeData(nRow, aData))
                        return nHeight;   // shouldn't happen

                    SCROW nSegmentEnd = std::min( nLastRow, aData.mnRow2 );

                    // round-down a single height value, multiply resulting (pixel) values
                    sal_uLong nOneHeight = static_cast<sal_uLong>( aData.mnValue * fScale );
                    nHeight += nOneHeight * ( nSegmentEnd + 1 - nRow );

                    nRow = nSegmentEnd + 1;
                }
            }
            nRow = nLastRow + 1;
        }
        return nHeight;
    }
    else
        return static_cast<sal_uLong>((nEndRow - nStartRow + 1) * ScGlobal::nStdRowHeight * fScale);
}

sal_uInt16 ScTable::GetOriginalHeight( SCROW nRow ) const       // non-0 even if hidden
{
    OSL_ENSURE(ValidRow(nRow),"wrong row number");

    if (ValidRow(nRow) && mpRowHeights)
        return mpRowHeights->getValue(nRow);
    else
        return ScGlobal::nStdRowHeight;
}

//  Column/Row -Flags

SCROW ScTable::GetHiddenRowCount( SCROW nRow ) const
{
    if (!ValidRow(nRow))
        return 0;

    SCROW nLastRow = -1;
    if (!RowHidden(nRow, nullptr, &nLastRow) || !ValidRow(nLastRow))
        return 0;

    return nLastRow - nRow + 1;
}

//TODO: combine ShowRows / DBShowRows

void ScTable::ShowCol(SCCOL nCol, bool bShow)
{
    if (ValidCol(nCol))
    {
        bool bWasVis = !ColHidden(nCol);
        if (bWasVis != bShow)
        {
            SetColHidden(nCol, nCol, !bShow);

            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts )
                pCharts->SetRangeDirty(ScRange( nCol, 0, nTab, nCol, MAXROW, nTab ));
        }
    }
    else
    {
        OSL_FAIL("Invalid column number or no flags");
    }
}

void ScTable::ShowRow(SCROW nRow, bool bShow)
{
    if (ValidRow(nRow) && pRowFlags)
    {
        bool bWasVis = !RowHidden(nRow);
        if (bWasVis != bShow)
        {
            SetRowHidden(nRow, nRow, !bShow);
            if (bShow)
                SetRowFiltered(nRow, nRow, false);
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts )
                pCharts->SetRangeDirty(ScRange( 0, nRow, nTab, MAXCOL, nRow, nTab ));

            InvalidatePageBreaks();
        }
    }
    else
    {
        OSL_FAIL("Invalid row number or no flags");
    }
}

void ScTable::DBShowRow(SCROW nRow, bool bShow)
{
    if (ValidRow(nRow) && pRowFlags)
    {
        //  Always set filter flag; unchanged when Hidden
        bool bChanged = SetRowHidden(nRow, nRow, !bShow);
        SetRowFiltered(nRow, nRow, !bShow);

        if (bChanged)
        {
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts )
                pCharts->SetRangeDirty(ScRange( 0, nRow, nTab, MAXCOL, nRow, nTab ));

            if (pOutlineTable)
                UpdateOutlineRow( nRow, nRow, bShow );

            InvalidatePageBreaks();
        }
    }
    else
    {
        OSL_FAIL("Invalid row number or no flags");
    }
}

void ScTable::DBShowRows(SCROW nRow1, SCROW nRow2, bool bShow)
{
    SCROW nStartRow = nRow1;
    while (nStartRow <= nRow2)
    {
        SCROW nEndRow = -1;
        bool bWasVis = !RowHiddenLeaf(nStartRow, nullptr, &nEndRow);
        if (nEndRow > nRow2)
            nEndRow = nRow2;

        bool bChanged = ( bWasVis != bShow );

        SetRowHidden(nStartRow, nEndRow, !bShow);
        SetRowFiltered(nStartRow, nEndRow, !bShow);

        if ( bChanged )
        {
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts )
                pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));
        }

        nStartRow = nEndRow + 1;
    }

    //  #i12341# For Show/Hide rows, the outlines are updated separately from the outside.
    //  For filtering, the changes aren't visible to the caller, so UpdateOutlineRow has
    //  to be done here.
    if (pOutlineTable)
        UpdateOutlineRow( nRow1, nRow2, bShow );
}

void ScTable::ShowRows(SCROW nRow1, SCROW nRow2, bool bShow)
{
    SCROW nStartRow = nRow1;

    // #i116164# if there are no drawing objects within the row range, a single HeightChanged call is enough
    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    bool bHasObjects = pDrawLayer && pDrawLayer->HasObjectsInRows( nTab, nRow1, nRow2 );

    while (nStartRow <= nRow2)
    {
        SCROW nEndRow = -1;
        bool bWasVis = !RowHiddenLeaf(nStartRow, nullptr, &nEndRow);
        if (nEndRow > nRow2)
            nEndRow = nRow2;

        bool bChanged = ( bWasVis != bShow );

        SetRowHidden(nStartRow, nEndRow, !bShow);
        if (bShow)
            SetRowFiltered(nStartRow, nEndRow, false);

        if ( bChanged )
        {
            ScChartListenerCollection* pCharts = pDocument->GetChartListenerCollection();
            if ( pCharts )
                pCharts->SetRangeDirty(ScRange( 0, nStartRow, nTab, MAXCOL, nEndRow, nTab ));

            InvalidatePageBreaks();
        }

        nStartRow = nEndRow + 1;
    }

    if ( !bHasObjects )
    {
        // #i116164# set the flags for the whole range at once
        SetRowHidden(nRow1, nRow2, !bShow);
        if (bShow)
            SetRowFiltered(nRow1, nRow2, false);
    }
}

bool ScTable::IsDataFiltered(SCCOL nColStart, SCROW nRowStart, SCCOL nColEnd, SCROW nRowEnd) const
{
    for (SCROW i = nRowStart; i <= nRowEnd; ++i)
    {
        if (RowHidden(i))
            return true;
    }
    for (SCCOL i = nColStart; i <= nColEnd; ++i)
    {
        if (ColHidden(i))
            return true;
    }
    return false;
}

bool ScTable::IsDataFiltered(const ScRange& rRange) const
{
    return IsDataFiltered(rRange.aStart.Col(), rRange.aStart.Row(),
                rRange.aEnd.Col(), rRange.aEnd.Row());
}

void ScTable::SetRowFlags( SCROW nRow, CRFlags nNewFlags )
{
    if (ValidRow(nRow) && pRowFlags)
        pRowFlags->SetValue( nRow, nNewFlags);
    else
    {
        OSL_FAIL("Invalid row number or no flags");
    }
}

void ScTable::SetRowFlags( SCROW nStartRow, SCROW nEndRow, CRFlags nNewFlags )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow) && pRowFlags)
        pRowFlags->SetValue( nStartRow, nEndRow, nNewFlags);
    else
    {
        OSL_FAIL("Invalid row number(s) or no flags");
    }
}

CRFlags ScTable::GetColFlags( SCCOL nCol ) const
{
    if (ValidCol(nCol) && mpColFlags)
        return mpColFlags->GetValue(nCol);
    else
        return CRFlags::NONE;
}

CRFlags ScTable::GetRowFlags( SCROW nRow ) const
{
    if (ValidRow(nRow) && pRowFlags)
        return pRowFlags->GetValue(nRow);
    else
        return CRFlags::NONE;
}

SCROW ScTable::GetLastFlaggedRow() const
{
    SCROW nLastFound = 0;
    if (pRowFlags)
    {
        SCROW nRow = pRowFlags->GetLastAnyBitAccess( CRFlags::All );
        if (ValidRow(nRow))
            nLastFound = nRow;
    }

    if (!maRowManualBreaks.empty())
        nLastFound = ::std::max(nLastFound, *maRowManualBreaks.rbegin());

    if (mpHiddenRows)
    {
        SCROW nRow = mpHiddenRows->findLastTrue();
        if (ValidRow(nRow))
            nLastFound = ::std::max(nLastFound, nRow);
    }

    if (mpFilteredRows)
    {
        SCROW nRow = mpFilteredRows->findLastTrue();
        if (ValidRow(nRow))
            nLastFound = ::std::max(nLastFound, nRow);
    }

    return nLastFound;
}

SCCOL ScTable::GetLastChangedCol() const
{
    if ( !mpColFlags )
        return 0;

    SCCOL nLastFound = 0;
    const auto nColSize = aCol.size();
    auto colWidthIt = mpColWidth->begin() + 1;
    for (SCCOL nCol = 1; nCol < nColSize; (++nCol < nColSize) ? ++colWidthIt : (void)false)
        if ((mpColFlags->GetValue(nCol) & CRFlags::All) || (*colWidthIt != STD_COL_WIDTH))
            nLastFound = nCol;

    return nLastFound;
}

SCROW ScTable::GetLastChangedRow() const
{
    if ( !pRowFlags )
        return 0;

    SCROW nLastFlags = GetLastFlaggedRow();

    // Find the last row position where the height is NOT the standard row
    // height.
    // KOHEI: Test this to make sure it does what it's supposed to.
    SCROW nLastHeight = mpRowHeights->findLastTrue(ScGlobal::nStdRowHeight);
    if (!ValidRow(nLastHeight))
        nLastHeight = 0;

    return std::max( nLastFlags, nLastHeight);
}

bool ScTable::UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, bool bShow )
{
    if (pOutlineTable && mpColFlags)
    {
        return pOutlineTable->GetColArray().ManualAction( nStartCol, nEndCol, bShow, *this, true );
    }
    else
        return false;
}

bool ScTable::UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, bool bShow )
{
    if (pOutlineTable && pRowFlags)
        return pOutlineTable->GetRowArray().ManualAction( nStartRow, nEndRow, bShow, *this, false );
    else
        return false;
}

void ScTable::ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 )
{
    // Column-wise expansion

    while (rX1 > 0 && ColHidden(rX1-1))
        --rX1;

    while (rX2 < MAXCOL && ColHidden(rX2+1))
        ++rX2;

    // Row-wise expansion

    if (rY1 > 0)
    {
        ScFlatBoolRowSegments::RangeData aData;
        if (mpHiddenRows->getRangeData(rY1-1, aData) && aData.mbValue)
        {
            SCROW nStartRow = aData.mnRow1;
            if (ValidRow(nStartRow))
                rY1 = nStartRow;
        }
    }
    if (rY2 < MAXROW)
    {
        SCROW nEndRow = -1;
        if (RowHidden(rY2+1, nullptr, &nEndRow) && ValidRow(nEndRow))
            rY2 = nEndRow;
    }
}

void ScTable::StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2 )
{
    while ( rX2>rX1 && ColHidden(rX2) )
        --rX2;
    while ( rX2>rX1 && ColHidden(rX1) )
        ++rX1;

    if (rY1 < rY2)
    {
        ScFlatBoolRowSegments::RangeData aData;
        if (mpHiddenRows->getRangeData(rY2, aData) && aData.mbValue)
        {
            SCROW nStartRow = aData.mnRow1;
            if (ValidRow(nStartRow) && nStartRow >= rY1)
                rY2 = nStartRow;
        }
    }

    if (rY1 < rY2)
    {
        SCROW nEndRow = -1;
        if (RowHidden(rY1, nullptr, &nEndRow) && ValidRow(nEndRow) && nEndRow <= rY2)
            rY1 = nEndRow;
    }
}

//  Auto-Outline

template< typename T >
short DiffSign( T a, T b )
{
    return (a<b) ? -1 :
            (a>b) ? 1 : 0;
}

namespace {

class OutlineArrayFinder
{
    ScRange maRef;
    SCCOL mnCol;
    SCTAB mnTab;
    ScOutlineArray* mpArray;
    bool mbSizeChanged;

public:
    OutlineArrayFinder(const ScRange& rRef, SCCOL nCol, SCTAB nTab, ScOutlineArray* pArray, bool bSizeChanged) :
        maRef(rRef), mnCol(nCol), mnTab(nTab), mpArray(pArray),
        mbSizeChanged(bSizeChanged) {}

    bool operator() (size_t nRow, const ScFormulaCell* pCell)
    {
        SCROW nRow2 = static_cast<SCROW>(nRow);

        if (!pCell->HasRefListExpressibleAsOneReference(maRef))
            return false;

        if (maRef.aStart.Row() != nRow2 || maRef.aEnd.Row() != nRow2 ||
            maRef.aStart.Tab() != mnTab || maRef.aEnd.Tab() != mnTab)
            return false;

        if (DiffSign(maRef.aStart.Col(), mnCol) != DiffSign(maRef.aEnd.Col(), mnCol))
            return false;

        return mpArray->Insert(maRef.aStart.Col(), maRef.aEnd.Col(), mbSizeChanged);
    }
};

}

void ScTable::DoAutoOutline( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    typedef mdds::flat_segment_tree<SCROW, bool> UsedRowsType;

    bool bSizeChanged = false;

    SCCOL nCol;
    SCROW nRow;
    bool bFound;
    ScRange aRef;

    StartOutlineTable();

                            // Rows

    UsedRowsType aUsed(0, MAXROW+1, false);
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        aCol[nCol].FindUsed(nStartRow, nEndRow, aUsed);
    aUsed.build_tree();

    ScOutlineArray& rRowArray = pOutlineTable->GetRowArray();
    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        bool bUsed = false;
        SCROW nLastRow = nRow;
        aUsed.search_tree(nRow, bUsed, nullptr, &nLastRow);
        if (!bUsed)
        {
            nRow = nLastRow;
            continue;
        }

        bFound = false;
        for (nCol=nStartCol; nCol<=nEndCol && !bFound; nCol++)
        {
            ScRefCellValue aCell = aCol[nCol].GetCellValue(nRow);

            if (aCell.meType != CELLTYPE_FORMULA)
                continue;

            if (!aCell.mpFormula->HasRefListExpressibleAsOneReference(aRef))
                continue;

            if ( aRef.aStart.Col() == nCol && aRef.aEnd.Col() == nCol &&
                 aRef.aStart.Tab() == nTab && aRef.aEnd.Tab() == nTab &&
                 DiffSign( aRef.aStart.Row(), nRow ) ==
                    DiffSign( aRef.aEnd.Row(), nRow ) )
            {
                if (rRowArray.Insert( aRef.aStart.Row(), aRef.aEnd.Row(), bSizeChanged ))
                {
                    bFound = true;
                }
            }
        }
    }

    // Column
    ScOutlineArray& rColArray = pOutlineTable->GetColArray();
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        if (aCol[nCol].IsEmptyData())
            continue;

        OutlineArrayFinder aFunc(aRef, nCol, nTab, &rColArray, bSizeChanged);
        sc::FindFormula(aCol[nCol].maCells, nStartRow, nEndRow, aFunc);
    }
}

                                    //  CopyData - for Query in other range

void ScTable::CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab )
{
    //TODO: if used for multiple rows, optimize after columns!

    ScAddress aSrc( nStartCol, nStartRow, nTab );
    ScAddress aDest( nDestCol, nDestRow, nDestTab );
    ScRange aRange( aSrc, aDest );
    bool bThisTab = ( nDestTab == nTab );
    SCROW nDestY = nDestRow;
    for (SCROW nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        aSrc.SetRow( nRow );
        aDest.SetRow( nDestY );
        SCCOL nDestX = nDestCol;
        for (SCCOL nCol=nStartCol; nCol<=nEndCol; nCol++)
        {
            aSrc.SetCol( nCol );
            aDest.SetCol( nDestX );
            ScCellValue aCell;
            aCell.assign(*pDocument, ScAddress(nCol, nRow, nTab));

            if (aCell.meType == CELLTYPE_FORMULA)
            {
                sc::RefUpdateContext aCxt(*pDocument);
                aCxt.meMode = URM_COPY;
                aCxt.maRange = aRange;
                aCxt.mnColDelta = nDestCol - nStartCol;
                aCxt.mnRowDelta = nDestRow - nStartRow;
                aCxt.mnTabDelta = nDestTab - nTab;
                aCell.mpFormula->UpdateReference(aCxt);
                aCell.mpFormula->aPos = aDest;
            }

            if (bThisTab)
            {
                aCell.release(aCol[nDestX], nDestY);
                SetPattern( nDestX, nDestY, *GetPattern( nCol, nRow ) );
            }
            else
            {
                aCell.release(*pDocument, aDest);
                pDocument->SetPattern( aDest, *GetPattern( nCol, nRow ) );
            }

            ++nDestX;
        }
        ++nDestY;
    }
}

bool ScTable::RefVisible(const ScFormulaCell* pCell)
{
    ScRange aRef;

    if (pCell->HasOneReference(aRef))
    {
        if (aRef.aStart.Col()==aRef.aEnd.Col() && aRef.aStart.Tab()==aRef.aEnd.Tab())
        {
            SCROW nEndRow;
            if (!RowFiltered(aRef.aStart.Row(), nullptr, &nEndRow))
                // row not filtered.
                nEndRow = ::std::numeric_limits<SCROW>::max();

            if (!ValidRow(nEndRow) || nEndRow < aRef.aEnd.Row())
                return true;    // at least partly visible
            return false;       // completely invisible
        }
    }

    return true;                        // somehow different
}

void ScTable::GetUpperCellString(SCCOL nCol, SCROW nRow, OUString& rStr)
{
    GetInputString(nCol, nRow, rStr);
    rStr = ScGlobal::pCharClass->uppercase(rStr.trim());
}

// Calculate the size of the sheet and set the size on DrawPage

void ScTable::SetDrawPageSize(bool bResetStreamValid, bool bUpdateNoteCaptionPos)
{
    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if( pDrawLayer )
    {
        double fValX = GetColOffset( MAXCOL + 1 ) * HMM_PER_TWIPS;
        double fValY = GetRowOffset( MAXROW + 1 ) * HMM_PER_TWIPS;
        const long nMax = ::std::numeric_limits<long>::max();
        // #i113884# Avoid int32 overflow with possible negative results than can cause bad effects.
        // If the draw page size is smaller than all rows, only the bottom of the sheet is affected.
        long x = ( fValX > static_cast<double>(nMax) ) ? nMax : static_cast<long>(fValX);
        long y = ( fValY > static_cast<double>(nMax) ) ? nMax : static_cast<long>(fValY);

        if ( IsLayoutRTL() )        // IsNegativePage
            x = -x;

        pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( x, y ), bUpdateNoteCaptionPos );
    }

    // #i102616# actions that modify the draw page size count as sheet modification
    // (exception: InitDrawLayer)
    if (bResetStreamValid)
        SetStreamValid(false);
}

void ScTable::SetRangeName(std::unique_ptr<ScRangeName> pNew)
{
    mpRangeName = std::move(pNew);

    //fdo#39792: mark stream as invalid, otherwise new ScRangeName will not be written to file
    SetStreamValid(false);
}

ScRangeName* ScTable::GetRangeName() const
{
    if (!mpRangeName)
        mpRangeName.reset(new ScRangeName);
    return mpRangeName.get();
}

sal_uLong ScTable::GetRowOffset( SCROW nRow, bool bHiddenAsZero ) const
{
    sal_uLong n = 0;
    if ( mpHiddenRows && mpRowHeights )
    {
        if (nRow == 0)
            return 0;
        else if (nRow == 1)
            return GetRowHeight(0, nullptr, nullptr, bHiddenAsZero );

        n = GetTotalRowHeight(0, nRow-1, bHiddenAsZero);
#if OSL_DEBUG_LEVEL > 0
        if (n == ::std::numeric_limits<unsigned long>::max())
            OSL_FAIL("ScTable::GetRowOffset: row heights overflow");
#endif
    }
    else
    {
        OSL_FAIL("GetRowOffset: Data missing");
    }
    return n;
}

SCROW ScTable::GetRowForHeight(sal_uLong nHeight) const
{
    sal_uLong nSum = 0;

    ScFlatBoolRowSegments::RangeData aData;

    ScFlatUInt16RowSegments::RangeData aRowHeightRange;
    aRowHeightRange.mnRow2 = -1;
    aRowHeightRange.mnValue = 0; // silence MSVC C4701

    for (SCROW nRow = 0; nRow <= MAXROW; ++nRow)
    {
        if (!mpHiddenRows->getRangeData(nRow, aData))
            // Failed to fetch the range data for whatever reason.
            break;

        if (aData.mbValue)
        {
            // This row is hidden.  Skip ahead all hidden rows.
            nRow = aData.mnRow2;
            continue;
        }

        if (aRowHeightRange.mnRow2 < nRow)
        {
            if (!mpRowHeights->getRangeData(nRow, aRowHeightRange))
                // Failed to fetch the range data for whatever reason.
                break;
        }

        nSum += aRowHeightRange.mnValue;

        if (nSum > nHeight)
        {
            if (nRow >= MAXROW)
                return MAXROW;

            // Find the next visible row.
            ++nRow;

            if (!mpHiddenRows->getRangeData(nRow, aData))
                // Failed to fetch the range data for whatever reason.
                break;

            if (aData.mbValue)
                // These rows are hidden.
                nRow = aData.mnRow2 + 1;

            return nRow <= MAXROW ? nRow : MAXROW;
        }
    }
    return -1;
}

sal_uLong ScTable::GetColOffset( SCCOL nCol, bool bHiddenAsZero ) const
{
    sal_uLong n = 0;
    if ( mpColWidth )
    {
        auto colWidthIt = mpColWidth->begin();
        for (SCCOL i = 0; i < nCol; (++i < nCol) ? ++colWidthIt : (void)false)
            if (!( bHiddenAsZero && ColHidden(i) ))
                n += *colWidthIt;
    }
    else
    {
        OSL_FAIL("GetColumnOffset: Data missing");
    }
    return n;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
