/*************************************************************************
 *
 *  $RCSfile: fillinfo.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <svx/boxitem.hxx>
#include <svx/shaditem.hxx>

#include "document.hxx"
#include "cell.hxx"
#include "table.hxx"
#include "attrib.hxx"
#include "attarray.hxx"
#include "markarr.hxx"
#include "markdata.hxx"
#include "patattr.hxx"
#include "docpool.hxx"
#include "conditio.hxx"
#include "stlpool.hxx"


// -----------------------------------------------------------------------

enum FillInfoLinePos
    {
        FILP_TOP,
        FILP_BOTTOM,
        FILP_LEFT,
        FILP_RIGHT
    };


inline const SvxBorderLine* GetNullOrLine( const SvxBoxItem* pBox, FillInfoLinePos eWhich )
{
    if (pBox)
    {
        if (eWhich==FILP_TOP)
            return pBox->GetTop();
        else if (eWhich==FILP_BOTTOM)
            return pBox->GetBottom();
        else if (eWhich==FILP_LEFT)
            return pBox->GetLeft();
        else
            return pBox->GetRight();
    }
    else
        return NULL;
}

//  aehnlich wie in output.cxx

void lcl_GetMergeRange( short nX, short nY, USHORT nArrY,
                            ScDocument* pDoc, RowInfo* pRowInfo,
                            USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2, USHORT nTab,
                            short& rStartX, short& rStartY, short& rEndX, short& rEndY )
{
    CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nX+1];

    rStartX = nX;
    rStartY = nY;
    BOOL bHOver = pInfo->bHOverlapped;
    BOOL bVOver = pInfo->bVOverlapped;

    while (bHOver)              // nY konstant
    {
        --rStartX;
        if (rStartX >= (short) nX1 && (pDoc->GetColFlags(rStartX,nTab) & CR_HIDDEN) == 0)
        {
            bHOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bVOverlapped;
        }
        else
        {
            USHORT nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
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

        if (rStartX >= (short) nX1 && rStartY >= (short) nY1 &&
            (pDoc->GetColFlags(rStartX,nTab) & CR_HIDDEN) == 0 &&
            (pDoc->GetRowFlags(rStartY,nTab) & CR_HIDDEN) == 0 &&
            (short) pRowInfo[nArrY].nRowNo == rStartY)
        {
            bHOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bHOverlapped;
            bVOver = pRowInfo[nArrY].pCellInfo[rStartX+1].bVOverlapped;
        }
        else
        {
            USHORT nOverlap = ((ScMergeFlagAttr*)pDoc->GetAttr(
                                rStartX, rStartY, nTab, ATTR_MERGE_FLAG ))->GetValue();
            bHOver = ((nOverlap & SC_MF_HOR) != 0);
            bVOver = ((nOverlap & SC_MF_VER) != 0);
        }
    }

    const ScMergeAttr* pMerge;
    if (rStartX >= (short) nX1 && rStartY >= (short) nY1 &&
        (pDoc->GetColFlags(rStartX,nTab) & CR_HIDDEN) == 0 &&
        (pDoc->GetRowFlags(rStartY,nTab) & CR_HIDDEN) == 0 &&
        (short) pRowInfo[nArrY].nRowNo == rStartY)
    {
        pMerge = (const ScMergeAttr*) &pRowInfo[nArrY].pCellInfo[rStartX+1].pPatternAttr->
                                        GetItem(ATTR_MERGE);
    }
    else
        pMerge = (const ScMergeAttr*) pDoc->GetAttr(rStartX,rStartY,nTab,ATTR_MERGE);

    rEndX = rStartX + pMerge->GetColMerge() - 1;
    rEndY = rStartY + pMerge->GetRowMerge() - 1;
}

inline BOOL ScDocument::RowHidden( USHORT nRow, USHORT nTab )
{
    return ( pTab[nTab]->pRowFlags[nRow] & CR_HIDDEN ) != 0;
}


#define CELLINFO(x,y) pRowInfo[nArrY+y].pCellInfo[nArrX+x]

USHORT ScDocument::FillInfo( RowInfo* pRowInfo, USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                            USHORT nTab, double nScaleX, double nScaleY,
                            BOOL bPageMode, BOOL bFormulaMode, const ScMarkData* pMarkData )
{
    DBG_ASSERT( pTab[nTab], "Tabelle existiert nicht" );

    const SvxBrushItem* pDefBackground =
            (const SvxBrushItem*) &pDocPool->GetDefaultItem( ATTR_BACKGROUND );
    const ScMergeAttr* pDefMerge =
            (const ScMergeAttr*) &pDocPool->GetDefaultItem( ATTR_MERGE );
    const SvxBoxItem* pDefLines =
            (const SvxBoxItem*) &pDocPool->GetDefaultItem( ATTR_BORDER );
    const SvxShadowItem* pDefShadow =
            (const SvxShadowItem*) &pDocPool->GetDefaultItem( ATTR_SHADOW );

    USHORT nThisRow;
    USHORT nX;
    USHORT nY;
    short nSignedY;
    USHORT nArrX;
    USHORT nArrY;
    USHORT nArrCount;
    BOOL bAnyLines = FALSE;
    BOOL bAnyMerged = FALSE;
    BOOL bAnyShadow = FALSE;
    BOOL bAnyCondition = FALSE;

    BOOL bTabProtect = IsTabProtected(nTab);

                                                // fuer Blockmarken von zusammengefassten Zellen mit
                                                // versteckter erster Zeile / Spalte
    BOOL bPaintMarks = FALSE;
    BOOL bSkipMarks = FALSE;
    USHORT nBlockStartX, nBlockStartY, nBlockEndX, nBlockEndY;
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
                bSkipMarks = TRUE;
            else
                bPaintMarks = TRUE;
        }
    }

    //  zuerst nur die Eintraege fuer die ganze Spalte

    nArrY=0;
    USHORT nYExtra = nY2+1;
    for (nSignedY=((short)nY1)-1; nSignedY<=(short)nYExtra; nSignedY++)
    {
        if (nSignedY >= 0)
            nY = (USHORT) nSignedY;
        else
            nY = MAXROW+1;          // ungueltig

        USHORT nDocHeight;
        if (nY <= MAXROW)
            nDocHeight = GetRowHeight( nY, nTab );
        else
            nDocHeight = ScGlobal::nStdRowHeight;

        if ( nArrY==0 || nDocHeight || nY > MAXROW )
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            pThisRowInfo->pCellInfo = NULL;                 // wird unten belegt

            USHORT nHeight = (USHORT) ( nDocHeight * nScaleY );
            if (!nHeight)
                nHeight = 1;

            pThisRowInfo->nRowNo        = nY;               //! Fall < 0 ?
            pThisRowInfo->nHeight       = nHeight;
            pThisRowInfo->bEmptyBack    = TRUE;
            pThisRowInfo->bEmptyText    = TRUE;
            pThisRowInfo->bChanged      = TRUE;
            pThisRowInfo->bAutoFilter   = FALSE;
            pThisRowInfo->bPushButton   = FALSE;
            pThisRowInfo->nRotMaxCol    = SC_ROTMAX_NONE;

            ++nArrY;
            if (nArrY >= ROWINFO_MAX)
            {
                DBG_ERROR("Zu grosser Bereich bei FillInfo" );
                nYExtra = nSignedY;                                 // Ende
                nY2 = nYExtra - 1;                                  // Bereich anpassen
            }
        }
        else
            if (nSignedY==(short) nYExtra)                          // zusaetzliche Zeile verdeckt ?
                ++nYExtra;
    }
    nArrCount = nArrY;                                      // incl. Dummys

    //  rotierter Text...

    //  Attribut im Dokument ueberhaupt verwendet?
    BOOL bAnyItem = FALSE;
    USHORT nRotCount = pDocPool->GetItemCount( ATTR_ROTATE_VALUE );
    for (USHORT nItem=0; nItem<nRotCount; nItem++)
        if (pDocPool->GetItem( ATTR_ROTATE_VALUE, nItem ))
        {
            bAnyItem = TRUE;
            break;
        }

    USHORT nRotMax = nX2;
    if ( bAnyItem && HasAttrib( 0,nY1,nTab, MAXCOL,nY2+1,nTab,
                                HASATTR_ROTATE | HASATTR_CONDITIONAL ) )
    {
        //! Conditionals auch bei HASATTR_ROTATE abfragen ????

        DBG_ASSERT( nArrCount>2, "nArrCount zu klein" );
//      FindMaxRotCol( nTab, &pRowInfo[1], nArrCount-2, nX1, nX2 );
        FindMaxRotCol( nTab, &pRowInfo[1], nArrCount-1, nX1, nX2 );
        //  FindMaxRotCol setzt nRotMaxCol

        for (nArrY=0; nArrY<nArrCount; nArrY++)
            if (pRowInfo[nArrY].nRotMaxCol != SC_ROTMAX_NONE && pRowInfo[nArrY].nRotMaxCol > nRotMax)
                nRotMax = pRowInfo[nArrY].nRotMaxCol;
    }

    //  Zell-Infos erst nach dem Test auf gedrehte allozieren
    //  bis nRotMax wegen nRotateDir Flag

    for (nArrY=0; nArrY<nArrCount; nArrY++)
    {
        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
        nY = pThisRowInfo->nRowNo;
        pThisRowInfo->pCellInfo = new CellInfo[ nRotMax+1+2 ];  // vom Aufrufer zu loeschen !

        for (nArrX=0; nArrX<=nRotMax+2; nArrX++)                // Zell-Infos vorbelegen
        {
            if (nArrX>0)
                nX = nArrX-1;
            else
                nX = MAXCOL+1;      // ungueltig

            CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
            pInfo->bEmptyCellText = TRUE;
            pInfo->pCell = NULL;
            if (bPaintMarks)
                pInfo->bMarked = ( nX >= nBlockStartX && nX <= nBlockEndX
                                && nY >= nBlockStartY && nY <= nBlockEndY );
            else
                pInfo->bMarked = FALSE;
            pInfo->nWidth = 0;

            pInfo->bStandard    = FALSE;    //! umbenennen in bClipped
            pInfo->bMerged      = FALSE;
            pInfo->bHOverlapped = FALSE;
            pInfo->bVOverlapped = FALSE;
            pInfo->bAutoFilter  = FALSE;
            pInfo->bPushButton  = FALSE;
            pInfo->nRotateDir   = SC_ROTDIR_NONE;

            pInfo->bPrinted     = FALSE;                    //  view-intern
            pInfo->bHideGrid    = FALSE;                    //  view-intern
            pInfo->bEditEngine  = FALSE;                    //  view-intern

            pInfo->pBackground  = NULL;                     //! weglassen?
            pInfo->pPatternAttr = NULL;
            pInfo->pConditionSet= NULL;

            pInfo->pLinesAttr  = NULL;
            pInfo->pRightLine  = NULL;
            pInfo->pBottomLine = NULL;

            pInfo->pShadowAttr    = pDefShadow;
            pInfo->pHShadowOrigin = NULL;
            pInfo->pVShadowOrigin = NULL;
        }
    }

    for (nArrX=nX2+3; nArrX<=nRotMax+2; nArrX++)            // restliche Breiten eintragen
    {
        nX = nArrX-1;
        if ( nX <= MAXCOL )
        {
            if ( (GetColFlags(nX,nTab) & CR_HIDDEN) == 0 )          // Spalte nicht versteckt
            {
                USHORT nThisWidth = (USHORT) (GetColWidth( nX, nTab ) * nScaleX);
                if (!nThisWidth)
                    nThisWidth = 1;

                pRowInfo[0].pCellInfo[nArrX].nWidth = nThisWidth;
            }
        }
    }

    for (nArrX=0; nArrX<=nX2+2; nArrX++)                    // links & rechts + 1
    {
        nX = (nArrX>0) ? nArrX-1 : MAXCOL+1;                    // negativ -> ungueltig

        if ( nX <= MAXCOL )
        {
            if ( (GetColFlags(nX,nTab) & CR_HIDDEN) == 0 )          // Spalte nicht versteckt
            {
                USHORT nThisWidth = (USHORT) (GetColWidth( nX, nTab ) * nScaleX);
                if (!nThisWidth)
                    nThisWidth = 1;

                pRowInfo[0].pCellInfo[nArrX].nWidth = nThisWidth;           //! dies sollte reichen

                ScColumn* pThisCol = &pTab[nTab]->aCol[nX];                 // Spalten-Daten

                nArrY = 1;
                USHORT nUIndex;
                (void) pThisCol->Search( nY1, nUIndex );
                while ( nUIndex < pThisCol->nCount &&
                        (nThisRow=pThisCol->pItems[nUIndex].nRow) <= nY2 )
                {
                    if ( !RowHidden( nThisRow,nTab ) )
                    {
                        while ( pRowInfo[nArrY].nRowNo < nThisRow )
                            ++nArrY;
                        DBG_ASSERT( pRowInfo[nArrY].nRowNo == nThisRow, "Zeile nicht gefunden in FillInfo" );

                        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                        CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
                        pInfo->pCell = pThisCol->pItems[nUIndex].pCell;
                        if (pInfo->pCell->GetCellType() != CELLTYPE_NOTE)
                        {
                            pThisRowInfo->bEmptyText = FALSE;                   // Zeile nicht leer
                            pInfo->bEmptyCellText = FALSE;                      // Zelle nicht leer
                        }
                        ++nArrY;
                    }
                    ++nUIndex;
                }

                if (nX+1 >= nX1)                                // Attribute/Blockmarken ab nX1-1
                {
                    ScAttrArray* pThisAttrArr = pThisCol->pAttrArray;       // Attribute

                    nArrY = 0;
                    const ScPatternAttr* pPattern;
                    USHORT nCurRow=nY1;                 // einzelne Zeile
                    if (nCurRow>0)
                        --nCurRow;                      // oben 1 mehr
                    else
                        nArrY = 1;
                    nThisRow=nCurRow;                   // Ende des Bereichs
                    short  nIndex;
                    (void) pThisAttrArr->Search( nCurRow, nIndex );


                    do
                    {
                        nThisRow=pThisAttrArr->pData[nIndex].nRow;              // Ende des Bereichs
                        pPattern=pThisAttrArr->pData[nIndex].pPattern;

                        const SvxBrushItem* pBackground = (const SvxBrushItem*)
                                                        &pPattern->GetItem(ATTR_BACKGROUND);
                        const SvxBoxItem* pLinesAttr = (const SvxBoxItem*)
                                                        &pPattern->GetItem(ATTR_BORDER);
                        if (pLinesAttr != pDefLines)
                            bAnyLines = TRUE;

                        const SvxShadowItem* pShadowAttr = (const SvxShadowItem*)
                                                        &pPattern->GetItem(ATTR_SHADOW);
                        if (pShadowAttr != pDefShadow)
                            bAnyShadow = TRUE;

                        const ScMergeAttr* pMergeAttr = (const ScMergeAttr*)
                                                &pPattern->GetItem(ATTR_MERGE);
                        BOOL bMerged = ( pMergeAttr != pDefMerge && *pMergeAttr != *pDefMerge );
                        USHORT nOverlap = ((const ScMergeFlagAttr*) &pPattern->GetItemSet().
                                                        Get(ATTR_MERGE_FLAG))->GetValue();
                        BOOL bHOverlapped = ((nOverlap & SC_MF_HOR) != 0);
                        BOOL bVOverlapped = ((nOverlap & SC_MF_VER) != 0);
                        BOOL bAutoFilter  = ((nOverlap & SC_MF_AUTO) != 0);
                        BOOL bPushButton  = ((nOverlap & SC_MF_BUTTON) != 0);
                        BOOL bScenario    = ((nOverlap & SC_MF_SCENARIO) != 0);
                        if (bMerged||bHOverlapped||bVOverlapped)
                            bAnyMerged = TRUE;                              // intern

                        BOOL bHidden, bHideFormula;
                        if (bTabProtect)
                        {
                            const ScProtectionAttr& rProtAttr = (const ScProtectionAttr&)
                                                        pPattern->GetItem(ATTR_PROTECTION);
                            bHidden = rProtAttr.GetHideCell();
                            bHideFormula = rProtAttr.GetHideFormula();
                        }
                        else
                            bHidden = bHideFormula = FALSE;

                        ULONG nConditional = ((const SfxUInt32Item&)pPattern->
                                                GetItem(ATTR_CONDITIONAL)).GetValue();
                        const ScConditionalFormat* pCondForm = NULL;
                        if ( nConditional && pCondFormList )
                            pCondForm = pCondFormList->GetFormat( nConditional );

                        do
                        {
                            if ( nArrY==0 || !RowHidden( nCurRow,nTab ) )
                            {
                                RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                                if (pBackground != pDefBackground)          // Spalten-HG == Standard ?
                                    pThisRowInfo->bEmptyBack = FALSE;
                                if (bAutoFilter)
                                    pThisRowInfo->bAutoFilter = TRUE;
                                if (bPushButton)
                                    pThisRowInfo->bPushButton = TRUE;

                                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
                                pInfo->pBackground  = pBackground;
                                pInfo->pPatternAttr = pPattern;
                                pInfo->bMerged      = bMerged;
                                pInfo->bHOverlapped = bHOverlapped;
                                pInfo->bVOverlapped = bVOverlapped;
                                pInfo->bAutoFilter  = bAutoFilter;
                                pInfo->bPushButton  = bPushButton;
                                pInfo->pLinesAttr   = pLinesAttr;
                                pInfo->pShadowAttr  = pShadowAttr;
                                //  nWidth wird nicht mehr einzeln gesetzt

                                BOOL bEmbed = bIsEmbedded &&
                                        nTab    >= aEmbedRange.aStart.Tab() &&
                                        nTab    <= aEmbedRange.aEnd.Tab()   &&
                                        nX      >= aEmbedRange.aStart.Col() &&
                                        nX      <= aEmbedRange.aEnd.Col()   &&
                                        nCurRow >= aEmbedRange.aStart.Row() &&
                                        nCurRow <= aEmbedRange.aEnd.Row();

                                if (bPushButton || bScenario)
                                {
                                    pInfo->pBackground = ScGlobal::GetButtonBrushItem();
                                    pThisRowInfo->bEmptyBack = FALSE;
                                }
                                else if (bEmbed)
                                {
                                    pInfo->pBackground = ScGlobal::GetEmbeddedBrushItem();
                                    pThisRowInfo->bEmptyBack = FALSE;
                                }

                                if (bHidden || ( bFormulaMode && bHideFormula && pInfo->pCell
                                                    && pInfo->pCell->GetCellType()
                                                        == CELLTYPE_FORMULA ))
                                    pInfo->bEmptyCellText = TRUE;

                                if ( pCondForm )
                                {
                                    String aStyle = pCondForm->GetCellStyle( pInfo->pCell,
                                                        ScAddress( nX, nCurRow, nTab ) );
                                    if (aStyle.Len())
                                    {
                                        SfxStyleSheetBase* pStyleSheet =
                                                pStylePool->Find( aStyle, SFX_STYLE_FAMILY_PARA );
                                        if ( pStyleSheet )
                                        {
                                            //! Style-Sets cachen !!!
                                            pInfo->pConditionSet = &pStyleSheet->GetItemSet();
                                            bAnyCondition = TRUE;
                                        }
                                        else
                                            DBG_ERROR("Style nicht gefunden");
                                    }
                                }

                                ++nArrY;
                            }
                            ++nCurRow;
                        }
                        while (nCurRow <= nThisRow && nCurRow <= nYExtra);
                        ++nIndex;
                    }
                    while ( nIndex < (short) pThisAttrArr->nCount && nThisRow < nYExtra );


                    if (pMarkData && pMarkData->IsMultiMarked())
                    {
                        //  Blockmarken
                        const ScMarkArray* pThisMarkArr = pMarkData->GetArray()+nX;
                        BOOL bThisMarked;
                        nArrY = 1;
                        nCurRow = nY1;                                      // einzelne Zeile
                        nThisRow = nY1;                                     // Ende des Bereichs
                        (void) pThisMarkArr->Search( nY1, nIndex );

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
                                        BOOL bSkip = bSkipMarks &&
                                                    nX      >= nBlockStartX &&
                                                    nX      <= nBlockEndX   &&
                                                    nCurRow >= nBlockStartY &&
                                                    nCurRow <= nBlockEndY;
                                        if (!bSkip)
                                        {
                                            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                                            CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];
                                            pInfo->bMarked = TRUE;
                                        }
                                    }
                                    ++nArrY;
                                }
                                ++nCurRow;
                            }
                            while (nCurRow <= nThisRow && nCurRow <= nY2);
                            ++nIndex;
                        }
                        while ( nIndex < (short) pThisMarkArr->nCount && nThisRow < nY2 );
                    }
                }
                else                                    // vordere Spalten
                {
                    for (nArrY=1; nArrY+1<nArrCount; nArrY++)
                    {
                        RowInfo* pThisRowInfo = &pRowInfo[nArrY];
                        CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];

                        pInfo->nWidth       = nThisWidth;           //! oder nur 0 abfragen ??
                    }
                }
            }
        }
        else
            pRowInfo[0].pCellInfo[nArrX].nWidth = STD_COL_WIDTH;
        // STD_COL_WIDTH ganz links und rechts wird fuer DrawExtraShadow gebraucht
    }

    //-------------------------------------------------------------------------
    //  bedingte Formatierung auswerten

    if (bAnyCondition)
    {
        for (nArrY=0; nArrY<nArrCount; nArrY++)
        {
            for (nArrX=nX1; nArrX<=nX2+2; nArrX++)                  // links und rechts einer mehr
            {
                CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nArrX];
                const SfxItemSet* pCondSet = pInfo->pConditionSet;
                if (pCondSet)
                {
                    const SfxPoolItem* pItem;

                            //  Hintergrund
                    if ( pCondSet->GetItemState( ATTR_BACKGROUND, TRUE, &pItem ) == SFX_ITEM_SET )
                    {
                        pInfo->pBackground = (const SvxBrushItem*) pItem;
                        pRowInfo[nArrY].bEmptyBack = FALSE;
                    }

                            //  Umrandung
                    if ( pCondSet->GetItemState( ATTR_BORDER, TRUE, &pItem ) == SFX_ITEM_SET )
                    {
                        pInfo->pLinesAttr = (const SvxBoxItem*) pItem;
                        bAnyLines = TRUE;
                    }

                            //  Schatten
                    if ( pCondSet->GetItemState( ATTR_SHADOW, TRUE, &pItem ) == SFX_ITEM_SET )
                    {
                        pInfo->pShadowAttr = (const SvxShadowItem*) pItem;
                        bAnyShadow = TRUE;
                    }
                }
            }
        }
    }

    //  bedingte Formatierung Ende
    //-------------------------------------------------------------------------


    if (bAnyMerged)
        bAnyLines = TRUE;                           // Linien koennen aus Merge hereinkommen!

    if (bAnyLines)                                  // vier Seiten suchen
    {
        for (nArrY=0; nArrY<nArrCount; nArrY++)
        {
//          short nY = nArrY ? pRowInfo[nArrY].nRowNo : ((short)nY1)-1;

            for (nArrX=nX1; nArrX<=nX2+2; nArrX++)                  // links und rechts einer mehr
            {
                short nX = ((short) nArrX) - 1;
                CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nArrX];

                pInfo->pThisBottom = GetNullOrLine( pInfo->pLinesAttr, FILP_BOTTOM );
                if ( nArrY+1 < nArrCount )
                    pInfo->pNextTop =
                        GetNullOrLine( CELLINFO(0,1).pLinesAttr, FILP_TOP );
                else
                    pInfo->pNextTop = NULL;

                pInfo->pThisRight = GetNullOrLine( pInfo->pLinesAttr, FILP_RIGHT );
                if ( nX <= (short) nX2 )
                    pInfo->pNextLeft =
                        GetNullOrLine( CELLINFO(1,0).pLinesAttr, FILP_LEFT );
                else
                    pInfo->pNextLeft = NULL;
            }
        }
    }

                //
                //      Daten von zusammengefassten Zellen anpassen
                //

    if (bAnyMerged)
    {
        for (nArrY=0; nArrY<nArrCount; nArrY++)
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
            short nY = nArrY ? pThisRowInfo->nRowNo : ((short)nY1)-1;

            for (nArrX=nX1; nArrX<=nX2+2; nArrX++)                  // links und rechts einer mehr
            {
                short nX = ((short) nArrX) - 1;
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];

                if (pInfo->bMerged || pInfo->bHOverlapped || pInfo->bVOverlapped)
                {
                    short nStartX;
                    short nStartY;
                    short nEndX;
                    short nEndY;
                    lcl_GetMergeRange( nX,nY, nArrY, this,pRowInfo, nX1,nY1,nX2,nY2,nTab,
                                        nStartX,nStartY, nEndX,nEndY );
                    const ScPatternAttr* pStartPattern = GetPattern( nStartX,nStartY,nTab );
                    const SfxItemSet* pStartCond = GetCondResult( nStartX,nStartY,nTab );
                    const SfxPoolItem* pItem;

                    USHORT nVisStartX = (USHORT) nStartX;           // sichtbarer Bereich
                    USHORT nVisStartY = (USHORT) nStartY;
                    USHORT nVisEndX = (USHORT) nEndX;
                    USHORT nVisEndY = (USHORT) nEndY;
                    StripHidden( nVisStartX, nVisStartY, nVisEndX, nVisEndY, nTab );

                    if ( nX != (short) nVisStartX && nArrX > 0)
                        pThisRowInfo->pCellInfo[nArrX-1].pNextLeft = NULL;
                    if ( nY != (short) nVisStartY && nArrY > 0)
                        pRowInfo[nArrY-1].pCellInfo[nArrX].pNextTop = NULL;

                    if ( nX != (short) nVisEndX ) pInfo->pThisRight = NULL;
                    if ( nY != (short) nVisEndY ) pInfo->pThisBottom = NULL;

                    if ( nX == (short) nVisStartX || nY == (short) nVisStartY ||
                         nX == (short) nVisEndX   || nY == (short) nVisEndY )
                    {
                        if ( !pStartCond || pStartCond->
                                    GetItemState(ATTR_BORDER,TRUE,&pItem) != SFX_ITEM_SET )
                            pItem = &pStartPattern->GetItem(ATTR_BORDER);
                        const SvxBoxItem* pBox = (const SvxBoxItem*) pItem;
                        if ( nX == (short) nVisStartX && nArrX > 0 )
                            pThisRowInfo->pCellInfo[nArrX-1].pNextLeft = pBox->GetLeft();
                        if ( nY == (short) nVisStartY && nArrY > 0)
                            pRowInfo[nArrY-1].pCellInfo[nArrX].pNextTop = pBox->GetTop();
                        if ( nX == (short) nVisEndX )
                            pInfo->pThisRight = pBox->GetRight();
                        if ( nY == (short) nVisEndY )
                            pInfo->pThisBottom = pBox->GetBottom();
                    }

                    // Hintergrund kopieren (oder in output.cxx)

                    if ( !pStartCond || pStartCond->
                                    GetItemState(ATTR_BACKGROUND,TRUE,&pItem) != SFX_ITEM_SET )
                        pItem = &pStartPattern->GetItem(ATTR_BACKGROUND);
                    pInfo->pBackground = (const SvxBrushItem*) pItem;
                    pRowInfo[nArrY].bEmptyBack = FALSE;

                    // Schatten

                    if ( !pStartCond || pStartCond->
                                    GetItemState(ATTR_SHADOW,TRUE,&pItem) != SFX_ITEM_SET )
                        pItem = &pStartPattern->GetItem(ATTR_SHADOW);
                    pInfo->pShadowAttr = (const SvxShadowItem*) pItem;
                    if (pInfo->pShadowAttr != pDefShadow)
                        bAnyShadow = TRUE;

                    // Blockmarken - wieder mit Original-Merge-Werten

                    BOOL bCellMarked = FALSE;
                    if (bPaintMarks)
                        bCellMarked = ( nStartX >= (short) nBlockStartX
                                    && nStartX <= (short) nBlockEndX
                                    && nStartY >= (short) nBlockStartY
                                    && nStartY <= (short) nBlockEndY );
                    if (pMarkData && pMarkData->IsMultiMarked() && !bCellMarked)
                    {
                        const ScMarkArray* pThisMarkArr = pMarkData->GetArray()+nStartX;
                        short nIndex;
                        (void) pThisMarkArr->Search( nStartY, nIndex );
                        bCellMarked=pThisMarkArr->pData[nIndex].bMarked;
                    }

                    pInfo->bMarked = bCellMarked;
                }
            }
        }
    }

    if (bAnyLines)                                  // nach Merge Linien zusammenfassen
    {
        for (nArrY=0; nArrY<nArrCount; nArrY++)
        {
            RowInfo* pThisRowInfo = &pRowInfo[nArrY];
//          short nY = nArrY ? pRowInfo[nArrY].nRowNo : ((short)nY1)-1;

            for (nArrX=nX1; nArrX<=nX2+2; nArrX++)                  // links und rechts einer mehr
            {
//              short nX = ((short) nArrX) - 1;
                CellInfo* pInfo = &pThisRowInfo->pCellInfo[nArrX];

                if ( pInfo->pThisBottom || pInfo->pNextTop )
                {
                    if ( bPageMode && nArrY == 0 )
                        pInfo->pBottomLine = pInfo->pNextTop;
                    else if ( bPageMode && nArrY+2 >= nArrCount )
                        pInfo->pBottomLine = pInfo->pThisBottom;
                    else if (HasPriority(pInfo->pThisBottom,pInfo->pNextTop))
                        pInfo->pBottomLine = pInfo->pThisBottom;
                    else
                        pInfo->pBottomLine = pInfo->pNextTop;
                }
                else
                    pInfo->pBottomLine = NULL;                      //! Standard

                if ( pInfo->pThisRight || pInfo->pNextLeft )
                {
                    if ( bPageMode && nArrX == nX1 )
                        pInfo->pRightLine = pInfo->pNextLeft;
                    else if ( bPageMode && nArrX > nX2 )
                        pInfo->pRightLine = pInfo->pThisRight;
                    else if (HasPriority(pInfo->pThisRight,pInfo->pNextLeft))
                        pInfo->pRightLine = pInfo->pThisRight;
                    else
                        pInfo->pRightLine = pInfo->pNextLeft;
                }
                else
                    pInfo->pRightLine = NULL;                       //! Standard
            }
        }
    }

    if (bAnyShadow)                             // Schatten verteilen
    {
        for (nArrY=0; nArrY<nArrCount; nArrY++)
        {
            BOOL bTop = ( nArrY == 0 );
            BOOL bBottom = ( nArrY+1 == nArrCount );

//          short nY = nArrY ? pRowInfo[nArrY].nRowNo : ((short)nY1)-1;

            for (nArrX=nX1; nArrX<=nX2+2; nArrX++)                  // links und rechts einer mehr
            {
                BOOL bLeft = ( nArrX == nX1 );
                BOOL bRight = ( nArrX == nX2+2 );

//              short nX = ((short) nArrX) - 1;
                CellInfo* pInfo = &pRowInfo[nArrY].pCellInfo[nArrX];
                const SvxShadowItem* pThisAttr = pInfo->pShadowAttr;
                SvxShadowLocation eLoc = pThisAttr ? pThisAttr->GetLocation() : SVX_SHADOW_NONE;
                if (eLoc != SVX_SHADOW_NONE)
                {
                    //  oder Test auf != eLoc

                    short nDxPos = 1;
                    short nDxNeg = -1;

                    while ( nArrX+nDxPos < nX2+2 && pRowInfo[0].pCellInfo[nArrX+nDxPos].nWidth == 0 )
                        ++nDxPos;
                    while ( nArrX+nDxNeg > nX1 && pRowInfo[0].pCellInfo[nArrX+nDxNeg].nWidth == 0 )
                        --nDxNeg;

                    BOOL bLeftDiff = !bLeft &&
                            CELLINFO(nDxNeg,0).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
                    BOOL bRightDiff = !bRight &&
                            CELLINFO(nDxPos,0).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
                    BOOL bTopDiff = !bTop &&
                            CELLINFO(0,-1).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;
                    BOOL bBottomDiff = !bBottom &&
                            CELLINFO(0,1).pShadowAttr->GetLocation() == SVX_SHADOW_NONE;

                    switch (eLoc)
                    {
                        case SVX_SHADOW_BOTTOMRIGHT:
                            if (bBottomDiff)
                            {
                                CELLINFO(0,1).pHShadowOrigin = pThisAttr;
                                CELLINFO(0,1).eHShadowPart =
                                                bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bRightDiff)
                            {
                                CELLINFO(1,0).pVShadowOrigin = pThisAttr;
                                CELLINFO(1,0).eVShadowPart =
                                                bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bBottomDiff && bRightDiff)
                            {
                                CELLINFO(1,1).pHShadowOrigin = pThisAttr;
                                CELLINFO(1,1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SVX_SHADOW_BOTTOMLEFT:
                            if (bBottomDiff)
                            {
                                CELLINFO(0,1).pHShadowOrigin = pThisAttr;
                                CELLINFO(0,1).eHShadowPart =
                                                bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bLeftDiff)
                            {
                                CELLINFO(-1,0).pVShadowOrigin = pThisAttr;
                                CELLINFO(-1,0).eVShadowPart =
                                                bTopDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bBottomDiff && bLeftDiff)
                            {
                                CELLINFO(-1,1).pHShadowOrigin = pThisAttr;
                                CELLINFO(-1,1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SVX_SHADOW_TOPRIGHT:
                            if (bTopDiff)
                            {
                                CELLINFO(0,-1).pHShadowOrigin = pThisAttr;
                                CELLINFO(0,-1).eHShadowPart =
                                                bLeftDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bRightDiff)
                            {
                                CELLINFO(1,0).pVShadowOrigin = pThisAttr;
                                CELLINFO(1,0).eVShadowPart =
                                                bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bTopDiff && bRightDiff)
                            {
                                CELLINFO(1,-1).pHShadowOrigin = pThisAttr;
                                CELLINFO(1,-1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        case SVX_SHADOW_TOPLEFT:
                            if (bTopDiff)
                            {
                                CELLINFO(0,-1).pHShadowOrigin = pThisAttr;
                                CELLINFO(0,-1).eHShadowPart =
                                                bRightDiff ? SC_SHADOW_HSTART : SC_SHADOW_HORIZ;
                            }
                            if (bLeftDiff)
                            {
                                CELLINFO(-1,0).pVShadowOrigin = pThisAttr;
                                CELLINFO(-1,0).eVShadowPart =
                                                bBottomDiff ? SC_SHADOW_VSTART : SC_SHADOW_VERT;
                            }
                            if (bTopDiff && bLeftDiff)
                            {
                                CELLINFO(-1,-1).pHShadowOrigin = pThisAttr;
                                CELLINFO(-1,-1).eHShadowPart = SC_SHADOW_CORNER;
                            }
                            break;

                        default:
                            DBG_ERROR("falscher Shadow-Enum");
                    }
                }
            }
        }
    }

    return nArrCount;
}



