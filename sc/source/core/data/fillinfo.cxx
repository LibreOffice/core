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
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/brushitem.hxx>
#include <svx/framelink.hxx>
#include <osl/diagnose.h>

#include <fillinfo.hxx>
#include <document.hxx>
#include <table.hxx>
#include <attrib.hxx>
#include <attarray.hxx>
#include <markarr.hxx>
#include <markdata.hxx>
#include <patattr.hxx>
#include <poolhelp.hxx>
#include <docpool.hxx>
#include <conditio.hxx>
#include <stlpool.hxx>
#include <cellvalue.hxx>
#include <mtvcellfunc.hxx>

#include <algorithm>
#include <limits>
#include <vector>
#include <memory>

// Similar as in output.cxx

static void lcl_GetMergeRange( SCCOL nX, SCROW nY, SCSIZE nArrY,
                            const ScDocument* pDoc, RowInfo* pRowInfo,
                            SCCOL nX1, SCROW nY1, SCTAB nTab,
                            SCCOL& rStartX, SCROW& rStartY, SCCOL& rEndX, SCROW& rEndY )
{
    ScCellInfo* pInfo = &pRowInfo[nArrY].cellInfo(nX);

    rStartX = nX;
    rStartY = nY;
    bool bHOver = pInfo->bHOverlapped;
    bool bVOver = pInfo->bVOverlapped;
    SCCOL nLastCol;
    SCROW nLastRow;

    while (bHOver)              // nY constant
    {
        --rStartX;
        if (rStartX >= nX1 && !pDoc->ColHidden(rStartX, nTab, nullptr, &nLastCol))
        {
            bHOver = pRowInfo[nArrY].cellInfo(rStartX).bHOverlapped;
            bVOver = pRowInfo[nArrY].cellInfo(rStartX).bVOverlapped;
        }
        else
        {
            ScMF nOverlap = pDoc->GetAttr( rStartX, rStartY, nTab, ATTR_MERGE_FLAG )->GetValue();
            bHOver = bool(nOverlap & ScMF::Hor);
            bVOver = bool(nOverlap & ScMF::Ver);
        }
    }

    while (bVOver)
    {
        --rStartY;

        if (nArrY>0)
            --nArrY;                        // local copy !

        if (rStartX >= nX1 && rStartY >= nY1 &&
            !pDoc->ColHidden(rStartX, nTab, nullptr, &nLastCol) &&
            !pDoc->RowHidden(rStartY, nTab, nullptr, &nLastRow) &&
            pRowInfo[nArrY].nRowNo == rStartY)
        {
            bVOver = pRowInfo[nArrY].cellInfo(rStartX).bVOverlapped;
        }
        else
        {
            ScMF nOverlap = pDoc->GetAttr(
                                rStartX, rStartY, nTab, ATTR_MERGE_FLAG )->GetValue();
            bVOver = bool(nOverlap & ScMF::Ver);
        }
    }

    const ScMergeAttr* pMerge;
    if (rStartX >= nX1 && rStartY >= nY1 &&
        !pDoc->ColHidden(rStartX, nTab, nullptr, &nLastCol) &&
        !pDoc->RowHidden(rStartY, nTab, nullptr, &nLastRow) &&
        pRowInfo[nArrY].nRowNo == rStartY)
    {
        pMerge = &pRowInfo[nArrY].cellInfo(rStartX).pPatternAttr->
                                        GetItem(ATTR_MERGE);
    }
    else
        pMerge = pDoc->GetAttr(rStartX,rStartY,nTab,ATTR_MERGE);

    rEndX = rStartX + pMerge->GetColMerge() - 1;
    rEndY = rStartY + pMerge->GetRowMerge() - 1;
}

namespace {

class RowInfoFiller
{
    ScDocument& mrDoc;
    SCTAB mnTab;
    RowInfo* mpRowInfo;
    SCCOL mnCol;
    SCSIZE mnArrY;
    SCCOL mnStartCol;
    SCROW mnHiddenEndRow;
    bool mbHiddenRow;

    bool isHidden(size_t nRow)
    {
        SCROW nThisRow = static_cast<SCROW>(nRow);
        if (nThisRow > mnHiddenEndRow)
            mbHiddenRow = mrDoc.RowHidden(nThisRow, mnTab, nullptr, &mnHiddenEndRow);
        return mbHiddenRow;
    }

    void alignArray(size_t nRow)
    {
        while (mpRowInfo[mnArrY].nRowNo < static_cast<SCROW>(nRow))
            ++mnArrY;
    }

    void setInfo(size_t nRow, const ScRefCellValue& rCell)
    {
        alignArray(nRow);

        RowInfo& rThisRowInfo = mpRowInfo[mnArrY];
        if(mnCol >= mnStartCol-1)
            rThisRowInfo.cellInfo(mnCol).maCell = rCell;
        rThisRowInfo.basicCellInfo(mnCol).bEmptyCellText = false;
        ++mnArrY;
    }

public:
    RowInfoFiller(ScDocument& rDoc, SCTAB nTab, RowInfo* pRowInfo, SCCOL nCol, SCSIZE nArrY, SCCOL nStartCol) :
        mrDoc(rDoc), mnTab(nTab), mpRowInfo(pRowInfo), mnCol(nCol), mnArrY(nArrY), mnStartCol(nStartCol),
        mnHiddenEndRow(-1), mbHiddenRow(false) {}

    void operator() (size_t nRow, double fVal)
    {
        if (!isHidden(nRow))
            setInfo(nRow, ScRefCellValue(fVal));
    }

    void operator() (size_t nRow, const svl::SharedString& rStr)
    {
        if (!isHidden(nRow))
            setInfo(nRow, ScRefCellValue(&rStr));
    }

    void operator() (size_t nRow, const EditTextObject* p)
    {
        if (!isHidden(nRow))
            setInfo(nRow, ScRefCellValue(p));
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        if (!isHidden(nRow))
            setInfo(nRow, ScRefCellValue(const_cast<ScFormulaCell*>(p)));
    }
};

bool isRotateItemUsed(const ScDocumentPool *pPool)
{
    return pPool->GetItemSurrogates(ATTR_ROTATE_VALUE).size() > 0;
}

void initRowInfo(const ScDocument* pDoc, RowInfo* pRowInfo, const SCSIZE nMaxRow,
        double fRowScale, SCROW nRow1, SCTAB nTab, SCROW& rYExtra, SCSIZE& rArrRow, SCROW& rRow2)
{
    sal_uInt16 nDocHeight = pDoc->GetSheetOptimalMinRowHeight(nTab);
    SCROW nDocHeightEndRow = -1;
    for (SCROW nSignedY=nRow1-1; nSignedY<=rYExtra; nSignedY++)
    {
        SCROW nY;
        if (nSignedY >= 0)
            nY = nSignedY;
        else
            nY = pDoc->MaxRow()+1;          // invalid

        if (nY > nDocHeightEndRow)
        {
            if (pDoc->ValidRow(nY))
                nDocHeight = pDoc->GetRowHeight( nY, nTab, nullptr, &nDocHeightEndRow );
            else
                nDocHeight = pDoc->GetSheetOptimalMinRowHeight(nTab);
        }

        if ( rArrRow==0 || nDocHeight || nY > pDoc->MaxRow() )
        {
            RowInfo* pThisRowInfo = &pRowInfo[rArrRow];
            // pThisRowInfo->pCellInfo is set below using allocCellInfo()

            sal_uInt16 nHeight = static_cast<sal_uInt16>(
                std::clamp(
                    nDocHeight * fRowScale, 1.0, double(std::numeric_limits<sal_uInt16>::max())));

            pThisRowInfo->nRowNo        = nY;               //TODO: case < 0 ?
            pThisRowInfo->nHeight       = nHeight;
            pThisRowInfo->bEmptyBack    = true;
            pThisRowInfo->bChanged      = true;
            pThisRowInfo->bAutoFilter   = false;
            pThisRowInfo->bPivotButton  = false;
            pThisRowInfo->bPivotToggle  = false;
            pThisRowInfo->nRotMaxCol    = SC_ROTMAX_NONE;

            ++rArrRow;
            if (rArrRow >= nMaxRow)
            {
                OSL_FAIL("FillInfo: Range too big" );
                rYExtra = nSignedY;                         // End
                rRow2 = rYExtra - 1;                        // Adjust range
            }
        }
        else
            if (nSignedY == rYExtra)                 // hidden additional line?
                ++rYExtra;
    }
}

void initCellInfo(RowInfo* pRowInfo, SCSIZE nArrCount, SCCOL nStartCol, SCCOL nRotMax,
        const SvxShadowItem* pDefShadow)
{
    for (SCSIZE nArrRow = 0; nArrRow < nArrCount; ++nArrRow)
    {
        RowInfo& rThisRowInfo = pRowInfo[nArrRow];
        // A lot of memory (and performance allocating and initializing it) can
        // be saved if we do not allocate CellInfo for columns before nStartCol.
        // But code in ScOutputData::SetCellRotation(), ScOutputData::DrawRotatedFrame()
        // and ScOutputData::SetCellRotations() accesses those. That depends on
        // nRotMaxCol being set to something else than none, and the value is already
        // initialized here. So allocate all those cells starting from column 0 only if needed.
        SCCOL nMinCol = rThisRowInfo.nRotMaxCol != SC_ROTMAX_NONE ? 0 : nStartCol;
        rThisRowInfo.allocCellInfo( nMinCol, nRotMax + 1 );

        for (SCCOL nCol = nMinCol-1; nCol <= nRotMax+1; ++nCol) // Preassign cell info
        {
            ScCellInfo& rInfo = rThisRowInfo.cellInfo(nCol);
            rInfo.pShadowAttr    = pDefShadow;
        }
    }
}

void initColWidths(RowInfo* pRowInfo, const ScDocument* pDoc, double fColScale, SCTAB nTab, SCCOL nCol2, SCCOL nRotMax)
{
    for (SCCOL nCol=nCol2+2; nCol<=nRotMax+1; nCol++)    // Add remaining widths
    {
        if ( pDoc->ValidCol(nCol) )
        {
            if (!pDoc->ColHidden(nCol, nTab))
            {
                sal_uInt16 nThisWidth = static_cast<sal_uInt16>(pDoc->GetColWidth( nCol, nTab ) * fColScale);
                if (!nThisWidth)
                    nThisWidth = 1;

                pRowInfo[0].basicCellInfo(nCol).nWidth = nThisWidth;
            }
        }
    }
}

bool handleConditionalFormat(ScConditionalFormatList& rCondFormList, const ScCondFormatIndexes& rCondFormats,
        ScCellInfo* pInfo, ScTableInfo* pTableInfo, ScStyleSheetPool* pStlPool,
        const ScAddress& rAddr, bool& bHidden, bool& bHideFormula, bool bTabProtect)
{
    bool bAnyCondition = false;
    for(const auto& rCondFormat : rCondFormats)
    {
        ScConditionalFormat* pCondForm = rCondFormList.GetFormat(rCondFormat);
        if(!pCondForm)
            continue;

        ScCondFormatData aData = pCondForm->GetData(
                pInfo->maCell, rAddr);
        if (!bAnyCondition && !aData.aStyleName.isEmpty())
        {
            SfxStyleSheetBase* pStyleSheet =
                pStlPool->Find( aData.aStyleName, SfxStyleFamily::Para );
            if ( pStyleSheet )
            {
                //TODO: cache Style-Sets !!!
                pInfo->pConditionSet = &pStyleSheet->GetItemSet();
                bAnyCondition = true;

                // TODO: moggi: looks like there is a bug around bHidden and bHideFormula
                //              They are normally for the whole pattern and not for a single cell
                // we need to check already here for protected cells
                const ScProtectionAttr* pProtAttr;
                if ( bTabProtect && (pProtAttr = pInfo->pConditionSet->GetItemIfSet( ATTR_PROTECTION )) )
                {
                    bHidden = pProtAttr->GetHideCell();
                    bHideFormula = pProtAttr->GetHideFormula();

                }
            }
            // if style is not there, treat like no condition
        }

        if(aData.mxColorScale && !pInfo->mxColorScale)
        {
            pInfo->mxColorScale = aData.mxColorScale;
        }

        if(aData.pDataBar && !pInfo->pDataBar)
        {
            pInfo->pDataBar = aData.pDataBar.get();
            pTableInfo->addDataBarInfo(std::move(aData.pDataBar));
        }

        if(aData.pIconSet && !pInfo->pIconSet)
        {
            pInfo->pIconSet = aData.pIconSet.get();
            pTableInfo->addIconSetInfo(std::move(aData.pIconSet));
        }

        if (bAnyCondition && pInfo->mxColorScale && pInfo->pIconSet && pInfo->pDataBar)
            break;
    }

    return bAnyCondition;
}

}

void ScDocument::FillInfo(
    ScTableInfo& rTabInfo, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    SCTAB nTab, double fColScale, double fRowScale, bool bPageMode, bool bFormulaMode,
    const ScMarkData* pMarkData )
{
    OSL_ENSURE( maTabs[nTab], "Table does not exist" );

    bool bLayoutRTL = IsLayoutRTL( nTab );

    ScDocumentPool* pPool = mxPoolHelper->GetDocPool();
    ScStyleSheetPool* pStlPool = mxPoolHelper->GetStylePool();

    RowInfo* pRowInfo = rTabInfo.mpRowInfo.get();

    const SvxBrushItem* pDefBackground =
            &pPool->GetDefaultItem( ATTR_BACKGROUND );
    const ScMergeAttr* pDefMerge =
            &pPool->GetDefaultItem( ATTR_MERGE );
    const SvxShadowItem* pDefShadow =
            &pPool->GetDefaultItem( ATTR_SHADOW );

    SCSIZE nArrRow;
    SCSIZE nArrCount;
    bool bAnyMerged = false;
    bool bAnyShadow = false;
    bool bAnyCondition = false;
    bool bAnyPreview = false;

    bool bTabProtect = IsTabProtected(nTab);

    // first only the entries for the entire column

    nArrRow=0;
    SCROW nYExtra = nRow2+1;
    initRowInfo(this, pRowInfo, rTabInfo.mnArrCapacity, fRowScale, nRow1,
            nTab, nYExtra, nArrRow, nRow2);
    nArrCount = nArrRow;                                      // incl. Dummys

    // Rotated text...

    // Is Attribute really used in document?
    bool bAnyItem = isRotateItemUsed(pPool);

    SCCOL nRotMax = nCol2;
    if ( bAnyItem && HasAttrib( 0, nRow1, nTab, MaxCol(), nRow2+1, nTab,
                                HasAttrFlags::Rotate | HasAttrFlags::Conditional ) )
    {
        //TODO: check Conditionals also for HasAttrFlags::Rotate ????

        OSL_ENSURE( nArrCount>2, "nArrCount too small" );
        FindMaxRotCol( nTab, &pRowInfo[1], nArrCount-1, nCol1, nCol2 );
        //  FindMaxRotCol sets nRotMaxCol

        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
            if (pRowInfo[nArrRow].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nArrRow].nRotMaxCol > nRotMax)
                nRotMax = pRowInfo[nArrRow].nRotMaxCol;
    }

    //  Allocate cell information only after the test rotation
    //  to nRotMax due to nRotateDir Flag
    initCellInfo(pRowInfo, nArrCount, nCol1, nRotMax, pDefShadow);

    initColWidths(pRowInfo, this, fColScale, nTab, nCol2, nRotMax);

    ScConditionalFormatList* pCondFormList = GetCondFormList(nTab);
    if (pCondFormList)
        pCondFormList->startRendering();

    SCCOL nLastHiddenCheckedCol = -2;
    bool bColHidden = false;
    for (SCCOL nCol=-1; nCol<=nCol2+1; nCol++) // collect basic info also for all previous cols, and left & right + 1
    {
        if (ValidCol(nCol))
        {
            // #i58049#, #i57939# Hidden columns must be skipped here, or their attributes
            // will disturb the output

            if (nCol > nLastHiddenCheckedCol)
                bColHidden = ColHidden(nCol, nTab, nullptr, &nLastHiddenCheckedCol);
            // TODO: Optimize this loop.
            if (!bColHidden)
            {
                sal_uInt16 nColWidth = GetColWidth( nCol, nTab, false ); // false=no need to check for hidden, checked above
                sal_uInt16 nThisWidth = static_cast<sal_uInt16>(std::clamp(nColWidth * fColScale, 1.0, double(std::numeric_limits<sal_uInt16>::max())));

                pRowInfo[0].basicCellInfo(nCol).nWidth = nThisWidth;           //TODO: this should be enough

                const ScAttrArray* pThisAttrArr; // Attribute
                if (nCol < maTabs[nTab]->GetAllocatedColumnsCount())
                {
                    ScColumn* pThisCol = &maTabs[nTab]->aCol[nCol]; // Column data

                    nArrRow = 1;
                    // Iterate between rows nY1 and nY2 and pick up non-empty
                    // cells that are not hidden.
                    RowInfoFiller aFunc(*this, nTab, pRowInfo, nCol, nArrRow, nCol1);
                    sc::ParseAllNonEmpty(pThisCol->maCells.begin(), pThisCol->maCells, nRow1, nRow2,
                                         aFunc);

                    pThisAttrArr = pThisCol->pAttrArray.get();
                }
                else
                    pThisAttrArr = &maTabs[nTab]->aDefaultColData.AttrArray();

                if (nCol+1 >= nCol1)                                // Attribute/Blockmark from nX1-1
                {
                    nArrRow = 0;

                    SCROW nCurRow=nRow1;                  // single rows
                    if (nCurRow>0)
                        --nCurRow;                      // 1 more on top
                    else
                        nArrRow = 1;

                    SCROW nThisRow;
                    SCSIZE nIndex;
                    if ( pThisAttrArr->Count() )
                        (void) pThisAttrArr->Search( nCurRow, nIndex );
                    else
                        nIndex = 0;

                    do
                    {
                        const ScPatternAttr* pPattern = nullptr;
                        if ( pThisAttrArr->Count() )
                        {
                            nThisRow = pThisAttrArr->mvData[nIndex].nEndRow;              // End of range
                            pPattern = pThisAttrArr->mvData[nIndex].pPattern;
                        }
                        else
                        {
                            nThisRow = MaxRow();
                            pPattern = GetDefPattern();
                        }

                        const SvxBrushItem* pBackground = &pPattern->GetItem(ATTR_BACKGROUND);
                        const SvxBoxItem* pLinesAttr = &pPattern->GetItem(ATTR_BORDER);

                        const SvxLineItem* pTLBRLine = &pPattern->GetItem( ATTR_BORDER_TLBR );
                        const SvxLineItem* pBLTRLine = &pPattern->GetItem( ATTR_BORDER_BLTR );

                        const SvxShadowItem* pShadowAttr = &pPattern->GetItem(ATTR_SHADOW);
                        if (!SfxPoolItem::areSame(pShadowAttr, pDefShadow))
                            bAnyShadow = true;

                        const ScMergeAttr* pMergeAttr = &pPattern->GetItem(ATTR_MERGE);
                        bool bMerged = !SfxPoolItem::areSame( pMergeAttr, pDefMerge );
                        ScMF nOverlap = pPattern->GetItemSet().
                                                        Get(ATTR_MERGE_FLAG).GetValue();
                        bool bHOverlapped(nOverlap & ScMF::Hor);
                        bool bVOverlapped(nOverlap & ScMF::Ver);
                        bool bAutoFilter(nOverlap & ScMF::Auto);
                        bool bPivotButton(nOverlap & ScMF::Button);
                        bool bScenario(nOverlap & ScMF::Scenario);
                        bool bPivotPopupButton(nOverlap & ScMF::ButtonPopup);
                        bool bFilterActive(nOverlap & ScMF::HiddenMember);
                        bool bPivotCollapseButton(nOverlap & ScMF::DpCollapse);
                        bool bPivotExpandButton(nOverlap & ScMF::DpExpand);
                        bool bPivotPopupButtonMulti(nOverlap & ScMF::ButtonPopup2);
                        if (bMerged||bHOverlapped||bVOverlapped)
                            bAnyMerged = true;                              // internal

                        bool bHidden, bHideFormula;
                        if (bTabProtect)
                        {
                            const ScProtectionAttr& rProtAttr = pPattern->GetItem(ATTR_PROTECTION);
                            bHidden = rProtAttr.GetHideCell();
                            bHideFormula = rProtAttr.GetHideFormula();
                        }
                        else
                            bHidden = bHideFormula = false;

                        const ScCondFormatIndexes& rCondFormats = pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData();
                        bool bContainsCondFormat = !rCondFormats.empty();

                        do
                        {
                            SCROW nLastHiddenRow = -1;
                            bool bRowHidden = RowHidden(nCurRow, nTab, nullptr, &nLastHiddenRow);
                            if ( nArrRow==0 || !bRowHidden )
                            {
                                if ( GetPreviewCellStyle( nCol, nCurRow, nTab  ) != nullptr )
                                    bAnyPreview = true;
                                RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
                                if (!SfxPoolItem::areSame(pBackground, pDefBackground))          // Column background == Default ?
                                    pThisRowInfo->bEmptyBack = false;
                                if (bContainsCondFormat)
                                    pThisRowInfo->bEmptyBack = false;
                                if (bAutoFilter)
                                    pThisRowInfo->bAutoFilter = true;
                                if (bPivotButton || bPivotPopupButton || bPivotPopupButtonMulti)
                                    pThisRowInfo->bPivotButton = true;
                                if (bPivotCollapseButton || bPivotExpandButton)
                                    pThisRowInfo->bPivotToggle = true;

                                ScCellInfo* pInfo = &pThisRowInfo->cellInfo(nCol);
                                ScBasicCellInfo* pBasicInfo = &pThisRowInfo->basicCellInfo(nCol);
                                pInfo->pBackground  = pBackground;
                                pInfo->pPatternAttr = pPattern;
                                pInfo->bMerged      = bMerged;
                                pInfo->bHOverlapped = bHOverlapped;
                                pInfo->bVOverlapped = bVOverlapped;
                                pInfo->bAutoFilter  = bAutoFilter;
                                pInfo->bPivotButton  = bPivotButton;
                                pInfo->bPivotPopupButton = bPivotPopupButton;
                                pInfo->bPivotCollapseButton = bPivotCollapseButton;
                                pInfo->bPivotExpandButton = bPivotExpandButton;
                                pInfo->bPivotPopupButtonMulti = bPivotPopupButtonMulti;
                                pInfo->bFilterActive = bFilterActive;
                                pInfo->pLinesAttr   = pLinesAttr;
                                pInfo->mpTLBRLine   = pTLBRLine;
                                pInfo->mpBLTRLine   = pBLTRLine;
                                pInfo->pShadowAttr  = pShadowAttr;
                                // nWidth is no longer set individually

                                if (bScenario)
                                {
                                    pInfo->pBackground = ScGlobal::GetButtonBrushItem();
                                    pThisRowInfo->bEmptyBack = false;
                                }

                                if (bContainsCondFormat && pCondFormList)
                                {
                                    bAnyCondition |= handleConditionalFormat(*pCondFormList, rCondFormats,
                                            pInfo, &rTabInfo, pStlPool, ScAddress(nCol, nCurRow, nTab),
                                            bHidden, bHideFormula, bTabProtect);
                                }

                                if (bHidden || (bFormulaMode && bHideFormula && pInfo->maCell.getType() == CELLTYPE_FORMULA))
                                    pBasicInfo->bEmptyCellText = true;

                                ++nArrRow;
                            }
                            else if (nLastHiddenRow >= 0)
                            {
                                nCurRow = nLastHiddenRow;
                                if (nCurRow > nThisRow)
                                    nCurRow = nThisRow;
                            }
                            ++nCurRow;
                        }
                        while (nCurRow <= nThisRow && nCurRow <= nYExtra);
                        ++nIndex;
                    }
                    while ( nIndex < pThisAttrArr->Count() && nThisRow < nYExtra );

                    if (pMarkData && pMarkData->IsMultiMarked())
                    {
                        //  Block marks
                        ScMarkArray aThisMarkArr(pMarkData->GetMarkArray( nCol ));
                        nArrRow = 1;
                        nCurRow = nRow1;                                      // single rows

                        if ( aThisMarkArr.Search( nRow1, nIndex ) )
                        {
                            do
                            {
                                nThisRow=aThisMarkArr.mvData[nIndex].nRow;      // End of range

                                do
                                {
                                    if ( !RowHidden( nCurRow,nTab ) )
                                    {
                                        ++nArrRow;
                                    }
                                    ++nCurRow;
                                }
                                while (nCurRow <= nThisRow && nCurRow <= nRow2);
                                ++nIndex;
                            }
                            while ( nIndex < aThisMarkArr.mvData.size() && nThisRow < nRow2 );
                        }
                    }
                }
                else                                    // columns in front
                {
                    for (nArrRow=1; nArrRow+1<nArrCount; nArrRow++)
                        pRowInfo[nArrRow].basicCellInfo(nCol).nWidth = nThisWidth; //TODO: or check only 0 ??
                }
            }
        }
        else
            pRowInfo[0].basicCellInfo(nCol).nWidth = STD_COL_WIDTH;
        // STD_COL_WIDTH farthest to the left and right is needed for DrawExtraShadow
    }

    if (pCondFormList)
        pCondFormList->endRendering();

    // evaluate conditional formatting
    std::vector< std::unique_ptr<ScPatternAttr> > aAltPatterns;
    // favour preview over condition
    if (bAnyCondition || bAnyPreview)
    {
        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
        {
            for (SCCOL nCol=nCol1-1; nCol<=nCol2+1; nCol++)                  // 1 more left and right
            {
                ScCellInfo* pInfo = &pRowInfo[nArrRow].cellInfo(nCol);
                ScPatternAttr* pModifiedPatt = nullptr;

                if ( ValidCol(nCol) && pRowInfo[nArrRow].nRowNo <= MaxRow() )
                {
                    if ( ScStyleSheet* pPreviewStyle = GetPreviewCellStyle( nCol, pRowInfo[nArrRow].nRowNo, nTab ) )
                    {
                        aAltPatterns.push_back( std::make_unique<ScPatternAttr>( *pInfo->pPatternAttr ) );
                        pModifiedPatt = aAltPatterns.back().get();
                        pModifiedPatt->SetStyleSheet( pPreviewStyle );
                    }
                }
                // favour preview over condition
                const SfxItemSet* pCondSet = pModifiedPatt ? &pModifiedPatt->GetItemSet() : pInfo->pConditionSet;

                if (pCondSet)
                {
                            // Background
                    if ( const SvxBrushItem* pItem = pCondSet->GetItemIfSet( ATTR_BACKGROUND ) )
                    {
                        pInfo->pBackground = pItem;
                        pRowInfo[nArrRow].bEmptyBack = false;
                    }

                            // Border
                    if ( const SvxBoxItem* pItem = pCondSet->GetItemIfSet( ATTR_BORDER ) )
                        pInfo->pLinesAttr = pItem;

                    if ( const SvxLineItem* pItem = pCondSet->GetItemIfSet( ATTR_BORDER_TLBR ) )
                        pInfo->mpTLBRLine = pItem;
                    if ( const SvxLineItem* pItem = pCondSet->GetItemIfSet( ATTR_BORDER_BLTR ) )
                        pInfo->mpBLTRLine = pItem;

                            //  Shadow
                    if ( const SvxShadowItem* pItem = pCondSet->GetItemIfSet( ATTR_SHADOW ) )
                    {
                        pInfo->pShadowAttr = pItem;
                        bAnyShadow = true;
                    }
                }
                if( bAnyCondition && pInfo->mxColorScale)
                {
                    pRowInfo[nArrRow].bEmptyBack = false;
                    pInfo->pBackground = &pPool->DirectPutItemInPool(SvxBrushItem(*pInfo->mxColorScale, ATTR_BACKGROUND));
                }
            }
        }
    }

    // End conditional formatting

                //      Adjust data from merged cells

    if (bAnyMerged)
    {
        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
            SCROW nSignedY = nArrRow ? pThisRowInfo->nRowNo : nRow1-1;

            for (SCCOL nCol=nCol1-1; nCol<=nCol2+1; nCol++)                  // 1 more left and right
            {
                ScCellInfo* pInfo = &pThisRowInfo->cellInfo(nCol);

                if (pInfo->bMerged || pInfo->bHOverlapped || pInfo->bVOverlapped)
                {
                    SCCOL nStartX;
                    SCROW nStartY;
                    SCCOL nEndX;
                    SCROW nEndY;
                    lcl_GetMergeRange( nCol,nSignedY, nArrRow, this,pRowInfo, nCol1,nRow1,nTab,
                                        nStartX,nStartY, nEndX,nEndY );
                    const ScPatternAttr* pStartPattern = GetPattern( nStartX,nStartY,nTab );
                    const SfxItemSet* pStartCond = GetCondResult( nStartX,nStartY,nTab );

                    // Copy Background (or in output.cxx)

                    const SvxBrushItem* pBrushItem = nullptr;
                    if ( !pStartCond ||
                        !(pBrushItem = pStartCond->GetItemIfSet(ATTR_BACKGROUND)) )
                        pBrushItem = &pStartPattern->GetItem(ATTR_BACKGROUND);
                    pInfo->pBackground = pBrushItem;
                    pRowInfo[nArrRow].bEmptyBack = false;

                    // Shadow

                    const SvxShadowItem* pShadowItem = nullptr;
                    if ( !pStartCond ||
                        !(pShadowItem = pStartCond->GetItemIfSet(ATTR_SHADOW)) )
                        pShadowItem = &pStartPattern->GetItem(ATTR_SHADOW);
                    pInfo->pShadowAttr = pShadowItem;
                    if (!SfxPoolItem::areSame(pInfo->pShadowAttr, pDefShadow))
                        bAnyShadow = true;

                    const ScCondFormatIndexes& rCondFormatIndex
                        = pStartPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData();

                    if (pCondFormList && !pStartCond && !rCondFormatIndex.empty())
                    {
                        for (const auto& rItem : rCondFormatIndex)
                        {
                            const ScConditionalFormat* pCondForm = pCondFormList->GetFormat(rItem);
                            if (pCondForm)
                            {
                                ScCondFormatData aData = pCondForm->GetData(
                                    pInfo->maCell, ScAddress(nStartX, nStartY, nTab));

                                // Color scale
                                if (aData.mxColorScale && !pInfo->mxColorScale)
                                    pInfo->mxColorScale = aData.mxColorScale;
                            }
                        }
                    }
                }
            }
        }
    }

    if (bAnyShadow)                             // distribute Shadow
    {
        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
        {
            bool bTop = ( nArrRow == 0 );
            bool bBottom = ( nArrRow+1 == nArrCount );

            for (SCCOL nCol=nCol1-1; nCol<=nCol2+1; nCol++)                  // 1 more left and right
            {
                bool bLeft = ( nCol == nCol1-1 );
                bool bRight = ( nCol == nCol2+1 );

                ScCellInfo* pInfo = &pRowInfo[nArrRow].cellInfo(nCol);
                const SvxShadowItem* pThisAttr = pInfo->pShadowAttr;
                SvxShadowLocation eLoc = pThisAttr ? pThisAttr->GetLocation() : SvxShadowLocation::NONE;
                if (eLoc != SvxShadowLocation::NONE)
                {
                    //  or test on != eLoc

                    SCCOL nDxPos = 1;
                    SCCOL nDxNeg = -1;

                    while ( nCol+nDxPos < nCol2+1 && pRowInfo[0].basicCellInfo(nCol+nDxPos).nWidth == 0 )
                        ++nDxPos;
                    while ( nCol+nDxNeg > nCol1-1 && pRowInfo[0].basicCellInfo(nCol+nDxNeg).nWidth == 0 )
                        --nDxNeg;

                    bool bLeftDiff = !bLeft &&
                            pRowInfo[nArrRow].cellInfo(nCol+nDxNeg).pShadowAttr->GetLocation() == SvxShadowLocation::NONE;
                    bool bRightDiff = !bRight &&
                            pRowInfo[nArrRow].cellInfo(nCol+nDxPos).pShadowAttr->GetLocation() == SvxShadowLocation::NONE;
                    bool bTopDiff = !bTop &&
                            pRowInfo[nArrRow-1].cellInfo(nCol).pShadowAttr->GetLocation() == SvxShadowLocation::NONE;
                    bool bBottomDiff = !bBottom &&
                            pRowInfo[nArrRow+1].cellInfo(nCol).pShadowAttr->GetLocation() == SvxShadowLocation::NONE;

                    if ( bLayoutRTL )
                    {
                        switch (eLoc)
                        {
                            case SvxShadowLocation::BottomRight: eLoc = SvxShadowLocation::BottomLeft;  break;
                            case SvxShadowLocation::BottomLeft:  eLoc = SvxShadowLocation::BottomRight; break;
                            case SvxShadowLocation::TopRight:    eLoc = SvxShadowLocation::TopLeft;     break;
                            case SvxShadowLocation::TopLeft:     eLoc = SvxShadowLocation::TopRight;    break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }
                    }

                    switch (eLoc)
                    {
                        case SvxShadowLocation::BottomRight:
                            if (bBottomDiff)
                            {
                                pRowInfo[nArrRow+1].cellInfo(nCol).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].cellInfo(nCol).eHShadowPart =
                                                bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bRightDiff)
                            {
                                pRowInfo[nArrRow].cellInfo(nCol+1).pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].cellInfo(nCol+1).eVShadowPart =
                                                bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bBottomDiff && bRightDiff)
                            {
                                pRowInfo[nArrRow+1].cellInfo(nCol+1).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].cellInfo(nCol+1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SvxShadowLocation::BottomLeft:
                            if (bBottomDiff)
                            {
                                pRowInfo[nArrRow+1].cellInfo(nCol).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].cellInfo(nCol).eHShadowPart =
                                                bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bLeftDiff)
                            {
                                pRowInfo[nArrRow].cellInfo(nCol-1).pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].cellInfo(nCol-1).eVShadowPart =
                                                bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bBottomDiff && bLeftDiff)
                            {
                                pRowInfo[nArrRow+1].cellInfo(nCol-1).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].cellInfo(nCol-1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SvxShadowLocation::TopRight:
                            if (bTopDiff)
                            {
                                pRowInfo[nArrRow-1].cellInfo(nCol).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].cellInfo(nCol).eHShadowPart =
                                                bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bRightDiff)
                            {
                                pRowInfo[nArrRow].cellInfo(nCol+1).pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].cellInfo(nCol+1).eVShadowPart =
                                                bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bTopDiff && bRightDiff)
                            {
                                pRowInfo[nArrRow-1].cellInfo(nCol+1).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].cellInfo(nCol+1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SvxShadowLocation::TopLeft:
                            if (bTopDiff)
                            {
                                pRowInfo[nArrRow-1].cellInfo(nCol).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].cellInfo(nCol).eHShadowPart =
                                                bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bLeftDiff)
                            {
                                pRowInfo[nArrRow].cellInfo(nCol-1).pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].cellInfo(nCol-1).eVShadowPart =
                                                bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bTopDiff && bLeftDiff)
                            {
                                pRowInfo[nArrRow-1].cellInfo(nCol-1).pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].cellInfo(nCol-1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        default:
                            OSL_FAIL("wrong Shadow-Enum");
                    }
                }
            }
        }
    }

    rTabInfo.mnArrCount = sal::static_int_cast<sal_uInt16>(nArrCount);
    rTabInfo.mbPageMode = bPageMode;

    // *** create the frame border array ***

    // RowInfo structs are filled in the range [ 0 , nArrCount-1 ],
    // each RowInfo contains ScCellInfo structs in the range [ nCol1-1 , nCol2+1 ]
    // and ScBasicCellInfo structs in the range [ -1, nCol2+1 ]

    size_t nColCount = nCol2 - nCol1 + 1 + 2;
    size_t nRowCount = nArrCount;

    svx::frame::Array& rArray = rTabInfo.maArray;
    rArray.Initialize( nColCount, nRowCount );

    for( size_t nRow = 0; nRow < nRowCount; ++nRow )
    {
        sal_uInt16 nCellInfoY = static_cast< sal_uInt16 >( nRow );
        RowInfo& rThisRowInfo = pRowInfo[ nCellInfoY ];

        for( SCCOL nCol = nCol1 - 1; nCol <= nCol2 + 1; ++nCol ) // 1 more left and right
        {
            const ScCellInfo& rInfo = rThisRowInfo.cellInfo( nCol );
            const SvxBoxItem* pBox = rInfo.pLinesAttr;
            const SvxLineItem* pTLBR = rInfo.mpTLBRLine;
            const SvxLineItem* pBLTR = rInfo.mpBLTRLine;

            size_t colToIndex = -(nCol1 - 1);
            // These are rArray indexes (0-based), not really rows/columns.
            size_t nX = nCol + colToIndex;
            size_t nFirstCol = nX;
            size_t nFirstRow = nRow;

            // *** merged cells *** -------------------------------------------

            if( !rArray.IsMerged( nX, nRow ) && (rInfo.bMerged || rInfo.bHOverlapped || rInfo.bVOverlapped) )
            {
                // *** insert merged range in svx::frame::Array ***

                /*  #i69369# top-left cell of a merged range may be located in
                    a hidden column or row. Use lcl_GetMergeRange() to find the
                    complete merged range, then calculate dimensions and
                    document position of the visible range. */

                // note: document rows must be looked up in RowInfo structs

                // current column and row in document coordinates
                SCCOL nCurrDocCol = nCol;
                SCROW nCurrDocRow = static_cast< SCROW >( (nCellInfoY > 0) ? rThisRowInfo.nRowNo : (nRow1 - 1) );

                // find entire merged range in document, returns signed document coordinates
                SCCOL nFirstRealDocColS, nLastRealDocColS;
                SCROW nFirstRealDocRowS, nLastRealDocRowS;
                lcl_GetMergeRange( nCurrDocCol, nCurrDocRow,
                    nCellInfoY, this, pRowInfo, nCol1,nRow1,nTab,
                    nFirstRealDocColS, nFirstRealDocRowS, nLastRealDocColS, nLastRealDocRowS );

                // *complete* merged range in document coordinates
                SCCOL nFirstRealDocCol = nFirstRealDocColS;
                SCROW nFirstRealDocRow = nFirstRealDocRowS;
                SCCOL nLastRealDocCol  = nLastRealDocColS;
                SCROW nLastRealDocRow  = nLastRealDocRowS;

                // first visible column (nCol1-1 is first processed document column)
                SCCOL nFirstDocCol = (nCol1 > 0) ? ::std::max< SCCOL >( nFirstRealDocCol, nCol1 - 1 ) : nFirstRealDocCol;
                nFirstCol = nFirstDocCol + colToIndex;

                // last visible column (nCol2+1 is last processed document column)
                SCCOL nLastDocCol = (nCol2 < MaxCol()) ? ::std::min< SCCOL >( nLastRealDocCol, nCol2 + 1 ) : nLastRealDocCol;
                size_t nLastCol = nLastDocCol + colToIndex;

                // first visible row
                sal_uInt16 nFirstCellInfoY = nCellInfoY;
                while( ((nFirstCellInfoY > 1) && (pRowInfo[ nFirstCellInfoY - 1 ].nRowNo >= nFirstRealDocRow)) ||
                       ((nFirstCellInfoY == 1) && (static_cast< SCROW >( nRow1 - 1 ) >= nFirstRealDocRow)) )
                    --nFirstCellInfoY;
                SCROW nFirstDocRow = (nFirstCellInfoY > 0) ? pRowInfo[ nFirstCellInfoY ].nRowNo : static_cast< SCROW >( nRow1 - 1 );
                nFirstRow = static_cast< size_t >( nFirstCellInfoY );

                // last visible row
                sal_uInt16 nLastCellInfoY = nCellInfoY;
                while( (sal::static_int_cast<SCSIZE>(nLastCellInfoY + 1) < nArrCount) &&
                            (pRowInfo[ nLastCellInfoY + 1 ].nRowNo <= nLastRealDocRow) )
                    ++nLastCellInfoY;
                SCROW nLastDocRow = (nLastCellInfoY > 0) ? pRowInfo[ nLastCellInfoY ].nRowNo : static_cast< SCROW >( nRow1 - 1 );
                size_t nLastRow = static_cast< size_t >( nLastCellInfoY );

                // insert merged range
                rArray.SetMergedRange( nFirstCol, nFirstRow, nLastCol, nLastRow );

                // *** find additional size not included in svx::frame::Array ***

                // additional space before first column
                if( nFirstCol == 0 )
                {
                    tools::Long nSize = 0;
                    for( SCCOL nDocCol = nFirstRealDocCol; nDocCol < nFirstDocCol; ++nDocCol )
                        nSize += std::max( tools::Long(GetColWidth( nDocCol, nTab ) * fColScale), tools::Long(1) );
                    rArray.SetAddMergedLeftSize( nX, nRow, nSize );
                }
                // additional space after last column
                if( nLastCol + 1 == nColCount )
                {
                    tools::Long nSize = 0;
                    for( SCCOL nDocCol = nLastDocCol + 1; nDocCol <= nLastRealDocCol; ++nDocCol )
                        nSize += std::max( tools::Long(GetColWidth( nDocCol, nTab ) * fColScale), tools::Long(1) );
                    rArray.SetAddMergedRightSize( nX, nRow, nSize );
                }
                // additional space above first row
                if( nFirstRow == 0 )
                {
                    tools::Long nSize = 0;
                    for( SCROW nDocRow = nFirstRealDocRow; nDocRow < nFirstDocRow; ++nDocRow )
                        nSize += std::max( tools::Long(GetRowHeight( nDocRow, nTab ) * fRowScale), tools::Long(1) );
                    rArray.SetAddMergedTopSize( nX, nRow, nSize );
                }
                // additional space beyond last row
                if( nLastRow + 1 == nRowCount )
                {
                    tools::Long nSize = 0;
                    for( SCROW nDocRow = nLastDocRow + 1; nDocRow <= nLastRealDocRow; ++nDocRow )
                        nSize += std::max( tools::Long(GetRowHeight( nDocRow, nTab ) * fRowScale), tools::Long(1) );
                    rArray.SetAddMergedBottomSize( nX, nRow, nSize );
                }

                // *** use line attributes from real origin cell ***

                if( (nFirstRealDocCol != nCurrDocCol) || (nFirstRealDocRow != nCurrDocRow) )
                {
                    if( const ScPatternAttr* pPattern = GetPattern( nFirstRealDocCol, nFirstRealDocRow, nTab ) )
                    {
                        const SfxItemSet* pCond = GetCondResult( nFirstRealDocCol, nFirstRealDocRow, nTab );
                        pBox = &pPattern->GetItem( ATTR_BORDER, pCond );
                        pTLBR = &pPattern->GetItem( ATTR_BORDER_TLBR, pCond );
                        pBLTR = &pPattern->GetItem( ATTR_BORDER_BLTR, pCond );
                    }
                    else
                    {
                        pBox = nullptr;
                        pTLBR = pBLTR = nullptr;
                    }
                }
            }

            // *** borders *** ------------------------------------------------

            if( pBox )
            {
                rArray.SetCellStyleLeft(   nFirstCol, nFirstRow, svx::frame::Style( pBox->GetLeft(),   fColScale ) );
                rArray.SetCellStyleRight(  nFirstCol, nFirstRow, svx::frame::Style( pBox->GetRight(),  fColScale ) );
                rArray.SetCellStyleTop(    nFirstCol, nFirstRow, svx::frame::Style( pBox->GetTop(),    fRowScale ) );
                rArray.SetCellStyleBottom( nFirstCol, nFirstRow, svx::frame::Style( pBox->GetBottom(), fRowScale ) );
            }

            if( pTLBR )
                rArray.SetCellStyleTLBR( nFirstCol, nFirstRow, svx::frame::Style( pTLBR->GetLine(), fRowScale ) );
            if( pBLTR )
                rArray.SetCellStyleBLTR( nFirstCol, nFirstRow, svx::frame::Style( pBLTR->GetLine(), fRowScale ) );
        }
    }

    /*  Mirror the entire frame array. */
    if( bLayoutRTL )
        rArray.MirrorSelfX();
}

ScTableInfo::ScTableInfo(const SCSIZE capacity)
    : mpRowInfo(new RowInfo[capacity])
    , mnArrCount(0)
    , mnArrCapacity(capacity)
    , mbPageMode(false)
{
    memset(static_cast<void*>(mpRowInfo.get()), 0, mnArrCapacity * sizeof(RowInfo));
}

ScTableInfo::~ScTableInfo()
{
    for( SCSIZE nIdx = 0; nIdx < mnArrCapacity; ++nIdx )
        mpRowInfo[ nIdx ].freeCellInfo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
