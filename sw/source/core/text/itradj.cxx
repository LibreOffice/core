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

#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/outdev.hxx>
#include <IDocumentSettingAccess.hxx>

#include "frame.hxx"
#include "paratr.hxx"
#include "itrtxt.hxx"
#include "porglue.hxx"
#include "porlay.hxx"
#include "porfly.hxx"
#include "pordrop.hxx"
#include "pormulti.hxx"
#include <portab.hxx>

#define MIN_TAB_WIDTH 60

using namespace ::com::sun::star;

void SwTxtAdjuster::FormatBlock( )
{
    // Block format does not apply to the last line.
    // And for tabs it doesn't exist out of tradition
    // If we have Flys we continue.

    const SwLinePortion *pFly = 0;

    bool bSkip = !IsLastBlock() &&
        nStart + pCurr->GetLen() >= GetInfo().GetTxt().getLength();

    // Multi-line fields are tricky, because we need to check whether there are
    // any other text portions in the paragraph.
    if( bSkip )
    {
        const SwLineLayout *pLay = pCurr->GetNext();
        while( pLay && !pLay->GetLen() )
        {
            const SwLinePortion *pPor = pCurr->GetFirstPortion();
            while( pPor && bSkip )
            {
                if( pPor->InTxtGrp() )
                    bSkip = false;
                pPor = pPor->GetPortion();
            }
            pLay = bSkip ? pLay->GetNext() : 0;
        }
    }

    if( bSkip )
    {
        if( !GetInfo().GetParaPortion()->HasFly() )
        {
            if( IsLastCenter() )
                CalcFlyAdjust( pCurr );
            pCurr->FinishSpaceAdd();
            return;
        }
        else
        {
            const SwLinePortion *pTmpFly = NULL;

            // End at the last Fly
            const SwLinePortion *pPos = pCurr->GetFirstPortion();
            while( pPos )
            {
                // Look for the last Fly which has text coming after it:
                if( pPos->IsFlyPortion() )
                    pTmpFly = pPos; // Found a Fly
                else if ( pTmpFly && pPos->InTxtGrp() )
                {
                    pFly = pTmpFly; // A Fly with follow-up text!
                    pTmpFly = NULL;
                }
                pPos = pPos->GetPortion();
            }
            // End if we didn't find one
            if( !pFly )
            {
                if( IsLastCenter() )
                    CalcFlyAdjust( pCurr );
                pCurr->FinishSpaceAdd();
                return;
            }
        }
    }

    const sal_Int32 nOldIdx = GetInfo().GetIdx();
    GetInfo().SetIdx( nStart );
    CalcNewBlock( pCurr, pFly );
    GetInfo().SetIdx( nOldIdx );
    GetInfo().GetParaPortion()->GetRepaint()->SetOfst(0);
}

static bool lcl_CheckKashidaPositions( SwScriptInfo& rSI, SwTxtSizeInfo& rInf, SwTxtIter& rItr,
                                sal_Int32& rKashidas, sal_Int32& nGluePortion )
{
    // i60594 validate Kashida justification
    sal_Int32 nIdx = rItr.GetStart();
    sal_Int32 nEnd = rItr.GetEnd();

    // Note on calling KashidaJustify():
    // Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
    // total number of kashida positions, or the number of kashida positions after some positions
    // have been dropped.
    // Here we want the clean total, which is OK: We have called ClearKashidaInvalid() before.
    rKashidas = rSI.KashidaJustify ( 0, 0, rItr.GetStart(), rItr.GetLength(), 0 );

    if (rKashidas <= 0) // nothing to do
        return true;

    // kashida positions found in SwScriptInfo are not necessarily valid in every font
    // if two characters are replaced by a ligature glyph, there will be no place for a kashida
    sal_Int32* pKashidaPos = new sal_Int32[ rKashidas ];
    sal_Int32* pKashidaPosDropped = new sal_Int32[ rKashidas ];
    rSI.GetKashidaPositions ( nIdx, rItr.GetLength(), pKashidaPos );
    sal_Int32 nKashidaIdx = 0;
    while ( rKashidas && nIdx < nEnd )
    {
        rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
        sal_Int32 nNext = rItr.GetNextAttr();

        // is there also a script change before?
        // if there is, nNext should point to the script change
        sal_Int32 nNextScript = rSI.NextScriptChg( nIdx );
        if( nNextScript < nNext )
            nNext = nNextScript;

        if ( nNext == COMPLETE_STRING || nNext > nEnd )
            nNext = nEnd;
        sal_Int32 nKashidasInAttr = rSI.KashidaJustify ( 0, 0, nIdx, nNext - nIdx );
        if (nKashidasInAttr > 0)
        {
            // Kashida glyph looks suspicious, skip Kashida justification
            if ( rInf.GetOut()->GetMinKashida() <= 0 )
            {
                delete[] pKashidaPos;
                delete[] pKashidaPosDropped;
                return false;
            }

            sal_Int32 nKashidasDropped = 0;
            if ( !SwScriptInfo::IsArabicText( rInf.GetTxt(), nIdx, nNext - nIdx ) )
            {
                nKashidasDropped = nKashidasInAttr;
                rKashidas -= nKashidasDropped;
            }
            else
            {
                sal_uLong nOldLayout = rInf.GetOut()->GetLayoutMode();
                rInf.GetOut()->SetLayoutMode ( nOldLayout | TEXT_LAYOUT_BIDI_RTL );
                nKashidasDropped = rInf.GetOut()->ValidateKashidas ( rInf.GetTxt(), nIdx, nNext - nIdx,
                                               nKashidasInAttr, pKashidaPos + nKashidaIdx,
                                               pKashidaPosDropped );
                rInf.GetOut()->SetLayoutMode ( nOldLayout );
                if ( nKashidasDropped )
                {
                    rSI.MarkKashidasInvalid(nKashidasDropped, pKashidaPosDropped);
                    rKashidas -= nKashidasDropped;
                    nGluePortion -= nKashidasDropped;
                }
            }
            nKashidaIdx += nKashidasInAttr;
        }
        nIdx = nNext;
    }
    delete[] pKashidaPos;
    delete[] pKashidaPosDropped;

    // return false if all kashidas have been eliminated
    return (rKashidas > 0);
}

static bool lcl_CheckKashidaWidth ( SwScriptInfo& rSI, SwTxtSizeInfo& rInf, SwTxtIter& rItr, sal_Int32& rKashidas,
                             sal_Int32& nGluePortion, const long nGluePortionWidth, long& nSpaceAdd )
{
    // check kashida width
    // if width is smaller than minimal kashida width allowed by fonts in the current line
    // drop one kashida after the other until kashida width is OK
    bool bAddSpaceChanged;
    while (rKashidas)
    {
        bAddSpaceChanged = false;
        sal_Int32 nIdx = rItr.GetStart();
        sal_Int32 nEnd = rItr.GetEnd();
        while ( nIdx < nEnd )
        {
            rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
            sal_Int32 nNext = rItr.GetNextAttr();

            // is there also a script change before?
            // if there is, nNext should point to the script change
            sal_Int32 nNextScript = rSI.NextScriptChg( nIdx );
            if( nNextScript < nNext )
               nNext = nNextScript;

            if ( nNext == COMPLETE_STRING || nNext > nEnd )
                nNext = nEnd;
            sal_Int32 nKashidasInAttr = rSI.KashidaJustify ( 0, 0, nIdx, nNext - nIdx );

            long nFontMinKashida = rInf.GetOut()->GetMinKashida();
            if ( nFontMinKashida && nKashidasInAttr > 0 && SwScriptInfo::IsArabicText( rInf.GetTxt(), nIdx, nNext - nIdx ) )
            {
                sal_Int32 nKashidasDropped = 0;
                while ( rKashidas && nGluePortion && nKashidasInAttr > 0 &&
                        nSpaceAdd / SPACING_PRECISION_FACTOR < nFontMinKashida )
                {
                    --nGluePortion;
                    --rKashidas;
                    --nKashidasInAttr;
                    ++nKashidasDropped;
                    if( !rKashidas || !nGluePortion ) // nothing left, return false to
                        return false;                 // do regular blank justification

                    nSpaceAdd = nGluePortionWidth / nGluePortion;
                    bAddSpaceChanged = true;
               }
               if( nKashidasDropped )
                   rSI.MarkKashidasInvalid( nKashidasDropped, nIdx, nNext - nIdx );
            }
            if ( bAddSpaceChanged )
                break; // start all over again
            nIdx = nNext;
        }
        if ( !bAddSpaceChanged )
            break; // everything was OK
    }
   return true;
}

// CalcNewBlock() must only be called _after_ CalcLine()!
// We always span between two RandPortions or FixPortions (Tabs and Flys).
// We count the Glues and call ExpandBlock.
void SwTxtAdjuster::CalcNewBlock( SwLineLayout *pCurrent,
                                  const SwLinePortion *pStopAt, SwTwips nReal, bool bSkipKashida )
{
    OSL_ENSURE( GetInfo().IsMulti() || SVX_ADJUST_BLOCK == GetAdjust(),
            "CalcNewBlock: Why?" );
    OSL_ENSURE( pCurrent->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );

    pCurrent->InitSpaceAdd();
    sal_Int32 nGluePortion = 0;
    sal_Int32 nCharCnt = 0;
    MSHORT nSpaceIdx = 0;

    // i60591: hennerdrews
    SwScriptInfo& rSI = GetInfo().GetParaPortion()->GetScriptInfo();
    SwTxtSizeInfo aInf ( GetTxtFrm() );
    SwTxtIter aItr ( GetTxtFrm(), &aInf );

    if ( rSI.CountKashida() )
    {
        while (aItr.GetCurr() != pCurrent && aItr.GetNext())
           aItr.Next();

        if( bSkipKashida )
        {
            rSI.SetNoKashidaLine ( aItr.GetStart(), aItr.GetLength());
        }
        else
        {
            rSI.ClearKashidaInvalid ( aItr.GetStart(), aItr.GetLength() );
            rSI.ClearNoKashidaLine( aItr.GetStart(), aItr.GetLength() );
        }
    }

    // Do not forget: CalcRightMargin() sets pCurrent->Width() to the line width!
    if (!bSkipKashida)
        CalcRightMargin( pCurrent, nReal );

    // #i49277#
    const bool bDoNotJustifyLinesWithManualBreak =
                GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK);

    SwLinePortion *pPos = pCurrent->GetPortion();

    while( pPos )
    {
        if ( bDoNotJustifyLinesWithManualBreak &&
             pPos->IsBreakPortion() && !IsLastBlock() )
        {
           pCurrent->FinishSpaceAdd();
           break;
        }

        if ( pPos->InTxtGrp() )
            nGluePortion = nGluePortion + ((SwTxtPortion*)pPos)->GetSpaceCnt( GetInfo(), nCharCnt );
        else if( pPos->IsMultiPortion() )
        {
            SwMultiPortion* pMulti = (SwMultiPortion*)pPos;
            // a multiportion with a tabulator inside breaks the text adjustment
            // a ruby portion will not be stretched by text adjustment
            // a double line portion takes additional space for each blank
            // in the wider line
            if( pMulti->HasTabulator() )
            {
                if ( nSpaceIdx == pCurrent->GetLLSpaceAddCount() )
                    pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );

                nSpaceIdx++;
                nGluePortion = 0;
                nCharCnt = 0;
            }
            else if( pMulti->IsDouble() )
                nGluePortion = nGluePortion + ((SwDoubleLinePortion*)pMulti)->GetSpaceCnt();
            else if ( pMulti->IsBidi() )
                nGluePortion = nGluePortion + ((SwBidiPortion*)pMulti)->GetSpaceCnt( GetInfo() );  // i60594
        }

        if( pPos->InGlueGrp() )
        {
            if( pPos->InFixMargGrp() )
            {
                if ( nSpaceIdx == pCurrent->GetLLSpaceAddCount() )
                    pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );

                const long nGluePortionWidth = static_cast<SwGluePortion*>(pPos)->GetPrtGlue() *
                                               SPACING_PRECISION_FACTOR;

                sal_Int32 nKashidas = 0;
                if( nGluePortion && rSI.CountKashida() && !bSkipKashida )
                {
                    // kashida positions found in SwScriptInfo are not necessarily valid in every font
                    // if two characters are replaced by a ligature glyph, there will be no place for a kashida
                    if ( !lcl_CheckKashidaPositions ( rSI, aInf, aItr, nKashidas, nGluePortion ))
                    {
                        // all kashida positions are invalid
                        // do regular blank justification
                        pCurrent->FinishSpaceAdd();
                        GetInfo().SetIdx( nStart );
                        CalcNewBlock( pCurrent, pStopAt, nReal, true );
                        return;
                    }
                }

                if( nGluePortion )
                {
                    long nSpaceAdd = nGluePortionWidth / nGluePortion;

                    // i60594
                    if( rSI.CountKashida() && !bSkipKashida )
                    {
                        if( !lcl_CheckKashidaWidth( rSI, aInf, aItr, nKashidas, nGluePortion, nGluePortionWidth, nSpaceAdd ))
                        {
                            // no kashidas left
                            // do regular blank justification
                            pCurrent->FinishSpaceAdd();
                            GetInfo().SetIdx( nStart );
                            CalcNewBlock( pCurrent, pStopAt, nReal, true );
                            return;
                        }
                    }

                    pCurrent->SetLLSpaceAdd( nSpaceAdd , nSpaceIdx );
                    pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
                }
                else if ( IsOneBlock() && nCharCnt > 1 )
                {
                    const long nSpaceAdd = - nGluePortionWidth / ( nCharCnt - 1 );
                    pCurrent->SetLLSpaceAdd( nSpaceAdd, nSpaceIdx );
                    pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
                }

                nSpaceIdx++;
                nGluePortion = 0;
                nCharCnt = 0;
            }
            else
                ++nGluePortion;
        }
        GetInfo().SetIdx( GetInfo().GetIdx() + pPos->GetLen() );
        if ( pPos == pStopAt )
        {
            pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );
            break;
        }
        pPos = pPos->GetPortion();
    }
}

SwTwips SwTxtAdjuster::CalcKanaAdj( SwLineLayout* pCurrent )
{
    OSL_ENSURE( pCurrent->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );
    OSL_ENSURE( !pCurrent->GetpKanaComp(), "pKanaComp already exists!!" );

    std::deque<sal_uInt16> *pNewKana = new std::deque<sal_uInt16>();
    pCurrent->SetKanaComp( pNewKana );

    const sal_uInt16 nNull = 0;
    MSHORT nKanaIdx = 0;
    long nKanaDiffSum = 0;
    SwTwips nRepaintOfst = 0;
    SwTwips nX = 0;
    bool bNoCompression = false;

    // Do not forget: CalcRightMargin() sets pCurrent->Width() to the line width!
    CalcRightMargin( pCurrent, 0 );

    SwLinePortion* pPos = pCurrent->GetPortion();

    while( pPos )
    {
        if ( pPos->InTxtGrp() )
        {
            // get maximum portion width from info structure, calculated
            // during text formatting
            sal_uInt16 nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pPos );

            // check, if information is stored under other key
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pCurrent );

            // calculate difference between portion width and max. width
            nKanaDiffSum += nMaxWidthDiff;

            // we store the beginning of the first compressable portion
            // for repaint
            if ( nMaxWidthDiff && !nRepaintOfst )
                nRepaintOfst = nX + GetLeftMargin();
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            if ( nKanaIdx == pCurrent->GetKanaComp().size() )
                pCurrent->GetKanaComp().push_back( nNull );

            sal_uInt16 nRest;

            if ( pPos->InTabGrp() )
            {
                nRest = ! bNoCompression &&
                        ( pPos->Width() > MIN_TAB_WIDTH ) ?
                        pPos->Width() - MIN_TAB_WIDTH :
                        0;

                // for simplifying the handling of left, right ... tabs,
                // we do expand portions, which are lying behind
                // those special tabs
                bNoCompression = !pPos->IsTabLeftPortion();
            }
            else
            {
                nRest = ! bNoCompression ?
                        ((SwGluePortion*)pPos)->GetPrtGlue() :
                        0;

                bNoCompression = false;
            }

            if( nKanaDiffSum )
            {
                sal_uLong nCompress = ( 10000 * nRest ) / nKanaDiffSum;

                if ( nCompress >= 10000 )
                    // kanas can be expanded to 100%, and there is still
                    // some space remaining
                    nCompress = 0;

                else
                    nCompress = 10000 - nCompress;

                ( pCurrent->GetKanaComp() )[ nKanaIdx ] = (sal_uInt16)nCompress;
                nKanaDiffSum = 0;
            }

            nKanaIdx++;
        }

        nX += pPos->Width();
        pPos = pPos->GetPortion();
    }

    // set portion width
    nKanaIdx = 0;
    sal_uInt16 nCompress = ( pCurrent->GetKanaComp() )[ nKanaIdx ];
    pPos = pCurrent->GetPortion();
    long nDecompress = 0;
    nKanaDiffSum = 0;

    while( pPos )
    {
        if ( pPos->InTxtGrp() )
        {
            const sal_uInt16 nMinWidth = pPos->Width();

            // get maximum portion width from info structure, calculated
            // during text formatting
            sal_uInt16 nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pPos );

            // check, if information is stored under other key
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( (sal_uLong)pCurrent );
            nKanaDiffSum += nMaxWidthDiff;
            pPos->Width( nMinWidth +
                       ( ( 10000 - nCompress ) * nMaxWidthDiff ) / 10000 );
            nDecompress += pPos->Width() - nMinWidth;
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            if( nCompress )
            {
                nKanaDiffSum *= nCompress;
                nKanaDiffSum /= 10000;
            }

            pPos->Width( static_cast<sal_uInt16>(pPos->Width() - nDecompress) );

            if ( pPos->InTabGrp() )
                // set fix width to width
                ((SwTabPortion*)pPos)->SetFixWidth( pPos->Width() );

            if ( ++nKanaIdx < pCurrent->GetKanaComp().size() )
                nCompress = ( pCurrent->GetKanaComp() )[ nKanaIdx ];

            nKanaDiffSum = 0;
            nDecompress = 0;
        }
        pPos = pPos->GetPortion();
    }

    return nRepaintOfst;
}

SwMarginPortion *SwTxtAdjuster::CalcRightMargin( SwLineLayout *pCurrent,
    SwTwips nReal )
{
    long nRealWidth;
    const sal_uInt16 nRealHeight = GetLineHeight();
    const sal_uInt16 nLineHeight = pCurrent->Height();

    KSHORT nPrtWidth = pCurrent->PrtWidth();
    SwLinePortion *pLast = pCurrent->FindLastPortion();

    if( GetInfo().IsMulti() )
        nRealWidth = nReal;
    else
    {
        nRealWidth = GetLineWidth();
        // For each FlyFrm extending into the right margin, we create a FlyPortion.
        const long nLeftMar = GetLeftMargin();
        SwRect aCurrRect( nLeftMar + nPrtWidth, Y() + nRealHeight - nLineHeight,
                          nRealWidth - nPrtWidth, nLineHeight );

        SwFlyPortion *pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        while( pFly && long( nPrtWidth )< nRealWidth )
        {
            pLast->Append( pFly );
            pLast = pFly;
            if( pFly->Fix() > nPrtWidth )
                pFly->Width( ( pFly->Fix() - nPrtWidth) + pFly->Width() + 1);
            nPrtWidth += pFly->Width() + 1;
            aCurrRect.Left( nLeftMar + nPrtWidth );
            pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        }
        delete pFly;
    }

    SwMarginPortion *pRight = new SwMarginPortion( 0 );
    pLast->Append( pRight );

    if( long( nPrtWidth )< nRealWidth )
        pRight->PrtWidth( KSHORT( nRealWidth - nPrtWidth ) );

    // pCurrent->Width() is set to the real size, because we attach the
    // MarginPortions.
    // This trick gives miraculous results:
    // If pCurrent->Width() == nRealWidth, then the adjustment gets overruled
    // implicitly. GetLeftMarginAdjust() and IsJustified() think they have a
    // line filled with chars.

    pCurrent->PrtWidth( KSHORT( nRealWidth ) );
    return pRight;
}

void SwTxtAdjuster::CalcFlyAdjust( SwLineLayout *pCurrent )
{
    // 1) We insert a left margin:
    SwMarginPortion *pLeft = pCurrent->CalcLeftMargin();
    SwGluePortion *pGlue = pLeft; // the last GluePortion

    // 2) We attach a right margin:
    // CalcRightMargin also calculates a possible overlap with FlyFrms.
    CalcRightMargin( pCurrent );

    SwLinePortion *pPos = pLeft->GetPortion();
    sal_Int32 nLen = 0;

    // If we only have one line, the text portion is consecutive and we center, then ...
    bool bComplete = 0 == nStart;
    const bool bTabCompat = GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);
    bool bMultiTab = false;

    while( pPos )
    {
        if ( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasTabulator() )
            bMultiTab = true;
        else if( pPos->InFixMargGrp() &&
               ( bTabCompat ? ! pPos->InTabGrp() : ! bMultiTab ) )
        {
            // in tab compat mode we do not want to change tab portions
            // in non tab compat mode we do not want to change margins if we
            // found a multi portion with tabs
            if( SVX_ADJUST_RIGHT == GetAdjust() )
                ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
            else
            {
                // We set the first text portion to right-aligned and the last one
                // to left-aligned.
                // The first text portion gets the whole Glue, but only if we have
                // more than one line.
                if( bComplete && GetInfo().GetTxt().getLength() == nLen )
                    ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                else
                {
                    if ( ! bTabCompat )
                    {
                        if( pLeft == pGlue )
                        {
                            // If we only have a left and right margin, the
                            // margins share the Glue.
                            if( nLen + pPos->GetLen() >= pCurrent->GetLen() )
                                ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                            else
                                ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
                        }
                        else
                        {
                         // The last text portion retains its Glue.
                         if( !pPos->IsMarginPortion() )
                              ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                         }
                     }
                     else
                        ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
                }
            }

            pGlue = (SwFlyPortion*)pPos;
            bComplete = false;
        }
        nLen = nLen + pPos->GetLen();
        pPos = pPos->GetPortion();
     }

     if( ! bTabCompat && ! bMultiTab && SVX_ADJUST_RIGHT == GetAdjust() )
        // portions are moved to the right if possible
        pLeft->AdjustRight( pCurrent );
}

void SwTxtAdjuster::CalcAdjLine( SwLineLayout *pCurrent )
{
    OSL_ENSURE( pCurrent->IsFormatAdj(), "CalcAdjLine: Why?" );

    pCurrent->SetFormatAdj(false);

    SwParaPortion* pPara = GetInfo().GetParaPortion();

    switch( GetAdjust() )
    {
        case SVX_ADJUST_RIGHT:
        case SVX_ADJUST_CENTER:
        {
            CalcFlyAdjust( pCurrent );
            pPara->GetRepaint()->SetOfst( 0 );
            break;
        }
        case SVX_ADJUST_BLOCK:
        {
            FormatBlock();
            break;
        }
        default : return;
    }
}

// This is a quite complicated calculation: nCurrWidth is the width _before_
// adding the word, that still fits onto the line! For this reason the FlyPortion's
// width is still correct if we get a deadlock-situation of:
// bFirstWord && !WORDFITS
SwFlyPortion *SwTxtAdjuster::CalcFlyPortion( const long nRealWidth,
                                             const SwRect &rCurrRect )
{
    SwTxtFly aTxtFly( GetTxtFrm() );

    const KSHORT nCurrWidth = pCurr->PrtWidth();
    SwFlyPortion *pFlyPortion = 0;

    SwRect aLineVert( rCurrRect );
    if ( GetTxtFrm()->IsRightToLeft() )
        GetTxtFrm()->SwitchLTRtoRTL( aLineVert );
    if ( GetTxtFrm()->IsVertical() )
        GetTxtFrm()->SwitchHorizontalToVertical( aLineVert );

    // aFlyRect is document-global!
    SwRect aFlyRect( aTxtFly.GetFrm( aLineVert ) );

    if ( GetTxtFrm()->IsRightToLeft() )
        GetTxtFrm()->SwitchRTLtoLTR( aFlyRect );
    if ( GetTxtFrm()->IsVertical() )
        GetTxtFrm()->SwitchVerticalToHorizontal( aFlyRect );

    // If a Frame overlapps we open a Portion
    if( aFlyRect.HasArea() )
    {
        // aLocal is frame-local
        SwRect aLocal( aFlyRect );
        aLocal.Pos( aLocal.Left() - GetLeftMargin(), aLocal.Top() );
        if( nCurrWidth > aLocal.Left() )
            aLocal.Left( nCurrWidth );

        // If the rect is wider than the line, we adjust it to the right size
        KSHORT nLocalWidth = KSHORT( aLocal.Left() + aLocal.Width() );
        if( nRealWidth < long( nLocalWidth ) )
            aLocal.Width( nRealWidth - aLocal.Left() );
        GetInfo().GetParaPortion()->SetFly( true );
        pFlyPortion = new SwFlyPortion( aLocal );
        pFlyPortion->Height( KSHORT( rCurrRect.Height() ) );
        // The Width could be smaller than the FixWidth, thus:
        pFlyPortion->AdjFixWidth();
    }
    return pFlyPortion;
}

// CalcDropAdjust is called at the end by Format() if needed
void SwTxtAdjuster::CalcDropAdjust()
{
    OSL_ENSURE( 1<GetDropLines() && SVX_ADJUST_LEFT!=GetAdjust() && SVX_ADJUST_BLOCK!=GetAdjust(),
            "CalcDropAdjust: No reason for DropAdjustment." );

    const MSHORT nLineNumber = GetLineNr();

    // 1) Skip dummies
    Top();

    if( !pCurr->IsDummy() || NextLine() )
    {
        // Adjust first
        GetAdjusted();

        SwLinePortion *pPor = pCurr->GetFirstPortion();

        // 2) Make sure we include the ropPortion
        // 3) pLeft is the GluePor preceding the DropPor
        if( pPor->InGlueGrp() && pPor->GetPortion()
              && pPor->GetPortion()->IsDropPortion() )
        {
            const SwLinePortion *pDropPor = (SwDropPortion*) pPor->GetPortion();
            SwGluePortion *pLeft = (SwGluePortion*) pPor;

            // 4) pRight: Find the GluePor coming after the DropPor
            pPor = pPor->GetPortion();
            while( pPor && !pPor->InFixMargGrp() )
                pPor = pPor->GetPortion();

            SwGluePortion *pRight = ( pPor && pPor->InGlueGrp() ) ?
                                    (SwGluePortion*) pPor : 0;
            if( pRight && pRight != pLeft )
            {
                // 5) Calculate nMinLeft. Who is the most to left?
                const KSHORT nDropLineStart =
                    KSHORT(GetLineStart()) + pLeft->Width() + pDropPor->Width();
                KSHORT nMinLeft = nDropLineStart;
                for( MSHORT i = 1; i < GetDropLines(); ++i )
                {
                    if( NextLine() )
                    {
                        // Adjust first
                        GetAdjusted();

                        pPor = pCurr->GetFirstPortion();
                        const SwMarginPortion *pMar = pPor->IsMarginPortion() ?
                                                      (SwMarginPortion*)pPor : 0;
                        if( !pMar )
                            nMinLeft = 0;
                        else
                        {
                            const KSHORT nLineStart =
                                KSHORT(GetLineStart()) + pMar->Width();
                            if( nMinLeft > nLineStart )
                                nMinLeft = nLineStart;
                        }
                    }
                }

                // 6) Distribute the Glue anew between pLeft and pRight
                if( nMinLeft < nDropLineStart )
                {
                    // The Glue is always passed from pLeft to pRight, so that
                    // the text moves to the left.
                    const short nGlue = nDropLineStart - nMinLeft;
                    if( !nMinLeft )
                        pLeft->MoveAllGlue( pRight );
                    else
                        pLeft->MoveGlue( pRight, nGlue );
                }
            }
        }
    }

    if( nLineNumber != GetLineNr() )
    {
        Top();
        while( nLineNumber != GetLineNr() && Next() )
            ;
    }
}

void SwTxtAdjuster::CalcDropRepaint()
{
    Top();
    SwRepaint &rRepaint = *GetInfo().GetParaPortion()->GetRepaint();
    if( rRepaint.Top() > Y() )
        rRepaint.Top( Y() );
    for( MSHORT i = 1; i < GetDropLines(); ++i )
        NextLine();
    const SwTwips nBottom = Y() + GetLineHeight() - 1;
    if( rRepaint.Bottom() < nBottom )
        rRepaint.Bottom( nBottom );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
