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
#include <editeng/eeitem.hxx>

#include <printfun.hxx>

#include <editeng/adjustitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/editstat.hxx>
#include <svx/fmview.hxx>
#include <vcl/pdfextoutdevdata.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/pbinitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/sizeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <sfx2/printer.hxx>
#include <tools/multisel.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <osl/diagnose.h>

#include <editutil.hxx>
#include <docsh.hxx>
#include <output.hxx>
#include <viewdata.hxx>
#include <viewopti.hxx>
#include <stlpool.hxx>
#include <pagepar.hxx>
#include <attrib.hxx>
#include <patattr.hxx>
#include <dociter.hxx>
#include <globstr.hrc>
#include <scresid.hxx>
#include <pagedata.hxx>
#include <printopt.hxx>
#include <prevloc.hxx>
#include <scmod.hxx>
#include <drwlayer.hxx>
#include <fillinfo.hxx>
#include <postit.hxx>

#include <memory>
#include <com/sun/star/document/XDocumentProperties.hpp>

#define ZOOM_MIN    10

namespace{

bool lcl_GetBool(const SfxItemSet* pSet, sal_uInt16 nWhich)
{
    return static_cast<const SfxBoolItem&>(pSet->Get(nWhich)).GetValue();
}

sal_uInt16 lcl_GetUShort(const SfxItemSet* pSet, sal_uInt16 nWhich)
{
    return static_cast<const SfxUInt16Item&>(pSet->Get(nWhich)).GetValue();
}

bool lcl_GetShow(const SfxItemSet* pSet, sal_uInt16 nWhich)
{
    return ScVObjMode::VOBJ_MODE_SHOW == static_cast<const ScViewObjectModeItem&>(pSet->Get(nWhich)).GetValue();
}


} // namespace

ScPageRowEntry::ScPageRowEntry(const ScPageRowEntry& r)
{
    nStartRow = r.nStartRow;
    nEndRow   = r.nEndRow;
    nPagesX   = r.nPagesX;
    aHidden   = r.aHidden;
    aHidden.resize(nPagesX, false);
}

ScPageRowEntry& ScPageRowEntry::operator=(const ScPageRowEntry& r)
{
    nStartRow = r.nStartRow;
    nEndRow   = r.nEndRow;
    nPagesX   = r.nPagesX;
    aHidden   = r.aHidden;
    aHidden.resize(nPagesX, false);
    return *this;
}

void ScPageRowEntry::SetPagesX(size_t nNew)
{
    nPagesX = nNew;
    aHidden.resize(nPagesX, false);
}

void ScPageRowEntry::SetHidden(size_t nX)
{
    if ( nX < nPagesX )
    {
        if ( nX+1 == nPagesX )  // last page?
            --nPagesX;
        else
        {
            aHidden.resize(nPagesX, false);
            aHidden[nX] = true;
        }
    }
}

bool ScPageRowEntry::IsHidden(size_t nX) const
{
    return nX >= nPagesX || aHidden[nX];       //! inline?
}

size_t ScPageRowEntry::CountVisible() const
{
    if (!aHidden.empty())
    {
        size_t nVis = 0;
        for (size_t i=0; i<nPagesX; i++)
            if (!aHidden[i])
                ++nVis;
        return nVis;
    }
    else
        return nPagesX;
}

static tools::Long lcl_LineTotal(const ::editeng::SvxBorderLine* pLine)
{
    return pLine ? ( pLine->GetScaledWidth() ) : 0;
}

void ScPrintFunc::Construct( const ScPrintOptions* pOptions )
{
    pDocShell->UpdatePendingRowHeights( nPrintTab );

    SfxPrinter* pDocPrinter = rDoc.GetPrinter();   // use the printer, even for preview
    if (pDocPrinter)
        aOldPrinterMode = pDocPrinter->GetMapMode();

    //  unified MapMode for all calls (e.g. Repaint!!!)
    //  else, EditEngine outputs different text heights
    pDev->SetMapMode(MapMode(MapUnit::MapPixel));

    pBorderItem = nullptr;
    pBackgroundItem = nullptr;
    pShadowItem = nullptr;

    pEditEngine = nullptr;
    pEditDefaults = nullptr;

    ScStyleSheetPool* pStylePool    = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet  = pStylePool->Find(
                                            rDoc.GetPageStyle( nPrintTab ),
                                            SfxStyleFamily::Page );
    if (pStyleSheet)
        pParamSet = &pStyleSheet->GetItemSet();
    else
    {
        OSL_FAIL("Template not found" );
        pParamSet = nullptr;
    }

    if (!bFromPrintState)
        nZoom = 100;
    nManualZoom = 100;
    bClearWin = false;
    bUseStyleColor = false;
    bIsRender = false;

    InitParam(pOptions);

    pPageData = nullptr;       // is only needed for initialisation
}

ScPrintFunc::ScPrintFunc(ScDocShell* pShell, SfxPrinter* pNewPrinter, SCTAB nTab, tools::Long nPage,
                         tools::Long nDocP, const ScRange* pArea, const ScPrintOptions* pOptions,
                         ScPageBreakData* pData, Size aSize, bool bPrintLandscape, bool bUsed)
    :   pDocShell           ( pShell ),
        rDoc(pDocShell->GetDocument()),
        pPrinter            ( pNewPrinter ),
        pDrawView           ( nullptr ),
        nPrintTab           ( nTab ),
        nPageStart          ( nPage ),
        nDocPages           ( nDocP ),
        pUserArea           ( pArea ),
        bFromPrintState     ( false ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        nTabPages           ( 0 ),
        nTotalPages         ( 0 ),
        bPrintAreaValid     ( false ),
        pPageData           ( pData ),
        aPrintPageSize      ( aSize ),
        bPrintPageLandscape ( bPrintLandscape ),
        bUsePrintDialogSetting ( bUsed )
{
    pDev = pPrinter.get();
    aSrcOffset = pPrinter->PixelToLogic(pPrinter->GetPageOffsetPixel(), MapMode(MapUnit::Map100thMM));
    m_aRanges.m_xPageEndX = std::make_shared<std::vector<SCCOL>>();
    m_aRanges.m_xPageEndY = std::make_shared<std::vector<SCROW>>();
    m_aRanges.m_xPageRows = std::make_shared<std::map<size_t, ScPageRowEntry>>();
    Construct( pOptions );
}

ScPrintFunc::ScPrintFunc(ScDocShell* pShell, SfxPrinter* pNewPrinter, const ScPrintState& rState,
                         const ScPrintOptions* pOptions, Size aSize, bool bPrintLandscape,
                         bool bUsed)
    :   pDocShell           ( pShell ),
        rDoc(pDocShell->GetDocument()),
        pPrinter            ( pNewPrinter ),
        pDrawView           ( nullptr ),
        pUserArea           ( nullptr ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        pPageData           ( nullptr ),
        aPrintPageSize      ( aSize ),
        bPrintPageLandscape ( bPrintLandscape ),
        bUsePrintDialogSetting ( bUsed )
{
    pDev = pPrinter.get();

    nPrintTab   = rState.nPrintTab;
    nStartCol   = rState.nStartCol;
    nStartRow   = rState.nStartRow;
    nEndCol     = rState.nEndCol;
    nEndRow     = rState.nEndRow;
    bPrintAreaValid = rState.bPrintAreaValid;
    nZoom       = rState.nZoom;
//    m_aRanges.m_nPagesX = rState.nPagesX;
//    m_aRanges.m_nPagesY = rState.nPagesY;
    m_aRanges = rState.m_aRanges;
    nTabPages   = rState.nTabPages;
    nTotalPages = rState.nTotalPages;
    nPageStart  = rState.nPageStart;
    nDocPages   = rState.nDocPages;
    bFromPrintState = true;

    aSrcOffset = pPrinter->PixelToLogic(pPrinter->GetPageOffsetPixel(), MapMode(MapUnit::Map100thMM));
    Construct( pOptions );
}

ScPrintFunc::ScPrintFunc( OutputDevice* pOutDev, ScDocShell* pShell, SCTAB nTab,
                            tools::Long nPage, tools::Long nDocP, const ScRange* pArea,
                            const ScPrintOptions* pOptions )
    :   pDocShell           ( pShell ),
        rDoc(pDocShell->GetDocument()),
        pPrinter            ( nullptr ),
        pDrawView           ( nullptr ),
        nPrintTab           ( nTab ),
        nPageStart          ( nPage ),
        nDocPages           ( nDocP ),
        pUserArea           ( pArea ),
        bFromPrintState     ( false ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        nTabPages           ( 0 ),
        nTotalPages         ( 0 ),
        bPrintAreaValid     ( false ),
        pPageData           ( nullptr ),
        aPrintPageSize      ( Size() ),
        bPrintPageLandscape ( false ),
        bUsePrintDialogSetting ( false )
{
    pDev = pOutDev;
    m_aRanges.m_xPageEndX = std::make_shared<std::vector<SCCOL>>();
    m_aRanges.m_xPageEndY = std::make_shared<std::vector<SCROW>>();
    m_aRanges.m_xPageRows = std::make_shared<std::map<size_t, ScPageRowEntry>>();
    Construct( pOptions );
}

ScPrintFunc::ScPrintFunc(OutputDevice* pOutDev, ScDocShell* pShell, const ScPrintState& rState,
                         const ScPrintOptions* pOptions, Size aSize, bool bPrintLandscape,
                         bool bUsed)
    :   pDocShell           ( pShell ),
        rDoc(pDocShell->GetDocument()),
        pPrinter            ( nullptr ),
        pDrawView           ( nullptr ),
        pUserArea           ( nullptr ),
        bSourceRangeValid   ( false ),
        bPrintCurrentTable  ( false ),
        bMultiArea          ( false ),
        mbHasPrintRange(true),
        pPageData           ( nullptr ),
        aPrintPageSize      ( aSize ),
        bPrintPageLandscape ( bPrintLandscape ),
        bUsePrintDialogSetting ( bUsed )
{
    pDev = pOutDev;

    nPrintTab   = rState.nPrintTab;
    nStartCol   = rState.nStartCol;
    nStartRow   = rState.nStartRow;
    nEndCol     = rState.nEndCol;
    nEndRow     = rState.nEndRow;
    bPrintAreaValid = rState.bPrintAreaValid;
    nZoom       = rState.nZoom;
    m_aRanges   = rState.m_aRanges;
    nTabPages   = rState.nTabPages;
    nTotalPages = rState.nTotalPages;
    nPageStart  = rState.nPageStart;
    nDocPages   = rState.nDocPages;
    bFromPrintState = true;

    Construct( pOptions );
}

void ScPrintFunc::GetPrintState(ScPrintState& rState)
{
    rState.nPrintTab    = nPrintTab;
    rState.nStartCol    = nStartCol;
    rState.nStartRow    = nStartRow;
    rState.nEndCol      = nEndCol;
    rState.nEndRow      = nEndRow;
    rState.bPrintAreaValid = bPrintAreaValid;
    rState.nZoom        = nZoom;
    rState.nTabPages    = nTabPages;
    rState.nTotalPages  = nTotalPages;
    rState.nPageStart   = nPageStart;
    rState.nDocPages    = nDocPages;
    rState.m_aRanges = m_aRanges;
}

bool ScPrintFunc::GetLastSourceRange( ScRange& rRange ) const
{
    rRange = aLastSourceRange;
    return bSourceRangeValid;
}

void ScPrintFunc::FillPageData()
{
    if (!pPageData)
        return;

    sal_uInt16 nCount = sal::static_int_cast<sal_uInt16>( pPageData->GetCount() );
    ScPrintRangeData& rData = pPageData->GetData(nCount);       // count up

    assert( bPrintAreaValid );
    rData.SetPrintRange( ScRange( nStartCol, nStartRow, nPrintTab,
                                    nEndCol, nEndRow, nPrintTab ) );
    // #i123672#
    if(m_aRanges.m_xPageEndX->empty())
    {
        OSL_ENSURE(false, "vector access error for maPageEndX (!)");
    }
    else
    {
        rData.SetPagesX( m_aRanges.m_nPagesX, m_aRanges.m_xPageEndX->data());
    }

    // #i123672#
    if(m_aRanges.m_xPageEndY->empty())
    {
        OSL_ENSURE(false, "vector access error for maPageEndY (!)");
    }
    else
    {
        rData.SetPagesY( m_aRanges.m_nTotalY, m_aRanges.m_xPageEndY->data());
    }

    //  Settings
    rData.SetTopDown( aTableParam.bTopDown );
    rData.SetAutomatic( !aAreaParam.bPrintArea );
}

ScPrintFunc::~ScPrintFunc()
{
    pEditDefaults.reset();
    pEditEngine.reset();

    //  Printer settings are now restored from outside

    //  For DrawingLayer/Charts, the MapMode of the printer (RefDevice) must always be correct
    SfxPrinter* pDocPrinter = rDoc.GetPrinter();   // use Preview also for the printer
    if (pDocPrinter)
        pDocPrinter->SetMapMode(aOldPrinterMode);
}

void ScPrintFunc::SetDrawView( FmFormView* pNew )
{
    pDrawView = pNew;
}

static void lcl_HidePrint( const ScTableInfo& rTabInfo, SCCOL nX1, SCCOL nX2 )
{
    for (SCSIZE nArrY=1; nArrY+1<rTabInfo.mnArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &rTabInfo.mpRowInfo[nArrY];
        for (SCCOL nX=nX1; nX<=nX2; nX++)
        {
            ScCellInfo& rCellInfo = pThisRowInfo->cellInfo(nX);
            ScBasicCellInfo& rBasicCellInfo = pThisRowInfo->basicCellInfo(nX);
            if (!rBasicCellInfo.bEmptyCellText)
                if (rCellInfo.pPatternAttr->
                            GetItem(ATTR_PROTECTION, rCellInfo.pConditionSet).GetHidePrint())
                {
                    rCellInfo.maCell.clear();
                    rBasicCellInfo.bEmptyCellText = true;
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

void ScPrintFunc::DrawToDev(ScDocument& rDoc, OutputDevice* pDev, double /* nPrintFactor */,
                            const tools::Rectangle& rBound, ScViewData* pViewData, bool bMetaFile)
{
    if (rDoc.GetMaxTableNumber() < 0)
        return;

    //! evaluate nPrintFactor !!!

    SCTAB nTab = 0;
    if (pViewData)
        nTab = pViewData->GetTabNo();

    bool bDoGrid, bNullVal, bFormula;
    ScStyleSheetPool* pStylePool = rDoc.GetStyleSheetPool();
    SfxStyleSheetBase* pStyleSheet = pStylePool->Find( rDoc.GetPageStyle( nTab ), SfxStyleFamily::Page );
    if (pStyleSheet)
    {
        SfxItemSet& rSet = pStyleSheet->GetItemSet();
        bDoGrid  = rSet.Get(ATTR_PAGE_GRID).GetValue();
        bNullVal = rSet.Get(ATTR_PAGE_NULLVALS).GetValue();
        bFormula = rSet.Get(ATTR_PAGE_FORMULAS).GetValue();
    }
    else
    {
        const ScViewOptions& rOpt = rDoc.GetViewOptions();
        bDoGrid  = rOpt.GetOption(VOPT_GRID);
        bNullVal = rOpt.GetOption(VOPT_NULLVALS);
        bFormula = rOpt.GetOption(VOPT_FORMULAS);
    }

    MapMode aMode = pDev->GetMapMode();

    tools::Rectangle aRect = rBound;

    if (aRect.Right() < aRect.Left() || aRect.Bottom() < aRect.Top())
        aRect = tools::Rectangle( Point(), pDev->GetOutputSize() );

    SCCOL nX1 = 0;
    SCROW nY1 = 0;
    SCCOL nX2 = OLE_STD_CELLS_X - 1;
    SCROW nY2 = OLE_STD_CELLS_Y - 1;
    if (bMetaFile)
    {
        ScRange aRange = rDoc.GetRange( nTab, rBound );
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

    if (nX1 > rDoc.MaxCol()) nX1 = rDoc.MaxCol();
    if (nX2 > rDoc.MaxCol()) nX2 = rDoc.MaxCol();
    if (nY1 > rDoc.MaxRow()) nY1 = rDoc.MaxRow();
    if (nY2 > rDoc.MaxRow()) nY2 = rDoc.MaxRow();

    tools::Long nDevSizeX = aRect.Right()-aRect.Left()+1;
    tools::Long nDevSizeY = aRect.Bottom()-aRect.Top()+1;

    tools::Long nTwipsSizeX = 0;
    for (SCCOL i=nX1; i<=nX2; i++)
        nTwipsSizeX += rDoc.GetColWidth( i, nTab );
    tools::Long nTwipsSizeY = rDoc.GetRowHeight( nY1, nY2, nTab );

    //  if no lines, still space for the outline frame (20 Twips = 1pt)
    //  (HasLines initializes aLines to 0,0,0,0)
    nTwipsSizeX += 20;
    nTwipsSizeY += 20;

    double nScaleX = static_cast<double>(nDevSizeX) / nTwipsSizeX;
    double nScaleY = static_cast<double>(nDevSizeY) / nTwipsSizeY;

                            //!     hand over Flag at FillInfo !!!!!
    ScRange aERange;
    bool bEmbed = rDoc.IsEmbedded();
    if (bEmbed)
    {
        rDoc.GetEmbedded(aERange);
        rDoc.ResetEmbedded();
    }

    //  Assemble data

    ScTableInfo aTabInfo(nY1, nY2, true);
    rDoc.FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nTab,
                   nScaleX, nScaleY, false, bFormula );
    lcl_HidePrint( aTabInfo, nX1, nX2 );

    if (bEmbed)
        rDoc.SetEmbedded(aERange);

    tools::Long nScrX = aRect.Left();
    tools::Long nScrY = aRect.Top();

    //  If no lines, still leave space for grid lines
    //  (would be elseways cut away)
    nScrX += 1;
    nScrY += 1;

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, aTabInfo, &rDoc, nTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nScaleX, nScaleY );
    aOutputData.SetMetaFileMode(bMetaFile);
    aOutputData.SetShowNullValues(bNullVal);
    aOutputData.SetShowFormulas(bFormula);

    vcl::PDFExtOutDevData* pPDF = dynamic_cast<vcl::PDFExtOutDevData*>(pDev->GetExtOutDevData());
    bool bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF();
    if (bTaggedPDF)
    {
        bool bReopen = aOutputData.ReopenPDFStructureElement(vcl::PDFWriter::Part);
        if (!bReopen)
        {
            sal_Int32 nId = pPDF->EnsureStructureElement(nullptr);
            pPDF->InitStructureElement(nId, vcl::PDFWriter::Part, "Worksheet");
            pPDF->BeginStructureElement(nId);
            pPDF->GetScPDFState()->m_WorksheetId = nId;
        }
    }

    ScDrawLayer* pModel = rDoc.GetDrawLayer();
    std::unique_ptr<FmFormView> pDrawView;

    if( pModel )
    {
        pDrawView.reset(
            new FmFormView(
                *pModel,
                pDev));
        pDrawView->ShowSdrPage(pDrawView->GetModel().GetPage(nTab));
        pDrawView->SetPrintPreview();
        aOutputData.SetDrawView( pDrawView.get() );
    }

    //! SetUseStyleColor ??

    if ( bMetaFile && pDev->IsVirtual() )
        aOutputData.SetSnapPixel();

    Point aLogStart = pDev->PixelToLogic(Point(nScrX, nScrY), MapMode(MapUnit::Map100thMM));
    tools::Long nLogStX = aLogStart.X();
    tools::Long nLogStY = aLogStart.Y();

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
    aOutputData.DrawSparklines(*pDev);
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
        tools::Long nRight = nScrX + aOutputData.GetScrW() - aOne.Width();
        tools::Long nBottom = nScrY + aOutputData.GetScrH() - aOne.Height();

        bool bLayoutRTL = rDoc.IsLayoutRTL( nTab );

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

    if (bTaggedPDF)
        pPDF->EndStructureElement();

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
        rParam.bEnable  = pHFSet->Get(ATTR_PAGE_ON).GetValue();
        rParam.bDynamic = pHFSet->Get(ATTR_PAGE_DYNAMIC).GetValue();
        rParam.bShared  = pHFSet->Get(ATTR_PAGE_SHARED).GetValue();
        rParam.bSharedFirst = pHFSet->Get(ATTR_PAGE_SHARED_FIRST).GetValue();
        rParam.nHeight  = pHFSet->Get(ATTR_PAGE_SIZE).GetSize().Height();
        const SvxLRSpaceItem* pHFLR = &pHFSet->Get(ATTR_LRSPACE);
        tools::Long nTmp;
        nTmp = pHFLR->GetLeft();
        rParam.nLeft = nTmp < 0 ? 0 : sal_uInt16(nTmp);
        nTmp = pHFLR->GetRight();
        rParam.nRight = nTmp < 0 ? 0 : sal_uInt16(nTmp);
        rParam.pBorder  = &pHFSet->Get(ATTR_BORDER);
        rParam.pBack    = &pHFSet->Get(ATTR_BACKGROUND);
        rParam.pShadow  = &pHFSet->Get(ATTR_SHADOW);

//   now back in the dialog:
//      rParam.nHeight += rParam.nDistance;             // not in the dialog any more ???

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
        if (!rDoc.GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes ) && aTableParam.bSkipEmpty)
            return false;   // nothing
        bPrintAreaValid = true;
    }
    else
    {
        bool bFound = true;
        bChangeCol = ( nStartCol == 0 && nEndCol == rDoc.MaxCol() );
        bChangeRow = ( nStartRow == 0 && nEndRow == rDoc.MaxRow() );
        bool bForcedChangeRow = false;

        // #i53558# Crop entire column of old row limit to real print area with
        // some fuzzyness.
        if (!bChangeRow && nStartRow == 0)
        {
            SCROW nPAEndRow;
            bFound = rDoc.GetPrintAreaVer( nPrintTab, nStartCol, nEndCol, nPAEndRow, bNotes );
            // Say we don't want to print more than ~1000 empty rows, which are
            // about 14 pages intentionally left blank...
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
            bFound = rDoc.GetPrintArea( nPrintTab, nEndCol, nEndRow, bNotes );
        else if ( bChangeCol )
            bFound = rDoc.GetPrintAreaHor( nPrintTab, nStartRow, nEndRow, nEndCol );
        else if ( bChangeRow )
            bFound = rDoc.GetPrintAreaVer( nPrintTab, nStartCol, nEndCol, nEndRow, bNotes );

        if (!bFound)
            return false;   // empty

        bPrintAreaValid = true;
        if (bForcedChangeRow)
            bChangeRow = true;
    }

    assert( bPrintAreaValid );
    rDoc.ExtendMerge( nStartCol,nStartRow, nEndCol,nEndRow, nPrintTab );  // no Refresh, incl. Attrs

    if ( bChangeCol )
    {
        OutputDevice* pRefDev = rDoc.GetPrinter();     // use the printer also for Preview
        pRefDev->SetMapMode(MapMode(MapUnit::MapPixel)); // important for GetNeededSize

        rDoc.ExtendPrintArea( pRefDev,
                            nPrintTab, nStartCol, nStartRow, nEndCol, nEndRow );
        //  changing nEndCol
    }

    if ( nEndCol < rDoc.MaxCol() && rDoc.HasAttrib(
                    nEndCol,nStartRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HasAttrFlags::ShadowRight ) )
        ++nEndCol;
    if ( nEndRow < rDoc.MaxRow() && rDoc.HasAttrib(
                    nStartCol,nEndRow,nPrintTab, nEndCol,nEndRow,nPrintTab, HasAttrFlags::ShadowDown ) )
        ++nEndRow;

    if (!bChangeCol) nEndCol = nOldEndCol;
    if (!bChangeRow) nEndRow = nOldEndRow;

    return true;
}

tools::Long ScPrintFunc::TextHeight( const EditTextObject* pObject )
{
    if (!pObject)
        return 0;

    pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );

    return static_cast<tools::Long>(pEditEngine->GetTextHeight());
}

//  nZoom must be set !!!
//  and the respective Twip-MapMode configured

void ScPrintFunc::UpdateHFHeight( ScPrintHFParam& rParam )
{
    OSL_ENSURE( aPageSize.Width(), "UpdateHFHeight without aPageSize");

    if (!(rParam.bEnable && rParam.bDynamic))
        return;

    //  calculate nHeight from content

    MakeEditEngine();
    tools::Long nPaperWidth = ( aPageSize.Width() - nLeftMargin - nRightMargin -
                            rParam.nLeft - rParam.nRight ) * 100 / nZoom;
    if (rParam.pBorder)
        nPaperWidth -= ( rParam.pBorder->GetDistance(SvxBoxItemLine::LEFT) +
                         rParam.pBorder->GetDistance(SvxBoxItemLine::RIGHT) +
                         lcl_LineTotal(rParam.pBorder->GetLeft()) +
                         lcl_LineTotal(rParam.pBorder->GetRight()) ) * 100 / nZoom;

    if (rParam.pShadow && rParam.pShadow->GetLocation() != SvxShadowLocation::NONE)
        nPaperWidth -= ( rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::LEFT) +
                         rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::RIGHT) ) * 100 / nZoom;

    pEditEngine->SetPaperSize( Size( nPaperWidth, 10000 ) );

    tools::Long nMaxHeight = 0;
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
    if ( rParam.pFirst )
    {
        nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pFirst->GetLeftArea() ) );
        nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pFirst->GetCenterArea() ) );
        nMaxHeight = std::max( nMaxHeight, TextHeight( rParam.pFirst->GetRightArea() ) );
    }

    rParam.nHeight = nMaxHeight + rParam.nDistance;
    if (rParam.pBorder)
        rParam.nHeight += rParam.pBorder->GetDistance(SvxBoxItemLine::TOP) +
                          rParam.pBorder->GetDistance(SvxBoxItemLine::BOTTOM) +
                          lcl_LineTotal( rParam.pBorder->GetTop() ) +
                          lcl_LineTotal( rParam.pBorder->GetBottom() );
    if (rParam.pShadow && rParam.pShadow->GetLocation() != SvxShadowLocation::NONE)
        rParam.nHeight += rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::TOP) +
                          rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM);

    if (rParam.nHeight < rParam.nManHeight)
        rParam.nHeight = rParam.nManHeight;         // configured minimum
}

void ScPrintFunc::InitParam( const ScPrintOptions* pOptions )
{
    if (!pParamSet)
        return;

                                // TabPage "Page"
    const SvxLRSpaceItem* pLRItem = &pParamSet->Get( ATTR_LRSPACE );
    tools::Long nTmp;
    nTmp = pLRItem->GetLeft();
    nLeftMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
    nTmp = pLRItem->GetRight();
    nRightMargin = nTmp < 0 ? 0 : sal_uInt16(nTmp);
    const SvxULSpaceItem* pULItem = &pParamSet->Get( ATTR_ULSPACE );
    nTopMargin    = pULItem->GetUpper();
    nBottomMargin = pULItem->GetLower();

    const SvxPageItem* pPageItem = &pParamSet->Get( ATTR_PAGE );
    nPageUsage          = pPageItem->GetPageUsage();
    bLandscape          = bUsePrintDialogSetting ? bPrintPageLandscape : pPageItem->IsLandscape();
    aFieldData.eNumType = pPageItem->GetNumType();

    bCenterHor = pParamSet->Get(ATTR_PAGE_HORCENTER).GetValue();
    bCenterVer = pParamSet->Get(ATTR_PAGE_VERCENTER).GetValue();

    aPageSize = bUsePrintDialogSetting ? aPrintPageSize : pParamSet->Get(ATTR_PAGE_SIZE).GetSize();
    if ( !aPageSize.Width() || !aPageSize.Height() )
    {
        OSL_FAIL("PageSize Null ?!?!?");
        aPageSize = SvxPaperInfo::GetPaperSize( PAPER_A4 );
    }

    pBorderItem     = &pParamSet->Get(ATTR_BORDER);
    pBackgroundItem = &pParamSet->Get(ATTR_BACKGROUND);
    pShadowItem     = &pParamSet->Get(ATTR_SHADOW);

                                // TabPage "Headline"

    aHdr.pLeft      = &pParamSet->Get(ATTR_PAGE_HEADERLEFT);      // Content
    aHdr.pRight     = &pParamSet->Get(ATTR_PAGE_HEADERRIGHT);
    aHdr.pFirst     = &pParamSet->Get(ATTR_PAGE_HEADERFIRST);

    const SfxItemSet* pHeaderSet = nullptr;
    if ( const SvxSetItem* pHeaderSetItem = pParamSet->GetItemIfSet( ATTR_PAGE_HEADERSET, false ) )
    {
        pHeaderSet = &pHeaderSetItem->GetItemSet();
                                                        // Headline has space below
        aHdr.nDistance  = pHeaderSet->Get(ATTR_ULSPACE).GetLower();
    }
    lcl_FillHFParam( aHdr, pHeaderSet );

                                // TabPage "Footline"

    aFtr.pLeft      = &pParamSet->Get(ATTR_PAGE_FOOTERLEFT);      // Content
    aFtr.pRight     = &pParamSet->Get(ATTR_PAGE_FOOTERRIGHT);
    aFtr.pFirst     = &pParamSet->Get(ATTR_PAGE_FOOTERFIRST);

    const SfxItemSet* pFooterSet = nullptr;
    if ( const SvxSetItem* pFooterSetItem = pParamSet->GetItemIfSet( ATTR_PAGE_FOOTERSET, false ) )
    {
        pFooterSet = &pFooterSetItem->GetItemSet();
                                                        // Footline has space above
        aFtr.nDistance  = pFooterSet->Get(ATTR_ULSPACE).GetUpper();
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
        pScaleItem = &pParamSet->GetPool()->GetUserOrPoolDefaultItem( ATTR_PAGE_SCALE );

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALETO, false,
                                      reinterpret_cast<const SfxPoolItem**>(&pScaleToItem) );
    if ( SfxItemState::DEFAULT == eState )
        pScaleToItem = &pParamSet->GetPool()->GetUserOrPoolDefaultItem( ATTR_PAGE_SCALETO );

    eState = pParamSet->GetItemState( ATTR_PAGE_SCALETOPAGES, false,
                                      reinterpret_cast<const SfxPoolItem**>(&pScaleToPagesItem) );
    if ( SfxItemState::DEFAULT == eState )
        pScaleToPagesItem = &pParamSet->GetPool()->GetUserOrPoolDefaultItem( ATTR_PAGE_SCALETOPAGES );

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
        aTableParam.nFirstPageNo = static_cast<sal_uInt16>(nPageStart);     // from previous table

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
    const ScRange*  pPrintArea = rDoc.GetPrintRange( nPrintTab, 0 );
    std::optional<ScRange> oRepeatCol = rDoc.GetRepeatColRange( nPrintTab );
    std::optional<ScRange> oRepeatRow = rDoc.GetRepeatRowRange( nPrintTab );

    //  ignoring ATTR_PAGE_PRINTTABLES

    bool bHasPrintRange = rDoc.HasPrintRange();
    sal_uInt16 nPrintRangeCount = rDoc.GetPrintRangeCount(nPrintTab);
    bool bPrintEntireSheet = rDoc.IsPrintEntireSheet(nPrintTab);

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
            bPrintCurrentTable = rDoc.IsPrintEntireSheet( nPrintTab ) && rDoc.IsVisible( nPrintTab );
            aAreaParam.bPrintArea = !bPrintCurrentTable;    // otherwise the table is always counted
        }
    }
    else
    {
        //  don't print hidden tables if there's no print range defined there
        if ( rDoc.IsVisible( nPrintTab ) )
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

    if ( oRepeatCol )
    {
        aAreaParam.bRepeatCol = true;
        nRepeatStartCol = oRepeatCol->aStart.Col();
        nRepeatEndCol   = oRepeatCol->aEnd  .Col();
    }
    else
    {
        aAreaParam.bRepeatCol = false;
        nRepeatStartCol = nRepeatEndCol = SCCOL_REPEAT_NONE;
    }

    if ( oRepeatRow )
    {
        aAreaParam.bRepeatRow = true;
        nRepeatStartRow = oRepeatRow->aStart.Row();
        nRepeatEndRow   = oRepeatRow->aEnd  .Row();
    }
    else
    {
        aAreaParam.bRepeatRow = false;
        nRepeatStartRow = nRepeatEndRow = SCROW_REPEAT_NONE;
    }

            //  Split pages

    if (!bPrintAreaValid)
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

    SetDateTime( DateTime( DateTime::SYSTEM ) );

    if( pDocShell->getDocProperties()->getTitle().getLength() != 0 )
        aFieldData.aTitle = pDocShell->getDocProperties()->getTitle();
    else
        aFieldData.aTitle = pDocShell->GetTitle();

    const INetURLObject& rURLObj = pDocShell->GetMedium()->GetURLObject();
    aFieldData.aLongDocName = rURLObj.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
    if ( !aFieldData.aLongDocName.isEmpty() )
        aFieldData.aShortDocName = rURLObj.GetLastName(INetURLObject::DecodeMechanism::Unambiguous);
    else
        aFieldData.aShortDocName = aFieldData.aLongDocName = aFieldData.aTitle;

    //  Printer settings (Orientation, Paper) at DoPrint
}

Size ScPrintFunc::GetDataSize() const
{
    Size aSize = aPageSize;
    aSize.AdjustWidth( -(nLeftMargin + nRightMargin) );
    aSize.AdjustHeight( -(nTopMargin + nBottomMargin) );
    aSize.AdjustHeight( -(aHdr.nHeight + aFtr.nHeight) );
    return aSize;
}

void ScPrintFunc::GetScaleData( Size& rPhysSize, tools::Long& rDocHdr, tools::Long& rDocFtr )
{
    rPhysSize = aPageSize;
    rPhysSize.AdjustWidth( -(nLeftMargin + nRightMargin) );
    rPhysSize.AdjustHeight( -(nTopMargin + nBottomMargin) );

    rDocHdr = aHdr.nHeight;
    rDocFtr = aFtr.nHeight;
}

void ScPrintFunc::SetDateTime( const DateTime& rDateTime )
{
    aFieldData.aDateTime = rDateTime;
}

static void lcl_DrawGraphic( const Graphic &rGraphic, vcl::RenderContext& rOutDev,
                      const tools::Rectangle &rGrf, const tools::Rectangle &rOut )
{
    const bool bNotInside = !rOut.Contains( rGrf );
    if ( bNotInside )
    {
        rOutDev.Push();
        rOutDev.IntersectClipRegion( rOut );
    }

    rGraphic.Draw(rOutDev, rGrf.TopLeft(), rGrf.GetSize());

    if ( bNotInside )
        rOutDev.Pop();
}

static void lcl_DrawGraphic( const SvxBrushItem &rBrush, vcl::RenderContext& rOutDev, const OutputDevice* pRefDev,
                        const tools::Rectangle &rOrg, const tools::Rectangle &rOut,
                        OUString const & referer )
{
    Size aGrfSize(0,0);
    const Graphic *pGraphic = rBrush.GetGraphic(referer);
    SvxGraphicPosition ePos;
    if ( pGraphic && pGraphic->IsSupportedGraphic() )
    {
        const MapMode aMapMM( MapUnit::Map100thMM );
        if ( pGraphic->GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
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
        case GPOS_MT: aPos.setY( rOrg.Top() );
                      aPos.setX( rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2 );
                      break;
        case GPOS_RT: aPos.setY( rOrg.Top() );
                      aPos.setX( rOrg.Right() - aGrfSize.Width() );
                      break;

        case GPOS_LM: aPos.setY( rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2 );
                      aPos.setX( rOrg.Left() );
                      break;
        case GPOS_MM: aPos.setY( rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2 );
                      aPos.setX( rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2 );
                      break;
        case GPOS_RM: aPos.setY( rOrg.Top() + rOrg.GetSize().Height()/2 - aGrfSize.Height()/2 );
                      aPos.setX( rOrg.Right() - aGrfSize.Width() );
                      break;

        case GPOS_LB: aPos.setY( rOrg.Bottom() - aGrfSize.Height() );
                      aPos.setX( rOrg.Left() );
                      break;
        case GPOS_MB: aPos.setY( rOrg.Bottom() - aGrfSize.Height() );
                      aPos.setX( rOrg.Left() + rOrg.GetSize().Width()/2 - aGrfSize.Width()/2 );
                      break;
        case GPOS_RB: aPos.setY( rOrg.Bottom() - aGrfSize.Height() );
                      aPos.setX( rOrg.Right() - aGrfSize.Width() );
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

                        if( rOutDev.GetOutDevType() == OUTDEV_PDF &&
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

                            aObject.DrawTiled( rOutDev, rOrg, aGrfSize, Size(0,0),
                                               ::std::max( 128, static_cast<int>( sqrt(sqrt( Abitmap)) + .5 ) ) );
                        }
                        else
                        {
                            aObject.DrawTiled( rOutDev, rOrg, aGrfSize, Size(0,0) );
                        }

                        bDraw = false;
                    }
                    break;

        case GPOS_NONE:
                      bDraw = false;
                      break;

        default: OSL_ENSURE( false, "new Graphic position?" );
    }
    tools::Rectangle aGrf( aPos,aDrawSize );
    if ( bDraw && aGrf.Overlaps( rOut ) )
    {
        lcl_DrawGraphic( *pGraphic, rOutDev, aGrf, rOut );
    }
}

// The frame is drawn inwards

void ScPrintFunc::DrawBorder( tools::Long nScrX, tools::Long nScrY, tools::Long nScrW, tools::Long nScrH,
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

    tools::Long nLeft   = 0;
    tools::Long nRight  = 0;
    tools::Long nTop    = 0;
    tools::Long nBottom = 0;

    //  aFrameRect - outside around frame, without shadow
    if ( pShadow && pShadow->GetLocation() != SvxShadowLocation::NONE )
    {
        nLeft   += static_cast<tools::Long>( pShadow->CalcShadowSpace(SvxShadowItemSide::LEFT)   * nScaleX );
        nRight  += static_cast<tools::Long>( pShadow->CalcShadowSpace(SvxShadowItemSide::RIGHT)  * nScaleX );
        nTop    += static_cast<tools::Long>( pShadow->CalcShadowSpace(SvxShadowItemSide::TOP)    * nScaleY );
        nBottom += static_cast<tools::Long>( pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM) * nScaleY );
    }
    tools::Rectangle aFrameRect( Point(nScrX+nLeft, nScrY+nTop),
                          Size(nScrW-nLeft-nRight, nScrH-nTop-nBottom) );

    //  center of frame, to paint lines through OutputData
    if (pBorderData)
    {
        nLeft   += static_cast<tools::Long>( lcl_LineTotal(pBorderData->GetLeft())   * nScaleX / 2 );
        nRight  += static_cast<tools::Long>( lcl_LineTotal(pBorderData->GetRight())  * nScaleX / 2 );
        nTop    += static_cast<tools::Long>( lcl_LineTotal(pBorderData->GetTop())    * nScaleY / 2 );
        nBottom += static_cast<tools::Long>( lcl_LineTotal(pBorderData->GetBottom()) * nScaleY / 2 );
    }
    tools::Long nEffHeight = nScrH - nTop - nBottom;
    tools::Long nEffWidth = nScrW - nLeft - nRight;
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
                pRefDev = rDoc.GetPrinter();   // use printer also for preview
            OUString referer;
            if (pDocShell->HasName()) {
                referer = pDocShell->GetMedium()->GetName();
            }
            lcl_DrawGraphic(*pBackground, *pDev, pRefDev, aFrameRect, aFrameRect, referer);
        }
        else
        {
            pDev->SetFillColor(pBackground->GetColor());
            pDev->SetLineColor();
            pDev->DrawRect(aFrameRect);
        }
    }

    if ( pShadow && pShadow->GetLocation() != SvxShadowLocation::NONE )
    {
        pDev->SetFillColor(pShadow->GetColor());
        pDev->SetLineColor();
        tools::Long nShadowX = static_cast<tools::Long>( pShadow->GetWidth() * nScaleX );
        tools::Long nShadowY = static_cast<tools::Long>( pShadow->GetWidth() * nScaleY );
        switch (pShadow->GetLocation())
        {
            case SvxShadowLocation::TopLeft:
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()-nShadowX, aFrameRect.Top() ) );
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Left(), aFrameRect.Bottom()-nShadowY ) );
                break;
            case SvxShadowLocation::TopRight:
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Left()+nShadowX, aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Top() ) );
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Right(), aFrameRect.Top()-nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()-nShadowY ) );
                break;
            case SvxShadowLocation::BottomLeft:
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Bottom(),
                        aFrameRect.Right()-nShadowX, aFrameRect.Bottom()+nShadowY ) );
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Left()-nShadowX, aFrameRect.Top()+nShadowY,
                        aFrameRect.Left(), aFrameRect.Bottom()+nShadowY ) );
                break;
            case SvxShadowLocation::BottomRight:
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Left()+nShadowX, aFrameRect.Bottom(),
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()+nShadowY ) );
                pDev->DrawRect( tools::Rectangle(
                        aFrameRect.Right(), aFrameRect.Top()+nShadowY,
                        aFrameRect.Right()+nShadowX, aFrameRect.Bottom()+nShadowY ) );
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    if (!pBorderData)
        return;

    ScDocumentUniquePtr pBorderDoc(new ScDocument( SCDOCMODE_UNDO ));
    pBorderDoc->InitUndo( rDoc, 0,0, true,true );
    pBorderDoc->ApplyAttr( 0,0,0, *pBorderData );

    ScTableInfo aTabInfo(0, 1, false);
    pBorderDoc->FillInfo( aTabInfo, 0,0, 0,0, 0,
                                        nScaleX, nScaleY, false, false );
    OSL_ENSURE(aTabInfo.mnArrCount,"nArrCount == 0");

    aTabInfo.mpRowInfo[1].nHeight = static_cast<sal_uInt16>(nEffHeight);
    aTabInfo.mpRowInfo[0].basicCellInfo(0).nWidth =
        aTabInfo.mpRowInfo[1].basicCellInfo(0).nWidth = static_cast<sal_uInt16>(nEffWidth);

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, aTabInfo, pBorderDoc.get(), 0,
                                nScrX+nLeft, nScrY+nTop, 0,0, 0,0, nScaleX, nScaleY );
    aOutputData.SetUseStyleColor( bUseStyleColor );

    aOutputData.DrawFrame(*pDev);
}

void ScPrintFunc::PrintColHdr( SCCOL nX1, SCCOL nX2, tools::Long nScrX, tools::Long nScrY )
{
    bool bLayoutRTL = rDoc.IsLayoutRTL( nPrintTab );
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    tools::Long nOneX = aOnePixel.Width();
    tools::Long nOneY = aOnePixel.Height();
    SCCOL nCol;

    tools::Long nHeight = static_cast<tools::Long>(PRINT_HEADER_HEIGHT * nScaleY);
    tools::Long nEndY = nScrY + nHeight - nOneY;

    tools::Long nPosX = nScrX;
    if ( bLayoutRTL )
    {
        for (nCol=nX1; nCol<=nX2; nCol++)
            nPosX += static_cast<tools::Long>( rDoc.GetColWidth( nCol, nPrintTab ) * nScaleX );
    }
    else
        nPosX -= nOneX;
    tools::Long nPosY = nScrY - nOneY;
    OUString aText;

    for (nCol=nX1; nCol<=nX2; nCol++)
    {
        sal_uInt16 nDocW = rDoc.GetColWidth( nCol, nPrintTab );
        if (nDocW)
        {
            tools::Long nWidth = static_cast<tools::Long>(nDocW * nScaleX);
            tools::Long nEndX = nPosX + nWidth * nLayoutSign;

            pDev->DrawRect( tools::Rectangle( nPosX,nPosY,nEndX,nEndY ) );

            aText = ::ScColToAlpha( nCol);
            tools::Long nTextWidth = pDev->GetTextWidth(aText);
            tools::Long nTextHeight = pDev->GetTextHeight();
            tools::Long nAddX = ( nWidth  - nTextWidth  ) / 2;
            tools::Long nAddY = ( nHeight - nTextHeight ) / 2;
            tools::Long nTextPosX = nPosX+nAddX;
            if ( bLayoutRTL )
                nTextPosX -= nWidth;
            pDev->DrawText( Point( nTextPosX,nPosY+nAddY ), aText );

            nPosX = nEndX;
        }
    }
}

void ScPrintFunc::PrintRowHdr( SCROW nY1, SCROW nY2, tools::Long nScrX, tools::Long nScrY )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    tools::Long nOneX = aOnePixel.Width();
    tools::Long nOneY = aOnePixel.Height();

    bool bLayoutRTL = rDoc.IsLayoutRTL( nPrintTab );

    tools::Long nWidth = static_cast<tools::Long>(PRINT_HEADER_WIDTH * nScaleX);
    tools::Long nEndX = nScrX + nWidth;
    tools::Long nPosX = nScrX;
    if ( !bLayoutRTL )
    {
        nEndX -= nOneX;
        nPosX -= nOneX;
    }
    tools::Long nPosY = nScrY - nOneY;
    OUString aText;

    for (SCROW nRow=nY1; nRow<=nY2; nRow++)
    {
        sal_uInt16 nDocH = rDoc.GetRowHeight( nRow, nPrintTab );
        if (nDocH)
        {
            tools::Long nHeight = static_cast<tools::Long>(nDocH * nScaleY);
            tools::Long nEndY = nPosY + nHeight;

            pDev->DrawRect( tools::Rectangle( nPosX,nPosY,nEndX,nEndY ) );

            aText = OUString::number( nRow+1 );
            tools::Long nTextWidth = pDev->GetTextWidth(aText);
            tools::Long nTextHeight = pDev->GetTextHeight();
            tools::Long nAddX = ( nWidth  - nTextWidth  ) / 2;
            tools::Long nAddY = ( nHeight - nTextHeight ) / 2;
            pDev->DrawText( Point( nPosX+nAddX,nPosY+nAddY ), aText );

            nPosY = nEndY;
        }
    }
}

void ScPrintFunc::LocateColHdr( SCCOL nX1, SCCOL nX2, tools::Long nScrX, tools::Long nScrY,
                                bool bRepCol, ScPreviewLocationData& rLocationData )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    tools::Long nOneX = aOnePixel.Width();
    tools::Long nOneY = aOnePixel.Height();

    tools::Long nHeight = static_cast<tools::Long>(PRINT_HEADER_HEIGHT * nScaleY);
    tools::Long nEndY = nScrY + nHeight - nOneY;

    tools::Long nPosX = nScrX - nOneX;
    for (SCCOL nCol=nX1; nCol<=nX2; nCol++)
    {
        sal_uInt16 nDocW = rDoc.GetColWidth( nCol, nPrintTab );
        if (nDocW)
            nPosX += static_cast<tools::Long>(nDocW * nScaleX);
    }
    tools::Rectangle aCellRect( nScrX, nScrY, nPosX, nEndY );
    rLocationData.AddColHeaders( aCellRect, nX1, nX2, bRepCol );
}

void ScPrintFunc::LocateRowHdr( SCROW nY1, SCROW nY2, tools::Long nScrX, tools::Long nScrY,
                                bool bRepRow, ScPreviewLocationData& rLocationData )
{
    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    tools::Long nOneX = aOnePixel.Width();
    tools::Long nOneY = aOnePixel.Height();

    bool bLayoutRTL = rDoc.IsLayoutRTL( nPrintTab );

    tools::Long nWidth = static_cast<tools::Long>(PRINT_HEADER_WIDTH * nScaleX);
    tools::Long nEndX = nScrX + nWidth;
    if ( !bLayoutRTL )
        nEndX -= nOneX;

    tools::Long nPosY = nScrY - nOneY;
    nPosY += rDoc.GetScaledRowHeight( nY1, nY2, nPrintTab, nScaleY);
    tools::Rectangle aCellRect( nScrX, nScrY, nEndX, nPosY );
    rLocationData.AddRowHeaders( aCellRect, nY1, nY2, bRepRow );
}

void ScPrintFunc::LocateArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                tools::Long nScrX, tools::Long nScrY, bool bRepCol, bool bRepRow,
                                ScPreviewLocationData& rLocationData )
{
    //  get MapMode for drawing objects (same MapMode as in ScOutputData::PrintDrawingLayer)

    Point aLogPos = OutputDevice::LogicToLogic(Point(nScrX,nScrY), aOffsetMode, aLogicMode);
    tools::Long nLogStX = aLogPos.X();
    tools::Long nLogStY = aLogPos.Y();

    SCCOL nCol;
    Point aTwipOffset;
    for (nCol=0; nCol<nX1; nCol++)
        aTwipOffset.AdjustX( -(rDoc.GetColWidth( nCol, nPrintTab )) );
    aTwipOffset.AdjustY( -sal_Int32(rDoc.GetRowHeight( 0, nY1-1, nPrintTab )) );

    Point aMMOffset(o3tl::convert(aTwipOffset, o3tl::Length::twip, o3tl::Length::mm100));
    aMMOffset += Point( nLogStX, nLogStY );
    MapMode aDrawMapMode( MapUnit::Map100thMM, aMMOffset, aLogicMode.GetScaleX(), aLogicMode.GetScaleY() );

    //  get pixel rectangle

    Size aOnePixel = pDev->PixelToLogic(Size(1,1));
    tools::Long nOneX = aOnePixel.Width();
    tools::Long nOneY = aOnePixel.Height();

    tools::Long nPosX = nScrX - nOneX;
    for (nCol=nX1; nCol<=nX2; nCol++)
    {
        sal_uInt16 nDocW = rDoc.GetColWidth( nCol, nPrintTab );
        if (nDocW)
            nPosX += static_cast<tools::Long>(nDocW * nScaleX);
    }

    tools::Long nPosY = nScrY - nOneY;
    nPosY += rDoc.GetScaledRowHeight( nY1, nY2, nPrintTab, nScaleY);
    tools::Rectangle aCellRect( nScrX, nScrY, nPosX, nPosY );
    rLocationData.AddCellRange( aCellRect, ScRange( nX1,nY1,nPrintTab, nX2,nY2,nPrintTab ),
                                bRepCol, bRepRow, aDrawMapMode );
}

void ScPrintFunc::PrintArea( SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                tools::Long nScrX, tools::Long nScrY,
                                bool bShLeft, bool bShTop, bool bShRight, bool bShBottom )
{
    // #i47547# nothing to do if the end of the print area is before the end of
    // the repeat columns/rows (don't use negative size for ScOutputData)
    if ( nX2 < nX1 || nY2 < nY1 )
        return;

                            //!     hand over Flag at FillInfo  !!!!!
    ScRange aERange;
    bool bEmbed = rDoc.IsEmbedded();
    if (bEmbed)
    {
        rDoc.GetEmbedded(aERange);
        rDoc.ResetEmbedded();
    }

    Point aPos = OutputDevice::LogicToLogic(Point(nScrX,nScrY), aOffsetMode, aLogicMode);
    tools::Long nLogStX = aPos.X();
    tools::Long nLogStY = aPos.Y();

                    //  Assemble data

    ScTableInfo aTabInfo(nY1, nY2, true);
    rDoc.FillInfo( aTabInfo, nX1, nY1, nX2, nY2, nPrintTab,
                                        nScaleX, nScaleY, true, aTableParam.bFormulas );
    lcl_HidePrint( aTabInfo, nX1, nX2 );

    if (bEmbed)
        rDoc.SetEmbedded(aERange);

    ScOutputData aOutputData( pDev, OUTTYPE_PRINTER, aTabInfo, &rDoc, nPrintTab,
                                nScrX, nScrY, nX1, nY1, nX2, nY2, nScaleX, nScaleY );

    vcl::PDFExtOutDevData* pPDF = dynamic_cast<vcl::PDFExtOutDevData*>(pDev->GetExtOutDevData());
    bool bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF();
    if (bTaggedPDF)
    {
        bool bReopen = aOutputData.ReopenPDFStructureElement(vcl::PDFWriter::Part);
        if (!bReopen)
        {
            sal_Int32 nId = pPDF->EnsureStructureElement(nullptr);
            pPDF->InitStructureElement(nId, vcl::PDFWriter::Part, "Worksheet");
            pPDF->BeginStructureElement(nId);
            pPDF->GetScPDFState()->m_WorksheetId = nId;
        }
    }

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
        aGridColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;
    aOutputData.SetGridColor( aGridColor );

    if ( !pPrinter )
    {
        OutputDevice* pRefDev = rDoc.GetPrinter();     // use the printer also for Preview
        Fraction aPrintFrac( nZoom, 100 );              // without nManualZoom
        //  MapMode, as it would arrive at the printer:
        pRefDev->SetMapMode( MapMode( MapUnit::Map100thMM, Point(), aPrintFrac, aPrintFrac ) );

        //  when rendering (PDF), don't use printer as ref device, but printer's MapMode
        //  has to be set anyway, as charts still use it (#106409#)
        if ( !bIsRender )
            aOutputData.SetRefDevice( pRefDev );
    }

    if( aTableParam.bCellContent )
        aOutputData.DrawBackground(*pDev);

    pDev->SetClipRegion(vcl::Region(tools::Rectangle(
                aPos, Size(aOutputData.GetScrW(), aOutputData.GetScrH()))));
    pDev->SetClipRegion();

    if( aTableParam.bCellContent )
    {
        aOutputData.DrawExtraShadow( bShLeft, bShTop, bShRight, bShBottom );
        aOutputData.DrawFrame(*pDev);
        aOutputData.DrawSparklines(*pDev);
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

    if (bTaggedPDF)
    {
        aOutputData.PrintDrawingLayer(SC_LAYER_CONTROLS, aMMOffset);
        pPDF->EndStructureElement();
    }

    // #i72502#
    aOutputData.PrintDrawingLayer(SC_LAYER_INTERN, aMMOffset);

    if (!bTaggedPDF)
        aOutputData.PostPrintDrawingLayer(aMMOffset); // #i74768#
}

bool ScPrintFunc::IsMirror( tools::Long nPageNo )          // Mirror margins?
{
    return nPageUsage == SvxPageUsage::Mirror && (nPageNo & 1);
}

bool ScPrintFunc::IsLeft( tools::Long nPageNo )            // left foot notes?
{
    bool bLeft;
    if (nPageUsage == SvxPageUsage::Left)
        bLeft = true;
    else if (nPageUsage == SvxPageUsage::Right)
        bLeft = false;
    else
        bLeft = (nPageNo & 1) != 0;
    return bLeft;
}

void ScPrintFunc::MakeTableString()
{
    OUString aTmp;
    rDoc.GetName(nPrintTab, aTmp);
    aFieldData.aTabName = aTmp;
}

void ScPrintFunc::MakeEditEngine()
{
    if (!pEditEngine)
    {
        //  can't use document's edit engine pool here,
        //  because pool must have twips as default metric
        pEditEngine.reset( new ScHeaderEditEngine( EditEngine::CreatePool().get() ) );

        pEditEngine->EnableUndo(false);
        //fdo#45869 we want text to be positioned as it would be for the
        //high dpi printed output, not as would be ideal for the 96dpi preview
        //window itself
        pEditEngine->SetRefDevice(pPrinter ? pPrinter : rDoc.GetRefDevice());
        pEditEngine->SetWordDelimiters(
                ScEditUtil::ModifyDelimiters( pEditEngine->GetWordDelimiters() ) );
        pEditEngine->SetControlWord( pEditEngine->GetControlWord() & ~EEControlBits::RTFSTYLESHEETS );
        rDoc.ApplyAsianEditSettings( *pEditEngine );
        pEditEngine->EnableAutoColor( bUseStyleColor );

        //  Default-Set for alignment
        pEditDefaults.reset( new SfxItemSet( pEditEngine->GetEmptyItemSet() ) );

        const ScPatternAttr& rPattern(rDoc.getCellAttributeHelper().getDefaultCellAttribute());

        rPattern.FillEditItemSet( pEditDefaults.get() );
        //  FillEditItemSet adjusts font height to 1/100th mm,
        //  but for header/footer twips is needed, as in the PatternAttr:
        pEditDefaults->Put( rPattern.GetItem(ATTR_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT) );
        pEditDefaults->Put( rPattern.GetItem(ATTR_CJK_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CJK) );
        pEditDefaults->Put( rPattern.GetItem(ATTR_CTL_FONT_HEIGHT).CloneSetWhich(EE_CHAR_FONTHEIGHT_CTL) );
        //  don't use font color, because background color is not used
        //! there's no way to set the background for note pages
        pEditDefaults->ClearItem( EE_CHAR_COLOR );
        if (ScGlobal::IsSystemRTL())
            pEditDefaults->Put( SvxFrameDirectionItem( SvxFrameDirection::Horizontal_RL_TB, EE_PARA_WRITINGDIR ) );
    }

    pEditEngine->SetData( aFieldData );     // Set page count etc.
}

//  nStartY = logic
void ScPrintFunc::PrintHF( tools::Long nPageNo, bool bHeader, tools::Long nStartY,
                            bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    vcl::PDFExtOutDevData* pPDF = dynamic_cast<vcl::PDFExtOutDevData*>(pDev->GetExtOutDevData());
    bool bTaggedPDF = pPDF && pPDF->GetIsExportTaggedPDF();
    if (bTaggedPDF)
        pPDF->WrapBeginStructureElement(vcl::PDFWriter::NonStructElement);

    const ScPrintHFParam& rParam = bHeader ? aHdr : aFtr;

    pDev->SetMapMode( aTwipMode );          // Head-/Footlines in Twips

    bool bFirst = 0 == nPageNo && !rParam.bSharedFirst;
    bool bLeft = IsLeft(nPageNo) && !rParam.bShared;
    const ScPageHFItem* pHFItem = bFirst ? rParam.pFirst : (bLeft ? rParam.pLeft : rParam.pRight);

    tools::Long nLineStartX = aPageRect.Left()  + rParam.nLeft;
    tools::Long nLineEndX   = aPageRect.Right() - rParam.nRight;
    tools::Long nLineWidth  = nLineEndX - nLineStartX + 1;

    //  Edit-Engine

    Point aStart( nLineStartX, nStartY );
    Size aPaperSize( nLineWidth, rParam.nHeight-rParam.nDistance );
    if ( rParam.pBorder )
    {
        tools::Long nLeft = lcl_LineTotal( rParam.pBorder->GetLeft() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::LEFT);
        tools::Long nTop = lcl_LineTotal( rParam.pBorder->GetTop() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::TOP);
        aStart.AdjustX(nLeft );
        aStart.AdjustY(nTop );
        aPaperSize.AdjustWidth( -(nLeft + lcl_LineTotal( rParam.pBorder->GetRight() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::RIGHT)) );
        aPaperSize.AdjustHeight( -(nTop + lcl_LineTotal( rParam.pBorder->GetBottom() ) + rParam.pBorder->GetDistance(SvxBoxItemLine::BOTTOM)) );
    }

    if ( rParam.pShadow && rParam.pShadow->GetLocation() != SvxShadowLocation::NONE )
    {
        tools::Long nLeft  = rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::LEFT);
        tools::Long nTop   = rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::TOP);
        aStart.AdjustX(nLeft );
        aStart.AdjustY(nTop );
        aPaperSize.AdjustWidth( -(nLeft + rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::RIGHT)) );
        aPaperSize.AdjustHeight( -(nTop + rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM)) );
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

        tools::Long nMaxHeight = 0;
        nMaxHeight = std::max( nMaxHeight, TextHeight( pHFItem->GetLeftArea() ) );
        nMaxHeight = std::max( nMaxHeight, TextHeight( pHFItem->GetCenterArea() ) );
        nMaxHeight = std::max( nMaxHeight, TextHeight( pHFItem->GetRightArea() ) );
        if (rParam.pBorder)
            nMaxHeight += lcl_LineTotal( rParam.pBorder->GetTop() ) +
                          lcl_LineTotal( rParam.pBorder->GetBottom() ) +
                                    rParam.pBorder->GetDistance(SvxBoxItemLine::TOP) +
                                    rParam.pBorder->GetDistance(SvxBoxItemLine::BOTTOM);
        if (rParam.pShadow && rParam.pShadow->GetLocation() != SvxShadowLocation::NONE)
            nMaxHeight += rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::TOP) +
                          rParam.pShadow->CalcShadowSpace(SvxShadowItemSide::BOTTOM);

        if (nMaxHeight < rParam.nManHeight-rParam.nDistance)
            nMaxHeight = rParam.nManHeight-rParam.nDistance;        // configured Minimum

        aBorderSize.setHeight( nMaxHeight );
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

        pDev->SetClipRegion(vcl::Region(tools::Rectangle(aStart, aPaperSize)));

        //  left

        const EditTextObject* pObject = pHFItem->GetLeftArea();
        if (pObject)
        {
            pEditDefaults->Put( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );
            pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );
            Point aDraw = aStart;
            tools::Long nDif = aPaperSize.Height() - static_cast<tools::Long>(pEditEngine->GetTextHeight());
            if (nDif > 0)
                aDraw.AdjustY(nDif / 2 );
            pEditEngine->Draw(*pDev, aDraw);
        }

        //  center

        pObject = pHFItem->GetCenterArea();
        if (pObject)
        {
            pEditDefaults->Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
            pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );
            Point aDraw = aStart;
            tools::Long nDif = aPaperSize.Height() - static_cast<tools::Long>(pEditEngine->GetTextHeight());
            if (nDif > 0)
                aDraw.AdjustY(nDif / 2 );
            pEditEngine->Draw(*pDev, aDraw);
        }

        //  right

        pObject = pHFItem->GetRightArea();
        if (pObject)
        {
            pEditDefaults->Put( SvxAdjustItem( SvxAdjust::Right, EE_PARA_JUST ) );
            pEditEngine->SetTextNewDefaults( *pObject, *pEditDefaults, false );
            Point aDraw = aStart;
            tools::Long nDif = aPaperSize.Height() - static_cast<tools::Long>(pEditEngine->GetTextHeight());
            if (nDif > 0)
                aDraw.AdjustY(nDif / 2 );
            pEditEngine->Draw(*pDev, aDraw);
        }

        pDev->SetClipRegion();
    }

    if ( pLocationData )
    {
        tools::Rectangle aHeaderRect( aBorderStart, aBorderSize );
        pLocationData->AddHeaderFooter( aHeaderRect, bHeader, bLeft );
    }

    if (bTaggedPDF)
        pPDF->EndStructureElement();
}

tools::Long ScPrintFunc::DoNotes( tools::Long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    if (bDoPrint)
        pDev->SetMapMode(aTwipMode);

    MakeEditEngine();
    pEditDefaults->Put( SvxAdjustItem( SvxAdjust::Left, EE_PARA_JUST ) );
    pEditEngine->SetDefaults( *pEditDefaults );

    vcl::Font aMarkFont;
    ScAutoFontColorMode eColorMode = bUseStyleColor ? ScAutoFontColorMode::Display : ScAutoFontColorMode::Print;
    rDoc.getCellAttributeHelper().getDefaultCellAttribute().fillFont(aMarkFont, eColorMode);
    pDev->SetFont(aMarkFont);
    tools::Long nMarkLen = pDev->GetTextWidth("GW99999:");
    // without Space-Char, because it rarely arrives there

    Size aDataSize = aPageRect.GetSize();
    if ( nMarkLen > aDataSize.Width() / 2 )     // everything much too small?
        nMarkLen = aDataSize.Width() / 2;       // split the page appropriately
    aDataSize.AdjustWidth( -nMarkLen );

    pEditEngine->SetPaperSize( aDataSize );
    tools::Long nPosX = aPageRect.Left() + nMarkLen;
    tools::Long nPosY = aPageRect.Top();

    tools::Long nCount = 0;
    tools::Long nSize = aNotePosList.size();
    bool bOk;
    do
    {
        bOk = false;
        if ( nNoteStart + nCount < nSize)
        {
            ScAddress &rPos = aNotePosList[ nNoteStart + nCount ];

            if( const ScPostIt* pNote = rDoc.GetNote( rPos ) )
            {
                if(const EditTextObject *pEditText = pNote->GetEditTextObject())
                    pEditEngine->SetTextCurrentDefaults(*pEditText);
                tools::Long nTextHeight = pEditEngine->GetTextHeight();
                if ( nPosY + nTextHeight < aPageRect.Bottom() )
                {
                    if (bDoPrint)
                    {
                        pEditEngine->Draw(*pDev, Point(nPosX, nPosY));

                        OUString aMarkStr(rPos.Format(ScRefFlags::VALID, &rDoc, rDoc.GetAddressConvention()) + ":");

                        //  cell position also via EditEngine, for correct positioning
                        pEditEngine->SetTextCurrentDefaults(aMarkStr);
                        pEditEngine->Draw(*pDev, Point(aPageRect.Left(), nPosY));
                    }

                    if ( pLocationData )
                    {
                        tools::Rectangle aTextRect( Point( nPosX, nPosY ), Size( aDataSize.Width(), nTextHeight ) );
                        pLocationData->AddNoteText( aTextRect, rPos );
                        tools::Rectangle aMarkRect( Point( aPageRect.Left(), nPosY ), Size( nMarkLen, nTextHeight ) );
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

tools::Long ScPrintFunc::PrintNotes( tools::Long nPageNo, tools::Long nNoteStart, bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    if ( nNoteStart >= static_cast<tools::Long>(aNotePosList.size()) || !aTableParam.bNotes )
        return 0;

    if ( bDoPrint && bClearWin )
    {
        //!  aggregate PrintPage !!!

        Color aBackgroundColor( COL_WHITE );
        if ( bUseStyleColor )
            aBackgroundColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;

        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor();
        pDev->SetFillColor(aBackgroundColor);
        pDev->DrawRect(tools::Rectangle(Point(),
                Size(static_cast<tools::Long>(aPageSize.Width() * nScaleX * 100 / nZoom),
                     static_cast<tools::Long>(aPageSize.Height() * nScaleY * 100 / nZoom))));
    }

    //      adjust aPageRect for left/right page

    tools::Rectangle aTempRect( Point(), aPageSize );
    if (IsMirror(nPageNo))
    {
        aPageRect.SetLeft( ( aTempRect.Left()  + nRightMargin ) * 100 / nZoom );
        aPageRect.SetRight( ( aTempRect.Right() - nLeftMargin  ) * 100 / nZoom );
    }
    else
    {
        aPageRect.SetLeft( ( aTempRect.Left()  + nLeftMargin  ) * 100 / nZoom );
        aPageRect.SetRight( ( aTempRect.Right() - nRightMargin ) * 100 / nZoom );
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
            tools::Long nHeaderY = aPageRect.Top()-aHdr.nHeight;
            PrintHF( nPageNo, true, nHeaderY, bDoPrint, pLocationData );
        }
        if (aFtr.bEnable)
        {
            tools::Long nFooterY = aPageRect.Bottom()+aFtr.nDistance;
            PrintHF( nPageNo, false, nFooterY, bDoPrint, pLocationData );
        }
    }

    tools::Long nCount = DoNotes( nNoteStart, bDoPrint, pLocationData );

    if ( pPrinter && bDoPrint )
    {
        OSL_FAIL( "EndPage does not exist anymore" );
    }

    return nCount;
}

void ScPrintFunc::PrintPage( tools::Long nPageNo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                                bool bDoPrint, ScPreviewLocationData* pLocationData )
{
    bool bLayoutRTL = rDoc.IsLayoutRTL( nPrintTab );
    tools::Long nLayoutSign = bLayoutRTL ? -1 : 1;

    //  nPageNo is the page number within all sheets of one "start page" setting

    if ( bClearWin && bDoPrint )
    {
        //  must exactly fit to painting the frame in preview.cxx !!!

        Color aBackgroundColor( COL_WHITE );
        if ( bUseStyleColor )
            aBackgroundColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;

        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor();
        pDev->SetFillColor(aBackgroundColor);
        pDev->DrawRect(tools::Rectangle(Point(),
                Size(static_cast<tools::Long>(aPageSize.Width() * nScaleX * 100 / nZoom),
                     static_cast<tools::Long>(aPageSize.Height() * nScaleY * 100 / nZoom))));
    }

    //      adjust aPageRect for left/right page

    tools::Rectangle aTempRect( Point(), aPageSize );
    if (IsMirror(nPageNo))
    {
        aPageRect.SetLeft( ( aTempRect.Left()  + nRightMargin ) * 100 / nZoom );
        aPageRect.SetRight( ( aTempRect.Right() - nLeftMargin  ) * 100 / nZoom );
    }
    else
    {
        aPageRect.SetLeft( ( aTempRect.Left()  + nLeftMargin  ) * 100 / nZoom );
        aPageRect.SetRight( ( aTempRect.Right() - nRightMargin ) * 100 / nZoom );
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
        tools::Long nHeaderY = aPageRect.Top()-aHdr.nHeight;
        PrintHF( nPageNo, true, nHeaderY, bDoPrint, pLocationData );
    }
    if (aFtr.bEnable)
    {
        tools::Long nFooterY = aPageRect.Bottom()+aFtr.nDistance;
        PrintHF( nPageNo, false, nFooterY, bDoPrint, pLocationData );
    }

    //  Position ( margins / centering )

    tools::Long nLeftSpace = aPageRect.Left();     // Document-Twips
    tools::Long nTopSpace  = aPageRect.Top();
    if ( bCenterHor || bLayoutRTL )
    {
        tools::Long nDataWidth = 0;
        SCCOL i;
        for (i=nX1; i<=nX2; i++)
            nDataWidth += rDoc.GetColWidth( i,nPrintTab );
        if (bDoRepCol)
            for (i=nRepeatStartCol; i<=nRepeatEndCol; i++)
                nDataWidth += rDoc.GetColWidth( i,nPrintTab );
        if (aTableParam.bHeaders)
            nDataWidth += tools::Long(PRINT_HEADER_WIDTH);
        if (pBorderItem)
            nDataWidth += pBorderItem->GetDistance(SvxBoxItemLine::LEFT) +
                           pBorderItem->GetDistance(SvxBoxItemLine::RIGHT);        //! Line width?
        if (pShadowItem && pShadowItem->GetLocation() != SvxShadowLocation::NONE)
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
        tools::Long nDataHeight = rDoc.GetRowHeight( nY1, nY2, nPrintTab);
        if (bDoRepRow)
            nDataHeight += rDoc.GetRowHeight( nRepeatStartRow,
                    nRepeatEndRow, nPrintTab);
        if (aTableParam.bHeaders)
            nDataHeight += tools::Long(PRINT_HEADER_HEIGHT);
        if (pBorderItem)
            nDataHeight += pBorderItem->GetDistance(SvxBoxItemLine::TOP) +
                           pBorderItem->GetDistance(SvxBoxItemLine::BOTTOM);       //! Line width?
        if (pShadowItem && pShadowItem->GetLocation() != SvxShadowLocation::NONE)
            nDataHeight += pShadowItem->CalcShadowSpace(SvxShadowItemSide::TOP) +
                           pShadowItem->CalcShadowSpace(SvxShadowItemSide::BOTTOM);
        nTopSpace += ( aPageRect.GetHeight() - nDataHeight ) / 2;
        if (pBorderItem)
            nTopSpace -= lcl_LineTotal(pBorderItem->GetTop());
    }

    //  calculate sizes of the elements for partitioning
    //  (header, repeat, data)

    tools::Long nHeaderWidth   = 0;
    tools::Long nHeaderHeight  = 0;
    tools::Long nRepeatWidth   = 0;
    tools::Long nRepeatHeight  = 0;
    tools::Long nContentWidth  = 0;        // scaled - not the same as nDataWidth above
    tools::Long nContentHeight = 0;
    if (aTableParam.bHeaders)
    {
        nHeaderWidth  = static_cast<tools::Long>(PRINT_HEADER_WIDTH * nScaleX);
        nHeaderHeight = static_cast<tools::Long>(PRINT_HEADER_HEIGHT * nScaleY);
    }
    if (bDoRepCol)
        for (SCCOL i=nRepeatStartCol; i<=nRepeatEndCol; i++)
            nRepeatWidth += static_cast<tools::Long>(rDoc.GetColWidth(i,nPrintTab) * nScaleX);
    if (bDoRepRow)
        nRepeatHeight += rDoc.GetScaledRowHeight( nRepeatStartRow,
                nRepeatEndRow, nPrintTab, nScaleY);
    for (SCCOL i=nX1; i<=nX2; i++)
        nContentWidth += static_cast<tools::Long>(rDoc.GetColWidth(i,nPrintTab) * nScaleX);
    nContentHeight += rDoc.GetScaledRowHeight( nY1, nY2, nPrintTab,
            nScaleY);

    //  partition the page

    tools::Long nStartX = static_cast<tools::Long>( nLeftSpace * nScaleX );
    tools::Long nStartY = static_cast<tools::Long>( nTopSpace  * nScaleY );
    tools::Long nInnerStartX = nStartX;
    tools::Long nInnerStartY = nStartY;
    if (pBorderItem)
    {
        nInnerStartX += static_cast<tools::Long>( ( lcl_LineTotal(pBorderItem->GetLeft()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::LEFT) ) * nScaleX );
        nInnerStartY += static_cast<tools::Long>( ( lcl_LineTotal(pBorderItem->GetTop()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::TOP) ) * nScaleY );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SvxShadowLocation::NONE)
    {
        nInnerStartX += static_cast<tools::Long>( pShadowItem->CalcShadowSpace(SvxShadowItemSide::LEFT) * nScaleX );
        nInnerStartY += static_cast<tools::Long>( pShadowItem->CalcShadowSpace(SvxShadowItemSide::TOP) * nScaleY );
    }

    if ( bLayoutRTL )
    {
        //  arrange elements starting from the right edge
        nInnerStartX += nHeaderWidth + nRepeatWidth + nContentWidth;

        //  make rounding easier so the elements are really next to each other in preview
        Size aOffsetOnePixel = pDev->PixelToLogic( Size(1,1), aOffsetMode );
        tools::Long nOffsetOneX = aOffsetOnePixel.Width();
        nInnerStartX += nOffsetOneX / 2;
    }

    tools::Long nFrameStartX = nInnerStartX;
    tools::Long nFrameStartY = nInnerStartY;

    tools::Long nRepStartX = nInnerStartX + nHeaderWidth * nLayoutSign;    // widths/heights are 0 if not used
    tools::Long nRepStartY = nInnerStartY + nHeaderHeight;
    tools::Long nDataX = nRepStartX + nRepeatWidth * nLayoutSign;
    tools::Long nDataY = nRepStartY + nRepeatHeight;
    tools::Long nEndX = nDataX + nContentWidth * nLayoutSign;
    tools::Long nEndY = nDataY + nContentHeight;
    tools::Long nFrameEndX = nEndX;
    tools::Long nFrameEndY = nEndY;

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

    tools::Long nBorderEndX = nEndX;
    tools::Long nBorderEndY = nEndY;
    if (pBorderItem)
    {
        nBorderEndX += static_cast<tools::Long>( ( lcl_LineTotal(pBorderItem->GetRight()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::RIGHT) ) * nScaleX );
        nBorderEndY += static_cast<tools::Long>( ( lcl_LineTotal(pBorderItem->GetBottom()) +
                                    pBorderItem->GetDistance(SvxBoxItemLine::BOTTOM) ) * nScaleY );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SvxShadowLocation::NONE)
    {
        nBorderEndX += static_cast<tools::Long>( pShadowItem->CalcShadowSpace(SvxShadowItemSide::RIGHT) * nScaleX );
        nBorderEndY += static_cast<tools::Long>( pShadowItem->CalcShadowSpace(SvxShadowItemSide::BOTTOM) * nScaleY );
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
        aGridColor = SC_MOD()->GetColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;

    if (aTableParam.bHeaders)
    {
        if ( bDoPrint )
        {
            pDev->SetLineColor( aGridColor );
            pDev->SetFillColor();
            pDev->SetMapMode(aOffsetMode);
        }

        ScPatternAttr aPattern(rDoc.getCellAttributeHelper());
        vcl::Font aFont;
        ScAutoFontColorMode eColorMode = bUseStyleColor ? ScAutoFontColorMode::Display : ScAutoFontColorMode::Print;
        aPattern.fillFont(aFont, eColorMode, pDev);
        pDev->SetFont(aFont);

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
        tools::Long nOneX = aOnePixel.Width();
        tools::Long nOneY = aOnePixel.Height();

        tools::Long nLeftX   = nFrameStartX;
        tools::Long nTopY    = nFrameStartY - nOneY;
        tools::Long nRightX  = nFrameEndX;
        tools::Long nBottomY = nFrameEndY - nOneY;
        if ( !bLayoutRTL )
        {
            nLeftX   -= nOneX;
            nRightX  -= nOneX;
        }
        pDev->SetMapMode(aOffsetMode);
        pDev->SetLineColor( aGridColor );
        pDev->SetFillColor();
        pDev->DrawRect( tools::Rectangle( nLeftX, nTopY, nRightX, nBottomY ) );
        //  nEndX/Y without frame-adaptation
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

    OUString aName = rDoc.GetPageStyle( nPrintTab );
    SCTAB nTabCount = rDoc.GetTableCount();
    for (SCTAB nTab=0; nTab<nTabCount; nTab++)
        if ( nTab==nPrintTab || rDoc.GetPageStyle(nTab)==aName )
        {
            //  Repeating rows/columns
            rDoc.SetRepeatArea( nTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );

            //  set breaks
            ResetBreaks(nTab);
            pDocShell->PostPaint(0,0,nTab,rDoc.MaxCol(),rDoc.MaxRow(),nTab, PaintPartFlags::Grid);
        }

    return true;
}

tools::Long ScPrintFunc::CountPages()                          // sets also nPagesX, nPagesY
{
    bool bAreaOk = false;

    if (rDoc.HasTable( nPrintTab ))
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
        tools::Long nPages = 0;
        size_t nY;
        if (bMultiArea)
        {
            sal_uInt16 nRCount = rDoc.GetPrintRangeCount( nPrintTab );
            for (sal_uInt16 i=0; i<nRCount; i++)
            {
                CalcZoom(i);
                if ( aTableParam.bSkipEmpty )
                    for (nY=0; nY< m_aRanges.m_nPagesY; nY++)
                        nPages += (*m_aRanges.m_xPageRows)[nY].CountVisible();
                else
                    nPages += static_cast<tools::Long>(m_aRanges.m_nPagesX) * m_aRanges.m_nPagesY;
                if ( pPageData )
                    FillPageData();
            }
        }
        else
        {
            CalcZoom(RANGENO_NORANGE);                      // calculate Zoom
            if ( aTableParam.bSkipEmpty )
                for (nY=0; nY<m_aRanges.m_nPagesY; nY++)
                    nPages += (*m_aRanges.m_xPageRows)[nY].CountVisible();
            else
                nPages += static_cast<tools::Long>(m_aRanges.m_nPagesX) * m_aRanges.m_nPagesY;
            if ( pPageData )
                FillPageData();
        }
        return nPages;
    }
    else
    {
        m_aRanges.m_nPagesX = m_aRanges.m_nPagesY = m_aRanges.m_nTotalY = 0;
        return 0;
    }
}

tools::Long ScPrintFunc::CountNotePages()
{
    if ( !aTableParam.bNotes || !bPrintCurrentTable )
        return 0;

    bool bError = false;
    if (!aAreaParam.bPrintArea)
        bError = !AdjustPrintArea(true);            // completely search in Doc

    sal_uInt16 nRepeats = 1;                            // how often go through it ?
    if (bMultiArea)
        nRepeats = rDoc.GetPrintRangeCount(nPrintTab);
    if (bError)
        nRepeats = 0;

    for (sal_uInt16 nStep=0; nStep<nRepeats; nStep++)
    {
        bool bDoThis = true;
        if (bMultiArea)             // go through all Areas
        {
            const ScRange* pThisRange = rDoc.GetPrintRange( nPrintTab, nStep );
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
            assert( bPrintAreaValid );
            for ( SCCOL nCol = nStartCol; nCol <= nEndCol; ++nCol )
            {
                if (rDoc.HasColNotes(nCol, nPrintTab))
                {
                    for ( SCROW nRow = nStartRow; nRow <= nEndRow; ++nRow )
                    {
                        if ( rDoc.HasNote(nCol, nRow, nPrintTab) )
                            aNotePosList.emplace_back( nCol, nRow, nPrintTab );
                    }
                }
            }
        }
    }

    tools::Long nPages = 0;
    tools::Long nNoteNr = 0;
    tools::Long nNoteAdd;
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

    tools::Long nEffZoom = nZoom * static_cast<tools::Long>(nManualZoom);
    constexpr auto HMM_PER_TWIPS = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100);
    nScaleX = nScaleY = HMM_PER_TWIPS;  // output in 1/100 mm

    Fraction aZoomFract( nEffZoom,10000 );
    Fraction aHorFract = aZoomFract;

    if ( !pPrinter && !bIsRender )                          // adjust scale for preview
    {
        double nFact = pDocShell->GetOutputFactor();
        aHorFract = Fraction( static_cast<tools::Long>( nEffZoom / nFact ), 10000 );
    }

    aLogicMode = MapMode( MapUnit::Map100thMM, Point(), aHorFract, aZoomFract );

    Point aLogicOfs( -aOffset.X(), -aOffset.Y() );
    aOffsetMode = MapMode( MapUnit::Map100thMM, aLogicOfs, aHorFract, aZoomFract );

    Point aTwipsOfs( static_cast<tools::Long>( -aOffset.X() / nScaleX + 0.5 ), static_cast<tools::Long>( -aOffset.Y() / nScaleY + 0.5 ) );
    aTwipMode = MapMode( MapUnit::MapTwip, aTwipsOfs, aHorFract, aZoomFract );
}

void ScPrintFunc::ApplyPrintSettings()
{
    if ( !pPrinter )
        return;

    //  Configure Printer to Printing

    Size aEnumSize = aPageSize;

    pPrinter->SetOrientation( bLandscape ? Orientation::Landscape : Orientation::Portrait );
    if ( bLandscape )
    {
            // landscape is always interpreted as a rotation by 90 degrees !
            // this leads to non WYSIWIG but at least it prints!
            // #i21775#
            tools::Long nTemp = aEnumSize.Width();
            aEnumSize.setWidth( aEnumSize.Height() );
            aEnumSize.setHeight( nTemp );
    }
    Paper ePaper = SvxPaperInfo::GetSvxPaper( aEnumSize, MapUnit::MapTwip );
    sal_uInt16 nPaperBin = pParamSet->Get(ATTR_PAGE_PAPERBIN).GetValue();

    pPrinter->SetPaper( ePaper );
    if ( PAPER_USER == ePaper )
    {
        MapMode aPrinterMode = pPrinter->GetMapMode();
        MapMode aLocalMode( MapUnit::MapTwip );
        pPrinter->SetMapMode( aLocalMode );
        pPrinter->SetPaperSizeUser( aEnumSize );
        pPrinter->SetMapMode( aPrinterMode );
    }

    pPrinter->SetPaperBin( nPaperBin );
}

//  rPageRanges   = range for all tables
//  nStartPage    = rPageRanges starts at nStartPage
//  nDisplayStart = continuous number for displaying the page number

tools::Long ScPrintFunc::DoPrint( const MultiSelection& rPageRanges,
                                tools::Long nStartPage, tools::Long nDisplayStart, bool bDoPrint,
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

    tools::Long nPageNo = 0;
    tools::Long nPrinted = 0;
    tools::Long nEndPage = rPageRanges.GetTotalRange().Max();

    sal_uInt16 nRepeats = 1;
    if (bMultiArea)
        nRepeats = rDoc.GetPrintRangeCount(nPrintTab);
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
            for (nCountX=0; nCountX<m_aRanges.m_nPagesX; nCountX++)
            {
                OSL_ENSURE(nCountX < m_aRanges.m_xPageEndX->size(), "vector access error for aPageEndX (!)");
                nX2 = (*m_aRanges.m_xPageEndX)[nCountX];
                for (nCountY=0; nCountY<m_aRanges.m_nPagesY; nCountY++)
                {
                    auto& rPageRow = (*m_aRanges.m_xPageRows)[nCountY];
                    nY1 = rPageRow.GetStartRow();
                    nY2 = rPageRow.GetEndRow();
                    if ( !aTableParam.bSkipEmpty || !rPageRow.IsHidden(nCountX) )
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
            for (nCountY=0; nCountY<m_aRanges.m_nPagesY; nCountY++)
            {
                auto& rPageRow = (*m_aRanges.m_xPageRows)[nCountY];
                nY1 = rPageRow.GetStartRow();
                nY2 = rPageRow.GetEndRow();
                nX1 = nStartCol;
                for (nCountX=0; nCountX<m_aRanges.m_nPagesX; nCountX++)
                {
                    OSL_ENSURE(nCountX < m_aRanges.m_xPageEndX->size(), "vector access error for aPageEndX");
                    nX2 = (*m_aRanges.m_xPageEndX)[nCountX];
                    if ( !aTableParam.bSkipEmpty || !rPageRow.IsHidden(nCountX) )
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

    aFieldData.aTabName = ScResId( STR_NOTES );

    tools::Long nNoteNr = 0;
    tools::Long nNoteAdd;
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
    sal_uInt16 nRCount = rDoc.GetPrintRangeCount( nPrintTab );
    const ScRange* pThisRange = nullptr;
    if (nRangeNo != RANGENO_NORANGE && nRangeNo < nRCount)
        pThisRange = rDoc.GetPrintRange( nPrintTab, nRangeNo );
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
        m_aRanges.m_nPagesX = m_aRanges.m_nPagesY = m_aRanges.m_nTotalY = 0;
        return;
    }

    rDoc.SetRepeatArea( nPrintTab, nRepeatStartCol,nRepeatEndCol, nRepeatStartRow,nRepeatEndRow );

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
             rDoc.GetAllRowBreaks(aRowBreaks, nPrintTab, false, true);
             rDoc.GetAllColBreaks(aColBreaks, nPrintTab, false, true);
             nMinPages = (aRowBreaks.size() + 1) * (aColBreaks.size() + 1);

             // #i54993# use min forced by breaks if it's > # pages in
             // scale parameter to avoid bottoming out at <= ZOOM_MIN
             nPagesToFit = std::max(nMinPages, nPagesToFit);
        }

        sal_uInt16 nLastFitZoom = 0, nLastNonFitZoom = 0;
        while (true)
        {
            if (nZoom <= ZOOM_MIN)
                break;

            CalcPages();
            bool bFitsPage = (m_aRanges.m_nPagesX * m_aRanges.m_nPagesY <= nPagesToFit);

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
             rDoc.GetAllRowBreaks(aRowBreaks, nPrintTab, false, true);
             rDoc.GetAllColBreaks(aColBreaks, nPrintTab, false, true);
             nMinPagesW = aColBreaks.size() + 1;
             nMinPagesH = aRowBreaks.size() + 1;

             // #i54993# use min forced by breaks if it's > # pages in
             // scale parameters to avoid bottoming out at <= ZOOM_MIN
             nW = std::max(nMinPagesW, nW);
             nH = std::max(nMinPagesH, nH);
        }

        sal_uInt16 nLastFitZoom = 0, nLastNonFitZoom = 0;
        while (true)
        {
            if (nZoom <= ZOOM_MIN)
                break;

            CalcPages();
            bool bFitsPage = ((!nW || (m_aRanges.m_nPagesX <= nW)) && (!nH || (m_aRanges.m_nPagesY <= nH)));

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
        // tdf#103516 remove the almost blank page(s) for better
        // interoperability by using slightly smaller zoom
        if (nW > 0 && nH == 0 && m_aRanges.m_nPagesY > 1)
        {
            sal_uInt32 nLastPagesY = m_aRanges.m_nPagesY;
            nLastFitZoom = nZoom;
            nZoom *= 0.98;
            if (nZoom < nLastFitZoom)
            {
                CalcPages();
                // same page count with smaller zoom: use the original zoom
                if (m_aRanges.m_nPagesY == nLastPagesY)
                {
                    nZoom = nLastFitZoom;
                    CalcPages();
                }
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

    aPageRect = tools::Rectangle( Point(), aPageSize );
    aPageRect.SetLeft( ( aPageRect.Left()   + nLeftMargin                  ) * 100 / nZoom );
    aPageRect.SetRight( ( aPageRect.Right()  - nRightMargin                 ) * 100 / nZoom );
    aPageRect.SetTop( ( aPageRect.Top()    + nTopMargin    ) * 100 / nZoom + aHdr.nHeight );
    aPageRect.SetBottom( ( aPageRect.Bottom() - nBottomMargin ) * 100 / nZoom - aFtr.nHeight );

    Size aDocPageSize = aPageRect.GetSize();
    if (aTableParam.bHeaders)
    {
        aDocPageSize.AdjustWidth( -(tools::Long(PRINT_HEADER_WIDTH)) );
        aDocPageSize.AdjustHeight( -(tools::Long(PRINT_HEADER_HEIGHT)) );
    }
    if (pBorderItem)
    {
        aDocPageSize.AdjustWidth( -(lcl_LineTotal(pBorderItem->GetLeft()) +
                                 lcl_LineTotal(pBorderItem->GetRight()) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::LEFT) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::RIGHT)) );
        aDocPageSize.AdjustHeight( -(lcl_LineTotal(pBorderItem->GetTop()) +
                                 lcl_LineTotal(pBorderItem->GetBottom()) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::TOP) +
                                 pBorderItem->GetDistance(SvxBoxItemLine::BOTTOM)) );
    }
    if (pShadowItem && pShadowItem->GetLocation() != SvxShadowLocation::NONE)
    {
        aDocPageSize.AdjustWidth( -(pShadowItem->CalcShadowSpace(SvxShadowItemSide::LEFT) +
                                 pShadowItem->CalcShadowSpace(SvxShadowItemSide::RIGHT)) );
        aDocPageSize.AdjustHeight( -(pShadowItem->CalcShadowSpace(SvxShadowItemSide::TOP) +
                                 pShadowItem->CalcShadowSpace(SvxShadowItemSide::BOTTOM)) );
    }
    return aDocPageSize;
}

void ScPrintFunc::ResetBreaks( SCTAB nTab )         // Set Breaks correctly for view
{
    rDoc.SetPageSize( nTab, GetDocPageSize() );
    rDoc.UpdatePageBreaks( nTab );
}

static void lcl_SetHidden( const ScDocument& rDoc, SCTAB nPrintTab, ScPageRowEntry& rPageRowEntry,
                    SCCOL nStartCol, const std::vector< SCCOL >& rPageEndX )
{
    size_t nPagesX   = rPageRowEntry.GetPagesX();
    SCROW nStartRow = rPageRowEntry.GetStartRow();
    SCROW nEndRow   = rPageRowEntry.GetEndRow();

    bool bLeftIsEmpty = false;
    ScRange aTempRange;
    tools::Rectangle aTempRect = rDoc.GetMMRect( 0,0, 0,0, 0 );

    for (size_t i=0; i<nPagesX; i++)
    {
        OSL_ENSURE(i < rPageEndX.size(), "vector access error for aPageEndX");
        SCCOL nEndCol = rPageEndX[i];
        if ( rDoc.IsPrintEmpty( nStartCol, nStartRow, nEndCol, nEndRow, nPrintTab,
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
    assert( bPrintAreaValid );

    sc::PrintPageRangesInput aInput(aTableParam.bSkipEmpty, aAreaParam.bPrintArea,
                                    ScRange(nStartCol, nStartRow, nPrintTab, nEndCol, nEndRow, nPrintTab),
                                    GetDocPageSize());
    m_aRanges.calculate(rDoc, aInput);
}

namespace sc
{

PrintPageRanges::PrintPageRanges()
    : m_nPagesX(0)
    , m_nPagesY(0)
    , m_nTotalY(0)
{}

void PrintPageRanges::calculate(ScDocument& rDoc, PrintPageRangesInput const& rInput)
{
    // Already calculated?
    if (m_aInput == rInput)
        return;

    m_aInput = rInput;

    rDoc.SetPageSize(m_aInput.getPrintTab(), m_aInput.m_aDocSize);

    // Clear the map to prevent any outdated values to "survive" when
    // we have to recalculate the new values anyway
    m_xPageRows->clear();

    // #i123672# use dynamic mem to react on size changes
    if (m_xPageEndX->size() < static_cast<size_t>(rDoc.MaxCol()) + 1)
    {
        m_xPageEndX->resize(rDoc.MaxCol()+1, SCCOL());
    }

    if (m_aInput.m_bPrintArea)
    {
        ScRange aRange(m_aInput.getStartColumn(), m_aInput.getStartRow(), m_aInput.getPrintTab(), m_aInput.getEndColumn(), m_aInput.getEndRow(), m_aInput.getPrintTab());
        rDoc.UpdatePageBreaks(m_aInput.getPrintTab(), &aRange);
    }
    else
    {
        rDoc.UpdatePageBreaks(m_aInput.getPrintTab()); // else, end is marked
    }

    const size_t nRealCnt = m_aInput.getEndRow() - m_aInput.getStartRow() + 1;

    // #i123672# use dynamic mem to react on size changes
    if (m_xPageEndY->size() < nRealCnt+1)
    {
        m_xPageEndY->resize(nRealCnt + 1, SCROW());
    }

    //  Page alignment/splitting after breaks in Col/RowFlags
    //  Of several breaks in a hidden area, only one counts.

    m_nPagesX = 0;
    m_nPagesY = 0;
    m_nTotalY = 0;

    bool bVisCol = false;
    for (SCCOL i = m_aInput.getStartColumn(); i <= m_aInput.getEndColumn(); i++)
    {
        bool bHidden = rDoc.ColHidden(i, m_aInput.getPrintTab());
        bool bPageBreak(rDoc.HasColBreak(i, m_aInput.getPrintTab()) & ScBreakType::Page);
        if (i > m_aInput.getStartColumn() && bVisCol && bPageBreak)
        {
            OSL_ENSURE(m_nPagesX < m_xPageEndX->size(), "vector access error for aPageEndX");
            (*m_xPageEndX)[m_nPagesX] = i-1;
            ++m_nPagesX;
            bVisCol = false;
        }
        if (!bHidden)
            bVisCol = true;
    }
    if (bVisCol) // also at the end, no empty pages
    {
        OSL_ENSURE(m_nPagesX < m_xPageEndX->size(), "vector access error for aPageEndX");
        (*m_xPageEndX)[m_nPagesX] = m_aInput.getEndColumn();
        ++m_nPagesX;
    }

    bool bVisRow = false;
    SCROW nPageStartRow = m_aInput.getStartRow();
    SCROW nLastVisibleRow = -1;

    std::unique_ptr<ScRowBreakIterator> pRowBreakIter(rDoc.GetRowBreakIterator(m_aInput.getPrintTab()));
    SCROW nNextPageBreak = pRowBreakIter->first();
    while (nNextPageBreak != ScRowBreakIterator::NOT_FOUND && nNextPageBreak < m_aInput.getStartRow())
        // Skip until the page break position is at the start row or greater.
        nNextPageBreak = pRowBreakIter->next();

    for (SCROW nRow = m_aInput.getStartRow(); nRow <= m_aInput.getEndRow(); ++nRow)
    {
        bool bPageBreak = (nNextPageBreak == nRow);
        if (bPageBreak)
            nNextPageBreak = pRowBreakIter->next();

        if (nRow > m_aInput.getStartRow() && bVisRow && bPageBreak)
        {
            OSL_ENSURE(m_nTotalY < m_xPageEndY->size(), "vector access error for rPageEndY");
            (*m_xPageEndY)[m_nTotalY] = nRow - 1;
            ++m_nTotalY;

            if (!m_aInput.m_bSkipEmpty || !rDoc.IsPrintEmpty(m_aInput.getStartColumn(), nPageStartRow, m_aInput.getEndColumn(), nRow-1, m_aInput.getPrintTab()))
            {
                auto& rPageRow = (*m_xPageRows)[m_nPagesY];
                rPageRow.SetStartRow(nPageStartRow);
                rPageRow.SetEndRow(nRow - 1);
                rPageRow.SetPagesX(m_nPagesX);
                if (m_aInput.m_bSkipEmpty)
                    lcl_SetHidden(rDoc, m_aInput.getPrintTab(), rPageRow, m_aInput.getStartColumn(), *m_xPageEndX);
                ++m_nPagesY;
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
        if (!rDoc.RowHidden(nRow, m_aInput.getPrintTab(), nullptr, &nLastRow))
        {
            bVisRow = true;
            nLastVisibleRow = nLastRow;
        }
        else
        {
            // Skip all hidden rows until next pagebreak.
            nRow = ((nNextPageBreak == ScRowBreakIterator::NOT_FOUND) ? nLastRow :
                    std::min(nLastRow, nNextPageBreak - 1));
        }
    }

    if (!bVisRow)
        return;

    OSL_ENSURE(m_nTotalY < m_xPageEndY->size(), "vector access error for maPageEndY");
    (*m_xPageEndY)[m_nTotalY] = m_aInput.getEndRow();
    ++m_nTotalY;

    if (!m_aInput.m_bSkipEmpty || !rDoc.IsPrintEmpty(m_aInput.getStartColumn(), nPageStartRow, m_aInput.getEndColumn(), m_aInput.getEndRow(), m_aInput.getPrintTab()))
    {
        auto& rPageRow = (*m_xPageRows)[m_nPagesY];
        rPageRow.SetStartRow(nPageStartRow);
        rPageRow.SetEndRow(m_aInput.getEndRow());
        rPageRow.SetPagesX(m_nPagesX);
        if (m_aInput.m_bSkipEmpty)
            lcl_SetHidden(rDoc, m_aInput.getPrintTab(), rPageRow, m_aInput.getStartColumn(), *m_xPageEndX);
        ++m_nPagesY;
    }
}

} // end namespace sc
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
