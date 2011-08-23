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


#include <bf_svx/linkmgr.hxx>
#include <bf_svtools/itemiter.hxx>
#include <tools/resid.hxx>

#include <fmtcntnt.hxx>
#include <txtftn.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <rootfrm.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <ftnidx.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <sectfrm.hxx>
#include <cntfrm.hxx>
#include <node2lay.hxx>
#include <doctxm.hxx>
#include <fmtftntx.hxx>
#include <hints.hxx>

#include <comcore.hrc>
namespace binfilter {

/*N*/ int lcl_IsInSameTblBox( SwNodes& rNds, const SwNode& rNd,
/*N*/ 							const SwNodeIndex& rIdx2 )
/*N*/ {
/*N*/ 	const SwTableNode* pTblNd = rNd.FindTableNode();
/*N*/ 	if( !pTblNd )
/*N*/ 		return TRUE;
/*N*/
/*?*/ 	// dann suche den StartNode der Box
/*?*/ 	const SwTableSortBoxes& rSortBoxes = pTblNd->GetTable().GetTabSortBoxes();
/*?*/ 	ULONG nIdx = rNd.GetIndex();
/*?*/ 	for( USHORT n = 0; n < rSortBoxes.Count(); ++n )
/*?*/ 	{
/*?*/ 		const SwStartNode* pNd = rSortBoxes[ n ]->GetSttNd();
/*?*/ 		if( pNd->GetIndex() < nIdx &&
/*?*/ 			nIdx < pNd->EndOfSectionIndex() )
/*?*/ 		{
/*?*/ 			// dann muss der andere Index in derselben Section liegen
/*?*/ 			nIdx = rIdx2.GetIndex();
/*?*/ 			return pNd->GetIndex() < nIdx && nIdx < pNd->EndOfSectionIndex();
/*?*/ 		}
/*?*/ 	}
/*?*/ 	return TRUE;
/*N*/ }


/*N*/ SwSection* SwDoc::Insert( const SwPaM& rRange, const SwSection& rNew,
/*N*/ 							const SfxItemSet* pAttr, BOOL bUpdate )
/*N*/ {
/*N*/ 	const SwNode* pPrvNd = 0;
/*N*/ 	USHORT nRegionRet = 0;
/*N*/ 	if( rRange.HasMark() &&
/*N*/ 		0 == ( nRegionRet = IsInsRegionAvailable( rRange, &pPrvNd ) ))
/*N*/ 	{
/*?*/ 		ASSERT( !this, "Selection ueber verschiedene Sections" );
/*?*/ 		return 0;
/*N*/ 	}
/*N*/
/*N*/ 	// Teste ob das gesamte Dokument versteckt werden soll,
/*N*/ 	// koennen wir zur Zeit nicht !!!!
/*N*/ 	if( rNew.IsHidden() && rRange.HasMark() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const SwPosition *pStt = rRange.Start(), *pEnd = rRange.End();
/*N*/ 	}
/*N*/
/*N*/ 	SwSectionFmt* pFmt = MakeSectionFmt( 0 );
/*N*/ 	if( pAttr )
/*N*/ 		pFmt->SetAttr( *pAttr );
/*N*/
/*N*/ 	SwSectionNode* pNewSectNode = 0;
/*N*/
/*N*/ 	SwRedlineMode eOld = GetRedlineMode();
/*N*/ 	SetRedlineMode_intern( (eOld & ~REDLINE_SHOW_MASK) | REDLINE_IGNORE );
/*N*/
/*N*/ 	if( rRange.HasMark() )
/*N*/ 	{
/*N*/ 		SwPosition *pSttPos = (SwPosition*)rRange.Start(),
/*N*/ 					*pEndPos = (SwPosition*)rRange.End();
/*N*/ 		if( pPrvNd && 3 == nRegionRet )
/*N*/ 		{
/*?*/ 			ASSERT( pPrvNd, "der SectionNode fehlt" );
/*?*/ 			SwNodeIndex aStt( pSttPos->nNode ), aEnd( pEndPos->nNode, +1 );
/*?*/ 			while( pPrvNd != aStt.GetNode().FindStartNode() )
/*?*/ 				aStt--;
/*?*/ 			while( pPrvNd != aEnd.GetNode().FindStartNode() )
/*?*/ 				aEnd++;
/*?*/
/*?*/ 			--aEnd;		// im InsertSection ist Ende inclusive
/*?*/ 			pNewSectNode = GetNodes().InsertSection( aStt, *pFmt, rNew, &aEnd );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			const SwCntntNode* pCNd;
/*N*/ 			if( pPrvNd && 1 == nRegionRet )
/*N*/ 			{
/*?*/ 				pSttPos->nNode.Assign( *pPrvNd );
/*?*/ 				pSttPos->nContent.Assign( pSttPos->nNode.GetNode().GetCntntNode(), 0 );
/*N*/ 			}
/*N*/ 			else if( pSttPos->nContent.GetIndex() )
/*?*/ 				SplitNode( *pSttPos );
/*N*/
/*N*/ 			if( pPrvNd && 2 == nRegionRet )
/*N*/ 			{
/*?*/ 				pEndPos->nNode.Assign( *pPrvNd );
/*?*/ 				pEndPos->nContent.Assign( pEndPos->nNode.GetNode().GetCntntNode(), 0 );
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pCNd = pEndPos->nNode.GetNode().GetCntntNode();
/*N*/ 				if( pCNd && pCNd->Len() != pEndPos->nContent.GetIndex() )
/*N*/ 				{
/*?*/ 					xub_StrLen nCntnt = pSttPos->nContent.GetIndex();
/*?*/ 					SplitNode( *pEndPos );
/*?*/
/*?*/ 					SwTxtNode* pTNd;
/*?*/ 					if( pEndPos->nNode.GetIndex() == pSttPos->nNode.GetIndex() )
/*?*/ 					{
/*?*/ 						pSttPos->nNode--;
/*?*/ 						pEndPos->nNode--;
/*?*/ 						pTNd = pSttPos->nNode.GetNode().GetTxtNode();
/*?*/ 						pSttPos->nContent.Assign( pTNd, nCntnt );
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						// wieder ans Ende vom vorherigen setzen
/*?*/ 						pEndPos->nNode--;
/*?*/ 						pTNd = pEndPos->nNode.GetNode().GetTxtNode();
/*?*/ 					}
/*?*/ 					if( pTNd ) nCntnt = pTNd->GetTxt().Len(); else nCntnt = 0;
/*?*/ 					pEndPos->nContent.Assign( pTNd, nCntnt );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			pNewSectNode = GetNodes().InsertSection( pSttPos->nNode, *pFmt, rNew,
/*N*/ 													&pEndPos->nNode );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		const SwPosition* pPos = rRange.GetPoint();
/*N*/ 		const SwCntntNode* pCNd = pPos->nNode.GetNode().GetCntntNode();
/*N*/ 		if( !pPos->nContent.GetIndex() )
/*N*/ 		{
/*N*/ 			pNewSectNode = GetNodes().InsertSection( pPos->nNode, *pFmt, rNew, 0, TRUE );
/*N*/ 		}
/*N*/ 		else if( pPos->nContent.GetIndex() == pCNd->Len() )
/*N*/ 		{
/*?*/ 			pNewSectNode = GetNodes().InsertSection( pPos->nNode, *pFmt, rNew, 0, FALSE );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			SplitNode( *pPos );
/*?*/ 			pNewSectNode = GetNodes().InsertSection( pPos->nNode, *pFmt, rNew, 0, TRUE );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	pNewSectNode->CheckSectionCondColl();
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 	SetRedlineMode_intern( eOld );
/*N*/
/*N*/ 	if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aPam( *pNewSectNode->EndOfSectionNode(), *pNewSectNode, 1 );
/*N*/ 	}
/*N*/
/*N*/ 	// ist eine Condition gesetzt
/*N*/ 	if( rNew.IsHidden() && rNew.GetCondition().Len() )
/*N*/ 	{
/*?*/ 		// dann berechne bis zu dieser Position
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwCalc aCalc( *this );
/*N*/ 	}
/*N*/
/*N*/ 	BOOL bUpdateFtn = FALSE;
/*N*/ 	if( GetFtnIdxs().Count() && pAttr )
/*N*/ 	{
/*?*/ 		USHORT nVal = ((SwFmtFtnAtTxtEnd&)pAttr->Get(
/*?*/ 											RES_FTN_AT_TXTEND )).GetValue();
/*?*/ 	   	if( ( FTNEND_ATTXTEND_OWNNUMSEQ == nVal ||
/*?*/ 			  FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ) ||
/*?*/ 			( FTNEND_ATTXTEND_OWNNUMSEQ == ( nVal = ((SwFmtEndAtTxtEnd&)
/*?*/ 							pAttr->Get( RES_END_AT_TXTEND )).GetValue() ) ||
/*?*/ 			  FTNEND_ATTXTEND_OWNNUMANDFMT == nVal ))
/*?*/ 			bUpdateFtn = TRUE;
/*N*/ 	}
/*N*/
/*N*/ 	if( rNew.IsLinkType() )
/*?*/ 		pNewSectNode->GetSection().CreateLink( bUpdate ? CREATE_UPDATE : CREATE_CONNECT );
/*N*/
/*N*/ 	if( bUpdateFtn )
/*?*/ 		GetFtnIdxs().UpdateFtn( SwNodeIndex( *pNewSectNode ));
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return &pNewSectNode->GetSection();
/*N*/ }

/*N*/ USHORT SwDoc::IsInsRegionAvailable( const SwPaM& rRange,
/*N*/ 								const SwNode** ppSttNd ) const
/*N*/ {
/*N*/ 	USHORT nRet = 1;
/*N*/ 	if( rRange.HasMark() )
/*N*/ 	{
/*N*/ 		// teste ob es sich um eine gueltige Selektion handelt
/*N*/ 		const SwPosition* pStt = rRange.Start(),
/*N*/ 						* pEnd = rRange.End();
/*N*/
/*N*/ 		const SwCntntNode* pCNd = pEnd->nNode.GetNode().GetCntntNode();
/*N*/ 		const SwNode* pNd = &pStt->nNode.GetNode();
/*N*/ 		const SwSectionNode* pSectNd = pNd->FindSectionNode();
/*N*/         const SwSectionNode* pEndSectNd = pCNd ? pCNd->FindSectionNode() : 0;
/*N*/ 		if( pSectNd && pEndSectNd && pSectNd != pEndSectNd )
/*N*/ 		{
/*?*/ 			// versuche eine umschliessende Section zu erzeugen
/*?*/ 			// Aber, nur wenn der Start am Sectionanfang und das Ende am
/*?*/ 			// Section Ende liegt!
/*?*/ 			nRet = 0;
/*?*/ 			if( !pStt->nContent.GetIndex() && pSectNd->GetIndex()
/*?*/ 				== pStt->nNode.GetIndex() - 1 && pEnd->nContent.GetIndex() ==
/*?*/ 				pCNd->Len() )
/*?*/ 			{
/*?*/ 				SwNodeIndex aIdx( pStt->nNode, -1 );
/*?*/ 				ULONG nCmp = pEnd->nNode.GetIndex();
/*?*/ 				const SwStartNode* pPrvNd;
/*?*/ 				const SwEndNode* pNxtNd;
/*?*/ 				while( 0 != ( pPrvNd = (pNd = &aIdx.GetNode())->GetSectionNode() ) &&
/*?*/ 					!( aIdx.GetIndex() < nCmp &&
/*?*/ 						nCmp < pPrvNd->EndOfSectionIndex() ) )
/*?*/ 				{
/*?*/ 					aIdx--;
/*?*/ 				}
/*?*/ 				if( !pPrvNd )
/*?*/ 					pPrvNd = pNd->IsStartNode() ? (SwStartNode*)pNd
/*?*/ 												: pNd->FindStartNode();
/*?*/
/*?*/ 				aIdx = pEnd->nNode.GetIndex() + 1;
/*?*/ 				nCmp = pStt->nNode.GetIndex();
/*?*/ 				while( 0 != ( pNxtNd = (pNd = &aIdx.GetNode())->GetEndNode() ) &&
/*?*/ 					pNxtNd->FindStartNode()->IsSectionNode() &&
/*?*/ 					!( pNxtNd->StartOfSectionIndex() < nCmp &&
/*?*/ 						nCmp < aIdx.GetIndex() ) )
/*?*/ 				{
/*?*/ 					aIdx++;
/*?*/ 				}
/*?*/ 				if( !pNxtNd )
/*?*/ 					pNxtNd = pNd->EndOfSectionNode();
/*?*/
/*?*/ 				if( pPrvNd && pNxtNd && pPrvNd == pNxtNd->FindStartNode() )
/*?*/ 				{
/*?*/ 					nRet = 3;
/*?*/
/*?*/ 					if( ppSttNd )
/*?*/ 						*ppSttNd = pPrvNd;
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if( !pSectNd && pEndSectNd )
/*N*/ 		{
/*?*/ 			// versuche eine umschliessende Section zu erzeugen
/*?*/ 			// Aber, nur wenn das Ende am Section Ende liegt!
/*?*/ 			nRet = 0;
/*?*/ 			if( pEnd->nContent.GetIndex() == pCNd->Len() )
/*?*/ 			{
/*?*/ 				SwNodeIndex aIdx( pEnd->nNode, 1 );
/*?*/ 				if( aIdx.GetNode().IsEndNode() &&
/*?*/ 						0 != aIdx.GetNode().FindSectionNode() )
/*?*/ 				{
/*?*/ 					do {
/*?*/ 						aIdx++;
/*?*/ 					} while( aIdx.GetNode().IsEndNode() &&
/*?*/ 								0 != aIdx.GetNode().FindSectionNode() );
/*?*/ //					if( !aIdx.GetNode().IsEndNode() )
/*?*/ 					{
/*?*/ 						nRet = 2;
/*?*/ 						if( ppSttNd )
/*?*/ 						{
/*?*/ 							aIdx--;
/*?*/ 							*ppSttNd = &aIdx.GetNode();
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if( pSectNd && !pEndSectNd )
/*N*/ 		{
/*?*/ 			// versuche eine umschliessende Section zu erzeugen
/*?*/ 			// Aber, nur wenn der Start am Section Anfang liegt!
/*?*/ 			nRet = 0;
/*?*/ 			if( !pStt->nContent.GetIndex() )
/*?*/ 			{
/*?*/ 				SwNodeIndex aIdx( pStt->nNode, -1 );
/*?*/ 				if( aIdx.GetNode().IsSectionNode() )
/*?*/ 				{
/*?*/ 					do {
/*?*/ 						aIdx--;
/*?*/ 					} while( aIdx.GetNode().IsSectionNode() );
/*?*/ 					if( !aIdx.GetNode().IsSectionNode() )
/*?*/ 					{
/*?*/ 						nRet = 1;
/*?*/ 						if( ppSttNd )
/*?*/ 						{
/*?*/ 							aIdx++;
/*?*/ 							*ppSttNd = &aIdx.GetNode();
/*?*/ 						}
/*?*/ 					}
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nRet;
/*N*/ }

/*N*/ SwSection* SwDoc::GetCurrSection( const SwPosition& rPos ) const
/*N*/ {
/*N*/ 	const SwSectionNode* pSectNd = rPos.nNode.GetNode().FindSectionNode();
/*N*/ 	if( pSectNd )
/*N*/ 		return (SwSection*)&pSectNd->GetSection();
/*N*/ 	return 0;
/*N*/ }

/*N*/ SwSectionFmt* SwDoc::MakeSectionFmt( SwSectionFmt *pDerivedFrom )
/*N*/ {
/*N*/ 	if( !pDerivedFrom )
/*N*/ 		pDerivedFrom = (SwSectionFmt*)pDfltFrmFmt;
/*N*/ 	SwSectionFmt* pNew = new SwSectionFmt( pDerivedFrom, this );
/*N*/ 	pSectionFmtTbl->Insert( pNew, pSectionFmtTbl->Count() );
/*N*/ 	return pNew;
/*N*/ }

/*N*/ void SwDoc::DelSectionFmt( SwSectionFmt *pFmt, BOOL bDelNodes )
/*N*/ {
/*N*/ 	USHORT nPos = pSectionFmtTbl->GetPos( pFmt );
/*N*/ 	if( USHRT_MAX != nPos )
/*N*/ 	{
/*N*/ 		const SwNodeIndex* pIdx = pFmt->GetCntnt( FALSE ).GetCntntIdx();
/*N*/ 		const SfxPoolItem* pFtnEndAtTxtEnd;
/*N*/ 		if( SFX_ITEM_SET != pFmt->GetItemState(
/*N*/ 							RES_FTN_AT_TXTEND, TRUE, &pFtnEndAtTxtEnd ) ||
/*N*/ 			SFX_ITEM_SET != pFmt->GetItemState(
/*N*/ 							RES_END_AT_TXTEND, TRUE, &pFtnEndAtTxtEnd ))
/*N*/ 			pFtnEndAtTxtEnd = 0;
/*N*/
/*N*/ 		const SwSectionNode* pSectNd;
/*N*/
/*N*/       if( bDelNodes && pIdx && &GetNodes() == &pIdx->GetNodes() &&
/*N*/ 				0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
/*N*/ 		{
/*?*/ 			SwNodeIndex aUpdIdx( *pIdx );
/*?*/ 			DeleteSection( (SwNode*)pSectNd );
/*?*/ 			if( pFtnEndAtTxtEnd )
/*?*/ 				GetFtnIdxs().UpdateFtn( aUpdIdx );
/*?*/ 			SetModified();
/*?*/ 			return ;
/*N*/ 		}
/*N*/
/*N*/ 		{
/*N*/ 			SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFmt );
/*N*/ 			pFmt->Modify( &aMsgHint, &aMsgHint );
/*N*/ 		}
/*N*/
/*N*/ 		// ACHTUNG: erst aus dem Array entfernen und dann loeschen.
/*N*/ 		//			Der Section-DTOR versucht selbst noch sein Format
/*N*/ 		//			zu loeschen!
/*N*/ 		pSectionFmtTbl->Remove( nPos );
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		ULONG nCnt = 0, nSttNd = 0;
/*N*/ 		if( pIdx && &GetNodes() == &pIdx->GetNodes() &&
/*N*/ 			0 != (pSectNd = pIdx->GetNode().GetSectionNode() ))
/*N*/ 		{
/*?*/ 			nSttNd = pSectNd->GetIndex();
/*?*/ 			nCnt = pSectNd->EndOfSectionIndex() - nSttNd - 1;
/*N*/ 		}
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 		delete pFmt;
/*N*/
/*N*/ 		if( nSttNd && pFtnEndAtTxtEnd )
/*N*/ 		{
/*?*/ 			SwNodeIndex aUpdIdx( GetNodes(), nSttNd );
/*?*/ 			GetFtnIdxs().UpdateFtn( aUpdIdx );
/*N*/ 		}
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 		SwCntntNode* pCNd;
/*N*/ 		for( ; nCnt--; ++nSttNd )
/*N*/ 			if( 0 != (pCNd = GetNodes()[ nSttNd ]->GetCntntNode() ) &&
/*?*/ 				RES_CONDTXTFMTCOLL == pCNd->GetFmtColl()->Which() )
/*?*/ 				pCNd->ChkCondColl();
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	}
/*N*/ 	SetModified();
/*N*/ }

/*N*/ void SwDoc::ChgSection( USHORT nPos, const SwSection& rSect,
/*N*/ 						const SfxItemSet* pAttr,
/*N*/ 						sal_Bool bPreventLinkUpdate )
/*N*/ {
/*N*/ 	SwSectionFmt* pFmt = (*pSectionFmtTbl)[ nPos ];
/*N*/ 	SwSection* pSection = pFmt->GetSection();
/*N*/     /// OD 04.10.2002 #102894#
/*N*/     /// remember hidden condition flag of SwSection before changes
/*N*/     bool bOldCondHidden = pSection->IsCondHidden() ? true : false;
/*N*/
/*N*/ 	if( *pSection == rSect )
/*N*/ 	{
/*N*/ 		// die Attribute ueberpruefen
/*N*/ 		BOOL bOnlyAttrChg = FALSE;
/*N*/ 		if( pAttr && pAttr->Count() )
/*N*/ 		{
/*N*/ 			SfxItemIter aIter( *pAttr );
/*N*/ 			USHORT nWhich = aIter.GetCurItem()->Which();
/*N*/ 			while( TRUE )
/*N*/ 			{
/*N*/ 				if( pFmt->GetAttr( nWhich ) != *aIter.GetCurItem() )
/*N*/ 				{
/*N*/ 					bOnlyAttrChg = TRUE;
/*N*/ 					break;
/*N*/ 				}
/*N*/
/*?*/ 				if( aIter.IsAtEnd() )
/*?*/ 					break;
/*?*/ 				nWhich = aIter.NextItem()->Which();
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if( bOnlyAttrChg )
/*N*/ 		{
/*N*/ 			pFmt->SetAttr( *pAttr );
/*N*/ 			SetModified();
/*N*/ 		}
/*N*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	// Teste ob eine gesamte Content-Section (Dokument/TabellenBox/Fly)
/*N*/ 	// versteckt werden soll, koennen wir zur Zeit nicht !!!!
/*N*/ 	const SwNodeIndex* pIdx = 0;
/*N*/ 	{
/*N*/ 		const SwSectionNode* pSectNd;
/*N*/ 		if( rSect.IsHidden() && 0 != (pIdx = pFmt->GetCntnt().GetCntntIdx() )
/*N*/ 			&& 0 != (pSectNd = pIdx->GetNode().GetSectionNode() ) )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ::lcl_CheckEmptyLayFrm( GetNodes(), (SwSection&)rSect,
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/
/*N*/ 	// #56167# Der LinkFileName koennte auch nur aus Separatoren bestehen
/*N*/     String sCompareString = ::binfilter::cTokenSeperator;
/*N*/     sCompareString += ::binfilter::cTokenSeperator;
/*N*/ 	BOOL bUpdate = ( !pSection->IsLinkType() && rSect.IsLinkType() ) ||
/*N*/ 						( rSect.GetLinkFileName().Len() &&
/*N*/ 							rSect.GetLinkFileName() != sCompareString &&
/*N*/ 							rSect.GetLinkFileName() !=
/*N*/ 							pSection->GetLinkFileName());
/*N*/
/*N*/ 	String sSectName( rSect.GetName() );
/*N*/ 	if( sSectName != pSection->GetName() )
/*?*/ 		GetUniqueSectionName( &sSectName );
/*N*/ 	else
/*N*/ 		sSectName.Erase();
/*N*/
/*N*/     /// OD 04.10.2002 #102894# - NOTE
/*N*/     /// In SwSection::operator=(..) class member bCondHiddenFlag is always set to TRUE.
/*N*/     /// IMHO this have to be changed, but I can't estimate the consequences:
/*N*/     /// Either it is set to TRUE using corresponding method <SwSection.SetCondHidden(..)>,
/*N*/     /// or it is set to the value of SwSection which is assigned to it.
/*N*/     /// Discussion with AMA results that the adjustment to the assignment operator
/*N*/     /// could be very risky -> see notes in bug #102894#.
/*N*/ 	*pSection = rSect;
/*N*/
/*N*/ 	if( pAttr )
/*?*/ 		pSection->GetFmt()->SetAttr( *pAttr );
/*N*/
/*N*/ 	if( sSectName.Len() )
/*?*/ 		pSection->SetName( sSectName );
/*N*/
/*N*/ 	// ist eine Condition gesetzt
/*N*/ 	if( pSection->IsHidden() && pSection->GetCondition().Len() )
/*N*/ 	{
/*?*/ 		// dann berechne bis zu dieser Position
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwCalc aCalc( *this );
/*N*/ 	}
/*N*/
/*N*/ 	if( bUpdate )
/*N*/ 		pSection->CreateLink( bPreventLinkUpdate ? CREATE_CONNECT : CREATE_UPDATE );
/*N*/ 	else if( !pSection->IsLinkType() && pSection->IsConnected() )
/*N*/ 	{
/*?*/ 		pSection->Disconnect();
/*?*/ 		GetLinkManager().Remove( &pSection->GetBaseLink() );
/*N*/ 	}
/*N*/
/*N*/ 	SetModified();
/*N*/ }


/* -----------------19.02.99 09:31-------------------
 * LockFrms wurde im InsertSection genutzt, um zu verhindern, dass
 * SectionFrms durch das DelFrms zerstoert werden. Dies ist durch
 * den Destroy-Listen-Mechanismus ueberfluessig geworden.
 * Falls diese Methode doch noch einmal reanimiert wird, bietet es
 * sich vielleicht an, beim Entlocken die SectionFrms auf Inhalt zu
 * pruefen und dann ggf. zur Zerstoerung anzumelden.
 * --------------------------------------------------*/

// und dann waren da noch die Fussnoten:
/*N*/ void lcl_DeleteFtn( SwSectionNode *pNd, ULONG nStt, ULONG nEnd )
/*N*/ {
/*N*/ 	SwFtnIdxs& rFtnArr = pNd->GetDoc()->GetFtnIdxs();
/*N*/ 	if( rFtnArr.Count() )
/*N*/ 	{
/*?*/       USHORT nPos;
 /*?*/      rFtnArr.SeekEntry( SwNodeIndex( *pNd ), &nPos );
 /*?*/      SwTxtFtn* pSrch;
 /*?*/
 /*?*/      // loesche erstmal alle, die dahinter stehen
 /*?*/      while( nPos < rFtnArr.Count() &&
 /*?*/          _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) <= nEnd )
 /*?*/      {
 /*?*/          // Werden die Nodes nicht geloescht mussen sie bei den Seiten
 /*?*/          // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
 /*?*/          // stehen (Undo loescht sie nicht!)
 /*?*/          pSrch->DelFrms();
 /*?*/          ++nPos;
 /*?*/      }
 /*?*/
 /*?*/      while( nPos-- &&
 /*?*/          _SwTxtFtn_GetIndex( (pSrch = rFtnArr[ nPos ]) ) >= nStt )
 /*?*/      {
 /*?*/          // Werden die Nodes nicht geloescht mussen sie bei den Seiten
 /*?*/          // abmeldet (Frms loeschen) werden, denn sonst bleiben sie
 /*?*/          // stehen (Undo loescht sie nicht!)
 /*?*/          pSrch->DelFrms();
 /*?*/      }
/*N*/ 	}
/*N*/ }

/*N*/ inline BOOL lcl_IsTOXSection( const SwSection& rSection )
/*N*/ {
/*N*/ 	return TOX_CONTENT_SECTION == rSection.GetType() ||
/*N*/ 			TOX_HEADER_SECTION == rSection.GetType();
/*N*/ }

/*N*/ SwSectionNode* SwNodes::InsertSection( const SwNodeIndex& rNdIdx,
/*N*/ 								SwSectionFmt& rSectionFmt,
/*N*/ 								const SwSection& rSection,
/*N*/ 								const SwNodeIndex* pEnde,
/*N*/ 								BOOL bInsAtStart, BOOL bCreateFrms )
/*N*/ {
/*N*/ 	SwNodeIndex aInsPos( rNdIdx );
/*N*/ 	if( !pEnde )		// kein Bereich also neue Section davor/hinter anlegen
/*N*/ 	{
/*N*/ 		if( bInsAtStart )
/*N*/ 		{
/*?*/ 			if( !lcl_IsTOXSection( rSection ))
/*?*/ 			{
/*?*/ 				do {
/*?*/ 					aInsPos--;
/*?*/ 				} while( aInsPos.GetNode().IsSectionNode() );
/*?*/ 				aInsPos++;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*?*/ 			SwNode* pNd;
/*?*/ 			aInsPos++;
/*?*/ 			if( !lcl_IsTOXSection( rSection ))
/*?*/ 				while( aInsPos.GetIndex() < Count() - 1 &&
/*?*/ 						( pNd = &aInsPos.GetNode())->IsEndNode() &&
/*?*/ 						pNd->FindStartNode()->IsSectionNode())
/*?*/ 					aInsPos++;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SwSectionNode* pSectNd = new SwSectionNode( aInsPos, rSectionFmt );
/*N*/ 	if( pEnde )
/*N*/ 	{
/*N*/ 		// Sonderfall fuer die Reader/Writer
/*N*/ 		if( &pEnde->GetNode() != &GetEndOfContent() )
/*N*/ 			aInsPos = pEnde->GetIndex()+1;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwTxtNode* pCpyTNd = rNdIdx.GetNode().GetTxtNode();
/*N*/ 		if( pCpyTNd )
/*N*/ 		{
/*N*/ 			SwTxtNode* pTNd = new SwTxtNode( aInsPos, pCpyTNd->GetTxtColl() );
/*N*/ 			if( pCpyTNd->GetpSwAttrSet() )
/*N*/ 			{
/*?*/ 				// Task 70955 - move PageDesc/Break to the first Node of the
/*?*/ 				//				section
/*?*/ 				const SfxItemSet& rSet = *pCpyTNd->GetpSwAttrSet();
/*?*/ 				if( SFX_ITEM_SET == rSet.GetItemState( RES_BREAK ) ||
/*?*/ 					SFX_ITEM_SET == rSet.GetItemState( RES_PAGEDESC ))
/*?*/ 				{
/*?*/ 					SfxItemSet aSet( rSet );
/*?*/ 					if( bInsAtStart )
/*?*/ 						pCpyTNd->ResetAttr( RES_PAGEDESC, RES_BREAK );
/*?*/ 					else
/*?*/ 					{
/*?*/ 						aSet.ClearItem( RES_PAGEDESC );
/*?*/ 						aSet.ClearItem( RES_BREAK );
/*?*/ 					}
/*?*/ 					pTNd->SwCntntNode::SetAttr( aSet );
/*?*/ 				}
/*?*/ 				else
/*?*/ 					pTNd->SwCntntNode::SetAttr( rSet );
/*N*/ 			}
/*N*/ 			// den Frame anlegen nicht vergessen !!
/*N*/ 			pCpyTNd->MakeFrms( *pTNd );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			new SwTxtNode( aInsPos, (SwTxtFmtColl*)GetDoc()->GetDfltTxtFmtColl() );
/*N*/ 	}
/*N*/ 	SwEndNode* pEndNd = new SwEndNode( aInsPos, *pSectNd );
/*N*/
/*N*/ 	pSectNd->GetSection() = rSection;
/*N*/ 	SwSectionFmt* pSectFmt = pSectNd->GetSection().GetFmt();
/*N*/
/*N*/ 	// Hier bietet sich als Optimierung an, vorhandene Frames nicht zu
/*N*/ 	// zerstoeren und wieder neu anzulegen, sondern nur umzuhaengen.
/*N*/ 	BOOL bInsFrm = bCreateFrms && !pSectNd->GetSection().IsHidden() &&
/*N*/ 				   GetDoc()->GetRootFrm();
/*N*/ 	SwNode2Layout *pNode2Layout = NULL;
/*N*/ 	if( bInsFrm )
/*N*/ 	{
/*N*/ 		SwNodeIndex aTmp( *pSectNd );
/*N*/ 		if( !pSectNd->GetNodes().FindPrvNxtFrmNode( aTmp, pSectNd->EndOfSectionNode() ) )
/*N*/ 			// dann sammel mal alle Uppers ein
/*?*/ 			pNode2Layout = new SwNode2Layout( *pSectNd );
/*N*/ 	}
/*N*/
/*N*/ 	// jetzt noch bei allen im Bereich den richtigen StartNode setzen
/*N*/ 	ULONG nEnde = pSectNd->EndOfSectionIndex();
/*N*/ 	ULONG nStart = pSectNd->GetIndex()+1;
/*N*/ 	ULONG nSkipIdx = ULONG_MAX;
/*N*/ 	for( ULONG n = nStart; n < nEnde; ++n )
/*N*/ 	{
/*N*/ 		SwNode* pNd = (*this)[n];
/*N*/
/*N*/ 		//JP 30.04.99: Bug 65644 - alle in der NodeSection liegenden
/*N*/ 		//				Sections unter die neue haengen
/*N*/ 		if( ULONG_MAX == nSkipIdx )
/*N*/ 			pNd->pStartOfSection = pSectNd;
/*N*/ 		else if( n >= nSkipIdx )
/*?*/ 			nSkipIdx = ULONG_MAX;
/*N*/
/*N*/ 		if( pNd->IsStartNode() )
/*N*/ 		{
/*?*/ 			// die Verschachtelung der Formate herstellen!
/*?*/ 			if( pNd->IsSectionNode() )
/*?*/ 			{
/*?*/ 				((SwSectionNode*)pNd)->GetSection().GetFmt()->
/*?*/ 									SetDerivedFrom( pSectFmt );
/*?*/ 				((SwSectionNode*)pNd)->DelFrms();
/*?*/ 				n = pNd->EndOfSectionIndex();
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				if( pNd->IsTableNode() )
/*?*/ 					((SwTableNode*)pNd)->DelFrms();
/*?*/
/*?*/ 				if( ULONG_MAX == nSkipIdx )
/*?*/ 					nSkipIdx = pNd->EndOfSectionIndex();
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else if( pNd->IsCntntNode() )
/*N*/ 			((SwCntntNode*)pNd)->DelFrms();
/*N*/ 	}
/*N*/
/*N*/ 	lcl_DeleteFtn( pSectNd, nStart, nEnde );
/*N*/
/*N*/ 	if( bInsFrm )
/*N*/ 	{
/*N*/ 		if( pNode2Layout )
/*N*/ 		{
/*?*/ 			ULONG nIdx = pSectNd->GetIndex();
/*?*/ 			pNode2Layout->RestoreUpperFrms( pSectNd->GetNodes(), nIdx, nIdx + 1 );
/*?*/ 			delete pNode2Layout;
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pSectNd->MakeFrms( &aInsPos );
/*N*/ 	}
/*N*/
/*N*/ 	return pSectNd;
/*N*/ }

/*N*/ SwSectionNode* SwNode::FindSectionNode()
/*N*/ {
/*N*/ 	if( IsSectionNode() )
/*N*/ 		return GetSectionNode();
/*N*/ 	SwStartNode* pTmp = pStartOfSection;
/*N*/ 	while( !pTmp->IsSectionNode() && pTmp->GetIndex() )
/*N*/ #if defined( ALPHA ) && defined( UNX )
/*?*/ 		pTmp = ((SwNode*)pTmp)->pStartOfSection;
/*N*/ #else
/*N*/ 		pTmp = pTmp->pStartOfSection;
/*N*/ #endif
/*N*/ 	return pTmp->GetSectionNode();
/*N*/ }


//---------
// SwSectionNode
//---------

/*N*/ SwSectionNode::SwSectionNode( const SwNodeIndex& rIdx, SwSectionFmt& rFmt )
/*N*/ 	: SwStartNode( rIdx, ND_SECTIONNODE )
/*N*/ {
/*N*/ 	SwSectionNode* pParent = FindStartNode()->FindSectionNode();
/*N*/ 	if( pParent )
/*N*/ 	{
/*N*/ 		// das Format beim richtigen Parent anmelden.
/*N*/ 		rFmt.SetDerivedFrom( pParent->GetSection().GetFmt() );
/*N*/ 	}
/*N*/ 	pSection = new SwSection( CONTENT_SECTION, rFmt.GetName(), &rFmt );
/*N*/
/*N*/ 	// jetzt noch die Verbindung von Format zum Node setzen
/*N*/ 	// Modify unterdruecken, interresiert keinen
/*N*/ 	rFmt.LockModify();
/*N*/ 	rFmt.SetAttr( SwFmtCntnt( this ) );
/*N*/ 	rFmt.UnlockModify();
/*N*/ }

//Hier werden ueberfluessige SectionFrms entfernt

/*N*/ SwSectionNode::~SwSectionNode()
/*N*/ {
/*N*/ 	{
/*N*/ 		SwClientIter aIter( *(pSection->GetFmt()) );
/*N*/ 		SwClient *pLast = aIter.GoStart();
/*N*/ 		while ( pLast )
/*N*/ 		{
/*N*/ 			if ( pLast->IsA( TYPE(SwFrm) ) )
/*N*/ 			{
/*?*/ 				SwSectionFrm *pSectFrm = (SwSectionFrm*)pLast;
/*?*/ 				SwSectionFrm::MoveCntntAndDelete( pSectFrm, TRUE );
/*?*/ 				pLast = aIter.GoStart();
/*N*/ 			}
/*N*/ 			else
/*N*/ 				pLast = aIter++;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SwDoc* pDoc = GetDoc();
/*N*/
/*N*/ 	SwSectionFmt* pFmt = pSection->GetFmt();
/*N*/ 	if( pFmt )
/*N*/ 	{
/*N*/ 		// das Attribut entfernen, weil die Section ihr Format loescht
/*N*/ 		// und falls das Cntnt-Attribut vorhanden ist, die Section aufhebt.
/*N*/ 		pFmt->LockModify();
/*N*/ 		pFmt->ResetAttr( RES_CNTNT );
/*N*/ 		pFmt->UnlockModify();
/*N*/ 	}
/*N*/
/*N*/ 	DELETEZ( pSection );
/*N*/ }

// setze ein neues SectionObject. Erstmal nur gedacht fuer die
// neuen VerzeichnisSections. Der geht ueber in den Besitz des Nodes!
/*N*/ void SwSectionNode::SetNewSection( SwSection* pNewSection )
/*N*/ {
/*N*/ 	ASSERT( pNewSection, "ohne Pointer geht hier nichts" );
/*N*/ 	if( pNewSection )
/*N*/ 	{
/*N*/ 		SwNode2Layout aN2L( *this );
/*N*/
/*N*/ 		// einige Flags sollten ueber nommen werden!
/*N*/ 		pNewSection->bProtectFlag = pSection->bProtectFlag;
/*N*/ 		pNewSection->bHiddenFlag = pSection->bHiddenFlag;
/*N*/ 		pNewSection->bHidden = pSection->bHidden;
/*N*/ 		pNewSection->bCondHiddenFlag = pSection->bCondHiddenFlag;
/*N*/
/*N*/ 		// The section frame contains a pointer to the section. That for,
/*N*/ 		// the frame must be destroyed before deleting the section.
/*N*/ 		DelFrms();
/*N*/
/*N*/ 		delete pSection;
/*N*/ 		pSection = pNewSection;
/*N*/
/*N*/ 		ULONG nIdx = GetIndex();
/*N*/ 		aN2L.RestoreUpperFrms( GetNodes(), nIdx, nIdx + 1 );
/*N*/ 	}
/*N*/ }

/*N*/ SwFrm *SwSectionNode::MakeFrm()
/*N*/ {
/*N*/ 	pSection->bHiddenFlag = FALSE;
/*N*/ 	return new SwSectionFrm( *pSection );
/*N*/ }

//Fuer jedes vorkommen im Layout einen SectionFrm anlegen und vor den
//entsprechenden CntntFrm pasten.

/*N*/ void SwSectionNode::MakeFrms( SwNodeIndex* pIdxBehind, SwNodeIndex* pEndIdx )
/*N*/ {
/*N*/ 	ASSERT( pIdxBehind, "kein Index" );
/*N*/ 	SwNodes& rNds = GetNodes();
/*N*/ 	SwDoc* pDoc = rNds.GetDoc();
/*N*/
/*N*/ 	*pIdxBehind = *this;
/*N*/
/*N*/ 	pSection->bHiddenFlag = TRUE;
/*N*/
/*N*/ 	if( rNds.IsDocNodes() )
/*N*/ 	{
/*N*/ 		SwNodeIndex *pEnd = pEndIdx ? pEndIdx :
/*N*/ 							new SwNodeIndex( *EndOfSectionNode(), 1 );
/*N*/ 		::binfilter::MakeFrms( pDoc, *pIdxBehind, *pEnd );
/*N*/ 		if( !pEndIdx )
/*N*/ 			delete pEnd;
/*N*/ 	}
/*N*/
/*N*/ }

/*N*/ void SwSectionNode::DelFrms()
/*N*/ {
/*N*/ 	ULONG nStt = GetIndex()+1, nEnd = EndOfSectionIndex();
/*N*/ 	if( nStt >= nEnd )
/*N*/ 	{
/*?*/ 		// unser Flag muessen wir noch aktualisieren
/*?*/ 		// pSection->bHiddenFlag = TRUE;
/*?*/ 		return ;
/*N*/ 	}
/*N*/
/*N*/ 	SwNodes& rNds = GetNodes();
/*N*/ 	pSection->GetFmt()->DelFrms();
/*N*/
/*N*/ 	// unser Flag muessen wir noch aktualisieren
/*N*/ 	pSection->bHiddenFlag = TRUE;
/*N*/
/*N*/ 	// Bug 30582: falls der Bereich in Fly oder TabellenBox ist, dann
/*N*/ 	//				kann er nur "gehiddet" werden, wenn weiterer Content
/*N*/ 	//				vorhanden ist, der "Frames" haelt. Sonst hat der
/*N*/ 	//				Fly/TblBox-Frame keinen Lower !!!
/*N*/ 	{
/*N*/ 		SwNodeIndex aIdx( *this );
/*N*/ 		if( !rNds.GoPrevSection( &aIdx, TRUE, FALSE ) ||
/*N*/ 			!CheckNodesRange( *this, aIdx, TRUE ) ||
/*N*/ 			!lcl_IsInSameTblBox( rNds, *this, aIdx ))
/*N*/ 		{
/*N*/ 			aIdx = *EndOfSectionNode();
/*N*/ 			if( !rNds.GoNextSection( &aIdx, TRUE, FALSE ) ||
/*N*/ 				!CheckNodesRange( *EndOfSectionNode(), aIdx, TRUE ) ||
/*N*/ 				!lcl_IsInSameTblBox( rNds, *EndOfSectionNode(), aIdx ))
/*?*/ 				pSection->bHiddenFlag = FALSE;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


/*N*/ String SwDoc::GetUniqueSectionName( const String* pChkStr ) const
/*N*/ {
/*N*/ 	ResId aId( STR_REGION_DEFNAME, *pSwResMgr );
/*N*/ 	String aName( aId );
/*N*/ 	xub_StrLen nNmLen = aName.Len();
/*N*/
/*N*/ 	USHORT nNum, nTmp, nFlagSize = ( pSectionFmtTbl->Count() / 8 ) +2;
/*N*/ 	BYTE* pSetFlags = new BYTE[ nFlagSize ];
/*N*/ 	memset( pSetFlags, 0, nFlagSize );
/*N*/
/*N*/ 	const SwSectionNode* pSectNd;
        USHORT n=0;
/*N*/ 	for( n = 0; n < pSectionFmtTbl->Count(); ++n )
/*N*/ 		if( 0 != ( pSectNd = (*pSectionFmtTbl)[ n ]->GetSectionNode( FALSE ) ))
/*N*/ 		{
/*N*/ 			const String& rNm = pSectNd->GetSection().GetName();
/*N*/ 			if( rNm.Match( aName ) == nNmLen )
/*N*/ 			{
/*?*/ 				// Nummer bestimmen und das Flag setzen
/*?*/ 				nNum = rNm.Copy( nNmLen ).ToInt32();
/*?*/ 				if( nNum-- && nNum < pSectionFmtTbl->Count() )
/*?*/ 					pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
/*N*/ 			}
/*N*/ 			if( pChkStr && pChkStr->Equals( rNm ) )
/*?*/ 				pChkStr = 0;
/*N*/ 		}
/*N*/
/*N*/ 	if( !pChkStr )
/*N*/ 	{
/*?*/ 		// alle Nummern entsprechend geflag, also bestimme die richtige Nummer
/*?*/ 		nNum = pSectionFmtTbl->Count();
/*?*/ 		for( n = 0; n < nFlagSize; ++n )
/*?*/ 			if( 0xff != ( nTmp = pSetFlags[ n ] ))
/*?*/ 			{
/*?*/ 				// also die Nummer bestimmen
/*?*/ 				nNum = n * 8;
/*?*/ 				while( nTmp & 1 )
/*?*/ 					++nNum, nTmp >>= 1;
/*?*/ 				break;
/*?*/ 			}
/*?*/
/*N*/ 	}
/*N*/ 	delete [] pSetFlags;
/*N*/ 	if( pChkStr )
/*N*/ 		return *pChkStr;
/*?*/ 	return aName += String::CreateFromInt32( ++nNum );
/*N*/ }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
