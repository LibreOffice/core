/*************************************************************************
 *
 *  $RCSfile: guess.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: fme $ $Date: 2001-07-24 07:56:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <ctype.h>

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   // ASSERTs
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _TXTCFG_HXX
#include <txtcfg.hxx>
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>   // SwTxtSizeInfo, SwTxtFormatInfo
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _VIEWSH_HXX
#include <viewsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_BREAKTYPE_HPP_
#include <com/sun/star/i18n/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

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

    // Leere Strings sind immer 0
    if( !rInf.GetLen() || !rInf.GetTxt().Len() )
        return sal_False;

    ASSERT( rInf.GetIdx() < rInf.GetTxt().Len(),
            "+SwTxtGuess::Guess: invalid SwTxtFormatInfo" );

    ASSERT( nPorHeight, "+SwTxtGuess::Guess: no height" );

    USHORT nMinSize;
    USHORT nMaxSizeDiff;

    const SwScriptInfo& rSI =
            ((SwParaPortion*)rInf.GetParaPortion())->GetScriptInfo();

    USHORT nMaxComp = ( SW_CJK == rInf.GetFont()->GetActual() ) &&
                        rSI.CountCompChg() &&
                        ! rInf.IsMulti() &&
                        ! rPor.InFldGrp() &&
                        ! rPor.IsDropPortion() ?
                        10000 :
                            0 ;

    SwTwips nLineWidth = rInf.Width() - rInf.X();
    const xub_StrLen nMaxLen = Min( xub_StrLen(rInf.GetTxt().Len() - rInf.GetIdx()),
                                rInf.GetLen() );
    // special case: char width > line width
    if( !nMaxLen || !nLineWidth )
        return sal_False;

    KSHORT nItalic = 0;
    if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
    {
#ifdef DEBUG
        static MSHORT nDiv = 12;
        nItalic = nPorHeight / nDiv;
#else
#ifdef MAC
        nItalic = nPorHeight / 4;
#else
        nItalic = nPorHeight / 12;
#endif
#endif
        if( nItalic >= nLineWidth )
        {
            nBreakWidth = nItalic;
            nCutPos = rInf.GetIdx();
            return sal_False;
        }
        else
            nLineWidth -= nItalic;
    }

    // first check if everything fits to line
    if ( long ( nLineWidth ) * 2 > long ( nMaxLen ) * nPorHeight )
    {
        // call GetTxtSize with maximum compression (for kanas)
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen,
                         nMaxComp, nMinSize, nMaxSizeDiff );

        nBreakWidth = nMinSize;

        if ( nBreakWidth <= nLineWidth )
        {
            // portion fits to line
            nCutPos = rInf.GetIdx() + nMaxLen - 1;
            if( nItalic && ( nCutPos + 1 ) >= rInf.GetTxt().Len() )
                nBreakWidth += nItalic;

            // save maximum width for later use
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( (ULONG)&rPor, nMaxSizeDiff );

            return sal_True;
        }
    }

    sal_Bool bHyph = rInf.IsHyphenate() && !rInf.IsHyphForbud();
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

#ifndef PRODUCT
        if ( STRING_LEN != nCutPos )
        {
            rInf.GetTxtSize( &rSI, rInf.GetIdx(), nCutPos - rInf.GetIdx(),
                             nMaxComp, nMinSize, nMaxSizeDiff );
            ASSERT( nMinSize <= nLineWidth, "What a Guess!!!" );
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

        // Der folgende Vergleich sollte eigenlich immer sal_True ergeben, sonst
        // hat es wohl bei GetTxtBreak einen Pixel-Rundungsfehler gegeben...
        if ( nBreakWidth <= nLineWidth )
        {
            if( nItalic && ( nBreakPos + 1 ) >= rInf.GetTxt().Len() )
                nBreakWidth += nItalic;

            // save maximum width for later use
            if ( nMaxSizeDiff )
                rInf.SetMaxWidthDiff( (ULONG)&rPor, nMaxSizeDiff );

            return sal_True;
        }
    }

    sal_Bool bChgLocale = sal_False;

    // if the current character does not fit to the current line,
    // we check for possible hanging punctuation:
    if ( nCutPos && nCutPos == rInf.GetIdx() )
    {
        ASSERT( rSI.ScriptType( nCutPos - 1 ), "Script is not between 1 and 4" );

        // compare current script with last script
        bChgLocale = ( rSI.ScriptType( nCutPos - 1 ) - 1 !=
                       rInf.GetFont()->GetActual() );
    }

    xub_StrLen nPorLen = 0;
    if( CH_BLANK == rInf.GetTxt().GetChar( nCutPos ) )
    {
        nBreakPos = nCutPos;
        xub_StrLen nX = nBreakPos;

        // we step back until a non blank character has been found
        // or there is only one more character left
        while( nX && nBreakPos > rInf.GetLineStart() + 1 &&
               CH_BLANK == rInf.GetChar( --nX ) )
            --nBreakPos;

        if( nBreakPos > rInf.GetIdx() )
            nPorLen = nBreakPos - rInf.GetIdx();
        while( ++nCutPos < rInf.GetTxt().Len() &&
               CH_BLANK == rInf.GetChar( nCutPos ) )
            ; // nothing
        nBreakStart = nCutPos;
    }
    else if( pBreakIt->xBreak.is() )
    {
        LineBreakHyphenationOptions aHyphOpt;
        Reference< XHyphenator >  xHyph;
        if( bHyph )
        {
            xHyph = ::GetHyphenator();
            aHyphOpt = LineBreakHyphenationOptions( xHyph,
                                rInf.GetHyphValues(), nHyphPos );
        }

        // We have to switch the current language if we have a script
        // change at nCutPos. Otherwise LATIN punctuation would never
        // be allowed to be hanging punctuation.
        LanguageType aLang = bChgLocale ?
                             rInf.GetTxtFrm()->GetTxtNode()->GetLang( nCutPos - 1 ) :
                             rInf.GetFont()->GetLanguage();
        const ForbiddenCharacters aForbidden(
                *rInf.GetTxtFrm()->GetNode()->GetDoc()->
                            GetForbiddenCharacters( aLang, TRUE ));
        LineBreakUserOptions aUserOpt(
                aForbidden.beginLine, aForbidden.endLine,
                rInf.HasForbiddenChars(), rInf.IsHanging() && !rInf.IsMulti(),
                sal_False );
        // determines first possible line break from nRightPos to
        // start index of current line
        LineBreakResults aResult = pBreakIt->xBreak->getLineBreak(
                rInf.GetTxt(), nCutPos, pBreakIt->GetLocale(aLang),
                rInf.GetLineStart(), aHyphOpt, aUserOpt );
        nBreakPos = (xub_StrLen)aResult.breakIndex;

        // if we are formatting multi portions we want to allow line breaks
        // at the border between single line and multi line portion
        // we have to be carefull with footnote portions, they always come in
        // with an index 0
        if ( nBreakPos < rInf.GetLineStart() &&
             rInf.IsFirstMulti() &&
             ! rInf.IsFtnInside() )
            nBreakPos = rInf.GetLineStart();

        nBreakStart = nBreakPos;

        bHyph = BreakType::HYPHENATION == aResult.breakType;

        if ( bHyph && nBreakPos != STRING_LEN)
        {
            // found hyphenation position within line
            // nBreakPos is set to the hyphenation position
            xHyphWord = aResult.rHyphenatedWord;
            nBreakPos += xHyphWord->getHyphenPos() + 1;
            if( nBreakPos >= rInf.GetIdx() )
            {
                nPorLen = nBreakPos - rInf.GetIdx();
                if( '-' == rInf.GetTxt().GetChar( nBreakPos - 1 ) )
                    xHyphWord = NULL;
            }
        }
        else if ( !bHyph && nBreakPos >= rInf.GetLineStart() )
        {
            // found break position within line
            xHyphWord = NULL;
            // check, if break position is soft hyphen
            if( nBreakPos > rInf.GetLineStart() &&
                CHAR_SOFTHYPHEN == rInf.GetTxt().GetChar( nBreakPos - 1 ) )
            {
                // soft hyphen found, we make sure, that an underflow is
                // triggered by setting nBreakPos to index - 1
                nBreakPos = rInf.GetIdx() - 1;
            }
            xub_StrLen nX = nBreakPos;
            while( nX > rInf.GetLineStart() && CH_BLANK == rInf.GetChar(--nX) )
                nBreakPos = nX;
            if( nBreakPos > rInf.GetIdx() )
                nPorLen = nBreakPos - rInf.GetIdx();
        }
        else
        {
            // no line break found, setting nBreakPos to STRING_LEN
            // causes a break cut
            nBreakPos = STRING_LEN;
            ASSERT( nCutPos >= rInf.GetIdx(), "Deep cut" );
            nPorLen = nCutPos - rInf.GetIdx();
        }

        if( nBreakPos > nCutPos && nBreakPos != STRING_LEN )
        {
            SwPosSize aTmpSize = rInf.GetTxtSize( &rSI, nCutPos,
                                                  nBreakPos - nCutPos, 0 );
            ASSERT( !pHanging, "A hanging portion is hanging around" );
            pHanging = new SwHangingPortion( aTmpSize );
            nPorLen = nCutPos - rInf.GetIdx();
        }
    }

    if( nPorLen )
    {
        rInf.GetTxtSize( &rSI, rInf.GetIdx(), nPorLen,
                         nMaxComp, nMinSize, nMaxSizeDiff );

        // save maximum width for later use
        if ( nMaxSizeDiff )
            rInf.SetMaxWidthDiff( (ULONG)&rPor, nMaxSizeDiff );

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

sal_Bool SwTxtGuess::AlternativeSpelling( const SwTxtFormatInfo &rInf,
    const xub_StrLen nPos )
{
    // get word boundaries
    xub_StrLen nWordLen;

    Boundary aBound =
        pBreakIt->xBreak->getWordBoundary( rInf.GetTxt(), nPos,
        pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
        WordType::DICTIONARY_WORD, sal_True );
    nBreakStart = (xub_StrLen)aBound.startPos;
    nWordLen = aBound.endPos - nBreakStart;

    // if everything else fails, we want to cut at nPos
    nCutPos = nPos;

    XubString aTxt( rInf.GetTxt().Copy( nBreakStart, nWordLen ) );

    // check, if word has alternative spelling
    Reference< XHyphenator >  xHyph( ::GetHyphenator() );
    ASSERT( xHyph.is(), "Hyphenator is missing");
    //! subtract 1 since the UNO-interface is 0 based
    xHyphWord = xHyph->queryAlternativeSpelling( OUString(aTxt),
                        pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
                        nPos - nBreakStart, rInf.GetHyphValues() );
    return xHyphWord.is() && xHyphWord->isAlternativeSpelling();
}

