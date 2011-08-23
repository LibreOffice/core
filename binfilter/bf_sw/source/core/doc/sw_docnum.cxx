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


#include <ftninfo.hxx>
#include <ftnidx.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <docary.hxx>
#include <mvsave.hxx>
#include <txtfrm.hxx>
#include <hints.hxx>
#include <redline.hxx>
#include <comcore.hrc>
#include <bf_svx/adjitem.hxx>
namespace binfilter {

/*N*/ inline BYTE GetUpperLvlChg( BYTE nCurLvl, BYTE nLevel, USHORT nMask )
/*N*/ {
/*N*/ 	if( 1 < nLevel )
/*N*/ 	{
/*N*/ 		if( nCurLvl + 1 >= nLevel )
/*N*/ 			nCurLvl -= nLevel - 1;
/*N*/ 		else
/*N*/ 			nCurLvl = 0;
/*N*/ 	}
/*N*/ 	return (nMask - 1) & ~(( 1 << nCurLvl ) - 1);
/*N*/ }

/*N*/ void SwDoc::SetOutlineNumRule( const SwNumRule& rRule )
/*N*/ {
/*N*/ 	USHORT nChkLevel = 0, nChgFmtLevel = 0;
/*N*/ 	if( pOutlineRule )
/*N*/ 	{
/*N*/ 		USHORT nMask = 1;
/*N*/ 		for( BYTE n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
/*N*/ 		{
/*N*/ 			const SwNumFmt& rOld = pOutlineRule->Get( n ),
/*N*/ 						  & rNew = rRule.Get( n );
/*N*/ 			if( rOld != rNew )
/*N*/ 			{
/*N*/ 				nChgFmtLevel |= nMask;
/*N*/ 				if( rOld.GetAbsLSpace() != rNew.GetAbsLSpace() ||
/*N*/ 					rOld.GetFirstLineOffset() != rNew.GetFirstLineOffset() )
/*N*/ 					nChkLevel |= nMask;
/*N*/ 			}
/*N*/ 			else if( SVX_NUM_NUMBER_NONE > rNew.GetNumberingType() && 1 < rNew.GetIncludeUpperLevels() &&
/*N*/ 					0 != (nChgFmtLevel & GetUpperLvlChg( n,
/*N*/ 											rNew.GetIncludeUpperLevels(), nMask )) )
/*?*/ 				nChgFmtLevel |= nMask;
/*N*/ 		}
/*N*/ 		(*pOutlineRule) = rRule;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*?*/ 		nChgFmtLevel = nChkLevel = 0xffff;
/*?*/ 		pOutlineRule = new SwNumRule( rRule );
/*N*/ 	}
/*N*/ 	pOutlineRule->SetRuleType( OUTLINE_RULE );
/*N*/ 	pOutlineRule->SetName( String::CreateFromAscii(
/*N*/ 										SwNumRule::GetOutlineRuleName() ));

    // teste ob die evt. gesetzen CharFormate in diesem Document
    // definiert sind
/*N*/ 	pOutlineRule->CheckCharFmts( this );

    // losche aus dem Array alle Nodes, die ohne Outline Nummerierung sind
/*N*/ 	SwOutlineNodes& rArr = (SwOutlineNodes&)GetNodes().GetOutLineNds();
/*N*/ 	{
/*N*/ 		SwNodeNum aNoNum( NO_NUMBERING );
/*N*/ 		for( USHORT n = 0; n < rArr.Count(); ++n )
/*N*/ 		{
/*N*/ 			SwTxtNode* pTxtNd = rArr[n]->GetTxtNode();
/*N*/ 			if( pTxtNd && NO_NUMBERING == pTxtNd->GetTxtColl()->GetOutlineLevel() )
/*N*/ 			{
/*?*/ 				pTxtNd->UpdateOutlineNum( aNoNum );
/*?*/ 				rArr.Remove( n-- );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
    // suche alle Nodes, die neu aufgenommen werden muessen !!
    // (eigentlich koennte das auch per Modify am die Nodes propagiert
    // werden !! )
/*N*/ 	ULONG nStt = GetNodes().GetEndOfContent().StartOfSectionIndex();
        USHORT n=0;
/*N*/ 	for( n = 0; n < pTxtFmtCollTbl->Count(); ++n )
/*N*/ 	{
/*N*/ 		SwTxtFmtColl* pColl = (*pTxtFmtCollTbl)[ n ];
/*N*/ 		BYTE nLevel = pColl->GetOutlineLevel();
/*N*/ 		if( NO_NUMBERING != nLevel )
/*N*/ 		{
/*N*/ #ifndef NUM_RELSPACE
/*N*/ 			// JP 08.07.98: Einzuege aus der Outline uebernehmen.
/*N*/ 			// 				??Aber nur wenn sie veraendert wurden??
/*N*/ 			if( ( nLevel = GetRealLevel( nLevel )) < MAXLEVEL
/*N*/ 				/*&& 0 != (nChkLevel & (1 << nLevel ))*/ )
/*N*/ 			{
/*N*/ 				SvxLRSpaceItem aLR( (SvxLRSpaceItem&)pColl->GetAttr( RES_LR_SPACE ) );
/*N*/ 				const SwNumFmt& rNFmt = pOutlineRule->Get( nLevel );
/*N*/
/*N*/ 				// ohne Nummer immer ohne FirstLineOffset!!!!
/*N*/ 				short nFOfst;
/*N*/ 				if( pColl->GetOutlineLevel() & NO_NUMLEVEL )
/*N*/ 					nFOfst = 0;
/*N*/ 				else
/*N*/ 					nFOfst = rNFmt.GetFirstLineOffset();
/*N*/
/*N*/ 				if( aLR.GetTxtLeft() != rNFmt.GetAbsLSpace() ||
/*N*/ 					aLR.GetTxtFirstLineOfst() != nFOfst )
/*N*/ 				{
/*N*/ 					aLR.SetTxtFirstLineOfstValue( nFOfst );
/*N*/ 					aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );
/*N*/
/*N*/ 					pColl->SetAttr( aLR );
/*N*/ 				}
/*N*/ 			}
/*N*/ #endif
/*N*/ 			SwClientIter aIter( *pColl );
/*N*/ 			for( SwTxtNode* pNd = (SwTxtNode*)aIter.First( TYPE( SwTxtNode ));
/*N*/ 					pNd; pNd = (SwTxtNode*)aIter.Next() )
/*N*/ 				if( pNd->GetNodes().IsDocNodes() && nStt < pNd->GetIndex() )
/*N*/ 					rArr.Insert( pNd );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	for( n = 0; n < rArr.Count(); ++n )
/*N*/ 	{
/*N*/ 		SwTxtNode* pNd = rArr[ n ]->GetTxtNode();
/*N*/ 		ASSERT( pNd, "was ist das fuer ein Node?" );
/*N*/ 		if( ( 1 << (pNd->GetTxtColl()->GetOutlineLevel() & ~NO_NUMLEVEL )
/*N*/ 			& nChgFmtLevel ))
/*?*/ 			pNd->NumRuleChgd();
/*N*/ 	}
/*N*/ 	GetNodes().UpdateOutlineNodes();        // update der Nummern
/*N*/
/*N*/ 	// gibt es Fussnoten && gilt Kapitelweises Nummerieren, dann updaten
/*N*/ 	if( GetFtnIdxs().Count() && FTNNUM_CHAPTER == GetFtnInfo().eNum )
/*?*/ 		GetFtnIdxs().UpdateAllFtn();
/*N*/
/*N*/ 	UpdateExpFlds();
/*N*/
/*N*/ 	SetModified();
/*N*/ }



    // Hoch-/Runterstufen



    // Hoch-/Runter - Verschieben !






    // zu diesem Gliederungspunkt


    // JP 13.06.96:
    // im Namen kann eine Nummer oder/und der Text stehen.
    // zuerst wird ueber die Nummer versucht den richtigen Eintrag zu finden.
    // Gibt es diesen, dann wird ueber den Text verglichen, od es der
    // gewuenschte ist. Ist das nicht der Fall, wird noch mal nur ueber den
    // Text gesucht. Wird dieser gefunden ist es der Eintrag. Ansonsten der,
    // der ueber die Nummer gefunden wurde.
    // Ist keine Nummer angegeben, dann nur den Text suchen.


/*N*/ void SwDoc::SetOutlineLSpace( BYTE nLevel, short nFirstLnOfst, USHORT nLSpace )
/*N*/ {
/*N*/ 	if( MAXLEVEL >= nLevel )
/*N*/ 	{
/*N*/ 		const SwNumFmt& rNFmt = pOutlineRule->Get( nLevel );
/*N*/ 		if( nLSpace != rNFmt.GetAbsLSpace() ||
/*N*/ 			nFirstLnOfst != rNFmt.GetFirstLineOffset() )
/*N*/ 		{
/*N*/ 			SwNumFmt aFmt( rNFmt );
/*N*/ 			aFmt.SetAbsLSpace( nLSpace );
/*N*/ 			aFmt.SetFirstLineOffset( nFirstLnOfst );
/*N*/ 			pOutlineRule->Set( nLevel, aFmt );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*  */

// --- Nummerierung -----------------------------------------

/*N*/ void SwNumRuleInfo::MakeList( SwDoc& rDoc )
/*N*/ {
/*N*/ 	SwModify* pMod;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	USHORT i, nMaxItems = rDoc.GetAttrPool().GetItemCount( RES_PARATR_NUMRULE);
/*N*/ 	for( i = 0; i < nMaxItems; ++i )
/*N*/ 		if( 0 != (pItem = rDoc.GetAttrPool().GetItem( RES_PARATR_NUMRULE, i ) ) &&
/*N*/ 			0 != ( pMod = (SwModify*)((SwNumRuleItem*)pItem)->GetDefinedIn()) &&
/*N*/ 			((SwNumRuleItem*)pItem)->GetValue().Len() &&
/*N*/ 			((SwNumRuleItem*)pItem)->GetValue() == rName )
/*N*/ 		{
/*N*/ 			if( pMod->IsA( TYPE( SwFmt )) )
/*?*/ 				pMod->GetInfo( *this );
/*N*/ 			else if( ((SwTxtNode*)pMod)->GetNodes().IsDocNodes() )
/*N*/ 				AddNode( *(SwTxtNode*)pMod );
/*N*/ 		}
/*N*/ }


/*N*/ void lcl_ChgNumRule( SwDoc& rDoc, const SwNumRule& rRule, SwHistory* pHist,
/*N*/ 						SwNumRuleInfo* pRuleInfo = 0 )
/*N*/ {
/*N*/ 	SwNumRule* pOld = rDoc.FindNumRulePtr( rRule.GetName() );
/*N*/ 	ASSERT( pOld, "ohne die alte NumRule geht gar nichts" );
/*N*/
/*N*/ 	USHORT nChkLevel = 0, nChgFmtLevel = 0, nMask = 1;
/*N*/
        BYTE n=0;
/*N*/ 	for( n = 0; n < MAXLEVEL; ++n, nMask <<= 1 )
/*N*/ 	{
/*N*/ 		const SwNumFmt& rOldFmt = pOld->Get( n ),
/*N*/ 					  & rNewFmt = rRule.Get( n );
/*N*/
/*N*/ 		if( rOldFmt != rNewFmt )
/*N*/ 		{
/*N*/ 			nChgFmtLevel |= nMask;
/*N*/ 			if( rOldFmt.GetAbsLSpace() != rNewFmt.GetAbsLSpace() ||
/*N*/ 				rOldFmt.GetFirstLineOffset() != rNewFmt.GetFirstLineOffset() )
/*N*/ 				nChkLevel |= nMask;
/*N*/ 		}
/*N*/ 		else if( SVX_NUM_NUMBER_NONE > rNewFmt.GetNumberingType() && 1 < rNewFmt.GetIncludeUpperLevels() &&
/*N*/ 				0 != (nChgFmtLevel & GetUpperLvlChg( n, rNewFmt.GetIncludeUpperLevels(),nMask )) )
/*?*/ 			nChgFmtLevel |= nMask;
/*N*/ 	}
/*N*/
/*N*/ 	if( !nChgFmtLevel )			// es wurde nichts veraendert?
/*N*/ 	{
/*N*/ 		pOld->CheckCharFmts( &rDoc );
/*N*/ 		pOld->SetContinusNum( rRule.IsContinusNum() );
/*N*/ 		return ;
/*N*/ 	}
/*N*/
/*N*/ 	SwNumRuleInfo* pUpd;
/*N*/ 	if( !pRuleInfo )
/*N*/ 	{
/*N*/ 		pUpd = new SwNumRuleInfo( rRule.GetName() );
/*N*/ 		pUpd->MakeList( rDoc );
/*N*/ 	}
/*N*/ 	else
/*?*/ 		pUpd = pRuleInfo;
/*N*/
/*N*/ 	BYTE nLvl;
/*N*/ 	for( ULONG nFirst = 0, nLast = pUpd->GetList().Count();
/*N*/ 		nFirst < nLast; ++nFirst )
/*N*/ 	{
/*?*/ 		SwTxtNode* pTxtNd = pUpd->GetList().GetObject( nFirst );
/*?*/ 		if( pTxtNd->GetNum() && ( nLvl = (~NO_NUMLEVEL &
/*?*/ 			pTxtNd->GetNum()->GetLevel() ) ) < MAXLEVEL )
/*?*/ 		{
/*?*/ 			if( nChgFmtLevel & ( 1 << nLvl ))
/*?*/ 			{
/*?*/ 				pTxtNd->NumRuleChgd();
/*?*/
/*?*/ #ifndef NUM_RELSPACE
/*?*/ 				if( nChkLevel && (nChkLevel & ( 1 << nLvl )) &&
/*?*/ 					pOld->IsRuleLSpace( *pTxtNd ) )
/*?*/ 					pTxtNd->SetNumLSpace( TRUE );
/*?*/
/*?*/ 				if( pHist )
/*?*/ 				{
/*?*/ 					const SfxPoolItem& rItem =
/*?*/ 								pTxtNd->SwCntntNode::GetAttr( RES_LR_SPACE );
/*?*/ 					pHist->Add( &rItem, &rItem, pTxtNd->GetIndex() );
/*?*/ 				}
/*?*/ #endif
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	for( n = 0; n < MAXLEVEL; ++n )
/*N*/ 		if( nChgFmtLevel & ( 1 << n ))
/*N*/ 			pOld->Set( n, rRule.GetNumFmt( n ));
/*N*/
/*N*/ 	pOld->CheckCharFmts( &rDoc );
/*N*/ 	pOld->SetInvalidRule( TRUE );
/*N*/ 	pOld->SetContinusNum( rRule.IsContinusNum() );
/*N*/
/*N*/ 	if( !pRuleInfo )
/*N*/ 		delete pUpd;
/*N*/ }

/*N*/ void SwDoc::SetNumRule( const SwPaM& rPam, const SwNumRule& rRule,
/*N*/ 						sal_Bool bSetAbsLSpace, sal_Bool bCalledFromShell )
/*N*/ {
/*N*/ 	ULONG nPamPos = rPam.Start()->nNode.GetIndex();
/*N*/ 	BOOL bSetItem = TRUE;
/*N*/ 	SwNumRule* pNew = FindNumRulePtr( rRule.GetName() );
/*N*/
/*N*/ 	if( !pNew )
/*N*/     {
/*N*/ 		pNew = (*pNumRuleTbl)[ MakeNumRule( rRule.GetName(), &rRule ) ];
/*N*/
        /* #109308# If called from a shell propagate an existing
            adjust item at the beginning am rPam into the new
            numbering rule. */
/*N*/         if (bCalledFromShell)
/*N*/         {
/*N*/             SwCntntNode * pCntntNode = rPam.GetCntntNode();
/*N*/
/*N*/             if (pCntntNode)
/*N*/             {
/*N*/                 SwAttrSet & rAttrSet = pCntntNode->GetSwAttrSet();
/*N*/                 const SfxPoolItem * pItem = NULL;
/*N*/
/*N*/                 if (SFX_ITEM_SET == rAttrSet.GetItemState(RES_PARATR_ADJUST,
/*N*/                                                           TRUE,
/*N*/                                                           &pItem))
/*N*/                     pNew->SetNumAdjust(((SvxAdjustItem *) pItem)->GetAdjust());
/*N*/             }
/*N*/         }
/*N*/
/*N*/     }
/*N*/ 	else if( rRule.IsAutoRule() && !(*pNew == rRule) )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ 	}
/*N*/
/*N*/ 	if( bSetItem )
/*N*/ 	{
/*N*/ 		Insert( rPam, SwNumRuleItem( pNew->GetName() ) );
/*N*/ 	}
/*N*/ 	UpdateNumRule( pNew->GetName(), nPamPos );
/*N*/
/*N*/ 	SetModified();
/*N*/ }

/*N*/ void SwDoc::SetNumRuleStart( const SwPosition& rPos, BOOL bFlag )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
/*N*/ }

/*N*/ void SwDoc::SetNodeNumStart( const SwPosition& rPos, USHORT nStt )
/*N*/ {
/*N*/ 	SwTxtNode* pTxtNd = rPos.nNode.GetNode().GetTxtNode();
/*N*/ 	const SwNumRule* pRule;
/*N*/ 	if( pTxtNd && pTxtNd->GetNum() && 0 != ( pRule = pTxtNd->GetNumRule() )
/*N*/ 		&& nStt != pTxtNd->GetNum()->GetSetValue() )
/*N*/ 	{
/*N*/ 		SwNodeNum aNum( *pTxtNd->GetNum() );
/*N*/ 		aNum.SetSetValue( nStt );
/*N*/ 		pTxtNd->UpdateNum( aNum );
/*N*/ 		UpdateNumRule( pRule->GetName(), USHRT_MAX != nStt
/*N*/ 							? rPos.nNode.GetIndex() : ULONG_MAX );
/*N*/ 		SetModified();
/*N*/ 	}
/*N*/ }

    // loeschen geht nur, wenn die Rule niemand benutzt!
/*N*/ BOOL SwDoc::DelNumRule( const String& rName )
/*N*/ {
/*N*/ 	USHORT nPos = FindNumRule( rName );
/*N*/ 	if( USHRT_MAX != nPos && !IsUsed( *(*pNumRuleTbl)[ nPos ] ))
/*N*/ 	{
/*N*/ 		pNumRuleTbl->DeleteAndDestroy( nPos );
/*N*/ 		SetModified();
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SwDoc::ChgNumRuleFmts( const SwNumRule& rRule )
/*N*/ {
/*N*/ 	SwNumRule* pRule = FindNumRulePtr( rRule.GetName() );
/*N*/ 	if( pRule )
/*N*/ 	{
/*N*/ 		SwHistory* pHistory = 0;
/*N*/ 		::binfilter::lcl_ChgNumRule( *this, rRule, pHistory );
/*N*/
/*N*/ 		SetModified();
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::StopNumRuleAnimations( OutputDevice* pOut )
/*N*/ {
/*N*/ 	for( USHORT n = GetNumRuleTbl().Count(); n; )
/*N*/ 	{
/*N*/ 		SwNumRuleInfo aUpd( GetNumRuleTbl()[ --n ]->GetName() );
/*N*/ 		aUpd.MakeList( *this );
/*N*/
/*N*/ 		for( ULONG nFirst = 0, nLast = aUpd.GetList().Count();
/*N*/ 				nFirst < nLast; ++nFirst )
/*N*/ 		{
/*N*/ 			SwTxtNode* pTNd = aUpd.GetList().GetObject( nFirst );
/*N*/
/*N*/ 			SwClientIter aIter( *pTNd );
/*N*/ 			for( SwFrm* pFrm = (SwFrm*)aIter.First( TYPE(SwFrm) );
/*N*/ 					pFrm; pFrm = (SwFrm*)aIter.Next() )
/*N*/ 				if( ((SwTxtFrm*)pFrm)->HasAnimation() )
/*?*/ 					{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 ((SwTxtFrm*)pFrm)->StopAnimation( pOut );
/*N*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ BOOL SwDoc::DelNumRules( const SwPaM& rPam )
/*N*/ {
    DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 ULONG nStt = rPam.GetPoint()->nNode.GetIndex(),
/*N*/ }


    // zum naechsten/vorhergehenden Punkt auf gleicher Ebene








/*N*/ BOOL SwDoc::NumUpDown( const SwPaM& rPam, BOOL bDown )
/*N*/ {
/*N*/ 	ULONG nStt = rPam.GetPoint()->nNode.GetIndex(),
/*N*/ 			nEnd = rPam.GetMark()->nNode.GetIndex();
/*N*/ 	if( nStt > nEnd )
/*N*/ 	{
/*?*/ 		ULONG nTmp = nStt; nStt = nEnd; nEnd = nTmp;
/*N*/ 	}
/*N*/
/*N*/   signed char nDiff = bDown ? 1 : -1;
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	String sNumRule;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	const String* pName;
/*N*/ 	for( ; nStt <= nEnd; ++nStt )
/*N*/ 	{
/*N*/ 		SwTxtNode* pTNd = GetNodes()[ nStt ]->GetTxtNode();
/*N*/ 		if( pTNd && 0 != ( pItem = pTNd->GetNoCondAttr(
/*N*/ 			RES_PARATR_NUMRULE, TRUE ) ) &&
/*N*/ 			( pName = &((SwNumRuleItem*)pItem)->GetValue())->Len() )
/*N*/ 		{
/*N*/ 			BYTE nLevel = pTNd->GetNum()->GetLevel();
/*N*/ 			if( ( -1 == nDiff && 0 < ( nLevel & ~NO_NUMLEVEL )) ||
/*N*/ 				( 1 == nDiff && MAXLEVEL - 1 > ( nLevel & ~NO_NUMLEVEL ) ) )
/*N*/ 			{
/*N*/ 				nLevel += nDiff;
/*N*/ 				SwNodeNum aNum( *pTNd->GetNum() );
/*N*/ 				aNum.SetLevel( nLevel );
/*N*/
/*N*/ 				pTNd->UpdateNum( aNum );
/*N*/ #ifndef NUM_RELSPACE
/*N*/ 				pTNd->SetNumLSpace( TRUE );
/*N*/ #endif
/*N*/ 				if( *pName != sNumRule )
/*N*/ 				{
/*N*/ 					sNumRule = *pName;
/*N*/ 					SwNumRule* pRule = FindNumRulePtr( *pName );
/*N*/ 					pRule->SetInvalidRule( TRUE );
/*N*/ 				}
/*N*/ 				bRet = TRUE;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if( bRet )
/*N*/ 	{
/*N*/ 		UpdateNumRule();
/*N*/ 		SetModified();
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }



/*N*/ SwNumRule* SwDoc::GetCurrNumRule( const SwPosition& rPos ) const
/*N*/ {
/*N*/ 	SwNumRule* pRet = 0;
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	SwTxtNode* pTNd = rPos.nNode.GetNode().GetTxtNode();
/*N*/
/*N*/ 	if( pTNd && 0 != ( pItem = pTNd->GetNoCondAttr( RES_PARATR_NUMRULE, TRUE ) ) &&
/*N*/ 		((SwNumRuleItem*)pItem)->GetValue().Len() )
/*N*/ 		pRet = FindNumRulePtr( ((SwNumRuleItem*)pItem)->GetValue() );
/*N*/ 	return pRet;
/*N*/ }

/*N*/ USHORT SwDoc::FindNumRule( const String& rName ) const
/*N*/ {
/*N*/ 	for( USHORT n = pNumRuleTbl->Count(); n; )
/*N*/ 		if( (*pNumRuleTbl)[ --n ]->GetName() == rName )
/*N*/ 			return n;
/*
//JP 20.11.97: sollte man im Find neue Rule anlegen??
                erstmal nicht
    USHORT nPoolId = GetPoolId( rName, GET_POOLID_NUMRULE );
    if( USHRT_MAX != nPoolId )
    {
        SwDoc* pThis = (SwDoc*)this;
        SwNumRule* pR = pThis->GetNumRuleFromPool( nPoolId );
        for( n = pNumRuleTbl->Count(); n; )
            if( (*pNumRuleTbl)[ --n ] == pR )
                 return n;
    }
*/
/*N*/ 	return USHRT_MAX;
/*N*/ }

/*N*/ SwNumRule* SwDoc::FindNumRulePtr( const String& rName ) const
/*N*/ {
/*N*/ 	for( USHORT n = pNumRuleTbl->Count(); n; )
/*N*/ 		if( (*pNumRuleTbl)[ --n ]->GetName() == rName )
/*N*/ 			return (*pNumRuleTbl)[ n ];
/*N*/
/*
//JP 20.11.97: sollte man im Find neue Rule anlegen??
                 erstmal nicht
    USHORT nPoolId = GetPoolId( rName, GET_POOLID_NUMRULE );
     if( USHRT_MAX != nPoolId )
    {
         SwDoc* pThis = (SwDoc*)this;
        return pThis->GetNumRuleFromPool( nPoolId );
    }
*/
/*N*/ 	return 0;
/*N*/ }

/*N*/ USHORT SwDoc::MakeNumRule( const String &rName, const SwNumRule* pCpy )
/*N*/ {
/*N*/ 	SwNumRule* pNew;
/*N*/ 	if( pCpy )
/*N*/ 	{
/*N*/ 		pNew = new SwNumRule( *pCpy );
/*N*/ 		pNew->SetName( GetUniqueNumRuleName( &rName ));
/*N*/ 		if( pNew->GetName() != rName )
/*N*/ 		{
/*?*/ 			pNew->SetPoolFmtId( USHRT_MAX );
/*?*/ 			pNew->SetPoolHelpId( USHRT_MAX );
/*?*/ 			pNew->SetPoolHlpFileId( UCHAR_MAX );
/*N*/ 		}
/*N*/ 		pNew->CheckCharFmts( this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pNew = new SwNumRule( GetUniqueNumRuleName( &rName ) );
/*N*/ 	USHORT nRet = pNumRuleTbl->Count();
/*N*/ 	pNumRuleTbl->Insert( pNew, nRet );
/*N*/ 	return nRet;
/*N*/ }
/*N*/
/*N*/ String SwDoc::GetUniqueNumRuleName( const String* pChkStr, BOOL bAutoNum ) const
/*N*/ {
/*N*/ 	String aName;
/*N*/ 	if( bAutoNum )
/*N*/ 	{
/*N*/ 		long n = Time().GetTime();
/*N*/ 		n += Date().GetDate();
/*N*/ 		aName = String::CreateFromInt32( n );
/*N*/ 		if( pChkStr && !pChkStr->Len() )
/*N*/ 			pChkStr = 0;
/*N*/ 	}
/*N*/ 	else if( pChkStr && pChkStr->Len() )
/*N*/ 		aName = *pChkStr;
/*N*/ 	else
/*N*/ 	{
/*?*/ 		pChkStr = 0;
/*?*/ 		aName = SW_RESSTR( STR_NUMRULE_DEFNAME );
/*N*/ 	}
/*N*/
/*N*/ 	USHORT nNum, nTmp, nFlagSize = ( pNumRuleTbl->Count() / 8 ) +2;
/*N*/ 	BYTE* pSetFlags = new BYTE[ nFlagSize ];
/*N*/ 	memset( pSetFlags, 0, nFlagSize );
/*N*/
/*N*/ 	xub_StrLen nNmLen = aName.Len();
/*N*/ 	if( !bAutoNum && pChkStr )
/*N*/ 	{
/*?*/ 		while( nNmLen-- && '0' <= aName.GetChar( nNmLen ) &&
/*?*/ 						   '9' >= aName.GetChar( nNmLen ) )
/*?*/ 			; //nop
/*?*/
/*?*/ 		if( ++nNmLen < aName.Len() )
/*?*/ 		{
/*?*/ 			aName.Erase( nNmLen );
/*?*/ 			pChkStr = 0;
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	const SwNumRule* pNumRule;
        USHORT n=0;
/*N*/ 	for( n = 0; n < pNumRuleTbl->Count(); ++n )
/*N*/ 		if( 0 != ( pNumRule = (*pNumRuleTbl)[ n ] ) )
/*N*/ 		{
/*N*/ 			const String& rNm = pNumRule->GetName();
/*N*/ 			if( rNm.Match( aName ) == nNmLen )
/*N*/ 			{
/*N*/ 				// Nummer bestimmen und das Flag setzen
/*N*/ 				nNum = rNm.Copy( nNmLen ).ToInt32();
/*N*/ 				if( nNum-- && nNum < pNumRuleTbl->Count() )
/*N*/ 					pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
/*N*/ 			}
/*N*/ 			if( pChkStr && pChkStr->Equals( rNm ) )
/*N*/ 				pChkStr = 0;
/*N*/ 		}
/*N*/
/*N*/ 	if( !pChkStr )
/*N*/ 	{
/*N*/ 		// alle Nummern entsprechend geflag, also bestimme die richtige Nummer
/*N*/ 		nNum = pNumRuleTbl->Count();
/*N*/ 		for( n = 0; n < nFlagSize; ++n )
/*N*/ 			if( 0xff != ( nTmp = pSetFlags[ n ] ))
/*N*/ 			{
/*N*/ 				// also die Nummer bestimmen
/*N*/ 				nNum = n * 8;
/*N*/ 				while( nTmp & 1 )
/*N*/ 					++nNum, nTmp >>= 1;
/*N*/ 				break;
/*N*/ 			}
/*N*/
/*N*/ 	}
/*N*/ 	delete [] pSetFlags;
/*N*/ 	if( pChkStr && pChkStr->Len() )
/*N*/ 		return *pChkStr;
/*N*/ 	return aName += String::CreateFromInt32( ++nNum );
/*N*/ }

/*N*/ const SwNode* lcl_FindBaseNode( const SwNode& rNd )
/*N*/ {
/*N*/ 	const SwNodes& rNds = rNd.GetNodes();
/*N*/ 	ULONG nNdIdx = rNd.GetIndex();
/*N*/ 	if( nNdIdx > rNds.GetEndOfExtras().GetIndex() )
/*N*/ 		return rNds.GetEndOfContent().FindStartNode();
/*N*/
/*N*/ 	const SwNode* pSttNd = rNds[ ULONG(0) ]->FindStartNode();
/*N*/ 	const SwNode* pNd = rNd.FindStartNode();
/*N*/ 	while( pSttNd != pNd->FindStartNode()->FindStartNode() )
/*?*/ 		pNd = pNd->FindStartNode();
/*N*/ 	return pNd;
/*N*/ }


/*N*/ void SwDoc::UpdateNumRule()
/*N*/ {
/*N*/ 	SwNumRuleTbl& rNmTbl = GetNumRuleTbl();
/*N*/ 	for( USHORT n = 0; n < rNmTbl.Count(); ++n )
/*N*/ 		if( rNmTbl[ n ]->IsInvalidRule() )
/*N*/ 			UpdateNumRule( rNmTbl[ n ]->GetName(), ULONG_MAX );
/*N*/ }
/*N*/
/*N*/ void SwDoc::UpdateNumRule( const String& rName, ULONG nUpdPos )
/*N*/ {
/*N*/ 	SwNumRuleInfo aUpd( rName );
/*N*/ 	aUpd.MakeList( *this );
/*N*/
/*N*/ 	if( ULONG_MAX == nUpdPos )
/*N*/ 		nUpdPos = 0;
/*N*/ 	else
/*N*/ 		aUpd.GetList().SearchKey( nUpdPos, &nUpdPos );
/*N*/
/*N*/ 	SwNumRule* pRule = FindNumRulePtr( rName );
/*N*/ 	if( nUpdPos < aUpd.GetList().Count() )
/*N*/ 	{
/*N*/ 		USHORT nInitLevels = USHRT_MAX; // Bitfeld fuer die Levels!
/*N*/ 										// TRUE: starte mit NumFmt Start
/*N*/ 		USHORT nNumVal = 0;
/*N*/ 		SwNodeNum aNum( 0 );
/*N*/
/*N*/ 		if( pRule->IsContinusNum() )
/*?*/ 			nNumVal = pRule->Get( 0 ).GetStart();
/*N*/
/*N*/ 		SwTxtNode* pStt = aUpd.GetList().GetObject( nUpdPos );
/*N*/ 		SwTxtNode* pPrev = nUpdPos ? aUpd.GetList().GetObject( nUpdPos-1 ) : 0;
/*N*/ 		const SwNode* pBaseNd = lcl_FindBaseNode( *pStt );
/*N*/ 		if( pPrev  && lcl_FindBaseNode( *pPrev ) == pBaseNd )
/*N*/ 		{
/*N*/ 			if( pPrev->GetNum() )
/*N*/ 			{
/*N*/ 				const SwNodeNum* pPrevNdNum = pPrev->GetNum();
/*N*/ 				if( pPrevNdNum->GetLevel() & NO_NUMLEVEL )
/*N*/ 				{
/*N*/                     // OD 10.12.2002 #106111# - use correct search level
/*N*/                     BYTE nSrchLvl = GetRealLevel( pStt->GetNum()->GetLevel() );
/*N*/ 					pPrevNdNum = 0;
/*N*/ 					ULONG nArrPos = nUpdPos-1;
/*N*/ 					while( nArrPos-- )
/*N*/ 					{
/*N*/ 						pPrev = aUpd.GetList().GetObject( nArrPos );
/*N*/ 						if( lcl_FindBaseNode( *pPrev ) != pBaseNd )
/*N*/ 							break;
/*N*/
/*N*/ 						if( 0 != ( pPrevNdNum = pPrev->GetNum() ))
/*N*/ 						{
/*N*/ 							// uebergeordnete Ebene
/*N*/ 							if( nSrchLvl > (pPrevNdNum->GetLevel() &~ NO_NUMLEVEL))
/*N*/ 							{
/*N*/ 								pPrevNdNum = 0;
/*N*/ 								break;
/*N*/ 							}
/*N*/ 							// gleiche Ebene und kein NO_NUMLEVEL
/*N*/ 							if( nSrchLvl == (pPrevNdNum->GetLevel() &~ NO_NUMLEVEL)
/*N*/ 								&& !( pPrevNdNum->GetLevel() & NO_NUMLEVEL ))
/*N*/ 								break;
/*N*/
/*N*/ 							pPrevNdNum = 0;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				if( pPrevNdNum )
/*N*/ 				{
/*N*/ 					aNum = *pPrevNdNum;
/*N*/ 					aNum.SetStart( FALSE );
/*N*/ 					aNum.SetSetValue( USHRT_MAX );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			nInitLevels = 0;
/*N*/             // OD 10.12.2002 #106111# - sublevels have to be restarted.
/*N*/             for ( int nSubLvl = GetRealLevel( aNum.GetLevel() ) + 1; nSubLvl < MAXLEVEL; ++nSubLvl)
/*N*/                 nInitLevels |= ( 1 << nSubLvl );
/*N*/ 			nNumVal = aNum.GetLevelVal()[ GetRealLevel( aNum.GetLevel() ) ];
/*N*/ 		}
/*N*/
/*N*/ 		const SwNode* pOutlNd = 0;
/*N*/ 		for( ; nUpdPos < aUpd.GetList().Count(); ++nUpdPos )
/*N*/ 		{
/*N*/ 			pStt = aUpd.GetList().GetObject( nUpdPos );
/*N*/
/*N*/ 			const SwNode* pTmpBaseNd = lcl_FindBaseNode( *pStt );
/*N*/ 			if( pTmpBaseNd != pBaseNd )
/*N*/ 			{
/*?*/ 				aNum.SetLevel( 0 );
/*?*/ 				memset( aNum.GetLevelVal(), 0,
/*?*/ 						(MAXLEVEL) * sizeof( aNum.GetLevelVal()[0]) );
/*?*/ 				pBaseNd = pTmpBaseNd;
/*N*/ 			}
/*N*/
/*N*/ 			BYTE nLevel = aNum.GetLevel();
/*N*/ 			BYTE nNdOldLvl = MAXLEVEL;
/*N*/ 			if( pStt->GetNum() )
/*N*/ 			{
/*N*/ 				if( NO_NUMBERING != pStt->GetNum()->GetLevel() )
/*N*/ 					nNdOldLvl = nLevel = pStt->GetNum()->GetLevel();
/*N*/ 				if( pStt->GetNum()->IsStart() )
/*N*/ 				{
/*?*/ 					aNum.SetStart( TRUE );
/*?*/                     // OD 10.12.2002 #106111# - correct reset of level numbers
/*?*/                     for ( int nSubLvl = nLevel; nSubLvl < MAXLEVEL; ++nSubLvl)
/*?*/                         aNum.GetLevelVal()[ nSubLvl ] = 0;
/*?*/ 					if( pRule->IsContinusNum() )
/*?*/ 					{
/*?*/ 						nNumVal = pRule->Get( 0 ).GetStart();
/*?*/ 						nInitLevels |= 1;
/*?*/ 					}
/*?*/ 					else
/*?*/ 						nInitLevels |= ( 1 << GetRealLevel( nLevel ));
/*N*/ 				}
/*N*/ 							else if( USHRT_MAX != pStt->GetNum()->GetSetValue() )
/*N*/ 				{
/*N*/ 					aNum.SetSetValue( nNumVal = pStt->GetNum()->GetSetValue() );
/*N*/                     // OD 10.12.2002 #106111# - init <nInitLevels> for continues
/*N*/                     // numbering.
/*N*/                     if( pRule->IsContinusNum() )
/*N*/                         nInitLevels |= 1;
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			if( NO_NUMLEVEL & nLevel )		// NoNum mit Ebene
/*N*/ 			{
/*N*/ 				BYTE nPrevLvl = GetRealLevel( aNum.GetLevel() ),
/*N*/ 					nCurrLvl = GetRealLevel( nLevel );
/*N*/
/*N*/ 				if( nPrevLvl < nCurrLvl )
/*N*/ 				{
/*N*/ 					if( !(nInitLevels & ( 1 << nPrevLvl )) )
/*N*/ 						++nPrevLvl;
/*N*/ 					for( ; nPrevLvl < nCurrLvl; ++nPrevLvl )
/*?*/ 						nInitLevels |= ( 1 << nPrevLvl );
/*N*/ 				}
/*N*/
/*N*/ 				aNum.SetLevel( nLevel );
/*N*/ 				pStt->UpdateNum( aNum );
/*N*/ 			}
/*N*/ 			else if( NO_NUM != nLevel )
/*N*/ 			{
/*N*/ 				// beim Format mit Bitmap die Graphicen schon mal anfordern
/*N*/ 				const SwNumFmt* pNumFmt = pRule->GetNumFmt( GetRealLevel( nLevel ));
/*N*/ 				if( pNumFmt && SVX_NUM_BITMAP == pNumFmt->GetNumberingType() )
/*?*/ 					pNumFmt->GetGraphic();
/*N*/
/*N*/ 				if( pRule->IsContinusNum() )
/*N*/ 				{
/*N*/ 					if( !(nInitLevels & 1) &&
/*N*/ 						!( pNumFmt && (SVX_NUM_CHAR_SPECIAL == pNumFmt->GetNumberingType() ||
/*N*/ 									   SVX_NUM_BITMAP == pNumFmt->GetNumberingType() ||
/*N*/ 									   SVX_NUM_NUMBER_NONE == pNumFmt->GetNumberingType() )))
/*?*/ 						++nNumVal;
/*?*/ 					aNum.GetLevelVal()[ nLevel ] = nNumVal;
/*?*/                     // OD 10.12.2002 #106111# - reset <nInitLevels>
/*?*/                     nInitLevels &= ~1;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					BYTE nPrevLvl = GetRealLevel( aNum.GetLevel() );
/*N*/ 					if( nPrevLvl < nLevel  )
/*N*/ 					{
/*N*/ 						// Erfrage wie geloescht werden soll:
/*N*/ 						// z.B von Stufe 0 -> 1: 1 -> 0.1 ; wenn nStart = 1
/*N*/ 						// 	aber Stufe 1 -> 2: 1.1 -> 1.1.1 !!, nur 0.1 -> 0.0.1
/*N*/ 						if( !(nInitLevels & ( 1 << nPrevLvl )) )
/*N*/ 							++nPrevLvl;
/*N*/
/*N*/ 						for( int ii = nPrevLvl; ii < nLevel; ++ii )
/*N*/ 						{
/*N*/ 							nInitLevels &= ~( 1 << ii );
/*N*/ 							aNum.GetLevelVal()[ ii ] =
/*N*/ 										pRule->Get( ii ).GetStart();
/*N*/ 						}
/*N*/ 						aNum.GetLevelVal()[ nLevel ] =
/*N*/ 							USHRT_MAX == aNum.GetSetValue()
/*N*/ 								? pRule->Get( nLevel ).GetStart()
/*N*/ 								: aNum.GetSetValue();
/*N*/ 					}
/*N*/ 					else if( USHRT_MAX != aNum.GetSetValue() )
/*N*/ 						aNum.GetLevelVal()[ nLevel ] = aNum.GetSetValue();
/*N*/ 					else if( nInitLevels & ( 1 << nLevel ))
/*N*/ 						aNum.GetLevelVal()[ nLevel ] =
/*N*/ 										pRule->Get( nLevel ).GetStart();
/*N*/ 					else
/*N*/ 						aNum.GetLevelVal()[ nLevel ]++;
/*N*/ 				}
/*N*/ 				nInitLevels &= ~( 1 << nLevel );
/*N*/ 				aNum.SetLevel( nLevel );
/*N*/
/*N*/                 // OD 10.12.2002 #106111# - reset numbers of all sublevels and
/*N*/                 // note in <nInitLevels> that numbering of all sublevels have
/*N*/                 // to be restarted.
/*N*/                 for ( int nSubLvl = nLevel+1; nSubLvl < MAXLEVEL; ++nSubLvl)
/*N*/                 {
/*N*/                     aNum.GetLevelVal()[ nSubLvl ] = 0;
/*N*/                     nInitLevels |= ( 1 << nSubLvl );
/*N*/                 }
/*N*/
/*N*/ 				pStt->UpdateNum( aNum );
/*N*/ 			}
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 			BOOL bCheck = TRUE;
/*N*/ 			if( RES_CONDTXTFMTCOLL == pStt->GetFmtColl()->Which() )
/*N*/ 			{
/*N*/ //				SwFmtColl* pChgColl = pStt->GetCondFmtColl();
/*N*/ 				pStt->ChkCondColl();
/*
//JP 19.11.97:
// setzen der bedingten Vorlage aendert nichts an den Einzuegen, die bleiben
// als harte vorhanden
                if( pStt->GetCondFmtColl() )
                {
                    // es gab eine Aenderung -> harte Einzuege entfernen
                    if( pChgColl != pStt->GetCondFmtColl() )
                        pStt->ResetAttr( RES_LR_SPACE );
                    bCheck = FALSE;
                }
*/
            }
/*N*/ 			else if( !pOutlNd && NO_NUMBERING !=
/*N*/ 					((SwTxtFmtColl*)pStt->GetFmtColl())->GetOutlineLevel() )
/*N*/ 				pOutlNd = pStt;
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ #ifndef NUM_RELSPACE
/*N*/ 			// hat sich eine Level - Aenderung ergeben, so setze jetzt die
/*N*/ 			// gueltigen Einzuege
/*N*/ 			if( bCheck && ( nLevel != nNdOldLvl || pStt->IsSetNumLSpace())
/*N*/ 				&& GetRealLevel( nLevel ) < MAXLEVEL )
/*N*/ 			{
/*N*/ 				SvxLRSpaceItem aLR( ((SvxLRSpaceItem&)pStt->SwCntntNode::GetAttr(
/*N*/ 									RES_LR_SPACE )) );
/*N*/
/*N*/ 				const SwNumFmt& rNFmt = pRule->Get( GetRealLevel( nLevel ));
/*N*/
/*N*/ 				// ohne Nummer immer ohne FirstLineOffset!!!!
/*N*/ 				short nFOfst = rNFmt.GetFirstLineOffset();
/*N*/ 				if( nLevel & NO_NUMLEVEL ) nFOfst = 0;
/*N*/ 				aLR.SetTxtFirstLineOfstValue( nFOfst );
/*N*/ 				aLR.SetTxtLeft( rNFmt.GetAbsLSpace() );
/*N*/
/*N*/ 				pStt->SwCntntNode::SetAttr( aLR );
/*N*/ 			}
/*N*/ 			// Flag immer loeschen!
/*N*/ 			pStt->SetNumLSpace( FALSE );
/*N*/ #endif
/*N*/ 			aNum.SetStart( FALSE );
/*N*/ 			aNum.SetSetValue( USHRT_MAX );
/*N*/ 		}
/*N*/ 		if( pOutlNd )
/*?*/ 			GetNodes().UpdtOutlineIdx( *pOutlNd );
/*N*/ 	}
/*N*/
/*N*/ 	ASSERT( pRule, "die NumRule sollte schon vorhanden sein!" );
/*N*/ 	if( pRule )
/*N*/ 		pRule->SetInvalidRule( FALSE );
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
