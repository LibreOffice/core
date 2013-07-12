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

#include "column.hxx"
#include "scitems.hxx"
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
#include "tokenarray.hxx"
#include "globalnames.hxx"
#include "formulagroup.hxx"
#include "listenercontext.hxx"
#include "mtvcellfunc.hxx"
#include "scmatrix.hxx"

#include <math.h>

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
#include "formula/errorcodes.hxx"

#include <boost/scoped_ptr.hpp>

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
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end() || it->type == sc::element_type_empty)
        // Empty cell, or invalid row.
        return 0;

    long nValue = 0;
    ScRefCellValue aCell = GetCellValue(it, aPos.second);
    double nPPT = bWidth ? nPPTX : nPPTY;

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
        OUString aValStr;
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
    return nValue;
}

namespace {

class MaxStrLenFinder
{
    ScDocument& mrDoc;
    sal_uInt32 mnFormat;
    OUString maMaxLenStr;
    sal_Int32 mnMaxLen;

    void checkLength(ScRefCellValue& rCell)
    {
        Color* pColor;
        OUString aValStr;
        ScCellFormat::GetString(
            rCell, mnFormat, aValStr, &pColor, *mrDoc.GetFormatTable(), true, false, ftCheck);

        if (aValStr.getLength() > mnMaxLen)
        {
            mnMaxLen = aValStr.getLength();
            maMaxLenStr = aValStr;
        }
    }

public:
    MaxStrLenFinder(ScDocument& rDoc, sal_uInt32 nFormat) :
        mrDoc(rDoc), mnFormat(nFormat), mnMaxLen(0) {}

    void operator() (size_t /*nRow*/, double f)
    {
        ScRefCellValue aCell(f);
        checkLength(aCell);
    }

    void operator() (size_t /*nRow*/, const OUString& rStr)
    {
        ScRefCellValue aCell(&rStr);
        checkLength(aCell);
    }

    void operator() (size_t /*nRow*/, const EditTextObject* p)
    {
        ScRefCellValue aCell(p);
        checkLength(aCell);
    }

    void operator() (size_t /*nRow*/, const ScFormulaCell* p)
    {
        ScRefCellValue aCell(const_cast<ScFormulaCell*>(p));
        checkLength(aCell);
    }
};

}

sal_uInt16 ScColumn::GetOptimalColWidth(
    OutputDevice* pDev, double nPPTX, double nPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
    bool bFormula, sal_uInt16 nOldWidth, const ScMarkData* pMarkData, const ScColWidthParam* pParam) const
{
    if (maCells.block_size() == 1 && maCells.begin()->type == sc::element_type_empty)
        // All cells are empty.
        return nOldWidth;

    sc::SingleColumnSpanSet aSpanSet;
    sc::SingleColumnSpanSet::SpansType aMarkedSpans;
    if (pMarkData)
    {
        aSpanSet.scan(*pMarkData, nTab, nCol);
        aSpanSet.getSpans(aMarkedSpans);
    }
    else
        // "Select" the entire column if no selection exists.
        aMarkedSpans.push_back(sc::SingleColumnSpanSet::Span(0, MAXROW));

    sal_uInt16 nWidth = static_cast<sal_uInt16>(nOldWidth*nPPTX);
    bool bFound = false;

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
        OUString aLongStr;
        Color* pColor;
        if (pParam->mnMaxTextRow >= 0)
        {
            ScRefCellValue aCell = GetCellValue(pParam->mnMaxTextRow);
            ScCellFormat::GetString(
                aCell, nFormat, aLongStr, &pColor, *pFormatter, true, false, ftCheck);
        }
        else
        {
            // Go though all non-empty cells within selection.
            MaxStrLenFinder aFunc(*pDocument, nFormat);
            sc::CellStoreType::const_iterator itPos = maCells.begin();
            sc::SingleColumnSpanSet::SpansType::const_iterator it = aMarkedSpans.begin(), itEnd = aMarkedSpans.end();
            for (; it != itEnd; ++it)
                itPos = sc::ParseAllNonEmpty(itPos, maCells, it->mnRow1, it->mnRow2, aFunc);
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

        // Go though all non-empty cells within selection.
        sc::CellStoreType::const_iterator itPos = maCells.begin();
        sc::SingleColumnSpanSet::SpansType::const_iterator it = aMarkedSpans.begin(), itEnd = aMarkedSpans.end();
        for (; it != itEnd; ++it)
        {
            SCROW nRow1 = it->mnRow1, nRow2 = it->mnRow2;
            SCROW nRow = nRow1;
            while (nRow <= nRow2)
            {
                std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(itPos, nRow);
                itPos = aPos.first;
                if (itPos->type == sc::element_type_empty)
                {
                    // Skip empty cells.
                    nRow += itPos->size - aPos.second;
                    continue;
                }

                for (size_t nOffset = aPos.second; nOffset < itPos->size; ++nOffset, ++nRow)
                {
                    sal_uInt8 nScript = pDocument->GetScriptType(nCol, nRow, nTab);
                    if (nScript == 0)
                        nScript = ScGlobal::GetDefaultScriptType();

                    const ScPatternAttr* pPattern = GetPattern(nRow);
                    aOptions.pPattern = pPattern;
                    aOptions.bGetFont = (pPattern != pOldPattern || nScript != nOldScript);
                    sal_uInt16 nThis = (sal_uInt16) GetNeededSize(
                        nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, true, aOptions);
                    pOldPattern = pPattern;
                    if (nThis)
                    {
                        if (nThis > nWidth || !bFound)
                        {
                            nWidth = nThis;
                            bFound = true;
                        }
                    }
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
    bool bShrink, sal_uInt16 nMinHeight, SCROW nMinStart)
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

            sc::SingleColumnSpanSet aSpanSet;
            aSpanSet.scan(*this, nStart, nEnd);
            sc::SingleColumnSpanSet::SpansType aSpans;
            aSpanSet.getSpans(aSpans);

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

                for (SCROW nRow = nStart; nRow <= nStdEnd; ++nRow)
                    if (nDefHeight > pHeight[nRow-nStartRow])
                        pHeight[nRow-nStartRow] = nDefHeight;

                if ( bStdOnly )
                {
                    //  if cells are not handled individually below,
                    //  check for cells with different script type
                    sc::CellTextAttrStoreType::iterator itAttr = maCellTextAttrs.begin();
                    sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
                    for (; it != itEnd; ++it)
                    {
                        for (SCROW nRow = it->mnRow1; nRow <= it->mnRow2; ++nRow)
                        {
                            sal_uInt8 nScript = GetRangeScriptType(itAttr, nRow, nRow);
                            if (nScript == nDefScript)
                                continue;

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
                    }
                }
            }

            if (!bStdOnly)                      // search covered cells
            {
                ScNeededSizeOptions aOptions;

                sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
                for (; it != itEnd; ++it)
                {
                    for (SCROW nRow = it->mnRow1; nRow <= it->mnRow2; ++nRow)
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
                    }
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
    sc::CellStoreType::const_iterator it = maCells.position(nRow).first;
    mdds::mtv::element_t eType = it->type;
    if (!bInSel && it != maCells.end() && eType != sc::element_type_empty)
    {
        if ( (eType == sc::element_type_string || eType == sc::element_type_edittext) &&
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
                it = maCells.position(it, nRow).first;
                eType = it->type;
                if ( (eType == sc::element_type_string || eType == sc::element_type_edittext) &&
                     !(HasAttrib( nRow, nRow, HASATTR_PROTECTED) &&
                       pDocument->IsTabProtected(nTab)) )
                    return true;
                else
                    nRow++;
            }
        }
        else if (GetNextDataPos(nRow))
        {
            it = maCells.position(it, nRow).first;
            eType = it->type;
            if ( (eType == sc::element_type_string || eType == sc::element_type_edittext) &&
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

namespace {

class StrEntries
{
    sc::CellStoreType& mrCells;

protected:
    struct StrEntry
    {
        SCROW mnRow;
        OUString maStr;

        StrEntry(SCROW nRow, const OUString& rStr) : mnRow(nRow), maStr(rStr) {}
    };

    std::vector<StrEntry> maStrEntries;

    StrEntries(sc::CellStoreType& rCells) : mrCells(rCells) {}

public:
    void commitStrings()
    {
        sc::CellStoreType::iterator it = mrCells.begin();
        std::vector<StrEntry>::iterator itStr = maStrEntries.begin(), itStrEnd = maStrEntries.end();
        for (; itStr != itStrEnd; ++itStr)
            it = mrCells.set(it, itStr->mnRow, itStr->maStr);
    }
};

class RemoveAutoSpellObjHandler : public StrEntries
{
    ScDocument* mpDoc;
    boost::scoped_ptr<ScTabEditEngine> mpEngine;

public:
    RemoveAutoSpellObjHandler(sc::CellStoreType& rCells, ScDocument* pDoc) : StrEntries(rCells), mpDoc(pDoc) {}

    void operator() (size_t nRow, EditTextObject*& pObj)
    {
        //  no query on HasOnlineSpellErrors, this makes it also work after loading

        //  For the test on hard formatting (ScEditAttrTester), are the defaults in the
        //  EditEngine of no importance. When the tester would later recognise the same
        //  attributes in default and hard formatting and has to remove them, the correct
        //  defaults must be set in the EditEngine for each cell.

        //  test for attributes
        if (!mpEngine)
            mpEngine.reset(new ScTabEditEngine(mpDoc));

        mpEngine->SetText(*pObj);

        ScEditAttrTester aTester(mpEngine.get());
        if (aTester.NeedsObject())                    // only remove spelling errors
        {
            // Overwrite the existing object.
            delete pObj;
            pObj = mpEngine->CreateTextObject();
        }
        else
        {
            // Store the string replacement for later commits.
            OUString aText = ScEditUtil::GetSpaceDelimitedString(*mpEngine);
            maStrEntries.push_back(StrEntry(nRow, aText));
        }
    }
};

class RemoveEditAttribsHandler : public StrEntries
{
    ScDocument* mpDoc;
    boost::scoped_ptr<ScFieldEditEngine> mpEngine;

public:
    RemoveEditAttribsHandler(sc::CellStoreType& rCells, ScDocument* pDoc) : StrEntries(rCells), mpDoc(pDoc) {}

    void operator() (size_t nRow, EditTextObject*& pObj)
    {
        //  For the test on hard formatting (ScEditAttrTester), are the defaults in the
        //  EditEngine of no importance. When the tester would later recognise the same
        //  attributes in default and hard formatting and has to remove them, the correct
        //  defaults must be set in the EditEngine for each cell.

        //  test for attributes
        if (!mpEngine)
        {
            mpEngine.reset(new ScFieldEditEngine(mpDoc, mpDoc->GetEditPool()));
            //  EE_CNTRL_ONLINESPELLING if there are errors already
            mpEngine->SetControlWord(mpEngine->GetControlWord() | EE_CNTRL_ONLINESPELLING);
            mpDoc->ApplyAsianEditSettings(*mpEngine);
        }
        mpEngine->SetText(*pObj);
        sal_Int32 nParCount = mpEngine->GetParagraphCount();
        for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
        {
            mpEngine->QuickRemoveCharAttribs(nPar);
            const SfxItemSet& rOld = mpEngine->GetParaAttribs(nPar);
            if ( rOld.Count() )
            {
                SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   // empty
                mpEngine->SetParaAttribs( nPar, aNew );
            }
        }
        //  change URL field to text (not possible otherwise, thus pType=0)
        mpEngine->RemoveFields(true);

        bool bSpellErrors = mpEngine->HasOnlineSpellErrors();
        bool bNeedObject = bSpellErrors || nParCount>1;         // keep errors/paragraphs
        //  ScEditAttrTester is not needed anymore, arrays are gone

        if (bNeedObject)                                      // remains edit cell
        {
            sal_uInt32 nCtrl = mpEngine->GetControlWord();
            sal_uInt32 nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
            if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                mpEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );

            // Overwrite the existing object.
            delete pObj;
            pObj = mpEngine->CreateTextObject();
        }
        else                                            // create String
        {
            // Store the string replacement for later commits.
            OUString aText = ScEditUtil::GetSpaceDelimitedString(*mpEngine);
            maStrEntries.push_back(StrEntry(nRow, aText));
        }
    }
};

class TestTabRefAbsHandler
{
    SCTAB mnTab;
    bool mbTestResult;
public:
    TestTabRefAbsHandler(SCTAB nTab) : mnTab(nTab), mbTestResult(false) {}

    void operator() (size_t /*nRow*/, const ScFormulaCell* pCell)
    {
        if (const_cast<ScFormulaCell*>(pCell)->TestTabRefAbs(mnTab))
            mbTestResult = true;
    }

    bool getTestResult() const { return mbTestResult; }
};

}

void ScColumn::RemoveAutoSpellObj()
{
    RemoveAutoSpellObjHandler aFunc(maCells, pDocument);
    sc::ProcessEditText(maCells, aFunc);
    aFunc.commitStrings();
}

void ScColumn::RemoveEditAttribs( SCROW nStartRow, SCROW nEndRow )
{
    RemoveEditAttribsHandler aFunc(maCells, pDocument);
    sc::ProcessEditText(maCells.begin(), maCells, nStartRow, nEndRow, aFunc);
    aFunc.commitStrings();
}

bool ScColumn::TestTabRefAbs(SCTAB nTable) const
{
    TestTabRefAbsHandler aFunc(nTable);
    sc::ParseFormula(maCells, aFunc);
    return aFunc.getTestResult();
}

bool ScColumn::IsEmptyData() const
{
    return maCells.block_size() == 1 && maCells.begin()->type == sc::element_type_empty;
}

namespace {

class CellCounter
{
    size_t mnCount;
public:
    CellCounter() : mnCount(0) {}

    void operator() (
        const sc::CellStoreType::value_type& node, size_t /*nOffset*/, size_t nDataSize)
    {
        if (node.type == sc::element_type_empty)
            return;

        mnCount += nDataSize;
    }

    size_t getCount() const { return mnCount; }
};

}

SCSIZE ScColumn::VisibleCount( SCROW nStartRow, SCROW nEndRow ) const
{
    CellCounter aFunc;
    sc::ParseBlock(maCells.begin(), maCells, aFunc, nStartRow, nEndRow);
    return aFunc.getCount();
}

bool ScColumn::HasVisibleDataAt(SCROW nRow) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        // Likely invalid row number.
        return false;

    return it->type != sc::element_type_empty;
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
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nStartRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        // Invalid row number.
        return false;

    if (it->type != sc::element_type_empty)
        // Non-empty cell at the start position.
        return false;

    // start position of next block which is not empty.
    SCROW nNextRow = nStartRow + it->size - aPos.second;
    return nEndRow < nNextRow;
}

SCSIZE ScColumn::GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const
{
    // Given a range of rows, find a top or bottom empty segment.
    switch (eDir)
    {
        case DIR_TOP:
        {
            // Determine the length of empty head segment.
            size_t nLength = nEndRow - nStartRow + 1;
            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nStartRow);
            sc::CellStoreType::const_iterator it = aPos.first;
            if (it->type != sc::element_type_empty)
                // First row is already not empty.
                return 0;

            // length of this empty block minus the offset.
            size_t nThisLen = it->size - aPos.second;
            return std::min(nThisLen, nLength);
        }
        break;
        case DIR_BOTTOM:
        {
            // Determine the length empty tail segment.
            size_t nLength = nEndRow - nStartRow + 1;
            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nEndRow);
            sc::CellStoreType::const_iterator it = aPos.first;
            if (it->type != sc::element_type_empty)
                // end row is already not empty.
                return 0;

            // length of this empty block from the tip to the end row position.
            size_t nThisLen = aPos.second;
            return std::min(nThisLen, nLength);
        }
        break;
        default:
            ;
    }

    return 0;
}

SCROW ScColumn::GetFirstDataPos() const
{
    if (IsEmptyData())
        return 0;

    sc::CellStoreType::const_iterator it = maCells.begin();
    if (it->type != sc::element_type_empty)
        return 0;

    return it->size;
}

SCROW ScColumn::GetLastDataPos() const
{
    if (IsEmptyData())
        return 0;

    sc::CellStoreType::const_reverse_iterator it = maCells.rbegin();
    if (it->type != sc::element_type_empty)
        return MAXROW;

    return MAXROW - static_cast<SCROW>(it->size);
}

bool ScColumn::GetPrevDataPos(SCROW& rRow) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return false;

    if (it->type == sc::element_type_empty)
    {
        if (it == maCells.begin())
            // No more previous non-empty cell.
            return false;

        rRow -= aPos.second + 1; // Last row position of the previous block.
        return true;
    }

    // This block is not empty.
    if (aPos.second)
    {
        // There are preceding cells in this block. Simply move back one cell.
        --rRow;
        return true;
    }

    // This is the first cell in an non-empty block. Move back to the previous block.
    if (it == maCells.begin())
        // No more preceding block.
        return false;

    --rRow; // Move to the last cell of the previous block.
    --it;
    if (it->type == sc::element_type_empty)
    {
        // This block is empty.
        if (it == maCells.begin())
            // No more preceding blocks.
            return false;

        // Skip the whole empty block segment.
        rRow -= it->size;
    }

    return true;
}

bool ScColumn::GetNextDataPos(SCROW& rRow) const        // greater than rRow
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return false;

    if (it->type == sc::element_type_empty)
    {
        // This block is empty. Skip ahead to the next block (if exists).
        rRow += it->size - aPos.second;
        ++it;
        if (it == maCells.end())
            // No more next block.
            return false;

        // Next block exists, and is non-empty.
        return true;
    }

    if (aPos.second < it->size - 1)
    {
        // There are still cells following the current position.
        ++rRow;
        return true;
    }

    // This is the last cell in the block. Move ahead to the next block.
    rRow += it->size - aPos.second; // First cell in the next block.
    ++it;
    if (it == maCells.end())
        // No more next block.
        return false;

    if (it->type == sc::element_type_empty)
    {
        // Next block is empty. Move to the next block.
        rRow += it->size;
        ++it;
        if (it == maCells.end())
            return false;
    }

    return true;
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

SCROW ScColumn::FindNextVisibleRowWithContent(
    sc::CellStoreType::const_iterator& itPos, SCROW nRow, bool bForward) const
{
    if (bForward)
    {
        do
        {
            nRow++;
            SCROW nEndRow = 0;
            bool bHidden = pDocument->RowHidden(nRow, nTab, NULL, &nEndRow);
            if (bHidden)
            {
                nRow = nEndRow + 1;
                if(nRow >= MAXROW)
                    return MAXROW;
            }

            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(itPos, nRow);
            itPos = aPos.first;
            if (itPos == maCells.end())
                // Invalid row.
                return MAXROW;

            if (itPos->type != sc::element_type_empty)
                return nRow;

            // Move to the last cell of the current empty block.
            nRow += itPos->size - aPos.second - 1;
        }
        while (nRow < MAXROW);

        return MAXROW;
    }

    do
    {
        nRow--;
        SCROW nStartRow = MAXROW;
        bool bHidden = pDocument->RowHidden(nRow, nTab, &nStartRow, NULL);
        if (bHidden)
        {
            nRow = nStartRow - 1;
            if(nRow <= 0)
                return 0;
        }

        std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(itPos, nRow);
        itPos = aPos.first;
        if (itPos == maCells.end())
            // Invalid row.
            return 0;

        if (itPos->type != sc::element_type_empty)
            return nRow;

        // Move to the first cell of the current empty block.
        nRow -= aPos.second;
    }
    while (nRow > 0);

    return 0;
}

void ScColumn::CellStorageModified()
{
    // TODO: Update column's "last updated" timestamp here.

    mbDirtyGroups = true;

#if DEBUG_COLUMN_STORAGE
    if (maCells.size() != MAXROWCOUNT)
    {
        cout << "ScColumn::CellStorageModified: Size of the cell array is incorrect." << endl;
        cout.flush();
        abort();
    }

    if (maCellTextAttrs.size() != MAXROWCOUNT)
    {
        cout << "ScColumn::CellStorageModified: Size of the cell text attribute array is incorrect." << endl;
        cout.flush();
        abort();
    }

    if (maBroadcasters.size() != MAXROWCOUNT)
    {
        cout << "ScColumn::CellStorageModified: Size of the broadcaster array is incorrect." << endl;
        cout.flush();
        abort();
    }

    // Make sure that these two containers are synchronized wrt empty segments.
    sc::CellStoreType::const_iterator itCell = maCells.begin();
    sc::CellTextAttrStoreType::const_iterator itAttr = maCellTextAttrs.begin();

    // Move to the first empty blocks.
    while (itCell != maCells.end() && itCell->type != sc::element_type_empty)
        ++itCell;

    while (itAttr != maCellTextAttrs.end() && itAttr->type != sc::element_type_empty)
        ++itAttr;

    while (itCell != maCells.end())
    {
        if (itCell->position != itAttr->position || itCell->size != itAttr->size)
        {
            cout << "ScColumn::CellStorageModified: Cell array and cell text attribute array are out of sync." << endl;
            cout << "-- cell array" << endl;
            maCells.dump_blocks(cout);
            cout << "-- attribute array" << endl;
            maCellTextAttrs.dump_blocks(cout);
            cout.flush();
            abort();
        }

        // Move to the next empty blocks.
        ++itCell;
        while (itCell != maCells.end() && itCell->type != sc::element_type_empty)
            ++itCell;

        ++itAttr;
        while (itAttr != maCellTextAttrs.end() && itAttr->type != sc::element_type_empty)
            ++itAttr;
    }
#endif
}

#if DEBUG_COLUMN_STORAGE

namespace {

struct FormulaGroupDumper : std::unary_function<sc::CellStoreType::value_type, void>
{
    void operator() (const sc::CellStoreType::value_type& rNode) const
    {
        if (rNode.type != sc::element_type_formula)
            return;

        sc::formula_block::const_iterator it = sc::formula_block::begin(*rNode.data);
        sc::formula_block::const_iterator itEnd = sc::formula_block::end(*rNode.data);

        for (; it != itEnd; ++it)
        {
            const ScFormulaCell& rCell = **it;
            if (!rCell.IsShared())
                continue;

            if (rCell.GetSharedTopRow() != rCell.aPos.Row())
                continue;

            SCROW nLen = rCell.GetSharedLength();
            cout << "  * group: start=" << rCell.aPos.Row() << ", length=" << nLen << endl;
            std::advance(it, nLen-1);
        }
    }
};

}

void ScColumn::DumpFormulaGroups() const
{
    cout << "-- formua groups" << endl;
    std::for_each(maCells.begin(), maCells.end(), FormulaGroupDumper());
    cout << "--" << endl;
}
#endif

void ScColumn::CopyCellTextAttrsToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const
{
    rDestCol.maCellTextAttrs.set_empty(nRow1, nRow2); // Empty the destination range first.

    sc::CellTextAttrStoreType::const_iterator itBlk = maCellTextAttrs.begin(), itBlkEnd = maCellTextAttrs.end();

    // Locate the top row position.
    size_t nOffsetInBlock = 0;
    size_t nBlockStart = 0, nBlockEnd = 0, nRowPos = static_cast<size_t>(nRow1);
    for (; itBlk != itBlkEnd; ++itBlk)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
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
    sc::celltextattr_block::const_iterator itData, itDataEnd;
    for (; itBlk != itBlkEnd; ++itBlk, nBlockStart = nBlockEnd, nOffsetInBlock = 0)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (!itBlk->data)
        {
            // Empty block.
            if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
                // This block contains the end row.
                rDestCol.maCellTextAttrs.set_empty(nBlockStart + nOffsetInBlock, nRowPos);
            else
                rDestCol.maCellTextAttrs.set_empty(nBlockStart + nOffsetInBlock, nBlockEnd-1);

            continue;
        }

        // Non-empty block.
        itData = sc::celltextattr_block::begin(*itBlk->data);
        itDataEnd = sc::celltextattr_block::end(*itBlk->data);
        std::advance(itData, nOffsetInBlock);

        if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
        {
            // This block contains the end row. Only copy partially.
            size_t nOffset = nRowPos - nBlockStart + 1;
            itDataEnd = sc::celltextattr_block::begin(*itBlk->data);
            std::advance(itDataEnd, nOffset);

            rDestCol.maCellTextAttrs.set(nBlockStart + nOffsetInBlock, itData, itDataEnd);
            break;
        }

        rDestCol.maCellTextAttrs.set(nBlockStart + nOffsetInBlock, itData, itDataEnd);
    }
}

namespace {

class CellTextAttrInitializer
{
    sc::CellTextAttrStoreType maAttrs;
    sc::CellTextAttrStoreType::iterator miPos;
public:
    CellTextAttrInitializer() : maAttrs(MAXROWCOUNT), miPos(maAttrs.begin()) {}

    void operator() (const sc::CellStoreType::value_type& node)
    {
        if (node.type == sc::element_type_empty)
            return;

        // Fill with default values for non-empty cell segments.
        std::vector<sc::CellTextAttr> aDefaults(node.size);
        miPos = maAttrs.set(miPos, node.position, aDefaults.begin(), aDefaults.end());
    }

    void swap(sc::CellTextAttrStoreType& rAttrs)
    {
        maAttrs.swap(rAttrs);
    }
};

}

void ScColumn::ResetCellTextAttrs()
{
    CellTextAttrInitializer aFunc;
    std::for_each(maCells.begin(), maCells.end(), aFunc);
    aFunc.swap(maCellTextAttrs);
    CellStorageModified();
}

void ScColumn::SwapCellTextAttrs( SCROW nRow1, SCROW nRow2 )
{
    typedef std::pair<sc::CellTextAttrStoreType::iterator,size_t> PosType;

    if (nRow1 == nRow2)
        return;

    if (nRow1 > nRow2)
        std::swap(nRow1, nRow2);

    PosType aPos1 = maCellTextAttrs.position(nRow1);
    if (aPos1.first == maCellTextAttrs.end())
        return;

    PosType aPos2 = maCellTextAttrs.position(aPos1.first, nRow2);
    if (aPos2.first == maCellTextAttrs.end())
        return;

    sc::CellTextAttrStoreType::iterator it1 = aPos1.first, it2 = aPos2.first;
    if (it1->type == it2->type)
    {
        if (it1->type == sc::element_type_empty)
            // Both are empty. Nothing to swap.
            return;

        // Both are non-empty. Simply swap their values.
        std::swap(
            sc::celltextattr_block::at(*it1->data, aPos1.second),
            sc::celltextattr_block::at(*it2->data, aPos2.second));

        return;
    }

    // One is empty while the other isn't.
    if (it1->type == sc::element_type_empty)
    {
        // row 1 is empty while row 2 is non-empty.
        const sc::CellTextAttr& rVal2 = sc::celltextattr_block::at(*it2->data, aPos2.second);
        it1 = maCellTextAttrs.set(it1, nRow1, rVal2);
        maCellTextAttrs.set_empty(it1, nRow2, nRow2);
        return;
    }

    // row 1 is non-empty while row 2 is empty.
    sc::CellTextAttr aVal1 = sc::celltextattr_block::at(*it1->data, aPos1.second); // make a copy.
    it1 = maCellTextAttrs.set_empty(it1, nRow1, nRow1);
    maCellTextAttrs.set(it1, nRow2, aVal1);

    CellStorageModified();
}

SvtBroadcaster* ScColumn::GetBroadcaster(SCROW nRow)
{
    return maBroadcasters.get<SvtBroadcaster*>(nRow);
}

const SvtBroadcaster* ScColumn::GetBroadcaster(SCROW nRow) const
{
    return maBroadcasters.get<SvtBroadcaster*>(nRow);
}

void ScColumn::DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2 )
{
    rBlockPos.miBroadcasterPos =
        maBroadcasters.set_empty(rBlockPos.miBroadcasterPos, nRow1, nRow2);
}

bool ScColumn::HasBroadcaster() const
{
    sc::BroadcasterStoreType::const_iterator it = maBroadcasters.begin(), itEnd = maBroadcasters.end();
    for (; it != itEnd; ++it)
    {
        if (it->type == sc::element_type_broadcaster)
            // Having a broadcaster block automatically means there is at least one broadcaster.
            return true;
    }
    return false;
}

sal_uInt16 ScColumn::GetTextWidth(SCROW nRow) const
{
    return maCellTextAttrs.get<sc::CellTextAttr>(nRow).mnTextWidth;
}

void ScColumn::SetTextWidth(SCROW nRow, sal_uInt16 nWidth)
{
    sc::CellTextAttrStoreType::position_type aPos = maCellTextAttrs.position(nRow);
    if (aPos.first->type != sc::element_type_celltextattr)
        return;

    // Set new value only when the slot is not empty.
    sc::celltextattr_block::at(*aPos.first->data, aPos.second).mnTextWidth = nWidth;
    CellStorageModified();
}

sal_uInt8 ScColumn::GetScriptType( SCROW nRow ) const
{
    if (!ValidRow(nRow) || maCellTextAttrs.is_empty(nRow))
        return 0;

    return maCellTextAttrs.get<sc::CellTextAttr>(nRow).mnScriptType;
}

sal_uInt8 ScColumn::GetRangeScriptType(
    sc::CellTextAttrStoreType::iterator& itPos, SCROW nRow1, SCROW nRow2 )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return 0;

    SCROW nRow = nRow1;
    std::pair<sc::CellTextAttrStoreType::iterator,size_t> aRet =
        maCellTextAttrs.position(itPos, nRow1);

    itPos = aRet.first; // Track the position of cell text attribute array.

    sal_uInt8 nScriptType = 0;
    bool bUpdated = false;
    if (itPos->type == sc::element_type_celltextattr)
    {
        sc::celltextattr_block::iterator it = sc::celltextattr_block::begin(*itPos->data);
        sc::celltextattr_block::iterator itEnd = sc::celltextattr_block::end(*itPos->data);
        std::advance(it, aRet.second);
        for (; it != itEnd; ++it, ++nRow)
        {
            if (nRow > nRow2)
                return nScriptType;

            sc::CellTextAttr& rVal = *it;
            if (UpdateScriptType(rVal, nRow))
                bUpdated = true;
            nScriptType |= rVal.mnScriptType;
        }
    }
    else
    {
        // Skip this whole block.
        nRow += itPos->size - aRet.second;
    }

    while (nRow <= nRow2)
    {
        ++itPos;
        if (itPos == maCellTextAttrs.end())
            return nScriptType;

        if (itPos->type != sc::element_type_celltextattr)
        {
            // Skip this whole block.
            nRow += itPos->size;
            continue;
        }

        sc::celltextattr_block::iterator it = sc::celltextattr_block::begin(*itPos->data);
        sc::celltextattr_block::iterator itEnd = sc::celltextattr_block::end(*itPos->data);
        for (; it != itEnd; ++it, ++nRow)
        {
            if (nRow > nRow2)
                return nScriptType;

            sc::CellTextAttr& rVal = *it;
            if (UpdateScriptType(rVal, nRow))
                bUpdated = true;

            nScriptType |= rVal.mnScriptType;
        }
    }

    if (bUpdated)
        CellStorageModified();

    return nScriptType;
}

void ScColumn::SetScriptType( SCROW nRow, sal_uInt8 nType )
{
    if (!ValidRow(nRow))
        return;

    sc::CellTextAttrStoreType::position_type aPos = maCellTextAttrs.position(nRow);
    if (aPos.first->type != sc::element_type_celltextattr)
        // Set new value only when the slot is already set.
        return;

    sc::celltextattr_block::at(*aPos.first->data, aPos.second).mnScriptType = nType;
    CellStorageModified();
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

formula::FormulaTokenRef ScColumn::ResolveStaticReference( SCROW nRow )
{
    std::pair<sc::CellStoreType::iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it == maCells.end())
        // Invalid row. Return a null token.
        return formula::FormulaTokenRef();

    switch (it->type)
    {
        case sc::element_type_numeric:
        {
            double fVal = sc::numeric_block::at(*it->data, aPos.second);
            return formula::FormulaTokenRef(new formula::FormulaDoubleToken(fVal));
        }
        case sc::element_type_formula:
        {
            ScFormulaCell* p = sc::formula_block::at(*it->data, aPos.second);
            if (p->IsValue())
                return formula::FormulaTokenRef(new formula::FormulaDoubleToken(p->GetValue()));

            return formula::FormulaTokenRef(new formula::FormulaStringToken(p->GetString()));
        }
        case sc::element_type_empty:
        default:
            // Return a value of 0.0 in all the other cases.
            return formula::FormulaTokenRef(new formula::FormulaDoubleToken(0.0));
    }
}

namespace {

class ToMatrixHandler
{
    ScMatrix& mrMat;
    SCCOL mnMatCol;
    SCROW mnTopRow;
public:
    ToMatrixHandler(ScMatrix& rMat, SCCOL nMatCol, SCROW nTopRow) :
        mrMat(rMat), mnMatCol(nMatCol), mnTopRow(nTopRow) {}

    void operator() (size_t nRow, double fVal)
    {
        mrMat.PutDouble(fVal, mnMatCol, nRow - mnTopRow);
    }

    void operator() (size_t nRow, const ScFormulaCell* p)
    {
        // Formula cell may need to re-calculate.
        ScFormulaCell& rCell = const_cast<ScFormulaCell&>(*p);
        if (rCell.IsValue())
            mrMat.PutDouble(rCell.GetValue(), mnMatCol, nRow - mnTopRow);
        else
            mrMat.PutString(rCell.GetString(), mnMatCol, nRow - mnTopRow);
    }

    void operator() (size_t nRow, const OUString& rStr)
    {
        mrMat.PutString(rStr, mnMatCol, nRow - mnTopRow);
    }

    void operator() (size_t nRow, const EditTextObject* pStr)
    {
        mrMat.PutString(ScEditUtil::GetString(*pStr), mnMatCol, nRow - mnTopRow);
    }
};

}

bool ScColumn::ResolveStaticReference( ScMatrix& rMat, SCCOL nMatCol, SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 > nRow2)
        return false;

    ToMatrixHandler aFunc(rMat, nMatCol, nRow1);
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nRow1, nRow2, aFunc);
    return true;
}

namespace {

struct CellBucket
{
    SCSIZE mnNumValStart;
    SCSIZE mnStrValStart;
    std::vector<double> maNumVals;
    std::vector<OUString> maStrVals;

    CellBucket() : mnNumValStart(0), mnStrValStart(0) {}

    void flush(ScMatrix& rMat, SCSIZE nCol)
    {
        if (!maNumVals.empty())
        {
            const double* p = &maNumVals[0];
            rMat.PutDouble(p, maNumVals.size(), nCol, mnNumValStart);
            reset();
        }
        else if (!maStrVals.empty())
        {
            const OUString* p = &maStrVals[0];
            rMat.PutString(p, maStrVals.size(), nCol, mnStrValStart);
            reset();
        }
    }

    void reset()
    {
        mnNumValStart = mnStrValStart = 0;
        maNumVals.clear();
        maStrVals.clear();
    }
};

class FillMatrixHandler
{
    ScMatrix& mrMat;
    size_t mnMatCol;
    size_t mnTopRow;

    SCCOL mnCol;
    SCTAB mnTab;

public:
    FillMatrixHandler(ScMatrix& rMat, size_t nMatCol, size_t nTopRow, SCCOL nCol, SCTAB nTab) :
        mrMat(rMat), mnMatCol(nMatCol), mnTopRow(nTopRow), mnCol(nCol), mnTab(nTab) {}

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        size_t nMatRow = node.position + nOffset - mnTopRow;

        switch (node.type)
        {
            case sc::element_type_numeric:
            {
                const double* p = &sc::numeric_block::at(*node.data, nOffset);
                mrMat.PutDouble(p, nDataSize, mnMatCol, nMatRow);
            }
            break;
            case sc::element_type_string:
            {
                const OUString* p = &sc::string_block::at(*node.data, nOffset);
                mrMat.PutString(p, nDataSize, mnMatCol, nMatRow);
            }
            break;
            case sc::element_type_edittext:
            {
                std::vector<OUString> aStrs;
                aStrs.reserve(nDataSize);
                sc::edittext_block::const_iterator it = sc::edittext_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::edittext_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (; it != itEnd; ++it)
                    aStrs.push_back(ScEditUtil::GetString(**it));

                const OUString* p = &aStrs[0];
                mrMat.PutString(p, nDataSize, mnMatCol, nMatRow);
            }
            break;
            case sc::element_type_formula:
            {
                CellBucket aBucket;
                sc::formula_block::const_iterator it = sc::formula_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::formula_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);

                size_t nPrevRow = 0, nThisRow = node.position + nOffset;
                for (; it != itEnd; ++it, nPrevRow = nThisRow, ++nThisRow)
                {
                    ScFormulaCell& rCell = const_cast<ScFormulaCell&>(**it);

                    if (rCell.IsEmpty())
                    {
                        aBucket.flush(mrMat, mnMatCol);
                        continue;
                    }

                    sal_uInt16 nErr;
                    double fVal;
                    if (rCell.GetErrorOrValue(nErr, fVal))
                    {
                        ScAddress aAdr(mnCol, nThisRow, mnTab);

                        if (nErr)
                            fVal = CreateDoubleError(nErr);

                        if (!aBucket.maNumVals.empty() && nThisRow == nPrevRow + 1)
                        {
                            // Secondary numbers.
                            aBucket.maNumVals.push_back(fVal);
                        }
                        else
                        {
                            // First number.
                            aBucket.flush(mrMat, mnMatCol);
                            aBucket.mnNumValStart = nThisRow - mnTopRow;
                            aBucket.maNumVals.push_back(fVal);
                        }
                        continue;
                    }

                    OUString aStr = rCell.GetString();
                    if (!aBucket.maStrVals.empty() && nThisRow == nPrevRow + 1)
                    {
                        // Secondary strings.
                        aBucket.maStrVals.push_back(aStr);
                    }
                    else
                    {
                        // First string.
                        aBucket.flush(mrMat, mnMatCol);
                        aBucket.mnStrValStart = nThisRow - mnTopRow;
                        aBucket.maStrVals.push_back(aStr);
                    }
                }

                aBucket.flush(mrMat, mnMatCol);
            }
            break;
            default:
                ;
        }
    }
};

}

void ScColumn::FillMatrix( ScMatrix& rMat, size_t nMatCol, SCROW nRow1, SCROW nRow2 ) const
{
    FillMatrixHandler aFunc(rMat, nMatCol, nRow1, nCol, nTab);
    sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
}

namespace {

bool appendDouble(
    sc::FormulaGroupContext::DoubleArrayType& rArray, size_t nLen,
    sc::CellStoreType::iterator it, const sc::CellStoreType::iterator& itEnd )
{
    size_t nLenRemain = nLen;
    for (; it != itEnd; ++it)
    {
        switch (it->type)
        {
            case sc::element_type_numeric:
            {
                sc::numeric_block::iterator itData = sc::numeric_block::begin(*it->data);
                sc::numeric_block::iterator itDataEnd;
                if (nLenRemain >= it->size)
                {
                    // Block is shorter than the remaining requested length.
                    itDataEnd = sc::numeric_block::end(*it->data);
                    nLenRemain -= it->size;
                }
                else
                {
                    itDataEnd = itData;
                    std::advance(itDataEnd, nLenRemain);
                    nLenRemain = 0;
                }

                for (; itData != itDataEnd; ++itData)
                    rArray.push_back(*itData);
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::iterator itData = sc::formula_block::begin(*it->data);
                sc::formula_block::iterator itDataEnd;
                if (nLenRemain >= it->size)
                {
                    // Block is shorter than the remaining requested length.
                    itDataEnd = sc::formula_block::end(*it->data);
                    nLenRemain -= it->size;
                }
                else
                {
                    itDataEnd = itData;
                    std::advance(itDataEnd, nLenRemain);
                    nLenRemain = 0;
                }

                sal_uInt16 nErr;
                double fVal;
                for (; itData != itDataEnd; ++itData)
                {
                    ScFormulaCell& rFC = **itData;
                    if (!rFC.GetErrorOrValue(nErr, fVal) || nErr)
                    {
                        if (nErr == ScErrorCodes::errCircularReference)
                        {
                            // This cell needs to be recalculated on next visit.
                            rFC.SetErrCode(0);
                            rFC.SetDirtyVar();
                        }
                        return false;
                    }

                    rArray.push_back(fVal);
                }
            }
            break;
            case sc::element_type_empty:
            {
                // Fill it with 0's.
                if (nLenRemain >= it->size)
                {
                    rArray.resize(rArray.size() + it->size, 0);
                    nLenRemain -= it->size;
                }
                else
                {
                    rArray.resize(rArray.size() + nLenRemain, 0);
                    nLenRemain = 0;
                }
            }
            break;
            case sc::element_type_string:
            case sc::element_type_edittext:
            default:
                return false;
        }

        if (!nLenRemain)
            return true;
    }

    return false;
}

}

const double* ScColumn::FetchDoubleArray( sc::FormulaGroupContext& rCxt, SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 > nRow2)
        return NULL;

    size_t nLenRequested = nRow2 - nRow1 + 1;
    sc::CellStoreType::position_type aPos = maCells.position(nRow1);
    size_t nLen = aPos.first->size - aPos.second;
    switch (aPos.first->type)
    {
        case sc::element_type_numeric:
        {
            // This is a numeric cell block.
            if (nLenRequested <= nLen)
                // Requested length fits a single block.
                return &sc::numeric_block::at(*aPos.first->data, aPos.second);

            // Allocate a new array and copy the values to it.
            sc::numeric_block::const_iterator it = sc::numeric_block::begin(*aPos.first->data);
            sc::numeric_block::const_iterator itEnd = sc::numeric_block::end(*aPos.first->data);
            std::advance(it, aPos.second);
            rCxt.maArrays.push_back(new sc::FormulaGroupContext::DoubleArrayType(it, itEnd));
            sc::FormulaGroupContext::DoubleArrayType& rArray = rCxt.maArrays.back();
            rArray.reserve(nLenRequested);

            // Fill the remaining array with values from the following blocks.
            ++aPos.first;
            if (!appendDouble(rArray, nLenRequested - nLen, aPos.first, maCells.end()))
                return NULL;

            return &rArray[0];
        }
        break;
        case sc::element_type_formula:
        {
            sal_uInt16 nErr;
            double fVal;

            rCxt.maArrays.push_back(new sc::FormulaGroupContext::DoubleArrayType);
            sc::FormulaGroupContext::DoubleArrayType& rArray = rCxt.maArrays.back();
            rArray.reserve(nLenRequested);

            sc::formula_block::const_iterator it = sc::formula_block::begin(*aPos.first->data);
            std::advance(it, aPos.second);
            sc::formula_block::const_iterator itEnd;
            if (nLenRequested <= nLen)
            {
                // Requested length is within a single block.
                itEnd = it;
                std::advance(itEnd, nLenRequested);
                for (; it != itEnd; ++it)
                {
                    ScFormulaCell& rCell = **it;
                    if (!rCell.GetErrorOrValue(nErr, fVal) || nErr)
                    {
                        if (nErr == ScErrorCodes::errCircularReference)
                        {
                            // This cell needs to be recalculated on next visit.
                            rCell.SetErrCode(0);
                            rCell.SetDirtyVar();
                        }
                        return NULL;
                    }

                    rArray.push_back(fVal);
                }

                return &rArray[0];
            }

            // Requested length goes beyond a single block.  Fill the array
            // with the content of this formula block first.
            itEnd = sc::formula_block::end(*aPos.first->data);
            for (; it != itEnd; ++it)
            {
                ScFormulaCell& rCell = **it;
                if (!rCell.GetErrorOrValue(nErr, fVal) || nErr)
                {
                    if (nErr == ScErrorCodes::errCircularReference)
                    {
                        // This cell needs to be recalculated on next visit.
                        rCell.SetErrCode(0);
                        rCell.SetDirtyVar();
                    }
                    return NULL;
                }

                rArray.push_back(fVal);
            }

            // Fill the remaining array with values from the following blocks.
            ++aPos.first;
            if (!appendDouble(rArray, nLenRequested - nLen, aPos.first, maCells.end()))
                return NULL;

            return &rArray[0];
        }
        break;
        case sc::element_type_empty:
        {
            if (nLenRequested <= nLen)
            {
                // Fill the whole length with zero.
                rCxt.maArrays.push_back(new sc::FormulaGroupContext::DoubleArrayType(nLenRequested, 0.0));
                return &rCxt.maArrays.back()[0];
            }

            // Fill the array with zero for the length of the empty block.
            rCxt.maArrays.push_back(new sc::FormulaGroupContext::DoubleArrayType(nLen, 0.0));
            sc::FormulaGroupContext::DoubleArrayType& rArray = rCxt.maArrays.back();
            rArray.reserve(nLenRequested);

            // Fill the remaining array with values from the following blocks.
            ++aPos.first;
            if (!appendDouble(rArray, nLenRequested - nLen, aPos.first, maCells.end()))
                return NULL;

            return &rArray[0];
        }
        default:
            ;
    }

    return NULL;
}

void ScColumn::SetFormulaResults( SCROW nRow, const double* pResults, size_t nLen )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
        // This is not a formula block.
        return;

    size_t nBlockLen = it->size - aPos.second;
    if (nBlockLen < nLen)
        // Result array is longer than the length of formula cells. Not good.
        return;

    sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
    std::advance(itCell, aPos.second);

    const double* pResEnd = pResults + nLen;
    for (; pResults != pResEnd; ++pResults, ++itCell)
    {
        ScFormulaCell& rCell = **itCell;
        rCell.SetResultDouble(*pResults);
        rCell.ResetDirty();
        rCell.SetChanged(true);
    }
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

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return NULL;

    if (it->type != sc::element_type_formula)
        // Not a formula cell.
        return NULL;

    return sc::formula_block::at(*it->data, aPos.second);
}

void ScColumn::FindDataAreaPos(SCROW& rRow, bool bDown) const
{
    // If the cell is empty, find the next non-empty cell position. If the
    // cell is not empty, find the last non-empty cell position in the current
    // contiguous cell block.

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        // Invalid row.
        return;

    if (it->type == sc::element_type_empty)
    {
        // Current cell is empty. Find the next non-empty cell.
        rRow = FindNextVisibleRowWithContent(it, rRow, bDown);
        return;
    }

    // Current cell is not empty.
    SCROW nNextRow = FindNextVisibleRow(rRow, bDown);
    aPos = maCells.position(it, nNextRow);
    it = aPos.first;
    if (it->type == sc::element_type_empty)
    {
        // Next visible cell is empty. Find the next non-empty cell.
        rRow = FindNextVisibleRowWithContent(it, nNextRow, bDown);
        return;
    }

    // Next visible cell is non-empty. Find the edge that's still visible.
    SCROW nLastRow = nNextRow;
    do
    {
        nNextRow = FindNextVisibleRow(nLastRow, bDown);
        if (nNextRow == nLastRow)
            break;

        aPos = maCells.position(it, nNextRow);
        it = aPos.first;
        if (it->type != sc::element_type_empty)
            nLastRow = nNextRow;
    }
    while (it->type != sc::element_type_empty);

    rRow = nLastRow;
}

bool ScColumn::HasDataAt(SCROW nRow) const
{
    return maCells.get_type(nRow) != sc::element_type_empty;
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
        SCROW nLastData = GetLastDataPos();    // always including notes, 0 if none

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

namespace {

class FindUsedRowsHandler
{
    typedef mdds::flat_segment_tree<SCROW,bool> UsedRowsType;
    UsedRowsType& mrUsed;
    UsedRowsType::const_iterator miUsed;
public:
    FindUsedRowsHandler(UsedRowsType& rUsed) : mrUsed(rUsed), miUsed(rUsed.begin()) {}

    void operator() (const sc::CellStoreType::value_type& node, size_t nOffset, size_t nDataSize)
    {
        if (node.type == sc::element_type_empty)
            return;

        SCROW nRow1 = node.position + nOffset;
        SCROW nRow2 = nRow1 + nDataSize - 1;
        miUsed = mrUsed.insert(miUsed, nRow1, nRow2+1, true).first;
    }
};

}

void ScColumn::FindUsed( SCROW nStartRow, SCROW nEndRow, mdds::flat_segment_tree<SCROW,bool>& rUsed ) const
{
    FindUsedRowsHandler aFunc(rUsed);
    sc::ParseBlock(maCells.begin(), maCells, aFunc, nStartRow, nEndRow);
}

namespace {

void startListening(
    sc::BroadcasterStoreType& rStore, sc::BroadcasterStoreType::iterator& itBlockPos, size_t nElemPos,
    SCROW nRow, SvtListener& rLst)
{
    switch (itBlockPos->type)
    {
        case sc::element_type_broadcaster:
        {
            // Broadcaster already exists here.
            SvtBroadcaster* pBC = sc::broadcaster_block::at(*itBlockPos->data, nElemPos);
            rLst.StartListening(*pBC);
        }
        break;
        case mdds::mtv::element_type_empty:
        {
            // No broadcaster exists at this position yet.
            SvtBroadcaster* pBC = new SvtBroadcaster;
            rLst.StartListening(*pBC);
            itBlockPos = rStore.set(itBlockPos, nRow, pBC); // Store the block position for next iteration.
        }
        break;
        default:
#if DEBUG_COLUMN_STORAGE
            cout << "ScColumn::StartListening: wrong block type encountered in the broadcaster storage." << endl;
            cout.flush();
            abort();
#else
            ;
#endif
    }
}

}

void ScColumn::StartListening( SvtListener& rLst, SCROW nRow )
{
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(nRow);
    startListening(maBroadcasters, aPos.first, aPos.second, nRow, rLst);
}

void ScColumn::MoveListeners( SvtBroadcaster& rSource, SCROW nDestRow )
{
    // Move listeners from the source position to the destination position.
    if (!rSource.HasListeners())
        // No listeners to relocate. Bail out.
        return;

    // See if the destination position already has a broadcaster, if not, create one.
    SvtBroadcaster* pBC = GetBroadcaster(nDestRow);
    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        maBroadcasters.set(nDestRow, pBC);
    }

    SvtListenerIter aIter(rSource);
    for (SvtListener* pLst = aIter.GoStart(); pLst; pLst = aIter.GoNext())
    {
        pLst->StartListening(*pBC);
        pLst->EndListening(rSource);
    }
}

void ScColumn::EndListening( SvtListener& rLst, SCROW nRow )
{
    SvtBroadcaster* pBC = GetBroadcaster(nRow);
    if (!pBC)
        return;

    rLst.EndListening(*pBC);
    if (!pBC->HasListeners())
        // There is no more listeners for this cell. Remove the broadcaster.
        maBroadcasters.set_empty(nRow, nRow);
}

void ScColumn::StartListening( sc::StartListeningContext& rCxt, SCROW nRow, SvtListener& rLst )
{
    if (!ValidRow(nRow))
        return;

    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(nTab, nCol);
    if (!p)
        return;

    sc::BroadcasterStoreType::iterator& it = p->miBroadcasterPos;
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(it, nRow);
    it = aPos.first; // store the block position for next iteration.
    startListening(maBroadcasters, it, aPos.second, nRow, rLst);
}

void ScColumn::EndListening( sc::EndListeningContext& rCxt, SCROW nRow, SvtListener& rListener )
{
    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(nTab, nCol);
    if (!p)
        return;

    sc::BroadcasterStoreType::iterator& it = p->miBroadcasterPos;
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(it, nRow);
    it = aPos.first; // store the block position for next iteration.
    if (it->type != sc::element_type_broadcaster)
        return;

    SvtBroadcaster* pBC = sc::broadcaster_block::at(*it->data, aPos.second);
    OSL_ASSERT(pBC);

    rListener.EndListening(*pBC);
    if (!pBC->HasListeners())
        // There is no more listeners for this cell. Add it to the purge list for later purging.
        rCxt.addEmptyBroadcasterPosition(nTab, nCol, nRow);
}

namespace {

struct CompileDBFormulaHandler
{
    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileDBFormula();
    }
};

class CompileDBFormula2Handler
{
    bool mbCreateFormulaString;

public:
    CompileDBFormula2Handler(bool bCreateFormulaString) :
        mbCreateFormulaString(bCreateFormulaString) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileDBFormula(mbCreateFormulaString);
    }
};

class CompileNameFormulaHandler
{
    bool mbCreateFormulaString;

public:
    CompileNameFormulaHandler(bool bCreateFormulaString) :
        mbCreateFormulaString(bCreateFormulaString) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileNameFormula(mbCreateFormulaString);
    }
};

struct CompileColRowNameFormulaHandler
{
    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileColRowNameFormula();
    }
};

}

void ScColumn::CompileDBFormula()
{
    CompileDBFormulaHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

void ScColumn::CompileDBFormula( bool bCreateFormulaString )
{
    CompileDBFormula2Handler aFunc(bCreateFormulaString);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

void ScColumn::CompileNameFormula( bool bCreateFormulaString )
{
    CompileNameFormulaHandler aFunc(bCreateFormulaString);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

void ScColumn::CompileColRowNameFormula()
{
    CompileColRowNameFormulaHandler aFunc;
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

namespace {

class UpdateSubTotalHandler
{
    ScFunctionData& mrData;
    ScFlatBoolRowSegments& mrHiddenRows;

    void update(double fVal, bool bVal)
    {
        if (!mrData.bError)
            return;

        switch (mrData.eFunc)
        {
            case SUBTOTAL_FUNC_SUM:
            case SUBTOTAL_FUNC_AVE:
            {
                if (!bVal)
                    return;

                ++mrData.nCount;
                if (!SubTotal::SafePlus(mrData.nVal, fVal))
                    mrData.bError = true;
            }
            break;
            case SUBTOTAL_FUNC_CNT:             // only the value
            {
                if (!bVal)
                    return;

                ++mrData.nCount;
            }
            break;
            case SUBTOTAL_FUNC_CNT2:            // everything
                ++mrData.nCount;
            break;
            case SUBTOTAL_FUNC_MAX:
            {
                if (!bVal)
                    return;

                if (++mrData.nCount == 1 || fVal > mrData.nVal)
                    mrData.nVal = fVal;
            }
            break;
            case SUBTOTAL_FUNC_MIN:
            {
                if (!bVal)
                    return;

                if (++mrData.nCount == 1 || fVal < mrData.nVal)
                    mrData.nVal = fVal;
            }
            break;
            default:
            {
                // added to avoid warnings
            }
        }
    }

public:
    UpdateSubTotalHandler(ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows) :
        mrData(rData), mrHiddenRows(rHiddenRows) {}

    void operator() (size_t nRow, double fVal)
    {
        if (mrHiddenRows.getValue(nRow))
            return;

        update(fVal, true);
    }

    void operator() (size_t nRow, ScFormulaCell* pCell)
    {
        if (mrHiddenRows.getValue(nRow))
            return;

        double fVal = 0.0;
        bool bVal = false;
        if (mrData.eFunc != SUBTOTAL_FUNC_CNT2) // it doesn't interest us
        {

            if (pCell->GetErrCode())
            {
                if (mrData.eFunc != SUBTOTAL_FUNC_CNT) // simply remove from count
                    mrData.bError = true;
            }
            else if (pCell->IsValue())
            {
                fVal = pCell->GetValue();
                bVal = true;
            }
            // otherwise text
        }

        update(fVal, bVal);
    }

    void operator() (mdds::mtv::element_t eType, size_t nTopRow, size_t nDataSize)
    {
        if (eType == sc::element_type_empty)
            return;

        for (size_t i = 0; i < nDataSize; ++i)
        {
            size_t nRow = nTopRow + i;
            if (mrHiddenRows.getValue(nRow))
                continue;

            update(0.0, false);
        }
    }
};

}

//  multiple selections:
void ScColumn::UpdateSelectionFunction(
    const ScMarkData& rMark, ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows,
    bool bDoExclude, SCROW nExStartRow, SCROW nExEndRow)
{
    if ( rData.eFunc != SUBTOTAL_FUNC_SELECTION_COUNT )
    {
        sc::SingleColumnSpanSet aSpanSet;
        aSpanSet.scan(rMark, nTab, nCol);
        if (bDoExclude)
        {
            aSpanSet.set(0, nExStartRow, false);
            aSpanSet.set(nExEndRow+1, MAXROWCOUNT, false);
        }

        sc::SingleColumnSpanSet::SpansType aSpans;
        aSpanSet.getSpans(aSpans);
        UpdateSubTotalHandler aFunc(rData, rHiddenRows);
        sc::CellStoreType::iterator itCellPos = maCells.begin();
        sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
        for (; it != itEnd; ++it)
        {
            itCellPos = sc::ProcessFormulaNumeric(
                itCellPos, maCells, it->mnRow1, it->mnRow2, aFunc);
        }
    }
    else
    {
        SCROW nTop, nBottom;

        // ScMarkData::GetArray() returns a valid array only if
        // 'rMark.IsMultiMarked()' returns true.
        // Since ScTable::UpdateSelectionFunction() already checked that first
        // before calling this method it does not need to be repeated here.

        ScMarkArrayIter aIter(rMark.GetArray() + nCol);
        ScFlatBoolRowSegments::RangeData aData;

        while (aIter.Next( nTop, nBottom ))
        {
            sal_Int32 nCellCount = 0;    // to get the count of selected visible cells
            SCROW nRow = nTop;

            while ( nRow <= nBottom )
            {
                if (!rHiddenRows.getRangeData(nRow, aData))     // failed to get range data
                    break;

                if (aData.mnRow2 > nBottom)
                    aData.mnRow2 = nBottom;

                if (!aData.mbValue)
                {
                    nCellCount += aData.mnRow2 - nRow + 1;

                    // Till this point, nCellCount also includes count of those cells which are excluded
                    // So, they should be decremented now.

                    if ( bDoExclude && nExStartRow >= nRow && nExEndRow <= aData.mnRow2 )
                        nCellCount -= nExEndRow - nExStartRow + 1;
                }
                nRow = aData.mnRow2 + 1;
            }
            rData.nCount += nCellCount;
        }
    }
}

//  with bNoMarked ignore the multiple selections
void ScColumn::UpdateAreaFunction(
    ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows, SCROW nStartRow, SCROW nEndRow)
{
    if ( rData.eFunc != SUBTOTAL_FUNC_SELECTION_COUNT )
    {
        UpdateSubTotalHandler aFunc(rData, rHiddenRows);
        sc::ProcessFormulaNumeric(
            maCells.begin(), maCells, nStartRow, nEndRow, aFunc);
    }
    else
    {
        sal_Int32 nCellCount = 0;    // to get the count of selected visible cells
        SCROW nRow = nStartRow;
        ScFlatBoolRowSegments::RangeData aData;

        while (nRow <= nEndRow)
        {
            if (!rHiddenRows.getRangeData(nRow, aData))
               break;

            if (!aData.mbValue)
                nCellCount += (aData.mnRow2 <= nEndRow ? aData.mnRow2 : nEndRow) - nRow + 1;

            nRow = aData.mnRow2 + 1;
        }
        rData.nCount += nCellCount;
    }
}

namespace {

class WeightedCounter
{
    size_t mnCount;
public:
    WeightedCounter() : mnCount(0) {}

    void operator() (const sc::CellStoreType::value_type& node)
    {
        switch (node.type)
        {
            case sc::element_type_numeric:
            case sc::element_type_string:
                mnCount += node.size;
            break;
            case sc::element_type_formula:
            {
                // Each formula cell is worth its code length plus 5.
                sc::formula_block::const_iterator it = sc::formula_block::begin(*node.data);
                sc::formula_block::const_iterator itEnd = sc::formula_block::end(*node.data);
                for (; it != itEnd; ++it)
                {
                    const ScFormulaCell* p = *it;
                    mnCount += 5 + p->GetCode()->GetCodeLen();
                }
            }
            break;
            case sc::element_type_edittext:
                // each edit-text cell is worth 50.
                mnCount += node.size * 50;
            break;
            default:
                ;
        }
    }

    size_t getCount() const { return mnCount; }
};

}

sal_uInt32 ScColumn::GetWeightedCount() const
{
    WeightedCounter aFunc;
    std::for_each(maCells.begin(), maCells.end(), aFunc);
    return aFunc.getCount();
}

namespace {

class CodeCounter
{
    size_t mnCount;
public:
    CodeCounter() : mnCount(0) {}

    void operator() (size_t, const ScFormulaCell* p)
    {
        mnCount += p->GetCode()->GetCodeLen();
    }

    size_t getCount() const { return mnCount; }
};

}

sal_uInt32 ScColumn::GetCodeCount() const
{
    CodeCounter aFunc;
    sc::ParseFormula(maCells, aFunc);
    return aFunc.getCount();
}





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
