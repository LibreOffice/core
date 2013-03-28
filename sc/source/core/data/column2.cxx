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

#include <svx/algitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/emphasismarkitem.hxx>
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
#include "formulacell.hxx"
#include "document.hxx"
#include "docpool.hxx"
#include "attarray.hxx"
#include "patattr.hxx"
#include "cellform.hxx"
#include "stlsheet.hxx"
#include "rechead.hxx"
#include "brdcst.hxx"
#include "editutil.hxx"
#include "subtotal.hxx"
#include "markdata.hxx"
#include "compiler.hxx"         // ScTokenArray GetCodeLen
#include "dbdata.hxx"
#include "fillinfo.hxx"
#include "segmenttree.hxx"
#include "docparam.hxx"
#include "cellvalue.hxx"

#include <math.h>

#if DEBUG_COLUMN_STORAGE
#include "columniterator.hxx"
#include <iostream>
using std::cout;
using std::endl;
#endif

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
//  Data operations
//

// -----------------------------------------------------------------------------------------


long ScColumn::GetNeededSize(
    SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY,
    const Fraction& rZoomX, const Fraction& rZoomY,
    bool bWidth, const ScNeededSizeOptions& rOptions ) const
{
    long nValue=0;
    SCSIZE nIndex;
    double nPPT = bWidth ? nPPTX : nPPTY;
    if (Search(nRow,nIndex))
    {
        ScRefCellValue aCell;
        aCell.assign(*maItems[nIndex].pCell);

        const ScPatternAttr* pPattern = rOptions.pPattern;
        if (!pPattern)
            pPattern = pAttrArray->GetPattern( nRow );

        //      merged?
        //      Do not merge in conditional formatting

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

        //      conditional formatting
        const SfxItemSet* pCondSet = pDocument->GetCondResult( nCol, nRow, nTab );

        //  line break?

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

        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        sal_uLong nFormat = pPattern->GetNumberFormat( pFormatter, pCondSet );
        // #i111387# disable automatic line breaks only for "General" number format
        if (bBreak && aCell.hasNumeric() && ( nFormat % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 )
        {
            // also take formula result type into account for number format
            if (aCell.meType != CELLTYPE_FORMULA ||
                (aCell.mpFormula->GetStandardFormat(*pFormatter, nFormat) % SV_COUNTRY_LANGUAGE_OFFSET) == 0)
                bBreak = false;
        }

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
                    eRotMode = SVX_ROTATE_MODE_STANDARD;    // no overflow
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

        sal_uInt8 nScript = pDocument->GetScriptType(nCol, nRow, nTab);
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
        CellType eCellType = aCell.meType;

        bool bEditEngine = (eCellType == CELLTYPE_EDIT ||
                            eOrient == SVX_ORIENTATION_STACKED ||
                            IsAmbiguousScript(nScript) ||
                            ((eCellType == CELLTYPE_FORMULA) && aCell.mpFormula->IsMultilineResult()));

        if (!bEditEngine)                                   // direct output
        {
            Color* pColor;
            rtl::OUString aValStr;
            ScCellFormat::GetString(
                aCell, nFormat, aValStr, &pColor, *pFormatter, true, rOptions.bFormula, ftCheck);

            if (!aValStr.isEmpty())
            {
                //  SetFont is moved up

                Size aSize( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
                if ( eOrient != SVX_ORIENTATION_STANDARD )
                {
                    long nTemp = aSize.Width();
                    aSize.Width() = aSize.Height();
                    aSize.Height() = nTemp;
                }
                else if ( nRotate )
                {
                    //! take different X/Y scaling into consideration

                    double nRealOrient = nRotate * F_PI18000;   // nRotate is in 1/100 Grad
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
                        //  only to the right:
                        //! differ on direction up/down (only Text/whole height)
                        if ( pPattern->GetRotateDir( pCondSet ) == SC_ROTDIR_RIGHT )
                            nWidth += (long)( pDocument->GetRowHeight( nRow,nTab ) *
                                                nPPT * nCosAbs / nSinAbs );
                    }
                    else
                        nWidth  = (long)( aSize.Height() / nSinAbs );   //! limit?

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

                //  linebreak done ?

                if ( bBreak && !bWidth )
                {
                    //  test with EditEngine the safety at 90%
                    //  (due to rounding errors and because EditEngine formats partially differently)

                    long nDocPixel = (long) ( ( pDocument->GetColWidth( nCol,nTab ) -
                                        pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
                                        nIndent )
                                        * nPPT );
                    nDocPixel = (nDocPixel * 9) / 10;           // for safety
                    if ( aSize.Width() > nDocPixel )
                        bEditEngine = true;
                }
            }
        }

        if (bEditEngine)
        {
            //  the font is not reset each time with !bEditEngine
            Font aOldFont = pDev->GetFont();

            MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );

            // save in document ?
            ScFieldEditEngine* pEngine = pDocument->CreateFieldEditEngine();

            pEngine->SetUpdateMode( false );
            bool bTextWysiwyg = ( pDev->GetOutDevType() == OUTDEV_PRINTER );
            sal_uLong nCtrl = pEngine->GetControlWord();
            if ( bTextWysiwyg )
                nCtrl |= EE_CNTRL_FORMAT100;
            else
                nCtrl &= ~EE_CNTRL_FORMAT100;
            pEngine->SetControlWord( nCtrl );
            MapMode aOld = pDev->GetMapMode();
            pDev->SetMapMode( aHMMMode );
            pEngine->SetRefDevice( pDev );
            pDocument->ApplyAsianEditSettings( *pEngine );
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
                           + 1;     // output size is width-1 pixel (due to gridline)
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

            if (aCell.meType == CELLTYPE_EDIT)
            {
                pEngine->SetTextNewDefaults(*aCell.mpEditText, pSet);
            }
            else
            {
                Color* pColor;
                OUString aString;
                ScCellFormat::GetString(
                    aCell, nFormat, aString, &pColor, *pFormatter, true,
                    rOptions.bFormula, ftCheck);

                if (!aString.isEmpty())
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
                //! take different X/Y scaling into consideration

                Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
                double nRealOrient = nRotate * F_PI18000;   // nRotate is in 1/100 Grad
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
                    nWidth  = (long)( aSize.Height() / nSinAbs );   //! limit?
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
            else            // height
            {
                nValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ),
                                    aHMMMode).Height();

                // With non-100% zoom and several lines or paragraphs, don't shrink below the result with FORMAT100 set
                if ( !bTextWysiwyg && ( rZoomY.GetNumerator() != 1 || rZoomY.GetDenominator() != 1 ) &&
                     ( pEngine->GetParagraphCount() > 1 || ( bBreak && pEngine->GetLineCount(0) > 1 ) ) )
                {
                    pEngine->SetControlWord( nCtrl | EE_CNTRL_FORMAT100 );
                    pEngine->QuickFormatDoc( sal_True );
                    long nSecondValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ), aHMMMode).Height();
                    if ( nSecondValue > nValue )
                        nValue = nSecondValue;
                }
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
            //      place for Autofilter Button
            //      20 * nZoom/100
            //      Conditional formatting is not interesting here

            sal_Int16 nFlags = ((const ScMergeFlagAttr&)pPattern->GetItem(ATTR_MERGE_FLAG)).GetValue();
            if (nFlags & SC_MF_AUTO)
                nValue += (rZoomX.GetNumerator()*20)/rZoomX.GetDenominator();
        }
    }
    return nValue;
}


sal_uInt16 ScColumn::GetOptimalColWidth(
    OutputDevice* pDev, double nPPTX, double nPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
    bool bFormula, sal_uInt16 nOldWidth, const ScMarkData* pMarkData, const ScColWidthParam* pParam) const
{
    if ( maItems.empty() )
        return nOldWidth;

    sal_uInt16  nWidth = (sal_uInt16) (nOldWidth * nPPTX);
    bool    bFound = false;

    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, pMarkData, true);
    if ( pParam && pParam->mbSimpleText )
    {   // all the same except for number format
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
        rtl::OUString aLongStr;
        Color* pColor;
        if (pParam->mnMaxTextRow >= 0)
        {
            ScRefCellValue aCell = GetCellValue(pParam->mnMaxTextRow);
            ScCellFormat::GetString(
                aCell, nFormat, aLongStr, &pColor, *pFormatter, true, false, ftCheck);
        }
        else
        {
            xub_StrLen nLongLen = 0;
            while (aDataIter.Next(nIndex))
            {
                if (nIndex >= maItems.size())
                    // Out-of-bound reached.  No need to keep going.
                    break;

                ScRefCellValue aCell;
                aCell.assign(*maItems[nIndex].pCell);
                OUString aValStr;
                ScCellFormat::GetString(
                    aCell, nFormat, aValStr, &pColor, *pFormatter, true, false, ftCheck);

                if (aValStr.getLength() > nLongLen)
                {
                    nLongLen = aValStr.getLength();
                    aLongStr = aValStr;
                }
            }
        }

        if (!aLongStr.isEmpty())
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
            SCROW nRow = maItems[nIndex].nRow;

            sal_uInt8 nScript = pDocument->GetScriptType(nCol, nRow, nTab);
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

static sal_uInt16 lcl_GetAttribHeight( const ScPatternAttr& rPattern, sal_uInt16 nFontHeightId )
{
    sal_uInt16 nHeight = (sal_uInt16) ((const SvxFontHeightItem&) rPattern.GetItem(nFontHeightId)).GetHeight();
    const SvxMarginItem* pMargin = (const SvxMarginItem*) &rPattern.GetItem(ATTR_MARGIN);
    nHeight += nHeight / 5;
    //  for 10pt gives 240

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

    //  Standard height: TextHeight + margin - 23
    //  -> 257 for Windows

    if (nHeight > STD_ROWHEIGHT_DIFF)
        nHeight -= STD_ROWHEIGHT_DIFF;

    nHeight += pMargin->GetTopMargin() + pMargin->GetBottomMargin();

    return nHeight;
}

//  pHeight in Twips
//  optimize nMinHeight, nMinStart : with nRow >= nMinStart is at least nMinHeight
//  (is only evaluated with bStdAllowed)

void ScColumn::GetOptimalHeight(
    SCROW nStartRow, SCROW nEndRow, sal_uInt16* pHeight, OutputDevice* pDev,
    double nPPTX, double nPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
    bool bShrink, sal_uInt16 nMinHeight, SCROW nMinStart) const
{
    ScAttrIterator aIter( pAttrArray, nStartRow, nEndRow );

    SCROW nStart = -1;
    SCROW nEnd = -1;
    SCROW nEditPos = 0;
    SCROW nNextEnd = 0;

    //  with conditional formatting, always consider the individual cells

    const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
    while ( pPattern )
    {
        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
        if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
        {
            //  do nothing - vertically with merged and overlapping,
            //        horizontally only with overlapped (invisible) -
            //        only one horizontal merged is always considered
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

                // conditional formatting: loop all cells
                if (bStdOnly &&
                    !static_cast<const ScCondFormatItem&>(pPattern->GetItem(
                            ATTR_CONDITIONAL)).GetCondFormatData().empty())
                {
                    bStdOnly = false;
                }

                // rotated text: loop all cells
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
                        nEnd = nEditPos;                // calculate single
                        bStdAllowed = false;            // will be computed in any case per cell
                    }
                    else
                    {
                        nNextEnd = nEnd;
                        nEnd = nEditPos - 1;            // standard - part
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
                    while ( nIndex < maItems.size() && (nRow=maItems[nIndex].nRow) <= nEnd )
                    {
                        sal_uInt8 nScript = pDocument->GetScriptType(nCol, nRow, nTab);
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

            if (!bStdOnly)                      // search covered cells
            {
                ScNeededSizeOptions aOptions;

                SCSIZE nIndex;
                Search(nStart,nIndex);
                while ( (nIndex < maItems.size()) ? ((nRow=maItems[nIndex].nRow) <= nEnd) : false )
                {
                    //  only calculate the cell height when it's used later (#37928#)

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

    for (SCSIZE i=0; i<maItems.size(); i++)
        if ( maItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) maItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();
            //  no query on HasOnlineSpellErrors, this makes it also work after loading

            //  For the test on hard formatting (ScEditAttrTester), are the defaults in the
            //  EditEngine of no importance. When the tester would later recognise the same
            //  attributes in default and hard formatting and has to remove them, the correct
            //  defaults must be set in the EditEngine for each cell.

            //  test for attributes
            if ( !pEngine )
                pEngine = new ScTabEditEngine(pDocument);
            pEngine->SetText( *pData );
            ScEditAttrTester aTester( pEngine );
            if ( aTester.NeedsObject() )                    // only remove spelling errors
            {
                pOldCell->SetData(pEngine->CreateTextObject());
            }
            else                                            // create a string
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
                maItems[i].pCell = pNewCell;
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
    for (; i<maItems.size() && maItems[i].nRow <= nEndRow; i++)
        if ( maItems[i].pCell->GetCellType() == CELLTYPE_EDIT )
        {
            ScEditCell* pOldCell = (ScEditCell*) maItems[i].pCell;
            const EditTextObject* pData = pOldCell->GetData();

            //  For the test on hard formatting (ScEditAttrTester), are the defaults in the
            //  EditEngine of no importance. When the tester would later recognise the same
            //  attributes in default and hard formatting and has to remove them, the correct
            //  defaults must be set in the EditEngine for each cell.

            //  test for attributes
            if ( !pEngine )
            {
                pEngine = new ScFieldEditEngine(pDocument, pDocument->GetEditPool());
                //  EE_CNTRL_ONLINESPELLING if there are errors already
                pEngine->SetControlWord( pEngine->GetControlWord() | EE_CNTRL_ONLINESPELLING );
                pDocument->ApplyAsianEditSettings( *pEngine );
            }
            pEngine->SetText( *pData );
            sal_uInt16 nParCount = pEngine->GetParagraphCount();
            for (sal_uInt16 nPar=0; nPar<nParCount; nPar++)
            {
                pEngine->QuickRemoveCharAttribs( nPar );
                const SfxItemSet& rOld = pEngine->GetParaAttribs( nPar );
                if ( rOld.Count() )
                {
                    SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   // empty
                    pEngine->SetParaAttribs( nPar, aNew );
                }
            }
            //  change URL field to text (not possible otherwise, thus pType=0)
            pEngine->RemoveFields( true );

            bool bSpellErrors = pEngine->HasOnlineSpellErrors();
            bool bNeedObject = bSpellErrors || nParCount>1;         // keep errors/paragraphs
            //  ScEditAttrTester is not needed anymore, arrays are gone

            if ( bNeedObject )                                      // remains edit cell
            {
                sal_uInt32 nCtrl = pEngine->GetControlWord();
                sal_uInt32 nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
                if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                    pEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );
                pOldCell->SetData(pEngine->CreateTextObject());
            }
            else                                            // create String
            {
                String aText = ScEditUtil::GetSpaceDelimitedString( *pEngine );
                ScBaseCell* pNewCell = new ScStringCell( aText );
                pNewCell->TakeBroadcaster( pOldCell->ReleaseBroadcaster() );
                maItems[i].pCell = pNewCell;
                delete pOldCell;
            }
        }

    delete pEngine;
}

// =========================================================================================

bool ScColumn::TestTabRefAbs(SCTAB nTable) const
{
    bool bRet = false;
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
            if ( maItems[i].pCell->GetCellType() == CELLTYPE_FORMULA )
                if (((ScFormulaCell*)maItems[i].pCell)->TestTabRefAbs(nTable))
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
    if ( nPos < pColumn->maItems.size() )
    {
        rRow = pColumn->maItems[nPos].nRow;
        if ( rRow <= nBottom )
        {
            rpCell = pColumn->maItems[nPos].pCell;
            ++nPos;
            return true;
        }
    }

    rRow = 0;
    rpCell = NULL;
    return false;
}

SCSIZE ScColumnIterator::GetIndex() const           // Index of the last cell asked
{
    return nPos - 1;        // next time the position is incremented
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
                if (bAll)                   // complete column
                {
                    nTop    = 0;
                    nBottom = MAXROW;
                }
                else
                    return false;
            }
            pColumn->Search( nTop, nPos );
            bNext = false;
            bAll  = false;                  // only the first time
        }

        if ( nPos >= pColumn->maItems.size() )
            return false;

        if ( pColumn->maItems[nPos].nRow <= nBottom )
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
    return (maItems.empty());
}

bool ScColumn::IsEmptyVisData() const
{
    if ( maItems.empty() )
        return true;
    else
    {
        bool bVisData = false;
        SCSIZE i;
        for (i=0; i<maItems.size() && !bVisData; i++)
        {
            if(!maItems[i].pCell->IsBlank())
                bVisData = true;
        }
        return !bVisData;
    }
}

SCSIZE ScColumn::VisibleCount( SCROW nStartRow, SCROW nEndRow ) const
{
    //  Notes are not counted

    SCSIZE nVisCount = 0;
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < maItems.size() && maItems[nIndex].nRow <= nEndRow )
    {
        if ( maItems[nIndex].nRow >= nStartRow )
        {
            ++nVisCount;
        }
        ++nIndex;
    }
    return nVisCount;
}

SCROW ScColumn::GetLastVisDataPos() const
{
    SCROW nRet = 0;
    if ( !maItems.empty() )
    {
        SCSIZE i;
        bool bFound = false;
        for (i=maItems.size(); i>0 && !bFound; )
        {
            --i;
            if(!maItems[i].pCell->IsBlank())
            {
                bFound = true;
                nRet = maItems[i].nRow;
            }
        }
    }
    return nRet;
}

SCROW ScColumn::GetFirstVisDataPos() const
{
    SCROW nRet = 0;
    if ( !maItems.empty() )
    {
        SCSIZE i;
        bool bFound = false;
        for (i=0; i<maItems.size() && !bFound; i++)
        {
            bFound = true;
            nRet = maItems[i].nRow;
        }
    }
    return nRet;
}

bool ScColumn::HasVisibleDataAt(SCROW nRow) const
{
    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (!maItems[nIndex].pCell->IsBlank())
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

bool ScColumn::IsEmptyBlock(SCROW nStartRow, SCROW nEndRow) const
{
    if ( maItems.empty() )
        return true;

    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex < maItems.size() && maItems[nIndex].nRow <= nEndRow )
    {
        if ( !maItems[nIndex].pCell->IsBlank() )   // found a cell
            return false;                           // not empty
        ++nIndex;
    }
    return true;                                    // no cell found
}

SCSIZE ScColumn::GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const
{
    SCSIZE nLines = 0;
    SCSIZE i;
    if ( !maItems.empty() )
    {
        bool bFound = false;
        if (eDir == DIR_BOTTOM)
        {
            i = maItems.size();
            while (!bFound && (i > 0))
            {
                i--;
                if ( maItems[i].nRow < nStartRow )
                    break;
                bFound = maItems[i].nRow <= nEndRow && !maItems[i].pCell->IsBlank();
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(nEndRow - maItems[i].nRow);
            else
                nLines = static_cast<SCSIZE>(nEndRow - nStartRow);
        }
        else if (eDir == DIR_TOP)
        {
            i = 0;
            while (!bFound && (i < maItems.size()))
            {
                if ( maItems[i].nRow > nEndRow )
                    break;
                bFound = maItems[i].nRow >= nStartRow && !maItems[i].pCell->IsBlank();
                i++;
            }
            if (bFound)
                nLines = static_cast<SCSIZE>(maItems[i-1].nRow - nStartRow);
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
    if ( !maItems.empty() )
        return maItems[0].nRow;
    else
        return 0;
}

SCROW ScColumn::GetLastDataPos() const
{
    if ( !maItems.empty() )
        return maItems.back().nRow;
    else
        return 0;
}

bool ScColumn::GetPrevDataPos(SCROW& rRow) const
{
    bool bFound = false;
    SCSIZE i = maItems.size();
    while (!bFound && (i > 0))
    {
        --i;
        bFound = (maItems[i].nRow < rRow);
        if (bFound)
            rRow = maItems[i].nRow;
    }
    return bFound;
}

bool ScColumn::GetNextDataPos(SCROW& rRow) const        // greater than rRow
{
    SCSIZE nIndex;
    if (Search( rRow, nIndex ))
        ++nIndex;                   // next cell

    bool bMore = ( nIndex < maItems.size() );
    if ( bMore )
        rRow = maItems[nIndex].nRow;
    return bMore;
}

SCROW ScColumn::FindNextVisibleRow(SCROW nRow, bool bForward) const
{
    if(bForward)
    {
        nRow++;
        SCROW nEndRow = 0;
        bool bHidden = pDocument->RowHidden(nRow, nTab, NULL, &nEndRow);
        if(bHidden)
            return std::min<SCROW>(MAXROW, nEndRow + 1);
        else
            return nRow;
    }
    else
    {
        nRow--;
        SCROW nStartRow = MAXROW;
        bool bHidden = pDocument->RowHidden(nRow, nTab, &nStartRow, NULL);
        if(bHidden)
            return std::max<SCROW>(0, nStartRow - 1);
        else
            return nRow;
    }
}

SCROW ScColumn::FindNextVisibleRowWithContent(SCROW nRow, bool bForward) const
{
    if(bForward)
    {
        do
        {
            nRow++;
            SCROW nEndRow = 0;
            bool bHidden = pDocument->RowHidden(nRow, nTab, NULL, &nEndRow);
            if(bHidden)
            {
                nRow = nEndRow + 1;
                if(nRow >= MAXROW)
                    return MAXROW;
            }

            SCSIZE nIndex;
            bool bThere = Search( nRow, nIndex );
            if( bThere && !maItems[nIndex].pCell->IsBlank())
                return nRow;
            else if(nIndex >= maItems.size())
                return MAXROW;
            else
            {
                if(bThere)
                    nRow = maItems[nIndex+1].nRow - 1;
                else
                    nRow = maItems[nIndex].nRow - 1;
            }
        }
        while(nRow < MAXROW);

        return MAXROW;
    }
    else
    {
        do
        {
            nRow--;
            SCROW nStartRow = MAXROW;
            bool bHidden = pDocument->RowHidden(nRow, nTab, &nStartRow, NULL);
            if(bHidden)
            {
                nRow = nStartRow - 1;
                if(nRow <= 0)
                    return 0;
            }

            SCSIZE nIndex;
            bool bThere = Search( nRow, nIndex );
            if(bThere && !maItems[nIndex].pCell->IsBlank())
                return nRow;
            else if(nIndex == 0)
                return 0;
            else
                nRow = maItems[nIndex-1].nRow + 1;
        }
        while(nRow > 0);

        return 0;
    }
}

void ScColumn::CellStorageModified()
{
#if DEBUG_COLUMN_STORAGE
    if (maItems.empty())
    {
        if (maTextWidths.empty())
        {
            cout << "ScColumn::CellStorageModified: Text width array is empty, but shouldn't." << endl;
            cout.flush();
            abort();
        }

        if (maTextWidths.block_size() != 1 || maTextWidths.begin()->type != mdds::mtv::element_type_empty)
        {
            cout << "ScColumn::CellStorageModified: When the cell array is empty, the text with array should consist of one empty block." << endl;
            cout.flush();
            abort();
        }

        return;
    }

    cout << "-- begin" << endl;
    std::vector<ColEntry>::const_iterator it = maItems.begin(), itEnd = maItems.end();
    for (; it != itEnd; ++it)
        cout << "ScColumn::CellStorageModified: entry: row = " << it->nRow << "; cell = " << it->pCell << endl;

    ScColumnTextWidthIterator aIter(*this, 0, MAXROW);
    for (; aIter.hasCell(); aIter.next())
    {
        SCROW nRow = aIter.getPos();
        ScBaseCell* pCell = GetCell(nRow);
        cout << "ScColumn::CellStorageModified: row = " << nRow << "; cell = " << pCell << endl;
        if (!pCell)
        {
            cout << "ScColumn::CellStorageModified: Cell and text width storages are out of sync!" << endl;
            cout.flush();
            abort();
        }
    }
    cout << "-- end" << endl;
#endif
}

void ScColumn::CopyScriptTypesToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const
{
    rDestCol.maScriptTypes.set_empty(nRow1, nRow2); // Empty the destination range first.

    ScriptType::const_iterator itBlk = maScriptTypes.begin(), itBlkEnd = maScriptTypes.end();

    // Locate the top row position.
    size_t nOffsetInBlock = 0;
    size_t nBlockStart = 0, nBlockEnd = 0, nRowPos = static_cast<size_t>(nRow1);
    for (; itBlk != itBlkEnd; ++itBlk)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (nBlockStart <= nRowPos && nRowPos <= nBlockEnd)
        {
            // Found.
            nOffsetInBlock = nRowPos - nBlockStart;
            break;
        }
    }

    if (itBlk == itBlkEnd)
        // Specified range not found. Bail out.
        return;

    nRowPos = static_cast<size_t>(nRow2); // End row position.

    // Keep copying until we hit the end row position.
    mdds::mtv::ushort_element_block::const_iterator itData, itDataEnd;
    for (; itBlk != itBlkEnd; ++itBlk, nBlockStart = nBlockEnd, nOffsetInBlock = 0)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (!itBlk->data)
        {
            // Empty block.
            if (nBlockStart <= nRowPos && nRowPos <= nBlockEnd)
                // This block contains the end row.
                rDestCol.maScriptTypes.set_empty(nBlockStart + nOffsetInBlock, nRowPos);
            else
                rDestCol.maScriptTypes.set_empty(nBlockStart + nOffsetInBlock, nBlockEnd-1);

            continue;
        }

        // Non-empty block.
        itData = mdds::mtv::ushort_element_block::begin(*itBlk->data);
        itDataEnd = mdds::mtv::ushort_element_block::end(*itBlk->data);
        std::advance(itData, nOffsetInBlock);

        if (nBlockStart <= nRowPos && nRowPos <= nBlockEnd)
        {
            // This block contains the end row. Only copy partially.
            size_t nOffset = nRowPos - nBlockStart + 1;
            itDataEnd = mdds::mtv::ushort_element_block::begin(*itBlk->data);
            std::advance(itDataEnd, nOffset);

            rDestCol.maScriptTypes.set(nBlockStart + nOffsetInBlock, itData, itDataEnd);
            break;
        }

        rDestCol.maScriptTypes.set(nBlockStart + nOffsetInBlock, itData, itDataEnd);
    }
}

unsigned short ScColumn::GetTextWidth(SCROW nRow) const
{
    return maTextWidths.get<unsigned short>(nRow);
}

void ScColumn::SetTextWidth(SCROW nRow, unsigned short nWidth)
{
    maTextWidths.set(nRow, nWidth);
}

sal_uInt8 ScColumn::GetScriptType( SCROW nRow ) const
{
    if (!ValidRow(nRow) || maScriptTypes.is_empty(nRow))
        return 0;

    return maScriptTypes.get<unsigned short>(nRow);
}

void ScColumn::SetScriptType( SCROW nRow, sal_uInt8 nType )
{
    if (!ValidRow(nRow))
        return;

    if (!nType)
        maScriptTypes.set_empty(nRow, nRow);
    else
        maScriptTypes.set<unsigned short>(nRow, nType);
}

size_t ScColumn::GetFormulaHash( SCROW nRow ) const
{
    const ScFormulaCell* pCell = FetchFormulaCell(nRow);
    return pCell ? pCell->GetHash() : 0;
}

ScFormulaVectorState ScColumn::GetFormulaVectorState( SCROW nRow ) const
{
    const ScFormulaCell* pCell = FetchFormulaCell(nRow);
    return pCell ? pCell->GetVectorState() : FormulaVectorUnknown;
}

void ScColumn::SetNumberFormat( SCROW nRow, sal_uInt32 nNumberFormat )
{
    short eOldType = pDocument->GetFormatTable()->GetType(
        (sal_uLong)((SfxUInt32Item*)GetAttr(nRow, ATTR_VALUE_FORMAT))->GetValue());

    short eNewType = pDocument->GetFormatTable()->GetType(nNumberFormat);
    if (!pDocument->GetFormatTable()->IsCompatible(eOldType, eNewType))
        ApplyAttr(nRow, SfxUInt32Item(ATTR_VALUE_FORMAT, nNumberFormat));
}

const ScFormulaCell* ScColumn::FetchFormulaCell( SCROW nRow ) const
{
    if (!ValidRow(nRow))
        return NULL;

    SCSIZE nIndex;
    if (!Search(nRow, nIndex))
        // cell not found.
        return NULL;

    const ScBaseCell* pCell = maItems[nIndex].pCell;
    if (pCell->GetCellType() != CELLTYPE_FORMULA)
        // Not a formula cell.
        return NULL;

    return static_cast<const ScFormulaCell*>(pCell);
}

void ScColumn::FindDataAreaPos(SCROW& rRow, bool bDown) const
{
    // check if we are in a data area
    SCSIZE nIndex;
    bool bThere = Search(rRow, nIndex);
    if (bThere && maItems[nIndex].pCell->IsBlank())
        bThere = false;

    size_t nLastIndex = maItems.size() - 1;
    if (bThere)
    {
        SCROW nNextRow = FindNextVisibleRow(rRow, bDown);
        SCSIZE nNewIndex;
        bool bNextThere = Search(nNextRow, nNewIndex);
        if(bNextThere && maItems[nNewIndex].pCell->IsBlank())
            bNextThere = false;

        if(bNextThere)
        {
            SCROW nLastRow;
            nLastRow = nNextRow;
            do
            {
                nNextRow = FindNextVisibleRow(nLastRow, bDown);
                bNextThere = Search(nNextRow, nNewIndex);
                if(!bNextThere || maItems[nNewIndex].pCell->IsBlank())
                    bNextThere = false;
                else
                    nLastRow = nNextRow;
            }
            while(bNextThere && nNewIndex < nLastIndex && nNewIndex > 0);

            rRow = nLastRow;
        }
        else
        {
            rRow = FindNextVisibleRowWithContent(nNextRow, bDown);
        }
    }
    else
    {
        rRow = FindNextVisibleRowWithContent(rRow, bDown);
    }
}

bool ScColumn::HasDataAt(SCROW nRow) const
{
        //  are only visible cells interesting ?
        //! then HasVisibleDataAt out

    SCSIZE nIndex;
    if (Search(nRow, nIndex))
        if (!maItems[nIndex].pCell->IsBlank())
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

bool ScColumn::GetLastVisibleAttr( SCROW& rLastRow, bool bFullFormattedArea ) const
{
    if (pAttrArray)
    {
        // row of last cell is needed
        SCROW nLastData = GetLastVisDataPos();    // always including notes, 0 if none

        return pAttrArray->GetLastVisibleAttr( rLastRow, nLastData, bFullFormattedArea );
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
    while ( (nIndex < maItems.size()) ? ((nRow=maItems[nIndex].nRow) <= nEndRow) : false )
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
        pCell = maItems[nIndex].pCell;
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
        pCell = maItems[nIndex].pCell;
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
        ScBaseCell* pCell = maItems[nIndex].pCell;
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
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula();
        }
}

void ScColumn::CompileDBFormula( bool bCreateFormulaString )
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileDBFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileNameFormula( bool bCreateFormulaString )
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileNameFormula( bCreateFormulaString );
        }
}

void ScColumn::CompileColRowNameFormula()
{
    if ( !maItems.empty() )
        for (SCSIZE i = 0; i < maItems.size(); i++)
        {
            ScBaseCell* pCell = maItems[i].pCell;
            if ( pCell->GetCellType() == CELLTYPE_FORMULA )
                ((ScFormulaCell*) pCell)->CompileColRowNameFormula();
        }
}

static void lcl_UpdateSubTotal( ScFunctionData& rData, const ScBaseCell* pCell )
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
                if ( rData.eFunc != SUBTOTAL_FUNC_CNT2 )        // it doesn't interest us
                {
                    ScFormulaCell* pFC = (ScFormulaCell*)pCell;
                    if ( pFC->GetErrCode() )
                    {
                        if ( rData.eFunc != SUBTOTAL_FUNC_CNT ) // simply remove from count
                            rData.bError = true;
                    }
                    else if (pFC->IsValue())
                    {
                        nValue = pFC->GetValue();
                        bVal = true;
                    }
                    // otherwise text
                }
            }
            break;
        case CELLTYPE_NOTE:
            bCell = false;
            break;
        // nothing for strings
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
            case SUBTOTAL_FUNC_CNT:             // only the value
                if (bVal)
                    ++rData.nCount;
                break;
            case SUBTOTAL_FUNC_CNT2:            // everything
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

//  multiple selections:
void ScColumn::UpdateSelectionFunction(
    const ScMarkData& rMark, ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows,
    bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow) const
{
    SCSIZE nIndex;
    ScMarkedDataIter aDataIter(this, &rMark, false);
    while (aDataIter.Next( nIndex ))
    {
        SCROW nRow = maItems[nIndex].nRow;
        bool bRowHidden = rHiddenRows.getValue(nRow);
        if ( !bRowHidden )
            if ( !bDoExclude || nRow < nExStartRow || nRow > nExEndRow )
                lcl_UpdateSubTotal( rData, maItems[nIndex].pCell );
    }
}

//  with bNoMarked ignore the multiple selections
void ScColumn::UpdateAreaFunction(
    ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows, SCROW nStartRow, SCROW nEndRow) const
{
    SCSIZE nIndex;
    Search( nStartRow, nIndex );
    while ( nIndex<maItems.size() && maItems[nIndex].nRow<=nEndRow )
    {
        SCROW nRow = maItems[nIndex].nRow;
        bool bRowHidden = rHiddenRows.getValue(nRow);
        if ( !bRowHidden )
            lcl_UpdateSubTotal( rData, maItems[nIndex].pCell );
        ++nIndex;
    }
}

sal_uInt32 ScColumn::GetWeightedCount() const
{
    sal_uInt32 nTotal = 0;

    //  Notes are not counted

    for (SCSIZE i=0; i<maItems.size(); i++)
    {
        ScBaseCell* pCell = maItems[i].pCell;
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

    for (SCSIZE i=0; i<maItems.size(); i++)
    {
        ScBaseCell* pCell = maItems[i].pCell;
        if ( pCell->GetCellType() == CELLTYPE_FORMULA )
            nCodeCount += ((ScFormulaCell*)pCell)->GetCode()->GetCodeLen();
    }

    return nCodeCount;
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
