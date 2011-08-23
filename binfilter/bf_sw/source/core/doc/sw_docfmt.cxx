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

#define _ZFORLIST_DECLARE_TABLE
#define _SVSTDARR_USHORTSSORT
#define _SVSTDARR_USHORTS

#include <hintids.hxx>

#include <rtl/logfile.hxx>
#include <bf_svtools/itemiter.hxx>
#include <bf_so3/so2ref.hxx>
#include <bf_sfx2/app.hxx>
#include <bf_sfx2/misccfg.hxx>
#include <bf_svx/tstpitem.hxx>
#include <bf_svx/brkitem.hxx>
#ifndef _ZFORLIST_HXX //autogen
#define _ZFORLIST_DECLARE_TABLE
#include <bf_svtools/zforlist.hxx>
#endif

#include <com/sun/star/i18n/WordType.hdl>


#include <fmtpdsc.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <pagefrm.hxx>
#include <hints.hxx>			// fuer SwHyphenBug (in SetDefault)
#include <ndtxt.hxx>
#include <mvsave.hxx>			// servieren: Veraenderungen erkennen
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <paratr.hxx>
#include <redline.hxx>
#include <reffld.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <SwStyleNameMapper.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
namespace binfilter {
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

#ifndef SO2_DECL_SVLINKNAME_DEFINED
#define SO2_DECL_SVLINKNAME_DEFINED
SO2_DECL_REF(SvLinkName)
#endif

/*N*/ SV_IMPL_PTRARR(SwFrmFmts,SwFrmFmtPtr)
/*N*/ SV_IMPL_PTRARR(SwCharFmts,SwCharFmtPtr)

//Spezifische Frameformate (Rahmen)
/*N*/ SV_IMPL_PTRARR(SwSpzFrmFmts,SwFrmFmtPtr)

/*
 * interne Funktionen
 */


/*
 * Zuruecksetzen der harten Formatierung fuer Text
 */

// Uebergabeparameter fuer _Rst und lcl_SetTxtFmtColl
struct ParaRstFmt
{
    SwFmtColl* pFmtColl;
    const SwPosition *pSttNd, *pEndNd;
    const SfxItemSet* pDelSet;
    USHORT nWhich;
    BOOL bReset, bResetAll, bInclRefToxMark;

    ParaRstFmt( const SwPosition* pStt, const SwPosition* pEnd,
            USHORT nWhch = 0, const SfxItemSet* pSet = 0 )
        : pSttNd( pStt ), pEndNd( pEnd ), nWhich( nWhch ),
            pDelSet( pSet ), bResetAll( TRUE ), pFmtColl( 0 ),
            bInclRefToxMark( FALSE )
    {}

    ParaRstFmt(  )
        : pSttNd( 0 ), pEndNd( 0 ), nWhich( 0 ),
        pDelSet( 0 ), bResetAll( TRUE ), pFmtColl( 0 ),
        bInclRefToxMark( FALSE )
    {}
};

/* in pArgs steht die ChrFmtTablle vom Dokument
 * (wird bei Selectionen am Start/Ende und bei keiner SSelection benoetigt)
 */

/*N*/ BOOL lcl_RstTxtAttr( const SwNodePtr& rpNd, void* pArgs )
/*N*/ {
/*N*/ 	ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
/*N*/ 	SwTxtNode * pTxtNode = (SwTxtNode*)rpNd->GetTxtNode();
/*N*/ 	if( pTxtNode && pTxtNode->GetpSwpHints() )
/*N*/ 	{
/*N*/ 		SwIndex aSt( pTxtNode, 0 );
/*N*/ 		USHORT nEnd = pTxtNode->Len();
/*N*/
/*N*/ 		if( &pPara->pSttNd->nNode.GetNode() == pTxtNode &&
/*N*/ 			pPara->pSttNd->nContent.GetIndex() )
/*?*/ 			aSt = pPara->pSttNd->nContent.GetIndex();
/*N*/
/*N*/ 		if( &pPara->pEndNd->nNode.GetNode() == rpNd )
/*N*/ 			nEnd = pPara->pEndNd->nContent.GetIndex();
/*N*/
/*?*/ 			pTxtNode->RstAttr( aSt, nEnd - aSt.GetIndex(), pPara->nWhich,
/*?*/ 								pPara->pDelSet, pPara->bInclRefToxMark );
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL lcl_RstAttr( const SwNodePtr& rpNd, void* pArgs )
/*N*/ {
/*N*/ 	ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
/*N*/ 	SwCntntNode* pNode = (SwCntntNode*)rpNd->GetCntntNode();
/*N*/ 	if( pNode && pNode->GetpSwAttrSet() )
/*N*/ 	{
/*N*/ 		// das erhalten der Break-Attribute und der NumRule kommt nicht ins Undo
/*N*/ 		BOOL bLocked = pNode->IsModifyLocked();
/*N*/ 		pNode->LockModify();
/*N*/ 		SwDoc* pDoc = pNode->GetDoc();
/*N*/
/*N*/ 		SfxItemSet aSet( pDoc->GetAttrPool(),
/*N*/ 							RES_PAGEDESC, RES_BREAK,
/*N*/ 							RES_PARATR_NUMRULE, RES_PARATR_NUMRULE,
/*N*/ 							RES_LR_SPACE, RES_LR_SPACE,
/*N*/ 							0 );
/*N*/ 		SwAttrSet* pSet = pNode->GetpSwAttrSet();
/*N*/
/*N*/ 		USHORT __READONLY_DATA aSavIds[ 3 ] = { RES_PAGEDESC, RES_BREAK,
/*N*/ 												RES_PARATR_NUMRULE };
/*N*/
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		for( USHORT n = 0; n < 3; ++n )
/*N*/ 			if( SFX_ITEM_SET == pSet->GetItemState( aSavIds[ n ], FALSE, &pItem ))
/*N*/ 			{
/*?*/ 				BOOL bSave = FALSE;
/*?*/ 				switch( aSavIds[ n ] )
/*?*/ 				{
/*?*/ 				case RES_PAGEDESC:
/*?*/ 					bSave = 0 != ((SwFmtPageDesc*)pItem)->GetPageDesc();
/*?*/ 					break;
/*?*/ 				case RES_BREAK:
/*?*/ 					bSave = SVX_BREAK_NONE != ((SvxFmtBreakItem*)pItem)->GetBreak();
/*?*/ 					break;
/*?*/ 				case RES_PARATR_NUMRULE:
/*?*/ 					bSave = 0 != ((SwNumRuleItem*)pItem)->GetValue().Len();
/*?*/ 					break;
/*?*/ 				}
/*?*/ 				if( bSave )
/*?*/ 				{
/*?*/ 					aSet.Put( *pItem );
/*?*/ 					pSet->ClearItem( aSavIds[n] );
/*?*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 		if( !bLocked )
/*N*/ 			pNode->UnlockModify();
/*N*/
/*N*/ 		if( pPara )
/*N*/ 		{
/*N*/ 			if( pPara->pDelSet && pPara->pDelSet->Count() )
/*N*/ 			{
/*?*/ 				SfxItemIter aIter( *pPara->pDelSet );
/*?*/ 				pItem = aIter.FirstItem();
/*?*/ 				while( TRUE )
/*?*/ 				{
/*?*/ 					pNode->ResetAttr( pItem->Which() );
/*?*/ 					if( aIter.IsAtEnd() )
/*?*/ 						break;
/*?*/ 					pItem = aIter.NextItem();
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else if( pPara->bResetAll )
/*N*/ 				pNode->ResetAllAttr();
/*N*/ 			else
/*?*/ 				pNode->ResetAttr( RES_PARATR_BEGIN, POOLATTR_END - 1 );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			pNode->ResetAllAttr();
/*N*/
/*N*/ 		if( aSet.Count() )
/*N*/ 		{
/*?*/ 			pNode->LockModify();
/*?*/ 			pNode->SetAttr( aSet );
/*?*/
/*?*/ 			if( !bLocked )
/*?*/ 				pNode->UnlockModify();
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }


/*M*/ void SwDoc::ResetAttr( const SwPaM &rRg, BOOL bTxtAttr,
/*M*/ 						const SvUShortsSort* pAttrs )
/*M*/ {
/*M*/ 	SwPaM* pPam = (SwPaM*)&rRg;
/*M*/ 	BOOL bStopAttr = FALSE;
/*M*/ 	if( !bTxtAttr && pAttrs && pAttrs->Count() &&
/*M*/ 		RES_TXTATR_END > (*pAttrs)[ 0 ] )
/*M*/ 		bTxtAttr = TRUE;
/*M*/
/*M*/ 	if( !rRg.HasMark() )
/*M*/ 	{
/*M*/ 		SwTxtNode* pTxtNd = rRg.GetPoint()->nNode.GetNode().GetTxtNode();
/*M*/ 		if( !pTxtNd )
/*M*/ 			return ;
/*M*/
/*M*/ 		pPam = new SwPaM( *rRg.GetPoint() );
/*M*/
/*M*/ 		SwIndex& rSt = pPam->GetPoint()->nContent;
/*M*/ 		USHORT nMkPos = 0, nPtPos = rSt.GetIndex();
/*M*/ 		const String& rStr = pTxtNd->GetTxt();
/*M*/
/*M*/ 		// JP 22.08.96: Sonderfall: steht der Crsr in einem URL-Attribut
/*M*/ 		//				dann wird dessen Bereich genommen
/*M*/ 		const SwTxtAttr* pURLAttr;
/*M*/ 		if( pTxtNd->HasHints() &&
/*M*/ 			0 != ( pURLAttr = pTxtNd->GetTxtAttr( rSt, RES_TXTATR_INETFMT ))
/*M*/ 			&& pURLAttr->GetINetFmt().GetValue().Len() )
/*M*/ 		{
/*M*/ 			nMkPos = *pURLAttr->GetStart();
/*M*/ 			nPtPos = *pURLAttr->GetEnd();
/*M*/ 		}
/*M*/ 		else
/*M*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*M*/ 		}
/*M*/
/*M*/ 		rSt = nMkPos;
/*M*/ 		pPam->SetMark();
/*M*/ 		pPam->GetPoint()->nContent = nPtPos;
/*M*/ 	}
/*M*/
/*M*/ 	SwDataChanged aTmp( *pPam, 0 );
/*M*/
/*M*/ 	const SwPosition *pStt = pPam->Start(), *pEnd = pPam->End();
/*M*/   ParaRstFmt aPara( pStt, pEnd );
/*M*/
/*N*/     USHORT __FAR_DATA aResetableSetRange[] = {
/*N*/         RES_FRMATR_BEGIN, RES_FRMATR_END-1,
/*N*/         RES_CHRATR_BEGIN, RES_CHRATR_END-1,
/*N*/         RES_PARATR_BEGIN, RES_PARATR_END-1,
/*N*/         RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
/*N*/         RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
/*N*/         RES_TXTATR_CJK_RUBY, RES_TXTATR_UNKNOWN_CONTAINER,
/*N*/         RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
/*N*/         0
/*N*/     };
/*N*/
/*N*/ 	SfxItemSet aDelSet( GetAttrPool(), aResetableSetRange );
/*M*/ 	if( pAttrs && pAttrs->Count() )
/*M*/ 	{
/*M*/ 		for( USHORT n = pAttrs->Count(); n; )
/*M*/ 			if( POOLATTR_END > (*pAttrs)[ --n ] )
/*M*/ 				aDelSet.Put( *GetDfltAttr( (*pAttrs)[ n ] ));
/*M*/
/*M*/ 		if( aDelSet.Count() )
/*M*/ 			aPara.pDelSet = &aDelSet;
/*M*/ 	}
/*M*/
/*M*/ 	BOOL bAdd = TRUE;
/*M*/ 	SwNodeIndex aTmpStt( pStt->nNode );
/*M*/ 	SwNodeIndex aTmpEnd( pEnd->nNode );
/*M*/ 	if( pStt->nContent.GetIndex() )		// nur ein Teil
/*M*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*M*/ 	}
/*M*/ 	if( pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetCntntNode()->Len() )
/*M*/ 		// dann spaeter aufsetzen und alle CharFmtAttr -> TxtFmtAttr
/*M*/ 		aTmpEnd++, bAdd = FALSE;
/*M*/ 	else if( pStt->nNode != pEnd->nNode || !pStt->nContent.GetIndex() )
/*M*/ 	{
/*M*/ 		SwTxtNode* pTNd = aTmpEnd.GetNode().GetTxtNode();
/*M*/ 		if( pTNd && pTNd->GetpSwAttrSet() && pTNd->GetpSwAttrSet()->Count() )
/*M*/ 		{
/*M*/ 			SfxItemIter aIter( *pTNd->GetpSwAttrSet() );
/*M*/ 			const SfxPoolItem* pItem = aIter.GetCurItem();
/*M*/ 			while( TRUE )
/*M*/ 			{
/*M*/ 				if( IsInRange( aCharFmtSetRange, pItem->Which() ))
/*M*/ 				{
/*M*/ 					SwTxtAttr* pTAttr = pTNd->MakeTxtAttr( *pItem, 0,
/*M*/ 												pTNd->GetTxt().Len() );
/*M*/ 					if( !pTNd->pSwpHints )
/*M*/ 						pTNd->pSwpHints = new SwpHints;
/*M*/ 					pTNd->pSwpHints->SwpHintsArr::Insert( pTAttr );
/*M*/ 						pTNd->ResetAttr( pItem->Which() );
/*M*/ 				}
/*M*/ 				if( aIter.IsAtEnd() )
/*M*/ 					break;
/*M*/ 				pItem = aIter.NextItem();
/*M*/ 			}
/*M*/ 		}
/*M*/ 	}
/*M*/
/*M*/ 	if( aTmpStt < aTmpEnd )
/*M*/ 		GetNodes().ForEach( pStt->nNode, aTmpEnd, lcl_RstAttr, &aPara );
/*M*/ 	else if( !rRg.HasMark() )
/*M*/ 	{
/*M*/ 		aPara.bResetAll = FALSE;
/*M*/ 		::binfilter::lcl_RstAttr( &pStt->nNode.GetNode(), &aPara );
/*M*/ 		aPara.bResetAll = TRUE;
/*M*/ 	}
/*M*/
/*M*/ 	if( bTxtAttr )
/*M*/ 	{
/*M*/ 		if( bAdd )
/*M*/ 			aTmpEnd++;
/*M*/ 		GetNodes().ForEach( pStt->nNode, aTmpEnd, lcl_RstTxtAttr, &aPara );
/*M*/ 	}
/*M*/
/*M*/ 	if( pPam != &rRg )
/*M*/ 		delete pPam;
/*M*/
/*M*/ 	SetModified();
/*M*/ }



// Einfuegen der Hints nach Inhaltsformen;
// wird in SwDoc::Insert(..., SwFmtHint &rHt) benutzt

/*N*/ BOOL InsAttr( SwDoc *pDoc, const SwPaM &rRg, const SfxItemSet& rChgSet,
/*N*/               USHORT nFlags )
/*N*/ {
/*N*/ 	// teil die Sets auf (fuer Selektion in Nodes)
/*N*/ 	SfxItemSet aCharSet( pDoc->GetAttrPool(),
/*N*/ 						RES_CHRATR_BEGIN, RES_CHRATR_END-1,
/*N*/ 						RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
/*N*/ 						RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
/*N*/
/*N*/ 						RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
/*N*/ 						0 );
/*N*/ 	SfxItemSet aOtherSet( pDoc->GetAttrPool(),
/*N*/ 						RES_PARATR_BEGIN, RES_PARATR_END-1,
/*N*/ 						RES_FRMATR_BEGIN, RES_FRMATR_END-1,
/*N*/ 						RES_GRFATR_BEGIN, RES_GRFATR_END-1,
/*N*/ 						0 );
/*N*/
/*N*/ 	aCharSet.Put( rChgSet );
/*N*/ 	aOtherSet.Put( rChgSet );
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
/*N*/ 	SwCntntNode* pNode = pStt->nNode.GetNode().GetCntntNode();
/*N*/
/*N*/ 	if( pNode && pNode->IsTxtNode() )
/*N*/ 	{
/*N*/ 		const SwIndex& rSt = pStt->nContent;
/*N*/
/*N*/ 		// Attribute ohne Ende haben keinen Bereich
/*N*/ 		{
/*N*/ 			SfxItemSet aTxtSet( pDoc->GetAttrPool(),
/*N*/ 						RES_TXTATR_NOEND_BEGIN, RES_TXTATR_NOEND_END-1 );
/*N*/ 			aTxtSet.Put( rChgSet );
/*N*/ 			if( aTxtSet.Count() )
/*N*/ 			{
                    ((SwTxtNode*)pNode)->SetAttr( aTxtSet, rSt.GetIndex(), rSt.GetIndex(), nFlags );
/*N*/ 				bRet = TRUE;
/*N*/
/*N*/ 				if( pDoc->IsRedlineOn() || (!pDoc->IsIgnoreRedline() &&
/*N*/ 					pDoc->GetRedlineTbl().Count() ))
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aPam( pStt->nNode, pStt->nContent.GetIndex()-1,
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		// TextAttribute mit Ende expandieren nie ihren Bereich
/*N*/ 		{
/*N*/ 			// CharFmt wird gesondert behandelt !!!
/*N*/ 			// JP 22.08.96: URL-Attribute auch!!
/*N*/ 			SfxItemSet aTxtSet( pDoc->GetAttrPool(),
/*N*/ 								RES_TXTATR_DUMMY4, RES_TXTATR_TOXMARK,
/*N*/ 								RES_TXTATR_DUMMY5, RES_TXTATR_WITHEND_END-1,
/*N*/ 								0 );
/*N*/
/*N*/ 			aTxtSet.Put( rChgSet );
/*N*/ 			if( aTxtSet.Count() )
/*N*/ 			{
/*N*/ 				USHORT nInsCnt = rSt.GetIndex();
/*N*/ 				USHORT nEnd = pStt->nNode == pEnd->nNode
/*N*/ 								? pEnd->nContent.GetIndex()
/*N*/ 								: pNode->Len();
                    ((SwTxtNode*)pNode)->SetAttr( aTxtSet, nInsCnt, nEnd, nFlags );
/*N*/ 				bRet = TRUE;
/*N*/
/*N*/ 				if( pDoc->IsRedlineOn() || (!pDoc->IsIgnoreRedline() &&
/*N*/ 					 pDoc->GetRedlineTbl().Count() ) )
/*N*/ 				{
/*?*/ 					DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // wurde Text-Inhalt eingefuegt? (RefMark/TOXMarks ohne Ende)
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// bei PageDesc's, die am Node gesetzt werden, muss immer das
/*N*/ 	// Auto-Flag gesetzt werden!!
/*N*/ 	const SvxLRSpaceItem* pLRSpace = 0;
/*N*/ 	if( aOtherSet.Count() )
/*N*/ 	{
/*N*/ 		SwTableNode* pTblNd;
/*N*/ 		const SwFmtPageDesc* pDesc;
/*N*/ 		if( SFX_ITEM_SET == aOtherSet.GetItemState( RES_PAGEDESC,
/*N*/ 						FALSE, (const SfxPoolItem**)&pDesc ))
/*N*/ 		{
/*N*/ 			if( pNode )
/*N*/ 			{
/*N*/ 				// Auto-Flag setzen, nur in Vorlagen ist ohne Auto !
/*N*/ 				SwFmtPageDesc aNew( *pDesc );
/*N*/ 				// Bug 38479: AutoFlag wird jetzt in der WrtShell gesetzt
/*N*/ 				// aNew.SetAuto();
/*N*/
/*N*/ 				// Tabellen kennen jetzt auch Umbrueche
/*N*/ 				if( 0 == (nFlags & SETATTR_APICALL) &&
/*N*/ 					0 != ( pTblNd = pNode->FindTableNode() ) )
/*N*/ 				{
/*N*/ 					// dann am Tabellen Format setzen
/*?*/ 					SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
/*?*/ 					pFmt->SetAttr( aNew );
/*?*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					pNode->SetAttr( aNew );
/*N*/ 				}
/*N*/ 			}
/*N*/ 			aOtherSet.ClearItem( RES_PAGEDESC );
/*N*/ 			if( !aOtherSet.Count() )
/*N*/ 				return TRUE;
/*N*/ 		}
/*N*/
/*N*/ 		// Tabellen kennen jetzt auch Umbrueche
/*N*/ 		const SvxFmtBreakItem* pBreak;
/*N*/ 		if( pNode && 0 == (nFlags & SETATTR_APICALL) &&
/*N*/ 			0 != (pTblNd = pNode->FindTableNode() ) &&
/*N*/ 			SFX_ITEM_SET == aOtherSet.GetItemState( RES_BREAK,
/*N*/ 						FALSE, (const SfxPoolItem**)&pBreak ) )
/*N*/ 		{
/*?*/ 			// dann am Tabellen Format setzen
/*?*/ 			SwFrmFmt* pFmt = pTblNd->GetTable().GetFrmFmt();
/*?*/ 			pFmt->SetAttr( *pBreak );
/*?*/
/*?*/ 			aOtherSet.ClearItem( RES_BREAK );
/*?*/ 			if( !aOtherSet.Count() )
/*?*/ 				return TRUE;
/*N*/ 		}
/*N*/
/*N*/ 		// fuer Sonderbehandlung von LR-Space innerhalb einer Numerierung !!!
/*N*/ 		aOtherSet.GetItemState( RES_LR_SPACE, FALSE,
/*N*/ 								(const SfxPoolItem**)&pLRSpace );
/*N*/
/*N*/ 		{
/*N*/ 			// wenns eine PoolNumRule ist, diese ggfs. anlegen
/*N*/ 			const SwNumRuleItem* pRule;
/*N*/ 			USHORT nPoolId;
/*N*/ 			if( SFX_ITEM_SET == aOtherSet.GetItemState( RES_PARATR_NUMRULE,
/*N*/ 								FALSE, (const SfxPoolItem**)&pRule ) &&
/*N*/ 				!pDoc->FindNumRulePtr( pRule->GetValue() ) &&
/*N*/ 				USHRT_MAX != (nPoolId = SwStyleNameMapper::GetPoolIdFromUIName ( pRule->GetValue(),
/*N*/ 								GET_POOLID_NUMRULE )) )
/*?*/ 				pDoc->GetNumRuleFromPool( nPoolId );
/*N*/ 		}
/*N*/
/*N*/ 	}
/*N*/
/*N*/ 	if( !rRg.HasMark() )		// kein Bereich
/*N*/ 	{
/*N*/ 		if( !pNode )
/*N*/ 			return bRet;
/*N*/
/*N*/ 		if( pNode->IsTxtNode() && aCharSet.Count() )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwTxtNode* pTxtNd = (SwTxtNode*)pNode;
/*N*/ 		}
/*N*/ 		if( aOtherSet.Count() )
/*N*/ 		{
/*?*/ 			pNode->SetAttr( aOtherSet );
/*?*/ 			bRet = TRUE;
/*N*/ 		}
/*N*/ 		return bRet;
/*N*/ 	}
/*N*/
/*N*/ 	if( pDoc->IsRedlineOn() && aCharSet.Count() )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	if( pUndo )
/*N*/ 	}

    /* jetzt wenn Bereich */
/*N*/ 	ULONG nNodes = 0;
/*N*/
/*N*/ 	SwNodeIndex aSt( pDoc->GetNodes() );
/*N*/ 	SwNodeIndex aEnd( pDoc->GetNodes() );
/*N*/ 	SwIndex aCntEnd( pEnd->nContent );
/*N*/
/*N*/ 	if( pNode )
/*N*/ 	{
/*N*/ 		USHORT nLen = pNode->Len();
/*N*/ 		if( pStt->nNode != pEnd->nNode )
/*N*/ 			aCntEnd.Assign( pNode, nLen );
/*N*/
/*N*/ 		if( pStt->nContent.GetIndex() != 0 || aCntEnd.GetIndex() != nLen )
/*N*/ 		{
                ((SwTxtNode*)pNode)->SetAttr( aCharSet, pStt->nContent.GetIndex(), aCntEnd.GetIndex(), nFlags );
/*N*/ 			if( aOtherSet.Count() )
/*N*/ 			{
/*N*/ 				pNode->SetAttr( aOtherSet );
/*N*/ 			}
/*N*/
/*N*/ 			// lediglich Selektion in einem Node.
/*N*/ 			if( pStt->nNode == pEnd->nNode )
/*N*/ 				return TRUE;
/*N*/ 			++nNodes;
/*?*/ 			aSt.Assign( pStt->nNode.GetNode(), +1 );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			aSt = pStt->nNode;
/*N*/ 		aCntEnd = pEnd->nContent; // aEnd wurde veraendert !!
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aSt.Assign( pStt->nNode.GetNode(), +1 );
/*N*/
/*N*/ 	// aSt zeigt jetzt auf den ersten vollen Node
/*N*/
    /*
      * die Selektion umfasst mehr als einen Node
      */
/*N*/ 	if( pStt->nNode < pEnd->nNode )
/*N*/ 	{
/*N*/ 		pNode = pEnd->nNode.GetNode().GetCntntNode();
/*N*/ 		if(pNode)
/*N*/ 		{
/*N*/ 			USHORT nLen = pNode->Len();
/*N*/ 			if( aCntEnd.GetIndex() != nLen )
/*N*/ 			{
                    ((SwTxtNode*)pNode)->SetAttr( aCharSet, 0, aCntEnd.GetIndex(), nFlags );
/*N*/ 				if( aOtherSet.Count() )
/*N*/ 				{
/*N*/ 					pNode->SetAttr( aOtherSet );
/*N*/ 				}
/*N*/
/*N*/ 				++nNodes;
/*N*/ 				aEnd = pEnd->nNode;
/*N*/ 			}
/*N*/ 			else
/*?*/ 				aEnd.Assign( pEnd->nNode.GetNode(), +1 );
/*N*/ 		}
/*N*/ 		else
/*?*/ 			aEnd = pEnd->nNode;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aEnd.Assign( pEnd->nNode.GetNode(), +1 );
/*N*/
/*N*/ 	// aEnd zeigt jetzt HINTER den letzten voll Node
/*N*/
/*N*/ 	/* Bearbeitung der vollstaendig selektierten Nodes. */
/*N*/ // alle Attribute aus dem Set zuruecksetzen !!
/*N*/ 	if( aCharSet.Count() && !( SETATTR_DONTREPLACE & nFlags ) )
/*N*/ 	{
/*N*/
/*N*/       ParaRstFmt aPara( pStt, pEnd, 0, &aCharSet );
/*N*/ 		pDoc->GetNodes().ForEach( aSt, aEnd, lcl_RstTxtAttr, &aPara );
/*N*/ 	}
/*N*/
/*N*/ #ifdef USED
/*N*/ //JP 30.10.96: siehe unten
/*N*/ 	// sollte ueber mehrere Nodes das SwFmtChrFmt gesetzt werden ??
/*N*/ 	const SfxPoolItem* pChrFmtItem = 0;
/*N*/ 	aCharSet.GetItemState( RES_TXTATR_CHARFMT, FALSE, &pChrFmtItem );
/*N*/ #endif
/*N*/ 	BOOL bCreateSwpHints =
/*N*/ 		SFX_ITEM_SET == aCharSet.GetItemState( RES_TXTATR_CHARFMT, FALSE ) ||
/*N*/ 		SFX_ITEM_SET == aCharSet.GetItemState( RES_TXTATR_INETFMT, FALSE );
/*N*/
/*N*/ 	for(; aSt < aEnd; aSt++ )
/*N*/ 	{
/*N*/ 		pNode = aSt.GetNode().GetCntntNode();
/*N*/ 		if( !pNode )
/*N*/ 			continue;
/*N*/
/*N*/ 		SwTxtNode* pTNd = pNode->GetTxtNode();
/*N*/ 		{
/*N*/ 			if( pTNd && aCharSet.Count() )
/*?*/ 				pTNd->SetAttr( aCharSet, 0, pTNd->GetTxt().Len(), nFlags );
/*N*/ 			if( aOtherSet.Count() )
/*N*/ 				pNode->SetAttr( aOtherSet );
/*N*/ 		}
/*N*/ 		++nNodes;
/*N*/ 	}
/*N*/
/*N*/ 	return nNodes != 0;
/*N*/ }


/*N*/ BOOL SwDoc::Insert( const SwPaM &rRg, const SfxPoolItem &rHt, USHORT nFlags )
/*N*/ {
/*N*/ 	SwDataChanged aTmp( rRg, 0 );
/*N*/ 	BOOL bRet;
/*N*/
/*N*/ 	SfxItemSet aSet( GetAttrPool(), rHt.Which(), rHt.Which() );
/*N*/ 	aSet.Put( rHt );
/*N*/   bRet = InsAttr( this, rRg, aSet, nFlags );

/*	if( INSATTR_DONTEXPAND & nFlags )
    {
        USHORT nWhich = rHt.Which();
        const SwPosition* pPos = rRg.End();
        SwTxtNode* pTxtNd = GetNodes()[ pPos->nNode ]->GetTxtNode();
        SwpHints* pHts;
        if( pTxtNd && 0 != ( pHts = pTxtNd->GetpSwpHints()) )
        {
            USHORT nPos = pHts->GetEndCount();
            while( nPos )
            {
                SwTxtAttr *pTmp = pHts->GetEnd( --nPos );
                USHORT *pEnd = pTmp->GetEnd();
                if( !pEnd || *pEnd > nEnd )
                    continue;
                if( nEnd != *pEnd )
                    nPos = 0;
                else if( nWhich == pTmp->Which() )
                    pTmp->SetDontExpand( TRUE );
            }
        }
    }
*/
/*N*/ 	if( bRet )
/*N*/ 		SetModified();
/*N*/ 	return bRet;
/*N*/ }

/*N*/ BOOL SwDoc::Insert( const SwPaM &rRg, const SfxItemSet &rSet, USHORT nFlags )
/*N*/ {
/*N*/ 	SwDataChanged aTmp( rRg, 0 );
/*N*/   BOOL bRet = InsAttr( this, rRg, rSet, nFlags );
/*N*/
/*N*/ 	if( bRet )
/*N*/ 		SetModified();
/*N*/ 	return bRet;
/*N*/ }


    // Setze das Attribut im angegebenen Format. Ist Undo aktiv, wird
    // das alte in die Undo-History aufgenommen
/*N*/ void SwDoc::SetAttr( const SfxPoolItem& rAttr, SwFmt& rFmt )
/*N*/ {
/*N*/ 	SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
/*N*/ 	aSet.Put( rAttr );
/*N*/ 	SetAttr( aSet, rFmt );
/*N*/ }


    // Setze das Attribut im angegebenen Format. Ist Undo aktiv, wird
    // das alte in die Undo-History aufgenommen
/*N*/ void SwDoc::SetAttr( const SfxItemSet& rSet, SwFmt& rFmt )
/*N*/ {
/*?*/ 		rFmt.SetAttr( rSet );
/*N*/ 	SetModified();
/*N*/ }

/*N*/ int lcl_SetNewDefTabStops( SwTwips nOldWidth, SwTwips nNewWidth,
/*N*/ 								SvxTabStopItem& rChgTabStop )
/*N*/ {
/*N*/ 	// dann aender bei allen TabStop die default's auf den neuen Wert
/*N*/ 	// !!! Achtung: hier wird immer auf dem PoolAttribut gearbeitet,
/*N*/ 	// 				damit nicht in allen Sets die gleiche Berechnung
/*N*/ 	//				auf dem gleichen TabStop (gepoolt!) vorgenommen
/*N*/ 	//				wird. Als Modify wird ein FmtChg verschickt.
/*N*/
/*N*/ 	USHORT nOldCnt = rChgTabStop.Count();
/*N*/ 	if( !nOldCnt || nOldWidth == nNewWidth )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	// suche den Anfang der Defaults
/*N*/ 	SvxTabStop* pTabs = ((SvxTabStop*)rChgTabStop.GetStart())
/*N*/ 						+ (nOldCnt-1);
        USHORT n;
/*N*/ 	for( n = nOldCnt; n ; --n, --pTabs )
/*N*/ 		if( SVX_TAB_ADJUST_DEFAULT != pTabs->GetAdjustment() )
/*N*/ 			break;
/*N*/ 	++n;
/*N*/ 	if( n < nOldCnt )	// die DefTabStops loeschen
/*?*/ 		rChgTabStop.Remove( n, nOldCnt - n );
/*N*/ 	return TRUE;
/*N*/ }

    // Setze das Attribut als neues default Attribut in diesem Dokument.
    // Ist Undo aktiv, wird das alte in die Undo-History aufgenommen
/*N*/ void SwDoc::SetDefault( const SfxPoolItem& rAttr )
/*N*/ {
/*N*/ 	SfxItemSet aSet( GetAttrPool(), rAttr.Which(), rAttr.Which() );
/*N*/ 	aSet.Put( rAttr );
/*N*/ 	SetDefault( aSet );
/*N*/ }

/*N*/ void SwDoc::SetDefault( const SfxItemSet& rSet )
/*N*/ {
/*N*/ 	if( !rSet.Count() )
/*N*/ 		return;
/*N*/
/*N*/ 	SwModify aCallMod( 0 );
/*N*/ 	SwAttrSet aOld( GetAttrPool(), rSet.GetRanges() ),
/*N*/ 			aNew( GetAttrPool(), rSet.GetRanges() );
/*N*/ 	SfxItemIter aIter( rSet );
/*N*/ 	register USHORT nWhich;
/*N*/ 	const SfxPoolItem* pItem = aIter.GetCurItem();
/*N*/ 	SfxItemPool* pSdrPool = GetAttrPool().GetSecondaryPool();
/*N*/ 	while( TRUE )
/*N*/ 	{
/*N*/ 		BOOL bCheckSdrDflt = FALSE;
/*N*/ 		nWhich = pItem->Which();
/*N*/ 		aOld.Put( GetAttrPool().GetDefaultItem( nWhich ) );
/*N*/ 		GetAttrPool().SetPoolDefaultItem( *pItem );
/*N*/ 		aNew.Put( GetAttrPool().GetDefaultItem( nWhich ) );
/*N*/
/*N*/ 		if( RES_CHRATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich )
/*N*/ 		{
/*N*/ 			aCallMod.Add( pDfltTxtFmtColl );
/*N*/ 			aCallMod.Add( pDfltCharFmt );
/*N*/ 			bCheckSdrDflt = 0 != pSdrPool;
/*N*/ 		}
/*N*/ 		else if( RES_PARATR_BEGIN <= nWhich && RES_PARATR_END > nWhich )
/*N*/ 		{
/*N*/ 			aCallMod.Add( pDfltTxtFmtColl );
/*N*/ 			bCheckSdrDflt = 0 != pSdrPool;
/*N*/ 		}
/*N*/ 		else if( RES_GRFATR_BEGIN <= nWhich && RES_GRFATR_END > nWhich )
/*?*/ 			aCallMod.Add( pDfltGrfFmtColl );
/*?*/ 		else if( RES_FRMATR_BEGIN <= nWhich && RES_FRMATR_END > nWhich )
/*?*/ 		{
/*?*/ 			aCallMod.Add( pDfltGrfFmtColl );
/*?*/ 			aCallMod.Add( pDfltTxtFmtColl );
/*?*/ 			aCallMod.Add( pDfltFrmFmt );
/*?*/ 		}
/*?*/ 		else if( RES_BOXATR_BEGIN <= nWhich && RES_BOXATR_END > nWhich )
/*?*/ 			aCallMod.Add( pDfltFrmFmt );

        // copy also the defaults
/*N*/ 		if( bCheckSdrDflt )
/*N*/ 		{
/*N*/ 			USHORT nEdtWhich, nSlotId;
/*N*/ 			if( 0 != (nSlotId = GetAttrPool().GetSlotId( nWhich ) ) &&
/*N*/ 				nSlotId != nWhich &&
/*N*/ 				0 != (nEdtWhich = pSdrPool->GetWhich( nSlotId )) &&
/*N*/ 				nSlotId != nEdtWhich )
/*N*/ 			{
/*N*/ 				SfxPoolItem* pCpy = pItem->Clone();
/*N*/ 				pCpy->SetWhich( nEdtWhich );
/*N*/ 				pSdrPool->SetPoolDefaultItem( *pCpy );
/*N*/ 				delete pCpy;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if( aIter.IsAtEnd() )
/*N*/ 			break;
/*N*/ 		pItem = aIter.NextItem();
/*N*/ 	}
/*N*/
/*N*/ 	if( aNew.Count() && aCallMod.GetDepends() )
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		if( ( SFX_ITEM_SET ==
/*N*/                 aNew.GetItemState( RES_PARATR_TABSTOP, FALSE, &pItem ) ) &&
/*N*/ 			((SvxTabStopItem*)pItem)->Count() )
/*N*/ 		{
/*N*/ 			// TabStop-Aenderungen behandeln wir erstmal anders:
/*N*/ 			// dann aender bei allen TabStop die dafault's auf den neuen Wert
/*N*/ 			// !!! Achtung: hier wird immer auf dem PoolAttribut gearbeitet,
/*N*/ 			// 				damit nicht in allen Sets die gleiche Berechnung
/*N*/ 			//				auf dem gleichen TabStop (gepoolt!) vorgenommen
/*N*/ 			//				wird. Als Modify wird ein FmtChg verschickt.
/*N*/ 			SwTwips nNewWidth = (*(SvxTabStopItem*)pItem)[ 0 ].GetTabPos(),
/*N*/ 					nOldWidth = ((SvxTabStopItem&)aOld.Get(RES_PARATR_TABSTOP))[ 0 ].GetTabPos();
/*N*/
/*N*/ 			int bChg = FALSE;
/*N*/ 			USHORT nMaxItems = GetAttrPool().GetItemCount( RES_PARATR_TABSTOP );
/*N*/ 			for( USHORT n = 0; n < nMaxItems; ++n )
/*N*/ 				if( 0 != (pItem = GetAttrPool().GetItem( RES_PARATR_TABSTOP, n ) ))
/*N*/ 					bChg |= lcl_SetNewDefTabStops( nOldWidth, nNewWidth,
/*N*/ 												*(SvxTabStopItem*)pItem );
/*N*/
/*N*/ 			aNew.ClearItem( RES_PARATR_TABSTOP );
/*N*/ 			aOld.ClearItem( RES_PARATR_TABSTOP );
/*N*/ 			if( bChg )
/*N*/ 			{
/*N*/ 				SwFmtChg aChgFmt( pDfltCharFmt );
/*N*/ 				// dann sage mal den Frames bescheid
/*N*/ 				aCallMod.Modify( &aChgFmt, &aChgFmt );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if( aNew.Count() && aCallMod.GetDepends() )
/*N*/ 	{
/*N*/ 		SwAttrSetChg aChgOld( aOld, aOld );
/*N*/ 		SwAttrSetChg aChgNew( aNew, aNew );
/*N*/ 		aCallMod.Modify( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
/*N*/ 	}
/*N*/
/*N*/ 	// und die default-Formate wieder beim Object austragen
/*N*/ 	SwClient* pDep;
/*N*/ 	while( 0 != ( pDep = (SwClient*)aCallMod.GetDepends()) )
/*N*/ 		aCallMod.Remove( pDep );
/*N*/
/*N*/ 	SetModified();
/*N*/ }

    // Erfrage das Default Attribut in diesem Dokument.
/*N*/ const SfxPoolItem& SwDoc::GetDefault( USHORT nFmtHint ) const
/*N*/ {
/*N*/ 	return GetAttrPool().GetDefaultItem( nFmtHint );
/*N*/ }

/*
 * Loeschen der Formate
 */
void SwDoc::DelCharFmt(USHORT nFmt)
{
    pCharFmtTbl->DeleteAndDestroy(nFmt);
    SetModified();
}

void SwDoc::DelCharFmt( SwCharFmt *pFmt )
{
    USHORT nFmt = pCharFmtTbl->GetPos( pFmt );
    ASSERT( USHRT_MAX != nFmt, "Fmt not found," );
    DelCharFmt( nFmt );
}

/*N*/ void SwDoc::DelFrmFmt( SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	if( pFmt->ISA( SwTableBoxFmt ) || pFmt->ISA( SwTableLineFmt ))
/*N*/ 	{
/*?*/ 		ASSERT( !this, "Format steht nicht mehr im DocArray, "
/*?*/ 					   "kann per delete geloescht werden" );
/*?*/ 		delete pFmt;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		//Das Format muss in einem der beiden Arrays stehen, in welchem
/*N*/ 		//werden wir schon merken.
/*N*/ 		USHORT nPos;
/*N*/ 		if ( USHRT_MAX != ( nPos = pFrmFmtTbl->GetPos( pFmt )) )
/*N*/ 			pFrmFmtTbl->DeleteAndDestroy( nPos );
/*N*/ 		else
/*N*/ 		{
/*N*/ 			nPos = GetSpzFrmFmts()->GetPos( pFmt );
/*N*/ 			ASSERT( nPos != USHRT_MAX, "FrmFmt not found." );
/*N*/ 			if( USHRT_MAX != nPos )
/*N*/ 				GetSpzFrmFmts()->DeleteAndDestroy( nPos );
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::DelTblFrmFmt( SwTableFmt *pFmt )
/*N*/ {
/*N*/ 	USHORT nPos = pTblFrmFmtTbl->GetPos( pFmt );
/*N*/ 	ASSERT( USHRT_MAX != nPos, "Fmt not found," );
/*N*/ 	pTblFrmFmtTbl->DeleteAndDestroy( nPos );
/*N*/ }

/*
 * Erzeugen der Formate
 */
/*N*/ SwFlyFrmFmt *SwDoc::MakeFlyFrmFmt( const String &rFmtName,
/*N*/ 									SwFrmFmt *pDerivedFrom )
/*N*/ {
/*N*/ 	SwFlyFrmFmt *pFmt = new SwFlyFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );
/*N*/ 	GetSpzFrmFmts()->Insert(pFmt, GetSpzFrmFmts()->Count());
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ SwDrawFrmFmt *SwDoc::MakeDrawFrmFmt( const String &rFmtName,
/*N*/ 									 SwFrmFmt *pDerivedFrom )
/*N*/ {
/*N*/ 	SwDrawFrmFmt *pFmt = new SwDrawFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom);
/*N*/ 	GetSpzFrmFmts()->Insert(pFmt,GetSpzFrmFmts()->Count());
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }


/*N*/ USHORT SwDoc::GetTblFrmFmtCount(BOOL bUsed) const
/*N*/ {
/*?*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	USHORT nCount = pTblFrmFmtTbl->Count();
/*N*/ }


/*N*/ SwFrmFmt& SwDoc::GetTblFrmFmt(USHORT nFmt, BOOL bUsed ) const
/*N*/ {
/*?*/	USHORT nRemoved = 0;
/*?*/	if(bUsed)
/*?*/	{
/*?*/		SwAutoFmtGetDocNode aGetHt( &aNodes );
/*?*/		for ( USHORT i = 0; i <= nFmt; i++ )
/*?*/		{
/*?*/			while ( (*pTblFrmFmtTbl)[ i + nRemoved]->GetInfo( aGetHt ))
/*?*/			{
/*?*/				nRemoved++;
/*?*/			}
/*?*/		}
/*?*/	}
/*?*/	return *((*pTblFrmFmtTbl)[nRemoved + nFmt]);
/*N*/}

/*N*/ SwTableFmt* SwDoc::MakeTblFrmFmt( const String &rFmtName,
/*N*/ 									SwFrmFmt *pDerivedFrom )
/*N*/ {
/*N*/ 	SwTableFmt* pFmt = new SwTableFmt( GetAttrPool(), rFmtName, pDerivedFrom );
/*N*/ 	pTblFrmFmtTbl->Insert( pFmt, pTblFrmFmtTbl->Count() );
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ SwFrmFmt *SwDoc::MakeFrmFmt(const String &rFmtName,
/*N*/ 							SwFrmFmt *pDerivedFrom)
/*N*/ {
/*N*/ 	SwFrmFmt *pFmt = new SwFrmFmt( GetAttrPool(), rFmtName, pDerivedFrom );
/*N*/ 	pFrmFmtTbl->Insert( pFmt, pFrmFmtTbl->Count());
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ SwCharFmt *SwDoc::MakeCharFmt( const String &rFmtName,
/*N*/ 								SwCharFmt *pDerivedFrom)
/*N*/ {
/*N*/ 	SwCharFmt *pFmt = new SwCharFmt( GetAttrPool(), rFmtName, pDerivedFrom );
/*N*/ 	pCharFmtTbl->Insert( pFmt, pCharFmtTbl->Count() );
/*N*/ 	pFmt->SetAuto( FALSE );
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }


/*
 * Erzeugen der FormatCollections
 */
// TXT

/*N*/ SwTxtFmtColl* SwDoc::MakeTxtFmtColl( const String &rFmtName,
/*N*/ 									 SwTxtFmtColl *pDerivedFrom)
/*N*/ {
/*N*/ 	SwTxtFmtColl *pFmtColl = new SwTxtFmtColl( GetAttrPool(), rFmtName,
/*N*/ 												pDerivedFrom );
/*N*/ 	pTxtFmtCollTbl->Insert(pFmtColl, pTxtFmtCollTbl->Count());
/*N*/ 	pFmtColl->SetAuto( FALSE );
/*N*/ 	SetModified();
/*N*/ 	return pFmtColl;
/*N*/ }

//FEATURE::CONDCOLL
/*NBFF*/SwConditionTxtFmtColl* SwDoc::MakeCondTxtFmtColl( const String &rFmtName,
/*NBFF*/												SwTxtFmtColl *pDerivedFrom )
/*NBFF*/{
/*NBFF*/	SwConditionTxtFmtColl*pFmtColl = new SwConditionTxtFmtColl( GetAttrPool(),
/*NBFF*/													rFmtName, pDerivedFrom );
/*NBFF*/	pTxtFmtCollTbl->Insert(pFmtColl, pTxtFmtCollTbl->Count());
/*NBFF*/	pFmtColl->SetAuto( FALSE );
/*NBFF*/	SetModified();
/*NBFF*/	return pFmtColl;
/*NBFF*/}
//FEATURE::CONDCOLL

// GRF






/*N*/ BOOL lcl_SetTxtFmtColl( const SwNodePtr& rpNode, void* pArgs )
/*N*/ {
/*N*/ 	// ParaSetFmtColl * pPara = (ParaSetFmtColl*)pArgs;
/*N*/ 	SwCntntNode* pCNd = (SwCntntNode*)rpNode->GetTxtNode();
/*N*/ 	if( pCNd )
/*N*/ 	{
/*N*/ 		ParaRstFmt* pPara = (ParaRstFmt*)pArgs;
/*N*/ 		if ( pPara->bReset )
/*N*/ 			lcl_RstAttr( pCNd, pPara );
/*N*/
/*N*/ 		// erst in die History aufnehmen, damit ggfs. alte Daten
/*N*/ 		// gesichert werden koennen
/*N*/ 		pCNd->ChgFmtColl( pPara->pFmtColl );
/*N*/ 		pPara->nWhich++;
/*N*/ 	}
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL SwDoc::SetTxtFmtColl(const SwPaM &rRg, SwTxtFmtColl *pFmt, BOOL bReset)
/*N*/ {
/*N*/ 	SwDataChanged aTmp( rRg, 0 );
/*N*/ 	const SwPosition *pStt = rRg.Start(), *pEnd = rRg.End();
/*N*/ 	BOOL bRet = TRUE;
/*N*/
/*N*/ 	if( rRg.HasMark() )
/*N*/ 	{
/*N*/       ParaRstFmt aPara( pStt, pEnd );
/*N*/ 		aPara.pFmtColl = pFmt;
/*N*/ 		aPara.bReset = bReset;
/*N*/ 		GetNodes().ForEach( pStt->nNode.GetIndex(), pEnd->nNode.GetIndex()+1,
/*N*/ 							lcl_SetTxtFmtColl, &aPara );
/*N*/ 		if( !aPara.nWhich )
/*N*/ 			bRet = FALSE;			// keinen gueltigen Node gefunden
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// ein enzelner Node:
/*N*/ 		SwCntntNode* pCNd = rRg.GetPoint()->nNode.GetNode().GetCntntNode();
/*N*/ 		if( pCNd )
/*N*/ 		{
/*N*/ 			if( bReset && pCNd->GetpSwAttrSet() )
/*N*/ 			{
/*?*/               ParaRstFmt aPara;
/*?*/ 				aPara.pFmtColl = pFmt;
/*?*/ 				lcl_RstAttr( pCNd, &aPara );
/*N*/ 			}
/*N*/
/*N*/ 			pCNd->ChgFmtColl( pFmt );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			bRet = FALSE;
/*N*/ 	}
/*N*/
/*N*/ 	if( bRet )
/*N*/ 		SetModified();
/*N*/ 	return bRet;
/*N*/ }


// ---- Kopiere die Formate in sich selbst (SwDoc) ----------------------

/*N*/ SwFmt* SwDoc::CopyFmt( const SwFmt& rFmt,
/*N*/ 						const SvPtrarr& rFmtArr,
/*N*/ 						FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt )
/*N*/ {
/*N*/ 	//  kein-Autoformat || default Format || Collection-Format
/*N*/ 	// dann suche danach.
/*N*/ 	if( !rFmt.IsAuto() || !rFmt.GetRegisteredIn() )
/*N*/ 		for( USHORT n = 0; n < rFmtArr.Count(); n++ )
/*N*/ 		{
/*N*/ 			// ist die Vorlage schon im Doc vorhanden ??
/*N*/ 			if( ((SwFmt*)rFmtArr[n])->GetName().Equals( rFmt.GetName() ))
/*N*/ 				return (SwFmt*)rFmtArr[n];
/*N*/ 		}
/*N*/
/*N*/ 	// suche erstmal nach dem "Parent"
/*N*/ 	SwFmt* pParent = (SwFmt*)&rDfltFmt;
/*N*/ 	if( rFmt.DerivedFrom() && pParent != rFmt.DerivedFrom() )
/*N*/ 		pParent = CopyFmt( *rFmt.DerivedFrom(), rFmtArr,
/*N*/ 								fnCopyFmt, rDfltFmt );
/*N*/
/*N*/ 	// erzeuge das Format und kopiere die Attribute
/*?*/ 	SwFmt* pNewFmt = (this->*fnCopyFmt)( rFmt.GetName(), pParent );
/*?*/ 	pNewFmt->SetAuto( rFmt.IsAuto() );
/*?*/ 	pNewFmt->CopyAttrs( rFmt, TRUE );			// kopiere Attribute
/*?*/
/*?*/ 	pNewFmt->SetPoolFmtId( rFmt.GetPoolFmtId() );
/*?*/ 	pNewFmt->SetPoolHelpId( rFmt.GetPoolHelpId() );
/*?*/
/*?*/ 	// HelpFile-Id immer auf dflt setzen !!
/*?*/ 	pNewFmt->SetPoolHlpFileId( UCHAR_MAX );
/*?*/
/*?*/ 	return pNewFmt;
/*N*/ }

// ---- kopiere das Frame-Format --------
/*N*/ SwFrmFmt* SwDoc::CopyFrmFmt( const SwFrmFmt& rFmt )
/*N*/ {
/*N*/ 	return (SwFrmFmt*)CopyFmt( rFmt, *GetFrmFmts(),
/*N*/ 								(FNCopyFmt)&SwDoc::MakeFrmFmt,
/*N*/ 								*GetDfltFrmFmt() );
/*N*/ }

// ---- kopiere das Char-Format --------
/*N*/ SwCharFmt* SwDoc::CopyCharFmt( const SwCharFmt& rFmt )
/*N*/ {
/*N*/ 	return (SwCharFmt*)CopyFmt( rFmt, *GetCharFmts(),
/*N*/ 								(FNCopyFmt)&SwDoc::MakeCharFmt,
/*N*/ 								*GetDfltCharFmt() );
/*N*/ }


// --- Kopiere TextNodes ----

/*N*/ SwTxtFmtColl* SwDoc::CopyTxtColl( const SwTxtFmtColl& rColl )
/*N*/ {
/*N*/ 	SwTxtFmtColl* pNewColl = FindTxtFmtCollByName( rColl.GetName() );
/*N*/ 	if( pNewColl )
/*N*/ 		return pNewColl;

    // suche erstmal nach dem "Parent"
/*?*/ 	SwTxtFmtColl* pParent = pDfltTxtFmtColl;
/*?*/ 	if( pParent != rColl.DerivedFrom() )
/*?*/ 		pParent = CopyTxtColl( *(SwTxtFmtColl*)rColl.DerivedFrom() );
/*?*/

//FEATURE::CONDCOLL
/*?*/ 	if( RES_CONDTXTFMTCOLL == rColl.Which() )
/*?*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	pNewColl = new SwConditionTxtFmtColl( GetAttrPool(), rColl.GetName(),
/*?*/ 	}
/*?*/ 	else
/*?*/ //FEATURE::CONDCOLL
/*?*/ 		pNewColl = MakeTxtFmtColl( rColl.GetName(), pParent );
/*?*/
/*?*/ 	// kopiere jetzt noch die Auto-Formate oder kopiere die Attribute
/*?*/ 	pNewColl->CopyAttrs( rColl, TRUE );
/*?*/
/*?*/ 	// setze noch den Outline-Level
/*?*/ 	if( NO_NUMBERING != rColl.GetOutlineLevel() )
/*?*/ 		pNewColl->SetOutlineLevel( rColl.GetOutlineLevel() );
/*?*/
/*?*/ 	pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
/*?*/ 	pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );
/*?*/
/*?*/ 	// HelpFile-Id immer auf dflt setzen !!
/*?*/ 	pNewColl->SetPoolHlpFileId( UCHAR_MAX );
/*?*/
/*?*/ 	if( &rColl.GetNextTxtFmtColl() != &rColl )
/*?*/ 		pNewColl->SetNextTxtFmtColl( *CopyTxtColl( rColl.GetNextTxtFmtColl() ));
/*?*/
/*?*/ 	// ggfs. die NumRule erzeugen
/*?*/ 	if( this != rColl.GetDoc() )
/*?*/ 	{
/*?*/ 		const SfxPoolItem* pItem;
/*?*/ 		if( SFX_ITEM_SET == pNewColl->GetItemState( RES_PARATR_NUMRULE,
/*?*/ 			FALSE, &pItem ))
/*?*/ 		{
/*?*/ 			const SwNumRule* pRule;
/*?*/ 			const String& rName = ((SwNumRuleItem*)pItem)->GetValue();
/*?*/ 			if( rName.Len() &&
/*?*/ 				0 != ( pRule = rColl.GetDoc()->FindNumRulePtr( rName )) &&
/*?*/ 				!pRule->IsAutoRule() )
/*?*/ 			{
/*?*/ 				SwNumRule* pDestRule = FindNumRulePtr( rName );
/*?*/ 				if( pDestRule )
/*?*/ 					pDestRule->SetInvalidRule( TRUE );
/*?*/ 				else
/*?*/ 					MakeNumRule( rName, pRule );
/*?*/ 			}
/*?*/ 		}
/*?*/ 	}
/*?*/ 	return pNewColl;
/*N*/ }

// --- Kopiere GrafikNodes ----

/*N*/ SwGrfFmtColl* SwDoc::CopyGrfColl( const SwGrfFmtColl& rColl )
/*N*/ {
/*N*/ 	SwGrfFmtColl* pNewColl = FindGrfFmtCollByName( rColl.GetName() );
/*N*/ 	if( pNewColl )
/*N*/ 		return pNewColl;

    // suche erstmal nach dem "Parent"
/*?*/ 	SwGrfFmtColl* pParent = pDfltGrfFmtColl;
/*?*/ 	if( pParent != rColl.DerivedFrom() )
/*?*/ 		pParent = CopyGrfColl( *(SwGrfFmtColl*)rColl.DerivedFrom() );
/*?*/
/*?*/ 	// falls nicht, so kopiere sie
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pNewColl = MakeGrfFmtColl( rColl.GetName(), pParent );
/*?*/
/*?*/ 	// noch die Attribute kopieren
/*?*/ 	pNewColl->CopyAttrs( rColl );
/*?*/
/*?*/ 	pNewColl->SetPoolFmtId( rColl.GetPoolFmtId() );
/*?*/ 	pNewColl->SetPoolHelpId( rColl.GetPoolHelpId() );
/*?*/
/*?*/ 	// HelpFile-Id immer auf dflt setzen !!
/*?*/ 	pNewColl->SetPoolHlpFileId( UCHAR_MAX );
/*?*/
/*?*/ 	return pNewColl;
/*N*/ }


/*N*/ SwFmt* SwDoc::FindFmtByName( const SvPtrarr& rFmtArr,
/*N*/ 									const String& rName ) const
/*N*/ {
/*N*/ 	SwFmt* pFnd = 0;
/*N*/ 	for( USHORT n = 0; n < rFmtArr.Count(); n++ )
/*N*/ 	{
/*N*/ 		// ist die Vorlage schon im Doc vorhanden ??
/*N*/ 		if( ((SwFmt*)rFmtArr[n])->GetName() == rName )
/*N*/ 		{
/*N*/ 			pFnd = (SwFmt*)rFmtArr[n];
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return pFnd;
/*N*/ }



/*N*/ SwTableBoxFmt* SwDoc::MakeTableBoxFmt()
/*N*/ {
/*N*/ 	SwTableBoxFmt* pFmt = new SwTableBoxFmt( GetAttrPool(), aEmptyStr,
/*N*/ 												pDfltFrmFmt );
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ SwTableLineFmt* SwDoc::MakeTableLineFmt()
/*N*/ {
/*N*/ 	SwTableLineFmt* pFmt = new SwTableLineFmt( GetAttrPool(), aEmptyStr,
/*N*/ 												pDfltFrmFmt );
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ void SwDoc::_CreateNumberFormatter()
/*N*/ {
/*N*/ 	RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "SW", "JP93722",  "SwDoc::_CreateNumberFormatter" );
/*N*/
/*N*/ 	ASSERT( !pNumberFormatter, "ist doch schon vorhanden" );
/*N*/
/*N*/
/*N*/ 	LanguageType eLang = LANGUAGE_SYSTEM; //System::GetLanguage();
/*				((const SvxLanguageItem&)GetAttrPool().
                    GetDefaultItem( RES_CHRATR_LANGUAGE )).GetLanguage();
*/
/*N*/ 	Reference< XMultiServiceFactory > xMSF = ::legacy_binfilters::getLegacyProcessServiceFactory();
/*N*/ 	pNumberFormatter = new SvNumberFormatter( xMSF, eLang );
/*N*/ 	pNumberFormatter->SetEvalDateFormat( NF_EVALDATEFORMAT_FORMAT_INTL );
/*N*/ 	pNumberFormatter->SetYear2000(SFX_APP()->GetMiscConfig()->GetYear2000());
/*N*/
/*N*/ }

/*N*/ SwTblNumFmtMerge::SwTblNumFmtMerge( const SwDoc& rSrc, SwDoc& rDest )
/*N*/ 	: pNFmt( 0 )
/*N*/ {
/*N*/ 	// ein anderes Doc -> Numberformatter muessen gemergt werden
/*N*/ 	SvNumberFormatter* pN;
/*N*/ 	if( &rSrc != &rDest && 0 != ( pN = ((SwDoc&)rSrc).GetNumberFormatter( FALSE ) ))
/*N*/ 		( pNFmt = rDest.GetNumberFormatter( TRUE ))->MergeFormatter( *pN );
/*N*/
/*N*/ 	if( &rSrc != &rDest )
/*N*/ 		((SwGetRefFieldType*)rSrc.GetSysFldType( RES_GETREFFLD ))->
/*N*/ 			MergeWithOtherDoc( rDest );
/*N*/ }

/*N*/ SwTblNumFmtMerge::~SwTblNumFmtMerge()
/*N*/ {
/*N*/ 	if( pNFmt )
/*N*/ 		pNFmt->ClearMergeTable();
/*N*/ }




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
