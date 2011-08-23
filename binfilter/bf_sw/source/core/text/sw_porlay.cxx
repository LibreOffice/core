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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "txtcfg.hxx"
#include "itrform2.hxx"
#include "redlnitr.hxx" // SwRedlineItr
#include "porfly.hxx"	// SwFlyCntPortion
#include <porrst.hxx>		// SwHangingPortion
#include <pormulti.hxx> 	// SwMultiPortion
#include <breakit.hxx>
#include <com/sun/star/i18n/ScriptType.hdl>
#include <com/sun/star/i18n/WordType.hdl>

#include <horiornt.hxx>

#include <doc.hxx>
#ifdef BIDI
#include <paratr.hxx>
#include <bf_svx/adjitem.hxx>
#endif
#include <bf_svx/scripttypeitem.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n::ScriptType;

#ifdef BIDI
#include <unicode/ubidi.h>
namespace binfilter {

/*************************************************************************
 *                 lcl_IsLigature
 *
 * Checks if cCh + cNectCh builds a ligature (used for Kashidas)
 *************************************************************************/


/*************************************************************************
 *                 lcl_ConnectToPrev
 *
 * Checks if cCh is connectable to cPrevCh (used for Kashidas)
 *************************************************************************/


#endif


/*************************************************************************
 *				   SwLineLayout::~SwLineLayout()
 *
 * class SwLineLayout: Das Layout einer einzelnen Zeile. Dazu
 * gehoeren vor allen Dingen die Dimension, die Anzahl der
 * Character und der Wortzwischenraeume in der Zeile.
 * Zeilenobjekte werden in einem eigenen Pool verwaltet, um zu
 * erreichen, dass sie im Speicher moeglichst beeinander liegen
 * (d.h. zusammen gepaged werden und den Speicher nicht
 * fragmentieren).
 *************************************************************************/

/*N*/ SwLineLayout::~SwLineLayout()
/*N*/ {
/*N*/ 	Truncate();
/*N*/ 	if( GetNext() )
/*N*/ 		delete GetNext();
/*N*/ 	delete pSpaceAdd;
/*N*/     if ( pKanaComp )
/*?*/         delete pKanaComp;
/*N*/ }

/*************************************************************************
 *				  virtual SwLineLayout::Insert()
 *************************************************************************/

SwLinePortion *SwLineLayout::Insert( SwLinePortion *pIns )
/*N*/ {
/*N*/ 	// Erster Attributwechsel, Masse und Laengen
/*N*/ 	// aus *pCurr in die erste Textportion kopieren.
/*N*/ 	if( !pPortion )
/*N*/ 	{
/*N*/ 		if( GetLen() )
/*N*/ 		{
/*N*/ 			pPortion = new SwTxtPortion( *(SwLinePortion*)this );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SetPortion( pIns );
/*N*/ 			return pIns;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// mit Skope aufrufen, sonst Rekursion !
/*N*/ 	return pPortion->SwLinePortion::Insert( pIns );
/*N*/ }

/*************************************************************************
 *				  virtual SwLineLayout::Append()
 *************************************************************************/


/*************************************************************************
 *				  virtual SwLineLayout::Format()
 *************************************************************************/

// fuer die Sonderbehandlung bei leeren Zeilen

/*N*/ sal_Bool SwLineLayout::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	if( GetLen() )
/*N*/ 		return SwTxtPortion::Format( rInf );
/*N*/ 	else
/*N*/ 	{
/*?*/ 		Height( rInf.GetTxtHeight() );
/*?*/ 		return sal_True;
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *					  SwLineLayout::CalcLeftMargin()
 *
 * Wir sammeln alle FlyPortions am Anfang der Zeile zu einer MarginPortion.
 *************************************************************************/

/*N*/ SwMarginPortion *SwLineLayout::CalcLeftMargin()
/*N*/ {
/*N*/ 	SwMarginPortion *pLeft = (GetPortion() && GetPortion()->IsMarginPortion()) ?
/*N*/ 		(SwMarginPortion *)GetPortion() : 0;
/*N*/ 	if( !GetPortion() )
/*N*/ 		 SetPortion( new SwTxtPortion( *(SwLinePortion*)this ) );
/*N*/ 	if( !pLeft )
/*N*/ 	{
/*N*/ 		pLeft = new SwMarginPortion( 0 );
/*N*/ 		pLeft->SetPortion( GetPortion() );
/*N*/ 		SetPortion( pLeft );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		pLeft->Height( 0 );
/*?*/ 		pLeft->Width( 0 );
/*?*/ 		pLeft->SetLen( 0 );
/*?*/ 		pLeft->SetAscent( 0 );
/*?*/ 		pLeft->SetPortion( NULL );
/*?*/ 		pLeft->SetFixWidth(0);
/*N*/ 	}
/*N*/ 
/*N*/ 	SwLinePortion *pPos = pLeft->GetPortion();
/*N*/ 	while( pPos )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		if( pPos->IsFlyPortion() )
/*N*/ 		{
/*N*/ 			// Die FlyPortion wird ausgesogen ...
/*N*/ 			pLeft->Join( (SwGluePortion*)pPos );
/*N*/ 			pPos = pLeft->GetPortion();
/*N*/ 			if( GetpKanaComp() )
/*N*/ 				GetKanaComp().Remove( 0, 1 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pPos = 0;
/*N*/ 	}
/*N*/ 	return pLeft;
/*N*/ }

/*************************************************************************
 *					  SwLineLayout::CreateSpaceAdd()
 *************************************************************************/

/*N*/ void SwLineLayout::CreateSpaceAdd( const short nInit )
/*N*/ {
/*N*/ 	pSpaceAdd = new SvShorts;
/*N*/ 	pSpaceAdd->Insert( nInit, 0 );
/*N*/ }

/*************************************************************************
 *					  SwLineLayout::CalcLine()
 *
 * Aus FormatLine() ausgelagert.
 *************************************************************************/

/*N*/ void SwLineLayout::CalcLine( SwTxtFormatter &rLine, SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	const KSHORT nLineWidth = rInf.RealWidth();
/*N*/ 
/*N*/ 	KSHORT nFlyAscent;
/*N*/ 	KSHORT nFlyHeight;
/*N*/ 	KSHORT nFlyDescent;
/*N*/ 	sal_Bool bOnlyPostIts = sal_True;
/*N*/ 	SetHanging( sal_False );
/*N*/ 
/*N*/ 	sal_Bool bTmpDummy = ( 0 == GetLen() );
/*N*/ 	SwFlyCntPortion* pFlyCnt = 0;
/*N*/ 	if( bTmpDummy )
/*N*/ 	{
/*N*/ 		nFlyAscent = 0;
/*N*/ 		nFlyHeight = 0;
/*N*/ 		nFlyDescent = 0;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pPortion )
/*N*/ 	{
/*N*/ 		SetCntnt( sal_False );
/*N*/ 		if( pPortion->IsBreakPortion() )
/*N*/ 		{
/*N*/ 			SetLen( pPortion->GetLen() );
/*N*/ 			if( GetLen() )
/*N*/ 				bTmpDummy = sal_False;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			Init( GetPortion() );
/*N*/ 			SwLinePortion *pPos = pPortion;
/*N*/ 			SwLinePortion *pLast = this;
/*N*/ 			KSHORT nMaxDescent = 0;
/*N*/ 
/*N*/ 			//	Eine Gruppe ist ein Abschnitt in der Portion-Kette von
/*N*/ 			//	pCurr oder einer Fix-Portion bis zum Ende bzw. zur naechsten
/*N*/ 			//	Fix-Portion.
/*N*/ 			while( pPos )
/*N*/ 			{
///*N*/ 				DBG_LOOP;
/*N*/ 				ASSERT( POR_LIN != pPos->GetWhichPor(),
/*N*/ 						"SwLineLayout::CalcLine: don't use SwLinePortions !" );
/*N*/ 				// Null-Portions werden eliminiert. Sie koennen entstehen,
/*N*/ 				// wenn zwei FlyFrms ueberlappen.
/*N*/ 				if( !pPos->Compress() )
/*N*/ 				{
/*N*/ 					// 8110: Hoehe und Ascent nur uebernehmen, wenn sonst in der
/*N*/ 					// Zeile nichts mehr los ist.
/*N*/ 					if( !pPos->GetPortion() )
/*N*/ 					{
/*N*/ 						if( !Height() )
/*N*/ 							Height( pPos->Height() );
/*N*/ 						if( !GetAscent() )
/*N*/ 							SetAscent( pPos->GetAscent() );
/*N*/ 					}
/*N*/ 					delete pLast->Cut( pPos );
/*N*/ 					pPos = pLast->GetPortion();
/*N*/ 					continue;
/*N*/ 				}
/*N*/ 
/*N*/ 				// Es gab Attributwechsel: Laengen und Masse aufaddieren;
/*N*/ 				// bzw.Maxima bilden.
/*N*/ 
/*N*/ 				nLineLength += pPos->GetLen();
/*N*/ 
/*N*/                 KSHORT nPosHeight = pPos->Height();
/*N*/ 				KSHORT nPosAscent = pPos->GetAscent();
/*N*/ 				AddPrtWidth( pPos->Width() );
/*N*/ 
/*N*/ 				ASSERT( nPosHeight >= nPosAscent,
/*N*/ 						"SwLineLayout::CalcLine: bad ascent or height" );
/*N*/ 				if( pPos->IsHangingPortion() )
/*N*/ 				{
/*?*/ 					SetHanging( sal_True );
/*?*/ 					rInf.GetParaPortion()->SetMargin( sal_True );
/*N*/ 				}
/*N*/ 
/*N*/ 				// Damit ein Paragraphende-Zeichen nicht durch ein Descent zu einer
/*N*/ 				// geaenderten Zeilenhoehe und zum Umformatieren fuehrt.
/*N*/ 				if ( !pPos->IsBreakPortion() || !Height() )
/*N*/ 				{
/*N*/ 					bOnlyPostIts &= pPos->IsPostItsPortion();
/*N*/ 					if( bTmpDummy && !nLineLength )
/*N*/ 					{
/*N*/ 						if( pPos->IsFlyPortion() )
/*N*/ 						{
/*N*/ 							if( nFlyHeight < nPosHeight )
/*N*/ 								nFlyHeight = nPosHeight;
/*N*/ 							if( nFlyAscent < nPosAscent )
/*N*/ 								nFlyAscent = nPosAscent;
/*N*/ 							if( nFlyDescent < nPosHeight - nPosAscent )
/*N*/ 								nFlyDescent = nPosHeight - nPosAscent;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							if( pPos->InNumberGrp() )
/*N*/ 							{
/*N*/ 								KSHORT nTmp = rInf.GetFont()->GetAscent(
/*N*/ 												rInf.GetVsh(), rInf.GetOut() );
/*N*/ 								if( nTmp > nPosAscent )
/*N*/ 								{
/*N*/ 									nPosHeight += nTmp - nPosAscent;
/*N*/ 									nPosAscent = nTmp;
/*N*/ 								}
/*N*/ 								nTmp = rInf.GetFont()->GetHeight( rInf.GetVsh(),
/*N*/ 																rInf.GetOut() );
/*N*/ 								if( nTmp > nPosHeight )
/*N*/ 									nPosHeight = nTmp;
/*N*/ 							}
/*N*/ 							Height( nPosHeight );
/*N*/ 							nAscent = nPosAscent;
/*N*/ 							nMaxDescent = nPosHeight - nPosAscent;
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else if( !pPos->IsFlyPortion() )
/*N*/ 					{
/*N*/ 						if( Height() < nPosHeight )
/*N*/ 							Height( nPosHeight );
/*N*/ 						if( pPos->IsFlyCntPortion() || ( pPos->IsMultiPortion()
/*N*/ 							&& ((SwMultiPortion*)pPos)->HasFlyInCntnt() ) )
/*N*/ 							rLine.SetFlyInCntBase();
/*N*/ 						if( pPos->IsFlyCntPortion() &&
/*N*/ 							((SwFlyCntPortion*)pPos)->GetAlign() )
/*N*/ 						{
/*N*/ 							((SwFlyCntPortion*)pPos)->SetMax( sal_False );
/*N*/ 							if( !pFlyCnt || pPos->Height() > pFlyCnt->Height() )
/*N*/ 								pFlyCnt = (SwFlyCntPortion*)pPos;
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							if( nAscent < nPosAscent )
/*N*/ 								nAscent = nPosAscent;
/*N*/ 							if( nMaxDescent < nPosHeight - nPosAscent )
/*N*/ 								nMaxDescent = nPosHeight - nPosAscent;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else if( pPos->GetLen() )
/*N*/ 					bTmpDummy = sal_False;
/*N*/ 				if( !HasCntnt() && !pPos->InNumberGrp() )
/*N*/ 				{
/*N*/ 					if ( pPos->InExpGrp() )
/*N*/ 					{
/*N*/ 						XubString aTxt;
/*N*/ 						if( pPos->GetExpTxt( rInf, aTxt ) && aTxt.Len() )
/*N*/ 							SetCntnt( sal_True );
/*N*/ 					}
/*N*/                     else if( ( pPos->InTxtGrp() || pPos->IsMultiPortion() ) &&
/*N*/                              pPos->GetLen() )
/*N*/ 						SetCntnt( sal_True );
/*N*/ 				}
/*N*/ 				bTmpDummy = bTmpDummy && !HasCntnt() &&
/*N*/ 							( !pPos->Width() || pPos->IsFlyPortion() );
/*N*/ 
/*N*/ 				pLast = pPos;
/*N*/ 				pPos = pPos->GetPortion();
/*N*/ 			}
/*N*/ 			if( pFlyCnt )
/*N*/ 			{
/*N*/ 				if( pFlyCnt->Height() == Height() )
/*N*/ 				{
/*N*/ 					pFlyCnt->SetMax( sal_True );
/*N*/ 					if( Height() > nMaxDescent + nAscent )
/*N*/ 					{
/*N*/ 						if( 3 == pFlyCnt->GetAlign() ) // Bottom
/*?*/ 							nAscent = Height() - nMaxDescent;
/*N*/ 						else if( 2 == pFlyCnt->GetAlign() ) // Center
/*N*/ 							nAscent = ( Height() + nAscent - nMaxDescent ) / 2;
/*N*/ 					}
/*N*/ 					pFlyCnt->SetAscent( nAscent );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( bTmpDummy && nFlyHeight )
/*N*/ 			{
/*N*/ 				nAscent = nFlyAscent;
/*N*/ 				if( nFlyDescent > nFlyHeight - nFlyAscent )
/*?*/ 					Height( nFlyHeight + nFlyDescent );
/*N*/ 				else
/*N*/ 					Height( nFlyHeight );
/*N*/ 			}
/*N*/ 			else if( nMaxDescent > Height() - nAscent )
/*N*/ 				Height( nMaxDescent + nAscent );
/*N*/ 			if( bOnlyPostIts )
/*N*/ 			{
/*?*/ 				Height( rInf.GetFont()->GetHeight( rInf.GetVsh(), rInf.GetOut() ) );
/*?*/ 				nAscent = rInf.GetFont()->GetAscent( rInf.GetVsh(), rInf.GetOut() );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SetCntnt( !bTmpDummy );
/*N*/ 	// Robust:
/*N*/ 	if( nLineWidth < Width() )
/*N*/ 		Width( nLineWidth );
/*N*/ 	ASSERT( nLineWidth >= Width(), "SwLineLayout::CalcLine: line is bursting" );
/*N*/ 	SetDummy( bTmpDummy );
/*N*/ 	SetRedline( rLine.GetRedln() &&
/*N*/ 		rLine.GetRedln()->CheckLine( rLine.GetStart(), rLine.GetEnd() ) );
/*N*/ }

/*************************************************************************
 *						class SwCharRange
 *************************************************************************/

/*N*/ SwCharRange &SwCharRange::operator+=(const SwCharRange &rRange)
/*N*/ {
/*N*/ 	if(0 != rRange.nLen ) {
/*N*/ 		if(0 == nLen) {
/*N*/ 			nStart = rRange.nStart;
/*N*/ 			nLen = rRange.nLen ;
/*N*/ 		}
/*N*/ 		else {
/*N*/ 			if(rRange.nStart + rRange.nLen > nStart + nLen) {
/*N*/ 				nLen = rRange.nStart + rRange.nLen - nStart;
/*N*/ 			}
/*N*/ 			if(rRange.nStart < nStart) {
/*N*/ 				nLen += nStart - rRange.nStart;
/*N*/ 				nStart = rRange.nStart;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return *this;
/*N*/ }

/*************************************************************************
 *                      WhichFont()
 *
 * Converts i18n Script Type (LATIN, ASIAN, COMPLEX, WEAK) to
 * Sw Script Types (SW_LATIN, SW_CJK, SW_CTL), used to identify the font
 *************************************************************************/

/*N*/ BYTE WhichFont( xub_StrLen nIdx, const String* pTxt, const SwScriptInfo* pSI )
/*N*/ {
/*N*/     ASSERT( pTxt || pSI,"How should I determine the script type?" );
/*N*/     USHORT nScript;
/*N*/ 
/*N*/     // First we try to use our SwScriptInfo
/*N*/     if ( pSI )
/*N*/         nScript = pSI->ScriptType( nIdx );
/*N*/     else
/*N*/         // Ok, we have to ask the break iterator
/*N*/         nScript = pBreakIt->GetRealScriptOfText( *pTxt, nIdx );
/*N*/ 
/*N*/     switch ( nScript ) {
/*N*/         case i18n::ScriptType::LATIN : return SW_LATIN;
/*?*/         case i18n::ScriptType::ASIAN : return SW_CJK;
/*?*/         case i18n::ScriptType::COMPLEX : return SW_CTL;
/*?*/     }
/*?*/ 
/*?*/     ASSERT( sal_False, "Somebody tells lies about the script type!" );
/*?*/     return SW_LATIN;
/*N*/ }


/*************************************************************************
 *						SwScriptInfo::InitScriptInfo()
 *
 * searches for script changes in rTxt and stores them
 *************************************************************************/

/*N*/ void SwScriptInfo::InitScriptInfo( const SwTxtNode& rNode, sal_Bool bRTL )
/*N*/ {
/*N*/     if( !pBreakIt->xBreak.is() )
/*N*/ 		return;
/*N*/ 
/*N*/     xub_StrLen nChg = nInvalidityPos;
/*N*/ 
/*N*/     // STRING_LEN means the data structure is up to date
/*N*/ 	nInvalidityPos = STRING_LEN;
/*N*/ 
/*N*/     // this is the default direction
/*N*/     nDefaultDir = bRTL ? UBIDI_RTL : UBIDI_LTR;
/*N*/ 
/*N*/     // counter for script info arrays
/*N*/     USHORT nCnt = 0;
/*N*/     // counter for compression information arrays
/*N*/     USHORT nCntComp = 0;
/*N*/ #ifdef BIDI
/*N*/     // counter for kashida array
/*N*/     USHORT nCntKash = 0;
/*N*/ #endif
/*N*/     BYTE nScript;
/*N*/ 
/*N*/     const String& rTxt = rNode.GetTxt();
/*N*/ 
/*N*/     // compression type
/*N*/     const SwCharCompressType aCompEnum = rNode.GetDoc()->GetCharCompressType();
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/     // justification type
/*N*/     const sal_Bool bAdjustBlock = SVX_ADJUST_BLOCK ==
/*N*/                                   rNode.GetSwAttrSet().GetAdjust().GetAdjust();
/*N*/ #endif
/*N*/ 
/*N*/ 
/*N*/     //
/*N*/     // FIND INVALID RANGES IN SCRIPT INFO ARRAYS:
/*N*/     //
/*N*/ 
/*N*/     if( nChg )
/*N*/ 	{
/*N*/         // if change position = 0 we do not use any data from the arrays
/*N*/         // because by deleting all characters of the first group at the beginning
/*N*/         // of a paragraph nScript is set to a wrong value
/*N*/         ASSERT( CountScriptChg(), "Where're my changes of script?" );
/*N*/ 		while( nCnt < CountScriptChg() )
/*N*/ 		{
/*N*/ 			if ( nChg <= GetScriptChg( nCnt ) )
/*N*/ 			{
/*N*/ 				nScript = GetScriptType( nCnt );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nCnt++;
/*N*/ 		}
/*N*/         if( CHARCOMPRESS_NONE != aCompEnum )
/*N*/ 		{
/*N*/             while( nCntComp < CountCompChg() )
/*N*/ 			{
/*N*/             	if ( nChg <= GetCompStart( nCntComp ) )
/*N*/ 					break;
/*N*/ 				else
/*N*/                 	nCntComp++;
/*N*/ 			}
/*N*/ 		}
/*N*/ #ifdef BIDI
/*N*/         if ( bAdjustBlock )
/*N*/         {
/*N*/             while( nCntKash < CountKashida() )
/*N*/             {
/*N*/                 if ( nChg <= GetKashida( nCntKash ) )
/*N*/                     break;
/*N*/                 else
/*N*/                     nCntKash++;
/*N*/             }
/*N*/         }
/*N*/ #endif
/*N*/ 	}
/*N*/ 	else
/*N*/         nScript = (BYTE)pBreakIt->xBreak->getScriptType( rTxt, 0 );
/*N*/ 
/*N*/ 
/*N*/     //
/*N*/     // ADJUST nChg VALUE:
/*N*/     //
/*N*/ 
/*N*/     // by stepping back one position we know that we are inside a group
/*N*/     // declared as an nScript group
/*N*/     if ( nChg )
/*N*/         --nChg;
/*N*/ 
/*N*/     const xub_StrLen nGrpStart = nCnt ? GetScriptChg( nCnt - 1 ) : 0;
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/     // we go back in our group until we reach the first character of
/*N*/     // type nScript
/*N*/     while ( nChg > nGrpStart &&
/*N*/             nScript != pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
/*N*/         --nChg;
/*N*/ #else
/*N*/     // we go back in our group until we reach a non-weak character
/*N*/     while ( nChg > nGrpStart &&
/*N*/             WEAK == pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
/*N*/         --nChg;
/*N*/ #endif
/*N*/ 
/*N*/ 
/*N*/     //
/*N*/     // INVALID DATA FROM THE SCRIPT INFO ARRAYS HAS TO BE DELETED:
/*N*/     //
/*N*/ 
/*N*/     // remove invalid entries from script information arrays
/*N*/     const USHORT nScriptRemove = aScriptChg.Count() - nCnt;
/*N*/     aScriptChg.Remove( nCnt, nScriptRemove );
/*N*/     aScriptType.Remove( nCnt, nScriptRemove );
/*N*/ 
/*N*/     // get the start of the last compression group
/*N*/     USHORT nLastCompression = nChg;
/*N*/ 	if( nCntComp )
/*N*/ 	{
/*N*/ 		--nCntComp;
/*N*/         nLastCompression = GetCompStart( nCntComp );
/*N*/         if( nChg >= nLastCompression + GetCompLen( nCntComp ) )
/*N*/ 		{
/*N*/             nLastCompression = nChg;
/*N*/ 			++nCntComp;
/*N*/ 		}
/*N*/     }
/*N*/ 
/*N*/     // remove invalid entries from compression information arrays
/*N*/     const USHORT nCompRemove = aCompChg.Count() - nCntComp;
/*N*/     aCompChg.Remove( nCntComp, nCompRemove );
/*N*/     aCompLen.Remove( nCntComp, nCompRemove );
/*N*/     aCompType.Remove( nCntComp, nCompRemove );
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/     // get the start of the last kashida group
/*N*/     USHORT nLastKashida = nChg;
/*N*/     if( nCntKash && i18n::ScriptType::COMPLEX == nScript )
/*N*/     {
/*N*/         --nCntKash;
/*N*/         nLastKashida = GetKashida( nCntKash );
/*N*/     }
/*N*/ 
/*N*/     // remove invalid entries from kashida array
/*N*/     aKashida.Remove( nCntKash, aKashida.Count() - nCntKash );
/*N*/ #endif
/*N*/ 
/*N*/ 
/*N*/     //
/*N*/     // TAKE CARE OF WEAK CHARACTERS: WE MUST FIND AN APPROPRIATE
/*N*/     // SCRIPT FOR WEAK CHARACTERS AT THE BEGINNING OF A PARAGRAPH
/*N*/     //
/*N*/ 
/*N*/     if( WEAK == pBreakIt->xBreak->getScriptType( rTxt, nChg ) )
/*N*/ 	{
/*N*/         // If the beginning of the current group is weak, this means that
/*N*/         // all of the characters in this grounp are weak. We have to assign
/*N*/         // the scripts to these characters depending on the fonts which are
/*N*/         // set for these characters to display them.
/*N*/         xub_StrLen nEnd =
/*N*/                 (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, WEAK );
/*N*/ 
/*N*/         if( nEnd > rTxt.Len() )
/*N*/             nEnd = rTxt.Len();
/*N*/ 
/*N*/         nScript = (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
/*N*/ 
/*N*/         ASSERT( i18n::ScriptType::LATIN == nScript ||
/*N*/                 i18n::ScriptType::ASIAN == nScript ||
/*N*/                 i18n::ScriptType::COMPLEX == nScript, "Wrong default language" );
/*N*/ 
/*N*/         nChg = nEnd;
/*N*/         aScriptChg.Insert( nEnd, nCnt );
/*N*/         aScriptType.Insert( nScript, nCnt++ );
/*N*/ 
/*N*/         // Get next script type or set to weak in order to exit
/*N*/         nScript = ( nEnd < rTxt.Len() ) ?
/*N*/                   (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nEnd ) :
/*N*/                   (BYTE)WEAK;
/*N*/     }
/*N*/ 
/*N*/     //
/*N*/     // UPDATE THE SCRIPT INFO ARRAYS:
/*N*/     //
/*N*/ 
/*N*/     // if there are only weak characters in paragraph we are finished
/*N*/     if ( WEAK == nScript )
/*N*/         return;
/*N*/ 
/*N*/     ASSERT( WEAK != (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nChg ),
/*N*/             "Oh my god, it's weak again" );
/*N*/ 
/*N*/     do
/*N*/ 	{
/*N*/         ASSERT( i18n::ScriptType::WEAK != nScript,
/*N*/                 "Inserting WEAK into SwScriptInfo structure" );
/*N*/         ASSERT( STRING_LEN != nChg, "65K? Strange length of script section" );
/*N*/ 
/*N*/         nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
/*N*/ 
/*N*/         if ( nChg > rTxt.Len() )
/*N*/             nChg = rTxt.Len();
/*N*/ 
/*N*/         aScriptChg.Insert( nChg, nCnt );
/*N*/         aScriptType.Insert( nScript, nCnt++ );
/*N*/ 
/*N*/         // if current script is asian, we search for compressable characters
/*N*/         // in this range
/*N*/         if ( CHARCOMPRESS_NONE != aCompEnum &&
/*N*/              i18n::ScriptType::ASIAN == nScript )
/*N*/         {
/*?*/             BYTE ePrevState = NONE;
/*?*/             BYTE eState;
/*?*/             USHORT nPrevChg = nLastCompression;
/*?*/ 
/*?*/             while ( nLastCompression < nChg )
/*?*/             {
/*?*/                 xub_Unicode cChar = rTxt.GetChar( nLastCompression );
/*?*/ 
/*?*/                 // examine current character
/*?*/                 switch ( cChar )
/*?*/                 {
/*?*/                 // Left punctuation found
/*?*/                 case 0x3008: case 0x300A: case 0x300C: case 0x300E:
/*?*/                 case 0x3010: case 0x3014: case 0x3016: case 0x3018:
/*?*/                 case 0x301A: case 0x301D:
/*?*/                     eState = SPECIAL_LEFT;
/*?*/                     break;
/*?*/                 // Right punctuation found
/*?*/                 case 0x3001: case 0x3002: case 0x3009: case 0x300B:
/*?*/                 case 0x300D: case 0x300F: case 0x3011: case 0x3015:
/*?*/                 case 0x3017: case 0x3019: case 0x301B: case 0x301E:
/*?*/                 case 0x301F:
/*?*/                     eState = SPECIAL_RIGHT;
/*?*/                     break;
/*?*/                 default:
/*?*/                     eState = ( 0x3040 <= cChar && 0x3100 > cChar ) ?
/*?*/                                KANA :
/*?*/                                NONE;
/*?*/                 }
/*?*/ 
/*?*/                 // insert range of compressable characters
/*?*/                 if( ePrevState != eState )
/*?*/                 {
/*?*/                     if ( ePrevState != NONE )
/*?*/                     {
/*?*/                         // insert start and type
/*?*/                         if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
/*?*/                              ePrevState != KANA )
/*?*/                         {
/*?*/                             aCompChg.Insert( nPrevChg, nCntComp );
/*?*/                             BYTE nTmpType = ePrevState;
/*?*/                             aCompType.Insert( nTmpType, nCntComp );
/*?*/                             aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
/*?*/                         }
/*?*/                     }
/*?*/ 
/*?*/                     ePrevState = eState;
/*?*/                     nPrevChg = nLastCompression;
/*?*/                 }
/*?*/ 
/*?*/                 nLastCompression++;
/*?*/             }
/*?*/ 
/*?*/             // we still have to examine last entry
/*?*/             if ( ePrevState != NONE )
/*?*/             {
/*?*/                 // insert start and type
/*?*/                 if ( CHARCOMPRESS_PUNCTUATION_KANA == aCompEnum ||
/*?*/                      ePrevState != KANA )
/*?*/                 {
/*?*/                     aCompChg.Insert( nPrevChg, nCntComp );
/*?*/                     BYTE nTmpType = ePrevState;
/*?*/                     aCompType.Insert( nTmpType, nCntComp );
/*?*/                     aCompLen.Insert( nLastCompression - nPrevChg, nCntComp++ );
/*?*/                 }
/*?*/             }
/*N*/         }
/*N*/ #ifdef BIDI
/*N*/         // we search for connecting opportunities (kashida)
/*N*/         else if ( bAdjustBlock && i18n::ScriptType::COMPLEX == nScript )
/*N*/         {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/         }
/*N*/ #endif
/*N*/ 
/*N*/         if ( nChg >= rTxt.Len() )
/*N*/             break;
/*N*/ 
/*N*/         nScript = (BYTE)pBreakIt->xBreak->getScriptType( rTxt, nChg );
/*N*/         nLastCompression = nChg;
/*N*/ #ifdef BIDI
/*N*/         nLastKashida = nChg;
/*N*/ #endif
/*N*/ 
/*N*/     } while ( TRUE );
/*N*/ 
/*N*/ #ifdef DBG_UTIL
/*N*/     // check kashida data
/*N*/     long nTmpKashidaPos = -1;
/*N*/     sal_Bool bWrongKash = sal_False;
/*N*/     for ( USHORT i = 0; i < aKashida.Count(); ++i )
/*N*/     {
/*N*/         long nCurrKashidaPos = GetKashida( i );
/*N*/         if ( nCurrKashidaPos <= nTmpKashidaPos )
/*N*/         {
/*N*/             bWrongKash = sal_True;
/*N*/             break;
/*N*/         }
/*N*/         nTmpKashidaPos = nCurrKashidaPos;
/*N*/     }
/*N*/     ASSERT( ! bWrongKash, "Kashida array contains wrong data" )
/*N*/ #endif
/*N*/ 
/*N*/     // remove invalid entries from direction information arrays
/*N*/     const USHORT nDirRemove = aDirChg.Count();
/*N*/     aDirChg.Remove( 0, nDirRemove );
/*N*/     aDirType.Remove( 0, nDirRemove );
/*N*/ 
/*N*/     // Perform Unicode Bidi Algorithm for text direction information
/*N*/     nCnt = 0;
/*N*/     sal_Bool bLatin = sal_False;
/*N*/     sal_Bool bAsian = sal_False;
/*N*/     sal_Bool bComplex = sal_False;
/*N*/ 
/*N*/     while( nCnt < CountScriptChg() )
/*N*/     {
/*N*/         nScript = GetScriptType( nCnt++ );
/*N*/         switch ( nScript )
/*N*/         {
/*N*/         case i18n::ScriptType::LATIN:
/*N*/             bLatin = sal_True;
/*N*/             break;
/*N*/         case i18n::ScriptType::ASIAN:
/*N*/             bAsian = sal_True;
/*N*/             break;
/*N*/         case i18n::ScriptType::COMPLEX:
/*N*/             bComplex = sal_True;
/*N*/             break;
/*N*/         default:
/*N*/             ASSERT( ! rTxt.Len(), "Wrong script found" )
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     // do not call the unicode bidi algorithm if not required
/*N*/     if ( UBIDI_LTR != nDefaultDir || bComplex )
/*N*/         UpdateBidiInfo( rTxt );
/*N*/ }

/*N*/ void SwScriptInfo::UpdateBidiInfo( const String& rTxt )
/*N*/ {
/*N*/     // remove invalid entries from direction information arrays
/*N*/     const USHORT nDirRemove = aDirChg.Count();
/*N*/     aDirChg.Remove( 0, nDirRemove );
/*N*/     aDirType.Remove( 0, nDirRemove );
/*N*/ 
/*N*/     //
/*N*/     // Bidi functions from icu 2.0
/*N*/     //
/*N*/     UErrorCode nError = U_ZERO_ERROR;
/*N*/     UBiDi* pBidi = ubidi_openSized( rTxt.Len(), 0, &nError );
/*N*/     nError = U_ZERO_ERROR;
/*N*/ 
/*N*/     ubidi_setPara( pBidi, reinterpret_cast<const UChar *>(rTxt.GetBuffer()), rTxt.Len(),	// UChar != sal_Unicode in MinGW
/*N*/                    nDefaultDir, NULL, &nError );
/*N*/     nError = U_ZERO_ERROR;
/*N*/     long nCount = ubidi_countRuns( pBidi, &nError );
/*N*/     int32_t nStart = 0;
/*N*/     int32_t nEnd;
/*N*/     UBiDiLevel nCurrDir;
/*N*/     // counter for direction information arrays
/*N*/     USHORT nCntDir = 0;
/*N*/ 
/*N*/     for ( USHORT nIdx = 0; nIdx < nCount; ++nIdx )
/*N*/     {
/*N*/         ubidi_getLogicalRun( pBidi, nStart, &nEnd, &nCurrDir );
/*N*/         aDirChg.Insert( (USHORT)nEnd, nCntDir );
/*N*/         aDirType.Insert( (BYTE)nCurrDir, nCntDir++ );
/*N*/         nStart = nEnd;
/*N*/     }
/*N*/ 
/*N*/     ubidi_close( pBidi );
/*N*/ }


/*************************************************************************
 *						  SwScriptInfo::NextScriptChg(..)
 * returns the position of the next character which belongs to another script
 * than the character of the actual (input) position.
 * If there's no script change until the end of the paragraph, it will return
 * STRING_LEN.
 * Scripts are Asian (Chinese, Japanese, Korean),
 * 			   Latin ( English etc.)
 *         and Complex ( Hebrew, Arabian )
 *************************************************************************/

/*N*/ xub_StrLen SwScriptInfo::NextScriptChg( const xub_StrLen nPos )  const
/*N*/ {
/*N*/     USHORT nEnd = CountScriptChg();
/*N*/     for( USHORT nX = 0; nX < nEnd; ++nX )
/*N*/     {
/*N*/ 		if( nPos < GetScriptChg( nX ) )
/*N*/ 			return GetScriptChg( nX );
/*N*/     }
/*N*/ 
/*N*/ 	return STRING_LEN;
/*N*/ }

/*************************************************************************
 *						  SwScriptInfo::ScriptType(..)
 * returns the script of the character at the input position
 *************************************************************************/

/*N*/ BYTE SwScriptInfo::ScriptType( const xub_StrLen nPos ) const
/*N*/ {
/*N*/     USHORT nEnd = CountScriptChg();
/*N*/     for( USHORT nX = 0; nX < nEnd; ++nX )
/*N*/     {
/*N*/         if( nPos < GetScriptChg( nX ) )
/*N*/ 			return GetScriptType( nX );
/*N*/     }
/*N*/ 
/*N*/     // the default is the application language script
/*N*/     return (BYTE)GetI18NScriptTypeOfLanguage( (USHORT)GetAppLanguage() );
/*N*/ }

#ifdef BIDI

/*N*/ xub_StrLen SwScriptInfo::NextDirChg( const xub_StrLen nPos,
/*N*/                                      const BYTE* pLevel )  const
/*N*/ {
/*N*/     BYTE nCurrDir = pLevel ? *pLevel : 62;
/*N*/     USHORT nEnd = CountDirChg();
/*N*/     for( USHORT nX = 0; nX < nEnd; ++nX )
/*N*/     {
/*N*/         if( nPos < GetDirChg( nX ) &&
/*N*/             ( nX + 1 == nEnd || GetDirType( nX + 1 ) <= nCurrDir ) )
/*?*/             return GetDirChg( nX );
/*N*/     }
/*N*/ 
/*N*/ 	return STRING_LEN;
/*N*/ }

/*N*/ BYTE SwScriptInfo::DirType( const xub_StrLen nPos ) const
/*N*/ {
/*N*/     USHORT nEnd = CountDirChg();
/*N*/     for( USHORT nX = 0; nX < nEnd; ++nX )
/*N*/     {
/*?*/         if( nPos < GetDirChg( nX ) )
/*?*/             return GetDirType( nX );
/*N*/     }
/*N*/ 
/*N*/     return 0;
/*N*/ }

#endif

/*************************************************************************
 *                        SwScriptInfo::CompType(..)
 * returns the type of the compressed character
 *************************************************************************/


/*************************************************************************
 *                      SwScriptInfo::HasKana()
 * returns, if there are compressable kanas or specials
 * betwenn nStart and nEnd
 *************************************************************************/

/*N*/ USHORT SwScriptInfo::HasKana( xub_StrLen nStart, const xub_StrLen nLen ) const
/*N*/ {
/*N*/     USHORT nCnt = CountCompChg();
/*N*/     xub_StrLen nEnd = nStart + nLen;
/*N*/ 
/*N*/     for( USHORT nX = 0; nX < nCnt; ++nX )
/*N*/     {
/*N*/         xub_StrLen nKanaStart  = GetCompStart( nX );
/*N*/         xub_StrLen nKanaEnd = nKanaStart + GetCompLen( nX );
/*N*/ 
/*N*/         if ( nKanaStart >= nEnd )
/*N*/             return USHRT_MAX;
/*N*/ 
/*N*/         if ( nStart < nKanaEnd )
/*N*/             return nX;
/*N*/     }
/*N*/ 
/*N*/     return USHRT_MAX;
/*N*/ }

/*************************************************************************
 *                      SwScriptInfo::Compress()
 *************************************************************************/

/*N*/ long SwScriptInfo::Compress( sal_Int32* pKernArray, xub_StrLen nIdx, xub_StrLen nLen,
/*N*/                              const USHORT nCompress, const USHORT nFontHeight,
/*N*/                              Point* pPoint ) const
/*N*/ {
/*N*/ 	ASSERT( nCompress, "Compression without compression?!" );
/*N*/ 	ASSERT( nLen, "Compression without text?!" );
/*N*/     USHORT nCompCount = CountCompChg();
/*N*/ 
/*N*/     // In asian typography, there are full width and half width characters.
/*N*/     // Full width punctuation characters can be compressed by 50 %
/*N*/     // to determine this, we compare the font width with 75 % of its height
/*N*/     USHORT nMinWidth = ( 3 * nFontHeight ) / 4;
/*N*/ 
/*N*/     USHORT nCompIdx = HasKana( nIdx, nLen );
/*N*/ 
/*N*/     if ( USHRT_MAX == nCompIdx )
/*N*/         return 0;
/*N*/ 
/*N*/     xub_StrLen nChg = GetCompStart( nCompIdx );
/*N*/     xub_StrLen nCompLen = GetCompLen( nCompIdx );
/*N*/     USHORT nI = 0;
/*N*/     nLen += nIdx;
/*N*/ 
/*N*/     if( nChg > nIdx )
/*N*/     {
/*N*/         nI = nChg - nIdx;
/*N*/         nIdx = nChg;
/*N*/     }
/*N*/     else if( nIdx < nChg + nCompLen )
/*N*/         nCompLen -= nIdx - nChg;
/*N*/ 
/*N*/     if( nIdx > nLen || nCompIdx >= nCompCount )
/*N*/ 		return 0;
/*N*/ 
/*N*/     long nSub = 0;
/*N*/ 	long nLast = nI ? pKernArray[ nI - 1 ] : 0;
/*N*/ 	do
/*N*/ 	{
/*N*/         USHORT nType = GetCompType( nCompIdx );
/*N*/         ASSERT( nType == CompType( nIdx ), "Gimme the right type!" );
/*N*/ 		nCompLen += nIdx;
/*N*/ 		if( nCompLen > nLen )
/*N*/ 			nCompLen = nLen;
/*N*/ 
/*N*/         // are we allowed to compress the character?
/*N*/         if ( pKernArray[ nI ] - nLast < nMinWidth )
/*N*/         {
/*N*/             nIdx++; nI++;
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             while( nIdx < nCompLen )
/*N*/             {
/*N*/                 ASSERT( SwScriptInfo::NONE != nType, "None compression?!" );
/*N*/ 
/*N*/                 // nLast is width of current character
/*N*/                 nLast -= pKernArray[ nI ];
/*N*/ 
/*N*/                 nLast *= nCompress;
/*N*/                 long nMove = 0;
/*N*/                 if( SwScriptInfo::KANA != nType )
/*N*/                 {
/*N*/                     nLast /= 20000;
/*N*/                     if( pPoint && SwScriptInfo::SPECIAL_LEFT == nType )
/*N*/                     {
/*N*/                         if( nI )
/*N*/                             nMove = nLast;
/*N*/                         else
/*N*/                         {
/*N*/                             pPoint->X() += nLast;
/*N*/                             nLast = 0;
/*N*/                         }
/*N*/                     }
/*N*/                 }
/*N*/                 else
/*N*/                     nLast /= 100000;
/*N*/                 nSub -= nLast;
/*N*/                 nLast = pKernArray[ nI ];
/*N*/                 if( nMove )
/*N*/                     pKernArray[ nI - 1 ] += nMove;
/*N*/                 pKernArray[ nI++ ] -= nSub;
/*N*/                 ++nIdx;
/*N*/             }
/*N*/         }
/*N*/ 
/*N*/         if( nIdx < nLen )
/*N*/ 		{
/*N*/ 			xub_StrLen nChg;
/*N*/ 			if( ++nCompIdx < nCompCount )
/*N*/ 			{
/*N*/                 nChg = GetCompStart( nCompIdx );
/*N*/ 				if( nChg > nLen )
/*N*/ 					nChg = nLen;
/*N*/                 nCompLen = GetCompLen( nCompIdx );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nChg = nLen;
/*N*/ 			while( nIdx < nChg )
/*N*/ 			{
/*N*/ 				nLast = pKernArray[ nI ];
/*N*/ 				pKernArray[ nI++ ] -= nSub;
/*N*/ 				++nIdx;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			break;
/*N*/ 	} while( nIdx < nLen );
/*N*/ 	return nSub;
/*N*/ }

/*************************************************************************
 *						class SwParaPortion
 *************************************************************************/

/*N*/ SwParaPortion::SwParaPortion()
/*N*/ {
/*N*/ 	FormatReset();
/*N*/ 	bFlys = bFtnNum = bMargin = sal_False;
/*N*/ 	SetWhichPor( POR_PARA );
/*N*/ }

/*************************************************************************
 *						SwParaPortion::GetParLen()
 *************************************************************************/

/*N*/ xub_StrLen SwParaPortion::GetParLen() const
/*N*/ {
/*N*/ 	xub_StrLen nLen = 0;
/*N*/ 	const SwLineLayout *pLay = this;
/*N*/ 	while( pLay )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		nLen += pLay->GetLen();
/*N*/ 		pLay = pLay->GetNext();
/*N*/ 	}
/*N*/ 	return nLen;
/*N*/ }

/*************************************************************************
 *						SwParaPortion::FindDropPortion()
 *************************************************************************/

/*N*/ const SwDropPortion *SwParaPortion::FindDropPortion() const
/*N*/ {
/*N*/ 	const SwLineLayout *pLay = this;
/*N*/ 	while( pLay && pLay->IsDummy() )
/*N*/ 		pLay = pLay->GetNext();
/*N*/ 	while( pLay )
/*N*/ 	{
/*N*/ 		const SwLinePortion *pPos = pLay->GetPortion();
/*N*/ 		while ( pPos && !pPos->GetLen() )
/*N*/ 			pPos = pPos->GetPortion();
/*N*/ 		if( pPos && pPos->IsDropPortion() )
/*N*/ 			return (SwDropPortion *)pPos;
/*N*/ 		pLay = pLay->GetLen() ? NULL : pLay->GetNext();
/*N*/ 	}
/*N*/ 	return NULL;
/*N*/ }

/*************************************************************************
 *						SwLineLayout::Init()
 *************************************************************************/

/*N*/ void SwLineLayout::Init( SwLinePortion* pNextPortion )
/*N*/ {
/*N*/ 	Height( 0 );
/*N*/ 	Width( 0 );
/*N*/ 	SetLen( 0 );
/*N*/ 	SetAscent( 0 );
/*N*/     SetRealHeight( 0 );
/*N*/ 	SetPortion( pNextPortion );
/*N*/ }

/*-----------------16.11.00 11:04-------------------
 * HangingMargin()
 * looks for hanging punctuation portions in the paragraph
 * and return the maximum right offset of them.
 * If no such portion is found, the Margin/Hanging-flags will be atualized.
 * --------------------------------------------------*/

/*N*/ SwTwips SwLineLayout::_GetHangingMargin() const
/*N*/ {
/*N*/ 	SwLinePortion* pPor = GetPortion();
/*N*/ 	BOOL bFound = sal_False;
/*N*/ 	SwTwips nDiff = 0;
/*N*/ 	while( pPor)
/*N*/ 	{
/*N*/ 		if( pPor->IsHangingPortion() )
/*N*/ 		{
/*?*/ 			nDiff = ((SwHangingPortion*)pPor)->GetInnerWidth() - pPor->Width();
/*?*/ 			if( nDiff )
/*?*/ 				bFound = sal_True;
/*N*/ 		}
/*N*/         // the last post its portion
/*N*/         else if ( pPor->IsPostItsPortion() && ! pPor->GetPortion() )
/*N*/             nDiff = nAscent;
/*N*/ 
/*N*/         pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 	if( !bFound ) // actualize the hanging-flag
/*N*/ 		((SwLineLayout*)this)->SetHanging( sal_False );
/*N*/ 	return nDiff;
/*N*/ }

/*N*/ SwTwips SwTxtFrm::HangingMargin() const
/*N*/ {
/*N*/ 	ASSERT( HasPara(), "Don't call me without a paraportion" );
/*N*/ 	if( !GetPara()->IsMargin() )
/*N*/ 		return 0;
/*?*/ 	const SwLineLayout* pLine = GetPara();
/*?*/ 	SwTwips nRet = 0;
/*?*/ 	do
/*?*/ 	{
/*?*/ 		SwTwips nDiff = pLine->GetHangingMargin();
/*?*/ 		if( nDiff > nRet )
/*?*/ 			nRet = nDiff;
/*?*/ 		pLine = pLine->GetNext();
/*?*/ 	} while ( pLine );
/*?*/ 	if( !nRet ) // actualize the margin-flag
/*?*/ 		((SwParaPortion*)GetPara())->SetMargin( sal_False );
/*?*/ 	return nRet;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
