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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <swscanner.hxx>
#include <i18nutil/kashida.hxx>

#include <IDocumentSettingAccess.hxx>
#include <doc.hxx>

#include "itrtxt.hxx"
#include "porglue.hxx"
#include "porlay.hxx"
#include "porfly.hxx"
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
        // don't skip, if the last paragraph line needs space shrinking
        m_pCurr->ExtraShrunkWidth() <= m_pCurr->Width() &&
        m_nStart + m_pCurr->GetLen() >= TextFrameIndex(GetInfo().GetText().getLength());

    // tdf#162725 if the last line is longer, than the paragraph width,
    // it contains shrinking spaces: don't skip block format here
    if( bSkip )
    {
        // sum width of the text portions to calculate the line width without shrinking
        tools::Long nBreakWidth = 0;
        const SwLinePortion *pPos = m_pCurr->GetNextPortion();
        while( pPos && bSkip )
        {
            if( !pPos->InGlueGrp() &&
                // don't calculate with the terminating space,
                // otherwise it would result justified line mistakenly
                ( pPos->GetNextPortion() || !pPos->IsHolePortion() ) )
            {
                nBreakWidth += pPos->Width();
            }

            if( nBreakWidth > m_pCurr->Width() )
                bSkip = false;

            pPos = pPos->GetNextPortion();
        }
    }

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
    GetInfo().GetParaPortion()->GetRepaint().SetOffset(0);
}

static bool lcl_ComputeKashidaPositions(SwTextSizeInfo& rInf, SwTextIter& rItr,
                                        TextFrameIndex& nGluePortion,
                                        const tools::Long nGluePortionWidth,
                                        SwLineLayout* pCurrLine, TextFrameIndex nLineBaseIndex)
{
    // i60594 validate Kashida justification
    TextFrameIndex nIdx = rItr.GetStart();
    TextFrameIndex nEnd = rItr.GetEnd();

    std::vector<TextFrameIndex> aKashidaPositions;
    std::vector<tools::Long> aKashidaWidths;
    tools::Long nMaxKashidaWidth = 0;

    // Parse the text, and apply the kashida insertion rules
    std::function<LanguageType(sal_Int32, sal_Int32, bool)> const pGetLangOfChar(
        [&rInf](sal_Int32 const nBegin, sal_uInt16 const nScript, bool const bNoChar)
        { return rInf.GetTextFrame()->GetLangOfChar(TextFrameIndex{ nBegin }, nScript, bNoChar); });
    SwScanner aScanner(pGetLangOfChar, rInf.GetText(), nullptr, ModelToViewHelper(),
                       i18n::WordType::DICTIONARY_WORD, sal_Int32(nIdx), sal_Int32(nEnd));

    std::vector<bool> aValidPositions;
    while (aScanner.NextWord())
    {
        const OUString& rWord = aScanner.GetWord();

        // Fetch the set of valid positions from VCL, where possible
        if (SwScriptInfo::IsKashidaScriptText(rInf.GetText(), TextFrameIndex{ aScanner.GetBegin() },
                                              TextFrameIndex{ aScanner.GetLen() }))
        {
            aValidPositions.clear();

            rItr.SeekAndChgAttrIter(TextFrameIndex{ aScanner.GetBegin() }, rInf.GetRefDev());

            // Kashida glyph looks suspicious, skip Kashida justification
            auto nFontMinKashida = rInf.GetRefDev()->GetMinKashida();
            if (nFontMinKashida <= 0)
                continue;

            vcl::text::ComplexTextLayoutFlags nOldLayout = rInf.GetRefDev()->GetLayoutMode();
            rInf.GetRefDev()->SetLayoutMode(nOldLayout
                                            | vcl::text::ComplexTextLayoutFlags::BiDiRtl);

            rInf.GetRefDev()->GetWordKashidaPositions(rWord, &aValidPositions);

            rInf.GetRefDev()->SetLayoutMode(nOldLayout);

            auto stKashidaPos = i18nutil::GetWordKashidaPosition(rWord, aValidPositions);
            if (stKashidaPos.has_value())
            {
                TextFrameIndex nNewKashidaPos{ aScanner.GetBegin() + stKashidaPos->nIndex };

                // tdf#164098: The above algorithm can return out-of-range kashida positions. This
                // can happen if, for example, a single word is split across multiple lines, and
                // the best kashida candidate position is on the first line.
                if (nNewKashidaPos >= nIdx && nNewKashidaPos < nEnd)
                {
                    aKashidaPositions.push_back(nNewKashidaPos - nLineBaseIndex);
                    aKashidaWidths.push_back(nFontMinKashida);
                    nMaxKashidaWidth = std::max(nMaxKashidaWidth, nFontMinKashida);
                }
            }
        }
    }

    nGluePortion += TextFrameIndex{ aKashidaPositions.size() };

    // The line may not have enough extra space for all possible kashida.
    // Remove them from the beginning of the line to the end.
    std::reverse(aKashidaPositions.begin(), aKashidaPositions.end());
    std::reverse(aKashidaWidths.begin(), aKashidaWidths.end());

    while (nGluePortion && !aKashidaPositions.empty())
    {
        tools::Long nSpaceAdd = nGluePortionWidth / sal_Int32(nGluePortion);
        if (nSpaceAdd / SPACING_PRECISION_FACTOR >= nMaxKashidaWidth)
        {
            break;
        }

        aKashidaPositions.pop_back();
        aKashidaWidths.pop_back();

        nMaxKashidaWidth = 0;
        if (!aKashidaWidths.empty())
        {
            nMaxKashidaWidth = *std::max_element(aKashidaWidths.begin(), aKashidaWidths.end());
        }

        --nGluePortion;
    }

    std::reverse(aKashidaPositions.begin(), aKashidaPositions.end());
    pCurrLine->SetKashida(std::move(aKashidaPositions));

    return !aKashidaWidths.empty();
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

    TextFrameIndex nLineBase{ 0 };
    if (rSI.ParagraphContainsKashidaScript())
    {
        while (aItr.GetCurr() != pCurrent && aItr.GetNext())
           aItr.Next();

        nLineBase = aItr.GetStart();
        rSI.ReplaceKashidaPositions({});
        pCurrent->SetKashida({});
    }

    // Do not forget: CalcRightMargin() sets pCurrent->Width() to the line width!
    if (!bSkipKashida)
        CalcRightMargin( pCurrent, nReal );

    // #i49277#
    const bool bDoNotJustifyLinesWithManualBreak =
        GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::DO_NOT_JUSTIFY_LINES_WITH_MANUAL_BREAK);
    bool bDoNotJustifyTab = false;

    SwLinePortion *pPos = pCurrent->GetNextPortion();
    // calculate real text width for shrinking
    tools::Long nBreakWidth = 0;

    while( pPos )
    {
        if ( ( bDoNotJustifyLinesWithManualBreak || bDoNotJustifyTab ) &&
             pPos->IsBreakPortion() && !IsLastBlock() )
        {
           pCurrent->FinishSpaceAdd();
           break;
        }

        switch ( pPos->GetWhichPor() )
        {
            case PortionType::TabCenter :
            case PortionType::TabRight :
            case PortionType::TabDecimal :
                bDoNotJustifyTab = true;
                break;
            case PortionType::TabLeft :
            case PortionType::Break:
                bDoNotJustifyTab = false;
                break;
            default: break;
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

                const tools::Long nGluePortionWidth = static_cast<SwGluePortion*>(pPos)->GetPrtGlue() *
                                               SPACING_PRECISION_FACTOR;

                if (rSI.ParagraphContainsKashidaScript() && !bSkipKashida)
                {
                    if (!lcl_ComputeKashidaPositions(aInf, aItr, nGluePortion, nGluePortionWidth,
                                                     pCurrent, nLineBase))
                    {
                        // no kashidas left
                        // do regular blank justification
                        pCurrent->FinishSpaceAdd();
                        GetInfo().SetIdx(m_nStart);
                        CalcNewBlock(pCurrent, pStopAt, nReal, true);
                        return;
                    }
                }

                if( nGluePortion )
                {
                    tools::Long nSpaceAdd = nGluePortionWidth / sal_Int32(nGluePortion);
                    // shrink, if not shrunk line width exceed the set line width
                    // i.e. if pCurrent->ExtraShrunkWidth() > 0
                    // tdf#163720 but at hyphenated lines, still nBreakWidth contains the correct
                    // not shrunk line width (ExtraShrunkWidth + hyphen length), so use that
                    if ( pCurrent->ExtraShrunkWidth() > nBreakWidth )
                        nBreakWidth = pCurrent->ExtraShrunkWidth();
                    // shrink, if portions exceed the line width
                    tools::Long nSpaceSub = ( nBreakWidth > pCurrent->Width() )
                        ? (nBreakWidth - pCurrent->Width()) * SPACING_PRECISION_FACTOR /
                                sal_Int32(nGluePortion) + LONG_MAX/2
                        : ( nSpaceAdd < 0 )
                            // shrink, if portions exceed the line width available before an image
                            ? -nSpaceAdd + LONG_MAX/2
                            : 0;

                    SwLinePortion *pPortion = pCurrent->GetFirstPortion();
                    tools::Long nSpaceKern = pPortion->GetLetterSpacing() > 0 && pPortion->GetSpaceCount()
                        ? tools::Long(pPortion->GetLetterSpacing()) / sal_Int32(pPortion->GetSpaceCount()) * 100
                        : 0;
                    // set expansion in 1/100 twips/space
                    pCurrent->SetLLSpaceAdd( nSpaceSub ? nSpaceSub : (nSpaceAdd > nSpaceKern ? nSpaceAdd - nSpaceKern : 0), nSpaceIdx );
                    pPos->Width( static_cast<SwGluePortion*>(pPos)->GetFixWidth() );
                }
                else if (IsOneBlock() && nCharCnt > TextFrameIndex(1))
                {
                    const tools::Long nSpaceAdd = - nGluePortionWidth / (sal_Int32(nCharCnt) - 1);
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
        else
        {
            nBreakWidth += pPos->Width();
        }
        GetInfo().SetIdx( GetInfo().GetIdx() + pPos->GetLen() );
        if ( pPos == pStopAt )
        {
            pCurrent->SetLLSpaceAdd( 0, nSpaceIdx );
            break;
        }
        pPos = pPos->GetNextPortion();
    }

    // tdf#164140: Rebuild kashida position indices after line adjustment
    if (rSI.ParagraphContainsKashidaScript())
    {
        std::vector<TextFrameIndex> aKashidaPositions;

        SwTextSizeInfo aKashInf(GetTextFrame());
        SwTextIter aKashItr(GetTextFrame(), &aKashInf);

        while (true)
        {
            const SwLineLayout* pCurrLine = aKashItr.GetCurr();
            for (const auto& nPos : pCurrLine->GetKashida())
            {
                aKashidaPositions.push_back(nPos + aKashItr.GetStart());
            }

            if (!aKashItr.GetNextLine())
            {
                break;
            }

            aKashItr.NextLine();
        }

        rSI.ReplaceKashidaPositions(std::move(aKashidaPositions));
    }
}

SwTwips SwTextAdjuster::CalcKanaAdj( SwLineLayout* pCurrent )
{
    OSL_ENSURE( pCurrent->Height(), "SwTextAdjuster::CalcBlockAdjust: missing CalcLine()" );
    OSL_ENSURE( !pCurrent->GetpKanaComp(), "pKanaComp already exists!!" );

    pCurrent->SetKanaComp( std::make_unique<std::deque<sal_uInt16>>() );

    const sal_uInt16 nNull = 0;
    size_t nKanaIdx = 0;
    tools::Long nKanaDiffSum = 0;
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
            SwTwips nMaxWidthDiff = GetInfo().GetMaxWidthDiff(pPos);

            // check, if information is stored under other key
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pCurrent );

            // calculate difference between portion width and max. width
            nKanaDiffSum += nMaxWidthDiff;

            // we store the beginning of the first compressible portion
            // for repaint
            if ( nMaxWidthDiff && !nRepaintOfst )
                nRepaintOfst = nX + GetLeftMargin();
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            if ( nKanaIdx == pCurrent->GetKanaComp().size() )
                pCurrent->GetKanaComp().push_back( nNull );

            tools::Long nRest;

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

                ( pCurrent->GetKanaComp() )[ nKanaIdx ] = o3tl::narrowing<sal_uInt16>(nCompress);
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
    tools::Long nDecompress = 0;

    while( pPos )
    {
        if ( pPos->InTextGrp() )
        {
            const SwTwips nMinWidth = pPos->Width();

            // get maximum portion width from info structure, calculated
            // during text formatting
            SwTwips nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pPos );

            // check, if information is stored under other key
            if ( !nMaxWidthDiff && pPos == pCurrent->GetFirstPortion() )
                nMaxWidthDiff = GetInfo().GetMaxWidthDiff( pCurrent );
            pPos->Width( nMinWidth +
                       ( ( 10000 - nCompress ) * nMaxWidthDiff ) / 10000 );
            nDecompress += pPos->Width() - nMinWidth;
        }
        else if( pPos->InGlueGrp() && pPos->InFixMargGrp() )
        {
            pPos->Width(pPos->Width() - nDecompress);

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
    tools::Long nRealWidth;
    const SwTwips nRealHeight = GetLineHeight();
    const SwTwips nLineHeight = pCurrent->Height();

    SwTwips nPrtWidth = pCurrent->PrtWidth();
    SwLinePortion *pLast = pCurrent->FindLastPortion();

    if( GetInfo().IsMulti() )
        nRealWidth = nReal;
    else
    {
        nRealWidth = GetLineWidth();
        // For each FlyFrame extending into the right margin, we create a FlyPortion.
        const tools::Long nLeftMar = GetLeftMargin();
        SwRect aCurrRect( nLeftMar + nPrtWidth, Y() + nRealHeight - nLineHeight,
                          nRealWidth - nPrtWidth, nLineHeight );

        SwFlyPortion *pFly = CalcFlyPortion( nRealWidth, aCurrRect );
        while( pFly && tools::Long( nPrtWidth )< nRealWidth )
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

    if( tools::Long( nPrtWidth )< nRealWidth )
        pRight->PrtWidth(nRealWidth - nPrtWidth);

    // pCurrent->Width() is set to the real size, because we attach the
    // MarginPortions.
    // This trick gives miraculous results:
    // If pCurrent->Width() == nRealWidth, then the adjustment gets overruled
    // implicitly. GetLeftMarginAdjust() and IsJustified() think they have a
    // line filled with chars.

    pCurrent->PrtWidth(nRealWidth);
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
            pPara->GetRepaint().SetOffset( 0 );
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
SwFlyPortion *SwTextAdjuster::CalcFlyPortion( const tools::Long nRealWidth,
                                             const SwRect &rCurrRect )
{
    SwTextFly aTextFly( GetTextFrame() );

    const SwTwips nCurrWidth = m_pCurr->PrtWidth();
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
        const tools::Long nLocalWidth = aLocal.Left() + aLocal.Width();
        if( nRealWidth < nLocalWidth )
            aLocal.Width( nRealWidth - aLocal.Left() );
        GetInfo().GetParaPortion()->SetFly();
        pFlyPortion = new SwFlyPortion( aLocal );
        pFlyPortion->Height(rCurrRect.Height());
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

    const sal_Int32 nLineNumber = GetLineNr();

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
            const SwLinePortion *pDropPor = pPor->GetNextPortion();
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
                for( sal_Int32 i = 1; i < GetDropLines(); ++i )
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
    for( sal_Int32 i = 1; i < GetDropLines(); ++i )
        NextLine();
    const SwTwips nBottom = Y() + GetLineHeight() - 1;
    if( rRepaint.Bottom() < nBottom )
        rRepaint.Bottom( nBottom );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
