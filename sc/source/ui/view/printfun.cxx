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

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include "printfun.hxx"

#include <svx/svxids.hrc>
#include <editeng/adjustitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editstat.hxx>
#include <svx/fmview.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/app.hxx>
#include <sfx2/printer.hxx>
#include <tools/multisel.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <svx/xoutbmp.hxx>

#include "editutil.hxx"
#include "docsh.hxx"
#include "output.hxx"
#include "viewdata.hxx"
#include "viewopti.hxx"
#include "stlpool.hxx"
#include "pagepar.hxx"
#include "attrib.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "dociter.hxx"
#include "formulacell.hxx"
#include "drawutil.hxx"
#include "globstr.hrc"
#include "scresid.hxx"
#include "sc.hrc"
#include "pagedata.hxx"
#include "printopt.hxx"
#include "prevloc.hxx"
#include "scmod.hxx"
#include "drwlayer.hxx"
#include "fillinfo.hxx"
#include "postit.hxx"

#include <vcl/lineinfo.hxx>

#include <memory>
#include <com/sun/star/document/XDocumentProperties.hpp>

#define ZOOM_MIN    10

namespace{

inline bool lcl_GetBool(const SfxItemSet* pSet, sal_uInt16 nWhich)
{
    return static_cast<const SfxBoolItem&>(pSet->Get(nWhich)).GetValue();
}

inline sal_uInt16 lcl_GetUShort(const SfxItemSet* pSet, sal_uInt16 nWhich)
{
    return static_cast<const SfxUInt16Item&>(pSet->Get(nWhich)).GetValue();
}

inline bool lcl_GetShow(const SfxItemSet* pSet, sal_uInt16 nWhich)
{
    return ScVObjMode::VOBJ_MODE_SHOW == ScVObjMode( static_cast<const ScViewObjectModeItem&>(pSet->Get(nWhich)).GetValue() );
}


} // namespace

ScPageRowEntry::ScPageRowEntry(const ScPageRowEntry& r)
{
    nStartRow = r.nStartRow;
    nEndRow   = r.nEndRow;
    nPagesX   = r.nPagesX;
    if (r.pHidden && nPagesX)
    {
        pHidden = new bool[nPagesX];
        memcpy( pHidden, r.pHidden, nPagesX * sizeof(bool) );
    }
    else
        pHidden = nullptr;
}

ScPageRowEntry& ScPageRowEntry::operator=(const ScPageRowEntry& r)
{
    delete[] pHidden;

    nStartRow = r.nStartRow;
    nEndRow   = r.nEndRow;
    nPagesX   = r.nPagesX;
    if (r.pHidden && nPagesX)
    {
        pHidden = new bool[nPagesX];
        memcpy( pHidden, r.pHidden, nPagesX * sizeof(bool) );
    }
    else
        pHidden = nullptr;

    return *this;
}

void ScPageRowEntry::SetPagesX(size_t nNew)
{
    if (pHidden)
    {
        OSL_FAIL("SetPagesX nicht nach SetHidden");
        delete[] pHidden;
        pHidden = nullptr;
    }
    nPagesX = nNew;
}

void ScPageRowEntry::SetHidden(size_t nX)
{
    if ( nX < nPagesX )
    {
        if ( nX+1 == nPagesX )  // last page?
            --nPagesX;
        else
        {
            if (!pHidden)
            {
                pHidden = new bool[nPagesX];
                memset( pHidden, false, nPagesX * sizeof(bool) );
            }
            pHidden[nX] = true;
        }
    }
}

bool ScPageRowEntry::IsHidden(size_t nX) const
{
    return nX>=nPagesX || ( pHidden && pHidden[nX] );       //! inline?
}

size_t ScPageRowEntry::CountVisible() const
{
    if ( pHidden )
    {
        size_t nVis = 0;
        for (size_t i=0; i<nPagesX; i++)
            if (!pHidden[i])
                ++nVis;
        return nVis;
    }
    else
        return nPagesX;
}

static long lcl_LineTotal(const ::editeng::SvxBorderLine* pLine)
{
    return pLine ? ( pLine->GetScaledWidth() ) : 0;
}

void ScPrintFunc::Construct( const ScPrintOptions* pOptions )
{
    pDocShell->UpdatePendingRowHeights( nPrintTab );
    pDoc = &pDocShell->GetDocument();

    SfxPrinter* pDocPrinter = pDoc->GetPrinter();   // use the printer, even for preview
    if (pDocPrinter)
        aOldPrinterMode = pDocPrinter->GetMapMode();

    //  unified MapMode for all calls (e.g. Repaint!!!)
    //  else, EditEngine outputs different text heights
    pDev->SetMapMode(MAP_PIXEL);

    pBorderItem = nullptr;
    pBackgroundItem = nullptr;
    pShadowItem = nullptr;

    pEditEngine = nullptr;
    pEditDefaults = nullptr;

    ScStyleSheetPool* pStylePool    = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet  = pStylePool->Find(
                                            pDoc->GetPageStyle( nPrintTab ),
                                            SfxStyleFamily::Page );
    if (pStyleSheet)
        pParamSet = &pStyleSheet->GetItemSet();
    else
    {
        OSL_FAIL("Seitenvorlage nicht gefunden" );
        pParamSet = nullptr;
    }

    if (!bState)
        nZoom = 100;
    nManualZoom = 100;
    bClearWin = false;
    bUseStyleColor = false;
    bIsRender = false;

    InitParam(pOptions);

    pPageData = nullptr;       // is only needed for initialisation
}

ScPrintFunc::ScPrintFunc( ScDocShell* pShell, SfxPrinter* pNewPrinter, SCTAB nTab,
                            long nPage, long nDocP, const ScRange* pArea,
                            const ScPrintOptions* pOptions,
                            ScPageBreakData* pData )
    :   pDocShell           ( pShell ),
        pPrinter            ( pNewPrinter ),
        pDrawView           ( nullptr ),
        nPrintTab           ( nTab ),
        nPageStart          ( nPage ),
        nDocPages           ( nDocP ),
        pUserArea           ( pArea ),
        bState              ( false ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        nTabPages           ( 0 ),
        nTotalPages         ( 0 ),
        nPagesX(0),
        nPagesY(0),
        nTotalY(0),
        pPageData           ( pData )
{
    pDev = pPrinter.get();
    aSrcOffset = pPrinter->PixelToLogic( pPrinter->GetPageOffsetPixel(), MAP_100TH_MM );
    Construct( pOptions );
}

ScPrintFunc::ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell, SCTAB nTab,
                            long nPage, long nDocP, const ScRange* pArea,
                            const ScPrintOptions* pOptions )
    :   pDocShell           ( pShell ),
        pPrinter            ( nullptr ),
        pDrawView           ( nullptr ),
        nPrintTab           ( nTab ),
        nPageStart          ( nPage ),
        nDocPages           ( nDocP ),
        pUserArea           ( pArea ),
        bState              ( false ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        nTabPages           ( 0 ),
        nTotalPages         ( 0 ),
        nPagesX(0),
        nPagesY(0),
        nTotalY(0),
        pPageData           ( nullptr )
{
    pDev = pOutDev;
    Construct( pOptions );
}

ScPrintFunc::ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell,
                             const ScPrintState& rState, const ScPrintOptions* pOptions )
    :   pDocShell           ( pShell ),
        pPrinter            ( nullptr ),
        pDrawView           ( nullptr ),
        pUserArea           ( nullptr ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        nPagesX(0),
        nPagesY(0),
        nTotalY(0),
        pPageData           ( nullptr )
{
    pDev = pOutDev;

    nPrintTab   = rState.nPrintTab;
    nStartCol   = rState.nStartCol;
    nStartRow   = rState.nStartRow;
    nEndCol     = rState.nEndCol;
    nEndRow     = rState.nEndRow;
    nZoom       = rState.nZoom;
    nPagesX     = rState.nPagesX;
    nPagesY     = rState.nPagesY;
    nTabPages   = rState.nTabPages;
    nTotalPages = rState.nTotalPages;
    nPageStart  = rState.nPageStart;
    nDocPages   = rState.nDocPages;
    bState      = true;

    Construct( pOptions );
}

void ScPrintFunc::GetPrintState( ScPrintState& rState )
{
    rState.nPrintTab    = nPrintTab;
    rState.nStartCol    = nStartCol;
    rState.nStartRow    = nStartRow;
    rState.nEndCol      = nEndCol;
    rState.nEndRow      = nEndRow;
    rState.nZoom        = nZoom;
    rState.nPagesX      = nPagesX;
    rState.nPagesY      = nPagesY;
    rState.nTabPages    = nTabPages;
    rState.nTotalPages  = nTotalPages;
    rState.nPageStart   = nPageStart;
    rState.nDocPages    = nDocPages;
}

bool ScPrintFunc::GetLastSourceRange( ScRange& rRange ) const
{
    rRange = aLastSourceRange;
    return bSourceRangeValid;
}

void ScPrintFunc::FillPageData()
{
    if (pPageData)
    {
        sal_uInt16 nCount = sal::static_int_cast<sal_uInt16>( pPageData->GetCount() );
        ScPrintRangeData& rData = pPageData->GetData(nCount);       // count up

        rData.SetPrintRange( ScRange( nStartCol, nStartRow, nPrintTab,
                                        nEndCol, nEndRow, nPrintTab ) );
        // #i123672#
        if(maPageEndX.empty())
        {
            OSL_ENSURE(false, "vector access error for maPageEndX (!)");
        }
        else
        {
            rData.SetPagesX( nPagesX, &maPageEndX[0]);
        }

        // #i123672#
        if(maPageEndY.empty())
        {
            OSL_ENSURE(false, "vector access error for maPageEndY (!)");
        }
        else
        {
            rData.SetPagesY( nTotalY, &maPageEndY[0]);
        }

        //  Settings
        rData.SetTopDown( aTableParam.bTopDown );
        rData.SetAutomatic( !aAreaParam.bPrintArea );
    }
}

ScPrintFunc::~ScPrintFunc()
{
    delete pEditDefaults;
    delete pEditEngine;

    //  Printer settings are now restored from outside

    //  For DrawingLayer/Charts, the MapMode of the printer (RefDevice) must always be correct
    SfxPrinter* pDocPrinter = pDoc->GetPrinter();   // use Preview also for the printer
    if (pDocPrinter)
        pDocPrinter->SetMapMode(aOldPrinterMode);
}

void ScPrintFunc::SetDrawView( FmFormView* pNew )
{
    pDrawView = pNew;
}

static void lcl_HidePrint( ScTableInfo& rTabInfo, SCCOL nX1, SCCOL nX2 )
{
    for (SCSIZE nArrY=1; nArrY+1<rTabInfo.mnArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &rTabInfo.mpRowInfo[nArrY];
        for (SCCOL nX=nX1; nX<=nX2; nX++)
        {
            const CellInfo& rCellInfo = pThisRowInfo->pCellInfo[nX+1];
            if (!rCellInfo.bEmptyCellText)
                if (static_cast<const ScProtectionAttr&>(rCellInfo.pPatternAttr->
                            GetItem(ATTR_PROTECTION, rCellInfo.pConditionSet)).GetHidePrint())
                {
                    pThisRowInfo->pCellInfo[nX+1].maCell.clear();
                    pThisRowInfo->pCellInfo[nX+1].bEmptyCellText = true;
                }
        }
    }
}

//          output to Device (static)
//
//      us used for:
//      -   Clipboard/Bitmap
//      -   Ole-Object (DocShell::Draw)
//      -   Preview of templates

void ScPrintFunc::DrawToDev( ScDocument* pDoc, OutputDevice* pDev, double /* nPrintFactor */,
                            const Rectangle& rBound, ScViewData* pViewData, bool bMetaFile )
{
    //! evaluate nPrintFactor !!!

    SCTAB nTab = 0;
    if (pViewData)
        nTab = pViewData->GetTabNo();

    bool bDoGrid, bNullVal, bFormula;
    ScStyleSheetPool* pStylePool = pDoc->GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( pDoc->GetPageStyle( nTab ), SfxStyleFamily::Page );
    if (pStyleSheet)
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        bDoGrid  = static_cast<const SfxBoolItem&>(rSet.Get(ATTR_PAGE_GRID)).GetValue();
        bNullVal = static_cast<const SfxBoolItem&>(rSet.Get(ATTR_PAGE_NULLVALS)).GetValue();
        bFormula = static_cast<const SfxBoolItem&>(rSet.Get(ATTR_PAGE_FORMULAS)).GetValue();
    }
    else
    {
        const ScViewOptions& rOpt = pDoc->GetViewOptions();
        bDoGrid  = rOpt.GetOption(VOPT_GRID);
        bNullVal = rOpt.GetOption(VOPT_NULLVALS);
        bFormula = rOpt.GetOption(VOPT_FORMULAS);
    }

    MapMode aMode = pDev->GetMapMode();

    Rectangle aRect = rBound;

    if (aRect.Right() < aRect.Left() || aRect.Bottom() < aRect.Top())
        aRect = Rectangle( Point(), pDev->GetOutputSize() );

    SCCOL nX1 = 0;
    SCROW nY1 = 0;
    SCCOL nX2 = OLE_STD_CELLS_X - 1;
    SCROW nY2 = OLE_STD_CELLS_Y - 1;
    if (bMetaFile)
    {
        ScRange aRange = pDoc->GetRange( nTab, rBound );
        nX1 = aRange.aStart.Col();
        nY1 = aRange.aStart.Row();
        nX2 = aRange.aEnd.Col();
        nY2 = aRange.aEnd.Row();
    }
    else if (pViewData)
    {
        ScSplitPos eWhich = pViewData->GetActivePart();
        ScHSplitPos eHWhich = WhichH(eWhich);
        ScVSplitPos eVWhich = WhichV(eWhich);
        nX1 = pViewData->GetPosX(eHWhich);
        nY1 = pViewData->GetPosY(eVWhich);
        nX2 = nX1 + pViewData->VisibleCellsX(eHWhich);
        if (nX2>nX1) --nX2;
        nY2 = nY1 + pViewData->VisibleCellsY(eVWhich);
        if (nY2>nY1) --nY2;
    }

    if (nX1 > MAXCOL) nX1 = MAXCOL;
    if (nX2 > MAXCOL) nX2 = MAXCOL;
    if (nY1 > MAXROW) nY1 = MAXROW;
    if (nY2 > MAXROW) nY2 = MAXROW;

    long nDevSizeX = aRect.Right()-aRect.Left()+1;
    long nDevSizeY = aRect.Bottom()-aRect.Top()+1;

    Rectangle aLines;
    ScRange aRange( nX1,nY1,nTab, nX2,nY2,nTab );

    long nTwipsSizeX = 0;
    for (SCCOL i=nX1; i<=nX2; i++)
        nTwipsSizeX += pDoc->GetColWidth( i, nTab );
    long nTwipsSizeY = (long) pDoc->GetRowHeight( nY1, nY2, nTab );

    //  if no lines, still space for the outline frame (20 Twips = 1pt)
    //  (HasLines initalizes aLines to 0,0,0,0)
    nTwipsSizeX += aLines.Left() + std::max( aLines.Right(), 20L );
    nTwipsSizeY += aLines.Top() +  std::max( aLines.Bottom(), 20L );

    double nScaleX = (double) nDevSizeX / nTwipsSizeX;
    double nScaleY = (double) nDevSizeY / nTwipsSizeY;

                            //!     hand over Flag at FillInfo !!!!!
    ScRange aERange;
    bool bEmbed = pDoc->IsEmbedded();
    if (bEmbed)
    {
        pDoc->GetEmbedded(aERange);
        pDoc->ResetEmbedded();
    }

    //  Assemble data

    ScTableInfo aTabInfo;
    pDoc->FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nTab,
                                        nScaleX, nScaleY, false, bFormula );
    lcl_HidePrint( aTabInfo, nX1, nX2 );

    if (bEmbed)
        pDoc->SetEmbedded(aERange);

    long nScrX = aRect.Left();
    long nScrY = aRect.Top();

    //  If no lines, still leave space for grid lines
    //  (would be elseways cut away)
    long nAddX = (long)( aLines.Left() * nScaleX );
    nScrX += ( nAddX ? nAddX : 1 );
    long nAddY = (long)( aLines.Top() * nScaleY );
    nScrY += ( nAddY ? nAddY : 1 );

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, aTabInfo, pDoc, nTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nScaleX, nScaleY );
    aOutputData.SetMetaFileMode(bMetaFile);
    aOutputData.SetShowNullValues(bNullVal);
    aOutputData.SetShowFormulas(bFormula);

    ScDrawLayer* pModel = pDoc->GetDrawLayer();
    std::unique_ptr<FmFormView> pDrawView;

    if( pModel )
    {
        pDrawView.reset(new FmFormView( pModel, pDev ));
        pDrawView->ShowSdrPage(pDrawView->GetModel()->GetPage(nTab));
        pDrawView->SetPrintPreview();
        aOutputData.SetDrawView( pDrawView.get() );
    }

    //! SetUseStyleColor ??

    if ( bMetaFile && pDev->GetOutDevType() == OUTDEV_VIRDEV )
        aOutputData.SetSnapPixel();

    Point aLogStart = pDev->PixelToLogic( Point(nScrX,nScrY), MAP_100TH_MM );
    long nLogStX = aLogStart.X();
    long nLogStY = aLogStart.Y();

    //!     nZoom for GetFont in OutputData ???

    if (!bMetaFile && pViewData)
        pDev->SetMapMode(pViewData->GetLogicMode(pViewData->GetActivePart()));

    // #i72502#
    const Point aMMOffset(aOutputData.PrePrintDrawingLayer(nLogStX, nLogStY));
    aOutputData.PrintDrawingLayer(SC_LAYER_BACK, aMMOffset);

    if (!bMetaFile && pViewData)
        pDev->SetMapMode(aMode);

    aOutputData.DrawBackground(*pDev);

    aOutputData.DrawShadow();
    aOutputData.DrawFrame(*pDev);
    aOutputData.DrawStrings();

    if (!bMetaFile && pViewData)
        pDev->SetMapMode(pViewData->GetLogicMode(pViewData->GetActivePart()));

    aOutputData.DrawEdit(!bMetaFile);

    if (bDoGrid)
    {
        if (!bMetaFile && pViewData)
            pDev->SetMapMode(aMode);

        aOutputData.DrawGrid(*pDev, true, false);    // no page breaks

        pDev->SetLineColor( COL_BLACK );

        Size aOne = pDev->PixelToLogic( Size(1,1) );
        if (bMetaFile)
            aOne = Size(1,1);   // compatible with DrawGrid
        long nRight = nScrX + aOutputData.GetScrW() - aOne.Width();
        long nBottom = nScrY + aOutputData.GetScrH() - aOne.Height();

        bool bLayoutRTL = pDoc->IsLayoutRTL( nTab );

        // extra line at the left edge for left-to-right, right for right-to-left
        if ( bLayoutRTL )
            pDev->DrawLine( Point(nRight,nScrY), Point(nRight,nBottom) );
        else
            pDev->DrawLine( Point(nScrX,nScrY), Point(nScrX,nBottom) );
        // extra line at the top in both cases
        pDev->DrawLine( Point(nScrX,nScrY), Point(nRight,nScrY) );
    }

    // #i72502#
    aOutputData.PrintDrawingLayer(SC_LAYER_FRONT, aMMOffset);
    aOutputData.PrintDrawingLayer(SC_LAYER_INTERN, aMMOffset);
    aOutputData.PostPrintDrawingLayer(aMMOffset); // #i74768#
}

//          Printing

static void lcl_FillHFParam( ScPrintHFParam& rParam, const SfxItemSet* pHFSet )
{
    //  nDistance must be initialized differently before

    if ( pHFSet == nullptr )
    {
        rParam.bEnable  = false;
        rParam.pBorder  = nullptr;
        rParam.pBack    = nullptr;
        rParam.pShadow  = nullptr;
    }
    else
    {
        rParam.bEnable  = static_cast<const SfxBoolItem&>( pHFSet->Get(ATTR_PAGE_ON)).GetValue();
        rParam.bDynamic = static_cast<const SfxBoolItem&>( pHFSet->Get(ATTR_PAGE_DYNAMIC)).GetValue();
        rParam.bShared  = static_cast<const SfxBoolItem&>( pHFSet->Get(ATTR_PAGE_SHARED)).GetValue();
        rParam.nHeight  = static_cast<const SvxSizeItem&>( pHFSet->Get(ATTR_PAGE_SIZE)).GetSize().Height();
        const SvxLRSpaceItem* pHFLR = &static_cast<const SvxLRSpaceItem&>(pHFSet->Get(ATTR_LRSPACE));
        long nTmp;
        nTmp = pHFLR->GetLeft();
        rParam.nLeft = nTmp < 0 ? 0 : sal_uInt16(nTmp);
        nTmp = pHFLR->GetRight();
        rParam.nRight = nTmp < 0 ? 0 : sal_uInt16(nTmp);
        rParam.pBorder  = static_cast<const SvxBoxItem*>   (&pHFSet->Get(ATTR_BORDER));
        rParam.pBack    = static_cast<const SvxBrushItem*> (&pHFSet->Get(ATTR_BACKGROUND));
        rParam.pShadow  = static_cast<const SvxShadowItem*>(&pHFSet->Get(ATTR_SHADOW));

//   now back in the dialog:
//      rParam.nHeight += rParam.nDistance;             // not in the dialog any more ???

        if (rParam.pBorder)
            rParam.nHeight += lcl_LineTotal( rParam.pBorder->GetTop() ) +
                              lcl_LineTotal( rParam.pBorder->GetBottom() );

        rParam.nManHeight = rParam.nHeight;
    }

    if (!rParam.bEnable)
        rParam.nHeight = 0;
}

//  bNew = TRUE:    search for used part of the document
//  bNew = FALSE:   only limit whole lines/columns

bool ScPrintFunc::AdjustPrintArea( bool bNew )
{
    SCCOL nOldEndCol = nEndCol; // only important for !bNew
    SCROW nOldEndRow = nEndRow;
    bool bChangeCol = true;         // at bNew both are being adjusted
    bool bChangeRow = true;

    bool bNotes = aTableParam.bNotes;
    if ( bNew )
    {
        nStartCol = 0;
        nStartRow = 0;
        if (!pDoc->GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes ))
            return false;   // nothing
    }
    else
    {
        bool bFound = true;
        bChangeCol = ( nStartCol == 0 && nEndCol == MAXCOL );
        bChangeRow = ( nStartRow == 0 && nEndRow == MAXROW );
        bool bForcedChangeRow = false;

        // #i53558# Crop entire column of old row limit to real print area with
        // some fuzzyness.
        if (!bChangeRow && nStartRow == 0)
        {
            SCROW nPAEndRow;
            bFound = pDoc->GetPrintAreaVer( nPrintTab, nStartCol, nEndCol, nPAEndRow, bNotes );
            // Say we don't want to print more than ~1000 empty rows, which are
            // about 14 pages intentionally left blank..
            const SCROW nFuzzy = 23*42;
            if (nPAEndRow + nFuzzy < nEndRow)
            {
                bForcedChangeRow = true;
                nEndRow = nPAEndRow;
            }
            else
                bFound = true;  // user seems to _want_ to print some empty rows
        }
        // TODO: in case we extend the number of columns we may have to do the
        // same for horizontal cropping.

        if ( bChangeCol && bChangeRow )
            bFound = pDoc->GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes );
        else if ( bChangeCol )
            bFound = pDoc->GetPrintAreaHor( nPrintTab, nStartRow, nEndRow, nEndCol, bNotes );
        else if ( bChangeRow )
            bFound = pDoc->GetPrintAreaVer( nPrintTab, nStartCol, nEndCol, nEndRow, bNotes );

        if (!bFound)
            return false;   // empty

        if (bForcedChangeRow)
            bChangeRow = true;
    }

    pDoc->ExtendMerge( nStartCol,nStartRow, nEndCol,nEndRow, nPrintTab );  // no Refresh, incl. Attrs

    if ( bChangeCol )
    {
        OutputDevice* pRefDev = pDoc->GetPrinter();     // use the printer also for Preview
        pRefDev->SetMapMode( MAP_PIXEL );               // important for GetNeededSize

        pDoc->ExtendPrintArea( pRefDev,
                            nPrintTab, nStartCol, nStartRow, nEndCol, nEndRow );
        //  changing nEndCol
    }

    if ( nEndCol < MAXCOL && pDoc->HasAttrib(
                    nEndCol,nStartRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HASATTR_SHADOW_RIGHT ) )
        ++nEndCol;
    if ( nEndRow < MAXROW && pDoc->HasAttrib(
                    nStartCol,nEndRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HASATTR_SHADOW_DOWN ) )
        ++nEndRow;

    if (!bChangeCol) nEndCol = nOldEndCol;
    if (!bChangeRow) nEndRow = nOldEndRow;

    return true;
}

long ScPrintFunc::TextHeight( const EditTextObject* pObject )
{
    if (!pObject)
        return 0;

    pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );

    return (long) pEditEngine->GetTextHeight();
}

//  nZoom must be set !!!
//  and the respective Twip-MapMode configured

void ScPrintFunc::UpdateHFHeight( ScPrintHFParam& rParam )
{
    OSL_ENSURE( aPageSize.Width(), "UpdateHFHeight ohne aPageSize");

    if (rParam.bEnable && rParam.bDynamic)
    {
        //  calculate nHeight from content

        MakeEditEngine();
        long nPaperWidth = ( aPageSize.Width() - nLeftMargin - nRightMargin -
                                rParam.nLeft - rParam.nRight ) * 100 / nZoom;
        if (rParam.pBorder)
            nPaperWidth -= ( rParam.pBorder->GetDistance(SvxBoxItemLine::LEFT) +
                             rParam.pBorder->GetDistance(SvxBoxItemLine::RIGHT) +
                             lcl_LineTotal(rParam.pBorder->GetLeft()) +
                             lcl_LineTotal(rParam.pBorder->GetRight()) ) * 100 / nZoom;

        if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
            nPaperWidth -= ( rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::LEFT) +
                             rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::RIGHT) ) * 100L / nZoom;

        pEditEngine->SetPaperSize( Size( nPaperWidth, 10000 ) );

        long nMaxHeight = 0;
        if ( rParam.pLeft )
        {
            nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pLeft->GetLeftArea() ) );
            nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pLeft->GetCenterArea() ) );
            nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pLeft->GetRightArea() ) );
        }
        if ( rParam.pRight )
        {
            nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pRight->GetLeftArea() ) );
            nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pRight->GetCenterArea() ) );
            nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pRight->GetRightArea() ) );
        }

        rParam.nHeight = nMaxHeight + rParam.nDistance;
        if (rParam.pBorder)
            rParam.nHeight += rParam.pBorder->GetDistance(SvxBoxItemLine::TOP) +
                              rParam.pBorder->GetDistance(SvxBoxItemLine::BOTTOM) +
                              lcl_LineTotal( rParam.pBorder->GetTop() ) +
                              lcl_LineTotal( rParam.pBorder->GetBottom() );
        if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
            rParam.nHeight += rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::TOP) +
                              rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM);

        if (rParam.nHeight < rParam.nManHeight)
            rParam.nHeight = rParam.nManHeight;         // configured minimum
    }
}

void ScPrintFunc::InitParam( const ScPrintOptions* pOptions )
{
    if (!pParamSet)
        return;

                                // TabPage "Page"
    const SvxLRSpaceItem* pLRItem = static_cast<const SvxLRSpaceItem*>( &pParamSet->Get( ATTR_LRSPACE ) );
    long nTmp;
    nTmp = pLRItem->GetLeft();
    nLeftMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
    nTmp = pLRItem->GetRight();
    nRightMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
    const SvxULSpaceItem* pULItem = static_cast<const SvxULSpaceItem*>( &pParamSet->Get( ATTR_ULSPACE ) );
    nTopMargin    = pULItem->GetUpper();
    nBottomMargin = pULItem->GetLower();

    const SvxPageItem* pPageItem = static_cast<const SvxPageItem*>( &pParamSet->Get( ATTR_PAGE ) );
    nPageUsage          = pPageItem->GetPageUsage();
    bLandscape          = pPageItem->IsLandscape();
    aFieldData.eNumType = pPageItem->GetNumType();

    bCenterHor = static_cast<const SfxBoolItem&>( pParamSet->Get(ATTR_PAGE_HORCENTER)).GetValue();
    bCenterVer = static_cast<const SfxBoolItem&>( pParamSet->Get(ATTR_PAGE_VERCENTER)).GetValue();

    aPageSize = static_cast<const SvxSizeItem&>( pParamSet->Get(ATTR_PAGE_SIZE)).GetSize();
    if ( !aPageSize.Width() || !aPageSize.Height() )
    {
        OSL_FAIL("PageSize Null ?!?!?");
        aPageSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );
    }

    pBorderItem     = static_cast<const SvxBoxItem*>   (&pParamSet->Get(ATTR_BORDER) );
    pBackgroundItem = static_cast<const SvxBrushItem*> (&pParamSet->Get(ATTR_BACKGROUND) );
    pShadowItem     = static_cast<const SvxShadowItem*>(&pParamSet->Get(ATTR_SHADOW) );

                                // TabPage "Headline"

    aHdr.pLeft      = static_cast<const ScPageHFItem*>( &pParamSet->Get(ATTR_PAGE_HEADERLEFT) );      // Content
    aHdr.pRight     = static_cast<const ScPageHFItem*>( &pParamSet->Get(ATTR_PAGE_HEADERRIGHT) );

    const SvxSetItem* pHeaderSetItem;
    const SfxItemSet* pHeaderSet = nullptr;
    if ( pParamSet->GetItemState( ATTR_PAGE_HEADERSET, false,
                            reinterpret_cast<const SfxPoolItem**>(&pHeaderSetItem) ) == SfxItemState::SET )
    {
        pHeaderSet = &pHeaderSetItem->GetItemSet();
                                                        // Headline has space below
        aHdr.nDistance  = static_cast<const SvxULSpaceItem&>(pHeaderSet->Get(ATTR_ULSPACE)).GetLower();
    }
    lcl_FillHFParam( aHdr, pHeaderSet );

                                // TabPage "Footline"

    aFtr.pLeft      = static_cast<const ScPageHFItem*>( &pParamSet->Get(ATTR_PAGE_FOOTERLEFT) );      // Content
    aFtr.pRight     = static_cast<const ScPageHFItem*>( &pParamSet->Get(ATTR_PAGE_FOOTERRIGHT) );

    const SvxSetItem* pFooterSetItem;
    const SfxItemSet* pFooterSet = nullptr;
    if ( pParamSet->GetItemState( ATTR_PAGE_FOOTERSET, false,
                            reinterpret_cast<const SfxPoolItem**>(&pFooterSetItem) ) == SfxItemState::SET )
    {
        pFooterSet = &pFooterSetItem->GetItemSet();
                                                        // Footline has space above
        aFtr.nDistance  = static_cast<const SvxULSpaceItem&>(pFooterSet->Get(ATTR_ULSPACE)).GetUpper();
    }
    lcl_FillHFParam( aFtr, pFooterSet );

    // Compile Table-/Area-Params from single Items

    // TabPage "Table"

    const SfxUInt16Item*     pScaleItem          = nullptr;
    const ScPageScaleToItem* pScaleToItem        = nullptr;
    const SfxUInt16Item*     pScaleToPagesItem   = nullptr;
    SfxItemState             eState;

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALE, false,
                                      reinterpret_cast<const SfxPoolItem**>(&pScaleItem) );
    if ( SfxItemState::DEFAULT == eState )
        pScaleItem = static_cast<const SfxUInt16Item*>(
                    &pParamSet->GetPool()->GetDefaultItem( ATTR_PAGE_SCALE ));

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALETO, false,
                                      reinterpret_cast<const SfxPoolItem**>(&pScaleToItem) );
    if ( SfxItemState::DEFAULT == eState )
        pScaleToItem = static_cast<const ScPageScaleToItem*>(
                    &pParamSet->GetPool()->GetDefaultItem( ATTR_PAGE_SCALETO ));

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALETOPAGES, false,
                                      reinterpret_cast<const SfxPoolItem**>(&pScaleToPagesItem) );
    if ( SfxItemState::DEFAULT == eState )
        pScaleToPagesItem = static_cast<const SfxUInt16Item*>(
                    &pParamSet->GetPool()->GetDefaultItem( ATTR_PAGE_SCALETOPAGES ));

    OSL_ENSURE( pScaleItem && pScaleToItem && pScaleToPagesItem, "Missing ScaleItem! :-/" );

    aTableParam.bCellContent    = true;
    aTableParam.bNotes          = lcl_GetBool(pParamSet,ATTR_PAGE_NOTES);
    aTableParam.bGrid           = lcl_GetBool(pParamSet,ATTR_PAGE_GRID);
    aTableParam.bHeaders        = lcl_GetBool(pParamSet,ATTR_PAGE_HEADERS);
    aTableParam.bFormulas       = lcl_GetBool(pParamSet,ATTR_PAGE_FORMULAS);
    aTableParam.bNullVals       = lcl_GetBool(pParamSet,ATTR_PAGE_NULLVALS);
    aTableParam.bCharts         = lcl_GetShow(pParamSet,ATTR_PAGE_CHARTS);
    aTableParam.bObjects        = lcl_GetShow(pParamSet,ATTR_PAGE_OBJECTS);
    aTableParam.bDrawings       = lcl_GetShow(pParamSet,ATTR_PAGE_DRAWINGS);
    aTableParam.bTopDown        = lcl_GetBool(pParamSet,ATTR_PAGE_TOPDOWN);
    aTableParam.bLeftRight      = !aTableParam.bLeftRight;
    aTableParam.nFirstPageNo    = lcl_GetUShort(pParamSet,ATTR_PAGE_FIRSTPAGENO);
    if (!aTableParam.nFirstPageNo)
        aTableParam.nFirstPageNo = (sal_uInt16) nPageStart;     // from previous table

    if ( pScaleItem && pScaleToItem && pScaleToPagesItem )
    {
        sal_uInt16  nScaleAll     = pScaleItem->GetValue();
        sal_uInt16  nScaleToPages = pScaleToPagesItem->GetValue();

        aTableParam.bScaleNone      = (nScaleAll     == 100);
        aTableParam.bScaleAll       = (nScaleAll      > 0  );
        aTableParam.bScaleTo        = pScaleToItem->IsValid();
        aTableParam.bScalePageNum   = (nScaleToPages  > 0  );
        aTableParam.nScaleAll       = nScaleAll;
        aTableParam.nScaleWidth     = pScaleToItem->GetWidth();
        aTableParam.nScaleHeight    = pScaleToItem->GetHeight();
        aTableParam.nScalePageNum   = nScaleToPages;
    }
    else
    {
        aTableParam.bScaleNone      = true;
        aTableParam.bScaleAll       = false;
        aTableParam.bScaleTo        = false;
        aTableParam.bScalePageNum   = false;
        aTableParam.nScaleAll       = 0;
        aTableParam.nScaleWidth     = 0;
        aTableParam.nScaleHeight    = 0;
        aTableParam.nScalePageNum   = 0;
    }

    //  skip empty pages only if options with that flag are passed
    aTableParam.bSkipEmpty = pOptions && pOptions->GetSkipEmpty();
    if ( pPageData )
        aTableParam.bSkipEmpty = false;
    // If pPageData is set, only the breaks are interesting for the
    // pagebreak preview, empty pages are not addressed separately.

    aTableParam.bForceBreaks = pOptions && pOptions->GetForceBreaks();

    // TabPage "Parts":

    //! walk through all PrintAreas of the table !!!
    const ScRange*  pPrintArea = pDoc->GetPrintRange( nPrintTab, 0 );
    const ScRange*  pRepeatCol = pDoc->GetRepeatColRange( nPrintTab );
    const ScRange*  pRepeatRow = pDoc->GetRepeatRowRange( nPrintTab );

    //  ignoring ATTR_PAGE_PRINTTABLES

    bool bHasPrintRange = pDoc->HasPrintRange();
    sal_uInt16 nPrintRangeCount = pDoc->GetPrintRangeCount(nPrintTab);
    bool bPrintEntireSheet = pDoc->IsPrintEntireSheet(nPrintTab);

    if (!bPrintEntireSheet && !nPrintRangeCount)
        mbHasPrintRange = false;

    if ( pUserArea )                // UserArea (selection) has priority
    {
        bPrintCurrentTable    =
        aAreaParam.bPrintArea = true;                   // Selection
        aAreaParam.aPrintArea = *pUserArea;

        //  The table-query is already in DocShell::Print, here always
        aAreaParam.aPrintArea.aStart.SetTab(nPrintTab);
        aAreaParam.aPrintArea.aEnd.SetTab(nPrintTab);
    }
    else if (bHasPrintRange)
    {
        if ( pPrintArea )                               // at least one set?
        {
            bPrintCurrentTable    =
            aAreaParam.bPrintArea = true;
            aAreaParam.aPrintArea = *pPrintArea;

            bMultiArea = nPrintRangeCount > 1;
        }
        else
        {
            // do not print hidden sheets with "Print entire sheet" flag
            bPrintCurrentTable = pDoc->IsPrintEntireSheet( nPrintTab ) && pDoc->IsVisible( nPrintTab );
            aAreaParam.bPrintArea = !bPrintCurrentTable;    // otherwise the table is always counted
        }
    }
    else
    {
        //  don't print hidden tables if there's no print range defined there
        if ( pDoc->IsVisible( nPrintTab ) )
        {
            aAreaParam.bPrintArea = false;
            bPrintCurrentTable = true;
        }
        else
        {
            aAreaParam.bPrintArea = true;   // otherwise the table is always counted
            bPrintCurrentTable = false;
        }
    }

    if ( pRepeatCol )
    {
        aAreaParam.bRepeatCol = true;
        aAreaParam.aRepeatCol = *pRepeatCol;
        nRepeatStartCol = pRepeatCol->aStart.Col();
        nRepeatEndCol   = pRepeatCol->aEnd  .Col();
    }
    else
    {
        aAreaParam.bRepeatCol = false;
        nRepeatStartCol = nRepeatEndCol = SCCOL_REPEAT_NONE;
    }

    if ( pRepeatRow )
    {
        aAreaParam.bRepeatRow = true;
        aAreaParam.aRepeatRow = *pRepeatRow;
        nRepeatStartRow = pRepeatRow->aStart.Row();
        nRepeatEndRow   = pRepeatRow->aEnd  .Row();
    }
    else
    {
        aAreaParam.bRepeatRow = false;
        nRepeatStartRow = nRepeatEndRow = SCROW_REPEAT_NONE;
    }

            //  Split pages

    if (!bState)
    {
        nTabPages = CountPages();                                   // also calculates zoom
        nTotalPages = nTabPages;
        nTotalPages += CountNotePages();
    }
    else
    {
        CalcPages();            // search breaks only
        CountNotePages();       // Count notes, even if number of pages is already known
    }

    if (nDocPages)
        aFieldData.nTotalPages = nDocPages;
    else
        aFieldData.nTotalPages = nTotalPages;

    SetDateTime( Date( Date::SYSTEM ), tools::Time( tools::Time::SYSTEM ) );

    if( pDocShell->getDocProperties()->getTitle().getLength() != 0 )
        aFieldData.aTitle = pDocShell->getDocProperties()->getTitle();
    else
        aFieldData.aTitle = pDocShell->GetTitle();

    const INetURLObject& rURLObj = pDocShell->GetMedium()->GetURLObject();
    aFieldData.aLongDocName = rURLObj.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
    if ( !aFieldData.aLongDocName.isEmpty() )
        aFieldData.aShortDocName = rURLObj.GetName( INetURLObject::DECODE_UNAMBIGUOUS );
    else
        aFieldData.aShortDocName = aFieldData.aLongDocName = aFieldData.aTitle;

    //  Printer settings (Orientation, Paper) at DoPrint
}

Size ScPrintFunc::GetDataSize() const
{
    Size aSize = aPageSize;
    aSize.Width()  -= nLeftMargin + nRightMargin;
    aSize.Height() -= nTopMargin + nBottomMargin;
    aSize.Height() -= aHdr.nHeight + aFtr.nHeight;
    return aSize;
}

void ScPrintFunc::GetScaleData( Size& rPhysSize, long& rDocHdr, long& rDocFtr )
{
    rPhysSize = aPageSize;
    rPhysSize.Width()  -= nLeftMargin + nRightMargin;
    rPhysSize.Height() -= nTopMargin + nBottomMargin;

    rDocHdr = aHdr.nHeight;
    rDocFtr = aFtr.nHeight;
}

void ScPrintFunc::SetDateTime( const Date& rDate, const tools::Time& rTime )
{
    aFieldData.aDate = rDate;
    aFieldData.aTime = rTime;
}

static void lcl_DrawGraphic( const Graphic &rGraphic, vcl::RenderContext *pOut,
                      const Rectangle &rGrf, const Rectangle &rOut )
{
    const bool bNotInside = !rOut.IsInside( rGrf );
    if ( bNotInside )
    {
        pOut->Push();
        pOut->IntersectClipRegion( rOut );
    }

    ((Graphic&)rGraphic).Draw( pOut, rGrf.TopLeft(), rGrf.GetSize() );

    if ( bNotInside )
        pOut->Pop();
}

static void lcl_DrawGraphic( const SvxBrushItem &rBrush, vcl::RenderContext *pOut, OutputDevice* pRefDev,
                        const Rectangle &rOrg, const Rectangle &rOut,
                        OUString const & referer )
{
    Size aGrfSize(0,0);
    const Graphic *pGraphic = rBrush.GetGraphic(referer);
    SvxGraphicPosition ePos;
    if ( pGraphic && pGraphic->IsSupportedGraphic() )
    {
        const MapMode aMapMM( MAP_100TH_MM );
        if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
            aGrfSize = pRefDev->PixelToLogic( pGraphic->GetPrefSize(), aMapMM );
        else
            aGrfSize = OutputDevice::LogicToLogic( pGraphic->GetPrefSize(),
                                    pGraphic->GetPrefMapMode(), aMapMM );
        ePos = rBrush.GetGraphicPos();
    }
    else
        ePos = GPOS_NONE;

    Point aPos;
    Size aDrawSize = aGrfSize;

    bool bDraw = true;
    switch ( ePos )
    {
        case GPOS_LT: aPos = rOrg.TopLeft();
                      break;
        case GPOS_MT: aPos.Y() = rOrg.Top();
                      aPos.X() = rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2;
                      break;
        case GPOS_RT: aPos.Y() = rOrg.Top();
                      aPos.X() = rOrg.Right() - aGrfSize.Width();
                      break;

        case GPOS_LM: aPos.Y() = rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2;
                      aPos.X() = rOrg.Left();
                      break;
        case GPOS_MM: aPos.Y() = rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2;
                      aPos.X() = rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2;
                      break;
        case GPOS_RM: aPos.Y() = rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2;
                      aPos.X() = rOrg.Right() - aGrfSize.Width();
                      break;

        case GPOS_LB: aPos.Y() = rOrg.Bottom() - aGrfSize.Height();
                      aPos.X() = rOrg.Left();
                      break;
        case GPOS_MB: aPos.Y() = rOrg.Bottom() - aGrfSize.Height();
                      aPos.X() = rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2;
                      break;
        case GPOS_RB: aPos.Y() = rOrg.Bottom() - aGrfSize.Height();
                      aPos.X() = rOrg.Right() - aGrfSize.Width();
                      break;

        case GPOS_AREA:
                      aPos = rOrg.TopLeft();
                      aDrawSize = rOrg.GetSize();
                      break;
        case GPOS_TILED:
                    {
                        //  use GraphicObject::DrawTiled instead of an own loop
                        //  (pixel rounding is handled correctly, and a very small bitmap
                        //  is duplicated into a bigger one for better performance)

                        GraphicObject aObject( *pGraphic );

                        if( pOut->GetPDFWriter() &&
                            (aObject.GetType() == GraphicType::Bitmap || aObject.GetType() == GraphicType::Default) )
                        {
                            // For PDF export, every draw
                            // operation for bitmaps takes a noticeable
                            // amount of place (~50 characters). Thus,
                            // optimize between tile bitmap size and
                            // number of drawing operations here.
                            //
                            //                  A_out
                            // n_chars = k1 *  ---------- + k2 * A_bitmap
                            //                  A_bitmap
                            //
                            // minimum n_chars is obtained for (derive for
                            // A_bitmap, set to 0, take positive
                            // solution):
                            //                   k1
                            // A_bitmap = Sqrt( ---- A_out )
                            //                   k2
                            //
                            // where k1 is the number of chars per draw
                            // operation, and k2 is the number of chars
                            // per bitmap pixel. This is approximately 50
                            // and 7 for current PDF writer, respectively.

                            const double    k1( 50 );
                            const double    k2( 7 );
                            const Size      aSize( rOrg.GetSize() );
                            const double    Abitmap( k1/k2 * aSize.Width()*aSize.Height() );

                            aObject.DrawTiled( pOut, rOrg, aGrfSize, Size(0,0),
                                               nullptr, GraphicManagerDrawFlags::STANDARD,
                                               ::std::max( 128, static_cast<int>( sqrt(sqrt( Abitmap)) + .5 ) ) );
                        }
                        else
                        {
                            aObject.DrawTiled( pOut, rOrg, aGrfSize, Size(0,0) );
                        }

                        bDraw = false;
                    }
                    break;

        case GPOS_NONE:
                      bDraw = false;
                      break;

        default: OSL_ENSURE( !pOut, "new Graphic position?" );
    }
    Rectangle aGrf( aPos,aDrawSize );
    if ( bDraw && aGrf.IsOver( rOut ) )
    {
        lcl_DrawGraphic( *pGraphic, pOut, aGrf, rOut );
    }
}

// The frame is drawn inwards

void ScPrintFunc::DrawBorder( long nScrX, long nScrY, long nScrW, long nScrH,
                                const SvxBoxItem* pBorderData, const SvxBrushItem* pBackground,
                                const SvxShadowItem* pShadow )
{
    //!     direct output from SvxBoxItem !!!

    if (pBorderData)
        if ( !pBorderData->GetTop() && !pBorderData->GetBottom() && !pBorderData->GetLeft() &&
                                        !pBorderData->GetRight() )
            pBorderData = nullptr;

    if (!pBorderData && !pBackground && !pShadow)
        return;                                     // nothing to do

    long nLeft   = 0;
    long nRight  = 0;
    long nTop    = 0;
    long nBottom = 0;

    //  aFrameRect - outside around frame, without shadow
    if ( pShadow && pShadow->GetLocation() != SVX_SHADOW_NONE )
    {
        nLeft   += (long) ( pShadow->CalcShadowSpace(SvxShadowItemSide::LEFT)   * nScaleX );
        nRight  += (long) ( pShadow->CalcShadowSpace(SvxShadowItemSide::RIGHT)  * nScaleX );
        nTop    += (long) ( pShadow->CalcShadowSpace(SvxShadowItemSide::TOP)    * nScaleY );
        nBottom += (long) ( pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM) * nScaleY );
    }
    Rectangle aFrameRect( Point(nScrX+nLeft, nScrY+nTop),
                          Size(nScrW-nLeft-nRight, nScrH-nTop-nBottom) );

    //  center of frame, to paint lines through OutputData
    if (pBorderData)
    {
        nLeft   += (long) ( lcl_LineTotal(pBorderData->GetLeft())   * nScaleX / 2 );
        nRight  += (long) ( lcl_LineTotal(pBorderData->GetRight())  * nScaleX / 2 );
        nTop    += (long) ( lcl_LineTotal(pBorderData->GetTop())    * nScaleY / 2 );
        nBottom += (long) ( lcl_LineTotal(pBorderData->GetBottom()) * nScaleY / 2 );
    }
    long nEffHeight = nScrH - nTop - nBottom;
    long nEffWidth = nScrW - nLeft - nRight;
    if (nEffHeight<=0 || nEffWidth<=0)
        return;                                         // empty

    if ( pBackground )
    {
        if (pBackground->GetGraphicPos() != GPOS_NONE)
        {
            OutputDevice* pRefDev;
            if ( bIsRender )
                pRefDev = pDev;                 // don't use printer for PDF
            else
                pRefDev = pDoc->GetPrinter();   // use printer also for preview
            OUString referer;
            if (pDocShell->HasName()) {
                referer = pDocShell->GetMedium()->GetName();
            }
            lcl_DrawGraphic( *pBackground, pDev, pRefDev, aFrameRect, aFrameRect, referer );
        }
        else
        {
            pDev->SetFillColor(pBackground->GetColor());
            pDev->SetLineColor();
            pDev->DrawRect(aFrameRect);
        }
    }

    if ( pShadow && pShadow->GetLocation() != SVX_SHADOW_NONE )
    {
        pDev->SetFillColor(pShadow->GetColor());
        pDev->SetLineColor();
        long nShadowX = (long) ( pShadow->GetWidth() * nScaleX );
        long nShadowY = (long) ( pShadow->GetWidth() * nScaleY );
        switch (pShadow->GetLocation())
        {
            case SVX_SHADOW_TOPLEFT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()-nShadowX, aFrameRect.Top() ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Left(), aFrameRect.Bottom()-nShadowY ) );
                break;
            case SVX_SHADOW_TOPRIGHT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()+nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Top() ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Right(), aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()-nShadowY ) );
                break;
            case SVX_SHADOW_BOTTOMLEFT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Bottom(),
                        aFrameRect.Right()-nShadowX, aFrameRect.Bottom()+nShadowY ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()+nShadowY,
                        aFrameRect.Left(), aFrameRect.Bottom()+nShadowY ) );
                break;
            case SVX_SHADOW_BOTTOMRIGHT:
                pDev->DrawRect( Rectangle(
                        aFrameRect.Left()+nShadowX, aFrameRect.Bottom(),
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()+nShadowY ) );
                pDev->DrawRect( Rectangle(
                        aFrameRect.Right(), aFrameRect.Top()+nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()+nShadowY ) );
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    if (pBorderData)
    {
        std::unique_ptr<ScDocument> pBorderDoc(new ScDocument( SCDOCMODE_UNDO ));
        pBorderDoc->InitUndo( pDoc, 0,0, true,true );
        if (pBorderData)
            pBorderDoc->ApplyAttr( 0,0,0, *pBorderData );

        ScTableInfo aTabInfo;
        pBorderDoc->FillInfo( aTabInfo, 0,0, 0,0, 0,
                                            nScaleX, nScaleY, false, false );
        OSL_ENSURE(aTabInfo.mnArrCount,"nArrCount == 0");

        aTabInfo.mpRowInfo[1].nHeight = (sal_uInt16) nEffHeight;
        aTabInfo.mpRowInfo[0].pCellInfo[1].nWidth =
            aTabInfo.mpRowInfo[1].pCellInfo[1].nWidth = (sal_uInt16) nEffWidth;

        ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, aTabInfo, pBorderDoc.get(), 0,
                                    nScrX+nLeft, nScrY+nTop, 0,0, 0,0, nScaleX, nScaleY );
        aOutputData.SetUseStyleColor( bUseStyleColor );

        if (pBorderData)
            aOutputData.DrawFrame(*pDev);
    }
}

void ScPrintFunc::PrintColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY )
{
    bool bLayoutRTL = pDoc->IsLayoutRTL( nPrintTab );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();
    SCCOL nCol;

    long nHeight = (long) (PRINT_HEADER_HEIGHT * nScaleY);
    long nEndY = nScrY + nHeight - nOneY;

    long nPosX = nScrX;
    if ( bLayoutRTL )
    {
        for (nCol=nX1; nCol<=nX2; nCol++)
            nPosX += (long)( pDoc->GetColWidth( nCol, nPrintTab ) * nScaleX );
    }
    else
        nPosX -= nOneX;
    long nPosY = nScrY - nOneY;
    OUString aText;

    for (nCol=nX1; nCol<=nX2; nCol++)
    {
        sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nPrintTab );
        if (nDocW)
        {
            long nWidth = (long) (nDocW * nScaleX);
            long nEndX = nPosX + nWidth * nLayoutSign;

            pDev->DrawRect( Rectangle( nPosX,nPosY,nEndX,nEndY ) );

            aText = ::ScColToAlpha( nCol);
            long nTextWidth = pDev->GetTextWidth(aText);
            long nTextHeight = pDev->GetTextHeight();
            long nAddX = ( nWidth  - nTextWidth  ) / 2;
            long nAddY = ( nHeight - nTextHeight ) / 2;
            long nTextPosX = nPosX+nAddX;
            if ( bLayoutRTL )
                nTextPosX -= nWidth;
            pDev->DrawText( Point( nTextPosX,nPosY+nAddY ), aText );

            nPosX = nEndX;
        }
    }
}

void ScPrintFunc::PrintRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    bool bLayoutRTL = pDoc->IsLayoutRTL( nPrintTab );

    long nWidth = (long) (PRINT_HEADER_WIDTH * nScaleX);
    long nEndX = nScrX + nWidth;
    long nPosX = nScrX;
    if ( !bLayoutRTL )
    {
        nEndX -= nOneX;
        nPosX -= nOneX;
    }
    long nPosY = nScrY - nOneY;
    OUString aText;

    for (SCROW nRow=nY1; nRow<=nY2; nRow++)
    {
        sal_uInt16 nDocH = pDoc->GetRowHeight( nRow, nPrintTab );
        if (nDocH)
        {
            long nHeight = (long) (nDocH * nScaleY);
            long nEndY = nPosY + nHeight;

            pDev->DrawRect( Rectangle( nPosX,nPosY,nEndX,nEndY ) );

            aText = OUString::number( nRow+1 );
            long nTextWidth = pDev->GetTextWidth(aText);
            long nTextHeight = pDev->GetTextHeight();
            long nAddX = ( nWidth  - nTextWidth  ) / 2;
            long nAddY = ( nHeight - nTextHeight ) / 2;
            pDev->DrawText( Point( nPosX+nAddX,nPosY+nAddY ), aText );

            nPosY = nEndY;
        }
    }
}

void ScPrintFunc::LocateColHdr( SCCOL nX1, SCCOL nX2, long nScrX, long nScrY,
                                bool bRepCol, ScPreviewLocationData& rLocationData )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    long nHeight = (long) (PRINT_HEADER_HEIGHT * nScaleY);
    long nEndY = nScrY + nHeight - nOneY;

    long nPosX = nScrX - nOneX;
    for (SCCOL nCol=nX1; nCol<=nX2; nCol++)
    {
        sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nPrintTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
    }
    Rectangle aCellRect( nScrX, nScrY, nPosX, nEndY );
    rLocationData.AddColHeaders( aCellRect, nX1, nX2, bRepCol );
}

void ScPrintFunc::LocateRowHdr( SCROW nY1, SCROW nY2, long nScrX, long nScrY,
                                bool bRepRow, ScPreviewLocationData& rLocationData )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    bool bLayoutRTL = pDoc->IsLayoutRTL( nPrintTab );

    long nWidth = (long) (PRINT_HEADER_WIDTH * nScaleX);
    long nEndX = nScrX + nWidth;
    if ( !bLayoutRTL )
        nEndX -= nOneX;

    long nPosY = nScrY - nOneY;
    nPosY += pDoc->GetScaledRowHeight( nY1, nY2, nPrintTab, nScaleY);
    Rectangle aCellRect( nScrX, nScrY, nEndX, nPosY );
    rLocationData.AddRowHeaders( aCellRect, nY1, nY2, bRepRow );
}

void ScPrintFunc::LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                long nScrX, long nScrY, bool bRepCol, bool bRepRow,
                                ScPreviewLocationData& rLocationData )
{
    //  get MapMode for drawing objects (same MapMode as in ScOutputData::PrintDrawingLayer)

    Point aLogPos = OutputDevice::LogicToLogic(Point(nScrX,nScrY), aOffsetMode, aLogicMode);
    long nLogStX = aLogPos.X();
    long nLogStY = aLogPos.Y();

    SCCOL nCol;
    Point aTwipOffset;
    for (nCol=0; nCol<nX1; nCol++)
        aTwipOffset.X() -= pDoc->GetColWidth( nCol, nPrintTab );
    aTwipOffset.Y() -= pDoc->GetRowHeight( 0, nY1-1, nPrintTab );

    Point aMMOffset( aTwipOffset );
    aMMOffset.X() = (long)(aMMOffset.X() * HMM_PER_TWIPS);
    aMMOffset.Y() = (long)(aMMOffset.Y() * HMM_PER_TWIPS);
    aMMOffset += Point( nLogStX, nLogStY );
    MapMode aDrawMapMode( MAP_100TH_MM, aMMOffset, aLogicMode.GetScaleX(), aLogicMode.GetScaleY() );

    //  get pixel rectangle

    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    long nOneX = aOnePixel.Width();
    long nOneY = aOnePixel.Height();

    long nPosX = nScrX - nOneX;
    for (nCol=nX1; nCol<=nX2; nCol++)
    {
        sal_uInt16 nDocW = pDoc->GetColWidth( nCol, nPrintTab );
        if (nDocW)
            nPosX += (long) (nDocW * nScaleX);
    }

    long nPosY = nScrY - nOneY;
    nPosY += pDoc->GetScaledRowHeight( nY1, nY2, nPrintTab, nScaleY);
    Rectangle aCellRect( nScrX, nScrY, nPosX, nPosY );
    rLocationData.AddCellRange( aCellRect, ScRange( nX1,nY1,nPrintTab, nX2,nY2,nPrintTab ),
                                bRepCol, bRepRow, aDrawMapMode );
}

void ScPrintFunc::PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                long nScrX, long nScrY,
                                bool bShLeft, bool bShTop, bool bShRight, bool bShBottom )
{
    // #i47547# nothing to do if the end of the print area is before the end of
    // the repeat columns/rows (don't use negative size for ScOutputData)
    if ( nX2 < nX1 || nY2 < nY1 )
        return;

                            //!     hand over Flag at FillInfo  !!!!!
    ScRange aERange;
    bool bEmbed = pDoc->IsEmbedded();
    if (bEmbed)
    {
        pDoc->GetEmbedded(aERange);
        pDoc->ResetEmbedded();
    }

    Point aPos = OutputDevice::LogicToLogic(Point(nScrX,nScrY), aOffsetMode, aLogicMode);
    long nLogStX = aPos.X();
    long nLogStY = aPos.Y();

                    //  Assemble data

    ScTableInfo aTabInfo;
    pDoc->FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nPrintTab,
                                        nScaleX, nScaleY, true, aTableParam.bFormulas );
    lcl_HidePrint( aTabInfo, nX1, nX2 );

    if (bEmbed)
        pDoc->SetEmbedded(aERange);

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, aTabInfo, pDoc, nPrintTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nScaleX, nScaleY );

    aOutputData.SetDrawView( pDrawView );

    // test if all paint parts are hidden, then a paint is not necessary at all
    const Point aMMOffset(aOutputData.PrePrintDrawingLayer(nLogStX, nLogStY));
    const bool bHideAllDrawingLayer( pDrawView && pDrawView->getHideOle() && pDrawView->getHideChart()
            && pDrawView->getHideDraw() && pDrawView->getHideFormControl() );

    if(!bHideAllDrawingLayer)
    {
        pDev->SetMapMode(aLogicMode);
        //  don's set Clipping here (Mapmode is being moved)

        // #i72502#
        aOutputData.PrintDrawingLayer(SC_LAYER_BACK, aMMOffset);
    }

    pDev->SetMapMode(aOffsetMode);

    aOutputData.SetShowFormulas( aTableParam.bFormulas );
    aOutputData.SetShowNullValues( aTableParam.bNullVals );
    aOutputData.SetUseStyleColor( bUseStyleColor );

    Color aGridColor( COL_BLACK );
    if ( bUseStyleColor )
        aGridColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );
    aOutputData.SetGridColor( aGridColor );

    if ( !pPrinter )
    {
        OutputDevice* pRefDev = pDoc->GetPrinter();     // use the printer also for Preview
        Fraction aPrintFrac( nZoom, 100 );              // without nManualZoom
        //  MapMode, as it would arrive at the printer:
        pRefDev->SetMapMode( MapMode( MAP_100TH_MM, Point(), aPrintFrac, aPrintFrac ) );

        //  when rendering (PDF), don't use printer as ref device, but printer's MapMode
        //  has to be set anyway, as charts still use it (#106409#)
        if ( !bIsRender )
            aOutputData.SetRefDevice( pRefDev );
    }

    if( aTableParam.bCellContent )
        aOutputData.DrawBackground(*pDev);

    pDev->SetClipRegion(vcl::Region(Rectangle(
                aPos, Size(aOutputData.GetScrW(), aOutputData.GetScrH()))));
    pDev->SetClipRegion();

    if( aTableParam.bCellContent )
    {
        aOutputData.DrawExtraShadow( bShLeft, bShTop, bShRight, bShBottom );
        aOutputData.DrawFrame(*pDev);
        aOutputData.DrawStrings();
        aOutputData.DrawEdit(false);
    }

    if (aTableParam.bGrid)
        aOutputData.DrawGrid(*pDev, true, false);    // no page breaks

    aOutputData.AddPDFNotes();      // has no effect if not rendering PDF with notes enabled

    // test if all paint parts are hidden, then a paint is not necessary at all
    if(!bHideAllDrawingLayer)
    {
        // #i72502#
        aOutputData.PrintDrawingLayer(SC_LAYER_FRONT, aMMOffset);
    }

    // #i72502#
    aOutputData.PrintDrawingLayer(SC_LAYER_INTERN, aMMOffset);
    aOutputData.PostPrintDrawingLayer(aMMOffset); // #i74768#
}

bool ScPrintFunc::IsMirror( long nPageNo )          // Mirror margins?
{
    SvxPageUsage eUsage = (SvxPageUsage) ( nPageUsage & 0x000f );
    return ( eUsage == SVX_PAGE_MIRROR && (nPageNo & 1) );
}

bool ScPrintFunc::IsLeft( long nPageNo )            // left foot notes?
{
    SvxPageUsage eUsage = (SvxPageUsage) ( nPageUsage & 0x000f );
    bool bLeft;
    if (eUsage == SVX_PAGE_LEFT)
        bLeft = true;
    else if (eUsage == SVX_PAGE_RIGHT)
        bLeft = false;
    else
        bLeft = (nPageNo & 1) != 0;
    return bLeft;
}

void ScPrintFunc::MakeTableString()
{
    OUString aTmp;
    pDoc->GetName(nPrintTab, aTmp);
    aFieldData.aTabName = aTmp;
}

void ScPrintFunc::MakeEditEngine()
{
    if (!pEditEngine)
    {
        //  can't use document's edit engine pool here,
        //  because pool must have twips as default metric
        pEditEngine = new ScHeaderEditEngine( EditEngine::CreatePool() );

        pEditEngine->EnableUndo(false);
        //fdo#45869 we want text to be positioned as it would be for the
        //high dpi printed output, not as would be ideal for the 96dpi preview
        //window itself
        pEditEngine->SetRefDevice(pPrinter ? pPrinter : pDoc->GetRefDevice());
        pEditEngine->SetWordDelimiters(
                ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );
        pEditEngine->SetControlWord( pEditEngine->GetControlWord() & ~EEControlBits::RTFSTYLESHEETS );
        pDoc->ApplyAsianEditSettings( *pEditEngine );
        pEditEngine->EnableAutoColor( bUseStyleColor );

        //  Default-Set for alignment
        pEditDefaults = new SfxItemSet( pEditEngine->GetEmptyItemSet() );

        const ScPatternAttr& rPattern = static_cast<const ScPatternAttr&>(pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN));
        rPattern.FillEditItemSet( pEditDefaults );
        //  FillEditItemSet adjusts font height to 1/100th mm,
        //  but for header/footer twips is needed, as in the PatternAttr:
        std::unique_ptr<SfxPoolItem> pNewItem(rPattern.GetItem(ATTR_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT));
        pEditDefaults->Put( *pNewItem );
        pNewItem.reset(rPattern.GetItem(ATTR_CJK_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CJK));
        pEditDefaults->Put( *pNewItem );
        pNewItem.reset(rPattern.GetItem(ATTR_CTL_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CTL));
        pEditDefaults->Put( *pNewItem );
        //  don't use font color, because background color is not used
        //! there's no way to set the background for note pages
        pEditDefaults->ClearItem( EE_CHAR_COLOR );
        if (ScGlobal::IsSystemRTL())
            pEditDefaults->Put( SvxFrameDirectionItem( FRMDIR_HORI_RIGHT_TOP, EE_PARA_WRITINGDIR ) );
    }

    pEditEngine->SetData( aFieldData );     // Set page count etc.
}

//  nStartY = logic
void ScPrintFunc::PrintHF( long nPageNo, bool bHeader, long nStartY,
                            bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    const ScPrintHFParam& rParam = bHeader ? aHdr : aFtr;

    pDev->SetMapMode( aTwipMode );          // Head-/Footlines in Twips

    bool bLeft = IsLeft(nPageNo) && !rParam.bShared;
    const ScPageHFItem* pHFItem = bLeft ? rParam.pLeft : rParam.pRight;

    long nLineStartX = aPageRect.Left()  + rParam.nLeft;
    long nLineEndX   = aPageRect.Right() - rParam.nRight;
    long nLineWidth  = nLineEndX - nLineStartX + 1;

    //  Edit-Engine

    Point aStart( nLineStartX, nStartY );
    Size aPaperSize( nLineWidth, rParam.nHeight-rParam.nDistance );
    if ( rParam.pBorder )
    {
        long nLeft = lcl_LineTotal( rParam.pBorder->GetLeft() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::LEFT);
        long nTop = lcl_LineTotal( rParam.pBorder->GetTop() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::TOP);
        aStart.X() += nLeft;
        aStart.Y() += nTop;
        aPaperSize.Width() -= nLeft + lcl_LineTotal( rParam.pBorder->GetRight() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::RIGHT);
        aPaperSize.Height() -= nTop + lcl_LineTotal( rParam.pBorder->GetBottom() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::BOTTOM);
    }

    if ( rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE )
    {
        long nLeft  = rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::LEFT);
        long nTop   = rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::TOP);
        aStart.X() += nLeft;
        aStart.Y() += nTop;
        aPaperSize.Width() -= nLeft + rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::RIGHT);
        aPaperSize.Height() -= nTop + rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM);
    }

    aFieldData.nPageNo = nPageNo+aTableParam.nFirstPageNo;
    MakeEditEngine();

    pEditEngine->SetPaperSize(aPaperSize);

    //  Frame / Background

    Point aBorderStart( nLineStartX, nStartY );
    Size aBorderSize( nLineWidth, rParam.nHeight-rParam.nDistance );
    if ( rParam.bDynamic )
    {
        //  adjust here again, for even/odd head-/footlines
        //  and probably other breaks by variable (page number etc.)

        long nMaxHeight = 0;
        nMaxHeight = std::max( nMaxHeight, TextHeight( pHFItem->GetLeftArea() ) );
        nMaxHeight = std::max( nMaxHeight, TextHeight( pHFItem->GetCenterArea() ) );
        nMaxHeight = std::max( nMaxHeight, TextHeight( pHFItem->GetRightArea() ) );
        if (rParam.pBorder)
            nMaxHeight += lcl_LineTotal( rParam.pBorder->GetTop() ) +
                          lcl_LineTotal( rParam.pBorder->GetBottom() ) +
                                    rParam.pBorder->GetDistance(SvxBoxItemLine::TOP) +
                                    rParam.pBorder->GetDistance(SvxBoxItemLine::BOTTOM);
        if (rParam.pShadow && rParam.pShadow->GetLocation() != SVX_SHADOW_NONE)
            nMaxHeight += rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::TOP) +
                          rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM);

        if (nMaxHeight < rParam.nManHeight-rParam.nDistance)
            nMaxHeight = rParam.nManHeight-rParam.nDistance;        // configured Minimum

        aBorderSize.Height() = nMaxHeight;
    }

    if ( bDoPrint )
    {
        double nOldScaleX = nScaleX;
        double nOldScaleY = nScaleY;
        nScaleX = nScaleY = 1.0;            // output directly in Twips
        DrawBorder( aBorderStart.X(), aBorderStart.Y(), aBorderSize.Width(), aBorderSize.Height(),
                        rParam.pBorder, rParam.pBack, rParam.pShadow );
        nScaleX = nOldScaleX;
        nScaleY = nOldScaleY;

        //  Clipping for Text

        pDev->SetClipRegion(vcl::Region(Rectangle(aStart, aPaperSize)));

        //  left

        const EditTextObject* pObject = pHFItem->GetLeftArea();
        if (pObject)
        {
            pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
            pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );
            Point aDraw = aStart;
            long nDif = aPaperSize.Height() - (long) pEditEngine->GetTextHeight();
            if (nDif > 0)
                aDraw.Y() += nDif / 2;
            pEditEngine->Draw( pDev, aDraw );
        }

        //  center

        pObject = pHFItem->GetCenterArea();
        if (pObject)
        {
            pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
            pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );
            Point aDraw = aStart;
            long nDif = aPaperSize.Height() - (long) pEditEngine->GetTextHeight();
            if (nDif > 0)
                aDraw.Y() += nDif / 2;
            pEditEngine->Draw( pDev, aDraw );
        }

        //  right

        pObject = pHFItem->GetRightArea();
        if (pObject)
        {
            pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_RIGHT, EE_PARA_JUST ) );
            pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );
            Point aDraw = aStart;
            long nDif = aPaperSize.Height() - (long) pEditEngine->GetTextHeight();
            if (nDif > 0)
                aDraw.Y() += nDif / 2;
            pEditEngine->Draw( pDev, aDraw );
        }

        pDev->SetClipRegion();
    }

    if ( pLocationData )
    {
        Rectangle aHeaderRect( aBorderStart, aBorderSize );
        pLocationData->AddHeaderFooter( aHeaderRect, bHeader, bLeft );
    }
}

long ScPrintFunc::DoNotes( long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    if (bDoPrint)
        pDev->SetMapMode(aTwipMode);

    MakeEditEngine();
    pEditDefaults->Put( SvxAdjustItem( SVX_ADJUST_LEFT, EE_PARA_JUST ) );
    pEditEngine->SetDefaults( *pEditDefaults );

    vcl::Font aMarkFont;
    ScAutoFontColorMode eColorMode = bUseStyleColor ? SC_AUTOCOL_DISPLAY : SC_AUTOCOL_PRINT;
    static_cast<const ScPatternAttr&>(pDoc->GetPool()->GetDefaultItem(ATTR_PATTERN)).GetFont( aMarkFont, eColorMode );
    pDev->SetFont( aMarkFont );
    long nMarkLen = pDev->GetTextWidth("GW99999:");
    // without Space-Char, because it rarely arrives there

    Size aDataSize = aPageRect.GetSize();
    if ( nMarkLen > aDataSize.Width() / 2 )     // everything much too small?
        nMarkLen = aDataSize.Width() / 2;       // split the page appropriately
    aDataSize.Width() -= nMarkLen;

    pEditEngine->SetPaperSize( aDataSize );
    long nPosX = aPageRect.Left() + nMarkLen;
    long nPosY = aPageRect.Top();

    long nCount = 0;
    long nSize = aNotePosList.size();
    bool bOk;
    do
    {
        bOk = false;
        if ( nNoteStart + nCount < nSize)
        {
            ScAddress &rPos = aNotePosList[ nNoteStart + nCount ];

            if( const ScPostIt* pNote = pDoc->GetNote( rPos ) )
            {
                if(const EditTextObject *pEditText = pNote->GetEditTextObject())
                    pEditEngine->SetText(*pEditText);
                long nTextHeight = pEditEngine->GetTextHeight();
                if ( nPosY + nTextHeight < aPageRect.Bottom() )
                {
                    if (bDoPrint)
                    {
                        pEditEngine->Draw( pDev, Point( nPosX, nPosY ) );

                        OUString aMarkStr(rPos.Format(ScRefFlags::VALID, pDoc, pDoc->GetAddressConvention()));
                        aMarkStr += ":";

                        //  cell position also via EditEngine, for correct positioning
                        pEditEngine->SetText(aMarkStr);
                        pEditEngine->Draw( pDev, Point( aPageRect.Left(), nPosY ) );
                    }

                    if ( pLocationData )
                    {
                        Rectangle aTextRect( Point( nPosX, nPosY ), Size( aDataSize.Width(), nTextHeight ) );
                        pLocationData->AddNoteText( aTextRect, rPos );
                        Rectangle aMarkRect( Point( aPageRect.Left(), nPosY ), Size( nMarkLen, nTextHeight ) );
                        pLocationData->AddNoteMark( aMarkRect, rPos );
                    }

                    nPosY += nTextHeight;
                    nPosY += 200;                   // Distance
                    ++nCount;
                    bOk = true;
                }
            }
        }
    }
    while (bOk);

    return nCount;
}

long ScPrintFunc::PrintNotes( long nPageNo, long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    if ( nNoteStart >= (long) aNotePosList.size() || !aTableParam.bNotes )
        return 0;

    if ( bDoPrint && bClearWin )
    {
        //!  aggregate PrintPage !!!

        Color aBackgroundColor( COL_WHITE );
        if ( bUseStyleColor )
            aBackgroundColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );

        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor();
        pDev->SetFillColor(aBackgroundColor);
        pDev->DrawRect(Rectangle(Point(),
                Size((long)(aPageSize.Width() * nScaleX * 100 / nZoom),
                     (long)(aPageSize.Height() * nScaleY * 100 / nZoom))));
    }

    //      adjust aPageRect for left/right page

    Rectangle aTempRect = Rectangle( Point(), aPageSize );
    if (IsMirror(nPageNo))
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nRightMargin ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nLeftMargin  ) * 100 / nZoom;
    }
    else
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nLeftMargin  ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nRightMargin ) * 100 / nZoom;
    }

    if ( pPrinter && bDoPrint )
    {
        OSL_FAIL( "StartPage does not exist anymore" );
    }

    if ( bDoPrint || pLocationData )
    {
        //  Head and foot lines

        if (aHdr.bEnable)
        {
            long nHeaderY = aPageRect.Top()-aHdr.nHeight;
            PrintHF( nPageNo, true, nHeaderY, bDoPrint, pLocationData );
        }
        if (aFtr.bEnable)
        {
            long nFooterY = aPageRect.Bottom()+aFtr.nDistance;
            PrintHF( nPageNo, false, nFooterY, bDoPrint, pLocationData );
        }
    }

    long nCount = DoNotes( nNoteStart, bDoPrint, pLocationData );

    if ( pPrinter && bDoPrint )
    {
        OSL_FAIL( "EndPage does not exist anymore" );
    }

    return nCount;
}

void ScPrintFunc::PrintPage( long nPageNo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    bool bLayoutRTL = pDoc->IsLayoutRTL( nPrintTab );
    long nLayoutSign = bLayoutRTL ? -1 : 1;

    //  nPageNo is the page number within all sheets of one "start page" setting

    if ( bClearWin && bDoPrint )
    {
        //  must exactly fit to painting the frame in preview.cxx !!!

        Color aBackgroundColor( COL_WHITE );
        if ( bUseStyleColor )
            aBackgroundColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor );

        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor();
        pDev->SetFillColor(aBackgroundColor);
        pDev->DrawRect(Rectangle(Point(),
                Size((long)(aPageSize.Width() * nScaleX * 100 / nZoom),
                     (long)(aPageSize.Height() * nScaleY * 100 / nZoom))));
    }

    //      adjust aPageRect for left/right page

    Rectangle aTempRect = Rectangle( Point(), aPageSize );
    if (IsMirror(nPageNo))
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nRightMargin ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nLeftMargin  ) * 100 / nZoom;
    }
    else
    {
        aPageRect.Left()  = ( aTempRect.Left()  + nLeftMargin  ) * 100 / nZoom;
        aPageRect.Right() = ( aTempRect.Right() - nRightMargin ) * 100 / nZoom;
    }

    if ( aAreaParam.bRepeatCol )
        if ( nX1 > nRepeatStartCol && nX1 <= nRepeatEndCol )
            nX1 = nRepeatEndCol + 1;
    bool bDoRepCol = (aAreaParam.bRepeatCol && nX1 > nRepeatEndCol);
    if ( aAreaParam.bRepeatRow )
        if ( nY1 > nRepeatStartRow && nY1 <= nRepeatEndRow )
            nY1 = nRepeatEndRow + 1;
    bool bDoRepRow = (aAreaParam.bRepeatRow && nY1 > nRepeatEndRow);

    // use new object hide flags in SdrPaintView
    if(pDrawView)
    {
        pDrawView->setHideOle(!aTableParam.bObjects);
        pDrawView->setHideChart(!aTableParam.bCharts);
        pDrawView->setHideDraw(!aTableParam.bDrawings);
        pDrawView->setHideFormControl(!aTableParam.bDrawings);
    }

    if ( pPrinter && bDoPrint )
    {
        OSL_FAIL( "StartPage does not exist anymore" );
    }

    //  head and foot lines (without centering)

    if (aHdr.bEnable)
    {
        long nHeaderY = aPageRect.Top()-aHdr.nHeight;
        PrintHF( nPageNo, true, nHeaderY, bDoPrint, pLocationData );
    }
    if (aFtr.bEnable)
    {
        long nFooterY = aPageRect.Bottom()+aFtr.nDistance;
        PrintHF( nPageNo, false, nFooterY, bDoPrint, pLocationData );
    }

    //  Position ( margins / centering )

    long nLeftSpace = aPageRect.Left();     // Document-Twips
    long nTopSpace  = aPageRect.Top();
    if ( bCenterHor || bLayoutRTL )
    {
        long nDataWidth = 0;
        SCCOL i;
        for (i=nX1; i<=nX2; i++)
            nDataWidth += pDoc->GetColWidth( i,nPrintTab );
        if (bDoRepCol)
            for (i=nRepeatStartCol; i<=nRepeatEndCol; i++)
                nDataWidth += pDoc->GetColWidth( i,nPrintTab );
        if (aTableParam.bHeaders)
            nDataWidth += (long) PRINT_HEADER_WIDTH;
        if (pBorderItem)
            nDataWidth += pBorderItem->GetDistance(SvxBoxItemLine::LEFT) +
                           pBorderItem->GetDistance(SvxBoxItemLine::RIGHT);        //! Line width?
        if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
            nDataWidth += pShadowItem->CalcShadowSpace(SvxShadowItemSide::LEFT) +
                           pShadowItem->CalcShadowSpace(SvxShadowItemSide::RIGHT);
        if ( bCenterHor )
        {
            nLeftSpace += ( aPageRect.GetWidth() - nDataWidth ) / 2;        // LTR or RTL
            if (pBorderItem)
                nLeftSpace -= lcl_LineTotal(pBorderItem->GetLeft());
        }
        else if ( bLayoutRTL )
            nLeftSpace += aPageRect.GetWidth() - nDataWidth;                // align to the right edge of the page
    }
    if ( bCenterVer )
    {
        long nDataHeight = pDoc->GetRowHeight( nY1, nY2, nPrintTab);
        if (bDoRepRow)
            nDataHeight += pDoc->GetRowHeight( nRepeatStartRow,
                    nRepeatEndRow, nPrintTab);
        if (aTableParam.bHeaders)
            nDataHeight += (long) PRINT_HEADER_HEIGHT;
        if (pBorderItem)
            nDataHeight += pBorderItem->GetDistance(SvxBoxItemLine::TOP) +
                           pBorderItem->GetDistance(SvxBoxItemLine::BOTTOM);       //! Line width?
        if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
            nDataHeight += pShadowItem->CalcShadowSpace(SvxShadowItemSide::TOP) +
                           pShadowItem->CalcShadowSpace(SvxShadowItemSide::BOTTOM);
        nTopSpace += ( aPageRect.GetHeight() - nDataHeight ) / 2;
        if (pBorderItem)
            nTopSpace -= lcl_LineTotal(pBorderItem->GetTop());
    }

    //  calculate sizes of the elements for partitioning
    //  (header, repeat, data)

    long nHeaderWidth   = 0;
    long nHeaderHeight  = 0;
    long nRepeatWidth   = 0;
    long nRepeatHeight  = 0;
    long nContentWidth  = 0;        // scaled - not the same as nDataWidth above
    long nContentHeight = 0;
    if (aTableParam.bHeaders)
    {
        nHeaderWidth  = (long) (PRINT_HEADER_WIDTH * nScaleX);
        nHeaderHeight = (long) (PRINT_HEADER_HEIGHT * nScaleY);
    }
    if (bDoRepCol)
        for (SCCOL i=nRepeatStartCol; i<=nRepeatEndCol; i++)
            nRepeatWidth += (long) (pDoc->GetColWidth(i,nPrintTab) * nScaleX);
    if (bDoRepRow)
        nRepeatHeight += pDoc->GetScaledRowHeight( nRepeatStartRow,
                nRepeatEndRow, nPrintTab, nScaleY);
    for (SCCOL i=nX1; i<=nX2; i++)
        nContentWidth += (long) (pDoc->GetColWidth(i,nPrintTab) * nScaleX);
    nContentHeight += pDoc->GetScaledRowHeight( nY1, nY2, nPrintTab,
            nScaleY);

    //  partition the page

    long nStartX = ((long) ( nLeftSpace * nScaleX ));
    long nStartY = ((long) ( nTopSpace  * nScaleY ));
    long nInnerStartX = nStartX;
    long nInnerStartY = nStartY;
    if (pBorderItem)
    {
        nInnerStartX += (long) ( ( lcl_LineTotal(pBorderItem->GetLeft()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::LEFT) ) * nScaleX );
        nInnerStartY += (long) ( ( lcl_LineTotal(pBorderItem->GetTop()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::TOP) ) * nScaleY );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
    {
        nInnerStartX += (long) ( pShadowItem->CalcShadowSpace(SvxShadowItemSide::LEFT) * nScaleX );
        nInnerStartY += (long) ( pShadowItem->CalcShadowSpace(SvxShadowItemSide::TOP) * nScaleY );
    }

    if ( bLayoutRTL )
    {
        //  arrange elements starting from the right edge
        nInnerStartX += nHeaderWidth + nRepeatWidth + nContentWidth;

        //  make rounding easier so the elements are really next to each other in preview
        Size aOffsetOnePixel = pDev->PixelToLogic( Size(1,1), aOffsetMode );
        long nOffsetOneX = aOffsetOnePixel.Width();
        nInnerStartX += nOffsetOneX / 2;
    }

    long nFrameStartX = nInnerStartX;
    long nFrameStartY = nInnerStartY;

    long nRepStartX = nInnerStartX + nHeaderWidth * nLayoutSign;    // widths/heights are 0 if not used
    long nRepStartY = nInnerStartY + nHeaderHeight;
    long nDataX = nRepStartX + nRepeatWidth * nLayoutSign;
    long nDataY = nRepStartY + nRepeatHeight;
    long nEndX = nDataX + nContentWidth * nLayoutSign;
    long nEndY = nDataY + nContentHeight;
    long nFrameEndX = nEndX;
    long nFrameEndY = nEndY;

    if ( bLayoutRTL )
    {
        //  each element's start position is its left edge
        //! subtract one pixel less?
        nInnerStartX -= nHeaderWidth;       // used for header
        nRepStartX   -= nRepeatWidth;
        nDataX       -= nContentWidth;

        //  continue right of the main elements again
        nEndX += nHeaderWidth + nRepeatWidth + nContentWidth;
    }

    //  Page frame / background

    //! adjust nEndX/Y

    long nBorderEndX = nEndX;
    long nBorderEndY = nEndY;
    if (pBorderItem)
    {
        nBorderEndX += (long) ( ( lcl_LineTotal(pBorderItem->GetRight()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::RIGHT) ) * nScaleX );
        nBorderEndY += (long) ( ( lcl_LineTotal(pBorderItem->GetBottom()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::BOTTOM) ) * nScaleY );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
    {
        nBorderEndX += (long) ( pShadowItem->CalcShadowSpace(SvxShadowItemSide::RIGHT) * nScaleX );
        nBorderEndY += (long) ( pShadowItem->CalcShadowSpace(SvxShadowItemSide::BOTTOM) * nScaleY );
    }

    if ( bDoPrint )
    {
        pDev->SetMapMode( aOffsetMode );
        DrawBorder( nStartX, nStartY, nBorderEndX-nStartX, nBorderEndY-nStartY,
                        pBorderItem, pBackgroundItem, pShadowItem );

        pDev->SetMapMode( aTwipMode );
    }

    pDev->SetMapMode( aOffsetMode );

    //  Output repeating rows/columns

    if (bDoRepCol && bDoRepRow)
    {
        if ( bDoPrint )
            PrintArea( nRepeatStartCol,nRepeatStartRow, nRepeatEndCol,nRepeatEndRow,
                            nRepStartX,nRepStartY, true, true, false, false );
        if ( pLocationData )
            LocateArea( nRepeatStartCol,nRepeatStartRow, nRepeatEndCol,nRepeatEndRow,
                            nRepStartX,nRepStartY, true, true, *pLocationData );
    }
    if (bDoRepCol)
    {
        if ( bDoPrint )
            PrintArea( nRepeatStartCol,nY1, nRepeatEndCol,nY2, nRepStartX,nDataY,
                        true, !bDoRepRow, false, true );
        if ( pLocationData )
            LocateArea( nRepeatStartCol,nY1, nRepeatEndCol,nY2, nRepStartX,nDataY, true, false, *pLocationData );
    }
    if (bDoRepRow)
    {
        if ( bDoPrint )
            PrintArea( nX1,nRepeatStartRow, nX2,nRepeatEndRow, nDataX,nRepStartY,
                        !bDoRepCol, true, true, false );
        if ( pLocationData )
            LocateArea( nX1,nRepeatStartRow, nX2,nRepeatEndRow, nDataX,nRepStartY, false, true, *pLocationData );
    }

    //  output data

    if ( bDoPrint )
        PrintArea( nX1,nY1, nX2,nY2, nDataX,nDataY, !bDoRepCol,!bDoRepRow, true, true );
    if ( pLocationData )
        LocateArea( nX1,nY1, nX2,nY2, nDataX,nDataY, false,false, *pLocationData );

    //  output column/row headers
    //  after data (through probably shadow)

    Color aGridColor( COL_BLACK );
    if ( bUseStyleColor )
        aGridColor.SetColor( SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor );

    if (aTableParam.bHeaders)
    {
        if ( bDoPrint )
        {
            pDev->SetLineColor( aGridColor );
            pDev->SetFillColor();
            pDev->SetMapMode(aOffsetMode);
        }

        ScPatternAttr aPattern( pDoc->GetPool() );
        vcl::Font aFont;
        ScAutoFontColorMode eColorMode = bUseStyleColor ? SC_AUTOCOL_DISPLAY : SC_AUTOCOL_PRINT;
        aPattern.GetFont( aFont, eColorMode, pDev );
        pDev->SetFont( aFont );

        if (bDoRepCol)
        {
            if ( bDoPrint )
                PrintColHdr( nRepeatStartCol,nRepeatEndCol, nRepStartX,nInnerStartY );
            if ( pLocationData )
                LocateColHdr( nRepeatStartCol,nRepeatEndCol, nRepStartX,nInnerStartY, true, *pLocationData );
        }
        if ( bDoPrint )
            PrintColHdr( nX1,nX2, nDataX,nInnerStartY );
        if ( pLocationData )
            LocateColHdr( nX1,nX2, nDataX,nInnerStartY, false, *pLocationData );
        if (bDoRepRow)
        {
            if ( bDoPrint )
                PrintRowHdr( nRepeatStartRow,nRepeatEndRow, nInnerStartX,nRepStartY );
            if ( pLocationData )
                LocateRowHdr( nRepeatStartRow,nRepeatEndRow, nInnerStartX,nRepStartY, true, *pLocationData );
        }
        if ( bDoPrint )
            PrintRowHdr( nY1,nY2, nInnerStartX,nDataY );
        if ( pLocationData )
            LocateRowHdr( nY1,nY2, nInnerStartX,nDataY, false, *pLocationData );
    }

    //  simple frame

    if ( bDoPrint && ( aTableParam.bGrid || aTableParam.bHeaders ) )
    {
        Size aOnePixel = pDev->PixelToLogic(Size(1,1));
        long nOneX = aOnePixel.Width();
        long nOneY = aOnePixel.Height();

        long nLeftX   = nFrameStartX;
        long nTopY    = nFrameStartY - nOneY;
        long nRightX  = nFrameEndX;
        long nBottomY = nFrameEndY - nOneY;
        if ( !bLayoutRTL )
        {
            nLeftX   -= nOneX;
            nRightX  -= nOneX;
        }
        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor( aGridColor );
        pDev->SetFillColor();
        pDev->DrawRect( Rectangle( nLeftX, nTopY, nRightX, nBottomY ) );
        //  nEndX/Y without frame-adaption
    }

    if ( pPrinter && bDoPrint )
    {
        OSL_FAIL( "EndPage does not exist anymore" );
    }

    aLastSourceRange = ScRange( nX1, nY1, nPrintTab, nX2, nY2, nPrintTab );
    bSourceRangeValid = true;
}

void ScPrintFunc::SetOffset( const Point& rOfs )
{
    aSrcOffset = rOfs;
}

void ScPrintFunc::SetManualZoom( sal_uInt16 nNewZoom )
{
    nManualZoom = nNewZoom;
}

void ScPrintFunc::SetClearFlag( bool bFlag )
{
    bClearWin = bFlag;
}

void ScPrintFunc::SetUseStyleColor( bool bFlag )
{
    bUseStyleColor = bFlag;
    if (pEditEngine)
        pEditEngine->EnableAutoColor( bUseStyleColor );
}

void ScPrintFunc::SetRenderFlag( bool bFlag )
{
    bIsRender = bFlag;      // set when using XRenderable (PDF)
}

void ScPrintFunc::SetExclusivelyDrawOleAndDrawObjects()
{
    aTableParam.bCellContent = false;
    aTableParam.bNotes = false;
    aTableParam.bGrid = false;
    aTableParam.bHeaders = false;
    aTableParam.bFormulas = false;
    aTableParam.bNullVals = false;
}

//  UpdatePages is only called from outside to set the breaks correctly for viewing
//  - always without UserArea

bool ScPrintFunc::UpdatePages()
{
    if (!pParamSet)
        return false;

    //  Zoom

    nZoom = 100;
    if (aTableParam.bScalePageNum || aTableParam.bScaleTo)
        nZoom = ZOOM_MIN;                       // correct for breaks
    else if (aTableParam.bScaleAll)
    {
        nZoom = aTableParam.nScaleAll;
        if ( nZoom <= ZOOM_MIN )
            nZoom = ZOOM_MIN;
    }

    OUString aName = pDoc->GetPageStyle( nPrintTab );
    SCTAB nTabCount = pDoc->GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        if ( nTab==nPrintTab || pDoc->GetPageStyle(nTab)==aName )
        {
            //  Repeating rows/columns
            pDoc->SetRepeatArea( nTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );

            //  set breaks
            ResetBreaks(nTab);
            pDocShell->PostPaint(0,0,nTab, MAXCOL,MAXROW,nTab, PAINT_GRID);
        }

    return true;
}

long ScPrintFunc::CountPages()                          // sets also nPagesX, nPagesY
{
    bool bAreaOk = false;

    if (pDoc->HasTable( nPrintTab ))
    {
        if (aAreaParam.bPrintArea)                          // Specify print area?
        {
            if ( bPrintCurrentTable )
            {
                ScRange& rRange = aAreaParam.aPrintArea;

                //  Here, no comparison of the tables any more. Area is always valid for this table
                //  If comparison should be done here, the table of print ranges must be adjusted
                //  when inserting tables etc.!

                nStartCol = rRange.aStart.Col();
                nStartRow = rRange.aStart.Row();
                nEndCol   = rRange.aEnd  .Col();
                nEndRow   = rRange.aEnd  .Row();
                bAreaOk   = AdjustPrintArea(false);         // limit
            }
            else
                bAreaOk = false;
        }
        else                                                // search from document
            bAreaOk = AdjustPrintArea(true);
    }

    if (bAreaOk)
    {
        long nPages = 0;
        size_t nY;
        if (bMultiArea)
        {
            sal_uInt16 nRCount = pDoc->GetPrintRangeCount( nPrintTab );
            for (sal_uInt16 i=0; i<nRCount; i++)
            {
                CalcZoom(i);
                if ( aTableParam.bSkipEmpty )
                    for (nY=0; nY<nPagesY; nY++)
                    {
                        OSL_ENSURE(nY < maPageRows.size(), "vector access error for maPageRows (!)");
                        nPages += maPageRows[nY].CountVisible();
                    }
                else
                    nPages += ((long) nPagesX) * nPagesY;
                if ( pPageData )
                    FillPageData();
            }
        }
        else
        {
            CalcZoom(RANGENO_NORANGE);                      // calculate Zoom
            if ( aTableParam.bSkipEmpty )
                for (nY=0; nY<nPagesY; nY++)
                {
                    OSL_ENSURE(nY < maPageRows.size(), "vector access error for maPageRows (!)");
                    nPages += maPageRows[nY].CountVisible();
                }
            else
                nPages += ((long) nPagesX) * nPagesY;
            if ( pPageData )
                FillPageData();
        }
        return nPages;
    }
    else
    {
        nPagesX = nPagesY = nTotalY = 0;
        return 0;
    }
}

long ScPrintFunc::CountNotePages()
{
    if ( !aTableParam.bNotes || !bPrintCurrentTable )
        return 0;

    bool bError = false;
    if (!aAreaParam.bPrintArea)
        bError = !AdjustPrintArea(true);            // completely search in Doc

    sal_uInt16 nRepeats = 1;                            // how often go through it ?
    if (bMultiArea)
        nRepeats = pDoc->GetPrintRangeCount(nPrintTab);
    if (bError)
        nRepeats = 0;

    for (sal_uInt16 nStep=0; nStep<nRepeats; nStep++)
    {
        bool bDoThis = true;
        if (bMultiArea)             // go through all Areas
        {
            const ScRange* pThisRange = pDoc->GetPrintRange( nPrintTab, nStep );
            if ( pThisRange )
            {
                nStartCol = pThisRange->aStart.Col();
                nStartRow = pThisRange->aStart.Row();
                nEndCol   = pThisRange->aEnd  .Col();
                nEndRow   = pThisRange->aEnd  .Row();
                bDoThis = AdjustPrintArea(false);
            }
        }

        if (bDoThis)
        {
            for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
            {
                if (pDoc->HasColNotes(nCol, nPrintTab))
                {
                    for ( SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow )
                        {
                            if ( pDoc->HasNote(nCol, nRow, nPrintTab) )
                                aNotePosList.push_back( ScAddress( nCol, nRow, nPrintTab ) );
                        }
                }
            }
        }
    }

    long nPages = 0;
    long nNoteNr = 0;
    long nNoteAdd;
    do
    {
        nNoteAdd = PrintNotes( nPages, nNoteNr, false, nullptr );
        if (nNoteAdd)
        {
            nNoteNr += nNoteAdd;
            ++nPages;
        }
    }
    while (nNoteAdd);

    return nPages;
}

void ScPrintFunc::InitModes()               // set MapModes from  nZoom etc.
{
    aOffset = Point( aSrcOffset.X()*100/nZoom, aSrcOffset.Y()*100/nZoom );

    long nEffZoom = nZoom * (long) nManualZoom;
    nScaleX = nScaleY = HMM_PER_TWIPS;  // output in 1/100 mm

    Fraction aZoomFract( nEffZoom,10000 );
    Fraction aHorFract = aZoomFract;

    if ( !pPrinter && !bIsRender )                          // adjust scale for preview
    {
        double nFact = pDocShell->GetOutputFactor();
        aHorFract = Fraction( (long)( nEffZoom / nFact ), 10000 );
    }

    aLogicMode = MapMode( MAP_100TH_MM, Point(), aHorFract, aZoomFract );

    Point aLogicOfs( -aOffset.X(), -aOffset.Y() );
    aOffsetMode = MapMode( MAP_100TH_MM, aLogicOfs, aHorFract, aZoomFract );

    Point aTwipsOfs( (long) ( -aOffset.X() / nScaleX + 0.5 ), (long) ( -aOffset.Y() / nScaleY + 0.5 ) );
    aTwipMode = MapMode( MAP_TWIP, aTwipsOfs, aHorFract, aZoomFract );
}

void ScPrintFunc::ApplyPrintSettings()
{
    if ( pPrinter )
    {

        //  Configure Printer to Printing

        Size aEnumSize = aPageSize;

        pPrinter->SetOrientation( bLandscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT );
        if ( bLandscape )
        {
                // landscape is always interpreted as a rotation by 90 degrees !
                // this leads to non WYSIWIG but at least it prints!
                // #i21775#
                long nTemp = aEnumSize.Width();
                aEnumSize.Width() = aEnumSize.Height();
                aEnumSize.Height() = nTemp;
        }
        Paper ePaper = SvxPaperInfo::GetSvxPaper( aEnumSize, MAP_TWIP, true );
        sal_uInt16 nPaperBin = static_cast<const SvxPaperBinItem&>(pParamSet->Get(ATTR_PAGE_PAPERBIN)).GetValue();

        pPrinter->SetPaper( ePaper );
        if ( PAPER_USER == ePaper )
        {
            MapMode aPrinterMode = pPrinter->GetMapMode();
            MapMode aLocalMode( MAP_TWIP );
            pPrinter->SetMapMode( aLocalMode );
            pPrinter->SetPaperSizeUser( aEnumSize );
            pPrinter->SetMapMode( aPrinterMode );
        }

        pPrinter->SetPaperBin( nPaperBin );
    }
}

//  rPageRanges   = range for all tables
//  nStartPage    = rPageRanges starts at nStartPage
//  nDisplayStart = continuous number for displaying the page number

long ScPrintFunc::DoPrint( const MultiSelection& rPageRanges,
                                long nStartPage, long nDisplayStart, bool bDoPrint,
                                ScPreviewLocationData* pLocationData )
{
    OSL_ENSURE(pDev,"Device == NULL");
    if (!pParamSet)
        return 0;

    if ( pPrinter && bDoPrint )
        ApplyPrintSettings();

    InitModes();
    if ( pLocationData )
    {
        pLocationData->SetCellMapMode( aOffsetMode );
        pLocationData->SetPrintTab( nPrintTab );
    }

    MakeTableString();

    long nPageNo = 0;
    long nPrinted = 0;
    long nEndPage = rPageRanges.GetTotalRange().Max();

    sal_uInt16 nRepeats = 1;
    if (bMultiArea)
        nRepeats = pDoc->GetPrintRangeCount(nPrintTab);
    for (sal_uInt16 nStep=0; nStep<nRepeats; nStep++)
    {
        if (bMultiArea)                     // replace area
        {
            CalcZoom(nStep);                // also sets nStartCol etc. new
            InitModes();
        }

        SCCOL nX1;
        SCROW nY1;
        SCCOL nX2;
        SCROW nY2;
        size_t nCountX;
        size_t nCountY;

        if (aTableParam.bTopDown)                           // top-bottom
        {
            nX1 = nStartCol;
            for (nCountX=0; nCountX<nPagesX; nCountX++)
            {
                OSL_ENSURE(nCountX < maPageEndX.size(), "vector access error for maPageEndX (!)");
                nX2 = maPageEndX[nCountX];
                for (nCountY=0; nCountY<nPagesY; nCountY++)
                {
                    OSL_ENSURE(nCountY < maPageRows.size(), "vector access error for maPageRows (!)");
                    nY1 = maPageRows[nCountY].GetStartRow();
                    nY2 = maPageRows[nCountY].GetEndRow();
                    if ( !aTableParam.bSkipEmpty || !maPageRows[nCountY].IsHidden(nCountX) )
                    {
                        if ( rPageRanges.IsSelected( nPageNo+nStartPage+1 ) )
                        {
                            PrintPage( nPageNo+nDisplayStart, nX1, nY1, nX2, nY2,
                                        bDoPrint, pLocationData );
                            ++nPrinted;
                        }
                        ++nPageNo;
                    }
                }
                nX1 = nX2 + 1;
            }
        }
        else                                                // left to right
        {
            for (nCountY=0; nCountY<nPagesY; nCountY++)
            {
                OSL_ENSURE(nCountY < maPageRows.size(), "vector access error for maPageRows (!)");
                nY1 = maPageRows[nCountY].GetStartRow();
                nY2 = maPageRows[nCountY].GetEndRow();
                nX1 = nStartCol;
                for (nCountX=0; nCountX<nPagesX; nCountX++)
                {
                    OSL_ENSURE(nCountX < maPageEndX.size(), "vector access error for maPageEndX (!)");
                    nX2 = maPageEndX[nCountX];
                    if ( !aTableParam.bSkipEmpty || !maPageRows[nCountY].IsHidden(nCountX) )
                    {
                        if ( rPageRanges.IsSelected( nPageNo+nStartPage+1 ) )
                        {
                            PrintPage( nPageNo+nDisplayStart, nX1, nY1, nX2, nY2,
                                        bDoPrint, pLocationData );
                            ++nPrinted;
                        }
                        ++nPageNo;
                    }
                    nX1 = nX2 + 1;
                }
            }
        }
    }

    aFieldData.aTabName = ScGlobal::GetRscString( STR_NOTES );

    long nNoteNr = 0;
    long nNoteAdd;
    do
    {
        if ( nPageNo+nStartPage <= nEndPage )
        {
            bool bPageSelected = rPageRanges.IsSelected( nPageNo+nStartPage+1 );
            nNoteAdd = PrintNotes( nPageNo+nStartPage, nNoteNr, bDoPrint && bPageSelected,
                                    ( bPageSelected ? pLocationData : nullptr ) );
            if ( nNoteAdd )
            {
                nNoteNr += nNoteAdd;
                if (bPageSelected)
                {
                    ++nPrinted;
                    bSourceRangeValid = false;      // last page was no cell range
                }
                ++nPageNo;
            }
        }
        else
            nNoteAdd = 0;
    }
    while (nNoteAdd);

    if ( bMultiArea )
        ResetBreaks(nPrintTab);                         //breaks correct for displaying

    return nPrinted;
}

void ScPrintFunc::CalcZoom( sal_uInt16 nRangeNo )                       // calculate zoom
{
    sal_uInt16 nRCount = pDoc->GetPrintRangeCount( nPrintTab );
    const ScRange* pThisRange = nullptr;
    if ( nRangeNo != RANGENO_NORANGE || nRangeNo < nRCount )
        pThisRange = pDoc->GetPrintRange( nPrintTab, nRangeNo );
    if ( pThisRange )
    {
        nStartCol = pThisRange->aStart.Col();
        nStartRow = pThisRange->aStart.Row();
        nEndCol   = pThisRange->aEnd  .Col();
        nEndRow   = pThisRange->aEnd  .Row();
    }

    if (!AdjustPrintArea(false))                        // empty
    {
        nZoom = 100;
        nPagesX = nPagesY = nTotalY = 0;
        return;
    }

    pDoc->SetRepeatArea( nPrintTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );

    if (aTableParam.bScalePageNum)
    {
        nZoom = 100;
        sal_uInt16 nPagesToFit = aTableParam.nScalePageNum;

        // If manual breaks are forced, calculate minimum # pages required
        if (aTableParam.bForceBreaks)
        {
             sal_uInt16 nMinPages = 0;
             std::set<SCROW> aRowBreaks;
             std::set<SCCOL> aColBreaks;
             pDoc->GetAllRowBreaks(aRowBreaks, nPrintTab, false, true);
             pDoc->GetAllColBreaks(aColBreaks, nPrintTab, false, true);
             nMinPages = (aRowBreaks.size() + 1) * (aColBreaks.size() + 1);

             // #i54993# use min forced by breaks if it's > # pages in
             // scale parameter to avoid bottoming out at <= ZOOM_MIN
             nPagesToFit = nMinPages > nPagesToFit ? nMinPages : nPagesToFit;
        }

        sal_uInt16 nLastFitZoom = 0, nLastNonFitZoom = 0;
        while (true)
        {
            if (nZoom <= ZOOM_MIN)
                break;

            CalcPages();
            bool bFitsPage = (nPagesX * nPagesY <= nPagesToFit);

            if (bFitsPage)
            {
                if (nZoom == 100)
                    // If it fits at 100%, it's good enough for me.
                    break;

                nLastFitZoom = nZoom;
                nZoom = (nLastNonFitZoom + nZoom) / 2;

                if (nLastFitZoom == nZoom)
                    // It converged.  Use this zoom level.
                    break;
            }
            else
            {
                if (nZoom - nLastFitZoom <= 1)
                {
                    nZoom = nLastFitZoom;
                    CalcPages();
                    break;
                }

                nLastNonFitZoom = nZoom;
                nZoom = (nLastFitZoom + nZoom) / 2;
            }
        }
    }
    else if (aTableParam.bScaleTo)
    {
        nZoom = 100;
        sal_uInt16 nW = aTableParam.nScaleWidth;
        sal_uInt16 nH = aTableParam.nScaleHeight;

        // If manual breaks are forced, calculate minimum # pages required
        if (aTableParam.bForceBreaks)
        {
             sal_uInt16 nMinPagesW = 0, nMinPagesH = 0;
             std::set<SCROW> aRowBreaks;
             std::set<SCCOL> aColBreaks;
             pDoc->GetAllRowBreaks(aRowBreaks, nPrintTab, false, true);
             pDoc->GetAllColBreaks(aColBreaks, nPrintTab, false, true);
             nMinPagesW = aColBreaks.size() + 1;
             nMinPagesH = aRowBreaks.size() + 1;

             // #i54993# use min forced by breaks if it's > # pages in
             // scale parameters to avoid bottoming out at <= ZOOM_MIN
             nW = nMinPagesW > nW ? nMinPagesW : nW;
             nH = nMinPagesH > nH ? nMinPagesH : nH;
        }

        sal_uInt16 nLastFitZoom = 0, nLastNonFitZoom = 0;
        while (true)
        {
            if (nZoom <= ZOOM_MIN)
                break;

            CalcPages();
            bool bFitsPage = ((!nW || (nPagesX <= nW)) && (!nH || (nPagesY <= nH)));

            if (bFitsPage)
            {
                if (nZoom == 100)
                    // If it fits at 100%, it's good enough for me.
                    break;

                nLastFitZoom = nZoom;
                nZoom = (nLastNonFitZoom + nZoom) / 2;

                if (nLastFitZoom == nZoom)
                    // It converged.  Use this zoom level.
                    break;
            }
            else
            {
                if (nZoom - nLastFitZoom <= 1)
                {
                    nZoom = nLastFitZoom;
                    CalcPages();
                    break;
                }

                nLastNonFitZoom = nZoom;
                nZoom = (nLastFitZoom + nZoom) / 2;
            }
        }
    }
    else if (aTableParam.bScaleAll)
    {
        nZoom = aTableParam.nScaleAll;
        if ( nZoom <= ZOOM_MIN )
            nZoom = ZOOM_MIN;
        CalcPages();
    }
    else
    {
        OSL_ENSURE( aTableParam.bScaleNone, "no scale flag is set" );
        nZoom = 100;
        CalcPages();
    }
}

Size ScPrintFunc::GetDocPageSize()
{
                        // Adjust height of head/foot line

    InitModes();                            // initialize aTwipMode from nZoom
    pDev->SetMapMode( aTwipMode );          // head/foot line in Twips
    UpdateHFHeight( aHdr );
    UpdateHFHeight( aFtr );

                        //  Page size in Document-Twips
                        //  Calculating Left / Right also in PrintPage

    aPageRect = Rectangle( Point(), aPageSize );
    aPageRect.Left()   = ( aPageRect.Left()   + nLeftMargin                  ) * 100 / nZoom;
    aPageRect.Right()  = ( aPageRect.Right()  - nRightMargin                 ) * 100 / nZoom;
    aPageRect.Top()    = ( aPageRect.Top()    + nTopMargin    ) * 100 / nZoom + aHdr.nHeight;
    aPageRect.Bottom() = ( aPageRect.Bottom() - nBottomMargin ) * 100 / nZoom - aFtr.nHeight;

    Size aDocPageSize = aPageRect.GetSize();
    if (aTableParam.bHeaders)
    {
        aDocPageSize.Width()  -= (long) PRINT_HEADER_WIDTH;
        aDocPageSize.Height() -= (long) PRINT_HEADER_HEIGHT;
    }
    if (pBorderItem)
    {
        aDocPageSize.Width()  -= lcl_LineTotal(pBorderItem->GetLeft()) +
                                 lcl_LineTotal(pBorderItem->GetRight()) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::LEFT) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::RIGHT);
        aDocPageSize.Height() -= lcl_LineTotal(pBorderItem->GetTop()) +
                                 lcl_LineTotal(pBorderItem->GetBottom()) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::TOP) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::BOTTOM);
    }
    if (pShadowItem && pShadowItem->GetLocation() != SVX_SHADOW_NONE)
    {
        aDocPageSize.Width()  -= pShadowItem->CalcShadowSpace(SvxShadowItemSide::LEFT) +
                                 pShadowItem->CalcShadowSpace(SvxShadowItemSide::RIGHT);
        aDocPageSize.Height() -= pShadowItem->CalcShadowSpace(SvxShadowItemSide::TOP) +
                                 pShadowItem->CalcShadowSpace(SvxShadowItemSide::BOTTOM);
    }
    return aDocPageSize;
}

void ScPrintFunc::ResetBreaks( SCTAB nTab )         // Set Breaks correctly for view
{
    pDoc->SetPageSize( nTab, GetDocPageSize() );
    pDoc->UpdatePageBreaks( nTab );
}

static void lcl_SetHidden( ScDocument* pDoc, SCTAB nPrintTab, ScPageRowEntry& rPageRowEntry,
                    SCCOL nStartCol, const std::vector< SCCOL >& rPageEndX )
{
    size_t nPagesX   = rPageRowEntry.GetPagesX();
    SCROW nStartRow = rPageRowEntry.GetStartRow();
    SCROW nEndRow   = rPageRowEntry.GetEndRow();

    bool bLeftIsEmpty = false;
    ScRange aTempRange;
    Rectangle aTempRect = pDoc->GetMMRect( 0,0, 0,0, 0 );

    for (size_t i=0; i<nPagesX; i++)
    {
        OSL_ENSURE(i < rPageEndX.size(), "vector access error for maPageEndX (!)");
        SCCOL nEndCol = rPageEndX[i];
        if ( pDoc->IsPrintEmpty( nPrintTab, nStartCol, nStartRow, nEndCol, nEndRow,
                                    bLeftIsEmpty, &aTempRange, &aTempRect ) )
        {
            rPageRowEntry.SetHidden(i);
            bLeftIsEmpty = true;
        }
        else
            bLeftIsEmpty = false;

        nStartCol = nEndCol+1;
    }
}

void ScPrintFunc::CalcPages()               // calculates aPageRect and pages from nZoom
{
    // #i123672# use dynamic mem to react on size changes
    if (maPageEndX.size() < MAXCOL+1)
    {
        maPageEndX.resize(MAXCOL+1, SCCOL());
    }

    pDoc->SetPageSize( nPrintTab, GetDocPageSize() );
    if (aAreaParam.bPrintArea)
    {
        ScRange aRange( nStartCol, nStartRow, nPrintTab, nEndCol, nEndRow, nPrintTab );
        pDoc->UpdatePageBreaks( nPrintTab, &aRange );
    }
    else
    {
        pDoc->UpdatePageBreaks( nPrintTab );      // else, end is marked
    }

    const size_t nRealCnt = nEndRow-nStartRow+1;

    // #i123672# use dynamic mem to react on size changes
    if (maPageEndY.size() < nRealCnt+1)
    {
        maPageEndY.resize(nRealCnt+1, SCROW());
    }

    // #i123672# use dynamic mem to react on size changes
    if (maPageRows.size() < nRealCnt+1)
    {
        maPageRows.resize(nRealCnt+1, ScPageRowEntry());
    }

    //  Page alignment/splitting after breaks in Col/RowFlags
    //  Of several breaks in a hidden area, only one counts.

    nPagesX = 0;
    nPagesY = 0;
    nTotalY = 0;

    bool bVisCol = false;
    for (SCCOL i=nStartCol; i<=nEndCol; i++)
    {
        bool bHidden = pDoc->ColHidden(i, nPrintTab);
        bool bPageBreak = (pDoc->HasColBreak(i, nPrintTab) & ScBreakType::Page) != ScBreakType::None;
        if ( i>nStartCol && bVisCol && bPageBreak )
        {
            OSL_ENSURE(nPagesX < maPageEndX.size(), "vector access error for maPageEndX (!)");
            maPageEndX[nPagesX] = i-1;
            ++nPagesX;
            bVisCol = false;
        }
        if (!bHidden)
            bVisCol = true;
    }
    if (bVisCol)    // also at the end, no empty pages
    {
        OSL_ENSURE(nPagesX < maPageEndX.size(), "vector access error for maPageEndX (!)");
        maPageEndX[nPagesX] = nEndCol;
        ++nPagesX;
    }

    bool bVisRow = false;
    SCROW nPageStartRow = nStartRow;
    SCROW nLastVisibleRow = -1;

    std::unique_ptr<ScRowBreakIterator> pRowBreakIter(pDoc->GetRowBreakIterator(nPrintTab));
    SCROW nNextPageBreak = pRowBreakIter->first();
    while (nNextPageBreak != ScRowBreakIterator::NOT_FOUND && nNextPageBreak < nStartRow)
        // Skip until the page break position is at the start row or greater.
        nNextPageBreak = pRowBreakIter->next();

    for (SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow)
    {
        bool bPageBreak = (nNextPageBreak == nRow);
        if (bPageBreak)
            nNextPageBreak = pRowBreakIter->next();

        if (nRow > nStartRow && bVisRow && bPageBreak )
        {
            OSL_ENSURE(nTotalY < maPageEndY.size(), "vector access error for maPageEndY (!)");
            maPageEndY[nTotalY] = nRow-1;
            ++nTotalY;

            if ( !aTableParam.bSkipEmpty ||
                    !pDoc->IsPrintEmpty( nPrintTab, nStartCol, nPageStartRow, nEndCol, nRow-1 ) )
            {
                OSL_ENSURE(nPagesY < maPageRows.size(), "vector access error for maPageRows (!)");
                maPageRows[nPagesY].SetStartRow( nPageStartRow );
                maPageRows[nPagesY].SetEndRow( nRow-1 );
                maPageRows[nPagesY].SetPagesX( nPagesX );
                if (aTableParam.bSkipEmpty)
                    lcl_SetHidden( pDoc, nPrintTab, maPageRows[nPagesY], nStartCol, maPageEndX );
                ++nPagesY;
            }

            nPageStartRow = nRow;
            bVisRow = false;
        }

        if (nRow <= nLastVisibleRow)
        {
            // This row is still visible.  Don't bother calling RowHidden() to
            // find out, for speed optimization.
            bVisRow = true;
            continue;
        }

        SCROW nLastRow = -1;
        if (!pDoc->RowHidden(nRow, nPrintTab, nullptr, &nLastRow))
        {
            bVisRow = true;
            nLastVisibleRow = nLastRow;
        }
        else
            // skip all hidden rows.
            nRow = nLastRow;
    }

    if (bVisRow)
    {
        OSL_ENSURE(nTotalY < maPageEndY.size(), "vector access error for maPageEndY (!)");
        maPageEndY[nTotalY] = nEndRow;
        ++nTotalY;

        if ( !aTableParam.bSkipEmpty ||
                !pDoc->IsPrintEmpty( nPrintTab, nStartCol, nPageStartRow, nEndCol, nEndRow ) )
        {
            OSL_ENSURE(nPagesY < maPageRows.size(), "vector access error for maPageRows (!)");
            maPageRows[nPagesY].SetStartRow( nPageStartRow );
            maPageRows[nPagesY].SetEndRow( nEndRow );
            maPageRows[nPagesY].SetPagesX( nPagesX );
            if (aTableParam.bSkipEmpty)
                lcl_SetHidden( pDoc, nPrintTab, maPageRows[nPagesY], nStartCol, maPageEndX );
            ++nPagesY;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
