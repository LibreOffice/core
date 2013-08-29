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
#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/editdata.hxx>     // can be removed if table has a bLayoutRTL flag
#include <editeng/shaditem.hxx>
#include <editeng/brushitem.hxx>

#include "fillinfo.hxx"
#include "document.hxx"
#include "formulacell.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "attarray.hxx"
#include "markarr.hxx"
#include "markdata.hxx"
#include "patattr.hxx"
#include "poolhelp.hxx"
#include "docpool.hxx"
#include "conditio.hxx"
#include "colorscale.hxx"
#include "stlpool.hxx"
#include "cellvalue.hxx"
#include "mtvcellfunc.hxx"

const sal_uInt16 ROWINFO_MAX = 1024;


enum FillInfoLinePos
    {
        FILP_TOP,
        FILP_BOTTOM,
        FILP_LEFT,
        FILP_RIGHT
    };


//  aehnlich wie in output.cxx

static void lcl_GetMergeRange( SCsCOL nX, SCsROW nY, SCSIZE nArrY,
                            ScDocument* pDoc, RowInfo* pRowInfo,
                            SCCOL nX1, SCROW nY1, SCCOL /* nX2 */, SCROW /* nY2 */, SCTAB nTab,
                            SCsCOL& rStartX, SCsROW& rStartY, SCsCOL& rEndX, SCsROW& rEndY )
{
    CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nX+1];

    rStartX = nX;
    rStartY = nY;
    bool bHOver = pInfo->bHOverlapped;
    bool bVOver = pInfo->bVOverlapped;
    SCCOL nLastCol;
    SCROW nLastRow;

    while (bHOver)              // nY konstant
    {
        --rStartX;
        if (rStartX >= (SCsCOL) nX1 && !pDoc->ColHidden(rStartX, nTab, NULL, &nLastCol))
        {
            bHOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bVOverlapped;
        }
        else
        {
            sal_uInt16 nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rStartX, rStartY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    while (bVOver)
    {
        --rStartY;

        if (nArrY>0)
            --nArrY;                        // lokale Kopie !

        if (rStartX >= (SCsCOL) nX1 && rStartY >= (SCsROW) nY1 &&
            !pDoc->ColHidden(rStartX, nTab, NULL, &nLastCol) &&
            !pDoc->RowHidden(rStartY, nTab, NULL, &nLastRow) &&
            (SCsROW) pRowInfo[nArrY].nRowNo == rStartY)
        {
            bVOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bVOverlapped;
        }
        else
        {
            sal_uInt16 nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rStartX, rStartY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    const ScMergeAttr* pMerge;
    if (rStartX >= (SCsCOL) nX1 && rStartY >= (SCsROW) nY1 &&
        !pDoc->ColHidden(rStartX, nTab, NULL, &nLastCol) &&
        !pDoc->RowHidden(rStartY, nTab, NULL, &nLastRow) &&
        (SCsROW) pRowInfo[nArrY].nRowNo == rStartY)
    {
        pMerge = (const ScMergeAttr*) &pRowInfo[nArrY].pCellInfo[rStartX+1].pPatternAttr->
                                        GetItem(ATTR_MERGE);
    }
    else
        pMerge = (const ScMergeAttr*) pDoc->GetAttr(rStartX,rStartY,nTab,ATTR_MERGE);

    rEndX = rStartX + pMerge->GetColMerge() - 1;
    rEndY = rStartY + pMerge->GetRowMerge() - 1;
}

namespace {

class RowInfoFiller
{
    ScDocument& mrDoc;
    SCTAB mnTab;
    RowInfo* mpRowInfo;
    SCCOL mnArrX;
    SCSIZE& mrArrY;
    SCROW mnHiddenEndRow;
    bool mbHiddenRow;

    bool isHidden(size_t nRow)
    {
        SCROW nThisRow = static_cast<SCROW>(nRow);
        if (nThisRow > mnHiddenEndRow)
            mbHiddenRow = mrDoc.RowHidden(nThisRow, mnTab, NULL, &mnHiddenEndRow);
        return mbHiddenRow;
    }

    void alignArray(size_t nRow)
    {
        while (mpRowInfo[mrArrY].nRowNo < static_cast<SCROW>(nRow))
            ++mrArrY;
    }

    void setInfo(size_t nRow, const ScRefCellValue& rCell)
    {
        alignArray(nRow);

        RowInfo* pThisRowInfo = &mpRowInfo[mrArrY];
        CellInfo* pInfo = &pThisRowInfo->pCellInfo[mnArrX];
        pInfo->maCell = rCell;
        pThisRowInfo->bEmptyText = false;
        pInfo->bEmptyCellText = false;
        ++mrArrY;
    }

public:
    RowInfoFiller(ScDocument& rDoc, SCTAB nTab, RowInfo* pRowInfo, SCCOL nArrX, SCSIZE& rArrY) :
        mrDoc(rDoc), mnTab(nTab), mpRowInfo(pRowInfo), mnArrX(nArrX), mrArrY(rArrY),
        mnHiddenEndRow(-1), mbHiddenRow(false) {}

    void operator() (size_t nRow, double fVal)
    {
        if (!isHidden(nRow))
            setInfo(nRow, ScRefCellValue(fVal));
    }

    void operator() (size_t nRow, const OUString& rStr)
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

}

void ScDocument::FillInfo(
    ScTableInfo& rTabInfo, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
    SCTAB nTab, double fColScale, double fRowScale, bool bPageMode, bool bFormulaMode,
    const ScMarkData* pMarkData )
{
    OSL_ENSURE( maTabs[nTab], "Table does not exist" );

    bool bLayoutRTL = IsLayoutRTL( nTab );

    ScDocumentPool* pPool = xPoolHelper->GetDocPool();
    ScStyleSheetPool* pStlPool = xPoolHelper->GetStylePool();

    RowInfo* pRowInfo = rTabInfo.mpRowInfo;

    const SvxBrushItem* pDefBackground =
            (const SvxBrushItem*) &pPool->GetDefaultItem( ATTR_BACKGROUND );
    const ScMergeAttr* pDefMerge =
            (const ScMergeAttr*) &pPool->GetDefaultItem( ATTR_MERGE );
    const SvxShadowItem* pDefShadow =
            (const SvxShadowItem*) &pPool->GetDefaultItem( ATTR_SHADOW );

    SCROW nThisRow;
    SCCOL nX;
    SCROW nY;
    SCsROW nSignedY;
    SCCOL nArrCol;
    SCSIZE nArrRow;
    SCSIZE nArrCount;
    bool bAnyMerged = false;
    bool bAnyShadow = false;
    bool bAnyCondition = false;
    bool bAnyPreview = false;

    bool bTabProtect = IsTabProtected(nTab);

                                                // fuer Blockmarken von zusammengefassten Zellen mit
                                                // versteckter erster Zeile / Spalte
    bool bPaintMarks = false;
    bool bSkipMarks = false;
    SCCOL nBlockStartX = 0, nBlockEndX = 0;
    SCROW nBlockEndY = 0, nBlockStartY = 0;
    if (pMarkData && pMarkData->IsMarked())
    {
        ScRange aTmpRange;
        pMarkData->GetMarkArea(aTmpRange);
        if ( nTab >= aTmpRange.aStart.Tab() && nTab <= aTmpRange.aEnd.Tab() )
        {
            nBlockStartX = aTmpRange.aStart.Col();
            nBlockStartY = aTmpRange.aStart.Row();
            nBlockEndX = aTmpRange.aEnd.Col();
            nBlockEndY = aTmpRange.aEnd.Row();
            ExtendHidden( nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY, nTab );   //? noetig ?
            if (pMarkData->IsMarkNegative())
                bSkipMarks = true;
            else
                bPaintMarks = true;
        }
    }

    //  zuerst nur die Eintraege fuer die ganze Spalte

    nArrRow=0;
    SCROW nYExtra = nRow2+1;
    sal_uInt16 nDocHeight = ScGlobal::nStdRowHeight;
    SCROW nDocHeightEndRow = -1;
    for (nSignedY=((SCsROW)nRow1)-1; nSignedY<=(SCsROW)nYExtra; nSignedY++)
    {
        if (nSignedY >= 0)
            nY = (SCROW) nSignedY;
        else
            nY = MAXROW+1;          // ungueltig

        if (nY > nDocHeightEndRow)
        {
            if (ValidRow(nY))
                nDocHeight = GetRowHeight( nY, nTab, NULL, &nDocHeightEndRow );
            else
                nDocHeight = ScGlobal::nStdRowHeight;
        }

        if ( nArrRow==0 || nDocHeight || nY > MAXROW )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
            pThisRowInfo->pCellInfo = NULL;                 // wird unten belegt

            sal_uInt16 nHeight = (sal_uInt16) ( nDocHeight * fRowScale );
            if (!nHeight)
                nHeight = 1;

            pThisRowInfo->nRowNo        = nY;               //! Fall < 0 ?
            pThisRowInfo->nHeight       = nHeight;
            pThisRowInfo->bEmptyBack    = true;
            pThisRowInfo->bEmptyText    = true;
            pThisRowInfo->bChanged      = true;
            pThisRowInfo->bAutoFilter   = false;
            pThisRowInfo->bPivotButton  = false;
            pThisRowInfo->nRotMaxCol    = SC_ROTMAX_NONE;

            ++nArrRow;
            if (nArrRow >= ROWINFO_MAX)
            {
                OSL_FAIL("FillInfo: Range too big" );
                nYExtra = nSignedY;                                 // Ende
                nRow2 = nYExtra - 1;                                  // Bereich anpassen
            }
        }
        else
            if (nSignedY==(SCsROW) nYExtra)                         // zusaetzliche Zeile verdeckt ?
                ++nYExtra;
    }
    nArrCount = nArrRow;                                      // incl. Dummys

    //  rotierter Text...

    //  Attribut im Dokument ueberhaupt verwendet?
    bool bAnyItem = false;
    sal_uInt32 nRotCount = pPool->GetItemCount2( ATTR_ROTATE_VALUE );
    for (sal_uInt32 nItem=0; nItem<nRotCount; nItem++)
        if (pPool->GetItem2( ATTR_ROTATE_VALUE, nItem ))
        {
            bAnyItem = true;
            break;
        }

    SCCOL nRotMax = nCol2;
    if ( bAnyItem && HasAttrib( 0,nRow1,nTab, MAXCOL,nRow2+1,nTab,
                                HASATTR_ROTATE | HASATTR_CONDITIONAL ) )
    {
        //! Conditionals auch bei HASATTR_ROTATE abfragen ????

        OSL_ENSURE( nArrCount>2, "nArrCount too small" );
        FindMaxRotCol( nTab, &pRowInfo[1], nArrCount-1, nCol1, nCol2 );
        //  FindMaxRotCol setzt nRotMaxCol

        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
            if (pRowInfo[nArrRow].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nArrRow].nRotMaxCol > nRotMax)
                nRotMax = pRowInfo[nArrRow].nRotMaxCol;
    }

    //  Zell-Infos erst nach dem Test auf gedrehte allozieren
    //  bis nRotMax wegen nRotateDir Flag

    for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
        nY = pThisRowInfo->nRowNo;
        pThisRowInfo->pCellInfo = new CellInfo[ nRotMax+1+2 ];  // vom Aufrufer zu loeschen !

        for (nArrCol=0; nArrCol<=nRotMax+2; nArrCol++)                // Zell-Infos vorbelegen
        {
            if (nArrCol>0)
                nX = nArrCol-1;
            else
                nX = MAXCOL+1;      // ungueltig

            CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrCol];
            pInfo->bEmptyCellText = true;
            pInfo->maCell.clear();
            if (bPaintMarks)
                pInfo->bMarked = ( nX >= nBlockStartX && nX <= nBlockEndX
                                && nY >= nBlockStartY && nY <= nBlockEndY );
            else
                pInfo->bMarked = false;
            pInfo->nWidth = 0;

            pInfo->nClipMark    = SC_CLIPMARK_NONE;
            pInfo->bMerged      = false;
            pInfo->bHOverlapped = false;
            pInfo->bVOverlapped = false;
            pInfo->bAutoFilter  = false;
            pInfo->bPivotButton  = false;
            pInfo->bPivotPopupButton = false;
            pInfo->bFilterActive = false;
            pInfo->nRotateDir   = SC_ROTDIR_NONE;

            pInfo->bPrinted     = false;                    //  view-intern
            pInfo->bHideGrid    = false;                    //  view-intern
            pInfo->bEditEngine  = false;                    //  view-intern

            pInfo->pBackground  = NULL;                     //! weglassen?
            pInfo->pPatternAttr = NULL;
            pInfo->pConditionSet= NULL;

            pInfo->pLinesAttr   = NULL;
            pInfo->mpTLBRLine   = NULL;
            pInfo->mpBLTRLine   = NULL;

            pInfo->pShadowAttr    = pDefShadow;
            pInfo->pHShadowOrigin = NULL;
            pInfo->pVShadowOrigin = NULL;
        }
    }

    for (nArrCol=nCol2+3; nArrCol<=nRotMax+2; nArrCol++)            // restliche Breiten eintragen
    {
        nX = nArrCol-1;
        if ( ValidCol(nX) )
        {
            if (!ColHidden(nX, nTab))
            {
                sal_uInt16 nThisWidth = (sal_uInt16) (GetColWidth( nX, nTab ) * fColScale);
                if (!nThisWidth)
                    nThisWidth = 1;

                pRowInfo[0].pCellInfo[nArrCol].nWidth = nThisWidth;
            }
        }
    }

    ScConditionalFormatList* pCondFormList = GetCondFormList(nTab);
    if(pCondFormList)
        pCondFormList->startRendering();

    for (nArrCol=0; nArrCol<=nCol2+2; nArrCol++)                    // links & rechts + 1
    {
        nX = (nArrCol>0) ? nArrCol-1 : MAXCOL+1;                    // negativ -> ungueltig

        if ( ValidCol(nX) )
        {
            // #i58049#, #i57939# Hidden columns must be skipped here, or their attributes
            // will disturb the output

            // TODO: Optimize this loop.
            if (!ColHidden(nX, nTab))
            {
                sal_uInt16 nThisWidth = (sal_uInt16) (GetColWidth( nX, nTab ) * fColScale);
                if (!nThisWidth)
                    nThisWidth = 1;

                pRowInfo[0].pCellInfo[nArrCol].nWidth = nThisWidth;           //! dies sollte reichen

                ScColumn* pThisCol = &maTabs[nTab]->aCol[nX];                   // Spalten-Daten

                nArrRow = 1;
                // Iterate between rows nY1 and nY2 and pick up non-empty
                // cells that are not hidden.
                RowInfoFiller aFunc(*this, nTab, pRowInfo, nArrCol, nArrRow);
                sc::ParseAllNonEmpty(
                    pThisCol->maCells.begin(), pThisCol->maCells, nRow1, nRow2, aFunc);

                if (nX+1 >= nCol1)                                // Attribute/Blockmarken ab nX1-1
                {
                    ScAttrArray* pThisAttrArr = pThisCol->pAttrArray;       // Attribute

                    nArrRow = 0;
                    const ScPatternAttr* pPattern;
                    SCROW nCurRow=nRow1;                  // einzelne Zeile
                    if (nCurRow>0)
                        --nCurRow;                      // oben 1 mehr
                    else
                        nArrRow = 1;
                    nThisRow=nCurRow;                   // Ende des Bereichs
                    SCSIZE  nIndex;
                    (void) pThisAttrArr->Search( nCurRow, nIndex );


                    do
                    {
                        nThisRow=pThisAttrArr->pData[nIndex].nRow;              // Ende des Bereichs
                        pPattern=pThisAttrArr->pData[nIndex].pPattern;

                        const SvxBrushItem* pBackground = (const SvxBrushItem*)
                                                        &pPattern->GetItem(ATTR_BACKGROUND);
                        const SvxBoxItem* pLinesAttr = (const SvxBoxItem*)
                                                        &pPattern->GetItem(ATTR_BORDER);

                        const SvxLineItem* pTLBRLine = static_cast< const SvxLineItem* >(
                            &pPattern->GetItem( ATTR_BORDER_TLBR ) );
                        const SvxLineItem* pBLTRLine = static_cast< const SvxLineItem* >(
                            &pPattern->GetItem( ATTR_BORDER_BLTR ) );

                        const SvxShadowItem* pShadowAttr = (const SvxShadowItem*)
                                                        &pPattern->GetItem(ATTR_SHADOW);
                        if (pShadowAttr != pDefShadow)
                            bAnyShadow = true;

                        const ScMergeAttr* pMergeAttr = (const ScMergeAttr*)
                                                &pPattern->GetItem(ATTR_MERGE);
                        bool bMerged = ( pMergeAttr != pDefMerge && *pMergeAttr != *pDefMerge );
                        sal_uInt16 nOverlap = ((const ScMergeFlagAttr*) &pPattern->GetItemSet().
                                                        Get(ATTR_MERGE_FLAG))->GetValue();
                        bool bHOverlapped = ((nOverlap & SC_MF_HOR) != 0);
                        bool bVOverlapped = ((nOverlap & SC_MF_VER) != 0);
                        bool bAutoFilter  = ((nOverlap & SC_MF_AUTO) != 0);
                        bool bPivotButton  = ((nOverlap & SC_MF_BUTTON) != 0);
                        bool bScenario    = ((nOverlap & SC_MF_SCENARIO) != 0);
                        bool bPivotPopupButton = ((nOverlap & SC_MF_BUTTON_POPUP) != 0);
                        bool bFilterActive = ((nOverlap & SC_MF_HIDDEN_MEMBER) != 0);
                        if (bMerged||bHOverlapped||bVOverlapped)
                            bAnyMerged = true;                              // intern

                        bool bHidden, bHideFormula;
                        if (bTabProtect)
                        {
                            const ScProtectionAttr& rProtAttr = (const ScProtectionAttr&)
                                                        pPattern->GetItem(ATTR_PROTECTION);
                            bHidden = rProtAttr.GetHideCell();
                            bHideFormula = rProtAttr.GetHideFormula();
                        }
                        else
                            bHidden = bHideFormula = false;

                        const std::vector<sal_uInt32>& rCondFormats = static_cast<const ScCondFormatItem&>(pPattern->GetItem(ATTR_CONDITIONAL)).GetCondFormatData();
                        bool bContainsCondFormat = !rCondFormats.empty();

                        do
                        {
                            SCROW nLastHiddenRow = -1;
                            bool bRowHidden = RowHidden(nCurRow, nTab, NULL, &nLastHiddenRow);
                            if ( nArrRow==0 || !bRowHidden )
                            {
                                if ( GetPreviewCellStyle( nX, nCurRow, nTab  ) != NULL )
                                    bAnyPreview = true;
                                RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
                                if (pBackground != pDefBackground)          // Spalten-HG == Standard ?
                                    pThisRowInfo->bEmptyBack = false;
                                if (bContainsCondFormat)
                                    pThisRowInfo->bEmptyBack = false;
                                if (bAutoFilter)
                                    pThisRowInfo->bAutoFilter = true;
                                if (bPivotButton || bPivotPopupButton)
                                    pThisRowInfo->bPivotButton = true;

                                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrCol];
                                pInfo->pBackground  = pBackground;
                                pInfo->pPatternAttr = pPattern;
                                pInfo->bMerged      = bMerged;
                                pInfo->bHOverlapped = bHOverlapped;
                                pInfo->bVOverlapped = bVOverlapped;
                                pInfo->bAutoFilter  = bAutoFilter;
                                pInfo->bPivotButton  = bPivotButton;
                                pInfo->bPivotPopupButton = bPivotPopupButton;
                                pInfo->bFilterActive = bFilterActive;
                                pInfo->pLinesAttr   = pLinesAttr;
                                pInfo->mpTLBRLine   = pTLBRLine;
                                pInfo->mpBLTRLine   = pBLTRLine;
                                pInfo->pShadowAttr  = pShadowAttr;
                                //  nWidth wird nicht mehr einzeln gesetzt

                                if (bScenario)
                                {
                                    pInfo->pBackground = ScGlobal::GetButtonBrushItem();
                                    pThisRowInfo->bEmptyBack = false;
                                }

                                if ( bContainsCondFormat )
                                {
                                    bool bFound = false;
                                    for(std::vector<sal_uInt32>::const_iterator itr = rCondFormats.begin();
                                            itr != rCondFormats.end() && !bFound; ++itr)
                                    {
                                        ScConditionalFormat* pCondForm = pCondFormList->GetFormat(*itr);
                                        if(!pCondForm)
                                            continue;

                                        ScCondFormatData aData = pCondForm->GetData(
                                            pInfo->maCell, ScAddress(nX, nCurRow, nTab));
                                        if (!aData.aStyleName.isEmpty())
                                        {
                                            SfxStyleSheetBase* pStyleSheet =
                                                pStlPool->Find( aData.aStyleName, SFX_STYLE_FAMILY_PARA );
                                            if ( pStyleSheet )
                                            {
                                                //! Style-Sets cachen !!!
                                                pInfo->pConditionSet = &pStyleSheet->GetItemSet();
                                                bAnyCondition = true;

                                                // we need to check already here for protected cells
                                                const SfxPoolItem* pItem;
                                                if ( bTabProtect && pInfo->pConditionSet->GetItemState( ATTR_PROTECTION, true, &pItem ) == SFX_ITEM_SET )
                                                {
                                                    const ScProtectionAttr* pProtAttr = static_cast<const ScProtectionAttr*>(pItem);
                                                    bHidden = pProtAttr->GetHideCell();
                                                    bHideFormula = pProtAttr->GetHideFormula();

                                                }
                                                bFound = true;

                                            }
                                            // if style is not there, treat like no condition
                                        }
                                        if(aData.pColorScale)
                                        {
                                            pInfo->pColorScale = aData.pColorScale;
                                            bFound = true;
                                        }

                                        if(aData.pDataBar)
                                        {
                                            pInfo->pDataBar = aData.pDataBar;
                                            bFound = true;
                                        }
                                        if(aData.pIconSet)
                                        {
                                            pInfo->pIconSet = aData.pIconSet;
                                            bFound = true;
                                        }
                                    }
                                }

                                if (bHidden || (bFormulaMode && bHideFormula && pInfo->maCell.meType == CELLTYPE_FORMULA))
                                    pInfo->bEmptyCellText = true;

                                ++nArrRow;
                            }
                            else if (bRowHidden && nLastHiddenRow >= 0)
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
                    while ( nIndex < pThisAttrArr->nCount && nThisRow < nYExtra );


                    if (pMarkData && pMarkData->IsMultiMarked())
                    {
                        //  Blockmarken
                        const ScMarkArray* pThisMarkArr = pMarkData->GetArray()+nX;
                        nArrRow = 1;
                        nCurRow = nRow1;                                      // einzelne Zeile
                        nThisRow = nRow1;                                     // Ende des Bereichs

                        if ( pThisMarkArr->Search( nRow1, nIndex ) )
                        {
                            bool bThisMarked;
                            do
                            {
                                nThisRow=pThisMarkArr->pData[nIndex].nRow;      // Ende des Bereichs
                                bThisMarked=pThisMarkArr->pData[nIndex].bMarked;

                                do
                                {
                                    if ( !RowHidden( nCurRow,nTab ) )
                                    {
                                        if ( bThisMarked )
                                        {
                                            bool bSkip = bSkipMarks &&
                                                        nX      >= nBlockStartX &&
                                                        nX      <= nBlockEndX   &&
                                                        nCurRow >= nBlockStartY &&
                                                        nCurRow <= nBlockEndY;
                                            if (!bSkip)
                                            {
                                                RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
                                                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrCol];
                                                pInfo->bMarked = true;
                                            }
                                        }
                                        ++nArrRow;
                                    }
                                    ++nCurRow;
                                }
                                while (nCurRow <= nThisRow && nCurRow <= nRow2);
                                ++nIndex;
                            }
                            while ( nIndex < pThisMarkArr->nCount && nThisRow < nRow2 );
                        }
                    }
                }
                else                                    // vordere Spalten
                {
                    for (nArrRow=1; nArrRow+1<nArrCount; nArrRow++)
                    {
                        RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
                        CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrCol];

                        pInfo->nWidth       = nThisWidth;           //! oder nur 0 abfragen ??
                    }
                }
            }
        }
        else
            pRowInfo[0].pCellInfo[nArrCol].nWidth = STD_COL_WIDTH;
        // STD_COL_WIDTH ganz links und rechts wird fuer DrawExtraShadow gebraucht
    }

    if(pCondFormList)
        pCondFormList->endRendering();
    //-------------------------------------------------------------------------
    //  bedingte Formatierung auswerten
    ::boost::ptr_vector<ScPatternAttr> aAltPatterns;
    // favour preview over condition
    if (bAnyCondition || bAnyPreview)
    {
        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
        {
            for (nArrCol=nCol1; nArrCol<=nCol2+2; nArrCol++)                  // links und rechts einer mehr
            {
                CellInfo* pInfo = &pRowInfo[nArrRow].pCellInfo[nArrCol];
                SCCOL nCol = (nArrCol>0) ? nArrCol-1 : MAXCOL+1;
                ScPatternAttr* pModifiedPatt = NULL;

                if ( ValidCol(nCol) && pRowInfo[nArrRow].nRowNo <= MAXROW )
                {
                    if ( ScStyleSheet* pPreviewStyle = GetPreviewCellStyle( nCol, pRowInfo[nArrRow].nRowNo, nTab ) )
                    {
                        aAltPatterns.push_back( new ScPatternAttr( *pInfo->pPatternAttr ) );
                        pModifiedPatt = &aAltPatterns.back();
                        pModifiedPatt->SetStyleSheet( pPreviewStyle );
                    }
                }
                // favour preview over condition
                const SfxItemSet* pCondSet = pModifiedPatt ? &pModifiedPatt->GetItemSet() : pInfo->pConditionSet;

                if (pCondSet)
                {
                    const SfxPoolItem* pItem;

                            //  Hintergrund
                    if ( pCondSet->GetItemState( ATTR_BACKGROUND, true, &pItem ) == SFX_ITEM_SET )
                    {
                        pInfo->pBackground = (const SvxBrushItem*) pItem;
                        pRowInfo[nArrRow].bEmptyBack = false;
                    }

                            //  Umrandung
                    if ( pCondSet->GetItemState( ATTR_BORDER, true, &pItem ) == SFX_ITEM_SET )
                        pInfo->pLinesAttr = (const SvxBoxItem*) pItem;

                    if ( pCondSet->GetItemState( ATTR_BORDER_TLBR, true, &pItem ) == SFX_ITEM_SET )
                        pInfo->mpTLBRLine = static_cast< const SvxLineItem* >( pItem );
                    if ( pCondSet->GetItemState( ATTR_BORDER_BLTR, true, &pItem ) == SFX_ITEM_SET )
                        pInfo->mpBLTRLine = static_cast< const SvxLineItem* >( pItem );

                            //  Schatten
                    if ( pCondSet->GetItemState( ATTR_SHADOW, true, &pItem ) == SFX_ITEM_SET )
                    {
                        pInfo->pShadowAttr = (const SvxShadowItem*) pItem;
                        bAnyShadow = true;
                    }
                }
                if( bAnyCondition && pInfo->pColorScale)
                {
                    pRowInfo[nArrRow].bEmptyBack = false;
                    pInfo->pBackground = new SvxBrushItem(*pInfo->pColorScale, ATTR_BACKGROUND);
                }
            }
        }
    }

    //  bedingte Formatierung Ende
    //-------------------------------------------------------------------------

                //
                //      Daten von zusammengefassten Zellen anpassen
                //

    if (bAnyMerged)
    {
        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrRow];
            nSignedY = nArrRow ? pThisRowInfo->nRowNo : ((SCsROW)nRow1)-1;

            for (nArrCol=nCol1; nArrCol<=nCol2+2; nArrCol++)                  // links und rechts einer mehr
            {
                SCsCOL nSignedX = ((SCsCOL) nArrCol) - 1;
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrCol];

                if (pInfo->bMerged || pInfo->bHOverlapped || pInfo->bVOverlapped)
                {
                    SCsCOL nStartX;
                    SCsROW nStartY;
                    SCsCOL nEndX;
                    SCsROW nEndY;
                    lcl_GetMergeRange( nSignedX,nSignedY, nArrRow, this,pRowInfo, nCol1,nRow1,nCol2,nRow2,nTab,
                                        nStartX,nStartY, nEndX,nEndY );
                    const ScPatternAttr* pStartPattern = GetPattern( nStartX,nStartY,nTab );
                    const SfxItemSet* pStartCond = GetCondResult( nStartX,nStartY,nTab );
                    const SfxPoolItem* pItem;

                    // Hintergrund kopieren (oder in output.cxx)

                    if ( !pStartCond || pStartCond->
                                    GetItemState(ATTR_BACKGROUND,true,&pItem) != SFX_ITEM_SET )
                        pItem = &pStartPattern->GetItem(ATTR_BACKGROUND);
                    pInfo->pBackground = (const SvxBrushItem*) pItem;
                    pRowInfo[nArrRow].bEmptyBack = false;

                    // Schatten

                    if ( !pStartCond || pStartCond->
                                    GetItemState(ATTR_SHADOW,true,&pItem) != SFX_ITEM_SET )
                        pItem = &pStartPattern->GetItem(ATTR_SHADOW);
                    pInfo->pShadowAttr = (const SvxShadowItem*) pItem;
                    if (pInfo->pShadowAttr != pDefShadow)
                        bAnyShadow = true;

                    // Blockmarken - wieder mit Original-Merge-Werten

                    bool bCellMarked = false;
                    if (bPaintMarks)
                        bCellMarked = ( nStartX >= (SCsCOL) nBlockStartX
                                    && nStartX <= (SCsCOL) nBlockEndX
                                    && nStartY >= (SCsROW) nBlockStartY
                                    && nStartY <= (SCsROW) nBlockEndY );
                    if (pMarkData && pMarkData->IsMultiMarked() && !bCellMarked)
                    {
                        const ScMarkArray* pThisMarkArr = pMarkData->GetArray()+nStartX;
                        SCSIZE nIndex;
                        if ( pThisMarkArr->Search( nStartY, nIndex ) )
                            bCellMarked=pThisMarkArr->pData[nIndex].bMarked;
                    }

                    pInfo->bMarked = bCellMarked;
                }
            }
        }
    }

    if (bAnyShadow)                             // Schatten verteilen
    {
        for (nArrRow=0; nArrRow<nArrCount; nArrRow++)
        {
            bool bTop = ( nArrRow == 0 );
            bool bBottom = ( nArrRow+1 == nArrCount );

            for (nArrCol=nCol1; nArrCol<=nCol2+2; nArrCol++)                  // links und rechts einer mehr
            {
                bool bLeft = ( nArrCol == nCol1 );
                bool bRight = ( nArrCol == nCol2+2 );

                CellInfo* pInfo = &pRowInfo[nArrRow].pCellInfo[nArrCol];
                const SvxShadowItem* pThisAttr = pInfo->pShadowAttr;
                SvxShadowLocation eLoc = pThisAttr ? pThisAttr->GetLocation() : SVX_SHADOW_NONE;
                if (eLoc != SVX_SHADOW_NONE)
                {
                    //  oder Test auf != eLoc

                    SCsCOL nDxPos = 1;
                    SCsCOL nDxNeg = -1;

                    while ( nArrCol+nDxPos < nCol2+2 && pRowInfo[0].pCellInfo[nArrCol+nDxPos].nWidth == 0 )
                        ++nDxPos;
                    while ( nArrCol+nDxNeg > nCol1 && pRowInfo[0].pCellInfo[nArrCol+nDxNeg].nWidth == 0 )
                        --nDxNeg;

                    bool bLeftDiff = !bLeft &&
                            pRowInfo[nArrRow].pCellInfo[nArrCol+nDxNeg].pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
                    bool bRightDiff = !bRight &&
                            pRowInfo[nArrRow].pCellInfo[nArrCol+nDxPos].pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
                    bool bTopDiff = !bTop &&
                            pRowInfo[nArrRow-1].pCellInfo[nArrCol].pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
                    bool bBottomDiff = !bBottom &&
                            pRowInfo[nArrRow+1].pCellInfo[nArrCol].pShadowAttr->GetLocation() == SVX_SHADOW_NONE;

                    if ( bLayoutRTL )
                    {
                        switch (eLoc)
                        {
                            case SVX_SHADOW_BOTTOMRIGHT: eLoc = SVX_SHADOW_BOTTOMLEFT;  break;
                            case SVX_SHADOW_BOTTOMLEFT:  eLoc = SVX_SHADOW_BOTTOMRIGHT; break;
                            case SVX_SHADOW_TOPRIGHT:    eLoc = SVX_SHADOW_TOPLEFT;     break;
                            case SVX_SHADOW_TOPLEFT:     eLoc = SVX_SHADOW_TOPRIGHT;    break;
                            default:
                            {
                                // added to avoid warnings
                            }
                        }
                    }

                    switch (eLoc)
                    {
                        case SVX_SHADOW_BOTTOMRIGHT:
                            if (bBottomDiff)
                            {
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol].eHShadowPart =
                                                bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bRightDiff)
                            {
                                pRowInfo[nArrRow].pCellInfo[nArrCol+1].pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].pCellInfo[nArrCol+1].eVShadowPart =
                                                bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bBottomDiff && bRightDiff)
                            {
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol+1].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol+1].eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SVX_SHADOW_BOTTOMLEFT:
                            if (bBottomDiff)
                            {
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol].eHShadowPart =
                                                bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bLeftDiff)
                            {
                                pRowInfo[nArrRow].pCellInfo[nArrCol-1].pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].pCellInfo[nArrCol-1].eVShadowPart =
                                                bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bBottomDiff && bLeftDiff)
                            {
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol-1].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow+1].pCellInfo[nArrCol-1].eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SVX_SHADOW_TOPRIGHT:
                            if (bTopDiff)
                            {
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol].eHShadowPart =
                                                bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bRightDiff)
                            {
                                pRowInfo[nArrRow].pCellInfo[nArrCol+1].pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].pCellInfo[nArrCol+1].eVShadowPart =
                                                bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bTopDiff && bRightDiff)
                            {
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol+1].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol+1].eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SVX_SHADOW_TOPLEFT:
                            if (bTopDiff)
                            {
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol].eHShadowPart =
                                                bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bLeftDiff)
                            {
                                pRowInfo[nArrRow].pCellInfo[nArrCol-1].pVShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow].pCellInfo[nArrCol-1].eVShadowPart =
                                                bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bTopDiff && bLeftDiff)
                            {
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol-1].pHShadowOrigin = pThisAttr;
                                pRowInfo[nArrRow-1].pCellInfo[nArrCol-1].eHShadowPart = SC_SHADOW_CORNER;
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

    // ========================================================================
    // *** create the frame border array ***

    // RowInfo structs are filled in the range [ 0 , nArrCount-1 ]
    // each RowInfo contains CellInfo structs in the range [ nX1-1 , nX2+1 ]

    size_t nColCount = nCol2 - nCol1 + 3;
    size_t nRowCount = nArrCount;

    svx::frame::Array& rArray = rTabInfo.maArray;
    rArray.Initialize( nColCount, nRowCount );
    rArray.SetUseDiagDoubleClipping( false );

    for( size_t nRow = 0; nRow < nRowCount; ++nRow )
    {
        sal_uInt16 nCellInfoY = static_cast< sal_uInt16 >( nRow );
        RowInfo& rThisRowInfo = pRowInfo[ nCellInfoY ];

        for( size_t nCol = 0; nCol < nColCount; ++nCol )
        {
            sal_uInt16 nCellInfoX = static_cast< sal_uInt16 >( nCol + nCol1 );
            const CellInfo& rInfo = rThisRowInfo.pCellInfo[ nCellInfoX ];

            const SvxBoxItem* pBox = rInfo.pLinesAttr;
            const SvxLineItem* pTLBR = rInfo.mpTLBRLine;
            const SvxLineItem* pBLTR = rInfo.mpBLTRLine;

            size_t nFirstCol = nCol;
            size_t nFirstRow = nRow;

            // *** merged cells *** -------------------------------------------

            if( !rArray.IsMerged( nCol, nRow ) && (rInfo.bMerged || rInfo.bHOverlapped || rInfo.bVOverlapped) )
            {
                // *** insert merged range in svx::frame::Array ***

                /*  #i69369# top-left cell of a merged range may be located in
                    a hidden column or row. Use lcl_GetMergeRange() to find the
                    complete merged range, then calculate dimensions and
                    document position of the visible range. */

                // note: document columns are always one less than CellInfoX coords
                // note: document rows must be looked up in RowInfo structs

                // current column and row in document coordinates
                SCCOL nCurrDocCol = static_cast< SCCOL >( nCellInfoX - 1 );
                SCROW nCurrDocRow = static_cast< SCROW >( (nCellInfoY > 0) ? rThisRowInfo.nRowNo : (nRow1 - 1) );

                // find entire merged range in document, returns signed document coordinates
                SCsCOL nFirstRealDocColS, nLastRealDocColS;
                SCsROW nFirstRealDocRowS, nLastRealDocRowS;
                lcl_GetMergeRange( static_cast< SCsCOL >( nCurrDocCol ), static_cast< SCsROW >( nCurrDocRow ),
                    nCellInfoY, this, pRowInfo, nCol1,nRow1,nCol2,nRow2,nTab,
                    nFirstRealDocColS, nFirstRealDocRowS, nLastRealDocColS, nLastRealDocRowS );

                // *complete* merged range in document coordinates
                SCCOL nFirstRealDocCol = static_cast< SCCOL >( nFirstRealDocColS );
                SCROW nFirstRealDocRow = static_cast< SCROW >( nFirstRealDocRowS );
                SCCOL nLastRealDocCol  = static_cast< SCCOL >( nLastRealDocColS );
                SCROW nLastRealDocRow  = static_cast< SCROW >( nLastRealDocRowS );

                // first visible column (nX1-1 is first processed document column)
                SCCOL nFirstDocCol = (nCol1 > 0) ? ::std::max< SCCOL >( nFirstRealDocCol, nCol1 - 1 ) : nFirstRealDocCol;
                sal_uInt16 nFirstCellInfoX = static_cast< sal_uInt16 >( nFirstDocCol + 1 );
                nFirstCol = static_cast< size_t >( nFirstCellInfoX - nCol1 );

                // last visible column (nX2+1 is last processed document column)
                SCCOL nLastDocCol = (nCol2 < MAXCOL) ? ::std::min< SCCOL >( nLastRealDocCol, nCol2 + 1 ) : nLastRealDocCol;
                sal_uInt16 nLastCellInfoX = static_cast< sal_uInt16 >( nLastDocCol + 1 );
                size_t nLastCol = static_cast< size_t >( nLastCellInfoX - nCol1 );

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
                    long nSize = 0;
                    for( SCCOL nDocCol = nFirstRealDocCol; nDocCol < nFirstDocCol; ++nDocCol )
                        nSize += std::max( static_cast< long >( GetColWidth( nDocCol, nTab ) * fColScale ), 1L );
                    rArray.SetAddMergedLeftSize( nCol, nRow, nSize );
                }
                // additional space after last column
                if( nLastCol + 1 == nColCount )
                {
                    long nSize = 0;
                    for( SCCOL nDocCol = nLastDocCol + 1; nDocCol <= nLastRealDocCol; ++nDocCol )
                        nSize += std::max( static_cast< long >( GetColWidth( nDocCol, nTab ) * fColScale ), 1L );
                    rArray.SetAddMergedRightSize( nCol, nRow, nSize );
                }
                // additional space above first row
                if( nFirstRow == 0 )
                {
                    long nSize = 0;
                    for( SCROW nDocRow = nFirstRealDocRow; nDocRow < nFirstDocRow; ++nDocRow )
                        nSize += std::max( static_cast< long >( GetRowHeight( nDocRow, nTab ) * fRowScale ), 1L );
                    rArray.SetAddMergedTopSize( nCol, nRow, nSize );
                }
                // additional space beyond last row
                if( nLastRow + 1 == nRowCount )
                {
                    long nSize = 0;
                    for( SCROW nDocRow = nLastDocRow + 1; nDocRow <= nLastRealDocRow; ++nDocRow )
                        nSize += std::max( static_cast< long >( GetRowHeight( nDocRow, nTab ) * fRowScale ), 1L );
                    rArray.SetAddMergedBottomSize( nCol, nRow, nSize );
                }

                // *** use line attributes from real origin cell ***

                if( (nFirstRealDocCol != nCurrDocCol) || (nFirstRealDocRow != nCurrDocRow) )
                {
                    if( const ScPatternAttr* pPattern = GetPattern( nFirstRealDocCol, nFirstRealDocRow, nTab ) )
                    {
                        const SfxItemSet* pCond = GetCondResult( nFirstRealDocCol, nFirstRealDocRow, nTab );
                        pBox = static_cast< const SvxBoxItem* >( &pPattern->GetItem( ATTR_BORDER, pCond ) );
                        pTLBR = static_cast< const SvxLineItem* >( &pPattern->GetItem( ATTR_BORDER_TLBR, pCond ) );
                        pBLTR = static_cast< const SvxLineItem* >( &pPattern->GetItem( ATTR_BORDER_BLTR, pCond ) );
                    }
                    else
                    {
                        pBox = 0;
                        pTLBR = pBLTR = 0;
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
            if( rInfo.mpBLTRLine )
                rArray.SetCellStyleBLTR( nFirstCol, nFirstRow, svx::frame::Style( pBLTR->GetLine(), fRowScale ) );
        }
    }

    /*  Mirror the entire frame array.
        1st param = Mirror the vertical double line styles as well.
        2nd param = Do not swap diagonal lines.
     */
    if( bLayoutRTL )
        rArray.MirrorSelfX( true, false );
}

// ============================================================================

ScTableInfo::ScTableInfo() :
    mpRowInfo( new RowInfo[ ROWINFO_MAX ] ),
    mbPageMode( false )
{
    for( sal_uInt16 nIdx = 0; nIdx < ROWINFO_MAX; ++nIdx )
        mpRowInfo[ nIdx ].pCellInfo = 0;
}

ScTableInfo::~ScTableInfo()
{
    for( sal_uInt16 nIdx = 0; nIdx < ROWINFO_MAX; ++nIdx )
        delete [] mpRowInfo[ nIdx ].pCellInfo;
    delete [] mpRowInfo;
}

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
