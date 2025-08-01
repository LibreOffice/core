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
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <breakit.hxx>
#include <hintids.hxx>
#include <EnhancedPDFExportHelper.hxx>
#include <SwPortionHandler.hxx>
#include "porlay.hxx"
#include "inftxt.hxx"
#include "guess.hxx"
#include "porfld.hxx"
#include <pagefrm.hxx>
#include <tgrditem.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentMarkAccess.hxx>

#include <IMark.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <o3tl/temporary.hxx>
#include <xmloff/odffields.hxx>
#include <viewopt.hxx>

using namespace ::sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::ScriptType;

static TextFrameIndex lcl_AddSpace_Latin(const SwTextSizeInfo& rInf, const OUString* pStr,
                                         const SwLinePortion& rPor, TextFrameIndex nPos,
                                         TextFrameIndex nEnd, const SwScriptInfo* pSI,
                                         sal_uInt8 nScript);

// Returns for how many characters an extra space has to be added
// (for justified alignment).
static TextFrameIndex lcl_AddSpace(const SwTextSizeInfo &rInf,
        const OUString* pStr, const SwLinePortion& rPor)
{
    TextFrameIndex nPos, nEnd;
    const SwScriptInfo* pSI = nullptr;

    if ( pStr )
    {
        // passing a string means we are inside a field
        nPos = TextFrameIndex(0);
        nEnd = TextFrameIndex(pStr->getLength());
    }
    else
    {
        nPos = rInf.GetIdx();
        nEnd = rInf.GetIdx() + rPor.GetLen();
        pStr = &rInf.GetText();
        pSI = &const_cast<SwParaPortion*>(rInf.GetParaPortion())->GetScriptInfo();
    }

    TextFrameIndex nCnt(0);
    sal_uInt8 nScript = 0;

    // If portion consists of Asian characters and language is not
    // Korean, we add extra space to each character.
    // first we get the script type
    if ( pSI )
        nScript = pSI->ScriptType( nPos );
    else
        nScript = static_cast<sal_uInt8>(
            g_pBreakIt->GetBreakIter()->getScriptType(*pStr, sal_Int32(nPos)));

    // Note: rInf.GetIdx() can differ from nPos,
    // e.g., when rPor is a field portion. nPos refers to the string passed
    // to the function, rInf.GetIdx() refers to the original string.

    // We try to find out which justification mode is required. This is done by
    // evaluating the script type and the language attribute set for this portion

    // Asian Justification: Each character get some extra space
    if ( nEnd > nPos && ASIAN == nScript )
    {
        LanguageType aLang =
            rInf.GetTextFrame()->GetLangOfChar(rInf.GetIdx(), nScript);

        if (!MsLangId::isKorean(aLang))
        {
            const SwLinePortion* pPor = rPor.GetNextPortion();
            if ( pPor && ( pPor->IsKernPortion() ||
                           pPor->IsControlCharPortion() ||
                           pPor->IsPostItsPortion() ) )
                pPor = pPor->GetNextPortion();

            nCnt += SwScriptInfo::CountCJKCharacters( *pStr, nPos, nEnd, aLang );

            if ( !pPor || pPor->IsHolePortion() || pPor->InFixMargGrp() ||
                  pPor->IsBreakPortion() )
                --nCnt;

            return nCnt;
        }
    }

    // Kashida Justification: Insert Kashidas
    if ( nEnd > nPos && pSI && COMPLEX == nScript )
    {
        if (pSI->ParagraphContainsKashidaScript()
            && SwScriptInfo::IsKashidaScriptText(*pStr, nPos, nEnd - nPos))
        {
            // tdf#163105: For kashida justification, also expand whitespace.
            return lcl_AddSpace_Latin(rInf, pStr, rPor, nPos, nEnd, pSI, nScript)
                   + TextFrameIndex{ pSI->CountKashidaPositions(nPos, nEnd) };
        }
    }

    // Thai Justification: Each character cell gets some extra space
    if ( nEnd > nPos && COMPLEX == nScript )
    {
        LanguageType aLang =
            rInf.GetTextFrame()->GetLangOfChar(rInf.GetIdx(), nScript);

        if ( LANGUAGE_THAI == aLang )
        {
            nCnt = SwScriptInfo::ThaiJustify(*pStr, nullptr, nPos, nEnd - nPos);

            const SwLinePortion* pPor = rPor.GetNextPortion();
            if ( pPor && ( pPor->IsKernPortion() ||
                           pPor->IsControlCharPortion() ||
                           pPor->IsPostItsPortion() ) )
                pPor = pPor->GetNextPortion();

            if ( nCnt && ( ! pPor || pPor->IsHolePortion() || pPor->InFixMargGrp() ) )
                --nCnt;

            return nCnt;
        }
    }

    return lcl_AddSpace_Latin(rInf, pStr, rPor, nPos, nEnd, pSI, nScript);
}

static TextFrameIndex lcl_AddSpace_Latin(const SwTextSizeInfo& rInf, const OUString* pStr,
                                         const SwLinePortion& rPor, TextFrameIndex nPos,
                                         TextFrameIndex nEnd, const SwScriptInfo* pSI,
                                         sal_uInt8 nScript)
{
    TextFrameIndex nCnt(0);

    // Here starts the good old "Look for blanks and add space to them" part.
    // Note: We do not want to add space to an isolated latin blank in front
    // of some complex characters in RTL environment
    const bool bDoNotAddSpace =
            LATIN == nScript && (nEnd == nPos + TextFrameIndex(1)) && pSI &&
            ( i18n::ScriptType::COMPLEX ==
              pSI->ScriptType(nPos + TextFrameIndex(1))) &&
            rInf.GetTextFrame() && rInf.GetTextFrame()->IsRightToLeft();

    if ( bDoNotAddSpace )
        return nCnt;

    TextFrameIndex nTextEnd = std::min(nEnd, TextFrameIndex(pStr->getLength()));
    for ( ; nPos < nTextEnd; ++nPos )
    {
        if (CH_BLANK == (*pStr)[ sal_Int32(nPos) ])
            ++nCnt;
    }

    // We still have to examine the next character:
    // If the next character is ASIAN and not KOREAN we have
    // to add an extra space
    // nPos refers to the original string, even if a field string has
    // been passed to this function
    nPos = rInf.GetIdx() + rPor.GetLen();
    if (nPos < TextFrameIndex(rInf.GetText().getLength()))
    {
        sal_uInt8 nNextScript = 0;
        const SwLinePortion* pPor = rPor.GetNextPortion();
        if ( pPor && pPor->IsKernPortion() )
            pPor = pPor->GetNextPortion();

        if (!pPor || pPor->InFixMargGrp())
            return nCnt;

        // next character is inside a field?
        if ( CH_TXTATR_BREAKWORD == rInf.GetChar( nPos ) && pPor->InExpGrp() )
        {
            bool bOldOnWin = rInf.OnWin();
            const_cast<SwTextSizeInfo &>(rInf).SetOnWin( false );

            OUString aStr;
            pPor->GetExpText( rInf, aStr );
            const_cast<SwTextSizeInfo &>(rInf).SetOnWin( bOldOnWin );

            nNextScript = static_cast<sal_uInt8>(g_pBreakIt->GetBreakIter()->getScriptType( aStr, 0 ));
        }
        else
            nNextScript = static_cast<sal_uInt8>(
                g_pBreakIt->GetBreakIter()->getScriptType(rInf.GetText(), sal_Int32(nPos)));

        if( ASIAN == nNextScript )
        {
            LanguageType aLang =
                rInf.GetTextFrame()->GetLangOfChar(nPos, nNextScript);

            if (!MsLangId::isKorean(aLang))
                ++nCnt;
        }
    }

    return nCnt;
}

static void GetLimitedStringPart(const SwTextFormatInfo& rInf, TextFrameIndex nIndex,
                                 TextFrameIndex nLength, sal_uInt16 nComp, SwTwips nOriginalWidth,
                                 SwTwips nMaxWidth, TextFrameIndex& rOutLength, SwTwips& rOutWidth)
{
    assert(nLength >= TextFrameIndex(0));
    const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();
    rOutLength = nLength;
    rOutWidth = nOriginalWidth;
    if (nMaxWidth < 0)
        nMaxWidth = 0;
    while (rOutWidth > nMaxWidth)
    {
        TextFrameIndex nNewOnLineLengthGuess(rOutLength.get() * nMaxWidth / rOutWidth);
        assert(nNewOnLineLengthGuess < rOutLength);
        if (nNewOnLineLengthGuess < (rOutLength - TextFrameIndex(1)))
            ++nNewOnLineLengthGuess; // to avoid too aggressive decrease
        rOutLength = nNewOnLineLengthGuess;
        rInf.GetTextSize(&rSI, nIndex, rOutLength, std::nullopt, nComp, rOutWidth,
                         o3tl::temporary(tools::Long()), o3tl::temporary(SwTwips()),
                         o3tl::temporary(SwTwips()), rInf.GetCachedVclData().get());
    }
}

static bool IsMsWordUlTrailSpace(const SwTextFormatInfo& rInf)
{
    const auto& settings = rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess();
    return settings.get(DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS)
           && settings.get(DocumentSettingId::MS_WORD_UL_TRAIL_SPACE);
}

SwTextPortion * SwTextPortion::CopyLinePortion(const SwLinePortion &rPortion)
{
    SwTextPortion *const pNew(new SwTextPortion);
    static_cast<SwLinePortion&>(*pNew) = rPortion;
    pNew->SetWhichPor( PortionType::Text ); // overwrite that!
    return pNew;
}

void SwTextPortion::BreakCut( SwTextFormatInfo &rInf, const SwTextGuess &rGuess )
{
    // The word/char is larger than the line
    // Special case 1: The word is larger than the line
    // We truncate ...
    const SwTwips nLineWidth = rInf.Width() - rInf.X();
    TextFrameIndex nLen = rGuess.CutPos() - rInf.GetIdx();
    if (nLen > TextFrameIndex(0))
    {
        // special case: guess does not always provide the correct
        // width, only in common cases.
        if ( !rGuess.BreakWidth() )
        {
            rInf.SetLen( nLen );
            SetLen( nLen );
            CalcTextSize( rInf );

            // changing these values requires also changing them in
            // guess.cxx
            SwTwips nItalic = 0;
            if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
            {
                nItalic = Height() / 12;
            }
            Width( Width() + nItalic );
        }
        else
        {
            Width( rGuess.BreakWidth() );
            SetLen( nLen );
        }
    }
    // special case: first character does not fit to line
    else if ( rGuess.CutPos() == rInf.GetLineStart() )
    {
        SetLen( TextFrameIndex(1) );
        Width( nLineWidth );
    }
    else
    {
        SetLen( TextFrameIndex(0) );
        Width( 0 );
        ExtraShrunkWidth( 0 );
        ExtraSpaceSize( 0 );
        SetLetterSpacing( 0 );
    }
}

void SwTextPortion::BreakUnderflow( SwTextFormatInfo &rInf )
{
    Truncate();
    Height( 0 );
    Width( 0 );
    ExtraShrunkWidth( 0 );
    ExtraSpaceSize( 0 );
    SetLetterSpacing( 0 );
    SetLen( TextFrameIndex(0) );
    SetAscent( 0 );
    rInf.SetUnderflow( this );
}

static bool lcl_HasContent( const SwFieldPortion& rField, SwTextFormatInfo const &rInf )
{
    OUString aText;
    return rField.GetExpText( rInf, aText ) && !aText.isEmpty();
}

sal_uInt16 SwTextPortion::GetMaxComp(const SwTextFormatInfo& rInf) const
{
    const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();
    return (SwFontScript::CJK == rInf.GetFont()->GetActual()) && rSI.CountCompChg()
                   && !rInf.IsMulti() && !InFieldGrp() && !IsDropPortion()
               ? 10000
               : 0;
}

bool SwTextPortion::Format_( SwTextFormatInfo &rInf )
{
    // 5744: If only the hyphen does not fit anymore, we still need to wrap
    // the word, or else return true!
    if( rInf.IsUnderflow() && rInf.GetSoftHyphPos() )
    {
        // soft hyphen portion has triggered an underflow event because
        // of an alternative spelling position
        bool bFull = false;
        const bool bHyph = rInf.ChgHyph( true );
        if( rInf.IsHyphenate() )
        {
            SwTextGuess aGuess;
            // check for alternative spelling left from the soft hyphen
            // this should usually be true but
            aGuess.AlternativeSpelling(rInf, rInf.GetSoftHyphPos() - TextFrameIndex(1));
            bFull = CreateHyphen( rInf, aGuess );
            OSL_ENSURE( bFull, "Problem with hyphenation!!!" );
        }
        rInf.ChgHyph( bHyph );
        rInf.SetSoftHyphPos( TextFrameIndex(0) );
        return bFull;
    }

    ExtraShrunkWidth( 0 );
    ExtraSpaceSize( 0 );
    SetLetterSpacing( 0 );
    std::optional<SwTextGuess> pGuess(std::in_place);
    bool bFull = !pGuess->Guess( *this, rInf, Height() );

    // tdf#158776 for the last full text portion, call Guess() again to allow more text in the
    // adjusted line by shrinking spaces using the know space count from the first Guess() call
    SvxAdjustItem aAdjustItem = rInf.GetTextFrame()->GetTextNodeForParaProps()->GetSwAttrSet().GetAdjust();
    const SvxAdjust aAdjust = aAdjustItem.GetAdjust();
    bool bFullJustified = bFull && aAdjust == SvxAdjust::Block &&
         pGuess->BreakPos() != TextFrameIndex(COMPLETE_STRING);
    bool bInteropSmartJustify = bFullJustified &&
            rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
                    DocumentSettingId::JUSTIFY_LINES_WITH_SHRINKING);
    bool bNoWordSpacing = aAdjustItem.GetPropWordSpacing() == 100 &&
                    aAdjustItem.GetPropWordSpacingMinimum() == 100 &&
                    aAdjustItem.GetPropWordSpacingMaximum() == 100 &&
                    aAdjustItem.GetPropLetterSpacingMaximum() == 0;
    // support old ODT documents, where only JustifyLinesWithShrinking was set
    bool bOldInterop = bInteropSmartJustify && bNoWordSpacing;
    bool bWordSpacing = bFullJustified && (!bNoWordSpacing || bOldInterop);
    bool bWordSpacingMaximum = bWordSpacing && !bOldInterop &&
           aAdjustItem.GetPropWordSpacingMaximum() > aAdjustItem.GetPropWordSpacing();
    bool bWordSpacingMinimum = bWordSpacing && ( bOldInterop ||
           aAdjustItem.GetPropWordSpacingMinimum() < aAdjustItem.GetPropWordSpacing() );

    if ( ( bInteropSmartJustify || bWordSpacing || bWordSpacingMaximum || bWordSpacingMinimum ) &&
         // tdf#164499 no shrinking in tabulated line
         ( !rInf.GetLast() || !rInf.GetLast()->InTabGrp() ) &&
         // tdf#158436 avoid shrinking at underflow, e.g. no-break space after a
         // very short word resulted endless loop
         !rInf.IsUnderflow() )
    {
        sal_Int32 nSpacesInLine = rInf.GetLineSpaceCount( pGuess->BreakPos() );
        sal_Int32 nSpacesInLineOrig = nSpacesInLine;
        SwTextSizeInfo aOrigInf( rInf );

        // call with an extra space: shrinking can result a new word in the line
        // and a new space before that, which is also a shrank space
        // (except if the line was already broken inside a word with hyphenation)
        // TODO: handle the case, if the line contains extra amount of spaces
        if (
             // no automatic hyphenation
             !pGuess->HyphWord().is() &&
             // no hyphenation at soft hyphen
             pGuess->BreakPos() < TextFrameIndex(rInf.GetText().getLength()) &&
             rInf.GetText()[sal_Int32(pGuess->BreakPos())] != CHAR_SOFTHYPHEN )
        {
            ++nSpacesInLine;
        }

        // there are spaces in the line, so it's possible to shrink them
        if ( nSpacesInLineOrig > 0 )
        {
            SwTwips nOldWidth = pGuess->BreakWidth();
            bool bIsPortion = rInf.GetLineWidth() < rInf.GetBreakWidth();

            // measure ten spaces for higher precision
            static constexpr OUStringLiteral STR_BLANK = u"          ";
            sal_Int16 nSpaceWidth = rInf.GetTextSize(STR_BLANK).Width();
            sal_Int32 nRealSpaces = rInf.GetLineSpaceCount( pGuess->BreakPos() );
            float fSpaceNormal = (rInf.GetLineWidth() - (rInf.GetBreakWidth() - nRealSpaces * nSpaceWidth/10.0))/nRealSpaces;

            bool bOrigHyphenated = pGuess->HyphWord().is() &&
                        pGuess->BreakPos() > rInf.GetLineStart();

            // calculate available word spacing for letter spacing, and for the word spacing indicator
            // for non-hyphenated single portion lines
            // TODO: enable letter spacing for multiportion lines
            if ( !bOrigHyphenated && rInf.GetLineStart() == rInf.GetIdx() )
            {
                // TODO calculate correct value for letter spacing of hyphenated lines
                float fExpansionOverMax = fSpaceNormal - nSpaceWidth/10.0 * aAdjustItem.GetPropWordSpacingMaximum()/100.0;
                ExtraSpaceSize( rInf.GetBreakWidth() > rInf.GetLineWidth()/2 && fExpansionOverMax > 0 ? fExpansionOverMax : 0);
                int nLetterCount = sal_Int32(pGuess->BreakPos()) - sal_Int32(rInf.GetIdx());
                float fAvailableLetterSpacing = ((fSpaceNormal - nSpaceWidth/10.0) * nRealSpaces) / nLetterCount;
                float fCustomLetterSpacing = nSpaceWidth/10.0 * aAdjustItem.GetPropLetterSpacingMaximum() / 100.0;
                // TODO fix resolution applying 1/100 twips instead of 1 twip
                SwTwips nLetterSpacing = std::min(fAvailableLetterSpacing, fCustomLetterSpacing);
                SetLetterSpacing(SwTwips(nLetterSpacing * nLetterCount));
                SetSpaceCount(TextFrameIndex(nRealSpaces));
            }

            // calculate line breaking with desired word spacing, also
            // if the desired word spacing is 100%, but there is a greater
            // maximum word spacing, and the word is hyphenated at the desired
            // word spacing: to skip hyphenation, if the maximum word spacing allows it
            if ( bWordSpacing || ( bWordSpacingMaximum && bOrigHyphenated ) )
            {
                pGuess.emplace();
                bFull = !pGuess->Guess( *this, rInf, Height(), nSpacesInLine, aAdjustItem.GetPropWordSpacing(), nSpaceWidth );
                sal_Int32 nSpacesInLine2 = rInf.GetLineSpaceCount( pGuess->BreakPos() );

                if ( rInf.GetBreakWidth() <= rInf.GetLineWidth() )
                {
                    fSpaceNormal = (rInf.GetLineWidth() - (rInf.GetBreakWidth() - nSpacesInLine2 * nSpaceWidth/10.0))/nSpacesInLine2;
                    // TODO: enable letter spacing for multiportion lines
                    if ( !bOrigHyphenated && rInf.GetLineStart() == rInf.GetIdx() )
                    {
                        float fExpansionOverMax = fSpaceNormal - nSpaceWidth/10.0 * aAdjustItem.GetPropWordSpacingMaximum()/100.0;
                        ExtraSpaceSize( rInf.GetBreakWidth() > rInf.GetLineWidth()/2 && fExpansionOverMax > 0 ? fExpansionOverMax : 0);
                        int nLetterCount = sal_Int32(pGuess->BreakPos()) - sal_Int32(rInf.GetIdx());
                        float fAvailableLetterSpacing = ((fSpaceNormal - nSpaceWidth/10.0) * nRealSpaces) / nLetterCount;
                        float fCustomLetterSpacing = nSpaceWidth/10.0 * aAdjustItem.GetPropLetterSpacingMaximum() / 100.0;
                        // TODO fix resolution applying 1/100 twips instead of 1 twip
                        SwTwips nLetterSpacing = std::min(fAvailableLetterSpacing, fCustomLetterSpacing);
                        SetLetterSpacing(SwTwips(nLetterSpacing * nLetterCount));
                        SetSpaceCount(TextFrameIndex(nRealSpaces));
                    }
                }
            }

            sal_Int32 nSpacesInLineShrink = 0;
            // TODO if both maximum word spacing or minimum word spacing can disable hyphenation, prefer the last one
            if ( bWordSpacingMinimum )
            {
                std::optional<SwTextGuess> pGuess2(std::in_place);
                SwTwips nOldExtraSpace = rInf.GetExtraSpace();
                // break the line after the hyphenated word, if it's possible
                // (hyphenation is disabled in Guess(), when called with GetPropWordSpacingMinimum())
                sal_uInt16 nMinimum = bOldInterop ? 75 : aAdjustItem.GetPropWordSpacingMinimum();
                bool bFull2 = !pGuess2->Guess( *this, rInf, Height(), nSpacesInLine, nMinimum, nSpaceWidth );
                nSpacesInLineShrink = rInf.GetLineSpaceCount( pGuess2->BreakPos() );
                if ( pGuess2->BreakWidth() > nOldWidth )
                {
                    // instead of the maximum shrinking, break after the word which was hyphenated before
                    sal_Int32 i = sal_Int32(pGuess->BreakPos());
                    sal_Int32 j = sal_Int32(pGuess2->BreakPos());
                    // skip terminal spaces
                    for (; i < j && rInf.GetText()[i] == CH_BLANK; ++i);
                    for (; j > i && rInf.GetText()[i] == CH_BLANK; --j);
                    sal_Int32 nOldBreakTrim = i;
                    sal_Int32 nOldBreak = j - i;
                    for (; i < j; ++i)
                    {
                        sal_Unicode cChar = rInf.GetText()[i];
                        // first space after the hyphenated word, and it's not the chosen one
                        if ( cChar == CH_BLANK )
                        {
                            // using a weighted word spacing, try to break the line after the hyphenated word
                            sal_Int32 nNewBreak = i - nOldBreakTrim;
                            SwTwips nWeightedSpacing = nMinimum * (1.0 * nNewBreak/nOldBreak) +
                                   aAdjustItem.GetPropWordSpacing() * (1.0 * (nOldBreak - nNewBreak)/nOldBreak);
                            std::optional<SwTextGuess> pGuess3(std::in_place);
                            pGuess3->Guess( *this, rInf, Height(), nSpacesInLineShrink-1, nWeightedSpacing, nSpaceWidth );

                            sal_Int32 nSpacesInLineShrink2 = rInf.GetLineSpaceCount( pGuess3->BreakPos() );
                            if ( nSpacesInLineShrink2 == nSpacesInLineShrink )
                            {
                                nNewBreak = i - nOldBreakTrim - 1;
                                nWeightedSpacing = nMinimum * (1.0 * nNewBreak/nOldBreak) +
                                    aAdjustItem.GetPropWordSpacing() * (1.0 * (nOldBreak - nNewBreak)/nOldBreak);
                                pGuess3->Guess( *this, rInf, Height(), nSpacesInLineShrink-1, nWeightedSpacing, nSpaceWidth );
                            }

                            if ( pGuess3->BreakWidth() > nOldWidth )
                            {
                                pGuess2.emplace();
                                pGuess2 = std::move(pGuess3);
                            }
                            break;
                        }
                    }

                    nSpacesInLineShrink = rInf.GetLineSpaceCount( pGuess2->BreakPos() );
                    if ( rInf.GetBreakWidth() > rInf.GetLineWidth() || bIsPortion )
                    {
                        float fExpansionWeight = static_cast<float>(1/1.7);
                        float fSpaceShrunk = nSpacesInLineShrink > 0
                                ? (rInf.GetLineWidth() - (rInf.GetBreakWidth() - nSpacesInLineShrink * nSpaceWidth/10.0))/nSpacesInLineShrink
                                : 1;
                        float z0 = (nSpaceWidth/10.0)/fSpaceShrunk;
                        float z1 = (nSpaceWidth/10.0+((fSpaceNormal-nSpaceWidth/10.0)*fExpansionWeight))/(nSpaceWidth/10.0);
                        // TODO shrink line portions only if needed
                        if ( z1 >= z0 || bIsPortion )
                        {
                            pGuess = std::move(pGuess2);
                            ExtraSpaceSize(0);
                            SetLetterSpacing(0);
                            bFull = bFull2;
                        }
                    }
                    else if ( bOldInterop )
                    {
                        pGuess = std::move(pGuess2);
                        ExtraSpaceSize(0);
                        SetLetterSpacing(0);
                        bFull = bFull2;
                    }
                }
                else
                    // minimum word spacing is not applicable
                    rInf.SetExtraSpace(nOldExtraSpace);
            }

            if ( pGuess->BreakWidth() != nOldWidth )
            {
                ExtraShrunkWidth( pGuess->BreakWidth() );
                ExtraSpaceSize( 0 );
                SetLetterSpacing(0);
            }
        }
    }

    // these are the possible cases:
    // A Portion fits to current line
    // B Portion does not fit to current line but a possible line break
    //   within the portion has been found by the break iterator, 2 subcases
    //   B1 break is hyphen
    //   B2 break is word end
    // C Portion does not fit to current line and no possible line break
    //   has been found by break iterator, 2 subcases:
    //   C1 break iterator found a possible line break in portion before us
    //      ==> this break is used (underflow)
    //   C2 break iterator does not found a possible line break at all:
    //      ==> line break

    // case A: line not yet full
    if ( !bFull )
    {
        Width( pGuess->BreakWidth() );
        ExtraBlankWidth(pGuess->ExtraBlankWidth());
        // Caution!
        if( !InExpGrp() || InFieldGrp() )
            SetLen( rInf.GetLen() );

        short nKern = rInf.GetFont()->CheckKerning();
        if( nKern > 0 && rInf.Width() < rInf.X() + Width() + nKern )
        {
            nKern = static_cast<short>(rInf.Width() - rInf.X() - Width() - 1);
            if( nKern < 0 )
                nKern = 0;
        }
        if( nKern )
            new SwKernPortion( *this, nKern );
    }
    // special case: hanging portion
    else if( pGuess->GetHangingPortion() )
    {
        Width( pGuess->BreakWidth() );
        SetLen( pGuess->BreakPos() - rInf.GetIdx() );
        pGuess->GetHangingPortion()->SetAscent( GetAscent() );
        Insert( pGuess->ReleaseHangingPortion() );
    }
    // breakPos >= index
    else if (pGuess->BreakPos() >= rInf.GetIdx() && pGuess->BreakPos() != TextFrameIndex(COMPLETE_STRING))
    {
        // case B1
        if( pGuess->HyphWord().is() && pGuess->BreakPos() > rInf.GetLineStart()
            && ( pGuess->BreakPos() > rInf.GetIdx() ||
               ( rInf.GetLast() && ! rInf.GetLast()->IsFlyPortion() ) ) )
        {
            CreateHyphen( rInf, *pGuess );
            if ( rInf.GetFly() )
                rInf.GetRoot()->SetMidHyph( true );
            else
                rInf.GetRoot()->SetEndHyph( true );
        }
        // case C1
        // - Footnote portions with fake line start (i.e., not at beginning of line)
        //   should keep together with the text portion. (Note: no keep together
        //   with only footnote portions.
        // - TabPortions not at beginning of line should keep together with the
        //   text portion, if they are not followed by a blank
        //   (work around different definition of tab stop character - breaking or
        //   non breaking character - in compatibility mode)
        else if ( ( IsFootnotePortion() && rInf.IsFakeLineStart() &&

                    rInf.IsOtherThanFootnoteInside() ) ||
                  ( rInf.GetLast() &&
                    rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(DocumentSettingId::TAB_COMPAT) &&
                    rInf.GetLast()->InTabGrp() &&
                    rInf.GetLineStart() + rInf.GetLast()->GetLen() < rInf.GetIdx() &&
                    pGuess->BreakPos() == rInf.GetIdx()  &&
                    CH_BLANK != rInf.GetChar( rInf.GetIdx() ) &&
                    CH_FULL_BLANK != rInf.GetChar( rInf.GetIdx() ) &&
                    CH_SIX_PER_EM != rInf.GetChar( rInf.GetIdx() ) ) )
            BreakUnderflow( rInf );
        // case B2
        else if( rInf.GetIdx() > rInf.GetLineStart() ||
                 pGuess->BreakPos() > rInf.GetIdx() ||
                 // this is weird: during formatting the follow of a field
                 // the values rInf.GetIdx and rInf.GetLineStart are replaced
                 // IsFakeLineStart indicates GetIdx > GetLineStart
                 rInf.IsFakeLineStart() ||
                 rInf.GetFly() ||
                 rInf.IsFirstMulti() ||
                 ( rInf.GetLast() &&
                    ( rInf.GetLast()->IsFlyPortion() ||
                        ( rInf.GetLast()->InFieldGrp() &&
                          ! rInf.GetLast()->InNumberGrp() &&
                          ! rInf.GetLast()->IsErgoSumPortion() &&
                          lcl_HasContent(*static_cast<SwFieldPortion*>(rInf.GetLast()),rInf ) ) ) ) )
        {
            Width( pGuess->BreakWidth() );

            SetLen( pGuess->BreakPos() - rInf.GetIdx() );

            // Clamp layout context to the end of the line
            if(auto stClampedContext = GetLayoutContext(); stClampedContext.has_value()) {
                stClampedContext->m_nEnd = pGuess->BreakPos().get();
                SetLayoutContext(stClampedContext);
            }

            OSL_ENSURE( pGuess->BreakStart() >= pGuess->FieldDiff(),
                    "Trouble with expanded field portions during line break" );
            TextFrameIndex const nRealStart = pGuess->BreakStart() - pGuess->FieldDiff();
            if( pGuess->BreakPos() < nRealStart && !InExpGrp() )
            {
                TextFrameIndex nTotalExtraLen(nRealStart - pGuess->BreakPos());
                TextFrameIndex nExtraLen(nTotalExtraLen);
                TextFrameIndex nExtraLenOutOfLine(0);
                SwTwips nTotalExtraWidth(pGuess->ExtraBlankWidth());
                SwTwips nExtraWidth(nTotalExtraWidth);
                SwTwips nExtraWidthOutOfLine(0);
                SwTwips nAvailableLineWidth(rInf.GetLineWidth() - Width());
                const bool bMsWordUlTrailSpace(IsMsWordUlTrailSpace(rInf));
                if (nExtraWidth > nAvailableLineWidth && bMsWordUlTrailSpace)
                {
                    GetLimitedStringPart(rInf, pGuess->BreakPos(), nTotalExtraLen, GetMaxComp(rInf),
                                         nTotalExtraWidth, nAvailableLineWidth, nExtraLen,
                                         nExtraWidth);
                    nExtraLenOutOfLine = nTotalExtraLen - nExtraLen;
                    nExtraWidthOutOfLine = nTotalExtraWidth - nExtraWidth;
                }

                SwHolePortion* pNew = new SwHolePortion(*this, bMsWordUlTrailSpace);
                pNew->SetLen(nExtraLen);
                pNew->ExtraBlankWidth(nExtraWidth);
                Insert( pNew );

                if (nExtraWidthOutOfLine)
                {
                    // Out-of-line hole portion - will not show underline
                    SwHolePortion* pNewOutOfLine = new SwHolePortion(*this, false);
                    pNewOutOfLine->SetLen(nExtraLenOutOfLine);
                    pNewOutOfLine->ExtraBlankWidth(nExtraWidthOutOfLine);
                    pNew->Insert(pNewOutOfLine);
                }

                // UAX #14 Unicode Line Breaking Algorithm Non-tailorable Line breaking rule LB6:
                // https://www.unicode.org/reports/tr14/#LB6 Do not break before hard line breaks
                if (auto ch = rInf.GetChar(pGuess->BreakStart()); !ch || ch == CH_BREAK)
                    bFull = false; // Keep following SwBreakPortion / para break in the same line
            }
        }
        else    // case C2, last exit
            BreakCut( rInf, *pGuess );
    }
    // breakPos < index or no breakpos at all
    else
    {
        bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if (pGuess->BreakPos() != TextFrameIndex(COMPLETE_STRING) &&
            pGuess->BreakPos() != rInf.GetLineStart() &&
            ( !bFirstPor || rInf.GetFly() || rInf.GetLast()->IsFlyPortion() ||
              rInf.IsFirstMulti() ) &&
            ( !rInf.GetLast()->IsBlankPortion() ||
              SwBlankPortion::MayUnderflow(rInf, rInf.GetIdx() - TextFrameIndex(1), true)))
        {       // case C1 (former BreakUnderflow())
            BreakUnderflow( rInf );
        }
        else
             // case C2, last exit
            BreakCut(rInf, *pGuess);
    }

    return bFull;
}

bool SwTextPortion::Format( SwTextFormatInfo &rInf )
{
    // GetLineWidth() takes care of DocumentSettingId::TAB_OVER_MARGIN.
    if( rInf.GetLineWidth() + rInf.GetExtraSpace() < 0 || (!GetLen() && !InExpGrp()) )
    {
        Height( 0 );
        Width( 0 );
        ExtraShrunkWidth( 0 );
        ExtraSpaceSize( 0 );
        SetLetterSpacing( 0 );
        SetLen( TextFrameIndex(0) );
        SetAscent( 0 );
        SetNextPortion( nullptr );  // ????
        return true;
    }

    OSL_ENSURE( rInf.RealWidth() || (rInf.X() == rInf.Width()),
        "SwTextPortion::Format: missing real width" );
    OSL_ENSURE( Height(), "SwTextPortion::Format: missing height" );

    return Format_( rInf );
}

// Format end of line
// 5083: We can have awkward cases e.g.:
// "from {Santa}"
// Santa wraps, "from " turns into "from" and " " in a justified
// paragraph, in which the glue gets expanded instead of merged
// with the MarginPortion.

// rInf.nIdx points to the next word, nIdx-1 is the portion's last char
void SwTextPortion::FormatEOL( SwTextFormatInfo &rInf )
{
    if( ( GetNextPortion() &&
          ( !GetNextPortion()->IsKernPortion() || GetNextPortion()->GetNextPortion() ) ) ||
        !GetLen() ||
        rInf.GetIdx() >= TextFrameIndex(rInf.GetText().getLength()) ||
        TextFrameIndex(1) >= rInf.GetIdx() ||
        ' ' != rInf.GetChar(rInf.GetIdx() - TextFrameIndex(1)) ||
        rInf.GetLast()->IsHolePortion() )
        return;

    // calculate number of blanks
    TextFrameIndex nX(rInf.GetIdx() - TextFrameIndex(1));
    TextFrameIndex nHoleLen(1);
    while( nX && nHoleLen < GetLen() && CH_BLANK == rInf.GetChar( --nX ) )
        nHoleLen++;

    // First set ourselves and the insert, because there could be
    // a SwLineLayout
    SwTwips nBlankSize;
    if( nHoleLen == GetLen() )
        nBlankSize = Width();
    else
        nBlankSize = sal_Int32(nHoleLen) * rInf.GetTextSize(OUString(' ')).Width();
    Width( Width() - nBlankSize );
    rInf.X( rInf.X() - nBlankSize );
    SetLen( GetLen() - nHoleLen );
    SwHolePortion* pHole = new SwHolePortion(*this);
    pHole->SetBlankWidth(nBlankSize);
    pHole->SetLen(nHoleLen);
    Insert( pHole );

}

TextFrameIndex SwTextPortion::GetModelPositionForViewPoint(const SwTwips nOfst) const
{
    OSL_ENSURE( false, "SwTextPortion::GetModelPositionForViewPoint: don't use this method!" );
    return SwLinePortion::GetModelPositionForViewPoint( nOfst );
}

// The GetTextSize() assumes that the own length is correct
SwPositiveSize SwTextPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwPositiveSize aSize = rInf.GetTextSize(GetLayoutContext());
    if( !GetJoinBorderWithPrev() )
        aSize.Width(aSize.Width() + rInf.GetFont()->GetLeftBorderSpace() );
    if( !GetJoinBorderWithNext() )
        aSize.Width(aSize.Width() + rInf.GetFont()->GetRightBorderSpace() );

    aSize.Height(aSize.Height() +
        rInf.GetFont()->GetTopBorderSpace() +
        rInf.GetFont()->GetBottomBorderSpace() );

    return aSize;
}

void SwTextPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if (rInf.OnWin() && TextFrameIndex(1) == rInf.GetLen()
        && CH_TXT_ATR_FIELDEND == rInf.GetText()[sal_Int32(rInf.GetIdx())])
    {
        assert(false); // this is some debugging only code
        rInf.DrawBackBrush( *this );
        const OUString aText(CH_TXT_ATR_SUBST_FIELDEND);
        rInf.DrawText(aText, *this, TextFrameIndex(0), TextFrameIndex(aText.getLength()));
    }
    else if (rInf.OnWin() && TextFrameIndex(1) == rInf.GetLen()
        && CH_TXT_ATR_FIELDSTART == rInf.GetText()[sal_Int32(rInf.GetIdx())])
    {
        assert(false); // this is some debugging only code
        rInf.DrawBackBrush( *this );
        const OUString aText(CH_TXT_ATR_SUBST_FIELDSTART);
        rInf.DrawText(aText, *this, TextFrameIndex(0), TextFrameIndex(aText.getLength()));
    }
    else if( GetLen() )
    {
        rInf.DrawBackBrush( *this );
        rInf.DrawBorder( *this );

        rInf.DrawCSDFHighlighting(*this);

        // do we have to repaint a post it portion?
        if( rInf.OnWin() && mpNextPortion && !mpNextPortion->Width() )
            mpNextPortion->PrePaint( rInf, this );

        auto const* pWrongList = rInf.GetpWrongList();
        auto const* pGrammarCheckList = rInf.GetGrammarCheckList();
        auto const* pSmarttags = rInf.GetSmartTags();

        const bool bWrong = nullptr != pWrongList;
        const bool bGrammarCheck = nullptr != pGrammarCheckList;
        const bool bSmartTags = nullptr != pSmarttags;

        if ( bWrong || bSmartTags || bGrammarCheck )
            rInf.DrawMarkedText( *this, rInf.GetLen(), bWrong, bSmartTags, bGrammarCheck );
        else
            rInf.DrawText( *this, rInf.GetLen() );
    }
}

bool SwTextPortion::GetExpText( const SwTextSizeInfo &, OUString & ) const
{
    return false;
}

// Responsible for the justified paragraph. They calculate the blank
// count and the resulting added space.
TextFrameIndex SwTextPortion::GetSpaceCnt(const SwTextSizeInfo &rInf,
                                          TextFrameIndex& rCharCnt) const
{
    TextFrameIndex nCnt(0);
    TextFrameIndex nPos(0);

    if ( rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetTextFrame()->FindPageFrame()));
        if (pGrid && SwTextGrid::LinesAndChars == pGrid->GetGridType() && pGrid->IsSnapToChars())
            return TextFrameIndex(0);
    }

    if ( InExpGrp() || PortionType::InputField == GetWhichPor() )
    {
        if (OUString ExpOut;
            (!IsBlankPortion()
             || (GetExpText(rInf, ExpOut) && OUStringChar(CH_BLANK) == ExpOut))
            && !InNumberGrp() && !IsCombinedPortion())
        {
            // OnWin() likes to return a blank instead of an empty string from
            // time to time. We cannot use that here at all, however.
            bool bOldOnWin = rInf.OnWin();
            const_cast<SwTextSizeInfo &>(rInf).SetOnWin( false );

            OUString aStr;
            GetExpText( rInf, aStr );
            const_cast<SwTextSizeInfo &>(rInf).SetOnWin( bOldOnWin );

            nCnt = nCnt + lcl_AddSpace( rInf, &aStr, *this );
            nPos = TextFrameIndex(aStr.getLength());
        }
    }
    else if( !IsDropPortion() )
    {
        nCnt = nCnt + lcl_AddSpace( rInf, nullptr, *this );
        nPos = GetLen();
    }
    rCharCnt = rCharCnt + nPos;
    return nCnt;
}

SwTwips SwTextPortion::CalcSpacing( tools::Long nSpaceAdd, const SwTextSizeInfo &rInf ) const
{
    TextFrameIndex nCnt(0);

    if ( rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetTextFrame()->FindPageFrame()));
        if (pGrid && SwTextGrid::LinesAndChars == pGrid->GetGridType() && pGrid->IsSnapToChars())
            return 0;
    }

    if ( InExpGrp() || PortionType::InputField == GetWhichPor() )
    {
        if (OUString ExpOut;
            (!IsBlankPortion()
             || (GetExpText(rInf, ExpOut) && OUStringChar(CH_BLANK) == ExpOut))
            && !InNumberGrp() && !IsCombinedPortion())
        {
            // OnWin() likes to return a blank instead of an empty string from
            // time to time. We cannot use that here at all, however.
            bool bOldOnWin = rInf.OnWin();
            const_cast<SwTextSizeInfo &>(rInf).SetOnWin( false );

            OUString aStr;
            GetExpText( rInf, aStr );
            const_cast<SwTextSizeInfo &>(rInf).SetOnWin( bOldOnWin );
            if( nSpaceAdd > 0 )
                nCnt = nCnt + lcl_AddSpace( rInf, &aStr, *this );
            else
            {
                nSpaceAdd = -nSpaceAdd;
                nCnt = TextFrameIndex(aStr.getLength());
            }
        }
    }
    else if( !IsDropPortion() )
    {
        if( nSpaceAdd > 0 )
            nCnt = nCnt + lcl_AddSpace( rInf, nullptr, *this );
        else
        {
            nSpaceAdd = -nSpaceAdd;
            nCnt = GetLen();
            SwLinePortion* pPor = GetNextPortion();

            // we do not want an extra space in front of margin portions
            if ( nCnt )
            {
                while ( pPor && !pPor->Width() && ! pPor->IsHolePortion() )
                    pPor = pPor->GetNextPortion();

                if ( !pPor || pPor->InFixMargGrp() || pPor->IsHolePortion() )
                    --nCnt;
            }
        }
    }

    return sal_Int32(nCnt) * (nSpaceAdd > LONG_MAX/2 ? LONG_MAX/2 - nSpaceAdd : nSpaceAdd)
             / SPACING_PRECISION_FACTOR;
}

void SwTextPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

SwTextInputFieldPortion::SwTextInputFieldPortion()
{
    SetWhichPor( PortionType::InputField );
}

bool SwTextInputFieldPortion::Format(SwTextFormatInfo &rTextFormatInfo)
{
    return SwTextPortion::Format(rTextFormatInfo);
}

void SwTextInputFieldPortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if ( Width() )
    {
        rInf.DrawViewOpt( *this, PortionType::InputField );
        SwTextSlot aPaintText( &rInf, this, true, true, OUString() );
        SwTextPortion::Paint( rInf );
    }
    else
    {
        // highlight empty input field, elsewhere they are completely invisible for the user
        SwRect aIntersect;
        rInf.CalcRect(*this, &aIntersect);
        const SwTwips aAreaWidth = rInf.GetTextSize(OUString(' ')).Width();
        aIntersect.Left(aIntersect.Left() - aAreaWidth/2);
        aIntersect.Width(aAreaWidth);

        if (aIntersect.HasArea()
            && rInf.OnWin()
            && rInf.GetOpt().IsFieldShadings()
            && !rInf.GetOpt().IsPagePreview())
        {
            OutputDevice* pOut = const_cast<OutputDevice*>(rInf.GetOut());
            pOut->Push(vcl::PushFlags::LINECOLOR | vcl::PushFlags::FILLCOLOR);
            pOut->SetFillColor(rInf.GetOpt().GetFieldShadingsColor());
            pOut->SetLineColor();
            pOut->DrawRect(aIntersect.SVRect());
            pOut->Pop();
        }
    }
}

bool SwTextInputFieldPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    sal_Int32 nIdx(rInf.GetIdx());
    sal_Int32 nLen(GetLen());
    if ( rInf.GetChar( rInf.GetIdx() ) == CH_TXT_ATR_INPUTFIELDSTART )
    {
        ++nIdx;
        --nLen;
    }
    if (rInf.GetChar(rInf.GetIdx() + GetLen() - TextFrameIndex(1)) == CH_TXT_ATR_INPUTFIELDEND)
    {
        --nLen;
    }
    rText = rInf.GetText().copy( nIdx, std::min( nLen, rInf.GetText().getLength() - nIdx ) );

    return true;
}

SwPositiveSize SwTextInputFieldPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwTextSlot aFormatText( &rInf, this, true, false );
    if (rInf.GetLen() == TextFrameIndex(0))
    {
        return SwPositiveSize( 0, 0 );
    }

    return rInf.GetTextSize();
}

SwHolePortion::SwHolePortion(const SwTextPortion& rPor, bool bShowUnderline)
    : m_nBlankWidth( 0 )
    , m_bShowUnderline(bShowUnderline)
{
    SetLen( TextFrameIndex(1) );
    Height( rPor.Height() );
    Width(0);
    SetAscent( rPor.GetAscent() );
    SetWhichPor( PortionType::Hole );
}

SwLinePortion *SwHolePortion::Compress() { return this; }

// The GetTextSize() assumes that the own length is correct
SwPositiveSize SwHolePortion::GetTextSize(const SwTextSizeInfo& rInf) const
{
    SwPositiveSize aSize = rInf.GetTextSize();
    if (!GetJoinBorderWithPrev())
        aSize.Width(aSize.Width() + rInf.GetFont()->GetLeftBorderSpace());
    if (!GetJoinBorderWithNext())
        aSize.Width(aSize.Width() + rInf.GetFont()->GetRightBorderSpace());

    aSize.Height(aSize.Height() +
        rInf.GetFont()->GetTopBorderSpace() +
        rInf.GetFont()->GetBottomBorderSpace());

    return aSize;
}

void SwHolePortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( !rInf.GetOut() )
        return;

    bool bPDFExport = rInf.GetVsh()->GetViewOptions()->IsPDFExport();

    // #i16816# export stuff only needed for tagged pdf support
    if (bPDFExport && !SwTaggedPDFHelper::IsExportTaggedPDF( *rInf.GetOut()) )
        return;

    // #i68503# the hole must have no decoration for a consistent visual appearance
    const SwFont* pOrigFont = rInf.GetFont();
    std::unique_ptr<SwFont> pHoleFont;
    std::optional<SwFontSave> oFontSave;
    if( (!m_bShowUnderline && pOrigFont->GetUnderline() != LINESTYLE_NONE)
    ||  pOrigFont->GetOverline() != LINESTYLE_NONE
    ||  pOrigFont->GetStrikeout() != STRIKEOUT_NONE )
    {
        pHoleFont.reset(new SwFont( *pOrigFont ));
        if (!m_bShowUnderline)
            pHoleFont->SetUnderline(LINESTYLE_NONE);
        pHoleFont->SetOverline( LINESTYLE_NONE );
        pHoleFont->SetStrikeout( STRIKEOUT_NONE );
        oFontSave.emplace( rInf, pHoleFont.get() );
    }

    if (bPDFExport)
    {
        rInf.DrawText(u" "_ustr, *this, TextFrameIndex(0), TextFrameIndex(1));
    }
    else
    {
        // tdf#43244: Paint spaces even at end of line,
        // but only if this paint is not called for pdf export, to keep that pdf export intact
        rInf.DrawText(*this, rInf.GetLen());
    }

    oFontSave.reset();
    pHoleFont.reset();
}

bool SwHolePortion::Format( SwTextFormatInfo &rInf )
{
    return rInf.IsFull() || rInf.X() >= rInf.Width();
}

void SwHolePortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor() );
}

void SwHolePortion::dumpAsXml(xmlTextWriterPtr pWriter, const OUString& rText, TextFrameIndex& nOffset) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwHolePortion"));
    dumpAsXmlAttributes(pWriter, rText, nOffset);
    nOffset += GetLen();

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("blank-width"),
                                      BAD_CAST(OString::number(m_nBlankWidth).getStr()));

    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("show-underline"),
                                      BAD_CAST(OString::boolean(m_bShowUnderline).getStr()));

    (void)xmlTextWriterEndElement(pWriter);
}

void SwFieldMarkPortion::Paint( const SwTextPaintInfo & /*rInf*/) const
{
    // These shouldn't be painted!
    //SwTextPortion::Paint(rInf);
}

bool SwFieldMarkPortion::Format( SwTextFormatInfo & )
{
    Width(0);
    return false;
}

void SwFieldFormCheckboxPortion::Paint( const SwTextPaintInfo& rInf ) const
{
    SwPosition const aPosition(rInf.GetTextFrame()->MapViewToModelPos(rInf.GetIdx()));

    Fieldmark const*const pBM = rInf.GetTextFrame()->GetDoc().getIDocumentMarkAccess()->getFieldmarkAt(aPosition);

    OSL_ENSURE(pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX,
        "Where is my form field bookmark???");

    if (pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX)
    {
        const CheckboxFieldmark* pCheckboxFm = dynamic_cast<CheckboxFieldmark const*>(pBM);
        bool bChecked = pCheckboxFm && pCheckboxFm->IsChecked();
        rInf.DrawCheckBox(*this, bChecked);
    }
}

bool SwFieldFormCheckboxPortion::Format( SwTextFormatInfo & rInf )
{
    SwPosition const aPosition(rInf.GetTextFrame()->MapViewToModelPos(rInf.GetIdx()));
    Fieldmark const*const pBM = rInf.GetTextFrame()->GetDoc().getIDocumentMarkAccess()->getFieldmarkAt(aPosition);
    OSL_ENSURE(pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX, "Where is my form field bookmark???");
    if (pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX)
    {
        // the width of the checkbox portion is the same as its height since it's a square
        // and that size depends on the font size.
        // See:
        // http://document-foundation-mail-archive.969070.n3.nabble.com/Wrong-copy-paste-in-SwFieldFormCheckboxPortion-Format-td4269112.html
        Width( rInf.GetTextHeight(  ) );
        Height( rInf.GetTextHeight(  ) );
        SetAscent( rInf.GetAscent(  ) );
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
