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

#include <hintids.hxx>

#include <bf_svx/protitem.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>

#include <fmtanchr.hxx>
#include <txtftn.hxx>
#include <ftnfrm.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <section.hxx>
#include <flyfrm.hxx>
#include <txtfrm.hxx>
#include <paratr.hxx>
#include <ftnidx.hxx>
#include <fmtftn.hxx>
#include <fmtcntnt.hxx>
#include <frmtool.hxx>
#include <pagefrm.hxx>
#include <node2lay.hxx>
#include <hints.hxx>
#include <breakit.hxx>
#include <crsskip.hxx>
#include <SwStyleNameMapper.hxx>
namespace binfilter {
using namespace ::com::sun::star::i18n;

/*N*/ TYPEINIT2( SwCntntNode, SwModify, SwIndexReg )

/*******************************************************************
|*
|*	SwNode::GetSectionLevel
|*
|*	Beschreibung
|*		Die Funktion liefert den Sectionlevel an der durch
|*		aIndex bezeichneten Position.
|*
|*		Die Logik ist wie folgt:   ( S -> Start, E -> End, C -> CntntNode)
|*			Level 	0		E
|*					1 	S  E
|*					2  	 SC
|*
|*		alle EndNodes der GrundSection haben den Level 0
|*		alle StartNodes der GrundSection haben den Level 1
|*
|*	Ersterstellung
|*		VER0100 vb 901214
|*
|*	Aenderung:	JP	11.08.93
|*		keine Rekursion mehr !!
|*
*******************************************************************/



/*******************************************************************
|*
|*	SwNode::SwNode
|*
|*	Beschreibung
|*		Konstruktor; dieser fuegt einen Node in das Array rNodes
|*		an der Position rWhere ein. Dieser bekommt als
|*		theEndOfSection den EndOfSection-Index des Nodes
|*		unmittelbar vor ihm. Falls er sich an der Position 0
|*		innerhalb des variablen Arrays befindet, wird
|*		theEndOfSection 0 (der neue selbst).
|*
|*	Parameter
|*		IN
|*		rNodes bezeichnet das variable Array, in das der Node
|*		eingefuegt werden soll
|*		IN
|*		rWhere bezeichnet die Position innerhalb dieses Arrays,
|*		an der der Node eingefuegt werden soll
|*
|*	Ersterstellung
|*		VER0100 vb 901214
|*
|*	Stand
|*		VER0100 vb 901214
|*
*******************************************************************/


/*N*/ SwNode::SwNode( const SwNodeIndex &rWhere, const BYTE nNdType )
/*N*/ 	: pStartOfSection( 0 ), nNodeType( nNdType )
/*N*/ {
/*N*/ 	bWrongDirty = bACmplWrdDirty = TRUE;
/*N*/ 	bSetNumLSpace = bIgnoreDontExpand = FALSE;
/*N*/ 	nAFmtNumLvl = 0;
/*N*/
/*N*/ 	SwNodes& rNodes = (SwNodes&)rWhere.GetNodes();
/*N*/ 	SwNode* pInsNd = this; 		// der MAC kann this nicht einfuegen !!
/*N*/ 	if( rWhere.GetIndex() )
/*N*/ 	{
/*N*/ 		SwNode* pNd = rNodes[ rWhere.GetIndex() -1 ];
/*N*/ 		rNodes.Insert( pInsNd, rWhere );
/*N*/ 		if( 0 == ( pStartOfSection = pNd->GetStartNode()) )
/*N*/ 		{
/*N*/ 			pStartOfSection = pNd->pStartOfSection;
/*N*/ 			if( pNd->GetEndNode() )		// EndNode ? Section ueberspringen!
/*N*/ 			{
/*N*/ 				pNd = pStartOfSection;
/*N*/ 				pStartOfSection = pNd->pStartOfSection;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		rNodes.Insert( pInsNd, rWhere );
/*?*/ 		pStartOfSection = (SwStartNode*)this;
/*N*/ 	}
/*N*/ }

/*N*/ SwNode::SwNode( SwNodes& rNodes, ULONG nPos, const BYTE nNdType )
/*N*/ 	: pStartOfSection( 0 ), nNodeType( nNdType )
/*N*/ {
/*N*/ 	bWrongDirty = bACmplWrdDirty = TRUE;
/*N*/ 	bSetNumLSpace = bIgnoreDontExpand = FALSE;
/*N*/ 	nAFmtNumLvl = 0;
/*N*/
/*N*/ 	SwNode* pInsNd = this; 		// der MAC kann this nicht einfuegen !!
/*N*/ 	if( nPos )
/*N*/ 	{
/*N*/ 		SwNode* pNd = rNodes[ nPos - 1 ];
/*N*/ 		rNodes.Insert( pInsNd, nPos );
/*N*/ 		if( 0 == ( pStartOfSection = pNd->GetStartNode()) )
/*N*/ 		{
/*N*/ 			pStartOfSection = pNd->pStartOfSection;
/*N*/ 			if( pNd->GetEndNode() )		// EndNode ? Section ueberspringen!
/*N*/ 			{
/*N*/ 				pNd = pStartOfSection;
/*N*/ 				pStartOfSection = pNd->pStartOfSection;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		rNodes.Insert( pInsNd, nPos );
/*N*/ 		pStartOfSection = (SwStartNode*)this;
/*N*/ 	}
/*N*/ }

/*N*/ SwNode::~SwNode()
/*N*/ {
/*N*/ }

// suche den TabellenNode, in dem dieser steht. Wenn in keiner
// Tabelle wird 0 returnt.


/*N*/ SwTableNode* SwNode::FindTableNode()
/*N*/ {
/*N*/ 	if( IsTableNode() )
/*N*/ 		return GetTableNode();
/*N*/ 	SwStartNode* pTmp = pStartOfSection;
/*N*/ 	while( !pTmp->IsTableNode() && pTmp->GetIndex() )
/*N*/ #if defined( ALPHA ) && defined( UNX )
/*?*/ 		pTmp = ((SwNode*)pTmp)->pStartOfSection;
/*N*/ #else
/*N*/ 		pTmp = pTmp->pStartOfSection;
/*N*/ #endif
/*N*/ 	return pTmp->GetTableNode();
/*N*/ }


// liegt der Node im Sichtbarenbereich der Shell ?

/*N*/ BOOL SwNode::IsInProtectSect() const
/*N*/ {
/*N*/ 	const SwNode* pNd = ND_SECTIONNODE == nNodeType ? pStartOfSection : this;
/*N*/ 	const SwSectionNode* pSectNd = pNd->FindSectionNode();
/*N*/ 	return pSectNd && pSectNd->GetSection().IsProtectFlag();
/*N*/ }

    // befindet sich der Node in irgendetwas geschuetzten ?
    // (Bereich/Rahmen/Tabellenzellen/... incl. des Ankers bei
    //	Rahmen/Fussnoten/..)
/*N*/ BOOL SwNode::IsProtect() const
/*N*/ {
/*N*/ 	const SwNode* pNd = ND_SECTIONNODE == nNodeType ? pStartOfSection : this;
/*N*/ 	const SwStartNode* pSttNd = pNd->FindSectionNode();
/*N*/ 	if( pSttNd && ((SwSectionNode*)pSttNd)->GetSection().IsProtectFlag() )
/*N*/ 		return TRUE;
/*N*/
/*N*/ 	if( 0 != ( pSttNd = FindTableBoxStartNode() ) )
/*N*/ 	{
/*N*/ 		SwCntntFrm* pCFrm;
/*N*/ 		if( IsCntntNode() && 0 != (pCFrm = ((SwCntntNode*)this)->GetFrm() ))
/*N*/ 			return pCFrm->IsProtected();
/*N*/
/*N*/ 		const SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().
/*N*/ 										GetTblBox( pSttNd->GetIndex() );
/*N*/ 		if( pBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
/*?*/ 			return TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	SwFrmFmt* pFlyFmt = GetFlyFmt();
/*N*/ 	if( pFlyFmt )
/*N*/ 	{
/*N*/ 		if( pFlyFmt->GetProtect().IsCntntProtected() )
/*?*/ 			return TRUE;
/*N*/ 		const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
/*N*/ 		return rAnchor.GetCntntAnchor()
/*N*/ 				? rAnchor.GetCntntAnchor()->nNode.GetNode().IsProtect()
/*N*/ 				: FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	if( 0 != ( pSttNd = FindFootnoteStartNode() ) )
/*N*/ 	{
/*?*/ 		const SwTxtFtn* pTFtn = GetDoc()->GetFtnIdxs().SeekEntry(
/*?*/ 								SwNodeIndex( *pSttNd ) );
/*?*/ 		if( pTFtn )
/*?*/ 			return pTFtn->GetTxtNode().IsProtect();
/*N*/ 	}
/*N*/
/*N*/ 	return FALSE;
/*N*/ }

    // suche den PageDesc, mit dem dieser Node formatiert ist. Wenn das
    // Layout vorhanden ist wird ueber das gesucht, ansonsten gibt es nur
    // die harte Tour ueber die Nodes nach vorne suchen!!


    // falls der Node in einem Fly steht, dann wird das entsprechende Format
    // returnt
/*N*/ SwFrmFmt* SwNode::GetFlyFmt() const
/*N*/ {
/*N*/ 	SwFrmFmt* pRet = 0;
/*N*/ 	const SwNode* pSttNd = FindFlyStartNode();
/*N*/ 	if( pSttNd )
/*N*/ 	{
/*N*/ 		if( IsCntntNode() )
/*N*/ 		{
/*N*/ 			SwClientIter aIter( *(SwCntntNode*)this );
/*N*/ 			SwClient* pCli = aIter.First( TYPE( SwCntntFrm ));
/*N*/ 			if( pCli )
/*N*/ 				pRet = ((SwCntntFrm*)pCli)->FindFlyFrm()->GetFmt();
/*N*/ 		}
/*N*/ 		if( !pRet )
/*N*/ 		{
/*N*/ 			// dann gibts noch harten steinigen Weg uebers Dokument:
/*N*/ 			const SwSpzFrmFmts& rFrmFmtTbl = *GetDoc()->GetSpzFrmFmts();
/*N*/ 			for( USHORT n = 0; n < rFrmFmtTbl.Count(); ++n )
/*N*/ 			{
/*N*/ 				SwFrmFmt* pFmt = rFrmFmtTbl[n];
/*N*/ 				const SwFmtCntnt& rCntnt = pFmt->GetCntnt();
/*N*/ 				if( rCntnt.GetCntntIdx() &&
/*N*/ 					&rCntnt.GetCntntIdx()->GetNode() == pSttNd )
/*N*/ 				{
/*N*/ 					pRet = pFmt;
/*N*/ 					break;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }


/*N*/ SwTableBox* SwNode::GetTblBox() const
/*N*/ {
/*N*/ 	SwTableBox* pBox = 0;
/*N*/ 	const SwNode* pSttNd = FindTableBoxStartNode();
/*N*/ 	if( pSttNd )
/*N*/ 		pBox = (SwTableBox*)pSttNd->FindTableNode()->GetTable().GetTblBox(
/*N*/ 													pSttNd->GetIndex() );
/*N*/ 	return pBox;
/*N*/ }

/*N*/ SwStartNode* SwNode::FindSttNodeByType( SwStartNodeType eTyp )
/*N*/ {
/*N*/ 	SwStartNode* pTmp = IsStartNode() ? (SwStartNode*)this : pStartOfSection;
/*N*/
/*N*/ 	while( eTyp != pTmp->GetStartNodeType() && pTmp->GetIndex() )
/*N*/ #if defined( ALPHA ) && defined( UNX )
/*?*/ 		pTmp = ((SwNode*)pTmp)->pStartOfSection;
/*N*/ #else
/*N*/ 		pTmp = pTmp->pStartOfSection;
/*N*/ #endif
/*N*/ 	return eTyp == pTmp->GetStartNodeType() ? pTmp : 0;
/*N*/ }

/*N*/ const SwTxtNode* SwNode::FindOutlineNodeOfLevel( BYTE nLvl ) const
/*N*/ {
/*N*/ 	const SwTxtNode* pRet = 0;
/*N*/ 	const SwOutlineNodes& rONds = GetNodes().GetOutLineNds();
/*N*/ 	if( MAXLEVEL > nLvl && rONds.Count() )
/*N*/ 	{
/*N*/ 		USHORT nPos;
/*N*/ 		SwNode* pNd = (SwNode*)this;
/*N*/ 		BOOL bCheckFirst = FALSE;
/*N*/ 		if( !rONds.Seek_Entry( pNd, &nPos ))
/*N*/ 		{
/*N*/ 			if( nPos )
/*N*/ 				nPos = nPos-1;
/*N*/ 			else
/*?*/ 				bCheckFirst = TRUE;
/*N*/ 		}
/*N*/
/*N*/ 		if( bCheckFirst )
/*N*/ 		{
/*?*/ 			// der 1.GliederungsNode liegt hinter dem Fragenden. Dann
/*?*/ 			// teste mal, ob dieser auf der gleichen Seite steht. Wenn
/*?*/ 			// nicht, ist das ein ungueltiger. Bug 61865
/*?*/ 			pRet = rONds[0]->GetTxtNode();
/*?*/
/*?*/ 			const SwCntntNode* pCNd = GetCntntNode();
/*?*/
/*?*/ 			Point aPt( 0, 0 );
/*?*/ 			const SwFrm* pFrm = pRet->GetFrm( &aPt, 0, FALSE ),
/*?*/ 					   * pMyFrm = pCNd ? pCNd->GetFrm( &aPt, 0, FALSE ) : 0;
/*?*/ 			const SwPageFrm* pPgFrm = pFrm ? pFrm->FindPageFrm() : 0;
/*?*/ 			if( pPgFrm && pMyFrm &&
/*?*/ 				pPgFrm->Frm().Top() > pMyFrm->Frm().Top() )
/*?*/ 			{
/*?*/ 				// der Fragende liegt vor der Seite, also ist er ungueltig
/*?*/ 				pRet = 0;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// oder ans Feld und von dort holen !!
/*N*/ 			while( nPos && nLvl < ( pRet = rONds[nPos]->GetTxtNode() )
/*N*/ 					->GetTxtColl()->GetOutlineLevel() )
/*N*/ 				--nPos;
/*N*/
/*N*/ 			if( !nPos )		// bei 0 gesondert holen !!
/*N*/ 				pRet = rONds[0]->GetTxtNode();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pRet;
/*N*/ }

// is the node the first and/or last node of a section?
// This information is used for the export filters. Our layout never have a
// distance before or after if the node is the first or last in a section.



/*******************************************************************
|*
|*	SwNode::StartOfSection
|*
|*	Beschreibung
|*		Die Funktion liefert die StartOfSection des Nodes.
|*
|*	Parameter
|*		IN
|*		rNodes bezeichnet das variable Array, in dem sich der Node
|*		befindet
|*	Ersterstellung
|*		VER0100 vb 901214
|*
|*	Stand
|*		VER0100 vb 901214
|*
*******************************************************************/


/*N*/ SwStartNode::SwStartNode( const SwNodeIndex &rWhere, const BYTE nNdType,
/*N*/ 							SwStartNodeType eSttNd )
/*N*/ 	: SwNode( rWhere, nNdType ), eSttNdTyp( eSttNd )
/*N*/ {
/*N*/ 	// erstmal temporaer, bis der EndNode eingefuegt wird.
/*N*/ 	pEndOfSection = (SwEndNode*)this;
/*N*/ }

/*N*/ SwStartNode::SwStartNode( SwNodes& rNodes, ULONG nPos )
/*N*/ 	: SwNode( rNodes, nPos, ND_STARTNODE ), eSttNdTyp( SwNormalStartNode )
/*N*/ {
/*N*/ 	// erstmal temporaer, bis der EndNode eingefuegt wird.
/*N*/ 	pEndOfSection = (SwEndNode*)this;
/*N*/ }


/*N*/ void SwStartNode::CheckSectionCondColl() const
/*N*/ {
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	SwNodeIndex aIdx( *this );
/*N*/ 	ULONG nEndIdx = EndOfSectionIndex();
/*N*/ 	const SwNodes& rNds = GetNodes();
/*N*/ 	SwCntntNode* pCNd;
/*N*/ 	while( 0 != ( pCNd = rNds.GoNext( &aIdx )) && pCNd->GetIndex() < nEndIdx )
/*N*/ 		pCNd->ChkCondColl();
/*N*/ //FEATURE::CONDCOLL
/*N*/ }

/*******************************************************************
|*
|*	SwEndNode::SwEndNode
|*
|*	Beschreibung
|*		Konstruktor; dieser fuegt einen Node in das Array rNodes
|*		an der Position aWhere ein. Der
|*		theStartOfSection-Pointer wird entsprechend gesetzt,
|*		und der EndOfSection-Pointer des zugehoerigen
|*		Startnodes -- durch rStartOfSection bezeichnet --
|*		wird auf diesen Node gesetzt.
|*
|*	Parameter
|*		IN
|*		rNodes bezeichnet das variable Array, in das der Node
|*		eingefuegt werden soll
|*		IN
|*		aWhere bezeichnet die Position innerhalb dieses Arrays,
|*		an der der Node eingefuegt werden soll
|*		!!!!!!!!!!!!
|*		Es wird eine Kopie uebergeben!
|*
|*	Ersterstellung
|*		VER0100 vb 901214
|*
|*	Stand
|*		VER0100 vb 901214
|*
*******************************************************************/


/*N*/ SwEndNode::SwEndNode( const SwNodeIndex &rWhere, SwStartNode& rSttNd )
/*N*/ 	: SwNode( rWhere, ND_ENDNODE )
/*N*/ {
/*N*/ 	pStartOfSection = &rSttNd;
/*N*/ 	pStartOfSection->pEndOfSection = this;
/*N*/ }

/*N*/ SwEndNode::SwEndNode( SwNodes& rNds, ULONG nPos, SwStartNode& rSttNd )
/*N*/ 	: SwNode( rNds, nPos, ND_ENDNODE )
/*N*/ {
/*N*/ 	pStartOfSection = &rSttNd;
/*N*/ 	pStartOfSection->pEndOfSection = this;
/*N*/ }



// --------------------
// SwCntntNode
// --------------------


/*N*/ SwCntntNode::SwCntntNode( const SwNodeIndex &rWhere, const BYTE nNdType,
/*N*/ 							SwFmtColl *pColl )
/*N*/ 	: SwNode( rWhere, nNdType ),
/*N*/ 	pAttrSet( 0 ),
/*N*/ 	pCondColl( 0 ),
/*N*/ 	SwModify( pColl )	 // CrsrsShell, FrameFmt
/*N*/ #ifdef OLD_INDEX
/*N*/ 	,SwIndexReg(2)
/*N*/ #endif
/*N*/ {
/*N*/ }


/*N*/ SwCntntNode::~SwCntntNode()
/*N*/ {
/*N*/ 	// Die Basisklasse SwClient vom SwFrm nimmt sich aus
/*N*/ 	// der Abhaengikeitsliste raus!
/*N*/ 	// Daher muessen alle Frames in der Abhaengigkeitsliste geloescht werden.
/*N*/ 	if( GetDepends() )
/*N*/ 		DelFrms();
/*N*/
/*N*/ 	if( pAttrSet )
/*N*/ 		delete pAttrSet;
/*N*/ 	if( pCondColl )
/*?*/ 		delete pCondColl;
/*N*/ }


/*N*/ void SwCntntNode::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
/*N*/ {
/*N*/ 	USHORT nWhich = pOldValue ? pOldValue->Which() :
/*N*/ 					pNewValue ? pNewValue->Which() : 0 ;
/*N*/ 	BOOL bNumRuleSet = FALSE, bCallModify = TRUE;
/*N*/ 	String sNumRule, sOldNumRule;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 	case RES_OBJECTDYING :
/*N*/ 		{
/*?*/ 			SwFmt * pFmt = (SwFmt *) ((SwPtrMsgPoolItem *)pNewValue)->pObject;
/*?*/
/*?*/ 			// nicht umhaengen wenn dieses das oberste Format ist !!
/*?*/ 			if( pRegisteredIn == pFmt )
/*?*/ 			{
/*?*/ 				if( pFmt->GetRegisteredIn() )
/*?*/ 				{
/*?*/ 					// wenn Parent, dann im neuen Parent wieder anmelden
/*?*/ 					((SwModify*)pFmt->GetRegisteredIn())->Add( this );
/*?*/ 					if ( pAttrSet )
/*?*/ 						pAttrSet->SetParent(
/*?*/ 								&((SwFmt*)GetRegisteredIn())->GetAttrSet() );
/*?*/ 				}
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// sonst auf jeden Fall beim sterbenden abmelden
/*?*/ 					((SwModify*)GetRegisteredIn())->Remove( this );
/*?*/ 					if ( pAttrSet )
/*?*/ 						pAttrSet->SetParent( 0 );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/
/*N*/ 	case RES_FMT_CHG:
/*N*/ 		// falls mein Format Parent umgesetzt wird, dann melde ich
/*N*/ 		// meinen Attrset beim Neuen an.
/*N*/
/*N*/ 		// sein eigenes Modify ueberspringen !!
/*N*/ 		if( pAttrSet &&
/*N*/ 			((SwFmtChg*)pNewValue)->pChangedFmt == GetRegisteredIn() )
/*N*/ 		{
/*N*/ 			// den Set an den neuen Parent haengen
/*N*/ 			pAttrSet->SetParent( GetRegisteredIn() ?
/*N*/ 				&((SwFmt*)GetRegisteredIn())->GetAttrSet() : 0 );
/*N*/ 		}
/*N*/ 		if( GetNodes().IsDocNodes() && IsTxtNode() )
/*N*/ 		{
/*N*/ 			if( 0 != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, TRUE )))
/*N*/ 			{
/*?*/ 				bNumRuleSet = TRUE;
/*?*/ 				sNumRule = ((SwNumRuleItem*)pItem)->GetValue();
/*N*/ 			}
/*N*/ 			sOldNumRule = ((SwFmtChg*)pOldValue)->pChangedFmt->GetNumRule().GetValue();
/*N*/ 		}
/*N*/ 		break;
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	case RES_CONDCOLL_CONDCHG:
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( ((SwCondCollCondChg*)pNewValue)->pChangedFmt == GetRegisteredIn() &&
/*?*/ 		return ;	// nicht an die Basisklasse / Frames weitergeben
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 	case RES_ATTRSET_CHG:
/*N*/ 		if( GetNodes().IsDocNodes() && IsTxtNode() )
/*N*/ 		{
/*N*/ 			if( SFX_ITEM_SET == ((SwAttrSetChg*)pNewValue)->GetChgSet()->GetItemState(
/*N*/ 				RES_PARATR_NUMRULE, FALSE, &pItem ))
/*N*/ 			{
/*N*/ 				bNumRuleSet = TRUE;
/*N*/ 				sNumRule = ((SwNumRuleItem*)pItem)->GetValue();
/*N*/ 			}
/*N*/ 			if( SFX_ITEM_SET == ((SwAttrSetChg*)pOldValue)->GetChgSet()->GetItemState(
/*N*/ 				RES_PARATR_NUMRULE, FALSE, &pItem ))
/*N*/ 				sOldNumRule = ((SwNumRuleItem*)pItem)->GetValue();
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RES_PARATR_NUMRULE:
/*?*/ 		if( GetNodes().IsDocNodes() && IsTxtNode() )
/*?*/ 		{
/*?*/ 			if( pNewValue )
/*?*/ 			{
/*?*/ 				bNumRuleSet = TRUE;
/*?*/ 				sNumRule = ((SwNumRuleItem*)pNewValue)->GetValue();
/*?*/ 			}
/*?*/ 			if( pOldValue )
/*?*/ 				sOldNumRule = ((SwNumRuleItem*)pOldValue)->GetValue();
/*?*/ 		}
/*?*/ 		break;
/*?*/ 	}
/*N*/
/*N*/ 	if( bNumRuleSet )
/*N*/ 	{
/*N*/ 		if( sNumRule.Len() )
/*N*/ 		{
/*N*/ 			if( !((SwTxtNode*)this)->GetNum() )
/*N*/ 				((SwTxtNode*)this)->UpdateNum( SwNodeNum(0) );
/*N*/ #ifndef NUM_RELSPACE
/*N*/ 			SetNumLSpace( TRUE );
/*N*/ #endif
/*N*/ 			SwNumRule* pRule = GetDoc()->FindNumRulePtr( sNumRule );
/*N*/ 			if( !pRule )
/*N*/ 			{
/*?*/ 				USHORT nPoolId = SwStyleNameMapper::GetPoolIdFromUIName( sNumRule, GET_POOLID_NUMRULE );
/*?*/ 				if( USHRT_MAX != nPoolId )
/*?*/ 					pRule = GetDoc()->GetNumRuleFromPool( nPoolId );
/*N*/ 			}
/*N*/ 			if( pRule )
/*N*/ 				pRule->SetInvalidRule( TRUE );
/*N*/ 		}
/*N*/ 		else if( ((SwTxtNode*)this)->GetNum() )
/*N*/ 		{
/*?*/ 			bCallModify = FALSE;
/*?*/ 			SwModify::Modify( pOldValue, pNewValue );
/*?*/ 			((SwTxtNode*)this)->UpdateNum( SwNodeNum(NO_NUMBERING) );
/*?*/ #ifndef NUM_RELSPACE
/*?*/ 			SetNumLSpace( TRUE );
/*?*/ #endif
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if( sOldNumRule.Len() && sNumRule != sOldNumRule )
/*N*/ 	{
/*?*/ 		SwNumRule* pRule = GetDoc()->FindNumRulePtr( sOldNumRule );
/*?*/ 		if( pRule )
/*?*/ 			pRule->SetInvalidRule( TRUE );
/*N*/ 	}
/*N*/
/*N*/ 	if( bCallModify )
/*N*/ 		SwModify::Modify( pOldValue, pNewValue );
/*N*/ }

/*N*/ BOOL SwCntntNode::InvalidateNumRule()
/*N*/ {
/*N*/ 	SwNumRule* pRule = 0;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if( GetNodes().IsDocNodes() &&
/*N*/ 		0 != ( pItem = GetNoCondAttr( RES_PARATR_NUMRULE, TRUE )) &&
/*N*/ 		((SwNumRuleItem*)pItem)->GetValue().Len() &&
/*N*/ 		0 != (pRule = GetDoc()->FindNumRulePtr(
/*N*/ 								((SwNumRuleItem*)pItem)->GetValue() ) ) )
/*N*/ 	{
/*N*/ 		pRule->SetInvalidRule( TRUE );
/*N*/ 	}
/*N*/ 	return 0 != pRule;
/*N*/ }


/*N*/ SwCntntFrm *SwCntntNode::GetFrm( const Point* pPoint,
/*N*/ 								const SwPosition *pPos,
/*N*/ 								const BOOL bCalcFrm ) const
/*N*/ {
/*N*/ 	return (SwCntntFrm*) ::binfilter::GetFrmOfModify( *(SwModify*)this, FRM_CNTNT,
/*N*/ 											pPoint, pPos, bCalcFrm );
/*N*/ }



/*N*/ xub_StrLen SwCntntNode::Len() const { return 0; }



/*N*/ SwFmtColl *SwCntntNode::ChgFmtColl( SwFmtColl *pNewColl )
/*N*/ {
/*N*/ 	ASSERT( pNewColl, Collectionpointer ist 0. );
/*N*/ 	SwFmtColl *pOldColl = GetFmtColl();
/*N*/ 	if( pNewColl != pOldColl )
/*N*/ 	{
/*N*/ 		pNewColl->Add( this );
/*N*/
/*N*/ 		// setze den Parent von unseren Auto-Attributen auf die neue
/*N*/ 		// Collection:
/*N*/ 		if( pAttrSet )
/*?*/ 			pAttrSet->SetParent( &pNewColl->GetAttrSet() );
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		// HACK: hier muss die entsprechend der neuen Vorlage die Bedingungen
/*N*/ 		//		neu ueberprueft werden!
/*N*/ 		if( TRUE /*pNewColl */ )
/*N*/ 		{
/*N*/ 			SetCondFmtColl( 0 );
/*N*/ 		}
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 		if( !IsModifyLocked() )
/*N*/ 		{
/*N*/ 			SwFmtChg aTmp1( pOldColl );
/*N*/ 			SwFmtChg aTmp2( pNewColl );
/*N*/ //			SwModify::Modify( &aTmp1, &aTmp2 );
/*N*/ 			// damit alles was im Modify passiert hier nicht noch impl.
/*N*/ 			// werden muss
/*N*/ 			SwCntntNode::Modify( &aTmp1, &aTmp2 );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*?*/ 		SwFrm::GetCache().Delete( this );
/*?*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	return pOldColl;
/*N*/ }


/*N*/ BOOL SwCntntNode::GoNext(SwIndex * pIdx, USHORT nMode ) const
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	if( pIdx->GetIndex() < Len() )
/*N*/ 	{
/*N*/ 		if( !IsTxtNode() )
/*?*/ 			(*pIdx)++;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwTxtNode& rTNd = *GetTxtNode();
/*N*/ 			xub_StrLen nPos = pIdx->GetIndex();
/*N*/ 			if( pBreakIt->xBreak.is() )
/*N*/ 			{
/*N*/ 				sal_Int32 nDone = 0;
/*N*/ 				sal_uInt16 nItrMode = CRSR_SKIP_CHARS == nMode
/*N*/ 									? CharacterIteratorMode::SKIPCONTROLCHARACTER
/*N*/ 									: CharacterIteratorMode::SKIPCELL;
/*N*/     			nPos = pBreakIt->xBreak->nextCharacters( rTNd.GetTxt(), nPos,
/*N*/ 								pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
/*N*/ 								nItrMode, 1, nDone );
/*N*/ 				if( 1 == nDone )
/*N*/ 					*pIdx = nPos;
/*N*/ 				else
/*?*/ 					bRet = FALSE;
/*N*/ 			}
/*N*/ 			else if( nPos < rTNd.GetTxt().Len() )
/*?*/ 				(*pIdx)++;
/*N*/ 			else
/*?*/ 				bRet = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bRet = FALSE;
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SwCntntNode::GoPrevious(SwIndex * pIdx, USHORT nMode ) const
/*N*/ {
/*N*/ 	BOOL bRet = TRUE;
/*N*/ 	if( pIdx->GetIndex() > 0 )
/*N*/ 	{
/*N*/ 		if( !IsTxtNode() )
/*?*/ 			(*pIdx)--;
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwTxtNode& rTNd = *GetTxtNode();
/*N*/ 			xub_StrLen nPos = pIdx->GetIndex();
/*N*/ 			if( pBreakIt->xBreak.is() )
/*N*/ 			{
/*N*/ 				sal_Int32 nDone = 0;
/*N*/ 				sal_uInt16 nItrMode = CRSR_SKIP_CHARS == nMode
/*N*/ 								? CharacterIteratorMode::SKIPCONTROLCHARACTER
/*N*/ 								: CharacterIteratorMode::SKIPCELL;
/*N*/     			nPos = pBreakIt->xBreak->previousCharacters( rTNd.GetTxt(), nPos,
/*N*/ 								pBreakIt->GetLocale( rTNd.GetLang( nPos ) ),
/*N*/ 								nItrMode, 1, nDone );
/*N*/ 				if( 1 == nDone )
/*N*/ 					*pIdx = nPos;
/*N*/ 				else
/*?*/ 					bRet = FALSE;
/*N*/ 			}
/*N*/ 			else if( nPos )
/*?*/ 				(*pIdx)--;
/*N*/ 			else
/*?*/ 				bRet = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 		bRet = FALSE;
/*N*/ 	return bRet;
/*N*/ }


/*
 * Methode erzeugt fuer den vorhergehenden Node alle Ansichten vom
 * Dokument. Die erzeugten Contentframes werden in das entsprechende
 * Layout gehaengt.
 */


/*N*/ void SwCntntNode::MakeFrms( SwCntntNode& rNode )
/*N*/ {
/*N*/ 	ASSERT( &rNode != this,
/*N*/ 			"Kein Contentnode oder Copy-Node und neuer Node identisch." );
/*N*/
/*N*/ 	if( !GetDepends() || &rNode == this )	// gibt es ueberhaupt Frames ??
/*?*/ 		return;
/*N*/
/*N*/ 	SwFrm *pFrm, *pNew;
/*N*/ 	SwLayoutFrm *pUpper;
/*N*/ 	// Frames anlegen fuer Nodes, die vor oder hinter der Tabelle stehen ??
/*N*/ 	ASSERT( FindTableNode() == rNode.FindTableNode(), "Table confusion" )
/*N*/
/*N*/ 	SwNode2Layout aNode2Layout( *this, rNode.GetIndex() );
/*N*/
/*N*/ 	while( 0 != (pUpper = aNode2Layout.UpperFrm( pFrm, rNode )) )
/*N*/ 	{
/*N*/ 		pNew = rNode.MakeFrm();
/*N*/ 		pNew->Paste( pUpper, pFrm );
/*N*/ 	}
/*N*/ }

/*
 * Methode loescht fuer den Node alle Ansichten vom
 * Dokument. Die Contentframes werden aus dem entsprechenden
 * Layout ausgehaengt.
 */


/*N*/ void SwCntntNode::DelFrms()
/*N*/ {
/*N*/ 	if( !GetDepends() )
/*N*/ 		return;
/*N*/
/*N*/ 	SwClientIter aIter( *this );
/*N*/ 	SwCntntFrm *pFrm;
/*N*/
/*N*/ 	for( pFrm = (SwCntntFrm*)aIter.First( TYPE(SwCntntFrm)); pFrm;
/*N*/ 		 pFrm = (SwCntntFrm*)aIter.Next() )
/*N*/ 	{
/*N*/         if( pFrm->HasFollow() )
/*N*/             pFrm->GetFollow()->_SetIsFollow( pFrm->IsFollow() );
/*N*/         if( pFrm->IsFollow() )
/*N*/         {
/*?*/             SwCntntFrm* pMaster = (SwTxtFrm*)pFrm->FindMaster();
/*?*/             pMaster->SetFollow( pFrm->GetFollow() );
/*?*/             pFrm->_SetIsFollow( FALSE );
/*N*/         }
/*N*/ 		pFrm->SetFollow( 0 );//Damit er nicht auf dumme Gedanken kommt.
/*N*/ 								//Andernfalls kann es sein, dass ein Follow
/*N*/ 								//vor seinem Master zerstoert wird, der Master
/*N*/ 								//greift dann ueber den ungueltigen
/*N*/ 								//Follow-Pointer auf fremdes Memory zu.
/*N*/ 								//Die Kette darf hier zerknauscht werden, weil
/*N*/ 								//sowieso alle zerstoert werden.
/*N*/ 		if( pFrm->GetUpper() && pFrm->IsInFtn() && !pFrm->GetIndNext() &&
/*N*/ 			!pFrm->GetIndPrev() )
/*N*/ 		{
/*?*/ 			SwFtnFrm *pFtn = pFrm->FindFtnFrm();
/*?*/ 			ASSERT( pFtn, "You promised a FtnFrm?" );
/*?*/ 			SwCntntFrm* pCFrm;
/*?*/ 			if( !pFtn->GetFollow() && !pFtn->GetMaster() &&
/*?*/ 				0 != ( pCFrm = pFtn->GetRefFromAttr()) && pCFrm->IsFollow() )
/*?*/ 			{
/*?*/ 				ASSERT( pCFrm->IsTxtFrm(), "NoTxtFrm has Footnote?" );
/*?*/ 				((SwTxtFrm*)pCFrm->FindMaster())->Prepare( PREP_FTN_GONE );
/*?*/ 			}
/*N*/ 		}
/*N*/ 		pFrm->Cut();
/*N*/ 		delete pFrm;
/*N*/ 	}
/*N*/ 	if( IsTxtNode() )
/*N*/ 	{
/*N*/ 		((SwTxtNode*)this)->SetWrong( NULL );
/*N*/ 		SetWrongDirty( TRUE );
/*N*/ 		SetAutoCompleteWordDirty( TRUE );
/*N*/ 	}
/*N*/ }


 SwCntntNode *SwCntntNode::JoinNext()
 {
    return this;
 }


 SwCntntNode *SwCntntNode::JoinPrev()
 {
    return this;
 }



    // erfrage vom Modify Informationen
/*N*/ BOOL SwCntntNode::GetInfo( SfxPoolItem& rInfo ) const
/*N*/ {
/*N*/ 	const SwNumRuleItem* pItem;
/*N*/ 	switch( rInfo.Which() )
/*N*/ 	{
/*N*/ 	case RES_AUTOFMT_DOCNODE:
/*N*/ 		if( &GetNodes() == ((SwAutoFmtGetDocNode&)rInfo).pNodes )
/*N*/ 		{
/*N*/ 			((SwAutoFmtGetDocNode&)rInfo).pCntntNode = this;
/*N*/ 			return FALSE;
/*N*/ 		}
/*N*/ 		break;
/*?*/ 	case RES_GETNUMNODES:
/*?*/ 		if( IsTxtNode() && 0 != ( pItem = (SwNumRuleItem*)GetNoCondAttr(
/*?*/ 			RES_PARATR_NUMRULE, TRUE )) &&
/*?*/ 			pItem->GetValue().Len() &&
/*?*/ 			pItem->GetValue() == ((SwNumRuleInfo&)rInfo).GetName() &&
/*?*/ 			GetNodes().IsDocNodes() )
/*?*/ 		{
/*?*/ 			((SwNumRuleInfo&)rInfo).AddNode( *(SwTxtNode*)this );
/*?*/ 		}
/*?*/ 		return TRUE;
/*?*/
/*?*/ 	case RES_GETLOWERNUMLEVEL:
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( IsTxtNode() && ((SwTxtNode*)this)->GetNum() &&
/*?*/ 		break;
/*?*/
/*?*/ 	case RES_FINDNEARESTNODE:
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( ((SwFmtPageDesc&)GetAttr( RES_PAGEDESC )).GetPageDesc() )
/*?*/ 		return TRUE;
/*?*/
/*?*/ 	case RES_CONTENT_VISIBLE:
/*?*/ 		{
/*?*/ 			((SwPtrMsgPoolItem&)rInfo).pObject =
/*?*/ 				SwClientIter( *(SwCntntNode*)this ).First( TYPE(SwFrm) );
/*?*/ 		}
/*?*/ 		return FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	return SwModify::GetInfo( rInfo );
/*N*/ }


    // setze ein Attribut
/*N*/ BOOL SwCntntNode::SetAttr(const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	if( !pAttrSet )			// lasse von den entsprechenden Nodes die
/*N*/ 		NewAttrSet( GetDoc()->GetAttrPool() );		// AttrSets anlegen
/*N*/
/*N*/ 	ASSERT( pAttrSet, "warum wurde kein AttrSet angelegt?" );
/*N*/
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt
/*N*/ 	if( IsModifyLocked() ||
/*N*/ 		( !GetDepends() &&  RES_PARATR_NUMRULE != rAttr.Which() ))
/*N*/ 	{
/*N*/ 		if( 0 != ( bRet = (0 != pAttrSet->Put( rAttr )) ))
/*N*/ 			// einige Sonderbehandlungen fuer Attribute
/*N*/ 			pAttrSet->SetModifyAtAttr( this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
/*N*/ 					aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
/*N*/ 		if( 0 != ( bRet = pAttrSet->Put_BC( rAttr, &aOld, &aNew ) ))
/*N*/ 		{
/*N*/ 			// einige Sonderbehandlungen fuer Attribute
/*N*/ 			pAttrSet->SetModifyAtAttr( this );
/*N*/
/*N*/ 			SwAttrSetChg aChgOld( *pAttrSet, aOld );
/*N*/ 			SwAttrSetChg aChgNew( *pAttrSet, aNew );
/*N*/ 			Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SwCntntNode::SetAttr( const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	if( !pAttrSet )			// lasse von den entsprechenden Nodes die
/*N*/ 		NewAttrSet( GetDoc()->GetAttrPool() );		// AttrSets anlegen
/*N*/
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt
/*N*/ 	if( IsModifyLocked() || ( !GetDepends() &&
/*N*/ 		SFX_ITEM_SET != rSet.GetItemState( RES_PARATR_NUMRULE, FALSE )) )
/*N*/ 	{
/*N*/ 		// einige Sonderbehandlungen fuer Attribute
/*N*/ 		if( 0 != (bRet = (0 != pAttrSet->Put( rSet ))) )
/*N*/ 			pAttrSet->SetModifyAtAttr( this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
/*N*/ 					aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
/*N*/ 		if( 0 != (bRet = pAttrSet->Put_BC( rSet, &aOld, &aNew )) )
/*N*/ 		{
/*N*/ 			// einige Sonderbehandlungen fuer Attribute
/*N*/ 			pAttrSet->SetModifyAtAttr( this );
/*N*/ 			SwAttrSetChg aChgOld( *pAttrSet, aOld );
/*N*/ 			SwAttrSetChg aChgNew( *pAttrSet, aNew );
/*N*/ 			Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }

// Nimmt den Hint mit nWhich aus dem Delta-Array


/*N*/ BOOL SwCntntNode::ResetAttr( USHORT nWhich1, USHORT nWhich2 )
/*N*/ {
/*N*/ 	if( !pAttrSet )
/*?*/ 		return FALSE;
/*N*/
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*?*/ 		SwFrm::GetCache().Delete( this );
/*?*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt
/*N*/ 	if( IsModifyLocked() )
/*N*/ 	{
/*?*/ 		USHORT nDel = (!nWhich2 || nWhich2 < nWhich1)
/*?*/ 				? pAttrSet->ClearItem( nWhich1 )
/*?*/ 				: pAttrSet->ClearItem_BC( nWhich1, nWhich2 );
/*?*/
/*?*/ 		if( !pAttrSet->Count() )	// leer, dann loeschen
/*?*/ 			DELETEZ( pAttrSet );
/*?*/ 		return 0 != nDel;
/*N*/ 	}
/*N*/
/*N*/ 	// sollte kein gueltiger Bereich definiert sein ?
/*N*/ 	if( !nWhich2 || nWhich2 < nWhich1 )
/*N*/ 		nWhich2 = nWhich1;		// dann setze auf 1. Id, nur dieses Item
/*N*/
/*N*/ 	SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
/*N*/ 				aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
/*N*/ 	BOOL bRet = 0 != pAttrSet->ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );
/*N*/
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		SwAttrSetChg aChgOld( *pAttrSet, aOld );
/*N*/ 		SwAttrSetChg aChgNew( *pAttrSet, aNew );
/*N*/ 		Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/
/*N*/ 		if( !pAttrSet->Count() )	// leer, dann loeschen
/*N*/ 			DELETEZ( pAttrSet );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ USHORT SwCntntNode::ResetAllAttr()
/*N*/ {
/*N*/ 	if( !pAttrSet )
/*N*/ 		return 0;
/*N*/
/*N*/ 	if ( IsInCache() )
/*N*/ 	{
/*?*/ 		SwFrm::GetCache().Delete( this );
/*?*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/
/*N*/ 	// wenn Modify gelockt ist, werden keine Modifies verschickt
/*N*/ 	if( IsModifyLocked() )
/*N*/ 	{
/*?*/ 		USHORT nDel = pAttrSet->ClearItem( 0 );
/*?*/ 		if( !pAttrSet->Count() )	// leer, dann loeschen
/*?*/ 			DELETEZ( pAttrSet );
/*?*/ 		return nDel;
/*N*/ 	}
/*N*/
/*N*/ 	SwAttrSet aOld( *pAttrSet->GetPool(), pAttrSet->GetRanges() ),
/*N*/ 				aNew( *pAttrSet->GetPool(), pAttrSet->GetRanges() );
/*N*/ 	BOOL bRet = 0 != pAttrSet->ClearItem_BC( 0, &aOld, &aNew );
/*N*/
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		SwAttrSetChg aChgOld( *pAttrSet, aOld );
/*N*/ 		SwAttrSetChg aChgNew( *pAttrSet, aNew );
/*N*/ 		Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/
/*N*/ 		if( !pAttrSet->Count() )	// leer, dann loeschen
/*N*/ 			DELETEZ( pAttrSet );
/*N*/ 	}
/*N*/ 	return aNew.Count();
/*N*/ }


/*N*/ BOOL SwCntntNode::GetAttr( SfxItemSet& rSet, BOOL bInParent ) const
/*N*/ {
/*N*/ 	if( rSet.Count() )
/*?*/ 		rSet.ClearItem();
/*N*/
/*N*/ 	const SwAttrSet& rAttrSet = GetSwAttrSet();
/*N*/ 	if( bInParent )
/*N*/ 		return rSet.Set( rAttrSet, TRUE ) ? TRUE : FALSE;
/*N*/
/*?*/ 	rSet.Put( rAttrSet );
/*?*/ 	return rSet.Count() ? TRUE : FALSE;
/*N*/ }

/*N*/ const SfxPoolItem* SwCntntNode::GetNoCondAttr( USHORT nWhich,
/*N*/ 												BOOL bInParents ) const
/*N*/ {
/*N*/ 	const SfxPoolItem* pFnd = 0;
/*N*/ 	if( pCondColl && pCondColl->GetRegisteredIn() )
/*N*/ 	{
/*?*/ 		if( !pAttrSet || ( SFX_ITEM_SET != pAttrSet->GetItemState(
/*?*/ 					nWhich, FALSE, &pFnd ) && bInParents ))
/*?*/ 			((SwFmt*)GetRegisteredIn())->GetItemState( nWhich, bInParents, &pFnd );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		GetSwAttrSet().GetItemState( nWhich, bInParents, &pFnd );
/*N*/ 	return pFnd;
/*N*/ }

    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
/*N*/ int SwCntntNode::CanJoinNext( SwNodeIndex* pIdx ) const
/*N*/ {
/*N*/ 	const SwNodes& rNds = GetNodes();
/*N*/ 	BYTE nNdType = GetNodeType();
/*N*/ 	SwNodeIndex aIdx( *this, 1 );
/*N*/
/*N*/ 	const SwNode* pNd = this;
/*N*/ 	while( aIdx < rNds.Count()-1 &&
/*N*/ 		(( pNd = &aIdx.GetNode())->IsSectionNode() ||
/*N*/ 			( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode() )))
/*N*/ 		aIdx++;
/*N*/
/*N*/ 	if( pNd->GetNodeType() != nNdType || rNds.Count()-1 == aIdx.GetIndex() )
/*N*/ 		return FALSE;
/*N*/ 	if( pIdx )
/*N*/ 		*pIdx = aIdx;
/*N*/ 	return TRUE;
/*N*/ }


    // koennen 2 Nodes zusammengefasst werden ?
    // in pIdx kann die 2. Position returnt werden.
/*N*/ int SwCntntNode::CanJoinPrev( SwNodeIndex* pIdx ) const
/*N*/ {
/*N*/ 	const SwNodes& rNds = GetNodes();
/*N*/ 	BYTE nNdType = GetNodeType();
/*N*/ 	SwNodeIndex aIdx( *this, -1 );
/*N*/
/*N*/ 	const SwNode* pNd = this;
/*N*/ 	while( aIdx.GetIndex() &&
/*N*/ 		(( pNd = &aIdx.GetNode())->IsSectionNode() ||
/*N*/ 			( pNd->IsEndNode() && pNd->FindStartNode()->IsSectionNode() )))
/*?*/ 		aIdx--;
/*N*/
/*N*/ 	if( pNd->GetNodeType() != nNdType || 0 == aIdx.GetIndex() )
/*?*/ 		return FALSE;
/*N*/ 	if( pIdx )
/*N*/ 		*pIdx = aIdx;
/*N*/ 	return TRUE;
/*N*/ }


//FEATURE::CONDCOLL


/*N*/ void SwCntntNode::SetCondFmtColl( SwFmtColl* pColl )
/*N*/ {
/*N*/ 	if( (!pColl && pCondColl) || ( pColl && !pCondColl ) ||
/*N*/ 		( pColl && pColl != pCondColl->GetRegisteredIn() ) )
/*N*/ 	{
/*?*/ 		SwFmtColl* pOldColl = GetCondFmtColl();
/*?*/ 		delete pCondColl;
/*?*/ 		if( pColl )
/*?*/ 			pCondColl = new SwDepend( this, pColl );
/*?*/ 		else
/*?*/ 			pCondColl = 0;
/*?*/
/*?*/ 		if( pAttrSet )
/*?*/ 		{
/*?*/ // Attrset beibehalten oder loeschen??
/*?*/ // 13.04.99: Bisher wurden er geloescht, jetzt wird er beibehalten.
/*?*/ //		     #64637#: Beim Laden eines Dokuments wird die bedingte
/*?*/ //			 Vorlage nach dem Laden der harten Attribute gesetzt. Deshalb
/*?*/ //			 wurden die harten Attribute geloescht.
/*?*/
/*?*/ 			pAttrSet->SetParent( &GetAnyFmtColl().GetAttrSet() );
/*?*/ // steht im docfmt.cxx
/*?*/ //extern BOOL lcl_RstAttr( const SwNodePtr&, void* );
/*?*/ //			lcl_RstAttr( this, 0 );
/*?*/ //			if( pAttrSet && !pAttrSet->Count() )
/*?*/ //				delete pAttrSet, pAttrSet = 0;
/*?*/ 		}
/*?*/
/*?*/ 		if( !IsModifyLocked() )
/*?*/ 		{
/*?*/ 			SwFmtChg aTmp1( pOldColl ? pOldColl : GetFmtColl() );
/*?*/ 			SwFmtChg aTmp2( pColl ? pColl : GetFmtColl() );
/*?*/ 			SwModify::Modify( &aTmp1, &aTmp2 );
/*?*/ 		}
/*?*/ 		if( IsInCache() )
/*?*/ 		{
/*?*/ 			SwFrm::GetCache().Delete( this );
/*?*/ 			SetInCache( FALSE );
/*?*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ BOOL SwCntntNode::IsAnyCondition( SwCollCondition& rTmp ) const
/*N*/ {
/*N*/ 	const SwNodes& rNds = GetNodes();
/*N*/ 	{
/*N*/ 		int nCond = 0;
/*N*/ 		const SwStartNode* pSttNd = FindStartNode();
/*N*/ 		while( pSttNd )
/*N*/ 		{
/*N*/ 			switch( pSttNd->GetNodeType() )
/*N*/ 			{
/*?*/ 			case ND_TABLENODE:		nCond = PARA_IN_TABLEBODY; break;
/*N*/ 			case ND_SECTIONNODE: 	nCond = PARA_IN_SECTION; break;
/*N*/
/*N*/ 			default:
/*N*/ 				switch( pSttNd->GetStartNodeType() )
/*N*/ 				{
/*N*/ 				case SwTableBoxStartNode:
/*N*/ 					{
/*N*/ 						nCond = PARA_IN_TABLEBODY;
/*N*/ 						const SwTableNode* pTblNd = pSttNd->FindTableNode();
/*N*/ 						const SwTableBox* pBox;
/*N*/ 						if( pTblNd && 0 != ( pBox = pTblNd->GetTable().
/*N*/ 							GetTblBox( pSttNd->GetIndex() ) ) &&
/*N*/ 							pBox->IsInHeadline( &pTblNd->GetTable() ) )
/*N*/ 							nCond = PARA_IN_TABLEHEAD;
/*N*/ 					}
/*N*/ 					break;
/*N*/ 				case SwFlyStartNode:		nCond = PARA_IN_FRAME; break;
/*N*/ 				case SwFootnoteStartNode:
/*?*/ 					{
/*?*/ 						nCond = PARA_IN_FOOTENOTE;
/*?*/ 						const SwFtnIdxs& rFtnArr = rNds.GetDoc()->GetFtnIdxs();
/*?*/ 						const SwTxtFtn* pTxtFtn;
/*?*/ 						const SwNode* pSrchNd = pSttNd;
/*?*/
/*?*/ 						for( USHORT n = 0; n < rFtnArr.Count(); ++n )
/*?*/ 							if( 0 != ( pTxtFtn = rFtnArr[ n ])->GetStartNode() &&
/*?*/ 								pSrchNd == &pTxtFtn->GetStartNode()->GetNode() )
/*?*/ 							{
/*?*/ 								if( pTxtFtn->GetFtn().IsEndNote() )
/*?*/ 									nCond = PARA_IN_ENDNOTE;
/*?*/ 								break;
/*?*/ 							}
/*?*/ 					}
/*?*/ 					break;
/*?*/ 				case SwHeaderStartNode:     nCond = PARA_IN_HEADER; break;
/*?*/ 				case SwFooterStartNode:     nCond = PARA_IN_FOOTER; break;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			if( nCond )
/*N*/ 			{
/*N*/ 				rTmp.SetCondition( (Master_CollConditions)nCond, 0 );
/*N*/ 				return TRUE;
/*N*/ 			}
/*N*/ 			pSttNd = pSttNd->GetIndex()
/*N*/ 						? pSttNd->FindStartNode()
/*N*/ 						: 0;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	{
/*N*/ 		USHORT nPos;
/*N*/ 		const SwOutlineNodes& rOutlNds = rNds.GetOutLineNds();
/*N*/ 		if( rOutlNds.Count() )
/*N*/ 		{
/*N*/ 			if( !rOutlNds.Seek_Entry( (SwCntntNode*)this, &nPos ) && nPos )
/*N*/ 				--nPos;
/*N*/ 			if( nPos < rOutlNds.Count() &&
/*N*/ 				rOutlNds[ nPos ]->GetIndex() < GetIndex() )
/*N*/ 			{
/*N*/ 				SwTxtNode* pOutlNd = rOutlNds[ nPos ]->GetTxtNode();
/*N*/
/*N*/ 				if( pOutlNd->GetOutlineNum() && !pOutlNd->GetNumRule() )
/*N*/ 				{
/*N*/ 					rTmp.SetCondition( PARA_IN_OUTLINE,
/*N*/ 									pOutlNd->GetOutlineNum()->GetLevel() );
/*N*/ 					return TRUE;
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void SwCntntNode::ChkCondColl()
/*N*/ {
/*N*/ 	// zur Sicherheit abfragen
/*N*/ 	if( RES_CONDTXTFMTCOLL == GetFmtColl()->Which() )
/*N*/ 	{
/*N*/ 		SwCollCondition aTmp( 0, 0, 0 );
/*N*/ 		const SwCollCondition* pCColl;
/*N*/
/*N*/ 		if( IsAnyCondition( aTmp ) && 0 != ( pCColl =
/*N*/ 				((SwConditionTxtFmtColl*)GetFmtColl())->HasCondition( aTmp )))
/*?*/ 			SetCondFmtColl( pCColl->GetTxtFmtColl() );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( IsTxtNode() && ((SwTxtNode*)this)->GetNumRule() &&
/*N*/ 					((SwTxtNode*)this)->GetNum() )
/*N*/ 			{
/*N*/ 				// steht in einer Numerierung
/*N*/ 				// welcher Level?
/*N*/ 				aTmp.SetCondition( PARA_IN_LIST,
/*N*/ 								((SwTxtNode*)this)->GetNum()->GetLevel() );
/*N*/ 				pCColl = ((SwConditionTxtFmtColl*)GetFmtColl())->
/*N*/ 								HasCondition( aTmp );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pCColl = 0;
/*N*/
/*N*/ 			if( pCColl )
/*?*/ 				SetCondFmtColl( pCColl->GetTxtFmtColl() );
/*N*/ 			else if( pCondColl )
/*?*/ 				SetCondFmtColl( 0 );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

//FEATURE::CONDCOLL
// Metoden aus Node.hxx - erst hier ist der TxtNode bekannt !!
// os: nur fuer ICC, da der zum optimieren zu dumm ist
/*N*/ #ifdef ICC
/*N*/ SwTxtNode   *SwNode::GetTxtNode()
/*N*/ {
/*N*/ 	 return ND_TEXTNODE == nNodeType ? (SwTxtNode*)this : 0;
/*N*/ }
/*N*/ const SwTxtNode   *SwNode::GetTxtNode() const
/*N*/ {
/*N*/ 	 return ND_TEXTNODE == nNodeType ? (const SwTxtNode*)this : 0;
/*N*/ }
/*N*/ #endif




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
