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

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>
#include <unotools/textsearch.hxx>
#include <sfx2/objsh.hxx>

#include "attrib.hxx"
#include "patattr.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "olinetab.hxx"
#include "stlsheet.hxx"
#include "global.hxx"
#include "globstr.hrc"
#include "refupdat.hxx"
#include "markdata.hxx"
#include "progress.hxx"
#include "hints.hxx"        // fuer Paint-Broadcast
#include "prnsave.hxx"
#include "tabprotection.hxx"
#include "sheetevents.hxx"
#include "segmenttree.hxx"
#include "dbdata.hxx"
#include "colorscale.hxx"
#include "conditio.hxx"

#include <vector>

using ::std::vector;

namespace {

ScProgress* GetProgressBar(
    SCSIZE nCount, SCSIZE nTotalCount, ScProgress* pOuterProgress, ScDocument* pDoc)
{
    if (nTotalCount < 1000)
    {
        // if the total number of rows is less than 1000, don't even bother
        // with the progress bar because drawing progress bar can be very
        // expensive especially in GTK.
        return NULL;
    }

    if (pOuterProgress)
        return pOuterProgress;

    if (nCount > 1)
        return new ScProgress(
            pDoc->GetDocumentShell(), ScGlobal::GetRscString(STR_PROGRESS_HEIGHTING), nTotalCount);

    return NULL;
}

void GetOptimalHeightsInColumn(
    ScColumn* pCol, SCROW nStartRow, SCROW nEndRow, vector<sal_uInt16>& aHeights,
    OutputDevice* pDev, double nPPTX, double nPPTY, const Fraction& rZoomX, const Fraction& rZoomY, bool bForce,
    ScProgress* pProgress, sal_uInt32 nProgressStart)
{
    SCSIZE nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    //  zuerst einmal ueber den ganzen Bereich
    //  (mit der letzten Spalte in der Hoffnung, dass die am ehesten noch auf
    //   Standard formatiert ist)

    pCol[MAXCOL].GetOptimalHeight(
            nStartRow, nEndRow, &aHeights[0], pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce, 0, 0 );

    //  daraus Standardhoehe suchen, die im unteren Bereich gilt

    sal_uInt16 nMinHeight = aHeights[nCount-1];
    SCSIZE nPos = nCount-1;
    while ( nPos && aHeights[nPos-1] >= nMinHeight )
        --nPos;
    SCROW nMinStart = nStartRow + nPos;

    sal_uLong nWeightedCount = 0;
    for (SCCOL nCol=0; nCol<MAXCOL; nCol++)     // MAXCOL schon oben
    {
        pCol[nCol].GetOptimalHeight(
            nStartRow, nEndRow, &aHeights[0], pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
            nMinHeight, nMinStart );

        if (pProgress)
        {
            sal_uLong nWeight = pCol[nCol].GetWeightedCount();
            if (nWeight)        // nochmal denselben Status muss auch nicht sein
            {
                nWeightedCount += nWeight;
                pProgress->SetState( nWeightedCount + nProgressStart );
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
    SetRowHeightOnlyFunc(ScTable* pTab) :
        mpTab(pTab)
    {}

    virtual bool operator() (SCROW nStartRow, SCROW nEndRow, sal_uInt16 nHeight)
    {
        mpTab->SetRowHeightOnly(nStartRow, nEndRow, nHeight);
        return false;
    }
};

struct SetRowHeightRangeFunc : public OptimalHeightsFuncObjBase
{
    ScTable* mpTab;
    double mnPPTX;
    double mnPPTY;

    SetRowHeightRangeFunc(ScTable* pTab, double nPPTX, double nPPTY) :
        mpTab(pTab),
        mnPPTX(nPPTX),
        mnPPTY(nPPTY)
    {}

    virtual bool operator() (SCROW nStartRow, SCROW nEndRow, sal_uInt16 nHeight)
    {
        return mpTab->SetRowHeightRange(nStartRow, nEndRow, nHeight, mnPPTX, mnPPTY);
    }
};

bool SetOptimalHeightsToRows( OptimalHeightsFuncObjBase& rFuncObj, ScTable* pTab,
    ScBitMaskCompressedArray<SCROW, sal_uInt8>* pRowFlags, SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
    const vector<sal_uInt16>& aHeights, bool bForce)
{
    pTab->IncRecalcLevel();       // #i116460# avoid problems with Excel files

    SCSIZE nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);
    bool bChanged = false;
    SCROW nRngStart = 0;
    SCROW nRngEnd = 0;
    sal_uInt16 nLast = 0;
    for (SCSIZE i=0; i<nCount; i++)
    {
        size_t nIndex;
        SCROW nRegionEndRow;
        sal_uInt8 nRowFlag = pRowFlags->GetValue( nStartRow+i, nIndex, nRegionEndRow );
        if ( nRegionEndRow > nEndRow )
            nRegionEndRow = nEndRow;
        SCSIZE nMoreRows = nRegionEndRow - ( nStartRow+i );     // additional equal rows after first

        bool bAutoSize = ((nRowFlag & CR_MANUALSIZE) == 0);
        if ( bAutoSize || bForce )
        {
            if (nExtra)
            {
                if (bAutoSize)
                    pRowFlags->SetValue( nStartRow+i, nRegionEndRow, nRowFlag | CR_MANUALSIZE);
            }
            else if (!bAutoSize)
                pRowFlags->SetValue( nStartRow+i, nRegionEndRow, nRowFlag & ~CR_MANUALSIZE);

            for (SCSIZE nInner = i; nInner <= i + nMoreRows; ++nInner)
            {
                if (nLast)
                {
                    if (aHeights[nInner]+nExtra == nLast)
                        nRngEnd = nStartRow+nInner;
                    else
                    {
                        bChanged |= rFuncObj(nRngStart, nRngEnd, nLast);
                        nLast = 0;
                    }
                }
                if (!nLast)
                {
                    nLast = aHeights[nInner]+nExtra;
                    nRngStart = nStartRow+nInner;
                    nRngEnd = nStartRow+nInner;
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

    pTab->DecRecalcLevel();       // #i116460# avoid problems with Excel files

    return bChanged;
}

}

// -----------------------------------------------------------------------

ScTable::ScTable( ScDocument* pDoc, SCTAB nNewTab, const rtl::OUString& rNewName,
                    bool bColInfo, bool bRowInfo ) :
    aName( rNewName ),
    aCodeName( rNewName ),
    nLinkMode( 0 ),
    aPageStyle( ScGlobal::GetRscString(STR_STYLENAME_STANDARD) ),
    nRepeatStartX( SCCOL_REPEAT_NONE ),
    nRepeatEndX( SCCOL_REPEAT_NONE ),
    nRepeatStartY( SCROW_REPEAT_NONE ),
    nRepeatEndY( SCROW_REPEAT_NONE ),
    pTabProtection( NULL ),
    pColWidth( NULL ),
    mpRowHeights( static_cast<ScFlatUInt16RowSegments*>(NULL) ),
    pColFlags( NULL ),
    pRowFlags( NULL ),
    mpHiddenCols(new ScFlatBoolColSegments),
    mpHiddenRows(new ScFlatBoolRowSegments),
    mpFilteredCols(new ScFlatBoolColSegments),
    mpFilteredRows(new ScFlatBoolRowSegments),
    pOutlineTable( NULL ),
    pSheetEvents( NULL ),
    nTab( nNewTab ),
    nRecalcLvl( 0 ),
    pDocument( pDoc ),
    pSearchText ( NULL ),
    pSortCollator( NULL ),
    pRepeatColRange( NULL ),
    pRepeatRowRange( NULL ),
    nLockCount( 0 ),
    pScenarioRanges( NULL ),
    aScenarioColor( COL_LIGHTGRAY ),
    aTabBgColor( COL_AUTO ),
    nScenarioFlags( 0 ),
    pDBDataNoName(NULL),
    mpRangeName(NULL),
    mpCondFormatList( new ScConditionalFormatList() ),
    maNotes(pDoc),
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
    mbPageBreaksValid(false)
{

    if (bColInfo)
    {
        pColWidth  = new sal_uInt16[ MAXCOL+1 ];
        pColFlags  = new sal_uInt8[ MAXCOL+1 ];

        for (SCCOL i=0; i<=MAXCOL; i++)
        {
            pColWidth[i] = STD_COL_WIDTH;
            pColFlags[i] = 0;
        }
    }

    if (bRowInfo)
    {
        mpRowHeights.reset(new ScFlatUInt16RowSegments(ScGlobal::nStdRowHeight));
        pRowFlags  = new ScBitMaskCompressedArray< SCROW, sal_uInt8>( MAXROW, 0);
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
            sal_uLong nx = (sal_uLong) ((double) (MAXCOL+1) * STD_COL_WIDTH           * HMM_PER_TWIPS );
            sal_uLong ny = (sal_uLong) ((double) (MAXROW+1) * ScGlobal::nStdRowHeight * HMM_PER_TWIPS );
            pDrawLayer->SetPageSize( static_cast<sal_uInt16>(nTab), Size( nx, ny ), false );
        }
    }

    for (SCCOL k=0; k<=MAXCOL; k++)
        aCol[k].Init( k, nTab, pDocument );
}

ScTable::~ScTable()
{
    if (!pDocument->IsInDtorClear())
    {
        //  nicht im dtor die Pages in der falschen Reihenfolge loeschen
        //  (nTab stimmt dann als Page-Number nicht!)
        //  In ScDocument::Clear wird hinterher per Clear am Draw Layer alles geloescht.

        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if (pDrawLayer)
            pDrawLayer->ScRemovePage( nTab );
    }

    delete[] pColWidth;
    delete[] pColFlags;
    delete pRowFlags;
    delete pSheetEvents;
    delete pOutlineTable;
    delete pSearchText;
    delete pRepeatColRange;
    delete pRepeatRowRange;
    delete pScenarioRanges;
    delete mpRangeName;
    delete pDBDataNoName;
    DestroySortCollator();
}

void ScTable::GetName( rtl::OUString& rName ) const
{
    rName = aName;
}

void ScTable::SetName( const rtl::OUString& rNewName )
{
    aName = rNewName;
    aUpperName = rtl::OUString(); // invalidated if the name is changed

    // SetStreamValid is handled in ScDocument::RenameTab
}

const rtl::OUString& ScTable::GetUpperName() const
{
    if (aUpperName.isEmpty() && !aName.isEmpty())
        aUpperName = ScGlobal::pCharClass->uppercase(aName);
    return aUpperName;
}

void ScTable::SetVisible( bool bVis )
{
    if (bVisible != bVis && IsStreamValid())
        SetStreamValid(false);

    bVisible = bVis;
}

void ScTable::SetStreamValid( bool bSet, bool bIgnoreLock )
{
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

const Color& ScTable::GetTabBgColor() const
{
    return aTabBgColor;
}

void ScTable::SetTabBgColor(const Color& rColor)
{
    if (aTabBgColor != rColor)
    {
        // The tab color has changed.  Set this table 'modified'.
        aTabBgColor = rColor;
        if (IsStreamValid())
            SetStreamValid(false);
    }
}

void ScTable::SetScenario( bool bFlag )
{
    bScenario = bFlag;
}

void ScTable::SetLink( sal_uInt8 nMode,
                        const String& rDoc, const String& rFlt, const String& rOpt,
                        const String& rTab, sal_uLong nRefreshDelay )
{
    nLinkMode = nMode;
    aLinkDoc = rDoc;        // Datei
    aLinkFlt = rFlt;        // Filter
    aLinkOpt = rOpt;        // Filter-Optionen
    aLinkTab = rTab;        // Tabellenname in Quelldatei
    nLinkRefreshDelay = nRefreshDelay;  // refresh delay in seconds, 0==off

    if (IsStreamValid())
        SetStreamValid(false);
}

sal_uInt16 ScTable::GetOptimalColWidth( SCCOL nCol, OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bFormula, const ScMarkData* pMarkData,
                                    const ScColWidthParam* pParam )
{
    return aCol[nCol].GetOptimalColWidth( pDev, nPPTX, nPPTY, rZoomX, rZoomY,
        bFormula, STD_COL_WIDTH - STD_EXTRA_WIDTH, pMarkData, pParam );
}

long ScTable::GetNeededSize( SCCOL nCol, SCROW nRow,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                bool bWidth, bool bTotalSize )
{
    ScNeededSizeOptions aOptions;
    aOptions.bSkipMerged = false;       // zusammengefasste mitzaehlen
    aOptions.bTotalSize  = bTotalSize;

    return aCol[nCol].GetNeededSize
        ( nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, aOptions );
}

bool ScTable::SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                bool bForce, ScProgress* pOuterProgress, sal_uLong nProgressStart )
{
    OSL_ENSURE( nExtra==0 || bForce, "autom. OptimalHeight mit Extra" );

    if ( !pDocument->IsAdjustHeightEnabled() )
    {
        return false;
    }

    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = GetProgressBar(nCount, GetWeightedCount(), pOuterProgress, pDocument);

    vector<sal_uInt16> aHeights(nCount, 0);

    GetOptimalHeightsInColumn(
        aCol, nStartRow, nEndRow, aHeights, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
        pProgress, nProgressStart);

    SetRowHeightRangeFunc aFunc(this, nPPTX, nPPTY);
    bool bChanged = SetOptimalHeightsToRows(
        aFunc, this, pRowFlags, nStartRow, nEndRow, nExtra, aHeights, bForce);

    if ( pProgress != pOuterProgress )
        delete pProgress;

    return bChanged;
}

void ScTable::SetOptimalHeightOnly( SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                bool bForce, ScProgress* pOuterProgress, sal_uLong nProgressStart )
{
    OSL_ENSURE( nExtra==0 || bForce, "autom. OptimalHeight mit Extra" );

    if ( !pDocument->IsAdjustHeightEnabled() )
        return;

    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = GetProgressBar(nCount, GetWeightedCount(), pOuterProgress, pDocument);

    vector<sal_uInt16> aHeights(nCount, 0);

    GetOptimalHeightsInColumn(
        aCol, nStartRow, nEndRow, aHeights, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bForce,
        pProgress, nProgressStart);

    SetRowHeightOnlyFunc aFunc(this);
    SetOptimalHeightsToRows(
        aFunc, this, pRowFlags, nStartRow, nEndRow, nExtra, aHeights, bForce);

    if ( pProgress != pOuterProgress )
        delete pProgress;
}

bool ScTable::GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    for (SCCOL i=0; i<=MAXCOL; i++)
        if (!aCol[i].IsEmptyVisData())
        {
            bFound = true;
            nMaxX = i;
            SCROW nColY = aCol[i].GetLastVisDataPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    for (ScNotes::const_iterator itr = maNotes.begin(); itr != maNotes.end(); ++itr)
    {
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;

        if (nMaxX < nCol)
            nMaxX = nCol;
        if (nMaxY < nRow)
            nMaxY = nRow;
    }

    rEndCol = nMaxX;
    rEndRow = nMaxY;
    return bFound;
}

bool ScTable::GetTableArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
    bool bRet = true;               //! merken?
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

bool ScTable::GetPrintArea( SCCOL& rEndCol, SCROW& rEndRow, bool bNotes, bool bFullFormattedArea ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    SCCOL i;

    for (i=0; i<=MAXCOL; i++)               // Daten testen
        if (!aCol[i].IsEmptyVisData())
        {
            bFound = true;
            if (i>nMaxX)
                nMaxX = i;
            SCROW nColY = aCol[i].GetLastVisDataPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    if (bNotes)
    {
        for (ScNotes::const_iterator itr = maNotes.begin(); itr != maNotes.end(); ++itr)
        {
            SCCOL nCol = itr->first.first;
            SCROW nRow = itr->first.second;

            if (nMaxX < nCol)
                nMaxX = nCol;
            if (nMaxY < nRow)
                nMaxY = nRow;
        }
    }

    SCCOL nMaxDataX = nMaxX;

    for (i=0; i<=MAXCOL; i++)               // Attribute testen
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow, bFullFormattedArea ))
        {
            bFound = true;
            nMaxX = i;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    if (nMaxX == MAXCOL)                    // Attribute rechts weglassen
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
        while ( nAttrStartX < MAXCOL )
        {
            SCCOL nAttrEndX = nAttrStartX;
            while ( nAttrEndX < MAXCOL && aCol[nAttrStartX].IsVisibleAttrEqual(aCol[nAttrEndX+1]) )
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
                                SCCOL& rEndCol, bool /* bNotes */ ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCCOL i;

    for (i=0; i<=MAXCOL; i++)               // Attribute testen
    {
        if (aCol[i].HasVisibleAttrIn( nStartRow, nEndRow ))
        {
            bFound = true;
            nMaxX = i;
        }
    }

    if (nMaxX == MAXCOL)                    // Attribute rechts weglassen
    {
        --nMaxX;
        while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1], nStartRow, nEndRow) )
            --nMaxX;
    }

    for (i=0; i<=MAXCOL; i++)               // Daten testen
    {
        if (!aCol[i].IsEmptyBlock( nStartRow, nEndRow ))        //! bNotes ??????
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
    bool bFound = false;
    SCROW nMaxY = 0;
    SCCOL i;

    for (i=nStartCol; i<=nEndCol; i++)              // Attribute testen
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow ))
        {
            bFound = true;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    for (i=nStartCol; i<=nEndCol; i++)              // Daten testen
        if (!aCol[i].IsEmptyVisData())
        {
            bFound = true;
            SCROW nColY = aCol[i].GetLastVisDataPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }

    if (bNotes)
    {
        for (ScNotes::const_iterator itr = maNotes.begin(); itr != maNotes.end(); ++itr)
        {
            SCCOL nCol = itr->first.first;
            SCROW nRow = itr->first.second;

            if (nStartCol > nCol || nEndCol < nCol)
                continue;

            if (nMaxY < nRow)
                nMaxY = nRow;
        }
    }

    rEndRow = nMaxY;
    return bFound;
}

bool ScTable::GetDataStart( SCCOL& rStartCol, SCROW& rStartRow ) const
{
    bool bFound = false;
    SCCOL nMinX = MAXCOL;
    SCROW nMinY = MAXROW;
    SCCOL i;

    for (i=0; i<=MAXCOL; i++)                   // Attribute testen
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

    if (nMinX == 0)                                     // Attribute links weglassen
    {
        if ( aCol[0].IsVisibleAttrEqual(aCol[1]) )      // keine einzelnen
        {
            ++nMinX;
            while ( nMinX<MAXCOL && aCol[nMinX].IsVisibleAttrEqual(aCol[nMinX-1]) )
                ++nMinX;
        }
    }

    bool bDatFound = false;
    for (i=0; i<=MAXCOL; i++)                   // Daten testen
        if (!aCol[i].IsEmptyVisData())
        {
            if (!bDatFound && i<nMinX)
                nMinX = i;
            bFound = bDatFound = true;
            SCROW nColY = aCol[i].GetFirstVisDataPos();
            if (nColY < nMinY)
                nMinY = nColY;
        }

    for (ScNotes::const_iterator itr = maNotes.begin(); itr != maNotes.end(); ++itr)
    {
        bFound = bDatFound = true;
        SCCOL nCol = itr->first.first;
        SCROW nRow = itr->first.second;

        if (nMinX > nCol)
            nMinX = nCol;
        if (nMinY > nRow)
            nMinY = nRow;
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
    //                   independently of the emptniess of rows / columns (i.e. does not allow shrinking)
    //     bOnlyDown = true means extend / shrink the inputed area only down, i.e modifiy only rEndRow

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

            if (rEndCol < MAXCOL)
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
            while ( aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) && rStartCol < MAXCOL && rStartCol < rEndCol)
                ++rStartCol;

        if ( !bRight )
            while ( aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) && rEndCol > 0 && rStartCol < rEndCol)
                --rEndCol;

        if ( !bTop && rStartRow < MAXROW && rStartRow < rEndRow )
        {
            bool shrink = true;
            do
            {
                for ( SCCOL i = rStartCol; i<=rEndCol && shrink; i++)
                    if (aCol[i].HasDataAt(rStartRow))
                        shrink = false;
                if (shrink)
                    ++rStartRow;
            }while( shrink && rStartRow < MAXROW && rStartRow < rEndRow);
        }
    }

    if ( !bIncludeOld )
    {
        if ( !bBottom && rEndRow > 0 && rStartRow < rEndRow )
        {
            bool shrink = true;
            do
            {
                for ( SCCOL i = rStartCol; i<=rEndCol && shrink; i++)
                    if (aCol[i].HasDataAt(rEndRow))
                        shrink = false;
                if (shrink)
                    --rEndRow;
            }while( shrink && rEndRow > 0 && rStartRow < rEndRow );
        }
    }
}


bool ScTable::ShrinkToUsedDataArea( bool& o_bShrunk, SCCOL& rStartCol, SCROW& rStartRow,
        SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const
{
    o_bShrunk = false;

    PutInOrder( rStartCol, rEndCol);
    PutInOrder( rStartRow, rEndRow);
    if (rStartCol < 0)
        rStartCol = 0, o_bShrunk = true;
    if (rStartRow < 0)
        rStartRow = 0, o_bShrunk = true;
    if (rEndCol > MAXCOL)
        rEndCol = MAXCOL, o_bShrunk = true;
    if (rEndRow > MAXROW)
        rEndRow = MAXROW, o_bShrunk = true;

    bool bChanged;
    do
    {
        bChanged = false;

        while (rStartCol < rEndCol)
        {
            if (aCol[rEndCol].IsEmptyBlock( rStartRow, rEndRow))
            {
                --rEndCol;
                bChanged = true;
            }
            else
                break;  // while
        }

        while (rStartCol < rEndCol)
        {
            if (aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow))
            {
                ++rStartCol;
                bChanged = true;
            }
            else
                break;  // while
        }

        if (!bColumnsOnly)
        {
            if (rStartRow < rEndRow)
            {
                bool bFound = false;
                for (SCCOL i=rStartCol; i<=rEndCol && !bFound; i++)
                    if (aCol[i].HasDataAt( rStartRow))
                        bFound = true;
                if (!bFound)
                {
                    ++rStartRow;
                    bChanged = true;
                }
            }

            if (rStartRow < rEndRow)
            {
                bool bFound = false;
                for (SCCOL i=rStartCol; i<=rEndCol && !bFound; i++)
                    if (aCol[i].HasDataAt( rEndRow))
                        bFound = true;
                if (!bFound)
                {
                    --rEndRow;
                    bChanged = true;
                }
            }
        }

        if (bChanged)
            o_bShrunk = true;
    } while( bChanged );

    return rStartCol != rEndCol || (bColumnsOnly ?
            !aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow) :
            (rStartRow != rEndRow || aCol[rStartCol].HasDataAt( rStartRow)));
}


SCSIZE ScTable::GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir )
{
    SCSIZE nCount = 0;
    SCCOL nCol;
    if ((eDir == DIR_BOTTOM) || (eDir == DIR_TOP))
    {
        nCount = static_cast<SCSIZE>(nEndRow - nStartRow);
        for (nCol = nStartCol; nCol <= nEndCol; nCol++)
            nCount = Min(nCount, aCol[nCol].GetEmptyLinesInBlock(nStartRow, nEndRow, eDir));
    }
    else if (eDir == DIR_RIGHT)
    {
        nCol = nEndCol;
        while (((SCsCOL)nCol >= (SCsCOL)nStartCol) &&
                 aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
        {
            nCount++;
            nCol--;
        }
    }
    else
    {
        nCol = nStartCol;
        while ((nCol <= nEndCol) && aCol[nCol].IsEmptyBlock(nStartRow, nEndRow))
        {
            nCount++;
            nCol++;
        }
    }
    return nCount;
}

bool ScTable::IsEmptyLine( SCROW nRow, SCCOL nStartCol, SCCOL nEndCol )
{
    bool bFound = false;
    for (SCCOL i=nStartCol; i<=nEndCol && !bFound; i++)
        if (aCol[i].HasDataAt(nRow))
            bFound = true;
    return !bFound;
}

void ScTable::LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow )
{
    while ( rStartCol<rEndCol && aCol[rStartCol].IsEmptyBlock(rStartRow,rEndRow) )
        ++rStartCol;

    while ( rStartCol<rEndCol && aCol[rEndCol].IsEmptyBlock(rStartRow,rEndRow) )
        --rEndCol;

    while ( rStartRow<rEndRow && IsEmptyLine(rStartRow, rStartCol, rEndCol) )
        ++rStartRow;

    while ( rStartRow<rEndRow && IsEmptyLine(rEndRow, rStartCol, rEndCol) )
        --rEndRow;
}

SCCOL ScTable::FindNextVisibleCol( SCCOL nCol, bool bRight ) const
{
    if(bRight)
    {
        nCol++;
        SCCOL nEnd = 0;
        bool bHidden = pDocument->ColHidden(nCol, nTab, NULL, &nEnd);
        if(bHidden)
            nCol = nEnd +1;

        return std::min<SCCOL>(MAXCOL, nCol);
    }
    else
    {
        nCol--;
        SCCOL nStart = MAXCOL;
        bool bHidden = pDocument->ColHidden(nCol, nTab, &nStart, NULL);
        if(bHidden)
            nCol = nStart - 1;

        return std::max<SCCOL>(0, nCol);
    }
}

SCCOL ScTable::FindNextVisibleColWithContent( SCCOL nCol, bool bRight, SCROW nRow ) const
{
    if(bRight)
    {
        if(nCol == MAXCOL)
            return MAXCOL;

        do
        {
            nCol++;
            SCCOL nEndCol = 0;
            bool bHidden = pDocument->ColHidden( nCol, nTab, NULL, &nEndCol );
            if(bHidden)
            {
                nCol = nEndCol +1;
                if(nEndCol >= MAXCOL)
                    return MAXCOL;
            }

            if(aCol[nCol].HasVisibleDataAt(nRow))
                return nCol;
        }
        while(nCol < MAXCOL);

        return MAXCOL;
    }
    else
    {
        if(nCol == 0)
            return 0;

        do
        {
            nCol--;
            SCCOL nStartCol = MAXCOL;
            bool bHidden = pDocument->ColHidden( nCol, nTab, &nStartCol, NULL );
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
    if (eDirection == SC_MOVE_LEFT || eDirection == SC_MOVE_RIGHT)
    {
        SCCOL nNewCol = rCol;
        bool bThere = aCol[nNewCol].HasVisibleDataAt(rRow);
        bool bRight = (eDirection == SC_MOVE_RIGHT);
        if (bThere)
        {
            if(nNewCol >= MAXCOL && eDirection == SC_MOVE_RIGHT)
                return;
            else if(nNewCol == 0 && eDirection == SC_MOVE_LEFT)
                return;

            SCCOL nNextCol = FindNextVisibleCol( nNewCol, bRight );

            if(aCol[nNextCol].HasVisibleDataAt(rRow))
            {
                bool bFound = false;
                nNewCol = nNextCol;
                do
                {
                    nNextCol = FindNextVisibleCol( nNewCol, bRight );
                    if(aCol[nNextCol].HasVisibleDataAt(rRow))
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
        aCol[rCol].FindDataAreaPos(rRow,eDirection == SC_MOVE_DOWN);
    }
}

bool ScTable::ValidNextPos( SCCOL nCol, SCROW nRow, const ScMarkData& rMark,
                                bool bMarked, bool bUnprotected )
{
    if (!ValidCol(nCol) || !ValidRow(nRow))
        return false;

    if (pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED))
        // Skip an overlapped cell.
        return false;

    if (bMarked && !rMark.IsCellMarked(nCol,nRow))
        return false;

    if (bUnprotected && ((const ScProtectionAttr*)
                        GetAttr(nCol,nRow,ATTR_PROTECTION))->GetProtection())
        return false;

    if (bMarked || bUnprotected)        //! auch sonst ???
    {
        //  ausgeblendete muessen uebersprungen werden, weil der Cursor sonst
        //  auf der naechsten Zelle landet, auch wenn die geschuetzt/nicht markiert ist.
        //! per Extra-Parameter steuern, nur fuer Cursor-Bewegung ???

        if (RowHidden(nRow))
            return false;

        if (ColHidden(nCol))
            return false;
    }

    return true;
}

void ScTable::GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark )
{
    if (bUnprotected && !IsProtected())     // Tabelle ueberhaupt geschuetzt?
        bUnprotected = false;

    sal_uInt16 nWrap = 0;
    SCsCOL nCol = rCol;
    SCsROW nRow = rRow;

    nCol = sal::static_int_cast<SCsCOL>( nCol + nMovX );
    nRow = sal::static_int_cast<SCsROW>( nRow + nMovY );

    OSL_ENSURE( !nMovY || !bUnprotected,
                "GetNextPos mit bUnprotected horizontal nicht implementiert" );

    if ( nMovY && bMarked )
    {
        bool bUp = ( nMovY < 0 );
        nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        while ( VALIDROW(nRow) &&
                (RowHidden(nRow) || pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED)) )
        {
            //  ausgeblendete ueberspringen (s.o.)
            nRow += nMovY;
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        }

        while ( nRow < 0 || nRow > MAXROW )
        {
            nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );
            while ( VALIDCOL(nCol) && ColHidden(nCol) )
                nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );   //  skip hidden rows (see above)
            if (nCol < 0)
            {
                nCol = MAXCOL;
                if (++nWrap >= 2)
                    return;
            }
            else if (nCol > MAXCOL)
            {
                nCol = 0;
                if (++nWrap >= 2)
                    return;
            }
            if (nRow < 0)
                nRow = MAXROW;
            else if (nRow > MAXROW)
                nRow = 0;
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );
            while ( VALIDROW(nRow) &&
                    (RowHidden(nRow) || pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED)) )
            {
                //  ausgeblendete ueberspringen (s.o.)
                nRow += nMovY;
                nRow = rMark.GetNextMarked( nCol, nRow, bUp );
            }
        }
    }

    if ( nMovX && ( bMarked || bUnprotected ) )
    {
        // initiales Weiterzaehlen wrappen:
        if (nCol<0)
        {
            nCol = MAXCOL;
            --nRow;
            if (nRow<0)
                nRow = MAXROW;
        }
        if (nCol>MAXCOL)
        {
            nCol = 0;
            ++nRow;
            if (nRow>MAXROW)
                nRow = 0;
        }

        if ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) )
        {
            SCsROW* pNextRows = new SCsROW[MAXCOL+1];
            SCCOL i;

            if ( nMovX > 0 )                            //  vorwaerts
            {
                for (i=0; i<=MAXCOL; i++)
                    pNextRows[i] = (i<nCol) ? (nRow+1) : nRow;
                do
                {
                    SCsROW nNextRow = pNextRows[nCol] + 1;
                    if ( bMarked )
                        nNextRow = rMark.GetNextMarked( nCol, nNextRow, false );
                    if ( bUnprotected )
                        nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, false );
                    pNextRows[nCol] = nNextRow;

                    SCsROW nMinRow = MAXROW+1;
                    for (i=0; i<=MAXCOL; i++)
                        if (pNextRows[i] < nMinRow)     // bei gleichen den linken
                        {
                            nMinRow = pNextRows[i];
                            nCol = i;
                        }
                    nRow = nMinRow;

                    if ( nRow > MAXROW )
                    {
                        if (++nWrap >= 2) break;        // ungueltigen Wert behalten
                        nCol = 0;
                        nRow = 0;
                        for (i=0; i<=MAXCOL; i++)
                            pNextRows[i] = 0;           // alles ganz von vorne
                    }
                }
                while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
            }
            else                                        //  rueckwaerts
            {
                for (i=0; i<=MAXCOL; i++)
                    pNextRows[i] = (i>nCol) ? (nRow-1) : nRow;
                do
                {
                    SCsROW nNextRow = pNextRows[nCol] - 1;
                    if ( bMarked )
                        nNextRow = rMark.GetNextMarked( nCol, nNextRow, true );
                    if ( bUnprotected )
                        nNextRow = aCol[nCol].GetNextUnprotected( nNextRow, true );
                    pNextRows[nCol] = nNextRow;

                    SCsROW nMaxRow = -1;
                    for (i=0; i<=MAXCOL; i++)
                        if (pNextRows[i] >= nMaxRow)    // bei gleichen den rechten
                        {
                            nMaxRow = pNextRows[i];
                            nCol = i;
                        }
                    nRow = nMaxRow;

                    if ( nRow < 0 )
                    {
                        if (++nWrap >= 2) break;        // ungueltigen Wert behalten
                        nCol = MAXCOL;
                        nRow = MAXROW;
                        for (i=0; i<=MAXCOL; i++)
                            pNextRows[i] = MAXROW;      // alles ganz von vorne
                    }
                }
                while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
            }

            delete[] pNextRows;
        }
    }

    //  ungueltige Werte kommen z.b. bei Tab heraus,
    //  wenn nicht markiert und nicht geschuetzt ist (linker / rechter Rand),
    //  dann Werte unveraendert lassen

    if (VALIDCOLROW(nCol,nRow))
    {
        rCol = nCol;
        rRow = nRow;
    }
}

bool ScTable::GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, const ScMarkData& rMark )
{
    const ScMarkArray* pMarkArray = rMark.GetArray();
    OSL_ENSURE(pMarkArray,"GetNextMarkedCell ohne MarkArray");
    if ( !pMarkArray )
        return false;

    ++rRow;                 // naechste Zelle ist gesucht

    while ( rCol <= MAXCOL )
    {
        const ScMarkArray& rArray = pMarkArray[rCol];
        while ( rRow <= MAXROW )
        {
            SCROW nStart = (SCROW) rArray.GetNextMarked( (SCsROW) rRow, false );
            if ( nStart <= MAXROW )
            {
                SCROW nEnd = rArray.GetMarkEnd( nStart, false );
                ScColumnIterator aColIter( &aCol[rCol], nStart, nEnd );
                SCROW nCellRow;
                ScBaseCell* pCell = NULL;
                while ( aColIter.Next( nCellRow, pCell ) )
                {
                    if ( pCell && pCell->GetCellType() != CELLTYPE_NOTE )
                    {
                        rRow = nCellRow;
                        return true;            // Zelle gefunden
                    }
                }
                rRow = nEnd + 1;                // naechsten markierten Bereich suchen
            }
            else
                rRow = MAXROW + 1;              // Ende der Spalte
        }
        rRow = 0;
        ++rCol;                                 // naechste Spalte testen
    }

    return false;                               // alle Spalten durch
}

void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bUpdateNoteCaptionPos )
{
    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       // only within the table
    {
        InitializeNoteCaptions();
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

void ScTable::UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2, SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                     ScDocument* pUndoDoc, bool bIncludeDraw, bool bUpdateNoteCaptionPos )
{
    bool bUpdated = false;
    SCCOL i;
    SCCOL iMax;
    if ( eUpdateRefMode == URM_COPY )
    {
        i = nCol1;
        iMax = nCol2;
    }
    else
    {
        i = 0;
        iMax = MAXCOL;
    }

    // Named expressions need to be updated before formulas acessing them.
    if (mpRangeName)
    {
        ScRange aRange( nCol1, nRow1, nTab1, nCol2, nRow2, nTab2 );;
        mpRangeName->UpdateReference( eUpdateRefMode, aRange, nDx, nDy, nDz, true );
    }

    for ( ; i<=iMax; i++)
        bUpdated |= aCol[i].UpdateReference(
            eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz, pUndoDoc );

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

        for ( ScRangeVec::iterator aIt = aPrintRanges.begin(), aEnd = aPrintRanges.end(); aIt != aEnd; ++aIt )
        {
            nSCol = aIt->aStart.Col();
            nSRow = aIt->aStart.Row();
            nECol = aIt->aEnd.Col();
            nERow = aIt->aEnd.Row();

            // do not try to modify sheet index of print range
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *aIt = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
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
                nRepeatStartX = nSCol;  // fuer UpdatePageBreaks
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
                nRepeatStartY = nSRow;  // fuer UpdatePageBreaks
                nRepeatEndY = nERow;
            }
        }

        //  updating print ranges is not necessary with multiple print ranges
        if ( bRecalcPages && GetPrintRangeCount() <= 1 )
        {
            UpdatePageBreaks(NULL);

            pDocument->RepaintRange( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab) );
        }
    }

    if (bUpdated && IsStreamValid())
        SetStreamValid(false);

    if(mpCondFormatList)
        mpCondFormatList->UpdateReference( eUpdateRefMode, ScRange(nCol1, nRow1, nTab1, nCol2, nRow2, nTab2), nDx, nDy, nDz);
}

void ScTable::UpdateTranspose( const ScRange& rSource, const ScAddress& rDest,
                                    ScDocument* pUndoDoc )
{
    for ( SCCOL i=0; i<=MAXCOL; i++ )
        aCol[i].UpdateTranspose( rSource, rDest, pUndoDoc );
}

void ScTable::UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY )
{
    for ( SCCOL i=0; i<=MAXCOL; i++ )
        aCol[i].UpdateGrow( rArea, nGrowX, nGrowY );
}

void ScTable::UpdateInsertTab(SCTAB nTable, SCTAB nNewSheets)
{
    if (nTab >= nTable)
    {
        nTab += nNewSheets;
        if (pDBDataNoName)
            pDBDataNoName->UpdateMoveTab(nTab - 1 ,nTab);
    }
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTab(nTable, nNewSheets);

    if (mpRangeName)
        mpRangeName->UpdateTabRef( nTable, 1, 0, nNewSheets);

    if (mpRangeName)
        mpRangeName->UpdateTabRef( nTable, 1);

    if (IsStreamValid())
        SetStreamValid(false);

    if(mpCondFormatList)
        mpCondFormatList->UpdateReference( URM_INSDEL, ScRange(0,0, nTable, MAXCOL, MAXROW, nTable+nNewSheets-1),0,0, nNewSheets);
}

void ScTable::UpdateDeleteTab( SCTAB nTable, bool bIsMove, ScTable* pRefUndo, SCTAB nSheets )
{
    if (nTab > nTable)
    {
        nTab -= nSheets;
        if (pDBDataNoName)
            pDBDataNoName->UpdateMoveTab(nTab + 1,nTab);
    }

    SCCOL i;
    if (pRefUndo)
        for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, &pRefUndo->aCol[i], nSheets);
    else
        for (i=0; i <= MAXCOL; i++) aCol[i].UpdateDeleteTab(nTable, bIsMove, NULL, nSheets);

    if (mpRangeName)
    {
        for (SCTAB aTab = 0; aTab < nSheets; ++aTab)
        {
            mpRangeName->UpdateTabRef( nTable + aTab, 2 );
        }
    }

    if (mpRangeName)
    {
        mpRangeName->UpdateTabRef( nTable, 2 );
    }

    if (IsStreamValid())
        SetStreamValid(false);

    if(mpCondFormatList)
        mpCondFormatList->UpdateReference( URM_INSDEL, ScRange(0,0, nTable, MAXCOL, MAXROW, nTable+nSheets-1),0,0, -1*nSheets);
}

void ScTable::UpdateMoveTab( SCTAB nOldPos, SCTAB nNewPos, SCTAB nTabNo,
        ScProgress* pProgress )
{
    nTab = nTabNo;
    for ( SCCOL i=0; i <= MAXCOL; i++ )
    {
        aCol[i].UpdateMoveTab( nOldPos, nNewPos, nTabNo );
        if (pProgress)
            pProgress->SetState(pProgress->GetState() + aCol[i].GetCodeCount());
    }

    if (mpRangeName)
        mpRangeName->UpdateTabRef(nOldPos, 3, nNewPos);

    if (IsStreamValid())
        SetStreamValid(false);
   if (pDBDataNoName)
        pDBDataNoName->UpdateMoveTab(nOldPos, nNewPos);

    if(mpCondFormatList)
        mpCondFormatList->UpdateMoveTab(nOldPos, nNewPos);
}

void ScTable::UpdateCompile( bool bForceIfNameInUse )
{
    for (SCCOL i=0; i <= MAXCOL; i++)
    {
        aCol[i].UpdateCompile( bForceIfNameInUse );
    }
}

void ScTable::SetTabNo(SCTAB nNewTab)
{
    nTab = nNewTab;
    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].SetTabNo(nNewTab);
}

void ScTable::FindRangeNamesInUse(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                               std::set<sal_uInt16>& rIndexes) const
{
    for (SCCOL i = nCol1; i <= nCol2 && ValidCol(i); i++)
        aCol[i].FindRangeNamesInUse(nRow1, nRow2, rIndexes);
}

void ScTable::ExtendPrintArea( OutputDevice* pDev,
                    SCCOL /* nStartCol */, SCROW nStartRow, SCCOL& rEndCol, SCROW nEndRow )
{
    if ( !pColFlags || !pRowFlags )
    {
        OSL_FAIL("keine ColInfo oder RowInfo in ExtendPrintArea");
        return;
    }

    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    // First, mark those columns that we need to skip i.e. hidden and empty columns.

    ScFlatBoolColSegments aSkipCols;
    aSkipCols.setFalse(0, MAXCOL);
    for (SCCOL i = 0; i <= MAXCOL; ++i)
    {
        SCCOL nLastCol = i;
        if (ColHidden(i, NULL, &nLastCol))
        {
            // Columns are hidden in this range.
            aSkipCols.setTrue(i, nLastCol);
        }
        else
        {
            // These columns are visible.  Check for empty columns.
            for (SCCOL j = i; j <= nLastCol; ++j)
            {
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
            ScBaseCell* pCell = aIter.reset(nStartRow);
            if (!pCell)
                // No visible cells found in this column.  Skip it.
                continue;

            while (pCell)
            {
                SCCOL nNewCol = nDataCol;
                SCROW nRow = aIter.getRow();
                if (nRow > nEndRow)
                    // Went past the last row position.  Bail out.
                    break;

                MaybeAddExtraColumn(nNewCol, nRow, pDev, nPPTX, nPPTY);
                if (nNewCol > nPrintCol)
                    nPrintCol = nNewCol;
                pCell = aIter.next();
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
    ScBaseCell* pCell = aCol[rCol].GetCell(nRow);
    if (!pCell || !pCell->HasStringData())
        return;

    bool bFormula = false;  //! ueberge
    long nPixel = pCell->GetTextWidth();

    // Breite bereits im Idle-Handler berechnet?
    if ( TEXTWIDTH_DIRTY == nPixel )
    {
        ScNeededSizeOptions aOptions;
        aOptions.bTotalSize  = true;
        aOptions.bFormula    = bFormula;
        aOptions.bSkipMerged = false;

        Fraction aZoom(1,1);
        nPixel = aCol[rCol].GetNeededSize(
            nRow, pDev, nPPTX, nPPTY, aZoom, aZoom, true, aOptions );
        pCell->SetTextWidth( (sal_uInt16)nPixel );
    }

    long nTwips = (long) (nPixel / nPPTX);
    long nDocW = GetColWidth( rCol );

    long nMissing = nTwips - nDocW;
    if ( nMissing > 0 )
    {
        //  look at alignment

        const ScPatternAttr* pPattern = GetPattern( rCol, nRow );
        const SfxItemSet* pCondSet = NULL;
        if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
            pCondSet = pDocument->GetCondResult( rCol, nRow, nTab );

        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                        pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet )).GetValue();
        if ( eHorJust == SVX_HOR_JUSTIFY_CENTER )
            nMissing /= 2;                          // distributed into both directions
        else
        {
            // STANDARD is LEFT (only text is handled here)
            bool bRight = ( eHorJust == SVX_HOR_JUSTIFY_RIGHT );
            if ( IsLayoutRTL() )
                bRight = !bRight;
            if ( bRight )
                nMissing = 0;       // extended only to the left (logical)
        }
    }

    SCCOL nNewCol = rCol;
    while (nMissing > 0 && nNewCol < MAXCOL)
    {
        ScBaseCell* pNextCell = aCol[nNewCol+1].GetCell(nRow);
        if (pNextCell && pNextCell->GetCellType() != CELLTYPE_NOTE)
            // Cell content in a next column ends display of this string.
            nMissing = 0;
        else
            nMissing -= GetColWidth(++nNewCol);
    }
    rCol = nNewCol;
}

namespace {

class SetTableIndex : public ::std::unary_function<ScRange, void>
{
    SCTAB mnTab;
public:
    SetTableIndex(SCTAB nTab) : mnTab(nTab) {}

    void operator() (ScRange& rRange) const
    {
        rRange.aStart.SetTab(mnTab);
        rRange.aEnd.SetTab(mnTab);
    }
};

void setPrintRange(ScRange*& pRange1, const ScRange* pRange2)
{
    if (pRange2)
    {
        if (pRange1)
            *pRange1 = *pRange2;
        else
            pRange1 = new ScRange(*pRange2);
    }
    else
        DELETEZ(pRange1);
}

}

void ScTable::CopyPrintRange(const ScTable& rTable)
{
    // The table index shouldn't be used when the print range is used, but
    // just in case set the correct table index.

    aPrintRanges = rTable.aPrintRanges;
    ::std::for_each(aPrintRanges.begin(), aPrintRanges.end(), SetTableIndex(nTab));

    bPrintEntireSheet = rTable.bPrintEntireSheet;

    delete pRepeatColRange;
    pRepeatColRange = NULL;
    if (rTable.pRepeatColRange)
    {
        pRepeatColRange = new ScRange(*rTable.pRepeatColRange);
        pRepeatColRange->aStart.SetTab(nTab);
        pRepeatColRange->aEnd.SetTab(nTab);
    }

    delete pRepeatRowRange;
    pRepeatRowRange = NULL;
    if (rTable.pRepeatRowRange)
    {
        pRepeatRowRange = new ScRange(*rTable.pRepeatRowRange);
        pRepeatRowRange->aStart.SetTab(nTab);
        pRepeatRowRange->aEnd.SetTab(nTab);
    }
}

void ScTable::DoColResize( SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd )
{
    for (SCCOL nCol=nCol1; nCol<=nCol2; nCol++)
        aCol[nCol].Resize(aCol[nCol].GetCellCount() + nAdd);
}

void ScTable::SetRepeatColRange( const ScRange* pNew )
{
    setPrintRange( pRepeatColRange, pNew );

    if (IsStreamValid())
        SetStreamValid(false);

    InvalidatePageBreaks();
}

void ScTable::SetRepeatRowRange( const ScRange* pNew )
{
    setPrintRange( pRepeatRowRange, pNew );

    if (IsStreamValid())
        SetStreamValid(false);

    InvalidatePageBreaks();
}

void ScTable::ClearPrintRanges()
{
    aPrintRanges.clear();
    bPrintEntireSheet = false;

    if (IsStreamValid())
        SetStreamValid(false);

    InvalidatePageBreaks();
}

void ScTable::AddPrintRange( const ScRange& rNew )
{
    bPrintEntireSheet = false;
    if( aPrintRanges.size() < 0xFFFF )
        aPrintRanges.push_back( rNew );

    if (IsStreamValid())
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
    return (nPos < GetPrintRangeCount()) ? &aPrintRanges[ nPos ] : NULL;
}

void ScTable::FillPrintSaver( ScPrintSaverTab& rSaveTab ) const
{
    rSaveTab.SetAreas( aPrintRanges, bPrintEntireSheet );
    rSaveTab.SetRepeat( pRepeatColRange, pRepeatRowRange );
}

void ScTable::RestorePrintRanges( const ScPrintSaverTab& rSaveTab )
{
    aPrintRanges = rSaveTab.GetPrintRanges();
    bPrintEntireSheet = rSaveTab.IsEntireSheet();
    SetRepeatColRange( rSaveTab.GetRepeatCol() );
    SetRepeatRowRange( rSaveTab.GetRepeatRow() );

    InvalidatePageBreaks();     // #i117952# forget page breaks for an old print range
    UpdatePageBreaks(NULL);
}

SCROW ScTable::VisibleDataCellIterator::ROW_NOT_FOUND = -1;

ScTable::VisibleDataCellIterator::VisibleDataCellIterator(ScFlatBoolRowSegments& rRowSegs, ScColumn& rColumn) :
    mrRowSegs(rRowSegs),
    mrColumn(rColumn),
    mpCell(NULL),
    mnCurRow(ROW_NOT_FOUND),
    mnUBound(ROW_NOT_FOUND)
{
}

ScTable::VisibleDataCellIterator::~VisibleDataCellIterator()
{
}

ScBaseCell* ScTable::VisibleDataCellIterator::reset(SCROW nRow)
{
    if (nRow > MAXROW)
    {
        mnCurRow = ROW_NOT_FOUND;
        return NULL;
    }

    ScFlatBoolRowSegments::RangeData aData;
    if (!mrRowSegs.getRangeData(nRow, aData))
    {
        mnCurRow = ROW_NOT_FOUND;
        return NULL;
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
            return NULL;
        }
    }

    mpCell = mrColumn.GetCell(mnCurRow);
    if (mpCell)
        // First visible cell found.
        return mpCell;

    // Find a first visible cell below this row (if any).
    return next();
}

ScBaseCell* ScTable::VisibleDataCellIterator::next()
{
    if (mnCurRow == ROW_NOT_FOUND)
        return NULL;

    while (mrColumn.GetNextDataPos(mnCurRow))
    {
        if (mnCurRow > mnUBound)
        {
            // We don't know the visibility of this row range.  Query it.
            ScFlatBoolRowSegments::RangeData aData;
            if (!mrRowSegs.getRangeData(mnCurRow, aData))
            {
                mnCurRow = ROW_NOT_FOUND;
                return NULL;
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

        mpCell = mrColumn.GetCell(mnCurRow);
        if (mpCell)
            return mpCell;
    }
    mnCurRow = ROW_NOT_FOUND;
    return NULL;
}

SCROW ScTable::VisibleDataCellIterator::getRow() const
{
    return mnCurRow;
}

void ScTable::SetAnonymousDBData(ScDBData* pDBData)
{
    delete pDBDataNoName;
    pDBDataNoName = pDBData;
}

ScDBData* ScTable::GetAnonymousDBData()
{
    return pDBDataNoName;
}

sal_uLong ScTable::AddCondFormat( ScConditionalFormat* pNew )
{
    if(!mpCondFormatList)
        mpCondFormatList.reset(new ScConditionalFormatList());

    sal_uLong nMax = 0;
    for(ScConditionalFormatList::const_iterator itr = mpCondFormatList->begin();
            itr != mpCondFormatList->end(); ++itr)
    {
        sal_uLong nKey = itr->GetKey();
        if(nKey > nMax)
            nMax = nKey;
    }

    pNew->SetKey(nMax+1);
    mpCondFormatList->InsertNew(pNew);

    return nMax + 1;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
