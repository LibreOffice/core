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

#if OSL_DEBUG_LEVEL > 1
# include "ndtxt.hxx"        // pSwpHints, Ausgabeoperator
#endif

#include "itrtxt.hxx"
#include "porfly.hxx"       // CalcFlyAdjust()
#include "pormulti.hxx"


#include <horiornt.hxx>

#include <doc.hxx>
namespace binfilter {

#define MIN_TAB_WIDTH 60

/*************************************************************************
 *                    SwTxtAdjuster::FormatBlock()
 *************************************************************************/

/*N*/ void SwTxtAdjuster::FormatBlock( )
/*N*/ {
/*N*/ 	// In der letzten Zeile gibt's keinen Blocksatz.
/*N*/ 	// Und bei Tabulatoren aus Tradition auch nicht.
/*N*/ 	// 7701: wenn Flys im Spiel sind, geht's weiter
/*N*/ 
/*N*/ 	const SwLinePortion *pFly = 0;
/*N*/ 
/*N*/ 	sal_Bool bSkip = !IsLastBlock() &&
/*N*/ 		nStart + pCurr->GetLen() >= GetInfo().GetTxt().Len();
/*N*/ 
/*N*/ 	// ????: mehrzeilige Felder sind fies: wir muessen kontrollieren,
/*N*/ 	// ob es noch andere Textportions im Absatz gibt.
/*N*/ 	if( bSkip )
/*N*/ 	{
/*N*/ 		const SwLineLayout *pLay = pCurr->GetNext();
/*N*/ 		while( pLay && !pLay->GetLen() )
/*N*/ 		{
/*?*/ 			const SwLinePortion *pPor = pCurr->GetFirstPortion();
/*?*/ 			while( pPor && bSkip )
/*?*/ 			{
/*?*/ 				if( pPor->InTxtGrp() )
/*?*/ 					bSkip = sal_False;
/*?*/ 				pPor = pPor->GetPortion();
/*?*/ 			}
/*?*/ 			pLay = bSkip ? pLay->GetNext() : 0;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bSkip )
/*N*/ 	{
/*N*/ 		if( !GetInfo().GetParaPortion()->HasFly() )
/*N*/ 		{
/*?*/ 			if( IsLastCenter() )
/*?*/ 				CalcFlyAdjust( pCurr );
/*?*/ 			pCurr->FinishSpaceAdd();
/*?*/ 			return;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwLinePortion *pTmpFly = NULL;
/*N*/ 
/*N*/ 			// 7701: beim letzten Fly soll Schluss sein
/*N*/ 			const SwLinePortion *pPos = pCurr->GetFirstPortion();
/*N*/ 			while( pPos )
/*N*/ 			{
/*N*/ 				// Ich suche jetzt den letzten Fly, hinter dem noch Text ist:
/*N*/ 				if( pPos->IsFlyPortion() )
/*N*/ 					pTmpFly = pPos; // Ein Fly wurde gefunden
/*N*/ 				else if ( pTmpFly && pPos->InTxtGrp() )
/*N*/ 				{
/*N*/ 					pFly = pTmpFly; // Ein Fly mit nachfolgendem Text!
/*N*/ 					pTmpFly = NULL;
/*N*/ 				}
/*N*/ 				pPos = pPos->GetPortion();
/*N*/ 			}
/*N*/ 			// 8494: Wenn keiner gefunden wurde, ist sofort Schluss!
/*N*/ 			if( !pFly )
/*N*/ 			{
/*N*/ 				if( IsLastCenter() )
/*?*/ 					CalcFlyAdjust( pCurr );
/*N*/ 				pCurr->FinishSpaceAdd();
/*N*/ 				return;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	const int nOldIdx = GetInfo().GetIdx();
/*N*/ 	GetInfo().SetIdx( nStart );
/*N*/ 	CalcNewBlock( pCurr, pFly );
/*N*/ 	GetInfo().SetIdx( nOldIdx );
/*N*/ 	GetInfo().GetParaPortion()->GetRepaint()->SetOfst(0);
/*N*/ }

/*************************************************************************
 *                    SwTxtAdjuster::CalcNewBlock()
 *
 * CalcNewBlock() darf erst nach CalcLine() gerufen werden !
 * Aufgespannt wird immer zwischen zwei RandPortions oder FixPortions
 * (Tabs und Flys). Dabei werden die Glues gezaehlt und ExpandBlock gerufen.
 *************************************************************************/

/*N*/ void SwTxtAdjuster::CalcNewBlock( SwLineLayout *pCurr,
/*N*/ 								  const SwLinePortion *pStopAt, SwTwips nReal )
/*N*/ {
/*N*/ 	ASSERT( GetInfo().IsMulti() || SVX_ADJUST_BLOCK == GetAdjust(),
/*N*/ 			"CalcNewBlock: Why?" );
/*N*/ 	ASSERT( pCurr->Height(), "SwTxtAdjuster::CalcBlockAdjust: missing CalcLine()" );
/*N*/ 
/*N*/ 	pCurr->InitSpaceAdd();
/*N*/ 	MSHORT nNull = 0;
/*N*/ 	xub_StrLen nGluePortion = 0;
/*N*/ 	xub_StrLen nCharCnt = 0;
/*N*/ 	MSHORT nSpaceIdx = 0;
/*N*/ 
/*N*/ 	// Nicht vergessen:
/*N*/ 	// CalcRightMargin() setzt pCurr->Width() auf die Zeilenbreite !
/*N*/ 	CalcRightMargin( pCurr, nReal );
/*N*/ 
/*N*/ 	SwLinePortion *pPos = pCurr->GetPortion();
/*N*/ 
/*N*/ 	while( pPos )
/*N*/ 	{
/*N*/ 		if ( pPos->IsBreakPortion() && !IsLastBlock() )
/*N*/ 		{
/*?*/ 			pCurr->FinishSpaceAdd();
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		if ( pPos->InTxtGrp() )
/*N*/ 			nGluePortion += ((SwTxtPortion*)pPos)->GetSpaceCnt( GetInfo(), nCharCnt );
/*N*/ 		else if( pPos->IsMultiPortion() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*N*/ 		{
/*N*/ 
/*N*/ 		if( pPos->InGlueGrp() )
/*N*/ 		{
/*N*/ 			if( pPos->InFixMargGrp() )
/*N*/ 			{
/*N*/ 				if ( nSpaceIdx == pCurr->GetSpaceAdd().Count() )
/*N*/ 					pCurr->GetSpaceAdd().Insert( nNull, nSpaceIdx );
/*N*/ 				if( nGluePortion )
/*N*/ 				{
/*N*/ 					( pCurr->GetSpaceAdd() )[nSpaceIdx] =
/*N*/ 						 ( (SwGluePortion*)pPos )->GetPrtGlue()	/ nGluePortion;
/*N*/ 					pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
/*N*/ 				}
/*N*/ 				else if ( IsOneBlock() && nCharCnt > 1 )
/*N*/ 				{
/*?*/ 					( pCurr->GetSpaceAdd() )[nSpaceIdx] =
/*?*/ 						- ( (SwGluePortion*)pPos )->GetPrtGlue() / (nCharCnt-1);
/*?*/ 					pPos->Width( ( (SwGluePortion*)pPos )->GetFixWidth() );
/*N*/ 				}
/*N*/ 				nSpaceIdx++;
/*N*/ 				nGluePortion = 0;
/*N*/ 				nCharCnt = 0;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				++nGluePortion;
/*N*/ 		}
/*N*/ 		GetInfo().SetIdx( GetInfo().GetIdx() + pPos->GetLen() );
/*N*/ 		if ( pPos == pStopAt )
/*N*/ 		{
/*?*/ 			if ( nSpaceIdx == pCurr->GetSpaceAdd().Count() )
/*?*/ 				pCurr->GetSpaceAdd().Insert( nNull, nSpaceIdx );
/*?*/ 			else
/*?*/ 				pCurr->GetSpaceAdd()[nSpaceIdx] = 0;
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		pPos = pPos->GetPortion();
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *                    SwTxtAdjuster::CalcKanaAdj()
 *************************************************************************/


/*************************************************************************
 *                    SwTxtAdjuster::CalcRightMargin()
 *************************************************************************/

/*N*/ SwMarginPortion *SwTxtAdjuster::CalcRightMargin( SwLineLayout *pCurr,
/*N*/ 	SwTwips nReal )
/*N*/ {
/*N*/ 	long nRealWidth;
/*N*/     const USHORT nRealHeight = GetLineHeight();
/*N*/     const USHORT nLineHeight = pCurr->Height();
/*N*/ 
/*N*/ 	KSHORT nPrtWidth = pCurr->PrtWidth();
/*N*/ 	SwLinePortion *pLast = pCurr->FindLastPortion();
/*N*/ 
/*N*/ 	if( GetInfo().IsMulti() )
/*N*/ 		nRealWidth = nReal;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		nRealWidth = GetLineWidth();
/*N*/ 		// Fuer jeden FlyFrm, der in den rechten Rand hineinragt,
/*N*/ 		// wird eine FlyPortion angelegt.
/*N*/ 		const long nLeftMar = GetLeftMargin();
/*N*/         SwRect aCurrRect( nLeftMar + nPrtWidth, Y() + nRealHeight - nLineHeight,
/*N*/                           nRealWidth - nPrtWidth, nLineHeight );
/*N*/ 
/*N*/ 		SwFlyPortion *pFly = CalcFlyPortion( nRealWidth, aCurrRect );
/*N*/ 		while( pFly && long( nPrtWidth )< nRealWidth )
/*N*/ 		{
/*N*/ 			pLast->Append( pFly );
/*N*/ 			pLast = pFly;
/*N*/ 			if( pFly->Fix() > nPrtWidth )
/*?*/ 				pFly->Width( ( pFly->Fix() - nPrtWidth) + pFly->Width() + 1);
/*N*/ 			nPrtWidth += pFly->Width() + 1;
/*N*/ 			aCurrRect.Left( nLeftMar + nPrtWidth );
/*N*/ 			pFly = CalcFlyPortion( nRealWidth, aCurrRect );
/*N*/ 		}
/*N*/ 		if( pFly )
/*?*/ 			delete pFly;
/*N*/ 	}
/*N*/ 
/*N*/ 	SwMarginPortion *pRight = new SwMarginPortion( 0 );
/*N*/ 	pLast->Append( pRight );
/*N*/ 
/*N*/ 	if( long( nPrtWidth )< nRealWidth )
/*N*/ 		pRight->PrtWidth( KSHORT( nRealWidth - nPrtWidth ) );
/*N*/ 
/*N*/ 	// pCurr->Width() wird auf die reale Groesse gesetzt,
/*N*/ 	// da jetzt die MarginPortions eingehaengt sind.
/*N*/ 	// Dieser Trick hat wundersame Auswirkungen.
/*N*/ 	// Wenn pCurr->Width() == nRealWidth ist, dann wird das gesamte
/*N*/ 	// Adjustment implizit ausgecontert. GetLeftMarginAdjust() und
/*N*/ 	// IsBlocksatz() sind der Meinung, sie haetten eine mit Zeichen
/*N*/ 	// gefuellte Zeile.
/*N*/ 
/*N*/ 	pCurr->PrtWidth( KSHORT( nRealWidth ) );
/*N*/ 	return pRight;
/*N*/ }

/*************************************************************************
 *                    SwTxtAdjuster::CalcFlyAdjust()
 *************************************************************************/

/*N*/ void SwTxtAdjuster::CalcFlyAdjust( SwLineLayout *pCurr )
/*N*/ {
/*N*/ 	// 1) Es wird ein linker Rand eingefuegt:
/*N*/ 	SwMarginPortion *pLeft = pCurr->CalcLeftMargin();
/*N*/ 	SwGluePortion *pGlue = pLeft;       // die letzte GluePortion
/*N*/ 
/*N*/ 
/*N*/ 	// 2) Es wird ein rechter Rand angehaengt:
/*N*/ 	// CalcRightMargin berechnet auch eventuelle Ueberlappungen mit
/*N*/ 	// FlyFrms.
/*N*/ 	CalcRightMargin( pCurr );
/*N*/ 
/*N*/ 	SwLinePortion *pPos = pLeft->GetPortion();
/*N*/ 	xub_StrLen nLen = 0;
/*N*/ 
/*N*/ 	// Wenn wir nur eine Zeile vorliegen haben und die Textportion zusammen
/*N*/ 	// haengend ist und wenn zentriert wird, dann ...
/*N*/ 
/*N*/ 	sal_Bool bComplete = 0 == nStart;
/*N*/     const sal_Bool bTabCompat = GetTxtFrm()->GetNode()->GetDoc()->IsTabCompat();
/*N*/     sal_Bool bMultiTab = sal_False;
/*N*/ 
/*N*/ 	while( pPos )
/*N*/ 	{
/*N*/         if ( pPos->IsMultiPortion() && ((SwMultiPortion*)pPos)->HasTabulator() )
/*N*/             bMultiTab = sal_True;
/*N*/         else if( pPos->InFixMargGrp() &&
/*N*/                ( bTabCompat ? ! pPos->InTabGrp() : ! bMultiTab ) )
/*N*/         {
/*N*/             // in tab compat mode we do not want to change tab portions
/*N*/             // in non tab compat mode we do not want to change margins if we
/*N*/             // found a multi portion with tabs
/*N*/             if( SVX_ADJUST_RIGHT == GetAdjust() )
/*N*/                 ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
/*N*/             else
/*N*/             {
/*N*/                 // Eine schlaue Idee von MA:
/*N*/                 // Fuer die erste Textportion wird rechtsbuendig eingestellt,
/*N*/                 // fuer die letzte linksbuendig.
/*N*/ 
/*N*/                 // Die erste Textportion kriegt den ganzen Glue
/*N*/                 // Aber nur, wenn wir mehr als eine Zeile besitzen.
/*N*/                 if( bComplete && GetInfo().GetTxt().Len() == nLen )
/*N*/                     ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
/*N*/                 else
/*N*/                 {
/*N*/                     if ( ! bTabCompat )
/*N*/                     {
/*N*/                         if( pLeft == pGlue )
/*N*/                         {
/*N*/                             // Wenn es nur einen linken und rechten Rand gibt,
/*N*/                             // dann teilen sich die Raender den Glue.
/*N*/                             if( nLen + pPos->GetLen() >= pCurr->GetLen() )
/*N*/                                 ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
/*N*/                             else
/*?*/                                 ((SwGluePortion*)pPos)->MoveAllGlue( pGlue );
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             // Die letzte Textportion behaelt sein Glue
/*N*/                          if( !pPos->IsMarginPortion() )
/*?*/                               ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
/*N*/                          }
/*N*/                      }
/*N*/                      else
/*?*/                         ((SwGluePortion*)pPos)->MoveHalfGlue( pGlue );
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/             pGlue = (SwFlyPortion*)pPos;
/*N*/             bComplete = sal_False;
/*N*/         }
/*N*/ 		nLen += pPos->GetLen();
/*N*/ 		pPos = pPos->GetPortion();
/*N*/      }
/*N*/ 
/*N*/      if( ! bTabCompat && ! bMultiTab && SVX_ADJUST_RIGHT == GetAdjust() )
/*N*/         // portions are moved to the right if possible
/*N*/         pLeft->AdjustRight( pCurr );
/*N*/ }

/*************************************************************************
 *                  SwTxtAdjuster::CalcAdjLine()
 *************************************************************************/

/*N*/ void SwTxtAdjuster::CalcAdjLine( SwLineLayout *pCurr )
/*N*/ {
/*N*/ 	ASSERT( pCurr->IsFormatAdj(), "CalcAdjLine: Why?" );
/*N*/ 
/*N*/ 	pCurr->SetFormatAdj(sal_False);
/*N*/ 
/*N*/     SwParaPortion* pPara = GetInfo().GetParaPortion();
/*N*/ 
/*N*/     switch( GetAdjust() )
/*N*/ 	{
/*N*/ 		case SVX_ADJUST_RIGHT:
/*N*/ 		case SVX_ADJUST_CENTER:
/*N*/ 		{
/*N*/ 			CalcFlyAdjust( pCurr );
/*N*/             pPara->GetRepaint()->SetOfst( 0 );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SVX_ADJUST_BLOCK:
/*N*/ 		{
/*N*/ 			// 8311: In Zeilen mit LineBreaks gibt es keinen Blocksatz!
/*N*/ 			if( pCurr->GetLen() &&
/*N*/ 				CH_BREAK == GetInfo().GetChar( nStart + pCurr->GetLen() - 1 ) &&
/*N*/ 				!IsLastBlock() )
/*N*/ 			{
/*N*/ 				if( IsLastCenter() )
/*N*/ 				{
/*?*/ 					CalcFlyAdjust( pCurr );
/*?*/                     pPara->GetRepaint()->SetOfst( 0 );
/*?*/ 					break;
/*N*/ 				}
/*N*/ 				return;
/*N*/ 			}
/*N*/ 			FormatBlock();
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		default : return;
/*N*/ 	}
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
 /*
     if( OPTDBG( *pInf ) )
     {
         pCurr->DebugPortions( aDbstream, pInf->GetTxt(), nStart );
         if( GetHints() )
         {
             const SwpHints &rHt = *GetHints();
             aDbstream << rHt;
             SwAttrIter::Dump( aDbstream );
         }
     }
  */
/*N*/ #endif
/*N*/ }

/*************************************************************************
 *                    SwTxtAdjuster::CalcFlyPortion()
 *
 * Die Berechnung hat es in sich: nCurrWidth geibt die Breite _vor_ dem
 * aufaddieren des Wortes das noch auf die Zeile passt! Aus diesem Grund
 * stimmt die Breite der FlyPortion auch, wenn die Blockierungssituation
 * bFirstWord && !WORDFITS eintritt.
 *************************************************************************/

/*N*/ SwFlyPortion *SwTxtAdjuster::CalcFlyPortion( const long nRealWidth,
/*N*/ 											 const SwRect &rCurrRect )
/*N*/ {
/*N*/     SwTxtFly aTxtFly( GetTxtFrm() );
/*N*/ 
/*N*/ 	const KSHORT nCurrWidth = pCurr->PrtWidth();
/*N*/ 	SwFlyPortion *pFlyPortion = 0;
/*N*/ 
/*N*/     SwRect aLineVert( rCurrRect );
/*N*/     if ( GetTxtFrm()->IsRightToLeft() )
/*?*/         {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 GetTxtFrm()->SwitchLTRtoRTL( aLineVert );
/*N*/     if ( GetTxtFrm()->IsVertical() )
/*?*/         GetTxtFrm()->SwitchHorizontalToVertical( aLineVert );
/*N*/ 
/*N*/ 	// aFlyRect ist dokumentglobal !
/*N*/     SwRect aFlyRect( aTxtFly.GetFrm( aLineVert ) );
/*N*/ 
/*N*/     if ( GetTxtFrm()->IsRightToLeft() )
/*?*/         {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 GetTxtFrm()->SwitchRTLtoLTR( aFlyRect );
/*N*/     if ( GetTxtFrm()->IsVertical() )
/*?*/         {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 GetTxtFrm()->SwitchVerticalToHorizontal( aFlyRect );
/*N*/ 
/*N*/ 	// Wenn ein Frame ueberlappt, wird eine Portion eroeffnet.
/*N*/ 	if( aFlyRect.HasArea() )
/*N*/ 	{
/*N*/ 		// aLocal ist framelokal
/*?*/ 		SwRect aLocal( aFlyRect );
/*?*/ 		aLocal.Pos( aLocal.Left() - GetLeftMargin(), aLocal.Top() );
/*?*/         if( nCurrWidth > aLocal.Left() )
/*?*/ 			aLocal.Left( nCurrWidth );
/*?*/ 
/*?*/ 		// Wenn das Rechteck breiter als die Zeile ist, stutzen
/*?*/ 		// wir es ebenfalls zurecht.
/*?*/ 		KSHORT nLocalWidth = KSHORT( aLocal.Left() + aLocal.Width() );
/*?*/ 		if( nRealWidth < long( nLocalWidth ) )
/*?*/ 			aLocal.Width( nRealWidth - aLocal.Left() );
/*?*/ 		GetInfo().GetParaPortion()->SetFly( sal_True );
/*?*/ 		pFlyPortion = new SwFlyPortion( aLocal );
/*?*/ 		pFlyPortion->Height( KSHORT( rCurrRect.Height() ) );
/*?*/ 		// Die Width koennte kleiner sein als die FixWidth, daher:
/*?*/ 		pFlyPortion->AdjFixWidth();
/*N*/ 	}
/*N*/ 	return pFlyPortion;
/*N*/ }

/*************************************************************************
 *                SwTxtPainter::_CalcDropAdjust()
 *************************************************************************/

// 6721: Drops und Adjustment
// CalcDropAdjust wird ggf. am Ende von Format() gerufen.


/*************************************************************************
 *                SwTxtAdjuster::CalcDropRepaint()
 *************************************************************************/

/*N*/ void SwTxtAdjuster::CalcDropRepaint()
/*N*/ {
/*N*/ 	Top();
/*N*/ 	SwRepaint &rRepaint = *GetInfo().GetParaPortion()->GetRepaint();
/*N*/ 	if( rRepaint.Top() > Y() )
/*?*/ 		rRepaint.Top( Y() );
/*N*/ 	for( MSHORT i = 1; i < GetDropLines(); ++i )
/*N*/ 		NextLine();
/*N*/ 	const SwTwips nBottom = Y() + GetLineHeight() - 1;
/*N*/ 	if( rRepaint.Bottom() < nBottom )
/*?*/ 		rRepaint.Bottom( nBottom );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
