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
#include <hintids.hxx>
#include <EnhancedPDFExportHelper.hxx>
#include <SwPortionHandler.hxx>
#include "porlay.hxx"
#include "inftxt.hxx"
#include "guess.hxx"
#include "porglue.hxx"
#include "portab.hxx"
#include "porfld.hxx"
#include <pagefrm.hxx>
#include <tgrditem.hxx>
#include <wrong.hxx>
#include <viewsh.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <viewopt.hxx>
#include <editeng/borderline.hxx>

#include <IMark.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <xmloff/odffields.hxx>

using namespace ::sw::mark;
using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::ScriptType;

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
    // e.g., when rPor is a field portion. nPos referes to the string passed
    // to the function, rInf.GetIdx() referes to the original string.

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
        if ( SwScriptInfo::IsArabicText( *pStr, nPos, nEnd - nPos ) && rInf.GetOut()->GetMinKashida()
            && pSI->CountKashida() )
        {
            const sal_Int32 nKashRes = pSI->KashidaJustify( nullptr, nullptr, nPos, nEnd - nPos );
            // i60591: need to check result of KashidaJustify
            // determine if kashida justification is applicable
            if (nKashRes != -1)
                return TextFrameIndex(nKashRes);
        }
    }

    // Thai Justification: Each character cell gets some extra space
    if ( nEnd > nPos && COMPLEX == nScript )
    {
        LanguageType aLang =
            rInf.GetTextFrame()->GetLangOfChar(rInf.GetIdx(), nScript);

        if ( LANGUAGE_THAI == aLang )
        {
            nCnt = SwScriptInfo::ThaiJustify( *pStr, nullptr, nullptr, nPos, nEnd - nPos );

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
    // nPos referes to the original string, even if a field string has
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
    const sal_uInt16 nLineWidth = static_cast<sal_uInt16>(rInf.Width() - rInf.X());
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
            sal_uInt16 nItalic = 0;
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
    }
}

void SwTextPortion::BreakUnderflow( SwTextFormatInfo &rInf )
{
    Truncate();
    Height( 0 );
    Width( 0 );
    SetLen( TextFrameIndex(0) );
    SetAscent( 0 );
    rInf.SetUnderflow( this );
}

static bool lcl_HasContent( const SwFieldPortion& rField, SwTextFormatInfo const &rInf )
{
    OUString aText;
    return rField.GetExpText( rInf, aText ) && !aText.isEmpty();
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

    SwTextGuess aGuess;
    const bool bFull = !aGuess.Guess( *this, rInf, Height() );

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
        Width( aGuess.BreakWidth() );
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
    else if( bFull && aGuess.GetHangingPortion() )
    {
        Width( aGuess.BreakWidth() );
        SetLen( aGuess.BreakPos() - rInf.GetIdx() );
        aGuess.GetHangingPortion()->SetAscent( GetAscent() );
        Insert( aGuess.ReleaseHangingPortion() );
    }
    // breakPos >= index
    else if (aGuess.BreakPos() >= rInf.GetIdx() && aGuess.BreakPos() != TextFrameIndex(COMPLETE_STRING))
    {
        // case B1
        if( aGuess.HyphWord().is() && aGuess.BreakPos() > rInf.GetLineStart()
            && ( aGuess.BreakPos() > rInf.GetIdx() ||
               ( rInf.GetLast() && ! rInf.GetLast()->IsFlyPortion() ) ) )
        {
            CreateHyphen( rInf, aGuess );
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
                    aGuess.BreakPos() == rInf.GetIdx()  &&
                    CH_BLANK != rInf.GetChar( rInf.GetIdx() ) &&
                    0x3000 != rInf.GetChar( rInf.GetIdx() ) ) )
            BreakUnderflow( rInf );
        // case B2
        else if( rInf.GetIdx() > rInf.GetLineStart() ||
                 aGuess.BreakPos() > rInf.GetIdx() ||
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
            // GetLineWidth() takes care of DocumentSettingId::TAB_OVER_MARGIN.
            if (aGuess.BreakWidth() <= rInf.GetLineWidth())
                Width( aGuess.BreakWidth() );
            else
                // this actually should not happen
                Width( sal_uInt16(rInf.Width() - rInf.X()) );

            SetLen( aGuess.BreakPos() - rInf.GetIdx() );

            OSL_ENSURE( aGuess.BreakStart() >= aGuess.FieldDiff(),
                    "Trouble with expanded field portions during line break" );
            TextFrameIndex const nRealStart = aGuess.BreakStart() - aGuess.FieldDiff();
            if( aGuess.BreakPos() < nRealStart && !InExpGrp() )
            {
                SwHolePortion *pNew = new SwHolePortion( *this );
                pNew->SetLen( nRealStart - aGuess.BreakPos() );
                Insert( pNew );
            }
        }
        else    // case C2, last exit
            BreakCut( rInf, aGuess );
    }
    // breakPos < index or no breakpos at all
    else
    {
        bool bFirstPor = rInf.GetLineStart() == rInf.GetIdx();
        if (aGuess.BreakPos() != TextFrameIndex(COMPLETE_STRING) &&
            aGuess.BreakPos() != rInf.GetLineStart() &&
            ( !bFirstPor || rInf.GetFly() || rInf.GetLast()->IsFlyPortion() ||
              rInf.IsFirstMulti() ) &&
            ( !rInf.GetLast()->IsBlankPortion() ||
              SwBlankPortion::MayUnderflow(rInf, rInf.GetIdx() - TextFrameIndex(1), true)))
        {       // case C1 (former BreakUnderflow())
            BreakUnderflow( rInf );
        }
        else
             // case C2, last exit
            BreakCut( rInf, aGuess );
    }

    return bFull;
}

bool SwTextPortion::Format( SwTextFormatInfo &rInf )
{
    if( rInf.X() > rInf.Width() || (!GetLen() && !InExpGrp()) )
    {
        Height( 0 );
        Width( 0 );
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
    if( !(
        ( !GetNextPortion() || ( GetNextPortion()->IsKernPortion() &&
          !GetNextPortion()->GetNextPortion() ) ) &&
        GetLen() &&
        rInf.GetIdx() < TextFrameIndex(rInf.GetText().getLength()) &&
        TextFrameIndex(1) < rInf.GetIdx() &&
        ' ' == rInf.GetChar(rInf.GetIdx() - TextFrameIndex(1)) &&
        !rInf.GetLast()->IsHolePortion()) )
        return;

    // calculate number of blanks
    TextFrameIndex nX(rInf.GetIdx() - TextFrameIndex(1));
    TextFrameIndex nHoleLen(1);
    while( nX && nHoleLen < GetLen() && CH_BLANK == rInf.GetChar( --nX ) )
        nHoleLen++;

    // First set ourselves and the insert, because there could be
    // a SwLineLayout
    sal_uInt16 nBlankSize;
    if( nHoleLen == GetLen() )
        nBlankSize = Width();
    else
        nBlankSize = sal_Int32(nHoleLen) * rInf.GetTextSize(OUString(' ')).Width();
    Width( Width() - nBlankSize );
    rInf.X( rInf.X() - nBlankSize );
    SetLen( GetLen() - nHoleLen );
    SwLinePortion *pHole = new SwHolePortion( *this );
    static_cast<SwHolePortion *>( pHole )->SetBlankWidth( nBlankSize );
    static_cast<SwHolePortion *>( pHole )->SetLen( nHoleLen );
    Insert( pHole );

}

TextFrameIndex SwTextPortion::GetCursorOfst(const sal_uInt16 nOfst) const
{
    OSL_ENSURE( false, "SwTextPortion::GetCursorOfst: don't use this method!" );
    return SwLinePortion::GetCursorOfst( nOfst );
}

// The GetTextSize() assumes that the own length is correct
SwPosSize SwTextPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwPosSize aSize = rInf.GetTextSize();
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
        if (pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars())
            return TextFrameIndex(0);
    }

    if ( InExpGrp() )
    {
        if( !IsBlankPortion() && !InNumberGrp() && !IsCombinedPortion() )
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

long SwTextPortion::CalcSpacing( long nSpaceAdd, const SwTextSizeInfo &rInf ) const
{
    TextFrameIndex nCnt(0);

    if ( rInf.SnapToGrid() )
    {
        SwTextGridItem const*const pGrid(GetGridItem(rInf.GetTextFrame()->FindPageFrame()));
        if (pGrid && GRID_LINES_CHARS == pGrid->GetGridType() && pGrid->IsSnapToChars())
            return 0;
    }

    if ( InExpGrp() )
    {
        if( !IsBlankPortion() && !InNumberGrp() && !IsCombinedPortion() )
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

    return sal_Int32(nCnt) * nSpaceAdd / SPACING_PRECISION_FACTOR;
}

void SwTextPortion::HandlePortion( SwPortionHandler& rPH ) const
{
    rPH.Text( GetLen(), GetWhichPor(), Height(), Width() );
}

SwTextInputFieldPortion::SwTextInputFieldPortion()
    : SwTextPortion()
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
}

bool SwTextInputFieldPortion::GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const
{
    sal_Int32 nIdx(rInf.GetIdx());
    sal_Int32 nLen(rInf.GetLen());
    if ( rInf.GetChar( rInf.GetIdx() ) == CH_TXT_ATR_INPUTFIELDSTART )
    {
        ++nIdx;
        --nLen;
    }
    if (rInf.GetChar(rInf.GetIdx() + rInf.GetLen() - TextFrameIndex(1)) == CH_TXT_ATR_INPUTFIELDEND)
    {
        --nLen;
    }
    rText = rInf.GetText().copy( nIdx, std::min( nLen, rInf.GetText().getLength() - nIdx ) );

    return true;
}

SwPosSize SwTextInputFieldPortion::GetTextSize( const SwTextSizeInfo &rInf ) const
{
    SwTextSlot aFormatText( &rInf, this, true, false );
    if (rInf.GetLen() == TextFrameIndex(0))
    {
        return SwPosSize( 0, 0 );
    }

    return rInf.GetTextSize();
}

SwHolePortion::SwHolePortion( const SwTextPortion &rPor )
    : nBlankWidth( 0 )
{
    SetLen( TextFrameIndex(1) );
    Height( rPor.Height() );
    SetAscent( rPor.GetAscent() );
    SetWhichPor( PortionType::Hole );
}

SwLinePortion *SwHolePortion::Compress() { return this; }

void SwHolePortion::Paint( const SwTextPaintInfo &rInf ) const
{
    if( !rInf.GetOut() )
        return;

    // #i16816# export stuff only needed for tagged pdf support
    if (!SwTaggedPDFHelper::IsExportTaggedPDF( *rInf.GetOut()) )
        return;

    // #i68503# the hole must have no decoration for a consistent visual appearance
    const SwFont* pOrigFont = rInf.GetFont();
    std::unique_ptr<SwFont> pHoleFont;
    std::unique_ptr<SwFontSave> pFontSave;
    if( pOrigFont->GetUnderline() != LINESTYLE_NONE
    ||  pOrigFont->GetOverline() != LINESTYLE_NONE
    ||  pOrigFont->GetStrikeout() != STRIKEOUT_NONE )
    {
        pHoleFont.reset(new SwFont( *pOrigFont ));
        pHoleFont->SetUnderline( LINESTYLE_NONE );
        pHoleFont->SetOverline( LINESTYLE_NONE );
        pHoleFont->SetStrikeout( STRIKEOUT_NONE );
        pFontSave.reset(new SwFontSave( rInf, pHoleFont.get() ));
    }

    const OUString aText( ' ' );
    rInf.DrawText(aText, *this, TextFrameIndex(0), TextFrameIndex(1));

    pFontSave.reset();
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

    IFieldmark const*const pBM = rInf.GetTextFrame()->GetDoc().getIDocumentMarkAccess()->getFieldmarkFor( aPosition );

    OSL_ENSURE(pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX,
        "Where is my form field bookmark???");

    if (pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX)
    {
        const ICheckboxFieldmark* pCheckboxFm = dynamic_cast<ICheckboxFieldmark const*>(pBM);
        bool bChecked = pCheckboxFm && pCheckboxFm->IsChecked();
        rInf.DrawCheckBox(*this, bChecked);
    }
}

bool SwFieldFormCheckboxPortion::Format( SwTextFormatInfo & rInf )
{
    SwPosition const aPosition(rInf.GetTextFrame()->MapViewToModelPos(rInf.GetIdx()));
    IFieldmark const*const pBM = rInf.GetTextFrame()->GetDoc().getIDocumentMarkAccess()->getFieldmarkFor( aPosition );
    OSL_ENSURE(pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX, "Where is my form field bookmark???");
    if (pBM && pBM->GetFieldname( ) == ODF_FORMCHECKBOX)
    {
        Width( rInf.GetTextHeight(  ) );
        Height( rInf.GetTextHeight(  ) );
        SetAscent( rInf.GetAscent(  ) );
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
