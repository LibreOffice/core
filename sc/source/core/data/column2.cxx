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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "scitems.hxx"
#include <editeng/eeitem.hxx>

#include <svx/algitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/forbiddencharacterstable.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/zforlist.hxx>
#include <svl/broadcast.hxx>
#include <svl/listeneriter.hxx>
#include <vcl/outdev.hxx>

#include "column.hxx"
#include "cell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "collect.hxx"
#include "stlsheet.hxx"
#include "rechead.hxx"
#include "brdcst.hxx"
#include "editutil.hxx"
#include "subtotal.hxx"
#include "markdata.hxx"
#include "compiler.hxx"         // ScTokenArray GetCodeLen
#include "dbcolect.hxx"
#include "fillinfo.hxx"
#include "segmenttree.hxx"
#include "docparam.hxx"

#include <math.h>

// -----------------------------------------------------------------------

// factor from font size to optimal cell height (text width)
#define SC_ROT_BREAK_FACTOR     6

// -----------------------------------------------------------------------

inline bool IsAmbiguousScript( sal_uInt8 nScript )
{
    //! move to a header file
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}

// -----------------------------------------------------------------------------------------

//
//  Datei-Operationen
//

// -----------------------------------------------------------------------------------------


long ScColumn::GetNeededSize( SCROW nRow, OutputDevice* pDev,
                              double nPPTX, double nPPTY,
                              const Fraction& rZoomX, const Fraction& rZoomY,
                              bool bWidth, const ScNeededSizeOptions& rOptions )
{
    long nValue=0;
    SCSIZE nIndex;
    double nPPT = bWidth ? nPPTX : nPPTY;
    if (Search(nRow,nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        const ScPatternAttr* pPattern = rOptions.pPattern;
        if (!pPattern)
            pPattern = pAttrArray->GetPattern( nRow );

        //      zusammengefasst?
        //      Merge nicht in bedingter Formatierung

        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);

        if ( bWidth )
        {
            if ( pFlag->IsHorOverlapped() )
                return 0;
            if ( rOptions.bSkipMerged && pMerge->GetColMerge() > 1 )
                return 0;
        }
        else
        {
            if ( pFlag->IsVerOverlapped() )
                return 0;
            if ( rOptions.bSkipMerged && pMerge->GetRowMerge() > 1 )
                return 0;
        }

        //      bedingte Formatierung
        const SfxItemSet* pCondSet = NULL;
        if ( ((const SfxUInt32Item&)pPattern->GetItem(ATTR_CONDITIONAL)).GetValue() )
            pCondSet = pDocument->GetCondResult( nCol, nRow, nTab );

        //  Zeilenumbruch?

        const SfxPoolItem* pCondItem;
        SvxCellHorJustify eHorJust;
        if (pCondSet &&
                pCondSet->GetItemState(ATTR_HOR_JUSTIFY, true, &pCondItem) == SFX_ITEM_SET)
            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem*)pCondItem)->GetValue();
        else
            eHorJust = (SvxCellHorJustify)((const SvxHorJustifyItem&)
                                            pPattern->GetItem( ATTR_HOR_JUSTIFY )).GetValue();
        bool bBreak;
        if ( eHorJust == SVX_HOR_JUSTIFY_BLOCK )
            bBreak = true;
        else if ( pCondSet &&
                    pCondSet->GetItemState(ATTR_LINEBREAK, true, &pCondItem) == SFX_ITEM_SET)
            bBreak = ((const SfxBoolItem*)pCondItem)->GetValue();
        else
            bBreak = ((const SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue();

        if (pCell->HasValueData())
            // Cell has a value.  Disable line break.
            bBreak = false;

        //  get other attributes from pattern and conditional formatting

        SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
        bool bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
                ((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet )).GetValue() );
        if ( bAsianVertical )
            bBreak = false;

        if ( bWidth && bBreak )     // after determining bAsianVertical (bBreak may be reset)
            return 0;

        long nRotate = 0;
        SvxRotateMode eRotMode = SVX_ROTATE_MODE_STANDARD;
        if ( eOrient == SVX_ORIENTATION_STANDARD )
        {
            if (pCondSet &&
                    pCondSet->GetItemState(ATTR_ROTATE_VALUE, true, &pCondItem) == SFX_ITEM_SET)
                nRotate = ((const SfxInt32Item*)pCondItem)->GetValue();
            else
                nRotate = ((const SfxInt32Item&)pPattern->GetItem(ATTR_ROTATE_VALUE)).GetValue();
            if ( nRotate )
            {
                if (pCondSet &&
                        pCondSet->GetItemState(ATTR_ROTATE_MODE, true, &pCondItem) == SFX_ITEM_SET)
                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem*)pCondItem)->GetValue();
                else
                    eRotMode = (SvxRotateMode)((const SvxRotateModeItem&)
                                                pPattern->GetItem(ATTR_ROTATE_MODE)).GetValue();

                if ( nRotate == 18000 )
                    eRotMode = SVX_ROTATE_MODE_STANDARD;    // keinen Ueberlauf
            }
        }

        if ( eHorJust == SVX_HOR_JUSTIFY_REPEAT )
        {
            // ignore orientation/rotation if "repeat" is active
            eOrient = SVX_ORIENTATION_STANDARD;
            nRotate = 0;
            bAsianVertical = false;
        }

        const SvxMarginItem* pMargin;
        if (pCondSet &&
                pCondSet->GetItemState(ATTR_MARGIN, true, &pCondItem) == SFX_ITEM_SET)
            pMargin = (const SvxMarginItem*) pCondItem;
        else
            pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        sal_uInt16 nIndent = 0;
        if ( eHorJust == SVX_HOR_JUSTIFY_LEFT )
        {
            if (pCondSet &&
                    pCondSet->GetItemState(ATTR_INDENT, true, &pCondItem) == SFX_ITEM_SET)
                nIndent = ((const SfxUInt16Item*)pCondItem)->GetValue();
            else
                nIndent = ((const SfxUInt16Item&)pPattern->GetItem(ATTR_INDENT)).GetValue();
        }

        sal_uInt8 nScript = pDocument->GetScriptType( nCol, nRow, nTab, pCell );
        if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

        //  also call SetFont for edit cells, because bGetFont may be set only once
        //  bGetFont is set also if script type changes
        if (rOptions.bGetFont)
        {
            Fraction aFontZoom = ( eOrient == SVX_ORIENTATION_STANDARD ) ? rZoomX : rZoomY;
            Font aFont;
            // font color doesn't matter here
            pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &aFontZoom, pCondSet, nScript );
            pDev->SetFont(aFont);
        }

        bool bAddMargin = true;
        CellType eCellType = pCell->GetCellType();

        bool bEditEngine = ( eCellType == CELLTYPE_EDIT ||
                                eOrient == SVX_ORIENTATION_STACKED ||
                                IsAmbiguousScript( nScript ) ||
                                ((eCellType == CELLTYPE_FORMULA) && ((ScFormulaCell*)pCell)->IsMultilineResult()) );

        if (!bEditEngine)                                   // direkte Ausgabe
        {
            String aValStr;
            Color* pColor;
            SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
            sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
            ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
                                        *pFormatter,
                                        true, rOptions.bFormula, ftCheck );
            if (aValStr.Len())
            {
                //  SetFont ist nach oben verschoben

                Size aSize( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
                if ( eOrient != SVX_ORIENTATION_STANDARD )
                {
                    long nTemp = aSize.Width();
                    aSize.Width() = aSize.Height();
                    aSize.Height() = nTemp;
                }
                else if ( nRotate )
                {
                    //! unterschiedliche Skalierung X/Y beruecksichtigen

                    double nRealOrient = nRotate * F_PI18000;   // nRotate sind 1/100 Grad
                    double nCosAbs = fabs( cos( nRealOrient ) );
                    double nSinAbs = fabs( sin( nRealOrient ) );
                    long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                    long nWidth;
                    if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                        nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                    else if ( rOptions.bTotalSize )
                    {
                        nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
                        bAddMargin = false;
                        //  nur nach rechts:
                        //! unterscheiden nach Ausrichtung oben/unten (nur Text/ganze Hoehe)
                        if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                            nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                                nPPT * nCosAbs / nSinAbs );
                    }
                    else
                        nWidth  = (long)( aSize.Height() / nSinAbs );   //! begrenzen?

                    if ( bBreak && !rOptions.bTotalSize )
                    {
                        //  limit size for line break
                        long nCmp = pDev->GetFont().GetSize().Height() * SC_ROT_BREAK_FACTOR;
                        if ( nHeight > nCmp )
                            nHeight = nCmp;
                    }

                    aSize = Size( nWidth, nHeight );
                }
                nValue = bWidth ? aSize.Width() : aSize.Height();

                if ( bAddMargin )
                {
                    if (bWidth)
                    {
                        nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
                                  (long) ( pMargin->GetRightMargin() * nPPT );
                        if ( nIndent )
                            nValue += (long) ( nIndent * nPPT );
                    }
                    else
                        nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
                                  (long) ( pMargin->GetBottomMargin() * nPPT );
                }

                                                //  Zeilenumbruch ausgefuehrt ?

                if ( bBreak && !bWidth )
                {
                    //  Test mit EditEngine zur Sicherheit schon bei 90%
                    //  (wegen Rundungsfehlern und weil EditEngine teilweise anders formatiert)

                    long nDocPixel = (long) ( ( pDocument->GetColWidth( nCol,nTab ) -
                                        pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
                                        nIndent )
                                        * nPPT );
                    nDocPixel = (nDocPixel * 9) / 10;           // zur Sicherheit
                    if ( aSize.Width() > nDocPixel )
                        bEditEngine = true;
                }
            }
        }

        if (bEditEngine)
        {
            //  der Font wird bei !bEditEngine nicht jedesmal neu gesetzt
            Font aOldFont = pDev->GetFont();

            MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );

            // am Dokument speichern ?
            ScFieldEditEngine* pEngine = pDocument->CreateFieldEditEngine();

            pEngine->SetUpdateMode( false );
            MapMode aOld = pDev->GetMapMode();
            pDev->SetMapMode( aHMMMode );
            pEngine->SetRefDevice( pDev );
            pEngine->SetForbiddenCharsTable( pDocument->GetForbiddenCharacters() );
            pEngine->SetAsianCompressionMode( pDocument->GetAsianCompression() );
            pEngine->SetKernAsianPunctuation( pDocument->GetAsianKerning() );
            SfxItemSet* pSet = new SfxItemSet( pEngine->GetEmptyItemSet() );
            pPattern->FillEditItemSet( pSet, pCondSet );

//          no longer needed, are setted with the text (is faster)
//          pEngine->SetDefaults( pSet );

            if ( ((const SfxBoolItem&)pSet->Get(EE_PARA_HYPHENATE)).GetValue() ) {

                com::sun::star::uno::Reference<com::sun::star::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
                pEngine->SetHyphenator( xXHyphenator );
            }

            Size aPaper = Size( 1000000, 1000000 );
            if ( eOrient==SVX_ORIENTATION_STACKED && !bAsianVertical )
                aPaper.Width() = 1;
            else if (bBreak)
            {
                double fWidthFactor = nPPTX;
                bool bTextWysiwyg = ( pDev->GetOutDevType() == OUTDEV_PRINTER );
                if ( bTextWysiwyg )
                {
                    //  if text is formatted for printer, don't use PixelToLogic,
                    //  to ensure the exact same paper width (and same line breaks) as in
                    //  ScEditUtil::GetEditArea, used for output.

                    fWidthFactor = HMM_PER_TWIPS;
                }

                // use original width for hidden columns:
                long nDocWidth = (long) ( pDocument->GetOriginalWidth(nCol,nTab) * fWidthFactor );
                SCCOL nColMerge = pMerge->GetColMerge();
                if (nColMerge > 1)
                    for (SCCOL nColAdd=1; nColAdd<nColMerge; nColAdd++)
                        nDocWidth += (long) ( pDocument->GetColWidth(nCol+nColAdd,nTab) * fWidthFactor );
                nDocWidth -= (long) ( pMargin->GetLeftMargin() * fWidthFactor )
                           + (long) ( pMargin->GetRightMargin() * fWidthFactor )
                           + 1;     // Ausgabebereich ist Breite-1 Pixel (wegen Gitterlinien)
                if ( nIndent )
                    nDocWidth -= (long) ( nIndent * fWidthFactor );

                // space for AutoFilter button:  20 * nZoom/100
                if ( pFlag->HasAutoFilter() && !bTextWysiwyg )
                    nDocWidth -= (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();

                aPaper.Width() = nDocWidth;

                if ( !bTextWysiwyg )
                    aPaper = pDev->PixelToLogic( aPaper, aHMMMode );
            }
            pEngine->SetPaperSize(aPaper);

            if ( pCell->GetCellType() == CELLTYPE_EDIT )
            {
                const EditTextObject* pData;
                ((ScEditCell*)pCell)->GetData(pData);
                pEngine->SetTextNewDefaults(*pData, pSet);
            }
            else
            {
                Color* pColor;
                SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
                sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
                String aString;
                ScCellFormat::GetString( pCell, nFormat, aString, &pColor,
                                            *pFormatter,
                                            true, rOptions.bFormula, ftCheck );
                if (aString.Len())
                    pEngine->SetTextNewDefaults(aString, pSet);
                else
                    pEngine->SetDefaults(pSet);
            }

            bool bEngineVertical = pEngine->IsVertical();
            pEngine->SetVertical( bAsianVertical );
            pEngine->SetUpdateMode( true );

            bool bEdWidth = bWidth;
            if ( eOrient != SVX_ORIENTATION_STANDARD && eOrient != SVX_ORIENTATION_STACKED )
                bEdWidth = !bEdWidth;
            if ( nRotate )
            {
                //! unterschiedliche Skalierung X/Y beruecksichtigen

                Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
                double nRealOrient = nRotate * F_PI18000;   // nRotate sind 1/100 Grad
                double nCosAbs = fabs( cos( nRealOrient ) );
                double nSinAbs = fabs( sin( nRealOrient ) );
                long nHeight = (long)( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                long nWidth;
                if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                    nWidth  = (long)( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                else if ( rOptions.bTotalSize )
                {
                    nWidth = (long) ( pDocument->GetColWidth( nCol,nTab ) * nPPT );
                    bAddMargin = false;
                    if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                        nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                            nPPT * nCosAbs / nSinAbs );
                }
                else
                    nWidth  = (long)( aSize.Height() / nSinAbs );   //! begrenzen?
                aSize = Size( nWidth, nHeight );

                Size aPixSize = pDev->LogicToPixel( aSize, aHMMMode );
                if ( bEdWidth )
                    nValue = aPixSize.Width();
                else
                {
                    nValue = aPixSize.Height();

                    if ( bBreak && !rOptions.bTotalSize )
                    {
                        //  limit size for line break
                        long nCmp = aOldFont.GetSize().Height() * SC_ROT_BREAK_FACTOR;
                        if ( nValue > nCmp )
                            nValue = nCmp;
                    }
                }
            }
            else if ( bEdWidth )
            {
                if (bBreak)
                    nValue = 0;
                else
                    nValue = pDev->LogicToPixel(Size( pEngine->CalcTextWidth(), 0 ),
                                        aHMMMode).Width();
            }
            else            // Hoehe
            {
                nValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ),
                                    aHMMMode).Height();
            }

            if ( nValue && bAddMargin )
            {
                if (bWidth)
                {
                    nValue += (long) ( pMargin->GetLeftMargin() * nPPT ) +
                              (long) ( pMargin->GetRightMargin() * nPPT );
                    if (nIndent)
                        nValue += (long) ( nIndent * nPPT );
                }
                else
                {
                    nValue += (long) ( pMargin->GetTopMargin() * nPPT ) +
                              (long) ( pMargin->GetBottomMargin() * nPPT );

                    if ( bAsianVertical && pDev->GetOutDevType() != OUTDEV_PRINTER )
                    {
                        //  add 1pt extra (default margin value) for line breaks with SetVertical
                        nValue += (long) ( 20 * nPPT );
                    }
                }
            }

            //  EditEngine is cached and re-used, so the old vertical flag must be restored
            pEngine->SetVertical( bEngineVertical );

            pDocument->DisposeFieldEditEngine(pEngine);

            pDev->SetMapMode( aOld );
            pDev->SetFont( aOldFont );
        }

        if (bWidth)
        {
            //      Platz fuer Autofilter-Button
            //      20 * nZoom/100
            //      bedingte Formatierung hier nicht interessant

            sal_Int16 nFlags = ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).GetValue();
            if (nFlags & SC_MF_AUTO)
                nValue += (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();
        }
    }
    return nValue;
}

long ScColumn::GetSimpleTextNeededSize( SCSIZE nIndex, OutputDevice* pDev,
        bool bWidth )
{
    long nValue=0;
    if ( nIndex < nCount )
    {
        SCROW nRow = pItems[nIndex].nRow;
        const ScPatternAttr* pPattern = pAttrArray->GetPattern( nRow );
        ScBaseCell* pCell = pItems[nIndex].pCell;
        String aValStr;
        Color* pColor;
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter );
        ScCellFormat::GetString( pCell, nFormat, aValStr, &pColor,
                                    *pFormatter, true, false, ftCheck );
        if ( aValStr.Len() )
        {
            if ( bWidth )
                nValue = pDev->GetTextWidth( aValStr );
            else
                nValue = pDev->GetTextHeight();
        }
    }
    return nValue;
}

sal_uInt16 ScColumn::GetOptimalColWidth( OutputDevice* pDev, double nPPTX, double nPPTY,
                                     const Fraction& rZoomX, const Fraction& rZoomY,
                                     bool bFormula, sal_uInt16 nOldWidth,
                                     const ScMarkData* pMarkData,
                                     const ScColWidthParam* pParam )
{
    if (nCount == 0)
        return nOldWidth;

    sal_uInt16  nWidth = (sal_uInt16) (nOldWidth * nPPTX);
    bool    bFound = false;

    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, pMarkData, true);
    if ( pParam && pParam->mbSimpleText )
    {   // alles eins bis auf NumberFormate
        const ScPatternAttr* pPattern = GetPattern( 0 );
        Font aFont;
        // font color doesn't matter here
        pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &rZoomX, NULL );
        pDev->SetFont( aFont );
        const SvxMarginItem* pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        long nMargin = (long) ( pMargin->GetLeftMargin() * nPPTX ) +
                        (long) ( pMargin->GetRightMargin() * nPPTX );

        // Try to find the row that has the longest string, and measure the width of that string.
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter );
        String aLongStr;
        Color* pColor;
        if (pParam->mnMaxTextRow >= 0)
        {
            ScBaseCell* pCell = GetCell(pParam->mnMaxTextRow);
            ScCellFormat::GetString(
                pCell, nFormat, aLongStr, &pColor, *pFormatter, true, false, ftCheck );
        }
        else
        {
            xub_StrLen nLongLen = 0;
            while (aDataIter.Next(nIndex))
            {
                if (nIndex >= nCount)
                    // Out-of-bound reached.  No need to keep going.
                    break;

                ScBaseCell* pCell = pItems[nIndex].pCell;
                String aValStr;
                ScCellFormat::GetString(
                    pCell, nFormat, aValStr, &pColor, *pFormatter, true, false, ftCheck );

                if (aValStr.Len() > nLongLen)
                {
                    nLongLen = aValStr.Len();
                    aLongStr = aValStr;
                }
            }
        }

        if (aLongStr.Len())
        {
            nWidth = pDev->GetTextWidth(aLongStr) + static_cast<sal_uInt16>(nMargin);
            bFound = true;
        }
    }
    else
    {
        ScNeededSizeOptions aOptions;
        aOptions.bFormula = bFormula;
        const ScPatternAttr* pOldPattern = NULL;
        sal_uInt8 nOldScript = 0;

        while (aDataIter.Next( nIndex ))
        {
            SCROW nRow = pItems[nIndex].nRow;

            sal_uInt8 nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
            if (nScript == 0) nScript = ScGlobal::GetDefaultScriptType();

            const ScPatternAttr* pPattern = GetPattern( nRow );
            aOptions.pPattern = pPattern;
            aOptions.bGetFont = (pPattern != pOldPattern || nScript != nOldScript);
            sal_uInt16 nThis = (sal_uInt16) GetNeededSize( nRow, pDev, nPPTX, nPPTY,
                rZoomX, rZoomY, true, aOptions );
            pOldPattern = pPattern;
            if (nThis)
            {
                if (nThis>nWidth || !bFound)
                {
                    nWidth = nThis;
                    bFound = true;
                }
            }
        }
    }

    if (bFound)
    {
        nWidth += 2;
        sal_uInt16 nTwips = (sal_uInt16) (nWidth / nPPTX);
        return nTwips;
    }
    else
        return nOldWidth;
}

sal_uInt16 lcl_GetAttribHeight( const ScPatternAttr& rPattern, sal_uInt16 nFontHeightId )
{
    sal_uInt16 nHeight = (sal_uInt16) ((const SvxFontHeightItem&) rPattern.GetItem(nFontHeightId)).GetHeight();
    const SvxMarginItem* pMargin = (const SvxMarginItem*) &rPattern.GetItem(ATTR_MARGIN);
    nHeight += nHeight / 5;
    //  gibt bei 10pt 240

    if ( ((const SvxEmphasisMarkItem&)rPattern.
            GetItem(ATTR_FONT_EMPHASISMARK)).GetEmphasisMark() != EMPHASISMARK_NONE )
    {
        //  add height for emphasis marks
        //! font metrics should be used instead
        nHeight += nHeight / 4;
    }

    if ( nHeight + 240 > ScGlobal::nDefFontHeight )
    {
        nHeight = sal::static_int_cast<sal_uInt16>( nHeight + ScGlobal::nDefFontHeight );
        nHeight -= 240;
    }

    //  Standard-Hoehe: TextHeight + Raender - 23
    //  -> 257 unter Windows

    if (nHeight > STD_ROWHEIGHT_DIFF)
        nHeight -= STD_ROWHEIGHT_DIFF;

    nHeight += pMargin->GetTopMargin() + pMargin->GetBottomMargin();

    return nHeight;
}

//  pHeight in Twips
//  nMinHeight, nMinStart zur Optimierung: ab nRow >= nMinStart ist mindestens nMinHeight
//  (wird nur bei bStdAllowed ausgewertet)

void ScColumn::GetOptimalHeight( SCROW nStartRow, SCROW nEndRow, sal_uInt16* pHeight,
                                OutputDevice* pDev,
                                double nPPTX, double nPPTY,
                                const Fraction& rZoomX, const Fraction& rZoomY,
                                bool bShrink, sal_uInt16 nMinHeight, SCROW nMinStart )
{
    ScAttrIterator aIter( pAttrArray, nStartRow, nEndRow );

    SCROW nStart = -1;
    SCROW nEnd = -1;
    SCROW nEditPos = 0;
    SCROW nNextEnd = 0;

    //  bei bedingter Formatierung werden immer die einzelnen Zellen angesehen

    const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
    while ( pPattern )
    {
        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
        if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
        {
            //  nix - vertikal bei der zusammengefassten und den ueberdeckten,
            //        horizontal nur bei den ueberdeckten (unsichtbaren) -
            //        eine nur horizontal zusammengefasste wird aber beruecksichtigt
        }
        else
        {
            SCROW nRow = 0;
            bool bStdAllowed = (pPattern->GetCellOrientation() == SVX_ORIENTATION_STANDARD);
            bool bStdOnly = false;
            if (bStdAllowed)
            {
                bool bBreak = ((SfxBoolItem&)pPattern->GetItem(ATTR_LINEBREAK)).GetValue() ||
                                ((SvxCellHorJustify)((const SvxHorJustifyItem&)pPattern->
                                    GetItem( ATTR_HOR_JUSTIFY )).GetValue() ==
                                    SVX_HOR_JUSTIFY_BLOCK);
                bStdOnly = !bBreak;

                // bedingte Formatierung: Zellen durchgehen
                if ( bStdOnly && ((const SfxUInt32Item&)pPattern->
                                    GetItem(ATTR_CONDITIONAL)).GetValue() )
                    bStdOnly = false;

                // gedrehter Text: Zellen durchgehen
                if ( bStdOnly && ((const SfxInt32Item&)pPattern->
                                    GetItem(ATTR_ROTATE_VALUE)).GetValue() )
                    bStdOnly = false;
            }

            if (bStdOnly)
                if (HasEditCells(nStart,nEnd,nEditPos))     // includes mixed script types
                {
                    if (nEditPos == nStart)
                    {
                        bStdOnly = false;
                        if (nEnd > nEditPos)
                            nNextEnd = nEnd;
                        nEnd = nEditPos;                // einzeln ausrechnen
                        bStdAllowed = false;            // wird auf jeden Fall per Zelle berechnet
                    }
                    else
                    {
                        nNextEnd = nEnd;
                        nEnd = nEditPos - 1;            // Standard - Teil
                    }
                }

            if (bStdAllowed)
            {
                sal_uInt16 nLatHeight = 0;
                sal_uInt16 nCjkHeight = 0;
                sal_uInt16 nCtlHeight = 0;
                sal_uInt16 nDefHeight;
                sal_uInt8 nDefScript = ScGlobal::GetDefaultScriptType();
                if ( nDefScript == SCRIPTTYPE_ASIAN )
                    nDefHeight = nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
                else if ( nDefScript == SCRIPTTYPE_COMPLEX )
                    nDefHeight = nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
                else
                    nDefHeight = nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );

                //  if everything below is already larger, the loop doesn't have to
                //  be run again
                SCROW nStdEnd = nEnd;
                if ( nDefHeight <= nMinHeight && nStdEnd >= nMinStart )
                    nStdEnd = (nMinStart>0) ? nMinStart-1 : 0;

                for (nRow=nStart; nRow<=nStdEnd; nRow++)
                    if (nDefHeight > pHeight[nRow-nStartRow])
                        pHeight[nRow-nStartRow] = nDefHeight;

                if ( bStdOnly )
                {
                    //  if cells are not handled individually below,
                    //  check for cells with different script type

                    SCSIZE nIndex;
                    Search(nStart,nIndex);
                    while ( nIndex < nCount && (nRow=pItems[nIndex].nRow) <= nEnd )
                    {
                        sal_uInt8 nScript = pDocument->GetScriptType( nCol, nRow, nTab, pItems[nIndex].pCell );
                        if ( nScript != nDefScript )
                        {
                            if ( nScript == SCRIPTTYPE_ASIAN )
                            {
                                if ( nCjkHeight == 0 )
                                    nCjkHeight = lcl_GetAttribHeight( *pPattern, ATTR_CJK_FONT_HEIGHT );
                                if (nCjkHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nCjkHeight;
                            }
                            else if ( nScript == SCRIPTTYPE_COMPLEX )
                            {
                                if ( nCtlHeight == 0 )
                                    nCtlHeight = lcl_GetAttribHeight( *pPattern, ATTR_CTL_FONT_HEIGHT );
                                if (nCtlHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nCtlHeight;
                            }
                            else
                            {
                                if ( nLatHeight == 0 )
                                    nLatHeight = lcl_GetAttribHeight( *pPattern, ATTR_FONT_HEIGHT );
                                if (nLatHeight > pHeight[nRow-nStartRow])
                                    pHeight[nRow-nStartRow] = nLatHeight;
                            }
                        }
                        ++nIndex;
                    }
                }
            }

            if (!bStdOnly)                      // belegte Zellen suchen
            {
                ScNeededSizeOptions aOptions;

                SCSIZE nIndex;
                Search(nStart,nIndex);
                while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEnd) : false )
                {
                    //  Zellhoehe nur berechnen, wenn sie spaeter auch gebraucht wird (#37928#)

                    if ( bShrink || !(pDocument->GetRowFlags(nRow, nTab) & CR_MANUALSIZE) )
                    {
                        aOptions.pPattern = pPattern;
                        sal_uInt16 nHeight = (sal_uInt16)
                                ( GetNeededSize( nRow, pDev, nPPTX, nPPTY,
                                                    rZoomX, rZoomY, false, aOptions ) / nPPTY );
                        if (nHeight > pHeight[nRow-nStartRow])
                            pHeight[nRow-nStartRow] = nHeight;
                    }
                    ++nIndex;
                }
            }
        }

        if (nNextEnd > 0)
        {
            nStart = nEnd + 1;
            nEnd = nNextEnd;
            nNextEnd = 0;
        }
        else
            pPattern = aIter.Next(nStart,nEnd);
    }
}

bool ScColumn::GetNextSpellingCell(SCROW& nRow, bool bInSel, const ScMarkData& rData) const
{
    bool bStop = false;
    CellType eCellType;
    SCSIZE nIndex;
    if (!bInSel && Search(nRow, nIndex))
    {
        eCellType = GetCellType(nRow);
        if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
             !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
               pDocument->IsTabProtected(nTab)) )
                return true;
    }
    while (!bStop)
    {
        if (bInSel)
        {
            nRow = rData.GetNextMarked(nCol, nRow, false);
            if (!ValidRow(nRow))
            {
                nRow = MAXROW+1;
                bStop = true;
            }
            else
            {
                eCellType = GetCellType(nRow);
                if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
                     !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                       pDocument->IsTabProtected(nTab)) )
                        return true;
                else
                    nRow++;
            }
        }
        else if (GetNextDataPos(nRow))
        {
            eCellType = GetCellType(nRow);
            if ( (eCellType == CELLTYPE_STRING || eCellType == CELLTYPE_EDIT) &&
                 !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                   pDocument->IsTabProtected(nTab)) )
                    return true;
            else
                nRow++;
        }
        else
        {
            nRow = MAXROW+1;
            bStop = true;
        }
    }
    return false;
}

// =========================================================================================

void ScColumn::RemoveAutoSpellObj()
{
    ScTabEditEngine* pEngine = NULL;

    for (SCSIZE i=0; i<nCount; i++)
        if ( pItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) pItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();
            //  keine Abfrage auf HasOnlineSpellErrors, damit es auch
            //  nach dem Laden funktioniert

            //  Fuer den Test auf harte Formatierung (ScEditAttrTester) sind die Defaults
            //  in der EditEngine unwichtig. Wenn der Tester spaeter einmal gleiche
            //  Attribute in Default und harter Formatierung erkennen und weglassen sollte,
            //  muessten an der EditEngine zu jeder Zelle die richtigen Defaults gesetzt
            //  werden!

            //  auf Attribute testen
            if ( !pEngine )
                pEngine = new ScTabEditEngine(pDocument);
            pEngine->SetText( *pData );
            ScEditAttrTester aTester( pEngine );
            if ( aTester.NeedsObject() )                    // nur Spell-Errors entfernen
            {
                EditTextObject* pNewData = pEngine->CreateTextObject(); // ohne BIGOBJ
                pOldCell->SetData( pNewData, pEngine->GetEditTextObjectPool() );
                delete pNewData;
            }
            else                                            // String erzeugen
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
                pNewCell->TakeNote( pOldCell->ReleaseNote() );
                pItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

void ScColumn::RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow )
{
    ScFieldEditEngine* pEngine = NULL;

    SCSIZE i;
    Search( nStartRow, i );
    for (; i<nCount && pItems[i].nRow <= nEndRow; i++)
        if ( pItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) pItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();

            //  Fuer den Test auf harte Formatierung (ScEditAttrTester) sind die Defaults
            //  in der EditEngine unwichtig. Wenn der Tester spaeter einmal gleiche
            //  Attribute in Default und harter Formatierung erkennen und weglassen sollte,
            //  muessten an der EditEngine zu jeder Zelle die richtigen Defaults gesetzt
            //  werden!

            //  auf Attribute testen
            if ( !pEngine )
            {
                //pEngine = new ScTabEditEngine(pDocument);
                pEngine = new ScFieldEditEngine( pDocument->GetEditPool() );
                //  EE_CNTRL_ONLINESPELLING falls schon Fehler drin sind
                pEngine->SetControlWord( pEngine->GetControlWord() | EE_CNTRL_ONLINESPELLING );
                pEngine->SetForbiddenCharsTable( pDocument->GetForbiddenCharacters() );
                pEngine->SetAsianCompressionMode( pDocument->GetAsianCompression() );
                pEngine->SetKernAsianPunctuation( pDocument->GetAsianKerning() );
            }
            pEngine->SetText( *pData );
            sal_uInt16 nParCount = pEngine->GetParagraphCount();
            for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
            {
                pEngine->QuickRemoveCharAttribs( nPar );
                const SfxItemSet& rOld = pEngine->GetParaAttribs( nPar );
                if ( rOld.Count() )
                {
                    SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   // leer
                    pEngine->SetParaAttribs( nPar, aNew );
                }
            }
            //  URL-Felder in Text wandeln (andere gibt's nicht, darum pType=0)
            pEngine->RemoveFields( true );

            bool bSpellErrors = pEngine->HasOnlineSpellErrors();
            bool bNeedObject = bSpellErrors || nParCount>1;         // Errors/Absaetze behalten
            //  ScEditAttrTester nicht mehr noetig, Felder sind raus

            if ( bNeedObject )                                      // bleibt Edit-Zelle
            {
                sal_uInt32 nCtrl = pEngine->GetControlWord();
                sal_uInt32 nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
                if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                    pEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );
                EditTextObject* pNewData = pEngine->CreateTextObject();
                pOldCell->SetData( pNewData, pEngine->GetEditTextObjectPool() );
                delete pNewData;
            }
            else                                            // String erzeugen
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
                pNewCell->TakeNote( pOldCell->ReleaseNote() );
                pItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

// =========================================================================================

bool ScColumn::TestTabRefAbs(SCTAB nTable)
{
    bool bRet = false;
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
            if ( pItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)pItems[i].pCell)->TestTabRefAbs(nTable))
                    bRet = true;
    return bRet;
}

// =========================================================================================

ScColumnIterator::ScColumnIterator( const ScColumn* pCol, SCROW nStart, SCROW nEnd ) :
    pColumn( pCol ),
    nTop( nStart ),
    nBottom( nEnd )
{
    pColumn->Search( nTop, nPos );
}

ScColumnIterator::~ScColumnIterator()
{
}

bool ScColumnIterator::Next( SCROW& rRow, ScBaseCell*& rpCell )
{
    if ( nPos < pColumn->nCount )
    {
        rRow = pColumn->pItems[nPos].nRow;
        if ( rRow <= nBottom )
        {
            rpCell = pColumn->pItems[nPos].pCell;
            ++nPos;
            return true;
        }
    }

    rRow = 0;
    rpCell = NULL;
    return false;
}

SCSIZE ScColumnIterator::GetIndex() const           // Index zur letzen abgefragten Zelle
{
    return nPos - 1;        // bei Next ist Pos hochgezaehlt worden
}

// -----------------------------------------------------------------------------------------

ScMarkedDataIter::ScMarkedDataIter( const ScColumn* pCol, const ScMarkData* pMarkData,
                                    bool bAllIfNone ) :
    pColumn( pCol ),
    pMarkIter( NULL ),
    bNext( true ),
    bAll( bAllIfNone )
{
    if (pMarkData && pMarkData->IsMultiMarked())
        pMarkIter = new ScMarkArrayIter( pMarkData->GetArray() + pCol->GetCol() );
}

ScMarkedDataIter::~ScMarkedDataIter()
{
    delete pMarkIter;
}

bool ScMarkedDataIter::Next( SCSIZE& rIndex )
{
    bool bFound = false;
    do
    {
        if (bNext)
        {
            if (!pMarkIter || !pMarkIter->Next( nTop, nBottom ))
            {
                if (bAll)                   // ganze Spalte
                {
                    nTop    = 0;
                    nBottom = MAXROW;
                }
                else
                    return false;
            }
            pColumn->Search( nTop, nPos );
            bNext = false;
            bAll  = false;                  // nur beim ersten Versuch
        }

        if ( nPos >= pColumn->nCount )
            return false;

        if ( pColumn->pItems[nPos].nRow <= nBottom )
            bFound = true;
        else
            bNext = true;
    }
    while (!bFound);

    rIndex = nPos++;
    return true;
}


//------------

bool ScColumn::IsEmptyData() const
{
    return (nCount == 0);
}

bool ScColumn::IsEmptyVisData(bool bNotes) const
{
    if (!pItems || nCount == 0)
        return true;
    else
    {
        bool bVisData = false;
        SCSIZE i;
        for (i=0; i<nCount && !bVisData; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->HasNote()) )
                bVisData = true;
        }
        return !bVisData;
    }
}

SCSIZE ScColumn::VisibleCount( SCROW nStartRow, SCROW nEndRow ) const
{
    //  Notizen werden nicht mitgezaehlt

    SCSIZE nVisCount = 0;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
    {
        if ( pItems[nIndex].nRow >= nStartRow &&
             pItems[nIndex].pCell->GetCellType() != CELLTYPE_NOTE )
        {
            ++nVisCount;
        }
        ++nIndex;
    }
    return nVisCount;
}

SCROW ScColumn::GetLastVisDataPos(bool bNotes) const
{
    SCROW nRet = 0;
    if (pItems)
    {
        SCSIZE i;
        bool bFound = false;
        for (i=nCount; i>0 && !bFound; )
        {
            --i;
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->HasNote()) )
            {
                bFound = true;
                nRet = pItems[i].nRow;
            }
        }
    }
    return nRet;
}

SCROW ScColumn::GetFirstVisDataPos(bool bNotes) const
{
    SCROW nRet = 0;
    if (pItems)
    {
        SCSIZE i;
        bool bFound = false;
        for (i=0; i<nCount && !bFound; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() != CELLTYPE_NOTE || (bNotes && pCell->HasNote()) )
            {
                bFound = true;
                nRet = pItems[i].nRow;
            }
        }
    }
    return nRet;
}

bool ScColumn::HasVisibleDataAt(SCROW nRow) const
{
    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (!pItems[nIndex].pCell->IsBlank())
            return true;

    return false;
}

bool ScColumn::IsEmptyAttr() const
{
    if (pAttrArray)
        return pAttrArray->IsEmpty();
    else
        return true;
}

bool ScColumn::IsEmpty() const
{
    return (IsEmptyData() && IsEmptyAttr());
}

bool ScColumn::IsEmptyBlock(SCROW nStartRow, SCROW nEndRow, bool bIgnoreNotes) const
{
    if ( nCount == 0 || !pItems )
        return true;

    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < nCount && pItems[nIndex].nRow <= nEndRow )
    {
        if ( !pItems[nIndex].pCell->IsBlank( bIgnoreNotes ) )   // found a cell
            return false;                           // not empty
        ++nIndex;
    }
    return true;                                    // no cell found
}

SCSIZE ScColumn::GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const
{
    SCSIZE nLines = 0;
    bool bFound = false;
    SCSIZE i;
    if (pItems && (nCount > 0))
    {
        if (eDir == DIR_BOTTOM)
        {
            i = nCount;
            while (!bFound && (i > 0))
            {
                i--;
                if ( pItems[i].nRow < nStartRow )
                    break;
                bFound = pItems[i].nRow <= nEndRow && !pItems[i].pCell->IsBlank();
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(nEndRow - pItems[i].nRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
        else if (eDir == DIR_TOP)
        {
            i = 0;
            while (!bFound && (i < nCount))
            {
                if ( pItems[i].nRow > nEndRow )
                    break;
                bFound = pItems[i].nRow >= nStartRow && !pItems[i].pCell->IsBlank();
                i++;
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(pItems[i-1].nRow - nStartRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
    }
    else
        nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
    return nLines;
}

SCROW ScColumn::GetFirstDataPos() const
{
    if (nCount)
        return pItems[0].nRow;
    else
        return 0;
}

SCROW ScColumn::GetLastDataPos() const
{
    if (nCount)
        return pItems[nCount-1].nRow;
    else
        return 0;
}

bool ScColumn::GetPrevDataPos(SCROW& rRow) const
{
    bool bFound = false;
    SCSIZE i = nCount;
    while (!bFound && (i > 0))
    {
        --i;
        bFound = (pItems[i].nRow < rRow);
        if (bFound)
            rRow = pItems[i].nRow;
    }
    return bFound;
}

bool ScColumn::GetNextDataPos(SCROW& rRow) const        // greater than rRow
{
    SCSIZE nIndex;
    if (Search( rRow, nIndex ))
        ++nIndex;                   // next cell

    bool bMore = ( nIndex < nCount );
    if ( bMore )
        rRow = pItems[nIndex].nRow;
    return bMore;
}

void ScColumn::FindDataAreaPos(SCROW& rRow, long nMovY) const
{
    if (!nMovY) return;
    bool bForward = (nMovY>0);

    SCSIZE nIndex;
    bool bThere = Search(rRow, nIndex);
    if (bThere && pItems[nIndex].pCell->IsBlank())
        bThere = false;

    if (bThere)
    {
        SCROW nLast = rRow;
        SCSIZE nOldIndex = nIndex;
        if (bForward)
        {
            if (nIndex<nCount-1)
            {
                ++nIndex;
                while (nIndex<nCount-1 && pItems[nIndex].nRow==nLast+1
                                        && !pItems[nIndex].pCell->IsBlank())
                {
                    ++nIndex;
                    ++nLast;
                }
                if (nIndex==nCount-1)
                    if (pItems[nIndex].nRow==nLast+1 && !pItems[nIndex].pCell->IsBlank())
                        ++nLast;
            }
        }
        else
        {
            if (nIndex>0)
            {
                --nIndex;
                while (nIndex>0 && pItems[nIndex].nRow+1==nLast
                                        && !pItems[nIndex].pCell->IsBlank())
                {
                    --nIndex;
                    --nLast;
                }
                if (nIndex==0)
                    if (pItems[nIndex].nRow+1==nLast && !pItems[nIndex].pCell->IsBlank())
                        --nLast;
            }
        }
        if (nLast==rRow)
        {
            bThere = false;
            nIndex = bForward ? nOldIndex+1 : nOldIndex;
        }
        else
            rRow = nLast;
    }

    if (!bThere)
    {
        if (bForward)
        {
            while (nIndex<nCount && pItems[nIndex].pCell->IsBlank())
                ++nIndex;
            if (nIndex<nCount)
                rRow = pItems[nIndex].nRow;
            else
                rRow = MAXROW;
        }
        else
        {
            while (nIndex>0 && pItems[nIndex-1].pCell->IsBlank())
                --nIndex;
            if (nIndex>0)
                rRow = pItems[nIndex-1].nRow;
            else
                rRow = 0;
        }
    }
}

bool ScColumn::HasDataAt(SCROW nRow) const
{
        //  immer nur sichtbare interessant ?
        //! dann HasVisibleDataAt raus

    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (!pItems[nIndex].pCell->IsBlank())
            return true;

    return false;

}

bool ScColumn::IsAllAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray && rCol.pAttrArray)
        return pAttrArray->IsAllEqual( *rCol.pAttrArray, nStartRow, nEndRow );
    else
        return !pAttrArray && !rCol.pAttrArray;
}

bool ScColumn::IsVisibleAttrEqual( const ScColumn& rCol, SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray && rCol.pAttrArray)
        return pAttrArray->IsVisibleEqual( *rCol.pAttrArray, nStartRow, nEndRow );
    else
        return !pAttrArray && !rCol.pAttrArray;
}

bool ScColumn::GetFirstVisibleAttr( SCROW& rFirstRow ) const
{
    if (pAttrArray)
        return pAttrArray->GetFirstVisibleAttr( rFirstRow );
    else
        return false;
}

bool ScColumn::GetLastVisibleAttr( SCROW& rLastRow ) const
{
    if (pAttrArray)
    {
        // row of last cell is needed
        SCROW nLastData = GetLastVisDataPos( true );    // always including notes, 0 if none

        return pAttrArray->GetLastVisibleAttr( rLastRow, nLastData );
    }
    else
        return false;
}

bool ScColumn::HasVisibleAttrIn( SCROW nStartRow, SCROW nEndRow ) const
{
    if (pAttrArray)
        return pAttrArray->HasVisibleAttrIn( nStartRow, nEndRow );
    else
        return false;
}

void ScColumn::FindUsed( SCROW nStartRow, SCROW nEndRow, bool* pUsed ) const
{
    SCROW nRow = 0;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( (nIndex < nCount) ? ((nRow=pItems[nIndex].nRow) <= nEndRow) : false )
    {
        pUsed[nRow-nStartRow] = true;
        ++nIndex;
    }
}

void ScColumn::StartListening( SvtListener& rLst, SCROW nRow )
{
    SvtBroadcaster* pBC = NULL;
    ScBaseCell* pCell;

    SCSIZE nIndex;
    if (Search(nRow,nIndex))
    {
        pCell = pItems[nIndex].pCell;
        pBC = pCell->GetBroadcaster();
    }
    else
    {
        pCell = new ScNoteCell;
        Insert(nRow, pCell);
    }

    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        pCell->TakeBroadcaster(pBC);
    }
    rLst.StartListening(*pBC);
}

void ScColumn::MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow )
{
    SvtBroadcaster* pBC = NULL;
    ScBaseCell* pCell;

    SCSIZE nIndex;
    if (Search(nDestRow,nIndex))
    {
        pCell = pItems[nIndex].pCell;
        pBC = pCell->GetBroadcaster();
    }
    else
    {
        pCell = new ScNoteCell;
        Insert(nDestRow, pCell);
    }

    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        pCell->TakeBroadcaster(pBC);
    }

    if (rSource.HasListeners())
    {
        SvtListenerIter aIter( rSource);
        for (SvtListener* pLst = aIter.GoStart(); pLst; pLst = aIter.GoNext())
        {
            pLst->StartListening( *pBC);
            pLst->EndListening( rSource);
        }
    }
}

void ScColumn::EndListening( SvtListener& rLst, SCROW nRow )
{
    SCSIZE nIndex;
    if (Search(nRow,nIndex))
    {
        ScBaseCell* pCell = pItems[nIndex].pCell;
        SvtBroadcaster* pBC = pCell->GetBroadcaster();
        if (pBC)
        {
            rLst.EndListening(*pBC);

            if (!pBC->HasListeners())
            {
                if (pCell->IsBlank())
                    DeleteAtIndex(nIndex);
                else
                    pCell->DeleteBroadcaster();
            }
        }
    }
}

void ScColumn::CompileDBFormula()
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula();
        }
}

void ScColumn::CompileDBFormula( bool bCreateFormulaString )
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileNameFormula( bool bCreateFormulaString )
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileNameFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileColRowNameFormula()
{
    if (pItems)
        for (SCSIZE i = 0; i < nCount; i++)
        {
            ScBaseCell* pCell = pItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileColRowNameFormula();
        }
}

void lcl_UpdateSubTotal( ScFunctionData& rData, ScBaseCell* pCell )
{
    double nValue = 0.0;
    bool bVal = false;
    bool bCell = true;
    switch (pCell->GetCellType())
    {
        case CELLTYPE_VALUE:
            nValue = ((ScValueCell*)pCell)->GetValue();
            bVal = true;
            break;
        case CELLTYPE_FORMULA:
            {
                if ( rData.eFunc != SUBTOTAL_FUNC_CNT2 )        // da interessiert's nicht
                {
                    ScFormulaCell* pFC = (ScFormulaCell*)pCell;
                    if ( pFC->GetErrCode() )
                    {
                        if ( rData.eFunc != SUBTOTAL_FUNC_CNT ) // fuer Anzahl einfach weglassen
                            rData.bError = true;
                    }
                    else if (pFC->IsValue())
                    {
                        nValue = pFC->GetValue();
                        bVal = true;
                    }
                    // sonst Text
                }
            }
            break;
        case CELLTYPE_NOTE:
            bCell = false;
            break;
        // bei Strings nichts
        default:
        {
            // added to avoid warnings
        }
    }

    if (!rData.bError)
    {
        switch (rData.eFunc)
        {
            case SUBTOTAL_FUNC_SUM:
            case SUBTOTAL_FUNC_AVE:
                if (bVal)
                {
                    ++rData.nCount;
                    if (!SubTotal::SafePlus( rData.nVal, nValue ))
                        rData.bError = true;
                }
                break;
            case SUBTOTAL_FUNC_CNT:             // nur Werte
                if (bVal)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_CNT2:            // alle
                if (bCell)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_MAX:
                if (bVal)
                    if (++rData.nCount == 1 || nValue > rData.nVal )
                        rData.nVal = nValue;
                break;
            case SUBTOTAL_FUNC_MIN:
                if (bVal)
                    if (++rData.nCount == 1 || nValue < rData.nVal )
                        rData.nVal = nValue;
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }
}

//  Mehrfachselektion:
void ScColumn::UpdateSelectionFunction( const ScMarkData& rMark,
                                        ScFunctionData& rData,
                                        ScFlatBoolRowSegments& rHiddenRows,
                                        bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow )
{
    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, &rMark, false);
    while (aDataIter.Next( nIndex ))
    {
        SCROW nRow = pItems[nIndex].nRow;
        bool bRowHidden = rHiddenRows.getValue(nRow);
        if ( !bRowHidden )
            if ( !bDoExclude || nRow < nExStartRow || nRow > nExEndRow )
                lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
    }
}

//  bei bNoMarked die Mehrfachselektion weglassen
void ScColumn::UpdateAreaFunction( ScFunctionData& rData,
                                   ScFlatBoolRowSegments& rHiddenRows,
                                    SCROW nStartRow, SCROW nEndRow )
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex<nCount && pItems[nIndex].nRow<=nEndRow )
    {
        SCROW nRow = pItems[nIndex].nRow;
        bool bRowHidden = rHiddenRows.getValue(nRow);
        if ( !bRowHidden )
            lcl_UpdateSubTotal( rData, pItems[nIndex].pCell );
        ++nIndex;
    }
}

sal_uInt32 ScColumn::GetWeightedCount() const
{
    sal_uInt32 nTotal = 0;

    //  Notizen werden nicht gezaehlt

    for (SCSIZE i=0; i<nCount; i++)
    {
        ScBaseCell* pCell = pItems[i].pCell;
        switch ( pCell->GetCellType() )
        {
            case CELLTYPE_VALUE:
            case CELLTYPE_STRING:
                ++nTotal;
                break;
            case CELLTYPE_FORMULA:
                nTotal += 5 + ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
                break;
            case CELLTYPE_EDIT:
                nTotal += 50;
                break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

    return nTotal;
}

sal_uInt32 ScColumn::GetCodeCount() const
{
    sal_uInt32 nCodeCount = 0;

    for (SCSIZE i=0; i<nCount; i++)
    {
        ScBaseCell* pCell = pItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            nCodeCount += ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
    }

    return nCodeCount;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
