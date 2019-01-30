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

#include <sal/config.h>
#include <sal/log.hxx>

#include <unotools/transliterationwrapper.hxx>
#include <unotools/charclass.hxx>

#include <dbdata.hxx>
#include <globalnames.hxx>
#include <refupdat.hxx>
#include <rechead.hxx>
#include <document.hxx>
#include <queryparam.hxx>
#include <queryentry.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <subtotalparam.hxx>
#include <sortparam.hxx>
#include <dociter.hxx>
#include <brdcst.hxx>

#include <comphelper/stl_types.hxx>

#include <memory>
#include <utility>

using ::std::unique_ptr;
using ::std::for_each;
using ::std::find_if;
using ::std::remove_if;
using ::std::pair;

bool ScDBData::less::operator() (const std::unique_ptr<ScDBData>& left, const std::unique_ptr<ScDBData>& right) const
{
    return ScGlobal::GetpTransliteration()->compareString(left->GetUpperName(), right->GetUpperName()) < 0;
}

ScDBData::ScDBData( const OUString& rName,
                    SCTAB nTab,
                    SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                    bool bByR, bool bHasH, bool bTotals) :
    // Listeners are to be setup by the "parent" container.
    mpSortParam(new ScSortParam),
    mpQueryParam(new ScQueryParam),
    mpSubTotal(new ScSubTotalParam),
    mpImportParam(new ScImportParam),
    mpContainer (nullptr),
    aName       (rName),
    aUpper      (rName),
    nTable      (nTab),
    nStartCol   (nCol1),
    nStartRow   (nRow1),
    nEndCol     (nCol2),
    nEndRow     (nRow2),
    bByRow      (bByR),
    bHasHeader  (bHasH),
    bHasTotals  (bTotals),
    bDoSize     (false),
    bKeepFmt    (false),
    bStripData  (false),
    bIsAdvanced (false),
    bDBSelection(false),
    nIndex      (0),
    bAutoFilter (false),
    bModified   (false),
    mbTableColumnNamesDirty(true),
    nFilteredRowCount(0)
{
    aUpper = ScGlobal::pCharClass->uppercase(aUpper);
}

ScDBData::ScDBData( const ScDBData& rData ) :
    // Listeners are to be setup by the "parent" container.
    SvtListener         (),
    ScRefreshTimer      ( rData ),
    mpSortParam(new ScSortParam(*rData.mpSortParam)),
    mpQueryParam(new ScQueryParam(*rData.mpQueryParam)),
    mpSubTotal(new ScSubTotalParam(*rData.mpSubTotal)),
    mpImportParam(new ScImportParam(*rData.mpImportParam)),
    mpContainer         (nullptr),
    aName               (rData.aName),
    aUpper              (rData.aUpper),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bHasTotals          (rData.bHasTotals),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bDBSelection        (rData.bDBSelection),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified),
    maTableColumnNames  (rData.maTableColumnNames),
    mbTableColumnNamesDirty(rData.mbTableColumnNamesDirty),
    nFilteredRowCount   (rData.nFilteredRowCount)
{
}

ScDBData::ScDBData( const OUString& rName, const ScDBData& rData ) :
    // Listeners are to be setup by the "parent" container.
    SvtListener         (),
    ScRefreshTimer      ( rData ),
    mpSortParam(new ScSortParam(*rData.mpSortParam)),
    mpQueryParam(new ScQueryParam(*rData.mpQueryParam)),
    mpSubTotal(new ScSubTotalParam(*rData.mpSubTotal)),
    mpImportParam(new ScImportParam(*rData.mpImportParam)),
    mpContainer         (nullptr),
    aName               (rName),
    aUpper              (rName),
    nTable              (rData.nTable),
    nStartCol           (rData.nStartCol),
    nStartRow           (rData.nStartRow),
    nEndCol             (rData.nEndCol),
    nEndRow             (rData.nEndRow),
    bByRow              (rData.bByRow),
    bHasHeader          (rData.bHasHeader),
    bHasTotals          (rData.bHasTotals),
    bDoSize             (rData.bDoSize),
    bKeepFmt            (rData.bKeepFmt),
    bStripData          (rData.bStripData),
    bIsAdvanced         (rData.bIsAdvanced),
    aAdvSource          (rData.aAdvSource),
    bDBSelection        (rData.bDBSelection),
    nIndex              (rData.nIndex),
    bAutoFilter         (rData.bAutoFilter),
    bModified           (rData.bModified),
    maTableColumnNames  (rData.maTableColumnNames),
    mbTableColumnNamesDirty (rData.mbTableColumnNamesDirty),
    nFilteredRowCount   (rData.nFilteredRowCount)
{
    aUpper = ScGlobal::pCharClass->uppercase(aUpper);
}

ScDBData& ScDBData::operator= (const ScDBData& rData)
{
    if (this != &rData)
    {
        // Don't modify the name.  The name is not mutable as it is used as a key
        // in the container to keep the db ranges sorted by the name.

        bool bHeaderRangeDiffers = (nTable != rData.nTable || nStartCol != rData.nStartCol ||
                nEndCol != rData.nEndCol || nStartRow != rData.nStartRow);
        bool bNeedsListening = ((bHasHeader && bHeaderRangeDiffers) || (!bHasHeader && rData.bHasHeader));
        if (bHasHeader && (!rData.bHasHeader || bHeaderRangeDiffers))
        {
            EndTableColumnNamesListener();
        }
        ScRefreshTimer::operator=( rData );
        mpSortParam.reset(new ScSortParam(*rData.mpSortParam));
        mpQueryParam.reset(new ScQueryParam(*rData.mpQueryParam));
        mpSubTotal.reset(new ScSubTotalParam(*rData.mpSubTotal));
        mpImportParam.reset(new ScImportParam(*rData.mpImportParam));
        // Keep mpContainer.
        nTable              = rData.nTable;
        nStartCol           = rData.nStartCol;
        nStartRow           = rData.nStartRow;
        nEndCol             = rData.nEndCol;
        nEndRow             = rData.nEndRow;
        bByRow              = rData.bByRow;
        bHasHeader          = rData.bHasHeader;
        bHasTotals          = rData.bHasTotals;
        bDoSize             = rData.bDoSize;
        bKeepFmt            = rData.bKeepFmt;
        bStripData          = rData.bStripData;
        bIsAdvanced         = rData.bIsAdvanced;
        aAdvSource          = rData.aAdvSource;
        bDBSelection        = rData.bDBSelection;
        nIndex              = rData.nIndex;
        bAutoFilter         = rData.bAutoFilter;
        nFilteredRowCount   = rData.nFilteredRowCount;

        if (bHeaderRangeDiffers)
            InvalidateTableColumnNames( true);
        else
        {
            maTableColumnNames  = rData.maTableColumnNames;
            mbTableColumnNamesDirty = rData.mbTableColumnNamesDirty;
        }

        if (bNeedsListening)
            StartTableColumnNamesListener();
    }
    return *this;
}

bool ScDBData::operator== (const ScDBData& rData) const
{
    // Data that is not in sort or query params.

    if ( nTable     != rData.nTable     ||
         bDoSize    != rData.bDoSize    ||
         bKeepFmt   != rData.bKeepFmt   ||
         bIsAdvanced!= rData.bIsAdvanced||
         bStripData != rData.bStripData ||
//       SAB: I think this should be here, but I don't want to break something
//         bAutoFilter!= rData.bAutoFilter||
         ScRefreshTimer::operator!=( rData )
        )
        return false;

    if ( bIsAdvanced && aAdvSource != rData.aAdvSource )
        return false;

    ScSortParam aSort1, aSort2;
    GetSortParam(aSort1);
    rData.GetSortParam(aSort2);
    if (!(aSort1 == aSort2))
        return false;

    ScQueryParam aQuery1, aQuery2;
    GetQueryParam(aQuery1);
    rData.GetQueryParam(aQuery2);
    if (!(aQuery1 == aQuery2))
        return false;

    ScSubTotalParam aSubTotal1, aSubTotal2;
    GetSubTotalParam(aSubTotal1);
    rData.GetSubTotalParam(aSubTotal2);
    if (!(aSubTotal1 == aSubTotal2))
        return false;

    ScImportParam aImport1, aImport2;
    GetImportParam(aImport1);
    rData.GetImportParam(aImport2);
    return aImport1 == aImport2;
}

ScDBData::~ScDBData()
{
    StopRefreshTimer();
}

OUString ScDBData::GetSourceString() const
{
    OUStringBuffer aBuf;
    if (mpImportParam->bImport)
    {
        aBuf.append(mpImportParam->aDBName);
        aBuf.append('/');
        aBuf.append(mpImportParam->aStatement);
    }
    return aBuf.makeStringAndClear();
}

OUString ScDBData::GetOperations() const
{
    OUStringBuffer aBuf;
    if (mpQueryParam->GetEntryCount())
    {
        const ScQueryEntry& rEntry = mpQueryParam->GetEntry(0);
        if (rEntry.bDoQuery)
            aBuf.append(ScResId(STR_OPERATION_FILTER));
    }

    if (mpSortParam->maKeyState[0].bDoSort)
    {
        if (!aBuf.isEmpty())
            aBuf.append(", ");
        aBuf.append(ScResId(STR_OPERATION_SORT));
    }

    if (mpSubTotal->bGroupActive[0] && !mpSubTotal->bRemoveOnly)
    {
        if (!aBuf.isEmpty())
            aBuf.append(", ");
        aBuf.append(ScResId(STR_OPERATION_SUBTOTAL));
    }

    if (aBuf.isEmpty())
        aBuf.append(ScResId(STR_OPERATION_NONE));

    return aBuf.makeStringAndClear();
}

void ScDBData::GetArea(SCTAB& rTab, SCCOL& rCol1, SCROW& rRow1, SCCOL& rCol2, SCROW& rRow2) const
{
    rTab  = nTable;
    rCol1 = nStartCol;
    rRow1 = nStartRow;
    rCol2 = nEndCol;
    rRow2 = nEndRow;
}

void ScDBData::GetArea(ScRange& rRange) const
{
    SCROW nNewEndRow = nEndRow;
    rRange = ScRange( nStartCol, nStartRow, nTable, nEndCol, nNewEndRow, nTable );
}

ScRange ScDBData::GetHeaderArea() const
{
    if (HasHeader())
        return ScRange( nStartCol, nStartRow, nTable, nEndCol, nStartRow, nTable);
    return ScRange( ScAddress::INITIALIZE_INVALID);
}

void ScDBData::SetArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    bool bHeaderRangeChange = (nTab != nTable || nCol1 != nStartCol || nCol2 != nEndCol || nRow1 != nStartRow);
    if (bHeaderRangeChange)
        EndTableColumnNamesListener();

    nTable  = nTab;
    nStartCol = nCol1;
    nStartRow = nRow1;
    nEndCol   = nCol2;
    nEndRow   = nRow2;

    if (bHeaderRangeChange)
    {
        SAL_WARN_IF( !maTableColumnNames.empty(), "sc.core", "ScDBData::SetArea - invalidating column names/offsets");
        // Invalidate *after* new area has been set above to add the proper
        // header range to dirty list.
        InvalidateTableColumnNames( true);
        StartTableColumnNamesListener();
    }
}

void ScDBData::MoveTo(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    long nDifX = static_cast<long>(nCol1) - static_cast<long>(nStartCol);
    long nDifY = static_cast<long>(nRow1) - static_cast<long>(nStartRow);

    long nSortDif = bByRow ? nDifX : nDifY;
    long nSortEnd = bByRow ? static_cast<long>(nCol2) : static_cast<long>(nRow2);

    for (sal_uInt16 i=0; i<mpSortParam->GetSortKeyCount(); i++)
    {
        mpSortParam->maKeyState[i].nField += nSortDif;
        if (mpSortParam->maKeyState[i].nField > nSortEnd)
        {
            mpSortParam->maKeyState[i].nField = 0;
            mpSortParam->maKeyState[i].bDoSort = false;
        }
    }

    SCSIZE nCount = mpQueryParam->GetEntryCount();
    for (SCSIZE i = 0; i < nCount; ++i)
    {
        ScQueryEntry& rEntry = mpQueryParam->GetEntry(i);
        rEntry.nField += nDifX;
        if (rEntry.nField > nCol2)
        {
            rEntry.nField = 0;
            rEntry.bDoQuery = false;
        }
    }
    for (sal_uInt16 i=0; i<MAXSUBTOTAL; i++)
    {
        mpSubTotal->nField[i] = sal::static_int_cast<SCCOL>( mpSubTotal->nField[i] + nDifX );
        if (mpSubTotal->nField[i] > nCol2)
        {
            mpSubTotal->nField[i] = 0;
            mpSubTotal->bGroupActive[i] = false;
        }
    }

    SetArea( nTab, nCol1, nRow1, nCol2, nRow2 );
}

void ScDBData::GetSortParam( ScSortParam& rSortParam ) const
{
    rSortParam = *mpSortParam;
    rSortParam.nCol1 = nStartCol;
    rSortParam.nRow1 = nStartRow;
    rSortParam.nCol2 = nEndCol;
    rSortParam.nRow2 = nEndRow;
    rSortParam.bByRow = bByRow;
    rSortParam.bHasHeader = bHasHeader;
    /* TODO: add Totals to ScSortParam? */
}

void ScDBData::SetSortParam( const ScSortParam& rSortParam )
{
    mpSortParam.reset(new ScSortParam(rSortParam));
    bByRow = rSortParam.bByRow;
}

void ScDBData::UpdateFromSortParam( const ScSortParam& rSortParam )
{
    bHasHeader = rSortParam.bHasHeader;
}

void ScDBData::GetQueryParam( ScQueryParam& rQueryParam ) const
{
    rQueryParam = *mpQueryParam;
    rQueryParam.nCol1 = nStartCol;
    rQueryParam.nRow1 = nStartRow;
    rQueryParam.nCol2 = nEndCol;
    rQueryParam.nRow2 = nEndRow;
    rQueryParam.nTab  = nTable;
    rQueryParam.bByRow = bByRow;
    rQueryParam.bHasHeader = bHasHeader;
    /* TODO: add Totals to ScQueryParam? */
}

void ScDBData::SetQueryParam(const ScQueryParam& rQueryParam)
{
    mpQueryParam.reset(new ScQueryParam(rQueryParam));

    //  set bIsAdvanced to false for everything that is not from the
    //  advanced filter dialog
    bIsAdvanced = false;
}

void ScDBData::SetAdvancedQuerySource(const ScRange* pSource)
{
    if (pSource)
    {
        aAdvSource = *pSource;
        bIsAdvanced = true;
    }
    else
        bIsAdvanced = false;
}

bool ScDBData::GetAdvancedQuerySource(ScRange& rSource) const
{
    rSource = aAdvSource;
    return bIsAdvanced;
}

void ScDBData::GetSubTotalParam(ScSubTotalParam& rSubTotalParam) const
{
    rSubTotalParam = *mpSubTotal;

    // Share the data range with the parent db data.  The range in the subtotal
    // param struct is not used.
    rSubTotalParam.nCol1 = nStartCol;
    rSubTotalParam.nRow1 = nStartRow;
    rSubTotalParam.nCol2 = nEndCol;
    rSubTotalParam.nRow2 = nEndRow;
}

void ScDBData::SetSubTotalParam(const ScSubTotalParam& rSubTotalParam)
{
    mpSubTotal.reset(new ScSubTotalParam(rSubTotalParam));
}

void ScDBData::GetImportParam(ScImportParam& rImportParam) const
{
    rImportParam = *mpImportParam;
    // set the range.
    rImportParam.nCol1 = nStartCol;
    rImportParam.nRow1 = nStartRow;
    rImportParam.nCol2 = nEndCol;
    rImportParam.nRow2 = nEndRow;
}

void ScDBData::SetImportParam(const ScImportParam& rImportParam)
{
    // the range is ignored.
    mpImportParam.reset(new ScImportParam(rImportParam));
}

bool ScDBData::IsDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const
{
    if (nTab == nTable)
    {
        switch (ePortion)
        {
            case ScDBDataPortion::TOP_LEFT:
                return nCol == nStartCol && nRow == nStartRow;
            case ScDBDataPortion::AREA:
                return nCol >= nStartCol && nCol <= nEndCol && nRow >= nStartRow && nRow <= nEndRow;
        }
    }

    return false;
}

bool ScDBData::IsDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    return (nTab == nTable)
           && (nCol1 == nStartCol) && (nRow1 == nStartRow)
           && (nCol2 == nEndCol) && (nRow2 == nEndRow);
}

bool ScDBData::HasImportParam() const
{
    return mpImportParam && mpImportParam->bImport;
}

bool ScDBData::HasQueryParam() const
{
    if (!mpQueryParam)
        return false;

    if (!mpQueryParam->GetEntryCount())
        return false;

    return mpQueryParam->GetEntry(0).bDoQuery;
}

bool ScDBData::HasSortParam() const
{
    return mpSortParam &&
        !mpSortParam->maKeyState.empty() &&
        mpSortParam->maKeyState[0].bDoSort;
}

bool ScDBData::HasSubTotalParam() const
{
    return mpSubTotal && mpSubTotal->bGroupActive[0];
}

void ScDBData::UpdateMoveTab(SCTAB nOldPos, SCTAB nNewPos)
{
        ScRange aRange;
        GetArea( aRange );
        SCTAB nTab = aRange.aStart.Tab();               // a database range is only on one sheet

        //  customize as the current table as ScTablesHint (tabvwsh5.cxx)

        if ( nTab == nOldPos )                          // moved sheet
            nTab = nNewPos;
        else if ( nOldPos < nNewPos )                   // moved to the back
        {
            if ( nTab > nOldPos && nTab <= nNewPos )    // move this sheet
                --nTab;
        }
        else                                            // moved to the front
        {
            if ( nTab >= nNewPos && nTab < nOldPos )    // move this sheet
                ++nTab;
        }

        bool bChanged = ( nTab != aRange.aStart.Tab() );
        if (bChanged)
        {
            // SetArea() invalidates column names, but it is the same column range
            // just on a different sheet; remember and set new.
            ::std::vector<OUString> aNames( maTableColumnNames);
            bool bTableColumnNamesDirty = mbTableColumnNamesDirty;
            // Same column range.
            SetArea( nTab, aRange.aStart.Col(), aRange.aStart.Row(),
                    aRange.aEnd.Col(),aRange.aEnd.Row() );
            // Do not use SetTableColumnNames() because that resets mbTableColumnNamesDirty.
            maTableColumnNames = aNames;
            mbTableColumnNamesDirty = bTableColumnNamesDirty;
        }

        //  MoveTo() is not necessary if only the sheet changed.

        SetModified(bChanged);

}

void ScDBData::UpdateReference(const ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCCOL nDx, SCROW nDy, SCTAB nDz)
{
    SCCOL theCol1;
    SCROW theRow1;
    SCTAB theTab1;
    SCCOL theCol2;
    SCROW theRow2;
    SCTAB theTab2;
    GetArea( theTab1, theCol1, theRow1, theCol2, theRow2 );
    theTab2 = theTab1;
    SCCOL nOldCol1 = theCol1, nOldCol2 = theCol2;

    bool bDoUpdate = ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                            nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                            theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) != UR_NOTHING;
    if (bDoUpdate)
    {
        // MoveTo() invalidates column names via SetArea(); adjust, remember and set new.
        AdjustTableColumnNames( eUpdateRefMode, nDx, nCol1, nOldCol1, nOldCol2, theCol1, theCol2);
        ::std::vector<OUString> aNames( maTableColumnNames);
        bool bTableColumnNamesDirty = mbTableColumnNamesDirty;
        MoveTo( theTab1, theCol1, theRow1, theCol2, theRow2 );
        // Do not use SetTableColumnNames() because that resets mbTableColumnNamesDirty.
        maTableColumnNames = aNames;
        mbTableColumnNamesDirty = bTableColumnNamesDirty;
    }

    ScRange aRangeAdvSource;
    if ( GetAdvancedQuerySource(aRangeAdvSource) )
    {
        aRangeAdvSource.GetVars( theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 );
        if ( ScRefUpdate::Update( pDoc, eUpdateRefMode,
                                    nCol1,nRow1,nTab1, nCol2,nRow2,nTab2, nDx,nDy,nDz,
                                    theCol1,theRow1,theTab1, theCol2,theRow2,theTab2 ) )
        {
            aRangeAdvSource.aStart.Set( theCol1,theRow1,theTab1 );
            aRangeAdvSource.aEnd.Set( theCol2,theRow2,theTab2 );
            SetAdvancedQuerySource( &aRangeAdvSource );

            bDoUpdate = true;       // DBData is modified
        }
    }

    SetModified(bDoUpdate);

    //TODO: check if something was deleted/inserted with-in the range !!!
}

void ScDBData::ExtendDataArea(const ScDocument* pDoc)
{
    // Extend the DB area to include data rows immediately below.
    SCCOL nOldCol1 = nStartCol, nOldCol2 = nEndCol;
    SCROW nOldEndRow = nEndRow;
    pDoc->GetDataArea(nTable, nStartCol, nStartRow, nEndCol, nEndRow, false, true);
    // nOldEndRow==MAXROW may easily happen when selecting whole columns and
    // setting an AutoFilter (i.e. creating an anonymous database-range). We
    // certainly don't want to iterate over nearly a million empty cells, but
    // keep only an intentionally user selected range.
    if (nOldEndRow < MAXROW && nEndRow < nOldEndRow)
        nEndRow = nOldEndRow;
    if (nStartCol != nOldCol1 || nEndCol != nOldCol2)
    {
        SAL_WARN_IF( !maTableColumnNames.empty(), "sc.core", "ScDBData::ExtendDataArea - invalidating column names/offsets");
        InvalidateTableColumnNames( true);
    }
}

void ScDBData::StartTableColumnNamesListener()
{
    if (mpContainer && bHasHeader)
    {
        ScDocument& rDoc = mpContainer->GetDocument();
        if (!rDoc.IsClipOrUndo())
            rDoc.StartListeningArea( GetHeaderArea(), false, this);
    }
}

void ScDBData::EndTableColumnNamesListener()
{
    EndListeningAll();
}

void ScDBData::SetTableColumnNames( const ::std::vector< OUString >& rNames )
{
    maTableColumnNames = rNames;
    mbTableColumnNamesDirty = false;
}

void ScDBData::AdjustTableColumnNames( UpdateRefMode eUpdateRefMode, SCCOL nDx, SCCOL nCol1,
        SCCOL nOldCol1, SCCOL nOldCol2, SCCOL nNewCol1, SCCOL nNewCol2 )
{
    if (maTableColumnNames.empty())
        return;

    SCCOL nDiff1 = nNewCol1 - nOldCol1;
    SCCOL nDiff2 = nNewCol2 - nOldCol2;
    if (nDiff1 == nDiff2)
        return;     // not moved or entirely moved, nothing to do

    ::std::vector<OUString> aNewNames;
    if (eUpdateRefMode == URM_INSDEL)
    {
        if (nDx > 0)
            mbTableColumnNamesDirty = true;     // inserted columns will have empty names

        // nCol1 is the first column of the block that gets shifted, determine
        // the head and tail elements that are to be copied for deletion or
        // insertion.
        size_t nHead = static_cast<size_t>(::std::max( nCol1 + std::min<SCCOL>(nDx, 0) - nOldCol1, 0));
        size_t nTail = static_cast<size_t>(::std::max( nOldCol2 - nCol1 + 1, 0));
        size_t n = nHead + nTail;
        if (0 < n && n <= maTableColumnNames.size())
        {
            if (nDx > 0)
                n += nDx;
            aNewNames.resize(n);
            // Copy head.
            for (size_t i = 0; i < nHead; ++i)
            {
                aNewNames[i] = maTableColumnNames[i];
            }
            // Copy tail, inserted middle range, if any, stays empty.
            for (size_t i = n - nTail, j = maTableColumnNames.size() - nTail; i < n; ++i, ++j)
            {
                aNewNames[i] = maTableColumnNames[j];
            }
        }
    } // else   empty aNewNames invalidates names/offsets

    SAL_WARN_IF( !maTableColumnNames.empty() && aNewNames.empty(),
            "sc.core", "ScDBData::AdjustTableColumnNames - invalidating column names/offsets");
    aNewNames.swap( maTableColumnNames);
    if (maTableColumnNames.empty())
        mbTableColumnNamesDirty = true;
    if (mbTableColumnNamesDirty)
        InvalidateTableColumnNames( false);     // preserve new column names array
}

void ScDBData::InvalidateTableColumnNames( bool bSwapToEmptyNames )
{
    mbTableColumnNamesDirty = true;
    if (bSwapToEmptyNames && !maTableColumnNames.empty())
        ::std::vector<OUString>().swap( maTableColumnNames);
    if (mpContainer)
    {
        // Add header range to dirty list.
        if (HasHeader())
            mpContainer->GetDirtyTableColumnNames().Join( GetHeaderArea());
        else
        {
            // We need *some* range in the dirty list even without header area,
            // otherwise the container would not attempt to call a refresh.
            mpContainer->GetDirtyTableColumnNames().Join( ScRange( nStartCol, nStartRow, nTable));
        }
    }
}

namespace {
class TableColumnNameSearch
{
public:
    explicit TableColumnNameSearch( const OUString& rSearchName ) :
        maSearchName( rSearchName )
    {
    }

    bool operator()( const OUString& rName ) const
    {
        return ScGlobal::GetpTransliteration()->isEqual( maSearchName, rName);
    }

private:
    OUString const maSearchName;
};

/** Set a numbered table column name at given nIndex, preventing duplicates,
    numbering starting at nCount. If nCount==0 then the first attempt is made
    with an unnumbered name and if already present the next attempt with
    nCount=2, so "Original" and "Original2". No check whether nIndex is valid. */
void SetTableColumnName( ::std::vector<OUString>& rVec, size_t nIndex, const OUString& rName, size_t nCount )
{
    OUString aStr;
    do
    {
        if (nCount)
            aStr = rName + OUString::number( nCount);
        else
        {
            aStr = rName;
            ++nCount;
        }

        if (std::none_of( rVec.begin(), rVec.end(), TableColumnNameSearch( aStr)))
        {
            rVec[nIndex] = aStr;
            break;  // do while
        }
        ++nCount;
    } while(true);
}
}

void ScDBData::RefreshTableColumnNames( ScDocument* pDoc )
{
    ::std::vector<OUString> aNewNames;
    aNewNames.resize( nEndCol - nStartCol + 1);
    bool bHaveEmpty = false;
    if (!HasHeader() || !pDoc)
        bHaveEmpty = true;  // Assume we have empty ones and fill below.
    else
    {
        ScHorizontalCellIterator aIter( pDoc, nTable, nStartCol, nStartRow, nEndCol, nStartRow);  // header row only
        ScRefCellValue* pCell;
        SCCOL nCol, nLastColFilled = nStartCol - 1;
        SCROW nRow;
        for (size_t i=0; (pCell = aIter.GetNext( nCol, nRow)) != nullptr; ++i)
        {
            if (pCell->hasString())
            {
                const OUString& rStr = pCell->getString( pDoc);
                if (rStr.isEmpty())
                    bHaveEmpty = true;
                else
                {
                    SetTableColumnName( aNewNames, nCol-nStartCol, rStr, 0);
                    if (nLastColFilled < nCol-1)
                        bHaveEmpty = true;
                }
                nLastColFilled = nCol;
            }
            else
                bHaveEmpty = true;
        }
    }

    // Never leave us with empty names, try to remember previous name that
    // might had been used to compile formulas, but only if same number of
    // columns and no duplicates.
    if (bHaveEmpty && aNewNames.size() == maTableColumnNames.size())
    {
        bHaveEmpty = false;
        for (size_t i=0, n=aNewNames.size(); i < n; ++i)
        {
            if (aNewNames[i].isEmpty())
            {
                const OUString& rStr = maTableColumnNames[i];
                if (rStr.isEmpty())
                    bHaveEmpty = true;
                else
                    SetTableColumnName( aNewNames, i, rStr, 0);
            }
        }
    }

    // If we still have empty ones then fill those with "Column#" with #
    // starting at the column offset number. Still no duplicates of course.
    if (bHaveEmpty)
    {
        OUString aColumn( ScResId(STR_COLUMN));
        for (size_t i=0, n=aNewNames.size(); i < n; ++i)
        {
            if (aNewNames[i].isEmpty())
                SetTableColumnName( aNewNames, i, aColumn, i+1);
        }
    }

    aNewNames.swap( maTableColumnNames);
    mbTableColumnNamesDirty = false;
}

void ScDBData::RefreshTableColumnNames( ScDocument* pDoc, const ScRange& rRange )
{
    // Header-less tables get names generated, completely empty a full refresh.
    if (mbTableColumnNamesDirty && (!HasHeader() || maTableColumnNames.empty()))
    {
        RefreshTableColumnNames( pDoc);
        return;
    }

    // Check if this is affected for the range requested.
    ScRange aIntersection( GetHeaderArea().Intersection( rRange));
    if (!aIntersection.IsValid())
        return;

    // Always fully refresh, only one cell of a range was broadcasted per area
    // listener if multiple cells were affected. We don't know if there were
    // more. Also, we need the full check anyway in case a duplicated name was
    // entered.
    RefreshTableColumnNames( pDoc);
}

sal_Int32 ScDBData::GetColumnNameOffset( const OUString& rName ) const
{
    if (maTableColumnNames.empty())
        return -1;

    ::std::vector<OUString>::const_iterator it(
            ::std::find_if( maTableColumnNames.begin(), maTableColumnNames.end(), TableColumnNameSearch( rName)));
    if (it != maTableColumnNames.end())
        return it - maTableColumnNames.begin();

    return -1;
}

const OUString& ScDBData::GetTableColumnName( SCCOL nCol ) const
{
    if (maTableColumnNames.empty())
        return EMPTY_OUSTRING;

    SCCOL nOffset = nCol - nStartCol;
    if (nOffset <  0 || maTableColumnNames.size() <= static_cast<size_t>(nOffset))
        return EMPTY_OUSTRING;

    return maTableColumnNames[nOffset];
}

void ScDBData::Notify( const SfxHint& rHint )
{
    const ScHint* pScHint = dynamic_cast<const ScHint*>(&rHint);
    if (!pScHint)
        return;

    if (pScHint->GetId() == SfxHintId::ScDataChanged)
    {
        mbTableColumnNamesDirty = true;
        if (!mpContainer)
            assert(!"ScDBData::Notify - how did we end up here without container?");
        else
        {
            // Only one cell of a range is broadcasted per area listener if
            // multiple cells are affected. Expand the range to what this is
            // listening to. Broadcasted address outside should not happen,
            // but.. let it trigger a refresh if.
            ScRange aHeaderRange( GetHeaderArea());
            if (aHeaderRange.IsValid())
            {
                mpContainer->GetDirtyTableColumnNames().Join( aHeaderRange);
                if (!aHeaderRange.In( pScHint->GetAddress()))
                    mpContainer->GetDirtyTableColumnNames().Join( pScHint->GetAddress());
            }
            else
                mpContainer->GetDirtyTableColumnNames().Join( pScHint->GetAddress());
        }
    }

    // Do not refresh column names here, which might trigger unwanted
    // recalculation.
}

void ScDBData::CalcSaveFilteredCount( SCSIZE nNonFilteredRowCount )
{
    SCSIZE nTotal = nEndRow - nStartRow + 1;
    if ( bHasHeader )
        nTotal -= 1;
    nFilteredRowCount = nTotal - nNonFilteredRowCount;
}

void ScDBData::GetFilterSelCount( SCSIZE& nSelected, SCSIZE& nTotal )
{
    nTotal = nEndRow - nStartRow + 1;
    if ( bHasHeader )
        nTotal -= 1;
    nSelected = nTotal - nFilteredRowCount;
}

namespace {

class FindByTable
{
    SCTAB const mnTab;
public:
    explicit FindByTable(SCTAB nTab) : mnTab(nTab) {}

    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        ScRange aRange;
        p->GetArea(aRange);
        return aRange.aStart.Tab() == mnTab;
    }
};

class UpdateRefFunc
{
    ScDocument* const mpDoc;
    UpdateRefMode const meMode;
    SCCOL const mnCol1;
    SCROW const mnRow1;
    SCTAB const mnTab1;
    SCCOL const mnCol2;
    SCROW const mnRow2;
    SCTAB const mnTab2;
    SCCOL const mnDx;
    SCROW const mnDy;
    SCTAB const mnDz;

public:
    UpdateRefFunc(ScDocument* pDoc, UpdateRefMode eMode,
                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                    SCCOL nDx, SCROW nDy, SCTAB nDz) :
        mpDoc(pDoc), meMode(eMode),
        mnCol1(nCol1), mnRow1(nRow1), mnTab1(nTab1),
        mnCol2(nCol2), mnRow2(nRow2), mnTab2(nTab2),
        mnDx(nDx), mnDy(nDy), mnDz(nDz) {}

    void operator() (std::unique_ptr<ScDBData> const& p)
    {
        p->UpdateReference(mpDoc, meMode, mnCol1, mnRow1, mnTab1, mnCol2, mnRow2, mnTab2, mnDx, mnDy, mnDz);
    }
};

class UpdateMoveTabFunc
{
    SCTAB const mnOldTab;
    SCTAB const mnNewTab;
public:
    UpdateMoveTabFunc(SCTAB nOld, SCTAB nNew) : mnOldTab(nOld), mnNewTab(nNew) {}
    void operator() (std::unique_ptr<ScDBData> const& p)
    {
        p->UpdateMoveTab(mnOldTab, mnNewTab);
    }
};

class FindByCursor
{
    SCCOL const mnCol;
    SCROW const mnRow;
    SCTAB const mnTab;
    ScDBDataPortion const mePortion;
public:
    FindByCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) :
        mnCol(nCol), mnRow(nRow), mnTab(nTab), mePortion(ePortion) {}

    bool operator() (std::unique_ptr<ScDBData> const& p)
    {
        return p->IsDBAtCursor(mnCol, mnRow, mnTab, mePortion);
    }
};

class FindByRange
{
    const ScRange& mrRange;
public:
    explicit FindByRange(const ScRange& rRange) : mrRange(rRange) {}

    bool operator() (std::unique_ptr<ScDBData> const& p)
    {
        return p->IsDBAtArea(
            mrRange.aStart.Tab(), mrRange.aStart.Col(), mrRange.aStart.Row(), mrRange.aEnd.Col(), mrRange.aEnd.Row());
    }
};

class FindByIndex
{
    sal_uInt16 const mnIndex;
public:
    explicit FindByIndex(sal_uInt16 nIndex) : mnIndex(nIndex) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetIndex() == mnIndex;
    }
};

class FindByUpperName
{
    const OUString& mrName;
public:
    explicit FindByUpperName(const OUString& rName) : mrName(rName) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p->GetUpperName() == mrName;
    }
};

class FindByPointer
{
    const ScDBData* mpDBData;
public:
    explicit FindByPointer(const ScDBData* pDBData) : mpDBData(pDBData) {}
    bool operator() (std::unique_ptr<ScDBData> const& p) const
    {
        return p.get() == mpDBData;
    }
};

}

ScDocument& ScDBDataContainerBase::GetDocument() const
{
    return mrDoc;
}

ScRangeList& ScDBDataContainerBase::GetDirtyTableColumnNames()
{
    return maDirtyTableColumnNames;
}

ScDBCollection::NamedDBs::NamedDBs(ScDBCollection& rParent, ScDocument& rDoc) :
    ScDBDataContainerBase(rDoc), mrParent(rParent) {}

ScDBCollection::NamedDBs::NamedDBs(const NamedDBs& r, ScDBCollection& rParent)
    : ScDBDataContainerBase(r.mrDoc)
    , mrParent(rParent)
{
    for (auto const& it : r.m_DBs)
    {
        ScDBData* p = new ScDBData(*it);
        std::unique_ptr<ScDBData> pData(p);
        if (m_DBs.insert( std::move(pData)).second)
            initInserted(p);
    }
}

ScDBCollection::NamedDBs::~NamedDBs()
{
}

void ScDBCollection::NamedDBs::initInserted( ScDBData* p )
{
    p->SetContainer( this);
    if (!mrDoc.IsClipOrUndo())
    {
        p->StartTableColumnNamesListener(); // needs the container be set already
        if (p->AreTableColumnNamesDirty())
        {
            if (p->HasHeader())
            {
                // Refresh table column names in next round.
                maDirtyTableColumnNames.Join( p->GetHeaderArea());
            }
            else
            {
                // Header-less table can generate its column names
                // already without accessing the document.
                p->RefreshTableColumnNames( nullptr);
            }
        }
    }
}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::begin()
{
    return m_DBs.begin();
}

ScDBCollection::NamedDBs::iterator ScDBCollection::NamedDBs::end()
{
    return m_DBs.end();
}

ScDBCollection::NamedDBs::const_iterator ScDBCollection::NamedDBs::begin() const
{
    return m_DBs.begin();
}

ScDBCollection::NamedDBs::const_iterator ScDBCollection::NamedDBs::end() const
{
    return m_DBs.end();
}

ScDBData* ScDBCollection::NamedDBs::findByIndex(sal_uInt16 nIndex)
{
    DBsType::iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByIndex(nIndex));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

ScDBData* ScDBCollection::NamedDBs::findByUpperName(const OUString& rName)
{
    DBsType::iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByUpperName(rName));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

auto ScDBCollection::NamedDBs::findByUpperName2(const OUString& rName) -> iterator
{
    return find_if(
        m_DBs.begin(), m_DBs.end(), FindByUpperName(rName));
}

bool ScDBCollection::NamedDBs::insert(std::unique_ptr<ScDBData> pData)
{
    auto p = pData.get();
    if (!pData->GetIndex())
        pData->SetIndex(mrParent.nEntryIndex++);

    pair<DBsType::iterator, bool> r = m_DBs.insert(std::move(pData));

    if (r.second)
    {
        initInserted(p);

        /* TODO: shouldn't the import refresh not be setup for
         * clipboard/undo documents? It was already like this before.. */
        if (p->HasImportParam() && !p->HasImportSelection())
        {
            p->SetRefreshHandler(mrParent.GetRefreshHandler());
            p->SetRefreshControl(&mrDoc.GetRefreshTimerControlAddress());
        }
    }
    return r.second;
}

void ScDBCollection::NamedDBs::erase(const iterator& itr)
{
    m_DBs.erase(itr);
}

bool ScDBCollection::NamedDBs::empty() const
{
    return m_DBs.empty();
}

size_t ScDBCollection::NamedDBs::size() const
{
    return m_DBs.size();
}

bool ScDBCollection::NamedDBs::operator== (const NamedDBs& r) const
{
    return ::comphelper::ContainerUniquePtrEquals(m_DBs, r.m_DBs);
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::begin()
{
    return m_DBs.begin();
}

ScDBCollection::AnonDBs::iterator ScDBCollection::AnonDBs::end()
{
    return m_DBs.end();
}

ScDBCollection::AnonDBs::const_iterator ScDBCollection::AnonDBs::begin() const
{
    return m_DBs.begin();
}

ScDBCollection::AnonDBs::const_iterator ScDBCollection::AnonDBs::end() const
{
    return m_DBs.end();
}

const ScDBData* ScDBCollection::AnonDBs::findAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
        ScDBDataPortion ePortion) const
{
    DBsType::const_iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByCursor(nCol, nRow, nTab, ePortion));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

const ScDBData* ScDBCollection::AnonDBs::findByRange(const ScRange& rRange) const
{
    DBsType::const_iterator itr = find_if(
        m_DBs.begin(), m_DBs.end(), FindByRange(rRange));
    return itr == m_DBs.end() ? nullptr : itr->get();
}

void ScDBCollection::AnonDBs::deleteOnTab(SCTAB nTab)
{
    FindByTable func(nTab);
    m_DBs.erase(std::remove_if(m_DBs.begin(), m_DBs.end(), func), m_DBs.end());
}

ScDBData* ScDBCollection::AnonDBs::getByRange(const ScRange& rRange)
{
    const ScDBData* pData = findByRange(rRange);
    if (!pData)
    {
        // Insert a new db data.  They all have identical names.
        ::std::unique_ptr<ScDBData> pNew(new ScDBData(
            STR_DB_GLOBAL_NONAME, rRange.aStart.Tab(), rRange.aStart.Col(), rRange.aStart.Row(),
            rRange.aEnd.Col(), rRange.aEnd.Row(), true, false, false));
        pData = pNew.get();
        m_DBs.push_back(std::move(pNew));
    }
    return const_cast<ScDBData*>(pData);
}

void ScDBCollection::AnonDBs::insert(ScDBData* p)
{
    m_DBs.push_back(std::unique_ptr<ScDBData>(p));
}

bool ScDBCollection::AnonDBs::empty() const
{
    return m_DBs.empty();
}

bool ScDBCollection::AnonDBs::has( const ScDBData* p ) const
{
    return any_of(m_DBs.begin(), m_DBs.end(), FindByPointer(p));
}

bool ScDBCollection::AnonDBs::operator== (const AnonDBs& r) const
{
    return ::comphelper::ContainerUniquePtrEquals(m_DBs, r.m_DBs);
}

ScDBCollection::AnonDBs::AnonDBs()
{
}

ScDBCollection::AnonDBs::AnonDBs(AnonDBs const& r)
{
    m_DBs.reserve(r.m_DBs.size());
    for (auto const& it : r.m_DBs)
    {
        m_DBs.push_back(std::make_unique<ScDBData>(*it));
    }
}

ScDBCollection::ScDBCollection(ScDocument* pDocument) :
    pDoc(pDocument), nEntryIndex(1), maNamedDBs(*this, *pDocument) {}

ScDBCollection::ScDBCollection(const ScDBCollection& r) :
    pDoc(r.pDoc), nEntryIndex(r.nEntryIndex), maNamedDBs(r.maNamedDBs, *this), maAnonDBs(r.maAnonDBs) {}

const ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const
{
    // First, search the global named db ranges.
    NamedDBs::DBsType::const_iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByCursor(nCol, nRow, nTab, ePortion));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    const ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtCursor(nCol,nRow,nTab,ePortion))
            return pNoNameData;

    // Check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findAtCursor(nCol, nRow, nTab, ePortion);
    if (pData)
        return pData;

    // Do NOT check for the document global temporary anonymous db range here.

    return nullptr;
}

ScDBData* ScDBCollection::GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion)
{
    // First, search the global named db ranges.
    NamedDBs::DBsType::iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByCursor(nCol, nRow, nTab, ePortion));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtCursor(nCol,nRow,nTab,ePortion))
            return pNoNameData;

    // Check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findAtCursor(nCol, nRow, nTab, ePortion);
    if (pData)
        return const_cast<ScDBData*>(pData);

    // Do NOT check for the document global temporary anonymous db range here.

    return nullptr;
}

const ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const
{
    // First, search the global named db ranges.
    ScRange aRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    NamedDBs::DBsType::const_iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByRange(aRange));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    // Lastly, check the global anonymous db ranges.
    const ScDBData* pData = maAnonDBs.findByRange(aRange);
    if (pData)
        return pData;

    // As a last resort, check for the document global temporary anonymous db range.
    pNoNameData = pDoc->GetAnonymousDBData();
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    return nullptr;
}

ScDBData* ScDBCollection::GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2)
{
    // First, search the global named db ranges.
    ScRange aRange(nCol1, nRow1, nTab, nCol2, nRow2, nTab);
    NamedDBs::DBsType::iterator itr = find_if(
        maNamedDBs.begin(), maNamedDBs.end(), FindByRange(aRange));
    if (itr != maNamedDBs.end())
        return itr->get();

    // Check for the sheet-local anonymous db range.
    ScDBData* pNoNameData = pDoc->GetAnonymousDBData(nTab);
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    // Lastly, check the global anonymous db ranges.
    const ScDBData* pData = getAnonDBs().findByRange(aRange);
    if (pData)
        return const_cast<ScDBData*>(pData);

    // As a last resort, check for the document global temporary anonymous db range.
    pNoNameData = pDoc->GetAnonymousDBData();
    if (pNoNameData)
        if (pNoNameData->IsDBAtArea(nTab, nCol1, nRow1, nCol2, nRow2))
            return pNoNameData;

    return nullptr;
}

void ScDBCollection::RefreshDirtyTableColumnNames()
{
    for (size_t i=0; i < maNamedDBs.maDirtyTableColumnNames.size(); ++i)
    {
        const ScRange & rRange = maNamedDBs.maDirtyTableColumnNames[i];
        for (auto const& it : maNamedDBs)
        {
            if (it->AreTableColumnNamesDirty())
                it->RefreshTableColumnNames( &maNamedDBs.mrDoc, rRange);
        }
    }
    maNamedDBs.maDirtyTableColumnNames.RemoveAll();
}

void ScDBCollection::DeleteOnTab( SCTAB nTab )
{
    FindByTable func(nTab);
    // First, collect the positions of all items that need to be deleted.
    ::std::vector<NamedDBs::DBsType::iterator> v;
    {
        NamedDBs::DBsType::iterator itr = maNamedDBs.begin(), itrEnd = maNamedDBs.end();
        for (; itr != itrEnd; ++itr)
        {
            if (func(*itr))
                v.push_back(itr);
        }
    }

    // Delete them all.
    for (auto& rIter : v)
        maNamedDBs.erase(rIter);

    maAnonDBs.deleteOnTab(nTab);
}

void ScDBCollection::UpdateReference(UpdateRefMode eUpdateRefMode,
                                SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                SCCOL nDx, SCROW nDy, SCTAB nDz )
{
    ScDBData* pData = pDoc->GetAnonymousDBData(nTab1);
    if (pData)
    {
        if (nTab1 == nTab2 && nDz == 0)
        {
            pData->UpdateReference(
                pDoc, eUpdateRefMode,
                nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz);
        }
        else
        {
            //this will perhaps break undo
        }
    }

    UpdateRefFunc func(pDoc, eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz);
    for_each(maNamedDBs.begin(), maNamedDBs.end(), func);
    for_each(maAnonDBs.begin(), maAnonDBs.end(), func);
}

void ScDBCollection::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos )
{
    UpdateMoveTabFunc func(nOldPos, nNewPos);
    for_each(maNamedDBs.begin(), maNamedDBs.end(), func);
    for_each(maAnonDBs.begin(), maAnonDBs.end(), func);
}

ScDBData* ScDBCollection::GetDBNearCursor(SCCOL nCol, SCROW nRow, SCTAB nTab )
{
    ScDBData* pNearData = nullptr;
    for (const auto& rxNamedDB : maNamedDBs)
    {
        SCTAB nAreaTab;
        SCCOL nStartCol, nEndCol;
        SCROW nStartRow, nEndRow;
        rxNamedDB->GetArea( nAreaTab, nStartCol, nStartRow, nEndCol, nEndRow );
        if ( nTab == nAreaTab && nCol+1 >= nStartCol && nCol <= nEndCol+1 &&
                                 nRow+1 >= nStartRow && nRow <= nEndRow+1 )
        {
            if ( nCol < nStartCol || nCol > nEndCol || nRow < nStartRow || nRow > nEndRow )
            {
                if (!pNearData)
                    pNearData = rxNamedDB.get(); // remember first adjacent area
            }
            else
                return rxNamedDB.get();          // not "unbenannt"/"unnamed" and cursor within
        }
    }
    if (pNearData)
        return pNearData;                   // adjacent, if no direct hit
    return pDoc->GetAnonymousDBData(nTab);  // "unbenannt"/"unnamed" only if nothing else
}

bool ScDBCollection::empty() const
{
    return maNamedDBs.empty() && maAnonDBs.empty();
}

bool ScDBCollection::operator== (const ScDBCollection& r) const
{
    return maNamedDBs == r.maNamedDBs && maAnonDBs == r.maAnonDBs &&
        nEntryIndex == r.nEntryIndex && pDoc == r.pDoc && aRefreshHandler == r.aRefreshHandler;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
