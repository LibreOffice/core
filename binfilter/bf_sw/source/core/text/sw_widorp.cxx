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


#include "ftnboss.hxx"
#include "paratr.hxx"
#ifdef DBG_UTIL
#endif

#include <bf_svx/orphitem.hxx>
#include <bf_svx/widwitem.hxx>
#include <bf_svx/keepitem.hxx>
#include <bf_svx/spltitem.hxx>
#include <frmatr.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>

#include "txtcfg.hxx"
#include "widorp.hxx"
#include "sectfrm.hxx"	//SwSectionFrm
#include "ftnfrm.hxx"
namespace binfilter {

#undef WIDOWTWIPS


/*************************************************************************
 *					inline IsNastyFollow()
 *************************************************************************/
// Ein Follow, der auf der selben Seite steht, wie sein Master ist nasty.
/*N*/ inline sal_Bool IsNastyFollow( const SwTxtFrm *pFrm )
/*N*/ {
/*N*/ 	ASSERT(	!pFrm->IsFollow() || !pFrm->GetPrev() ||
/*N*/ 			((const SwTxtFrm*)pFrm->GetPrev())->GetFollow() == pFrm,
/*N*/ 			"IsNastyFollow: Was ist denn hier los?" );
/*N*/ 	return	pFrm->IsFollow() && pFrm->GetPrev();
/*N*/ }

/*************************************************************************
 *					SwTxtFrmBreak::SwTxtFrmBreak()
 *************************************************************************/

/*N*/ SwTxtFrmBreak::SwTxtFrmBreak( SwTxtFrm *pFrm, const SwTwips nRst )
/*N*/     : pFrm(pFrm), nRstHeight(nRst)
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( pFrm )
/*N*/     SWRECTFN( pFrm )
/*N*/     nOrigin = (pFrm->*fnRect->fnGetPrtTop)();
/*N*/     SwSectionFrm* pSct;
/*N*/ 	bKeep = !pFrm->IsMoveable() || IsNastyFollow( pFrm ) ||
/*N*/ 			( pFrm->IsInSct() && (pSct=pFrm->FindSctFrm())->Lower()->IsColumnFrm()
/*N*/ 			  && !pSct->MoveAllowed( pFrm ) ) ||
/*N*/ 			!pFrm->GetTxtNode()->GetSwAttrSet().GetSplit().GetValue() ||
/*N*/ 			pFrm->GetTxtNode()->GetSwAttrSet().GetKeep().GetValue();
/*N*/ 	bBreak = sal_False;
/*N*/ 
/*N*/ 	if( !nRstHeight && !pFrm->IsFollow() && pFrm->IsInFtn() && pFrm->HasPara() )
/*N*/ 	{
/*N*/ 		nRstHeight = pFrm->GetFtnFrmHeight();
/*N*/         nRstHeight += (pFrm->Prt().*fnRect->fnGetHeight)() -
/*N*/                       (pFrm->Frm().*fnRect->fnGetHeight)();
/*N*/ 		if( nRstHeight < 0 )
/*N*/ 			nRstHeight = 0;
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( pFrm )
/*N*/ }

/* BP 18.6.93: Widows.
 * Im Gegensatz zur ersten Implementierung werden die Widows nicht
 * mehr vorausschauend berechnet, sondern erst beim Formatieren des
 * gesplitteten Follows festgestellt. Im Master faellt die Widows-
 * Berechnung also generell weg (nWidows wird manipuliert).
 * Wenn der Follow feststellt, dass die Widowsregel zutrifft,
 * verschickt er an seinen Vorgaenger ein Prepare.
 * Ein besonderes Problem ergibt sich, wenn die Widows zuschlagen,
 * aber im Master noch ein paar Zeilen zur Verfuegung stehen.
 *
 */

/*************************************************************************
 *					SwTxtFrmBreak::IsInside()
 *************************************************************************/

/* BP(22.07.92): Berechnung von Witwen und Waisen.
 * Die Methode liefert sal_True zurueck, wenn eine dieser Regelung zutrifft.
 *
 * Eine Schwierigkeit gibt es im Zusammenhang mit Widows und
 * unterschiedlichen Formaten zwischen Master- und Folgeframes:
 * Beispiel: Wenn die erste Spalte 3cm und die zweite 4cm breit ist
 * und Widows auf sagen wir 3 gesetzt ist, so ist erst bei der Formatierung
 * des Follows entscheidbar, ob die Widowsbedingung einhaltbar ist oder
 * nicht. Leider ist davon abhaengig, ob der Absatz als Ganzes auf die
 * naechste Seite rutscht.
 */

/*N*/ sal_Bool SwTxtFrmBreak::IsInside( SwTxtMargin &rLine ) const
/*N*/ {
/*N*/ 	register sal_Bool bFit = sal_False;
/*N*/ 
/*N*/     SWAP_IF_SWAPPED( pFrm )
/*N*/     SWRECTFN( pFrm )
/*N*/     // nOrigin is an absolut value, rLine referes to the swapped situation.
/*N*/ 
/*N*/     SwTwips nTmpY;
/*N*/     if ( pFrm->IsVertical() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/         nTmpY = pFrm->SwitchHorizontalToVertical( rLine.Y() + rLine.GetLineHeight() );
/*N*/     else
/*N*/         nTmpY = rLine.Y() + rLine.GetLineHeight();
/*N*/ 
/*N*/     SwTwips nLineHeight = (*fnRect->fnYDiff)( nTmpY , nOrigin );
/*N*/ 
/*N*/ 	// 7455 und 6114: Raum fuer die Umrandung unten einkalkulieren.
/*N*/     nLineHeight += (pFrm->*fnRect->fnGetBottomMargin)();
/*N*/ 
/*N*/ 
/*N*/ 	if( nRstHeight )
/*N*/ 		bFit = nRstHeight >= nLineHeight;
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// Der Frm besitzt eine Hoehe, mit der er auf die Seite passt.
/*N*/         SwTwips nHeight =
/*N*/             (*fnRect->fnYDiff)( (pFrm->GetUpper()->*fnRect->fnGetPrtBottom)(), nOrigin );
/*N*/ 
/*N*/ 		// Wenn sich alles innerhalb des bestehenden Frames abspielt,
/*N*/ 		// ist das Ergebnis sal_True;
/*N*/ 		bFit = nHeight >= nLineHeight;
/*N*/ 		if( !bFit )
/*N*/ 		{
/*N*/ 			// Die LineHeight sprengt die aktuelle Frm-Hoehe.
/*N*/ 			// Nun rufen wir ein Probe-Grow, um zu ermitteln, ob der
/*N*/ 			// Frame um den gewuenschten Bereich wachsen wuerde.
/*N*/ 			nHeight += pFrm->GrowTst( LONG_MAX );
/*N*/ 
/*N*/ 			// Das Grow() returnt die Hoehe, um die der Upper des TxtFrm
/*N*/ 			// den TxtFrm wachsen lassen wuerde.
/*N*/ 			// Der TxtFrm selbst darf wachsen wie er will.
/*N*/ 			bFit = nHeight >= nLineHeight;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( pFrm );
/*N*/ 
/*N*/     return bFit;
/*N*/ }

/*************************************************************************
 *					SwTxtFrmBreak::IsBreakNow()
 *************************************************************************/

/*N*/ sal_Bool SwTxtFrmBreak::IsBreakNow( SwTxtMargin &rLine )
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( pFrm )
/*N*/ 
/*N*/ 	// bKeep ist staerker als IsBreakNow()
/*N*/ 	// Ist noch genug Platz ?
/*N*/ 	if( bKeep || IsInside( rLine ) )
/*N*/ 		bBreak = sal_False;
/*N*/ 	else
/*N*/ 	{
         /* Diese Klasse geht davon aus, dass der SwTxtMargin von Top nach Bottom
          * durchgearbeitet wird. Aus Performancegruenden wird in folgenden
          * Faellen der Laden fuer das weitere Aufspalten dicht gemacht:
          * Wenn eine einzige Zeile nicht mehr passt.
          * Sonderfall: bei DummyPortions ist LineNr == 1, obwohl wir splitten
          * wollen.
          */
/*N*/ 		// 6010: DropLines mit einbeziehen
/*N*/ 
/*N*/ 		sal_Bool bFirstLine = 1 == rLine.GetLineNr() && !rLine.GetPrev();
/*N*/ 		bBreak = sal_True;
/*N*/ 		if( ( bFirstLine && pFrm->GetIndPrev() )
/*N*/ 			|| ( rLine.GetLineNr() <= rLine.GetDropLines() ) )
/*N*/ 		{
/*N*/ 			bKeep = sal_True;
/*N*/ 			bBreak = sal_False;
/*N*/ 		}
/*N*/ 		else if(bFirstLine && pFrm->IsInFtn() && !pFrm->FindFtnFrm()->GetPrev())
/*N*/ 		{
/*?*/ 			SwLayoutFrm* pTmp = pFrm->FindFtnBossFrm()->FindBodyCont();
/*?*/ 			if( !pTmp || !pTmp->Lower() )
/*?*/ 				bBreak = sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( pFrm )
/*N*/ 
/*N*/ 	return bBreak;
/*N*/ }


/*MA ehemals fuer COMPACT
// WouldFit() liefert sal_True, wenn der Absatz ganz oder teilweise passen wuerde

sal_Bool SwTxtFrmBreak::WouldFit( SwTxtMargin &rLine )
{
    rLine.Bottom();
    if( IsInside( rLine ) )
        return sal_True;

    rLine.Top();
    // Suche die erste Trennmoeglichkeit ...
    while( !IsBreakNow( rLine ) )
    {
        DBG_LOOP;
        if( !rLine.NextLine() )
            return sal_False;
    }
    return sal_True;
}
*/

/*************************************************************************
 *					WidowsAndOrphans::WidowsAndOrphans()
 *************************************************************************/

/*N*/ WidowsAndOrphans::WidowsAndOrphans( SwTxtFrm *pFrm, const SwTwips nRst,
/*N*/ 	sal_Bool bChkKeep	)
/*N*/ 	: SwTxtFrmBreak( pFrm, nRst ), nOrphLines( 0 ), nWidLines( 0 )
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( pFrm )
/*N*/     SWRECTFN( pFrm )
/*N*/ 
/*N*/ 	if( bKeep )
/*N*/ 	{
/*N*/ 		// 5652: bei Absaetzen, die zusammengehalten werden sollen und
/*N*/ 		// groesser sind als die Seite wird bKeep aufgehoben.
/*N*/ 		if( bChkKeep && !pFrm->GetPrev() && !pFrm->IsInFtn() &&
/*N*/ 			pFrm->IsMoveable() &&
/*N*/ 			( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
/*N*/ 			bKeep = sal_False;
/*N*/ 		//Auch bei gesetztem Keep muessen Orphans beachtet werden,
/*N*/ 		//z.B. bei verketteten Rahmen erhaelt ein Follow im letzten Rahmen ein Keep,
/*N*/ 		//da er nicht (vorwaerts) Moveable ist,
/*N*/ 		//er darf aber trotzdem vom Master Zeilen anfordern wg. der Orphanregel.
/*N*/ 		if( pFrm->IsFollow() )
/*N*/ 			nWidLines = pFrm->GetTxtNode()->GetSwAttrSet().GetWidows().GetValue();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
/*N*/ 		const SvxOrphansItem  &rOrph = rSet.GetOrphans();
/*N*/ 		if ( rOrph.GetValue() > 1 )
/*N*/ 			nOrphLines = rOrph.GetValue();
/*N*/ 		if ( pFrm->IsFollow() )
/*N*/ 			nWidLines = rSet.GetWidows().GetValue();
/*N*/ 
/*N*/ 	}
/*N*/ 	if( pFrm->IsInFtn() && !pFrm->GetIndPrev() &&
/*N*/ 		( bKeep || nWidLines || nOrphLines ) )
/*N*/ 	{
/*N*/ 		// Innerhalb von Fussnoten gibt es gute Gruende, das Keep-Attribut und
/*N*/ 		// die Widows/Orphans abzuschalten.
/*?*/ 		SwFtnFrm *pFtn = pFrm->FindFtnFrm();
/*?*/ 		sal_Bool bFt = !pFtn->GetAttr()->GetFtn().IsEndNote();
/*?*/ 		if( !pFtn->GetPrev() &&
/*?*/ 			pFtn->FindFtnBossFrm( bFt ) != pFtn->GetRef()->FindFtnBossFrm( bFt )
/*?*/ 			&& ( !pFrm->IsInSct() || pFrm->FindSctFrm()->MoveAllowed(pFrm) ) )
/*?*/ 		{
/*?*/ 			bKeep = sal_False;
/*?*/ 			nOrphLines = 0;
/*?*/ 			nWidLines = 0;
/*?*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/     UNDO_SWAP( pFrm )
/*N*/ }

/*************************************************************************
 *					WidowsAndOrphans::FindBreak()
 *************************************************************************/

/* Die Find*-Methoden suchen nicht nur, sondern stellen den SwTxtMargin auf
 * die Zeile ein, wo der Absatz gebrochen werden soll und kuerzen ihn dort.
 * FindBreak()
 */

/*N*/ sal_Bool WidowsAndOrphans::FindBreak( SwTxtFrm *pFrame, SwTxtMargin &rLine,
/*N*/ 	sal_Bool bHasToFit )
/*N*/ {
/*N*/     SWAP_IF_SWAPPED( pFrm )
/*N*/ 
/*N*/ 	sal_Bool bRet = sal_True;
/*N*/ 	MSHORT nOldOrphans = nOrphLines;
/*N*/     if( bHasToFit )
/*N*/ 		nOrphLines = 0;
/*N*/ 	rLine.Bottom();
/*N*/ 	if( !IsBreakNow( rLine ) )
/*N*/ 		bRet = sal_False;
/*N*/     if( !FindWidows( pFrame, rLine ) )
/*N*/ 	{
/*N*/ 		sal_Bool bBack = sal_False;
/*N*/ 		while( IsBreakNow( rLine ) )
/*N*/ 		{
/*N*/ 			if( rLine.PrevLine() )
/*N*/ 				bBack = sal_True;
/*N*/ 			else
/*N*/ 				break;
/*N*/ 		}
/*N*/ 		// Eigentlich werden bei HasToFit Schusterjungen (Orphans) nicht
/*N*/ 		// beruecksichtigt, wenn allerdings Dummy-Lines im Spiel sind und
/*N*/ 		// die Orphansregel verletzt wird, machen wir mal eine Ausnahme:
/*N*/ 		// Wir lassen einfach eine Dummyline zurueck und wandern mit dem Text
/*N*/ 		// komplett auf die naechste Seite/Spalte.
/*N*/         if( rLine.GetLineNr() <= nOldOrphans &&
/*N*/             rLine.GetInfo().GetParaPortion()->IsDummy() &&
/*N*/             ( ( bHasToFit && bRet ) || SwTxtFrmBreak::IsBreakNow( rLine ) ) )
/*N*/ 			rLine.Top();
/*N*/ 
/*N*/ 		rLine.TruncLines( sal_True );
/*N*/ 		bRet = bBack;
/*N*/ 	}
/*N*/ 	nOrphLines = nOldOrphans;
/*N*/ 
/*N*/     UNDO_SWAP( pFrm )
/*N*/ 
/*N*/ 	return bRet;
/*N*/ }

/*************************************************************************
 *					WidowsAndOrphans::FindWidows()
 *************************************************************************/

/*	FindWidows positioniert den SwTxtMargin des Masters auf die umzubrechende
 *	Zeile, indem der Follow formatiert und untersucht wird.
 *	Liefert sal_True zurueck, wenn die Widows-Regelung in Kraft tritt,
 *	d.h. der Absatz _zusammengehalten_ werden soll !
 */

/*N*/ sal_Bool WidowsAndOrphans::FindWidows( SwTxtFrm *pFrm, SwTxtMargin &rLine )
/*N*/ {
/*N*/     ASSERT( ! pFrm->IsVertical() || ! pFrm->IsSwapped(),
/*N*/             "WidowsAndOrphans::FindWidows with swapped frame" )
/*N*/ 
/*N*/ 	if( !nWidLines || !pFrm->IsFollow() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	rLine.Bottom();
/*N*/ 
/*N*/ 	// Wir koennen noch was abzwacken
/*N*/ 	SwTxtFrm *pMaster = pFrm->FindMaster();
/*N*/ 	ASSERT(pMaster, "+WidowsAndOrphans::FindWidows: Widows in a master?");
/*N*/ 	if( !pMaster )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	// 5156: Wenn die erste Zeile des Follows nicht passt, wird der Master
/*N*/ 	// wohl voll mit Dummies sein. In diesem Fall waere ein PREP_WIDOWS fatal.
/*N*/ 	if( pMaster->GetOfst() == pFrm->GetOfst() )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	// Resthoehe des Masters
/*N*/     SWRECTFN( pFrm )
/*N*/ 
/*N*/     const SwTwips nDocPrtTop = (pFrm->*fnRect->fnGetPrtTop)();
/*N*/     SwTwips nOldHeight;
/*N*/     SwTwips nTmpY = rLine.Y() + rLine.GetLineHeight();
/*N*/ 
/*N*/     if ( bVert )
/*N*/     {
            DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/         nTmpY = pFrm->SwitchHorizontalToVertical( nTmpY );
/*N*/     }
/*N*/     else
/*N*/         nOldHeight = (pFrm->Prt().*fnRect->fnGetHeight)();
/*N*/ 
/*N*/     const SwTwips nChg = (*fnRect->fnYDiff)( nTmpY, nDocPrtTop + nOldHeight );
/*N*/ 
/*N*/ 	// Unterhalb der Widows-Schwelle...
/*N*/ 	if( rLine.GetLineNr() >= nWidLines )
/*N*/ 	{
/*N*/ 		// 8575: Follow to Master I
/*N*/ 		// Wenn der Follow *waechst*, so besteht fuer den Master die Chance,
/*N*/ 		// Zeilen entgegenzunehmen, die er vor Kurzem gezwungen war an den
/*N*/ 		// Follow abzugeben: Prepare(Need); diese Abfrage unterhalb von nChg!
/*N*/ 		// (0W, 2O, 2M, 2F) + 1F = 3M, 2F
/*N*/ 		if( rLine.GetLineNr() > nWidLines && pFrm->IsJustWidow() )
/*N*/ 		{
/*?*/ 			// Wenn der Master gelockt ist, so hat er vermutlich gerade erst
/*?*/ 			// eine Zeile an uns abgegeben, diese geben nicht zurueck, nur
/*?*/ 			// weil bei uns daraus mehrere geworden sind (z.B. durch Rahmen).
/*?*/             if( !pMaster->IsLocked() && pMaster->GetUpper() )
/*?*/             {
/*?*/                 const SwTwips nRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
/*?*/                             ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
/*?*/                 if ( nRstHeight >=
/*?*/                      SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
/*?*/                 {
/*?*/                     pMaster->Prepare( PREP_ADJUST_FRM );
/*?*/                     pMaster->_InvalidateSize();
/*?*/                     pMaster->InvalidatePage();
/*?*/                 }
/*?*/             }
/*?*/ 
/*?*/ 			pFrm->SetJustWidow( sal_False );
/*N*/ 		}
/*N*/ 		return sal_False;
/*N*/ 				}

    // 8575: Follow to Master II
    // Wenn der Follow *schrumpft*, so besteht fuer den Master die Chance,
    // den kompletten Orphan zu inhalieren.
    // (0W, 2O, 2M, 1F) - 1F = 3M, 0F	  -> PREP_ADJUST_FRM
    // (0W, 2O, 3M, 2F) - 1F = 2M, 2F	  -> PREP_WIDOWS

/*N*/     if( 0 > nChg && !pMaster->IsLocked() && pMaster->GetUpper() )
/*N*/     {
/*N*/         SwTwips nRstHeight = (pMaster->Frm().*fnRect->fnBottomDist)
/*N*/                              ( (pMaster->GetUpper()->*fnRect->fnGetPrtBottom)() );
/*N*/         if( nRstHeight >= SwTwips(rLine.GetInfo().GetParaPortion()->Height() ) )
/*N*/         {
/*N*/             pMaster->Prepare( PREP_ADJUST_FRM );
/*N*/             pMaster->_InvalidateSize();
/*N*/             pMaster->InvalidatePage();
/*N*/             pFrm->SetJustWidow( sal_False );
/*N*/             return sal_False;
/*N*/         }
/*N*/ 	}
/*N*/ 
/*N*/ 	// Master to Follow
/*N*/ 	// Wenn der Follow nach seiner Formatierung weniger Zeilen enthaelt
/*N*/ 	// als Widows, so besteht noch die Chance, einige Zeilen des Masters
/*N*/ 	// abzuzwacken. Wenn dadurch die Orphans-Regel des Masters in Kraft
/*N*/ 	// tritt muss im CalcPrep() des Master-Frame der Frame so vergroessert
/*N*/ 	// werden, dass er nicht mehr auf seine urspruengliche Seite passt.
/*N*/ 	// Wenn er noch ein paar Zeilen entbehren kann, dann muss im CalcPrep()
/*N*/ 	// ein Shrink() erfolgen, der Follow mit dem Widows rutscht dann auf
/*N*/ 	// die Seite des Masters, haelt sich aber zusammen, so dass er (endlich)
/*N*/ 	// auf die naechste Seite rutscht. - So die Theorie!
/*N*/ 
/*N*/ 
/*N*/ 	// Wir fordern nur noch ein Zeile zur Zeit an, weil eine Zeile des Masters
/*N*/ 	// bei uns durchaus mehrere Zeilen ergeben koennten.
/*N*/ 	// Dafuer behaelt CalcFollow solange die Kontrolle, bis der Follow alle
/*N*/ 	// notwendigen Zeilen bekommen hat.
/*N*/ 	MSHORT nNeed = 1; // frueher: nWidLines - rLine.GetLineNr();
/*N*/ 
/*N*/     // Special case: Master cannot give lines to follow
/*N*/     if ( ! pMaster->GetIndPrev() && pMaster->GetThisLines() <= nNeed )
/*N*/         return sal_False;
/*N*/ 
/*N*/ 	pMaster->Prepare( PREP_WIDOWS, (void*)&nNeed );
/*N*/ 	return sal_True;
/*N*/ }

/*************************************************************************
 *					WidowsAndOrphans::WouldFit()
 *************************************************************************/

/*N*/ sal_Bool WidowsAndOrphans::WouldFit( SwTxtMargin &rLine, SwTwips &rMaxHeight )
/*N*/ {
/*N*/     // Here it does not matter, if pFrm is swapped or not.
/*N*/     // IsInside() takes care for itself
/*N*/ 
/*N*/ 	// Wir erwarten, dass rLine auf der letzten Zeile steht!!
/*N*/ 	ASSERT( !rLine.GetNext(), "WouldFit: aLine::Bottom missed!" );
/*N*/ 	MSHORT nLineCnt = rLine.GetLineNr();
/*N*/ 
/*N*/ 	// Erstmal die Orphansregel und den Initialenwunsch erfuellen ...
/*N*/ #ifndef USED
/*N*/ 	const MSHORT nMinLines = Max( GetOrphansLines(), rLine.GetDropLines() );
/*N*/ #else
/*N*/ 	const MSHORT nMinLines = rLine.GetDropLines();
/*N*/ #endif
/*N*/ 	if ( nLineCnt < nMinLines )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	rLine.Top();
/*N*/ 	SwTwips nLineSum = rLine.GetLineHeight();
/*N*/ 
/*N*/ 	while( nMinLines > rLine.GetLineNr() )
/*N*/ 	{
///*N*/ 		DBG_LOOP;
/*N*/ 		if( !rLine.NextLine() )
/*N*/ 			return sal_False;
/*N*/ 		nLineSum += rLine.GetLineHeight();
/*N*/ 	}
/*N*/ 
/*N*/ 	// Wenn wir jetzt schon nicht mehr passen ...
/*N*/ 	if( !IsInside( rLine ) )
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	// Jetzt noch die Widows-Regel ueberpruefen
/*N*/ 	if( !nWidLines && !pFrm->IsFollow() )
/*N*/ 	{
/*N*/ 		// I.A. brauchen Widows nur ueberprueft werden, wenn wir ein Follow
/*N*/ 		// sind. Bei WouldFit muss aber auch fuer den Master die Regel ueber-
/*N*/ 		// prueft werden, weil wir ja gerade erst die Trennstelle ermitteln.
/*N*/ 		// Im Ctor von WidowsAndOrphans wurde nWidLines aber nur fuer Follows
/*N*/ 		// aus dem AttrSet ermittelt, deshalb holen wir es hier nach:
/*N*/ 		const SwAttrSet& rSet = pFrm->GetTxtNode()->GetSwAttrSet();
/*N*/ 		nWidLines = rSet.GetWidows().GetValue();
/*N*/ 	}
/*N*/ 
/*N*/ 	// Sind nach Orphans/Initialen noch genug Zeilen fuer die Widows uebrig?
/*N*/ 	if( nLineCnt - nMinLines >= GetWidowsLines() )
/*N*/ 	{
/*N*/         if( rMaxHeight >= nLineSum )
/*N*/ 		{
/*N*/ 			rMaxHeight -= nLineSum;
/*N*/ 			return sal_True;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return sal_False;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
