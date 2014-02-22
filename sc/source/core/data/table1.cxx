/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "scitems.hxx"
#include <svx/algitem.hxx>
#include <editeng/justifyitem.hxx>
#include <unotools/textsearch.hxx>
#include <sfx2/objsh.hxx>

#include "attrib.hxx"
#include "patattr.hxx"
#include "formulacell.hxx"
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
#include "hints.hxx"
#include "prnsave.hxx"
#include "tabprotection.hxx"
#include "sheetevents.hxx"
#include "segmenttree.hxx"
#include "dbdata.hxx"
#include "colorscale.hxx"
#include "conditio.hxx"
#include "globalnames.hxx"
#include "cellvalue.hxx"
#include "scmatrix.hxx"
#include "refupdatecontext.hxx"
#include <rowheightcontext.hxx>

#include "formula/vectortoken.hxx"

#include <vector>

using ::std::vector;

namespace {

ScProgress* GetProgressBar(
    SCSIZE nCount, SCSIZE nTotalCount, ScProgress* pOuterProgress, ScDocument* pDoc)
{
    if (nTotalCount < 1000)
    {
        
        
        
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
    sc::RowHeightContext& rCxt, ScColumn* pCol, SCROW nStartRow, SCROW nEndRow,
    vector<sal_uInt16>& aHeights, ScProgress* pProgress, sal_uInt32 nProgressStart )
{
    SCSIZE nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    
    
    

    pCol[MAXCOL].GetOptimalHeight(rCxt, nStartRow, nEndRow, &aHeights[0], 0, 0);

    

    sal_uInt16 nMinHeight = aHeights[nCount-1];
    SCSIZE nPos = nCount-1;
    while ( nPos && aHeights[nPos-1] >= nMinHeight )
        --nPos;
    SCROW nMinStart = nStartRow + nPos;

    sal_uLong nWeightedCount = 0;
    for (SCCOL nCol=0; nCol<MAXCOL; nCol++)     
    {
        pCol[nCol].GetOptimalHeight(rCxt, nStartRow, nEndRow, &aHeights[0], nMinHeight, nMinStart);

        if (pProgress)
        {
            sal_uLong nWeight = pCol[nCol].GetWeightedCount();
            if (nWeight)        
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

bool SetOptimalHeightsToRows(OptimalHeightsFuncObjBase& rFuncObj,
    ScBitMaskCompressedArray<SCROW, sal_uInt8>* pRowFlags, SCROW nStartRow, SCROW nEndRow, sal_uInt16 nExtra,
    const vector<sal_uInt16>& aHeights, bool bForce)
{
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
        SCSIZE nMoreRows = nRegionEndRow - ( nStartRow+i );     

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
        i += nMoreRows;     
    }
    if (nLast)
        bChanged |= rFuncObj(nRngStart, nRngEnd, nLast);

    return bChanged;
}

}



ScTable::ScTable( ScDocument* pDoc, SCTAB nNewTab, const OUString& rNewName,
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
    mbForceBreaks(false)
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
        
        
        bLayoutRTL = ScGlobal::IsSystemRTL();
    }

    ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
    if (pDrawLayer)
    {
        if ( pDrawLayer->ScAddPage( nTab ) )    
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

    for (SCCOL k=0; k<=MAXCOL; k++)
        aCol[k].PrepareBroadcastersForDestruction();
}

void ScTable::GetName( OUString& rName ) const
{
    rName = aName;
}

void ScTable::SetName( const OUString& rNewName )
{
    aName = rNewName;
    aUpperName = OUString(); 

    
}

const OUString& ScTable::GetUpperName() const
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
                        const OUString& rDoc, const OUString& rFlt, const OUString& rOpt,
                        const OUString& rTab, sal_uLong nRefreshDelay )
{
    nLinkMode = nMode;
    aLinkDoc = rDoc;        
    aLinkFlt = rFlt;        
    aLinkOpt = rOpt;        
    aLinkTab = rTab;        
    nLinkRefreshDelay = nRefreshDelay;  

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
    aOptions.bSkipMerged = false;       
    aOptions.bTotalSize  = bTotalSize;

    return aCol[nCol].GetNeededSize
        ( nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, bWidth, aOptions );
}

bool ScTable::SetOptimalHeight(
    sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow,
    ScProgress* pOuterProgress, sal_uLong nProgressStart )
{
    OSL_ENSURE( rCxt.getExtraHeight() == 0 || rCxt.isForceAutoSize(),
        "automatic OptimalHeight with Extra" );

    if ( !pDocument->IsAdjustHeightEnabled() )
    {
        return false;
    }

    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = GetProgressBar(nCount, GetWeightedCount(), pOuterProgress, pDocument);

    vector<sal_uInt16> aHeights(nCount, 0);

    GetOptimalHeightsInColumn(rCxt, aCol, nStartRow, nEndRow, aHeights, pProgress, nProgressStart);

    SetRowHeightRangeFunc aFunc(this, rCxt.getPPTX(), rCxt.getPPTY());
    bool bChanged = SetOptimalHeightsToRows(
        aFunc, pRowFlags, nStartRow, nEndRow, rCxt.getExtraHeight(), aHeights, rCxt.isForceAutoSize());

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

    if ( !pDocument->IsAdjustHeightEnabled() )
        return;

    SCSIZE  nCount = static_cast<SCSIZE>(nEndRow-nStartRow+1);

    ScProgress* pProgress = GetProgressBar(nCount, GetWeightedCount(), pOuterProgress, pDocument);

    vector<sal_uInt16> aHeights(nCount, 0);

    GetOptimalHeightsInColumn(rCxt, aCol, nStartRow, nEndRow, aHeights, pProgress, nProgressStart);

    SetRowHeightOnlyFunc aFunc(this);
    SetOptimalHeightsToRows(
        aFunc, pRowFlags, nStartRow, nEndRow, rCxt.getExtraHeight(), aHeights, rCxt.isForceAutoSize());

    if ( pProgress != pOuterProgress )
        delete pProgress;
}

bool ScTable::GetCellArea( SCCOL& rEndCol, SCROW& rEndRow ) const
{
    bool bFound = false;
    SCCOL nMaxX = 0;
    SCROW nMaxY = 0;
    for (SCCOL i=0; i<=MAXCOL; i++)
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
    bool bRet = true;               
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

    for (i=0; i<=MAXCOL; i++)               
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
            if (bNotes)
            {
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
        }

    SCCOL nMaxDataX = nMaxX;

    for (i=0; i<=MAXCOL; i++)               
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

    if (nMaxX == MAXCOL)                    
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
                
                nMaxX = nAttrStartX - 1;

                
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

    for (i=0; i<=MAXCOL; i++)               
    {
        if (aCol[i].HasVisibleAttrIn( nStartRow, nEndRow ))
        {
            bFound = true;
            nMaxX = i;
        }
    }

    if (nMaxX == MAXCOL)                    
    {
        --nMaxX;
        while ( nMaxX>0 && aCol[nMaxX].IsVisibleAttrEqual(aCol[nMaxX+1], nStartRow, nEndRow) )
            --nMaxX;
    }

    for (i=0; i<=MAXCOL; i++)               
    {
        if (!aCol[i].IsEmptyBlock( nStartRow, nEndRow ))        
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

    for (i=nStartCol; i<=nEndCol; i++)              
    {
        SCROW nLastRow;
        if (aCol[i].GetLastVisibleAttr( nLastRow ))
        {
            bFound = true;
            if (nLastRow > nMaxY)
                nMaxY = nLastRow;
        }
    }

    for (i=nStartCol; i<=nEndCol; i++)              
    {
        if (!aCol[i].IsEmptyData())
        {
            bFound = true;
            SCROW nColY = aCol[i].GetLastDataPos();
            if (nColY > nMaxY)
                nMaxY = nColY;
        }
        if (bNotes)
        {
            if ( aCol[i].HasCellNotes() )
            {
                SCROW maxNoteRow =aCol[i].GetCellNotesMaxRow();
                if (maxNoteRow > nMaxY)
                {
                    bFound = true;
                    nMaxY = maxNoteRow;
                }
            }
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

    for (i=0; i<=MAXCOL; i++)                   
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

    if (nMinX == 0)                                     
    {
        if ( aCol[0].IsVisibleAttrEqual(aCol[1]) )      
        {
            ++nMinX;
            while ( nMinX<MAXCOL && aCol[nMinX].IsVisibleAttrEqual(aCol[nMinX-1]) )
                ++nMinX;
        }
    }

    bool bDatFound = false;
    for (i=0; i<=MAXCOL; i++)                   
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
    
    
    
    //
    
    
    

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
                break;  
        }

        while (rStartCol < rEndCol)
        {
            if (aCol[rStartCol].IsEmptyBlock( rStartRow, rEndRow))
            {
                ++rStartCol;
                bChanged = true;
            }
            else
                break;  
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
                SCROW nLastDataRow = GetLastDataRow( rStartCol, rEndCol, rEndRow);
                if (0 <= nLastDataRow && nLastDataRow < rEndRow)
                {
                    rEndRow = std::max( rStartRow, nLastDataRow);
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

SCROW ScTable::GetLastDataRow( SCCOL nCol1, SCCOL nCol2, SCROW nLastRow ) const
{
    if (!ValidCol(nCol1) || !ValidCol(nCol2))
        return -1;

    SCROW nNewLastRow = 0;
    for (SCCOL i = nCol1; i <= nCol2; ++i)
    {
        SCROW nThis = aCol[i].GetLastDataPos(nLastRow);
        if (nNewLastRow < nThis)
            nNewLastRow = nThis;
    }

    return nNewLastRow;
}

SCSIZE ScTable::GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, ScDirection eDir ) const
{
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

bool ScTable::IsEmptyLine( SCROW nRow, SCCOL nStartCol, SCCOL nEndCol ) const
{
    bool bFound = false;
    for (SCCOL i=nStartCol; i<=nEndCol && !bFound; i++)
        if (aCol[i].HasDataAt(nRow))
            bFound = true;
    return !bFound;
}

void ScTable::LimitChartArea( SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow ) const
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
                                bool bMarked, bool bUnprotected ) const
{
    if (!ValidCol(nCol) || !ValidRow(nRow))
        return false;

    if (pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED))
        
        return false;

    if (bMarked && !rMark.IsCellMarked(nCol,nRow))
        return false;

    if (bUnprotected && ((const ScProtectionAttr*)
                        GetAttr(nCol,nRow,ATTR_PROTECTION))->GetProtection())
        return false;

    if (bMarked || bUnprotected)        
    {
        
        
        

        if (RowHidden(nRow))
            return false;

        if (ColHidden(nCol))
            return false;
    }

    return true;
}

void ScTable::GetNextPos( SCCOL& rCol, SCROW& rRow, SCsCOL nMovX, SCsROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark ) const
{
    if (bUnprotected && !IsProtected())     
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
        while ( ValidRow(nRow) &&
                (RowHidden(nRow) || pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED)) )
        {
            
            nRow += nMovY;
            nRow = rMark.GetNextMarked( nCol, nRow, bUp );
        }

        while ( nRow < 0 || nRow > MAXROW )
        {
            nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );
            while ( ValidCol(nCol) && ColHidden(nCol) )
                nCol = sal::static_int_cast<SCsCOL>( nCol + static_cast<SCsCOL>(nMovY) );   
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
            while ( ValidRow(nRow) &&
                    (RowHidden(nRow) || pDocument->HasAttrib(nCol, nRow, nTab, nCol, nRow, nTab, HASATTR_OVERLAPPED)) )
            {
                
                nRow += nMovY;
                nRow = rMark.GetNextMarked( nCol, nRow, bUp );
            }
        }
    }

    if ( nMovX && ( bMarked || bUnprotected ) )
    {
        
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

            if ( nMovX > 0 )                            
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
                        if (pNextRows[i] < nMinRow)     
                        {
                            nMinRow = pNextRows[i];
                            nCol = i;
                        }
                    nRow = nMinRow;

                    if ( nRow > MAXROW )
                    {
                        if (++nWrap >= 2) break;        
                        nCol = 0;
                        nRow = 0;
                        for (i=0; i<=MAXCOL; i++)
                            pNextRows[i] = 0;           
                    }
                }
                while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
            }
            else                                        
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
                        if (pNextRows[i] >= nMaxRow)    
                        {
                            nMaxRow = pNextRows[i];
                            nCol = i;
                        }
                    nRow = nMaxRow;

                    if ( nRow < 0 )
                    {
                        if (++nWrap >= 2) break;        
                        nCol = MAXCOL;
                        nRow = MAXROW;
                        for (i=0; i<=MAXCOL; i++)
                            pNextRows[i] = MAXROW;      
                    }
                }
                while ( !ValidNextPos(nCol, nRow, rMark, bMarked, bUnprotected) );
            }

            delete[] pNextRows;
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
    const ScMarkArray* pMarkArray = rMark.GetArray();
    OSL_ENSURE(pMarkArray,"GetNextMarkedCell without MarkArray");
    if ( !pMarkArray )
        return false;

    ++rRow;                 

    while ( rCol <= MAXCOL )
    {
        const ScMarkArray& rArray = pMarkArray[rCol];
        while ( rRow <= MAXROW )
        {
            SCROW nStart = (SCROW) rArray.GetNextMarked( (SCsROW) rRow, false );
            if ( nStart <= MAXROW )
            {
                SCROW nEnd = rArray.GetMarkEnd( nStart, false );

                const sc::CellStoreType& rCells = aCol[rCol].maCells;
                std::pair<sc::CellStoreType::const_iterator,size_t> aPos = rCells.position(nStart);
                sc::CellStoreType::const_iterator it = aPos.first;
                SCROW nTestRow = nStart;
                if (it->type == sc::element_type_empty)
                {
                    
                    nTestRow += it->size - aPos.second;
                    ++it;
                    if (it == rCells.end())
                    {
                        
                        rRow = MAXROW + 1;
                        return false;
                    }
                }

                if (nTestRow <= nEnd)
                {
                    
                    rRow = nTestRow;
                    return true;
                }

                rRow = nEnd + 1;                
            }
            else
                rRow = MAXROW + 1;              
        }
        rRow = 0;
        ++rCol;                                 
    }

    return false;                               
}

void ScTable::UpdateDrawRef( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz, bool bUpdateNoteCaptionPos )
{
    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       
    {
        ScDrawLayer* pDrawLayer = pDocument->GetDrawLayer();
        if ( eUpdateRefMode != URM_COPY && pDrawLayer )
        {
            if ( eUpdateRefMode == URM_MOVE )
            {                                               
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

    
    if (mpRangeName)
        mpRangeName->UpdateReference(rCxt, nTab);

    for ( ; i<=iMax; i++)
        bUpdated |= aCol[i].UpdateReference(rCxt, pUndoDoc);

    if ( bIncludeDraw )
        UpdateDrawRef( eUpdateRefMode, nCol1, nRow1, nTab1, nCol2, nRow2, nTab2, nDx, nDy, nDz, bUpdateNoteCaptionPos );

    if ( nTab >= nTab1 && nTab <= nTab2 && nDz == 0 )       
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

            
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatColRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = true;
                nRepeatStartX = nSCol;  
                nRepeatEndX = nECol;
            }
        }

        if ( pRepeatRowRange )
        {
            nSCol = pRepeatRowRange->aStart.Col();
            nSRow = pRepeatRowRange->aStart.Row();
            nECol = pRepeatRowRange->aEnd.Col();
            nERow = pRepeatRowRange->aEnd.Row();

            
            if ( ScRefUpdate::Update( pDocument, eUpdateRefMode,
                                      nCol1,nRow1,nTab, nCol2,nRow2,nTab,
                                      nDx,nDy,0,
                                      nSCol,nSRow,nSTab, nECol,nERow,nETab ) )
            {
                *pRepeatRowRange = ScRange( nSCol, nSRow, 0, nECol, nERow, 0 );
                bRecalcPages = true;
                nRepeatStartY = nSRow;  
                nRepeatEndY = nERow;
            }
        }

        
        if ( bRecalcPages && GetPrintRangeCount() <= 1 )
        {
            UpdatePageBreaks(NULL);

            pDocument->RepaintRange( ScRange(0,0,nTab,MAXCOL,MAXROW,nTab) );
        }
    }

    if (bUpdated && IsStreamValid())
        SetStreamValid(false);

    if(mpCondFormatList)
        mpCondFormatList->UpdateReference(rCxt);
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

void ScTable::UpdateInsertTab( sc::RefUpdateInsertTabContext& rCxt )
{
    if (nTab >= rCxt.mnInsertPos)
    {
        nTab += rCxt.mnSheets;
        if (pDBDataNoName)
            pDBDataNoName->UpdateMoveTab(nTab - 1 ,nTab);
    }

    if (mpRangeName)
        mpRangeName->UpdateInsertTab(rCxt, nTab);

    if (mpCondFormatList)
        mpCondFormatList->UpdateInsertTab(rCxt);

    for (SCCOL i=0; i <= MAXCOL; i++) aCol[i].UpdateInsertTab(rCxt);

    if (IsStreamValid())
        SetStreamValid(false);
}

void ScTable::UpdateDeleteTab( sc::RefUpdateDeleteTabContext& rCxt )
{
    if (nTab > rCxt.mnDeletePos)
    {
        nTab -= rCxt.mnSheets;
        if (pDBDataNoName)
            pDBDataNoName->UpdateMoveTab(nTab + 1,nTab);
    }

    if (mpRangeName)
        mpRangeName->UpdateDeleteTab(rCxt, nTab);

    if (mpCondFormatList)
        mpCondFormatList->UpdateDeleteTab(rCxt);

    for (SCCOL i = 0; i <= MAXCOL; ++i)
        aCol[i].UpdateDeleteTab(rCxt);

    if (IsStreamValid())
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

    for ( SCCOL i=0; i <= MAXCOL; i++ )
    {
        aCol[i].UpdateMoveTab(rCxt, nTabNo);
        if (pProgress)
            pProgress->SetState(pProgress->GetState() + aCol[i].GetCodeCount());
    }

    if (IsStreamValid())
        SetStreamValid(false);
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
        OSL_FAIL("ExtendPrintArea: No ColInfo or RowInfo");
        return;
    }

    Point aPix1000 = pDev->LogicToPixel( Point(1000,1000), MAP_TWIP );
    double nPPTX = aPix1000.X() / 1000.0;
    double nPPTY = aPix1000.Y() / 1000.0;

    

    ScFlatBoolColSegments aSkipCols;
    aSkipCols.setFalse(0, MAXCOL);
    for (SCCOL i = 0; i <= MAXCOL; ++i)
    {
        SCCOL nLastCol = i;
        if (ColHidden(i, NULL, &nLastCol))
        {
            
            aSkipCols.setTrue(i, nLastCol);
        }
        else
        {
            
            for (SCCOL j = i; j <= nLastCol; ++j)
            {
                if (aCol[j].GetCellCount() == 0)
                    
                    aSkipCols.setTrue(j,j);
            }
        }
        i = nLastCol;
    }

    ScFlatBoolColSegments::RangeData aColData;
    for (SCCOL nCol = rEndCol; nCol >= 0; --nCol)
    {
        if (!aSkipCols.getRangeData(nCol, aColData))
            
            return;

        if (aColData.mbValue)
        {
            
            nCol = aColData.mnCol1; 
            continue;
        }

        
        for (SCCOL nDataCol = nCol; 0 <= nDataCol && nDataCol >= aColData.mnCol1; --nDataCol)
        {
            SCCOL nPrintCol = nDataCol;
            VisibleDataCellIterator aIter(*mpHiddenRows, aCol[nDataCol]);
            ScRefCellValue aCell = aIter.reset(nStartRow);
            if (aCell.isEmpty())
                
                continue;

            while (!aCell.isEmpty())
            {
                SCCOL nNewCol = nDataCol;
                SCROW nRow = aIter.getRow();
                if (nRow > nEndRow)
                    
                    break;

                MaybeAddExtraColumn(nNewCol, nRow, pDev, nPPTX, nPPTY);
                if (nNewCol > nPrintCol)
                    nPrintCol = nNewCol;
                aCell = aIter.next();
            }

            if (nPrintCol > rEndCol)
                
                rEndCol = nPrintCol;
        }
        nCol = aColData.mnCol1; 
    }
}

void ScTable::MaybeAddExtraColumn(SCCOL& rCol, SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY)
{
    ScRefCellValue aCell = aCol[rCol].GetCellValue(nRow);
    if (!aCell.hasString())
        return;

    bool bFormula = false;  
    long nPixel = aCol[rCol].GetTextWidth(nRow);

    
    if ( TEXTWIDTH_DIRTY == nPixel )
    {
        ScNeededSizeOptions aOptions;
        aOptions.bTotalSize  = true;
        aOptions.bFormula    = bFormula;
        aOptions.bSkipMerged = false;

        Fraction aZoom(1,1);
        nPixel = aCol[rCol].GetNeededSize(
            nRow, pDev, nPPTX, nPPTY, aZoom, aZoom, true, aOptions );

        aCol[rCol].SetTextWidth(nRow, static_cast<sal_uInt16>(nPixel));
    }

    long nTwips = (long) (nPixel / nPPTX);
    long nDocW = GetColWidth( rCol );

    long nMissing = nTwips - nDocW;
    if ( nMissing > 0 )
    {
        

        const ScPatternAttr* pPattern = GetPattern( rCol, nRow );
        const SfxItemSet* pCondSet = pDocument->GetCondResult( rCol, nRow, nTab );

        SvxCellHorJustify eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                        pPattern->GetItem( ATTR_HOR_JUSTIFY, pCondSet )).GetValue();
        if ( eHorJust == SVX_HOR_JUSTIFY_CENTER )
            nMissing /= 2;                          
        else
        {
            
            bool bRight = ( eHorJust == SVX_HOR_JUSTIFY_RIGHT );
            if ( IsLayoutRTL() )
                bRight = !bRight;
            if ( bRight )
                nMissing = 0;       
        }
    }

    SCCOL nNewCol = rCol;
    while (nMissing > 0 && nNewCol < MAXCOL)
    {
        ScRefCellValue aNextCell = aCol[nNewCol+1].GetCellValue(nRow);
        if (!aNextCell.isEmpty())
            
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

    InvalidatePageBreaks();     
    UpdatePageBreaks(NULL);
}

SCROW ScTable::VisibleDataCellIterator::ROW_NOT_FOUND = -1;

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
        
        mnCurRow = nRow;
        mnUBound = aData.mnRow2;
    }
    else
    {
        
        
        mnCurRow = aData.mnRow2 + 1;
        mnUBound = mnCurRow; 
        if (mnCurRow > MAXROW)
        {
            
            mnCurRow = ROW_NOT_FOUND;
            return ScRefCellValue();
        }
    }

    maCell = mrColumn.GetCellValue(mnCurRow);
    if (!maCell.isEmpty())
        
        return maCell;

    
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
            
            ScFlatBoolRowSegments::RangeData aData;
            if (!mrRowSegs.getRangeData(mnCurRow, aData))
            {
                mnCurRow = ROW_NOT_FOUND;
                return ScRefCellValue();
            }

            if (aData.mbValue)
            {
                
                
                mnCurRow = mnUBound = aData.mnRow2;
                continue;
            }

            
            mnUBound = aData.mnRow2;
        }

        maCell = mrColumn.GetCellValue(mnCurRow);
        if (!maCell.isEmpty())
            return maCell;
    }

    mnCurRow = ROW_NOT_FOUND;
    return ScRefCellValue();
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

sal_uInt8 ScTable::GetScriptType( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidCol(nCol))
        return 0;

    return aCol[nCol].GetScriptType(nRow);
}

void ScTable::SetScriptType( SCCOL nCol, SCROW nRow, sal_uInt8 nType )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].SetScriptType(nRow, nType);
}

sal_uInt8 ScTable::GetRangeScriptType(
    sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    if (!ValidCol(nCol))
        return 0;

    sc::CellStoreType::iterator itr = aCol[nCol].maCells.begin();
    return aCol[nCol].GetRangeScriptType(rBlockPos.miCellTextAttrPos, nRow1, nRow2, itr);
}

size_t ScTable::GetFormulaHash( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidCol(nCol))
        return 0;

    return aCol[nCol].GetFormulaHash(nRow);
}

ScFormulaVectorState ScTable::GetFormulaVectorState( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidCol(nCol))
        return FormulaVectorUnknown;

    return aCol[nCol].GetFormulaVectorState(nRow);
}

formula::FormulaTokenRef ScTable::ResolveStaticReference( SCCOL nCol, SCROW nRow )
{
    if (!ValidCol(nCol) || !ValidRow(nRow))
        return formula::FormulaTokenRef();

    return aCol[nCol].ResolveStaticReference(nRow);
}

formula::FormulaTokenRef ScTable::ResolveStaticReference( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 )
{
    if (nCol2 < nCol1 || nRow2 < nRow1)
        return formula::FormulaTokenRef();

    if (!ValidCol(nCol1) || !ValidCol(nCol2) || !ValidRow(nRow1) || !ValidRow(nRow2))
        return formula::FormulaTokenRef();

    ScMatrixRef pMat(new ScMatrix(nCol2-nCol1+1, nRow2-nRow1+1, 0.0));
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol)
    {
        if (!aCol[nCol].ResolveStaticReference(*pMat, nCol2-nCol1, nRow1, nRow2))
            
            return formula::FormulaTokenRef();
    }

    return formula::FormulaTokenRef(new ScMatrixToken(pMat));
}

formula::VectorRefArray ScTable::FetchVectorRefArray( SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    if (nRow2 < nRow1)
        return formula::VectorRefArray();

    if (!ValidCol(nCol) || !ValidRow(nRow1) || !ValidRow(nRow2))
        return formula::VectorRefArray();

    return aCol[nCol].FetchVectorRefArray(nRow1, nRow2);
}

ScRefCellValue ScTable::GetRefCellValue( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol, nRow))
        return ScRefCellValue();

    return aCol[nCol].GetCellValue(nRow);
}

SvtBroadcaster* ScTable::GetBroadcaster( SCCOL nCol, SCROW nRow )
{
    if (!ValidColRow(nCol, nRow))
        return NULL;

    return aCol[nCol].GetBroadcaster(nRow);
}

void ScTable::DeleteBroadcasters(
    sc::ColumnBlockPosition& rBlockPos, SCCOL nCol, SCROW nRow1, SCROW nRow2 )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].DeleteBroadcasters(rBlockPos, nRow1, nRow2);
}

bool ScTable::HasBroadcaster( SCCOL nCol ) const
{
    if (!ValidCol(nCol))
        return false;

    return aCol[nCol].HasBroadcaster();
}

void ScTable::FillMatrix( ScMatrix& rMat, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 ) const
{
    size_t nMatCol = 0;
    for (SCCOL nCol = nCol1; nCol <= nCol2; ++nCol, ++nMatCol)
        aCol[nCol].FillMatrix(rMat, nMatCol, nRow1, nRow2);
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

void ScTable::SetFormulaResults(
    SCCOL nCol, SCROW nRow, const formula::FormulaTokenRef* pResults, size_t nLen )
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].SetFormulaResults(nRow, pResults, nLen);
}

#if DEBUG_COLUMN_STORAGE
void ScTable::DumpFormulaGroups( SCCOL nCol ) const
{
    if (!ValidCol(nCol))
        return;

    aCol[nCol].DumpFormulaGroups();
}
#endif

const SvtBroadcaster* ScTable::GetBroadcaster( SCCOL nCol, SCROW nRow ) const
{
    if (!ValidColRow(nCol, nRow))
        return NULL;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
