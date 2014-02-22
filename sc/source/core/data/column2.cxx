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
#include "compiler.hxx"
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
#include <rowheightcontext.hxx>

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
#include <vcl/outdev.hxx>
#include "formula/errorcodes.hxx"
#include "formula/vectortoken.hxx"

#include <boost/scoped_ptr.hpp>




#define SC_ROT_BREAK_FACTOR     6



inline bool IsAmbiguousScript( sal_uInt8 nScript )
{
    
    return ( nScript != SCRIPTTYPE_LATIN &&
             nScript != SCRIPTTYPE_ASIAN &&
             nScript != SCRIPTTYPE_COMPLEX );
}



//

//




long ScColumn::GetNeededSize(
    SCROW nRow, OutputDevice* pDev, double nPPTX, double nPPTY,
    const Fraction& rZoomX, const Fraction& rZoomY,
    bool bWidth, const ScNeededSizeOptions& rOptions ) const
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end() || it->type == sc::element_type_empty)
        
        return 0;

    long nValue = 0;
    ScRefCellValue aCell = GetCellValue(it, aPos.second);
    double nPPT = bWidth ? nPPTX : nPPTY;

    const ScPatternAttr* pPattern = rOptions.pPattern;
    if (!pPattern)
        pPattern = pAttrArray->GetPattern( nRow );

    
    

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

    
    const SfxItemSet* pCondSet = pDocument->GetCondResult( nCol, nRow, nTab );

    

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
    
    if (bBreak && aCell.hasNumeric() && ( nFormat % SV_COUNTRY_LANGUAGE_OFFSET ) == 0 )
    {
        bBreak = false;
    }

    

    SvxCellOrientation eOrient = pPattern->GetCellOrientation( pCondSet );
    bool bAsianVertical = ( eOrient == SVX_ORIENTATION_STACKED &&
            ((const SfxBoolItem&)pPattern->GetItem( ATTR_VERTICAL_ASIAN, pCondSet )).GetValue() );
    if ( bAsianVertical )
        bBreak = false;

    if ( bWidth && bBreak )     
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
                eRotMode = SVX_ROTATE_MODE_STANDARD;    
        }
    }

    if ( eHorJust == SVX_HOR_JUSTIFY_REPEAT )
    {
        
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

    
    
    if (rOptions.bGetFont)
    {
        Fraction aFontZoom = ( eOrient == SVX_ORIENTATION_STANDARD ) ? rZoomX : rZoomY;
        Font aFont;
        
        pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &aFontZoom, pCondSet, nScript );
        pDev->SetFont(aFont);
    }

    bool bAddMargin = true;
    CellType eCellType = aCell.meType;

    bool bEditEngine = (eCellType == CELLTYPE_EDIT ||
                        eOrient == SVX_ORIENTATION_STACKED ||
                        IsAmbiguousScript(nScript) ||
                        ((eCellType == CELLTYPE_FORMULA) && aCell.mpFormula->IsMultilineResult()));

    if (!bEditEngine)                                   
    {
        Color* pColor;
        OUString aValStr;
        ScCellFormat::GetString(
            aCell, nFormat, aValStr, &pColor, *pFormatter, pDocument, true, rOptions.bFormula, ftCheck);

        if (!aValStr.isEmpty())
        {
            

            Size aSize( pDev->GetTextWidth( aValStr ), pDev->GetTextHeight() );
            if ( eOrient != SVX_ORIENTATION_STANDARD )
            {
                long nTemp = aSize.Width();
                aSize.Width() = aSize.Height();
                aSize.Height() = nTemp;
            }
            else if ( nRotate )
            {
                

                double nRealOrient = nRotate * F_PI18000;   
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
                    nWidth  = (long)( aSize.Height() / nSinAbs );   

                if ( bBreak && !rOptions.bTotalSize )
                {
                    
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

            

            if ( bBreak && !bWidth )
            {
                
                

                long nDocPixel = (long) ( ( pDocument->GetColWidth( nCol,nTab ) -
                                    pMargin->GetLeftMargin() - pMargin->GetRightMargin() -
                                    nIndent )
                                    * nPPT );
                nDocPixel = (nDocPixel * 9) / 10;           
                if ( aSize.Width() > nDocPixel )
                    bEditEngine = true;
            }
        }
    }

    if (bEditEngine)
    {
        
        Font aOldFont = pDev->GetFont();

        MapMode aHMMMode( MAP_100TH_MM, Point(), rZoomX, rZoomY );

        
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
        if ( ScStyleSheet* pPreviewStyle = pDocument->GetPreviewCellStyle( nCol, nRow, nTab ) )
        {
            ScPatternAttr* pPreviewPattern = new ScPatternAttr( *pPattern );
            pPreviewPattern->SetStyleSheet(pPreviewStyle);
            pPreviewPattern->FillEditItemSet( pSet, pCondSet );
            delete pPreviewPattern;
        }
        else
        {
            SfxItemSet* pFontSet = pDocument->GetPreviewFont( nCol, nRow, nTab );
            pPattern->FillEditItemSet( pSet, pFontSet ? pFontSet : pCondSet );
        }



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
                
                
                

                fWidthFactor = HMM_PER_TWIPS;
            }

            
            long nDocWidth = (long) ( pDocument->GetOriginalWidth(nCol,nTab) * fWidthFactor );
            SCCOL nColMerge = pMerge->GetColMerge();
            if (nColMerge > 1)
                for (SCCOL nColAdd=1; nColAdd<nColMerge; nColAdd++)
                    nDocWidth += (long) ( pDocument->GetColWidth(nCol+nColAdd,nTab) * fWidthFactor );
            nDocWidth -= (long) ( pMargin->GetLeftMargin() * fWidthFactor )
                       + (long) ( pMargin->GetRightMargin() * fWidthFactor )
                       + 1;     
            if ( nIndent )
                nDocWidth -= (long) ( nIndent * fWidthFactor );

            
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
                aCell, nFormat, aString, &pColor, *pFormatter, pDocument, true,
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
            

            Size aSize( pEngine->CalcTextWidth(), pEngine->GetTextHeight() );
            double nRealOrient = nRotate * F_PI18000;   
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
                nWidth  = (long)( aSize.Height() / nSinAbs );   
            aSize = Size( nWidth, nHeight );

            Size aPixSize = pDev->LogicToPixel( aSize, aHMMMode );
            if ( bEdWidth )
                nValue = aPixSize.Width();
            else
            {
                nValue = aPixSize.Height();

                if ( bBreak && !rOptions.bTotalSize )
                {
                    
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
        else            
        {
            nValue = pDev->LogicToPixel(Size( 0, pEngine->GetTextHeight() ),
                                aHMMMode).Height();

            
            if ( !bTextWysiwyg && ( rZoomY.GetNumerator() != 1 || rZoomY.GetDenominator() != 1 ) &&
                 ( pEngine->GetParagraphCount() > 1 || ( bBreak && pEngine->GetLineCount(0) > 1 ) ) )
            {
                pEngine->SetControlWord( nCtrl | EE_CNTRL_FORMAT100 );
                pEngine->QuickFormatDoc( true );
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
                    
                    nValue += (long) ( 20 * nPPT );
                }
            }
        }

        
        pEngine->SetVertical( bEngineVertical );

        pDocument->DisposeFieldEditEngine(pEngine);

        pDev->SetMapMode( aOld );
        pDev->SetFont( aOldFont );
    }

    if (bWidth)
    {
        
        
        

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
            rCell, mnFormat, aValStr, &pColor, *mrDoc.GetFormatTable(), &mrDoc, true, false, ftCheck);

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

    void operator() (size_t /*nRow*/, const svl::SharedString& rSS)
    {
        if (rSS.getLength() > mnMaxLen)
        {
            mnMaxLen = rSS.getLength();
            maMaxLenStr = rSS.getString();
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
        
        return nOldWidth;

    sc::SingleColumnSpanSet aSpanSet;
    sc::SingleColumnSpanSet::SpansType aMarkedSpans;
    if (pMarkData && (pMarkData->IsMarked() || pMarkData->IsMultiMarked()))
    {
        aSpanSet.scan(*pMarkData, nTab, nCol);
        aSpanSet.getSpans(aMarkedSpans);
    }
    else
        
        aMarkedSpans.push_back(sc::RowSpan(0, MAXROW));

    sal_uInt16 nWidth = static_cast<sal_uInt16>(nOldWidth*nPPTX);
    bool bFound = false;

    if ( pParam && pParam->mbSimpleText )
    {   
        const ScPatternAttr* pPattern = GetPattern( 0 );
        Font aFont;
        
        pPattern->GetFont( aFont, SC_AUTOCOL_BLACK, pDev, &rZoomX, NULL );
        pDev->SetFont( aFont );
        const SvxMarginItem* pMargin = (const SvxMarginItem*) &pPattern->GetItem(ATTR_MARGIN);
        long nMargin = (long) ( pMargin->GetLeftMargin() * nPPTX ) +
                        (long) ( pMargin->GetRightMargin() * nPPTX );

        
        SvNumberFormatter* pFormatter = pDocument->GetFormatTable();
        sal_uInt32 nFormat = pPattern->GetNumberFormat( pFormatter );
        OUString aLongStr;
        Color* pColor;
        if (pParam->mnMaxTextRow >= 0)
        {
            ScRefCellValue aCell = GetCellValue(pParam->mnMaxTextRow);
            ScCellFormat::GetString(
                aCell, nFormat, aLongStr, &pColor, *pFormatter, pDocument, true, false, ftCheck);
        }
        else
        {
            
            MaxStrLenFinder aFunc(*pDocument, nFormat);
            sc::CellStoreType::const_iterator itPos = maCells.begin();
            sc::SingleColumnSpanSet::SpansType::const_iterator it = aMarkedSpans.begin(), itEnd = aMarkedSpans.end();
            for (; it != itEnd; ++it)
                itPos = sc::ParseAllNonEmpty(itPos, maCells, it->mnRow1, it->mnRow2, aFunc);

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
        const ScPatternAttr* pOldPattern = NULL;
        sal_uInt8 nOldScript = 0;

        
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
    const SvxFontHeightItem& rFontHeight =
        static_cast<const SvxFontHeightItem&>(rPattern.GetItem(nFontHeightId));

    sal_uInt16 nHeight = rFontHeight.GetHeight();
    nHeight *= 1.18;

    if ( ((const SvxEmphasisMarkItem&)rPattern.
            GetItem(ATTR_FONT_EMPHASISMARK)).GetEmphasisMark() != EMPHASISMARK_NONE )
    {
        
        
        nHeight += nHeight / 4;
    }

    const SvxMarginItem& rMargin =
        static_cast<const SvxMarginItem&>(rPattern.GetItem(ATTR_MARGIN));

    nHeight += rMargin.GetTopMargin() + rMargin.GetBottomMargin();

    if (nHeight > STD_ROWHEIGHT_DIFF)
        nHeight -= STD_ROWHEIGHT_DIFF;

    if (nHeight < ScGlobal::nStdRowHeight)
        nHeight = ScGlobal::nStdRowHeight;

    return nHeight;
}





void ScColumn::GetOptimalHeight(
    sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow, sal_uInt16* pHeight,
    sal_uInt16 nMinHeight, SCROW nMinStart )
{
    ScAttrIterator aIter( pAttrArray, nStartRow, nEndRow );

    SCROW nStart = -1;
    SCROW nEnd = -1;
    SCROW nEditPos = 0;
    SCROW nNextEnd = 0;

    

    const ScPatternAttr* pPattern = aIter.Next(nStart,nEnd);
    ::boost::ptr_vector<ScPatternAttr> aAltPatterns;
    while ( pPattern )
    {
        
        
        if ( ScStyleSheet* pStyle = pDocument->GetPreviewCellStyle( nCol, nStartRow, nTab ) )
        {
            aAltPatterns.push_back( new ScPatternAttr( *pPattern ) );
            ScPatternAttr* pModifiedPatt = &aAltPatterns.back();
            pModifiedPatt->SetStyleSheet( pStyle );
            pPattern = pModifiedPatt;
        }
        const ScMergeAttr*      pMerge = (const ScMergeAttr*)&pPattern->GetItem(ATTR_MERGE);
        const ScMergeFlagAttr*  pFlag = (const ScMergeFlagAttr*)&pPattern->GetItem(ATTR_MERGE_FLAG);
        if ( pMerge->GetRowMerge() > 1 || pFlag->IsOverlapped() )
        {
            
            
            
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

                
                if (bStdOnly &&
                    !static_cast<const ScCondFormatItem&>(pPattern->GetItem(
                            ATTR_CONDITIONAL)).GetCondFormatData().empty())
                {
                    bStdOnly = false;
                }

                
                if ( bStdOnly && ((const SfxInt32Item&)pPattern->
                                    GetItem(ATTR_ROTATE_VALUE)).GetValue() )
                    bStdOnly = false;
            }

            if (bStdOnly)
                if (HasEditCells(nStart,nEnd,nEditPos))     
                {
                    if (nEditPos == nStart)
                    {
                        bStdOnly = false;
                        if (nEnd > nEditPos)
                            nNextEnd = nEnd;
                        nEnd = nEditPos;                
                        bStdAllowed = false;            
                    }
                    else
                    {
                        nNextEnd = nEnd;
                        nEnd = nEditPos - 1;            
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

                
                
                SCROW nStdEnd = nEnd;
                if ( nDefHeight <= nMinHeight && nStdEnd >= nMinStart )
                    nStdEnd = (nMinStart>0) ? nMinStart-1 : 0;

                for (SCROW nRow = nStart; nRow <= nStdEnd; ++nRow)
                    if (nDefHeight > pHeight[nRow-nStartRow])
                        pHeight[nRow-nStartRow] = nDefHeight;

                if ( bStdOnly )
                {
                    
                    
                    sc::CellTextAttrStoreType::iterator itAttr = maCellTextAttrs.begin();
                    sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
                    sc::CellStoreType::iterator itCells = maCells.begin();
                    for (; it != itEnd; ++it)
                    {
                        for (SCROW nRow = it->mnRow1; nRow <= it->mnRow2; ++nRow)
                        {
                            sal_uInt8 nScript = GetRangeScriptType(itAttr, nRow, nRow, itCells);
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

            if (!bStdOnly)                      
            {
                ScNeededSizeOptions aOptions;

                sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();
                for (; it != itEnd; ++it)
                {
                    for (SCROW nRow = it->mnRow1; nRow <= it->mnRow2; ++nRow)
                    {
                        

                        if (rCxt.isForceAutoSize() || !(pDocument->GetRowFlags(nRow, nTab) & CR_MANUALSIZE) )
                        {
                            aOptions.pPattern = pPattern;
                            sal_uInt16 nHeight = (sal_uInt16)
                                    ( GetNeededSize( nRow, rCxt.getOutputDevice(), rCxt.getPPTX(), rCxt.getPPTY(),
                                                        rCxt.getZoomX(), rCxt.getZoomY(), false, aOptions ) / rCxt.getPPTY() );
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
    ScDocument* mpDoc;

    StrEntries(sc::CellStoreType& rCells, ScDocument* pDoc) : mrCells(rCells), mpDoc(pDoc) {}

public:
    void commitStrings()
    {
        svl::SharedStringPool& rPool = mpDoc->GetSharedStringPool();
        sc::CellStoreType::iterator it = mrCells.begin();
        std::vector<StrEntry>::iterator itStr = maStrEntries.begin(), itStrEnd = maStrEntries.end();
        for (; itStr != itStrEnd; ++itStr)
            it = mrCells.set(it, itStr->mnRow, rPool.intern(itStr->maStr));
    }
};

class RemoveEditAttribsHandler : public StrEntries
{
    boost::scoped_ptr<ScFieldEditEngine> mpEngine;

public:
    RemoveEditAttribsHandler(sc::CellStoreType& rCells, ScDocument* pDoc) : StrEntries(rCells, pDoc) {}

    void operator() (size_t nRow, EditTextObject*& pObj)
    {
        
        
        
        

        
        if (!mpEngine)
        {
            mpEngine.reset(new ScFieldEditEngine(mpDoc, mpDoc->GetEditPool()));
            
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
                SfxItemSet aNew( *rOld.GetPool(), rOld.GetRanges() );   
                mpEngine->SetParaAttribs( nPar, aNew );
            }
        }
        
        mpEngine->RemoveFields(true);

        bool bSpellErrors = mpEngine->HasOnlineSpellErrors();
        bool bNeedObject = bSpellErrors || nParCount>1;         
        

        if (bNeedObject)                                      
        {
            sal_uInt32 nCtrl = mpEngine->GetControlWord();
            sal_uInt32 nWantBig = bSpellErrors ? EE_CNTRL_ALLOWBIGOBJS : 0;
            if ( ( nCtrl & EE_CNTRL_ALLOWBIGOBJS ) != nWantBig )
                mpEngine->SetControlWord( (nCtrl & ~EE_CNTRL_ALLOWBIGOBJS) | nWantBig );

            
            delete pObj;
            pObj = mpEngine->CreateTextObject();
        }
        else                                            
        {
            
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
        
        return false;

    if (it->type != sc::element_type_empty)
        
        return false;

    
    SCROW nNextRow = nStartRow + it->size - aPos.second;
    return nEndRow < nNextRow;
}

bool ScColumn::IsNotesEmptyBlock(SCROW nStartRow, SCROW nEndRow) const
{
    std::pair<sc::CellNoteStoreType::const_iterator,size_t> aPos = maCellNotes.position(nStartRow);
    sc::CellNoteStoreType::const_iterator it = aPos.first;
    if (it == maCellNotes.end())
        
        return false;

    if (it->type != sc::element_type_empty)
        
        return false;

    
    SCROW nNextRow = nStartRow + it->size - aPos.second;
    return nEndRow < nNextRow;
}

SCSIZE ScColumn::GetEmptyLinesInBlock( SCROW nStartRow, SCROW nEndRow, ScDirection eDir ) const
{
    
    switch (eDir)
    {
        case DIR_TOP:
        {
            
            size_t nLength = nEndRow - nStartRow + 1;
            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nStartRow);
            sc::CellStoreType::const_iterator it = aPos.first;
            if (it->type != sc::element_type_empty)
                
                return 0;

            
            size_t nThisLen = it->size - aPos.second;
            return std::min(nThisLen, nLength);
        }
        break;
        case DIR_BOTTOM:
        {
            
            size_t nLength = nEndRow - nStartRow + 1;
            std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(nEndRow);
            sc::CellStoreType::const_iterator it = aPos.first;
            if (it->type != sc::element_type_empty)
                
                return 0;

            
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

SCROW ScColumn::GetLastDataPos( SCROW nLastRow ) const
{
    sc::CellStoreType::const_position_type aPos = maCells.position(nLastRow);
    if (aPos.first->type != sc::element_type_empty)
        return nLastRow;

    if (aPos.first == maCells.begin())
        
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
            
            return false;

        rRow -= aPos.second + 1; 
        return true;
    }

    
    if (aPos.second)
    {
        
        --rRow;
        return true;
    }

    
    if (it == maCells.begin())
        
        return false;

    --rRow; 
    --it;
    if (it->type == sc::element_type_empty)
    {
        
        if (it == maCells.begin())
            
            return false;

        
        rRow -= it->size;
    }

    return true;
}

bool ScColumn::GetNextDataPos(SCROW& rRow) const        
{
    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        return false;

    if (it->type == sc::element_type_empty)
    {
        
        rRow += it->size - aPos.second;
        ++it;
        if (it == maCells.end())
            
            return false;

        
        return true;
    }

    if (aPos.second < it->size - 1)
    {
        
        ++rRow;
        return true;
    }

    
    rRow += it->size - aPos.second; 
    ++it;
    if (it == maCells.end())
        
        return false;

    if (it->type == sc::element_type_empty)
    {
        
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
                
                return MAXROW;

            if (itPos->type != sc::element_type_empty)
                return nRow;

            
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
            
            return 0;

        if (itPos->type != sc::element_type_empty)
            return nRow;

        
        nRow -= aPos.second;
    }
    while (nRow > 0);

    return 0;
}

void ScColumn::CellStorageModified()
{
    

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

    
    sc::CellStoreType::const_iterator itCell = maCells.begin();
    sc::CellTextAttrStoreType::const_iterator itAttr = maCellTextAttrs.begin();

    
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

        cout << "  -- formula block" << endl;
        sc::formula_block::const_iterator it = sc::formula_block::begin(*rNode.data);
        sc::formula_block::const_iterator itEnd = sc::formula_block::end(*rNode.data);

        for (; it != itEnd; ++it)
        {
            const ScFormulaCell& rCell = **it;
            if (!rCell.IsShared())
            {
                cout << "  + row " << rCell.aPos.Row() << " not shared" << endl;
                continue;
            }

            if (rCell.GetSharedTopRow() != rCell.aPos.Row())
            {
                cout << "  + row " << rCell.aPos.Row() << " shared with top row " << rCell.GetSharedTopRow() << " with length " << rCell.GetSharedLength() << endl;
                continue;
            }

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
    rDestCol.maCellTextAttrs.set_empty(nRow1, nRow2); 

    sc::CellTextAttrStoreType::const_iterator itBlk = maCellTextAttrs.begin(), itBlkEnd = maCellTextAttrs.end();

    
    size_t nOffsetInBlock = 0;
    size_t nBlockStart = 0, nBlockEnd = 0, nRowPos = static_cast<size_t>(nRow1);
    for (; itBlk != itBlkEnd; ++itBlk)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
        {
            
            nOffsetInBlock = nRowPos - nBlockStart;
            break;
        }
    }

    if (itBlk == itBlkEnd)
        
        return;

    nRowPos = static_cast<size_t>(nRow2); 

    
    sc::celltextattr_block::const_iterator itData, itDataEnd;
    for (; itBlk != itBlkEnd; ++itBlk, nBlockStart = nBlockEnd, nOffsetInBlock = 0)
    {
        nBlockEnd = nBlockStart + itBlk->size;
        if (!itBlk->data)
        {
            
            if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
                
                rDestCol.maCellTextAttrs.set_empty(nBlockStart + nOffsetInBlock, nRowPos);
            else
                rDestCol.maCellTextAttrs.set_empty(nBlockStart + nOffsetInBlock, nBlockEnd-1);

            continue;
        }

        
        itData = sc::celltextattr_block::begin(*itBlk->data);
        itDataEnd = sc::celltextattr_block::end(*itBlk->data);
        std::advance(itData, nOffsetInBlock);

        if (nBlockStart <= nRowPos && nRowPos < nBlockEnd)
        {
            
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
    SCROW mnDestOffset; 
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
        miPos = mrDestNotes.set(miPos, nDestRow, p->Clone(aSrcPos, mrDestCol.GetDoc(), aDestPos, mbCloneCaption));
    }
};

}

void ScColumn::CopyCellNotesToDocument(
    SCROW nRow1, SCROW nRow2, ScColumn& rDestCol, bool bCloneCaption, SCROW nRowOffsetDest ) const
{
    CopyCellNotesHandler aFunc(*this, rDestCol, nRowOffsetDest, bCloneCaption);
    sc::ParseNote(maCellNotes.begin(), maCellNotes, nRow1, nRow2, aFunc);
}

void ScColumn::DuplicateNotes(SCROW nStartRow, size_t nDataSize, ScColumn& rDestCol, sc::ColumnBlockPosition& maDestBlockPos,
                              bool bCloneCaption, SCROW nRowOffsetDest ) const
{
    CopyCellNotesToDocument(nStartRow, nStartRow + nDataSize -1, rDestCol, bCloneCaption, nRowOffsetDest);
    maDestBlockPos.miCellNotePos = rDestCol.maCellNotes.begin();
}

void ScColumn::SwapCellTextAttrs( SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 == nRow2)
        return;

    if (nRow1 > nRow2)
        std::swap(nRow1, nRow2);

    sc::CellTextAttrStoreType::position_type aPos1 = maCellTextAttrs.position(nRow1);
    if (aPos1.first == maCellTextAttrs.end())
        return;

    sc::CellTextAttrStoreType::position_type aPos2 = maCellTextAttrs.position(aPos1.first, nRow2);
    if (aPos2.first == maCellTextAttrs.end())
        return;

    sc::CellTextAttrStoreType::iterator it1 = aPos1.first, it2 = aPos2.first;
    if (it1->type == it2->type)
    {
        if (it1->type == sc::element_type_empty)
            
            return;

        
        std::swap(
            sc::celltextattr_block::at(*it1->data, aPos1.second),
            sc::celltextattr_block::at(*it2->data, aPos2.second));

        return;
    }

    
    if (it1->type == sc::element_type_empty)
    {
        
        const sc::CellTextAttr& rVal2 = sc::celltextattr_block::at(*it2->data, aPos2.second);
        it1 = maCellTextAttrs.set(it1, nRow1, rVal2);
        maCellTextAttrs.set_empty(it1, nRow2, nRow2);
        return;
    }

    
    sc::CellTextAttr aVal1 = sc::celltextattr_block::at(*it1->data, aPos1.second); 
    it1 = maCellTextAttrs.set_empty(it1, nRow1, nRow1);
    maCellTextAttrs.set(it1, nRow2, aVal1);

    CellStorageModified();
}

void ScColumn::SwapCellNotes( SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 == nRow2)
        return;

    if (nRow1 > nRow2)
        std::swap(nRow1, nRow2);

    sc::CellNoteStoreType::position_type aPos1 = maCellNotes.position(nRow1);
    if (aPos1.first == maCellNotes.end())
        return;

    sc::CellNoteStoreType::position_type aPos2 = maCellNotes.position(aPos1.first, nRow2);
    if (aPos2.first == maCellNotes.end())
        return;

    sc::CellNoteStoreType::iterator it1 = aPos1.first, it2 = aPos2.first;
    if (it1->type == it2->type)
    {
        if (it1->type == sc::element_type_empty)
            
            return;

        
        std::swap(
            sc::cellnote_block::at(*it1->data, aPos1.second),
            sc::cellnote_block::at(*it2->data, aPos2.second));

        
        ScPostIt* pNote = sc::cellnote_block::at(*it1->data, aPos1.second);
        pNote->UpdateCaptionPos(ScAddress(nCol,nRow2,nTab));
        pNote = sc::cellnote_block::at(*it2->data, aPos2.second);
        pNote->UpdateCaptionPos(ScAddress(nCol,nRow1,nTab));

        return;
    }

    
    if (it1->type == sc::element_type_empty)
    {
        
        ScPostIt* pVal2 = sc::cellnote_block::at(*it2->data, aPos2.second);
        it1 = maCellNotes.set(it1, nRow1, pVal2);
        maCellNotes.release(it1, nRow2, pVal2);
        pVal2->UpdateCaptionPos(ScAddress(nCol,nRow1,nTab)); 

        return;
    }

    
    ScPostIt* pVal1 = NULL;
    it1 = maCellNotes.release(it1, nRow1, pVal1);
    assert(pVal1);
    maCellNotes.set(it1, nRow2, pVal1);
    pVal1->UpdateCaptionPos(ScAddress(nCol,nRow1,nTab));     
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
    sc::BroadcasterStoreType::iterator itPos = maBroadcasters.begin(), itPosEnd = maBroadcasters.end();
    for (; itPos != itPosEnd; ++itPos)
    {
        if (itPos->type == sc::element_type_broadcaster)
        {
            sc::broadcaster_block::iterator it = sc::broadcaster_block::begin(*itPos->data);
            sc::broadcaster_block::iterator itEnd = sc::broadcaster_block::end(*itPos->data);
            for (; it != itEnd; ++it)
                (*it)->PrepareForDestruction();
        }
    }
}

bool ScColumn::HasBroadcaster() const
{
    sc::BroadcasterStoreType::const_iterator it = maBroadcasters.begin(), itEnd = maBroadcasters.end();
    for (; it != itEnd; ++it)
    {
        if (it->type == sc::element_type_broadcaster)
            
            return true;
    }
    return false;
}

ScPostIt* ScColumn::GetCellNote(SCROW nRow)
{
    return maCellNotes.get<ScPostIt*>(nRow);
}

const ScPostIt* ScColumn::GetCellNote(SCROW nRow) const
{
    return maCellNotes.get<ScPostIt*>(nRow);
}

void ScColumn::SetCellNote(SCROW nRow, ScPostIt* pNote)
{
    
    maCellNotes.set(nRow, pNote);
}
void ScColumn::DeleteCellNote(SCROW nRow)
{
    maCellNotes.set_empty(nRow, nRow);
}
void ScColumn::DeleteCellNotes( sc::ColumnBlockPosition& rBlockPos, SCROW nRow1, SCROW nRow2 )
{
    rBlockPos.miCellNotePos =
        maCellNotes.set_empty(rBlockPos.miCellNotePos, nRow1, nRow2);
}

bool ScColumn::HasCellNotes() const
{
    sc::CellNoteStoreType::const_iterator it = maCellNotes.begin(), itEnd = maCellNotes.end();
    for (; it != itEnd; ++it)
    {
        if (it->type == sc::element_type_cellnote)
            
            return true;
    }
    return false;
}

SCROW ScColumn::GetCellNotesMaxRow() const
{
    
    SCROW maxRow = 0;
    sc::CellNoteStoreType::const_iterator it = maCellNotes.begin(), itEnd = maCellNotes.end();
    for (; it != itEnd; ++it)
    {
        if (it->type == sc::element_type_cellnote)
            maxRow = it->position + it->size -1;
    }
    return maxRow;
}
SCROW ScColumn::GetCellNotesMinRow() const
{
    
    SCROW minRow = 0;
    bool bFound = false;
    sc::CellNoteStoreType::const_iterator it = maCellNotes.begin(), itEnd = maCellNotes.end();
    for (; it != itEnd && !bFound; ++it)
    {
        if (it->type == sc::element_type_cellnote)
        {
            bFound = true;
            minRow = it->position;
        }
    }
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
    sc::CellTextAttrStoreType::iterator& itPos, SCROW nRow1, SCROW nRow2, sc::CellStoreType::iterator itrCells )
{
    if (!ValidRow(nRow1) || !ValidRow(nRow2) || nRow1 > nRow2)
        return 0;

    SCROW nRow = nRow1;
    std::pair<sc::CellTextAttrStoreType::iterator,size_t> aRet =
        maCellTextAttrs.position(itPos, nRow1);

    itPos = aRet.first; 

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
            if (UpdateScriptType(rVal, nRow, itrCells))
                bUpdated = true;
            nScriptType |= rVal.mnScriptType;
        }
    }
    else
    {
        
        nRow += itPos->size - aRet.second;
    }

    while (nRow <= nRow2)
    {
        ++itPos;
        if (itPos == maCellTextAttrs.end())
            return nScriptType;

        if (itPos->type != sc::element_type_celltextattr)
        {
            
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

void ScColumn::SetScriptType( SCROW nRow, sal_uInt8 nType )
{
    if (!ValidRow(nRow))
        return;

    sc::CellTextAttrStoreType::position_type aPos = maCellTextAttrs.position(nRow);
    if (aPos.first->type != sc::element_type_celltextattr)
        
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
            return formula::FormulaTokenRef(new formula::FormulaStringToken(rSS.getString()));
        }
        case sc::element_type_edittext:
        {
            const EditTextObject* pText = sc::edittext_block::at(*it->data, aPos.second);
            OUString aStr = ScEditUtil::GetString(*pText, pDocument);
            return formula::FormulaTokenRef(new formula::FormulaStringToken(aStr));
        }
        case sc::element_type_empty:
        default:
            
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

    ToMatrixHandler aFunc(rMat, nMatCol, nRow1, pDocument);
    sc::ParseAllNonEmpty(maCells.begin(), maCells, nRow1, nRow2, aFunc);
    return true;
}

namespace {

struct CellBucket
{
    SCSIZE mnNumValStart;
    SCSIZE mnStrValStart;
    std::vector<double> maNumVals;
    std::vector<svl::SharedString> maStrVals;

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
            const svl::SharedString* p = &maStrVals[0];
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
    ScDocument* mpDoc;
    svl::SharedStringPool& mrPool;

public:
    FillMatrixHandler(ScMatrix& rMat, size_t nMatCol, size_t nTopRow, SCCOL nCol, SCTAB nTab, ScDocument* pDoc) :
        mrMat(rMat), mnMatCol(nMatCol), mnTopRow(nTopRow), mnCol(nCol), mnTab(nTab), mpDoc(pDoc), mrPool(pDoc->GetSharedStringPool()) {}

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
                const svl::SharedString* p = &sc::string_block::at(*node.data, nOffset);
                mrMat.PutString(p, nDataSize, mnMatCol, nMatRow);
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
                    aSSs.push_back(mrPool.intern(aStr));
                }

                const svl::SharedString* p = &aSSs[0];
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
                            
                            aBucket.maNumVals.push_back(fVal);
                        }
                        else
                        {
                            
                            aBucket.flush(mrMat, mnMatCol);
                            aBucket.mnNumValStart = nThisRow - mnTopRow;
                            aBucket.maNumVals.push_back(fVal);
                        }
                        continue;
                    }

                    svl::SharedString aStr = rCell.GetString();
                    if (!aBucket.maStrVals.empty() && nThisRow == nPrevRow + 1)
                    {
                        
                        aBucket.maStrVals.push_back(aStr);
                    }
                    else
                    {
                        
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
    FillMatrixHandler aFunc(rMat, nMatCol, nRow1, nCol, nTab, pDocument);
    sc::ParseBlock(maCells.begin(), maCells, aFunc, nRow1, nRow2);
}

namespace {

template<typename _Blk>
void getBlockIterators(
    sc::CellStoreType::iterator it, size_t& rLenRemain,
    typename _Blk::iterator& rData, typename _Blk::iterator& rDataEnd )
{
    rData = _Blk::begin(*it->data);
    if (rLenRemain >= it->size)
    {
        
        rDataEnd = _Blk::end(*it->data);
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
    size_t nPos, size_t nArrayLen, sc::CellStoreType::iterator it, const sc::CellStoreType::iterator& itEnd )
{
    svl::SharedStringPool& rPool = pDoc->GetSharedStringPool();
    size_t nLenRemain = nArrayLen - nPos;
    double fNan;
    rtl::math::setNan(&fNan);

    for (; it != itEnd; ++it)
    {
        switch (it->type)
        {
            case sc::element_type_string:
            {
                sc::string_block::iterator itData, itDataEnd;
                getBlockIterators<sc::string_block>(it, nLenRemain, itData, itDataEnd);
                rCxt.ensureStrArray(rColArray, nArrayLen);

                for (; itData != itDataEnd; ++itData, ++nPos)
                    (*rColArray.mpStrArray)[nPos] = itData->getDataIgnoreCase();
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
                    (*rColArray.mpStrArray)[nPos] = rPool.intern(aStr).getDataIgnoreCase();
                }
            }
            break;
            case sc::element_type_formula:
            {
                sc::formula_block::iterator itData, itDataEnd;
                getBlockIterators<sc::formula_block>(it, nLenRemain, itData, itDataEnd);

                for (; itData != itDataEnd; ++itData, ++nPos)
                {
                    ScFormulaCell& rFC = **itData;
                    sc::FormulaResultValue aRes = rFC.GetResult();
                    if (aRes.meType == sc::FormulaResultValue::Invalid || aRes.mnError)
                    {
                        if (aRes.mnError == ScErrorCodes::errCircularReference)
                        {
                            
                            rFC.SetErrCode(0);
                            rFC.SetDirtyVar();
                        }
                        return false;
                    }

                    if (aRes.meType == sc::FormulaResultValue::String)
                    {
                        rCxt.ensureStrArray(rColArray, nArrayLen);
                        (*rColArray.mpStrArray)[nPos] = aRes.maString.getDataIgnoreCase();
                    }
                    else
                    {
                        rCxt.ensureNumArray(rColArray, nArrayLen);
                        (*rColArray.mpNumArray)[nPos] = aRes.mfValue;
                    }
                }
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
                *itArray = it->getDataIgnoreCase();
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
                *itArray = rPool.intern(aStr).getDataIgnoreCase();
            }
        }
        break;
        default:
            ;
    }
}

sc::FormulaGroupContext::ColArray*
copyFirstFormulaBlock(
    sc::FormulaGroupContext& rCxt, sc::CellStoreType::iterator itBlk, size_t nArrayLen,
    SCTAB nTab, SCCOL nCol )
{
    double fNan;
    rtl::math::setNan(&fNan);

    size_t nLen = std::min(itBlk->size, nArrayLen);

    sc::formula_block::iterator it = sc::formula_block::begin(*itBlk->data);
    sc::formula_block::iterator itEnd;

    sc::FormulaGroupContext::NumArrayType* pNumArray = NULL;
    sc::FormulaGroupContext::StrArrayType* pStrArray = NULL;

    itEnd = it;
    std::advance(itEnd, nLen);
    size_t nPos = 0;
    for (; it != itEnd; ++it, ++nPos)
    {
        ScFormulaCell& rFC = **it;
        sc::FormulaResultValue aRes = rFC.GetResult();
        if (aRes.meType == sc::FormulaResultValue::Invalid || aRes.mnError)
        {
            if (aRes.mnError == ScErrorCodes::errCircularReference)
            {
                
                rFC.SetErrCode(0);
                rFC.SetDirtyVar();
            }
            return NULL;
        }

        if (aRes.meType == sc::FormulaResultValue::Value)
        {
            if (!pNumArray)
            {
                rCxt.maNumArrays.push_back(
                    new sc::FormulaGroupContext::NumArrayType(nArrayLen, fNan));
                pNumArray = &rCxt.maNumArrays.back();
            }

            (*pNumArray)[nPos] = aRes.mfValue;
        }
        else
        {
            if (!pStrArray)
            {
                rCxt.maStrArrays.push_back(
                    new sc::FormulaGroupContext::StrArrayType(nArrayLen, NULL));
                pStrArray = &rCxt.maStrArrays.back();
            }

            (*pStrArray)[nPos] = aRes.maString.getDataIgnoreCase();
        }
    }

    if (!pNumArray && !pStrArray)
        
        return NULL;

    return rCxt.setCachedColArray(nTab, nCol, pNumArray, pStrArray);
}

struct FiniteValueFinder : std::unary_function<double, bool>
{
    bool operator() (double f) const { return !rtl::math::isNan(f); }
};

struct NonNullStringFinder : std::unary_function<const rtl_uString*, bool>
{
    bool operator() (const rtl_uString* p) const { return p != NULL; }
};

bool hasNonEmpty( const sc::FormulaGroupContext::StrArrayType& rArray, SCROW nRow1, SCROW nRow2 )
{
    
    sc::FormulaGroupContext::StrArrayType::const_iterator it = rArray.begin();
    std::advance(it, nRow1);
    sc::FormulaGroupContext::StrArrayType::const_iterator itEnd = it;
    std::advance(itEnd, nRow2-nRow1+1);
    return std::find_if(it, itEnd, NonNullStringFinder()) != itEnd;
}

}

formula::VectorRefArray ScColumn::FetchVectorRefArray( SCROW nRow1, SCROW nRow2 )
{
    if (nRow1 > nRow2)
        return formula::VectorRefArray();

    
    sc::FormulaGroupContext& rCxt = pDocument->GetFormulaGroupContext();
    sc::FormulaGroupContext::ColArray* pColArray = rCxt.getCachedColArray(nTab, nCol, nRow2+1);
    if (pColArray)
    {
        const double* pNum = NULL;
        if (pColArray->mpNumArray)
            pNum = &(*pColArray->mpNumArray)[nRow1];

        rtl_uString** pStr = NULL;
        if (pColArray->mpStrArray && hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
            pStr = &(*pColArray->mpStrArray)[nRow1];

        return formula::VectorRefArray(pNum, pStr);
    }

    double fNan;
    rtl::math::setNan(&fNan);

    
    sc::CellStoreType::iterator itBlk = maCells.begin();
    switch (itBlk->type)
    {
        case sc::element_type_numeric:
        {
            if (static_cast<size_t>(nRow2) < itBlk->size)
            {
                
                const double* p = &sc::numeric_block::at(*itBlk->data, nRow1);
                return formula::VectorRefArray(p);
            }

            
            sc::numeric_block::const_iterator it = sc::numeric_block::begin(*itBlk->data);
            sc::numeric_block::const_iterator itEnd = sc::numeric_block::end(*itBlk->data);
            rCxt.maNumArrays.push_back(new sc::FormulaGroupContext::NumArrayType(it, itEnd));
            sc::FormulaGroupContext::NumArrayType& rArray = rCxt.maNumArrays.back();
            rArray.resize(nRow2+1, fNan); 

            pColArray = rCxt.setCachedColArray(nTab, nCol, &rArray, NULL);
            if (!pColArray)
                
                return formula::VectorRefArray();

            
            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(pDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
                return formula::VectorRefArray();

            if (pColArray->mpStrArray)
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1], &(*pColArray->mpStrArray)[nRow1]);
            else
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1]);
        }
        break;
        case sc::element_type_string:
        case sc::element_type_edittext:
        {
            rCxt.maStrArrays.push_back(new sc::FormulaGroupContext::StrArrayType(nRow2+1, NULL));
            sc::FormulaGroupContext::StrArrayType& rArray = rCxt.maStrArrays.back();
            pColArray = rCxt.setCachedColArray(nTab, nCol, NULL, &rArray);
            if (!pColArray)
                
                return formula::VectorRefArray();

            if (static_cast<size_t>(nRow2) < itBlk->size)
            {
                
                copyFirstStringBlock(*pDocument, rArray, nRow2+1, itBlk);
                return formula::VectorRefArray(&rArray[nRow1]);
            }

            copyFirstStringBlock(*pDocument, rArray, itBlk->size, itBlk);

            
            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(pDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
                return formula::VectorRefArray();

            if (pColArray->mpNumArray)
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1], &(*pColArray->mpStrArray)[nRow1]);
            else
                return formula::VectorRefArray(&(*pColArray->mpStrArray)[nRow1]);
        }
        break;
        case sc::element_type_formula:
        {
            if (static_cast<size_t>(nRow2) < itBlk->size)
            {
                
                
                pColArray = copyFirstFormulaBlock(rCxt, itBlk, nRow2+1, nTab, nCol);
                if (!pColArray)
                    
                    return formula::VectorRefArray();

                const double* pNum = NULL;
                rtl_uString** pStr = NULL;
                if (pColArray->mpNumArray)
                    pNum = &(*pColArray->mpNumArray)[nRow1];
                if (pColArray->mpStrArray)
                    pStr = &(*pColArray->mpStrArray)[nRow1];

                return formula::VectorRefArray(pNum, pStr);
            }

            pColArray = copyFirstFormulaBlock(rCxt, itBlk, nRow2+1, nTab, nCol);
            if (!pColArray)
                
                return formula::VectorRefArray();

            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(pDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
                return formula::VectorRefArray();

            const double* pNum = NULL;
            rtl_uString** pStr = NULL;
            if (pColArray->mpNumArray)
                pNum = &(*pColArray->mpNumArray)[nRow1];
            if (pColArray->mpStrArray)
                pStr = &(*pColArray->mpStrArray)[nRow1];

            return formula::VectorRefArray(pNum, pStr);
        }
        break;
        case sc::element_type_empty:
        {
            
            rCxt.maNumArrays.push_back(new sc::FormulaGroupContext::NumArrayType(nRow2+1, fNan));
            sc::FormulaGroupContext::NumArrayType& rArray = rCxt.maNumArrays.back();
            pColArray = rCxt.setCachedColArray(nTab, nCol, &rArray, NULL);
            if (!pColArray)
                
                return formula::VectorRefArray();

            if (static_cast<size_t>(nRow2) < itBlk->size)
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1]);

            
            size_t nPos = itBlk->size;
            ++itBlk;
            if (!appendToBlock(pDocument, rCxt, *pColArray, nPos, nRow2+1, itBlk, maCells.end()))
                return formula::VectorRefArray();

            if (pColArray->mpStrArray && hasNonEmpty(*pColArray->mpStrArray, nRow1, nRow2))
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1], &(*pColArray->mpStrArray)[nRow1]);
            else
                return formula::VectorRefArray(&(*pColArray->mpNumArray)[nRow1]);
        }
        break;
        default:
            ;
    }

    return formula::VectorRefArray();
}

void ScColumn::SetFormulaResults( SCROW nRow, const double* pResults, size_t nLen )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
        
        return;

    size_t nBlockLen = it->size - aPos.second;
    if (nBlockLen < nLen)
        
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

void ScColumn::SetFormulaResults( SCROW nRow, const formula::FormulaTokenRef* pResults, size_t nLen )
{
    sc::CellStoreType::position_type aPos = maCells.position(nRow);
    sc::CellStoreType::iterator it = aPos.first;
    if (it->type != sc::element_type_formula)
        
        return;

    size_t nBlockLen = it->size - aPos.second;
    if (nBlockLen < nLen)
        
        return;

    sc::formula_block::iterator itCell = sc::formula_block::begin(*it->data);
    std::advance(itCell, aPos.second);

    const formula::FormulaTokenRef* pResEnd = pResults + nLen;
    for (; pResults != pResEnd; ++pResults, ++itCell)
    {
        ScFormulaCell& rCell = **itCell;
        rCell.SetResultToken(pResults->get());
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
        
        return NULL;

    return sc::formula_block::at(*it->data, aPos.second);
}

void ScColumn::FindDataAreaPos(SCROW& rRow, bool bDown) const
{
    
    
    

    std::pair<sc::CellStoreType::const_iterator,size_t> aPos = maCells.position(rRow);
    sc::CellStoreType::const_iterator it = aPos.first;
    if (it == maCells.end())
        
        return;

    if (it->type == sc::element_type_empty)
    {
        
        rRow = FindNextVisibleRowWithContent(it, rRow, bDown);
        return;
    }

    
    SCROW nNextRow = FindNextVisibleRow(rRow, bDown);
    aPos = maCells.position(it, nNextRow);
    it = aPos.first;
    if (it->type == sc::element_type_empty)
    {
        
        rRow = FindNextVisibleRowWithContent(it, nNextRow, bDown);
        return;
    }

    
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
        
        SCROW nLastData = GetLastDataPos();    

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
            
            SvtBroadcaster* pBC = sc::broadcaster_block::at(*itBlockPos->data, nElemPos);
            rLst.StartListening(*pBC);
        }
        break;
        case mdds::mtv::element_type_empty:
        {
            
            SvtBroadcaster* pBC = new SvtBroadcaster;
            rLst.StartListening(*pBC);
            itBlockPos = rStore.set(itBlockPos, nRow, pBC); 
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
    
    if (!rSource.HasListeners())
        
        return;

    
    SvtBroadcaster* pBC = GetBroadcaster(nDestRow);
    if (!pBC)
    {
        pBC = new SvtBroadcaster;
        maBroadcasters.set(nDestRow, pBC);
    }

    SvtBroadcaster::ListenersType& rListeners = rSource.GetAllListeners();
    SvtBroadcaster::ListenersType::iterator it = rListeners.begin(), itEnd = rListeners.end();
    for (; it != itEnd; ++it)
    {
        SvtListener& rLst = **it;
        rLst.StartListening(*pBC);
        rLst.EndListening(rSource);
    }
}

void ScColumn::EndListening( SvtListener& rLst, SCROW nRow )
{
    SvtBroadcaster* pBC = GetBroadcaster(nRow);
    if (!pBC)
        return;

    rLst.EndListening(*pBC);
    if (!pBC->HasListeners())
        
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
    it = aPos.first; 
    startListening(maBroadcasters, it, aPos.second, nRow, rLst);
}

void ScColumn::EndListening( sc::EndListeningContext& rCxt, SCROW nRow, SvtListener& rListener )
{
    sc::ColumnBlockPosition* p = rCxt.getBlockPosition(nTab, nCol);
    if (!p)
        return;

    sc::BroadcasterStoreType::iterator& it = p->miBroadcasterPos;
    std::pair<sc::BroadcasterStoreType::iterator,size_t> aPos = maBroadcasters.position(it, nRow);
    it = aPos.first; 
    if (it->type != sc::element_type_broadcaster)
        return;

    SvtBroadcaster* pBC = sc::broadcaster_block::at(*it->data, aPos.second);
    OSL_ASSERT(pBC);

    rListener.EndListening(*pBC);
    if (!pBC->HasListeners())
        
        rCxt.addEmptyBroadcasterPosition(nTab, nCol, nRow);
}

namespace {

class CompileDBFormulaHandler
{
    sc::CompileFormulaContext& mrCxt;

public:
    CompileDBFormulaHandler( sc::CompileFormulaContext& rCxt ) :
        mrCxt(rCxt) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileDBFormula(mrCxt);
    }
};

class CompileDBFormula2Handler
{
    sc::CompileFormulaContext& mrCxt;
    bool mbCreateFormulaString;

public:
    CompileDBFormula2Handler( sc::CompileFormulaContext& rCxt, bool bCreateFormulaString ) :
        mrCxt(rCxt), mbCreateFormulaString(bCreateFormulaString) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileDBFormula(mrCxt, mbCreateFormulaString);
    }
};

class CompileNameFormulaHandler
{
    sc::CompileFormulaContext& mrCxt;
    bool mbCreateFormulaString;

public:
    CompileNameFormulaHandler( sc::CompileFormulaContext& rCxt, bool bCreateFormulaString) :
        mrCxt(rCxt), mbCreateFormulaString(bCreateFormulaString) {}

    void operator() (size_t, ScFormulaCell* p)
    {
        p->CompileNameFormula(mrCxt, mbCreateFormulaString);
    }
};

struct CompileColRowNameFormulaHandler
{
    sc::CompileFormulaContext& mrCxt;
public:
    CompileColRowNameFormulaHandler( sc::CompileFormulaContext& rCxt ) : mrCxt(rCxt) {}

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

void ScColumn::CompileDBFormula( sc::CompileFormulaContext& rCxt, bool bCreateFormulaString )
{
    CompileDBFormula2Handler aFunc(rCxt, bCreateFormulaString);
    sc::ProcessFormula(maCells, aFunc);
    RegroupFormulaCells();
}

void ScColumn::CompileNameFormula( sc::CompileFormulaContext& rCxt, bool bCreateFormulaString )
{
    CompileNameFormulaHandler aFunc(rCxt, bCreateFormulaString);
    sc::ProcessFormula(maCells, aFunc);
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
        if (mrData.bError)
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
            case SUBTOTAL_FUNC_CNT:             
            {
                if (!bVal)
                    return;

                ++mrData.nCount;
            }
            break;
            case SUBTOTAL_FUNC_CNT2:            
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
                
            }
        }
    }

public:
    UpdateSubTotalHandler(ScFunctionData& rData) : mrData(rData) {}

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
        if (mrData.eFunc != SUBTOTAL_FUNC_CNT2) 
        {

            if (pCell->GetErrCode())
            {
                if (mrData.eFunc != SUBTOTAL_FUNC_CNT) 
                    mrData.bError = true;
            }
            else if (pCell->IsValue())
            {
                fVal = pCell->GetValue();
                bVal = true;
            }
            
        }

        update(fVal, bVal);
    }
};

}


void ScColumn::UpdateSelectionFunction(
    const ScMarkData& rMark, ScFunctionData& rData, ScFlatBoolRowSegments& rHiddenRows )
{
    sc::SingleColumnSpanSet aSpanSet;
    aSpanSet.scan(rMark, nTab, nCol); 

    
    ScFlatBoolRowSegments::RangeData aRange;
    SCROW nRow = 0;
    while (nRow <= MAXROW)
    {
        if (!rHiddenRows.getRangeData(nRow, aRange))
            break;

        if (aRange.mbValue)
            
            aSpanSet.set(nRow, aRange.mnRow2, false);

        nRow = aRange.mnRow2 + 1;
    }

    sc::SingleColumnSpanSet::SpansType aSpans;
    aSpanSet.getSpans(aSpans);

    sc::SingleColumnSpanSet::SpansType::const_iterator it = aSpans.begin(), itEnd = aSpans.end();

    switch (rData.eFunc)
    {
        case SUBTOTAL_FUNC_SELECTION_COUNT:
        {
            
            for (; it != itEnd; ++it)
                rData.nCount += it->mnRow2 - it->mnRow1 + 1;
        }
        break;
        case SUBTOTAL_FUNC_CNT2:
        {
            
            sc::CellStoreType::const_iterator itCellPos = maCells.begin();
            UpdateSubTotalHandler aFunc(rData);
            for (; it != itEnd; ++it)
            {
                itCellPos = sc::ParseAllNonEmpty(
                    itCellPos, maCells, it->mnRow1, it->mnRow2, aFunc);
            }
        }
        break;
        default:
        {
            
            sc::CellStoreType::const_iterator itCellPos = maCells.begin();
            UpdateSubTotalHandler aFunc(rData);
            for (; it != itEnd; ++it)
            {
                itCellPos = sc::ParseFormulaNumeric(
                    itCellPos, maCells, it->mnRow1, it->mnRow2, aFunc);
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
        switch (node.type)
        {
            case sc::element_type_numeric:
            case sc::element_type_string:
                mnCount += node.size;
            break;
            case sc::element_type_formula:
            {
                
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

SCSIZE ScColumn::GetPatternCount() const
{
    return pAttrArray ? pAttrArray->Count() : 0;
}

SCSIZE ScColumn::GetPatternCount( SCROW nRow1, SCROW nRow2 ) const
{
    return pAttrArray ? pAttrArray->Count( nRow1, nRow2 ) : 0;
}

bool ScColumn::ReservePatternCount( SCSIZE nReserve )
{
    return pAttrArray ? pAttrArray->Reserve( nReserve ) : false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
