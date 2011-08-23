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

#include "paratr.hxx" 	// pTabStop, ADJ*


#include "txtcfg.hxx"
#include "porlay.hxx" 	// SwParaPortion, SetFull
#include "porfly.hxx" 	// SwParaPortion, SetFull
namespace binfilter {

/*************************************************************************
 *						class SwGluePortion
 *************************************************************************/

/*N*/ SwGluePortion::SwGluePortion( const KSHORT nInitFixWidth )
/*N*/ 	: nFixWidth( nInitFixWidth )
/*N*/ {
/*N*/ 	PrtWidth( nFixWidth );
/*N*/ 	SetWhichPor( POR_GLUE );
/*N*/ }

/*************************************************************************
 *				  virtual SwGluePortion::GetCrsrOfst()
 *************************************************************************/


/*************************************************************************
 *				  virtual SwGluePortion::GetTxtSize()
 *************************************************************************/


/*************************************************************************
 *				virtual SwGluePortion::GetExpTxt()
 *************************************************************************/


/*************************************************************************
 *				  virtual SwGluePortion::Paint()
 *************************************************************************/


/*************************************************************************
 *						SwGluePortion::MoveGlue()
 *************************************************************************/

/*N*/ void SwGluePortion::MoveGlue( SwGluePortion *pTarget, const short nPrtGlue )
/*N*/ {
/*N*/ 	short nPrt = Min( nPrtGlue, GetPrtGlue() );
/*N*/ 	if( 0 < nPrt )
/*N*/ 	{
/*N*/ 		pTarget->AddPrtWidth( nPrt );
/*N*/ 		SubPrtWidth( nPrt );
/*N*/ 	}
/*N*/ }

/*************************************************************************
 *				  void SwGluePortion::Join()
 *************************************************************************/

/*N*/ void SwGluePortion::Join( SwGluePortion *pVictim )
/*N*/ {
/*N*/ 	// Die GluePortion wird ausgesogen und weggespuelt ...
/*N*/ 	AddPrtWidth( pVictim->PrtWidth() );
/*N*/ 	SetLen( pVictim->GetLen() + GetLen() );
/*N*/ 	if( Height() < pVictim->Height() )
/*N*/ 		Height( pVictim->Height() );
/*N*/ 
/*N*/ 	AdjFixWidth();
/*N*/ 	Cut( pVictim );
/*N*/ 	delete pVictim;
/*N*/ }

/*************************************************************************
 *				  class SwFixPortion
 *************************************************************************/

// Wir erwarten ein framelokales SwRect !
/*N*/ SwFixPortion::SwFixPortion( const SwRect &rRect )
/*N*/ 	   :SwGluePortion( KSHORT(rRect.Width()) ), nFix( KSHORT(rRect.Left()) )
/*N*/ {
/*N*/ 	Height( KSHORT(rRect.Height()) );
/*N*/ 	SetWhichPor( POR_FIX );
/*N*/ }
/*N*/ 
/*N*/ SwFixPortion::SwFixPortion(const KSHORT nFixWidth, const KSHORT nFixPos)
/*N*/ 	   : SwGluePortion(nFixWidth), nFix(nFixPos)
/*N*/ {
/*N*/ 	SetWhichPor( POR_FIX );
/*N*/ }

/*************************************************************************
 *				  class SwMarginPortion
 *************************************************************************/

/*N*/ SwMarginPortion::SwMarginPortion( const KSHORT nFixWidth )
/*N*/ 	:SwGluePortion( nFixWidth )
/*N*/ {
/*N*/ 	SetWhichPor( POR_MARGIN );
/*N*/ }

/*************************************************************************
 *				  SwMarginPortion::AdjustRight()
 *
 * In der umschliessenden Schleife werden alle Portions durchsucht,
 * dabei werden erst die am Ende liegenden GluePortions verarbeitet.
 * Das Ende wird nach jeder Schleife nach vorne verlegt, bis keine
 * GluePortions mehr vorhanden sind.
 * Es werden immer GluePortion-Paare betrachtet (pLeft und pRight),
 * wobei Textportions zwischen pLeft und pRight hinter pRight verschoben
 * werden, wenn pRight genuegend Glue besitzt. Bei jeder Verschiebung
 * wandert ein Teil des Glues von pRight nach pLeft.
 * Im naechsten Schleifendurchlauf ist pLeft das pRight und das Spiel
 * beginnt von vorne.
 *************************************************************************/

/*N*/ void SwMarginPortion::AdjustRight( const SwLineLayout *pCurr )
/*N*/ {
/*N*/ 	SwGluePortion *pRight = 0;
/*N*/ 	BOOL bNoMove = 0 != pCurr->GetpKanaComp();
/*N*/ 	while( pRight != this )
/*N*/ 	{
/*N*/ 
/*N*/ 		// 1) Wir suchen den linken Glue
/*N*/ 		SwLinePortion *pPos = (SwLinePortion*)this;
/*N*/ 		SwGluePortion *pLeft = 0;
/*N*/ 		while( pPos )
/*N*/ 		{
///*N*/ 			DBG_LOOP;
/*N*/ 			if( pPos->InFixMargGrp() )
/*N*/ 				pLeft = (SwGluePortion*)pPos;
/*N*/ 			pPos = pPos->GetPortion();
/*N*/ 			if( pPos == pRight)
/*N*/ 				pPos = 0;
/*N*/ 		}
/*N*/ 
/*N*/ 		// Zwei nebeneinander liegende FlyPortions verschmelzen
/*N*/ 		if( pRight && pLeft->GetPortion() == pRight )
/*N*/ 		{
/*?*/ 			pRight->MoveAllGlue( pLeft );
/*?*/ 			pRight = 0;
/*N*/ 		}
/*N*/ 		KSHORT nRightGlue = pRight && 0 < pRight->GetPrtGlue()
/*N*/ 						  ? KSHORT(pRight->GetPrtGlue()) : 0;
/*N*/ 		// 2) linken und rechten Glue ausgleichen
/*N*/ 		//	  Bei Tabs haengen wir nix um ...
/*N*/ 		if( pLeft && nRightGlue && !pRight->InTabGrp() )
/*N*/ 		{
/*?*/ 			// pPrev ist die Portion, die unmittelbar vor pRight liegt.
/*?*/ 			SwLinePortion *pPrev = pRight->FindPrevPortion( pLeft );
/*?*/ 
/*?*/ 			if ( pRight->IsFlyPortion() && pRight->GetLen() )
/*?*/ 			{
/*?*/ 				SwFlyPortion *pFly = (SwFlyPortion *)pRight;
/*?*/ 				if ( pFly->GetBlankWidth() < nRightGlue )
/*?*/ 				{
/*?*/ 					// Hier entsteht eine neue TxtPortion, die dass zuvor
/*?*/ 					// vom Fly verschluckte Blank reaktiviert.
/*?*/ 					nRightGlue -= pFly->GetBlankWidth();
/*?*/ 					pFly->SubPrtWidth( pFly->GetBlankWidth() );
/*?*/ 					pFly->SetLen( 0 );
/*?*/ 					SwTxtPortion *pNewPor = new SwTxtPortion;
/*?*/ 					pNewPor->SetLen( 1 );
/*?*/ 					pNewPor->Height( pFly->Height() );
/*?*/ 					pNewPor->Width( pFly->GetBlankWidth() );
/*?*/ 					pFly->Insert( pNewPor );
/*?*/ 				}
/*?*/ 				else
/*?*/ 					pPrev = pLeft;
/*?*/ 			}
/*?*/ 			while( pPrev != pLeft )
/*?*/ 			{
///*?*/ 				DBG_LOOP;
/*?*/ 
/*?*/ 				if( bNoMove || pPrev->PrtWidth() >= nRightGlue ||
/*?*/ 					pPrev->InHyphGrp() || pPrev->IsKernPortion() )
/*?*/ 				{
/*?*/ 					// Die Portion, die vor pRight liegt kann nicht
/*?*/ 					// verschoben werden, weil kein Glue mehr vorhanden ist.
/*?*/ 					// Wir fuehren die Abbruchbedingung herbei:
/*?*/ 					pPrev = pLeft;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					nRightGlue -= pPrev->PrtWidth();
/*?*/ 					// pPrev wird hinter pRight verschoben.
/*?*/ 					// Dazu wird der Gluewert zwischen pRight und pLeft
/*?*/ 					// ausgeglichen.
/*?*/ 					pRight->MoveGlue( pLeft, short( pPrev->PrtWidth() ) );
/*?*/ 					// Jetzt wird die Verkettung gerichtet.
/*?*/ 					SwLinePortion *pPrevPrev = pPrev->FindPrevPortion( pLeft );
/*?*/ 					pPrevPrev->SetPortion( pRight );
/*?*/ 					pPrev->SetPortion( pRight->GetPortion() );
/*?*/ 					pRight->SetPortion( pPrev );
/*?*/ 					if ( pPrev->GetPortion() && pPrev->InTxtGrp()
/*?*/ 						 && pPrev->GetPortion()->IsHolePortion() )
/*?*/ 					{
/*?*/ 						SwHolePortion *pHolePor =
/*?*/ 							(SwHolePortion*)pPrev->GetPortion();
/*?*/ 						if ( !pHolePor->GetPortion() ||
/*?*/ 							 !pHolePor->GetPortion()->InFixMargGrp() )
/*?*/ 						{
/*?*/ 							pPrev->AddPrtWidth( pHolePor->GetBlankWidth() );
/*?*/ 							pPrev->SetLen( pPrev->GetLen() + 1 );
/*?*/ 							pPrev->SetPortion( pHolePor->GetPortion() );
/*?*/ 							delete pHolePor;
/*?*/ 						}
/*?*/ 					}
/*?*/ 					pPrev = pPrevPrev;
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		// Wenn es keinen linken Glue mehr gibt, wird die Abbruchbedingung
/*N*/ 		// herbeigefuehrt.
/*N*/ 		pRight = pLeft ? pLeft : (SwGluePortion*)this;
/*N*/ 	}
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
