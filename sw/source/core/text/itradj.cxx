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

#include <vcl/outdev.hxx>
#include <IDocumentSettingAccess.hxx>
#include <doc.hxx>

#include <frame.hxx>
#include <paratr.hxx>
#include "itrtxt.hxx"
#include "porglue.hxx"
#include "porlay.hxx"
#include "porfly.hxx"
#include "pordrop.hxx"
#include "pormulti.hxx"
#include "portab.hxx"
#include <memory>

#define MIN_TAB_WIDTH 60

using namespace ::com::sun::star;

void SwTextAdjuster::FormatBlock( )
{
    // Block format does not apply to the last line.
    // And for tabs it doesn't exist out of tradition
    // If we have Flys we continue.

    const SwLinePortion *pFly = nullptr;

    bool bSkip = !IsLastBlock() &&
        m_nStart + m_pCurr->GetLen() >= TextFrameIndex(GetInfo().GetText().getLength());

    // Multi-line fields are tricky, because we need to check whether there are
    // any other text portions in the paragraph.
    if( bSkip )
    {
        const SwLineLayout *pLay = m_pCurr->GetNext();
        while( pLay && !pLay->GetLen() )
        {
            const SwLinePortion *pPor = m_pCurr->GetFirstPortion();
            while( pPor && bSkip )
            {
                if( pPor->InTextGrp() )
                    bSkip = false;
                pPor = pPor->GetNextPortion();
            }
            pLay = bSkip ? pLay->GetNext() : nullptr;
        }
    }

    if( bSkip )
    {
        if( !GetInfo().GetParaPortion()->HasFly() )
        {
            if( IsLastCenter() )
                CalcFlyAdjust( m_pCurr );
            m_pCurr->FinishSpaceAdd();
            return;
        }
        else
        {
            const SwLinePortion *pTmpFly = nullptr;

            // End at the last Fly
            const SwLinePortion *pPos = m_pCurr->GetFirstPortion();
            while( pPos )
            {
                // Look for the last Fly which has text coming after it:
                if( pPos->IsFlyPortion() )
                    pTmpFly = pPos; // Found a Fly
                else if ( pTmpFly && pPos->InTextGrp() )
                {
                    pFly = pTmpFly; // A Fly with follow-up text!
                    pTmpFly = nullptr;
                }
                pPos = pPos->GetNextPortion();
            }
            // End if we didn't find one
            if( !pFly )
            {
                if( IsLastCenter() )
                    CalcFlyAdjust( m_pCurr );
                m_pCurr->FinishSpaceAdd();
                return;
            }
        }
    }

    const TextFrameIndex nOldIdx = GetInfo().GetIdx();
    GetInfo().SetIdx( m_nStart );
    CalcNewBlock( m_pCurr, pFly );
    GetInfo().SetIdx( nOldIdx );
    GetInfo().GetParaPortion()->GetRepaint().SetOfst(0);
}

static bool lcl_CheckKashidaPositions( SwScriptInfo& rSI, SwTextSizeInfo& rInf, SwTextIter& rItr,
            sal_Int32& rKashidas, TextFrameIndex& nGluePortion)
{
    if ( rInf.GetOut()->GetMinKashida() <= 0 )
        return false;

    // i60594 validate Kashida justification
    TextFrameIndex nIdx = rItr.GetStart();
    TextFrameIndex nEnd = rItr.GetEnd();

    // Note on calling KashidaJustify():
    // Kashida positions may be marked as invalid. Therefore KashidaJustify may return the clean
    // total number of kashida positions, or the number of kashida positions after some positions
    // have been dropped.
    // Here we want the clean total, which is OK: We have called ClearKashidaInvalid() before.
    rKashidas = rSI.KashidaJustify ( nullptr, nullptr, rItr.GetStart(), rItr.GetLength() );

    if (rKashidas <= 0) // nothing to do
        return true;

    // kashida positions found in SwScriptInfo are not necessarily valid in every font
    // if two characters are replaced by a ligature glyph, there will be no place for a kashida
    std::vector<TextFrameIndex> aKashidaPos;
    rSI.GetKashidaPositions(nIdx, rItr.GetLength(), aKashidaPos);
    assert(aKashidaPos.size() >= static_cast<size_t>(rKashidas));
    std::vector<TextFrameIndex> aKashidaPosDropped(aKashidaPos.size());
    sal_Int32 nKashidaIdx = 0;
    while ( rKashidas && nIdx < nEnd )
    {
        rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
        TextFrameIndex nNext = rItr.GetNextAttr();

        // is there also a script change before?
        // if there is, nNext should point to the script change
        TextFrameIndex const nNextScript = rSI.NextScriptChg( nIdx );
        if( nNextScript < nNext )
            nNext = nNextScript;

        if (nNext == TextFrameIndex(COMPLETE_STRING) || nNext > nEnd)
            nNext = nEnd;
        sal_Int32 nKashidasInAttr = rSI.KashidaJustify ( nullptr, nullptr, nIdx, nNext - nIdx );
        if (nKashidasInAttr > 0)
        {
            sal_Int32 nKashidasDropped = 0;
            if ( !SwScriptInfo::IsArabicText( rInf.GetText(), nIdx, nNext - nIdx ) )
            {
                nKashidasDropped = nKashidasInAttr;
                rKashidas -= nKashidasDropped;
            }
            else
            {
                ComplexTextLayoutFlags nOldLayout = rInf.GetOut()->GetLayoutMode();
                rInf.GetOut()->SetLayoutMode ( nOldLayout | ComplexTextLayoutFlags::BiDiRtl );
                nKashidasDropped = rInf.GetOut()->ValidateKashidas(
                    rInf.GetText(), sal_Int32(nIdx), sal_Int32(nNext - nIdx),
                    nKashidasInAttr,
                    reinterpret_cast<sal_Int32*>(aKashidaPos.data() + nKashidaIdx),
                    reinterpret_cast<sal_Int32*>(aKashidaPosDropped.data()));
                rInf.GetOut()->SetLayoutMode ( nOldLayout );
                if ( nKashidasDropped )
                {
                    rSI.MarkKashidasInvalid(nKashidasDropped, aKashidaPosDropped.data());
                    rKashidas -= nKashidasDropped;
                    nGluePortion -= TextFrameIndex(nKashidasDropped);
                }
            }
            nKashidaIdx += nKashidasInAttr;
        }
        nIdx = nNext;
    }

    // return false if all kashidas have been eliminated
    return (rKashidas > 0);
}

static bool lcl_CheckKashidaWidth ( SwScriptInfo& rSI, SwTextSizeInfo& rInf, SwTextIter& rItr, sal_Int32& rKashidas,
                             TextFrameIndex& nGluePortion, const long nGluePortionWidth, long& nSpaceAdd )
{
    // check kashida width
    // if width is smaller than minimal kashida width allowed by fonts in the current line
    // drop one kashida after the other until kashida width is OK
    while (rKashidas)
    {
        bool bAddSpaceChanged = false;
        TextFrameIndex nIdx = rItr.GetStart();
        TextFrameIndex nEnd = rItr.GetEnd();
        while ( nIdx < nEnd )
        {
            rItr.SeekAndChgAttrIter( nIdx, rInf.GetOut() );
            TextFrameIndex nNext = rItr.GetNextAttr();

            // is there also a script change before?
            // if there is, nNext should point to the script change
            TextFrameIndex const nNextScript = rSI.NextScriptChg( nIdx );
            if( nNextScript < nNext )
               nNext = nNextScript;

            if (nNext == TextFrameIndex(COMPLETE_STRING) || nNext > nEnd)
                nNext = nEnd;
            sal_Int32 nKashidasInAttr = rSI.KashidaJustify ( nullptr, nullptr, nIdx, nNext - nIdx );

            long nFontMinKashida = rInf.GetOut()->GetMinKashida();
            if ( nKashidasInAttr > 0 && SwScriptInfo::IsArabicText( rInf.GetText(), nIdx, nNext - nIdx ) )
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

                    nSpaceAdd = nGluePortionWidth / sal_Int32(nGluePortion);
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
void SwTextAdjuster::CalcNewBlock( SwLineLayout *pCurrent,
                                  const SwLinePortion *pStopAt, SwTwips nReal, bool bSkipKashida )
{
    OSL_ENSURE( GetInfo().IsMulti() || SvxAdjust::Block == GetAdjust(),
            "CalcNewBlock: Why?" );
    OSL_ENSURE( pCurrent->Height(), "SwTextAdjuster::CalcBlockAdjust: missing CalcLine()" );

    pCurrent->InitSpaceAdd();
    TextFrameIndex nGluePortion(0);
    TextFrameIndex nCharCnt(0);
    sal_uInt16 nSpaceIdx = 0;

    // i60591: hennerdrews
    SwScriptInfo& rSI = GetInfo().GetParaPortion()->GetScriptInfo();
    SwTextSizeInfo aInf ( GetTextFrame() );
    SwTextIter aItr ( GetTextFrame(), &aInf );

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
        GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK);

    SwLinePortion *pPos = pCurrent->GetNextPortion();

    while( pPos )
    {
        if ( bDoNotJustifyLinesWithManualBreak &&
             pPos->IsBreakPortion() && !IsLastBlock() )
        {
           pCurrent->FinishSpaceAdd();
           break;
        }

        if ( pPos->InTextGrp() )
            nGluePortion = nGluePortion + static_cast<SwTextPortion*>(pPos)->GetSpaceCnt( GetInfo(), nCharCnt );
        else if( pPos->IsMultiPortion() )
        {
            SwMultiPortion* pMulti = static_cast<SwMultiPortion*>(pPos);
            // a multiportion with a tabulator inside breaks the text adjustment
            // a ruby portion will not be stretched by text adjustment
            // a double line portion takes additional space for each blank
            // in the wider line
            if( pMulti->HasTabulator() )
            {
                if ( nSpaceIdx == pCurrent->GetLLSpaceAddCount() )
                    pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );

                nSpaceIdx++;
                nGluePortion = TextFrameIndex(0);
                nCharCnt = TextFrameIndex(0);
            }
            else if( pMulti->IsDouble() )
                nGluePortion = nGluePortion + static_cast<SwDoubleLinePortion*>(pMulti)->GetSpaceCnt();
            else if ( pMulti->IsBidi() )
                nGluePortion = nGluePortion + static_cast<SwBidiPortion*>(pMulti)->GetSpaceCnt( GetInfo() );  // i60594
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
                        GetInfo().SetIdx( m_nStart );
                        CalcNewBlock( pCurrent, pStopAt, nReal, true );
                        return;
                    }
                }

                if( nGluePortion )
                {
                    long nSpaceAdd = nGluePortionWidth / sal_Int32(nGluePortion);

                    // i60594
                    if( rSI.CountKashida() && !bSkipKashida )
                    {
                        if( !lcl_CheckKashidaWidth( rSI, aInf, aItr, nKashidas, nGluePortion, nGluePortionWidth, nSpaceAdd ))
                        {
                            // no kashidas left
                            // do regular blank justification
                            pCurrent->FinishSpaceAdd();
                            GetInfo().SetIdx( m_nStart );
                            CalcNewBlock( pCurrent, pStopAt, nReal, true );
                            return;
                        }
                    }

                    pCurrent->SetLLSpaceAdd( nSpaceAdd , nSpaceIdx );
                    pPos->Width( static_cast<SwGluePortion*>(pPos)->GetFixWidth() );
                }
                else if (IsOneBlock() && nCharCnt > TextFrameIndex(1))
                {
                    const long nSpaceAdd = - nGluePortionWidth / (sal_Int32(nCharCnt) - 1);
                    pCurrent->SetLLSpaceAdd( nSpaceAdd, nSpaceIdx );
                    pPos->Width( static_cast<SwGluePortion*>(pPos)->GetFixWidth() );
                }

                nSpaceIdx++;
                nGluePortion = TextFrameIndex(0);
                nCharCnt = TextFrameIndex(0);
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
        pPos = pPos->GetNextPortion();
    }
}

SwTwips SwTextAdjuster::CalcKanaAdj( SwLineLayout* pCurrent )
{
    OSL_ENSURE( pCurrent->Height(), "SwTextAdjuster::CalcBlockAdjust: missing CalcLine()" );
    OSL_ENSURE( !pCurrent->GetpKanaComp(), "pKanaComp already exists!!" );

    pCurrent->SetKanaComp( std::make_unique<std::deque<sal_uInt16>>() );

    const sal_uInt16 nNull = 0;
    size_t nKanaIdx = 0;
    long nKanaDiffSum = 0;
    SwTwips nRepaintOfst = 0;
    SwTwips nX = 0;
    bool bNoCompression = false;

    // Do not forget: CalcRightMargin() sets pCurrent->Width() to the line width!
    CalcRightMargin( pCurrent );

    SwLinePortion* pPos = pCurrent->GetNextPortion();

    while( pPos )
    {
        if ( pPos->InTextGrp() )
        {
            // get maximum portion width from info structure, calculated
            // during text formatting
            sal_uInt16 nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pPos );

            // check, if information is stored under other key
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pCurrent );

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

            long nRest;

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
                        static_cast<SwGluePortion*>(pPos)->GetPrtGlue() :
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

                ( pCurrent->GetKanaComp() )[ nKanaIdx ] = static_cast<sal_uInt16>(nCompress);
                nKanaDiffSum = 0;
            }

            nKanaIdx++;
        }

        nX += pPos->Width();
        pPos = pPos->GetNextPortion();
    }

    // set portion width
    nKanaIdx = 0;
    sal_uInt16 nCompress = ( pCurrent->GetKanaComp() )[ nKanaIdx ];
    pPos = pCurrent->GetNextPortion();
    long nDecompress = 0;

    while( pPos )
    {
        if ( pPos->InTextGrp() )
        {
            const sal_uInt16 nMinWidth = pPos->Width();

            // get maximum portion width from info structure, calculated
            // during text formatting
            sal_uInt16 nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pPos );

            // check, if information is stored under other key
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pCurrent );
            pPos->Width( nMinWidth +
                       ( ( 10000 - nCompress ) * nMaxWidthDiff ) / 10000 );
            nDecompress += pPos->Width() - nMinWidth;
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            pPos->Width( static_cast<sal_uInt16>(pPos->Width() - nDecompress) );

            if ( pPos->InTabGrp() )
                // set fix width to width
                static_cast<SwTabPortion*>(pPos)->SetFixWidth( pPos->Width() );

            if ( ++nKanaIdx < pCurrent->GetKanaComp().size() )
                nCompress = ( pCurrent->GetKanaComp() )[ nKanaIdx ];

            nDecompress = 0;
        }
        pPos = pPos->GetNextPortion();
    }

    return nRepaintOfst;
}

SwMarginPortion *SwTextAdjuster::CalcRightMargin( SwLineLayout *pCurrent,
    SwTwips nReal )
{
    long nRealWidth;
    const sal_uInt16 nRealHeight = GetLineHeight();
    const sal_uInt16 nLineHeight = pCurrent->Height();

    sal_uInt16 nPrtWidth = pCurrent->PrtWidth();
    SwLinePortion *pLast = pCurrent->FindLastPortion();

    if( GetInfo().IsMulti() )
        nRealWidth = nReal;
    else
    {
        nRealWidth = GetLineWidth();
        // For each FlyFrame extending into the right margin, we create a FlyPortion.
        const long nLeftMar = GetLeftMargin();
        SwRect aCurrRect( nLeftMar + nPrtWidth, Y() + nRealHeight - nLineHeight,
                          nRealWidth - nPrtWidth, nLineHeight );

        SwFlyPortion *pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        while( pFly && long( nPrtWidth )< nRealWidth )
        {
            pLast->Append( pFly );
            pLast = pFly;
            if( pFly->GetFix() > nPrtWidth )
                pFly->Width( ( pFly->GetFix() - nPrtWidth) + pFly->Width() + 1);
            nPrtWidth += pFly->Width() + 1;
            aCurrRect.Left( nLeftMar + nPrtWidth );
            pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        }
        delete pFly;
    }

    SwMarginPortion *pRight = new SwMarginPortion;
    pLast->Append( pRight );

    if( long( nPrtWidth )< nRealWidth )
        pRight->PrtWidth( sal_uInt16( nRealWidth - nPrtWidth ) );

    // pCurrent->Width() is set to the real size, because we attach the
    // MarginPortions.
    // This trick gives miraculous results:
    // If pCurrent->Width() == nRealWidth, then the adjustment gets overruled
    // implicitly. GetLeftMarginAdjust() and IsJustified() think they have a
    // line filled with chars.

    pCurrent->PrtWidth( sal_uInt16( nRealWidth ) );
    return pRight;
}

void SwTextAdjuster::CalcFlyAdjust( SwLineLayout *pCurrent )
{
    // 1) We insert a left margin:
    SwMarginPortion *pLeft = pCurrent->CalcLeftMargin();
    SwGluePortion *pGlue = pLeft; // the last GluePortion

    // 2) We attach a right margin:
    // CalcRightMargin also calculates a possible overlap with FlyFrames.
    CalcRightMargin( pCurrent );

    SwLinePortion *pPos = pLeft->GetNextPortion();
    TextFrameIndex nLen(0);

    // If we only have one line, the text portion is consecutive and we center, then ...
    bool bComplete = TextFrameIndex(0) == m_nStart;
    const bool bTabCompat = GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::TAB_COMPAT);
    bool bMultiTab = false;

    while( pPos )
    {
        if ( pPos->IsMultiPortion() && static_cast<SwMultiPortion*>(pPos)->HasTabulator() )
            bMultiTab = true;
        else if( pPos->InFixMargGrp() &&
               ( bTabCompat ? ! pPos->InTabGrp() : ! bMultiTab ) )
        {
            // in tab compat mode we do not want to change tab portions
            // in non tab compat mode we do not want to change margins if we
            // found a multi portion with tabs
            if( SvxAdjust::Right == GetAdjust() )
                static_cast<SwGluePortion*>(pPos)->MoveAllGlue( pGlue );
            else
            {
                // We set the first text portion to right-aligned and the last one
                // to left-aligned.
                // The first text portion gets the whole Glue, but only if we have
                // more than one line.
                if (bComplete && TextFrameIndex(GetInfo().GetText().getLength()) == nLen)
                    static_cast<SwGluePortion*>(pPos)->MoveHalfGlue( pGlue );
                else
                {
                    if ( ! bTabCompat )
                    {
                        if( pLeft == pGlue )
                        {
                            // If we only have a left and right margin, the
                            // margins share the Glue.
                            if( nLen + pPos->GetLen() >= pCurrent->GetLen() )
                                static_cast<SwGluePortion*>(pPos)->MoveHalfGlue( pGlue );
                            else
                                static_cast<SwGluePortion*>(pPos)->MoveAllGlue( pGlue );
                        }
                        else
                        {
                         // The last text portion retains its Glue.
                         if( !pPos->IsMarginPortion() )
                              static_cast<SwGluePortion*>(pPos)->MoveHalfGlue( pGlue );
                         }
                    }
                    else
                        static_cast<SwGluePortion*>(pPos)->MoveHalfGlue( pGlue );
                }
            }

            pGlue = static_cast<SwGluePortion*>(pPos);
            bComplete = false;
        }
        nLen = nLen + pPos->GetLen();
        pPos = pPos->GetNextPortion();
    }

    if( ! bTabCompat && ! bMultiTab && SvxAdjust::Right == GetAdjust() )
        // portions are moved to the right if possible
        pLeft->AdjustRight( pCurrent );
}

void SwTextAdjuster::CalcAdjLine( SwLineLayout *pCurrent )
{
    OSL_ENSURE( pCurrent->IsFormatAdj(), "CalcAdjLine: Why?" );

    pCurrent->SetFormatAdj(false);

    SwParaPortion* pPara = GetInfo().GetParaPortion();

    switch( GetAdjust() )
    {
        case SvxAdjust::Right:
        case SvxAdjust::Center:
        {
            CalcFlyAdjust( pCurrent );
            pPara->GetRepaint().SetOfst( 0 );
            break;
        }
        case SvxAdjust::Block:
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
SwFlyPortion *SwTextAdjuster::CalcFlyPortion( const long nRealWidth,
                                             const SwRect &rCurrRect )
{
    SwTextFly aTextFly( GetTextFrame() );

    const sal_uInt16 nCurrWidth = m_pCurr->PrtWidth();
    SwFlyPortion *pFlyPortion = nullptr;

    SwRect aLineVert( rCurrRect );
    if ( GetTextFrame()->IsRightToLeft() )
        GetTextFrame()->SwitchLTRtoRTL( aLineVert );
    if ( GetTextFrame()->IsVertical() )
        GetTextFrame()->SwitchHorizontalToVertical( aLineVert );

    // aFlyRect is document-global!
    SwRect aFlyRect( aTextFly.GetFrame( aLineVert ) );

    if ( GetTextFrame()->IsRightToLeft() )
        GetTextFrame()->SwitchRTLtoLTR( aFlyRect );
    if ( GetTextFrame()->IsVertical() )
        GetTextFrame()->SwitchVerticalToHorizontal( aFlyRect );

    // If a Frame overlapps we open a Portion
    if( aFlyRect.HasArea() )
    {
        // aLocal is frame-local
        SwRect aLocal( aFlyRect );
        aLocal.Pos( aLocal.Left() - GetLeftMargin(), aLocal.Top() );
        if( nCurrWidth > aLocal.Left() )
            aLocal.Left( nCurrWidth );

        // If the rect is wider than the line, we adjust it to the right size
        const long nLocalWidth = aLocal.Left() + aLocal.Width();
        if( nRealWidth < nLocalWidth )
            aLocal.Width( nRealWidth - aLocal.Left() );
        GetInfo().GetParaPortion()->SetFly();
        pFlyPortion = new SwFlyPortion( aLocal );
        pFlyPortion->Height( sal_uInt16( rCurrRect.Height() ) );
        // The Width could be smaller than the FixWidth, thus:
        pFlyPortion->AdjFixWidth();
    }
    return pFlyPortion;
}

// CalcDropAdjust is called at the end by Format() if needed
void SwTextAdjuster::CalcDropAdjust()
{
    OSL_ENSURE( 1<GetDropLines() && SvxAdjust::Left!=GetAdjust() && SvxAdjust::Block!=GetAdjust(),
            "CalcDropAdjust: No reason for DropAdjustment." );

    const sal_uInt16 nLineNumber = GetLineNr();

    // 1) Skip dummies
    Top();

    if( !m_pCurr->IsDummy() || NextLine() )
    {
        // Adjust first
        GetAdjusted();

        SwLinePortion *pPor = m_pCurr->GetFirstPortion();

        // 2) Make sure we include the ropPortion
        // 3) pLeft is the GluePor preceding the DropPor
        if( pPor->InGlueGrp() && pPor->GetNextPortion()
              && pPor->GetNextPortion()->IsDropPortion() )
        {
            const SwLinePortion *pDropPor = static_cast<SwDropPortion*>( pPor->GetNextPortion() );
            SwGluePortion *pLeft = static_cast<SwGluePortion*>( pPor );

            // 4) pRight: Find the GluePor coming after the DropPor
            pPor = pPor->GetNextPortion();
            while( pPor && !pPor->InFixMargGrp() )
                pPor = pPor->GetNextPortion();

            SwGluePortion *pRight = ( pPor && pPor->InGlueGrp() ) ?
                                    static_cast<SwGluePortion*>(pPor) : nullptr;
            if( pRight && pRight != pLeft )
            {
                // 5) Calculate nMinLeft. Who is the most to left?
                const auto nDropLineStart =
                    GetLineStart() + pLeft->Width() + pDropPor->Width();
                auto nMinLeft = nDropLineStart;
                for( sal_uInt16 i = 1; i < GetDropLines(); ++i )
                {
                    if( NextLine() )
                    {
                        // Adjust first
                        GetAdjusted();

                        pPor = m_pCurr->GetFirstPortion();
                        const SwMarginPortion *pMar = pPor->IsMarginPortion() ?
                                                      static_cast<SwMarginPortion*>(pPor) : nullptr;
                        if( !pMar )
                            nMinLeft = 0;
                        else
                        {
                            const auto nLineStart =
                                GetLineStart() + pMar->Width();
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
                    const auto nGlue = nDropLineStart - nMinLeft;
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

void SwTextAdjuster::CalcDropRepaint()
{
    Top();
    SwRepaint &rRepaint = GetInfo().GetParaPortion()->GetRepaint();
    if( rRepaint.Top() > Y() )
        rRepaint.Top( Y() );
    for( sal_uInt16 i = 1; i < GetDropLines(); ++i )
        NextLine();
    const SwTwips nBottom = Y() + GetLineHeight() - 1;
    if( rRepaint.Bottom() < nBottom )
        rRepaint.Bottom( nBottom );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
