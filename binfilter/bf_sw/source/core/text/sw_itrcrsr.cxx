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

#include "errhdl.hxx"
#include "paratr.hxx"

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif


#ifndef _SVX_ADJITEM_HXX //autogen
#include <bf_svx/adjitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <bf_svx/lspcitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif

#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif

#ifdef VERTICAL_LAYOUT
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif
#endif

#include "txtcfg.hxx"
#include "itrtxt.hxx"

#include "porfld.hxx"		// SwFldPortion::IsFollow()
#include "porfly.hxx"		// GetFlyCrsrOfst()
#include "pordrop.hxx"
#include "crstate.hxx"      // SwCrsrMoveState
#ifndef _PORMULTI_HXX
#include <pormulti.hxx> 	// SwMultiPortion
#endif
namespace binfilter {

extern BYTE WhichFont( xub_StrLen nIdx, const String* pTxt,
                       const SwScriptInfo* pSI );

// Nicht reentrant !!!
// wird in GetCharRect gesetzt und im UnitUp/Down ausgewertet.
sal_Bool SwTxtCursor::bRightMargin = sal_False;


/*************************************************************************
 *                    lcl_GetPositionInsideField
 *
 * After calculating the position of a character during GetCharRect
 * this function allows to find the coordinates of a position (defined
 * in pCMS->pSpecialPos) inside a special portion (e.g., a field)
 *************************************************************************/

/*************************************************************************
 *				  SwTxtMargin::CtorInit()
 *************************************************************************/
/*M*/ void SwTxtMargin::CtorInit( SwTxtFrm *pFrm, SwTxtSizeInfo *pNewInf )
/*M*/ {
/*M*/ 	SwTxtIter::CtorInit( pFrm, pNewInf );
/*M*/ 
/*M*/ 	pInf = pNewInf;
/*M*/ 	GetInfo().SetFont( GetFnt() );
/*M*/ 	SwTxtNode *pNode = pFrm->GetTxtNode();
/*M*/ 
/*M*/ 	const SvxLRSpaceItem &rSpace =
/*M*/ 		pFrm->GetTxtNode()->GetSwAttrSet().GetLRSpace();
/*M*/ 
/*M*/ #ifdef BIDI
/*M*/     //
/*M*/     // Carefully adjust the text formatting ranges.
/*M*/     //
/*M*/     const int nLMWithNum = pNode->GetLeftMarginWithNum( sal_True );
/*M*/     if ( pFrm->IsRightToLeft() )
/*M*/         nLeft = pFrm->Frm().Left() + pFrm->Prt().Left() + nLMWithNum -
/*M*/                 ( rSpace.GetTxtFirstLineOfst() < 0 ?
/*M*/                   rSpace.GetTxtFirstLineOfst() :
/*M*/                   0 );
/*M*/     else
/*M*/         nLeft = Max( long( rSpace.GetTxtLeft() + nLMWithNum), pFrm->Prt().Left() ) +
/*M*/                 pFrm->Frm().Left();
/*M*/ #else
/*M*/     nLeft = Max( long( rSpace.GetTxtLeft() + pNode->GetLeftMarginWithNum(sal_True) ),
/*M*/                  pFrm->Prt().Left() ) +
/*M*/             pFrm->Frm().Left();
/*M*/ #endif
/*M*/ 
/*M*/     nRight = pFrm->Frm().Left() + pFrm->Prt().Left() + pFrm->Prt().Width();
/*M*/ 
/*M*/ 	if( nLeft >= nRight )
/*M*/ 		nLeft = pFrm->Prt().Left() + pFrm->Frm().Left();
/*M*/ 	if( nLeft >= nRight ) // z.B. bei grossen Absatzeinzuegen in schmalen Tabellenspalten
/*M*/ 		nRight = nLeft + 1; // einen goennen wir uns immer
/*M*/ 	if( pFrm->IsFollow() && pFrm->GetOfst() )
/*M*/ 		nFirst = nLeft;
/*M*/ 	else
/*M*/ 	{
/*M*/ 		short nFLOfst;
/*M*/ 		long nFirstLineOfs;
/*M*/ 		if( !pNode->GetFirstLineOfsWithNum( nFLOfst ) &&
/*M*/ 			rSpace.IsAutoFirst() )
/*M*/ 		{
/*M*/ 			nFirstLineOfs = GetFnt()->GetSize( GetFnt()->GetActual() ).Height();
/*M*/ 			const SvxLineSpacingItem *pSpace = aLineInf.GetLineSpacing();
/*M*/ 			if( pSpace )
/*M*/ 			{
/*M*/ 				switch( pSpace->GetLineSpaceRule() )
/*M*/ 				{
/*M*/ 					case SVX_LINE_SPACE_AUTO:
/*M*/ 					break;
/*M*/ 					case SVX_LINE_SPACE_MIN:
/*M*/ 					{
/*M*/ 						if( nFirstLineOfs < KSHORT( pSpace->GetLineHeight() ) )
/*M*/ 							nFirstLineOfs = pSpace->GetLineHeight();
/*M*/ 						break;
/*M*/ 					}
/*M*/ 					case SVX_LINE_SPACE_FIX:
/*M*/ 						nFirstLineOfs = pSpace->GetLineHeight();
/*M*/ 					break;
/*M*/ 					default: ASSERT( sal_False, ": unknown LineSpaceRule" );
/*M*/ 				}
/*M*/ 				switch( pSpace->GetInterLineSpaceRule() )
/*M*/ 				{
/*M*/ 					case SVX_INTER_LINE_SPACE_OFF:
/*M*/ 					break;
/*M*/ 					case SVX_INTER_LINE_SPACE_PROP:
/*M*/ 					{
/*M*/ 						long nTmp = pSpace->GetPropLineSpace();
/*M*/ 						// 50% ist das Minimum, bei 0% schalten wir auf
/*M*/ 						// den Defaultwert 100% um ...
/*M*/ 						if( nTmp < 50 )
/*M*/ 							nTmp = nTmp ? 50 : 100;
/*M*/ 
/*M*/ 						nTmp *= nFirstLineOfs;
/*M*/ 						nTmp /= 100;
/*M*/ 						if( !nTmp )
/*M*/ 							++nTmp;
/*M*/ 						nFirstLineOfs = (KSHORT)nTmp;
/*M*/ 						break;
/*M*/ 					}
/*M*/ 					case SVX_INTER_LINE_SPACE_FIX:
/*M*/ 					{
/*M*/ 						nFirstLineOfs += pSpace->GetInterLineSpace();
/*M*/ 						break;
/*M*/ 					}
/*M*/ 					default: ASSERT( sal_False, ": unknown InterLineSpaceRule" );
/*M*/ 				}
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else
/*M*/ 			nFirstLineOfs = nFLOfst;
/*M*/ 
/*M*/ #ifdef BIDI
/*M*/         if ( pFrm->IsRightToLeft() )
/*M*/             nFirst = nLeft + nFirstLineOfs;
/*M*/         else
/*M*/             nFirst = Max( rSpace.GetTxtLeft() + nLMWithNum + nFirstLineOfs,
/*M*/                           pFrm->Prt().Left() ) + pFrm->Frm().Left();
/*M*/ #else
/*M*/ 		nFirst = Max( rSpace.GetTxtLeft() + pNode->GetLeftMarginWithNum( sal_True )
/*M*/ 			+ nFirstLineOfs, pFrm->Prt().Left() ) + pFrm->Frm().Left();
/*M*/ #endif
/*M*/ 
/*M*/ 		if( nFirst >= nRight )
/*M*/ 			nFirst = nRight - 1;
/*M*/ 	}
/*M*/     const SvxAdjustItem& rAdjust = pFrm->GetTxtNode()->GetSwAttrSet().GetAdjust();
/*M*/ 	nAdjust = rAdjust.GetAdjust();
/*M*/ 
/*M*/ #ifdef BIDI
/*M*/     // left is left and right is right
/*M*/     if ( pFrm->IsRightToLeft() )
/*M*/     {
/*M*/         if ( SVX_ADJUST_LEFT == nAdjust )
/*M*/             nAdjust = SVX_ADJUST_RIGHT;
/*M*/         else if ( SVX_ADJUST_RIGHT == nAdjust )
/*M*/             nAdjust = SVX_ADJUST_LEFT;
/*M*/     }
/*M*/ #endif
/*M*/ 
/*M*/ 	bOneBlock = rAdjust.GetOneWord() == SVX_ADJUST_BLOCK;
/*M*/ 	bLastBlock = rAdjust.GetLastBlock() == SVX_ADJUST_BLOCK;
/*M*/ 	bLastCenter = rAdjust.GetLastBlock() == SVX_ADJUST_CENTER;
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*M*/ 	static sal_Bool bOne = sal_False;
/*M*/ 	static sal_Bool bLast = sal_False;
/*M*/ 	static sal_Bool bCenter = sal_False;
/*M*/ 	bOneBlock |= bOne;
/*M*/ 	bLastBlock |= bLast;
/*M*/ 	bLastCenter |= bCenter;
/*M*/ #endif
/*M*/ 	DropInit();
/*M*/ }

/*************************************************************************
 *				  SwTxtMargin::DropInit()
 *************************************************************************/
/*N*/ void SwTxtMargin::DropInit()
/*N*/ {
/*N*/ 	nDropLeft = nDropLines = nDropHeight = nDropDescent = 0;
/*N*/ 	const SwParaPortion *pPara = GetInfo().GetParaPortion();
/*N*/ 	if( pPara )
/*N*/ 	{
/*N*/ 		const SwDropPortion *pPorDrop = pPara->FindDropPortion();
/*N*/ 		if ( pPorDrop )
/*N*/ 		{
/*N*/ 			nDropLeft = pPorDrop->GetDropLeft();
/*N*/ 			nDropLines = pPorDrop->GetLines();
/*N*/ 			nDropHeight = pPorDrop->GetDropHeight();
/*N*/ 			nDropDescent = pPorDrop->GetDropDescent();
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *				  SwTxtMargin::GetLineStart()
 *************************************************************************/

// Unter Beruecksichtigung des Erstzeileneinzuges und der angebenen Breite.
/*N*/ SwTwips SwTxtMargin::GetLineStart() const
/*N*/ {
/*N*/ 	SwTwips nRet = GetLeftMargin();
/*N*/ 	if( GetAdjust() != SVX_ADJUST_LEFT &&
/*N*/ 		!pCurr->GetFirstPortion()->IsMarginPortion() )
/*N*/ 	{
/*N*/ 		// Wenn die erste Portion ein Margin ist, dann wird das
/*N*/ 		// Adjustment durch die Portions ausgedrueckt.
/*N*/ 		if( GetAdjust() == SVX_ADJUST_RIGHT )
/*N*/ 			nRet = Right() - CurrWidth();
/*N*/ 		else if( GetAdjust() == SVX_ADJUST_CENTER )
/*N*/ 			nRet += (GetLineWidth() - CurrWidth()) / 2;
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*************************************************************************
 *						SwTxtCursor::CtorInit()
 *************************************************************************/
/*N*/ void SwTxtCursor::CtorInit( SwTxtFrm *pFrm, SwTxtSizeInfo *pInf )
/*N*/ {
/*N*/ 	SwTxtMargin::CtorInit( pFrm, pInf );
/*N*/ 	// 6096: Vorsicht, die Iteratoren sind abgeleitet!
/*N*/ 	// GetInfo().SetOut( GetInfo().GetWin() );
/*N*/ }

/*************************************************************************
 *						SwTxtCursor::GetEndCharRect()
 *************************************************************************/

// 1170: Antikbug: Shift-Ende vergisst das letzte Zeichen ...


/*************************************************************************
 * void	SwTxtCursor::_GetCharRect(..)
 * internal function, called by SwTxtCursor::GetCharRect() to calculate
 * the relative character position in the current line.
 * pOrig referes to x and y coordinates, width and height of the cursor
 * pCMS is used for restricting the cursor, if there are different font
 * heights in one line ( first value = offset to y of pOrig, second
 * value = real height of (shortened) cursor
 *************************************************************************/

/*N*/ void SwTxtCursor::_GetCharRect( SwRect* pOrig, const xub_StrLen nOfst,
/*N*/     SwCrsrMoveState* pCMS )
/*N*/ {
/*N*/ 	const XubString &rText = GetInfo().GetTxt();
/*N*/ 	SwTxtSizeInfo aInf( GetInfo(), rText, nStart );
/*N*/ 	if( GetPropFont() )
/*?*/ 		aInf.GetFont()->SetProportion( GetPropFont() );
/*N*/ 	KSHORT nTmpAscent, nTmpHeight;	// Zeilenhoehe
/*N*/ 	CalcAscentAndHeight( nTmpAscent, nTmpHeight );
/*N*/ 	const Size	aCharSize( 1, nTmpHeight );
/*N*/ 	const Point aCharPos;
/*N*/ 	pOrig->Pos( aCharPos );
/*N*/ 	pOrig->SSize( aCharSize );
/*N*/ 
/*N*/     // If we are looking for a position inside a field which covers
/*N*/     // more than one line we may not skip any "empty portions" at the
/*N*/     // beginning of a line
/*N*/     const sal_Bool bInsideFirstField = pCMS && pCMS->pSpecialPos &&
/*N*/                                        ( pCMS->pSpecialPos->nLineOfst ||
/*N*/                                          SP_EXTEND_RANGE_BEFORE ==
/*N*/                                          pCMS->pSpecialPos->nExtendRange );
/*N*/ 
/*N*/ 	sal_Bool bWidth = pCMS && pCMS->bRealWidth;
/*N*/ 	if( !pCurr->GetLen() && !pCurr->Width() )
/*N*/ 	{
/*N*/ 		if ( pCMS && pCMS->bRealHeight )
/*N*/ 		{
/*N*/ 			pCMS->aRealHeight.X() = 0;
/*N*/ 			pCMS->aRealHeight.Y() = nTmpHeight;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		KSHORT nPorHeight = nTmpHeight;
/*N*/ 		KSHORT nPorAscent = nTmpAscent;
/*N*/ 		SwTwips nX = 0;
/*N*/ 		SwTwips nFirst = 0;
/*N*/ 		SwLinePortion *pPor = pCurr->GetFirstPortion();
/*N*/         SvShorts* pSpaceAdd = pCurr->GetpSpaceAdd();
/*N*/         SvUShorts* pKanaComp = pCurr->GetpKanaComp();
/*N*/         MSHORT nSpaceIdx = 0;
/*N*/         MSHORT nKanaIdx = 0;
/*N*/ 		short nSpaceAdd = pSpaceAdd ? (*pSpaceAdd)[0] : 0;
/*N*/ 
/*N*/         sal_Bool bNoTxt = sal_True;
/*N*/ 
/*N*/ 		// Zuerst werden alle Portions ohne Len am Zeilenanfang uebersprungen.
/*N*/ 		// Ausnahme bilden die fiesen Spezialportions aus WhichFirstPortion:
/*N*/ 		// Num, ErgoSum, FtnNum, FeldReste
/*N*/ 		// 8477: aber auch die einzige Textportion einer leeren Zeile mit
/*N*/ 		// Right/Center-Adjustment! Also nicht nur pPor->GetExpandPortion() ...
/*N*/ 
/*N*/         while( pPor && !pPor->GetLen() && ! bInsideFirstField )
/*N*/ 		{
/*N*/ 			nX += pPor->Width();
/*N*/ 			if ( pPor->InSpaceGrp() && nSpaceAdd )
                    {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 				nX += pPor->CalcSpacing( nSpaceAdd, aInf );
/*N*/ 			if( bNoTxt )
/*N*/ 				nFirst = nX;
/*N*/ 			// 8670: EndPortions zaehlen hier einmal als TxtPortions.
/*N*/ 			if( pPor->InTxtGrp() || pPor->IsBreakPortion() )
/*N*/ 			{
/*N*/ 				bNoTxt = sal_False;
/*N*/ 				nFirst = nX;
/*N*/ 			}
/*N*/             if( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator() )
/*N*/ 			{
/*?*/                 if ( pSpaceAdd )
/*?*/                 {
/*?*/                     if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                         nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                     else
/*?*/                         nSpaceAdd = 0;
/*?*/                 }
/*?*/ 
/*?*/                 if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->Count() )
/*?*/                     ++nKanaIdx;
/*N*/ 			}
/*N*/ 			if( pPor->InFixMargGrp() )
/*N*/ 			{
/*N*/ 				if( pPor->IsMarginPortion() )
/*N*/ 					bNoTxt = sal_False;
/*N*/                 else
/*N*/                 {
/*N*/                     // fix margin portion => next SpaceAdd, KanaComp value
/*N*/                     if( pSpaceAdd )
/*N*/                     {
/*N*/                         if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*N*/                             nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*N*/                         else
/*N*/                             nSpaceAdd = 0;
/*N*/                     }
/*N*/ 
/*N*/                     if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->Count() )
/*N*/                         ++nKanaIdx;
/*N*/                 }
/*N*/ 			}
/*N*/ 			pPor = pPor->GetPortion();
/*N*/ 		}
/*N*/ 
/*N*/ 		if( !pPor )
/*N*/ 		{
/*N*/ 			// Es sind nur Spezialportions unterwegs.
/*N*/ 			nX = nFirst;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
/*N*/ 				(!pPor->InFldGrp() || pPor->GetAscent() ) )
/*N*/ 			{
/*N*/ 				nPorHeight = pPor->Height();
/*N*/ 				nPorAscent = pPor->GetAscent();
/*N*/ 			}
/*N*/ #ifdef BIDI
/*N*/ 			while( pPor && !pPor->IsBreakPortion() && ( aInf.GetIdx() < nOfst ||
/*N*/                    ( bWidth && ( pPor->IsKernPortion() || pPor->IsMultiPortion() ) ) ) )
/*N*/ #else
/*N*/ 			while( pPor && !pPor->IsBreakPortion() && ( aInf.GetIdx() < nOfst ||
/*N*/                    ( bWidth && pPor->IsMultiPortion() ) ) )
/*N*/ #endif
/*N*/ 			{
/*N*/ 				if( !pPor->IsMarginPortion() && !pPor->IsPostItsPortion() &&
/*N*/ 					(!pPor->InFldGrp() || pPor->GetAscent() ) )
/*N*/ 				{
/*N*/ 					nPorHeight = pPor->Height();
/*N*/ 					nPorAscent = pPor->GetAscent();
/*N*/ 				}
/*N*/ 
/*N*/                 // If we are behind the portion, we add the portion width to
/*N*/                 // nX. Special case: nOfst = aInf.GetIdx() + pPor->GetLen().
/*N*/                 // For common portions (including BidiPortions) we want to add
/*N*/                 // the portion width to nX. For MultiPortions, nExtra = 0,
/*N*/                 // therefore we go to the 'else' branch and start a recursion.
/*N*/                 const BYTE nExtra = pPor->IsMultiPortion() &&
/*N*/                                     ! ((SwMultiPortion*)pPor)->IsBidi() &&
/*N*/                                     ! bWidth ? 0 : 1;
/*N*/                 if ( aInf.GetIdx() + pPor->GetLen() < nOfst + nExtra )
/*N*/ 				{
/*N*/ 					if ( pPor->InSpaceGrp() && nSpaceAdd )
                            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*N*/ 						nX += pPor->PrtWidth() +
/*N*/ 					else
/*N*/ 					{
/*N*/                         if( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() )
/*N*/ 						{
/*N*/                             // update to current SpaceAdd, KanaComp values
/*N*/                             if ( pSpaceAdd )
/*N*/                             {
/*?*/                                 if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                                     nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                                 else
/*?*/                                     nSpaceAdd = 0;
/*?*/                             }
/*N*/ 
/*N*/                             if ( pKanaComp &&
/*N*/                                 ( nKanaIdx + 1 ) < pKanaComp->Count()
/*N*/                                 )
/*N*/                                 ++nKanaIdx;
/*N*/                         }
/*N*/ 						if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
/*N*/ 								!pPor->GetPortion()->IsMarginPortion() ) )
/*N*/ 							nX += pPor->PrtWidth();
/*N*/ 					}
/*N*/                     if( pPor->IsMultiPortion() )
/*N*/                     {
/*N*/                         if ( ((SwMultiPortion*)pPor)->HasTabulator() )
/*N*/ 					{
/*?*/                         if ( pSpaceAdd )
/*?*/                         {
/*?*/                             if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                                 nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                             else
/*?*/                                 nSpaceAdd = 0;
/*?*/                         }
/*?*/ 
/*?*/                         if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->Count() )
/*?*/                             ++nKanaIdx;
/*N*/ 					}
/*N*/ 
/*N*/                     }
/*N*/ 
/*N*/ 					aInf.SetIdx( aInf.GetIdx() + pPor->GetLen() );
/*N*/                     pPor = pPor->GetPortion();
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( pPor->IsMultiPortion() )
/*N*/ 					{
                            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ #ifdef VERTICAL_LAYOUT
/*N*/ 					}
/*N*/ 					if ( pPor->PrtWidth() )
/*N*/ 					{
/*N*/ 						xub_StrLen nOldLen = pPor->GetLen();
/*N*/ 						pPor->SetLen( nOfst - aInf.GetIdx() );
/*N*/ 						aInf.SetLen( pPor->GetLen() );
/*N*/ 						if( nX || !pPor->InNumberGrp() )
/*N*/ 						{
/*N*/ 							SeekAndChg( aInf );
/*N*/ 							const sal_Bool bOldOnWin = aInf.OnWin();
/*N*/ 							aInf.SetOnWin( sal_False ); // keine BULLETs!
/*N*/ 							SwTwips nTmp = nX;
/*N*/                             aInf.SetKanaComp( pKanaComp );
/*N*/                             aInf.SetKanaIdx( nKanaIdx );
/*N*/                             nX += pPor->GetTxtSize( aInf ).Width();
/*N*/ 							aInf.SetOnWin( bOldOnWin );
/*N*/ 							if ( pPor->InSpaceGrp() && nSpaceAdd )
/*N*/ 								nX += pPor->CalcSpacing( nSpaceAdd, aInf );
/*N*/ 							if( bWidth )
/*N*/ 							{
/*?*/ 								pPor->SetLen( pPor->GetLen() + 1 );
/*?*/ 								aInf.SetLen( pPor->GetLen() );
/*?*/ 								aInf.SetOnWin( sal_False ); // keine BULLETs!
/*?*/ 								nTmp += pPor->GetTxtSize( aInf ).Width();
/*?*/ 								aInf.SetOnWin( bOldOnWin );
/*?*/ 								if ( pPor->InSpaceGrp() && nSpaceAdd )
/*?*/ 									nTmp += pPor->CalcSpacing(nSpaceAdd, aInf);
/*?*/ 								pOrig->Width( nTmp - nX );
/*N*/ 							}
/*N*/ 						}
/*N*/ 						pPor->SetLen( nOldLen );
/*N*/ 					}
/*N*/ 					bWidth = sal_False;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		if( pPor )
/*N*/ 		{
/*N*/             ASSERT( !pPor->InNumberGrp() || bInsideFirstField, "Number surprise" );
/*N*/ 			sal_Bool bEmptyFld = sal_False;
/*N*/ 			if( pPor->InFldGrp() && pPor->GetLen() )
/*N*/ 			{
/*N*/ 				SwFldPortion *pTmp = (SwFldPortion*)pPor;
/*N*/ 				while( pTmp->HasFollow() && !pTmp->GetExp().Len() )
/*N*/ 				{
/*?*/ 					KSHORT nAddX = pTmp->Width();
/*?*/ 					SwLinePortion *pNext = pTmp->GetPortion();
/*?*/ 					while( pNext && !pNext->InFldGrp() )
/*?*/ 					{
/*?*/ 						ASSERT( !pNext->GetLen(), "Where's my field follow?" );
/*?*/ 						nAddX += pNext->Width();
/*?*/ 						pNext = pNext->GetPortion();
/*?*/ 					}
/*?*/ 					if( !pNext )
/*?*/ 						break;
/*?*/ 					pTmp = (SwFldPortion*)pNext;
/*?*/ 					nPorHeight = pTmp->Height();
/*?*/ 					nPorAscent = pTmp->GetAscent();
/*?*/ 					nX += nAddX;
/*?*/ 					bEmptyFld = sal_True;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			// 8513: Felder im Blocksatz, ueberspringen
/*N*/             while( pPor && !pPor->GetLen() && ! bInsideFirstField &&
/*N*/                    ( pPor->IsFlyPortion() || pPor->IsKernPortion() ||
/*N*/                      pPor->IsBlankPortion() || pPor->InTabGrp() ||
/*N*/                      ( !bEmptyFld && pPor->InFldGrp() ) ) )
/*N*/ 			{
/*?*/ 				if ( pPor->InSpaceGrp() && nSpaceAdd )
/*?*/ 					nX += pPor->PrtWidth() +
/*?*/ 						  pPor->CalcSpacing( nSpaceAdd, aInf );
/*?*/ 				else
/*?*/ 				{
/*?*/                     if( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() )
/*?*/ 					{
/*?*/                         if ( pSpaceAdd )
/*?*/                         {
/*?*/                             if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                                 nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                             else
/*?*/                                 nSpaceAdd = 0;
/*?*/                         }
/*?*/ 
/*?*/                         if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->Count() )
/*?*/                             ++nKanaIdx;
/*?*/ 					}
/*?*/ 					if ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
/*?*/ 							!pPor->GetPortion()->IsMarginPortion() ) )
/*?*/ 						nX += pPor->PrtWidth();
/*?*/ 				}
/*?*/                 if( pPor->IsMultiPortion() &&
/*?*/                     ((SwMultiPortion*)pPor)->HasTabulator() )
/*?*/ 				{
/*?*/                     if ( pSpaceAdd )
/*?*/                     {
/*?*/                         if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                             nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                         else
/*?*/                             nSpaceAdd = 0;
/*?*/                     }
/*?*/ 
/*?*/                     if( pKanaComp && ( nKanaIdx + 1 ) < pKanaComp->Count() )
/*?*/                         ++nKanaIdx;
/*?*/ 				}
/*?*/ 				if( !pPor->IsFlyPortion() )
/*?*/ 				{
/*?*/ 					nPorHeight = pPor->Height();
/*?*/ 					nPorAscent = pPor->GetAscent();
/*?*/ 				}
/*?*/ 				pPor = pPor->GetPortion();
/*N*/ 			}
/*N*/ 
/*N*/ 			if( aInf.GetIdx() == nOfst && pPor && pPor->InHyphGrp() &&
/*N*/ 				pPor->GetPortion() && pPor->GetPortion()->InFixGrp() )
/*N*/ 			{
/*N*/ 				// Alle Sonderportions muessen uebersprungen werden
/*N*/ 				// Beispiel: zu-[FLY]sammen, 'u' == 19, 's' == 20; Right()
/*N*/ 				// Ohne den Ausgleich landen wir vor '-' mit dem
/*N*/ 				// Ausgleich vor 's'.
/*N*/ 				while( pPor && !pPor->GetLen() )
/*N*/ 				{
///*N*/ 					DBG_LOOP;
/*N*/ 					nX += pPor->Width();
/*N*/ 					if( !pPor->IsMarginPortion() )
/*N*/ 					{
/*N*/ 						nPorHeight = pPor->Height();
/*N*/ 						nPorAscent = pPor->GetAscent();
/*N*/ 					}
/*N*/ 					pPor = pPor->GetPortion();
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if( pPor && pCMS )
/*N*/ 			{
/*N*/ 				if( pCMS->bFieldInfo &&	pPor->InFldGrp() && pPor->Width() )
/*N*/ 					pOrig->Width( pPor->Width() );
/*N*/ 				if( pPor->IsDropPortion() )
/*N*/                 {
/*?*/ 					nPorAscent = ((SwDropPortion*)pPor)->GetDropHeight();
/*?*/                     // The drop height is only calculated, if we have more than
/*?*/                     // one line. Otherwise it is 0.
/*?*/                     if ( ! nPorAscent)
/*?*/                         nPorAscent = pPor->Height();
/*?*/ 					nPorHeight = nPorAscent;
/*?*/ 					pOrig->Height( nPorHeight +
/*?*/ 						((SwDropPortion*)pPor)->GetDropDescent() );
/*?*/ 					if( nTmpHeight < pOrig->Height() )
/*?*/ 					{
/*?*/ 						nTmpAscent = nPorAscent;
/*?*/ 						nTmpHeight = USHORT( pOrig->Height() );
/*?*/ 					}
/*N*/ 				}
/*N*/ 				if( bWidth && pPor->PrtWidth() && pPor->GetLen() &&
/*N*/ 					aInf.GetIdx() == nOfst )
/*N*/ 				{
/*?*/ 					if( !pPor->IsFlyPortion() && pPor->Height() &&
/*?*/ 						pPor->GetAscent() )
/*?*/ 					{
/*?*/ 						nPorHeight = pPor->Height();
/*?*/ 						nPorAscent = pPor->GetAscent();
/*?*/ 					}
/*?*/ 					SwTwips nTmp;
/*?*/ 					if( 2 > pPor->GetLen() )
/*?*/ 					{
/*?*/ 						nTmp = pPor->Width();
/*?*/ 						if ( pPor->InSpaceGrp() && nSpaceAdd )
/*?*/ 							nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						const sal_Bool bOldOnWin = aInf.OnWin();
/*?*/ 						xub_StrLen nOldLen = pPor->GetLen();
/*?*/ 						pPor->SetLen( 1 );
/*?*/ 						aInf.SetLen( pPor->GetLen() );
/*?*/ 						SeekAndChg( aInf );
/*?*/ 						aInf.SetOnWin( sal_False ); // keine BULLETs!
/*?*/                         aInf.SetKanaComp( pKanaComp );
/*?*/                         aInf.SetKanaIdx( nKanaIdx );
/*?*/ 						nTmp = pPor->GetTxtSize( aInf ).Width();
/*?*/ 						aInf.SetOnWin( bOldOnWin );
/*?*/ 						if ( pPor->InSpaceGrp() && nSpaceAdd )
/*?*/ 							nTmp += pPor->CalcSpacing( nSpaceAdd, aInf );
/*?*/ 						pPor->SetLen( nOldLen );
/*?*/ 					}
/*?*/ 					pOrig->Width( nTmp );
/*N*/ 				}
/*N*/ 
/*N*/                 // travel inside field portion?
/*N*/                 if ( pCMS->pSpecialPos )
/*N*/                 {
/*N*/                     // apply attributes to font
                        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/                     Seek( nOfst );
/*N*/                 }
/*N*/             }
/*N*/         }
/*N*/ 						
/*N*/ 		pOrig->Pos().X() += nX;
/*N*/ 
/*N*/ 		if ( pCMS && pCMS->bRealHeight )
/*N*/ 		{
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/             nTmpAscent = AdjustBaseLine( *pCurr, 0, nPorHeight, nPorAscent );
/*N*/ #else
/*N*/             nTmpAscent = AdjustBaseLine( *pCurr, nPorHeight, nPorAscent );
/*N*/ #endif
/*N*/             if ( nTmpAscent > nPorAscent )
/*N*/ 				pCMS->aRealHeight.X() = nTmpAscent - nPorAscent;
/*N*/ 			else
/*N*/ 				pCMS->aRealHeight.X() = 0;
/*N*/ 			ASSERT( nPorHeight, "GetCharRect: Missing Portion-Height" );
/*N*/ 			if ( nTmpHeight > nPorHeight )
/*N*/ 				pCMS->aRealHeight.Y() = nPorHeight;
/*N*/ 			else
/*N*/ 				pCMS->aRealHeight.Y() = nTmpHeight;
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *						SwTxtCursor::GetCharRect()
 *************************************************************************/

/*N*/ sal_Bool SwTxtCursor::GetCharRect( SwRect* pOrig, const xub_StrLen nOfst,
/*N*/ 							   SwCrsrMoveState* pCMS, const long nMax )
/*N*/ {
/*N*/ 	CharCrsrToLine(nOfst);
/*N*/ 
/*N*/     // Indicates that a position inside a special portion (field, number portion)
/*N*/     // is requested.
/*N*/     const sal_Bool bSpecialPos = pCMS && pCMS->pSpecialPos;
/*N*/     xub_StrLen nFindOfst = nOfst;
/*N*/ 
/*N*/     if ( bSpecialPos )
/*N*/     {
/*?*/         xub_StrLen nLineOfst = pCMS->pSpecialPos->nLineOfst;
/*?*/         BYTE nExtendRange = pCMS->pSpecialPos->nExtendRange;
/*?*/ 
/*?*/         ASSERT( ! nLineOfst || SP_EXTEND_RANGE_BEFORE != nExtendRange,
/*?*/                 "LineOffset AND Number Portion?" )
/*?*/ 
/*?*/         // portions which are behind the string
/*?*/         if ( SP_EXTEND_RANGE_BEHIND == nExtendRange )
/*?*/             ++nFindOfst;
/*?*/ 
/*?*/         // skip lines for fields which cover more than one line
/*?*/         for ( USHORT i = 0; i < pCMS->pSpecialPos->nLineOfst; i++ )
/*?*/             Next();
/*N*/     }
/*N*/ 
/*N*/ 	// Adjustierung ggf. nachholen
/*N*/ 	GetAdjusted();
/*N*/ 
/*N*/ 	const Point aCharPos( GetTopLeft() );
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 
/*N*/     _GetCharRect( pOrig, nFindOfst, pCMS );
/*N*/ 
/*N*/     const SwTwips nRight = Right() - 12;
/*N*/ 
/*N*/     pOrig->Pos().X() += aCharPos.X();
/*N*/ 	pOrig->Pos().Y() += aCharPos.Y();
/*N*/ 
/*N*/ 	if( pCMS && pCMS->b2Lines && pCMS->p2Lines )
/*N*/ 	{
/*?*/ 		pCMS->p2Lines->aLine.Pos().X() += aCharPos.X();
/*?*/ 		pCMS->p2Lines->aLine.Pos().Y() += aCharPos.Y();
/*?*/ 		pCMS->p2Lines->aPortion.Pos().X() += aCharPos.X();
/*?*/ 		pCMS->p2Lines->aPortion.Pos().Y() += aCharPos.Y();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pOrig->Left() > nRight )
/*?*/ 		pOrig->Pos().X() = nRight;
/*N*/ 
/*N*/ 	if( nMax )
/*N*/ 	{
/*N*/ 
/*N*/ #ifdef VERTICAL_LAYOUT
/*N*/         if( pOrig->Top() + pOrig->Height() > nMax )
/*N*/ 		{
/*N*/ 			if( pOrig->Top() > nMax )
/*N*/ 				pOrig->Top( nMax );
/*N*/             pOrig->Height( nMax - pOrig->Top() );
/*N*/ #else
/*N*/ 		if( pOrig->Bottom() > nMax )
/*N*/ 		{
/*N*/ 			if( pOrig->Top() > nMax )
/*N*/ 				pOrig->Top( nMax );
/*N*/ 			pOrig->Bottom( nMax );
/*N*/ #endif
/*N*/ 		}
/*N*/ 		if ( pCMS && pCMS->bRealHeight && pCMS->aRealHeight.Y() >= 0 )
/*N*/ 		{
/*N*/ 			long nTmp = pCMS->aRealHeight.X() + pOrig->Top();
/*N*/ 			if(	nTmp >= nMax )
/*N*/ 			{
/*?*/ 				pCMS->aRealHeight.X() = nMax - pOrig->Top();
/*?*/ 				pCMS->aRealHeight.Y() = 0;
/*N*/ 			}
/*N*/ 			else if( nTmp + pCMS->aRealHeight.Y() > nMax )
/*?*/ 				pCMS->aRealHeight.Y() = nMax - nTmp;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	long nOut = pOrig->Right() - GetTxtFrm()->Frm().Right();
/*N*/ 	if( nOut > 0 )
/*N*/ 	{
/*N*/ 		if( GetTxtFrm()->Frm().Width() < GetTxtFrm()->Prt().Left()
/*N*/ 								   + GetTxtFrm()->Prt().Width() )
/*?*/ 			nOut += GetTxtFrm()->Frm().Width() - GetTxtFrm()->Prt().Left()
/*?*/ 					- GetTxtFrm()->Prt().Width();
/*N*/ 		if( nOut > 0 )
/*?*/ 			pOrig->Pos().X() -= nOut + 10;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
 *						SwTxtCursor::GetCrsrOfst()
 *
 * Return: Offset im String
 *************************************************************************/
/*N*/ xub_StrLen SwTxtCursor::GetCrsrOfst( SwPosition *pPos, const Point &rPoint,
/*N*/ 					 const MSHORT nChgNode, const SwCrsrMoveState* pCMS ) const
/*N*/ {
/*N*/ 	// Adjustierung ggf. nachholen
/*N*/ 	GetAdjusted();
/*N*/ 
/*N*/ 	const XubString &rText = GetInfo().GetTxt();
/*N*/ 	xub_StrLen nOffset = 0;
/*N*/ 
/*N*/ 	// x ist der horizontale Offset innerhalb der Zeile.
/*N*/ 	SwTwips x = rPoint.X();
/*N*/ 	CONST SwTwips nLeftMargin  = GetLineStart();
/*N*/ 	SwTwips nRightMargin = GetLineEnd();
/*N*/ 	if( nRightMargin == nLeftMargin )
/*N*/ 		nRightMargin += 30;
/*N*/ 
/*N*/ 	const sal_Bool bLeftOver = x < nLeftMargin;
/*N*/ 	if( bLeftOver )
/*N*/ 		x = nLeftMargin;
/*N*/ 	const sal_Bool bRightOver = x > nRightMargin;
/*N*/ 	if( bRightOver )
/*N*/ 		x = nRightMargin;
/*N*/ 
/*N*/ 	sal_Bool bRightAllowed = pCMS && ( pCMS->eState == MV_NONE );
/*N*/ 
/*N*/ 	// Bis hierher in Dokumentkoordinaten.
/*N*/ 	x -= nLeftMargin;
/*N*/ 
/*N*/ 	KSHORT nX = KSHORT( x );
/*N*/ 
/*N*/ 	// Wenn es in der Zeile Attributwechsel gibt, den Abschnitt
/*N*/ 	// suchen, in dem nX liegt.
/*N*/ 	SwLinePortion *pPor = pCurr->GetFirstPortion();
/*N*/ 	xub_StrLen nCurrStart  = nStart;
/*N*/ 	sal_Bool bLastPortion;
/*N*/ 	sal_Bool bHolePortion = sal_False;
/*N*/ 	sal_Bool bLastHyph = sal_False;
/*N*/ 
/*N*/ 	SvShorts *pSpaceAdd = pCurr->GetpSpaceAdd();
/*N*/     SvUShorts *pKanaComp = pCurr->GetpKanaComp();
/*N*/ 	xub_StrLen nOldIdx = GetInfo().GetIdx();
/*N*/ 	MSHORT nSpaceIdx = 0;
/*N*/     MSHORT nKanaIdx = 0;
/*N*/ 	short nSpaceAdd = pSpaceAdd ? (*pSpaceAdd)[0] : 0;
/*N*/     short nKanaComp = pKanaComp ? (*pKanaComp)[0] : 0;
/*N*/ 
/*N*/ 	// nWidth ist die Breite der Zeile, oder die Breite des
/*N*/ 	// Abschnitts mit dem Fontwechsel, in dem nX liegt.
/*N*/ 
/*N*/ 	KSHORT nWidth = pPor->Width();
/*N*/     if ( pSpaceAdd || pKanaComp )
/*N*/ 	{
/*?*/ 		if ( pPor->InSpaceGrp() && nSpaceAdd )
/*?*/ 		{
/*?*/ 			((SwTxtSizeInfo&)GetInfo()).SetIdx( nCurrStart );
/*?*/ 			nWidth += USHORT( pPor->CalcSpacing( nSpaceAdd, GetInfo() ) );
/*?*/ 		}
/*?*/         if( ( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() ) ||
/*?*/             ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator() )
/*?*/           )
/*?*/ 		{
/*?*/             if ( pSpaceAdd )
/*?*/             {
/*?*/                 if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                     nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                 else
/*?*/                     nSpaceAdd = 0;
/*?*/             }
/*?*/ 
/*?*/             if( pKanaComp )
/*?*/             {
/*?*/                 if ( nKanaIdx + 1 < pKanaComp->Count() )
/*?*/                     nKanaComp = (*pKanaComp)[++nKanaIdx];
/*?*/                 else
/*?*/                     nKanaComp = 0;
/*?*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     KSHORT nWidth30;
/*N*/     if ( pPor->IsPostItsPortion() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         nWidth30 = 30 + pPor->GetViewWidth( GetInfo() ) / 2;
/*N*/     else
/*N*/         nWidth30 = ! nWidth && pPor->GetLen() && pPor->InToxRefOrFldGrp() ?
/*N*/                      30 :
/*N*/                      nWidth;
/*N*/ 
/*N*/ 	while(!(bLastPortion = (0 == pPor->GetPortion())) && nWidth30 < nX &&
/*N*/ 		!pPor->IsBreakPortion() )
/*N*/ 	{
/*?*/ 		nX -= nWidth;
/*?*/ 		nCurrStart += pPor->GetLen();
/*?*/ 		bHolePortion = pPor->IsHolePortion();
/*?*/ 		pPor = pPor->GetPortion();
/*?*/ 		nWidth = pPor->Width();
/*?*/         if ( pSpaceAdd || pKanaComp )
/*?*/ 		{
/*?*/ 			if ( pPor->InSpaceGrp() && nSpaceAdd )
/*?*/ 			{
/*?*/ 				((SwTxtSizeInfo&)GetInfo()).SetIdx( nCurrStart );
/*?*/ 				nWidth += USHORT( pPor->CalcSpacing( nSpaceAdd, GetInfo() ) );
/*?*/ 			}
/*?*/ 
/*?*/             if( ( pPor->InFixMargGrp() && ! pPor->IsMarginPortion() ) ||
/*?*/                 ( pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->HasTabulator() )
/*?*/               )
/*?*/ 			{
/*?*/                 if ( pSpaceAdd )
/*?*/                 {
/*?*/                     if ( ++nSpaceIdx < pSpaceAdd->Count() )
/*?*/                         nSpaceAdd = (*pSpaceAdd)[nSpaceIdx];
/*?*/                     else
/*?*/                         nSpaceAdd = 0;
/*?*/                 }
/*?*/ 
/*?*/                 if ( pKanaComp )
/*?*/                 {
/*?*/                     if( nKanaIdx + 1 < pKanaComp->Count() )
/*?*/                         nKanaComp = (*pKanaComp)[++nKanaIdx];
/*?*/                     else
/*?*/                         nKanaComp = 0;
/*?*/                 }
/*?*/ 			}
/*?*/ 		}
/*?*/ 
/*?*/         if ( pPor->IsPostItsPortion() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/             nWidth30 = 30 + pPor->GetViewWidth( GetInfo() ) / 2;
/*?*/         else
/*?*/             nWidth30 = ! nWidth && pPor->GetLen() && pPor->InToxRefOrFldGrp() ?
/*?*/                          30 :
/*?*/                          nWidth;
/*?*/ 		if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
/*?*/ 			bLastHyph = pPor->InHyphGrp();
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nX==nWidth )
/*N*/ 	{
/*N*/ 		SwLinePortion *pNextPor = pPor->GetPortion();
/*N*/ 		while( pNextPor && pNextPor->InFldGrp() && !pNextPor->Width() )
/*N*/ 		{
/*?*/ 			nCurrStart += pPor->GetLen();
/*?*/ 			pPor = pNextPor;
/*?*/ 			if( !pPor->IsFlyPortion() && !pPor->IsMarginPortion() )
/*?*/ 				bLastHyph = pPor->InHyphGrp();
/*?*/ 			pNextPor = pPor->GetPortion();
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	((SwTxtSizeInfo&)GetInfo()).SetIdx( nOldIdx );
/*N*/ 
/*N*/ 	xub_StrLen nLength = pPor->GetLen();
/*N*/ 
/*N*/ 	sal_Bool bFieldInfo = pCMS && pCMS->bFieldInfo;
/*N*/ 
/*N*/ 	if( bFieldInfo && ( nWidth30 < nX || bRightOver || bLeftOver ||
/*N*/ 		( pPor->InNumberGrp() && !pPor->IsFtnNumPortion() ) ||
/*N*/ 		( pPor->IsMarginPortion() && nWidth > nX + 30 ) ) )
/*?*/ 		((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;
/*N*/ 
/*N*/ 	// 7684: Wir sind genau auf der HyphPortion angelangt und muessen dafuer
/*N*/ 	// sorgen, dass wir in dem String landen.
/*N*/ 	// 7993: Wenn die Laenge 0 ist muessen wir raus...
/*N*/ 	if( !nLength )
/*N*/ 	{
/*N*/ 		if( pCMS )
/*N*/ 		{
/*N*/ 			if( pPor->IsFlyPortion() && bFieldInfo )
/*?*/ 				((SwCrsrMoveState*)pCMS)->bPosCorr = sal_True;
/*N*/ 
/*N*/ 			if( pPor->IsFtnNumPortion() && !bRightOver && nX )
/*?*/ 				((SwCrsrMoveState*)pCMS)->bFtnNoInfo = sal_True;
/*N*/ 		}
/*N*/ 		if( !nCurrStart )
/*N*/ 			return 0;
/*N*/ 
/*N*/ 		 // 7849, 7816: auf pPor->GetHyphPortion kann nicht verzichtet werden!
/*N*/ 		if( bHolePortion || ( !bRightAllowed && bLastHyph ) ||
/*N*/ 			( pPor->IsMarginPortion() && !pPor->GetPortion() &&
/*N*/ 			// 46598: In der letzten Zeile eines zentrierten Absatzes wollen
/*N*/ 			// wir auch mal hinter dem letzten Zeichen landen.
/*?*/               nCurrStart < rText.Len() ) )
/*?*/ 			--nCurrStart;
/*N*/ 		else if( pPor->InFldGrp() && ((SwFldPortion*)pPor)->IsFollow()
/*N*/ 				 && nWidth > nX )
/*N*/         {
/*?*/ 			if( bFieldInfo )
/*?*/ 				--nCurrStart;
/*?*/ 			else
/*?*/ 			{
/*?*/ 				KSHORT nHeight = pPor->Height();
/*?*/ 				if ( !nHeight || nHeight > nWidth )
/*?*/ 					nHeight = nWidth;
/*?*/ 				if( nChgNode && nWidth - nHeight/2 > nX )
/*?*/ 					--nCurrStart;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		return nCurrStart;
/*N*/ 	}
/*N*/ 	if ( 1 == nLength )
/*N*/ 	{
/*?*/ 		if ( nWidth )
/*?*/ 		{
/*?*/ 			// Sonst kommen wir nicht mehr in zeichengeb. Rahmen hinein...
/*?*/ 			if( !( nChgNode && pPos && pPor->IsFlyCntPortion() ) )
/*?*/ 			{
/*?*/ 				if ( pPor->InFldGrp() ||
/*?*/                      ( pPor->IsMultiPortion() &&
/*?*/                        ((SwMultiPortion*)pPor)->IsBidi()  ) )
/*?*/ 				{
/*?*/ 					KSHORT nHeight = 0;
/*?*/ 					if( !bFieldInfo )
/*?*/ 					{
/*?*/ 						nHeight = pPor->Height();
/*?*/ 						if ( !nHeight || nHeight > nWidth )
/*?*/ 							nHeight = nWidth;
/*?*/ 					}
/*?*/ 					if( nWidth - nHeight/2 <= nX &&
/*?*/                         ( ! pPor->InFldGrp() ||
/*?*/                           !((SwFldPortion*)pPor)->HasFollow() ) )
/*?*/                         ++nCurrStart;
/*?*/ 				}
/*?*/                 else if ( ( !pPor->IsFlyPortion() || ( pPor->GetPortion() &&
/*?*/ 					!pPor->GetPortion()->IsMarginPortion() &&
/*?*/                     !pPor->GetPortion()->IsHolePortion() ) )
/*?*/                          && ( nWidth/2 < nX ) &&
/*?*/                          ( !bFieldInfo ||
/*?*/                             ( pPor->GetPortion() &&
/*?*/                               pPor->GetPortion()->IsPostItsPortion() ) )
/*?*/ 						 && ( bRightAllowed || !bLastHyph ))
/*?*/ 					++nCurrStart;
/*?*/ 				return nCurrStart;
/*?*/ 			}
/*?*/ 		}
/*?*/ 		else
/*?*/ 		{
/*?*/ 			if ( pPor->IsPostItsPortion() || pPor->IsBreakPortion() ||
/*?*/ 				 pPor->InToxRefGrp() )
/*?*/ 				return nCurrStart;
/*?*/ 			if ( pPor->InFldGrp() )
/*?*/ 			{
/*?*/ 				if( bRightOver && !((SwFldPortion*)pPor)->HasFollow() )
/*?*/ 					++nCurrStart;
/*?*/ 				return nCurrStart;
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bLastPortion && (pCurr->GetNext() || pFrm->GetFollow() ) )
/*N*/ 		--nLength;
/*N*/ 
/*N*/     if( nWidth > nX ||
/*N*/       ( nWidth == nX && pPor->IsMultiPortion() && ((SwMultiPortion*)pPor)->IsDouble() ) )
/*N*/ 	{
/*N*/ 		if( pPor->IsMultiPortion() )
/*N*/ 		{
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/             // In a multi-portion we use GetCrsrOfst()-function recursively
/*N*/ 		}
/*N*/ 		if( pPor->InTxtGrp() )
/*N*/ 		{
/*N*/ 			BYTE nOldProp;
/*N*/ 			if( GetPropFont() )
/*N*/ 			{
/*?*/ 				((SwFont*)GetFnt())->SetProportion( GetPropFont() );
/*?*/ 				nOldProp = GetFnt()->GetPropr();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				nOldProp = 0;
/*N*/ 			{
/*N*/ 				SwTxtSizeInfo aSizeInf( GetInfo(), rText, nCurrStart );
/*N*/ 				((SwTxtCursor*)this)->SeekAndChg( aSizeInf );
/*N*/ 				SwTxtSlot aDiffTxt( &aSizeInf, ((SwTxtPortion*)pPor) );
/*N*/ 				SwFontSave aSave( aSizeInf, pPor->IsDropPortion() ?
/*N*/ 						((SwDropPortion*)pPor)->GetFnt() : NULL );
/*N*/ 
/*N*/                 SwParaPortion* pPara = (SwParaPortion*)GetInfo().GetParaPortion();
/*N*/                 ASSERT( pPara, "No paragraph!" );
/*N*/ 
/*N*/                 SwDrawTextInfo aDrawInf( aSizeInf.GetVsh(),
/*N*/                                          *aSizeInf.GetOut(),
/*N*/                                          &pPara->GetScriptInfo(),
/*N*/                                          aSizeInf.GetTxt(),
/*N*/                                          aSizeInf.GetIdx(),
/*N*/                                          pPor->GetLen() );
/*N*/                 aDrawInf.SetOfst( nX );
/*N*/                 aDrawInf.SetSpace( nSpaceAdd );
/*N*/                 aDrawInf.SetFont( aSizeInf.GetFont() );
/*N*/                 aDrawInf.SetFrm( pFrm );
/*N*/                 aDrawInf.SetSnapToGrid( aSizeInf.SnapToGrid() );
/*N*/                 aDrawInf.SetPosMatchesBounds( pCMS && pCMS->bPosMatchesBounds );
/*N*/ 
/*N*/                 if ( SW_CJK == aSizeInf.GetFont()->GetActual() &&
/*N*/                      pPara->GetScriptInfo().CountCompChg() &&
/*N*/                     ! pPor->InFldGrp() )
/*?*/                     aDrawInf.SetKanaComp( nKanaComp );
/*N*/ 
/*N*/                 nLength = aSizeInf.GetFont()->_GetCrsrOfst( aDrawInf );
/*N*/ 				
/*N*/                 if ( pCMS )
/*N*/                     ((SwCrsrMoveState*)pCMS)->nCursorBidiLevel =
/*N*/                         aDrawInf.GetCursorBidiLevel();
/*N*/ 
/*N*/                 if( bFieldInfo && nLength == pPor->GetLen() &&
/*N*/                     ( ! pPor->GetPortion() ||
/*N*/                       ! pPor->GetPortion()->IsPostItsPortion() ) )
/*N*/ 					--nLength;
/*N*/ 			}
/*N*/ 			if( nOldProp )
/*?*/ 				((SwFont*)GetFnt())->SetProportion( nOldProp );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( nChgNode && pPos && pPor->IsFlyCntPortion()
/*N*/ 				&& !( (SwFlyCntPortion*)pPor )->IsDraw() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*N*/ 			{
/*N*/ 		}
/*N*/ 	}
/*N*/ 	nOffset = nCurrStart + nLength;
/*N*/ 
/*N*/ 	// 7684: Wir sind vor der HyphPortion angelangt und muessen dafuer
/*N*/ 	// sorgen, dass wir in dem String landen.
/*N*/ 	// Bei Zeilenenden vor FlyFrms muessen ebenso behandelt werden.
/*N*/ 
/*N*/ 	if( nOffset && pPor->GetLen() == nLength && pPor->GetPortion() &&
/*N*/ 		!pPor->GetPortion()->GetLen() && pPor->GetPortion()->InHyphGrp() )
/*N*/ 		--nOffset;
/*N*/ 
/*N*/ 	return nOffset;
/*N*/ }

}
