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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <ctype.h>

#ifndef _SHL_HXX
#include <tools/shl.hxx>    // needed for SW_MOD() macro
#endif

#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _GUESS_HXX
#include <guess.hxx>
#endif
#ifndef _INFTXT_HXX
#include <inftxt.hxx>   // SwTxtSizeInfo, SwTxtFormatInfo
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_BREAKTYPE_HPP_
#include <com/sun/star/i18n/BreakType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _PORFLD_HXX
#include <porfld.hxx>
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::beans;

#ifdef VERTICAL_LAYOUT
#define CH_FULL_BLANK 0x3000
#endif

/*************************************************************************
 *						SwTxtGuess::Guess
 *
 * provides information for line break calculation
 * returns true if no line break has to be performed
 * otherwise possible break or hyphenation position is determined
 *************************************************************************/

/*M*/ sal_Bool SwTxtGuess::Guess( const SwTxtPortion& rPor, SwTxtFormatInfo &rInf,
/*M*/                             const KSHORT nPorHeight )
/*M*/ {
/*M*/ 	nCutPos = rInf.GetIdx();
/*M*/ 
/*M*/ 	// Leere Strings sind immer 0
/*M*/ 	if( !rInf.GetLen() || !rInf.GetTxt().Len() )
/*M*/ 		return sal_False;
/*M*/ 
/*M*/     ASSERT( rInf.GetIdx() < rInf.GetTxt().Len(),
/*M*/ 			"+SwTxtGuess::Guess: invalid SwTxtFormatInfo" );
/*M*/ 
/*M*/     ASSERT( nPorHeight, "+SwTxtGuess::Guess: no height" );
/*M*/ 
/*M*/     USHORT nMinSize;
/*M*/     USHORT nMaxSizeDiff;
/*M*/ 
/*M*/     const SwScriptInfo& rSI =
/*M*/             ((SwParaPortion*)rInf.GetParaPortion())->GetScriptInfo();
/*M*/ 
/*M*/     USHORT nMaxComp = ( SW_CJK == rInf.GetFont()->GetActual() ) &&
/*M*/                         rSI.CountCompChg() &&
/*M*/                         ! rInf.IsMulti() &&
/*M*/                         ! rPor.InFldGrp() &&
/*M*/                         ! rPor.IsDropPortion() ?
/*M*/                         10000 :
/*M*/                             0 ;
/*M*/ 
/*M*/     SwTwips nLineWidth = rInf.Width() - rInf.X();
/*M*/ 	const xub_StrLen nMaxLen = Min( xub_StrLen(rInf.GetTxt().Len() - rInf.GetIdx()),
/*M*/ 								rInf.GetLen() );
/*M*/ 	// special case: char width > line width
/*M*/ 	if( !nMaxLen || !nLineWidth )
/*M*/ 		return sal_False;
/*M*/ 
/*M*/ 	KSHORT nItalic = 0;
/*M*/     if( ITALIC_NONE != rInf.GetFont()->GetItalic() && !rInf.NotEOL() )
/*M*/     {
/*M*/         sal_Bool bAddItalic = sal_True;
/*M*/ 
/*M*/         // do not add extra italic value if we have an active character grid
/*M*/         if ( rInf.SnapToGrid() )
/*M*/         {
/*M*/             GETGRID( rInf.GetTxtFrm()->FindPageFrm() )
/*M*/             bAddItalic = ! pGrid || GRID_LINES_CHARS != pGrid->GetGridType();
/*M*/         }
/*M*/ 
/*M*/ #ifdef MAC
/*M*/         nItalic = bAddItalic ? nPorHeight / 4 : 0;
/*M*/ #else
/*M*/         nItalic = bAddItalic ? nPorHeight / 12 : 0;
/*M*/ #endif
/*M*/         if( nItalic >= nLineWidth )
/*M*/         {
/*M*/             nBreakWidth = nItalic;
/*M*/             nCutPos = rInf.GetIdx();
/*M*/             return sal_False;
/*M*/         }
/*M*/         else
/*M*/             nLineWidth -= nItalic;
/*M*/ 	}
/*M*/ 
/*M*/ 	// first check if everything fits to line
/*M*/     if ( long ( nLineWidth ) * 2 > long ( nMaxLen ) * nPorHeight )
/*M*/ 	{
/*M*/         // call GetTxtSize with maximum compression (for kanas)
/*M*/         rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen,
/*M*/                          nMaxComp, nMinSize, nMaxSizeDiff );
/*M*/ 
/*M*/         nBreakWidth = nMinSize;
/*M*/ 
/*M*/ 		if ( nBreakWidth <= nLineWidth )
/*M*/ 		{
/*M*/ 			// portion fits to line
/*M*/ 			nCutPos = rInf.GetIdx() + nMaxLen - 1;
/*M*/ 			if( nItalic && ( nCutPos + 1 ) >= rInf.GetTxt().Len() )
/*M*/ 				nBreakWidth += nItalic;
/*M*/ 
/*M*/             // save maximum width for later use
/*M*/             if ( nMaxSizeDiff )
/*M*/                 rInf.SetMaxWidthDiff( (ULONG)&rPor, nMaxSizeDiff );
/*M*/ 
/*M*/             return sal_True;
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	sal_Bool bHyph = rInf.IsHyphenate() && !rInf.IsHyphForbud();
/*M*/ 	xub_StrLen nHyphPos = 0;
/*M*/ 
/*M*/ 	// nCutPos is the first character not fitting to the current line
/*M*/ 	// nHyphPos is the first character not fitting to the current line,
/*M*/ 	// considering an additional "-" for hyphenation
/*M*/ 	if( bHyph )
/*M*/ 	{
/*M*/         nCutPos = rInf.GetTxtBreak( nLineWidth, nMaxLen, nMaxComp, nHyphPos );
/*M*/ 
/*M*/         if ( !nHyphPos && rInf.GetIdx() )
/*M*/ 			nHyphPos = rInf.GetIdx() - 1;
/*M*/ 	}
/*M*/ 	else
/*M*/     {
/*M*/         nCutPos = rInf.GetTxtBreak( nLineWidth, nMaxLen, nMaxComp );
/*M*/ 
/*M*/ #ifdef DBG_UTIL
/*M*/         if ( STRING_LEN != nCutPos )
/*M*/         {
/*M*/             rInf.GetTxtSize( &rSI, rInf.GetIdx(), nCutPos - rInf.GetIdx(),
/*M*/                              nMaxComp, nMinSize, nMaxSizeDiff );
/*M*/             ASSERT( nMinSize <= nLineWidth, "What a Guess!!!" );
/*M*/         }
/*M*/ #endif
/*M*/     }
/*M*/ 
/*M*/ 	if( nCutPos > rInf.GetIdx() + nMaxLen )
/*M*/ 	{
/*M*/ 		// second check if everything fits to line
/*M*/ 		nCutPos = nBreakPos = rInf.GetIdx() + nMaxLen - 1;
/*M*/         rInf.GetTxtSize( &rSI, rInf.GetIdx(), nMaxLen, nMaxComp,
/*M*/                          nMinSize, nMaxSizeDiff );
/*M*/ 
/*M*/         nBreakWidth = nMinSize;
/*M*/ 
/*M*/ 		// Der folgende Vergleich sollte eigenlich immer sal_True ergeben, sonst
/*M*/ 		// hat es wohl bei GetTxtBreak einen Pixel-Rundungsfehler gegeben...
/*M*/ 		if ( nBreakWidth <= nLineWidth )
/*M*/ 		{
/*M*/ 			if( nItalic && ( nBreakPos + 1 ) >= rInf.GetTxt().Len() )
/*M*/ 				nBreakWidth += nItalic;
/*M*/ 
/*M*/             // save maximum width for later use
/*M*/             if ( nMaxSizeDiff )
/*M*/                 rInf.SetMaxWidthDiff( (ULONG)&rPor, nMaxSizeDiff );
/*M*/ 
/*M*/             return sal_True;
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/     // we have to trigger an underflow for a footnote portion
/*M*/     // which does not fit to the current line
/*M*/     if ( rPor.IsFtnPortion() )
/*M*/     {
/*M*/         nBreakPos = rInf.GetIdx();
/*M*/         nCutPos = rInf.GetLen();
/*M*/         return sal_False;
/*M*/     }
/*M*/ 
/*M*/     xub_StrLen nPorLen = 0;
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/     // do not call the break iterator nCutPos is a blank
/*M*/     xub_Unicode cCutChar = rInf.GetTxt().GetChar( nCutPos );
/*M*/     if( CH_BLANK == cCutChar || CH_FULL_BLANK == cCutChar )
/*M*/ #else
/*M*/     if( CH_BLANK == rInf.GetTxt().GetChar( nCutPos ) )
/*M*/ #endif
/*M*/ 	{
/*M*/ 		nBreakPos = nCutPos;
/*M*/ 		xub_StrLen nX = nBreakPos;
/*M*/ 
/*M*/         // we step back until a non blank character has been found
/*M*/         // or there is only one more character left
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/         while( nX && nBreakPos > rInf.GetLineStart() + 1 &&
/*M*/                ( CH_BLANK == ( cCutChar = rInf.GetChar( --nX ) ) ||
/*M*/                  CH_FULL_BLANK == cCutChar ) )
/*M*/             --nBreakPos;
/*M*/ #else
/*M*/         while( nX && nBreakPos > rInf.GetLineStart() + 1 &&
/*M*/                CH_BLANK == rInf.GetChar( --nX ) )
/*M*/             --nBreakPos;
/*M*/ #endif
/*M*/ 
/*M*/ 		if( nBreakPos > rInf.GetIdx() )
/*M*/ 			nPorLen = nBreakPos - rInf.GetIdx();
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/ 		while( ++nCutPos < rInf.GetTxt().Len() &&
/*M*/                ( CH_BLANK == ( cCutChar = rInf.GetChar( nCutPos ) ) ||
/*M*/                  CH_FULL_BLANK == cCutChar ) )
/*M*/ 			; // nothing
/*M*/ #else
/*M*/ 		while( ++nCutPos < rInf.GetTxt().Len() &&
/*M*/ 			   CH_BLANK == rInf.GetChar( nCutPos ) )
/*M*/ 			; // nothing
/*M*/ #endif
/*M*/ 
/*M*/ 		nBreakStart = nCutPos;
/*M*/ 	}
/*M*/ 	else if( pBreakIt->xBreak.is() )
/*M*/ 	{
/*M*/         // New: We should have a look into the last portion, if it was a
/*M*/         // field portion. For this, we expand the text of the field portion
/*M*/         // into our string. If the line break position is inside of before
/*M*/         // the field portion, we trigger an underflow.
/*M*/ 
/*M*/         xub_StrLen nOldIdx = rInf.GetIdx();
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/         xub_Unicode cFldChr = 0;
/*M*/ #else
/*M*/         sal_Char cFldChr = 0;
/*M*/ #endif
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/         XubString aDebugString;
/*M*/ #endif
/*M*/ 
/*M*/         // be careful: a field portion can be both: 0x01 (common field)
/*M*/         // or 0x02 (the follow of a footnode)
/*M*/         if ( rInf.GetLast() && rInf.GetLast()->InFldGrp() &&
/*M*/              ! rInf.GetLast()->IsFtnPortion() &&
/*M*/              rInf.GetIdx() > rInf.GetLineStart() &&
/*M*/              CH_TXTATR_BREAKWORD ==
/*M*/                 ( cFldChr = rInf.GetTxt().GetChar( rInf.GetIdx() - 1 ) ) )
/*M*/         {
/*M*/             SwFldPortion* pFld = (SwFldPortion*)rInf.GetLast();
/*M*/             XubString aTxt;
/*M*/             pFld->GetExpTxt( rInf, aTxt );
/*M*/ 
/*M*/             if ( aTxt.Len() )
/*M*/             {
/*M*/                 nFieldDiff = aTxt.Len() - 1;
/*M*/                 nCutPos += nFieldDiff;
/*M*/                 nHyphPos += nFieldDiff;
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/                 aDebugString = rInf.GetTxt();
/*M*/ #endif
/*M*/ 
/*M*/                 XubString& rOldTxt = (XubString&)rInf.GetTxt();
/*M*/                 rOldTxt.Erase( rInf.GetIdx() - 1, 1 );
/*M*/                 rOldTxt.Insert( aTxt, rInf.GetIdx() - 1 );
/*M*/                 rInf.SetIdx( rInf.GetIdx() + nFieldDiff );
/*M*/             }
/*M*/             else
/*M*/                 cFldChr = 0;
/*M*/         }
/*M*/ 
/*M*/         LineBreakHyphenationOptions aHyphOpt;
/*M*/ 		Reference< XHyphenator >  xHyph;
/*M*/ 		if( bHyph )
/*M*/ 		{
/*M*/ 			xHyph = ::binfilter::GetHyphenator();
/*M*/ 			aHyphOpt = LineBreakHyphenationOptions( xHyph,
/*M*/ 								rInf.GetHyphValues(), nHyphPos );
/*M*/ 		}
/*M*/ 
/*N*/         // Get Language for break iterator.
/*N*/         // We have to switch the current language if we have a script
/*N*/         // change at nCutPos. Otherwise LATIN punctuation would never
/*N*/         // be allowed to be hanging punctuation.
/*N*/         // NEVER call GetLang if the string has been modified!!!
/*N*/         LanguageType aLang = rInf.GetFont()->GetLanguage();
/*N*/ 
/*N*/         // If we are inside a field portion, we use a temporar string which
/*N*/         // differs from the string at the textnode. Therefore we are not allowed
/*N*/         // to call the GetLang function.
/*N*/         if ( nCutPos && ! rPor.InFldGrp() )
/*N*/         {
/*N*/             const CharClass& rCC = GetAppCharClass();
/*N*/ 
/*N*/             // step back until a non-punctuation character is reached
/*N*/             xub_StrLen nLangIndex = nCutPos;
/*N*/ 
/*N*/             // If a field has been expanded right in front of us we do not
/*N*/             // step further than the beginning of the expanded field
/*N*/             // (which is the position of the field placeholder in our
/*N*/             // original string).
/*N*/             const xub_StrLen nDoNotStepOver = CH_TXTATR_BREAKWORD == cFldChr ?
/*N*/                                               rInf.GetIdx() - nFieldDiff - 1:
/*N*/                                               0;
/*N*/ 
/*N*/             while ( nLangIndex > nDoNotStepOver &&
/*N*/                     ! rCC.isLetterNumeric( rInf.GetTxt(), nLangIndex ) )
/*N*/                 --nLangIndex;
/*N*/ 
/*N*/             // last "real" character is not inside our current portion
/*N*/             // we have to check the script type of the last "real" character
/*N*/             if ( nLangIndex < rInf.GetIdx() )
/*N*/             {
/*N*/                 USHORT nScript = pBreakIt->GetRealScriptOfText( rInf.GetTxt(),
/*N*/                                                                 nLangIndex );
/*N*/                 ASSERT( nScript, "Script is not between 1 and 4" );
/*N*/ 
/*N*/                 // compare current script with script from last "real" character
/*N*/                 if ( nScript - 1 != rInf.GetFont()->GetActual() )
/*N*/                     aLang = rInf.GetTxtFrm()->GetTxtNode()->GetLang(
/*N*/                         CH_TXTATR_BREAKWORD == cFldChr ?
/*N*/                         nDoNotStepOver :
/*N*/                         nLangIndex, 0, nScript );
/*N*/             }
/*N*/         }
/*N*/ 
/*M*/ 		const ForbiddenCharacters aForbidden(
/*M*/ 				*rInf.GetTxtFrm()->GetNode()->GetDoc()->
/*M*/ 							GetForbiddenCharacters( aLang, TRUE ));
/*M*/ 
/*M*/         const sal_Bool bAllowHanging = rInf.IsHanging() && ! rInf.IsMulti() &&
/*M*/                                       ! rPor.InFldGrp();
/*M*/ 
/*M*/ 		LineBreakUserOptions aUserOpt(
/*M*/ 				aForbidden.beginLine, aForbidden.endLine,
/*M*/                 rInf.HasForbiddenChars(), bAllowHanging, sal_False );
/*M*/ 
/*M*/         // !!! We must have a local copy of the locale, because inside
/*M*/         // getLineBreak the LinguEventListener can trigger a new formatting,
/*M*/         // which can corrupt the locale pointer inside pBreakIt.
/*M*/         const ::com::sun::star::lang::Locale aLocale = pBreakIt->GetLocale( aLang );
/*M*/ 
/*M*/         // determines first possible line break from nRightPos to
/*M*/         // start index of current line
/*M*/         LineBreakResults aResult = pBreakIt->xBreak->getLineBreak(
/*M*/             rInf.GetTxt(), nCutPos, aLocale,
/*M*/             rInf.GetLineStart(), aHyphOpt, aUserOpt );
/*M*/ 
/*M*/         nBreakPos = (xub_StrLen)aResult.breakIndex;
/*M*/ 
/*M*/         // if we are formatting multi portions we want to allow line breaks
/*M*/         // at the border between single line and multi line portion
/*M*/         // we have to be carefull with footnote portions, they always come in
/*M*/         // with an index 0
/*M*/         if ( nBreakPos < rInf.GetLineStart() && rInf.IsFirstMulti() &&
/*M*/              ! rInf.IsFtnInside() )
/*M*/             nBreakPos = rInf.GetLineStart();
/*M*/ 
/*M*/         nBreakStart = nBreakPos;
/*M*/ 
/*M*/         bHyph = BreakType::HYPHENATION == aResult.breakType;
/*M*/ 
/*M*/ 		if ( bHyph && nBreakPos != STRING_LEN)
/*M*/ 		{
/*M*/ 			// found hyphenation position within line
/*M*/ 			// nBreakPos is set to the hyphenation position
/*M*/ 			xHyphWord = aResult.rHyphenatedWord;
/*M*/             nBreakPos += xHyphWord->getHyphenationPos() + 1;
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/             // e.g., Schif-fahrt, referes to our string
/*M*/             const String aWord = xHyphWord->getWord();
/*M*/             // e.g., Schiff-fahrt, referes to the word after hyphenation
/*M*/             const String aHyphenatedWord = xHyphWord->getHyphenatedWord();
/*M*/             // e.g., Schif-fahrt: 5, referes to our string
/*M*/             const USHORT nHyphenationPos = xHyphWord->getHyphenationPos();
/*M*/             // e.g., Schiff-fahrt: 6, referes to the word after hyphenation
/*M*/             const USHORT nHyphenPos = xHyphWord->getHyphenPos();
/*M*/ #endif
/*M*/ 
/*M*/             // if not in interactive mode, we have to break behind a soft hyphen
/*M*/             if ( ! rInf.IsInterHyph() && rInf.GetIdx() )
/*M*/             {
/*M*/                 const long nSoftHyphPos =
/*M*/                         xHyphWord->getWord().indexOf( CHAR_SOFTHYPHEN );
/*M*/ 
/*M*/                 if ( nSoftHyphPos >= 0 &&
/*M*/                      nBreakStart + nSoftHyphPos <= nBreakPos &&
/*M*/                      nBreakPos > rInf.GetLineStart() )
/*M*/                     nBreakPos = rInf.GetIdx() - 1;
/*M*/             }
/*M*/ 
/*M*/             if( nBreakPos >= rInf.GetIdx() )
/*M*/ 			{
/*M*/ 				nPorLen = nBreakPos - rInf.GetIdx();
/*M*/ 				if( '-' == rInf.GetTxt().GetChar( nBreakPos - 1 ) )
/*M*/ 					xHyphWord = NULL;
/*M*/ 			}
/*M*/ 		}
/*M*/         else if ( !bHyph && nBreakPos >= rInf.GetLineStart() )
/*M*/ 		{
/*M*/             ASSERT( nBreakPos != STRING_LEN, "we should have found a break pos" );
/*M*/ 
/*M*/ 			// found break position within line
/*M*/ 			xHyphWord = NULL;
/*M*/ 
/*M*/             // check, if break position is soft hyphen and an underflow
/*M*/             // has to be triggered
/*M*/             if( nBreakPos > rInf.GetLineStart() && rInf.GetIdx() &&
/*M*/                 CHAR_SOFTHYPHEN == rInf.GetTxt().GetChar( nBreakPos - 1 ) )
/*M*/ 				nBreakPos = rInf.GetIdx() - 1;
/*M*/ 
/*M*/             // Delete any blanks at the end of a line, but be careful:
/*M*/             // If a field has been expanded, we do not want to delete any
/*M*/             // blanks inside the field portion. This would cause an unwanted
/*M*/             // underflow
/*M*/             xub_StrLen nX = nBreakPos;
/*M*/ #ifdef VERTICAL_LAYOUT
/*M*/             while( nX > rInf.GetLineStart() &&
/*M*/                    ( CH_TXTATR_BREAKWORD != cFldChr || nX > rInf.GetIdx() ) &&
/*M*/                    ( CH_BLANK == rInf.GetChar( --nX ) ||
/*M*/                      CH_FULL_BLANK == rInf.GetChar( nX ) ) )
/*M*/                 nBreakPos = nX;
/*M*/ #else
/*M*/             while( nX > rInf.GetLineStart() &&
/*M*/                    ( CH_TXTATR_BREAKWORD != cFldChr || nX > rInf.GetIdx() ) &&
/*M*/                    CH_BLANK == rInf.GetChar(--nX) )
/*M*/ 				nBreakPos = nX;
/*M*/ #endif
/*M*/ 			if( nBreakPos > rInf.GetIdx() )
/*M*/ 				nPorLen = nBreakPos - rInf.GetIdx();
/*M*/ 		}
/*M*/         else
/*M*/         {
/*M*/ 			// no line break found, setting nBreakPos to STRING_LEN
/*M*/ 			// causes a break cut
/*M*/ 			nBreakPos = STRING_LEN;
/*M*/ 			ASSERT( nCutPos >= rInf.GetIdx(), "Deep cut" );
/*M*/ 			nPorLen = nCutPos - rInf.GetIdx();
/*M*/ 		}
/*M*/ 
/*M*/ 		if( nBreakPos > nCutPos && nBreakPos != STRING_LEN )
/*M*/ 		{
/*M*/             const xub_StrLen nHangingLen = nBreakPos - nCutPos;
/*M*/             SwPosSize aTmpSize = rInf.GetTxtSize( &rSI, nCutPos,
/*M*/                                                   nHangingLen, 0 );
/*M*/ 			ASSERT( !pHanging, "A hanging portion is hanging around" );
/*M*/ 			pHanging = new SwHangingPortion( aTmpSize );
/*M*/             pHanging->SetLen( nHangingLen );
/*M*/             nPorLen = nCutPos - rInf.GetIdx();
/*M*/         }
/*M*/ 
/*M*/         // If we expanded a field, we must repair the original string.
/*M*/         // In case we do not trigger an underflow, we correct the nBreakPos
/*M*/         // value, but we cannot correct the nBreakStart value:
/*M*/         // If we have found a hyphenation position, nBreakStart can lie before
/*M*/         // the field.
/*M*/         if ( CH_TXTATR_BREAKWORD == cFldChr )
/*M*/         {
/*M*/             if ( nBreakPos < rInf.GetIdx() )
/*M*/                 nBreakPos = nOldIdx - 1;
/*M*/             else if ( STRING_LEN != nBreakPos )
/*M*/             {
/*M*/                 ASSERT( nBreakPos >= nFieldDiff, "I've got field trouble!" );
/*M*/                 nBreakPos -= nFieldDiff;
/*M*/             }
/*M*/ 
/*M*/             ASSERT( nCutPos >= rInf.GetIdx() && nCutPos >= nFieldDiff,
/*M*/                     "I've got field trouble, part2!" );
/*M*/             nCutPos -= nFieldDiff;
/*M*/ 
/*M*/             XubString& rOldTxt = (XubString&)rInf.GetTxt();
/*M*/             rOldTxt.Erase( nOldIdx - 1, nFieldDiff + 1 );
/*M*/             rOldTxt.Insert( cFldChr, nOldIdx - 1 );
/*M*/             rInf.SetIdx( nOldIdx );
/*M*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/             ASSERT( aDebugString == rInf.GetTxt(),
/*M*/                     "Somebody, somebody, somebody put something in my string" );
/*M*/ #endif
/*M*/         }
/*M*/     }
/*M*/ 
/*M*/ 	if( nPorLen )
/*M*/     {
/*M*/         rInf.GetTxtSize( &rSI, rInf.GetIdx(), nPorLen,
/*M*/                          nMaxComp, nMinSize, nMaxSizeDiff );
/*M*/ 
/*M*/         // save maximum width for later use
/*M*/         if ( nMaxSizeDiff )
/*M*/             rInf.SetMaxWidthDiff( (ULONG)&rPor, nMaxSizeDiff );
/*M*/ 
/*M*/         nBreakWidth = nItalic + nMinSize;
/*M*/     }
/*M*/ 	else
/*M*/ 		nBreakWidth = 0;
/*M*/ 
/*M*/     if( pHanging )
/*M*/         nBreakPos = nCutPos;
/*M*/ 
/*M*/     return sal_False;
/*M*/ }

/*************************************************************************
 *						SwTxtGuess::AlternativeSpelling
 *************************************************************************/

// returns true if word at position nPos has a diffenrent spelling
// if hyphenated at this position (old german spelling)

/*N*/ sal_Bool SwTxtGuess::AlternativeSpelling( const SwTxtFormatInfo &rInf,
/*N*/ 	const xub_StrLen nPos )
/*N*/ {
/*N*/ 	// get word boundaries
/*N*/ 	xub_StrLen nWordLen;
/*N*/ 
/*N*/ 	Boundary aBound =
/*N*/ 		pBreakIt->xBreak->getWordBoundary( rInf.GetTxt(), nPos,
/*N*/ 		pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
/*N*/ 		WordType::DICTIONARY_WORD, sal_True );
/*N*/ 	nBreakStart = (xub_StrLen)aBound.startPos;
/*N*/ 	nWordLen = aBound.endPos - nBreakStart;
/*N*/ 
/*N*/     // if everything else fails, we want to cut at nPos
/*N*/     nCutPos = nPos;
/*N*/ 
/*N*/ 	XubString aTxt( rInf.GetTxt().Copy( nBreakStart, nWordLen ) );
/*N*/ 
/*N*/ 	// check, if word has alternative spelling
/*N*/ 	Reference< XHyphenator >  xHyph( ::binfilter::GetHyphenator() );
/*N*/ 	ASSERT( xHyph.is(), "Hyphenator is missing");
/*N*/ 	//! subtract 1 since the UNO-interface is 0 based
/*N*/ 	xHyphWord =	xHyph->queryAlternativeSpelling( OUString(aTxt),
/*N*/ 						pBreakIt->GetLocale( rInf.GetFont()->GetLanguage() ),
/*N*/ 						nPos - nBreakStart, rInf.GetHyphValues() );
/*N*/ 	return xHyphWord.is() && xHyphWord->isAlternativeSpelling();
/*N*/ }

}
