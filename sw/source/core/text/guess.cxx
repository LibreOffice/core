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

#include <editeng/unolingu.hxx>
#include <dlelstnr.hxx>
#include <swmodule.hxx>
#include <IDocumentSettingAccess.hxx>
#include "guess.hxx"
#include "inftxt.hxx"
#include <pagefrm.hxx>
#include <pagedesc.hxx>
#include <tgrditem.hxx>
#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <unotools/charclass.hxx>
#include "porfld.hxx"
#include <paratr.hxx>
#include <doc.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

namespace{

const sal_Unicode CH_FULL_BLANK = 0x3000;
const sal_Unicode CH_NB_SPACE = 0xA0;

bool IsBlank(sal_Unicode ch) { return ch == CH_BLANK || ch == CH_FULL_BLANK || ch == CH_NB_SPACE; }

}

// provides information for line break calculation
// returns true if no line break has to be performed
// otherwise possible break or hyphenation position is determined
bool SwTextGuess::Guess( const SwTextPortion& rPor, SwTextFormatInfo &rInf,
                            const sal_uInt16 nPorHeight )
{
    nCutPos = rInf.GetIdx();

    // Empty strings are always 0
    if( !rInf.GetLen() || rInf.GetText().isEmpty() )
        return false;

    OSL_ENSURE( rInf.GetIdx() < TextFrameIndex(rInf.GetText().getLength()),
            "+SwTextGuess::Guess: invalid SwTextFormatInfo" );

    OSL_ENSURE( nPorHeight, "+SwTextGuess::Guess: no height" );

    sal_uInt16 nMaxSizeDiff;

    const SwScriptInfo& rSI = rInf.GetParaPortion()->GetScriptInfo();

    sal_uInt16 nMaxComp = ( SwFontScript::CJK == rInf.GetFont()->GetActual() ) &&
                        rSI.CountCompChg() &&
                        ! rInf.IsMulti() &&
                        ! rPor.InFieldGrp() &&
                        ! rPor.IsDropPortion() ?
                        10000 :
                            0 ;

    SwTwips nLineWidth = rInf.GetLineWidth();
    TextFrameIndex nMaxLen = TextFrameIndex(rInf.GetText().getLength()) - rInf.GetIdx();

    const SvxAdjust& rAdjust = rInf.GetTextFrame()->GetTextNodeForParaProps()->GetSwAttrSet().GetAdjust().GetAdjust();

    // tdf#104668 space chars at the end should be cut if the compatibility option is enabled
    // for LTR mode only
    if ( !rInf.GetTextFrame()->IsRightToLeft() )
    {
        if (rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().get(
                    DocumentSettingId::MS_WORD_COMP_TRAILING_BLANKS))
        {
            if ( rAdjust == SvxAdjust::Right || rAdjust == SvxAdjust::Center )
            {
                TextFrameIndex nSpaceCnt(0);
                for (sal_Int32 i = (rInf.GetText().getLength() - 1);
                     sal_Int32(rInf.GetIdx()) <= i; --i)
                {
                    sal_Unicode cChar = rInf.GetText()[i];
                    if ( cChar != CH_BLANK && cChar != CH_FULL_BLANK )
                        break;
                    ++nSpaceCnt;
                }
                TextFrameIndex nCharsCnt = nMaxLen - nSpaceCnt;
                if ( nSpaceCnt && nCharsCnt < rPor.GetLen() )
                {
                    nMaxLen = nCharsCnt;
                    if ( !nMaxLen )
                        return true;
                }
            }
        }
    }

    if ( rInf.GetLen() < nMaxLen )
        nMaxLen = rInf.GetLen();

    if( !nMaxLen )
        return false;

    sal_uInt16 nItalic = 0;
    if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
    {
        bool bAddItalic = true;

        // do not add extra italic value if we have an active character grid
        if ( rInf.SnapToGrid() )
        {
            SwTextGridItem const*const pGrid(
                    GetGridItem(rInf.GetTextFrame()->FindPageFrame()));
            bAddItalic = !pGrid || GRID_LINES_CHARS != pGrid->GetGridType();
        }

        // do not add extra italic value for an isolated blank:
        if (TextFrameIndex(1) == rInf.GetLen() &&
            CH_BLANK == rInf.GetText()[sal_Int32(rInf.GetIdx())])
        {
            bAddItalic = false;
        }

        nItalic = bAddItalic ? nPorHeight / 12 : 0;

        nLineWidth -= nItalic;

        // #i46524# LineBreak bug with italics
        if ( nLineWidth < 0 ) nLineWidth = 0;
    }

    const sal_Int32 nLeftRightBorderSpace =
        (!rPor.GetJoinBorderWithNext() ? rInf.GetFont()->GetRightBorderSpace() : 0) +
        (!rPor.GetJoinBorderWithPrev() ? rInf.GetFont()->GetLeftBorderSpace() : 0);

    nLineWidth -= nLeftRightBorderSpace;

    const bool bUnbreakableNumberings = rInf.GetTextFrame()->GetDoc()
        .getIDocumentSettingAccess().get(DocumentSettingId::UNBREAKABLE_NUMBERINGS);

    // first check if everything fits to line
    if ( ( nLineWidth * 2 > SwTwips(sal_Int32(nMaxLen)) * nPorHeight ) ||
         ( bUnbreakableNumberings && rPor.IsNumberPortion() ) )
    {
        // call GetTextSize with maximum compression (for kanas)
        rInf.GetTextSize( &rSI, rInf.GetIdx(), nMaxLen,
                         nMaxComp, nBreakWidth, nMaxSizeDiff );

        if ( ( nBreakWidth <= nLineWidth ) || ( bUnbreakableNumberings && rPor.IsNumberPortion() ) )
        {
            // portion fits to line
            nCutPos = rInf.GetIdx() + nMaxLen;
            if( nItalic &&
                (nCutPos >= TextFrameIndex(rInf.GetText().getLength()) ||
                  // #i48035# Needed for CalcFitToContent
                  // if first line ends with a manual line break
                  rInf.GetText()[sal_Int32(nCutPos)] == CH_BREAK))
                nBreakWidth = nBreakWidth + nItalic;

            // save maximum width for later use
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( &rPor, nMaxSizeDiff );

            nBreakWidth += nLeftRightBorderSpace;

            return true;
        }
    }

    bool bHyph = rInf.IsHyphenate() && !rInf.IsHyphForbud();
    TextFrameIndex nHyphPos = TextFrameIndex(0);

    // nCutPos is the first character not fitting to the current line
    // nHyphPos is the first character not fitting to the current line,
    // considering an additional "-" for hyphenation
    if( bHyph )
    {
        nCutPos = rInf.GetTextBreak( nLineWidth, nMaxLen, nMaxComp, nHyphPos, rInf.GetCachedVclData().get() );

        if ( !nHyphPos && rInf.GetIdx() )
            nHyphPos = rInf.GetIdx() - TextFrameIndex(1);
    }
    else
    {
        nCutPos = rInf.GetTextBreak( nLineWidth, nMaxLen, nMaxComp, rInf.GetCachedVclData().get() );

#if OSL_DEBUG_LEVEL > 1
        if ( COMPLETE_STRING != nCutPos )
        {
            sal_uInt16 nMinSize;
            rInf.GetTextSize( &rSI, rInf.GetIdx(), nCutPos - rInf.GetIdx(),
                             nMaxComp, nMinSize, nMaxSizeDiff );
            OSL_ENSURE( nMinSize <= nLineWidth, "What a Guess!!!" );
        }
#endif
    }

    if( nCutPos > rInf.GetIdx() + nMaxLen )
    {
        // second check if everything fits to line
        nCutPos = nBreakPos = rInf.GetIdx() + nMaxLen - TextFrameIndex(1);
        rInf.GetTextSize( &rSI, rInf.GetIdx(), nMaxLen, nMaxComp,
                         nBreakWidth, nMaxSizeDiff );

        // The following comparison should always give true, otherwise
        // there likely has been a pixel rounding error in GetTextBreak
        if ( nBreakWidth <= nLineWidth )
        {
            if (nItalic && (nBreakPos + TextFrameIndex(1)) >= TextFrameIndex(rInf.GetText().getLength()))
                nBreakWidth = nBreakWidth + nItalic;

            // save maximum width for later use
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( &rPor, nMaxSizeDiff );

            nBreakWidth += nLeftRightBorderSpace;

            return true;
        }
    }

    // we have to trigger an underflow for a footnote portion
    // which does not fit to the current line
    if ( rPor.IsFootnotePortion() )
    {
        nBreakPos = rInf.GetIdx();
        nCutPos = TextFrameIndex(-1);
        return false;
    }

    TextFrameIndex nPorLen(0);
    // do not call the break iterator nCutPos is a blank
    sal_Unicode cCutChar = nCutPos < TextFrameIndex(rInf.GetText().getLength())
        ? rInf.GetText()[sal_Int32(nCutPos)]
        : 0;
    if (IsBlank(cCutChar))
    {
        nBreakPos = nCutPos;
        TextFrameIndex nX = nBreakPos;

        if ( rAdjust == SvxAdjust::Left )
        {
            // we step back until a non blank character has been found
            // or there is only one more character left
            while (nX && TextFrameIndex(rInf.GetText().getLength()) < nBreakPos &&
                   IsBlank(rInf.GetChar(--nX)))
                --nBreakPos;
        }
        else // #i20878#
        {
            while (nX && nBreakPos > rInf.GetLineStart() + TextFrameIndex(1) &&
                   IsBlank(rInf.GetChar(--nX)))
                --nBreakPos;
        }

        if( nBreakPos > rInf.GetIdx() )
            nPorLen = nBreakPos - rInf.GetIdx();
        while (++nCutPos < TextFrameIndex(rInf.GetText().getLength()) &&
               IsBlank(rInf.GetChar(nCutPos)))
            ; // nothing

        nBreakStart = nCutPos;
    }
    else
    {
        // New: We should have a look into the last portion, if it was a
        // field portion. For this, we expand the text of the field portion
        // into our string. If the line break position is inside of before
        // the field portion, we trigger an underflow.

        TextFrameIndex nOldIdx = rInf.GetIdx();
        sal_Unicode cFieldChr = 0;

#if OSL_DEBUG_LEVEL > 0
        OUString aDebugString;
#endif

        // be careful: a field portion can be both: 0x01 (common field)
        // or 0x02 (the follow of a footnode)
        if ( rInf.GetLast() && rInf.GetLast()->InFieldGrp() &&
             ! rInf.GetLast()->IsFootnotePortion() &&
             rInf.GetIdx() > rInf.GetLineStart() &&
             CH_TXTATR_BREAKWORD ==
                (cFieldChr = rInf.GetText()[sal_Int32(rInf.GetIdx()) - 1]))
        {
            SwFieldPortion* pField = static_cast<SwFieldPortion*>(rInf.GetLast());
            OUString aText;
            pField->GetExpText( rInf, aText );

            if ( !aText.isEmpty() )
            {
                nFieldDiff = TextFrameIndex(aText.getLength() - 1);
                nCutPos = nCutPos + nFieldDiff;
                nHyphPos = nHyphPos + nFieldDiff;

#if OSL_DEBUG_LEVEL > 0
                aDebugString = rInf.GetText();
#endif

                // this is pretty nutso... reverted at the end...
                OUString& rOldText = const_cast<OUString&> (rInf.GetText());
                rOldText = rOldText.replaceAt(sal_Int32(rInf.GetIdx()) - 1, 1, aText);
                rInf.SetIdx( rInf.GetIdx() + nFieldDiff );
            }
            else
                cFieldChr = 0;
        }

        LineBreakHyphenationOptions aHyphOpt;
        Reference< XHyphenator >  xHyph;
        if( bHyph )
        {
            xHyph = ::GetHyphenator();
            aHyphOpt = LineBreakHyphenationOptions( xHyph,
                                rInf.GetHyphValues(), sal_Int32(nHyphPos));
        }

        // Get Language for break iterator.
        // We have to switch the current language if we have a script
        // change at nCutPos. Otherwise LATIN punctuation would never
        // be allowed to be hanging punctuation.
        // NEVER call GetLang if the string has been modified!!!
        LanguageType aLang = rInf.GetFont()->GetLanguage();

        // If we are inside a field portion, we use a temporary string which
        // differs from the string at the textnode. Therefore we are not allowed
        // to call the GetLang function.
        if ( nCutPos && ! rPor.InFieldGrp() )
        {
            const CharClass& rCC = GetAppCharClass();

            // step back until a non-punctuation character is reached
            TextFrameIndex nLangIndex = nCutPos;

            // If a field has been expanded right in front of us we do not
            // step further than the beginning of the expanded field
            // (which is the position of the field placeholder in our
            // original string).
            const TextFrameIndex nDoNotStepOver = CH_TXTATR_BREAKWORD == cFieldChr
                    ? rInf.GetIdx() - nFieldDiff - TextFrameIndex(1)
                    : TextFrameIndex(0);

            if ( nLangIndex > nDoNotStepOver &&
                    TextFrameIndex(rInf.GetText().getLength()) == nLangIndex)
                --nLangIndex;

            while ( nLangIndex > nDoNotStepOver &&
                    !rCC.isLetterNumeric(rInf.GetText(), sal_Int32(nLangIndex)))
                --nLangIndex;

            // last "real" character is not inside our current portion
            // we have to check the script type of the last "real" character
            if ( nLangIndex < rInf.GetIdx() )
            {
                sal_uInt16 nScript = g_pBreakIt->GetRealScriptOfText( rInf.GetText(),
                                        sal_Int32(nLangIndex));
                OSL_ENSURE( nScript, "Script is not between 1 and 4" );

                // compare current script with script from last "real" character
                if ( SwFontScript(nScript - 1) != rInf.GetFont()->GetActual() )
                {
                    aLang = rInf.GetTextFrame()->GetLangOfChar(
                        CH_TXTATR_BREAKWORD == cFieldChr
                            ? nDoNotStepOver
                            : nLangIndex,
                        nScript, true);
                }
            }
        }

        const ForbiddenCharacters aForbidden(
            *rInf.GetTextFrame()->GetDoc().getIDocumentSettingAccess().getForbiddenCharacters(aLang, true));

        const bool bAllowHanging = rInf.IsHanging() && ! rInf.IsMulti() &&
                                      ! rInf.GetTextFrame()->IsInTab() &&
                                      ! rPor.InFieldGrp();

        LineBreakUserOptions aUserOpt(
                aForbidden.beginLine, aForbidden.endLine,
                rInf.HasForbiddenChars(), bAllowHanging, false );

        //! register listener to LinguServiceEvents now in order to get
        //! notified about relevant changes in the future
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        // !!! We must have a local copy of the locale, because inside
        // getLineBreak the LinguEventListener can trigger a new formatting,
        // which can corrupt the locale pointer inside pBreakIt.
        const lang::Locale aLocale = g_pBreakIt->GetLocale( aLang );

        // determines first possible line break from nCutPos to
        // start index of current line
        LineBreakResults aResult = g_pBreakIt->GetBreakIter()->getLineBreak(
            rInf.GetText(), sal_Int32(nCutPos), aLocale,
            sal_Int32(rInf.GetLineStart()), aHyphOpt, aUserOpt );

        nBreakPos = TextFrameIndex(aResult.breakIndex);

        // if we are formatting multi portions we want to allow line breaks
        // at the border between single line and multi line portion
        // we have to be careful with footnote portions, they always come in
        // with an index 0
        if ( nBreakPos < rInf.GetLineStart() && rInf.IsFirstMulti() &&
             ! rInf.IsFootnoteInside() )
            nBreakPos = rInf.GetLineStart();

        nBreakStart = nBreakPos;

        bHyph = BreakType::HYPHENATION == aResult.breakType;

        if (bHyph && nBreakPos != TextFrameIndex(COMPLETE_STRING))
        {
            // found hyphenation position within line
            // nBreakPos is set to the hyphenation position
            xHyphWord = aResult.rHyphenatedWord;
            nBreakPos += TextFrameIndex(xHyphWord->getHyphenationPos() + 1);

            // if not in interactive mode, we have to break behind a soft hyphen
            if ( ! rInf.IsInterHyph() && rInf.GetIdx() )
            {
                sal_Int32 const nSoftHyphPos =
                        xHyphWord->getWord().indexOf( CHAR_SOFTHYPHEN );

                if ( nSoftHyphPos >= 0 &&
                     nBreakStart + TextFrameIndex(nSoftHyphPos) <= nBreakPos &&
                     nBreakPos > rInf.GetLineStart() )
                    nBreakPos = rInf.GetIdx() - TextFrameIndex(1);
            }

            if( nBreakPos >= rInf.GetIdx() )
            {
                nPorLen = nBreakPos - rInf.GetIdx();
                if ('-' == rInf.GetText()[ sal_Int32(nBreakPos) - 1 ])
                    xHyphWord = nullptr;
            }
        }
        else if ( !bHyph && nBreakPos >= rInf.GetLineStart() )
        {
            OSL_ENSURE(sal_Int32(nBreakPos) != COMPLETE_STRING, "we should have found a break pos");

            // found break position within line
            xHyphWord = nullptr;

            // check, if break position is soft hyphen and an underflow
            // has to be triggered
            if( nBreakPos > rInf.GetLineStart() && rInf.GetIdx() &&
                CHAR_SOFTHYPHEN == rInf.GetText()[ sal_Int32(nBreakPos) - 1 ])
            {
                nBreakPos = rInf.GetIdx() - TextFrameIndex(1);
            }

            if( rAdjust != SvxAdjust::Left )
            {
                // Delete any blanks at the end of a line, but be careful:
                // If a field has been expanded, we do not want to delete any
                // blanks inside the field portion. This would cause an unwanted
                // underflow
                TextFrameIndex nX = nBreakPos;
                while( nX > rInf.GetLineStart() &&
                       ( CH_TXTATR_BREAKWORD != cFieldChr || nX > rInf.GetIdx() ) &&
                       ( CH_BLANK == rInf.GetChar( --nX ) ||
                         CH_FULL_BLANK == rInf.GetChar( nX ) ) )
                    nBreakPos = nX;
            }
            if( nBreakPos > rInf.GetIdx() )
                nPorLen = nBreakPos - rInf.GetIdx();
        }
        else
        {
            // no line break found, setting nBreakPos to COMPLETE_STRING
            // causes a break cut
            nBreakPos = TextFrameIndex(COMPLETE_STRING);
            OSL_ENSURE( nCutPos >= rInf.GetIdx(), "Deep cut" );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        if (nBreakPos > nCutPos && nBreakPos != TextFrameIndex(COMPLETE_STRING))
        {
            const TextFrameIndex nHangingLen = nBreakPos - nCutPos;
            SwPosSize aTmpSize = rInf.GetTextSize( &rSI, nCutPos, nHangingLen );
            aTmpSize.Width(aTmpSize.Width() + nLeftRightBorderSpace);
            OSL_ENSURE( !pHanging, "A hanging portion is hanging around" );
            pHanging.reset( new SwHangingPortion( aTmpSize ) );
            pHanging->SetLen( nHangingLen );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        // If we expanded a field, we must repair the original string.
        // In case we do not trigger an underflow, we correct the nBreakPos
        // value, but we cannot correct the nBreakStart value:
        // If we have found a hyphenation position, nBreakStart can lie before
        // the field.
        if ( CH_TXTATR_BREAKWORD == cFieldChr )
        {
            if ( nBreakPos < rInf.GetIdx() )
                nBreakPos = nOldIdx - TextFrameIndex(1);
            else if (TextFrameIndex(COMPLETE_STRING) != nBreakPos)
            {
                OSL_ENSURE( nBreakPos >= nFieldDiff, "I've got field trouble!" );
                nBreakPos = nBreakPos - nFieldDiff;
            }

            OSL_ENSURE( nCutPos >= rInf.GetIdx() && nCutPos >= nFieldDiff,
                    "I've got field trouble, part2!" );
            nCutPos = nCutPos - nFieldDiff;

            OUString& rOldText = const_cast<OUString&> (rInf.GetText());
            OUString aReplacement( cFieldChr );
            rOldText = rOldText.replaceAt(sal_Int32(nOldIdx) - 1, sal_Int32(nFieldDiff) + 1, aReplacement);
            rInf.SetIdx( nOldIdx );

#if OSL_DEBUG_LEVEL > 0
            OSL_ENSURE( aDebugString == rInf.GetText(),
                    "Somebody, somebody, somebody put something in my string" );
#endif
        }
    }

    if( nPorLen )
    {
        rInf.GetTextSize( &rSI, rInf.GetIdx(), nPorLen,
                         nMaxComp, nBreakWidth, nMaxSizeDiff,
                         rInf.GetCachedVclData().get() );

        // save maximum width for later use
        if ( nMaxSizeDiff )
            rInf.SetMaxWidthDiff( &rPor, nMaxSizeDiff );

        nBreakWidth += nItalic + nLeftRightBorderSpace;
    }
    else
        nBreakWidth = 0;

    if( pHanging )
    {
        nBreakPos = nCutPos;
        // Keep following SwBreakPortion in the same line.
        if ( CH_BREAK == rInf.GetChar( nBreakPos + pHanging->GetLen() ) )
            return true;
    }

    return false;
}

// returns true if word at position nPos has a different spelling
// if hyphenated at this position (old german spelling)
bool SwTextGuess::AlternativeSpelling( const SwTextFormatInfo &rInf,
    const TextFrameIndex nPos)
{
    // get word boundaries
    Boundary aBound = g_pBreakIt->GetBreakIter()->getWordBoundary(
        rInf.GetText(), sal_Int32(nPos),
        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
        WordType::DICTIONARY_WORD, true );
    nBreakStart = TextFrameIndex(aBound.startPos);
    sal_Int32 nWordLen = aBound.endPos - sal_Int32(nBreakStart);

    // if everything else fails, we want to cut at nPos
    nCutPos = nPos;

    OUString const aText( rInf.GetText().copy(sal_Int32(nBreakStart), nWordLen) );

    // check, if word has alternative spelling
    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    OSL_ENSURE( xHyph.is(), "Hyphenator is missing");
    //! subtract 1 since the UNO-interface is 0 based
    xHyphWord = xHyph->queryAlternativeSpelling( aText,
                        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
                    sal::static_int_cast<sal_Int16>(sal_Int32(nPos - nBreakStart)),
                    rInf.GetHyphValues() );
    return xHyphWord.is() && xHyphWord->isAlternativeSpelling();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
