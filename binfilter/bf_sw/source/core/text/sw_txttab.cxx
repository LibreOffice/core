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

#include "hintids.hxx"

#include <bf_svx/lrspitem.hxx>
#include <bf_svx/tstpitem.hxx>

#include <frmatr.hxx>

#include "txtcfg.hxx"
#include "portab.hxx"
#include "itrform2.hxx"
namespace binfilter {


/*************************************************************************
 *                    SwLineInfo::GetTabStop()
 *************************************************************************/

/* Die Werte in SvxTabStop::nTabPos liegen immer relativ zum linken PrtRand
 * vor. Tabs, die im Bereich des Erstzeileneinzugs liegen, sind also negativ.
 * nLeft  ist der linke PrtRand
 * nRight ist der rechte PrtRand
 * nLinePos die aktuelle Position.
 * Es wird der erste Tabstop returnt, der groesser ist als nLinePos.
 */



/*N*/ const SvxTabStop *SwLineInfo::GetTabStop( const SwTwips nLinePos,
/*N*/ 	const SwTwips nLeft, const SwTwips nRight ) const
/*N*/ {
/*N*/ 	// Mit den KSHORTs aufpassen, falls nLinePos < nLeft
/*N*/ 	SwTwips nPos = nLinePos;
/*N*/ 	nPos -= nLeft;
/*N*/ 	for( MSHORT i = 0; i < pRuler->Count(); ++i )
/*N*/ 	{
/*N*/ 		const SvxTabStop &rTabStop = pRuler->operator[](i);
/*N*/ 		if( rTabStop.GetTabPos() > SwTwips(nRight) )
/*N*/ 		{
/*N*/ 			if ( i )
/*N*/ 				return 0;
/*N*/ 			else
/*N*/ 				return &rTabStop;
/*N*/ 		}
/*N*/ 		if( rTabStop.GetTabPos() > nPos )
/*N*/ 			return &rTabStop;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*************************************************************************
 *                      SwTxtFormatter::NewTabPortion()
 *************************************************************************/



/*N*/ SwTabPortion *SwTxtFormatter::NewTabPortion( SwTxtFormatInfo &rInf ) const
/*N*/ {
/*N*/ 	SwTabPortion *pTabPor;
/*N*/ 	SwTabPortion  *pLastTab = rInf.GetLastTab();
/*N*/ 	if( pLastTab && pLastTab->IsTabCntPortion() )
/*N*/ 		if( pLastTab->PostFormat( rInf ) )
/*N*/ 			return 0;
/*N*/ 
/*N*/ 	// Wir suchen den naechsten Tab. Wenn gerade ein rechts-Tab unterwegs
/*N*/ 	// ist, so koennen wir uns nicht auf rInf.X() beziehen.
/*N*/ 	KSHORT nTabPos = rInf.GetLastTab() ? rInf.GetLastTab()->GetTabPos() : 0;
/*N*/ 	if( nTabPos < rInf.X() )
/*N*/ 		nTabPos = rInf.X();
/*N*/ 
/*N*/ 	xub_Unicode cFill, cDec;
/*N*/ 	SvxTabAdjust eAdj;
/*N*/ 
/*N*/ 	KSHORT nNewTabPos;
/*N*/ 	{
        /*
         nPos ist der Offset in der Zeile.
         Die Tabulatoren haben ihren 0-Punkt bei Frm().Left().
         Die Zeilen beginnen ab Frm.Left() + Prt.Left().
         In dieser Methode wird zwischen beiden Koordinatensystemen
         konvertiert (vgl. rInf.GetTabPos).
        */
/*N*/ 		const SwTwips nTabLeft = pFrm->Frm().Left() +
/*N*/                                  ( pFrm->IsRightToLeft() ?
/*N*/                                    pFrm->GetAttrSet()->GetLRSpace().GetRight() :
/*N*/                                    pFrm->GetAttrSet()->GetLRSpace().GetTxtLeft() );
/*N*/ 
/*N*/ 		const SwTwips nLinePos = GetLeftMargin();
/*N*/ 		const SwTwips nLineTab = nLinePos + nTabPos;
/*N*/         SwTwips nRight = Right();
/*N*/ 
/*N*/         if ( pFrm->IsVertical() )
/*N*/         {
/*N*/             Point aRightTop( nRight, pFrm->Frm().Top() );
/*N*/             pFrm->SwitchHorizontalToVertical( aRightTop );
/*N*/             nRight = aRightTop.Y();
/*N*/         }
/*N*/ 
/*N*/ 		SwTwips nNextPos;
/*N*/ 		const SvxTabStop* pTabStop =
/*N*/ 			aLineInf.GetTabStop( nLineTab, nTabLeft, nRight );
/*N*/ 		if( pTabStop )
/*N*/ 		{
/*N*/ 			cFill = ' ' != pTabStop->GetFill() ? pTabStop->GetFill() : 0;
/*N*/ 			cDec = pTabStop->GetDecimal();
/*N*/ 			eAdj = pTabStop->GetAdjustment();
/*N*/ 			nNextPos = pTabStop->GetTabPos();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			KSHORT nDefTabDist = aLineInf.GetDefTabStop();
/*N*/ 			if( KSHRT_MAX == nDefTabDist )
/*N*/ 			{
/*N*/ 				const SvxTabStopItem& rTab =
/*N*/ 					(const SvxTabStopItem &)pFrm->GetAttrSet()->
/*N*/ 					GetPool()->GetDefaultItem( RES_PARATR_TABSTOP );
/*N*/ 				if( rTab.Count() )
/*N*/ 					nDefTabDist = (KSHORT)rTab.GetStart()->GetTabPos();
/*N*/ 				else
/*N*/ 					nDefTabDist = SVX_TAB_DEFDIST;
/*N*/ 				aLineInf.SetDefTabStop( nDefTabDist );
/*N*/ 			}
/*N*/ 			SwTwips	nCount = nLineTab;
/*N*/ 			nCount -= nTabLeft;
/*N*/ 			// Bei negativen Werten rundet "/" auf, "%" liefert negative Reste,
/*N*/ 			// bei positiven Werten rundet "/" ab, "%" liefert positvie Reste!
/*N*/ 			KSHORT nPlus = nCount < 0 ? 0 : 1;
/*N*/ 			nCount /= nDefTabDist;
/*N*/ 			nNextPos = ( nCount + nPlus ) * nDefTabDist ;
/*N*/ 			if( nNextPos + nTabLeft <= nLineTab + 50 )
/*N*/ 				nNextPos += nDefTabDist;
/*N*/ 			cFill = 0;
/*N*/ 			eAdj = SVX_TAB_ADJUST_LEFT;
/*N*/ 		}
/*N*/ 		long nForced = 0;
/*N*/ 		if( pCurr->HasForcedLeftMargin() )
/*N*/ 		{
/*?*/ 			SwLinePortion* pPor = pCurr->GetPortion();
/*?*/ 			while( pPor && !pPor->IsFlyPortion() )
/*?*/ 				pPor = pPor->GetPortion();
/*?*/ 			if( pPor )
/*?*/ 				nForced = pPor->Width();
/*N*/ 		}
/*N*/ 		if( nTabLeft + nForced > nLineTab && nNextPos > 0 )
/*N*/ 		{
/*N*/ 			eAdj = SVX_TAB_ADJUST_DEFAULT;
/*N*/ 			cFill = 0;
/*N*/ 			nNextPos = nForced;
/*N*/ 		}
/*N*/ 		nNextPos += nTabLeft;
/*N*/ 		nNextPos -= nLinePos;
/*N*/ 		ASSERT( nNextPos >= 0, "GetTabStop: Don't go back!" );
/*N*/ 		nNewTabPos = KSHORT(nNextPos);
/*N*/ 	}
/*N*/ 
/*N*/ 	switch( eAdj )
/*N*/ 	{
/*N*/ 		case SVX_TAB_ADJUST_RIGHT :
/*N*/ 		{
/*N*/ 			pTabPor = new SwTabRightPortion( nNewTabPos, cFill );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SVX_TAB_ADJUST_CENTER :
/*N*/ 		{
/*N*/ 			pTabPor = new SwTabCenterPortion( nNewTabPos, cFill );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 		case SVX_TAB_ADJUST_DECIMAL :
/*N*/ 		{
/*?*/ 			pTabPor = new SwTabDecimalPortion( nNewTabPos, cDec, cFill );
/*?*/ 			break;
/*N*/ 		}
/*N*/ 		default:
/*N*/ 		{
/*N*/ 			ASSERT( SVX_TAB_ADJUST_LEFT == eAdj || SVX_TAB_ADJUST_DEFAULT == eAdj,
/*N*/ 					"+SwTxtFormatter::NewTabPortion: unknown adjustment" );
/*N*/ 			pTabPor = new SwTabLeftPortion( nNewTabPos, cFill );
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	// Vorhandensein von Tabulatoren anzeigen ... ist nicht mehr noetig
/*N*/ 	// pCurr->SetTabulation();
/*N*/ 	// Aus Sicherheitsgruenden lassen wir uns die Daten errechnen
/*N*/ 	// pTabPor->Height( pLast->Height() );
/*N*/ 	// pTabPor->SetAscent( pLast->GetAscent() );
/*N*/ 	return pTabPor;
/*N*/ }

/*************************************************************************
 *                SwTabPortion::SwTabPortion()
 *************************************************************************/

// Die Basisklasse wird erstmal ohne alles initialisiert.


/*N*/ SwTabPortion::SwTabPortion( const KSHORT nTabPos, const xub_Unicode cFill )
/*N*/ 	: SwFixPortion( 0, 0 ), nTabPos(nTabPos), cFill(cFill)
/*N*/ {
/*N*/ 	nLineLength = 1;
/*N*/ #ifdef DBG_UTIL
/*N*/ 	if( IsFilled() )
/*N*/ 	{
/*N*/ 		ASSERT( ' ' != cFill, "SwTabPortion::CTOR: blanks ?!" );
/*N*/ 	}
/*N*/ #endif
/*N*/ 	SetWhichPor( POR_TAB );
/*N*/ }

/*************************************************************************
 *                 virtual SwTabPortion::Format()
 *************************************************************************/



/*N*/ sal_Bool SwTabPortion::Format( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	SwTabPortion *pLastTab = rInf.GetLastTab();
/*N*/ 	if( pLastTab == this )
/*?*/ 		return PostFormat( rInf );
/*N*/ 	if( pLastTab )
/*?*/ 		pLastTab->PostFormat( rInf );
/*N*/ 	return PreFormat( rInf );
/*N*/ }

/*************************************************************************
 *                 virtual SwTabPortion::FormatEOL()
 *************************************************************************/



/*N*/ void SwTabPortion::FormatEOL( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	if( rInf.GetLastTab() == this && !IsTabLeftPortion() )
/*N*/ 		PostFormat( rInf );
/*N*/ }

/*************************************************************************
 *                    SwTabPortion::PreFormat()
 *************************************************************************/



/*M*/ sal_Bool SwTabPortion::PreFormat( SwTxtFormatInfo &rInf )
/*M*/ {
/*M*/ 	ASSERT( rInf.X() <= GetTabPos(), "SwTabPortion::PreFormat: rush hour" );
/*M*/ 
/*M*/ 	// Hier lassen wir uns nieder...
/*M*/ 	Fix( rInf.X() );
/*M*/ 
/*M*/ 	// Die Mindestbreite eines Tabs ist immer mindestens ein Blank
/*M*/ 	{
/*M*/ 		XubString aTmp( ' ' );
/*M*/ 		SwTxtSizeInfo aInf( rInf, aTmp );
/*M*/ 		PrtWidth( aInf.GetTxtSize().Width() );
/*M*/ 	}
/*M*/ 
/*M*/ 	// 8532: CenterTabs, deren Blankbreite nicht mehr in die Zeile passt
/*M*/ 	sal_Bool bFull = rInf.Width() <= rInf.X() + PrtWidth();
/*M*/ 
/*M*/     // #95477# Rotated tab stops get the width of one blank
/*N*/     const USHORT nDir = rInf.GetFont()->GetOrientation( rInf.GetTxtFrm()->IsVertical() );
/*M*/ 
/*M*/     if( ! bFull && 0 == nDir )
/*M*/ 	{
/*M*/ 		const MSHORT nWhich = GetWhichPor();
/*M*/ 		switch( nWhich )
/*M*/ 		{
/*M*/ 			case POR_TABRIGHT:
/*M*/ 			case POR_TABDECIMAL:
/*M*/ 			case POR_TABCENTER:
/*M*/ 			{
/*M*/ 				if( POR_TABDECIMAL == nWhich )
/*M*/ 					rInf.SetTabDecimal(
/*M*/ 						((SwTabDecimalPortion*)this)->GetTabDecimal());
/*M*/ 				rInf.SetLastTab( this );
/*M*/ 				break;
/*M*/ 			}
/*M*/ 			case POR_TABLEFT:
/*M*/ 			{
/*M*/                 PrtWidth( GetTabPos() - rInf.X() );
/*M*/ 				bFull = rInf.Width() <= rInf.X() + PrtWidth();
/*M*/ 				break;
/*M*/ 			}
/*M*/ 			default: ASSERT( !this, "SwTabPortion::PreFormat: unknown adjustment" );
/*M*/ 		}
/*M*/ 	}
/*M*/ 
/*M*/ 	if( bFull )
/*M*/ 	{
/*M*/ 		// Wir muessen aufpassen, dass wir nicht endlos schleifen,
/*M*/ 		// wenn die Breite kleiner ist, als ein Blank ...
/*M*/ 		if( rInf.GetIdx() == rInf.GetLineStart() )
/*M*/ 		{
/*M*/ 			PrtWidth( rInf.Width() - rInf.X() );
/*M*/ 			SetFixWidth( PrtWidth() );
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{
/*M*/ 			Height( 0 );
/*M*/ 			Width( 0 );
/*M*/ 			SetLen( 0 );
/*M*/ 			SetAscent( 0 );
/*M*/ 			SetPortion( NULL ); //?????
/*M*/ 		}
/*M*/ 		return sal_True;
/*M*/ 	}
/*M*/ 	else
/*M*/ 	{
/*M*/ 		// Ein Kunstgriff mit Effekt: Die neuen Tabportions verhalten sich nun
/*M*/ 		// so, wie FlyFrms, die in der Zeile stehen - inklusive Adjustment !
/*M*/ 		SetFixWidth( PrtWidth() );
/*M*/ 		return sal_False;
/*M*/ 	}
/*M*/ }

/*************************************************************************
 *                      SwTabPortion::PostFormat()
 *************************************************************************/



/*N*/ sal_Bool SwTabPortion::PostFormat( SwTxtFormatInfo &rInf )
/*N*/ {
/*N*/ 	const KSHORT nRight = Min( GetTabPos(), rInf.Width() );
/*N*/ 	const SwLinePortion *pPor = GetPortion();
/*N*/ 	KSHORT nPorWidth = 0;
/*N*/ 	while( pPor )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		nPorWidth += pPor->Width();
/*N*/ 		pPor = pPor->GetPortion();
/*N*/ 	}
/*N*/ 
/*N*/ 	const MSHORT nWhich = GetWhichPor();
/*N*/ 	ASSERT( POR_TABLEFT != nWhich, "SwTabPortion::PostFormat: already formatted" );
/*N*/ 	const KSHORT nDiffWidth = nRight - Fix();
/*N*/ 
/*N*/ 	if( POR_TABCENTER == nWhich )
/*N*/ 	{
/*N*/ 		// zentrierte Tabs bereiten Probleme:
/*N*/ 		// Wir muessen den Anteil herausfinden, der noch auf die Zeile passt.
/*N*/ 		KSHORT nNewWidth = nPorWidth /2;
/*N*/ 		if( nNewWidth > rInf.Width() - nRight )
/*?*/ 			nNewWidth = nPorWidth - (rInf.Width() - nRight);
/*N*/ 		nPorWidth = nNewWidth;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( nDiffWidth > nPorWidth )
/*N*/ 	{
/*N*/ 		const KSHORT nOldWidth = GetFixWidth();
/*N*/ 		const KSHORT nAdjDiff = nDiffWidth - nPorWidth;
/*N*/ 		if( nAdjDiff > GetFixWidth() )
/*N*/ 			PrtWidth( nAdjDiff );
/*N*/ 		// Nicht erschrecken: wir muessen rInf weiterschieben.
/*N*/ 		// Immerhin waren wir als Rechtstab bislang nur ein Blank breit.
/*N*/ 		// Da wir uns jetzt aufgespannt haben, muss der Differenzbetrag
/*N*/ 		// auf rInf.X() addiert werden !
/*N*/ 		rInf.X( rInf.X() + PrtWidth() - nOldWidth );
/*N*/ 	}
/*N*/ 	SetFixWidth( PrtWidth() );
/*N*/ 	// letzte Werte zuruecksetzen
/*N*/ 	rInf.SetLastTab(0);
/*N*/ 	if( POR_TABDECIMAL == nWhich )
/*?*/ 		rInf.SetTabDecimal(0);
/*N*/ 
/*N*/ 	return rInf.Width() <= rInf.X();
/*N*/ }

/*************************************************************************
 *                virtual SwTabPortion::Paint()
 *
 * Ex: LineIter::DrawTab()
 *************************************************************************/




/*************************************************************************
 *              virtual SwTabPortion::HandlePortion()
 *************************************************************************/


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
