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

#include <com/sun/star/i18n/ScriptType.hpp>
#include <vcl/outdev.hxx>
#include <IDocumentSettingAccess.hxx>

#include "frame.hxx"       // CalcFlyAdjust()
#include "paratr.hxx"
#include "itrtxt.hxx"
#include "porglue.hxx"
#include "porlay.hxx"
#include "porfly.hxx"       // CalcFlyAdjust()
#include "pordrop.hxx"       // CalcFlyAdjust()
#include "pormulti.hxx"
#include <portab.hxx>

#define MIN_TAB_WIDTH 60

using namespace ::com::sun::star;

/*************************************************************************
 * SwTxtAdjuster::FormatBlock()
 *************************************************************************/

void SwTxtAdjuster::FormatBlock( )
{
    // Block format does not apply to the last line.
    // And for tabs it doesn't exist out of tradition
    // If we have Flys we continue.

    const SwLinePortion *pFly = 0;

    sal_Bool bSkip = !IsLastBlock() &&
        nStart + pCurr->GetLen() >= GetInfo().GetTxt().Len();

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
                    bSkip = sal_False;
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

    const xub_StrLen nOldIdx = GetInfo().GetIdx();
    GetInfo().SetIdx( nStart );
    CalcNewBlock( pCurr, pFly );
    GetInfo().SetIdx( nOldIdx );
    GetInfo().GetParaPortion()->GetRepaint()->SetOfst(0);
}

/*************************************************************************
 * lcl_CheckKashidaPositions()
 *************************************************************************/
static bool lcl_CheckKashidaPositions( SwScriptInfo& rSI, SwTxtSizeInfo& rInf, SwTxtIter& rItr,
                                xub_StrLen& nKashidas, xub_StrLen& nGluePortion )
{
    // i60594 validate Kashida justification
    xub_StrLen nIdx = rItr.GetStart();
    xub_StrLen nEnd = rItr.GetEnd();

    // Note on calling KashidaJustify():
    // Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
    // total number of kashida positions, or the number of kashida positions after some positions
    // have been dropped.
    // Here we want the clean total, which is OK: We have called ClearKashidaInvalid() before.
    nKashidas = rSI.KashidaJustify ( 0, 0, rItr.GetStart(), rItr.GetLength(), 0 );

    if (!nKashidas) // nothing to do
        return true;

    // kashida positions found in SwScriptInfo are not necessarily valid in every font
    // if two characters are replaced by a ligature glyph, there will be no place for a kashida
    xub_StrLen* pKashidaPos = new xub_StrLen [ nKashidas ];
    xub_StrLen* pKashidaPosDropped = new xub_StrLen [ nKashidas ];
    rSI.GetKashidaPositions ( nIdx, rItr.GetLength(), pKashidaPos );
    xub_StrLen nKashidaIdx = 0;
    while ( nKashidas && nIdx < nEnd )
    {
        rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
        xub_StrLen nNext = rItr.GetNextAttr();

        // is there also a script change before?
        // if there is, nNext should point to the script change
        xub_StrLen nNextScript = rSI.NextScriptChg( nIdx );
        if( nNextScript < nNext )
            nNext = nNextScript;

        if ( nNext == STRING_LEN || nNext > nEnd )
            nNext = nEnd;
        xub_StrLen nKashidasInAttr = rSI.KashidaJustify ( 0, 0, nIdx, nNext - nIdx );
        if ( nKashidasInAttr )
        {
            // Kashida glyph looks suspicious, skip Kashida justification
            if ( rInf.GetOut()->GetMinKashida() <= 0 )
                return false;

            xub_StrLen nKashidasDropped = 0;
            if ( !SwScriptInfo::IsArabicText( rInf.GetTxt(), nIdx, nNext - nIdx ) )
            {
                nKashidasDropped = nKashidasInAttr;
                nKashidas -= nKashidasDropped;
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
                    rSI.MarkKashidasInvalid ( nKashidasDropped, pKashidaPosDropped );
                    nKashidas -= nKashidasDropped;
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
    return (nKashidas > 0);
}

/*************************************************************************
 * lcl_CheckKashidaWidth()
 *************************************************************************/
static bool lcl_CheckKashidaWidth ( SwScriptInfo& rSI, SwTxtSizeInfo& rInf, SwTxtIter& rItr, xub_StrLen& nKashidas,
                             xub_StrLen& nGluePortion, const long nGluePortionWidth, long& nSpaceAdd )
{
    // check kashida width
    // if width is smaller than minimal kashida width allowed by fonts in the current line
    // drop one kashida after the other until kashida width is OK
    bool bAddSpaceChanged;
    while ( nKashidas )
    {
        bAddSpaceChanged = false;
        xub_StrLen nIdx = rItr.GetStart();
        xub_StrLen nEnd = rItr.GetEnd();
        while ( nIdx < nEnd )
        {
            rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
            xub_StrLen nNext = rItr.GetNextAttr();

            // is there also a script change before?
            // if there is, nNext should point to the script change
            xub_StrLen nNextScript = rSI.NextScriptChg( nIdx );
            if( nNextScript < nNext )
               nNext = nNextScript;

            if ( nNext == STRING_LEN || nNext > nEnd )
                nNext = nEnd;
            xub_StrLen nKashidasInAttr = rSI.KashidaJustify ( 0, 0, nIdx, nNext - nIdx );

            long nFontMinKashida = rInf.GetOut()->GetMinKashida();
            if ( nFontMinKashida && nKashidasInAttr && SwScriptInfo::IsArabicText( rInf.GetTxt(), nIdx, nNext - nIdx ) )
            {
                xub_StrLen nKashidasDropped = 0;
                while ( nKashidas && nGluePortion && nKashidasInAttr &&
                        nSpaceAdd / SPACING_PRECISION_FACTOR < nFontMinKashida )
                {
                    --nGluePortion;
                    --nKashidas;
                    --nKashidasInAttr;
                    ++nKashidasDropped;
                    if( !nKashidas || !nGluePortion ) // nothing left, return false to
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

/*************************************************************************
 * SwTxtAdjuster::CalcNewBlock()
 *
 * CalcNewBlock() must only be called _after_ CalcLine()!
 * We always span between two RandPortions or FixPortions (Tabs and Flys).
 * We count the Glues and call ExpandBlock.
 *************************************************************************/

void SwTxtAdjuster::CalcNewBlock( SwLineLayout *pCurrent,
                                  const SwLinePortion *pStopAt, SwTwips nReal, bool bSkipKashida )
{
    OSL_ENSURE( GetInfo().IsMulti() || SVX_ADJUST_BLOCK == GetAdjust(),
            "CalcNewBlock: Why?" );
    OSL_ENSURE( pCurrent->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );

    pCurrent->InitSpaceAdd();
    xub_StrLen nGluePortion = 0;
    xub_StrLen nCharCnt = 0;
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
    const sal_Bool bDoNotJustifyLinesWithManualBreak =
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

                xub_StrLen nKashidas = 0;
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

/*************************************************************************
 * SwTxtAdjuster::CalcKanaAdj()
 *************************************************************************/

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
    sal_Bool bNoCompression = sal_False;

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

                bNoCompression = sal_False;
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

/*************************************************************************
 * SwTxtAdjuster::CalcRightMargin()
 *************************************************************************/

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

/*************************************************************************
 * SwTxtAdjuster::CalcFlyAdjust()
 *************************************************************************/

void SwTxtAdjuster::CalcFlyAdjust( SwLineLayout *pCurrent )
{
    // 1) We insert a left margin:
    SwMarginPortion *pLeft = pCurrent->CalcLeftMargin();
    SwGluePortion *pGlue = pLeft; // the last GluePortion


    // 2) We attach a right margin:
    // CalcRightMargin also calculates a possible overlap with FlyFrms.
    CalcRightMargin( pCurrent );

    SwLinePortion *pPos = pLeft->GetPortion();
    xub_StrLen nLen = 0;

    // If we only have one line, the text portion is consecutive and we center, then ...
    sal_Bool bComplete = 0 == nStart;
    const sal_Bool bTabCompat = GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->get(IDocumentSettingAccess::TAB_COMPAT);
    sal_Bool bMultiTab = sal_False;

    while( pPos )
    {
        if ( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasTabulator() )
            bMultiTab = sal_True;
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
                if( bComplete && GetInfo().GetTxt().Len() == nLen )
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
            bComplete = sal_False;
        }
        nLen = nLen + pPos->GetLen();
        pPos = pPos->GetPortion();
     }

     if( ! bTabCompat && ! bMultiTab && SVX_ADJUST_RIGHT == GetAdjust() )
        // portions are moved to the right if possible
        pLeft->AdjustRight( pCurrent );
}

/*************************************************************************
 * SwTxtAdjuster::CalcAdjLine()
 *************************************************************************/

void SwTxtAdjuster::CalcAdjLine( SwLineLayout *pCurrent )
{
    OSL_ENSURE( pCurrent->IsFormatAdj(), "CalcAdjLine: Why?" );

    pCurrent->SetFormatAdj(sal_False);

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

/*************************************************************************
 * SwTxtAdjuster::CalcFlyPortion()
 *
 * This is a quite complicated calculation: nCurrWidth is the width _before_
 * adding the word, that still fits onto the line! For this reason the FlyPortion's
 * width is still correct if we get a deadlock-situation of:
 * bFirstWord && !WORDFITS
 *************************************************************************/

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
        GetInfo().GetParaPortion()->SetFly( sal_True );
        pFlyPortion = new SwFlyPortion( aLocal );
        pFlyPortion->Height( KSHORT( rCurrRect.Height() ) );
        // The Width could be smaller than the FixWidth, thus:
        pFlyPortion->AdjFixWidth();
    }
    return pFlyPortion;
}

/*************************************************************************
 * SwTxtPainter::_CalcDropAdjust()
 * Drops and Adjustment
 * CalcDropAdjust is called at the end by Format() if needed
 *************************************************************************/

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

/*************************************************************************
 * SwTxtAdjuster::CalcDropRepaint()
 *************************************************************************/

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
