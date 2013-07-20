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

#include "table.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "formulacell.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "rechead.hxx"
#include "stlpool.hxx"
#include "attarray.hxx"     // Iterator
#include "markdata.hxx"
#include "progress.hxx"
#include "dociter.hxx"
#include "conditio.hxx"
#include "chartlis.hxx"
#include "fillinfo.hxx"
#include "bcaslot.hxx"
#include "postit.hxx"
#include "sheetevents.hxx"
#include "globstr.hrc"
#include "segmenttree.hxx"
#include "queryparam.hxx"
#include "queryentry.hxx"
#include "dbdata.hxx"
#include "colorscale.hxx"
#include "tokenarray.hxx"
#include "clipcontext.hxx"
#include "types.hxx"
#include "editutil.hxx"
#include "mtvcellfunc.hxx"
#include "refupdatecontext.hxx"

#include "scitems.hxx"
#include <editeng/boxitem.hxx>
#include "editeng/editobj.hxx"
#include <svl/poolcach.hxx>
#include <unotools/charclass.hxx>
#include <math.h>
#include <svl/PasswordHelper.hxx>
#include <unotools/transliterationwrapper.hxx>

// STATIC DATA -----------------------------------------------------------

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
        nOldSizeX = pOutlineTable->GetColArray()->GetDepth();
        nOldSizeY = pOutlineTable->GetRowArray()->GetDepth();
        delete pOutlineTable;
    }

    if (pNewOutline)
    {
        pOutlineTable = new ScOutlineTable( *pNewOutline );
        nNewSizeX = pOutlineTable->GetColArray()->GetDepth();
        nNewSizeY = pOutlineTable->GetRowArray()->GetDepth();
    }
    else
        pOutlineTable = NULL;

    return ( nNewSizeX != nOldSizeX || nNewSizeY != nOldSizeY );        // Groesse geaendert ?
}


void ScTable::StartOutlineTable()
{
    if (!pOutlineTable)
        pOutlineTable = new ScOutlineTable;
}


void ScTable::SetSheetEvents( const ScSheetEvents* pNew )
{
    delete pSheetEvents;
    if (pNew)
        pSheetEvents = new ScSheetEvents(*pNew);
    else
        pSheetEvents = NULL;

    SetCalcNotification( false );       // discard notifications before the events were set

    if (IsStreamValid())
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
            mpRowHeights->insertSegment(nStartRow, nSize, false);
            sal_uInt8 nNewFlags = pRowFlags->Insert( nStartRow, nSize);
            // only copy manual size flag, clear all others
            if (nNewFlags && (nNewFlags != CR_MANUALSIZE))
                pRowFlags->SetValue( nStartRow, nStartRow + nSize - 1,
                        nNewFlags & CR_MANUALSIZE);
        }

        if (pOutlineTable)
            pOutlineTable->InsertRow( nStartRow, nSize );

        mpFilteredRows->insertSegment(nStartRow, nSize, true);
        mpHiddenRows->insertSegment(nStartRow, nSize, true);

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

    // Transfer those notes that will get shifted into another container.
    ScNotes aNotes(pDocument);
    ScNotes::iterator itr = maNotes.begin();
    while( itr != maNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        if (nStartRow <= nRow && nStartCol <= nCol && nCol <= nEndCol)
        {
            aNotes.insert(nCol, nRow + nSize, pPostIt);
            maNotes.ReleaseNote(nCol, nRow);
        }
    }

    // Re-insert the shifted notes.
    itr = aNotes.begin();
    while( itr != aNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        maNotes.insert( nCol, nRow, pPostIt);
        aNotes.ReleaseNote( nCol, nRow);
    }

    InvalidatePageBreaks();

    if (IsStreamValid())
        // TODO: In the future we may want to check if the table has been
        // really modified before setting the stream invalid.
        SetStreamValid(false);
}


void ScTable::DeleteRow( SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCSIZE nSize,
                            bool* pUndoOutline )
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

    // Transfer those notes that will get shifted into another container.
    ScNotes aNotes(pDocument);
    ScNotes::iterator itr = maNotes.begin();
    while( itr != maNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        if (nStartRow <= nRow && nStartCol <= nCol && nCol <= nEndCol)
        {
            SCROW nEndRow = nStartRow + nSize - 1; // last row of deleted region
            if (nEndRow < nRow)
            {
                // This note will get shifted.
                aNotes.insert(nCol, nRow - nSize, pPostIt);
                maNotes.ReleaseNote(nCol, nRow);
            }
            else
                // Note is in the deleted area. Remove it.
                maNotes.erase(nCol, nRow);
        }
    }

    // Re-insert the shifted notes.
    itr = aNotes.begin();
    while( itr != aNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        maNotes.insert( nCol, nRow, pPostIt);
        aNotes.ReleaseNote( nCol, nRow);
    }

    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
        for (SCCOL j=nStartCol; j<=nEndCol; j++)
            aCol[j].DeleteRow( nStartRow, nSize );
    }

    InvalidatePageBreaks();

    if (IsStreamValid())
        // TODO: In the future we may want to check if the table has been
        // really modified before setting the stream invalid.
        SetStreamValid(false);
}


bool ScTable::TestInsertCol( SCROW nStartRow, SCROW nEndRow, SCSIZE nSize ) const
{
    bool bTest = true;

    if ( nStartRow==0 && nEndRow==MAXROW && pOutlineTable )
        bTest = pOutlineTable->TestInsertCol(nSize);

    if ( nSize > static_cast<SCSIZE>(MAXCOL) )
        bTest = false;

    for (SCCOL i=MAXCOL; (i+static_cast<SCCOL>(nSize)>MAXCOL) && bTest; i--)
        bTest = aCol[i].TestInsertCol(nStartRow, nEndRow);

    return bTest;
}


void ScTable::InsertCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize )
{
    if (nStartRow==0 && nEndRow==MAXROW)
    {
        if (pColWidth && pColFlags)
        {
            memmove( &pColWidth[nStartCol+nSize], &pColWidth[nStartCol],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColWidth[0]) );
            memmove( &pColFlags[nStartCol+nSize], &pColFlags[nStartCol],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColFlags[0]) );
        }
        if (pOutlineTable)
            pOutlineTable->InsertCol( nStartCol, nSize );

        mpHiddenCols->insertSegment(nStartCol, static_cast<SCCOL>(nSize), true);
        mpFilteredCols->insertSegment(nStartCol, static_cast<SCCOL>(nSize), true);

        if (!maColManualBreaks.empty())
        {
            std::set<SCCOL>::reverse_iterator rit = maColManualBreaks.rbegin();
            while (rit != maColManualBreaks.rend())
            {
                SCCOL nCol = *rit;
                if (nCol < nStartCol)
                    break;  // while
                else
                {
                    maColManualBreaks.erase( (++rit).base());
                    maColManualBreaks.insert( static_cast<SCCOL>( nCol + nSize));
                }
            }
        }
    }


    if ((nStartRow == 0) && (nEndRow == MAXROW))
    {
        for (SCSIZE i=0; i < nSize; i++)
            for (SCCOL nCol = MAXCOL; nCol > nStartCol; nCol--)
                aCol[nCol].SwapCol(aCol[nCol-1]);
    }
    else
    {
        for (SCSIZE i=0; static_cast<SCCOL>(i+nSize)+nStartCol <= MAXCOL; i++)
            aCol[MAXCOL - nSize - i].MoveTo(nStartRow, nEndRow, aCol[MAXCOL - i]);
    }

    // Transfer those notes that will get shifted into another container.
    ScNotes aNotes(pDocument);
    ScNotes::iterator itr = maNotes.begin();
    while( itr != maNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        if (nStartCol <= nCol && nStartRow <= nRow && nRow <= nEndRow)
        {
            aNotes.insert(nCol + nSize, nRow, pPostIt);
            maNotes.ReleaseNote(nCol, nRow);
        }
    }

    // Re-insert the shifted notes.
    itr = aNotes.begin();
    while( itr != aNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        maNotes.insert( nCol, nRow, pPostIt);
        aNotes.ReleaseNote( nCol, nRow);
    }

    if (nStartCol>0)                        // copy old attributes
    {
        sal_uInt16 nWhichArray[3];
        nWhichArray[0] = ATTR_MERGE;
        nWhichArray[1] = ATTR_CONDITIONAL;
        nWhichArray[2] = 0;

        sc::CopyToDocContext aCxt(*pDocument);
        for (SCSIZE i=0; i<nSize; i++)
        {
            aCol[nStartCol-1].CopyToColumn(aCxt, nStartRow, nEndRow, IDF_ATTRIB,
                                                false, aCol[nStartCol+i] );
            aCol[nStartCol+i].RemoveFlags( nStartRow, nEndRow,
                                                SC_MF_HOR | SC_MF_VER | SC_MF_AUTO );
            aCol[nStartCol+i].ClearItems( nStartRow, nEndRow, nWhichArray );
        }
    }

    InvalidatePageBreaks();

    if (IsStreamValid())
        // TODO: In the future we may want to check if the table has been
        // really modified before setting the stream invalid.
        SetStreamValid(false);
}


void ScTable::DeleteCol( SCCOL nStartCol, SCROW nStartRow, SCROW nEndRow, SCSIZE nSize,
                            bool* pUndoOutline )
{
    if (nStartRow==0 && nEndRow==MAXROW)
    {
        if (pColWidth && pColFlags)
        {
            memmove( &pColWidth[nStartCol], &pColWidth[nStartCol+nSize],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColWidth[0]) );
            memmove( &pColFlags[nStartCol], &pColFlags[nStartCol+nSize],
                    (MAXCOL - nStartCol + 1 - nSize) * sizeof(pColFlags[0]) );
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
            std::set<SCCOL>::iterator it = maColManualBreaks.upper_bound( static_cast<SCCOL>( nStartCol + nSize - 1));
            maColManualBreaks.erase( maColManualBreaks.lower_bound( nStartCol), it);
            while (it != maColManualBreaks.end())
            {
                SCCOL nCol = *it;
                maColManualBreaks.erase( it++);
                maColManualBreaks.insert( static_cast<SCCOL>( nCol - nSize));
            }
        }
    }


    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
        for (SCSIZE i = 0; i < nSize; i++)
            aCol[nStartCol + i].DeleteArea(nStartRow, nEndRow, IDF_ALL);
    }

    if ((nStartRow == 0) && (nEndRow == MAXROW))
    {
        for (SCSIZE i=0; i < nSize; i++)
            for (SCCOL nCol = nStartCol; nCol < MAXCOL; nCol++)
                aCol[nCol].SwapCol(aCol[nCol+1]);
    }
    else
    {
        for (SCSIZE i=0; static_cast<SCCOL>(i+nSize)+nStartCol <= MAXCOL; i++)
            aCol[nStartCol + nSize + i].MoveTo(nStartRow, nEndRow, aCol[nStartCol + i]);
    }

    // Transfer those notes that will get shifted into another container.
    ScNotes aNotes(pDocument);
    ScNotes::iterator itr = maNotes.begin();
    while( itr != maNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        if (nStartCol <= nCol && nStartRow <= nRow && nRow <= nEndRow)
        {
            SCCOL nEndCol = nStartCol + nSize - 1;
            if (nEndCol < nCol)
            {
                // This note will get shifted.
                aNotes.insert(nCol - nSize, nRow, pPostIt);
                maNotes.ReleaseNote(nCol, nRow);
            }
            else
                // The note is in the deleted region. Remove it.
                maNotes.erase(nCol, nRow);
        }
    }

    // Re-insert the shifted notes.
    itr = aNotes.begin();
    while( itr != aNotes.end() )
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;
        ++itr;

        maNotes.insert( nCol, nRow, pPostIt);
        aNotes.ReleaseNote( nCol, nRow);
    }

    InvalidatePageBreaks();

    if (IsStreamValid())
        // TODO: In the future we may want to check if the table has been
        // really modified before setting the stream invalid.
        SetStreamValid(false);
}


void ScTable::DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nDelFlag)
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        {   // scope for bulk broadcast
            ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
            for (SCCOL i = nCol1; i <= nCol2; i++)
                aCol[i].DeleteArea(nRow1, nRow2, nDelFlag);
        }

            //
            // Zellschutz auf geschuetzter Tabelle nicht setzen
            //

        if ( IsProtected() && (nDelFlag & IDF_ATTRIB) )
        {
            ScPatternAttr aPattern(pDocument->GetPool());
            aPattern.GetItemSet().Put( ScProtectionAttr( false ) );
            ApplyPatternArea( nCol1, nRow1, nCol2, nRow2, aPattern );
        }

        if( nDelFlag & IDF_ATTRIB )
            mpCondFormatList->DeleteArea( nCol1, nRow1, nCol2, nRow2 );
    }

    if (nDelFlag & IDF_NOTE)
        maNotes.erase( nCol1, nRow1, nCol2, nRow2 );

    if (IsStreamValid())
        // TODO: In the future we may want to check if the table has been
        // really modified before setting the stream invalid.
        SetStreamValid(false);
}


void ScTable::DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark )
{
    {   // scope for bulk broadcast
        ScBulkBroadcast aBulkBroadcast( pDocument->GetBASM());
        for (SCCOL i=0; i<=MAXCOL; i++)
            aCol[i].DeleteSelection( nDelFlag, rMark );
    }

    ScRangeList aRangeList;
    rMark.FillRangeListWithMarks(&aRangeList, false);

    for (size_t i = 0; i < aRangeList.size(); ++i)
    {
        ScRange* pRange = aRangeList[i];
        if (nDelFlag & IDF_NOTE && pRange)
        {
            maNotes.erase(pRange->aStart.Col(), pRange->aStart.Row(), pRange->aEnd.Col(), pRange->aEnd.Row(), nDelFlag & IDF_NOCAPTIONS);
        }

        if((nDelFlag & IDF_ATTRIB) && pRange && pRange->aStart.Tab() == nTab)
            mpCondFormatList->DeleteArea( pRange->aStart.Col(), pRange->aStart.Row(), pRange->aEnd.Col(), pRange->aEnd.Row() );
    }

        //
        // Zellschutz auf geschuetzter Tabelle nicht setzen
        //

    if ( IsProtected() && (nDelFlag & IDF_ATTRIB) )
    {
        ScDocumentPool* pPool = pDocument->GetPool();
        SfxItemSet aSet( *pPool, ATTR_PATTERN_START, ATTR_PATTERN_END );
        aSet.Put( ScProtectionAttr( false ) );
        SfxItemPoolCache aCache( pPool, &aSet );
        ApplySelectionCache( &aCache, rMark );
    }

    if (IsStreamValid())
        // TODO: In the future we may want to check if the table has been
        // really modified before setting the stream invalid.
        SetStreamValid(false);
}


// pTable = Clipboard
void ScTable::CopyToClip(
    sc::CopyToClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    ScTable* pTable )
{
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        //  copy content
        //local range names need to be copied first for formula cells
        if (!pTable->mpRangeName && mpRangeName)
            pTable->mpRangeName = new ScRangeName(*mpRangeName);

        // notes
        maNotes.clone(
            pTable->pDocument, nCol1, nRow1, nCol2, nRow2, rCxt.isCloneNotes(), nTab, pTable->maNotes);

        SCCOL i;

        for ( i = nCol1; i <= nCol2; i++)
            aCol[i].CopyToClip(rCxt, nRow1, nRow2, pTable->aCol[i]);

        //  copy widths/heights, and only "hidden", "filtered" and "manual" flags
        //  also for all preceding columns/rows, to have valid positions for drawing objects

        if (pColWidth && pTable->pColWidth)
            for (i=0; i<=nCol2; i++)
                pTable->pColWidth[i] = pColWidth[i];

        pTable->CopyColHidden(*this, 0, nCol2);
        pTable->CopyColFiltered(*this, 0, nCol2);
        if (pDBDataNoName)
            pTable->SetAnonymousDBData(new ScDBData(*pDBDataNoName));


        if (pRowFlags && pTable->pRowFlags && mpRowHeights && pTable->mpRowHeights)
        {
            pTable->pRowFlags->CopyFromAnded( *pRowFlags, 0, nRow2, CR_MANUALSIZE);
            pTable->CopyRowHeight(*this, 0, nRow2, 0);
        }

        pTable->CopyRowHidden(*this, 0, nRow2);
        pTable->CopyRowFiltered(*this, 0, nRow2);

        //  ggf. Formeln durch Werte ersetzen

        if ( IsProtected() )
            for (i = nCol1; i <= nCol2; i++)
                pTable->aCol[i].RemoveProtected(nRow1, nRow2);

        pTable->mpCondFormatList.reset(new ScConditionalFormatList(pTable->pDocument, *mpCondFormatList));
    }
}

void ScTable::CopyToClip(
    sc::CopyToClipContext& rCxt, const ScRangeList& rRanges, ScTable* pTable )
{
    ScRangeList aRanges(rRanges);
    for ( size_t i = 0, nListSize = aRanges.size(); i < nListSize; ++i )
    {
        ScRange* p = aRanges[ i ];
        CopyToClip(
            rCxt, p->aStart.Col(), p->aStart.Row(), p->aEnd.Col(), p->aEnd.Row(), pTable);
    }
}

void ScTable::CopyStaticToDocument(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, ScTable* pDestTab)
{
    if (nCol1 > nCol2)
        return;

    for (SCCOL i = nCol1; i <= nCol2; ++i)
    {
        ScColumn& rSrcCol = aCol[i];
        ScColumn& rDestCol = pDestTab->aCol[i];
        rSrcCol.CopyStaticToDocument(nRow1, nRow2, rDestCol);
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

void ScTable::CopyConditionalFormat( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        SCsCOL nDx, SCsROW nDy, ScTable* pTable)
{
    ScRange aOldRange( nCol1 - nDx, nRow1 - nDy, pTable->nTab, nCol2 - nDx, nRow2 - nDy, pTable->nTab);
    ScRange aNewRange( nCol1, nRow1, nTab, nCol2, nRow2, nTab );
    bool bSameDoc = pDocument == pTable->pDocument;

    for(ScConditionalFormatList::const_iterator itr = pTable->mpCondFormatList->begin(),
            itrEnd = pTable->mpCondFormatList->end(); itr != itrEnd; ++itr)
    {
        const ScRangeList& rCondFormatRange = itr->GetRange();
        if(!rCondFormatRange.Intersects( aOldRange ))
            continue;

        ScRangeList aIntersectedRange = rCondFormatRange.GetIntersectedRange(aOldRange);
        ScConditionalFormat* pNewFormat = itr->Clone(pDocument);

        pNewFormat->AddRange(aIntersectedRange);
        pNewFormat->UpdateReference(URM_COPY, aNewRange, nDx, nDy, pTable->nTab - nTab, true);

        sal_uLong nMax = 0;
        for(ScConditionalFormatList::const_iterator itrCond = mpCondFormatList->begin();
                itrCond != mpCondFormatList->end(); ++itrCond)
        {
            if(itrCond->GetKey() > nMax)
                nMax = itrCond->GetKey();
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
                if(pEntry->GetType() == condformat::CONDITION)
                    aStyleName = static_cast<const ScCondFormatEntry*>(pEntry)->GetStyle();
                else if(pEntry->GetType() == condformat::DATE)
                    aStyleName = static_cast<const ScCondDateFormatEntry*>(pEntry)->GetStyleName();

                if(!aStyleName.isEmpty())
                {
                    if(pDocument->GetStyleSheetPool()->Find(aStyleName, SFX_STYLE_FAMILY_PARA))
                        continue;

                    pDocument->GetStyleSheetPool()->CopyStyleFrom(
                            pTable->pDocument->GetStyleSheetPool(), aStyleName, SFX_STYLE_FAMILY_PARA );
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
    SCsCOL nDx, SCsROW nDy, ScTable* pTable )
{

    if (nCol2 > MAXCOL)
        nCol2 = MAXCOL;
    if (nRow2 > MAXROW)
        nRow2 = MAXROW;

    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        for ( SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].CopyFromClip(rCxt, nRow1, nRow2, nDy, pTable->aCol[i - nDx]);


        if (rCxt.getInsertFlag() == IDF_ATTRIB)
        {
            // make sure that there are no old references to the cond formats
            sal_uInt16 nWhichArray[2];
            nWhichArray[0] = ATTR_CONDITIONAL;
            nWhichArray[1] = 0;
            for ( SCCOL i = nCol1; i <= nCol2; ++i)
                aCol[i].ClearItems(nRow1, nRow2, nWhichArray);
        }

        //remove old notes
        if (rCxt.getInsertFlag() & (IDF_NOTE|IDF_ADDNOTES))
            maNotes.erase(nCol1, nRow1, nCol2, nRow2);

        bool bAddNotes = rCxt.getInsertFlag() & (IDF_NOTE | IDF_ADDNOTES);
        if (bAddNotes)
        {
            bool bCloneCaption = (rCxt.getInsertFlag() & IDF_NOCAPTIONS) == 0;
            maNotes.CopyFromClip(pTable->maNotes, pDocument, nCol1, nRow1, nCol2, nRow2, nDx, nDy, nTab, bCloneCaption);
        }


        if ((rCxt.getInsertFlag() & IDF_ATTRIB) != 0)
        {
            if (nRow1==0 && nRow2==MAXROW && pColWidth && pTable->pColWidth)
                for (SCCOL i=nCol1; i<=nCol2; i++)
                    pColWidth[i] = pTable->pColWidth[i-nDx];

            if (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pTable->mpRowHeights &&
                                             pRowFlags && pTable->pRowFlags)
            {
                CopyRowHeight(*pTable, nRow1, nRow2, -nDy);
                // Must copy CR_MANUALSIZE bit too, otherwise pRowHeight doesn't make sense
                for (SCROW j=nRow1; j<=nRow2; j++)
                {
                    if ( pTable->pRowFlags->GetValue(j-nDy) & CR_MANUALSIZE )
                        pRowFlags->OrValue( j, CR_MANUALSIZE);
                    else
                        pRowFlags->AndValue( j, sal::static_int_cast<sal_uInt8>(~CR_MANUALSIZE));
                }
            }

            // Zellschutz auf geschuetzter Tabelle nicht setzen
            if (IsProtected() && (rCxt.getInsertFlag() & IDF_ATTRIB))
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
    sal_uInt16 nFunction, bool bSkipEmpty, const ScTable* pSrcTab )
{
    for (SCCOL i=nCol1; i<=nCol2; i++)
        aCol[i].MixData(rCxt, nRow1, nRow2, nFunction, bSkipEmpty, pSrcTab->aCol[i]);
}


//  Markierung von diesem Dokument
void ScTable::MixMarked(
    sc::MixDocContext& rCxt, const ScMarkData& rMark, sal_uInt16 nFunction,
    bool bSkipEmpty, const ScTable* pSrcTab )
{
    for (SCCOL i=0; i<=MAXCOL; i++)
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
        return new ScFormulaCell(&mrClipTab.GetDoc(), rDestPos, &aArr);
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

    void operator() (size_t nRow, const OUString& rStr)
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
            *p, mrClipTab.GetDoc(), getDestPos(nRow), SC_CLONECELL_STARTLISTENING);

        //  Referenzen drehen
        //  bei Cut werden Referenzen spaeter per UpdateTranspose angepasst

        if (!mbWasCut)
            pNew->TransposeReference();

        SCCOL nTransCol = nRow - mnTopRow;
        mrClipTab.SetFormulaCell(nTransCol, mnTransRow, pNew);
    }
};

}

void ScTable::TransposeClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                ScTable* pTransClip, sal_uInt16 nFlags, bool bAsLink )
{
    bool bWasCut = pDocument->IsCutMode();

    ScDocument* pDestDoc = pTransClip->pDocument;

    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
    {
        SCROW nRow;
        if ( bAsLink && nFlags == IDF_ALL )
        {
            //  with IDF_ALL, also create links (formulas) for empty cells

            for ( nRow=nRow1; nRow<=nRow2; nRow++ )
            {
                //  create simple formula, as in ScColumn::CreateRefCell

                ScAddress aDestPos( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), pTransClip->nTab );
                ScSingleRefData aRef;
                aRef.nCol = nCol;
                aRef.nRow = nRow;
                aRef.nTab = nTab;
                aRef.InitFlags();                           // -> all absolute
                aRef.SetFlag3D(true);
                aRef.CalcRelFromAbs( aDestPos );
                ScTokenArray aArr;
                aArr.AddSingleReference( aRef );

                pTransClip->SetFormulaCell(
                    static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1),
                    new ScFormulaCell(pDestDoc, aDestPos, &aArr));
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
        boost::scoped_ptr<ScAttrIterator> pAttrIter(aCol[nCol].CreateAttrIterator( nRow1, nRow2 ));
        while ( (pPattern = pAttrIter->Next( nAttrRow1, nAttrRow2 )) != 0 )
        {
            if ( !IsDefaultItem( pPattern ) )
            {
                const SfxItemSet& rSet = pPattern->GetItemSet();
                if ( rSet.GetItemState( ATTR_MERGE, false ) == SFX_ITEM_DEFAULT &&
                     rSet.GetItemState( ATTR_MERGE_FLAG, false ) == SFX_ITEM_DEFAULT &&
                     rSet.GetItemState( ATTR_BORDER, false ) == SFX_ITEM_DEFAULT )
                {
                    // no borders or merge items involved - use pattern as-is
                    for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( static_cast<SCCOL>(nRow-nRow1), static_cast<SCROW>(nCol-nCol1), *pPattern, true );
                }
                else
                {
                    // transpose borders and merge values, remove merge flags (refreshed after pasting)
                    ScPatternAttr aNewPattern( *pPattern );
                    SfxItemSet& rNewSet = aNewPattern.GetItemSet();

                    const SvxBoxItem& rOldBox = (const SvxBoxItem&)rSet.Get(ATTR_BORDER);
                    if ( rOldBox.GetTop() || rOldBox.GetBottom() || rOldBox.GetLeft() || rOldBox.GetRight() )
                    {
                        SvxBoxItem aNew( ATTR_BORDER );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_TOP ), BOX_LINE_LEFT );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_LEFT ), BOX_LINE_TOP );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_BOTTOM ), BOX_LINE_RIGHT );
                        aNew.SetLine( rOldBox.GetLine( BOX_LINE_RIGHT ), BOX_LINE_BOTTOM );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_TOP ), BOX_LINE_LEFT );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_LEFT ), BOX_LINE_TOP );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_BOTTOM ), BOX_LINE_RIGHT );
                        aNew.SetDistance( rOldBox.GetDistance( BOX_LINE_RIGHT ), BOX_LINE_BOTTOM );
                        rNewSet.Put( aNew );
                    }

                    const ScMergeAttr& rOldMerge = (const ScMergeAttr&)rSet.Get(ATTR_MERGE);
                    if (rOldMerge.IsMerged())
                        rNewSet.Put( ScMergeAttr( std::min(
                                        static_cast<SCsCOL>(rOldMerge.GetRowMerge()),
                                        static_cast<SCsCOL>(MAXCOL+1 - (nAttrRow2-nRow1))),
                                    std::min(
                                        static_cast<SCsROW>(rOldMerge.GetColMerge()),
                                        static_cast<SCsROW>(MAXROW+1 - (nCol-nCol1)))));
                    const ScMergeFlagAttr& rOldFlag = (const ScMergeFlagAttr&)rSet.Get(ATTR_MERGE_FLAG);
                    if (rOldFlag.IsOverlapped())
                    {
                        sal_Int16 nNewFlags = rOldFlag.GetValue() & ~( SC_MF_HOR | SC_MF_VER );
                        if ( nNewFlags )
                            rNewSet.Put( ScMergeFlagAttr( nNewFlags ) );
                        else
                            rNewSet.ClearItem( ATTR_MERGE_FLAG );
                    }

                    for (nRow = nAttrRow1; nRow<=nAttrRow2; nRow++)
                        pTransClip->SetPattern( static_cast<SCCOL>(nRow-nRow1),
                                static_cast<SCROW>(nCol-nCol1), aNewPattern, true);
                }
            }
        }
    }
}


void ScTable::StartAllListeners()
{
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].StartAllListeners();
}


void ScTable::StartNeededListeners()
{
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].StartNeededListeners();
}


void ScTable::BroadcastInArea( SCCOL nCol1, SCROW nRow1,
        SCCOL nCol2, SCROW nRow2 )
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
        for (SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].SetDirty(nRow1, nRow2);
}


void ScTable::StartListeningInArea(
    sc::StartListeningContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (nCol2 > MAXCOL) nCol2 = MAXCOL;
    if (nRow2 > MAXROW) nRow2 = MAXROW;
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
        for (SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].StartListeningInArea(rCxt, nRow1, nRow2);
}


void ScTable::CopyToTable(
    sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    sal_uInt16 nFlags, bool bMarked, ScTable* pDestTab, const ScMarkData* pMarkData,
    bool bAsLink, bool bColRowFlags )
{
    if (!ValidColRow(nCol1, nRow1) || !ValidColRow(nCol2, nRow2))
        return;

    if (nFlags)
        for (SCCOL i = nCol1; i <= nCol2; i++)
            aCol[i].CopyToColumn(rCxt, nRow1, nRow2, nFlags, bMarked,
                                pDestTab->aCol[i], pMarkData, bAsLink);

    if (!bColRowFlags)      // Spaltenbreiten/Zeilenhoehen/Flags
        return;

    //remove old notes
    if (nFlags & IDF_NOTE)
        pDestTab->maNotes.erase(nCol1, nRow1, nCol2, nRow2);

    bool bAddNotes = nFlags & (IDF_NOTE | IDF_ADDNOTES);
    if (bAddNotes)
    {
        bool bCloneCaption = (nFlags & IDF_NOCAPTIONS) == 0;
        pDestTab->maNotes.CopyFromClip(maNotes, pDestTab->pDocument, nCol1, nRow1, nCol2, nRow2, 0, 0, pDestTab->nTab, bCloneCaption);
    }

    if(pDestTab->pDocument->IsUndo() && (nFlags & IDF_ATTRIB))
    {
        pDestTab->mpCondFormatList.reset(new ScConditionalFormatList(pDestTab->pDocument, *mpCondFormatList));
    }

    if (pDBDataNoName)
    {
        ScDBData* pNewDBData = new ScDBData(*pDBDataNoName);
        SCCOL aCol1, aCol2;
        SCROW aRow1, aRow2;
        SCTAB aTab;
        pNewDBData->GetArea(aTab, aCol1, aRow1, aCol2, aRow2);
        pNewDBData->MoveTo(pDestTab->nTab, aCol1, aRow1, aCol2, aRow2);
        pDestTab->SetAnonymousDBData(pNewDBData);
    }
    //  Charts muessen beim Ein-/Ausblenden angepasst werden
    ScChartListenerCollection* pCharts = pDestTab->pDocument->GetChartListenerCollection();

    bool bFlagChange = false;

    bool bWidth  = (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth);
    bool bHeight = (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pDestTab->mpRowHeights);

    if (bWidth || bHeight)
    {
        if (bWidth)
        {
            for (SCCOL i = nCol1; i <= nCol2; ++i)
            {
                bool bThisHidden = ColHidden(i);
                bool bHiddenChange = (pDestTab->ColHidden(i) != bThisHidden);
                bool bChange = bHiddenChange || (pDestTab->pColWidth[i] != pColWidth[i]);
                pDestTab->pColWidth[i] = pColWidth[i];
                pDestTab->pColFlags[i] = pColFlags[i];
                pDestTab->SetColHidden(i, i, bThisHidden);
                //! Aenderungen zusammenfassen?
                if (bHiddenChange && pCharts)
                    pCharts->SetRangeDirty(ScRange( i, 0, nTab, i, MAXROW, nTab ));

                if (bChange)
                    bFlagChange = true;
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
                bool bHidden = RowHidden(i, NULL, &nLastRow);
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
                bool bFiltered = RowFiltered(i, NULL, &nLastRow);
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

    if(nFlags & IDF_ATTRIB)
    {
        pDestTab->mpCondFormatList->DeleteArea(nCol1, nRow1, nCol2, nRow2);
        pDestTab->CopyConditionalFormat(nCol1, nRow1, nCol2, nRow2, 0, 0, this);
    }

    pDestTab->SetOutlineTable( pOutlineTable );     // auch nur wenn bColRowFlags
}


void ScTable::UndoToTable(
    sc::CopyToDocContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    sal_uInt16 nFlags, bool bMarked, ScTable* pDestTab, const ScMarkData* pMarkData )
{
    if (ValidColRow(nCol1, nRow1) && ValidColRow(nCol2, nRow2))
    {
        bool bWidth  = (nRow1==0 && nRow2==MAXROW && pColWidth && pDestTab->pColWidth);
        bool bHeight = (nCol1==0 && nCol2==MAXCOL && mpRowHeights && pDestTab->mpRowHeights);

        for ( SCCOL i = 0; i <= MAXCOL; i++)
        {
            if ( i >= nCol1 && i <= nCol2 )
                aCol[i].UndoToColumn(rCxt, nRow1, nRow2, nFlags, bMarked, pDestTab->aCol[i], pMarkData);
            else
                aCol[i].CopyToColumn(rCxt, 0, MAXROW, IDF_FORMULA, false, pDestTab->aCol[i]);
        }

        //remove old notes
        if (nFlags & IDF_CONTENTS)
            pDestTab->maNotes.erase(nCol1, nRow1, nCol2, nRow2);

        if (nFlags & IDF_ATTRIB)
            pDestTab->mpCondFormatList.reset(new ScConditionalFormatList(pDestTab->pDocument, *mpCondFormatList));

        bool bAddNotes = nFlags & (IDF_NOTE | IDF_ADDNOTES);
        if (bAddNotes)
        {
            bool bCloneCaption = (nFlags & IDF_NOCAPTIONS) == 0;
            pDestTab->maNotes.CopyFromClip(maNotes, pDocument, nCol1, nRow1, nCol2, nRow2, 0, 0, pDestTab->nTab, bCloneCaption);
        }

        if (bWidth||bHeight)
        {
            if (bWidth)
            {
                for (SCCOL i=nCol1; i<=nCol2; i++)
                    pDestTab->pColWidth[i] = pColWidth[i];
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
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].CopyUpdated( pPosTab->aCol[i], pDestTab->aCol[i] );

    // insert notes with captions
    for(ScNotes::iterator itr = pDestTab->maNotes.begin(); itr != pDestTab->maNotes.end(); ++itr)
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;
        ScPostIt* pPostIt = itr->second;

        pDestTab->maNotes.insert(nCol, nRow, pPostIt->Clone( ScAddress(nCol, nRow, nTab),*pDestTab->pDocument, ScAddress(nCol, nRow, pDestTab->nTab), true ));
    }
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

    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].CopyScenarioTo( pDestTab->aCol[i] );
}

void ScTable::CopyScenarioFrom( const ScTable* pSrcTab )
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].CopyScenarioFrom( pSrcTab->aCol[i] );
}

void ScTable::MarkScenarioIn( ScMarkData& rDestMark, sal_uInt16 nNeededBits ) const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    if ( ( nScenarioFlags & nNeededBits ) != nNeededBits )  // alle Bits gesetzt?
        return;

    for (SCCOL i=0; i<=MAXCOL; i++)
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
            const ScRange* pR = (*pList)[j];
            if ( pR->Intersects( aTabRange ) )
                return true;
        }
    }

    return false;
}

void ScTable::InvalidateScenarioRanges()
{
    delete pScenarioRanges;
    pScenarioRanges = NULL;
}

const ScRangeList* ScTable::GetScenarioRanges() const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    if (!pScenarioRanges)
    {
        ((ScTable*)this)->pScenarioRanges = new ScRangeList;
        ScMarkData aMark;
        MarkScenarioIn( aMark, 0 );     // immer
        aMark.FillRangeListWithMarks( pScenarioRanges, false );
    }
    return pScenarioRanges;
}

bool ScTable::TestCopyScenarioTo( const ScTable* pDestTab ) const
{
    OSL_ENSURE( bScenario, "bScenario == FALSE" );

    if (!pDestTab->IsProtected())
        return true;

    bool bOk = true;
    for (SCCOL i=0; i<=MAXCOL && bOk; i++)
        bOk = aCol[i].TestCopyScenarioTo( pDestTab->aCol[i] );
    return bOk;
}

bool ScTable::SetString( SCCOL nCol, SCROW nRow, SCTAB nTabP, const String& rString,
                         ScSetStringParam* pParam )
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].SetString(
            nRow, nTabP, rString, pDocument->GetAddressConvention(), pParam );
    else
        return false;
}

void ScTable::SetEditText( SCCOL nCol, SCROW nRow, EditTextObject* pEditText )
{
    if (!ValidColRow(nCol, nRow))
    {
        delete pEditText;
        return;
    }

    aCol[nCol].SetEditText(nRow, pEditText);
}

void ScTable::SetEditText( SCCOL nCol, SCROW nRow, const EditTextObject& rEditText, const SfxItemPool* pEditPool )
{
    if (!ValidColRow(nCol, nRow))
        return;

    aCol[nCol].SetEditText(nRow, rEditText, pEditPool);
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
        return NULL;
    }

    return aCol[nCol].SetFormulaCell(nRow, pCell);
}

void ScTable::SetValue( SCCOL nCol, SCROW nRow, const double& rVal )
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetValue( nRow, rVal );
}

void ScTable::SetRawString( SCCOL nCol, SCROW nRow, const OUString& rStr )
{
    if (ValidColRow(nCol, nRow))
        aCol[nCol].SetRawString(nRow, rStr);
}

void ScTable::GetString( SCCOL nCol, SCROW nRow, OUString& rString ) const
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetString( nRow, rString );
    else
        rString = OUString();
}

const OUString* ScTable::GetStringCell( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol,nRow))
        return NULL;

    return aCol[nCol].GetStringCell(nRow);
}

double* ScTable::GetValueCell( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol,nRow))
        return NULL;

    return aCol[nCol].GetValueCell(nRow);
}

void ScTable::GetInputString( SCCOL nCol, SCROW nRow, OUString& rString ) const
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].GetInputString( nRow, rString );
    else
        rString = OUString();
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
        return NULL;

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
        rFormula = OUString();
}

const ScTokenArray* ScTable::GetFormulaTokens( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return NULL;

    return aCol[nCol].GetFormulaTokens(nRow);
}

const ScFormulaCell* ScTable::GetFormulaCell( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return NULL;

    return aCol[nCol].GetFormulaCell(nRow);
}

ScFormulaCell* ScTable::GetFormulaCell( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol, nRow))
        return NULL;

    return aCol[nCol].GetFormulaCell(nRow);
}

ScNotes* ScTable::GetNotes()
{
    return &maNotes;
}


void ScTable::InitializeNoteCaptions( bool bForced )
{
    if( mxUninitNotes.get() && (bForced || pDocument->IsUndoEnabled()) )
    {
        for( ScAddress2DVec::iterator aIt = mxUninitNotes->begin(), aEnd = mxUninitNotes->end(); aIt != aEnd; ++aIt )
            if( ScPostIt* pNote = maNotes.findByAddress( aIt->first, aIt->second ) )
                pNote->GetOrCreateCaption( ScAddress( aIt->first, aIt->second, nTab ) );
        mxUninitNotes.reset();
    }
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
    while (aCol[rCol].IsEmptyData() && rCol < MAXCOL)
        ++rCol;
    SCCOL nCol = rCol;
    while (nCol <= MAXCOL && rRow > 0)
    {
        if (!aCol[nCol].IsEmptyData())
            rRow = ::std::min( rRow, aCol[nCol].GetFirstDataPos());
        ++nCol;
    }
}

void ScTable::GetLastDataPos(SCCOL& rCol, SCROW& rRow) const
{
    rCol = MAXCOL;
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
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].SetDirtyVar();
}


void ScTable::SetDirty()
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].SetDirty();
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetDirty( const ScRange& rRange )
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    SCCOL nCol2 = rRange.aEnd.Col();
    for (SCCOL i=rRange.aStart.Col(); i<=nCol2; i++)
        aCol[i].SetDirty(rRange.aStart.Row(), rRange.aEnd.Row());
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
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].SetDirtyAfterLoad();
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetRelNameDirty()
{
    bool bOldAutoCalc = pDocument->GetAutoCalc();
    pDocument->SetAutoCalc( false );    // Mehrfachberechnungen vermeiden
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].SetRelNameDirty();
    pDocument->SetAutoCalc( bOldAutoCalc );
}


void ScTable::SetLoadingMedium(bool bLoading)
{
    mpRowHeights->enableTreeSearch(!bLoading);
}


void ScTable::CalcAll()
{
    for (SCCOL i=0; i<=MAXCOL; i++) aCol[i].CalcAll();
}


void ScTable::CompileAll()
{
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].CompileAll();

    if(mpCondFormatList)
        mpCondFormatList->CompileAll();
}


void ScTable::CompileXML( ScProgress& rProgress )
{
    if (mpRangeName)
        mpRangeName->CompileUnresolvedXML();

    for (SCCOL i=0; i <= MAXCOL; i++)
    {
        aCol[i].CompileXML( rProgress );
    }

    if(mpCondFormatList)
        mpCondFormatList->CompileXML();
}

bool ScTable::CompileErrorCells(sal_uInt16 nErrCode)
{
    bool bCompiled = false;
    for (SCCOL i = 0; i <= MAXCOL; ++i)
    {
        if (aCol[i].CompileErrorCells(nErrCode))
            bCompiled = true;
    }

    return bCompiled;
}

void ScTable::CalcAfterLoad()
{
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].CalcAfterLoad();
}

bool ScTable::IsEmptyData( SCCOL nCol ) const
{
    if (!ValidCol(nCol))
        return true;

    return aCol[nCol].IsEmptyData();
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
        return aCol[nCol].GetAttr( nRow, nWhich );
    else
        return NULL;
}

sal_uInt32 ScTable::GetNumberFormat( const ScAddress& rPos ) const
{
    return ValidColRow(rPos.Col(),rPos.Row()) ?
        aCol[rPos.Col()].GetNumberFormat( rPos.Row() ) :
        0;
}

sal_uInt32 ScTable::GetNumberFormat( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol,nRow))
        return aCol[nCol].GetNumberFormat( nRow );
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
        return NULL;
}


bool ScTable::HasAttrib( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, sal_uInt16 nMask ) const
{
    bool bFound = false;
    for (SCCOL i=nCol1; i<=nCol2 && !bFound; i++)
        bFound |= aCol[i].HasAttrib( nRow1, nRow2, nMask );
    return bFound;
}


bool ScTable::HasAttribSelection( const ScMarkData& rMark, sal_uInt16 nMask ) const
{
    bool bFound = false;
    for (SCCOL i=0; i<=MAXCOL && !bFound; i++)
        bFound |= aCol[i].HasAttribSelection( rMark, nMask );
    return bFound;
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
    bool bFound = false;
    SCCOL nOldEndX = rEndCol;
    SCROW nOldEndY = rEndRow;
    for (SCCOL i=nStartCol; i<=nOldEndX; i++)
        bFound |= aCol[i].ExtendMerge( i, nStartRow, nOldEndY, rEndCol, rEndRow, bRefresh );
    return bFound;
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
        if (!bIgnoreNotes)
        {
            for (ScNotes::const_iterator itr = maNotes.begin(); itr != maNotes.end() && bEmpty; ++itr)
            {
                SCCOL nCol = itr->first.first;
                SCROW nRow = itr->first.second;

                if (nCol >= nCol1 && nCol <= nCol2 && nRow >= nRow1 && nRow <= nRow2)
                    bEmpty = false;
            }
        }
    }
    return bEmpty;
}

SCSIZE ScTable::FillMaxRot( RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nX1, SCCOL nX2,
                            SCCOL nCol, SCROW nAttrRow1, SCROW nAttrRow2, SCSIZE nArrY,
                            const ScPatternAttr* pPattern, const SfxItemSet* pCondSet )
{
    //  Rueckgabe = neues nArrY

    sal_uInt8 nRotDir = pPattern->GetRotateDir( pCondSet );
    if ( nRotDir != SC_ROTDIR_NONE )
    {
        bool bHit = true;
        if ( nCol+1 < nX1 )                             // column to the left
            bHit = ( nRotDir != SC_ROTDIR_LEFT );
        else if ( nCol > nX2+1 )                        // column to the right
            bHit = ( nRotDir != SC_ROTDIR_RIGHT );      // SC_ROTDIR_STANDARD may now also be extended to the left

        if ( bHit )
        {
            double nFactor = 0.0;
            if ( nCol > nX2+1 )
            {
                long nRotVal = ((const SfxInt32Item&) pPattern->
                        GetItem( ATTR_ROTATE_VALUE, pCondSet )).GetValue();
                double nRealOrient = nRotVal * F_PI18000;   // 1/100 Grad
                double nCos = cos( nRealOrient );
                double nSin = sin( nRealOrient );
                //! begrenzen !!!
                //! zusaetzlich Faktor fuer unterschiedliche PPT X/Y !!!

                //  bei SC_ROTDIR_LEFT kommt immer ein negativer Wert heraus,
                //  wenn der Modus beruecksichtigt wird
                nFactor = -fabs( nCos / nSin );
            }

            for ( SCROW nRow = nAttrRow1; nRow <= nAttrRow2; nRow++ )
            {
                if (!RowHidden(nRow))
                {
                    bool bHitOne = true;
                    if ( nCol > nX2+1 )
                    {
                        // reicht die gedrehte Zelle bis in den sichtbaren Bereich?

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
    if ( !pColWidth || !mpRowHeights || !pColFlags || !pRowFlags )
    {
        OSL_FAIL( "Row/column info missing" );
        return;
    }

    //  nRotMaxCol ist auf SC_ROTMAX_NONE initialisiert, nRowNo ist schon gesetzt

    SCROW nY1 = pRowInfo[0].nRowNo;
    SCROW nY2 = pRowInfo[nArrCount-1].nRowNo;

    for (SCCOL nCol=0; nCol<=MAXCOL; nCol++)
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
                        == SFX_ITEM_SET )
                {
                    //  alle Formate durchgehen, damit die Zellen nicht einzeln
                    //  angeschaut werden muessen

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
                                    if(pEntry->GetType() != condformat::CONDITION)
                                        continue;

                                    String aStyleName = static_cast<const ScCondFormatEntry*>(pEntry)->GetStyle();
                                    if (aStyleName.Len())
                                    {
                                        SfxStyleSheetBase* pStyleSheet =
                                            pStylePool->Find( aStyleName, SFX_STYLE_FAMILY_PARA );
                                        if ( pStyleSheet )
                                        {
                                            FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
                                                    nCol, nAttrRow1, nAttrRow2,
                                                    nArrY, pPattern, &pStyleSheet->GetItemSet() );
                                            //  nArrY nicht veraendern
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                nArrY = FillMaxRot( pRowInfo, nArrCount, nX1, nX2,
                                    nCol, nAttrRow1, nAttrRow2,
                                    nArrY, pPattern, NULL );

                pPattern = aIter.GetNext( nAttrCol, nAttrRow1, nAttrRow2 );
            }
        }
    }
}

bool ScTable::HasBlockMatrixFragment( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    using namespace sc;

    sal_uInt16 nEdges = 0;

    if ( nCol1 == nCol2 )
    {   // linke und rechte Spalte
        const sal_uInt16 n = MatrixEdgeLeft | MatrixEdgeRight;
        nEdges = aCol[nCol1].GetBlockMatrixEdges( nRow1, nRow2, n );
        // nicht (4 und 16) oder 1 oder 32
        if (nEdges && (((nEdges & n) != n) || (nEdges & (MatrixEdgeInside|MatrixEdgeOpen))))
            return true;        // linke oder rechte Kante fehlt oder offen
    }
    else
    {   // linke Spalte
        nEdges = aCol[nCol1].GetBlockMatrixEdges(nRow1, nRow2, MatrixEdgeLeft);
        // nicht 4 oder 1 oder 32
        if (nEdges && (((nEdges & MatrixEdgeLeft) != MatrixEdgeLeft) || (nEdges & (MatrixEdgeInside|MatrixEdgeOpen))))
            return true;        // linke Kante fehlt oder offen
        // rechte Spalte
        nEdges = aCol[nCol2].GetBlockMatrixEdges(nRow1, nRow2, MatrixEdgeRight);
        // nicht 16 oder 1 oder 32
        if (nEdges && (((nEdges & MatrixEdgeRight) != MatrixEdgeRight) || (nEdges & (MatrixEdgeInside|MatrixEdgeOpen))))
            return true;        // rechte Kante fehlt oder offen
    }

    if ( nRow1 == nRow2 )
    {   // obere und untere Zeile
        bool bOpen = false;
        const sal_uInt16 n = MatrixEdgeBottom | MatrixEdgeTop;
        for ( SCCOL i=nCol1; i<=nCol2; i++)
        {
            nEdges = aCol[i].GetBlockMatrixEdges( nRow1, nRow1, n );
            if ( nEdges )
            {
                if ( (nEdges & n) != n )
                    return true;        // obere oder untere Kante fehlt
                if (nEdges & MatrixEdgeLeft)
                    bOpen = true;       // linke Kante oeffnet, weitersehen
                else if ( !bOpen )
                    return true;        // es gibt was, was nicht geoeffnet wurde
                if (nEdges & MatrixEdgeRight)
                    bOpen = false;      // rechte Kante schliesst
            }
        }
        if ( bOpen )
            return true;                // es geht noch weiter
    }
    else
    {
        sal_uInt16 j, n;
        SCROW nR;
        // erst obere Zeile, dann untere Zeile
        for ( j=0, nR=nRow1, n=8; j<2; j++, nR=nRow2, n=2 )
        {
            bool bOpen = false;
            for ( SCCOL i=nCol1; i<=nCol2; i++)
            {
                nEdges = aCol[i].GetBlockMatrixEdges( nR, nR, n );
                if ( nEdges )
                {
                    // in oberere Zeile keine obere Kante bzw.
                    // in unterer Zeile keine untere Kante
                    if ( (nEdges & n) != n )
                        return true;
                    if (nEdges & MatrixEdgeLeft)
                        bOpen = true;       // linke Kante oeffnet, weitersehen
                    else if ( !bOpen )
                        return true;        // es gibt was, was nicht geoeffnet wurde
                    if (nEdges & MatrixEdgeRight)
                        bOpen = false;      // rechte Kante schliesst
                }
            }
            if ( bOpen )
                return true;                // es geht noch weiter
        }
    }
    return false;
}


bool ScTable::HasSelectionMatrixFragment( const ScMarkData& rMark ) const
{
    bool bFound = false;
    for (SCCOL i=0; i<=MAXCOL && !bFound; i++)
        bFound |= aCol[i].HasSelectionMatrixFragment(rMark);
    return bFound;
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
        bIsEditable = !HasAttrib( nCol1, nRow1, nCol2, nRow2, HASATTR_PROTECTED );
        if(bIsEditable)
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
                    sal_uInt16 nFlags;
                    pDocument->GetScenarioFlags(nScenTab,nFlags);
                    bIsEditable = !((nFlags & SC_SCENARIO_PROTECT) && (nFlags & SC_SCENARIO_TWOWAY));
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
                sal_uInt16 nFlags;
                pDocument->GetScenarioFlags(nTab,nFlags);
                bIsEditable = !(nFlags & SC_SCENARIO_PROTECT);
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
        if((bIsEditable = !HasAttribSelection( rMark, HASATTR_PROTECTED )) != false)
        {
            // If Sheet is protected and cells are not protected then
            // check the active scenario protect flag if this area is
            // in the active scenario range.
            ScRangeList aRanges;
            rMark.FillRangeListWithMarks( &aRanges, false );
            SCTAB nScenTab = nTab+1;
            while(pDocument->IsScenario(nScenTab) && bIsEditable)
            {
                if(pDocument->IsActiveScenario(nScenTab))
                {
                    for (size_t i=0, nRange = aRanges.size(); (i < nRange) && bIsEditable; i++ )
                    {
                        ScRange aRange = *aRanges[ i ];
                        if(pDocument->HasScenarioRange(nScenTab, aRange))
                        {
                            sal_uInt16 nFlags;
                            pDocument->GetScenarioFlags(nScenTab,nFlags);
                            bIsEditable = !((nFlags & SC_SCENARIO_PROTECT) && (nFlags & SC_SCENARIO_TWOWAY));
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
                ScRange aRange = *aRanges[ i ];
                if(pDocument->HasScenarioRange(nTab, aRange))
                {
                    sal_uInt16 nFlags;
                    pDocument->GetScenarioFlags(nTab,nFlags);
                    bIsEditable = !(nFlags & SC_SCENARIO_PROTECT);
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
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].MergeSelectionPattern( rState, rMark, bDeep );
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


void ScTable::ApplyBlockFrame( const SvxBoxItem* pLineOuter, const SvxBoxInfoItem* pLineInner,
                    SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (SCCOL i=nStartCol; i<=nEndCol; i++)
            aCol[i].ApplyBlockFrame( pLineOuter, pLineInner,
                                    nStartRow, nEndRow, (i==nStartCol), nEndCol-i );
    }
}


void ScTable::ApplyPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyPattern( nRow, rAttr );
}


void ScTable::ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                     const ScPatternAttr& rAttr, ScEditDataArray* pDataArray )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            aCol[i].ApplyPatternArea(nStartRow, nEndRow, rAttr, pDataArray);
    }
}

bool ScTable::SetAttrEntries(SCCOL nCol, ScAttrEntry* pData, SCSIZE nSize)
{
    if (!ValidCol(nCol))
        return false;

    return aCol[nCol].SetAttrEntries(pData, nSize);
}

void ScTable::ApplyPatternIfNumberformatIncompatible( const ScRange& rRange,
        const ScPatternAttr& rPattern, short nNewType )
{
    SCCOL nEndCol = rRange.aEnd.Col();
    for ( SCCOL nCol = rRange.aStart.Col(); nCol <= nEndCol; nCol++ )
    {
        aCol[nCol].ApplyPatternIfNumberformatIncompatible( rRange, rPattern, nNewType );
    }
}

void ScTable::AddCondFormatData( const ScRangeList& rRange, sal_uInt32 nIndex )
{
    size_t n = rRange.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = rRange[i];
        SCCOL nColStart = pRange->aStart.Col();
        SCCOL nColEnd = pRange->aEnd.Col();
        SCROW nRowStart = pRange->aStart.Row();
        SCROW nRowEnd = pRange->aEnd.Row();
        for(SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        {
            aCol[nCol].AddCondFormat(nRowStart, nRowEnd, nIndex);
        }
    }
}

void ScTable::RemoveCondFormatData( const ScRangeList& rRange, sal_uInt32 nIndex )
{
    size_t n = rRange.size();
    for(size_t i = 0; i < n; ++i)
    {
        const ScRange* pRange = rRange[i];
        SCCOL nColStart = pRange->aStart.Col();
        SCCOL nColEnd = pRange->aEnd.Col();
        SCROW nRowStart = pRange->aStart.Row();
        SCROW nRowEnd = pRange->aEnd.Row();
        for(SCCOL nCol = nColStart; nCol <= nColEnd; ++nCol)
        {
            aCol[nCol].RemoveCondFormat(nRowStart, nRowEnd, nIndex);
        }
    }
}



void ScTable::ApplyStyle( SCCOL nCol, SCROW nRow, const ScStyleSheet& rStyle )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyStyle( nRow, rStyle );
}


void ScTable::ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, const ScStyleSheet& rStyle )
{
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
    {
        PutInOrder(nStartCol, nEndCol);
        PutInOrder(nStartRow, nEndRow);
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            aCol[i].ApplyStyleArea(nStartRow, nEndRow, rStyle);
    }
}


void ScTable::ApplySelectionStyle(const ScStyleSheet& rStyle, const ScMarkData& rMark)
{
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].ApplySelectionStyle( rStyle, rMark );
}


void ScTable::ApplySelectionLineStyle( const ScMarkData& rMark,
                            const ::editeng::SvxBorderLine* pLine, bool bColorOnly )
{
    if ( bColorOnly && !pLine )
        return;

    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].ApplySelectionLineStyle( rMark, pLine, bColorOnly );
}


const ScStyleSheet* ScTable::GetStyle( SCCOL nCol, SCROW nRow ) const
{
    if (ValidColRow(nCol, nRow))
        return aCol[nCol].GetStyle(nRow);
    else
        return NULL;
}


const ScStyleSheet* ScTable::GetSelectionStyle( const ScMarkData& rMark, bool& rFound ) const
{
    rFound = false;

    bool    bEqual = true;
    bool    bColFound;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    for (SCCOL i=0; i<=MAXCOL && bEqual; i++)
        if (rMark.HasMultiMarks(i))
        {
            pNewStyle = aCol[i].GetSelectionStyle( rMark, bColFound );
            if (bColFound)
            {
                rFound = true;
                if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                    bEqual = false;                                             // unterschiedliche
                pStyle = pNewStyle;
            }
        }

    return bEqual ? pStyle : NULL;
}


const ScStyleSheet* ScTable::GetAreaStyle( bool& rFound, SCCOL nCol1, SCROW nRow1,
                                           SCCOL nCol2, SCROW nRow2 ) const
{
    rFound = false;

    bool    bEqual = true;
    bool    bColFound;

    const ScStyleSheet* pStyle = NULL;
    const ScStyleSheet* pNewStyle;

    for (SCCOL i=nCol1; i<=nCol2 && bEqual; i++)
    {
        pNewStyle = aCol[i].GetAreaStyle(bColFound, nRow1, nRow2);
        if (bColFound)
        {
            rFound = true;
            if ( !pNewStyle || ( pStyle && pNewStyle != pStyle ) )
                bEqual = false;                                             // unterschiedliche
            pStyle = pNewStyle;
        }
    }

    return bEqual ? pStyle : NULL;
}


bool ScTable::IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const
{
    bool bIsUsed = false;

    for ( SCCOL i=0; i<=MAXCOL; i++ )
    {
        if ( aCol[i].IsStyleSheetUsed( rStyle, bGatherAllStyles ) )
        {
            if ( !bGatherAllStyles )
                return true;
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
    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].FindStyleSheet(pStyleSheet, aUsedRows, bRemoved);

    SCROW nRow = 0;
    while (nRow <= MAXROW)
    {
        ScFlatBoolRowSegments::RangeData aData;
        if (!aUsedRows.getRangeData(nRow, aData))
            // search failed!
            return;

        SCROW nEndRow = aData.mnRow2;
        if (aData.mbValue)
            SetOptimalHeight(nRow, nEndRow, 0, pDev, nPPTX, nPPTY, rZoomX, rZoomY, false);

        nRow = nEndRow + 1;
    }
}


bool ScTable::ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                          sal_Int16 nFlags )
{
    bool bChanged = false;
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            bChanged |= aCol[i].ApplyFlags(nStartRow, nEndRow, nFlags);
    return bChanged;
}


bool ScTable::RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                           sal_Int16 nFlags )
{
    bool bChanged = false;
    if (ValidColRow(nStartCol, nStartRow) && ValidColRow(nEndCol, nEndRow))
        for (SCCOL i = nStartCol; i <= nEndCol; i++)
            bChanged |= aCol[i].RemoveFlags(nStartRow, nEndRow, nFlags);
    return bChanged;
}


void ScTable::SetPattern( SCCOL nCol, SCROW nRow, const ScPatternAttr& rAttr, bool bPutToPool )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].SetPattern( nRow, rAttr, bPutToPool );
}


void ScTable::ApplyAttr( SCCOL nCol, SCROW nRow, const SfxPoolItem& rAttr )
{
    if (ValidColRow(nCol,nRow))
        aCol[nCol].ApplyAttr( nRow, rAttr );
}


void ScTable::ApplySelectionCache( SfxItemPoolCache* pCache, const ScMarkData& rMark,
                                   ScEditDataArray* pDataArray )
{
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].ApplySelectionCache( pCache, rMark, pDataArray );
}


void ScTable::ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark )
{
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].ChangeSelectionIndent( bIncrement, rMark );
}


void ScTable::ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark )
{
    for (SCCOL i=0; i<=MAXCOL; i++)
        aCol[i].ClearSelectionItems( pWhich, rMark );
}


//  Spaltenbreiten / Zeilenhoehen

void ScTable::SetColWidth( SCCOL nCol, sal_uInt16 nNewWidth )
{
    if (ValidCol(nCol) && pColWidth)
    {
        if (!nNewWidth)
        {
            nNewWidth = STD_COL_WIDTH;
        }

        if ( nNewWidth != pColWidth[nCol] )
        {
            pColWidth[nCol] = nNewWidth;
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
    if (!ValidCol(nCol) || !pColWidth)
        return;

    if (!nNewWidth)
        nNewWidth = STD_COL_WIDTH;

    if (nNewWidth != pColWidth[nCol])
        pColWidth[nCol] = nNewWidth;
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
                                    double /* nPPTX */, double nPPTY )
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
            mpRowHeights->getRangeData(nStartRow, aData);
            if (nNewHeight == aData.mnValue && nEndRow <= aData.mnRow2)
                bSingle = false;    // no difference in this range
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
                if (SetRowHeightRange( nStartRow, nMid, nNewHeight, 1.0, 1.0 ))
                    bChanged = true;
                if (SetRowHeightRange( nMid+1, nEndRow, nNewHeight, 1.0, 1.0 ))
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
            pRowFlags->OrValue( nStartRow, nEndRow, CR_MANUALSIZE);
        else
            pRowFlags->AndValue( nStartRow, nEndRow, sal::static_int_cast<sal_uInt8>(~CR_MANUALSIZE));
    }
    else
    {
        OSL_FAIL("Invalid row number or no column flags");
    }
}


sal_uInt16 ScTable::GetColWidth( SCCOL nCol, bool bHiddenAsZero ) const
{
    OSL_ENSURE(ValidCol(nCol),"wrong column number");

    if (ValidCol(nCol) && pColFlags && pColWidth)
    {
        if (bHiddenAsZero && ColHidden(nCol))
            return 0;
        else
            return pColWidth[nCol];
    }
    else
        return (sal_uInt16) STD_COL_WIDTH;
}


sal_uInt16 ScTable::GetOriginalWidth( SCCOL nCol ) const        // immer die eingestellte
{
    OSL_ENSURE(ValidCol(nCol),"wrong column number");

    if (ValidCol(nCol) && pColWidth)
        return pColWidth[nCol];
    else
        return (sal_uInt16) STD_COL_WIDTH;
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
            sal_uInt16 nThisWidth = pColWidth[nRangeStart];
            SCCOL nRangeEnd = nRangeStart;
            while ( nRangeEnd <= nEndCol && pColWidth[nRangeEnd] == nThisWidth )
            {
                ++nThisCount;
                ++nRangeEnd;

                //  skip hidden columns
                while ( nRangeEnd <= nEndCol && ColHidden(nRangeEnd) )
                    ++nRangeEnd;
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
        return (sal_uInt16) ScGlobal::nStdRowHeight;
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
            if (!( ( RowHidden(nRow, NULL, &nLastRow) ) && bHiddenAsZero ) )
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
        return (sal_uLong) ((nEndRow - nStartRow + 1) * ScGlobal::nStdRowHeight);
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
            if (!RowHidden(nRow, NULL, &nLastRow))
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
        return (sal_uLong) ((nEndRow - nStartRow + 1) * ScGlobal::nStdRowHeight * fScale);
}


sal_uInt16 ScTable::GetOriginalHeight( SCROW nRow ) const       // non-0 even if hidden
{
    OSL_ENSURE(ValidRow(nRow),"wrong row number");

    if (ValidRow(nRow) && mpRowHeights)
        return mpRowHeights->getValue(nRow);
    else
        return (sal_uInt16) ScGlobal::nStdRowHeight;
}


//  Spalten-/Zeilen-Flags


SCROW ScTable::GetHiddenRowCount( SCROW nRow ) const
{
    if (!ValidRow(nRow))
        return 0;

    SCROW nLastRow = -1;
    if (!RowHidden(nRow, NULL, &nLastRow) || !ValidRow(nLastRow))
        return 0;

    return nLastRow - nRow + 1;
}


//!     ShowRows / DBShowRows zusammenfassen

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
        //  Filter-Flag immer setzen, auch wenn Hidden unveraendert
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
        bool bWasVis = !RowHiddenLeaf(nStartRow, NULL, &nEndRow);
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
        bool bWasVis = !RowHiddenLeaf(nStartRow, NULL, &nEndRow);
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

void ScTable::SetRowFlags( SCROW nRow, sal_uInt8 nNewFlags )
{
    if (ValidRow(nRow) && pRowFlags)
        pRowFlags->SetValue( nRow, nNewFlags);
    else
    {
        OSL_FAIL("Invalid row number or no flags");
    }
}


void ScTable::SetRowFlags( SCROW nStartRow, SCROW nEndRow, sal_uInt8 nNewFlags )
{
    if (ValidRow(nStartRow) && ValidRow(nEndRow) && pRowFlags)
        pRowFlags->SetValue( nStartRow, nEndRow, nNewFlags);
    else
    {
        OSL_FAIL("Invalid row number(s) or no flags");
    }
}


sal_uInt8 ScTable::GetColFlags( SCCOL nCol ) const
{
    if (ValidCol(nCol) && pColFlags)
        return pColFlags[nCol];
    else
        return 0;
}


sal_uInt8 ScTable::GetRowFlags( SCROW nRow ) const
{
    if (ValidRow(nRow) && pRowFlags)
        return pRowFlags->GetValue(nRow);
    else
        return 0;
}


SCROW ScTable::GetLastFlaggedRow() const
{
    SCROW nLastFound = 0;
    if (pRowFlags)
    {
        SCROW nRow = pRowFlags->GetLastAnyBitAccess( 0, sal::static_int_cast<sal_uInt8>(CR_ALL) );
        if (ValidRow(nRow))
            nLastFound = nRow;
    }

    if (!maRowManualBreaks.empty())
        nLastFound = ::std::max(nLastFound, *maRowManualBreaks.rbegin());

    if (mpHiddenRows)
    {
        SCROW nRow = mpHiddenRows->findLastNotOf(false);
        if (ValidRow(nRow))
            nLastFound = ::std::max(nLastFound, nRow);
    }

    if (mpFilteredRows)
    {
        SCROW nRow = mpFilteredRows->findLastNotOf(false);
        if (ValidRow(nRow))
            nLastFound = ::std::max(nLastFound, nRow);
    }

    return nLastFound;
}


SCCOL ScTable::GetLastChangedCol() const
{
    if ( !pColFlags )
        return 0;

    SCCOL nLastFound = 0;
    for (SCCOL nCol = 1; nCol <= MAXCOL; nCol++)
        if ((pColFlags[nCol] & CR_ALL) || (pColWidth[nCol] != STD_COL_WIDTH))
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
    SCROW nLastHeight = mpRowHeights->findLastNotOf(ScGlobal::nStdRowHeight);
    if (!ValidRow(nLastHeight))
        nLastHeight = 0;

    return std::max( nLastFlags, nLastHeight);
}


bool ScTable::UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, bool bShow )
{
    if (pOutlineTable && pColFlags)
    {
        ScBitMaskCompressedArray< SCCOLROW, sal_uInt8> aArray( MAXCOL, pColFlags, MAXCOLCOUNT);
        return pOutlineTable->GetColArray()->ManualAction( nStartCol, nEndCol, bShow, *this, true );
    }
    else
        return false;
}


bool ScTable::UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, bool bShow )
{
    if (pOutlineTable && pRowFlags)
        return pOutlineTable->GetRowArray()->ManualAction( nStartRow, nEndRow, bShow, *this, false );
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
        if (RowHidden(rY2+1, NULL, &nEndRow) && ValidRow(nEndRow))
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
        if (RowHidden(rY1, NULL, &nEndRow) && ValidRow(nEndRow) && nEndRow <= rY2)
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
    ScOutlineArray* pArray;
    ScRange aRef;

    StartOutlineTable();

                            // Zeilen

    UsedRowsType aUsed(0, MAXROW+1, false);
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
        aCol[nCol].FindUsed(nStartRow, nEndRow, aUsed);
    aUsed.build_tree();

    pArray = pOutlineTable->GetRowArray();
    for (nRow=nStartRow; nRow<=nEndRow; nRow++)
    {
        bool bUsed = false;
        SCROW nLastRow = nRow;
        aUsed.search_tree(nRow, bUsed, NULL, &nLastRow);
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
                if (pArray->Insert( aRef.aStart.Row(), aRef.aEnd.Row(), bSizeChanged ))
                {
                    bFound = true;
                }
            }
        }
    }

    // Column
    pArray = pOutlineTable->GetColArray();
    for (nCol=nStartCol; nCol<=nEndCol; nCol++)
    {
        if (aCol[nCol].IsEmptyData())
            continue;

        OutlineArrayFinder aFunc(aRef, nCol, nTab, pArray, bSizeChanged);
        sc::FindFormula(aCol[nCol].maCells, nStartRow, nEndRow, aFunc);
    }
}

                                    //  CopyData - fuer Query in anderen Bereich

void ScTable::CopyData( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                            SCCOL nDestCol, SCROW nDestRow, SCTAB nDestTab )
{
    //!     wenn fuer mehrere Zeilen benutzt, nach Spalten optimieren!

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
                SetPattern( nDestX, nDestY, *GetPattern( nCol, nRow ), true );
            }
            else
            {
                aCell.release(*pDocument, aDest);
                pDocument->SetPattern( aDest, *GetPattern( nCol, nRow ), true );
            }

            ++nDestX;
        }
        ++nDestY;
    }
}


bool ScTable::RefVisible(ScFormulaCell* pCell)
{
    ScRange aRef;

    if (pCell->HasOneReference(aRef))
    {
        if (aRef.aStart.Col()==aRef.aEnd.Col() && aRef.aStart.Tab()==aRef.aEnd.Tab())
        {
            SCROW nEndRow;
            if (!RowFiltered(aRef.aStart.Row(), NULL, &nEndRow))
                // row not filtered.
                nEndRow = ::std::numeric_limits<SCROW>::max();

            if (!ValidRow(nEndRow) || nEndRow < aRef.aEnd.Row())
                return true;    // at least partly visible
            return false;       // completely invisible
        }
    }

    return true;                        // irgendwie anders
}


void ScTable::GetUpperCellString(SCCOL nCol, SCROW nRow, OUString& rStr)
{
    GetInputString(nCol, nRow, rStr);
    rStr = ScGlobal::pCharClass->uppercase(rStr.trim());
}


// Berechnen der Groesse der Tabelle und setzen der Groesse an der DrawPage

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
        long x = ( fValX > (double)nMax ) ? nMax : (long) fValX;
        long y = ( fValY > (double)nMax ) ? nMax : (long) fValY;

        if ( IsLayoutRTL() )        // IsNegativePage
            x = -x;

        pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( x, y ), bUpdateNoteCaptionPos );
    }

    // #i102616# actions that modify the draw page size count as sheet modification
    // (exception: InitDrawLayer)
    if (bResetStreamValid && IsStreamValid())
        SetStreamValid(false);
}

void ScTable::SetRangeName(ScRangeName* pNew)
{
    delete mpRangeName;
    mpRangeName = pNew;

    //fdo#39792: mark stream as invalid, otherwise new ScRangeName will not be written to file
    if (IsStreamValid())
        SetStreamValid(false);
}

ScRangeName* ScTable::GetRangeName() const
{
    if (!mpRangeName)
        mpRangeName = new ScRangeName;
    return mpRangeName;
}


sal_uLong ScTable::GetRowOffset( SCROW nRow, bool bHiddenAsZero ) const
{
    sal_uLong n = 0;
    if ( mpHiddenRows && mpRowHeights )
    {
        if (nRow == 0)
            return 0;
        else if (nRow == 1)
            return GetRowHeight(0, NULL, NULL, bHiddenAsZero );

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
    sal_uInt32 nSum = 0;

    ScFlatBoolRowSegments::RangeData aData;
    for (SCROW nRow = 0; nRow <= MAXROW; ++nRow)
    {
        if (!mpHiddenRows->getRangeData(nRow, aData))
            break;

        if (aData.mbValue)
        {
            nRow = aData.mnRow2;
            continue;
        }

        sal_uInt32 nNew = mpRowHeights->getValue(nRow);
        nSum += nNew;
        if (nSum > nHeight)
        {
            return nRow < MAXROW ? nRow + 1 : MAXROW;
        }
    }
    return -1;
}


sal_uLong ScTable::GetColOffset( SCCOL nCol, bool bHiddenAsZero ) const
{
    sal_uLong n = 0;
    if ( pColWidth )
    {
        SCCOL i;
        for( i = 0; i < nCol; i++ )
            if (!( bHiddenAsZero && ColHidden(i) ))
                n += pColWidth[i];
    }
    else
    {
        OSL_FAIL("GetColumnOffset: Data missing");
    }
    return n;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
