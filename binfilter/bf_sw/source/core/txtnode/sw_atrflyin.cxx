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

#include "cntfrm.hxx"       // _GetFly

#include <horiornt.hxx>

#include "doc.hxx"
#include "pam.hxx"          // fuer SwTxtFlyCnt
#include "ndtxt.hxx"        // SwFlyFrmFmt
#include "frmfmt.hxx"       // SwFlyFrmFmt

#include <fmtflcnt.hxx>
#include <txtflcnt.hxx>
#include <fmtanchr.hxx>
#include "txtfrm.hxx"
#include "flyfrms.hxx"
namespace binfilter {

/*N*/ SwFmtFlyCnt::SwFmtFlyCnt( SwFrmFmt *pFrmFmt )
/*N*/ 	: SfxPoolItem( RES_TXTATR_FLYCNT ),
/*N*/ 	pFmt( pFrmFmt ),
/*N*/ 	pTxtAttr( 0 )
/*N*/ {
/*N*/ }

int __EXPORT SwFmtFlyCnt::operator==( const SfxPoolItem& rAttr ) const
{
    DBG_BF_ASSERT(0, "STRIP"); return 0; //STRIP001 	ASSERT( SfxPoolItem::operator==( rAttr ), "keine gleichen Attribute" );
}

/*N*/ SfxPoolItem* __EXPORT SwFmtFlyCnt::Clone( SfxItemPool* ) const
/*N*/ {
/*N*/ 	return new SwFmtFlyCnt( pFmt );
/*N*/ }

/*N*/ SwTxtFlyCnt::SwTxtFlyCnt( const SwFmtFlyCnt& rAttr, xub_StrLen nStart )
/*N*/ 	: SwTxtAttr( rAttr, nStart )
/*N*/ {
/*N*/ 	((SwFmtFlyCnt&)rAttr).pTxtAttr = this;
/*N*/ }



/*************************************************************************
 *                  SwTxtFlyCnt::MakeTxtHint()
 *
 * An dieser Stelle soll einmal der Gesamtzusammenhang bei der Erzeugung
 * eines neuen SwTxtFlyCnt erlaeutert werden.
 * Das MakeTxtHint() wird z.B. im SwTxtNode::Copy() gerufen.
 * Fuer die komplette Verdopplung sind folgende Schritte notwendig:
 * 1) Duplizieren des pFmt incl. Inhalt, Attributen etc.
 * 2) Setzen des Ankers
 * 3) Benachrichtigung
 * Da fuer die Bewaeltigung der Aufgaben nicht immer alle Informationen
 * bereitstehen und darueber hinaus bestimmte Methoden erst zu einem
 * spaeteren Zeitpunkt gerufen werden duerfen (weil nocht nicht alle
 * Nodeinformationen vorliegen), verteilt sich der Ablauf.
 * ad 1) MakeTxtHint() wird durch den Aufruf von SwDoc::CopyLayout()
 * der das neue FlyFrmFmt erzeugt und mit dem duplizierten Inhalt des
 * FlyFrm verbunden.
 * ad 2) SetAnchor() wird von SwTxtNode::Insert() gerufen und sorgt fuer das
 * setzen des Ankers (die SwPosition des Dummy-Zeichens wird dem FlyFrmFmt
 * per SetAttr bekannt gegeben). Dies kann nicht im MakeTxtHint erledigt
 * werden, da der Zielnode unbestimmt ist.
 * ad 3) _GetFlyFrm() wird im Formatierungsprozess vom LineIter gerufen
 * und sucht den FlyFrm zum Dummyzeichen des aktuellen CntntFrm. Wird keiner
 * gefunden, so wird ein neuer FlyFrm angelegt.
 * Kritisch an diesem Vorgehen ist, dass das pCntnt->AppendFly() eine
 * sofortige Neuformatierung von pCntnt anstoesst. Die Rekursion kommt
 * allerdings durch den Lockmechanismus in SwTxtFrm::Format() nicht
 * zu stande.
 * Attraktiv ist der Umstand, dass niemand ueber die vom Node abhaengigen
 * CntntFrms iterieren braucht, um die FlyInCntFrm anzulegen. Dies geschieht
 * bei der Arbeit.
 *************************************************************************/

/*N*/ void SwTxtFlyCnt::CopyFlyFmt( SwDoc* pDoc )
/*N*/ {
/*N*/ 	SwFrmFmt* pFmt = GetFlyCnt().GetFrmFmt();
/*N*/ 	ASSERT( pFmt, "von welchem Format soll ich eine Kopie erzeugen?" )
/*N*/ 	// Das FlyFrmFmt muss dupliziert werden.
/*N*/ 	// In CopyLayoutFmt (siehe doclay.cxx) wird das FlyFrmFmt erzeugt
/*N*/ 	// und der Inhalt dupliziert.
/*N*/
/*N*/ 	// fuers kopieren vom Attribut das Undo immer abschalten
/*N*/ 	SwFmtAnchor aAnchor( pFmt->GetAnchor() );
/*N*/ 	if( FLY_PAGE != aAnchor.GetAnchorId() &&
/*N*/ 		pDoc != pFmt->GetDoc() )		// Unterschiedliche Docs?
/*N*/ 	{
/*N*/ 		// JP 03.06.96: dann sorge dafuer, das der koperierte Anker auf
/*N*/ 		//				gueltigen Content zeigt! Die Umsetzung auf die
/*N*/ 		//				richtige Position erfolgt spaeter.
/*?*/ 		SwNodeIndex aIdx( pDoc->GetNodes().GetEndOfExtras(), +2 );
/*?*/ 		SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
/*?*/ 		if( !pCNd )
/*?*/ 			pCNd = pDoc->GetNodes().GoNext( &aIdx );
/*?*/
/*?*/ 		SwPosition* pPos = (SwPosition*)aAnchor.GetCntntAnchor();
/*?*/ 		pPos->nNode = aIdx;
/*?*/ 		if( FLY_IN_CNTNT == aAnchor.GetAnchorId() )
/*?*/ 			pPos->nContent.Assign( pCNd, 0 );
/*?*/ 		else
/*?*/ 		{
/*?*/ 			pPos->nContent.Assign( 0, 0 );
/*?*/ 			ASSERT( !this, "CopyFlyFmt: Was fuer ein Anker?" );
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SwFrmFmt* pNew = pDoc->CopyLayoutFmt( *pFmt, aAnchor, FALSE, FALSE );
/*N*/ 	((SwFmtFlyCnt&)GetFlyCnt()).SetFlyFmt( pNew );
/*N*/ }

/*************************************************************************
 *                  SwTxtFlyCnt::SetAnchor()
 *
 * SetAnchor() wird von SwTxtNode::Insert() gerufen und sorgt fuer das
 * setzen des Ankers (die SwPosition des Dummy-Zeichens wird dem FlyFrmFmt
 * per SetAttr bekannt gegeben). Dies kann nicht im MakeTxtHint erledigt
 * werden, da der Zielnode unbestimmt ist.
 * (siehe Kommentar in SwTxtFlyCnt::MakeTxtHint)
 *************************************************************************/

/*N*/ void SwTxtFlyCnt::SetAnchor( const SwTxtNode *pNode )
/*N*/ {
/*N*/ 	// Wir ermitteln den Index im Nodesarray zum Node
/*N*/
/*N*/ 	SwDoc* pDoc = (SwDoc*)pNode->GetDoc();
/*N*/
/*N*/ 	SwIndex aIdx( (SwTxtNode*)pNode, *GetStart() );
/*N*/ 	SwPosition aPos( *pNode->StartOfSectionNode(), aIdx );
/*N*/ 	SwFrmFmt* pFmt = GetFlyCnt().GetFrmFmt();
/*N*/ 	SwFmtAnchor aAnchor( pFmt->GetAnchor() );
/*N*/
/*N*/ 	if( !aAnchor.GetCntntAnchor() ||
/*N*/ 		!aAnchor.GetCntntAnchor()->nNode.GetNode().GetNodes().IsDocNodes() ||
/*N*/ 		&aAnchor.GetCntntAnchor()->nNode.GetNode() != (SwNode*)pNode )
/*N*/ 		aPos.nNode = *pNode;
/*N*/ 	else
/*N*/ 		aPos.nNode = aAnchor.GetCntntAnchor()->nNode;
/*N*/
/*N*/ 	aAnchor.SetType( FLY_IN_CNTNT );		// defaulten !!
/*N*/ 	aAnchor.SetAnchor( &aPos );
/*N*/
/*N*/ 	// beim Ankerwechsel werden immer alle FlyFrms vom Attribut geloescht
/*N*/ 	// JP 25.04.95: wird innerhalb des SplitNodes die Frames verschoben
/*N*/ 	//				koennen die Frames erhalten bleiben.
/*N*/ 	if( ( !pNode->GetpSwpHints() || !pNode->GetpSwpHints()->IsInSplitNode() )
/*N*/ 		&& RES_DRAWFRMFMT != pFmt->Which() )
/*N*/ 		pFmt->DelFrms();
/*N*/
/*N*/ 	// stehen wir noch im falschen Dokument ?
/*N*/ 	if( pDoc != pFmt->GetDoc() )
/*N*/ 	{
/*?*/ 		SwFrmFmt* pNew = pDoc->CopyLayoutFmt( *pFmt, aAnchor, FALSE, FALSE );
/*?*/
/*?*/ 		pFmt->GetDoc()->DelLayoutFmt( pFmt );
/*?*/ 		((SwFmtFlyCnt&)GetFlyCnt()).SetFlyFmt( pNew );
/*N*/ 	}
/*N*/ 	else if( pNode->GetpSwpHints() &&
/*N*/ 			pNode->GetpSwpHints()->IsInSplitNode() &&
/*N*/ 			RES_DRAWFRMFMT != pFmt->Which() )
/*N*/ 	{
/*?*/ 		pFmt->LockModify();
/*?*/ 		pFmt->SetAttr( aAnchor );		// nur den Anker neu setzen
/*?*/ 		pFmt->UnlockModify();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pFmt->SetAttr( aAnchor );		// nur den Anker neu setzen
/*N*/
/*N*/ 	// Am Node haengen u.a. abhaengige CntFrms.
/*N*/ 	// Fuer jeden CntFrm wird ein SwFlyInCntFrm angelegt.
/*N*/ }

/*************************************************************************
 *                      SwTxtFlyCnt::_GetFlyFrm()
 *
 * _GetFlyFrm() wird im Formatierungsprozess vom LineIter gerufen
 * und sucht den FlyFrm zum Dummyzeichen des aktuellen CntntFrm. Wird keiner
 * gefunden, so wird ein neuer FlyFrm angelegt.
 * (siehe Kommentar ind SwTxtFlyCnt::MakeTxtHint)
 *************************************************************************/

/*N*/ SwFlyInCntFrm *SwTxtFlyCnt::_GetFlyFrm( const SwFrm *pCurrFrm )
/*N*/ {
/*N*/ 	SwFrmFmt* pFrmFmt = GetFlyCnt().GetFrmFmt();
/*N*/ 	if( RES_DRAWFRMFMT == pFrmFmt->Which() )
/*N*/ 	{
/*?*/ 		ASSERT(  !this, "SwTxtFlyCnt::_GetFlyFrm: DrawInCnt-Baustelle!" );
/*?*/ 		return NULL;
/*N*/ 	}
/*N*/
/*N*/ 	SwClientIter aIter( *GetFlyCnt().pFmt );
/*N*/ 	ASSERT( pCurrFrm->IsTxtFrm(), "SwTxtFlyCnt::_GetFlyFrm for TxtFrms only." );
/*N*/
/*N*/ 	if( aIter.GoStart() )
/*N*/ 	{
/*N*/ 		SwTxtFrm *pFirst = (SwTxtFrm*)pCurrFrm;
/*N*/ 		while ( pFirst->IsFollow() )
/*?*/ 			pFirst = pFirst->FindMaster();
/*N*/ 		do
/*N*/ 		{   SwFrm * pFrm = PTR_CAST( SwFrm, aIter() );
/*N*/ 			if ( pFrm )
/*N*/ 			{
/*N*/ 				SwTxtFrm *pTmp = pFirst;
/*N*/ 				do
/*N*/ 				{   if( ( (SwFlyFrm*)pFrm )->GetAnchor() == (SwFrm*) pTmp )
/*N*/ 					{
/*N*/ 						if ( pTmp != pCurrFrm )
/*N*/ 						{
/*?*/ 							pTmp->RemoveFly( (SwFlyFrm*)pFrm );
/*?*/ 							((SwTxtFrm*)pCurrFrm)->AppendFly( (SwFlyFrm*)pFrm );
/*N*/ 						}
/*N*/ 						return (SwFlyInCntFrm*)pFrm;
/*N*/ 					}
/*?*/ 					pTmp = pTmp->GetFollow();
/*N*/ 				} while ( pTmp );
/*N*/ 			}
/*N*/ 		} while( aIter++ );
/*N*/ 	}
/*N*/
/*N*/ 	// Wir haben keinen passenden FlyFrm gefunden, deswegen wird ein
/*N*/ 	// neuer angelegt.
/*N*/ 	// Dabei wird eine sofortige Neuformatierung von pCurrFrm angestossen.
/*N*/ 	// Die Rekursion wird durch den Lockmechanismus in SwTxtFrm::Format()
/*N*/ 	// abgewuergt.
/*N*/ 	SwFlyInCntFrm *pFly = new SwFlyInCntFrm( (SwFlyFrmFmt*)pFrmFmt, (SwFrm*)pCurrFrm );
/*N*/ 	((SwFrm*)pCurrFrm)->AppendFly( pFly );
/*N*/ 	pFly->RegistFlys();
/*N*/
/*N*/ 	// 7922: Wir muessen dafuer sorgen, dass der Inhalt des FlyInCnt
/*N*/ 	// nach seiner Konstruktion stramm durchformatiert wird.
/*N*/ 	SwCntntFrm *pFrm = pFly->ContainsCntnt();
/*N*/ 	while( pFrm )
/*N*/ 	{
/*N*/ 		pFrm->Calc();
/*N*/ 		pFrm = pFrm->GetNextCntntFrm();
/*N*/ 	}
/*N*/
/*N*/ 	return pFly;
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
