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

#include <ctype.h>
#include <editeng/unolingu.hxx>
#include <tools/shl.hxx>    // needed for SW_MOD() macro
#include <dlelstnr.hxx>
#include <swmodule.hxx>
#include <IDocumentSettingAccess.hxx>
#include <guess.hxx>
#include <inftxt.hxx>
#include <pagefrm.hxx>
#include <pagedesc.hxx> // SwPageDesc
#include <tgrditem.hxx>
#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <unotools/charclass.hxx>
#include <porfld.hxx>
#include <paratr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#define CH_FULL_BLANK 0x3000

/*************************************************************************
 *                      SwTxtGuess::Guess
 *
 * provides information for line break calculation
 * returns true if no line break has to be performed
 * otherwise possible break or hyphenation position is determined
 *************************************************************************/

sal_Bool SwTxtGuess::Guess( const SwTxtPortion& rPor, SwTxtFormatInfo &rInf,
                            const KSHORT nPorHeight )
{
    nCutPos = rInf.GetIdx();

    // Empty strings are always 0
    if( !rInf.GetLen() || rInf.GetTxt().isEmpty() )
        return sal_False;

    OSL_ENSURE( rInf.GetIdx() < rInf.GetTxt().getLength(),
            "+SwTxtGuess::Guess: invalid SwTxtFormatInfo" );

    OSL_ENSURE( nPorHeight, "+SwTxtGuess::Guess: no height" );

    sal_uInt16 nMinSize;
    sal_uInt16 nMaxSizeDiff;

    const SwScriptInfo& rSI =
            ((SwParaPortion*)rInf.GetParaPortion())->GetScriptInfo();

    sal_uInt16 nMaxComp = ( SW_CJK == rInf.GetFont()->GetActual() ) &&
                        rSI.CountCompChg() &&
                        ! rInf.IsMulti() &&
                        ! rPor.InFldGrp() &&
                        ! rPor.IsDropPortion() ?
                        10000 :
                            0 ;

    SwTwips nLineWidth = rInf.Width() - rInf.X();
    sal_Int32 nMaxLen = rInf.GetTxt().getLength() - rInf.GetIdx();

    if ( rInf.GetLen() < nMaxLen )
        nMaxLen = rInf.GetLen();

    if( !nMaxLen )
        return sal_False;

    KSHORT nItalic = 0;
    if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
    {
        bool bAddItalic = true;

        // do not add extra italic value if we have an active character grid
        if ( rInf.SnapToGrid() )
        {
            GETGRID( rInf.GetTxtFrm()->FindPageFrm() )
            bAddItalic = !pGrid || GRID_LINES_CHARS != pGrid->GetGridType();
        }

        // do not add extra italic value for an isolated blank:
        if ( 1 == rInf.GetLen() &&
             CH_BLANK == rInf.GetTxt()[ rInf.GetIdx() ] )
            bAddItalic = false;

        nItalic = bAddItalic ? nPorHeight / 12 : 0;

        nLineWidth -= nItalic;

        // #i46524# LineBreak bug with italics
        if ( nLineWidth < 0 ) nLineWidth = 0;
    }

    // Decrease the line width with the right and left border width
    if( rInf.GetFont()->GetRightBorder() )
        nLineWidth -= rInf.GetFont()->GetRightBorder().get().GetScaledWidth();
    if( rInf.GetFont()->GetLeftBorder() )
        nLineWidth -= rInf.GetFont()->GetLeftBorder().get().GetScaledWidth();

    const bool bUnbreakableNumberings = rInf.GetTxtFrm()->GetTxtNode()->
            getIDocumentSettingAccess()->get(IDocumentSettingAccess::UNBREAKABLE_NUMBERINGS);

    // first check if everything fits to line
    if ( ( long ( nLineWidth ) * 2 > long ( nMaxLen ) * nPorHeight ) ||
         ( bUnbreakableNumberings && rPor.IsNumberPortion() ) )
    {
        // call GetTxtSize with maximum compression (for kanas)
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen,
                         nMaxComp, nMinSize, nMaxSizeDiff );

        nBreakWidth = nMinSize;

        if ( ( nBreakWidth <= nLineWidth ) || ( bUnbreakableNumberings && rPor.IsNumberPortion() ) )
        {
            // portion fits to line
            nCutPos = rInf.GetIdx() + nMaxLen;
            if( nItalic &&
                ( nCutPos >= rInf.GetTxt().getLength() ||
                  // #i48035# Needed for CalcFitToContent
                  // if first line ends with a manual line break
                  rInf.GetTxt()[ nCutPos ] == CH_BREAK ) )
                nBreakWidth = nBreakWidth + nItalic;

            // save maximum width for later use
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( (sal_uLong)&rPor, nMaxSizeDiff );

            return sal_True;
        }
    }

    bool bHyph = rInf.IsHyphenate() && !rInf.IsHyphForbud();
    xub_StrLen nHyphPos = 0;

    // nCutPos is the first character not fitting to the current line
    // nHyphPos is the first character not fitting to the current line,
    // considering an additional "-" for hyphenation
    if( bHyph )
    {
        nCutPos = rInf.GetTxtBreak( nLineWidth, nMaxLen, nMaxComp, nHyphPos );

        if ( !nHyphPos && rInf.GetIdx() )
            nHyphPos = rInf.GetIdx() - 1;
    }
    else
    {
        nCutPos = rInf.GetTxtBreak( nLineWidth, nMaxLen, nMaxComp );

#if OSL_DEBUG_LEVEL > 1
        if ( STRING_LEN != nCutPos )
        {
            rInf.GetTxtSize( &rSI, rInf.GetIdx(), nCutPos - rInf.GetIdx(),
                             nMaxComp, nMinSize, nMaxSizeDiff );
            OSL_ENSURE( nMinSize <= nLineWidth, "What a Guess!!!" );
        }
#endif
    }

    if( nCutPos > rInf.GetIdx() + nMaxLen )
    {
        // second check if everything fits to line
        nCutPos = nBreakPos = rInf.GetIdx() + nMaxLen - 1;
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen, nMaxComp,
                         nMinSize, nMaxSizeDiff );

        nBreakWidth = nMinSize;

        // The following comparison should always give sal_True, otherwise
        // there likely has been a pixel rounding error in GetTxtBreak
        if ( nBreakWidth <= nLineWidth )
        {
            if( nItalic && ( nBreakPos + 1 ) >= rInf.GetTxt().getLength() )
                nBreakWidth = nBreakWidth + nItalic;

            // save maximum width for later use
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( (sal_uLong)&rPor, nMaxSizeDiff );

            return sal_True;
        }
    }

    // we have to trigger an underflow for a footnote portion
    // which does not fit to the current line
    if ( rPor.IsFtnPortion() )
    {
        nBreakPos = rInf.GetIdx();
        nCutPos = rInf.GetLen();
        return sal_False;
    }

    xub_StrLen nPorLen = 0;
    // do not call the break iterator nCutPos is a blank
    sal_Unicode cCutChar = rInf.GetTxt()[ nCutPos ];
    if( CH_BLANK == cCutChar || CH_FULL_BLANK == cCutChar )
    {
        nBreakPos = nCutPos;
        xub_StrLen nX = nBreakPos;

        const SvxAdjust& rAdjust = rInf.GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetAdjust().GetAdjust();
        if ( rAdjust == SVX_ADJUST_LEFT )
        {
            // we step back until a non blank character has been found
            // or there is only one more character left
            while( nX && nBreakPos > rInf.GetTxt().getLength() &&
                   ( CH_BLANK == ( cCutChar = rInf.GetChar( --nX ) ) ||
                     CH_FULL_BLANK == cCutChar ) )
                --nBreakPos;
        }
        else // #i20878#
        {
            while( nX && nBreakPos > rInf.GetLineStart() + 1 &&
                   ( CH_BLANK == ( cCutChar = rInf.GetChar( --nX ) ) ||
                     CH_FULL_BLANK == cCutChar ) )
                --nBreakPos;
        }

        if( nBreakPos > rInf.GetIdx() )
            nPorLen = nBreakPos - rInf.GetIdx();
        while( ++nCutPos < rInf.GetTxt().getLength() &&
               ( CH_BLANK == ( cCutChar = rInf.GetChar( nCutPos ) ) ||
                 CH_FULL_BLANK == cCutChar ) )
            ; // nothing

        nBreakStart = nCutPos;
    }
    else if( g_pBreakIt->GetBreakIter().is() )
    {
        // New: We should have a look into the last portion, if it was a
        // field portion. For this, we expand the text of the field portion
        // into our string. If the line break position is inside of before
        // the field portion, we trigger an underflow.

        xub_StrLen nOldIdx = rInf.GetIdx();
        sal_Unicode cFldChr = 0;

#if OSL_DEBUG_LEVEL > 0
        OUString aDebugString;
#endif

        // be careful: a field portion can be both: 0x01 (common field)
        // or 0x02 (the follow of a footnode)
        if ( rInf.GetLast() && rInf.GetLast()->InFldGrp() &&
             ! rInf.GetLast()->IsFtnPortion() &&
             rInf.GetIdx() > rInf.GetLineStart() &&
             CH_TXTATR_BREAKWORD ==
                ( cFldChr = rInf.GetTxt()[ rInf.GetIdx() - 1 ] ) )
        {
            SwFldPortion* pFld = (SwFldPortion*)rInf.GetLast();
            OUString aTxt;
            pFld->GetExpTxt( rInf, aTxt );

            if ( !aTxt.isEmpty() )
            {
                nFieldDiff = aTxt.getLength() - 1;
                nCutPos = nCutPos + nFieldDiff;
                nHyphPos = nHyphPos + nFieldDiff;

#if OSL_DEBUG_LEVEL > 0
                aDebugString = rInf.GetTxt();
#endif

                OUString& rOldTxt = const_cast<OUString&> (rInf.GetTxt());
                rOldTxt = rOldTxt.replaceAt( rInf.GetIdx() - 1, 1, aTxt );
                rInf.SetIdx( rInf.GetIdx() + nFieldDiff );
            }
            else
                cFldChr = 0;
        }

        LineBreakHyphenationOptions aHyphOpt;
        Reference< XHyphenator >  xHyph;
        if( bHyph )
        {
            xHyph = ::GetHyphenator();
            aHyphOpt = LineBreakHyphenationOptions( xHyph,
                                rInf.GetHyphValues(), nHyphPos );
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
        if ( nCutPos && ! rPor.InFldGrp() )
        {
            const CharClass& rCC = GetAppCharClass();

            // step back until a non-punctuation character is reached
            xub_StrLen nLangIndex = nCutPos;

            // If a field has been expanded right in front of us we do not
            // step further than the beginning of the expanded field
            // (which is the position of the field placeholder in our
            // original string).
            const xub_StrLen nDoNotStepOver = CH_TXTATR_BREAKWORD == cFldChr ?
                                              rInf.GetIdx() - nFieldDiff - 1:
                                              0;

            while ( nLangIndex > nDoNotStepOver &&
                    ! rCC.isLetterNumeric( rInf.GetTxt(), nLangIndex ) )
                --nLangIndex;

            // last "real" character is not inside our current portion
            // we have to check the script type of the last "real" character
            if ( nLangIndex < rInf.GetIdx() )
            {
                sal_uInt16 nScript = g_pBreakIt->GetRealScriptOfText( rInf.GetTxt(),
                                                                nLangIndex );
                OSL_ENSURE( nScript, "Script is not between 1 and 4" );

                // compare current script with script from last "real" character
                if ( nScript - 1 != rInf.GetFont()->GetActual() )
                    aLang = rInf.GetTxtFrm()->GetTxtNode()->GetLang(
                        CH_TXTATR_BREAKWORD == cFldChr ?
                        nDoNotStepOver :
                        nLangIndex, 0, nScript );
            }
        }

        const ForbiddenCharacters aForbidden(
                *rInf.GetTxtFrm()->GetNode()->getIDocumentSettingAccess()->getForbiddenCharacters( aLang, true ) );

        const sal_Bool bAllowHanging = rInf.IsHanging() && ! rInf.IsMulti() &&
                                      ! rPor.InFldGrp();

        LineBreakUserOptions aUserOpt(
                aForbidden.beginLine, aForbidden.endLine,
                rInf.HasForbiddenChars(), bAllowHanging, sal_False );

        //! register listener to LinguServiceEvents now in order to get
        //! notified about relevant changes in the future
        SwModule *pModule = SW_MOD();
        if (!pModule->GetLngSvcEvtListener().is())
            pModule->CreateLngSvcEvtListener();

        // !!! We must have a local copy of the locale, because inside
        // getLineBreak the LinguEventListener can trigger a new formatting,
        // which can corrupt the locale pointer inside pBreakIt.
        const lang::Locale aLocale = g_pBreakIt->GetLocale( aLang );

        // determines first possible line break from nRightPos to
        // start index of current line
        LineBreakResults aResult = g_pBreakIt->GetBreakIter()->getLineBreak(
            rInf.GetTxt(), nCutPos, aLocale,
            rInf.GetLineStart(), aHyphOpt, aUserOpt );

        nBreakPos = (xub_StrLen)aResult.breakIndex;

        // if we are formatting multi portions we want to allow line breaks
        // at the border between single line and multi line portion
        // we have to be carefull with footnote portions, they always come in
        // with an index 0
        if ( nBreakPos < rInf.GetLineStart() && rInf.IsFirstMulti() &&
             ! rInf.IsFtnInside() )
            nBreakPos = rInf.GetLineStart();

        nBreakStart = nBreakPos;

        bHyph = BreakType::HYPHENATION == aResult.breakType;

        if ( bHyph && nBreakPos != STRING_LEN)
        {
            // found hyphenation position within line
            // nBreakPos is set to the hyphenation position
            xHyphWord = aResult.rHyphenatedWord;
            nBreakPos += xHyphWord->getHyphenationPos() + 1;

#if OSL_DEBUG_LEVEL > 1
            // e.g., Schif-fahrt, referes to our string
            const String aWord = xHyphWord->getWord();
            // e.g., Schiff-fahrt, referes to the word after hyphenation
            const String aHyphenatedWord = xHyphWord->getHyphenatedWord();
            // e.g., Schif-fahrt: 5, referes to our string
            const sal_uInt16 nHyphenationPos = xHyphWord->getHyphenationPos();
            (void)nHyphenationPos;
            // e.g., Schiff-fahrt: 6, referes to the word after hyphenation
            const sal_uInt16 nHyphenPos = xHyphWord->getHyphenPos();
            (void)nHyphenPos;
#endif

            // if not in interactive mode, we have to break behind a soft hyphen
            if ( ! rInf.IsInterHyph() && rInf.GetIdx() )
            {
                const long nSoftHyphPos =
                        xHyphWord->getWord().indexOf( CHAR_SOFTHYPHEN );

                if ( nSoftHyphPos >= 0 &&
                     nBreakStart + nSoftHyphPos <= nBreakPos &&
                     nBreakPos > rInf.GetLineStart() )
                    nBreakPos = rInf.GetIdx() - 1;
            }

            if( nBreakPos >= rInf.GetIdx() )
            {
                nPorLen = nBreakPos - rInf.GetIdx();
                if( '-' == rInf.GetTxt()[ nBreakPos - 1 ] )
                    xHyphWord = NULL;
            }
        }
        else if ( !bHyph && nBreakPos >= rInf.GetLineStart() )
        {
            OSL_ENSURE( nBreakPos != STRING_LEN, "we should have found a break pos" );

            // found break position within line
            xHyphWord = NULL;

            // check, if break position is soft hyphen and an underflow
            // has to be triggered
            if( nBreakPos > rInf.GetLineStart() && rInf.GetIdx() &&
                CHAR_SOFTHYPHEN == rInf.GetTxt()[ nBreakPos - 1 ] )
                nBreakPos = rInf.GetIdx() - 1;

            const SvxAdjust& rAdjust = rInf.GetTxtFrm()->GetTxtNode()->GetSwAttrSet().GetAdjust().GetAdjust();
            if( rAdjust != SVX_ADJUST_LEFT )
            {
                // Delete any blanks at the end of a line, but be careful:
                // If a field has been expanded, we do not want to delete any
                // blanks inside the field portion. This would cause an unwanted
                // underflow
                xub_StrLen nX = nBreakPos;
                while( nX > rInf.GetLineStart() &&
                       ( CH_TXTATR_BREAKWORD != cFldChr || nX > rInf.GetIdx() ) &&
                       ( CH_BLANK == rInf.GetChar( --nX ) ||
                         CH_FULL_BLANK == rInf.GetChar( nX ) ) )
                    nBreakPos = nX;
            }
            if( nBreakPos > rInf.GetIdx() )
                nPorLen = nBreakPos - rInf.GetIdx();
        }
        else
        {
            // no line break found, setting nBreakPos to STRING_LEN
            // causes a break cut
            nBreakPos = STRING_LEN;
            OSL_ENSURE( nCutPos >= rInf.GetIdx(), "Deep cut" );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        if( nBreakPos > nCutPos && nBreakPos != STRING_LEN )
        {
            const xub_StrLen nHangingLen = nBreakPos - nCutPos;
            SwPosSize aTmpSize = rInf.GetTxtSize( &rSI, nCutPos,
                                                  nHangingLen, 0 );
            OSL_ENSURE( !pHanging, "A hanging portion is hanging around" );
            pHanging = new SwHangingPortion( aTmpSize );
            pHanging->SetLen( nHangingLen );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        // If we expanded a field, we must repair the original string.
        // In case we do not trigger an underflow, we correct the nBreakPos
        // value, but we cannot correct the nBreakStart value:
        // If we have found a hyphenation position, nBreakStart can lie before
        // the field.
        if ( CH_TXTATR_BREAKWORD == cFldChr )
        {
            if ( nBreakPos < rInf.GetIdx() )
                nBreakPos = nOldIdx - 1;
            else if ( STRING_LEN != nBreakPos )
            {
                OSL_ENSURE( nBreakPos >= nFieldDiff, "I've got field trouble!" );
                nBreakPos = nBreakPos - nFieldDiff;
            }

            OSL_ENSURE( nCutPos >= rInf.GetIdx() && nCutPos >= nFieldDiff,
                    "I've got field trouble, part2!" );
            nCutPos = nCutPos - nFieldDiff;

            OUString& rOldTxt = const_cast<OUString&> (rInf.GetTxt());
            OUString aReplacement( cFldChr );
            rOldTxt = rOldTxt.replaceAt( nOldIdx - 1, nFieldDiff + 1, aReplacement);
            rInf.SetIdx( nOldIdx );

#if OSL_DEBUG_LEVEL > 0
            OSL_ENSURE( aDebugString == rInf.GetTxt(),
                    "Somebody, somebody, somebody put something in my string" );
#endif
        }
    }

    if( nPorLen )
    {
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nPorLen,
                         nMaxComp, nMinSize, nMaxSizeDiff );

        // save maximum width for later use
        if ( nMaxSizeDiff )
            rInf.SetMaxWidthDiff( (sal_uLong)&rPor, nMaxSizeDiff );

        nBreakWidth = nItalic + nMinSize;
    }
    else
        nBreakWidth = 0;

    if( pHanging )
        nBreakPos = nCutPos;

    return sal_False;
}

/*************************************************************************
 *                      SwTxtGuess::AlternativeSpelling
 *************************************************************************/

// returns true if word at position nPos has a diffenrent spelling
// if hyphenated at this position (old german spelling)

bool SwTxtGuess::AlternativeSpelling( const SwTxtFormatInfo &rInf,
    const xub_StrLen nPos )
{
    // get word boundaries
    xub_StrLen nWordLen;

    Boundary aBound =
        g_pBreakIt->GetBreakIter()->getWordBoundary( rInf.GetTxt(), nPos,
        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
        WordType::DICTIONARY_WORD, sal_True );
    nBreakStart = (xub_StrLen)aBound.startPos;
    nWordLen = static_cast<xub_StrLen>(aBound.endPos - nBreakStart);

    // if everything else fails, we want to cut at nPos
    nCutPos = nPos;

    XubString aTxt( rInf.GetTxt().copy( nBreakStart, nWordLen ) );

    // check, if word has alternative spelling
    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    OSL_ENSURE( xHyph.is(), "Hyphenator is missing");
    //! subtract 1 since the UNO-interface is 0 based
    xHyphWord = xHyph->queryAlternativeSpelling( OUString(aTxt),
                        g_pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
                        nPos - nBreakStart, rInf.GetHyphValues() );
    return xHyphWord.is() && xHyphWord->isAlternativeSpelling();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
