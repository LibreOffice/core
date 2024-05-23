/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <column.hxx>
#include <docsh.hxx>
#include <scitems.hxx>
#include <formulacell.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <attarray.hxx>
#include <patattr.hxx>
#include <cellform.hxx>
#include <editutil.hxx>
#include <subtotal.hxx>
#include <markdata.hxx>
#include <fillinfo.hxx>
#include <segmenttree.hxx>
#include <docparam.hxx>
#include <cellvalue.hxx>
#include <tokenarray.hxx>
#include <formulagroup.hxx>
#include <listenercontext.hxx>
#include <mtvcellfunc.hxx>
#include <progress.hxx>
#include <scmatrix.hxx>
#include <rowheightcontext.hxx>
#include <tokenstringcontext.hxx>
#include <sortparam.hxx>
#include <SparklineGroup.hxx>
#include <SparklineList.hxx>

#include <editeng/eeitem.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svx/algitem.hxx>
#include <editeng/editobj.hxx>
#include <editeng/editstat.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <svx/rotmodit.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/justifyitem.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/broadcast.hxx>
#include <utility>
#include <vcl/outdev.hxx>
#include <formula/errorcodes.hxx>
#include <formula/vectortoken.hxx>

#include <algorithm>
#include <limits>
#include <memory>
#include <numeric>

// factor from font size to optimal cell height (text width)
#define SC_ROT_BREAK_FACTOR     6

static bool IsAmbiguousScript( SvtScriptType nScript )
{
    //TODO: move to a header file
    return ( nScript != SvtScriptType::LATIN &&
             nScript != SvtScriptType::ASIAN &&
             nScript != SvtScriptType::COMPLEX );
}

//  Data operations

tools::Long ScColumn::GetNeededSize(
    SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY,
    const Fraction& rZoomX, const Fraction& rZoomY,
    bool bWidth, const ScNeededSizeOptions& rOptions,
    const ScPatternAttr** ppPatternChange, bool bInPrintTwips ) const
{
    // If bInPrintTwips is set, the size calculated should be in print twips,
    // else it should be in pixels.

    // Switch unit to MapTwip instead ? (temporarily and then revert before exit).
    if (bInPrintTwips)
        assert(pDev->GetMapMode().GetMapUnit() == MapUnit::MapTwip);

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end() || it->type == sc::element_type_empty)
        // Empty cell, or invalid row.
        return 0;

    tools::Long nValue = 0;
    ScRefCellValue aCell = GetCellValue(it, aPos.second);
    double nPPT = bWidth ? nPPTX : nPPTY;

    auto conditionalScaleFunc = [bInPrintTwips](tools::Long nMeasure, double fScale) {
        return bInPrintTwips ? nMeasure : static_cast<tools::Long>(nMeasure * fScale);
    };

    const ScPatternAttr* pPattern = rOptions.aPattern.getScPatternAttr();
    if (!pPattern)
        pPattern = pAttrArray->GetPattern( nRow );

    //      merged?
    //      Do not merge in conditional formatting

    const ScMergeAttr*      pMerge = &pPattern->GetItem(ATTR_MERGE);
    const ScMergeFlagAttr*  pFlag = &pPattern->GetItem(ATTR_MERGE_FLAG);

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
    ScDocument& rDocument = GetDoc();
    const SfxItemSet* pCondSet = rDocument.GetCondResult( nCol, nRow, nTab );

    //The pPattern may change in GetCondResult
    if (aCell.getType() == CELLTYPE_FORMULA)
    {
        pPattern = pAttrArray->GetPattern( nRow );
        if (ppPatternChange)
            *ppPatternChange = pPattern;
    }
    //  line break?

    const SvxHorJustifyItem* pCondItem;
    SvxCellHorJustify eHorJust;
    if (pCondSet && (pCondItem = pCondSet->GetItemIfSet(ATTR_HOR_JUSTIFY)) )
        eHorJust = pCondItem->GetValue();
    else
        eHorJust = pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue();
    bool bBreak;
    const ScLineBreakCell* pLineBreakCell;
    if ( eHorJust == SvxCellHorJustify::Block )
        bBreak = true;
    else if ( pCondSet && (pLineBreakCell = pCondSet->GetItemIfSet(ATTR_LINEBREAK)) )
        bBreak = pLineBreakCell->GetValue();
    else
        bBreak = pPattern->GetItem(ATTR_LINEBREAK).GetValue();

    ScInterpreterContext& rContext = rDocument.GetNonThreadedContext();
    sal_uInt32 nFormat = pPattern->GetNumberFormat( rContext, pCondSet );

    // get "cell is value" flag
    // Must be synchronized with ScOutputData::LayoutStrings()
    bool bCellIsValue = (aCell.getType() == CELLTYPE_VALUE);
    if (aCell.getType() == CELLTYPE_FORMULA)
    {
        ScFormulaCell* pFCell = aCell.getFormula();
        bCellIsValue = pFCell->IsRunning();
        if (!bCellIsValue)
        {
            bCellIsValue = pFCell->IsValue();
            if (bCellIsValue)
            {
                // the pattern may change in IsValue()
                pPattern = pAttrArray->GetPattern( nRow );
                if (ppPatternChange)
                    *ppPatternChange = pPattern;
            }
        }
    }

    // #i111387#, tdf#121040: disable automatic line breaks for all number formats
    if (bBreak && bCellIsValue && (rContext.NFGetType(nFormat) == SvNumFormatType::NUMBER))
    {
        // If a formula cell needs to be interpreted during aCell.hasNumeric()
        // to determine the type, the pattern may get invalidated because the
        // result may set a number format. In which case there's also the
        // General format not set anymore...
        bool bMayInvalidatePattern = (aCell.getType() == CELLTYPE_FORMULA);
        const CellAttributeHolder aOldPattern(pPattern);
        bool bNumeric = aCell.hasNumeric();
        if (bMayInvalidatePattern)
        {
            pPattern = pAttrArray->GetPattern( nRow );
            if (ppPatternChange)
                *ppPatternChange = pPattern;    // XXX caller may have to check for change!
        }
        if (bNumeric)
        {
            if (!bMayInvalidatePattern || ScPatternAttr::areSame(pPattern, aOldPattern.getScPatternAttr()))
                bBreak = false;
            else
            {
                nFormat = pPattern->GetNumberFormat( rContext, pCondSet );
                if (rContext.NFGetType(nFormat) == SvNumFormatType::NUMBER)
                    bBreak = false;
            }
        }
    }

    //  get other attributes from pattern and conditional formatting

    SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
    bool bAsianVertical = ( eOrient == SvxCellOrientation::Stacked &&
            pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet ).GetValue() );
    if ( bAsianVertical )
        bBreak = false;

    if ( bWidth && bBreak )     // after determining bAsianVertical (bBreak may be reset)
        return 0;

    Degree100 nRotate(0);
    SvxRotateMode eRotMode = SVX_ROTATE_MODE_STANDARD;
    if ( eOrient == SvxCellOrientation::Standard )
    {
        const ScRotateValueItem* pRotateValueItem;
        if (pCondSet &&
                (pRotateValueItem = pCondSet->GetItemIfSet(ATTR_ROTATE_VALUE)) )
            nRotate = pRotateValueItem->GetValue();
        else
            nRotate = pPattern->GetItem(ATTR_ROTATE_VALUE).GetValue();
        if ( nRotate )
        {
            const SvxRotateModeItem* pRotateModeItem;
            if (pCondSet &&
                    (pRotateModeItem = pCondSet->GetItemIfSet(ATTR_ROTATE_MODE)) )
                eRotMode = pRotateModeItem->GetValue();
            else
                eRotMode = pPattern->GetItem(ATTR_ROTATE_MODE).GetValue();

            if ( nRotate == 18000_deg100 )
                eRotMode = SVX_ROTATE_MODE_STANDARD;    // no overflow
        }
    }

    if ( eHorJust == SvxCellHorJustify::Repeat )
    {
        // ignore orientation/rotation if "repeat" is active
        eOrient = SvxCellOrientation::Standard;
        nRotate = 0_deg100;
        bAsianVertical = false;
    }

    const SvxMarginItem* pMargin;
    if (pCondSet &&
            (pMargin = pCondSet->GetItemIfSet(ATTR_MARGIN)) )
        ;
    else
        pMargin = &pPattern->GetItem(ATTR_MARGIN);
    sal_uInt16 nIndent = 0;
    if ( eHorJust == SvxCellHorJustify::Left )
    {
        const ScIndentItem* pIndentItem;
        if (pCondSet &&
                (pIndentItem = pCondSet->GetItemIfSet(ATTR_INDENT)) )
            nIndent = pIndentItem->GetValue();
        else
            nIndent = pPattern->GetItem(ATTR_INDENT).GetValue();
    }

    SvtScriptType nScript = rDocument.GetScriptType(nCol, nRow, nTab);
    if (nScript == SvtScriptType::NONE) nScript = ScGlobal::GetDefaultScriptType();

    //  also call SetFont for edit cells, because bGetFont may be set only once
    //  bGetFont is set also if script type changes
    if (rOptions.bGetFont)
    {
        Fraction aFontZoom = ( eOrient == SvxCellOrientation::Standard ) ? rZoomX : rZoomY;
        vcl::Font aFont;
        aFont.SetKerning(FontKerning::NONE); // like ScDrawStringsVars::SetPattern
        // font color doesn't matter here
        pPattern->fillFontOnly(aFont, pDev, &aFontZoom, pCondSet, nScript);
        pDev->SetFont(aFont);
    }

    bool bAddMargin = true;
    CellType eCellType = aCell.getType();

    bool bEditEngine = (eCellType == CELLTYPE_EDIT ||
                        eOrient == SvxCellOrientation::Stacked ||
                        IsAmbiguousScript(nScript) ||
                        ((eCellType == CELLTYPE_FORMULA) && aCell.getFormula()->IsMultilineResult()));

    if (!bEditEngine)                                   // direct output
    {
        const Color* pColor;
        OUString aValStr = ScCellFormat::GetString(
            aCell, nFormat, &pColor, &rContext, rDocument, true, rOptions.bFormula);

        if (!aValStr.isEmpty())
        {
            //  SetFont is moved up

            Size aSize;
            if ( eOrient != SvxCellOrientation::Standard )
            {
                aSize = Size( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
                tools::Long nTemp = aSize.Width();
                aSize.setWidth( aSize.Height() );
                aSize.setHeight( nTemp );
            }
            else if ( nRotate )
            {
                //TODO: take different X/Y scaling into consideration

                aSize = Size( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
                double nRealOrient = toRadians(nRotate);
                double nCosAbs = fabs( cos( nRealOrient ) );
                double nSinAbs = fabs( sin( nRealOrient ) );
                tools::Long nHeight = static_cast<tools::Long>( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
                tools::Long nWidth;
                if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                    nWidth  = static_cast<tools::Long>( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
                else if ( rOptions.bTotalSize )
                {
                    nWidth = conditionalScaleFunc(rDocument.GetColWidth( nCol,nTab ), nPPT);
                    bAddMargin = false;
                    //  only to the right:
                    //TODO: differ on direction up/down (only Text/whole height)
                    if ( pPattern->GetRotateDir( pCondSet ) == ScRotateDir::Right )
                        nWidth += static_cast<tools::Long>( rDocument.GetRowHeight( nRow,nTab ) *
                                            (bInPrintTwips ? 1.0 : nPPT) * nCosAbs / nSinAbs );
                }
                else
                    nWidth  = static_cast<tools::Long>( aSize.Height() / nSinAbs );   //TODO: limit?

                if ( bBreak && !rOptions.bTotalSize )
                {
                    //  limit size for line break
                    tools::Long nCmp = pDev->GetFont().GetFontSize().Height() * SC_ROT_BREAK_FACTOR;
                    if ( nHeight > nCmp )
                        nHeight = nCmp;
                }

                aSize = Size( nWidth, nHeight );
                nValue = bWidth ? aSize.Width() : aSize.Height();
            }
            else if (bBreak && !bWidth)
                aSize = Size( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
            else
                // in the common case (height), avoid calling the expensive GetTextWidth
                nValue = bWidth ? pDev->GetTextWidth( aValStr ) : pDev->GetTextHeight();

            if ( bAddMargin )
            {
                if (bWidth)
                {
                    nValue += conditionalScaleFunc(pMargin->GetLeftMargin(), nPPT) +
                              conditionalScaleFunc(pMargin->GetRightMargin(), nPPT);
                    if ( nIndent )
                        nValue += conditionalScaleFunc(nIndent, nPPT);
                }
                else
                    nValue += conditionalScaleFunc(pMargin->GetTopMargin(), nPPT) +
                              conditionalScaleFunc(pMargin->GetBottomMargin(), nPPT);
            }

            //  linebreak done ?

            if ( bBreak && !bWidth )
            {
                //  test with EditEngine the safety at 90%
                //  (due to rounding errors and because EditEngine formats partially differently)

                tools::Long nDocSize = conditionalScaleFunc((rDocument.GetColWidth( nCol,nTab ) -
                                    pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
                                    nIndent), nPPTX);
                nDocSize = (nDocSize * 9) / 10;           // for safety
                if ( aSize.Width() > nDocSize )
                    bEditEngine = true;
            }
        }
    }

    if (bEditEngine)
    {
        //  the font is not reset each time with !bEditEngine
        vcl::Font aOldFont = pDev->GetFont();

        MapMode aHMMMode( MapUnit::Map100thMM, Point(), rZoomX, rZoomY );

        // save in document ?
        std::unique_ptr<ScFieldEditEngine> pEngine = rDocument.CreateFieldEditEngine();

        const bool bPrevUpdateLayout = pEngine->SetUpdateLayout( false );
        bool bTextWysiwyg = ( pDev->GetOutDevType() == OUTDEV_PRINTER );
        MapMode aOld = pDev->GetMapMode();
        pDev->SetMapMode( aHMMMode );
        pEngine->SetRefDevice( pDev );
        rDocument.ApplyAsianEditSettings( *pEngine );
        SfxItemSet aSet( pEngine->GetEmptyItemSet() );
        if ( ScStyleSheet* pPreviewStyle = rDocument.GetPreviewCellStyle( nCol, nRow, nTab ) )
        {
            ScPatternAttr aPreviewPattern( *pPattern );
            aPreviewPattern.SetStyleSheet(pPreviewStyle);
            aPreviewPattern.FillEditItemSet( &aSet, pCondSet );
        }
        else
        {
            SfxItemSet* pFontSet = rDocument.GetPreviewFont( nCol, nRow, nTab );
            pPattern->FillEditItemSet( &aSet, pFontSet ? pFontSet : pCondSet );
        }
//          no longer needed, are set with the text (is faster)
//          pEngine->SetDefaults( pSet );

        if ( aSet.Get(EE_PARA_HYPHENATE).GetValue() ) {

            css::uno::Reference<css::linguistic2::XHyphenator> xXHyphenator( LinguMgr::GetHyphenator() );
            pEngine->SetHyphenator( xXHyphenator );
        }

        Size aPaper( 1000000, 1000000 );
        if ( eOrient==SvxCellOrientation::Stacked && !bAsianVertical )
            aPaper.setWidth( 1 );
        else if (bBreak)
        {
            double fWidthFactor = bInPrintTwips ? 1.0 : nPPTX;
            if ( bTextWysiwyg )
            {
                //  if text is formatted for printer, don't use PixelToLogic,
                //  to ensure the exact same paper width (and same line breaks) as in
                //  ScEditUtil::GetEditArea, used for output.

                fWidthFactor = o3tl::convert(1.0, o3tl::Length::twip, o3tl::Length::mm100);
            }

            // use original width for hidden columns:
            tools::Long nDocWidth = static_cast<tools::Long>( rDocument.GetOriginalWidth(nCol,nTab) * fWidthFactor );
            SCCOL nColMerge = pMerge->GetColMerge();
            if (nColMerge > 1)
                for (SCCOL nColAdd=1; nColAdd<nColMerge; nColAdd++)
                    nDocWidth += static_cast<tools::Long>( rDocument.GetColWidth(nCol+nColAdd,nTab) * fWidthFactor );
            nDocWidth -= static_cast<tools::Long>( pMargin->GetLeftMargin() * fWidthFactor )
                       + static_cast<tools::Long>( pMargin->GetRightMargin() * fWidthFactor )
                       + 1;     // output size is width-1 pixel (due to gridline)
            if ( nIndent )
                nDocWidth -= static_cast<tools::Long>( nIndent * fWidthFactor );

            // space for AutoFilter button:  20 * nZoom/100
            constexpr tools::Long nFilterButtonWidthPix = 20; // Autofilter pixel width at 100% zoom.
            if ( pFlag->HasAutoFilter() && !bTextWysiwyg )
                nDocWidth -= bInPrintTwips ? o3tl::convert(nFilterButtonWidthPix, o3tl::Length::px,
                                                           o3tl::Length::twip)
                                           : tools::Long(rZoomX * nFilterButtonWidthPix);

            aPaper.setWidth( nDocWidth );

            if ( !bTextWysiwyg )
            {
                aPaper = bInPrintTwips ?
                        o3tl::convert(aPaper, o3tl::Length::twip, o3tl::Length::mm100) :
                        pDev->PixelToLogic(aPaper, aHMMMode);
            }
        }
        pEngine->SetPaperSize(aPaper);

        if (aCell.getType() == CELLTYPE_EDIT)
        {
            pEngine->SetTextNewDefaults(*aCell.getEditText(), std::move(aSet));
        }
        else
        {
            const Color* pColor;
            OUString aString = ScCellFormat::GetString(
                aCell, nFormat, &pColor, &rContext, rDocument, true,
                rOptions.bFormula);

            if (!aString.isEmpty())
                pEngine->SetTextNewDefaults(aString, std::move(aSet));
            else
                pEngine->SetDefaults(std::move(aSet));
        }

        bool bEngineVertical = pEngine->IsEffectivelyVertical();
        pEngine->SetVertical( bAsianVertical );
        pEngine->SetUpdateLayout( bPrevUpdateLayout );

        bool bEdWidth = bWidth;
        if ( eOrient != SvxCellOrientation::Standard && eOrient != SvxCellOrientation::Stacked )
            bEdWidth = !bEdWidth;
        if ( nRotate )
        {
            //TODO: take different X/Y scaling into consideration

            Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
            double nRealOrient = toRadians(nRotate);
            double nCosAbs = fabs( cos( nRealOrient ) );
            double nSinAbs = fabs( sin( nRealOrient ) );
            tools::Long nHeight = static_cast<tools::Long>( aSize.Height() * nCosAbs + aSize.Width() * nSinAbs );
            tools::Long nWidth;
            if ( eRotMode == SVX_ROTATE_MODE_STANDARD )
                nWidth  = static_cast<tools::Long>( aSize.Width() * nCosAbs + aSize.Height() * nSinAbs );
            else if ( rOptions.bTotalSize )
            {
                nWidth = conditionalScaleFunc(rDocument.GetColWidth( nCol,nTab ), nPPT);
                bAddMargin = false;
                if ( pPattern->GetRotateDir( pCondSet ) == ScRotateDir::Right )
                    nWidth += static_cast<tools::Long>( rDocument.GetRowHeight( nRow,nTab ) *
                                        (bInPrintTwips ? 1.0 : nPPT) * nCosAbs / nSinAbs );
            }
            else
                nWidth  = static_cast<tools::Long>( aSize.Height() / nSinAbs );   //TODO: limit?
            aSize = Size( nWidth, nHeight );

            Size aTextSize = bInPrintTwips ?
                    o3tl::toTwips(aSize, o3tl::Length::mm100) :
                    pDev->LogicToPixel(aSize, aHMMMode);

            if ( bEdWidth )
                nValue = aTextSize.Width();
            else
            {
                nValue = aTextSize.Height();

                if ( bBreak && !rOptions.bTotalSize )
                {
                    //  limit size for line break
                    tools::Long nCmp = aOldFont.GetFontSize().Height() * SC_ROT_BREAK_FACTOR;
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
            {
                sal_uInt32 aTextSize(pEngine->CalcTextWidth());
                nValue = bInPrintTwips ?
                        o3tl::toTwips(aTextSize, o3tl::Length::mm100) :
                        pDev->LogicToPixel(Size(aTextSize, 0), aHMMMode).Width();
            }
        }
        else            // height
        {
            sal_uInt32 aTextSize(pEngine->GetTextHeight());
            nValue = bInPrintTwips ?
                    o3tl::toTwips(aTextSize, o3tl::Length::mm100) :
                    pDev->LogicToPixel(Size(0, aTextSize), aHMMMode).Height();
        }

        if ( nValue && bAddMargin )
        {
            if (bWidth)
            {
                nValue += conditionalScaleFunc(pMargin->GetLeftMargin(), nPPT) +
                          conditionalScaleFunc(pMargin->GetRightMargin(), nPPT);
                if (nIndent)
                    nValue += conditionalScaleFunc(nIndent, nPPT);
            }
            else
            {
                nValue += conditionalScaleFunc(pMargin->GetTopMargin(), nPPT) +
                          conditionalScaleFunc(pMargin->GetBottomMargin(), nPPT);

                if ( bAsianVertical && pDev->GetOutDevType() != OUTDEV_PRINTER )
                {
                    //  add 1pt extra (default margin value) for line breaks with SetVertical
                    constexpr tools::Long nDefaultMarginInPoints = 1;
                    nValue += conditionalScaleFunc(
                        o3tl::convert(nDefaultMarginInPoints, o3tl::Length::pt, o3tl::Length::twip),
                        nPPT);
                }
            }
        }

        //  EditEngine is cached and re-used, so the old vertical flag must be restored
        pEngine->SetVertical( bEngineVertical );

        rDocument.DisposeFieldEditEngine(pEngine);

        pDev->SetMapMode( aOld );
        pDev->SetFont( aOldFont );
    }

    if (bWidth)
    {
        //      place for Autofilter Button
        //      20 * nZoom/100
        //      Conditional formatting is not interesting here
        constexpr tools::Long nFilterButtonWidthPix = 20; // Autofilter pixel width at 100% zoom.
        ScMF nFlags = pPattern->GetItem(ATTR_MERGE_FLAG).GetValue();
        if (nFlags & ScMF::Auto)
            nValue += bInPrintTwips ? o3tl::convert(nFilterButtonWidthPix, o3tl::Length::px,
                                                    o3tl::Length::twip)
                                    : tools::Long(rZoomX * nFilterButtonWidthPix);
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

    // tdf#59820 - search for the longest substring in a multiline string
    void checkLineBreak(const OUString& aStrVal)
    {
        sal_Int32 nFromIndex = 0;
        sal_Int32 nToIndex = aStrVal.indexOf('\n', nFromIndex);
        // if there is no line break, just take the length of the entire string
        if (nToIndex == -1)
        {
            mnMaxLen = aStrVal.getLength();
            maMaxLenStr = aStrVal;
        }
        else
        {
            sal_Int32 nMaxLen = 0;
            // search for the longest substring in the multiline string
            while (nToIndex != -1)
            {
                if (nMaxLen < nToIndex - nFromIndex)
                {
                    nMaxLen = nToIndex - nFromIndex;
                }
                nFromIndex = nToIndex + 1;
                nToIndex = aStrVal.indexOf('\n', nFromIndex);
            }
            // take into consideration the last part of multiline string
            nToIndex = aStrVal.getLength() - nFromIndex;
            if (nMaxLen < nToIndex)
            {
                nMaxLen = nToIndex;
            }
            // assign new maximum including its substring
            if (mnMaxLen < nMaxLen)
            {
                mnMaxLen = nMaxLen;
                maMaxLenStr = aStrVal.subView(nFromIndex);
            }
        }
    }

    void checkLength(const ScRefCellValue& rCell)
    {
        const Color* pColor;
        OUString aValStr = ScCellFormat::GetString(
            rCell, mnFormat, &pColor, nullptr, mrDoc);

        if (aValStr.getLength() <= mnMaxLen)
            return;

        switch (rCell.getType())
        {
            case CELLTYPE_NONE:
            case CELLTYPE_VALUE:
                mnMaxLen = aValStr.getLength();
                maMaxLenStr = aValStr;
                break;
            case CELLTYPE_EDIT:
            case CELLTYPE_STRING:
            case CELLTYPE_FORMULA:
            default:
                checkLineBreak(aValStr);
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

    void operator() (size_t /*nRow*/, const svl::SharedString& rSS)
    {
        if (rSS.getLength() > mnMaxLen)
        {
            checkLineBreak(rSS.getString());
        }
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

    const OUString& getMaxLenStr() const { return maMaxLenStr; }
};

}

sal_uInt16 ScColumn::GetOptimalColWidth(
    OutputDevice* pDev, double nPPTX, double nPPTY, const Fraction& rZoomX, const Fraction& rZoomY,
    bool bFormula, sal_uInt16 nOldWidth, const ScMarkData* pMarkData, const ScColWidthParam* pParam) const
{
    if (maCells.block_size() == 1 && maCells.begin()->type == sc::element_type_empty)
        // All cells are empty.
        return nOldWidth;

    sc::SingleColumnSpanSet::SpansType aMarkedSpans;
    if (pMarkData && (pMarkData->IsMarked() || pMarkData->IsMultiMarked()))
    {
        sc::SingleColumnSpanSet aSpanSet(GetDoc().GetSheetLimits());
        aSpanSet.scan(*pMarkData, nTab, nCol);
        aSpanSet.getSpans(aMarkedSpans);
    }
    else
        // "Select" the entire column if no selection exists.
        aMarkedSpans.emplace_back(0, GetDoc().MaxRow());

    sal_uInt16 nWidth = static_cast<sal_uInt16>(nOldWidth*nPPTX);
    bool bFound = false;
    ScDocument& rDocument = GetDoc();

    if ( pParam && pParam->mbSimpleText )
    {   // all the same except for number format
        SCROW nRow = 0;
        const ScPatternAttr* pPattern = GetPattern( nRow );
        vcl::Font aFont;
        aFont.SetKerning(FontKerning::NONE); // like ScDrawStringsVars::SetPattern
        // font color doesn't matter here
        pPattern->fillFontOnly(aFont, pDev, &rZoomX);
        pDev->SetFont(aFont);
        const SvxMarginItem* pMargin = &pPattern->GetItem(ATTR_MARGIN);
        tools::Long nMargin = static_cast<tools::Long>( pMargin->GetLeftMargin() * nPPTX ) +
                        static_cast<tools::Long>( pMargin->GetRightMargin() * nPPTX );

        // Try to find the row that has the longest string, and measure the width of that string.
        ScInterpreterContext& rContext = rDocument.GetNonThreadedContext();
        sal_uInt32 nFormat = pPattern->GetNumberFormat(rContext);
        while ((nFormat % SV_COUNTRY_LANGUAGE_OFFSET) == 0 && nRow <= 2)
        {
            // This is often used with CSV import or other data having a header
            // row; if there is no specific format set try next row for actual
            // data format.
            // Or again in case there was a leading sep=";" row or two header
            // rows..
            const ScPatternAttr* pNextPattern = GetPattern( ++nRow );
            if (!ScPatternAttr::areSame(pNextPattern, pPattern))
                nFormat = pNextPattern->GetNumberFormat(rContext);
        }
        OUString aLongStr;
        const Color* pColor;
        if (pParam->mnMaxTextRow >= 0)
        {
            ScRefCellValue aCell = GetCellValue(pParam->mnMaxTextRow);
            aLongStr = ScCellFormat::GetString(
                aCell, nFormat, &pColor, &rContext, rDocument);
        }
        else
        {
            // Go though all non-empty cells within selection.
            MaxStrLenFinder aFunc(rDocument, nFormat);
            sc::CellStoreType::const_iterator itPos = maCells.begin();
            for (const auto& rMarkedSpan : aMarkedSpans)
                itPos = sc::ParseAllNonEmpty(itPos, maCells, rMarkedSpan.mnRow1, rMarkedSpan.mnRow2, aFunc);

            aLongStr = aFunc.getMaxLenStr();
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
        const ScPatternAttr* pOldPattern = nullptr;

        // Go though all non-empty cells within selection.
        sc::CellStoreType::const_iterator itPos = maCells.begin();
        // coverity[auto_causes_copy] This trivial copy is intentional
        for (auto [ nRow, nRow2 ] : aMarkedSpans)
        {
            while (nRow <= nRow2)
            {
                size_t nOffset;
                std::tie(itPos, nOffset) = maCells.position(itPos, nRow);
                if (itPos->type == sc::element_type_empty)
                {
                    // Skip empty cells.
                    nRow += itPos->size - nOffset;
                    continue;
                }

                for (; nOffset < itPos->size && nRow <= nRow2; ++nOffset, ++nRow)
                {
                    SvtScriptType nScript = rDocument.GetScriptType(nCol, nRow, nTab);
                    if (nScript == SvtScriptType::NONE)
                        nScript = ScGlobal::GetDefaultScriptType();

                    const ScPatternAttr* pPattern = GetPattern(nRow);
                    aOptions.bGetFont = (!ScPatternAttr::areSame(pPattern, pOldPattern) || nScript != SvtScriptType::NONE);
                    aOptions.aPattern.setScPatternAttr(pPattern);
                    pOldPattern = aOptions.aPattern.getScPatternAttr();
                    sal_uInt16 nThis = static_cast<sal_uInt16>(GetNeededSize(
                        nRow, pDev, nPPTX, nPPTY, rZoomX, rZoomY, true, aOptions, &pOldPattern));
                    if (nThis && (nThis > nWidth || !bFound))
                    {
                        nWidth = nThis;
                        bFound = true;
                    }
                }
            }
        }
    }

    if (bFound)
    {
        nWidth += 2;
        sal_uInt16 nTwips = static_cast<sal_uInt16>(
            std::min(nWidth / nPPTX, std::numeric_limits<sal_uInt16>::max() / 2.0));
        return nTwips;
    }
    else
        return nOldWidth;
}

static sal_uInt16 lcl_GetAttribHeight(const ScPatternAttr& rPattern, sal_uInt16 nFontHeightId,
                                      sal_uInt16 nMinHeight)
{
    const SvxFontHeightItem& rFontHeight =
        static_cast<const SvxFontHeightItem&>(rPattern.GetItem(nFontHeightId));

    sal_uInt16 nHeight = rFontHeight.GetHeight();
    nHeight *= 1.18;

    if ( rPattern.GetItem(ATTR_FONT_EMPHASISMARK).GetEmphasisMark() != FontEmphasisMark::NONE )
    {
        //  add height for emphasis marks
        //TODO: font metrics should be used instead
        nHeight += nHeight / 4;
    }

    const SvxMarginItem& rMargin = rPattern.GetItem(ATTR_MARGIN);

    nHeight += rMargin.GetTopMargin() + rMargin.GetBottomMargin();

    if (nHeight > STD_ROWHEIGHT_DIFF)
        nHeight -= STD_ROWHEIGHT_DIFF;

    if (nHeight < nMinHeight)
        nHeight = nMinHeight;

    return nHeight;
}

//  pHeight in Twips
//  optimize nMinHeight, nMinStart : with nRow >= nMinStart is at least nMinHeight
//  (is only evaluated with bStdAllowed)

void ScColumn::GetOptimalHeight(
    sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow, sal_uInt16 nMinHeight, SCROW nMinStart )
{
    ScDocument& rDocument = GetDoc();
    RowHeightsArray& rHeights = rCxt.getHeightArray();
    ScAttrIterator aIter( pAttrArray.get(), nStartRow, nEndRow, &rDocument.getCellAttributeHelper().getDefaultCellAttribute() );

    SCROW nStart = -1;
    SCROW nEnd = -1;
    SCROW nEditPos = 0;
    SCROW nNextEnd = 0;

    //  with conditional formatting, always consider the individual cells

    const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
    const sal_uInt16 nOptimalMinRowHeight = GetDoc().GetSheetOptimalMinRowHeight(nTab);
    while ( pPattern )
    {
        const ScMergeAttr*      pMerge = &pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = &pPattern->GetItem(ATTR_MERGE_FLAG);
        if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
        {
            //  do nothing - vertically with merged and overlapping,
            //        horizontally only with overlapped (invisible) -
            //        only one horizontal merged is always considered
        }
        else
        {
            bool bStdAllowed = (pPattern->GetCellOrientation() == SvxCellOrientation::Standard);
            bool bStdOnly = false;
            if (bStdAllowed)
            {
                bool bBreak = pPattern->GetItem(ATTR_LINEBREAK).GetValue() ||
                              (pPattern->GetItem( ATTR_HOR_JUSTIFY ).GetValue() ==
                                    SvxCellHorJustify::Block);
                bStdOnly = !bBreak;

                // conditional formatting: loop all cells
                if (bStdOnly &&
                    !pPattern->GetItem(ATTR_CONDITIONAL).GetCondFormatData().empty())
                {
                    bStdOnly = false;
                }

                // rotated text: loop all cells
                if ( bStdOnly && pPattern->GetItem(ATTR_ROTATE_VALUE).GetValue() )
                    bStdOnly = false;
            }

            if (bStdOnly)
            {
                bool bHasEditCells = HasEditCells(nStart,nEnd,nEditPos);
                // Call to HasEditCells() may change pattern due to
                // calculation, => sync always.
                // We don't know which row changed first, but as pPattern
                // covered nStart to nEnd we can pick nStart. Worst case we
                // have to repeat that for every row in range if every row
                // changed.
                pPattern = aIter.Resync( nStart, nStart, nEnd);
                if (bHasEditCells && nEnd < nEditPos)
                    bHasEditCells = false;              // run into that again
                if (bHasEditCells)                      // includes mixed script types
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
            }

            sc::SingleColumnSpanSet aSpanSet(GetDoc().GetSheetLimits());
            aSpanSet.scan(*this, nStart, nEnd);
            sc::SingleColumnSpanSet::SpansType aSpans;
            aSpanSet.getSpans(aSpans);

            if (bStdAllowed)
            {
                sal_uInt16 nLatHeight = 0;
                sal_uInt16 nCjkHeight = 0;
                sal_uInt16 nCtlHeight = 0;
                sal_uInt16 nDefHeight;
                SvtScriptType nDefScript = ScGlobal::GetDefaultScriptType();
                if ( nDefScript == SvtScriptType::ASIAN )
                    nDefHeight = nCjkHeight = lcl_GetAttribHeight(*pPattern, ATTR_CJK_FONT_HEIGHT,
                                                                  nOptimalMinRowHeight);
                else if ( nDefScript == SvtScriptType::COMPLEX )
                    nDefHeight = nCtlHeight = lcl_GetAttribHeight(*pPattern, ATTR_CTL_FONT_HEIGHT,
                                                                  nOptimalMinRowHeight);
                else
                    nDefHeight = nLatHeight = lcl_GetAttribHeight(*pPattern, ATTR_FONT_HEIGHT,
                                                                  nOptimalMinRowHeight);

                //  if everything below is already larger, the loop doesn't have to
                //  be run again
                SCROW nStdEnd = nEnd;
                if ( nDefHeight <= nMinHeight && nStdEnd >= nMinStart )
                    nStdEnd = (nMinStart>0) ? nMinStart-1 : 0;

                if (nStart <= nStdEnd)
                {
                    SCROW nRow = nStart;
                    for (;;)
                    {
                        size_t nIndex;
                        SCROW nRangeEnd;
                        sal_uInt16 nRangeHeight = rHeights.GetValue(nRow, nIndex, nRangeEnd);
                        if (nRangeHeight < nDefHeight)
                            rHeights.SetValue(nRow, std::min(nRangeEnd, nStdEnd), nDefHeight);
                        nRow = nRangeEnd + 1;
                        if (nRow > nStdEnd)
                            break;
                    }
                }

                if ( bStdOnly )
                {
                    //  if cells are not handled individually below,
                    //  check for cells with different script type
                    sc::CellTextAttrStoreType::iterator itAttr = maCellTextAttrs.begin();
                    sc::CellStoreType::iterator itCells = maCells.begin();
                    for (const auto& rSpan : aSpans)
                    {
                        for (SCROW nRow = rSpan.mnRow1; nRow <= rSpan.mnRow2; ++nRow)
                        {
                            SvtScriptType nScript = GetRangeScriptType(itAttr, nRow, nRow, itCells);
                            if (nScript == nDefScript)
                                continue;

                            if ( nScript == SvtScriptType::ASIAN )
                            {
                                if ( nCjkHeight == 0 )
                                    nCjkHeight = lcl_GetAttribHeight(*pPattern,
                                                                     ATTR_CJK_FONT_HEIGHT,
                                                                     nOptimalMinRowHeight);
                                if (nCjkHeight > rHeights.GetValue(nRow))
                                    rHeights.SetValue(nRow, nRow, nCjkHeight);
                            }
                            else if ( nScript == SvtScriptType::COMPLEX )
                            {
                                if ( nCtlHeight == 0 )
                                    nCtlHeight = lcl_GetAttribHeight(*pPattern,
                                                                     ATTR_CTL_FONT_HEIGHT,
                                                                     nOptimalMinRowHeight);
                                if (nCtlHeight > rHeights.GetValue(nRow))
                                    rHeights.SetValue(nRow, nRow, nCtlHeight);
                            }
                            else
                            {
                                if ( nLatHeight == 0 )
                                    nLatHeight = lcl_GetAttribHeight(*pPattern, ATTR_FONT_HEIGHT,
                                                                     nOptimalMinRowHeight);
                                if (nLatHeight > rHeights.GetValue(nRow))
                                    rHeights.SetValue(nRow, nRow, nLatHeight);
                            }
                        }
                    }
                }
            }

            if (!bStdOnly)                      // search covered cells
            {
                ScNeededSizeOptions aOptions;
                CellAttributeHolder aOldPattern;

                for (const auto& rSpan : aSpans)
                {
                    for (SCROW nRow = rSpan.mnRow1; nRow <= rSpan.mnRow2; ++nRow)
                    {
                        //  only calculate the cell height when it's used later (#37928#)

                        if (rCxt.isForceAutoSize() || !(rDocument.GetRowFlags(nRow, nTab) & CRFlags::ManualSize) )
                        {
                            aOptions.aPattern.setScPatternAttr(pPattern);
                            aOldPattern.setScPatternAttr(aOptions.aPattern.getScPatternAttr());
                            sal_uInt16 nHeight = static_cast<sal_uInt16>(
                                std::min(
                                    GetNeededSize( nRow, rCxt.getOutputDevice(), rCxt.getPPTX(), rCxt.getPPTY(),
                                                   rCxt.getZoomX(), rCxt.getZoomY(), false, aOptions,
                                                   &pPattern) / rCxt.getPPTY(),
                                    double(std::numeric_limits<sal_uInt16>::max())));
                            if (nHeight > rHeights.GetValue(nRow))
                                rHeights.SetValue(nRow, nRow, nHeight);

                            // Pattern changed due to calculation? => sync.
                            if (!ScPatternAttr::areSame(pPattern, aOldPattern.getScPatternAttr()))
                            {
                                pPattern = aIter.Resync( nRow, nStart, nEnd);
                                nNextEnd = 0;
                            }
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
    ScDocument& rDocument = GetDoc();
    sc::CellStoreType::const_iterator it = maCells.position(nRow).first;
    mdds::mtv::element_t eType = it->type;
    if (!bInSel && it != maCells.end() && eType != sc::element_type_empty)
    {
        if ( (eType == sc::element_type_string || eType == sc::element_type_edittext) &&
             !(HasAttrib( nRow, nRow, HasAttrFlags::Protected) &&
               rDocument.IsTabProtected(nTab)) )
            return true;
    }
    if (bInSel)
    {
        SCROW lastDataPos = GetLastDataPos();
        for (;;)
        {
            nRow = rData.GetNextMarked(nCol, nRow, false);
            if (!rDocument.ValidRow(nRow) || nRow > lastDataPos )
            {
                nRow = GetDoc().MaxRow()+1;
                return false;
            }
            else
            {
                it = maCells.position(it, nRow).first;
                eType = it->type;
                if ( (eType == sc::element_type_string || eType == sc::element_type_edittext) &&
                     !(HasAttrib( nRow, nRow, HasAttrFlags::Protected) &&
                       rDocument.IsTabProtected(nTab)) )
                    return true;
                else
                    nRow++;
            }
        }
    }
    else
    {
        while (GetNextDataPos(nRow))
        {
            it = maCells.position(it, nRow).first;
            eType = it->type;
            if ( (eType == sc::element_type_string || eType == sc::element_type_edittext) &&
                 !(HasAttrib( nRow, nRow, HasAttrFlags::Protected) &&
                   rDocument.IsTabProtected(nTab)) )
                return true;
            else
                nRow++;
        }
        nRow = GetDoc().MaxRow()+1;
        return false;
    }
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

        StrEntry(SCROW nRow, OUString aStr) : mnRow(nRow), maStr(std::move(aStr)) {}
    };

    std::vector<StrEntry> maStrEntries;
    ScDocument* mpDoc;

    StrEntries(sc::CellStoreType& rCells, ScDocument* pDoc) : mrCells(rCells), mpDoc(pDoc) {}

public:
    void commitStrings()
    {
        svl::SharedStringPool& rPool = mpDoc->GetSharedStringPool();
        sc::CellStoreType::iterator it = mrCells.begin();
        for (const auto& rStrEntry : maStrEntries)
            it = mrCells.set(it, rStrEntry.mnRow, rPool.intern(rStrEntry.maStr));
    }
};

class RemoveEditAttribsHandler : public StrEntries
{
    std::unique_ptr<ScFieldEditEngine> mpEngine;

public:
    RemoveEditAttribsHandler(sc::CellStoreType& rCells, ScDocument* pDoc) : StrEntries(rCells, pDoc) {}

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
            //  EEControlBits::ONLINESPELLING if there are errors already
            mpEngine->SetControlWord(mpEngine->GetControlWord() | EEControlBits::ONLINESPELLING);
            mpDoc->ApplyAsianEditSettings(*mpEngine);
        }
        mpEngine->SetTextCurrentDefaults(*pObj);
        sal_Int32 nParCount = mpEngine->GetParagraphCount();
        for (sal_Int32 nPar=0; nPar<nParCount; nPar++)
        {
            mpEngine->RemoveCharAttribs(nPar);
            const SfxItemSet& rOld = mpEngine->GetParaAttribs(nPar);
            if ( rOld.Count() )
            {
                SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   // empty
                mpEngine->SetParaAttribs( nPar, aNew );
            }
        }
        //  change URL field to text (not possible otherwise, thus pType=0)
        mpEngine->RemoveFields();

        bool bSpellErrors = mpEngine->HasOnlineSpellErrors();
        bool bNeedObject = bSpellErrors || nParCount>1;         // keep errors/paragraphs
        //  ScEditAttrTester is not needed anymore, arrays are gone

        if (bNeedObject)                                      // remains edit cell
        {
            EEControlBits nCtrl = mpEngine->GetControlWord();
            EEControlBits nWantBig = bSpellErrors ? EEControlBits::ALLOWBIGOBJS : EEControlBits::NONE;
            if ( ( nCtrl & EEControlBits::ALLOWBIGOBJS ) != nWantBig )
                mpEngine->SetControlWord( (nCtrl & ~EEControlBits::ALLOWBIGOBJS) | nWantBig );

            // Overwrite the existing object.
            delete pObj;
            pObj = mpEngine->CreateTextObject().release();
        }
        else                                            // create String
        {
            // Store the string replacement for later commits.
            OUString aText = ScEditUtil::GetSpaceDelimitedString(*mpEngine);
            maStrEntries.emplace_back(nRow, aText);
        }
    }
};

class TestTabRefAbsHandler
{
    SCTAB mnTab;
    bool mbTestResult;
public:
    explicit TestTabRefAbsHandler(SCTAB nTab) : mnTab(nTab), mbTestResult(false) {}

    void operator() (size_t /*nRow*/, const ScFormulaCell* pCell)
    {
        if (const_cast<ScFormulaCell*>(pCell)->TestTabRefAbs(mnTab))
            mbTestResult = true;
    }

    bool getTestResult() const { return mbTestResult; }
};

}

void ScColumn::RemoveEditAttribs( sc::ColumnBlockPosition& rBlockPos, SCROW nStartRow, SCROW nEndRow )
{
    RemoveEditAttribsHandler aFunc(maCells, &GetDoc());

    rBlockPos.miCellPos = sc::ProcessEditText(
        rBlockPos.miCellPos, maCells, nStartRow, nEndRow, aFunc);

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

bool ScColumn::IsEmptyData(SCROW nStartRow, SCROW nEndRow) const
{
    // simple case
    if (maCells.block_size() == 1 && maCells.begin()->type == sc::element_type_empty)
        return true;

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

bool ScColumn::IsNotesEmptyBlock(SCROW nStartRow, SCROW nEndRow) const
{
    std::pair<sc::CellNoteStoreType::const_iterator,size_t> aPos = maCellNotes.position(nStartRow);
    sc::CellNoteStoreType::const_iterator it = aPos.first;
    if (it == maCellNotes.end())
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
            // Determine the length of empty tail segment.
            size_t nLength = nEndRow - nStartRow + 1;
            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nEndRow);
            sc::CellStoreType::const_iterator it = aPos.first;
            if (it->type != sc::element_type_empty)
                // end row is already not empty.
                return 0;

            // length of this empty block from the tip to the end row position.
            size_t nThisLen = aPos.second + 1;
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
        return GetDoc().MaxRow();

    return GetDoc().MaxRow() - static_cast<SCROW>(it->size);
}

SCROW ScColumn::GetLastDataPos( SCROW nLastRow, ScDataAreaExtras* pDataAreaExtras ) const
{
    nLastRow = std::min( nLastRow, GetDoc().MaxRow());

    if (pDataAreaExtras && pDataAreaExtras->mnEndRow < nLastRow)
    {
        // Check in order of likeliness.
        if (    (pDataAreaExtras->mbCellFormats && HasVisibleAttrIn(nLastRow, nLastRow)) ||
                (pDataAreaExtras->mbCellNotes && !IsNotesEmptyBlock(nLastRow, nLastRow)) ||
                (pDataAreaExtras->mbCellDrawObjects && !IsDrawObjectsEmptyBlock(nLastRow, nLastRow)))
            pDataAreaExtras->mnEndRow = nLastRow;
    }

    sc::CellStoreType::const_position_type aPos = maCells.position(nLastRow);

    if (aPos.first->type != sc::element_type_empty)
        return nLastRow;

    if (aPos.first == maCells.begin())
        // This is the first block, and is empty.
        return 0;

    return static_cast<SCROW>(aPos.first->position - 1);
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

    // This is the first cell in a non-empty block. Move back to the previous block.
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

bool ScColumn::TrimEmptyBlocks(SCROW& rRowStart, SCROW& rRowEnd) const
{
    assert(rRowStart <= rRowEnd);
    SCROW nRowStartNew = rRowStart, nRowEndNew = rRowEnd;

    // Trim down rRowStart first
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rRowStart);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return false;

    if (it->type == sc::element_type_empty)
    {
        // This block is empty. Skip ahead to the next block (if exists).
        nRowStartNew += it->size - aPos.second;
        if (nRowStartNew > rRowEnd)
            return false;
        ++it;
        if (it == maCells.end())
            // No more next block.
            return false;
    }

    // Trim up rRowEnd next
    aPos = maCells.position(rRowEnd);
    it = aPos.first;
    if (it == maCells.end())
    {
        rRowStart = nRowStartNew;
        return true; // Because trimming of rRowStart is ok
    }

    if (it->type == sc::element_type_empty)
    {
        // rRowEnd cannot be in the first block which is empty !
        assert(it != maCells.begin());
        // This block is empty. Skip to the previous block (it exists).
        nRowEndNew -= aPos.second + 1; // Last row position of the previous block.
        assert(nRowStartNew <= nRowEndNew);
    }

    rRowStart = nRowStartNew;
    rRowEnd = nRowEndNew;
    return true;
}

SCROW ScColumn::FindNextVisibleRow(SCROW nRow, bool bForward) const
{
    if(bForward)
    {
        nRow++;
        SCROW nEndRow = 0;
        bool bHidden = GetDoc().RowHidden(nRow, nTab, nullptr, &nEndRow);
        if(bHidden)
            return std::min<SCROW>(GetDoc().MaxRow(), nEndRow + 1);
        else
            return nRow;
    }
    else
    {
        nRow--;
        SCROW nStartRow = GetDoc().MaxRow();
        bool bHidden = GetDoc().RowHidden(nRow, nTab, &nStartRow);
        if(bHidden)
            return std::max<SCROW>(0, nStartRow - 1);
        else
            return nRow;
    }
}

SCROW ScColumn::FindNextVisibleRowWithContent(
    sc::CellStoreType::const_iterator& itPos, SCROW nRow, bool bForward) const
{
    ScDocument& rDocument = GetDoc();
    if (bForward)
    {
        do
        {
            nRow++;
            SCROW nEndRow = 0;
            bool bHidden = rDocument.RowHidden(nRow, nTab, nullptr, &nEndRow);
            if (bHidden)
            {
                nRow = nEndRow + 1;
                if(nRow >= GetDoc().MaxRow())
                    return GetDoc().MaxRow();
            }

            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(itPos, nRow);
            itPos = aPos.first;
            if (itPos == maCells.end())
                // Invalid row.
                return GetDoc().MaxRow();

            if (itPos->type != sc::element_type_empty)
                return nRow;

            // Move to the last cell of the current empty block.
            nRow += itPos->size - aPos.second - 1;
        }
        while (nRow < GetDoc().MaxRow());

        return GetDoc().MaxRow();
    }

    do
    {
        nRow--;
        SCROW nStartRow = GetDoc().MaxRow();
        bool bHidden = rDocument.RowHidden(nRow, nTab, &nStartRow);
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
    // Remove cached values. Given how often this function is called and how (not that) often
    // the cached values are used, it should be more efficient to just discard everything
    // instead of trying to figure out each time exactly what to discard.
    GetDoc().DiscardFormulaGroupContext();

    // TODO: Update column's "last updated" timestamp here.

    assert(sal::static_int_cast<SCROW>(maCells.size()) == GetDoc().GetMaxRowCount()
        && "Size of the cell array is incorrect." );

    assert(sal::static_int_cast<SCROW>(maCellTextAttrs.size()) == GetDoc().GetMaxRowCount()
        && "Size of the cell text attribute array is incorrect.");

    assert(sal::static_int_cast<SCROW>(maBroadcasters.size()) == GetDoc().GetMaxRowCount()
        && "Size of the broadcaster array is incorrect.");

#if DEBUG_COLUMN_STORAGE
    // Make sure that these two containers are synchronized wrt empty segments.
    auto lIsEmptyType = [](const auto& rElement) { return rElement.type == sc::element_type_empty; };
    // Move to the first empty blocks.
    auto itCell = std::find_if(maCells.begin(), maCells.end(), lIsEmptyType);
    auto itAttr = std::find_if(maCellTextAttrs.begin(), maCellTextAttrs.end(), lIsEmptyType);

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
        itCell = std::find_if(itCell, maCells.end(), lIsEmptyType);

        ++itAttr;
        itAttr = std::find_if(itAttr, maCellTextAttrs.end(), lIsEmptyType);
    }
#endif
}

#if DUMP_COLUMN_STORAGE

namespace {

#define DUMP_FORMULA_RESULTS 0

struct ColumnStorageDumper
{
    const ScDocument& mrDoc;

    ColumnStorageDumper( const ScDocument& rDoc ) : mrDoc(rDoc) {}

    void operator() (const sc::CellStoreType::value_type& rNode) const
    {
        switch (rNode.type)
        {
            case sc::element_type_numeric:
                cout << "  * numeric block (pos=" << rNode.position << ", length=" << rNode.size << ")" << endl;
                break;
            case sc::element_type_string:
                cout << "  * string block (pos=" << rNode.position << ", length=" << rNode.size << ")" << endl;
                break;
            case sc::element_type_edittext:
                cout << "  * edit-text block (pos=" << rNode.position << ", length=" << rNode.size << ")" << endl;
                break;
            case sc::element_type_formula:
                dumpFormulaBlock(rNode);
                break;
            case sc::element_type_empty:
                cout << "  * empty block (pos=" << rNode.position << ", length=" << rNode.size << ")" << endl;
                break;
            default:
                cout << "  * unknown block" << endl;
        }
    }

    void dumpFormulaBlock(const sc::CellStoreType::value_type& rNode) const
    {
        cout << "  * formula block (pos=" << rNode.position << ", length=" << rNode.size << ")" << endl;
        sc::formula_block::const_iterator it = sc::formula_block::begin(*rNode.data);
        sc::formula_block::const_iterator itEnd = sc::formula_block::end(*rNode.data);

        for (; it != itEnd; ++it)
        {
            const ScFormulaCell* pCell = *it;
            if (!pCell->IsShared())
            {
                cout << "    * row " << pCell->aPos.Row() << " not shared" << endl;
                printFormula(pCell);
                printResult(pCell);
                continue;
            }

            if (pCell->GetSharedTopRow() != pCell->aPos.Row())
            {
                cout << "    * row " << pCell->aPos.Row() << " shared with top row "
                    << pCell->GetSharedTopRow() << " with length " << pCell->GetSharedLength()
                    << endl;
                continue;
            }

            SCROW nLen = pCell->GetSharedLength();
            cout << "    * group: start=" << pCell->aPos.Row() << ", length=" << nLen << endl;
            printFormula(pCell);
            printResult(pCell);

            if (nLen > 1)
            {
                for (SCROW i = 0; i < nLen-1; ++i, ++it)
                {
                    pCell = *it;
                    printResult(pCell);
                }
            }
        }
    }

    void printFormula(const ScFormulaCell* pCell) const
    {
        sc::TokenStringContext aCxt(mrDoc, mrDoc.GetGrammar());
        OUString aFormula = pCell->GetCode()->CreateString(aCxt, pCell->aPos);
        cout << "      * formula: " << aFormula << endl;
    }

#if DUMP_FORMULA_RESULTS
    void printResult(const ScFormulaCell* pCell) const
    {
        sc::FormulaResultValue aRes = pCell->GetResult();
        cout << "    * result: ";
        switch (aRes.meType)
        {
            case sc::FormulaResultValue::Value:
                cout << aRes.mfValue << " (type: value)";
                break;
            case sc::FormulaResultValue::String:
                cout << "'" << aRes.maString.getString() << "' (type: string)";
                break;
            case sc::FormulaResultValue::Error:
                cout << "error (" << static_cast<int>(aRes.mnError) << ")";
                break;
            case sc::FormulaResultValue::Invalid:
                cout << "invalid";
                break;
        }

        cout << endl;
    }
#else
    void printResult(const ScFormulaCell*) const
    {
        (void) this; /* loplugin:staticmethods */
    }
#endif
};

}

void ScColumn::DumpColumnStorage() const
{
    cout << "-- table: " << nTab << "; column: " << nCol << endl;
    std::for_each(maCells.begin(), maCells.end(), ColumnStorageDumper(GetDoc()));
    cout << "--" << endl;
}
#endif

void ScColumn::CopyCellTextAttrsToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol) const
{
    rDestCol.maCellTextAttrs.set_empty(nRow1, nRow2); // Empty the destination range first.

    sc::CellTextAttrStoreType::const_iterator itBlk = maCellTextAttrs.begin(), itBlkEnd = maCellTextAttrs.end();

    // Locate the top row position.
    size_t nBlockStart = 0, nRowPos = static_cast<size_t>(nRow1);
    itBlk = std::find_if(itBlk, itBlkEnd, [&nRowPos, &nBlockStart](const auto& rAttr) {
        return nBlockStart <= nRowPos && nRowPos < nBlockStart + rAttr.size; });

    if (itBlk == itBlkEnd)
        // Specified range not found. Bail out.
        return;

    size_t nBlockEnd;
    size_t nOffsetInBlock = nRowPos - nBlockStart;

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

class CopyCellNotesHandler
{
    ScColumn& mrDestCol;
    sc::CellNoteStoreType& mrDestNotes;
    sc::CellNoteStoreType::iterator miPos;
    SCTAB mnSrcTab;
    SCCOL mnSrcCol;
    SCTAB mnDestTab;
    SCCOL mnDestCol;
    SCROW mnDestOffset; /// Add this to the source row position to get the destination row.
    bool mbCloneCaption;

public:
    CopyCellNotesHandler( const ScColumn& rSrcCol, ScColumn& rDestCol, SCROW nDestOffset, bool bCloneCaption ) :
        mrDestCol(rDestCol),
        mrDestNotes(rDestCol.GetCellNoteStore()),
        miPos(mrDestNotes.begin()),
        mnSrcTab(rSrcCol.GetTab()),
        mnSrcCol(rSrcCol.GetCol()),
        mnDestTab(rDestCol.GetTab()),
        mnDestCol(rDestCol.GetCol()),
        mnDestOffset(nDestOffset),
        mbCloneCaption(bCloneCaption) {}

    void operator() ( size_t nRow, const ScPostIt* p )
    {
        SCROW nDestRow = nRow + mnDestOffset;
        ScAddress aSrcPos(mnSrcCol, nRow, mnSrcTab);
        ScAddress aDestPos(mnDestCol, nDestRow, mnDestTab);
        ScPostIt* pNew = p->Clone(aSrcPos, mrDestCol.GetDoc(), aDestPos, mbCloneCaption).release();
        miPos = mrDestNotes.set(miPos, nDestRow, pNew);
        // Notify our LOK clients also
        ScDocShell::LOKCommentNotify(LOKCommentNotificationType::Add, mrDestCol.GetDoc(), aDestPos, pNew);
    }
};

}

void ScColumn::CopyCellNotesToDocument(
    SCROW nRow1, SCROW nRow2, ScColumn& rDestCol, bool bCloneCaption, SCROW nRowOffsetDest ) const
{
    if (IsNotesEmptyBlock(nRow1, nRow2))
        // The column has no cell notes to copy between specified rows.
        return;

    ScDrawLayer *pDrawLayer = rDestCol.GetDoc().GetDrawLayer();
    bool bWasLocked = bool();
    if (pDrawLayer)
    {
        // Avoid O(n^2) by temporary locking SdrModel which disables broadcasting.
        // Each cell note adds undo listener, and all of them would be woken up in ScPostIt::CreateCaption.
        bWasLocked = pDrawLayer->isLocked();
        pDrawLayer->setLock(true);
    }
    CopyCellNotesHandler aFunc(*this, rDestCol, nRowOffsetDest, bCloneCaption);
    sc::ParseNote(maCellNotes.begin(), maCellNotes, nRow1, nRow2, aFunc);
    if (pDrawLayer)
        pDrawLayer->setLock(bWasLocked);
}

void ScColumn::DuplicateNotes(SCROW nStartRow, size_t nDataSize, ScColumn& rDestCol, sc::ColumnBlockPosition& maDestBlockPos,
                              bool bCloneCaption, SCROW nRowOffsetDest ) const
{
    CopyCellNotesToDocument(nStartRow, nStartRow + nDataSize -1, rDestCol, bCloneCaption, nRowOffsetDest);
    maDestBlockPos.miCellNotePos = rDestCol.maCellNotes.begin();
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

void ScColumn::PrepareBroadcastersForDestruction()
{
    for (auto& rBroadcaster : maBroadcasters)
    {
        if (rBroadcaster.type == sc::element_type_broadcaster)
        {
            sc::broadcaster_block::iterator it = sc::broadcaster_block::begin(*rBroadcaster.data);
            sc::broadcaster_block::iterator itEnd = sc::broadcaster_block::end(*rBroadcaster.data);
            for (; it != itEnd; ++it)
                (*it)->PrepareForDestruction();
        }
    }
}

namespace
{
struct BroadcasterNoListenersPredicate
{
    bool operator()( size_t, const SvtBroadcaster* broadcaster )
    {
        return !broadcaster->HasListeners();
    }
};

}

void ScColumn::DeleteEmptyBroadcasters()
{
    if(!mbEmptyBroadcastersPending)
        return;
    // Clean up after ScDocument::EnableDelayDeletingBroadcasters().
    BroadcasterNoListenersPredicate predicate;
    sc::SetElementsToEmpty1<sc::broadcaster_block>( maBroadcasters, predicate );
    mbEmptyBroadcastersPending = false;
}

// Sparklines

namespace
{

class DeletingSparklinesHandler
{
    ScDocument& m_rDocument;
    SCTAB m_nTab;

public:
    DeletingSparklinesHandler(ScDocument& rDocument, SCTAB nTab)
        : m_rDocument(rDocument)
        , m_nTab(nTab)
    {}

    void operator() (size_t /*nRow*/, const sc::SparklineCell* pCell)
    {
        auto* pList = m_rDocument.GetSparklineList(m_nTab);
        pList->removeSparkline(pCell->getSparkline());
    }
};

} // end anonymous ns

sc::SparklineCell* ScColumn::GetSparklineCell(SCROW nRow)
{
    return maSparklines.get<sc::SparklineCell*>(nRow);
}

void ScColumn::CreateSparklineCell(SCROW nRow, std::shared_ptr<sc::Sparkline> const& pSparkline)
{
    auto* pList = GetDoc().GetSparklineList(GetTab());
    pList->addSparkline(pSparkline);
    maSparklines.set(nRow, new sc::SparklineCell(pSparkline));
}

void ScColumn::DeleteSparklineCells(sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2)
{
    DeletingSparklinesHandler aFunction(GetDoc(), nTab);
    sc::ParseSparkline(maSparklines.begin(), maSparklines, nRow1, nRow2, aFunction);

    rBlockPos.miSparklinePos = maSparklines.set_empty(rBlockPos.miSparklinePos, nRow1, nRow2);
}

bool ScColumn::DeleteSparkline(SCROW nRow)
{
    if (!GetDoc().ValidRow(nRow))
        return false;

    DeletingSparklinesHandler aFunction(GetDoc(), nTab);
    sc::ParseSparkline(maSparklines.begin(), maSparklines, nRow, nRow, aFunction);

    maSparklines.set_empty(nRow, nRow);
    return true;
}

bool ScColumn::IsSparklinesEmptyBlock(SCROW nStartRow, SCROW nEndRow) const
{
    std::pair<sc::SparklineStoreType::const_iterator,size_t> aPos = maSparklines.position(nStartRow);
    sc::SparklineStoreType::const_iterator it = aPos.first;
    if (it == maSparklines.end())
        return false;

    if (it->type != sc::element_type_empty)
        return false;

    // start position of next block which is not empty.
    SCROW nNextRow = nStartRow + it->size - aPos.second;
    return nEndRow < nNextRow;
}

namespace
{

class CopySparklinesHandler
{
    ScColumn& mrDestColumn;
    sc::SparklineStoreType& mrDestSparkline;
    sc::SparklineStoreType::iterator miDestPosition;
    SCROW mnDestOffset;

public:
    CopySparklinesHandler(ScColumn& rDestColumn, SCROW nDestOffset)
        : mrDestColumn(rDestColumn)
        , mrDestSparkline(mrDestColumn.GetSparklineStore())
        , miDestPosition(mrDestSparkline.begin())
        , mnDestOffset(nDestOffset)
    {}

    void operator() (size_t nRow, const sc::SparklineCell* pCell)
    {
        SCROW nDestRow = nRow + mnDestOffset;

        auto const& pSparkline = pCell->getSparkline();
        auto const& pGroup = pCell->getSparklineGroup();

        auto& rDestDoc = mrDestColumn.GetDoc();
        auto pDestinationGroup = rDestDoc.SearchSparklineGroup(pGroup->getID());
        if (!pDestinationGroup)
            pDestinationGroup = std::make_shared<sc::SparklineGroup>(*pGroup); // Copy the group
        auto pNewSparkline = std::make_shared<sc::Sparkline>(mrDestColumn.GetCol(), nDestRow, pDestinationGroup);
        pNewSparkline->setInputRange(pSparkline->getInputRange());

        auto* pList = rDestDoc.GetSparklineList(mrDestColumn.GetTab());
        pList->addSparkline(pNewSparkline);

        miDestPosition = mrDestSparkline.set(miDestPosition, nDestRow, new sc::SparklineCell(pNewSparkline));
    }
};

}

void ScColumn::CopyCellSparklinesToDocument(SCROW nRow1, SCROW nRow2, ScColumn& rDestCol, SCROW nRowOffsetDest) const
{
    if (IsSparklinesEmptyBlock(nRow1, nRow2))
        // The column has no cell sparklines to copy between specified rows.
        return;

    CopySparklinesHandler aFunctor(rDestCol, nRowOffsetDest);
    sc::ParseSparkline(maSparklines.begin(), maSparklines, nRow1, nRow2, aFunctor);
}

void ScColumn::DuplicateSparklines(SCROW nStartRow, size_t nDataSize, ScColumn& rDestCol,
                             sc::ColumnBlockPosition& rDestBlockPos, SCROW nRowOffsetDest) const
{
    CopyCellSparklinesToDocument(nStartRow, nStartRow + nDataSize - 1, rDestCol, nRowOffsetDest);
    rDestBlockPos.miSparklinePos = rDestCol.maSparklines.begin();
}

bool ScColumn::HasSparklines() const
{
    if (maSparklines.block_size() == 1 && maSparklines.begin()->type == sc::element_type_empty)
        return false; // all elements are empty
    return true; // otherwise some must be sparklines
}

SCROW ScColumn::GetSparklinesMaxRow() const
{
    SCROW maxRow = 0;
    for (const auto& rSparkline : maSparklines)
    {
        if (rSparkline.type == sc::element_type_sparkline)
            maxRow = rSparkline.position + rSparkline.size - 1;
    }
    return maxRow;
}

SCROW ScColumn::GetSparklinesMinRow() const
{
    SCROW minRow = 0;
    sc::SparklineStoreType::const_iterator it = std::find_if(maSparklines.begin(), maSparklines.end(),
        [](const auto& rSparkline)
        {
            return rSparkline.type == sc::element_type_sparkline;
        });
    if (it != maSparklines.end())
        minRow = it->position;
    return minRow;
}

// Notes

ScPostIt* ScColumn::GetCellNote(SCROW nRow)
{
    return maCellNotes.get<ScPostIt*>(nRow);
}

const ScPostIt* ScColumn::GetCellNote(SCROW nRow) const
{
    return maCellNotes.get<ScPostIt*>(nRow);
}

const ScPostIt* ScColumn::GetCellNote( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow ) const
{
    sc::CellNoteStoreType::const_position_type aPos = maCellNotes.position(rBlockPos.miCellNotePos, nRow);
    rBlockPos.miCellNotePos = aPos.first;

    if (aPos.first->type != sc::element_type_cellnote)
        return nullptr;

    return sc::cellnote_block::at(*aPos.first->data, aPos.second);
}

ScPostIt* ScColumn::GetCellNote( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow )
{
    return const_cast<ScPostIt*>(const_cast<const ScColumn*>(this)->GetCellNote( rBlockPos, nRow ));
}

void ScColumn::SetCellNote(SCROW nRow, std::unique_ptr<ScPostIt> pNote)
{
    //pNote->UpdateCaptionPos(ScAddress(nCol, nRow, nTab)); // TODO notes useful ? slow import with many notes
    maCellNotes.set(nRow, pNote.release());
}

namespace {
    class CellNoteHandler
    {
        const ScDocument& m_rDocument;
        const ScAddress m_aAddress; // 'incomplete' address consisting of tab, column
        const bool m_bForgetCaptionOwnership;

    public:
        CellNoteHandler(const ScDocument& rDocument, const ScAddress& rPos, bool bForgetCaptionOwnership) :
            m_rDocument(rDocument),
            m_aAddress(rPos),
            m_bForgetCaptionOwnership(bForgetCaptionOwnership) {}

        void operator() ( size_t nRow, ScPostIt* p )
        {
            if (m_bForgetCaptionOwnership)
                p->ForgetCaption();

            // Create a 'complete' address object
            ScAddress aAddr(m_aAddress);
            aAddr.SetRow(nRow);
            // Notify our LOK clients
            ScDocShell::LOKCommentNotify(LOKCommentNotificationType::Remove, m_rDocument, aAddr, p);
        }
    };
} // anonymous namespace

void ScColumn::CellNotesDeleting(SCROW nRow1, SCROW nRow2, bool bForgetCaptionOwnership)
{
    ScAddress aAddr(nCol, 0, nTab);
    CellNoteHandler aFunc(GetDoc(), aAddr, bForgetCaptionOwnership);
    sc::ParseNote(maCellNotes.begin(), maCellNotes, nRow1, nRow2, aFunc);
}

void ScColumn::DeleteCellNotes( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2, bool bForgetCaptionOwnership )
{
    CellNotesDeleting(nRow1, nRow2, bForgetCaptionOwnership);

    rBlockPos.miCellNotePos =
        maCellNotes.set_empty(rBlockPos.miCellNotePos, nRow1, nRow2);
}

bool ScColumn::HasCellNotes() const
{
    return mnBlkCountCellNotes != 0;
}

SCROW ScColumn::GetCellNotesMaxRow() const
{
    // hypothesis : the column has cell notes (should be checked before)
    SCROW maxRow = 0;
    for (const auto& rCellNote : maCellNotes)
    {
        if (rCellNote.type == sc::element_type_cellnote)
            maxRow = rCellNote.position + rCellNote.size -1;
    }
    return maxRow;
}
SCROW ScColumn::GetCellNotesMinRow() const
{
    // hypothesis : the column has cell notes (should be checked before)
    SCROW minRow = 0;
    sc::CellNoteStoreType::const_iterator it = std::find_if(maCellNotes.begin(), maCellNotes.end(),
        [](const auto& rCellNote) { return rCellNote.type == sc::element_type_cellnote; });
    if (it != maCellNotes.end())
        minRow = it->position;
    return minRow;
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

SvtScriptType ScColumn::GetScriptType( SCROW nRow ) const
{
    if (!GetDoc().ValidRow(nRow) || maCellTextAttrs.is_empty(nRow))
        return SvtScriptType::NONE;

    return maCellTextAttrs.get<sc::CellTextAttr>(nRow).mnScriptType;
}

SvtScriptType ScColumn::GetRangeScriptType(
    sc::CellTextAttrStoreType::iterator& itPos, SCROW nRow1, SCROW nRow2, const sc::CellStoreType::iterator& itrCells_ )
{
    if (!GetDoc().ValidRow(nRow1) || !GetDoc().ValidRow(nRow2) || nRow1 > nRow2)
        return SvtScriptType::NONE;

    SCROW nRow = nRow1;
    std::pair<sc::CellTextAttrStoreType::iterator,size_t> aRet =
        maCellTextAttrs.position(itPos, nRow1);

    itPos = aRet.first; // Track the position of cell text attribute array.
    sc::CellStoreType::iterator itrCells = itrCells_;

    SvtScriptType nScriptType = SvtScriptType::NONE;
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
            if (UpdateScriptType(rVal, nRow, itrCells))
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
            if (UpdateScriptType(rVal, nRow, itrCells))
                bUpdated = true;

            nScriptType |= rVal.mnScriptType;
        }
    }

    if (bUpdated)
        CellStorageModified();

    return nScriptType;
}

void ScColumn::SetScriptType( SCROW nRow, SvtScriptType nType )
{
    if (!GetDoc().ValidRow(nRow))
        return;

    sc::CellTextAttrStoreType::position_type aPos = maCellTextAttrs.position(nRow);
    if (aPos.first->type != sc::element_type_celltextattr)
        // Set new value only when the slot is already set.
        return;

    sc::celltextattr_block::at(*aPos.first->data, aPos.second).mnScriptType = nType;
    CellStorageModified();
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
        case sc::element_type_string:
        {
            const svl::SharedString& rSS = sc::string_block::at(*it->data, aPos.second);
            return formula::FormulaTokenRef(new formula::FormulaStringToken(rSS));
        }
        case sc::element_type_edittext:
        {
            const EditTextObject* pText = sc::edittext_block::at(*it->data, aPos.second);
            OUString aStr = ScEditUtil::GetString(*pText, &GetDoc());
            svl::SharedString aSS( GetDoc().GetSharedStringPool().intern(aStr));
            return formula::FormulaTokenRef(new formula::FormulaStringToken(std::move(aSS)));
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
    ScDocument* mpDoc;
    svl::SharedStringPool& mrStrPool;
public:
    ToMatrixHandler(ScMatrix& rMat, SCCOL nMatCol, SCROW nTopRow, ScDocument* pDoc) :
        mrMat(rMat), mnMatCol(nMatCol), mnTopRow(nTopRow),
        mpDoc(pDoc), mrStrPool(pDoc->GetSharedStringPool()) {}

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

    void operator() (size_t nRow, const svl::SharedString& rSS)
    {
        mrMat.PutString(rSS, mnMatCol, nRow - mnTopRow);
    }

    void operator() (size_t nRow, const EditTextObject* pStr)
    {
        mrMat.PutString(mrStrPool.intern(ScEditUtil::GetString(*pStr, mpDoc)), mnMatCol, nRow - mnTopRow);
    }
};

}

bool ScColumn::ResolveStaticReference( ScMatrix& rMat, SCCOL nMatCol, SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 > nRow2)
        return false;

    ToMatrixHandler aFunc(rMat, nMatCol, nRow1, &GetDoc());
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nRow1, nRow2, aFunc);
    return true;
}

namespace {

struct CellBucket
{
    SCSIZE mnEmpValStart;
    SCSIZE mnNumValStart;
    SCSIZE mnStrValStart;
    SCSIZE mnEmpValCount;
    std::vector<double> maNumVals;
    std::vector<svl::SharedString> maStrVals;

    CellBucket() : mnEmpValStart(0), mnNumValStart(0), mnStrValStart(0), mnEmpValCount(0) {}

    void flush(ScMatrix& rMat, SCSIZE nCol)
    {
        if (mnEmpValCount)
        {
            rMat.PutEmptyResultVector(mnEmpValCount, nCol, mnEmpValStart);
            reset();
        }
        else if (!maNumVals.empty())
        {
            const double* p = maNumVals.data();
            rMat.PutDouble(p, maNumVals.size(), nCol, mnNumValStart);
            reset();
        }
        else if (!maStrVals.empty())
        {
            const svl::SharedString* p = maStrVals.data();
            rMat.PutString(p, maStrVals.size(), nCol, mnStrValStart);
            reset();
        }
    }

    void reset()
    {
        mnEmpValStart = mnNumValStart = mnStrValStart = 0;
        mnEmpValCount = 0;
        maNumVals.clear();
        maStrVals.clear();
    }
};

class FillMatrixHandler
{
    ScMatrix& mrMat;
    size_t mnMatCol;
    size_t mnTopRow;

    ScDocument* mpDoc;
    svl::SharedStringPool& mrPool;
    svl::SharedStringPool* mpPool; // if matrix is not in the same document

public:
    FillMatrixHandler(ScMatrix& rMat, size_t nMatCol, size_t nTopRow, ScDocument* pDoc, svl::SharedStringPool* pPool) :
        mrMat(rMat), mnMatCol(nMatCol), mnTopRow(nTopRow),
        mpDoc(pDoc), mrPool(pDoc->GetSharedStringPool()), mpPool(pPool) {}

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
                if (!mpPool)
                {
                    const svl::SharedString* p = &sc::string_block::at(*node.data, nOffset);
                    mrMat.PutString(p, nDataSize, mnMatCol, nMatRow);
                }
                else
                {
                    std::vector<svl::SharedString> aStrings;
                    aStrings.reserve(nDataSize);
                    const svl::SharedString* p = &sc::string_block::at(*node.data, nOffset);
                    for (size_t i = 0; i < nDataSize; ++i)
                    {
                        aStrings.push_back(mpPool->intern(p[i].getString()));
                    }
                    mrMat.PutString(aStrings.data(), aStrings.size(), mnMatCol, nMatRow);
                }
            }
            break;
            case sc::element_type_edittext:
            {
                std::vector<svl::SharedString> aSSs;
                aSSs.reserve(nDataSize);
                sc::edittext_block::const_iterator it = sc::edittext_block::begin(*node.data);
                std::advance(it, nOffset);
                sc::edittext_block::const_iterator itEnd = it;
                std::advance(itEnd, nDataSize);
                for (; it != itEnd; ++it)
                {
                    OUString aStr = ScEditUtil::GetString(**it, mpDoc);
                    if (!mpPool)
                        aSSs.push_back(mrPool.intern(aStr));
                    else
                        aSSs.push_back(mpPool->intern(aStr));
                }

                const svl::SharedString* p = aSSs.data();
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
                    ScFormulaCell& rCell = **it;

                    if (rCell.IsEmpty())
                    {
                        if (aBucket.mnEmpValCount && nThisRow == nPrevRow + 1)
                        {
                            // Secondary empty results.
                            ++aBucket.mnEmpValCount;
                        }
                        else
                        {
                            // First empty result.
                            aBucket.flush(mrMat, mnMatCol);
                            aBucket.mnEmpValStart = nThisRow - mnTopRow;
                            ++aBucket.mnEmpValCount;
                        }
                        continue;
                    }

                    FormulaError nErr;
                    double fVal;
                    if (rCell.GetErrorOrValue(nErr, fVal))
                    {
                        if (nErr != FormulaError::NONE)
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

                    svl::SharedString aStr = rCell.GetString();
                    if (mpPool)
                        aStr = mpPool->intern(aStr.getString());
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

void ScColumn::FillMatrix( ScMatrix& rMat, size_t nMatCol, SCROW nRow1, SCROW nRow2, svl::SharedStringPool* pPool ) const
{
    FillMatrixHandler aFunc(rMat, nMatCol, nRow1, &GetDoc(), pPool);
    sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
}

namespace {

template<typename Blk>
void getBlockIterators(
    const sc::CellStoreType::iterator& it, size_t& rLenRemain,
    typename Blk::iterator& rData, typename Blk::iterator& rDataEnd )
{
    rData = Blk::begin(*it->data);
    if (rLenRemain >= it->size)
    {
        // Block is shorter than the remaining requested length.
        rDataEnd = Blk::end(*it->data);
        rLenRemain -= it->size;
    }
    else
    {
        rDataEnd = rData;
        std::advance(rDataEnd, rLenRemain);
        rLenRemain = 0;
    }
}

bool appendToBlock(
    ScDocument* pDoc, sc::FormulaGroupContext& rCxt, sc::FormulaGroupContext::ColArray& rColArray,
    size_t nPos, size_t nArrayLen, const sc::CellStoreType::iterator& _it, const sc::CellStoreType::iterator& itEnd )
{
    svl::SharedStringPool& rPool = pDoc->GetSharedStringPool();
    size_t nLenRemain = nArrayLen - nPos;

    for (sc::CellStoreType::iterator it = _it; it != itEnd; ++it)
    {
        switch (it->type)
        {
            case sc::element_type_string:
            {
                sc::string_block::iterator itData, itDataEnd;
                getBlockIterators<sc::string_block>(it, nLenRemain, itData, itDataEnd);
                rCxt.ensureStrArray(rColArray, nArrayLen);

                for (; itData != itDataEnd; ++itData, ++nPos)
                    (*rColArray.mpStrArray)[nPos] = itData->getData();
            }
            break;
            case sc::element_type_edittext:
            {
                sc::edittext_block::iterator itData, itDataEnd;
                getBlockIterators<sc::edittext_block>(it, nLenRemain, itData, itDataEnd);
                rCxt.ensureStrArray(rColArray, nArrayLen);

                for (; itData != itDataEnd; ++itData, ++nPos)
                {
                    OUString aStr = ScEditUtil::GetString(**itData, pDoc);
                    (*rColArray.mpStrArray)[nPos] = rPool.intern(aStr).getData();
                }
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::iterator itData, itDataEnd;
                getBlockIterators<sc::formula_block>(it, nLenRemain, itData, itDataEnd);

                /* tdf#91416 setting progress in triggers a resize of the window
                   and so ScTabView::DoResize and an InterpretVisible and
                   InterpretDirtyCells which resets the mpFormulaGroupCxt that
                   the current rCxt points to, which is bad, so disable progress
                   during GetResult
                 */
                ScProgress *pProgress = ScProgress::GetInterpretProgress();
                bool bTempDisableProgress = pProgress && pProgress->Enabled();
                if (bTempDisableProgress)
                    pProgress->Disable();

                for (; itData != itDataEnd; ++itData, ++nPos)
                {
                    ScFormulaCell& rFC = **itData;

                    sc::FormulaResultValue aRes = rFC.GetResult();

                    if (aRes.meType == sc::FormulaResultValue::Invalid || aRes.mnError != FormulaError::NONE)
                    {
                        if (aRes.mnError == FormulaError::CircularReference)
                        {
                            // This cell needs to be recalculated on next visit.
                            rFC.SetErrCode(FormulaError::NONE);
                            rFC.SetDirtyVar();
                        }
                        return false;
                    }

                    if (aRes.meType == sc::FormulaResultValue::String)
                    {
                        rCxt.ensureStrArray(rColArray, nArrayLen);
                        (*rColArray.mpStrArray)[nPos] = aRes.maString.getData();
                    }
                    else
                    {
                        rCxt.ensureNumArray(rColArray, nArrayLen);
                        (*rColArray.mpNumArray)[nPos] = aRes.mfValue;
                    }
                }

                if (bTempDisableProgress)
                    pProgress->Enable();
            }
            break;
            case sc::element_type_empty:
            {
                if (nLenRemain > it->size)
                {
                    nPos += it->size;
                    nLenRemain -= it->size;
                }
                else
                {
                    nPos = nArrayLen;
                    nLenRemain = 0;
                }
            }
            break;
            case sc::element_type_numeric:
            {
                sc::numeric_block::iterator itData, itDataEnd;
                getBlockIterators<sc::numeric_block>(it, nLenRemain, itData, itDataEnd);
                rCxt.ensureNumArray(rColArray, nArrayLen);

                for (; itData != itDataEnd; ++itData, ++nPos)
                    (*rColArray.mpNumArray)[nPos] = *itData;
            }
            break;
            default:
                return false;
        }

        if (!nLenRemain)
            return true;
    }

    return false;
}

void copyFirstStringBlock(
    ScDocument& rDoc, sc::FormulaGroupContext::StrArrayType& rArray, size_t nLen, const sc::CellStoreType::iterator& itBlk )
{
    sc::FormulaGroupContext::StrArrayType::iterator itArray = rArray.begin();

    switch (itBlk->type)
    {
        case sc::element_type_string:
        {
            sc::string_block::iterator it = sc::string_block::begin(*itBlk->data);
            sc::string_block::iterator itEnd = it;
            std::advance(itEnd, nLen);
            for (; it != itEnd; ++it, ++itArray)
                *itArray = it->getData();
        }
        break;
        case sc::element_type_edittext:
        {
            sc::edittext_block::iterator it = sc::edittext_block::begin(*itBlk->data);
            sc::edittext_block::iterator itEnd = it;
            std::advance(itEnd, nLen);

            svl::SharedStringPool& rPool = rDoc.GetSharedStringPool();
            for (; it != itEnd; ++it, ++itArray)
            {
                EditTextObject* pText = *it;
                OUString aStr = ScEditUtil::GetString(*pText, &rDoc);
                *itArray = rPool.intern(aStr).getData();
            }
        }
        break;
        default:
            ;
    }
}

sc::FormulaGroupContext::ColArray*
copyFirstFormulaBlock(
    sc::FormulaGroupContext& rCxt, const sc::CellStoreType::iterator& itBlk, size_t nArrayLen,
    SCTAB nTab, SCCOL nCol )
{
    size_t nLen = std::min(itBlk->size, nArrayLen);

    sc::formula_block::iterator it = sc::formula_block::begin(*itBlk->data);
    sc::formula_block::iterator itEnd;

    sc::FormulaGroupContext::NumArrayType* pNumArray = nullptr;
    sc::FormulaGroupContext::StrArrayType* pStrArray = nullptr;

    itEnd = it;
    std::advance(itEnd, nLen);
    size_t nPos = 0;
    for (; it != itEnd; ++it, ++nPos)
    {
        ScFormulaCell& rFC = **it;
        sc::FormulaResultValue aRes = rFC.GetResult();
        if (aRes.meType == sc::FormulaResultValue::Invalid || aRes.mnError != FormulaError::NONE)
        {
            if (aRes.mnError == FormulaError::CircularReference)
            {
                // This cell needs to be recalculated on next visit.
                rFC.SetErrCode(FormulaError::NONE);
                rFC.SetDirtyVar();
            }
            return nullptr;
        }

        if (aRes.meType == sc::FormulaResultValue::Value)
        {
            if (!pNumArray)
            {
                rCxt.m_NumArrays.push_back(
                    std::make_unique<sc::FormulaGroupContext::NumArrayType>(nArrayLen,
                        std::numeric_limits<double>::quiet_NaN()));
                pNumArray = rCxt.m_NumArrays.back().get();
            }

            (*pNumArray)[nPos] = aRes.mfValue;
        }
        else
        {
            if (!pStrArray)
            {
                rCxt.m_StrArrays.push_back(
                    std::make_unique<sc::FormulaGroupContext::StrArrayType>(nArrayLen, nullptr));
                pStrArray = rCxt.m_StrArrays.back().get();
            }

            (*pStrArray)[nPos] = aRes.maString.getData();
        }
    }

    if (!pNumArray && !pStrArray)
        // At least one of these arrays should be allocated.
        return nullptr;

    return rCxt.setCachedColArray(nTab, nCol, pNumArray, pStrArray);
}

struct NonNullStringFinder
{
    bool operator() (const rtl_uString* p) const { return p != nullptr; }
};

bool hasNonEmpty( const sc::FormulaGroupContext::StrArrayType& rArray, SCROW nRow1, SCROW nRow2 )
{
    // The caller has to make sure the array is at least nRow2+1 long.
    sc::FormulaGroupContext::StrArrayType::const_iterator it = rArray.begin();
    std::advance(it, nRow1);
    sc::FormulaGroupContext::StrArrayType::const_iterator itEnd = it;
    std::advance(itEnd, nRow2-nRow1+1);
    return std::any_of(it, itEnd, NonNullStringFinder());
}

struct ProtectFormulaGroupContext
{
    ProtectFormulaGroupContext( ScDocument* d )
        : doc( d ) { doc->BlockFormulaGroupContextDiscard( true ); }
    ~ProtectFormulaGroupContext()
        { doc->BlockFormulaGroupContextDiscard( false ); }
    ScDocument* doc;
};

}

formula::VectorRefArray ScColumn::FetchVectorRefArray( SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 > nRow2)
        return formula::VectorRefArray(formula::VectorRefArray::Invalid);

    // See if the requested range is already cached.
    ScDocument& rDocument = GetDoc();
    sc::FormulaGroupContext& rCxt = *(rDocument.GetFormulaGroupContext());
    sc::FormulaGroupContext::ColArray* pColArray = rCxt.getCachedColArray(nTab, nCol, nRow2+1);
    if (pColArray)
    {
        const double* pNum = nullptr;
        if (pColArray->mpNumArray)
            pNum = &(*pColArray->mpNumArray)[nRow1];

        rtl_uString** pStr = nullptr;
        if (pColArray->mpStrArray && hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
            pStr = &(*pColArray->mpStrArray)[nRow1];

        return formula::VectorRefArray(pNum, pStr);
    }

    // ScColumn::CellStorageModified() simply discards the entire cache (FormulaGroupContext)
    // on any modification. However getting cell values may cause this to be called
    // if interpreting a cell results in a change to it (not just its result though).
    // So temporarily block the discarding.
    ProtectFormulaGroupContext protectContext(&GetDoc());

    // We need to fetch all cell values from row 0 to nRow2 for caching purposes.
    sc::CellStoreType::iterator itBlk = maCells.begin();
    switch (itBlk->type)
    {
        case sc::element_type_numeric:
        {
            if (o3tl::make_unsigned(nRow2) < itBlk->size)
            {
                // Requested range falls within the first block. No need to cache.
                const double* p = &sc::numeric_block::at(*itBlk->data, nRow1);
                return formula::VectorRefArray(p);
            }

            // Allocate a new array and copy the values to it.
            sc::numeric_block::const_iterator it = sc::numeric_block::begin(*itBlk->data);
            sc::numeric_block::const_iterator itEnd = sc::numeric_block::end(*itBlk->data);
            rCxt.m_NumArrays.push_back(
                std::make_unique<sc::FormulaGroupContext::NumArrayType>(it, itEnd));
            sc::FormulaGroupContext::NumArrayType& rArray = *rCxt.m_NumArrays.back();
            // allocate to the requested length.
            rArray.resize(nRow2+1, std::numeric_limits<double>::quiet_NaN());

            pColArray = rCxt.setCachedColArray(nTab, nCol, &rArray, nullptr);
            if (!pColArray)
                // Failed to insert a new cached column array.
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);

            // Fill the remaining array with values from the following blocks.
            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(&rDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
            {
                rCxt.discardCachedColArray(nTab, nCol);
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);
            }

            rtl_uString** pStr = nullptr;
            if (pColArray->mpStrArray && hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
                pStr = &(*pColArray->mpStrArray)[nRow1];

            return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1], pStr);
        }
        break;
        case sc::element_type_string:
        case sc::element_type_edittext:
        {
            rCxt.m_StrArrays.push_back(
                std::make_unique<sc::FormulaGroupContext::StrArrayType>(nRow2+1, nullptr));
            sc::FormulaGroupContext::StrArrayType& rArray = *rCxt.m_StrArrays.back();
            pColArray = rCxt.setCachedColArray(nTab, nCol, nullptr, &rArray);
            if (!pColArray)
                // Failed to insert a new cached column array.
                return formula::VectorRefArray();

            if (o3tl::make_unsigned(nRow2) < itBlk->size)
            {
                // Requested range falls within the first block.
                copyFirstStringBlock(rDocument, rArray, nRow2+1, itBlk);
                return formula::VectorRefArray(&rArray[nRow1]);
            }

            copyFirstStringBlock(rDocument, rArray, itBlk->size, itBlk);

            // Fill the remaining array with values from the following blocks.
            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(&rDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
            {
                rCxt.discardCachedColArray(nTab, nCol);
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);
            }

            assert(pColArray->mpStrArray);

            rtl_uString** pStr = nullptr;
            if (hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
                pStr = &(*pColArray->mpStrArray)[nRow1];

            if (pColArray->mpNumArray)
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1], pStr);
            else
                return formula::VectorRefArray(pStr);
        }
        break;
        case sc::element_type_formula:
        {
            if (o3tl::make_unsigned(nRow2) < itBlk->size)
            {
                // Requested length is within a single block, and the data is
                // not cached.
                pColArray = copyFirstFormulaBlock(rCxt, itBlk, nRow2+1, nTab, nCol);
                if (!pColArray)
                    // Failed to insert a new cached column array.
                    return formula::VectorRefArray(formula::VectorRefArray::Invalid);

                const double* pNum = nullptr;
                rtl_uString** pStr = nullptr;
                if (pColArray->mpNumArray)
                    pNum = &(*pColArray->mpNumArray)[nRow1];
                if (pColArray->mpStrArray)
                    pStr = &(*pColArray->mpStrArray)[nRow1];

                return formula::VectorRefArray(pNum, pStr);
            }

            pColArray = copyFirstFormulaBlock(rCxt, itBlk, nRow2+1, nTab, nCol);
            if (!pColArray)
            {
                // Failed to insert a new cached column array.
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);
            }

            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(&rDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
            {
                rCxt.discardCachedColArray(nTab, nCol);
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);
            }

            const double* pNum = nullptr;
            rtl_uString** pStr = nullptr;
            if (pColArray->mpNumArray)
                pNum = &(*pColArray->mpNumArray)[nRow1];
            if (pColArray->mpStrArray && hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
                pStr = &(*pColArray->mpStrArray)[nRow1];

            return formula::VectorRefArray(pNum, pStr);
        }
        break;
        case sc::element_type_empty:
        {
            // Fill the whole length with NaN's.
            rCxt.m_NumArrays.push_back(
                std::make_unique<sc::FormulaGroupContext::NumArrayType>(nRow2+1,
                    std::numeric_limits<double>::quiet_NaN()));
            sc::FormulaGroupContext::NumArrayType& rArray = *rCxt.m_NumArrays.back();
            pColArray = rCxt.setCachedColArray(nTab, nCol, &rArray, nullptr);
            if (!pColArray)
                // Failed to insert a new cached column array.
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);

            if (o3tl::make_unsigned(nRow2) < itBlk->size)
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1]);

            // Fill the remaining array with values from the following blocks.
            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(&rDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
            {
                rCxt.discardCachedColArray(nTab, nCol);
                return formula::VectorRefArray(formula::VectorRefArray::Invalid);
            }

            if (pColArray->mpStrArray && hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1], &(*pColArray->mpStrArray)[nRow1]);
            else
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1]);
        }
        break;
        default:
            ;
    }

    return formula::VectorRefArray(formula::VectorRefArray::Invalid);
}

#ifdef DBG_UTIL
static void assertNoInterpretNeededHelper( const sc::CellStoreType::value_type& node,
    size_t nOffset, size_t nDataSize )
{
    switch (node.type)
    {
        case sc::element_type_formula:
        {
            sc::formula_block::const_iterator it = sc::formula_block::begin(*node.data);
            std::advance(it, nOffset);
            sc::formula_block::const_iterator itEnd = it;
            std::advance(itEnd, nDataSize);
            for (; it != itEnd; ++it)
            {
                const ScFormulaCell* pCell = *it;
                assert( !pCell->NeedsInterpret());
            }
            break;
        }
    }
}
void ScColumn::AssertNoInterpretNeeded( SCROW nRow1, SCROW nRow2 )
{
    assert(nRow2 >= nRow1);
    sc::ParseBlock( maCells.begin(), maCells, assertNoInterpretNeededHelper, 0, nRow2 );
}
#endif

void ScColumn::SetFormulaResults( SCROW nRow, const double* pResults, size_t nLen )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
    {
        // This is not a formula block.
        assert( false );
        return;
    }

    size_t nBlockLen = it->size - aPos.second;
    if (nBlockLen < nLen)
    {
        // Result array is longer than the length of formula cells. Not good.
        assert( false );
        return;
    }

    sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
    std::advance(itCell, aPos.second);

    const double* pResEnd = pResults + nLen;
    for (; pResults != pResEnd; ++pResults, ++itCell)
    {
        ScFormulaCell& rCell = **itCell;
        FormulaError nErr = GetDoubleErrorValue(*pResults);
        if (nErr != FormulaError::NONE)
            rCell.SetResultError(nErr);
        else
            rCell.SetResultDouble(*pResults);
        rCell.ResetDirty();
        rCell.SetChanged(true);
    }
}

void ScColumn::CalculateInThread( ScInterpreterContext& rContext, SCROW nRow, size_t nLen, size_t nOffset,
                                  unsigned nThisThread, unsigned nThreadsTotal)
{
    assert(GetDoc().IsThreadedGroupCalcInProgress());

    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
    {
        // This is not a formula block.
        assert( false );
        return;
    }

    size_t nBlockLen = it->size - aPos.second;
    if (nBlockLen < nLen)
    {
        // Length is longer than the length of formula cells. Not good.
        assert( false );
        return;
    }

    sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
    std::advance(itCell, aPos.second);

    for (size_t i = 0; i < nLen; ++i, ++itCell)
    {
        if (nThreadsTotal > 0 && ((i + nOffset) % nThreadsTotal) != nThisThread)
            continue;

        ScFormulaCell& rCell = **itCell;
        if (!rCell.NeedsInterpret())
            continue;
        // Here we don't call IncInterpretLevel() and DecInterpretLevel() as this call site is
        // always in a threaded calculation.
        rCell.InterpretTail(rContext, ScFormulaCell::SCITP_NORMAL);
    }
}

void ScColumn::HandleStuffAfterParallelCalculation( SCROW nRow, size_t nLen, ScInterpreter* pInterpreter )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
    {
        // This is not a formula block.
        assert( false );
        return;
    }

    size_t nBlockLen = it->size - aPos.second;
    if (nBlockLen < nLen)
    {
        // Length is longer than the length of formula cells. Not good.
        assert( false );
        return;
    }

    sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
    std::advance(itCell, aPos.second);

    for (size_t i = 0; i < nLen; ++i, ++itCell)
    {
        ScFormulaCell& rCell = **itCell;
        rCell.HandleStuffAfterParallelCalculation(pInterpreter);
    }
}

void ScColumn::SetNumberFormat( SCROW nRow, sal_uInt32 nNumberFormat )
{
    ApplyAttr(nRow, SfxUInt32Item(ATTR_VALUE_FORMAT, nNumberFormat));
}

ScFormulaCell * const * ScColumn::GetFormulaCellBlockAddress( SCROW nRow, size_t& rBlockSize ) const
{
    if (!GetDoc().ValidRow(nRow))
    {
        rBlockSize = 0;
        return nullptr;
    }

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
    {
        rBlockSize = 0;
        return nullptr;
    }

    if (it->type != sc::element_type_formula)
    {
        // Not a formula cell.
        rBlockSize = 0;
        return nullptr;
    }

    rBlockSize = it->size;
    return &sc::formula_block::at(*it->data, aPos.second);
}

const ScFormulaCell* ScColumn::FetchFormulaCell( SCROW nRow ) const
{
    size_t nBlockSize = 0;
    ScFormulaCell const * const * pp = GetFormulaCellBlockAddress( nRow, nBlockSize );
    return pp ? *pp : nullptr;
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

bool ScColumn::HasDataAt(SCROW nRow, ScDataAreaExtras* pDataAreaExtras ) const
{
    if (pDataAreaExtras)
        GetDataExtrasAt( nRow, *pDataAreaExtras);

    return maCells.get_type(nRow) != sc::element_type_empty;
}

bool ScColumn::HasDataAt( sc::ColumnBlockConstPosition& rBlockPos, SCROW nRow,
                          ScDataAreaExtras* pDataAreaExtras ) const
{
    if (pDataAreaExtras)
        GetDataExtrasAt( nRow, *pDataAreaExtras);

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rBlockPos.miCellPos, nRow);
    if (aPos.first == maCells.end())
        return false;
    rBlockPos.miCellPos = aPos.first; // Store this for next call.
    return aPos.first->type != sc::element_type_empty;
}

bool ScColumn::HasDataAt( sc::ColumnBlockPosition& rBlockPos, SCROW nRow,
                          ScDataAreaExtras* pDataAreaExtras )
{
    if (pDataAreaExtras)
        GetDataExtrasAt( nRow, *pDataAreaExtras);

    std::pair<sc::CellStoreType::iterator,size_t> aPos = maCells.position(rBlockPos.miCellPos, nRow);
    if (aPos.first == maCells.end())
        return false;
    rBlockPos.miCellPos = aPos.first; // Store this for next call.
    return aPos.first->type != sc::element_type_empty;
}

void ScColumn::GetDataExtrasAt( SCROW nRow, ScDataAreaExtras& rDataAreaExtras ) const
{
    if (rDataAreaExtras.mnStartRow <= nRow && nRow <= rDataAreaExtras.mnEndRow)
        return;

    // Check in order of likeliness.
    if (    (rDataAreaExtras.mbCellFormats && HasVisibleAttrIn(nRow, nRow)) ||
            (rDataAreaExtras.mbCellNotes && !IsNotesEmptyBlock(nRow, nRow)) ||
            (rDataAreaExtras.mbCellDrawObjects && !IsDrawObjectsEmptyBlock(nRow, nRow)))
    {
        if (rDataAreaExtras.mnStartRow > nRow)
            rDataAreaExtras.mnStartRow = nRow;
        if (rDataAreaExtras.mnEndRow < nRow)
            rDataAreaExtras.mnEndRow = nRow;
    }
}

namespace {

class FindUsedRowsHandler
{
    typedef mdds::flat_segment_tree<SCROW,bool> UsedRowsType;
    UsedRowsType& mrUsed;
    UsedRowsType::const_iterator miUsed;
public:
    explicit FindUsedRowsHandler(UsedRowsType& rUsed) : mrUsed(rUsed), miUsed(rUsed.begin()) {}

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
            assert(false && "wrong block type encountered in the broadcaster storage.");
    }
}

}

void ScColumn::StartListening( SvtListener& rLst, SCROW nRow )
{
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(nRow);
    startListening(maBroadcasters, aPos.first, aPos.second, nRow, rLst);
}

void ScColumn::EndListening( SvtListener& rLst, SCROW nRow )
{
    SvtBroadcaster* pBC = GetBroadcaster(nRow);
    if (!pBC)
        return;

    rLst.EndListening(*pBC);
    if (!pBC->HasListeners())
    {   // There is no more listeners for this cell. Remove the broadcaster.
        if(GetDoc().IsDelayedDeletingBroadcasters())
            mbEmptyBroadcastersPending = true;
        else
            maBroadcasters.set_empty(nRow, nRow);
    }
}

void ScColumn::StartListening( sc::StartListeningContext& rCxt, const ScAddress& rAddress, SvtListener& rLst )
{
    if (!GetDoc().ValidRow(rAddress.Row()))
        return;

    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(rAddress.Tab(), rAddress.Col());
    if (!p)
        return;

    sc::BroadcasterStoreType::iterator& it = p->miBroadcasterPos;
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(it, rAddress.Row());
    it = aPos.first; // store the block position for next iteration.
    startListening(maBroadcasters, it, aPos.second, rAddress.Row(), rLst);
}

void ScColumn::EndListening( sc::EndListeningContext& rCxt, const ScAddress& rAddress, SvtListener& rListener )
{
    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(rAddress.Tab(), rAddress.Col());
    if (!p)
        return;

    sc::BroadcasterStoreType::iterator& it = p->miBroadcasterPos;
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(it, rAddress.Row());
    it = aPos.first; // store the block position for next iteration.
    if (it->type != sc::element_type_broadcaster)
        return;

    SvtBroadcaster* pBC = sc::broadcaster_block::at(*it->data, aPos.second);
    assert(pBC);

    rListener.EndListening(*pBC);
    if (!pBC->HasListeners())
        // There is no more listeners for this cell. Add it to the purge list for later purging.
        rCxt.addEmptyBroadcasterPosition(rAddress.Tab(), rAddress.Col(), rAddress.Row());
}

namespace {

class CompileDBFormulaHandler
{
    sc::CompileFormulaContext& mrCxt;

public:
    explicit CompileDBFormulaHandler( sc::CompileFormulaContext& rCxt ) :
        mrCxt(rCxt) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileDBFormula(mrCxt);
    }
};

struct CompileColRowNameFormulaHandler
{
    sc::CompileFormulaContext& mrCxt;
public:
    explicit CompileColRowNameFormulaHandler( sc::CompileFormulaContext& rCxt ) : mrCxt(rCxt) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileColRowNameFormula(mrCxt);
    }
};

}

void ScColumn::CompileDBFormula( sc::CompileFormulaContext& rCxt )
{
    CompileDBFormulaHandler aFunc(rCxt);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

void ScColumn::CompileColRowNameFormula( sc::CompileFormulaContext& rCxt )
{
    CompileColRowNameFormulaHandler aFunc(rCxt);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

namespace {

class UpdateSubTotalHandler
{
    ScFunctionData& mrData;

    void update(double fVal, bool bVal)
    {
        if (mrData.getError())
            return;

        switch (mrData.getFunc())
        {
            case SUBTOTAL_FUNC_CNT2:    // everything
                mrData.update( fVal);
            break;
            default:                    // only numeric values
                if (bVal)
                    mrData.update( fVal);
        }
    }

public:
    explicit UpdateSubTotalHandler(ScFunctionData& rData) : mrData(rData) {}

    void operator() (size_t /*nRow*/, double fVal)
    {
        update(fVal, true);
    }

    void operator() (size_t /*nRow*/, const svl::SharedString&)
    {
        update(0.0, false);
    }

    void operator() (size_t /*nRow*/, const EditTextObject*)
    {
        update(0.0, false);
    }

    void operator() (size_t /*nRow*/, ScFormulaCell* pCell)
    {
        double fVal = 0.0;
        bool bVal = false;
        if (mrData.getFunc() != SUBTOTAL_FUNC_CNT2) // it doesn't interest us
        {

            if (pCell->GetErrCode() != FormulaError::NONE)
            {
                if (mrData.getFunc() != SUBTOTAL_FUNC_CNT) // simply remove from count
                    mrData.setError();
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
};

}

//  multiple selections:
void ScColumn::UpdateSelectionFunction(
    const ScRangeList& rRanges, ScFunctionData& rData, const ScFlatBoolRowSegments& rHiddenRows )
{
    sc::SingleColumnSpanSet aSpanSet(GetDoc().GetSheetLimits());
    aSpanSet.scan(rRanges, nTab, nCol); // mark all selected rows.

    if (aSpanSet.empty())
        return;     // nothing to do, bail out

    // Exclude all hidden rows.
    ScFlatBoolRowSegments::RangeData aRange;
    SCROW nRow = 0;
    while (nRow <= GetDoc().MaxRow())
    {
        if (!rHiddenRows.getRangeData(nRow, aRange))
            break;

        if (aRange.mbValue)
            // Hidden range detected.
            aSpanSet.set(nRow, aRange.mnRow2, false);

        nRow = aRange.mnRow2 + 1;
    }

    sc::SingleColumnSpanSet::SpansType aSpans;
    aSpanSet.getSpans(aSpans);

    switch (rData.getFunc())
    {
        case SUBTOTAL_FUNC_SELECTION_COUNT:
        {
            // Simply count selected rows regardless of cell contents.
            for (const auto& rSpan : aSpans)
                rData.update( rSpan.mnRow2 - rSpan.mnRow1 + 1);
        }
        break;
        case SUBTOTAL_FUNC_CNT2:
        {
            // We need to parse all non-empty cells.
            sc::CellStoreType::const_iterator itCellPos = maCells.begin();
            UpdateSubTotalHandler aFunc(rData);
            for (const auto& rSpan : aSpans)
            {
                itCellPos = sc::ParseAllNonEmpty(
                    itCellPos, maCells, rSpan.mnRow1, rSpan.mnRow2, aFunc);
            }
        }
        break;
        default:
        {
            // We need to parse only numeric values.
            sc::CellStoreType::const_iterator itCellPos = maCells.begin();
            UpdateSubTotalHandler aFunc(rData);
            for (const auto& rSpan : aSpans)
            {
                itCellPos = sc::ParseFormulaNumeric(
                    itCellPos, maCells, rSpan.mnRow1, rSpan.mnRow2, aFunc);
            }
        }
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
        mnCount += getWeight(node);
    }

    static size_t getWeight(const sc::CellStoreType::value_type& node)
    {
        switch (node.type)
        {
            case sc::element_type_numeric:
            case sc::element_type_string:
                return node.size;
            case sc::element_type_formula:
            {
                // Each formula cell is worth its code length plus 5.
                return std::accumulate(sc::formula_block::begin(*node.data), sc::formula_block::end(*node.data), size_t(0),
                    [](const size_t& rCount, const ScFormulaCell* p) { return rCount + 5 + p->GetCode()->GetCodeLen(); });
            }
            case sc::element_type_edittext:
                // each edit-text cell is worth 50.
                return node.size * 50;
            default:
                return 0;
        }
    }

    size_t getCount() const { return mnCount; }
};

class WeightedCounterWithRows
{
    const SCROW mnStartRow;
    const SCROW mnEndRow;
    size_t mnCount;

public:
    WeightedCounterWithRows(SCROW nStartRow, SCROW nEndRow)
        : mnStartRow(nStartRow)
        , mnEndRow(nEndRow)
        , mnCount(0)
    {
    }

    void operator() (const sc::CellStoreType::value_type& node)
    {
        const SCROW nRow1 = node.position;
        const SCROW nRow2 = nRow1 + 1;

        if ((nRow2 >= mnStartRow) && (nRow1 <= mnEndRow))
        {
            mnCount += WeightedCounter::getWeight(node);
        }
    }

    size_t getCount() const { return mnCount; }
};

}

sal_uInt64 ScColumn::GetWeightedCount() const
{
    const WeightedCounter aFunc = std::for_each(maCells.begin(), maCells.end(),
        WeightedCounter());
    return aFunc.getCount();
}

sal_uInt64 ScColumn::GetWeightedCount(SCROW nStartRow, SCROW nEndRow) const
{
    const WeightedCounterWithRows aFunc = std::for_each(maCells.begin(), maCells.end(),
        WeightedCounterWithRows(nStartRow, nEndRow));
    return aFunc.getCount();
}

namespace {

class CodeCounter
{
    sal_uInt64 mnCount;
public:
    CodeCounter() : mnCount(0) {}

    void operator() (size_t, const ScFormulaCell* p)
    {
        mnCount += p->GetCode()->GetCodeLen();
    }

    sal_uInt64 getCount() const { return mnCount; }
};

}

sal_uInt64 ScColumn::GetCodeCount() const
{
    CodeCounter aFunc;
    sc::ParseFormula(maCells, aFunc);
    return aFunc.getCount();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
