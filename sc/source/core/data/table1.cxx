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

#include <scitems.hxx>
#include <editeng/justifyitem.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/charclass.hxx>

#include <patattr.hxx>
#include <table.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <olinetab.hxx>
#include <global.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <refupdat.hxx>
#include <markdata.hxx>
#include <progress.hxx>
#include <prnsave.hxx>
#include <tabprotection.hxx>
#include <sheetevents.hxx>
#include <segmenttree.hxx>
#include <dbdata.hxx>
#include <conditio.hxx>
#include <globalnames.hxx>
#include <cellvalue.hxx>
#include <scmatrix.hxx>
#include <refupdatecontext.hxx>
#include <rowheightcontext.hxx>
#include <compressedarray.hxx>
#include <vcl/svapp.hxx>

#include <formula/vectortoken.hxx>
#include <token.hxx>

#include <vector>
#include <memory>

using ::std::vector;

namespace {

ScProgress* GetProgressBar(
    SCSIZE nCount, SCSIZE nTotalCount, ScProgress* pOuterProgress, const ScDocument* pDoc)
{
    if (nTotalCount < 1000)
    {
        // if the total number of rows is less than 1000, don't even bother
        // with the progress bar because drawing progress bar can be very
        // expensive especially in GTK.
        return nullptr;
    }

    if (pOuterProgress)
        return pOuterProgress;

    if (nCount > 1)
        return new ScProgress(
            pDoc->GetDocumentShell(), ScResId(STR_PROGRESS_HEIGHTING), nTotalCount, true);

    return nullptr;
}

void GetOptimalHeightsInColumn(
    sc::RowHeightContext& rCxt, ScColContainer& rCol, SCROW nStartRow, SCROW nEndRow,
    ScProgress* pProgress, sal_uLong nProgressStart )
{
    assert(nStartRow <= nEndRow);

    //  first, one time over the whole range
    //  (with the last column in the hope that they most likely still are
    //  on standard format)


    rCol.back().GetOptimalHeight(rCxt, nStartRow, nEndRow, 0, 0);

    //  from there search for the standard height that is in use in the lower part

    ScFlatUInt16RowSegments& rHeights = rCxt.getHeightArray();
    sal_uInt16 nMinHeight = rHeights.getValue(nEndRow);
    SCSIZE nPos = nEndRow-1;
    ScFlatUInt16RowSegments::RangeData aRangeData;
    while ( nPos && rHeights.getRangeData(nPos-1, aRangeData) )
    {
        if (aRangeData.mnValue >= nMinHeight)
            nPos = std::max<SCSIZE>(0, aRangeData.mnRow1);
        else
            break;
    }

    const SCROW nMinStart = nPos;

    sal_uLong nWeightedCount = nProgressStart + rCol.back().GetWeightedCount(nStartRow, nEndRow);
    const SCCOL maxCol = rCol.size() - 1; // last col done already above
    const SCCOL progressUpdateStep = rCol.size() / 10;
    for (SCCOL nCol=0; nCol<maxCol; nCol++)
    {
        rCol[nCol].GetOptimalHeight(rCxt, nStartRow, nEndRow, nMinHeight, nMinStart);

        if (pProgress)
        {
            nWeightedCount += rCol[nCol].GetWeightedCount(nStartRow, nEndRow);
            pProgress->SetState( nWeightedCount );

            if ((nCol % progressUpdateStep) == 0)
            {
                // try to make sure the progress dialog is painted before continuing
                Application::Reschedule(true);
            }
        }
    }
}

struct OptimalHeightsFuncObjBase
{
    virtual ~OptimalHeightsFuncObjBase() {}
    virtual bool operator() (SCROW nStartRow, SCROW nEndRow, sal_uInt16 nHeight) = 0;
};

struct SetRowHeightOnlyFunc : public OptimalHeightsFuncObjBase
{
    ScTable* mpTab;
    explicit SetRowHeightOnlyFunc(ScTable* pTab) :
        mpTab(pTab)
    {}

    virtual bool operator() (SCROW nStartRow, SCROW nEndRow, sal_uInt16 nHeight) override
    {
        mpTab->SetRowHeightOnly(nStartRow, nEndRow, nHeight);
        return false;
    }
};

struct SetRowHeightRangeFunc : public OptimalHeightsFuncObjBase
{
    ScTable* mpTab;
    double const mnPPTY;

    SetRowHeightRangeFunc(ScTable* pTab, double nPPTY) :
        mpTab(pTab),
        mnPPTY(nPPTY)
    {}

    virtual bool operator() (SCROW nStartRow, SCROW nEndRow, sal_uInt16 nHeight) override
    {
        return mpTab->SetRowHeightRange(nStartRow, nEndRow, nHeight, mnPPTY);
    }
};

bool SetOptimalHeightsToRows(
    sc::RowHeightContext& rCxt,
    OptimalHeightsFuncObjBase& rFuncObj,
    ScBitMaskCompressedArray<SCROW, CRFlags>* pRowFlags, SCROW nStartRow, SCROW nEndRow )
{
    bool bChanged = false;
    SCROW nRngStart = 0;
    SCROW nRngEnd = 0;
    sal_uInt16 nLast = 0;
    sal_uInt16 nExtraHeight = rCxt.getExtraHeight();
    for (SCSIZE i = nStartRow; i <= static_cast<SCSIZE>(nEndRow); i++)
    {
        size_t nIndex;
        SCROW nRegionEndRow;
        CRFlags nRowFlag = pRowFlags->GetValue( i, nIndex, nRegionEndRow );
        if ( nRegionEndRow > nEndRow )
            nRegionEndRow = nEndRow;
        SCSIZE nMoreRows = nRegionEndRow - i;     // additional equal rows after first

        bool bAutoSize = !(nRowFlag & CRFlags::ManualSize);
        if (bAutoSize || rCxt.isForceAutoSize())
        {
            if (nExtraHeight)
            {
                if (bAutoSize)
                    pRowFlags->SetValue( i, nRegionEndRow, nRowFlag | CRFlags::ManualSize);
            }
            else if (!bAutoSize)
                pRowFlags->SetValue( i, nRegionEndRow, nRowFlag & ~CRFlags::ManualSize);

            for (SCSIZE nInner = i; nInner <= i + nMoreRows; ++nInner)
            {
                if (nLast)
                {
                    ScFlatUInt16RowSegments::RangeData aRangeData;
                    (void)rCxt.getHeightArray().getRangeData(nInner, aRangeData);
                    if (aRangeData.mnValue + nExtraHeight == nLast)
                    {
                        nRngEnd = std::min<SCSIZE>(i + nMoreRows, aRangeData.mnRow2);
                        nInner = aRangeData.mnRow2;
                    }
                    else
                    {
                        bChanged |= rFuncObj(nRngStart, nRngEnd, nLast);
                        nLast = 0;
                    }
                }
                if (!nLast)
                {
                    nLast = rCxt.getHeightArray().getValue(nInner) + rCxt.getExtraHeight();
                    nRngStart = nInner;
                    nRngEnd = nInner;
                }
            }
        }
        else
        {
            if (nLast)
                bChanged |= rFuncObj(nRngStart, nRngEnd, nLast);
            nLast = 0;
        }
        i += nMoreRows;     // already handled - skip
    }
    if (nLast)
        bChanged |= rFuncObj(nRngStart, nRngEnd, nLast);

    return bChanged;
}

}

ScTable::ScTable( ScDocument* pDoc, SCTAB nNewTab, const OUString& rNewName,
                    bool bColInfo, bool bRowInfo ) :
    aCol( MAXCOLCOUNT ),
    aName( rNewName ),
    aCodeName( rNewName ),
    nLinkRefreshDelay( 0 ),
    nLinkMode( ScLinkMode::NONE ),
    aPageStyle( ScResId(STR_STYLENAME_STANDARD) ),
    nRepeatStartX( SCCOL_REPEAT_NONE ),
    nRepeatEndX( SCCOL_REPEAT_NONE ),
    nRepeatStartY( SCROW_REPEAT_NONE ),
    nRepeatEndY( SCROW_REPEAT_NONE ),
    mpRowHeights( static_cast<ScFlatUInt16RowSegments*>(nullptr) ),
    mpHiddenCols(new ScFlatBoolColSegments),
    mpHiddenRows(new ScFlatBoolRowSegments),
    mpFilteredCols(new ScFlatBoolColSegments),
    mpFilteredRows(new ScFlatBoolRowSegments),
    nTableAreaX( 0 ),
    nTableAreaY( 0 ),
    nTab( nNewTab ),
    pDocument( pDoc ),
    pSortCollator( nullptr ),
    nLockCount( 0 ),
    aScenarioColor( COL_LIGHTGRAY ),
    aTabBgColor( COL_AUTO ),
    nScenarioFlags(ScScenarioFlags::NONE),
    mpCondFormatList( new ScConditionalFormatList() ),
    bScenario(false),
    bLayoutRTL(false),
    bLoadingRTL(false),
    bPageSizeValid(false),
    bTableAreaValid(false),
    bVisible(true),
    bStreamValid(false),
    bPendingRowHeights(false),
    bCalcNotification(false),
    bGlobalKeepQuery(false),
    bPrintEntireSheet(true),
    bActiveScenario(false),
    mbPageBreaksValid(false),
    mbForceBreaks(false),
    aDefaultColAttrArray(static_cast<SCCOL>(-1), nNewTab, pDoc, nullptr)
{

    if (bColInfo)
    {
        mpColWidth.reset( new ScCompressedArray<SCCOL, sal_uInt16>( MAXCOL+1, STD_COL_WIDTH ) );
        mpColFlags.reset( new ScBitMaskCompressedArray<SCCOL, CRFlags>( MAXCOL+1, CRFlags::NONE ) );
    }

    if (bRowInfo)
    {
        mpRowHeights.reset(new ScFlatUInt16RowSegments(ScGlobal::nStdRowHeight));
        pRowFlags.reset(new ScBitMaskCompressedArray<SCROW, CRFlags>( MAXROW, CRFlags::NONE));
    }

    if ( pDocument->IsDocVisible() )
    {
        //  when a sheet is added to a visible document,
        //  initialize its RTL flag from the system locale
        bLayoutRTL = ScGlobal::IsSystemRTL();
    }

    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if (pDrawLayer)
    {
        if ( pDrawLayer->ScAddPage( nTab ) )    // sal_False (not inserted) during Undo
        {
            pDrawLayer->ScRenamePage( nTab, aName );
            sal_uLong const nx = sal_uLong(double(MAXCOL+1) * STD_COL_WIDTH           * HMM_PER_TWIPS );
            sal_uLong ny = static_cast<sal_uLong>(double(MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
            pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( nx, ny ), false );
        }
    }

    for (SCCOL k=0; k < aCol.size(); k++)
        aCol[k].Init( k, nTab, pDocument, true );
}

ScTable::~ScTable() COVERITY_NOEXCEPT_FALSE
{
    if (!pDocument->IsInDtorClear())
    {
        for (SCCOL nCol = 0; nCol < (aCol.size() - 1); ++nCol)
        {
            aCol[nCol].FreeNotes();
        }
        //  In the dtor, don't delete the pages in the wrong order.
        //  (or else nTab does not reflect the page number!)
        //  In ScDocument::Clear is afterwards used from Clear at the Draw Layer to delete everything.

        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->ScRemovePage( nTab );
    }

    pRowFlags.reset();
    pSheetEvents.reset();
    pOutlineTable.reset();
    pSearchText.reset();
    pRepeatColRange.reset();
    pRepeatRowRange.reset();
    pScenarioRanges.reset();
    mpRangeName.reset();
    pDBDataNoName.reset();
    DestroySortCollator();
}

sal_Int64 ScTable::GetHashCode() const
{
    return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
}

void ScTable::SetName( const OUString& rNewName )
{
    aName = rNewName;
    aUpperName.clear(); // invalidated if the name is changed

    // SetStreamValid is handled in ScDocument::RenameTab
}

const OUString& ScTable::GetUpperName() const
{
    if (aUpperName.isEmpty() && !aName.isEmpty())
        aUpperName = ScGlobal::pCharClass->uppercase(aName);
    return aUpperName;
}

void ScTable::SetVisible( bool bVis )
{
    if (bVisible != bVis)
        SetStreamValid(false);

    bVisible = bVis;
}

void ScTable::SetStreamValid( bool bSet, bool bIgnoreLock )
{
    if (!bStreamValid && !bSet)
        return; // shortcut
    if ( bIgnoreLock || !pDocument->IsStreamValidLocked() )
        bStreamValid = bSet;
}

void ScTable::SetPendingRowHeights( bool bSet )
{
    bPendingRowHeights = bSet;
}

void ScTable::SetLayoutRTL( bool bSet )
{
    bLayoutRTL = bSet;
}

void ScTable::SetLoadingRTL( bool bSet )
{
    bLoadingRTL = bSet;
}

void ScTable::SetTabBgColor(const Color& rColor)
{
    if (aTabBgColor != rColor)
    {
        // The tab color has changed.  Set this table 'modified'.
        aTabBgColor = rColor;
        SetStreamValid(false);
    }
}

void ScTable::SetScenario( bool bFlag )
{
    bScenario = bFlag;
}

void ScTable::SetLink( ScLinkMode nMode,
                        const OUString& rDoc, const OUString& rFlt, const OUString& rOpt,
                        const OUString& rTab, sal_uLong nRefreshDelay )
{
    nLinkMode = nMode;
    aLinkDoc = rDoc;        // File
    aLinkFlt = rFlt;        // Filter
    aLinkOpt = rOpt;        // Filter options
    aLinkTab = rTab;        // Sheet name in source file
    nLinkRefreshDelay = nRefreshDelay;  // refresh delay in seconds, 0==off

    SetStreamValid(false);
}

sal_uInt16 ScTable::GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bFormula, const ScMarkData* pMarkData,
                                    const ScColWidthParam* pParam )
{
    if ( nCol >= aCol.size() )
        return ( STD_COL_WIDTH - STD_EXTRA_WIDTH );

    return aCol[nCol].GetOptimalColWidth( pDev, nPPTX, nPPTY, rZoomX, rZoomY,
        bFormula, STD_COL_WIDTH - STD_EXTRA_WIDTH, pMarkData, pParam );
}

long ScTable::GetNeededSize( SCCOL nCol, SCROW nRow,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                bool bWidth, bool bTotalSize )
{
    if ( nCol >= aCol.size() )
        return 0;

    ScNeededSizeOptions aOptions;
    aOptions.bSkipMerged = false;       // count merged cells
    aOptions.bTotalSize  = bTotalSize;

    return aCol[nCol].GetNeededSize
        ( nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, aOptions, nullptr );
}

bool ScTable::SetOptimalHeight(
    sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow,
    ScProgress* pOuterProgress, sal_uLong nProgressStart )
{
    assert(nStartRow <= nEndRow);

    OSL_ENSURE( rCxt.getExtraHeight() == 0 || rCxt.isForceAutoSize(),
        "automatic OptimalHeight with Extra" );

    if ( pDocument->IsAdjustHeightLocked() )
    {
        return false;
    }

    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = GetProgressBar(nCount, GetWeightedCount(), pOuterProgress, pDocument);

    GetOptimalHeightsInColumn(rCxt, aCol, nStartRow, nEndRow, pProgress, nProgressStart);

    rCxt.getHeightArray().enableTreeSearch(true);
    SetRowHeightRangeFunc aFunc(this, rCxt.getPPTY());
    bool bChanged = SetOptimalHeightsToRows(rCxt, aFunc, pRowFlags.get(), nStartRow, nEndRow);

    if ( pProgress != pOuterProgress )
        delete pProgress;

    return bChanged;
}

void ScTable::SetOptimalHeightOnly(
    sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow,
    ScProgress* pOuterProgress, sal_uLong nProgressStart )
{
    OSL_ENSURE( rCxt.getExtraHeight() == 0 || rCxt.isForceAutoSize(),
        "automatic OptimalHeight with Extra" );

    if ( pDocument->IsAdjustHeightLocked() )
        return;

    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = GetProgressBar(nCount, GetWeightedCount(), pOuterProgress, pDocument);

    GetOptimalHeightsInColumn(rCxt, aCol, nStartRow, nEndRow, pProgress, nProgressStart);

    SetRowHeightOnlyFunc aFunc(this);

    rCxt.getHeightArray().enableTreeSearch(true);
    SetOptimalHeightsToRows(rCxt, aFunc, pRowFlags.get(), nStartRow, nEndRow);

    if ( pProgress != pOuterProgress )
        delete pProgress;
}

bool ScTable::GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    for (SCCOL i=0; i<aCol.size(); i++)
        {
            if (!aCol[i].IsEmptyData())
            {
                bFound = true;
                nMaxX = i;
                SCROW nRow = aCol[i].GetLastDataPos();
                if (nRow > nMaxY)
                    nMaxY = nRow;
            }
            if ( aCol[i].HasCellNotes() )
            {
                SCROW maxNoteRow = aCol[i].GetCellNotesMaxRow();
                if (maxNoteRow >= nMaxY)
                {
                    bFound = true;
                    nMaxY = maxNoteRow;
                }
                if (i>nMaxX)
                {
                    bFound = true;
                    nMaxX = i;
                }
            }
        }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

bool ScTable::GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
    bool bRet = true;               //TODO: remember?
    if (!bTableAreaValid)
    {
        bRet = GetPrintArea(nTableAreaX, nTableAreaY, true);
        bTableAreaValid = true;
    }
    rEndCol = nTableAreaX;
    rEndRow = nTableAreaY;
    return bRet;
}

const SCCOL SC_COLUMNS_STOP = 30;

bool ScTable::GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, bool bNotes ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    SCCOL i;

    for (i=0; i<aCol.size(); i++)               // Test data
        {
            if (!aCol[i].IsEmptyData())
            {
                bFound = true;
                if (i>nMaxX)
                    nMaxX = i;
                SCROW nColY = aCol[i].GetLastDataPos();
                if (nColY > nMaxY)
                    nMaxY = nColY;
            }
            if (bNotes && aCol[i].HasCellNotes() )
            {
                SCROW maxNoteRow = aCol[i].GetCellNotesMaxRow();
                if (maxNoteRow >= nMaxY)
                {
                    bFound = true;
                    nMaxY = maxNoteRow;
                }
                if (i>nMaxX)
                {
                    bFound = true;
                    nMaxX = i;
                }
            }
        }

    SCCOL nMaxDataX = nMaxX;

    for (i=0; i<aCol.size(); i++)               // Test attribute
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow ))
        {
            bFound = true;
            nMaxX = i;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    if (nMaxX == MAXCOL)                    // omit attribute at the right
    {
        --nMaxX;
        while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1]) )
            --nMaxX;
    }

    if ( nMaxX < nMaxDataX )
    {
        nMaxX = nMaxDataX;
    }
    else if ( nMaxX > nMaxDataX )
    {
        SCCOL nAttrStartX = nMaxDataX + 1;
        while ( nAttrStartX < (aCol.size()-1) )
        {
            SCCOL nAttrEndX = nAttrStartX;
            while ( nAttrEndX < (aCol.size()-1) && aCol[nAttrStartX].IsVisibleAttrEqual(aCol[nAttrEndX+1]) )
                ++nAttrEndX;
            if ( nAttrEndX + 1 - nAttrStartX >= SC_COLUMNS_STOP )
            {
                // found equally-formatted columns behind data -> stop before these columns
                nMaxX = nAttrStartX - 1;

                // also don't include default-formatted columns before that
                SCROW nDummyRow;
                while ( nMaxX > nMaxDataX && !aCol[nMaxX].GetLastVisibleAttr( nDummyRow ) )
                    --nMaxX;
                break;
            }
            nAttrStartX = nAttrEndX + 1;
        }
    }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

bool ScTable::GetPrintAreaHor( SCROW nStartRow, SCROW nEndRow,
                                SCCOL& rEndCol ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCCOL i;

    for (i=0; i<aCol.size(); i++)               // Test attribute
    {
        if (aCol[i].HasVisibleAttrIn( nStartRow, nEndRow ))
        {
            bFound = true;
            nMaxX = i;
        }
    }

    if (nMaxX == MAXCOL)                    // omit attribute at the right
    {
        --nMaxX;
        while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1], nStartRow, nEndRow) )
            --nMaxX;
    }

    for (i=0; i<aCol.size(); i++)               // test the data
    {
        if (!aCol[i].IsEmptyBlock( nStartRow, nEndRow ))        //TODO: bNotes ??????
        {
            bFound = true;
            if (i>nMaxX)
                nMaxX = i;
        }
    }

    rEndCol = nMaxX;
    return bFound;
}

bool ScTable::GetPrintAreaVer( SCCOL nStartCol, SCCOL nEndCol,
                                SCROW& rEndRow, bool bNotes ) const
{
    nStartCol = std::min<SCCOL>( nStartCol, aCol.size()-1 );
    nEndCol   = std::min<SCCOL>( nEndCol,   aCol.size()-1 );
    bool bFound = false;
    SCROW nMaxY = 0;
    SCCOL i;

    for (i=nStartCol; i<=nEndCol; i++)              // Test attribute
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow ))
        {
            bFound = true;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    for (i=nStartCol; i<=nEndCol; i++)              // Test data
    {
        if (!aCol[i].IsEmptyData())
        {
            bFound = true;
            SCROW nColY = aCol[i].GetLastDataPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }
        if (bNotes && aCol[i].HasCellNotes() )
        {
            SCROW maxNoteRow =aCol[i].GetCellNotesMaxRow();
            if (maxNoteRow > nMaxY)
            {
                bFound = true;
                nMaxY = maxNoteRow;
            }
        }
    }

    rEndRow = nMaxY;
    return bFound;
}

bool ScTable::GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const
{
    bool bFound = false;
    SCCOL nMinX = aCol.size()-1;
    SCROW nMinY = MAXROW;
    SCCOL i;

    for (i=0; i<aCol.size(); i++)                   // Test attribute
    {
        SCROW nFirstRow;
        if (aCol[i].GetFirstVisibleAttr( nFirstRow ))
        {
            if (!bFound)
                nMinX = i;
            bFound = true;
            if (nFirstRow < nMinY)
                nMinY = nFirstRow;
        }
    }

    if (nMinX == 0)                                     // omit attribute at the right
    {
        if ( aCol.size() > 1 && aCol[0].IsVisibleAttrEqual(aCol[1]) )      // no single ones
        {
            ++nMinX;
            while ( nMinX<(aCol.size()-1) && aCol[nMinX].IsVisibleAttrEqual(aCol[nMinX-1]) )
                ++nMinX;
        }
    }

    bool bDatFound = false;
    for (i=0; i<aCol.size(); i++)                   // Test data
    {
        if (!aCol[i].IsEmptyData())
        {
            if (!bDatFound && i<nMinX)
                nMinX = i;
            bFound = bDatFound = true;
            SCROW nRow = aCol[i].GetFirstDataPos();
            if (nRow < nMinY)
                nMinY = nRow;
        }
        if ( aCol[i].HasCellNotes() )
        {
            SCROW minNoteRow = aCol[i].GetCellNotesMinRow();
            if (minNoteRow <= nMinY)
            {
                bFound = true;
                nMinY = minNoteRow;
            }
            if (i<nMinX)
            {
                bFound = true;
                nMinX = i;
            }
        }
    }
    rStartCol = nMinX;
    rStartRow = nMinY;
    return bFound;
}

void ScTable::GetDataArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
                           bool bIncludeOld, bool bOnlyDown ) const
{
    // return the smallest area containing at least all contiguous cells having data. This area
    // is a square containing also empty cells. It may shrink or extend the area given as input
    // Flags as modifiers:
    //
    //     bIncludeOld = true ensure that the returned area contains at least the initial area,
    //                   independently of the emptiness of rows / columns (i.e. does not allow shrinking)
    //     bOnlyDown = true means extend / shrink the inputted area only down, i.e modify only rEndRow

    rStartCol = std::min<SCCOL>( rStartCol, aCol.size()-1 );
    rEndCol   = std::min<SCCOL>( rEndCol,   aCol.size()-1 );

    bool bLeft = false;
    bool bRight  = false;
    bool bTop = false;
    bool bBottom = false;
    bool bChanged = false;

    do
    {
        bChanged = false;

        if (!bOnlyDown)
        {
            SCROW nStart = rStartRow;
            SCROW nEnd = rEndRow;
            if (nStart>0) --nStart;
            if (nEnd<MAXROW) ++nEnd;

            if (rEndCol < (aCol.size()-1))
                if (!aCol[rEndCol+1].IsEmptyBlock(nStart,nEnd))
                {
                    ++rEndCol;
                    bChanged = true;
                    bRight = true;
                }

            if (rStartCol > 0)
                if (!aCol[rStartCol-1].IsEmptyBlock(nStart,nEnd))
                {
                    --rStartCol;
                    bChanged = true;
                    bLeft = true;
                }

            if (rStartRow > 0)
            {
                SCROW nTest = rStartRow-1;
                bool needExtend = false;
                for ( SCCOL i = rStartCol; i<=rEndCol && !needExtend; i++)
                    if (aCol[i].HasDataAt(nTest))
                        needExtend = true;
                if (needExtend)
                {
                    --rStartRow;
                    bChanged = true;
                    bTop = true;
                }
            }
        }

        if (rEndRow < MAXROW)
        {
            SCROW nTest = rEndRow+1;
            bool needExtend = false;
            for ( SCCOL i = rStartCol; i<=rEndCol && !needExtend; i++)
                if (aCol[i].HasDataAt(nTest))
                    needExtend = true;
            if (needExtend)
            {
                ++rEndRow;
                bChanged = true;
                bBottom = true;
            }
        }
    }
    while( bChanged );

    if ( !bIncludeOld && !bOnlyDown )
    {
        if ( !bLeft )
            while ( rStartCol < rEndCol && rStartCol < (aCol.size()-1) && aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
                ++rStartCol;

        if ( !bRight )
            while ( rEndCol > 0 && rStartCol < rEndCol && aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
                --rEndCol;

        if ( !bTop && rStartRow < MAXROW && rStartRow < rEndRow )
        {
            bool bShrink = true;
            do
            {
                for ( SCCOL i = rStartCol; i<=rEndCol && bShrink; i++)
                    if (aCol[i].HasDataAt(rStartRow))
                        bShrink = false;
                if (bShrink)
                    ++rStartRow;
            } while (bShrink && rStartRow < MAXROW && rStartRow < rEndRow);
        }
    }

    if ( !bIncludeOld )
    {
        if ( !bBottom && rEndRow > 0 && rStartRow < rEndRow )
        {
            SCROW nLastDataRow = GetLastDataRow( rStartCol, rEndCol, rEndRow);
            if (nLastDataRow < rEndRow)
                rEndRow = std::max( rStartRow, nLastDataRow);
        }
    }
}

bool ScTable::GetDataAreaSubrange( ScRange& rRange ) const
{
    SCCOL nCol1 = rRange.aStart.Col(), nCol2 = rRange.aEnd.Col();

    if ( nCol1 >= aCol.size() )
        return false;

    nCol2 = std::min<SCCOL>( nCol2, aCol.size()-1 );

    SCROW nRow1 = rRange.aStart.Row(), nRow2 = rRange.aEnd.Row();

    SCCOL nFirstNonEmptyCol = -1, nLastNonEmptyCol = -1;
    SCROW nRowStart = nRow2, nRowEnd = nRow1;

    for ( SCCOL nCol = nCol1; nCol <= nCol2; ++nCol )
    {
        SCROW nRowStartThis = nRow1, nRowEndThis = nRow2;
        bool bTrimmed = aCol[nCol].TrimEmptyBlocks(nRowStartThis, nRowEndThis);
        if ( bTrimmed )
        {
            if ( nFirstNonEmptyCol == -1 )
                nFirstNonEmptyCol = nCol;
            nLastNonEmptyCol = nCol;

            nRowStart = std::min<SCROW>(nRowStart, nRowStartThis);
            nRowEnd = std::max<SCROW>(nRowEnd, nRowEndThis);
        }
    }

    if ( nFirstNonEmptyCol == -1 )
        return false;

    assert(nFirstNonEmptyCol <= nLastNonEmptyCol);
    assert(nRowStart <= nRowEnd);

    rRange.aStart.Set(nFirstNonEmptyCol, nRowStart, rRange.aStart.Tab());
    rRange.aEnd.Set(nLastNonEmptyCol, nRowEnd, rRange.aEnd.Tab());

    return true;
}

bool ScTable::ShrinkToUsedDataArea( bool& o_bShrunk, SCCOL& rStartCol, SCROW& rStartRow,
        SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly, bool bStickyTopRow, bool bStickyLeftCol,
        bool bConsiderCellNotes, bool bConsiderCellDrawObjects ) const
{
    rStartCol = std::min<SCCOL>( rStartCol, aCol.size()-1 );
    // check for rEndCol is done below.

    o_bShrunk = false;

    PutInOrder( rStartCol, rEndCol);
    PutInOrder( rStartRow, rEndRow);
    if (rStartCol < 0)
    {
        rStartCol = 0;
        o_bShrunk = true;
    }
    if (rStartRow < 0)
    {
        rStartRow = 0;
        o_bShrunk = true;
    }
    if (rEndCol >= aCol.size())
    {
        rEndCol = aCol.size()-1;
        o_bShrunk = true;
    }
    if (rEndRow > MAXROW)
    {
        rEndRow = MAXROW;
        o_bShrunk = true;
    }

    while (rStartCol < rEndCol)
    {
        if (aCol[rEndCol].IsEmptyBlock( rStartRow, rEndRow))
        {
            if (bConsiderCellNotes && !aCol[rEndCol].IsNotesEmptyBlock( rStartRow, rEndRow ))
                break;

            if (bConsiderCellDrawObjects && !aCol[rEndCol].IsDrawObjectsEmptyBlock( rStartRow, rEndRow ))
                break;

            --rEndCol;
            o_bShrunk = true;
        }
        else
            break;  // while
    }

    if (!bStickyLeftCol)
    {
        while (rStartCol < rEndCol)
        {
            if (aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow))
            {
                if (bConsiderCellNotes && !aCol[rStartCol].IsNotesEmptyBlock( rStartRow, rEndRow ))
                    break;

                if (bConsiderCellDrawObjects && !aCol[rStartCol].IsDrawObjectsEmptyBlock( rStartRow, rEndRow ))
                    break;

                ++rStartCol;
                o_bShrunk = true;
            }
            else
                break;  // while
        }
    }

    if (!bColumnsOnly)
    {
        if (!bStickyTopRow)
        {
            while (rStartRow < rEndRow)
            {
                bool bFound = false;
                for (SCCOL i=rStartCol; i<=rEndCol && !bFound; i++)
                {
                    if (aCol[i].HasDataAt( rStartRow, bConsiderCellNotes, bConsiderCellDrawObjects))
                        bFound = true;
                }
                if (!bFound)
                {
                    ++rStartRow;
                    o_bShrunk = true;
                }
                else
                    break;  // while
            }
        }

        while (rStartRow < rEndRow)
        {
            SCROW nLastDataRow = GetLastDataRow( rStartCol, rEndCol, rEndRow,
                                                 bConsiderCellNotes, bConsiderCellDrawObjects);
            if (0 <= nLastDataRow && nLastDataRow < rEndRow)
            {
                rEndRow = std::max( rStartRow, nLastDataRow);
                o_bShrunk = true;
            }
            else
                break;  // while
        }
    }

    return rStartCol != rEndCol || (bColumnsOnly ?
            !aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow) :
            (rStartRow != rEndRow ||
                aCol[rStartCol].HasDataAt( rStartRow, bConsiderCellNotes, bConsiderCellDrawObjects)));
}

SCROW ScTable::GetLastDataRow( SCCOL nCol1, SCCOL nCol2, SCROW nLastRow,
                               bool bConsiderCellNotes, bool bConsiderCellDrawObjects ) const
{
    if ( !IsColValid( nCol1 ) || !ValidCol( nCol2 ) )
        return -1;

    nCol2 = std::min<SCCOL>( nCol2, aCol.size() - 1 );

    SCROW nNewLastRow = 0;
    for (SCCOL i = nCol1; i <= nCol2; ++i)
    {
        SCROW nThis = aCol[i].GetLastDataPos(nLastRow, bConsiderCellNotes, bConsiderCellDrawObjects);
        if (nNewLastRow < nThis)
            nNewLastRow = nThis;
    }

    return nNewLastRow;
}

SCSIZE ScTable::GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir ) const
{
    SCCOL nStartColOrig = nStartCol;
    SCCOL nEndColOrig   = nEndCol;
    nStartCol = std::min<SCCOL>( nStartCol, aCol.size()-1 );
    nEndCol   = std::min<SCCOL>( nEndCol,   aCol.size()-1 );

    // The region is not allocated and does not contain any data.
    if ( nStartColOrig != nStartCol )
        return ( ((eDir == DIR_BOTTOM) || (eDir == DIR_TOP)) ?
                 static_cast<SCSIZE>(nEndRow - nStartRow + 1) :
                 static_cast<SCSIZE>(nEndColOrig - nStartColOrig + 1) );

    SCSIZE nGapRight = static_cast<SCSIZE>(nEndColOrig - nEndCol);
    SCSIZE nCount = 0;
    SCCOL nCol;
    if ((eDir == DIR_BOTTOM) || (eDir == DIR_TOP))
    {
        nCount = static_cast<SCSIZE>(nEndRow - nStartRow + 1);
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            nCount = std::min(nCount, aCol[nCol].GetEmptyLinesInBlock(nStartRow, nEndRow, eDir));
    }
    else if (eDir == DIR_RIGHT)
    {
        nCol = nEndCol;
        while ((nCol >= nStartCol) &&
                 aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
        {
            nCount++;
            nCol--;
        }
        nCount += nGapRight;
    }
    else
    {
        nCol = nStartCol;
        while ((nCol <= nEndCol) && aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
        {
            nCount++;
            nCol++;
        }

        // If the area between nStartCol and nEndCol are empty,
        // add the count of unallocated columns on the right.
        if ( nCol > nEndCol )
            nCount += nGapRight;
    }
    return nCount;
}

bool ScTable::IsEmptyLine( SCROW nRow, SCCOL nStartCol, SCCOL nEndCol ) const
{
    // The range of columns are unallocated hence empty.
    if ( nStartCol >= aCol.size() )
        return true;

    nEndCol   = std::min<SCCOL>( nEndCol,   aCol.size()-1 );

    bool bFound = false;
    for (SCCOL i=nStartCol; i<=nEndCol && !bFound; i++)
        if (aCol[i].HasDataAt(nRow))
            bFound = true;
    return !bFound;
}

void ScTable::LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow ) const
{
    rStartCol = std::min<SCCOL>( rStartCol, aCol.size()-1 );
    rEndCol   = std::min<SCCOL>( rEndCol,   aCol.size()-1 );

    while ( rStartCol<rEndCol && aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
        ++rStartCol;

    while ( rStartCol<rEndCol && aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
        --rEndCol;

    while ( rStartRow<rEndRow && IsEmptyLine(rStartRow, rStartCol, rEndCol) )
        ++rStartRow;

    // Optimised loop for finding the bottom of the area, can be costly in large
    // spreadsheets.
    SCROW lastDataPos = 0;
    for (SCCOL i=rStartCol; i<=rEndCol; i++)
        lastDataPos = std::max(lastDataPos, aCol[i].GetLastDataPos());
    rEndRow = std::max( rStartRow, std::min(rEndRow, lastDataPos));
}

SCCOL ScTable::FindNextVisibleCol( SCCOL nCol, bool bRight ) const
{
    if(bRight)
    {
        nCol++;
        SCCOL nEnd = 0;
        bool bHidden = pDocument->ColHidden(nCol, nTab, nullptr, &nEnd);
        if(bHidden)
            nCol = nEnd +1;

        return std::min<SCCOL>(MAXCOL, nCol);
    }
    else
    {
        nCol--;
        SCCOL nStart = MAXCOL;
        bool bHidden = pDocument->ColHidden(nCol, nTab, &nStart);
        if(bHidden)
            nCol = nStart - 1;

        return std::max<SCCOL>(0, nCol);
    }
}

SCCOL ScTable::FindNextVisibleColWithContent( SCCOL nCol, bool bRight, SCROW nRow ) const
{
    const SCCOL nLastCol = aCol.size() - 1;
    if(bRight)
    {
        // If nCol is the last allocated column index, there won't be any content to its right.
        // To maintain the original return behaviour, return MAXCOL.
        if(nCol >= nLastCol)
            return MAXCOL;

        do
        {
            nCol++;
            SCCOL nEndCol = 0;
            bool bHidden = pDocument->ColHidden( nCol, nTab, nullptr, &nEndCol );
            if(bHidden)
            {
                nCol = nEndCol +1;
                // Can end search early as there is no data after nLastCol.
                // For nCol == nLastCol, it may still have data so don't want to return MAXCOL.
                if(nCol > nLastCol)
                    return MAXCOL;
            }

            if(aCol[nCol].HasVisibleDataAt(nRow))
                return nCol;
        }
        while(nCol < nLastCol); // Stop search as soon as the last allocated column is searched.

        return MAXCOL;
    }
    else
    {
        // If nCol is in the unallocated range [nLastCol+1, MAXCOL], then move it directly to nLastCol
        // as there is no data in the unallocated range. This also makes the search faster and avoids
        // the need for more range checks in the loop below.
        if ( nCol > nLastCol )
            nCol = nLastCol;

        if(nCol == 0)
            return 0;

        do
        {
            nCol--;
            SCCOL nStartCol = MAXCOL;
            bool bHidden = pDocument->ColHidden( nCol, nTab, &nStartCol );
            if(bHidden)
            {
                nCol = nStartCol -1;
                if(nCol <= 0)
                    return 0;
            }

            if(aCol[nCol].HasVisibleDataAt(nRow))
                return nCol;
        }
        while(nCol > 0);

        return 0;
    }
}

void ScTable::FindAreaPos( SCCOL& rCol, SCROW& rRow, ScMoveDirection eDirection ) const
{
    const SCCOL nLastCol = aCol.size() - 1;

    if (eDirection == SC_MOVE_LEFT || eDirection == SC_MOVE_RIGHT)
    {
        SCCOL nNewCol = rCol;
        bool bThere = ( nNewCol <= nLastCol ) && aCol[nNewCol].HasVisibleDataAt(rRow);
        bool bRight = (eDirection == SC_MOVE_RIGHT);
        if (bThere)
        {
            if(nNewCol >= MAXCOL && eDirection == SC_MOVE_RIGHT)
                return;
            else if(nNewCol == 0 && eDirection == SC_MOVE_LEFT)
                return;

            SCCOL nNextCol = FindNextVisibleCol( nNewCol, bRight );

            if( nNextCol <= nLastCol && aCol[nNextCol].HasVisibleDataAt(rRow) )
            {
                bool bFound = false;
                nNewCol = nNextCol;
                do
                {
                    nNextCol = FindNextVisibleCol( nNewCol, bRight );
                    if( nNextCol <= nLastCol && aCol[nNextCol].HasVisibleDataAt(rRow) )
                        nNewCol = nNextCol;
                    else
                        bFound = true;
                }
                while(!bFound && nNextCol > 0 && nNextCol < MAXCOL);
            }
            else
            {
                nNewCol = FindNextVisibleColWithContent(nNewCol, bRight, rRow);
            }
        }
        else
        {
            nNewCol = FindNextVisibleColWithContent(nNewCol, bRight, rRow);
        }

        if (nNewCol<0)
            nNewCol=0;
        if (nNewCol>MAXCOL)
            nNewCol=MAXCOL;
        rCol = nNewCol;
    }
    else
    {
        if ( rCol <= nLastCol )
            aCol[rCol].FindDataAreaPos(rRow,eDirection == SC_MOVE_DOWN);
        else
        {
            // The cell (rCol, rRow) is equivalent to an empty cell (although not allocated).
            // Set rRow to 0 or MAXROW depending on eDirection to maintain the behaviour of
            // ScColumn::FindDataAreaPos() when the given column is empty.
            rRow = ( eDirection == SC_MOVE_DOWN ) ? MAXROW : 0;
        }
    }
}

bool ScTable::ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
                                bool bMarked, bool bUnprotected ) const
{
    if (!ValidCol(nCol) || !ValidRow(nRow))
        return false;

    if (pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HasAttrFlags::Overlapped))
        // Skip an overlapped cell.
        return false;

    if (bMarked && !rMark.IsCellMarked(nCol,nRow))
        return false;

    /* TODO: for cursor movement *only* this should even take the protection
     * options (select locked, select unlocked) into account, see
     * ScTabView::SkipCursorHorizontal() and ScTabView::SkipCursorVertical(). */
    if (bUnprotected && pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HasAttrFlags::Protected))
        return false;

    if (bMarked || bUnprotected)        //TODO: also in other case ???
    {
        // Hidden cells must be skipped, as the cursor would end up on the next cell
        // even if it is protected or not marked.
        //TODO: control per Extra-Parameter, only for Cursor movement ???

        if (RowHidden(nRow))
            return false;

        if (ColHidden(nCol))
            return false;
    }

    return true;
}

// Skips the current cell if it is Hidden, Overlapped or Protected and Sheet is Protected
bool ScTable::SkipRow( const SCCOL nCol, SCROW& rRow, const SCROW nMovY,
        const ScMarkData& rMark, const bool bUp, const SCROW nUsedY,
        const bool bMarked, const bool bSheetProtected ) const
{
    if ( !ValidRow( rRow ))
        return false;

    if (bSheetProtected && pDocument->HasAttrib( nCol, rRow, nTab, nCol, rRow, nTab, HasAttrFlags::Protected))
    {
        if ( rRow > nUsedY )
            rRow = (bUp ? nUsedY : MAXROW + nMovY);
        else
            rRow += nMovY;

        if (bMarked)
            rRow  = rMark.GetNextMarked( nCol, rRow, bUp );

        return true;
    }
    else
    {
        bool bRowHidden  = RowHidden( rRow );
        bool bOverlapped = pDocument->HasAttrib( nCol, rRow, nTab, nCol, rRow, nTab, HasAttrFlags::Overlapped );

        if ( bRowHidden || bOverlapped )
        {
            rRow += nMovY;
            if (bMarked)
                rRow = rMark.GetNextMarked( nCol, rRow, bUp );

            return true;
        }
    }

    return false;
}

void ScTable::GetNextPos( SCCOL& rCol, SCROW& rRow, SCCOL nMovX, SCROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark ) const
{
    // Ensure bMarked is set only if there is a mark.
    assert( !bMarked || rMark.IsMarked() || rMark.IsMultiMarked());

    const bool bSheetProtected = IsProtected();

    if ( bUnprotected && !bSheetProtected )     // Is sheet really protected?
        bUnprotected = false;

    SCCOL nCol = rCol + nMovX;
    SCROW nRow = rRow + nMovY;

    SCCOL nStartCol, nEndCol;
    SCROW nStartRow, nEndRow;
    if (bMarked)
    {
        ScRange aRange( ScAddress::UNINITIALIZED);
        if (rMark.IsMarked())
            rMark.GetMarkArea( aRange);
        else if (rMark.IsMultiMarked())
            rMark.GetMultiMarkArea( aRange);
        else
        {
            // Covered by assert() above, but for NDEBUG build.
            if (ValidColRow(nCol,nRow))
            {
                rCol = nCol;
                rRow = nRow;
            }
            return;
        }
        nStartCol = aRange.aStart.Col();
        nStartRow = aRange.aStart.Row();
        nEndCol = aRange.aEnd.Col();
        nEndRow = aRange.aEnd.Row();
    }
    else if (bUnprotected)
    {
        nStartCol = 0;
        nStartRow = 0;
        nEndCol = rCol;
        nEndRow = rRow;
        pDocument->GetPrintArea( nTab, nEndCol, nEndRow, true );
        // Add some cols/rows to the print area (which is "content or
        // visually different from empty") to enable travelling through
        // protected forms with empty cells and no visual indicator.
        // 42 might be good enough and not too much..
        nEndCol = std::min<SCCOL>( nEndCol+42, MAXCOL);
        nEndRow = std::min<SCROW>( nEndRow+42, MAXROW);
    }
    else
    {
        // Invalid values show up for instance for Tab, when nothing is
        // selected and not protected (left / right edge), then leave values
        // unchanged.
        if (ValidColRow(nCol,nRow))
        {
            rCol = nCol;
            rRow = nRow;
        }
        return;
    }

    if ( nMovY && (bMarked || bUnprotected))
    {
        bool bUp = (nMovY < 0);
        const SCCOL nColAdd = (bUp ? -1 : 1);
        sal_uInt16 nWrap = 0;

        if (bMarked)
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );

        while ( SkipRow( nCol, nRow, nMovY, rMark, bUp, nEndRow, bMarked, bSheetProtected ))
            ;

        while ( nRow < nStartRow || nRow > nEndRow )
        {
            nCol += nColAdd;

            while (nStartCol <= nCol && nCol <= nEndCol && ValidCol(nCol) && ColHidden(nCol))
                nCol += nColAdd;    //  skip hidden cols

            if (nCol < nStartCol)
            {
                nCol = nEndCol;

                if (++nWrap >= 2)
                    return;
            }
            else if (nCol > nEndCol)
            {
                nCol = nStartCol;

                if (++nWrap >= 2)
                    return;
            }
            if (nRow < nStartRow)
                nRow = nEndRow;
            else if (nRow > nEndRow)
                nRow = nStartRow;

            if (bMarked)
                nRow = rMark.GetNextMarked( nCol, nRow, bUp );

            while ( SkipRow( nCol, nRow, nMovY, rMark, bUp, nEndRow, bMarked, bSheetProtected ))
                ;
        }
    }

    if ( nMovX && ( bMarked || bUnprotected ) )
    {
        // wrap initial skip counting:
        if (nCol < nStartCol)
        {
            nCol = nEndCol;
            --nRow;
            if (nRow < nStartRow)
                nRow = nEndRow;
        }
        if (nCol > nEndCol)
        {
            nCol = nStartCol;
            ++nRow;
            if (nRow > nEndRow)
                nRow = nStartRow;
        }

        if ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) )
        {
            const SCCOL nColCount = nEndCol - nStartCol + 1;
            std::unique_ptr<SCROW[]> pNextRows( new SCROW[nColCount]);
            const SCCOL nLastCol = aCol.size() - 1;
            const bool bUp = (nMovX < 0);   // Moving left also means moving up in rows.
            const SCROW nRowAdd = (bUp ? -1 : 1);
            sal_uInt16 nWrap = 0;

            if (bUp)
            {
                for (SCCOL i = 0; i < nColCount; ++i)
                    pNextRows[i] = (i + nStartCol > nCol) ? (nRow + nRowAdd) : nRow;
            }
            else
            {
                for (SCCOL i = 0; i < nColCount; ++i)
                    pNextRows[i] = (i + nStartCol < nCol) ? (nRow + nRowAdd) : nRow;
            }
            do
            {
                SCROW nNextRow = pNextRows[nCol - nStartCol] + nRowAdd;
                if ( bMarked )
                    nNextRow = rMark.GetNextMarked( nCol, nNextRow, bUp );
                if ( bUnprotected )
                    nNextRow = ( nCol <= nLastCol ) ? aCol[nCol].GetNextUnprotected( nNextRow, bUp ) :
                        aDefaultColAttrArray.GetNextUnprotected( nNextRow, bUp );
                pNextRows[nCol - nStartCol] = nNextRow;

                if (bUp)
                {
                    SCROW nMaxRow = nStartRow - 1;
                    for (SCCOL i = 0; i < nColCount; ++i)
                    {
                        if (pNextRows[i] >= nMaxRow)    // when two equal the right one
                        {
                            nMaxRow = pNextRows[i];
                            nCol = i + nStartCol;
                        }
                    }
                    nRow = nMaxRow;

                    if ( nRow < nStartRow )
                    {
                        if (++nWrap >= 2)
                            return;
                        nCol = nEndCol;
                        nRow = nEndRow;
                        for (SCCOL i = 0; i < nColCount; ++i)
                            pNextRows[i] = nEndRow;     // do it all over again
                    }
                }
                else
                {
                    SCROW nMinRow = nEndRow + 1;
                    for (SCCOL i = 0; i < nColCount; ++i)
                    {
                        if (pNextRows[i] < nMinRow)     // when two equal the left one
                        {
                            nMinRow = pNextRows[i];
                            nCol = i + nStartCol;
                        }
                    }
                    nRow = nMinRow;

                    if ( nRow > nEndRow )
                    {
                        if (++nWrap >= 2)
                            return;
                        nCol = nStartCol;
                        nRow = nStartRow;
                        for (SCCOL i = 0; i < nColCount; ++i)
                            pNextRows[i] = nStartRow;   // do it all over again
                    }
                }
            }
            while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
        }
    }

    if (ValidColRow(nCol,nRow))
    {
        rCol = nCol;
        rRow = nRow;
    }
}

bool ScTable::GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark ) const
{
    ++rRow;                 // next row

    while ( rCol < aCol.size() )
    {
        ScMarkArray aArray( rMark.GetMarkArray( rCol ) );
        while ( rRow <= MAXROW )
        {
            SCROW nStart = aArray.GetNextMarked( rRow, false );
            if ( nStart <= MAXROW )
            {
                SCROW nEnd = aArray.GetMarkEnd( nStart, false );

                const sc::CellStoreType& rCells = aCol[rCol].maCells;
                std::pair<sc::CellStoreType::const_iterator,size_t> aPos = rCells.position(nStart);
                sc::CellStoreType::const_iterator it = aPos.first;
                SCROW nTestRow = nStart;
                if (it->type == sc::element_type_empty)
                {
                    // Skip the empty block.
                    nTestRow += it->size - aPos.second;
                    ++it;
                    if (it == rCells.end())
                    {
                        // No more block.  Move on to the next column.
                        rRow = MAXROW + 1;
                        continue;
                    }
                }

                if (nTestRow <= nEnd)
                {
                    // Cell found.
                    rRow = nTestRow;
                    return true;
                }

                rRow = nEnd + 1;                // Search for next selected range
            }
            else
                rRow = MAXROW + 1;              // End of column
        }
        rRow = 0;
        ++rCol;                                 // test next column
    }

    // Though searched only the allocated columns, it is equivalent to a search till MAXCOL.
    rCol = MAXCOL + 1;
    return false;                               // Through all columns
}

void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCCOL nDx, SCROW nDy, SCTAB nDz, bool bUpdateNoteCaptionPos )
{
    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       // only within the table
    {
        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if ( eUpdateRefMode != URM_COPY && pDrawLayer )
        {
            if ( eUpdateRefMode == URM_MOVE )
            {                                               // source range
                nCol1 = sal::static_int_cast<SCCOL>( nCol1 - nDx );
                nRow1 = sal::static_int_cast<SCROW>( nRow1 - nDy );
                nCol2 = sal::static_int_cast<SCCOL>( nCol2 - nDx );
                nRow2 = sal::static_int_cast<SCROW>( nRow2 - nDy );
            }
            pDrawLayer->MoveArea( nTab, nCol1,nRow1, nCol2,nRow2, nDx,nDy,
                                    (eUpdateRefMode == URM_INSDEL), bUpdateNoteCaptionPos );
        }
    }
}

void ScTable::UpdateReference(
    sc::RefUpdateContext& rCxt, ScDocument* pUndoDoc, bool bIncludeDraw, bool bUpdateNoteCaptionPos )
{
    bool bUpdated = false;
    SCCOL i;
    SCCOL iMax;
    if (rCxt.meMode == URM_COPY )
    {
        i = rCxt.maRange.aStart.Col();
        iMax = rCxt.maRange.aEnd.Col();
    }
    else
    {
        i = 0;
        iMax = MAXCOL;
    }

    UpdateRefMode eUpdateRefMode = rCxt.meMode;
    SCCOL nDx = rCxt.mnColDelta;
    SCROW nDy = rCxt.mnRowDelta;
    SCTAB nDz = rCxt.mnTabDelta;
    SCCOL nCol1 = rCxt.maRange.aStart.Col(), nCol2 = rCxt.maRange.aEnd.Col();
    SCROW nRow1 = rCxt.maRange.aStart.Row(), nRow2 = rCxt.maRange.aEnd.Row();
    SCTAB nTab1 = rCxt.maRange.aStart.Tab(), nTab2 = rCxt.maRange.aEnd.Tab();

    // Named expressions need to be updated before formulas accessing them.
    if (mpRangeName)
        mpRangeName->UpdateReference(rCxt, nTab);

    for ( ; i<=iMax; i++)
        bUpdated |= aCol[i].UpdateReference(rCxt, pUndoDoc);

    if ( bIncludeDraw )
        UpdateDrawRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz, bUpdateNoteCaptionPos );

    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       // print ranges: only within the table
    {
        SCTAB nSTab = nTab;
        SCTAB nETab = nTab;
        SCCOL nSCol = 0;
        SCROW nSRow = 0;
        SCCOL nECol = 0;
        SCROW nERow = 0;
        bool bRecalcPages = false;

        for ( auto& rPrintRange : aPrintRanges )
        {
            nSCol = rPrintRange.aStart.Col();
            nSRow = rPrintRange.aStart.Row();
            nECol = rPrintRange.aEnd.Col();
            nERow = rPrintRange.aEnd.Row();

            // do not try to modify sheet index of print range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                rPrintRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = true;
            }
        }

        if ( pRepeatColRange )
        {
            nSCol = pRepeatColRange->aStart.Col();
            nSRow = pRepeatColRange->aStart.Row();
            nECol = pRepeatColRange->aEnd.Col();
            nERow = pRepeatColRange->aEnd.Row();

            // do not try to modify sheet index of repeat range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatColRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = true;
                nRepeatStartX = nSCol;  // for UpdatePageBreaks
                nRepeatEndX = nECol;
            }
        }

        if ( pRepeatRowRange )
        {
            nSCol = pRepeatRowRange->aStart.Col();
            nSRow = pRepeatRowRange->aStart.Row();
            nECol = pRepeatRowRange->aEnd.Col();
            nERow = pRepeatRowRange->aEnd.Row();

            // do not try to modify sheet index of repeat range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatRowRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = true;
                nRepeatStartY = nSRow;  // for UpdatePageBreaks
                nRepeatEndY = nERow;
            }
        }

        //  updating print ranges is not necessary with multiple print ranges
        if ( bRecalcPages && GetPrintRangeCount() <= 1 )
        {
            UpdatePageBreaks(nullptr);

            pDocument->RepaintRange( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab) );
        }
    }

    if (bUpdated)
        SetStreamValid(false);

    if(mpCondFormatList)
        mpCondFormatList->UpdateReference(rCxt);

    if (pTabProtection)
        pTabProtection->updateReference( eUpdateRefMode, pDocument, rCxt.maRange, nDx, nDy, nDz);
}

void ScTable::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc )
{
    for (auto const & rpCol : aCol)
        rpCol->UpdateTranspose( rSource, rDest, pUndoDoc );
}

void ScTable::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    for (auto const & rpCol : aCol)
        rpCol->UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScTable::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    // Store the old tab number in sc::UpdatedRangeNames for
    // ScTokenArray::AdjustReferenceOnInsertedTab() to check with
    // isNameModified()
    if (mpRangeName)
        mpRangeName->UpdateInsertTab(rCxt, nTab);

    if (nTab >= rCxt.mnInsertPos)
    {
        nTab += rCxt.mnSheets;
        if (pDBDataNoName)
            pDBDataNoName->UpdateMoveTab(nTab - 1 ,nTab);
    }

    if (mpCondFormatList)
        mpCondFormatList->UpdateInsertTab(rCxt);

    if (pTabProtection)
        pTabProtection->updateReference( URM_INSDEL, pDocument,
                ScRange( 0, 0, rCxt.mnInsertPos, MAXCOL, MAXROW, MAXTAB),
                0, 0, rCxt.mnSheets);

    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].UpdateInsertTab(rCxt);

    SetStreamValid(false);
}

void ScTable::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    // Store the old tab number in sc::UpdatedRangeNames for
    // ScTokenArray::AdjustReferenceOnDeletedTab() to check with
    // isNameModified()
    if (mpRangeName)
        mpRangeName->UpdateDeleteTab(rCxt, nTab);

    if (nTab > rCxt.mnDeletePos)
    {
        nTab -= rCxt.mnSheets;
        if (pDBDataNoName)
            pDBDataNoName->UpdateMoveTab(nTab + 1,nTab);
    }

    if (mpCondFormatList)
        mpCondFormatList->UpdateDeleteTab(rCxt);

    if (pTabProtection)
        pTabProtection->updateReference( URM_INSDEL, pDocument,
                ScRange( 0, 0, rCxt.mnDeletePos, MAXCOL, MAXROW, MAXTAB),
                0, 0, -rCxt.mnSheets);

    for (SCCOL i = 0; i < aCol.size(); ++i)
        aCol[i].UpdateDeleteTab(rCxt);

    SetStreamValid(false);
}

void ScTable::UpdateMoveTab(
    sc::RefUpdateMoveTabContext& rCxt, SCTAB nTabNo, ScProgress* pProgress )
{
    nTab = nTabNo;
    if (mpRangeName)
        mpRangeName->UpdateMoveTab(rCxt, nTab);

    if (pDBDataNoName)
        pDBDataNoName->UpdateMoveTab(rCxt.mnOldPos, rCxt.mnNewPos);

    if(mpCondFormatList)
        mpCondFormatList->UpdateMoveTab(rCxt);

    if (pTabProtection)
        pTabProtection->updateReference( URM_REORDER, pDocument,
                ScRange( 0, 0, rCxt.mnOldPos, MAXCOL, MAXROW, MAXTAB),
                0, 0, rCxt.mnNewPos - rCxt.mnOldPos);

    for ( SCCOL i=0; i < aCol.size(); i++ )
    {
        aCol[i].UpdateMoveTab(rCxt, nTabNo);
        if (pProgress)
            pProgress->SetState(pProgress->GetState() + aCol[i].GetCodeCount());
    }

    SetStreamValid(false);
}

void ScTable::UpdateCompile( bool bForceIfNameInUse )
{
    for (SCCOL i=0; i < aCol.size(); i++)
    {
        aCol[i].UpdateCompile( bForceIfNameInUse );
    }
}

void ScTable::SetTabNo(SCTAB nNewTab)
{
    nTab = nNewTab;
    for (SCCOL i=0; i < aCol.size(); i++)
        aCol[i].SetTabNo(nNewTab);
}

void ScTable::FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                               sc::UpdatedRangeNames& rIndexes) const
{
    for (SCCOL i = nCol1; i <= nCol2 && IsColValid( i ); i++)
        aCol[i].FindRangeNamesInUse(nRow1, nRow2, rIndexes);
}

void ScTable::ExtendPrintArea( OutputDevice* pDev,
                    SCCOL /* nStartCol */, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow )
{
    if ( !mpColFlags || !pRowFlags )
    {
        OSL_FAIL("ExtendPrintArea: No ColInfo or RowInfo");
        return;
    }

    Point aPix1000 = pDev->LogicToPixel(Point(1000,1000), MapMode(MapUnit::MapTwip));
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    // First, mark those columns that we need to skip i.e. hidden and empty columns.

    ScFlatBoolColSegments aSkipCols;
    aSkipCols.setFalse(0, MAXCOL);
    for (SCCOL i = 0; i <= MAXCOL; ++i)
    {
        SCCOL nLastCol = i;
        if (ColHidden(i, nullptr, &nLastCol))
        {
            // Columns are hidden in this range.
            aSkipCols.setTrue(i, nLastCol);
        }
        else
        {
            // These columns are visible.  Check for empty columns.
            for (SCCOL j = i; j <= nLastCol; ++j)
            {
                if ( j >= aCol.size() )
                {
                    aSkipCols.setTrue( j, MAXCOL );
                    break;
                }
                if (aCol[j].GetCellCount() == 0)
                    // empty
                    aSkipCols.setTrue(j,j);
            }
        }
        i = nLastCol;
    }

    ScFlatBoolColSegments::RangeData aColData;
    for (SCCOL nCol = rEndCol; nCol >= 0; --nCol)
    {
        if (!aSkipCols.getRangeData(nCol, aColData))
            // Failed to get the data.  This should never happen!
            return;

        if (aColData.mbValue)
        {
            // Skip these columns.
            nCol = aColData.mnCol1; // move toward 0.
            continue;
        }

        // These are visible and non-empty columns.
        for (SCCOL nDataCol = nCol; 0 <= nDataCol && nDataCol >= aColData.mnCol1; --nDataCol)
        {
            SCCOL nPrintCol = nDataCol;
            VisibleDataCellIterator aIter(*mpHiddenRows, aCol[nDataCol]);
            ScRefCellValue aCell = aIter.reset(nStartRow);
            if (aCell.isEmpty())
                // No visible cells found in this column.  Skip it.
                continue;

            while (!aCell.isEmpty())
            {
                SCCOL nNewCol = nDataCol;
                SCROW nRow = aIter.getRow();
                if (nRow > nEndRow)
                    // Went past the last row position.  Bail out.
                    break;

                MaybeAddExtraColumn(nNewCol, nRow, pDev, nPPTX, nPPTY);
                if (nNewCol > nPrintCol)
                    nPrintCol = nNewCol;
                aCell = aIter.next();
            }

            if (nPrintCol > rEndCol)
                // Make sure we don't shrink the print area.
                rEndCol = nPrintCol;
        }
        nCol = aColData.mnCol1; // move toward 0.
    }
}

void ScTable::MaybeAddExtraColumn(SCCOL& rCol, SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY)
{
    ScRefCellValue aCell = aCol[rCol].GetCellValue(nRow);
    if (!aCell.hasString())
        return;

    long nPixel = aCol[rCol].GetTextWidth(nRow);

    // Width already calculated in Idle-Handler ?
    if ( TEXTWIDTH_DIRTY == nPixel )
    {
        ScNeededSizeOptions aOptions;
        aOptions.bTotalSize  = true;
        aOptions.bFormula    = false; //TODO: pass as parameter
        aOptions.bSkipMerged = false;

        Fraction aZoom(1,1);
        nPixel = aCol[rCol].GetNeededSize(
            nRow, pDev, nPPTX, nPPTY, aZoom, aZoom, true, aOptions, nullptr );

        aCol[rCol].SetTextWidth(nRow, static_cast<sal_uInt16>(nPixel));
    }

    long nTwips = static_cast<long>(nPixel / nPPTX);
    long nDocW = GetColWidth( rCol );

    long nMissing = nTwips - nDocW;
    if ( nMissing > 0 )
    {
        //  look at alignment

        const ScPatternAttr* pPattern = GetPattern( rCol, nRow );
        const SfxItemSet* pCondSet = pDocument->GetCondResult( rCol, nRow, nTab );

        SvxCellHorJustify eHorJust =
                        pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet ).GetValue();
        if ( eHorJust == SvxCellHorJustify::Center )
            nMissing /= 2;                          // distributed into both directions
        else
        {
            // STANDARD is LEFT (only text is handled here)
            bool bRight = ( eHorJust == SvxCellHorJustify::Right );
            if ( IsLayoutRTL() )
                bRight = !bRight;
            if ( bRight )
                nMissing = 0;       // extended only to the left (logical)
        }
    }

    SCCOL nNewCol = rCol;
    while (nMissing > 0 && nNewCol < MAXCOL)
    {
        ScRefCellValue aNextCell = aCol[nNewCol+1].GetCellValue(nRow);
        if (!aNextCell.isEmpty())
            // Cell content in a next column ends display of this string.
            nMissing = 0;
        else
            nMissing -= GetColWidth(++nNewCol);
    }
    rCol = nNewCol;
}

namespace {

class SetTableIndex
{
    SCTAB const mnTab;
public:
    explicit SetTableIndex(SCTAB nTab) : mnTab(nTab) {}

    void operator() (ScRange& rRange) const
    {
        rRange.aStart.SetTab(mnTab);
        rRange.aEnd.SetTab(mnTab);
    }
};

}

void ScTable::CopyPrintRange(const ScTable& rTable)
{
    // The table index shouldn't be used when the print range is used, but
    // just in case set the correct table index.

    aPrintRanges = rTable.aPrintRanges;
    ::std::for_each(aPrintRanges.begin(), aPrintRanges.end(), SetTableIndex(nTab));

    bPrintEntireSheet = rTable.bPrintEntireSheet;

    pRepeatColRange.reset();
    if (rTable.pRepeatColRange)
    {
        pRepeatColRange.reset(new ScRange(*rTable.pRepeatColRange));
        pRepeatColRange->aStart.SetTab(nTab);
        pRepeatColRange->aEnd.SetTab(nTab);
    }

    pRepeatRowRange.reset();
    if (rTable.pRepeatRowRange)
    {
        pRepeatRowRange.reset(new ScRange(*rTable.pRepeatRowRange));
        pRepeatRowRange->aStart.SetTab(nTab);
        pRepeatRowRange->aEnd.SetTab(nTab);
    }
}

void ScTable::SetRepeatColRange( std::unique_ptr<ScRange> pNew )
{
    pRepeatColRange = std::move(pNew);

    SetStreamValid(false);

    InvalidatePageBreaks();
}

void ScTable::SetRepeatRowRange( std::unique_ptr<ScRange> pNew )
{
    pRepeatRowRange = std::move(pNew);

    SetStreamValid(false);

    InvalidatePageBreaks();
}

void ScTable::ClearPrintRanges()
{
    aPrintRanges.clear();
    bPrintEntireSheet = false;

    SetStreamValid(false);

    InvalidatePageBreaks();     // #i117952# forget page breaks for an old print range
}

void ScTable::AddPrintRange( const ScRange& rNew )
{
    bPrintEntireSheet = false;
    if( aPrintRanges.size() < 0xFFFF )
        aPrintRanges.push_back( rNew );

    SetStreamValid(false);

    InvalidatePageBreaks();
}

void ScTable::SetPrintEntireSheet()
{
    if( !IsPrintEntireSheet() )
    {
        ClearPrintRanges();
        bPrintEntireSheet = true;
    }
}

const ScRange* ScTable::GetPrintRange(sal_uInt16 nPos) const
{
    return (nPos < GetPrintRangeCount()) ? &aPrintRanges[ nPos ] : nullptr;
}

void ScTable::FillPrintSaver( ScPrintSaverTab& rSaveTab ) const
{
    rSaveTab.SetAreas( aPrintRanges, bPrintEntireSheet );
    rSaveTab.SetRepeat( pRepeatColRange.get(), pRepeatRowRange.get() );
}

void ScTable::RestorePrintRanges( const ScPrintSaverTab& rSaveTab )
{
    aPrintRanges = rSaveTab.GetPrintRanges();
    bPrintEntireSheet = rSaveTab.IsEntireSheet();
    auto p = rSaveTab.GetRepeatCol();
    SetRepeatColRange( std::unique_ptr<ScRange>(p ? new ScRange(*p) : nullptr) );
    p = rSaveTab.GetRepeatRow();
    SetRepeatRowRange( std::unique_ptr<ScRange>(p ? new ScRange(*p) : nullptr) );

    InvalidatePageBreaks();     // #i117952# forget page breaks for an old print range
    UpdatePageBreaks(nullptr);
}

ScTable::VisibleDataCellIterator::VisibleDataCellIterator(ScFlatBoolRowSegments& rRowSegs, ScColumn& rColumn) :
    mrRowSegs(rRowSegs),
    mrColumn(rColumn),
    mnCurRow(ROW_NOT_FOUND),
    mnUBound(ROW_NOT_FOUND)
{
}

ScTable::VisibleDataCellIterator::~VisibleDataCellIterator()
{
}

ScRefCellValue ScTable::VisibleDataCellIterator::reset(SCROW nRow)
{
    if (nRow > MAXROW)
    {
        mnCurRow = ROW_NOT_FOUND;
        return ScRefCellValue();
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mrRowSegs.getRangeData(nRow, aData))
    {
        mnCurRow = ROW_NOT_FOUND;
        return ScRefCellValue();
    }

    if (!aData.mbValue)
    {
        // specified row is visible.  Take it.
        mnCurRow = nRow;
        mnUBound = aData.mnRow2;
    }
    else
    {
        // specified row is not-visible.  The first visible row is the start of
        // the next segment.
        mnCurRow = aData.mnRow2 + 1;
        mnUBound = mnCurRow; // get range data on the next iteration.
        if (mnCurRow > MAXROW)
        {
            // Make sure the row doesn't exceed our current limit.
            mnCurRow = ROW_NOT_FOUND;
            return ScRefCellValue();
        }
    }

    maCell = mrColumn.GetCellValue(mnCurRow);
    if (!maCell.isEmpty())
        // First visible cell found.
        return maCell;

    // Find a first visible cell below this row (if any).
    return next();
}

ScRefCellValue ScTable::VisibleDataCellIterator::next()
{
    if (mnCurRow == ROW_NOT_FOUND)
        return ScRefCellValue();

    while (mrColumn.GetNextDataPos(mnCurRow))
    {
        if (mnCurRow > mnUBound)
        {
            // We don't know the visibility of this row range.  Query it.
            ScFlatBoolRowSegments::RangeData aData;
            if (!mrRowSegs.getRangeData(mnCurRow, aData))
            {
                mnCurRow = ROW_NOT_FOUND;
                return ScRefCellValue();
            }

            if (aData.mbValue)
            {
                // This row is invisible.  Skip to the last invisible row and
                // try again.
                mnCurRow = mnUBound = aData.mnRow2;
                continue;
            }

            // This row is visible.
            mnUBound = aData.mnRow2;
        }

        maCell = mrColumn.GetCellValue(mnCurRow);
        if (!maCell.isEmpty())
            return maCell;
    }

    mnCurRow = ROW_NOT_FOUND;
    return ScRefCellValue();
}

void ScTable::SetAnonymousDBData(std::unique_ptr<ScDBData> pDBData)
{
    pDBDataNoName = std::move(pDBData);
}

sal_uLong ScTable::AddCondFormat( std::unique_ptr<ScConditionalFormat> pNew )
{
    if(!mpCondFormatList)
        mpCondFormatList.reset(new ScConditionalFormatList());

    sal_uInt32 nMax = mpCondFormatList->getMaxKey();

    pNew->SetKey(nMax+1);
    mpCondFormatList->InsertNew(std::move(pNew));

    return nMax + 1;
}

SvtScriptType ScTable::GetScriptType( SCCOL nCol, SCROW nRow ) const
{
    if ( !IsColValid( nCol ) )
        return SvtScriptType::NONE;

    return aCol[nCol].GetScriptType(nRow);
}

void ScTable::SetScriptType( SCCOL nCol, SCROW nRow, SvtScriptType nType )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].SetScriptType(nRow, nType);
}

SvtScriptType ScTable::GetRangeScriptType(
    sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    if ( !IsColValid( nCol ) )
        return SvtScriptType::NONE;

    sc::CellStoreType::iterator itr = aCol[nCol].maCells.begin();
    return aCol[nCol].GetRangeScriptType(rBlockPos.miCellTextAttrPos, nRow1, nRow2, itr);
}

size_t ScTable::GetFormulaHash( SCCOL nCol, SCROW nRow ) const
{
    if ( !IsColValid( nCol ) )
        return 0;

    return aCol[nCol].GetFormulaHash(nRow);
}

ScFormulaVectorState ScTable::GetFormulaVectorState( SCCOL nCol, SCROW nRow ) const
{
    if ( !IsColValid( nCol ) )
        return FormulaVectorUnknown;

    return aCol[nCol].GetFormulaVectorState(nRow);
}

formula::FormulaTokenRef ScTable::ResolveStaticReference( SCCOL nCol, SCROW nRow )
{
    if ( !ValidCol( nCol ) || !ValidRow( nRow ) )
        return formula::FormulaTokenRef();
    if ( nCol >= aCol.size() )
        // Return a value of 0.0 if column not exists
        return formula::FormulaTokenRef(new formula::FormulaDoubleToken(0.0));
    return aCol[nCol].ResolveStaticReference(nRow);
}

formula::FormulaTokenRef ScTable::ResolveStaticReference( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (nCol2 < nCol1 || nRow2 < nRow1)
        return formula::FormulaTokenRef();

    if ( !ValidCol( nCol1 ) || !ValidCol( nCol2 ) || !ValidRow( nRow1 ) || !ValidRow( nRow2 ) )
        return formula::FormulaTokenRef();

    SCCOL nMaxCol;
    if ( nCol2 >= aCol.size() )
        nMaxCol = aCol.size() - 1;
    else
        nMaxCol = nCol2;

    ScMatrixRef pMat(new ScMatrix(nCol2-nCol1+1, nRow2-nRow1+1, 0.0));
    for (SCCOL nCol = nCol1; nCol <= nMaxCol; ++nCol)
    {
        if (!aCol[nCol].ResolveStaticReference(*pMat, nCol2-nCol1, nRow1, nRow2))
            // Column contains non-static cell. Failed.
            return formula::FormulaTokenRef();
    }

    return formula::FormulaTokenRef(new ScMatrixToken(pMat));
}

formula::VectorRefArray ScTable::FetchVectorRefArray( SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    if (nRow2 < nRow1)
        return formula::VectorRefArray();

    if ( !IsColValid( nCol ) || !ValidRow( nRow1 ) || !ValidRow( nRow2 ) )
        return formula::VectorRefArray();

    return aCol[nCol].FetchVectorRefArray(nRow1, nRow2);
}

#ifdef DBG_UTIL
void ScTable::AssertNoInterpretNeeded( SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    assert( nRow2 >= nRow1 );
    assert( IsColValid( nCol ) && ValidRow( nRow1 ) && ValidRow( nRow2 ) );
    return aCol[nCol].AssertNoInterpretNeeded(nRow1, nRow2);
}
#endif

bool ScTable::HandleRefArrayForParallelism( SCCOL nCol, SCROW nRow1, SCROW nRow2, const ScFormulaCellGroupRef& mxGroup )
{
    if (nRow2 < nRow1)
        return false;

    if ( !IsColValid( nCol ) || !ValidRow( nRow1 ) || !ValidRow( nRow2 ) )
        return false;

    return aCol[nCol].HandleRefArrayForParallelism(nRow1, nRow2, mxGroup);
}

ScRefCellValue ScTable::GetRefCellValue( SCCOL nCol, SCROW nRow )
{
    if ( !IsColRowValid( nCol, nRow ) )
        return ScRefCellValue();

    return aCol[nCol].GetCellValue(nRow);
}

SvtBroadcaster* ScTable::GetBroadcaster( SCCOL nCol, SCROW nRow )
{
    if ( !IsColRowValid( nCol, nRow ) )
        return nullptr;

    return aCol[nCol].GetBroadcaster(nRow);
}

void ScTable::DeleteBroadcasters(
    sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    if ( !IsColValid( nCol ) )
        return;

    aCol[nCol].DeleteBroadcasters(rBlockPos, nRow1, nRow2);
}

void ScTable::FillMatrix( ScMatrix& rMat, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, svl::SharedStringPool* pPool ) const
{
    size_t nMatCol = 0;
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol, ++nMatCol)
        aCol[nCol].FillMatrix(rMat, nMatCol, nRow1, nRow2, pPool);
}

void ScTable::InterpretDirtyCells( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
        aCol[nCol].InterpretDirtyCells(nRow1, nRow2);
}

void ScTable::SetFormulaResults( SCCOL nCol, SCROW nRow, const double* pResults, size_t nLen )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].SetFormulaResults(nRow, pResults, nLen);
}

void ScTable::CalculateInColumnInThread( ScInterpreterContext& rContext, SCCOL nCol, SCROW nRow, size_t nLen, unsigned nThisThread, unsigned nThreadsTotal)
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].CalculateInThread( rContext, nRow, nLen, nThisThread, nThreadsTotal );
}

void ScTable::HandleStuffAfterParallelCalculation( SCCOL nCol, SCROW nRow, size_t nLen)
{
    assert(ValidCol(nCol));

    aCol[nCol].HandleStuffAfterParallelCalculation( nRow, nLen );
}

#if DUMP_COLUMN_STORAGE
void ScTable::DumpColumnStorage( SCCOL nCol ) const
{
    if ( !IsColValid( nCol ) )
        return;

    aCol[nCol].DumpColumnStorage();
}
#endif

const SvtBroadcaster* ScTable::GetBroadcaster( SCCOL nCol, SCROW nRow ) const
{
    if ( !IsColRowValid( nCol, nRow ) )
        return nullptr;

    return aCol[nCol].GetBroadcaster(nRow);
}

void ScTable::DeleteConditionalFormat( sal_uLong nIndex )
{
    mpCondFormatList->erase(nIndex);
}

void ScTable::SetCondFormList( ScConditionalFormatList* pNew )
{
    mpCondFormatList.reset( pNew );
}

ScConditionalFormatList* ScTable::GetCondFormList()
{
    if(!mpCondFormatList)
        mpCondFormatList.reset( new ScConditionalFormatList() );

    return mpCondFormatList.get();
}

const ScConditionalFormatList* ScTable::GetCondFormList() const
{
    return mpCondFormatList.get();
}

ScColumnsRange ScTable::GetColumnsRange(SCCOL nColBegin, SCCOL nColEnd) const
{
    // Because the range is inclusive, some code will pass nColEnd<nColBegin to
    // indicate an empty range. Ensure that we create only valid iterators for
    // the range, limit columns to bounds.
    SCCOL nEffBegin, nEffEnd;
    if (nColBegin <= nColEnd)
    {
        if (nColBegin < 0)
            nEffBegin = 0;
        else
            nEffBegin = std::min<SCCOL>( nColBegin, aCol.size());
        if (nColEnd < 0)
            nEffEnd = 0;
        else
            nEffEnd = std::min<SCCOL>( nColEnd + 1, aCol.size());
    }
    else
    {
        // Any empty will do.
        nEffBegin = nEffEnd = 0;
    }
    return ScColumnsRange( ScColumnsRange::Iterator( aCol.begin() + nEffBegin),
                           ScColumnsRange::Iterator( aCol.begin() + nEffEnd));
}

// out-of-line the cold part of the CreateColumnIfNotExists function
void ScTable::CreateColumnIfNotExistsImpl( const SCCOL nScCol )
{
    const SCCOL aOldColSize = aCol.size();
    aCol.resize( static_cast< size_t >( nScCol + 1 ) );
    for (SCCOL i = aOldColSize; i <= nScCol; i++)
        aCol[i].Init( i, nTab, pDocument, false );
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
