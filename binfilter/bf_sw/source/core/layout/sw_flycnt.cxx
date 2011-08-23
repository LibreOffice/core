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


#include <tools/bigint.hxx>
#include "pagefrm.hxx"
#include "rootfrm.hxx"

#include <horiornt.hxx>

#include "txtfrm.hxx"
#include "doc.hxx"
#include "pam.hxx"
#include "frmfmt.hxx"
#include "frmtool.hxx"
#include "dflyobj.hxx"
#include "hints.hxx"
#include "ndtxt.hxx"

#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lrspitem.hxx>

#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <fmtsrnd.hxx>
#include "tabfrm.hxx"
#include "flyfrms.hxx"
#include "sectfrm.hxx"
namespace binfilter {

/*************************************************************************
|*
|*	SwFlyAtCntFrm::SwFlyAtCntFrm()
|*
|*	Ersterstellung		MA 11. Nov. 92
|*	Letzte Aenderung	MA 09. Apr. 99
|*
|*************************************************************************/

/*N*/ SwFlyAtCntFrm::SwFlyAtCntFrm( SwFlyFrmFmt *pFmt, SwFrm *pAnch ) :
/*N*/ 	SwFlyFreeFrm( pFmt, pAnch )
/*N*/ {
/*N*/ 	bAtCnt = TRUE;
/*N*/ 	bAutoPosition = FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId();
/*N*/ }

/*************************************************************************
|*
|*	SwFlyAtCntFrm::CheckCharRect()
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwFlyAtCntFrm::Modify()
|*
|*	Ersterstellung		MA 08. Feb. 93
|*	Letzte Aenderung	MA 23. Nov. 94
|*
|*************************************************************************/

/*N*/ void SwFlyAtCntFrm::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pNew ? pNew->Which() : 0;
/*N*/ 	const SwFmtAnchor *pAnch = 0;
/*N*/ 	if( RES_ATTRSET_CHG == nWhich && SFX_ITEM_SET ==
/*N*/ 		((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_ANCHOR, FALSE,
/*N*/ 			(const SfxPoolItem**)&pAnch ))
/*N*/ 		;		// Beim GetItemState wird der AnkerPointer gesetzt !
/*N*/ 
/*N*/ 	else if( RES_ANCHOR == nWhich )
/*N*/ 	{
/*N*/ 		//Ankerwechsel, ich haenge mich selbst um.
/*N*/ 		//Es darf sich nicht um einen Wechsel des Ankertyps handeln,
/*N*/ 		//dies ist nur ueber die SwFEShell moeglich.
/*?*/ 		pAnch = (const SwFmtAnchor*)pNew;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( pAnch )
/*N*/ 	{
/*N*/ 		ASSERT( pAnch->GetAnchorId() == GetFmt()->GetAnchor().GetAnchorId(),
/*N*/ 				"Unzulaessiger Wechsel des Ankertyps." );
/*N*/ 
/*N*/ 		//Abmelden, neuen Anker besorgen und 'dranhaengen.
/*N*/ 		SwRect aOld( AddSpacesToFrm() );
/*N*/ 		SwPageFrm *pOldPage = FindPageFrm();
/*N*/ 		const SwFrm *pOldAnchor = GetAnchor();
/*N*/ 		SwCntntFrm *pCntnt = (SwCntntFrm*)GetAnchor();
/*N*/ 		GetAnchor()->RemoveFly( this );
/*N*/ 
/*N*/ 		const BOOL bBodyFtn = (pCntnt->IsInDocBody() || pCntnt->IsInFtn());
/*N*/ 
/*N*/ 		//Den neuen Anker anhand des NodeIdx suchen, am alten und
/*N*/ 		//neuen NodeIdx kann auch erkannt werden, in welche Richtung
/*N*/ 		//gesucht werden muss.
/*N*/ 		const SwNodeIndex aNewIdx( pAnch->GetCntntAnchor()->nNode );
/*N*/ 		SwNodeIndex aOldIdx( *pCntnt->GetNode() );
/*N*/ 
/*N*/ 		//fix: Umstellung, ehemals wurde in der do-while-Schleife nach vorn bzw.
/*N*/ 		//nach hinten gesucht; je nachdem wie welcher Index kleiner war.
/*N*/ 		//Das kann aber u.U. zu einer Endlosschleife fuehren. Damit
/*N*/ 		//wenigstens die Schleife unterbunden wird suchen wir nur in eine
/*N*/ 		//Richtung. Wenn der neue Anker nicht gefunden wird koennen wir uns
/*N*/ 		//immer noch vom Node einen Frame besorgen. Die Change, dass dies dann
/*N*/ 		//der richtige ist, ist gut.
/*N*/ 		const FASTBOOL bNext = aOldIdx < aNewIdx;
/*N*/ 		while ( pCntnt && aOldIdx != aNewIdx )
/*N*/ 		{
/*N*/ 			do
/*N*/ 			{	if ( bNext )
/*?*/ 					pCntnt = pCntnt->GetNextCntntFrm();
/*N*/ 				else
/*N*/ 					pCntnt = pCntnt->GetPrevCntntFrm();
/*N*/ 			} while ( pCntnt &&
/*N*/ 					  !(bBodyFtn == (pCntnt->IsInDocBody() ||
/*N*/ 									 pCntnt->IsInFtn())) );
/*N*/ 			if (pCntnt)
/*N*/ 				aOldIdx = *pCntnt->GetNode();
/*N*/ 		}
/*N*/ 		if ( !pCntnt )
/*N*/ 		{
/*?*/ 			SwCntntNode *pNode = aNewIdx.GetNode().GetCntntNode();
/*?*/ 			pCntnt = pNode->GetFrm( &pOldAnchor->Frm().Pos(), 0, FALSE );
/*?*/ 			ASSERT( pCntnt, "Neuen Anker nicht gefunden" );
/*N*/ 		}
/*N*/ 		//Flys haengen niemals an einem Follow sondern immer am
/*N*/ 		//Master, den suchen wir uns jetzt.
/*N*/ 		const SwFlowFrm *pFlow = pCntnt;
/*N*/ 		while ( pFlow->IsFollow() )
/*?*/ 			pFlow = pFlow->FindMaster();
/*N*/ 		pCntnt = (SwCntntFrm*)pFlow->GetFrm();
/*N*/ 
/*N*/ 		//und schwupp angehaengt das teil...
/*N*/ 		pCntnt->AppendFly( this );
/*N*/ 		if ( pOldPage && pOldPage != FindPageFrm() )
/*N*/ 			NotifyBackground( pOldPage, aOld, PREP_FLY_LEAVE );
/*N*/ 
/*N*/ 		//Fix(3495)
/*N*/ 		_InvalidatePos();
/*N*/ 		InvalidatePage();
/*N*/ 		SetNotifyBack();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		SwFlyFrm::Modify( pOld, pNew );
/*N*/ }

/*************************************************************************
|*
|*	SwFlyAtCntFrm::MakeAll()
|*
|*	Beschreibung		Bei einem Absatzgebunden Fly kann es durchaus sein,
|*		das der Anker auf die Veraenderung des Flys reagiert. Auf diese
|* 		Reaktion hat der Fly natuerlich auch wieder zu reagieren.
|* 		Leider kann dies zu Oszillationen fuehren z.b. Der Fly will nach
|* 		unten, dadurch kann der Inhalt nach oben, der TxtFrm wird kleiner,
|*		der Fly muss wieder hoeher woduch der Text wieder nach unten
|*		verdraengt wird...
|*		Um derartige Oszillationen zu vermeiden, wird ein kleiner Positions-
|* 		stack aufgebaut. Wenn der Fly ein Position erreicht, die er bereits
|* 		einmal einnahm, so brechen wir den Vorgang ab. Um keine Risiken
|* 		einzugehen, wird der Positionsstack so aufgebaut, dass er fuenf
|* 		Positionen zurueckblickt.
|* 		Wenn der Stack ueberlaeuft, wird ebenfalls abgebrochen.
|* 		Der Abbruch fuer dazu, dass der Fly am Ende eine unguenste Position
|* 		einnimmt. Damit es nicht durch einen wiederholten Aufruf von
|* 		Aussen zu einer 'grossen Oszillation' kommen kann wird im Abbruch-
|*		fall das Attribut des Rahmens auf automatische Ausrichtung oben
|* 		eingestellt.
|*
|*	Ersterstellung		MA 12. Nov. 92
|*	Letzte Aenderung	MA 20. Sep. 96
|*
|*************************************************************************/
//Wir brauchen ein Paar Hilfsklassen zur Kontrolle der Ozillation und ein paar
//Funktionen um die Uebersicht zu gewaehrleisten.

/*N*/ class SwOszControl
/*N*/ {
/*N*/ 	static const SwFlyFrm *pStk1;
/*N*/ 	static const SwFlyFrm *pStk2;
/*N*/ 	static const SwFlyFrm *pStk3;
/*N*/ 	static const SwFlyFrm *pStk4;
/*N*/ 	static const SwFlyFrm *pStk5;
/*N*/ 
/*N*/ 	const SwFlyFrm *pFly;
/*N*/ 	Point aStk1, aStk2, aStk3, aStk4, aStk5;
/*N*/ 
/*N*/ public:
/*N*/ 	SwOszControl( const SwFlyFrm *pFrm );
/*N*/ 	~SwOszControl();
/*N*/ 	FASTBOOL ChkOsz();
/*N*/ 	static FASTBOOL IsInProgress( const SwFlyFrm *pFly );
/*N*/ };
/*N*/ const SwFlyFrm *SwOszControl::pStk1 = 0;
/*N*/ const SwFlyFrm *SwOszControl::pStk2 = 0;
/*N*/ const SwFlyFrm *SwOszControl::pStk3 = 0;
/*N*/ const SwFlyFrm *SwOszControl::pStk4 = 0;
/*N*/ const SwFlyFrm *SwOszControl::pStk5 = 0;

/*N*/ SwOszControl::SwOszControl( const SwFlyFrm *pFrm ) :
/*N*/ 	pFly( pFrm )
/*N*/ {
/*N*/ 	if ( !SwOszControl::pStk1 )
/*N*/ 		SwOszControl::pStk1 = pFly;
/*N*/ 	else if ( !SwOszControl::pStk2 )
/*?*/ 		SwOszControl::pStk2 = pFly;
/*?*/ 	else if ( !SwOszControl::pStk3 )
/*?*/ 		SwOszControl::pStk3 = pFly;
/*?*/ 	else if ( !SwOszControl::pStk4 )
/*?*/ 		SwOszControl::pStk4 = pFly;
/*?*/ 	else if ( !SwOszControl::pStk5 )
/*?*/ 		SwOszControl::pStk5 = pFly;
/*N*/ }

/*N*/ SwOszControl::~SwOszControl()
/*N*/ {
/*N*/ 	if ( SwOszControl::pStk1 == pFly )
/*N*/ 		SwOszControl::pStk1 = 0;
/*N*/ 	else if ( SwOszControl::pStk2 == pFly )
/*?*/ 		SwOszControl::pStk2 = 0;
/*?*/ 	else if ( SwOszControl::pStk3 == pFly )
/*?*/ 		SwOszControl::pStk3 = 0;
/*?*/ 	else if ( SwOszControl::pStk4 == pFly )
/*?*/ 		SwOszControl::pStk4 = 0;
/*?*/ 	else if ( SwOszControl::pStk5 == pFly )
/*?*/ 		SwOszControl::pStk5 = 0;
/*N*/ }

/*N*/ FASTBOOL IsInProgress( const SwFlyFrm *pFly )
/*N*/ {
        DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 //STRIP001 	return SwOszControl::IsInProgress( pFly );
/*N*/ }

/*N*/ FASTBOOL SwOszControl::IsInProgress( const SwFlyFrm *pFly )
/*N*/ {
/*N*/ 	if ( SwOszControl::pStk1 && !pFly->IsLowerOf( SwOszControl::pStk1 ) )
/*N*/ 		return TRUE;
/*N*/ 	if ( SwOszControl::pStk2 && !pFly->IsLowerOf( SwOszControl::pStk2 ) )
/*?*/ 		return TRUE;
/*N*/ 	if ( SwOszControl::pStk3 && !pFly->IsLowerOf( SwOszControl::pStk3 ) )
/*?*/ 		return TRUE;
/*N*/ 	if ( SwOszControl::pStk4 && !pFly->IsLowerOf( SwOszControl::pStk4 ) )
/*?*/ 		return TRUE;
/*N*/ 	if ( SwOszControl::pStk5 && !pFly->IsLowerOf( SwOszControl::pStk5 ) )
/*?*/ 		return TRUE;
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ FASTBOOL SwOszControl::ChkOsz()
/*N*/ {
/*N*/ 	FASTBOOL bRet = TRUE;
/*N*/ 	Point aTmp = pFly->Frm().Pos();
/*N*/ 	if( aTmp == Point() )
/*N*/ 		aTmp.X() = 1;
/*N*/ 	//Ist der Stack am Ende?
/*N*/ 	if ( aStk1 != Point() )
/*?*/ 		return TRUE;
/*N*/ 	if ( aTmp != aStk1 && aTmp != aStk2 && aTmp != aStk3 &&
/*N*/ 		 aTmp != aStk4 && aTmp != aStk5 )
/*N*/ 	{
/*N*/ 		aStk1 = aStk2;
/*N*/ 		aStk2 = aStk3;
/*N*/ 		aStk3 = aStk4;
/*N*/ 		aStk4 = aStk5;
/*N*/ 		aStk5 = aTmp;
/*N*/ 		bRet = FALSE;
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

/*M*/ void SwFlyAtCntFrm::MakeAll()
/*M*/ {
/*M*/     if ( !SwOszControl::IsInProgress( this ) && !IsLocked() && !IsColLocked() )
/*M*/     {
/*M*/         if( !GetPage() && GetAnchor() && GetAnchor()->IsInFly() )
/*M*/         {
/*M*/             SwFlyFrm* pFly = GetAnchor()->FindFlyFrm();
/*M*/             SwPageFrm *pPage = pFly ? pFly->FindPageFrm() : NULL;
/*M*/             if( pPage )
/*M*/                 pPage->SwPageFrm::AppendFly( this );
/*M*/         }
/*M*/         if( GetPage() )
/*M*/         {
/*M*/ 		//Den Anker muessen wir zwischendurch natuerlich Formatieren, damit
/*M*/ 		//Repaints usw. stimmen sollte er natuerlich trotzdem Invalid bleiben.
/*M*/ 		//Jetzt Stufe 2: Damit Repaints stimmen muessen alle Frms wieder Invalidiert
/*M*/ 		//werden, die unterwegs formatiert werden.
/*M*/ 		//Dazu werden sie ein ein PrtArr eingetragen; die Frms mit CompletePaint
/*M*/ 		//zu flaggen scheint mir hier das Mittel der Wahl.
/*M*/ 		//(Vielleicht sollte es einmal die Moeglichkeit geben sie einfach mit
/*M*/ 		//Paint zu flaggen; kein Formatieren, aber ein Paint-Aufruf, vor allem
/*M*/ 		//wohl fuer TxtFrms geeignet.
/*M*/ 		//Jetzt Stufe 3: einfach ein globales Flag und schon flaggen sie sich
/*M*/ 		//selbst.
/*M*/             bSetCompletePaintOnInvalidate = TRUE;
/*M*/             sal_Bool bLockedAnchor =
/*M*/                 static_cast<const SwTxtFrm*>( GetAnchor() )->IsAnyJoinLocked();
/*M*/             {
/*M*/                 SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
/*M*/                 const SwFmtFrmSize &rFrmSz = GetFmt()->GetFrmSize();
/*M*/                 if( rFrmSz.GetHeightPercent() != 0xFF &&
/*M*/                     rFrmSz.GetHeightPercent() >= 100 )
/*M*/                 {
/*M*/                     pFmt->LockModify();
/*M*/                     SwFmtSurround aMain( pFmt->GetSurround() );
/*M*/                     if ( aMain.GetSurround() == SURROUND_NONE )
/*M*/                     {
/*M*/                         aMain.SetSurround( SURROUND_THROUGHT );
/*M*/                         pFmt->SetAttr( aMain );
/*M*/                     }
/*M*/                     pFmt->UnlockModify();
/*M*/                 }
/*M*/             }
/*M*/             SwOszControl aOszCntrl( this );
/*M*/ 
/*M*/             if( !bLockedAnchor )
/*M*/             {
/*M*/                 if( GetAnchor()->IsInSct() )
/*M*/                 {
/*M*/                     SwSectionFrm *pSct = GetAnchor()->FindSctFrm();
/*M*/                     pSct->Calc();
/*M*/                 }
/*M*/ 
/*M*/                 GetAnchor()->Calc();
/*M*/             }
/*M*/ 
/*M*/             SwFrm* pFooter = GetAnchor()->FindFooterOrHeader();
/*M*/             if( pFooter && !pFooter->IsFooterFrm() )
/*M*/                 pFooter = NULL;
/*M*/             FASTBOOL bOsz = FALSE;
/*M*/             FASTBOOL bExtra = Lower() && Lower()->IsColumnFrm();
/*M*/ 
/*M*/             do {
/*M*/                 SWRECTFN( this )
/*M*/                 Point aOldPos( (Frm().*fnRect->fnGetPos)() );
/*M*/                 SwFlyFreeFrm::MakeAll();
/*M*/                 BOOL bPosChg = aOldPos != (Frm().*fnRect->fnGetPos)();
/*M*/                 if( !bLockedAnchor )
/*M*/                 {
/*M*/                     if( GetAnchor()->IsInSct() )
/*M*/                     {
/*M*/                         SwSectionFrm *pSct = GetAnchor()->FindSctFrm();
/*M*/                         pSct->Calc();
/*M*/                     }
/*M*/ 
/*M*/                     GetAnchor()->Calc();
/*M*/                 }
/*M*/ 
/*M*/                 if( aOldPos != (Frm().*fnRect->fnGetPos)() ||
/*M*/                     ( !GetValidPosFlag() &&( pFooter || bPosChg ) ) )
/*M*/                     bOsz = aOszCntrl.ChkOsz();
/*M*/                 if( bExtra && Lower() && !Lower()->GetValidPosFlag() )
/*M*/                 {  // Wenn ein mehrspaltiger Rahmen wg. Positionswechsel ungueltige
/*M*/                     // Spalten hinterlaesst, so drehen wir lieber hier eine weitere
/*M*/                     // Runde und formatieren unseren Inhalt via FormatWidthCols nochmal.
/*M*/                     _InvalidateSize();
/*M*/                     bExtra = FALSE; // Sicherhaltshalber gibt es nur eine Ehrenrunde.
/*M*/                 }
/*M*/             } while ( !IsValid() && !bOsz );
/*M*/ 
/*M*/             if ( bOsz )
/*M*/             {
/*M*/                 SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
/*M*/                 pFmt->LockModify();
/*M*/                 SwFmtSurround aMain( pFmt->GetSurround() );
/*M*/                 // Im Notfall setzen wir automatisch positionierte Rahmen mit
/*M*/                 // Rekursion auf Durchlauf, das duerfte beruhigend wirken.
/*M*/                 if( IsAutoPos() && aMain.GetSurround() != SURROUND_THROUGHT )
/*M*/                 {
/*M*/                     aMain.SetSurround( SURROUND_THROUGHT );
/*M*/                     pFmt->SetAttr( aMain );
/*M*/                 }
/*M*/                 else
/*M*/                 {
/*M*/                     SwFmtVertOrient aOrient( pFmt->GetVertOrient() );
/*M*/                     aOrient.SetVertOrient( VERT_TOP );
/*M*/                     pFmt->SetAttr( aOrient );
/*M*/                     //Wenn der Rahmen auf "Kein Umlauf" steht, versuchen wir es mal
/*M*/                     //mit Seitenumlauf.
/*M*/                     if ( aMain.GetSurround() == SURROUND_NONE )
/*M*/                     {
/*M*/                         aMain.SetSurround( SURROUND_PARALLEL );
/*M*/                         pFmt->SetAttr( aMain );
/*M*/                     }
/*M*/                 }
/*M*/                 pFmt->UnlockModify();
/*M*/ 
/*M*/                 _InvalidatePos();
/*M*/                 SwFlyFreeFrm::MakeAll();
/*M*/                 if( !bLockedAnchor )
/*M*/                     GetAnchor()->Calc();
/*M*/                 if ( !GetValidPosFlag() )
/*M*/                 {
/*M*/                     SwFlyFreeFrm::MakeAll();
/*M*/                     if( !bLockedAnchor )
/*M*/                         GetAnchor()->Calc();
/*M*/                 }
/*M*/                 //Osz auf jeden fall zum Stehen bringen.
/*M*/                 bValidPos = bValidSize = bValidPrtArea = TRUE;
/*M*/             }
/*M*/             bSetCompletePaintOnInvalidate = FALSE;
/*M*/         }
/*M*/     }
/*M*/ }

/*************************************************************************
|*
|*	FindAnchor() und Hilfsfunktionen.
|*
|*	Beschreibung:		Sucht ausgehend von pOldAnch einen Anker fuer
|*		Absatzgebundene Objekte.
|*		Wird beim Draggen von Absatzgebundenen Objekten zur Ankeranzeige sowie
|*		fuer Ankerwechsel benoetigt.
|*	Ersterstellung		MA 22. Jun. 93
|*	Letzte Aenderung	MA 30. Jan. 95
|*
|*************************************************************************/

/*N*/ class SwDistance
/*N*/ {
/*N*/ public:
/*N*/ 	SwTwips nMain, nSub;
/*N*/ 	SwDistance() { nMain = nSub = 0; }
/*N*/ 	SwDistance& operator=( const SwDistance &rTwo )
/*N*/ 		{ nMain = rTwo.nMain; nSub = rTwo.nSub; return *this; }
/*N*/ 	BOOL operator<( const SwDistance& rTwo )
/*N*/ 		{ return nMain < rTwo.nMain || ( nMain == rTwo.nMain && nSub &&
/*N*/ 		  rTwo.nSub && nSub < rTwo.nSub ); }
/*N*/ 	BOOL operator<=( const SwDistance& rTwo )
/*N*/ 		{ return nMain < rTwo.nMain || ( nMain == rTwo.nMain && ( !nSub ||
/*N*/ 		  !rTwo.nSub || nSub <= rTwo.nSub ) ); }
/*N*/ };

/*N*/ const SwFrm * MA_FASTCALL lcl_CalcDownDist( SwDistance &rRet,
/*N*/ 										 const Point &rPt,
/*N*/ 										 const SwCntntFrm *pCnt )
/*N*/ {
/*N*/ 	rRet.nSub = 0;
/*N*/ 	//Wenn der Point direkt innerhalb des Cnt steht ist die Sache klar und
/*N*/ 	//der Cntnt hat automatisch eine Entfernung von 0
/*N*/ 	if ( pCnt->Frm().IsInside( rPt ) )
/*N*/ 	{
/*N*/ 		rRet.nMain = 0;
/*N*/ 		return pCnt;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SwLayoutFrm *pUp = pCnt->IsInTab() ? pCnt->FindTabFrm()->GetUpper() : pCnt->GetUpper();
/*N*/ 		// einspaltige Bereiche muessen zu ihrem Upper durchschalten
/*N*/ 		while( pUp->IsSctFrm() )
/*N*/ 			pUp = pUp->GetUpper();
/*N*/         SWRECTFN( pUp )
/*N*/ 		//Dem Textflus folgen.
/*N*/         if ( pUp->Frm().IsInside( rPt ) )
/*N*/ 		{
/*N*/             if( bVert )
/*N*/                 rRet.nMain =  pCnt->Frm().Left() + pCnt->Frm().Width() -rPt.X();
/*N*/             else
/*N*/                 rRet.nMain =  rPt.Y() - pCnt->Frm().Top();
/*N*/ 			return pCnt;
/*N*/ 		}
/*N*/         else if ( rPt.Y() <= pUp->Frm().Top() )
/*N*/         {
/*N*/             rRet.nMain = LONG_MAX;
/*N*/         }
/*N*/         else if( rPt.X() < pUp->Frm().Left() &&
/*N*/                  rPt.Y() <= ( bVert ? pUp->Frm().Top() : pUp->Frm().Bottom() ) )
/*N*/ 		{
/*?*/ 			const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, FALSE, pCnt );
/*?*/             if( !pLay ||
/*?*/                 (bVert && (pLay->Frm().Top() + pLay->Prt().Bottom()) <rPt.Y())||
/*?*/                 (!bVert && (pLay->Frm().Left() + pLay->Prt().Right())<rPt.X()) )
/*?*/ 			{
/*?*/                 if( bVert )
/*?*/                     rRet.nMain =  pCnt->Frm().Left() + pCnt->Frm().Width()
/*?*/                                   - rPt.X();
/*?*/                 else
/*?*/                     rRet.nMain = rPt.Y() - pCnt->Frm().Top();
/*?*/ 				return pCnt;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				rRet.nMain = LONG_MAX;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/             rRet.nMain = bVert ? pCnt->Frm().Left() + pCnt->Frm().Width() -
/*N*/                                  (pUp->Frm().Left() + pUp->Prt().Left())
/*N*/                 : (pUp->Frm().Top() + pUp->Prt().Bottom()) - pCnt->Frm().Top();
/*N*/ 
/*N*/ 			const SwFrm *pPre = pCnt;
/*N*/ 			const SwFrm *pLay = pUp->GetLeaf( MAKEPAGE_NONE, TRUE, pCnt );
/*N*/ 			SwTwips nFrmTop, nPrtHeight;
/*N*/ 			BOOL bSct;
/*N*/ 			const SwSectionFrm *pSect = pUp->FindSctFrm();
/*N*/ 			if( pSect )
/*N*/ 			{
/*N*/ 				rRet.nSub = rRet.nMain;
/*N*/ 				rRet.nMain = 0;
/*N*/ 			}
/*N*/ 			if( pSect && !pSect->IsAnLower( pLay ) )
/*N*/ 			{
/*?*/ 				bSct = FALSE;
/*?*/ 				const SwSectionFrm* pNxtSect = pLay ? pLay->FindSctFrm() : 0;
/*?*/ 				if( pSect->IsAnFollow( pNxtSect ) )
/*?*/ 				{
/*?*/                     if( pLay->IsVertical() )
/*?*/                     {
/*?*/                         nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
/*?*/                         nPrtHeight = pLay->Prt().Width();
/*?*/                     }
/*?*/                     else
/*?*/                     {
/*?*/                         nFrmTop = pLay->Frm().Top();
/*?*/                         nPrtHeight = pLay->Prt().Height();
/*?*/                     }
/*?*/ 					pSect = pNxtSect;
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					pLay = pSect->GetUpper();
/*?*/                     if( pLay->IsVertical() )
/*?*/                     {
/*?*/                         nFrmTop = pSect->Frm().Left();
/*?*/                         nPrtHeight = pSect->Frm().Left() - pLay->Frm().Left()
/*?*/                                      - pLay->Prt().Left();
/*?*/                     }
/*?*/                     else
/*?*/                     {
/*?*/                         nFrmTop = pSect->Frm().Bottom();
/*?*/                         nPrtHeight = pLay->Frm().Top() + pLay->Prt().Top()
/*?*/                                      + pLay->Prt().Height() - pSect->Frm().Top()
/*?*/                                      - pSect->Frm().Height();
/*?*/                     }
/*?*/ 					pSect = 0;
/*?*/ 				}
/*?*/ 			}
/*N*/ 			else if( pLay )
/*N*/ 			{
/*N*/                 if( pLay->IsVertical() )
/*N*/                 {
/*N*/                     nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
/*N*/                     nPrtHeight = pLay->Prt().Width();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     nFrmTop = pLay->Frm().Top();
/*N*/                     nPrtHeight = pLay->Prt().Height();
/*N*/                 }
/*N*/ 				bSct = 0 != pSect;
/*N*/ 			}
/*N*/ 			while ( pLay && !pLay->Frm().IsInside( rPt ) &&
/*N*/ 					( pLay->Frm().Top() <= rPt.Y() || pLay->IsInFly() ||
/*N*/ 					  ( pLay->IsInSct() &&
/*N*/ 					  pLay->FindSctFrm()->GetUpper()->Frm().Top() <= rPt.Y())) )
/*N*/ 			{
/*N*/ 				if ( pLay->IsFtnContFrm() )
/*N*/ 				{
/*N*/ 					if ( !((SwLayoutFrm*)pLay)->Lower() )
/*N*/ 					{
/*N*/ 						SwFrm *pDel = (SwFrm*)pLay;
/*N*/ 						pDel->Cut();
/*N*/ 						delete pDel;
/*N*/ 						return pPre;
/*N*/ 					}
/*N*/ 					return 0;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					if( bSct || pSect )
/*N*/ 						rRet.nSub += nPrtHeight;
/*N*/ 					else
/*N*/ 						rRet.nMain += nPrtHeight;
/*N*/ 					pPre = pLay;
/*N*/ 					pLay = pLay->GetLeaf( MAKEPAGE_NONE, TRUE, pCnt );
/*N*/ 					if( pSect && !pSect->IsAnLower( pLay ) )
/*N*/ 					{   // If we're leaving a SwSectionFrm, the next Leaf-Frm
/*?*/ 						// is the part of the upper below the SectionFrm.
/*?*/ 						const SwSectionFrm* pNxtSect = pLay ?
/*?*/ 							pLay->FindSctFrm() : NULL;
/*?*/ 						bSct = FALSE;
/*?*/ 						if( pSect->IsAnFollow( pNxtSect ) )
/*?*/ 						{
/*?*/ 							pSect = pNxtSect;
/*?*/                             if( pLay->IsVertical() )
/*?*/                             {
/*?*/                                 nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
/*?*/                                 nPrtHeight = pLay->Prt().Width();
/*?*/                             }
/*?*/                             else
/*?*/                             {
/*?*/                                 nFrmTop = pLay->Frm().Top();
/*?*/                                 nPrtHeight = pLay->Prt().Height();
/*?*/                             }
/*?*/ 						}
/*?*/ 						else
/*?*/ 						{
/*?*/ 							pLay = pSect->GetUpper();
/*?*/                             if( pLay->IsVertical() )
/*?*/                             {
/*?*/                                 nFrmTop = pSect->Frm().Left();
/*?*/                                 nPrtHeight = pSect->Frm().Left() -
/*?*/                                         pLay->Frm().Left() - pLay->Prt().Left();
/*?*/                             }
/*?*/                             else
/*?*/                             {
/*?*/                                 nFrmTop = pSect->Frm().Bottom();
/*?*/                                 nPrtHeight = pLay->Frm().Top()+pLay->Prt().Top()
/*?*/                                      + pLay->Prt().Height() - pSect->Frm().Top()
/*?*/                                      - pSect->Frm().Height();
/*?*/                             }
/*?*/ 							pSect = 0;
/*?*/ 						}
/*N*/ 					}
/*N*/ 					else if( pLay )
/*N*/ 					{
/*N*/                         if( pLay->IsVertical() )
/*N*/                         {
/*N*/                              nFrmTop = pLay->Frm().Left() + pLay->Frm().Width();
/*N*/                              nPrtHeight = pLay->Prt().Width();
/*N*/                         }
/*N*/                         else
/*N*/                         {
/*N*/                             nFrmTop = pLay->Frm().Top();
/*N*/                             nPrtHeight = pLay->Prt().Height();
/*N*/                         }
/*N*/ 						bSct = 0 != pSect;
/*N*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( pLay )
/*N*/ 			{
/*?*/ 				if ( pLay->Frm().IsInside( rPt ) )
/*?*/ 				{
/*?*/                     SwTwips nDiff = pLay->IsVertical() ? ( nFrmTop - rPt.X() )
/*?*/                                                        : ( rPt.Y() - nFrmTop );
/*?*/                     if( bSct || pSect )
/*?*/                         rRet.nSub += nDiff;
/*?*/                     else
/*?*/                         rRet.nMain += nDiff;
/*?*/ 				}
/*?*/ 				if ( pLay->IsFtnContFrm() && !((SwLayoutFrm*)pLay)->Lower() )
/*?*/ 				{
/*?*/ 					SwFrm *pDel = (SwFrm*)pLay;
/*?*/ 					pDel->Cut();
/*?*/ 					delete pDel;
/*?*/ 					return 0;
/*?*/ 				}
/*?*/ 				return pLay;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				rRet.nMain = LONG_MAX;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

//Bug 3985, optimierungsproblem, vergleiche auch trvlfrm.cxx lcl_FindCntnt()

/*N*/ ULONG MA_FASTCALL lcl_FindCntDiff( const Point &rPt, const SwLayoutFrm *pLay,
/*N*/ 						  const SwCntntFrm *& rpCnt,
/*N*/ 						  const BOOL bBody, const BOOL bFtn )
/*N*/ {
/*N*/ 	//Sucht unterhalb von pLay den dichtesten Cnt zum Point. Der Bezugspunkt
/*N*/ 	//der Cntnts ist immer die linke obere Ecke.
/*N*/ 	//Der Cnt soll moeglichst ueber dem Point liegen.
/*N*/ 
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	Point arPoint( rPt );
/*N*/ #endif
/*N*/ 
/*N*/ 	rpCnt = 0;
/*N*/ 	ULONG nDistance = ULONG_MAX;
/*N*/ 	ULONG nNearest	= ULONG_MAX;
/*N*/ 	const SwCntntFrm *pCnt = pLay->ContainsCntnt();
/*N*/ 
/*N*/ 	while ( pCnt && (bBody != pCnt->IsInDocBody() || bFtn != pCnt->IsInFtn()))
/*N*/ 	{
/*N*/ 		pCnt = pCnt->GetNextCntntFrm();
/*N*/ 		if ( !pLay->IsAnLower( pCnt ) )
/*N*/ 			pCnt = 0;
/*N*/ 	}
/*N*/ 	const SwCntntFrm *pNearest = pCnt;
/*N*/ 	if ( pCnt )
/*N*/ 	{
/*N*/ 		do
/*N*/ 		{
/*N*/ 			//Jetzt die Entfernung zwischen den beiden Punkten berechnen.
/*N*/ 			//'Delta' X^2 + 'Delta'Y^2 = 'Entfernung'^2
/*N*/ 			sal_uInt32 dX = Max( pCnt->Frm().Left(), rPt.X() ) -
/*N*/ 					   Min( pCnt->Frm().Left(), rPt.X() ),
/*N*/ 				  dY = Max( pCnt->Frm().Top(), rPt.Y() ) -
/*N*/ 					   Min( pCnt->Frm().Top(), rPt.Y() );
/*N*/ 			BigInt dX1( dX ), dY1( dY );
/*N*/ 			dX1 *= dX1; dY1 *= dY1;
/*N*/ 			const ULONG nDiff = ::binfilter::SqRt( dX1 + dY1 );
/*N*/ 			if ( pCnt->Frm().Top() <= rPt.Y() )
/*N*/ 			{
/*N*/ 				if ( nDiff < nDistance )
/*N*/ 				{	//Der ist dichter dran
/*N*/ 					nDistance = nNearest = nDiff;
/*N*/ 					rpCnt = pNearest = pCnt;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( nDiff < nNearest )
/*N*/ 			{
/*N*/ 				nNearest = nDiff;
/*N*/ 				pNearest = pCnt;
/*N*/ 			}
/*N*/ 			pCnt = pCnt->GetNextCntntFrm();
/*N*/ 			while ( pCnt &&
/*N*/ 					(bBody != pCnt->IsInDocBody() || bFtn != pCnt->IsInFtn()))
/*N*/ 				pCnt = pCnt->GetNextCntntFrm();
/*N*/ 
/*N*/ 		}  while ( pCnt && pLay->IsAnLower( pCnt ) );
/*N*/ 	}
/*N*/ 	if ( nDistance == ULONG_MAX )
/*N*/ 	{	rpCnt = pNearest;
/*N*/ 		return nNearest;
/*N*/ 	}
/*N*/ 	return nDistance;
/*N*/ }

/*N*/ #ifdef _MSC_VER
/*N*/ #pragma optimize("e",on)
/*N*/ #endif

/*N*/ const SwCntntFrm * MA_FASTCALL lcl_FindCnt( const Point &rPt, const SwCntntFrm *pCnt,
/*N*/ 								  const BOOL bBody, const BOOL bFtn )
/*N*/ {
/*N*/ 	//Sucht ausgehen von pCnt denjenigen CntntFrm, dessen linke obere
/*N*/ 	//Ecke am dichtesten am Point liegt.
/*N*/ 	//Liefert _immer_ einen CntntFrm zurueck.
/*N*/ 
/*N*/ 	//Zunaechst wird versucht den dichtesten Cntnt innerhalt derjenigen
/*N*/ 	//Seite zu suchen innerhalb derer der Cntnt steht.
/*N*/ 	//Ausgehend von der Seite muessen die Seiten in beide
/*N*/ 	//Richtungen beruecksichtigt werden.
/*N*/ 	//Falls moeglich wird ein Cntnt geliefert, dessen Y-Position ueber der
/*N*/ 	//des Point sitzt.
/*N*/ 	const SwCntntFrm  *pRet, *pNew;
/*N*/ 	const SwLayoutFrm *pLay = pCnt->FindPageFrm();
/*N*/ 	ULONG nDist;
/*N*/ 
/*N*/ 	nDist = ::binfilter::lcl_FindCntDiff( rPt, pLay, pNew, bBody, bFtn );
/*N*/ 	if ( pNew )
/*N*/ 		pRet = pNew;
/*N*/ 	else
/*N*/ 	{	pRet  = pCnt;
/*N*/ 		nDist = ULONG_MAX;
/*N*/ 	}
/*N*/ 	const SwCntntFrm *pNearest = pRet;
/*N*/ 	ULONG nNearest = nDist;
/*N*/ 
/*N*/ 	if ( pLay )
/*N*/ 	{
/*N*/ 		const SwLayoutFrm *pPge = pLay;
/*N*/ 		ULONG nOldNew = ULONG_MAX;
/*N*/ 		for ( USHORT i = 0; pPge->GetPrev() && (i < 3); ++i )
/*N*/ 		{
/*N*/ 			pPge = (SwLayoutFrm*)pPge->GetPrev();
/*N*/ 			const ULONG nNew = ::binfilter::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
/*N*/ 			if ( nNew < nDist )
/*N*/ 			{
/*N*/ 				if ( pNew->Frm().Top() <= rPt.Y() )
/*N*/ 				{
/*N*/ 					pRet = pNearest = pNew;
/*N*/ 					nDist = nNearest = nNew;
/*N*/ 				}
/*N*/ 				else if ( nNew < nNearest )
/*N*/ 				{
/*N*/ 					pNearest = pNew;
/*N*/ 					nNearest = nNew;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( nOldNew != ULONG_MAX && nNew > nOldNew )
/*N*/ 				break;
/*N*/ 			else
/*N*/ 				nOldNew = nNew;
/*N*/ 
/*N*/ 		}
/*N*/ 		pPge = pLay;
/*N*/ 		nOldNew = ULONG_MAX;
/*N*/ 		for ( USHORT j = 0; pPge->GetNext() && (j < 3); ++j )
/*N*/ 		{
/*N*/ 			pPge = (SwLayoutFrm*)pPge->GetNext();
/*N*/ 			const ULONG nNew = ::binfilter::lcl_FindCntDiff( rPt, pPge, pNew, bBody, bFtn );
/*N*/ 			if ( nNew < nDist )
/*N*/ 			{
/*N*/ 				if ( pNew->Frm().Top() <= rPt.Y() )
/*N*/ 				{
/*N*/ 					pRet = pNearest = pNew;
/*N*/ 					nDist = nNearest = nNew;
/*N*/ 				}
/*N*/ 				else if ( nNew < nNearest )
/*N*/ 				{
/*N*/ 					pNearest = pNew;
/*N*/ 					nNearest = nNew;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else if ( nOldNew != ULONG_MAX && nNew > nOldNew )
/*N*/ 				break;
/*N*/ 			else
/*N*/ 				nOldNew = nNew;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( (pRet->Frm().Top() > rPt.Y()) )
/*N*/ 		return pNearest;
/*N*/ 	else
/*N*/ 		return pRet;
/*N*/ }

/*N*/ void lcl_PointToPrt( Point &rPoint, const SwFrm *pFrm )
/*N*/ {
/*N*/ 	SwRect aTmp( pFrm->Prt() );
/*N*/ 	aTmp += pFrm->Frm().Pos();
/*N*/ 	if ( rPoint.X() < aTmp.Left() )
/*N*/ 		rPoint.X() = aTmp.Left();
/*N*/ 	else if ( rPoint.X() > aTmp.Right() )
/*N*/ 		rPoint.X() = aTmp.Right();
/*N*/ 	if ( rPoint.Y() < aTmp.Top() )
/*N*/ 		rPoint.Y() = aTmp.Top();
/*N*/ 	else if ( rPoint.Y() > aTmp.Bottom() )
/*N*/ 		rPoint.Y() = aTmp.Bottom();
/*N*/ 
/*N*/ }

/*N*/ const SwCntntFrm *FindAnchor( const SwFrm *pOldAnch, const Point &rNew,
/*N*/ 							  const BOOL bBodyOnly )
/*N*/ { 
/*N*/ 	//Zu der angegebenen DokumentPosition wird der dichteste Cnt im
/*N*/ 	//Textfluss gesucht. AusgangsFrm ist der uebergebene Anker.
/*N*/ 	const SwCntntFrm *pCnt;
/*N*/ 	if ( pOldAnch->IsCntntFrm() )
/*N*/ 		pCnt = (const SwCntntFrm*)pOldAnch;
/*N*/ 	else
/*N*/ 	{	Point aTmp( rNew );
/*N*/ 		SwLayoutFrm *pTmpLay = (SwLayoutFrm*)pOldAnch;
/*N*/ 		if( pTmpLay->IsRootFrm() )
/*N*/ 		{
/*N*/ 			SwRect aTmpRect( aTmp, Size(0,0) );
/*N*/ 			pTmpLay = (SwLayoutFrm*)::binfilter::FindPage( aTmpRect, pTmpLay->Lower() );
/*N*/ 		}
/*N*/ 		pCnt = pTmpLay->GetCntntPos( aTmp, FALSE, bBodyOnly );
/*N*/ 	}
/*N*/ 
/*N*/ 	//Beim Suchen darauf achten, dass die Bereiche sinnvoll erhalten
/*N*/ 	//bleiben. D.h. in diesem Fall nicht in Header/Footer hinein und
/*N*/ 	//nicht aus Header/Footer hinaus.
/*N*/ 	const BOOL bBody = pCnt->IsInDocBody() || bBodyOnly;
/*N*/ 	const BOOL bFtn  = !bBodyOnly && pCnt->IsInFtn();
/*N*/ 
/*N*/ 	Point aNew( rNew );
/*N*/ 	if ( bBody )
/*N*/ 	{
/*N*/ 		//#38848 Vom Seitenrand in den Body ziehen.
/*N*/ 		const SwFrm *pPage = pCnt->FindPageFrm();
/*N*/ 		::binfilter::lcl_PointToPrt( aNew, pPage->GetUpper() );
/*N*/ 		SwRect aTmp( aNew, Size( 0, 0 ) );
/*N*/ 		pPage = ::binfilter::FindPage( aTmp, pPage );
/*N*/ 		::binfilter::lcl_PointToPrt( aNew, pPage );
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pCnt->IsInDocBody() == bBody && pCnt->Frm().IsInside( aNew ) )
/*N*/ 		return pCnt;
/*N*/ 	else if ( pOldAnch->IsInDocBody() || pOldAnch->IsPageFrm() )
/*N*/ 	{
/*N*/ 		//Vielleicht befindet sich der gewuenschte Anker ja auf derselben
/*N*/ 		//Seite wie der aktuelle Anker.
/*N*/ 		//So gibt es kein Problem mit Spalten.
/*N*/ 		Point aTmp( aNew );
/*N*/ 		const SwCntntFrm *pTmp = pCnt->FindPageFrm()->
/*N*/ 										GetCntntPos( aTmp, FALSE, TRUE, FALSE );
/*N*/ 		if ( pTmp && pTmp->Frm().IsInside( aNew ) )
/*N*/ 			return pTmp;
/*N*/ 	}
/*N*/ 
/*N*/ 	//Ausgehend vom Anker suche ich jetzt in beide Richtungen bis ich
/*N*/ 	//den jeweils dichtesten gefunden habe.
/*N*/ 	//Nicht die direkte Entfernung ist relevant sondern die Strecke die
/*N*/ 	//im Textfluss zurueckgelegt werden muss.
/*N*/ 	const SwCntntFrm *pUpLst;
/*N*/ 	const SwCntntFrm *pUpFrm = pCnt;
/*N*/ 	SwDistance nUp, nUpLst;
/*N*/ 	::binfilter::lcl_CalcDownDist( nUp, aNew, pUpFrm );
/*N*/ 	SwDistance nDown = nUp;
/*N*/ 	BOOL bNegAllowed = TRUE;//Einmal aus dem negativen Bereich heraus lassen.
/*N*/ 	do
/*N*/ 	{
/*N*/ 		pUpLst = pUpFrm; nUpLst = nUp;
/*N*/ 		pUpFrm = pUpLst->GetPrevCntntFrm();
/*N*/ 		while ( pUpFrm &&
/*N*/ 				(bBody != pUpFrm->IsInDocBody() || bFtn != pUpFrm->IsInFtn()))
/*N*/ 			pUpFrm = pUpFrm->GetPrevCntntFrm();
/*N*/ 		if ( pUpFrm )
/*N*/ 		{
/*N*/ 			::binfilter::lcl_CalcDownDist( nUp, aNew, pUpFrm );
/*N*/ 			//Wenn die Distanz innnerhalb einer Tabelle waechst, so lohnt es
/*N*/ 			//sich weiter zu suchen.
/*N*/ 			if ( pUpLst->IsInTab() && pUpFrm->IsInTab() )
/*N*/ 			{
/*N*/ 				while ( pUpFrm && ((nUpLst < nUp && pUpFrm->IsInTab()) ||
/*N*/ 						bBody != pUpFrm->IsInDocBody()) )
/*N*/ 				{
/*N*/ 					pUpFrm = pUpFrm->GetPrevCntntFrm();
/*N*/ 					if ( pUpFrm )
/*N*/ 						::binfilter::lcl_CalcDownDist( nUp, aNew, pUpFrm );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !pUpFrm )
/*N*/ 			nUp.nMain = LONG_MAX;
/*N*/         if ( nUp.nMain >= 0 && LONG_MAX != nUp.nMain )
/*N*/ 		{
/*N*/ 			bNegAllowed = FALSE;
/*N*/ 			if ( nUpLst.nMain < 0 ) //nicht den falschen erwischen, wenn der Wert
/*N*/ 									//gerade von negativ auf positiv gekippt ist.
/*N*/ 			{	pUpLst = pUpFrm;
/*N*/ 				nUpLst = nUp;
/*N*/ 			}
/*N*/ 		}
/*N*/     } while ( pUpFrm && ( ( bNegAllowed && nUp.nMain < 0 ) || ( nUp <= nUpLst ) ) );
/*N*/ 
/*N*/ 	const SwCntntFrm *pDownLst;
/*N*/ 	const SwCntntFrm *pDownFrm = pCnt;
/*N*/ 	SwDistance nDownLst;
/*N*/ 	if ( nDown.nMain < 0 )
/*N*/ 		nDown.nMain = LONG_MAX;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		pDownLst = pDownFrm; nDownLst = nDown;
/*N*/ 		pDownFrm = pDownLst->GetNextCntntFrm();
/*N*/ 		while ( pDownFrm &&
/*N*/ 				(bBody != pDownFrm->IsInDocBody() || bFtn != pDownFrm->IsInFtn()))
/*N*/ 			pDownFrm = pDownFrm->GetNextCntntFrm();
/*N*/ 		if ( pDownFrm )
/*N*/ 		{
/*N*/ 			::binfilter::lcl_CalcDownDist( nDown, aNew, pDownFrm );
/*N*/ 			if ( nDown.nMain < 0 )
/*N*/ 				nDown.nMain = LONG_MAX;
/*N*/ 			//Wenn die Distanz innnerhalb einer Tabelle waechst, so lohnt es
/*N*/ 			//sich weiter zu suchen.
/*N*/ 			if ( pDownLst->IsInTab() && pDownFrm->IsInTab() )
/*N*/ 			{
/*N*/ 				while ( pDownFrm && ( ( nDown.nMain != LONG_MAX && nDownLst < nDownLst
/*N*/ 						&& pDownFrm->IsInTab()) || bBody != pDownFrm->IsInDocBody() ) )
/*N*/ 				{
/*N*/ 					pDownFrm = pDownFrm->GetNextCntntFrm();
/*N*/ 					if ( pDownFrm )
/*N*/ 						::binfilter::lcl_CalcDownDist( nDown, aNew, pDownFrm );
/*N*/ 					if ( nDown.nMain < 0 )
/*N*/ 						nDown.nMain = LONG_MAX;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if ( !pDownFrm )
/*N*/ 			nDown.nMain = LONG_MAX;
/*N*/ 
/*N*/ 	} while ( pDownFrm && nDown <= nDownLst &&
/*N*/ 			  nDown.nMain != LONG_MAX && nDownLst.nMain != LONG_MAX );
/*N*/ 
/*N*/ 	//Wenn ich in beide Richtungen keinen gefunden habe, so suche ich mir
/*N*/ 	//denjenigen Cntnt dessen linke obere Ecke dem Point am naechsten liegt.
/*N*/ 	//Eine derartige Situation tritt z.b. auf, wenn der Point nicht im Text-
/*N*/ 	//fluss sondern in irgendwelchen Raendern steht.
/*N*/ 	if ( nDownLst.nMain == LONG_MAX && nUpLst.nMain == LONG_MAX )
/*N*/     {
/*N*/         // #102861# If an OLE objects, which is contained in a fly frame
/*N*/         // is resized in inplace mode and the new Position is outside the
/*N*/         // fly frame, we do not want to leave our fly frame.
/*N*/         if ( pCnt->IsInFly() )
/*N*/             return pCnt;
/*N*/ 
/*N*/ 		return ::binfilter::lcl_FindCnt( aNew, pCnt, bBody, bFtn );
/*N*/     }
/*N*/ 	else
/*N*/ 		return nDownLst < nUpLst ? pDownLst : pUpLst;
/*N*/ }

/*************************************************************************
|*
|*	SwFlyAtCntFrm::SetAbsPos()
|*
|*	Ersterstellung		MA 22. Jun. 93
|*	Letzte Aenderung	MA 11. Sep. 98
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwFlyAtCntFrm::MakeFlyPos()
|*
|*	Beschreibung:
|*
|* 		virtueller Anker: Der virtuelle Anker eines Flys ist der Anker selbst
|* 						  oder einer seiner Follows. Es ist genau derjenige
|* 						  Cntnt, der dem Fly aktuell am naechsten liegt,
|* 						  genauer der aktuellen relativen Position des Fly
|* 						  (siehe auch VertPos, Fix). Es wird nur die
|* 						  vertikale Entfernung gemessen.
|* 						  Der virtuelle Anker fuer die Horizontale Ausrichtung
|* 						  muss nicht ein CntntFrm sein, denn wenn der Fly
|* 						  z.B. ueber einer leeren Spalte steht, so muss eben
|* 						  der LayoutFrm als virtueller Anker dienen, der im
|* 						  Textfluss des Ankers liegt.
|*
|* 		HoriPos:
|* 			- Automatisch: Die automatische Ausrichtung orientiert sich
|* 			  an einem SwFrm der folgendermassen ermittelt wird: Abhaengig
|* 			  vom Attriut und ausgehend vom virtuellen Anker wird der
|* 			  Bezugsframe gesucht (CntntFrm, LayoutFrm).
|* 			- Fix: Der Wert der relativen Entfernung aus dem Attribut ist
|* 				   die relative Entfernung vom virtuellen Anker.
|* 		VertPos:
|* 			- Automatisch: Die automatische Ausrichtung orientiert sich immer
|* 			  am virtuellen Anker.
|* 			- Fix: Der Fly muss nicht in der Umgebung untergebracht sein, in
|* 				   der sein Anker steht; er folgt aber stets dem Textfluss dem
|* 				   der Anker folgt. Geclippt (Drawing) wird der Fly am Rootfrm.
|* 				   Damit die erstgenannte Bedingung erreicht wird, wird der
|* 				   Fly ggf. entsprechend verschoben. Dabei bleibt die relative
|* 				   Position des Attributes erhalten, die tatsaechliche relative
|* 				   Position verhaelt sich zu der des Attributes etwa wie ein
|* 				   Teleskoparm. Der Betrag der relativen Position ist die
|* 				   Entfernung zur AbsPos des Ankers im Textfluss.
|*
|* 		Es wird immer zuerst die vertikale Position bestimmt, denn erst dann
|* 		steht der virtuelle Anker fest.
|*		Die tatsaechliche relative Position (Member aRelPos) ist immer die
|* 		die Entfernung zum Anker - sie muss also nicht mit den im Attribut
|* 		angegebenen Werten uebereinstimmen, denn diese geben die Entfernung
|* 		'im Textfluss' an.
|*
|*	Ersterstellung		MA 19. Nov. 92
|*	Letzte Aenderung	MA 14. Nov. 96
|*
|*************************************************************************/

/*N*/ inline void ValidateSz( SwFrm *pFrm )
/*N*/ {
/*N*/ 	if ( pFrm )
/*N*/ 		pFrm->bValidSize = TRUE;
/*N*/ }

/*M*/ void DeepCalc( const SwFrm *pFrm )
/*M*/ {
/*M*/     if( pFrm->IsSctFrm() ||
/*M*/         ( pFrm->IsFlyFrm() && ((SwFlyFrm*)pFrm)->IsFlyInCntFrm() ) )
/*M*/ 		return;
/*M*/     const SwFlowFrm *pFlow = SwFlowFrm::CastFlowFrm( pFrm );
/*M*/     if( pFlow && pFlow->IsAnyJoinLocked() )
/*M*/         return;
/*M*/ 
/*M*/ 	USHORT nCnt = 0;
/*M*/ 
/*M*/ 	FASTBOOL bContinue = FALSE;
/*M*/ 	do
/*M*/ 	{	if ( ++nCnt == 10 )
/*M*/ 		{
/*M*/ 			ASSERT( !nCnt, "DeepCalc: Loop detected1?" );
/*M*/ 			break;
/*M*/ 		}
/*M*/ 
/*M*/ 		const FASTBOOL bSetComplete = !pFrm->IsValid();
/*M*/ 		const SwRect aOldFrm( pFrm->Frm() );
/*M*/ 		const SwRect aOldPrt( pFrm->Prt() );
/*M*/ 
/*M*/ 		const SwFrm *pUp = pFrm->GetUpper();
/*M*/ 		if ( pUp )
/*M*/ 		{
/*M*/ 			//Nicht weiter wenn der Up ein Fly mit Spalten ist.
/*M*/ 			if( ( !pUp->IsFlyFrm() || !((SwLayoutFrm*)pUp)->Lower() ||
/*M*/ 				 !((SwLayoutFrm*)pUp)->Lower()->IsColumnFrm() ) &&
/*M*/ 				 !pUp->IsSctFrm() )
/*M*/ 			{
/*M*/                 SWRECTFN( pUp )
/*M*/                 const Point aPt( (pUp->Frm().*fnRect->fnGetPos)() );
/*M*/ 				::binfilter::DeepCalc( pUp );
/*M*/                 bContinue = aPt != (pUp->Frm().*fnRect->fnGetPos)();
/*M*/ 			}
/*M*/ 		}
/*M*/ 		else
/*M*/ 			pUp = pFrm;
/*M*/ 
/*M*/ 		pFrm->Calc();
/*M*/ 		if ( bSetComplete && (aOldFrm != pFrm->Frm() || aOldPrt != pFrm->Prt()))
/*M*/ 			pFrm->SetCompletePaint();
/*M*/ 
/*M*/ //		bContinue = !pUp->IsValid();
/*M*/ 		if ( pUp->IsFlyFrm() )
/*M*/ 		{
/*M*/ 			if ( ((SwFlyFrm*)pUp)->IsLocked() ||
/*M*/ 				 (((SwFlyFrm*)pUp)->IsFlyAtCntFrm() &&
/*M*/ 				  SwOszControl::IsInProgress( (const SwFlyFrm*)pUp )) )
/*M*/ 			{
/*M*/ 				bContinue = FALSE;
/*M*/ 			}
/*M*/ 		}
/*M*/ 	} while ( bContinue );
/*M*/ }

//Ermittlung des virtuellen Ankers fuer die Positionierung.
//Dieser ist entweder der Anker selbst oder einer seiner Follows.

/*N*/ const SwCntntFrm *GetVirtualAnchor( const SwFlyAtCntFrm *pFly, xub_StrLen nOfs )
/*N*/ {
/*N*/ 	const SwTxtFrm *pAct = (const SwTxtFrm*)pFly->GetAnchor();
/*N*/ 	const SwTxtFrm* pTmp;
/*N*/ 	do
/*N*/ 	{
/*N*/ 		pTmp = pAct;
/*N*/ 		pAct = pTmp->GetFollow();
/*N*/ 	}
/*N*/ 	while( pAct && nOfs >= pAct->GetOfst() );
/*N*/ 	return pTmp;
/*N*/ }

//Ermittlung des virtuellen Ankers, an dem sich die horizontale Ausrichtung
//orientieren muss.
//pAssumed enthaelt entweder bereits den Anker (Es ist dann der Anker des
//Flys oder einer seiner Follows) oder die Umgebung die der Orientierung,
//mangels einer besseren Moeglichkeit, dienen muss.

/*N*/ const SwFrm *GetVirtualHoriAnchor( const SwFrm *pAssumed, const SwFlyFrm *pFly )
/*N*/ {
/*N*/ 	const SwFrm *pRet = pAssumed;
/*N*/ 
/*N*/ 	if ( !pRet->IsCntntFrm() )
/*N*/ 	{	//Wenn es Lower gibt, die selbst der Anker des Fly oder ein Follow
/*N*/ 		//desselben sind, so wird derjenige ausgewaehlt, der der aktuellen
/*N*/ 		//absoluten vertikalen Position des Fly am naechsten steht.
/*N*/ 		//Gibt es keinen, so bleib es bei pAssumed
/*N*/ 		const SwFrm *pFlow = ((SwLayoutFrm*)pRet)->Lower();
/*N*/ 		SwTwips nCntDiff = LONG_MAX;
/*N*/ 		while ( pFlow )
/*N*/ 		{
/*N*/ 			if ( pFlow->IsCntntFrm() &&
/*N*/ 				 ((SwCntntFrm*)pFly->GetAnchor())->IsAnFollow( (SwCntntFrm*)pFlow ) )
/*N*/ 			{
/*N*/                 SWRECTFN( pFlow )
/*N*/                 SwTwips nDiff = (pFly->Frm().*fnRect->fnGetTop)() -
/*N*/                                 (pFlow->Frm().*fnRect->fnGetTop)();
/*N*/ 				if ( (nDiff = Abs(nDiff)) < nCntDiff )
/*N*/ 				{
/*N*/ 					pRet = pFlow;			//Der ist dichter dran
/*N*/ 					nCntDiff = nDiff;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pFlow = pFlow->GetNext();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ 
/*N*/ }

/*N*/ void SwFlyAtCntFrm::AssertPage()
/*N*/ {
/*N*/ 	//Prueft ob der Fly an der Seite haengt, auf der er steht, falls nicht
/*N*/ 	//wird er umgehaengt. Zur Pruefung wird nur die vertikale Ausrichtung
/*N*/ 	//herangezogen.
/*N*/ 
/*N*/ 	SwPageFrm *pNewPage = FindPageFrm();
/*N*/ 	SwPageFrm *pMyPage	= pNewPage;
/*N*/ 	BOOL bSuperfluous = FALSE;
/*N*/ 
/*N*/ 	//#45516# Ausnahmebehandlung. Eine Tabelle ist zu gross und haengt aus der
/*N*/ 	//Seite heraus. Der Rahmen kann dann zwar richtig bei seinem Anker stehen,
/*N*/ 	//Positionsmaessig aber ueber der naechsten Seite haengen. Damit das dann
/*N*/ 	//noch halbwegs brauchbar gedruckt werden kann (HTML) und der Rahmen nicht
/*N*/ 	//wirr in der Gegend gepaintet wird, wird der Rahmen bei der Seite verankert,
/*N*/ 	//auf der auch sein Anker sitzt.
/*N*/ 	if ( GetAnchor()->GetValidSizeFlag() &&
/*N*/ 		 Frm().Top() >= GetAnchor()->Frm().Top() &&
/*N*/ 		 Frm().Top() < GetAnchor()->Frm().Bottom() )
/*N*/ 	{
/*N*/ 		pNewPage = GetAnchor()->FindPageFrm();
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		BOOL bFound = FALSE;
/*N*/ 		const BOOL bFtn = GetAnchor()->IsInFtn();
/*N*/ 		int nDir = INT_MAX;	// 1 == Forward, 2 == Backward.
/*N*/ 		while ( !bFound )
/*N*/ 		{
/*N*/ 			pNewPage->Calc();
/*N*/ 			if ( Frm().Top() < pNewPage->Frm().Top() && pNewPage->GetPrev() )
/*N*/ 			{
/*N*/ 				pNewPage = (SwPageFrm*)pNewPage->GetPrev();
/*N*/                 // OD 19.02.2003 #105643# - skip empty page and consider empty
/*N*/                 // page at the beginning of the document.
/*N*/                 // Assumption about document layout:
/*N*/                 //      No two empty pages following each other.
/*N*/                 if ( pNewPage->IsEmptyPage() )
/*N*/                 {
/*N*/                     if ( pNewPage->GetPrev() )
/*N*/                     {
/*N*/                         pNewPage = static_cast<SwPageFrm*>(pNewPage->GetPrev());
/*N*/                     }
/*N*/                     else
/*N*/                     {
/*N*/                         bFound = TRUE;
/*N*/                         pNewPage = static_cast<SwPageFrm*>(pNewPage->GetNext());
/*N*/                     }
/*N*/                 }
/*N*/ 				if ( nDir == 2 )
/*N*/ 				{
/*?*/ 					bFound = TRUE;
/*?*/ 					pNewPage = GetAnchor()->FindPageFrm();
/*N*/ 				}
/*N*/ 				else
/*N*/ 					nDir = 1;
/*N*/ 			}
/*N*/ 			else if ( Frm().Top() > pNewPage->Frm().Bottom() )
/*N*/ 			{
/*N*/ 				if ( nDir == 1 )
/*N*/ 				{
/*?*/ 					bFound = TRUE;
/*?*/ 					pNewPage = GetAnchor()->FindPageFrm();
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					nDir = 2;
/*N*/ 					if ( !pNewPage->GetNext() )
/*N*/ 					{
/*?*/ 						pNewPage->GetLeaf( bFtn ? MAKEPAGE_NONE : MAKEPAGE_APPEND,
/*?*/ 											TRUE, GetAnchor());
/*?*/ 						bSuperfluous = TRUE;
/*N*/ 					}
/*N*/ 					if ( pNewPage->GetNext() )
/*N*/ 					{
/*N*/ 						pNewPage = (SwPageFrm*)pNewPage->GetNext();
/*N*/ 						if( pNewPage->IsEmptyPage() )
/*N*/ 						{
/*?*/ 							if( pNewPage->GetNext() )
/*?*/ 								pNewPage = (SwPageFrm*)pNewPage->GetNext();
/*?*/ 							else
/*?*/ 							{
/*?*/ 								bFound = TRUE;
/*?*/ 								pNewPage = (SwPageFrm*)pNewPage->GetPrev();
/*?*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*?*/ 						bFound = TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 				bFound = TRUE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if ( pMyPage != pNewPage )
/*N*/ 	{
/*N*/ 		ASSERT( IsLocked(), "AssertPage: Unlocked Frame??" );
/*N*/ 		pMyPage->MoveFly( this, pNewPage );
/*N*/ 		if ( bSuperfluous && pMyPage->GetPhyPageNum() > pNewPage->GetPhyPageNum() )
/*?*/ 			((SwRootFrm*)pNewPage->GetUpper())->SetSuperfluous();
/*N*/ 	}
/*N*/ 
/*N*/ }

/*N*/ BOOL MA_FASTCALL lcl_IsMoveable( SwFlyFrm *pFly, SwLayoutFrm *pLay )
/*N*/ {
/*N*/ 	//Waere der Anker auch in der neuen Umgebung noch moveable?
/*N*/ 	BOOL bRet;
/*N*/ 	SwLayoutFrm *pUp = pFly->GetAnchor()->GetUpper();
/*N*/ 	SwFrm *pNext = pFly->GetAnchor()->GetNext();
/*N*/ 	pFly->GetAnchor()->Remove();
/*N*/ 	pFly->GetAnchor()->InsertBefore( pLay, pLay->Lower() );
/*N*/ 	bRet = pFly->GetAnchor()->IsMoveable();
/*N*/ 	pFly->GetAnchor()->Remove();
/*N*/ 	pFly->GetAnchor()->InsertBefore( pUp, pNext );
/*N*/ 	return bRet;
/*N*/ 
/*N*/ }

// Wer weicht wem aus bzw. welcher Bereich ist "linker"/"rechter" als welcher?
/*N*/ BOOL MA_FASTCALL lcl_Minor( SwRelationOrient eRelO, SwRelationOrient eRelO2,
/*N*/ 	BOOL bLeft )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 
/*N*/ }

/*N*/ void SwFlyAtCntFrm::MakeFlyPos()
/*N*/ {
/*N*/     /// OD 02.10.2002 #102646#
/*N*/     /// if fly frame position is valid, nothing is to do, Thus, return
/*N*/     if ( bValidPos )
/*N*/ 	{
/*N*/         return;
/*N*/     }
/*N*/ 
/*N*/     /// OD 02.10.2002 #102646# - NOTE
/*N*/     /// declare and set <pFooter> to footer frame, if fly frame is anchored
/*N*/     /// at a frame belonging to the footer.
/*N*/     const SwFrm* pFooter = GetAnchor()->FindFooterOrHeader();
/*N*/     if( pFooter && !pFooter->IsFooterFrm() )
/*N*/         pFooter = NULL;
/*N*/ 
/*N*/     /// OD 02.10.2002 #102646# - NOTE
/*N*/     /// declare and set <bBrowse> to TRUE, if document is in browser mode and
/*N*/     /// fly frame is anchored at the body, but not at frame belonging to a table.
/*N*/     const FASTBOOL bBrowse = GetAnchor()->IsInDocBody() && !GetAnchor()->IsInTab() ?
/*N*/                                  GetFmt()->GetDoc()->IsBrowseMode() : FALSE;
/*N*/ 
/*N*/     /// OD 02.10.2002 #102646# - NOTE
/*N*/     /// declare and init <bInvalidatePage> to FALSE, in order to invalidate
/*N*/     /// page size, if <bInvalidatePage> is set during the calculation of the
/*N*/     /// fly frame position.
/*N*/     FASTBOOL bInvalidatePage = FALSE;
/*N*/ 
/*N*/     /// OD 02.10.2002 #102646# - NOTE
/*N*/     /// determine fly frame format and its left/right and its upper/lower spacing.
/*N*/     SwFlyFrmFmt *pFmt = (SwFlyFrmFmt*)GetFmt();
/*N*/     const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();
/*N*/     const SvxULSpaceItem &rUL = pFmt->GetULSpace();
/*N*/ 
/*N*/     /// OD 02.10.2002 #102646# - NOTE
/*N*/     /// determine, if fly frame has no surrounding.
/*N*/     const SwFmtSurround& rSurround = pFmt->GetSurround();
/*N*/     const FASTBOOL bNoSurround =
/*N*/         rSurround.GetSurround() == SURROUND_NONE;
/*N*/     const FASTBOOL bWrapThrough =
/*N*/         rSurround.GetSurround() == SURROUND_THROUGHT;
/*N*/ 
/*N*/     BOOL bGrow =
/*N*/         !GetAnchor()->IsInTab() || !pFmt->GetFrmSize().GetHeightPercent();
/*N*/ 
/*N*/     for (;;)
/*N*/     {
/*N*/ 		bValidPos = TRUE;
/*N*/ 		if( !pFooter )
/*N*/ 			::binfilter::DeepCalc( GetAnchor() );
/*N*/ 		bValidPos = TRUE;
/*N*/ 
/*N*/ 		//Die Werte in den Attributen muessen ggf. upgedated werden,
/*N*/ 		//deshalb werden hier Attributinstanzen und Flags benoetigt.
/*N*/ 		SwFmtVertOrient aVert( pFmt->GetVertOrient() );
/*N*/ 		SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
/*N*/ 		BOOL bVertChgd = FALSE,
/*N*/ 			 bHoriChgd = FALSE,
/*N*/ 			 bMoveable = GetAnchor()->IsMoveable();
/*N*/ 
/*N*/ 		//Wird waehrend der Berechnung der vertikalen Position benutzt
/*N*/ 		//und enthaelt hinterher den Frm, an dem sich die horizontale
/*N*/ 		//Positionierung orientieren muss.
/*N*/ 		const SwFrm *pOrient = GetAnchor();
/*N*/ 
/*N*/ 		// Dies wird der Frame, der das Zeichen eines am Zeichen gebundenen
/*N*/ 		// Rahmens enthaelt.
/*N*/ 		const SwFrm *pAutoOrient = pOrient;
/*N*/ 
/*N*/ 		SwRect *pAutoPos;
/*N*/ 		if( FLY_AUTO_CNTNT == pFmt->GetAnchor().GetAnchorId() )
/*N*/ 		{
/*N*/ 			const SwFmtAnchor& rAnch = pFmt->GetAnchor();
/*N*/ 			if( !aLastCharRect.Height() &&
/*N*/ 				!((SwTxtFrm*)GetAnchor())->GetAutoPos( aLastCharRect,
/*N*/ 													*rAnch.GetCntntAnchor() ) )
/*N*/ 				return;
/*N*/ 			pAutoPos = &aLastCharRect;
/*N*/ 			pAutoOrient = ::binfilter::GetVirtualAnchor( this, rAnch.GetCntntAnchor()->
/*N*/ 											  nContent.GetIndex() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pAutoPos = NULL;
/*N*/ 
/*N*/ 		//Horizontale und vertikale Positionen werden getrennt berechnet.
/*N*/ 		//Sie koennen jeweils Fix oder Variabel sein.
/*N*/         SWRECTFN( pAutoOrient )
/*N*/ 
/*N*/ 		//Zuerst die vertikale Position, damit feststeht auf welcher Seite
/*N*/ 		//bzw. in welchen Upper sich der Fly befindet.
/*N*/ 		if ( aVert.GetVertOrient() != VERT_NONE )
/*N*/ 		{
/*N*/ 			pOrient = pAutoOrient;
/*N*/ 			if( !pFooter )
/*N*/ 				::binfilter::DeepCalc( pOrient );
/*N*/ 			SwTwips nHeight, nAdd;
/*N*/ 			if ( aVert.GetRelationOrient() == PRTAREA )
/*N*/ 			{
/*N*/                 nHeight = (pOrient->Prt().*fnRect->fnGetHeight)();
/*N*/                 nAdd = (pOrient->*fnRect->fnGetTopMargin)();
/*N*/ 			}
/*N*/ 			else if( pAutoPos && REL_CHAR == aVert.GetRelationOrient() )
/*N*/ 			{
/*N*/                 nHeight = (pAutoPos->*fnRect->fnGetHeight)();
/*N*/                 nAdd = (*fnRect->fnYDiff)( (pAutoPos->*fnRect->fnGetTop)(),
/*N*/                                         (pOrient->Frm().*fnRect->fnGetTop)() );
/*N*/ 			}
/*N*/ 			else
/*N*/             {   nHeight = (pOrient->Frm().*fnRect->fnGetHeight)();
/*N*/ 				nAdd = 0;
/*N*/ 			}
/*N*/             SwTwips nRelPosY;
/*N*/             SwTwips nFrmHeight = (Frm().*fnRect->fnGetHeight)();
/*N*/ 			if ( aVert.GetVertOrient() == VERT_CENTER )
/*N*/                 nRelPosY = (nHeight / 2) - (nFrmHeight / 2);
/*N*/             else
/*N*/             {
/*N*/                 SwTwips nUpper = bVert ? rLR.GetRight() : rUL.GetUpper();
/*N*/                 if ( aVert.GetVertOrient() == VERT_BOTTOM )
/*N*/                 {
/*N*/                     if( bNoSurround )
/*N*/                         nRelPosY = nHeight + nUpper;
/*N*/                     else
/*N*/                         nRelPosY = nHeight - (nFrmHeight + ( bVert ?
/*N*/                                               rLR.GetLeft() : rUL.GetLower()));
/*N*/                 }
/*N*/                 else if( pAutoPos && aVert.GetVertOrient() == VERT_CHAR_BOTTOM )
/*N*/                 {
/*N*/                     nRelPosY = nHeight + nUpper;
/*N*/                     if( bVert )
/*N*/                         nRelPosY += aFrm.Width();
/*N*/                 }
/*N*/                 else
/*N*/                     nRelPosY = nUpper;
/*N*/             }
/*N*/             nRelPosY += nAdd;
/*N*/             SwTwips nOTop = (pOrient->Frm().*fnRect->fnGetTop)();
/*N*/             SwTwips nBot = nRelPosY + nFrmHeight + (*fnRect->fnYDiff)( nOTop,
/*N*/                               (pOrient->GetUpper()->*fnRect->fnGetPrtBottom)());
/*N*/ 			if( nBot > 0 )
/*N*/                 nRelPosY -= nBot;
/*N*/             if( nRelPosY < 0 )
/*N*/                 nRelPosY = 0;
/*N*/ 			//Da die relative Position immer zum Anker relativ ist, muss dessen
/*N*/ 			//Entfernung zum virtuellen Anker aufaddiert werden.
/*N*/ 			if ( GetAnchor() != pOrient )
/*N*/                 nRelPosY += (*fnRect->fnYDiff)( nOTop,
/*N*/                                       (GetAnchor()->Frm().*fnRect->fnGetTop)());
/*N*/             if ( nRelPosY != aVert.GetPos() )
/*N*/             {   aVert.SetPos( nRelPosY );
/*N*/ 				bVertChgd = TRUE;
/*N*/ 			}
/*N*/             if( bVert )
/*N*/                 aRelPos.X() = nRelPosY;
/*N*/             else
/*N*/                 aRelPos.Y() = nRelPosY;
/*N*/ 		}
/*N*/ 
/*N*/ 		pOrient = aVert.GetVertOrient() == VERT_NONE ?
/*N*/ 				  GetAnchor()->GetUpper() :	pAutoOrient->GetUpper();
/*N*/ 		if( !pFooter )
/*N*/ 			::binfilter::DeepCalc( pOrient );
/*N*/ 
/*N*/ 		SwTwips nRelDiff = 0;
/*N*/ 		if ( aVert.GetVertOrient() == VERT_NONE )
/*N*/ 		{
/*N*/             /// OD 02.10.2002 #102646# - NOTE
/*N*/             /// local variable <nRel> for calculation of relative vertical
/*N*/             /// distance to anchor.
/*N*/ 			SwTwips nRel;
/*N*/ 			if( pAutoPos && REL_CHAR == aVert.GetRelationOrient() )
/*N*/ 			{
/*N*/                 nRel = (*fnRect->fnYDiff)( (pAutoPos->*fnRect->fnGetBottom)(),
/*N*/                                      (pAutoOrient->Frm().*fnRect->fnGetTop)() );
/*N*/                 nRel -= aVert.GetPos();
/*N*/ 				if( pAutoOrient != GetAnchor() )
/*N*/ 				{
/*N*/ 					SwTxtFrm* pTmp = (SwTxtFrm*)GetAnchor();
/*N*/                     SWREFRESHFN( pTmp )
/*N*/                     nRel -=(*fnRect->fnYDiff)((pTmp->Frm().*fnRect->fnGetTop)(),
/*N*/                                     (pTmp->GetUpper()->*fnRect->fnGetPrtTop)());
/*N*/                     while( pTmp != pAutoOrient )
/*N*/ 					{
/*N*/                         SWREFRESHFN( pTmp )
/*N*/                         nRel +=(pTmp->GetUpper()->Prt().*fnRect->fnGetHeight)();
/*N*/ 						pTmp = pTmp->GetFollow();
/*N*/ 					}
/*N*/                     SWREFRESHFN( pTmp )
/*N*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/             {
/*N*/                 /// OD 02.10.2002 #102646#
/*N*/                 /// consider that vertical position can be relativ to "margin"
/*N*/                 /// or to "text area".
/*N*/                 /// Thus, increase <nRel> by margin height, if position is
/*N*/                 /// vertical to "text area"
/*N*/                 nRel = aVert.GetPos();
/*N*/                 if ( aVert.GetRelationOrient() == PRTAREA )
/*N*/                 {
/*N*/                     nRel += (pAutoOrient->*fnRect->fnGetTopMargin)();
/*N*/                 }
/*N*/             }
/*N*/ 
/*N*/ 			// Einen einspaltigen Bereich koennen wir getrost ignorieren,
/*N*/ 			// er hat keine Auswirkung auf die Fly-Position
/*N*/ 			while( pOrient->IsSctFrm() )
/*N*/ 				pOrient = pOrient->GetUpper();
/*N*/ 			//pOrient ist das LayoutBlatt, das gerade verfolgt wird.
/*N*/ 			//nRel	  enthaelt die noch zu verarbeitende relative Entfernung.
/*N*/ 			//nAvail  enthaelt die Strecke die im LayoutBlatt, das gerade
/*N*/ 			//		  verfolgt wird zur Verfuegung steht.
/*N*/ 
/*N*/             if( nRel <= 0 )
/*N*/             {
/*N*/                 if( bVert )
/*N*/                     aRelPos.X() = 0;
/*N*/                 else
/*N*/                     aRelPos.Y() = 0;
/*N*/             }
/*N*/ 			else
/*N*/ 			{
/*N*/                 SWREFRESHFN( GetAnchor() )
/*N*/                 SwTwips nAvail =
/*N*/                     (*fnRect->fnYDiff)( (pOrient->*fnRect->fnGetPrtBottom)(),
/*N*/                                       (GetAnchor()->Frm().*fnRect->fnGetTop)());
/*N*/ 				const BOOL bFtn = GetAnchor()->IsInFtn();
/*N*/ 				while ( nRel )
/*N*/ 				{	if ( nRel <= nAvail ||
/*N*/ 							(bBrowse &&
/*N*/                             ((SwFrm*)pOrient)->Grow( nRel-nAvail, TRUE)) ||
/*N*/                             (pOrient->IsInTab() && bGrow &&
/*N*/                             ((SwFrm*)pOrient)->Grow( nRel-nAvail, TRUE)))
/*N*/                     {
/*N*/                         if( bVert )
/*N*/                             aRelPos.X() = GetAnchor()->Frm().Left() +
/*N*/                                           GetAnchor()->Frm().Width() -
/*N*/                                           pOrient->Frm().Left() -
/*N*/                                           pOrient->Prt().Left() - nAvail + nRel;
/*N*/                         else
/*N*/                             aRelPos.Y() = pOrient->Frm().Top() +
/*N*/                                 pOrient->Prt().Top() + pOrient->Prt().Height()
/*N*/                                 - nAvail + nRel - GetAnchor()->Frm().Top();
/*N*/ 						if ( ( bBrowse || ( pOrient->IsInTab() && bGrow ) )
/*N*/ 							 && nRel - nAvail > 0 )
/*N*/ 						{
/*N*/                             nRel = ((SwFrm*)pOrient)->Grow( nRel-nAvail );
/*N*/ 							SwFrm *pTmp = (SwFrm*) pOrient->FindPageFrm();
/*N*/ 							::binfilter::ValidateSz( pTmp );
/*N*/ 							bInvalidatePage = TRUE;
/*N*/ 							//Schon mal einstellen, weil wir wahrscheinlich
/*N*/ 							//wegen Invalidierung eine Ehrenrunde drehen.
/*N*/                             if( bVert )
/*N*/                                 aFrm.Pos().X() = aFrm.Left() - nRel;
/*N*/                             else
/*N*/                                 aFrm.Pos().Y() = aFrm.Top() + nRel;
/*N*/ 						}
/*N*/ 						nRel = 0;
/*N*/ 					}
/*N*/ 					else if ( bMoveable )
/*N*/ 					{	//Dem Textfluss folgen.
/*N*/ 						nRel -= nAvail;
/*N*/ 						const BOOL bSct = pOrient->IsInSct();
/*N*/ 						MakePageType eMakePage = bFtn ? MAKEPAGE_NONE
/*N*/ 													  : MAKEPAGE_APPEND;
/*N*/ 						if( bSct )
/*N*/ 							eMakePage = MAKEPAGE_NOSECTION;
/*N*/ 						const SwFrm *pTmp = pOrient->
/*N*/ 							GetLeaf( eMakePage,	TRUE, GetAnchor() );
/*N*/ 						if ( pTmp && ( !bSct || pOrient->FindSctFrm()->
/*N*/ 								IsAnFollow( pTmp->FindSctFrm() ) ) )
/*N*/ 						{
/*N*/ 							pOrient = pTmp;
/*N*/ 							bMoveable =
/*N*/ 									::binfilter::lcl_IsMoveable( this, (SwLayoutFrm*)pOrient);
/*N*/ 							if( !pFooter )
/*N*/ 								::binfilter::DeepCalc( pOrient );
/*N*/                             SWREFRESHFN( pOrient )
/*N*/                             nAvail = (pOrient->Prt().*fnRect->fnGetHeight)();
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							// Wenn wir innerhalb des (spaltigen) Bereichs nicht genug
/*N*/ 							// Platz ist, wird es Zeit, diesen zu verlassen. Wir gehen
/*N*/ 							// also in seinen Upper und nehmen als nAvail den Platz, der
/*N*/ 							// hinter dem Bereich ist. Sollte dieser immer noch nicht
/*N*/ 							// ausreichen, wandern wir weiter, es hindert uns aber nun
/*N*/ 							// niemand mehr, neue Seiten anzulegen.
/*N*/ 							if( bSct )
/*N*/ 							{
/*N*/ 								const SwFrm* pSct = pOrient->FindSctFrm();
/*N*/ 								pOrient = pSct->GetUpper();
/*N*/                                 nAvail = (*fnRect->fnYDiff)(
/*N*/                                            (pOrient->*fnRect->fnGetPrtBottom)(),
/*N*/                                            (pSct->*fnRect->fnGetPrtBottom)() );
/*N*/ 							}
/*N*/ 							else
/*N*/ 							{
/*N*/ 								nRelDiff = nRel;
/*N*/ 								nRel = 0;
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					else
/*N*/ 						nRel = 0;
/*N*/ 				}
/*N*/ 				if ( !bValidPos )
/*N*/ 					continue;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		//Damit das Teil ggf. auf die richtige Seite gestellt und in die
/*N*/ 		//PrtArea des LayLeaf gezogen werden kann, muss hier seine
/*N*/ 		//absolute Position berechnet werden.
/*N*/         if( bVert )
/*N*/         {
/*N*/             aFrm.Pos().X() = GetAnchor()->Frm().Left() - aFrm.Width() +
/*N*/                              GetAnchor()->Frm().Width() - aRelPos.X() +nRelDiff;
/*N*/         }
/*N*/         else
/*N*/             aFrm.Pos().Y() = GetAnchor()->Frm().Top() +
/*N*/                              (aRelPos.Y() - nRelDiff);
/*N*/ 
/*N*/ 		//Bei automatischer Ausrichtung nicht ueber die Oberkante hinausschiessen.
/*N*/ 		if ( aVert.GetVertOrient() != VERT_NONE )
/*N*/ 		{
/*N*/             SwTwips nTop;
/*N*/ 			if ( aVert.GetRelationOrient() == PRTAREA )
/*N*/                 nTop = (pOrient->*fnRect->fnGetPrtTop)();
/*N*/             else
/*N*/                 nTop = (pOrient->Frm().*fnRect->fnGetTop)();
/*N*/             SwTwips nTmp = (Frm().*fnRect->fnGetTop)();
/*N*/             if( (nTmp = (fnRect->fnYDiff)( nTmp, nTop ) ) < 0 )
/*N*/ 			{
/*N*/                 if( bVert )
/*N*/                 {
/*N*/                     aFrm.Pos().X() += nTmp;
/*N*/                     aRelPos.X() = nTop - GetAnchor()->Frm().Left()
/*N*/                                        - GetAnchor()->Frm().Width();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     aFrm.Pos().Y() = nTop;
/*N*/                     aRelPos.Y() = nTop - GetAnchor()->Frm().Top();
/*N*/                 }
/*N*/ 				bHeightClipped = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 
/*N*/ 		const BOOL bFtn = GetAnchor()->IsInFtn();
/*N*/ 		while( pOrient->IsSctFrm() )
/*N*/ 			pOrient = pOrient->GetUpper();
/*N*/         SwTwips nDist = (aFrm.*fnRect->fnBottomDist)(
/*N*/                          (pOrient->*fnRect->fnGetPrtBottom)() );
/*N*/         if( nDist < 0 )
/*N*/ 		{
/*N*/             if( ( bBrowse && GetAnchor()->IsMoveable() ) ||
/*N*/                 ( GetAnchor()->IsInTab() && bGrow ) )
/*N*/ 			{
/*N*/                 ((SwFrm*)pOrient)->Grow( -nDist );
/*N*/ 				SwFrm *pTmp = (SwFrm*) pOrient->FindPageFrm();
/*N*/ 				::binfilter::ValidateSz( pTmp );
/*N*/ 				bInvalidatePage = TRUE;
/*N*/ 			}
/*N*/ 
/*N*/             nDist = (aFrm.*fnRect->fnBottomDist)(
/*N*/                      (pOrient->*fnRect->fnGetPrtBottom)() );
/*N*/             while( bMoveable && nDist < 0 )
/*N*/ 			{
/*N*/ 				// Vorsicht, auch innerhalb von Bereichen duerfen keine neuen Seiten angelegt werden
/*N*/                 BOOL bSct = pOrient->IsInSct();
/*N*/                 if( bSct )
/*N*/ 				{
/*N*/ 					const SwFrm* pTmp = pOrient->FindSctFrm()->GetUpper();
/*N*/                     nDist = (aFrm.*fnRect->fnBottomDist)(
/*N*/                          (pTmp->*fnRect->fnGetPrtBottom)() );
/*N*/                     if( nDist < 0 )
/*N*/ 						pOrient = pTmp;
/*N*/ 					else
/*N*/ 						break;
/*N*/                     bSct = pOrient->IsInSct();
/*N*/                 }
/*N*/                 if( !bSct && (Frm().*fnRect->fnGetTop)() ==
/*N*/                              (pOrient->*fnRect->fnGetPrtTop)() )
/*N*/ 					//Das teil passt nimmer, da hilft auch kein moven.
/*N*/ 					break;
/*N*/ 
/*N*/ 				const SwLayoutFrm *pNextLay = pOrient->GetLeaf( bSct ?
/*N*/ 					MAKEPAGE_NOSECTION : bFtn ? MAKEPAGE_NONE : MAKEPAGE_APPEND,
/*N*/ 					TRUE, GetAnchor() );
/*N*/                 if( pNextLay )
/*N*/                 {
/*N*/                     SWRECTFNX( pNextLay )
/*N*/                     if( !bSct || ( pOrient->FindSctFrm()->IsAnFollow(
/*N*/                         pNextLay->FindSctFrm() ) &&
/*N*/                         (pNextLay->Prt().*fnRectX->fnGetHeight)() ) )
/*N*/                     {
/*N*/                         if( !pFooter )
/*N*/                             ::binfilter::DeepCalc( pNextLay );
/*N*/                         if( bVertX )
/*N*/                             aRelPos.X() = GetAnchor()->Frm().Left() +
/*N*/                                           GetAnchor()->Frm().Width() -
/*N*/                                           pNextLay->Frm().Left() -
/*N*/                                           pNextLay->Prt().Left()-
/*N*/                                           pNextLay->Prt().Width();
/*N*/                         else
/*N*/                             aRelPos.Y() = pNextLay->Frm().Top() +
/*N*/                                 pNextLay->Prt().Top() -GetAnchor()->Frm().Top();
/*N*/                         pOrient = pNextLay;
/*N*/                         SWREFRESHFN( pOrient )
/*N*/                         bMoveable = ::binfilter::lcl_IsMoveable( this,
/*N*/                                                         (SwLayoutFrm*)pOrient );
/*N*/                         if ( bMoveable && !pFooter )
/*N*/                             ::binfilter::DeepCalc( pOrient );
/*N*/                         if( bVertX )
/*N*/                             aFrm.Pos().X() = GetAnchor()->Frm().Left()
/*N*/                                              + GetAnchor()->Frm().Width()
/*N*/                                              - aRelPos.X() - aFrm.Width();
/*N*/                         else
/*N*/                             aFrm.Pos().Y() = GetAnchor()->Frm().Top()
/*N*/                                              + aRelPos.Y();
/*N*/                         nDist = (aFrm.*fnRect->fnBottomDist)(
/*N*/                              (pOrient->*fnRect->fnGetPrtBottom)() );
/*N*/                     }
/*N*/ 				}
/*N*/ 				else if( bSct )
/*N*/ 				{
/*N*/ 					// Wenn wir innerhalb des Bereich nicht genug Platz haben, gucken
/*N*/ 					// wir uns mal die Seite an.
/*N*/ 					const SwFrm* pTmp = pOrient->FindSctFrm()->GetUpper();
/*N*/                     nDist = (aFrm.*fnRect->fnBottomDist)(
/*N*/                          (pTmp->*fnRect->fnGetPrtBottom)() );
/*N*/                     if( nDist < 0 )
/*N*/ 						pOrient = pTmp;
/*N*/ 					else
/*N*/ 						break;
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bMoveable = FALSE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		AssertPage();
/*N*/ 
/*N*/ 		//Horizontale Ausrichtung.
/*N*/ 		//Die absolute Pos in der vertikalen muss schon mal eingestellt
/*N*/ 		//werden, sonst habe ich Schwierigkeiten den virtuellen Anker
/*N*/ 		//zu ermitteln.
/*N*/         if( bVert )
/*N*/             aFrm.Pos().X() = GetAnchor()->Frm().Left() - aFrm.Width() +
/*N*/                              GetAnchor()->Frm().Width() - aRelPos.X();
/*N*/         else
/*N*/             aFrm.Pos().Y() = aRelPos.Y() + GetAnchor()->Frm().Top();
/*N*/         //Den Frm besorgen, an dem sich die horizontale Ausrichtung orientiert.
/*N*/ 		pOrient = ::binfilter::GetVirtualHoriAnchor( pOrient, this );
/*N*/ 
/*N*/ 		if( !pFooter )
/*N*/ 			::binfilter::DeepCalc( pOrient );
/*N*/ 
/*N*/ 		// Achtung: pPage ist nicht unbedingt ein PageFrm, es kann auch ein
/*N*/ 		// SwFlyFrm oder SwCellFrm dahinterstecken
/*N*/ 		const SwFrm *pPage = pOrient;
/*N*/ 		while( !pPage->IsPageFrm() && !pPage->IsFlyFrm() && !pPage->IsCellFrm() )
/*N*/ 		{
/*N*/ 			ASSERT( pPage->GetUpper(), "MakeFlyPos: No Page/FlyFrm Found" );
/*N*/ 			pPage = pPage->GetUpper();
/*N*/ 		}
/*N*/ 
/*N*/ 		const BOOL bEven = !pPage->OnRightPage();
/*N*/ 		const BOOL bToggle = aHori.IsPosToggle() && bEven;
/*N*/ 		BOOL bTmpToggle = bToggle;
/*N*/ 		BOOL bPageRel = FALSE;
/*N*/ 		SwTwips nWidth, nAdd;
/*N*/         SWREFRESHFN( pOrient )
/*N*/ 		switch ( aHori.GetRelationOrient() )
/*N*/ 		{
/*N*/ 			case PRTAREA:
/*N*/ 			{
/*N*/                 nWidth = (pOrient->Prt().*fnRect->fnGetWidth)();
/*N*/                 nAdd = (pOrient->*fnRect->fnGetLeftMargin)();
/*N*/                 if ( pOrient->IsTxtFrm() )
/*N*/                     nAdd += ((SwTxtFrm*)pOrient)->GetBaseOfstForFly( !bWrapThrough );
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case REL_PG_LEFT:
/*N*/ 				bTmpToggle = !bToggle;
/*N*/ 				// kein break;
/*N*/ 			case REL_PG_RIGHT:
/*N*/ 			{
/*N*/ 				if ( bTmpToggle )    // linker Seitenrand
/*N*/                 {
/*N*/                     nAdd = (*fnRect->fnXDiff)((pPage->Frm().*fnRect->fnGetLeft)(),
/*N*/                                          (pOrient->Frm().*fnRect->fnGetLeft)());
/*N*/                     nWidth = (pPage->*fnRect->fnGetLeftMargin)();
/*N*/                 }
/*N*/ 				else			// rechter Seitenrand
/*N*/ 				{
/*N*/                     nAdd = (*fnRect->fnXDiff)((pPage->*fnRect->fnGetPrtRight)(),
/*N*/                                          (pOrient->Frm().*fnRect->fnGetLeft)());
/*N*/                     nWidth = (pPage->*fnRect->fnGetRightMargin)();
/*N*/ 				}
/*N*/ 				bPageRel = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case REL_FRM_LEFT:
/*N*/ 				bTmpToggle = !bToggle;
/*N*/ 				// kein break;
/*N*/ 			case REL_FRM_RIGHT:
/*N*/ 			{
/*N*/ 				if ( bTmpToggle )    // linker Absatzrand
/*N*/ 				{
/*N*/                     nWidth = (pOrient->*fnRect->fnGetLeftMargin)();
/*N*/ 					nAdd = 0;
/*N*/ 				}
/*N*/ 				else			// rechter Absatzrand
/*N*/ 				{
/*N*/                     nWidth = (pOrient->*fnRect->fnGetRightMargin)();
/*N*/                     nAdd = (pOrient->Frm().*fnRect->fnGetWidth)()-nWidth;
/*N*/ 				}
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case REL_CHAR:
/*N*/ 			{
/*N*/ 				if( pAutoPos )
/*N*/ 				{
/*N*/ 					nWidth = 0;
/*N*/                     nAdd = (*fnRect->fnXDiff)( (pAutoPos->*fnRect->fnGetLeft)(),
/*N*/                                     (pAutoOrient->Frm().*fnRect->fnGetLeft)() );
/*N*/ 					break;
/*N*/ 				}
/*N*/ 				// No Break!
/*N*/ 			}
/*N*/ 			case REL_PG_PRTAREA:
/*N*/ 			{
/*N*/                 nWidth = (pPage->Prt().*fnRect->fnGetWidth)();
/*N*/                 nAdd = (*fnRect->fnXDiff)( (pPage->*fnRect->fnGetPrtLeft)(),
/*N*/                                          (pOrient->Frm().*fnRect->fnGetLeft)());
/*N*/ 				bPageRel = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			case REL_PG_FRAME:
/*N*/ 			{
/*N*/                 nWidth = (pPage->Frm().*fnRect->fnGetWidth)();
/*N*/                 nAdd = (*fnRect->fnXDiff)( (pPage->Frm().*fnRect->fnGetLeft)(),
/*N*/                                          (pOrient->Frm().*fnRect->fnGetLeft)());
/*N*/ 				bPageRel = TRUE;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 			default:
/*N*/ 			{
/*N*/                 nWidth = (pOrient->Frm().*fnRect->fnGetWidth)();
/*N*/ 				nAdd = pOrient->IsTxtFrm() ?
/*N*/                        ((SwTxtFrm*)pOrient)->GetBaseOfstForFly( !bWrapThrough ) :
/*N*/                        0;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/         SwTwips nRelPosX = nAdd;
/*N*/         sal_Bool bR2L = GetAnchor()->IsRightToLeft();
/*N*/ 		if ( aHori.GetHoriOrient() == HORI_NONE )
/*N*/ 		{
/*N*/ 			if( pAutoPos && REL_CHAR == aHori.GetRelationOrient() )
/*N*/             {
/*N*/                 if( bR2L )
/*N*/                     nRelPosX -= aHori.GetPos();
/*N*/                 else
/*N*/                     nRelPosX += aHori.GetPos();
/*N*/             }
/*N*/             else if( bToggle || ( !aHori.IsPosToggle() && bR2L ) )
/*N*/                 nRelPosX = nWidth - aFrm.Width() - aHori.GetPos() +
/*N*/                             ( bR2L ? nAdd : 0 );
/*N*/ 			else
/*N*/                 nRelPosX += aHori.GetPos();
/*N*/ 			//Da die relative Position immer zum Anker relativ ist,
/*N*/ 			//muss dessen Entfernung zum virtuellen Anker aufaddiert werden.
/*N*/ 			if ( GetAnchor() != pOrient )
/*N*/ 			{
/*N*/                 long nTmp = (pOrient->Frm().*fnRect->fnGetLeft)();
/*N*/                 nRelPosX += (*fnRect->fnXDiff)( nTmp,
/*N*/                                     (GetAnchor()->Frm().*fnRect->fnGetLeft)() );
/*N*/ 				//fix(18546): Kleine Notbremse, wenn der Rahmen jetzt so positioniert
/*N*/ 				//wird, dass er den Anker verdraengt, muessen wir unbedingt agieren.
/*N*/ 				//fix(22698): in Ergaenzung zu obigem Bug passen wir jetzt etwas
/*N*/ 				//grundlicher auf.
/*N*/                 if( bVert )
/*N*/                 {
/*N*/                     if( !bPageRel && nTmp > pAnchor->Frm().Bottom() &&
/*N*/                         Frm().Right() > GetAnchor()->Frm().Left() )
/*N*/                     {
/*N*/                         nTmp = nRelPosX + GetAnchor()->Frm().Top();
/*N*/                         if( nTmp < GetAnchor()->Frm().Bottom() )
/*N*/                             nRelPosX = GetAnchor()->Frm().Height() + 1;
/*N*/                     }
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     if( !bPageRel && nTmp > pAnchor->Frm().Right() &&
/*N*/                         Frm().Top() < GetAnchor()->Frm().Bottom() )
/*N*/                     {
/*N*/                         nTmp = aRelPos.X() + GetAnchor()->Frm().Left();
/*N*/                         if ( nTmp < GetAnchor()->Frm().Right() )
/*N*/                             nRelPosX = GetAnchor()->Frm().Width()+1;
/*N*/                     }
/*N*/                 }
/*N*/ 			}
/*N*/             if( bVert )
/*N*/             {
/*N*/                 if( GetAnchor()->Frm().Top() + nRelPosX + aFrm.Height() >
/*N*/                     pPage->Frm().Bottom() )
/*N*/                     nRelPosX = pPage->Frm().Bottom() - GetAnchor()->Frm().Top()
/*N*/                                   - aFrm.Height();
/*N*/                 if( GetAnchor()->Frm().Top() + nRelPosX < pPage->Frm().Top() )
/*N*/                     nRelPosX = pPage->Frm().Top() - GetAnchor()->Frm().Top();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 if( GetAnchor()->Frm().Left() + nRelPosX + aFrm.Width() >
/*N*/                     pPage->Frm().Right() )
/*N*/                     nRelPosX = pPage->Frm().Right() -
/*N*/                                GetAnchor()->Frm().Left() - aFrm.Width();
/*N*/                 if( GetAnchor()->Frm().Left() + nRelPosX < pPage->Frm().Left() )
/*N*/                     nRelPosX = pPage->Frm().Left() - GetAnchor()->Frm().Left();
/*N*/             }
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			SwHoriOrient eHOri = aHori.GetHoriOrient();
/*N*/ 			SwRelationOrient eRelO = aHori.GetRelationOrient();
/*N*/ 			if( bToggle )
/*N*/ 			{
/*N*/ 				if( HORI_RIGHT == eHOri )
/*N*/ 					eHOri = HORI_LEFT;
/*N*/ 				else if( HORI_LEFT == eHOri )
/*N*/ 					eHOri = HORI_RIGHT;
/*N*/ 				if( REL_PG_RIGHT == eRelO )
/*N*/ 					eRelO = REL_PG_LEFT;
/*N*/ 				else if( REL_PG_LEFT == eRelO )
/*N*/ 					eRelO = REL_PG_RIGHT;
/*N*/ 				else if( REL_FRM_RIGHT == eRelO )
/*N*/ 					eRelO = REL_FRM_LEFT;
/*N*/ 				else if( REL_FRM_LEFT == eRelO )
/*N*/ 					eRelO = REL_FRM_RIGHT;
/*N*/ 			}
/*N*/             if( bVert )
/*N*/             {
/*N*/                 if ( eHOri == HORI_CENTER )
/*N*/                     nRelPosX = (nWidth / 2) - (aFrm.Height() / 2);
/*N*/                 else if ( eHOri == HORI_RIGHT )
/*N*/                     nRelPosX = nWidth - (aFrm.Height() + rUL.GetLower());
/*N*/                 else
/*N*/                     nRelPosX = rUL.GetUpper();
/*N*/                 nRelPosX += nAdd;
/*N*/ 
/*N*/                 if( GetAnchor() != pOrient )
/*N*/                     nRelPosX += pOrient->Frm().Top() -
/*N*/                                 GetAnchor()->Frm().Top();
/*N*/ 
/*N*/                 if( GetAnchor()->Frm().Top() + nRelPosX + aFrm.Height() >
/*N*/                     pPage->Frm().Bottom() )
/*N*/                     nRelPosX = pPage->Frm().Bottom() - GetAnchor()->Frm().Top()
/*N*/                                 - aFrm.Height();
/*N*/                 if( GetAnchor()->Frm().Top() + nRelPosX < pPage->Frm().Top() )
/*N*/                     nRelPosX = pPage->Frm().Top() - GetAnchor()->Frm().Top();
/*N*/             }
/*N*/             else
/*N*/             {
/*N*/                 if ( eHOri == HORI_CENTER )
/*N*/                     nRelPosX = (nWidth / 2) - (aFrm.Width() / 2);
/*N*/                 else if ( eHOri == HORI_RIGHT )
/*N*/                     nRelPosX = nWidth - (aFrm.Width() + rLR.GetRight());
/*N*/                 else
/*N*/                     nRelPosX = rLR.GetLeft();
/*N*/                 nRelPosX += nAdd;
/*N*/ 
/*N*/                 //Da die relative Position immer zum Anker relativ ist,
/*N*/                 //muss dessen Entfernung zum virtuellen Anker aufaddiert werden.
/*N*/                 if( GetAnchor() != pOrient )
/*N*/                     nRelPosX += pOrient->Frm().Left() -
/*N*/                                 GetAnchor()->Frm().Left();
/*N*/                 if( GetAnchor()->Frm().Left() + nRelPosX + aFrm.Width() >
/*N*/                     pPage->Frm().Right() )
/*N*/                     nRelPosX = pPage->Frm().Right() - GetAnchor()->Frm().Left()
/*N*/                                 - aFrm.Width();
/*N*/                 if( GetAnchor()->Frm().Left() + nRelPosX < pPage->Frm().Left() )
/*N*/                     nRelPosX = pPage->Frm().Left() - GetAnchor()->Frm().Left();
/*N*/             }
/*N*/ 
/*N*/ 			//Es muss allen Rahmen ausgewichen werden, die die selbe
/*N*/ 			//automatische Ausrichtung haben und die unter dem Rahmen liegen.
/*N*/ 			if ( HORI_CENTER != eHOri && REL_CHAR != eRelO )
/*N*/ 			{
/*N*/                 Point aTmpPos = (GetAnchor()->Frm().*fnRect->fnGetPos)();
/*N*/                 if( bVert )
/*N*/                 {
/*N*/                     aTmpPos.X() -= aRelPos.X() + aFrm.Width();
/*N*/                     aTmpPos.Y() += nRelPosX;
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*N*/                     aTmpPos.X() += nRelPosX;
/*N*/                     aTmpPos.Y() += aRelPos.Y();
/*N*/                 }
/*N*/                 SwRect aTmpFrm( aTmpPos, Frm().SSize() );
/*N*/ 				const UINT32 nMyOrd = GetVirtDrawObj()->GetOrdNum();
/*N*/                 const SwPageFrm *pPage = FindPageFrm();
/*N*/                 SwOrderIter aIter( pPage, TRUE );
/*N*/ 				const SwFlyFrm *pFly = ((SwVirtFlyDrawObj*)aIter.Bottom())->GetFlyFrm();
/*N*/ 				const SwFrm *pKontext = ::binfilter::FindKontext( GetAnchor(), FRM_COLUMN );
/*N*/                 ULONG nMyIndex = ((SwTxtFrm*)GetAnchor())->GetTxtNode()->GetIndex();
/*N*/ 				while ( pFly && nMyOrd > pFly->GetVirtDrawObj()->GetOrdNumDirect() )
/*N*/ 				{
/*N*/ 					if ( pFly->IsFlyAtCntFrm() && //pFly->IsValid() &&
/*N*/                          (pFly->Frm().*fnRect->fnBottomDist)(
/*N*/                             (aTmpFrm.*fnRect->fnGetTop)() ) < 0 &&
/*N*/                          (aTmpFrm.*fnRect->fnBottomDist)(
/*N*/                             (pFly->Frm().*fnRect->fnGetTop)() ) < 0 &&
/*N*/ 						 ::binfilter::FindKontext( pFly->GetAnchor(), FRM_COLUMN ) == pKontext )
/*N*/ 					{
/*N*/                         ULONG nOtherIndex = ((SwTxtFrm*)pFly->GetAnchor())
/*N*/                                             ->GetTxtNode()->GetIndex();
/*N*/                         if( nMyIndex >= nOtherIndex )
/*N*/                         {
/*N*/                             const SwFmtHoriOrient &rHori =
/*N*/                                                 pFly->GetFmt()->GetHoriOrient();
/*N*/                             SwRelationOrient eRelO2 = rHori.GetRelationOrient();
/*N*/                             if( REL_CHAR != eRelO2 )
/*N*/                             {
/*N*/                                 SwHoriOrient eHOri2 = rHori.GetHoriOrient();
/*N*/                                 if( bEven && rHori.IsPosToggle() )
/*N*/                                 {
/*N*/                                     if( HORI_RIGHT == eHOri2 )
/*N*/                                         eHOri2 = HORI_LEFT;
/*N*/                                     else if( HORI_LEFT == eHOri2 )
/*N*/                                         eHOri2 = HORI_RIGHT;
/*N*/                                     if( REL_PG_RIGHT == eRelO2 )
/*N*/                                         eRelO2 = REL_PG_LEFT;
/*N*/                                     else if( REL_PG_LEFT == eRelO2 )
/*N*/                                         eRelO2 = REL_PG_RIGHT;
/*N*/                                     else if( REL_FRM_RIGHT == eRelO2 )
/*N*/                                         eRelO2 = REL_FRM_LEFT;
/*N*/                                     else if( REL_FRM_LEFT == eRelO2 )
/*N*/                                         eRelO2 = REL_FRM_RIGHT;
/*N*/                                 }
/*N*/                                 if ( eHOri2 == eHOri &&
/*N*/                                     lcl_Minor( eRelO, eRelO2, HORI_LEFT == eHOri ) )
/*N*/                                 {
/*N*/                                     //Die Berechnung wird dadurch etwas aufwendiger, das die
/*N*/                                     //Ausgangsbasis der Flys unterschiedlich sein koennen.
/*N*/                                     if( bVert )
/*N*/                                     {
/*N*/                                         const SvxULSpaceItem &rULI = pFly->GetFmt()->GetULSpace();
/*N*/                                         const SwTwips nFlyTop = pFly->Frm().Top() - rULI.GetUpper();
/*N*/                                         const SwTwips nFlyBot = pFly->Frm().Bottom() + rULI.GetLower();
/*N*/                                         if( nFlyTop <= aTmpFrm.Bottom() + rUL.GetLower() &&
/*N*/                                             nFlyBot >= aTmpFrm.Top() - rUL.GetUpper() )
/*N*/                                         {
/*N*/                                             if ( eHOri == HORI_LEFT )
/*N*/                                             {
/*N*/                                                 SwTwips nTmp = nFlyBot + 1
/*N*/                                                     + rUL.GetUpper()
/*N*/                                                     - GetAnchor()->Frm().Top();
/*N*/                                                 if( nTmp > nRelPosX &&
/*N*/                                                     nTmp + Frm().Height() +
/*N*/                                                     GetAnchor()->Frm().Top() +
/*N*/                                                     rUL.GetLower() <=
/*N*/                                                     pPage->Frm().Height() +
/*N*/                                                     pPage->Frm().Top() )
/*N*/                                                 {
/*N*/                                                     nRelPosX = nTmp;
/*N*/                                                 }
/*N*/                                             }
/*N*/                                             else if ( eHOri == HORI_RIGHT )
/*N*/                                             {
/*N*/                                                 SwTwips nTmp = nFlyTop - 1
/*N*/                                                     - rUL.GetLower()
/*N*/                                                     - Frm().Height()
/*N*/                                                     - GetAnchor()->Frm().Top();
/*N*/                                                 if( nTmp < nRelPosX &&
/*N*/                                                     nTmp - rUL.GetUpper() +
/*N*/                                                     GetAnchor()->Frm().Top()
/*N*/                                                     >= pPage->Frm().Top() )
/*N*/                                                 {
/*N*/                                                     nRelPosX = nTmp;
/*N*/                                                 }
/*N*/                                             }
/*N*/                                             aTmpFrm.Pos().Y() = GetAnchor()->Frm().Top() + nRelPosX;
/*N*/                                         }
/*N*/                                     }
/*N*/                                     else
/*N*/                                     {
/*N*/                                         const SvxLRSpaceItem &rLRI = pFly->GetFmt()->GetLRSpace();
/*N*/                                         const SwTwips nFlyLeft = pFly->Frm().Left() - rLRI.GetLeft();
/*N*/                                         const SwTwips nFlyRight = pFly->Frm().Right() + rLRI.GetRight();
/*N*/                                         if( nFlyLeft <= aTmpFrm.Right() + rLR.GetRight() &&
/*N*/                                             nFlyRight >= aTmpFrm.Left() - rLR.GetLeft() )
/*N*/                                         {
/*N*/                                             if ( eHOri == HORI_LEFT )
/*N*/                                             {
/*N*/                                                 SwTwips nTmp = nFlyRight + 1
/*N*/                                                     + rLR.GetLeft()
/*N*/                                                     - GetAnchor()->Frm().Left();
/*N*/                                                 if( nTmp > nRelPosX &&
/*N*/                                                     nTmp + Frm().Width() +
/*N*/                                                     GetAnchor()->Frm().Left() +
/*N*/                                                     rLR.GetRight() <=
/*N*/                                                     pPage->Frm().Width() +
/*N*/                                                     pPage->Frm().Left() )
/*N*/                                                 {
/*N*/                                                     nRelPosX = nTmp;
/*N*/                                                 }
/*N*/                                             }
/*N*/                                             else if ( eHOri == HORI_RIGHT )
/*N*/                                             {
/*N*/                                                 SwTwips nTmp = nFlyLeft - 1
/*N*/                                                     - rLR.GetRight()
/*N*/                                                     - Frm().Width()
/*N*/                                                     - GetAnchor()->Frm().Left();
/*N*/                                                 if( nTmp < nRelPosX &&
/*N*/                                                     nTmp - rLR.GetLeft() +
/*N*/                                                     GetAnchor()->Frm().Left()
/*N*/                                                     >= pPage->Frm().Left() )
/*N*/                                                 {
/*N*/                                                     nRelPosX = nTmp;
/*N*/                                                 }
/*N*/                                             }
/*N*/                                             aTmpFrm.Pos().X() = GetAnchor()->Frm().Left() + nRelPosX;
/*N*/                                         }
/*N*/                                     }
/*N*/                                 }
/*N*/ 							}
/*N*/ 						}
/*N*/ 					}
/*N*/ 					pFly = ((SwVirtFlyDrawObj*)aIter.Next())->GetFlyFrm();
/*N*/ 				}
/*N*/ 			}
/*N*/ 
/*N*/             if ( aHori.GetPos() != nRelPosX )
/*N*/             {   aHori.SetPos( nRelPosX );
/*N*/ 				bHoriChgd = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/         if( bVert )
/*N*/             aRelPos.Y() = nRelPosX;
/*N*/         else
/*N*/             aRelPos.X() = nRelPosX;
/*N*/         AssertPage();
/*N*/ 
/*N*/ 		//Die AbsPos ergibt sich aus der Absoluten Position des Ankers
/*N*/ 		//plus der relativen Position
/*N*/         if( bVert )
/*N*/         {
/*N*/             aFrm.Pos().X() = GetAnchor()->Frm().Left() +
/*N*/                              GetAnchor()->Frm().Width() -
/*N*/                              aFrm.Width() - aRelPos.X();
/*N*/             aFrm.Pos().Y() = GetAnchor()->Frm().Top() + aRelPos.Y();
/*N*/             AssertPage();
/*N*/         }
/*N*/         else
/*N*/             aFrm.Pos( aRelPos + GetAnchor()->Frm().Pos() );
/*N*/ 		//Und ggf. noch die aktuellen Werte im Format updaten, dabei darf
/*N*/ 		//zu diesem Zeitpunkt natuerlich kein Modify verschickt werden.
/*N*/ 		pFmt->LockModify();
/*N*/ 		if ( bVertChgd )
/*N*/ 			pFmt->SetAttr( aVert );
/*N*/ 		if ( bHoriChgd )
/*N*/ 			pFmt->SetAttr( aHori );
/*N*/ 		pFmt->UnlockModify();
/*N*/ 
/*N*/ 		break;
/*N*/     } /// OD 02.10.2002 #102646# - End of loop
/*N*/ 
/*N*/     if ( bInvalidatePage )
/*N*/         FindPageFrm()->InvalidateSize();
/*N*/     if ( !bValidPos && !GetAnchor()->IsValid() )
/*N*/     {
/*N*/ //      ASSERT( StackHack::IsLocked(), "invalid Anchor" );
/*N*/         bValidPos = TRUE;
/*N*/     }
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
