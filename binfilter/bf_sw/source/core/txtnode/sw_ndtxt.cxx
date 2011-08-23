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

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_XINPUTSEQUENCECHECKER_HPP_
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <bf_svx/lrspitem.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <bf_svtools/urihelper.hxx>
#endif
#ifndef _SVTOOLS_CTLOPTIONS_HXX
#include <bf_svtools/ctloptions.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif

#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FTNIDX_HXX //autogen
#include <ftnidx.hxx>
#endif
#ifndef _FTNINFO_HXX //autogen
#include <ftninfo.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _FTNFRM_HXX
#include <ftnfrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>				// fuer SwFmtChg in ChgTxtColl
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>				// fuer SwTblField
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _SWCACHE_HXX
#include <swcache.hxx>
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>				// fuer die WrongList des OnlineSpellings
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _CHECKIT_HXX
#include <checkit.hxx>
#endif
namespace binfilter {


SV_DECL_PTRARR( TmpHints, SwTxtAttr*, 0, 4 )

/*N*/ TYPEINIT1( SwTxtNode, SwCntntNode )

SV_DECL_PTRARR(SwpHts,SwTxtAttr*,1,1)

// Leider ist das SwpHints nicht ganz wasserdicht:
// Jeder darf an den Hints rumfummeln, ohne die Sortierreihenfolge
// und Verkettung sicherstellen zu muessen.
#ifdef DBG_UTIL
#define CHECK_SWPHINTS(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc()->IsInReading() ) \
                                  pNd->GetpSwpHints()->Check(); }
#else
#define CHECK_SWPHINTS(pNd)
#endif

/*N*/ SwTxtNode *SwNodes::MakeTxtNode( const SwNodeIndex & rWhere,
/*N*/ 								 SwTxtFmtColl *pColl,
/*N*/ 								 SwAttrSet* pAutoAttr )
/*N*/ {
/*N*/ 	ASSERT( pColl, "Collectionpointer ist 0." );
/*N*/
/*N*/ 	SwTxtNode *pNode = new SwTxtNode( rWhere, pColl, pAutoAttr );
/*N*/
/*N*/ 	SwNodeIndex aIdx( *pNode );
/*N*/
/*N*/ 	if( pColl && NO_NUMBERING != pColl->GetOutlineLevel() && IsDocNodes() )
/*N*/ 		UpdateOutlineNode( *pNode, NO_NUMBERING, pColl->GetOutlineLevel() );
/*N*/
/*N*/ 	//Wenn es noch kein Layout gibt oder in einer versteckten Section
/*N*/ 	// stehen, brauchen wir uns um das MakeFrms nicht bemuehen.
/*N*/ 	const SwSectionNode* pSectNd;
/*N*/ 	if( !GetDoc()->GetRootFrm() ||
/*N*/ 		( 0 != (pSectNd = pNode->FindSectionNode()) &&
/*N*/ 			pSectNd->GetSection().IsHiddenFlag() ))
/*N*/ 		return pNode;
/*N*/
/*N*/ 	SwNodeIndex aTmp( rWhere );
/*N*/ 	do {
/*N*/ 		// max. 2 Durchlaeufe:
/*N*/ 		// 1. den Nachfolger nehmen
/*N*/ 		// 2. den Vorgaenger
/*N*/
/*N*/ 		SwNode *pNd;
/*N*/ 		switch( ( pNd = (*this)[aTmp] )->GetNodeType() )
/*N*/ 		{
/*?*/ 		case ND_TABLENODE:
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ((SwTableNode*)pNd)->MakeFrms( aIdx );
/*?*/ 			return pNode;
/*?*/
/*?*/ 		case ND_SECTIONNODE:
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( ((SwSectionNode*)pNd)->GetSection().IsHidden() ||
/*?*/ 			return pNode;
/*N*/
/*N*/ 		case ND_TEXTNODE:
/*N*/ 		case ND_GRFNODE:
/*N*/ 		case ND_OLENODE:
/*N*/ 			((SwCntntNode*)pNd)->MakeFrms( *pNode );
/*N*/ 			return pNode;
/*N*/
/*N*/ 		case ND_ENDNODE:
/*N*/ 			if( pNd->FindStartNode()->IsSectionNode() &&
/*N*/ 				aTmp.GetIndex() < rWhere.GetIndex() )
/*N*/ 			{
/*?*/ 				if( pNd->FindStartNode()->GetSectionNode()->GetSection().IsHiddenFlag())
/*?*/ 				{
/*?*/ 					if( !GoPrevSection( &aTmp, TRUE, FALSE ) ||
/*?*/ 						aTmp.GetNode().FindTableNode() !=
/*?*/ 							pNode->FindTableNode() )
/*?*/ 						return pNode;		// schade, das wars
/*?*/ 				}
/*?*/ 				else
/*?*/ 					aTmp = *pNd->FindStartNode();
/*?*/ 				break;
/*N*/ 			}
/*N*/ 			else if( pNd->FindStartNode()->IsTableNode() &&
/*N*/ 					aTmp.GetIndex() < rWhere.GetIndex() )
/*N*/ 			{
/*N*/ 				// wir stehen hinter einem TabellenNode
/*?*/ 				aTmp = *pNd->FindStartNode();
/*?*/ 				break;
/*N*/ 			}
/*N*/ 			// kein break !!!
/*N*/ 		default:
/*N*/ 			if( rWhere == aTmp )
/*N*/ 				aTmp -= 2;
/*N*/ 			else
/*N*/ 				return pNode;
/*N*/ 			break;
/*N*/ 		}
/*N*/ 	} while( TRUE );
/*N*/ }



// --------------------
// SwTxtNode
// --------------------

/*N*/ SwTxtNode::SwTxtNode( const SwNodeIndex &rWhere,
/*N*/ 					  SwTxtFmtColl *pTxtColl,
/*N*/ 					  SwAttrSet* pAutoAttr )
/*N*/ 	: SwCntntNode( rWhere, ND_TEXTNODE, pTxtColl ),
/*N*/ 	  pSwpHints( 0 ), pWrong( 0 ), pNdNum( 0 ), pNdOutl( 0 )
/*N*/ {
/*N*/ 	// soll eine Harte-Attributierung gesetzt werden?
/*N*/ 	if( pAutoAttr )
/*N*/ 		SwCntntNode::SetAttr( *pAutoAttr );
/*N*/
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	if( GetNodes().IsDocNodes() &&
/*N*/ 		SFX_ITEM_SET == GetSwAttrSet().GetItemState( RES_PARATR_NUMRULE,
/*N*/ 		TRUE, &pItem ) && ((SwNumRuleItem*)pItem)->GetValue().Len() )
/*N*/ 	{
/*?*/ 		pNdNum = new SwNodeNum( 0 );
/*?*/ 		SwNumRule* pRule = GetDoc()->FindNumRulePtr(
/*?*/ 									((SwNumRuleItem*)pItem)->GetValue() );
/*?*/ 		if( pRule )
/*?*/ 			pRule->SetInvalidRule( TRUE );
/*N*/ 	}
/*N*/ }

/*N*/ SwTxtNode::~SwTxtNode()
/*N*/ {
/*N*/ 	// delete loescht nur die Pointer, nicht die Arrayelemente!
/*N*/ 	if( pSwpHints )
/*N*/ 	{
/*N*/ 		// damit Attribute die ihren Inhalt entfernen nicht doppelt
/*N*/ 		// geloescht werden.
/*N*/ 		SwpHints* pTmpHints = pSwpHints;
/*N*/ 		pSwpHints = 0;
/*N*/
/*N*/ 		for( register USHORT j = pTmpHints->Count(); j; )
/*N*/ 			// erst muss das Attribut aus dem Array entfernt werden,
/*N*/ 			// denn sonst wuerde es sich selbst loeschen (Felder) !!!!
/*N*/ 			DestroyAttr( pTmpHints->GetHt( --j ) );
/*N*/
/*N*/ 		delete pTmpHints;
/*N*/ 	}
/*N*/ 	delete pWrong;
/*N*/ 	// Achtung. im Dtor von SwCntntNode kann DelFrms gerufen werden, wo
/*N*/ 	// ggf. pWrong nochmal deletet wird, deshalb diese Zuweisung
/*N*/ 	pWrong = NULL; // hier nicht wegoptimieren!
/*N*/
/*N*/ 	delete pNdNum, pNdNum = 0;		// ggfs. wird in der BasisKlasse noch
/*N*/ 	delete pNdOutl, pNdOutl = 0;	// darauf zugegriffen??
/*N*/ }

/*N*/ SwCntntFrm *SwTxtNode::MakeFrm()
/*N*/ {
/*N*/ 	SwCntntFrm *pFrm = new SwTxtFrm(this);
/*N*/ 	return pFrm;
/*N*/ }

/*N*/ xub_StrLen SwTxtNode::Len() const
/*N*/ {
/*N*/ 	return aText.Len();
/*N*/ }

/*---------------------------------------------------------------------------
 * lcl_ChangeFtnRef
 * 	After a split node, it's necessary to actualize the ref-pointer of the
 *  ftnfrms.
 * --------------------------------------------------------------------------*/

/*N*/ void lcl_ChangeFtnRef( SwTxtNode &rNode )
/*N*/ {
/*N*/ 	SwpHints *pSwpHints = rNode.GetpSwpHints();
/*N*/ 	if( pSwpHints && rNode.GetDoc()->GetRootFrm() )
/*N*/ 	{
/*N*/ 		SwTxtAttr* pHt;
/*N*/ 		SwCntntFrm* pFrm = NULL;
/*N*/         // OD 07.11.2002 #104840# - local variable to remember first footnote
/*N*/         // of node <rNode> in order to invalidate position of its first content.
/*N*/         // Thus, in its <MakeAll()> it will checked its position relative to its reference.
/*N*/         SwFtnFrm* pFirstFtnOfNode = 0;
/*N*/ 		for( register USHORT j = pSwpHints->Count(); j; )
/*N*/         {
/*N*/ 			if( RES_TXTATR_FTN == (pHt = pSwpHints->GetHt(--j))->Which() )
/*N*/ 			{
/*N*/ 				if( !pFrm )
/*N*/ 				{
/*N*/ 					SwClientIter aNew( rNode );
/*N*/ 					pFrm = (SwCntntFrm*)aNew.First( TYPE(SwCntntFrm) );
/*N*/ //JP 11.07.00: the assert's shows incorrect an error when nodes are converted
/*N*/ //				to a table. Then no layout exist!
/*N*/ //					ASSERT( pFrm, "lcl_ChangeFtnRef: No TxtFrm" );
/*N*/ //					ASSERT( pFrm && !aNew.Next(),"lcl_ChangeFtnRef: Doublefault");
/*N*/ 					if( !pFrm )
/*N*/ 						return;
/*N*/ 				}
/*N*/ 				SwTxtFtn *pAttr = (SwTxtFtn*)pHt;
/*N*/ 				ASSERT( pAttr->GetStartNode(), "FtnAtr ohne StartNode." );
/*N*/ 				SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
/*N*/ 				SwCntntNode *pNd = aIdx.GetNode().GetCntntNode();
/*N*/ 				if ( !pNd )
/*N*/ 					pNd = pFrm->GetAttrSet()->GetDoc()->
/*N*/ 			  			  GetNodes().GoNextSection( &aIdx, TRUE, FALSE );
/*N*/ 				if ( !pNd )
/*N*/ 					continue;
/*N*/ 				SwClientIter aIter( *pNd );
/*N*/ 				SwCntntFrm* pCntnt = (SwCntntFrm*)aIter.First(TYPE(SwCntntFrm));
/*N*/ 				if( pCntnt )
/*N*/ 				{
/*N*/ 					ASSERT( pCntnt->FindRootFrm() == pFrm->FindRootFrm(),
/*N*/ 							"lcl_ChangeFtnRef: Layout double?" );
/*N*/ 					SwFtnFrm *pFtn = pCntnt->FindFtnFrm();
/*N*/ 					if( pFtn && pFtn->GetAttr() == pAttr )
/*N*/ 					{
/*N*/ 						while( pFtn->GetMaster() )
/*N*/ 							pFtn = pFtn->GetMaster();
/*N*/                         // OD 07.11.2002 #104840# - remember footnote frame
/*N*/                         pFirstFtnOfNode = pFtn;
/*N*/                         while ( pFtn )
/*N*/ 						{
/*N*/ 							pFtn->SetRef( pFrm );
/*N*/ 							pFtn = pFtn->GetFollow();
/*N*/ 							((SwTxtFrm*)pFrm)->SetFtn( TRUE );
/*N*/ 						}
/*N*/ 					}
/*N*/ #ifdef DBG_UTIL
/*N*/ 					while( 0 != (pCntnt = (SwCntntFrm*)aIter.Next()) )
/*N*/ 					{
/*N*/ 						SwFtnFrm *pFtn = pCntnt->FindFtnFrm();
/*N*/ 						ASSERT( !pFtn || pFtn->GetRef() == pFrm,
/*N*/ 								"lcl_ChangeFtnRef: Who's that guy?" );
/*N*/ 					}
/*N*/ #endif
/*N*/ 				}
/*N*/ 			}
/*N*/         } // end of for-loop on <SwpHints>
/*N*/         // OD 08.11.2002 #104840# - invalidate
/*N*/         if ( pFirstFtnOfNode )
/*N*/         {
/*N*/             SwCntntFrm* pCntnt = pFirstFtnOfNode->ContainsCntnt();
/*N*/             if ( pCntnt )
/*N*/             {
/*N*/                 pCntnt->_InvalidatePos();
/*N*/             }
/*N*/         }
/*N*/ 	}
/*N*/ }

/*N*/ SwCntntNode *SwTxtNode::SplitNode( const SwPosition &rPos )
/*N*/ {
/*N*/ 	// lege den Node "vor" mir an
/*N*/ 	register xub_StrLen nSplitPos = rPos.nContent.GetIndex(),
/*N*/ 					nTxtLen = aText.Len();
/*N*/ 	SwTxtNode* pNode = _MakeNewTxtNode( rPos.nNode, FALSE, nSplitPos==nTxtLen );
/*N*/
/*N*/ 	if( GetDepends() && aText.Len() && (nTxtLen / 2) < nSplitPos )
/*N*/ 	{
/*?*/ // JP 25.04.95: Optimierung fuer SplitNode:
/*?*/ //				Wird am Ende vom Node gesplittet, dann verschiebe die
/*?*/ //				Frames vom akt. auf den neuen und erzeuge fuer den akt.
/*?*/ //				neue. Dadurch entfaellt das neu aufbauen vom Layout.
/*?*/
/*?*/ 		LockModify();	// Benachrichtigungen abschalten
/*?*/
/*?*/ 		// werden FlyFrames mit verschoben, so muessen diese nicht ihre
/*?*/ 		// Frames zerstoeren. Im SwTxtFly::SetAnchor wird es abgefragt!
/*?*/ 		if( pSwpHints )
/*?*/ 		{
/*?*/ 			if( !pNode->pSwpHints )
/*?*/ 				pNode->pSwpHints = new SwpHints;
/*?*/ 			pNode->pSwpHints->bInSplitNode = TRUE;
/*?*/ 		}
/*?*/
/*?*/ 		//Ersten Teil des Inhalts in den neuen Node uebertragen und
/*?*/ 		//im alten Node loeschen.
/*?*/ 		SwIndex aIdx( this );
/*?*/ 		Cut( pNode, aIdx, nSplitPos );
/*?*/
/*?*/ 		if( GetWrong() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/             pNode->SetWrong( GetWrong()->SplitList( nSplitPos ) );
/*?*/
/*?*/ 		SetWrongDirty( TRUE );
/*?*/
/*?*/ 		if( pNode->pSwpHints )
/*?*/ 		{
/*?*/ 			if ( pNode->pSwpHints->CanBeDeleted() )
/*?*/ 			{
/*?*/ 				delete pNode->pSwpHints;
/*?*/ 				pNode->pSwpHints = 0;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				pNode->pSwpHints->bInSplitNode = FALSE;
/*?*/
/*?*/ 			// alle zeichengebundenen Rahmen, die im neuen Absatz laden
/*?*/ 			// muessen aus den alten Frame entfernt werden:
/*?*/ 			// JP 01.10.96: alle leeren und nicht zu expandierenden
/*?*/ 			//				Attribute loeschen
/*?*/ 			if( pSwpHints )
/*?*/ 			{
/*?*/ 				SwTxtAttr* pHt;
/*?*/ 				xub_StrLen* pEnd;
/*?*/ 				for( register USHORT j = pSwpHints->Count(); j; )
/*?*/                     if( RES_TXTATR_FLYCNT ==
/*?*/                         ( pHt = pSwpHints->GetHt( --j ) )->Which() )
/*?*/ 						pHt->GetFlyCnt().GetFrmFmt()->DelFrms();
/*?*/ 					else if( pHt->DontExpand() && 0 != ( pEnd = pHt->GetEnd() )
/*?*/ 							&& *pHt->GetStart() == *pEnd )
/*?*/ 					{
/*?*/ 						// loeschen!
/*?*/ 						pSwpHints->DeleteAtPos( j );
/*?*/ 						DestroyAttr( pHt );
/*?*/ 					}
/*?*/ 			}
/*?*/
/*?*/ 		}
/*?*/
/*?*/ 		SwClientIter aIter( *this );
/*?*/ 		SwClient* pLast = aIter.GoStart();
/*?*/ 		if( pLast )
/*?*/ 			do
/*?*/ 			{	SwCntntFrm *pFrm = PTR_CAST( SwCntntFrm, pLast );
/*?*/ 				if ( pFrm )
/*?*/ 				{
/*?*/ 					pNode->Add( pFrm );
/*?*/ 					if( pFrm->IsTxtFrm() && !pFrm->IsFollow() &&
/*?*/ 						((SwTxtFrm*)pFrm)->GetOfst() )
/*?*/ 						((SwTxtFrm*)pFrm)->SetOfst( 0 );
/*?*/ 				}
/*?*/ 			} while( 0 != ( pLast = aIter++ ));
/*?*/
/*?*/ 		if ( IsInCache() )
/*?*/ 		{
/*?*/ 			SwFrm::GetCache().Delete( this );
/*?*/ 			SetInCache( FALSE );
/*?*/ 		}
/*?*/
/*?*/ 		UnlockModify();	// Benachrichtigungen wieder freischalten
/*?*/
/*?*/ 		const SwRootFrm *pRootFrm;
/*?*/ 		// If there is an accessible layout we must call modify even
/*?*/ 		// with length zero, because we have to notify about the changed
/*?*/ 		// text node.
/*?*/ 		if( nTxtLen != nSplitPos
/*?*/ #ifdef ACCESSIBLE_LAYOUT
/*?*/ 				||
/*?*/ 			( (pRootFrm = pNode->GetDoc()->GetRootFrm()) != 0 &&
/*?*/ 		 	  pRootFrm->IsAnyShellAccessible() )
/*?*/ #endif
/*?*/ 			  )
/*?*/
/*?*/ 		{
/*?*/ 			// dann sage den Frames noch, das am Ende etwas "geloescht" wurde
/*?*/ 			if( 1 == nTxtLen - nSplitPos )
/*?*/ 			{
/*?*/ 				SwDelChr aHint( nSplitPos );
/*?*/ 				pNode->SwModify::Modify( 0, &aHint );
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				SwDelTxt aHint( nSplitPos, nTxtLen - nSplitPos );
/*?*/ 				pNode->SwModify::Modify( 0, &aHint );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		if( pSwpHints )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 			MoveTxtAttr_To_AttrSet();
/*?*/ 		pNode->MakeFrms( *this );		// neue Frames anlegen.
/*?*/ 		lcl_ChangeFtnRef( *this );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/         SwWrongList *pList = GetWrong();
/*N*/         pWrong = NULL;
/*N*/
/*N*/ 		SetWrongDirty( TRUE );
/*N*/
/*N*/ 		SwIndex aIdx( this );
/*N*/ 		Cut( pNode, aIdx, rPos.nContent.GetIndex() );
/*N*/
/*N*/ 		// JP 01.10.96: alle leeren und nicht zu expandierenden
/*N*/ 		//				Attribute loeschen
/*N*/ 		if( pSwpHints )
/*N*/ 		{
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 			SwTxtAttr* pHt;
/*N*/ 		}
/*N*/
/*N*/         if( pList )
/*N*/         {
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/             pNode->SetWrong( pList->SplitList( nSplitPos ) );
/*N*/         }
/*N*/
/*N*/ 		if ( GetDepends() )
/*N*/ 			MakeFrms( *pNode );		// neue Frames anlegen.
/*N*/ 		lcl_ChangeFtnRef( *pNode );
/*N*/ 	}
/*N*/
/*N*/ 	{
/*N*/ 		//Hint fuer Pagedesc versenden. Das mueste eigntlich das Layout im
/*N*/ 		//Paste der Frames selbst erledigen, aber das fuehrt dann wiederum
/*N*/ 		//zu weiteren Folgefehlern, die mit Laufzeitkosten geloest werden
/*N*/ 		//muesten. #56977# #55001# #56135#
/*N*/ 		const SfxPoolItem *pItem;
/*N*/ 		if( GetDepends() && SFX_ITEM_SET == pNode->GetSwAttrSet().
/*N*/ 			GetItemState( RES_PAGEDESC, TRUE, &pItem ) )
/*?*/ 			pNode->Modify( (SfxPoolItem*)pItem, (SfxPoolItem*)pItem );
/*N*/ 	}
/*N*/ 	return pNode;
/*N*/ }


/*N*/ SwCntntNode *SwTxtNode::JoinNext()
/*N*/ {
/*N*/ 	SwNodes& rNds = GetNodes();
/*N*/ 	SwNodeIndex aIdx( *this );
/*N*/ 	if( SwCntntNode::CanJoinNext( &aIdx ) )
/*N*/ 	{
/*N*/ 		SwDoc* pDoc = rNds.GetDoc();
/*N*/ 		SvULongs aBkmkArr( 15, 15 );
/*N*/ 		_SaveCntntIdx( pDoc, aIdx.GetIndex(), USHRT_MAX, aBkmkArr, SAVEFLY );
/*N*/ 		SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
/*N*/ 		xub_StrLen nOldLen = aText.Len();
/*N*/         SwWrongList *pList = GetWrong();
/*N*/         if( pList )
/*N*/         {
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/             pList->JoinList( pTxtNode->GetWrong(), nOldLen );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             pList = pTxtNode->GetWrong();
/*N*/             if( pList )
/*N*/             {
                    DBG_BF_ASSERT(0, "STRIP");//STRIP001 /*?*/                 pList->Move( 0, nOldLen );
/*N*/             }
/*N*/         }
/*N*/ 		{ // wg. SwIndex
/*N*/ 			pTxtNode->Cut( this, SwIndex(pTxtNode), pTxtNode->Len() );
/*N*/ 		}
/*N*/ 		// verschiebe noch alle Bookmarks/TOXMarks
/*N*/ 		if( aBkmkArr.Count() )
/*N*/ 			_RestoreCntntIdx( pDoc, aBkmkArr, GetIndex(), nOldLen );
/*N*/
/*N*/ 		if( pTxtNode->HasAnyIndex() )
/*N*/ 		{
/*N*/ 			// alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
/*?*/ 			pDoc->CorrAbs( aIdx, SwPosition( *this ), nOldLen, TRUE );
/*N*/ 		}
/*N*/ 		rNds.Delete(aIdx);
/*N*/         pWrong = pList;
/*N*/ 		InvalidateNumRule();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		ASSERT( FALSE, "kein TxtNode." );
/*N*/
/*N*/ 	return this;
/*N*/ }

/*N*/ SwCntntNode *SwTxtNode::JoinPrev()
/*N*/ {
/*N*/ 	SwNodes& rNds = GetNodes();
/*N*/ 	SwNodeIndex aIdx( *this );
/*N*/ 	if( SwCntntNode::CanJoinPrev( &aIdx ) )
/*N*/ 	{
/*N*/ 		SwDoc* pDoc = rNds.GetDoc();
/*N*/ 		SvULongs aBkmkArr( 15, 15 );
/*N*/ 		_SaveCntntIdx( pDoc, aIdx.GetIndex(), USHRT_MAX, aBkmkArr, SAVEFLY );
/*N*/ 		SwTxtNode *pTxtNode = aIdx.GetNode().GetTxtNode();
/*N*/ 		xub_StrLen nLen = pTxtNode->Len();
/*N*/         SwWrongList *pList = pTxtNode->GetWrong();
/*N*/         if( pList )
/*N*/         {
                DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/             pList->JoinList( GetWrong(), Len() );
/*N*/         }
/*N*/         else
/*N*/         {
/*N*/             pList = GetWrong();
/*N*/             if( pList )
/*N*/             {
                    DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/                 pList->Move( 0, nLen );
/*N*/             }
/*N*/         }
/*N*/ 		{ // wg. SwIndex
/*N*/ 			pTxtNode->Cut( this, SwIndex( this ), SwIndex(pTxtNode), nLen );
/*N*/ 		}
/*N*/ 		// verschiebe noch alle Bookmarks/TOXMarks
/*N*/ 		if( aBkmkArr.Count() )
/*N*/ 			_RestoreCntntIdx( pDoc, aBkmkArr, GetIndex() );
/*N*/
/*N*/ 		if( pTxtNode->HasAnyIndex() )
/*N*/ 		{
/*N*/ 			// alle Crsr/StkCrsr/UnoCrsr aus dem Loeschbereich verschieben
/*?*/ 			pDoc->CorrAbs( aIdx, SwPosition( *this ), nLen, TRUE );
/*N*/ 		}
/*N*/ 		rNds.Delete(aIdx);
/*N*/         pWrong = pList;
/*N*/ 		InvalidateNumRule();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		ASSERT( FALSE, "kein TxtNode." );
/*N*/
/*N*/ 	return this;
/*N*/ }

// erzeugt einen AttrSet mit Bereichen fuer Frame-/Para/Char-Attributen
/*N*/ void SwTxtNode::NewAttrSet( SwAttrPool& rPool )
/*N*/ {
/*N*/ 	ASSERT( !pAttrSet, "AttrSet ist doch gesetzt" );
/*N*/ 	pAttrSet = new SwAttrSet( rPool, aTxtNodeSetRange );
/*N*/ //FEATURE::CONDCOLL
/*N*/ //	pAttrSet->SetParent( &GetFmtColl()->GetAttrSet() );
/*N*/ 	pAttrSet->SetParent( &GetAnyFmtColl().GetAttrSet() );
/*N*/ //FEATURE::CONDCOLL
/*N*/ }


// change the URL in the attribut - if it is a valid URL!
/*N*/ void lcl_CheckURLChanged( const SwFmtINetFmt& rURLAttr, const String& rText,
/*N*/ 							xub_StrLen nStt, xub_StrLen nEnd )
/*N*/ {
/*N*/ 	if( nStt < nEnd )
/*N*/ 	{
/*N*/ 		xub_StrLen nS = nStt, nE = nEnd;
/*N*/ 		String sNew( FindFirstURLInText( rText, nS, nE,
/*N*/ 													GetAppCharClass() ));
/*N*/ 		if( sNew.Len() && nS == nStt && nE == nEnd )
/*N*/ 		{
/*N*/ 			// it is an valid URL, so set it to the URL Object
/*N*/ 			((SwFmtINetFmt&)rURLAttr).SetValue( rText.Copy( nS, nE - nS ));
/*N*/ 		}
/*N*/ 	}
/*N*/ }

// Ueberladen der virtuellen Update-Methode von SwIndexReg. Dadurch
// benoetigen die Text-Attribute nur xub_StrLen statt SwIndizies!
/*N*/ void SwTxtNode::Update( const SwIndex & aPos, xub_StrLen nLen,
/*N*/ 						BOOL bNegativ )
/*N*/ {
/*N*/ 	SetAutoCompleteWordDirty( TRUE );
/*N*/
/*N*/ 	TmpHints* pCollector = NULL;
/*N*/ 	if( pSwpHints )
/*N*/ 	{
/*N*/ 		xub_StrLen nPos = aPos.GetIndex();
/*N*/ 		xub_StrLen* pIdx;
/*N*/ 		SwTxtAttr* pHt;
/*N*/ 		if( bNegativ )
/*N*/ 		{
/*N*/ 			xub_StrLen nMax = nPos + nLen;
/*N*/ 			for( USHORT n = 0; n < pSwpHints->Count(); ++n )
/*N*/ 			{
/*N*/ 				BOOL bCheckURL = FALSE, bSttBefore = FALSE;
/*N*/ 				pHt = pSwpHints->GetHt(n);
/*N*/ 				pIdx = pHt->GetStart();
/*N*/ 				if( *pIdx >= nPos )
/*N*/ 				{
/*N*/ 					if( *pIdx > nMax )
/*N*/ 						 *pIdx -= nLen;
/*N*/ 					else
/*N*/ 					{
/*N*/ 						if( *pIdx < nMax )
/*N*/ 						 	bCheckURL = TRUE;
/*N*/ 						 *pIdx = nPos;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					bSttBefore = TRUE;
/*N*/
/*N*/ 				if( 0 == (pIdx = pHt->GetEnd()) )
/*N*/ 					continue;
/*N*/
/*N*/ 				if( *pIdx >= nPos )
/*N*/ 				{
/*N*/ 					if( *pIdx > nMax )
/*N*/ 					{
/*N*/ 						*pIdx -= nLen;
/*N*/ 						if( bSttBefore )
/*N*/ 							bCheckURL = TRUE;
/*N*/ 					}
/*N*/ 					else if( *pIdx != nPos )
/*N*/ 					{
/*N*/ 						*pIdx = nPos;
/*N*/ 						if( bSttBefore )
/*N*/ 							bCheckURL = TRUE;
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				if( bCheckURL && RES_TXTATR_INETFMT == pHt->Which() )
/*N*/ 				{
/*N*/ 					// reset the URL in the attribut - if it is a valid URL!
/*N*/ 					lcl_CheckURLChanged( pHt->GetINetFmt(), aText,
/*N*/ 										*pHt->GetStart(), *pHt->GetEnd() );
/*N*/ 				}
/*N*/
/*N*/ //JP 01.10.96: fuers SplitNode sollte das Flag nicht geloescht werden!
/*N*/ //				pHt->SetDontExpand( FALSE );
/*N*/ 			}
/*N*/ 			// AMA: Durch das Loeschen koennen Attribute gleiche Start-
/*N*/ 			// 		und/oder Endwerte erhalten, die vorher echt ungleich
/*N*/ 			//		waren. Dadurch kann die Sortierung durcheinander geraten,
/*N*/ 			//		die bei gleichen Start/Endwerten den Pointer selbst
/*N*/ 			//		vergleicht, also ClearDummies ...
/*N*/ 			pSwpHints->ClearDummies( *this );
/*N*/ 			if ( !pSwpHints->Merge( *this ) )
/*N*/ 				((SwpHintsArr*)pSwpHints)->Resort();
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			xub_StrLen* pEnd;
/*N*/ 			BOOL bNoExp = FALSE;
/*N*/             BOOL bResort = FALSE;
/*N*/ 			const USHORT coArrSz = RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN +
/*N*/ 								( RES_UNKNOWNATR_END - RES_UNKNOWNATR_BEGIN );
/*N*/
/*N*/ 			BOOL aDontExp[ coArrSz ];
/*N*/ 			memset( &aDontExp, 0, coArrSz * sizeof(BOOL) );
/*N*/
/*N*/ 			for( USHORT n = 0; n < pSwpHints->Count(); ++n )
/*N*/ 			{
/*N*/ 				BOOL bCheckURL = FALSE;
/*N*/ 				pHt = pSwpHints->GetHt(n);
/*N*/ 				pIdx = pHt->GetStart();
/*N*/ 				if( *pIdx >= nPos )
/*N*/ 				{
/*N*/ 					*pIdx += nLen;
/*N*/ 					if( 0 != ( pEnd = pHt->GetEnd() ) )
/*N*/ 						*pEnd += nLen;
/*N*/ 				}
/*N*/ 				else if( 0 != ( pEnd = pHt->GetEnd() ) && *pEnd >= nPos )
/*N*/ 				{
/*N*/ 					if( *pEnd > nPos || IsIgnoreDontExpand() )
/*N*/ 					{
/*N*/ 						bCheckURL = TRUE;
/*N*/ 						*pEnd += nLen;
/*N*/ 					}
/*N*/ 					else
/*N*/ 					{
/*N*/ 						USHORT nWhPos, nWhich = pHt->Which();
/*N*/
/*N*/ 						if( RES_CHRATR_BEGIN <= nWhich &&
/*N*/ 							nWhich < RES_TXTATR_WITHEND_END )
/*N*/ 						 	nWhPos = nWhich - RES_CHRATR_BEGIN;
/*N*/ 						else if( RES_UNKNOWNATR_BEGIN <= nWhich &&
/*N*/ 								nWhich < RES_UNKNOWNATR_END )
/*?*/ 							nWhPos = nWhich - RES_UNKNOWNATR_BEGIN +
/*?*/ 								( RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN );
/*N*/ 						else
/*N*/ 							continue;
/*N*/
/*N*/ 						if( aDontExp[ nWhPos ] )
/*N*/ 							continue;
/*N*/
/*N*/ 						if( pHt->DontExpand() )
/*N*/ 						{
/*?*/ 							pHt->SetDontExpand( FALSE );
/*?*/                             bResort = TRUE;
/*?*/ 							if( pHt->IsCharFmtAttr() )
/*?*/ 							{
/*?*/ 								bNoExp = TRUE;
/*?*/ 								aDontExp[ RES_TXTATR_CHARFMT -RES_CHRATR_BEGIN ]
/*?*/ 									= TRUE;
/*?*/ 								aDontExp[ RES_TXTATR_INETFMT -RES_CHRATR_BEGIN ]
/*?*/ 									= TRUE;
/*?*/ 							}
/*?*/ 							else
/*?*/ 								aDontExp[ nWhPos ] = TRUE;
/*N*/ 						}
/*N*/ 						else if( bNoExp )
/*N*/ 						{
/*?*/ 							 if( !pCollector )
/*?*/ 								pCollector = new TmpHints;
/*?*/ 							 USHORT nCollCnt = pCollector->Count();
/*?*/ 							 for( USHORT i = 0; i < nCollCnt; ++i )
/*?*/ 							 {
/*?*/ 								SwTxtAttr *pTmp = (*pCollector)[ i ];
/*?*/ 								if( nWhich == pTmp->Which() )
/*?*/ 								{
/*?*/ 									pCollector->Remove( i );
/*?*/ 									delete pTmp;
/*?*/ 									break;
/*?*/ 								}
/*?*/ 							 }
/*?*/ 							 SwTxtAttr *pTmp = MakeTxtAttr( pHt->GetAttr(),
/*?*/ 												nPos, nPos + nLen );
/*?*/ 							 pCollector->C40_INSERT( SwTxtAttr, pTmp, pCollector->Count() );
/*N*/ 						}
/*N*/ 						else
/*N*/ 						{
/*N*/ 							*pEnd += nLen;
/*N*/ 							bCheckURL = TRUE;
/*N*/ 						}
/*N*/ 					}
/*N*/ 				}
/*N*/
/*N*/ 				if( bCheckURL && RES_TXTATR_INETFMT == pHt->Which() )
/*N*/ 				{
/*N*/ 					// reset the URL in the attribut - if it is a valid URL!
/*N*/ 					lcl_CheckURLChanged( pHt->GetINetFmt(), aText,
/*N*/ 										*pHt->GetStart(), *pHt->GetEnd() );
/*N*/ 				}
/*N*/ 			}
/*N*/             if( bResort )
/*?*/ 				((SwpHintsArr*)pSwpHints)->Resort();
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SwIndexReg aTmpIdxReg;
/*N*/ 	if( !bNegativ )
/*N*/ 	{
/*N*/ 		SwIndex* pIdx;
/*N*/ 		const SwRedlineTbl& rTbl = GetDoc()->GetRedlineTbl();
/*N*/ 		if( rTbl.Count() )
/*?*/ 			for( USHORT i = 0; i < rTbl.Count(); ++i )
/*?*/ 			{
/*?*/ 				SwRedline* pRedl = rTbl[ i ];
/*?*/ 				if( pRedl->HasMark() )
/*?*/ 				{
/*?*/ 					SwPosition* pEnd = pRedl->End();
/*?*/ 					if( this == &pEnd->nNode.GetNode() &&
/*?*/ 						*pRedl->GetPoint() != *pRedl->GetMark() &&
/*?*/ 						aPos.GetIndex() ==
/*?*/ 							(pIdx = &pEnd->nContent)->GetIndex() )
/*?*/ 						pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
/*?*/ 				}
/*?*/ 				else if( this == &pRedl->GetPoint()->nNode.GetNode() &&
/*?*/ 						aPos.GetIndex() == (pIdx = &pRedl->GetPoint()->
/*?*/ 							nContent)->GetIndex() )
/*?*/ 				{
/*?*/ 					pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
/*?*/ 					if( &pRedl->GetBound( TRUE ) == pRedl->GetPoint() )
/*?*/ 					{
/*?*/ 						pRedl->GetBound( FALSE ) = pRedl->GetBound( TRUE );
/*?*/ 						pIdx = &pRedl->GetBound( FALSE ).nContent;
/*?*/ 					}
/*?*/ 					else
/*?*/ 					{
/*?*/ 						pRedl->GetBound( TRUE ) = pRedl->GetBound( FALSE );
/*?*/ 						pIdx = &pRedl->GetBound( TRUE ).nContent;
/*?*/ 					}
/*?*/ 					pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
/*?*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 		const SwBookmarks& rBkmk = GetDoc()->GetBookmarks();
/*N*/ 		if( rBkmk.Count() )
/*N*/ 			for( USHORT i = 0; i < rBkmk.Count(); ++i )
/*N*/ 			{
/*N*/ 				SwBookmark* pBkmk = rBkmk[ i ];
/*N*/ 				if( (this == &pBkmk->GetPos().nNode.GetNode() &&
/*N*/ 					 aPos.GetIndex() == (pIdx = (SwIndex*)&pBkmk->GetPos().
/*N*/ 							nContent)->GetIndex() ) ||
/*N*/ 					( pBkmk->GetOtherPos() &&
/*N*/ 					  this == &pBkmk->GetOtherPos()->nNode.GetNode() &&
/*N*/ 					  aPos.GetIndex() == (pIdx = (SwIndex*)&pBkmk->
/*N*/ 					  		GetOtherPos()->nContent)->GetIndex() ) )
/*N*/ 						pIdx->Assign( &aTmpIdxReg, pIdx->GetIndex() );
/*N*/ 			}
/*N*/ 	}
/*N*/ 	SwIndexReg::Update( aPos, nLen, bNegativ );
/*N*/ 	if( pCollector )
/*N*/ 	{
/*?*/ 		USHORT nCount = pCollector->Count();
/*?*/ 		for( USHORT i = 0; i < nCount; ++i )
/*?*/ 			pSwpHints->Insert( (*pCollector)[ i ], *this, FALSE );
/*?*/ 		delete pCollector;
/*N*/ 	}
/*N*/
/*N*/ 	aTmpIdxReg.MoveTo( *this );
/*N*/ }

/*N*/ SwFmtColl* SwTxtNode::ChgFmtColl( SwFmtColl *pNewColl )
/*N*/ {
/*N*/ 	ASSERT( pNewColl,"ChgFmtColl: Collectionpointer ist 0." );
/*N*/ 	ASSERT( HAS_BASE( SwTxtFmtColl, pNewColl ),
/*N*/ 				"ChgFmtColl: ist kein Text-Collectionpointer." );
/*N*/
/*N*/ 	SwTxtFmtColl *pOldColl = GetTxtColl();
/*N*/ 	if( pNewColl != pOldColl )
/*N*/ 		SwCntntNode::ChgFmtColl( pNewColl );
/*N*/ 	// nur wenn im normalen Nodes-Array
/*N*/ 	if( GetNodes().IsDocNodes() )
/*N*/ 		_ChgTxtCollUpdateNum( pOldColl, (SwTxtFmtColl*)pNewColl );
/*N*/ 	return	pOldColl;
/*N*/ }

/*N*/ void SwTxtNode::_ChgTxtCollUpdateNum( const SwTxtFmtColl *pOldColl,
/*N*/ 										const SwTxtFmtColl *pNewColl)
/*N*/ {
/*N*/ 	SwDoc* pDoc = GetDoc();
/*N*/ 	ASSERT( pDoc, "Kein Doc?" );
/*N*/ 	// erfrage die OutlineLevel und update gegebenenfalls das Nodes-Array,
/*N*/ 	// falls sich die Level geaendert haben !
/*N*/ 	const BYTE nOldLevel = pOldColl ? pOldColl->GetOutlineLevel():NO_NUMBERING;
/*N*/ 	const BYTE nNewLevel = pNewColl ? pNewColl->GetOutlineLevel():NO_NUMBERING;
/*N*/
/*N*/ 	SwNodes& rNds = GetNodes();
/*N*/ 	if( nOldLevel != nNewLevel )
/*N*/ 	{
/*N*/ 		delete pNdOutl, pNdOutl = 0;
/*N*/ 		// Numerierung aufheben, falls sie aus der Vorlage kommt
/*N*/ 		// und nicht nicht aus der neuen
/*N*/ 		if( NO_NUMBERING != nNewLevel && pNdNum && ( !GetpSwAttrSet() ||
/*N*/ 			SFX_ITEM_SET != GetpSwAttrSet()->GetItemState(
/*N*/ 				RES_PARATR_NUMRULE, FALSE )) &&
/*N*/ 			(!pNewColl || SFX_ITEM_SET != pNewColl->GetItemState(
/*N*/ 				RES_PARATR_NUMRULE )) )
/*N*/ 			delete pNdNum, pNdNum = 0;
/*N*/ 		if( rNds.IsDocNodes() )
/*N*/ 			rNds.UpdateOutlineNode( *this, nOldLevel, nNewLevel );
/*N*/ 	}
/*N*/
/*N*/ 	// Update beim Level 0 noch die Fussnoten !!
/*N*/ 	if( (!nNewLevel || !nOldLevel) && pDoc->GetFtnIdxs().Count() &&
/*N*/ 		FTNNUM_CHAPTER == pDoc->GetFtnInfo().eNum &&
/*N*/ 		rNds.IsDocNodes() )
/*N*/ 	{
/*?*/ 		SwNodeIndex aTmpIndex( rNds, GetIndex());
/*?*/
/*?*/ 		pDoc->GetFtnIdxs().UpdateFtn( aTmpIndex);
/*N*/ 	}
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	if( /*pOldColl != pNewColl && pNewColl && */
/*N*/ 		RES_CONDTXTFMTCOLL == pNewColl->Which() )
/*N*/ 	{
/*N*/ 		// Erfrage die akt. Condition des TextNodes:
/*N*/ 		ChkCondColl();
/*N*/ 	}
/*N*/ //FEATURE::CONDCOLL
/*N*/ }

// Wenn man sich genau am Ende einer Text- bzw. INetvorlage befindet,
// bekommt diese das DontExpand-Flag verpasst



// gebe das vorgegebene Attribut, welches an der TextPosition (rIdx)
// gesetzt ist, zurueck. Gibt es keines, returne 0-Pointer.
// (gesetzt heisst, je nach bExpand ?
// 									  Start < rIdx <= End
//									: Start <= rIdx < End )

/*N*/ SwTxtAttr* SwTxtNode::GetTxtAttr( const SwIndex& rIdx, USHORT nWhichHt,
/*N*/ 								  BOOL bExpand ) const
/*N*/ {
/*N*/ 	const SwTxtAttr* pRet = 0;
/*N*/ 	const SwTxtAttr* pHt = 0;
/*N*/ 	const xub_StrLen *pEndIdx = 0;
/*N*/ 	const xub_StrLen nIdx = rIdx.GetIndex();
/*N*/ 	const USHORT  nSize = pSwpHints ? pSwpHints->Count() : 0;
/*N*/
/*N*/ 	for( USHORT i = 0; i < nSize; ++i )
/*N*/ 	{
/*N*/ 		// ist der Attribut-Anfang schon groesser als der Idx ?
/*N*/ 		if( nIdx < *((pHt = (*pSwpHints)[i])->GetStart()) )
/*N*/ 			break;			// beenden, kein gueltiges Attribut
/*N*/
/*N*/ 		// ist es das gewuenschte Attribut ?
/*N*/ 		if( pHt->Which() != nWhichHt )
/*N*/ 			continue;		// nein, weiter
/*N*/
/*N*/ 		pEndIdx = pHt->GetEnd();
/*N*/ 		// liegt innerhalb des Bereiches ??
/*N*/ 		if( !pEndIdx )
/*N*/ 		{
/*N*/ 			if( *pHt->GetStart() == nIdx )
/*N*/ 			{
/*N*/ 				pRet = pHt;
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		else if( *pHt->GetStart() <= nIdx && nIdx <= *pEndIdx )
/*N*/ 		{
/*N*/ 			// Wenn bExpand gesetzt ist, wird das Verhalten bei Eingabe
/*N*/ 			// simuliert, d.h. der Start wuede verschoben, das Ende expandiert,
/*N*/ 			if( bExpand )
/*N*/ 			{
/*?*/ 				if( *pHt->GetStart() < nIdx )
/*?*/ 					pRet = pHt;
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				if( nIdx < *pEndIdx )
/*N*/ 					pRet = pHt;		// den am dichtesten liegenden
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return (SwTxtAttr*)pRet;		// kein gueltiges Attribut gefunden !!
/*N*/ }

/*************************************************************************
 *							CopyHint()
 *************************************************************************/


/*N*/ void lcl_CopyHint( const USHORT nWhich, const SwTxtAttr *pHt,
/*N*/ 					SwTxtAttr *pNewHt, SwDoc* pOtherDoc, SwTxtNode *pDest )
/*N*/ {
/*N*/ 	ASSERT( nWhich == pHt->Which(), "Falsche Hint-Id" );
/*N*/ 	switch( nWhich )
/*N*/ 	{
/*N*/ 		// Wenn wir es mit einem Fussnoten-Attribut zu tun haben,
/*N*/ 		// muessen wir natuerlich auch den Fussnotenbereich kopieren.
/*N*/ 		case RES_TXTATR_FTN :
            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 			((SwTxtFtn*)pHt)->CopyFtn( (SwTxtFtn*)pNewHt );
/*?*/ 			break;
/*N*/
/*N*/ 		// Beim Kopieren von Feldern in andere Dokumente
/*N*/ 		// muessen die Felder bei ihren neuen Feldtypen angemeldet werden.
/*N*/
/*N*/ 		// TabellenFormel muessen relativ kopiert werden.
/*N*/ 		case RES_TXTATR_FIELD :
/*N*/ 			{
/*N*/ 				const SwFmtFld& rFld = pHt->GetFld();
/*N*/ 				if( pOtherDoc )
                        {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 					((SwTxtFld*)pHt)->CopyFld( (SwTxtFld*)pNewHt );
/*N*/
/*N*/ 				// Tabellenformel ??
/*N*/ 				if( RES_TABLEFLD == rFld.GetFld()->GetTyp()->Which()
/*N*/ 					&& ((SwTblField*)rFld.GetFld())->IsIntrnlName() )
/*N*/ 				{
/*N*/ 					// wandel die interne in eine externe Formel um
/*?*/ 					const SwTableNode* pDstTblNd = ((SwTxtFld*)pHt)->
/*?*/ 											GetTxtNode().FindTableNode();
/*?*/ 					if( pDstTblNd )
/*?*/ 					{
/*?*/ 						SwTblField* pTblFld = (SwTblField*)
/*?*/ 												pNewHt->GetFld().GetFld();
/*?*/ 						pTblFld->PtrToBoxNm( &pDstTblNd->GetTable() );
/*?*/ 					}
/*N*/ 				}
/*N*/ 			}
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_TXTATR_TOXMARK :
/*N*/ 			if( pOtherDoc && pDest && pDest->GetpSwpHints()
/*N*/ 				&& USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
/*N*/ 				// Beim Kopieren von TOXMarks(Client) in andere Dokumente
/*N*/ 				// muss der Verzeichnis (Modify) ausgetauscht werden
/*N*/ 				((SwTxtTOXMark*)pNewHt)->CopyTOXMark( pOtherDoc );
/*N*/ 			break;
/*N*/
/*N*/ 		case RES_TXTATR_CHARFMT :
/*N*/ 			// Wenn wir es mit einer Zeichenvorlage zu tun haben,
/*N*/ 			// muessen wir natuerlich auch die Formate kopieren.
/*N*/ 			if( pDest && pDest->GetpSwpHints()
/*N*/ 				&& USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
/*N*/ 			{
/*N*/ 				SwCharFmt* pFmt = (SwCharFmt*)pHt->GetCharFmt().GetCharFmt();
/*N*/
/*N*/ 				if( pFmt && pOtherDoc )
/*?*/ 					pFmt = pOtherDoc->CopyCharFmt( *pFmt );
/*N*/ 				((SwFmtCharFmt&)pNewHt->GetCharFmt()).SetCharFmt( pFmt );
/*N*/ 			}
/*N*/ 			break;
/*?*/ 		case RES_TXTATR_INETFMT :
/*?*/ 			// Wenn wir es mit benutzerdefinierten INet-Zeichenvorlagen
/*?*/ 			// zu tun haben, muessen wir natuerlich auch die Formate kopieren.
/*?*/ 			if( pOtherDoc && pDest && pDest->GetpSwpHints()
/*?*/ 				&& USHRT_MAX != pDest->GetpSwpHints()->GetPos( pNewHt ) )
/*?*/ 			{
/*?*/ 				const SwDoc* pDoc;
/*?*/ 				if( 0!=( pDoc = ((SwTxtINetFmt*)pHt)->GetTxtNode().GetDoc() ) )
/*?*/ 				{
                        DBG_BF_ASSERT(0, "STRIP"); //STRIP001 /*?*/ 					const SwCharFmts* pCharFmts = pDoc->GetCharFmts();
/*?*/ 				}
/*?*/ 			}
/*?*/ 			//JP 24.04.98: Bug 49753 - ein TextNode muss am Attribut
/*?*/ 			//				gesetzt sein, damit die Vorlagen erzeugt
/*?*/ 			//				werden koenne
/*?*/ 			if( !((SwTxtINetFmt*)pNewHt)->GetpTxtNode() )
/*?*/ 				((SwTxtINetFmt*)pNewHt)->ChgTxtNode( pDest );
/*?*/
/*?*/ 			//JP 22.10.97: Bug 44875 - Verbindung zum Format herstellen
/*?*/ 			((SwTxtINetFmt*)pNewHt)->GetCharFmt();
/*?*/ 			break;
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*	SwTxtNode::CopyAttr()
|*	Beschreibung	kopiert Attribute an der Position nStart in pDest.
|*	BP 7.6.93:		Es werden mit Absicht nur die Attribute _mit_ EndIdx
|*					kopiert! CopyAttr wird vornehmlich dann gerufen,
|*					wenn Attribute fuer einen Node mit leerem String
|*					gesetzt werden sollen.
*************************************************************************/

/*N*/ void SwTxtNode::CopyAttr( SwTxtNode *pDest, const xub_StrLen nTxtStartIdx,
/*N*/ 						  const xub_StrLen nOldPos )
/*N*/ {
/*N*/ 	if( pSwpHints ) 	   // keine Attribute, keine Kekse
/*N*/ 	{
/*N*/ 		const xub_StrLen *pEndIdx = 0;
/*N*/ 		const SwTxtAttr *pHt = 0;
/*N*/ 		SwTxtAttr *pNewHt = 0;
/*N*/ 		xub_StrLen nAttrStartIdx = 0;
/*N*/ 		USHORT nWhich;
/*N*/
/*N*/ 		SwDoc* pOtherDoc = pDest->GetDoc();
/*N*/ 		if( pOtherDoc == GetDoc() )
/*N*/ 			pOtherDoc = 0;
/*N*/
/*N*/ 		for( USHORT i = 0; i < pSwpHints->Count(); i++ )
/*N*/ 		{
/*?*/ 			pHt = (*pSwpHints)[i];
/*?*/ 			if( nTxtStartIdx < ( nAttrStartIdx = *pHt->GetStart() ) )
/*?*/ 				break;		// ueber das Textende, da nLen == 0
/*?*/
/*?*/ 			pEndIdx = pHt->GetEnd();
/*?*/ 			if( pEndIdx )
/*?*/ 			{
/*?*/ 				if( ( *pEndIdx > nTxtStartIdx ||
/*?*/ 					  ( *pEndIdx == nTxtStartIdx &&
/*?*/ 						nAttrStartIdx == nTxtStartIdx ) ) )
/*?*/ 				{
/*?*/ 					if( RES_TXTATR_REFMARK != ( nWhich = pHt->Which()) )
/*?*/ 					{
/*?*/ 						// Attribut liegt im Bereich, also kopieren
/*?*/ 						if( 0 != ( pNewHt = pDest->Insert( pHt->GetAttr(),
/*?*/ 												nOldPos, nOldPos ) ) )
/*?*/ 							lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
/*?*/ 					}
/*?*/ 					else if( !pOtherDoc ? GetDoc()->IsCopyIsMove()
/*?*/ 										: 0 == pOtherDoc->GetRefMark(
/*?*/ 										pHt->GetRefMark().GetRefName() ) )
/*?*/ 						pDest->Insert( pHt->GetAttr(), nOldPos, nOldPos );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	if( this != pDest )
/*N*/ 	{
/*N*/ 		// Frames benachrichtigen, sonst verschwinden die Ftn-Nummern
/*N*/ 		SwUpdateAttr aHint( nOldPos, nOldPos, 0 );
/*N*/ 		pDest->Modify( 0, &aHint );
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*	SwTxtNode::Copy()
|*	Beschreibung		kopiert Zeichen und Attibute in pDest,
|*						wird angehaengt
*************************************************************************/

/*N*/ void SwTxtNode::Copy( SwTxtNode *pDest, const SwIndex &rStart, xub_StrLen nLen )
/*N*/ {
/*N*/ 	SwIndex aIdx( pDest, pDest->aText.Len() );
/*N*/ 	Copy( pDest, aIdx, rStart, nLen );
/*N*/ }

/*N*/ void SwTxtNode::Copy( SwTxtNode *pDest, const SwIndex &rDestStart,
/*N*/ 					  const SwIndex &rStart, xub_StrLen nLen)
/*N*/ {
/*N*/ 	xub_StrLen nTxtStartIdx = rStart.GetIndex();
/*N*/ 	xub_StrLen nDestStart = rDestStart.GetIndex();		// alte Pos merken
/*N*/
/*N*/ 	if( !nLen )
/*N*/ 	{
/*N*/ 		// wurde keine Laenge angegeben, dann Kopiere die Attribute
/*N*/ 		// an der Position rStart.
/*N*/ 		CopyAttr( pDest, nTxtStartIdx, nDestStart );
/*N*/
/*N*/ 		// harte Absatz umspannende Attribute kopieren
/*N*/ 		if( GetpSwAttrSet() )
/*N*/ 		{
/*N*/ 			// alle, oder nur die CharAttribute ?
/*N*/ 			if( nDestStart || pDest->GetpSwAttrSet() ||
/*N*/ 				nLen != pDest->GetTxt().Len() )
/*N*/ 			{
/*?*/ 				SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
/*?*/ 									RES_CHRATR_BEGIN, RES_CHRATR_END-1,
/*?*/ 									RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
/*?*/ 									RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
/*?*/ 									RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
/*?*/ 									0 );
/*?*/ 				aCharSet.Put( *GetpSwAttrSet() );
/*N*/ 				if( aCharSet.Count() )
/*N*/ 					pDest->SetAttr( aCharSet, nDestStart, nDestStart );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				GetpSwAttrSet()->CopyToModify( *pDest );
/*N*/ 		}
/*N*/
/*N*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	// 1. Text kopieren
/*N*/ 	xub_StrLen i = pDest->aText.Len() - nDestStart;
/*N*/ 	//JP 15.02.96: Bug 25537 - Attributbehandlung am Ende fehlt! Darum
/*N*/ 	//				ueber die InsertMethode den Text einfuegen und nicht
/*N*/ 	//				selbst direkt
/*N*/ 	pDest->Insert( aText.Copy( nTxtStartIdx, nLen ), rDestStart,
/*N*/ 													INS_EMPTYEXPAND );
/*N*/
/*N*/ 	// um reale Groesse Updaten !
/*N*/ 	nLen = pDest->aText.Len() - nDestStart - i;
/*N*/ 	if( !nLen )				   					// String nicht gewachsen ??
/*N*/ 		return;
/*N*/
/*N*/ 	i = 0;
/*N*/ 	const xub_StrLen *pEndIdx = 0;
/*N*/ 	xub_StrLen nAttrStartIdx = 0;
/*N*/ 	const SwTxtAttr *pHt = 0;
/*N*/ 	SwTxtAttr *pNewHt = 0;
/*N*/
/*N*/ 	SwDoc* pOtherDoc = pDest->GetDoc();
/*N*/ 	if( pOtherDoc == GetDoc() )
/*N*/ 		pOtherDoc = 0;
/*N*/
/*N*/ 	// harte Absatz umspannende Attribute kopieren
/*N*/ 	if( GetpSwAttrSet() )
/*N*/ 	{
/*N*/ 		// alle, oder nur die CharAttribute ?
/*N*/ 		if( nDestStart || pDest->GetpSwAttrSet() ||
/*N*/ 			nLen != pDest->GetTxt().Len() )
/*N*/ 		{
/*N*/ 			SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
/*N*/ 								RES_CHRATR_BEGIN, RES_CHRATR_END-1,
/*N*/ 								RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
/*N*/ 								RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
/*N*/ 								RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
/*N*/ 								0 );
/*N*/ 			aCharSet.Put( *GetpSwAttrSet() );
/*N*/ 			if( aCharSet.Count() )
/*N*/ 				pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			GetpSwAttrSet()->CopyToModify( *pDest );
/*N*/ 	}
/*N*/
/*N*/
/*N*/   const BOOL bUndoNodes = FALSE; // !pOtherDoc && GetDoc()->GetUndoNds() == &GetNodes();
/*N*/
/*N*/ 	// Ende erst jetzt holen, weil beim Kopieren in sich selbst der
/*N*/ 	// Start-Index und alle Attribute vorher aktualisiert werden.
/*N*/ 	nTxtStartIdx = rStart.GetIndex();
/*N*/ 	xub_StrLen nEnd = nTxtStartIdx + nLen;
/*N*/
/*N*/ 	// 2. Attribute kopieren
/*N*/ 	// durch das Attribute-Array, bis der Anfang des Geltungsbereiches
/*N*/ 	// des Attributs hinter dem zu kopierenden Bereich liegt
/*N*/ 	USHORT nWhich, nSize = pSwpHints ? pSwpHints->Count() : 0;
/*N*/ 	xub_StrLen nAttrStt, nAttrEnd;
/*N*/
/*N*/ 	// wird in sich selbst kopiert, dann kann beim Einfuegen ein
/*N*/ 	// Attribut geloescht werden. Darum erst ins Tmp-Array kopieren und
/*N*/ 	// dann erst ins eigene uebertragen.
/*N*/ 	SwpHts aArr( 5 );
/*N*/
/*N*/ 	// Del-Array fuer alle RefMarks ohne Ausdehnung
/*N*/ 	SwpHts aRefMrkArr;
/*N*/
/*N*/ 		//Achtung: kann ungueltig sein!!
/*N*/ 	while( ( i < nSize ) &&
/*N*/ 		   ((nAttrStartIdx = *(*pSwpHints)[i]->GetStart()) < nEnd) )
/*N*/ 	{
/*N*/ 		pHt = (*pSwpHints)[i];
/*N*/ 		pNewHt = 0;
/*N*/ 		pEndIdx = pHt->GetEnd();
/*N*/ 		nWhich = pHt->Which();
/*N*/
/*N*/ 		// JP 26.04.94: REFMARK's werden nie kopiert. Hat das Refmark aber
/*N*/ 		//				keinen Bereich umspannt, so steht im Text ein 255
/*N*/ 		//				dieses muss entfernt werden. Trick: erst kopieren,
/*N*/ 		//				erkennen und sammeln, nach dem kopieren Loeschen.
/*N*/ 		//				Nimmt sein Zeichen mit ins Grab !!
/*N*/ 		// JP 14.08.95:	Duerfen RefMarks gemovt werden?
/*N*/ 		int bCopyRefMark = RES_TXTATR_REFMARK == nWhich && ( bUndoNodes ||
/*N*/ 						   (!pOtherDoc ? GetDoc()->IsCopyIsMove()
/*N*/ 									  : 0 == pOtherDoc->GetRefMark(
/*N*/ 										pHt->GetRefMark().GetRefName() )));
/*N*/
/*N*/ 		if( pEndIdx && RES_TXTATR_REFMARK == nWhich && !bCopyRefMark )
/*N*/ 		{
/*N*/ 			++i;
/*N*/ 			continue;
/*N*/ 		}
/*N*/
/*N*/ 		if( nAttrStartIdx < nTxtStartIdx )
/*N*/ 		{
/*N*/ 			// Anfang liegt vor dem Bereich
/*?*/ 			if( pEndIdx && ( nAttrEnd = *pEndIdx ) > nTxtStartIdx )
/*?*/ 			{
/*?*/ 				// Attribut mit einem Bereich
/*?*/ 				// und das Ende des Attribut liegt im Bereich
/*?*/ 				nAttrStt = nDestStart;
/*?*/ 				nAttrEnd = nAttrEnd > nEnd
/*?*/ 							? rDestStart.GetIndex()
/*?*/ 							: nDestStart + nAttrEnd - nTxtStartIdx;
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				++i;
/*?*/ 				continue;
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			// der Anfang liegt innerhalb des Bereiches
/*N*/ 			nAttrStt = nDestStart + ( nAttrStartIdx - nTxtStartIdx );
/*N*/ 			if( pEndIdx )
/*N*/ 				nAttrEnd = *pEndIdx > nEnd
/*N*/ 							? rDestStart.GetIndex()
/*N*/ 							: nDestStart + ( *pEndIdx - nTxtStartIdx );
/*N*/ 			else
/*N*/ 				nAttrEnd = nAttrStt;
/*N*/ 		}
/*N*/
/*N*/ 		if( pDest == this )
/*N*/ 		{
/*N*/ 			// die Daten kopieren
/*?*/ 			pNewHt = MakeTxtAttr( pHt->GetAttr(), nAttrStt, nAttrEnd );
/*?*/
/*?*/ //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*?*/ //JP 23.04.95:	erstmal so gesondert hier behandeln. Am Besten ist es
/*?*/ //				aber im CopyFtn wenn die pDestFtn keinen StartNode hat,
/*?*/ //				sich diesen dann anlegt.
/*?*/ //				Aber so kurz vor der BETA besser nicht anfassen.
/*?*/ 			if( RES_TXTATR_FTN == nWhich )
/*?*/ 			{
/*?*/ 				SwTxtFtn* pFtn = (SwTxtFtn*)pNewHt;
/*?*/ 				pFtn->ChgTxtNode( this );
/*?*/ 				pFtn->MakeNewTextSection( GetNodes() );
/*?*/ 				lcl_CopyHint( nWhich, pHt, pFtn, 0, 0 );
/*?*/ 				pFtn->ChgTxtNode( 0 );
/*?*/ 			}
/*?*/ 			else
/*?*/ //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*?*/ 			{
/*?*/ 				lcl_CopyHint( nWhich, pHt, pNewHt, 0, pDest );
/*?*/ 			}
/*?*/ 			aArr.C40_INSERT( SwTxtAttr, pNewHt, aArr.Count() );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			pNewHt = pDest->Insert( pHt->GetAttr(), nAttrStt,
/*N*/ 									nAttrEnd, SETATTR_NOTXTATRCHR );
/*N*/ 			if( pNewHt )
/*N*/ 				lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
/*N*/ 			else if( !pEndIdx )
/*N*/ 			{
/*N*/ 				// Attribut wurde nicht kopiert, hat seinen Inhalt mitgenommen!
/*N*/ 				// Damit der rest aber korrekt kopiert werden kann, muss das
/*N*/ 				// Zeichen wieder an der Position stehen. Darum hier ein
/*N*/ 				// "Dummy-TextAttribut" einfuegen, wird am Ende wieder entfernt!
/*?*/ 				pNewHt = pDest->Insert( SwFmtHardBlank( 0xB7 ), nAttrStt, 0
/*?*/ 										/*???, INS_NOHINTEXPAND*/ );
/*?*/ 				aRefMrkArr.C40_INSERT( SwTxtAttr, pNewHt, aRefMrkArr.Count() );
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		if( RES_TXTATR_REFMARK == nWhich && !pEndIdx && !bCopyRefMark )
/*N*/ 		{
/*?*/ 			aRefMrkArr.C40_INSERT( SwTxtAttr, pNewHt, aRefMrkArr.Count() );
/*N*/ 		}
/*N*/
/*N*/ 		++i;
/*N*/ 	}
/*N*/
/*N*/ 	// nur falls im Array Attribute stehen (kann nur beim Kopieren
/*N*/ 	// sich selbst passieren!!)
/*N*/ 	for( i = 0; i < aArr.Count(); ++i )
/*?*/ 		Insert( aArr[ i ], SETATTR_NOTXTATRCHR );
/*N*/
/*N*/ 	if( pDest->GetpSwpHints() )
/*N*/ 		for( i = 0; i < aRefMrkArr.Count(); ++i )
/*N*/ 		{
/*?*/ 			pNewHt = aRefMrkArr[i];
/*?*/ 			if( pNewHt->GetEnd() )
/*?*/ 			{
/*?*/ 				pDest->GetpSwpHints()->Delete( pNewHt );
/*?*/ 				pDest->DestroyAttr( pNewHt );
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				const SwIndex aIdx( pDest, *pNewHt->GetStart() );
/*?*/ 				pDest->Erase( aIdx, 1 );
/*?*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 	CHECK_SWPHINTS(this);
/*N*/ }


/*
 * Rudimentaeres Editieren, damit die SwDoc-Insert-Methoden
 * funktionieren.
 */

/*M*/ SwTxtNode& SwTxtNode::Insert( const XubString	&rStr,
/*M*/ 							  const SwIndex &rIdx, const USHORT nMode )
/*M*/ {
/*M*/ 	ASSERT( rIdx <= aText.Len(), "Array ueberindiziert." );
/*M*/ 	ASSERT( (ULONG)aText.Len() + (ULONG)rStr.Len() <= STRING_LEN,
/*M*/ 			"STRING_LEN ueberschritten." );
/*M*/
/*M*/ 	xub_StrLen aPos = rIdx.GetIndex();
/*M*/ 	xub_StrLen nLen = aText.Len() - aPos;
/*M*/
/*M*/     // sequence input checking
/*M*/     sal_Bool bInputChecked = sal_False;
/*M*/
/*M*/     // We check only buffers which contain less than MAX_SEQUENCE_CHECK_LEN
/*M*/     // characters. This is for performance reasons, because a "copy and paste"
/*M*/     // can give us a really big input string.
/*N*/     SvtCTLOptions& rCTLOptions = SW_MOD()->GetCTLOptions();
/*N*/     if ( rCTLOptions.IsCTLFontEnabled() &&
/*N*/          rCTLOptions.IsCTLSequenceChecking() && aPos &&
/*M*/          rStr.Len() < MAX_SEQUENCE_CHECK_LEN && pBreakIt->xBreak.is() &&
/*M*/          ::com::sun::star::i18n::ScriptType::COMPLEX ==
/*M*/          pBreakIt->xBreak->getScriptType( rStr, 0 ) )
/*M*/     {
/*M*/         // generate new sequence input checker if not already done
/*M*/         if ( ! pCheckIt )
/*M*/             pCheckIt = new SwCheckIt;
/*M*/
/*M*/         if ( pCheckIt->xCheck.is() )
/*M*/         {
/*M*/             xub_StrLen nI = 0;
/*M*/             xub_StrLen nTmpPos = aPos;
/*M*/             xub_Unicode cChar;
/*M*/
/*M*/             while ( nI < rStr.Len() )
/*M*/             {
/*M*/                 cChar = rStr.GetChar( nI++ );
/*M*/                 if ( pCheckIt->xCheck->checkInputSequence(
/*M*/                         aText, nTmpPos - 1, cChar,
/*N*/                         ::com::sun::star::i18n::InputSequenceCheckMode::BASIC ) )
/*M*/                 {
/*M*/                     // character can be inserted
/*M*/                     aText.Insert( cChar, nTmpPos++ );
/*M*/                 }
/*M*/             }
/*M*/             bInputChecked = sal_True;
/*M*/         }
/*M*/     }
/*M*/
/*M*/     if ( ! bInputChecked )
/*M*/         aText.Insert( rStr, aPos );
/*M*/
/*M*/     nLen = aText.Len() - aPos - nLen;
/*M*/ 	if( !nLen )							// String nicht gewachsen ??
/*M*/ 		return *this;
/*M*/ 	Update( rIdx, nLen );		// um reale Groesse Updaten !
/*M*/
/*M*/ 	// analog zu Insert(char) in txtedt.cxx:
/*M*/ 	// 1) bei bHintExp leere Hints an rIdx.GetIndex suchen und aufspannen
/*M*/ 	// 2) bei bHintExp == FALSE mitgezogene Feldattribute zuruecksetzen
/*M*/
/*M*/ 	register USHORT i;
/*M*/
/*M*/ 	if( pSwpHints )
/*M*/ 	{
/*M*/        for( i = 0; i < pSwpHints->Count() &&
/*M*/                 rIdx >= *(*pSwpHints)[i]->GetStart(); ++i )
/*M*/ 		{
/*M*/ 			SwTxtAttr *pHt = pSwpHints->GetHt( i );
/*M*/ 			xub_StrLen* pEndIdx = pHt->GetEnd();
/*M*/ 			if( !pEndIdx )
/*M*/ 				continue;
/*M*/
/*M*/ 			if( rIdx == *pEndIdx )
/*M*/ 			{
/*M*/ 				if( nMode & INS_NOHINTEXPAND || pHt->DontExpand() )
/*M*/ 				{
/*M*/ 					// bei leeren Attributen auch Start veraendern
/*M*/ 					if( rIdx == *pHt->GetStart() )
/*M*/ 						*pHt->GetStart() -= nLen;
/*M*/ 					*pEndIdx -= nLen;
/*M*/ 				}
/*M*/ 					// leere Hints an rIdx.GetIndex ?
/*M*/ 				else if( nMode & INS_EMPTYEXPAND &&
/*M*/ 						*pEndIdx == *pHt->GetStart() )
/*M*/ 				{
/*M*/ 					*pHt->GetStart() -= nLen;
/*M*/
/*M*/ 					// 8484: Symbol von 0-4, Roman von 4-6,
/*M*/ 					//		 neuer Hint: Roman 4-4
/*M*/
/*M*/ 					// - while ... die Vorgaenger ueberpruefen:
/*M*/ 					// wenn gleiches Ende und gleicher Which
/*M*/ 					// => das Ende des gefundenen zuruecksetzen
/*M*/ 					const USHORT nWhich = pHt->Which();
/*M*/ 					SwTxtAttr *pFound;
/*M*/ 					xub_StrLen *pFoundEnd;
/*M*/ 					for( USHORT j = 0; j < i; ++j )
/*M*/ 						if( 0 != (pFound = pSwpHints->GetHt( j )) &&
/*M*/ 							nWhich == pFound->Which() &&
/*M*/ 							0 != ( pFoundEnd = pFound->GetEnd() ) &&
/*M*/ 							rIdx == *pFoundEnd )
/*M*/ 						{
/*M*/ 							*pFoundEnd -= nLen;
/*M*/ 							const USHORT nAktHtLen = pSwpHints->Count();
/*M*/ 							pSwpHints->DeleteAtPos(j);
/*M*/ 							Insert( pFound, SETATTR_NOHINTADJUST );
/*M*/ 							// AMA: Sicher ist sicher, falls pFound weiter hinten
/*M*/ 							// einsortiert wurde, koennte sonst die neue Position
/*M*/ 							// j vergessen werden!
/*M*/ 							if ( j ) --j;
/*M*/ 							// falls Attribute zusammengefasst werden, steht
/*M*/ 							// der "Index" ins Array falsch !
/*M*/ 							i -= nAktHtLen - pSwpHints->Count();
/*M*/ 							// Insert und Delete ?
/*M*/ 						}
/*M*/
/*M*/ 					// ist unser Attribut ueberhaupt noch vorhanden ?
/*M*/ 					if( pHt == pSwpHints->GetHt( i ) )
/*M*/ 					{
/*M*/ 						const USHORT nAktLen = pSwpHints->Count();
/*M*/ 						pSwpHints->DeleteAtPos(i);
/*M*/ 						Insert( pHt, SETATTR_NOHINTADJUST );
/*M*/ 						if( nAktLen > pSwpHints->Count() && i )
/*M*/ 							--i;
/*M*/ 					}
/*M*/ 					continue;
/*M*/ 				}
/*M*/ 				else
/*M*/ 					continue;
/*M*/ 				pSwpHints->DeleteAtPos(i);
/*M*/ 				Insert( pHt, SETATTR_NOHINTADJUST );
/*M*/ 			}
/*M*/ 			if ( !(nMode & INS_NOHINTEXPAND) &&
/*M*/ 				 rIdx == nLen && *pHt->GetStart() == rIdx.GetIndex() &&
/*M*/ 				 !pHt->IsDontExpandStartAttr() )
/*M*/ 			{
/*M*/ 				// Kein Feld, am Absatzanfang, HintExpand
/*M*/ 				pSwpHints->DeleteAtPos(i);
/*M*/ 				*pHt->GetStart() -= nLen;
/*M*/ 				Insert( pHt, SETATTR_NOHINTADJUST );
/*M*/ 			}
/*M*/ 		}
/*M*/ 		if ( pSwpHints->CanBeDeleted() )
/*M*/ 			DELETEZ( pSwpHints );
/*M*/ 	}
/*M*/
/*M*/ 	if ( GetDepends() )
/*M*/ 	{
/*M*/ 		SwInsTxt aHint( aPos, nLen );
/*M*/ 		SwModify::Modify( 0, &aHint );
/*M*/ 	}
/*M*/
/*M*/ 	CHECK_SWPHINTS(this);
/*M*/ 	return *this;
/*M*/ }

/*************************************************************************
|*
|*	SwTxtNode::Cut()
|*
|*	Beschreibung		text.doc
|*	Ersterstellung		VB 20.03.91
|*	Letzte Aenderung	JP 11.08.94
|*
*************************************************************************/

/*N*/ void SwTxtNode::Cut( SwTxtNode *pDest, const SwIndex &rStart, xub_StrLen nLen )
/*N*/ {
/*N*/ 	if(pDest)
/*N*/ 	{
/*N*/ 		SwIndex aDestStt( pDest, pDest->GetTxt().Len() );
/*N*/ 		_Cut( pDest, aDestStt, rStart, nLen, FALSE );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		Erase( rStart, nLen );
/*N*/ }


/*N*/ void SwTxtNode::_Cut( SwTxtNode *pDest, const SwIndex& rDestStart,
/*N*/ 					 const SwIndex &rStart, xub_StrLen nLen, BOOL bUpdate )
/*N*/ {
/*N*/ 	if(!pDest)
/*N*/ 	{
/*?*/ 		Erase( rStart, nLen );
/*?*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	// nicht im Dokument verschieben ?
/*N*/ 	if( GetDoc() != pDest->GetDoc() )
/*N*/ 	{
/*?*/ 		Copy( pDest, rDestStart, rStart, nLen);
/*?*/ 		Erase(rStart,nLen);
/*?*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	if( !nLen )
/*N*/ 	{
/*N*/ 		// wurde keine Laenge angegeben, dann Kopiere die Attribute
/*N*/ 		// an der Position rStart.
/*N*/ 		CopyAttr( pDest, rStart.GetIndex(), rDestStart.GetIndex() );
/*N*/ 		return;
/*N*/ 	}
/*N*/
/*N*/ 	xub_StrLen nTxtStartIdx = rStart.GetIndex();
/*N*/ 	xub_StrLen nDestStart = rDestStart.GetIndex();		// alte Pos merken
/*N*/ 	xub_StrLen nInitSize = pDest->aText.Len();
/*N*/
/*N*/ 	xub_StrLen *pEndIdx = 0;
/*N*/ 	xub_StrLen nAttrStartIdx = 0;
/*N*/ 	SwTxtAttr *pHt = 0;
/*N*/ 	SwTxtAttr *pNewHt = 0;
/*N*/
/*N*/ 	// wird in sich selbst verschoben, muss es gesondert behandelt werden !!
/*N*/ 	if( pDest == this )
/*N*/ 	{
/*?*/ 		aText.Insert( aText, nTxtStartIdx, nLen, nDestStart );
/*?*/ 		aText.Erase( nTxtStartIdx + (nDestStart<nTxtStartIdx ? nLen : 0), nLen );
/*?*/
/*?*/ 		xub_StrLen nEnd = rStart.GetIndex() + nLen;
/*?*/ 		USHORT n;
/*?*/
/*?*/ 		// dann suche mal alle Attribute zusammen, die im verschobenen
/*?*/ 		// Bereich liegen. Diese werden in das extra Array verschoben,
/*?*/ 		// damit sich die Indizies beim Updaten nicht veraendern !!!
/*?*/ 		SwIndexReg aTmpRegArr;
/*?*/ 		SwpHts aArr( 5 );
/*?*/
/*?*/ 		// 2. Attribute verschieben
/*?*/ 		// durch das Attribute-Array, bis der Anfang des Geltungsbereiches
/*?*/ 		// des Attributs hinter dem zu verschiebenden Bereich liegt
/*?*/ 		USHORT nAttrCnt = 0, nWhich;
/*?*/ 		while(	pSwpHints && nAttrCnt < pSwpHints->Count() &&
/*?*/ 				(nAttrStartIdx = *(pHt = pSwpHints->GetHt(nAttrCnt))->
/*?*/ 									GetStart()) < nEnd )
/*?*/ 		{
/*?*/ 			pNewHt = 0;
/*?*/ 			pEndIdx = pHt->GetEnd();
/*?*/
/*?*/ 			if(nAttrStartIdx < nTxtStartIdx)
/*?*/ 			{
/*?*/ 				// Anfang liegt vor dem Bereich
/*?*/ 				if( RES_TXTATR_REFMARK != ( nWhich = pHt->Which() ) &&
/*?*/ 					pEndIdx && *pEndIdx > nTxtStartIdx )
/*?*/ 				{
/*?*/ 					// Attribut mit einem Bereich
/*?*/ 					// und das Ende des Attribut liegt im Bereich
/*?*/ 					pNewHt = MakeTxtAttr( pHt->GetAttr(), 0,
/*?*/ 										*pEndIdx > nEnd
/*?*/ 											? nLen
/*?*/ 											: *pEndIdx - nTxtStartIdx );
/*?*/ 				}
/*?*/ 			}
/*?*/ 			else
/*?*/ 			{
/*?*/ 				// der Anfang liegt vollstaendig im Bereich
/*?*/ 				if( !pEndIdx || *pEndIdx < nEnd )
/*?*/ 				{
/*?*/ 					// Attribut verschieben
/*?*/ 					pSwpHints->Delete( pHt );
/*?*/ 					// die Start/End Indicies neu setzen
/*?*/ 					*pHt->GetStart() = nAttrStartIdx - nTxtStartIdx;
/*?*/ 					if( pEndIdx )
/*?*/ 						*pHt->GetEnd() = *pEndIdx - nTxtStartIdx;
/*?*/ 					aArr.C40_INSERT( SwTxtAttr, pHt, aArr.Count() );
/*?*/ 					continue;			// while-Schleife weiter, ohne ++ !
/*?*/ 				}
/*?*/ 					// das Ende liegt dahinter
/*?*/ 				else if( RES_TXTATR_REFMARK != ( nWhich = pHt->Which() ))
/*?*/ 				{
/*?*/ 					pNewHt = MakeTxtAttr( pHt->GetAttr(),
/*?*/ 							nAttrStartIdx - nTxtStartIdx,
/*?*/ 							!pEndIdx ? 0
/*?*/ 									 : ( *pEndIdx > nEnd
/*?*/ 											? nLen
/*?*/ 											: *pEndIdx - nTxtStartIdx ));
/*?*/ 				}
/*?*/ 			}
/*?*/ 			if( pNewHt )
/*?*/ 			{
/*?*/ 				// die Daten kopieren
/*?*/ 				lcl_CopyHint( nWhich, pHt, pNewHt, 0, this );
/*?*/ 				aArr.C40_INSERT( SwTxtAttr, pNewHt, aArr.Count() );
/*?*/ 			}
/*?*/ 			++nAttrCnt;
/*?*/ 		}
/*?*/
/*?*/ 		if( bUpdate )
/*?*/ 			// Update aller Indizies
/*?*/ 			Update( rDestStart, nLen );
/*?*/ #ifdef CUTNOEXPAND
/*?*/ 		else
/*?*/ 			// wird am Ende eingefuegt, nur die Attribut-Indizies verschieben
/*?*/ 			if( 0 < nLen && 0 < nInitSize && pSwpHints )
/*?*/ 			{
/*?*/ 				// siehe nach, ob an der Einfuegeposition das Ende eines
/*?*/ 				// Attributes stand. Ist es kein Feld, muss es expandiert werden !!!
/*?*/ 				for( n = 0; n < pSwpHints->Count(); n++ )
/*?*/ 				{
/*?*/ 					pHt = pSwpHints->GetHt(n);
/*?*/ 					if( 0 != ( pEndIdx = pHt->GetEnd() ) &&
/*?*/ 						*pEndIdx == nInitSize )
/*?*/ 						*pEndIdx += nLen;
/*?*/ 				}
/*?*/ 			}
/*?*/ #endif
/*?*/ 		CHECK_SWPHINTS(this);
/*?*/
/*?*/ 		Update( rStart, nLen, TRUE );
/*?*/
/*?*/ 		CHECK_SWPHINTS(this);
/*?*/
/*?*/ 		// dann setze die kopierten/geloeschten Attribute in den Node
/*?*/ 		if( nDestStart <= nTxtStartIdx )
/*?*/ 			nTxtStartIdx += nLen;
/*?*/ 		else
/*?*/ 			nDestStart -= nLen;
/*?*/
/*?*/ 		for( n = 0; n < aArr.Count(); ++n )
/*?*/ 		{
/*?*/ 			pNewHt = aArr[n];
/*?*/ 			*pNewHt->GetStart() = nDestStart + *pNewHt->GetStart();
/*?*/ 			if( 0 != ( pEndIdx = pNewHt->GetEnd() ))
/*?*/ 				*pEndIdx = nDestStart + *pEndIdx;
/*?*/ 			Insert( pNewHt, SETATTR_NOTXTATRCHR );
/*?*/ 		}
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		xub_StrLen i = nInitSize - nDestStart;
/*N*/ 		pDest->aText.Insert( aText, nTxtStartIdx, nLen, nDestStart );
/*N*/ 		aText.Erase( nTxtStartIdx, nLen );
/*N*/ 		nLen = pDest->aText.Len() - nDestStart - i;	 // um reale Groesse Updaten !
/*N*/ 		if( !nLen )					// String nicht gewachsen ??
/*N*/ 			return;
/*N*/
/*N*/ 		i = 0;
/*N*/
/*N*/ 		if( bUpdate )
/*N*/ 			// Update aller Indizies
/*N*/ 			pDest->Update( rDestStart, nLen);
/*N*/ #ifdef CUTNOEXPAND
/*N*/ 		else
/*N*/ 			// wird am Ende eingefuegt, nur die Attribut-Indizies verschieben
/*N*/ 			if( 0 < nLen && 0 < nInitSize && pDest->pSwpHints )
/*N*/ 			{
/*N*/ 				// siehe nach, ob an der Einfuegeposition das Ende eines
/*N*/ 				// Attributes stand. Ist es kein Feld, muss es expandiert werden !!!
/*N*/ 				for( USHORT n = 0; n < pDest->pSwpHints->Count(); n++ )
/*N*/ 				{
/*N*/ 					pHt = pDest->pSwpHints->GetHt(n);
/*N*/ 					if( 0 != ( pEndIdx = pHt->GetEnd() ) &&
/*N*/ 						*pEndIdx == nInitSize )
/*N*/ 						*pEndIdx += nLen;
/*N*/ 				}
/*N*/ 			}
/*N*/ #endif
/*N*/ 		CHECK_SWPHINTS(pDest);
/*N*/
/*N*/ 		USHORT nEnd = rStart.GetIndex() + nLen;
/*N*/ 		SwDoc* pOtherDoc = pDest->GetDoc();
/*N*/ 		if( pOtherDoc == GetDoc() )
/*N*/ 			pOtherDoc = 0;
/*N*/       const BOOL bUndoNodes = FALSE; // !pOtherDoc && GetDoc()->GetUndoNds() == &GetNodes();
/*N*/
/*N*/ 		// harte Absatz umspannende Attribute kopieren
/*N*/ 		if( GetpSwAttrSet() )
/*N*/ 		{
/*N*/ 			// alle, oder nur die CharAttribute ?
/*N*/ 			if( nInitSize || pDest->GetpSwAttrSet() ||
/*N*/ 				nLen != pDest->GetTxt().Len() )
/*N*/ 			{
/*?*/ 				SfxItemSet aCharSet( pDest->GetDoc()->GetAttrPool(),
/*?*/ 									RES_CHRATR_BEGIN, RES_CHRATR_END-1,
/*?*/ 									RES_TXTATR_CHARFMT, RES_TXTATR_CHARFMT,
/*?*/ 									RES_TXTATR_INETFMT, RES_TXTATR_INETFMT,
/*?*/ 									RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END-1,
/*?*/ 									0 );
/*?*/ 				aCharSet.Put( *GetpSwAttrSet() );
/*?*/ 				if( aCharSet.Count() )
/*?*/ 					pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
/*N*/ 			}
/*N*/ 			else
/*N*/ 				GetpSwAttrSet()->CopyToModify( *pDest );
/*N*/ 		}
/*N*/
/*N*/ 		// 2. Attribute verschieben
/*N*/ 		// durch das Attribute-Array, bis der Anfang des Geltungsbereiches
/*N*/ 		// des Attributs hinter dem zu verschiebenden Bereich liegt
/*N*/ 		USHORT nAttrCnt = 0, nWhich;
/*N*/ 		while( pSwpHints && nAttrCnt < pSwpHints->Count() &&
/*N*/ 				( (nAttrStartIdx = *(pHt = pSwpHints->GetHt(nAttrCnt))->
/*N*/ 									GetStart()) < nEnd ) )
/*N*/ 		{
/*N*/ 			pNewHt = 0;
/*N*/ 			pEndIdx = pHt->GetEnd();
/*N*/
/*N*/ 			if(nAttrStartIdx < nTxtStartIdx)
/*N*/ 			{
/*N*/ 				// Anfang liegt vor dem Bereich
/*?*/ 				if( ( RES_TXTATR_REFMARK != ( nWhich = pHt->Which() )
/*?*/ 					|| bUndoNodes ) && pEndIdx && *pEndIdx > nTxtStartIdx )
/*?*/ 				{
/*?*/ 					// Attribut mit einem Bereich
/*?*/ 					// und das Ende des Attribut liegt im Bereich
/*?*/ 					pNewHt = pDest->MakeTxtAttr( pHt->GetAttr(), nDestStart,
/*?*/ 										nDestStart + (
/*?*/ 										*pEndIdx > nEnd
/*?*/ 											? nLen
/*?*/ 											: *pEndIdx - nTxtStartIdx ) );
/*?*/ 				}
/*N*/ 			}
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// der Anfang liegt vollstaendig im Bereich
/*N*/ 				nWhich = pHt->Which();
/*N*/ 				if( !pEndIdx || *pEndIdx < nEnd ||
/*N*/ 					( !pOtherDoc && !bUndoNodes && RES_TXTATR_REFMARK
/*N*/ 						== nWhich ) )
/*N*/ 				{
/*N*/ 					// Attribut verschieben
/*N*/ 					pSwpHints->Delete( pHt );
/*N*/ 					// die Start/End Indicies neu setzen
/*N*/ 					*pHt->GetStart() =
/*N*/ 							nDestStart + (nAttrStartIdx - nTxtStartIdx);
/*N*/ 					if( pEndIdx )
/*N*/ 						*pHt->GetEnd() = nDestStart + (
/*N*/ 										*pEndIdx > nEnd
/*N*/ 											? nLen
/*N*/ 											: *pEndIdx - nTxtStartIdx );
/*N*/ 					pDest->Insert( pHt, SETATTR_NOTXTATRCHR | SETATTR_DONTREPLACE );
/*N*/ 					continue;			// while-Schleife weiter, ohne ++ !
/*N*/ 				}
/*N*/ 					// das Ende liegt dahinter
/*N*/ 				else if( RES_TXTATR_REFMARK != nWhich || bUndoNodes )
/*N*/ 				{
/*N*/ 					pNewHt = MakeTxtAttr( pHt->GetAttr(),
/*N*/ 							nDestStart + (nAttrStartIdx - nTxtStartIdx),
/*N*/ 							!pEndIdx ? 0
/*N*/ 									 : nDestStart + ( *pEndIdx > nEnd
/*N*/ 											? nLen
/*N*/ 											: *pEndIdx - nTxtStartIdx ));
/*N*/ 				}
/*N*/ 			}
/*N*/ 			if ( pNewHt )
/*N*/ 			{
/*N*/ 				if( pDest->Insert( pNewHt, SETATTR_NOTXTATRCHR | SETATTR_DONTREPLACE ))
/*N*/ 					lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
/*N*/ 			}
/*N*/ 			++nAttrCnt;
/*N*/ 		}
/*N*/ 		// sollten jetzt noch leere Attribute rumstehen, dann haben diese
/*N*/ 		// eine hoehere Praezedenz. Also herausholen und das Array updaten.
/*N*/ 		// Die dabei entstehenden leeren Hints werden von den gesicherten
/*N*/ 		// "uebergeplaettet".	(Bug: 6977)
/*N*/ 		if( pSwpHints && nAttrCnt < pSwpHints->Count() )
/*N*/ 		{
/*?*/ 			SwpHts aArr( 5 );
/*?*/ 			for( ; nAttrCnt < pSwpHints->Count() &&
/*?*/ 					nEnd == *(pHt = pSwpHints->GetHt(nAttrCnt))->GetStart();
/*?*/ 				   ++nAttrCnt )
/*?*/ 			{
/*?*/ 				if( 0 != ( pEndIdx = pHt->GetEnd() ) && *pEndIdx == nEnd )
/*?*/ 				{
/*?*/ 					aArr.C40_INSERT( SwTxtAttr, pHt, aArr.Count() );
/*?*/ 					pSwpHints->Delete( pHt );
/*?*/ 					--nAttrCnt;
/*?*/ 				}
/*?*/ 			}
/*?*/ 			Update( rStart, nLen, TRUE );
/*?*/
/*?*/ 			for( nAttrCnt = 0; nAttrCnt < aArr.Count(); ++nAttrCnt )
/*?*/ 			{
/*?*/ 				pHt = aArr[ nAttrCnt ];
/*?*/ 				*pHt->GetStart() = *pHt->GetEnd() = rStart.GetIndex();
/*?*/ 				Insert( pHt );
/*?*/ 			}
/*N*/ 		}
/*N*/ 		else
/*N*/ 			Update( rStart, nLen, TRUE );
/*N*/
/*N*/ 		CHECK_SWPHINTS(this);
/*N*/ 	}
/*N*/
/*N*/ 	if( pSwpHints && pSwpHints->CanBeDeleted() )
/*N*/ 		DELETEZ( pSwpHints );
/*N*/
/*N*/ 	// Frames benachrichtigen;
/*N*/ 	SwInsTxt aInsHint( nDestStart, nLen );
/*N*/ 	pDest->SwCntntNode::Modify( 0, &aInsHint );
/*N*/ 	SwDelTxt aDelHint( nTxtStartIdx, nLen );
/*N*/ 	SwCntntNode::Modify( 0, &aDelHint );
/*N*/ }


/*N*/ SwTxtNode& SwTxtNode::Erase(const SwIndex &rIdx, xub_StrLen nCount,
/*N*/ 							const USHORT nMode )
/*N*/ {
/*N*/ 	ASSERT( rIdx <= aText.Len(), "Array ueberindiziert." );
/*N*/
/*N*/ 	const xub_StrLen nCnt = STRING_LEN == nCount
/*N*/ 					  ? aText.Len() - rIdx.GetIndex() : nCount;
/*N*/ 	aText.Erase( rIdx.GetIndex(), nCnt );
/*N*/
    /* GCAttr(); alle leeren weggwerfen ist zu brutal.
     * Es duerfen nur die wegggeworfen werden,
     * die im Bereich liegen und nicht am Ende des Bereiches liegen
     */
/*N*/
/*N*/ 	// Abfrage auf pSwpHints weil TextFelder und FlyFrames Text loeschen
/*N*/ 	// (Rekursion)!!
/*N*/ 	for( USHORT i = 0; pSwpHints && i < pSwpHints->Count(); ++i )
/*N*/ 	{
/*N*/ 		SwTxtAttr *pHt = pSwpHints->GetHt(i);
/*N*/
/*N*/ 		const xub_StrLen nHtStt = *pHt->GetStart();
/*N*/
/*N*/ 		if( nHtStt < rIdx.GetIndex() )
/*N*/ 			continue;
/*N*/
/*N*/ 		// TextFelder und FlyFrames loeschen Text (Rekursion)!!
/*N*/ 		const xub_StrLen nEndIdx = rIdx.GetIndex() + nCnt;
/*N*/ 		if( nHtStt > nEndIdx )
/*N*/ 			// die Hints sind nach Ende sortiert, also ist Start
/*N*/ 			// vom Hint groesser als EndIdx dann Abbrechen
/*N*/ 			break;
/*N*/
/*N*/ 		const xub_StrLen* pHtEndIdx = pHt->GetEnd();
/*N*/ 		const USHORT nWhich = pHt->Which();
/*N*/
/*N*/ 		if( !pHtEndIdx )
/*N*/ 		{
/*N*/ 			// TxtHints ohne EndIndex werden natuerlich auch geloescht:
/*N*/ 			if( RES_TXTATR_BEGIN <= nWhich && RES_TXTATR_END > nWhich &&
/*N*/ 				nHtStt >= rIdx.GetIndex() && nHtStt < nEndIdx )
/*N*/ 			{
/*N*/ 				pSwpHints->DeleteAtPos(i);
/*N*/ 				// Damit im Dtor der TxtAttr ohne End die CH_TXTATR nicht
/*N*/ 				// geloescht werden...
/*N*/ 				*(pHt->GetStart()) = USHRT_MAX;
/*N*/ 				DestroyAttr( pHt );
/*N*/ 				--i;
/*N*/ 			}
/*N*/ 			continue;
/*N*/ 		}
/*N*/
/*N*/ 		if( *pHtEndIdx >= nEndIdx && !(
/*N*/ 			!(INS_EMPTYEXPAND & nMode) && *pHtEndIdx == nEndIdx &&
/*N*/ 			(nWhich == RES_TXTATR_TOXMARK || nWhich == RES_TXTATR_REFMARK)) )
/*N*/ 			continue;
/*N*/
/*N*/ 		pSwpHints->DeleteAtPos(i);
/*N*/ 		DestroyAttr( pHt );
/*N*/ 		--i;
/*N*/ 	}
/*N*/
/*N*/ 	if ( pSwpHints && pSwpHints->CanBeDeleted() )
/*?*/ 		DELETEZ( pSwpHints );
/*N*/
/*N*/ 	Update( rIdx, nCnt, TRUE );
/*N*/
/*N*/ 	if( 1 == nCnt )
/*N*/ 	{
/*N*/ 		SwDelChr aHint( rIdx.GetIndex() );
/*N*/ 		SwModify::Modify( 0, &aHint );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwDelTxt aHint( rIdx.GetIndex(), nCnt );
/*N*/ 		SwModify::Modify( 0, &aHint );
/*N*/ 	}
/*N*/
/*N*/ 	CHECK_SWPHINTS(this);
/*N*/ 	return *this;
/*N*/ }

/***********************************************************************
#*	Class		:	SwTxtNode
#*	Methode 	:	GCAttr
#*
#*	Beschreibung
#*					text.doc
#*
#*	Datum		:	MS 28.11.90
#*	Update		:	VB 24.07.91
#***********************************************************************/


/*N*/ const SwNodeNum* SwTxtNode::UpdateNum( const SwNodeNum& rNum )
/*N*/ {
/*N*/ 	if( NO_NUMBERING == rNum.GetLevel() )		// kein Nummerierung mehr ?
/*N*/ 	{
/*N*/ 		if( !pNdNum )
/*N*/ 			return 0;
/*N*/ 		delete pNdNum, pNdNum = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( !pNdNum )
/*N*/ 			pNdNum = new SwNodeNum( rNum );
/*N*/ 		else if( !( *pNdNum == rNum ))
/*N*/ 			*pNdNum = rNum;
/*N*/ 	}
/*N*/ 	NumRuleChgd();
/*N*/ 	return pNdNum;
/*N*/ }

/*N*/ const SwNumRule* SwTxtNode::GetNumRule() const
/*N*/ {
/*N*/ 	const SwNumRule* pRet = 0;
/*N*/ 	const SfxPoolItem* pItem = GetNoCondAttr( RES_PARATR_NUMRULE, TRUE );
/*N*/ 	if( pItem && ((SwNumRuleItem*)pItem)->GetValue().Len() )
/*N*/ 		pRet = GetDoc()->FindNumRulePtr( ((SwNumRuleItem*)pItem)->GetValue() );
/*N*/ 	return pRet;
/*N*/ }

/*N*/ void SwTxtNode::NumRuleChgd()
/*N*/ {
/*N*/ #ifndef NUM_RELSPACE
/*N*/
/*N*/ 	// 6969: Aktualisierung der NumPortions auch bei leeren Zeilen!
/*N*/ 	SwInsTxt aHint( 0, 0 );
/*N*/ 	SwModify::Modify( 0, &aHint );
/*N*/
/*N*/ #else
/*N*/
/*N*/ 	if( IsInCache() )
/*N*/ 	{
/*N*/ 		SwFrm::GetCache().Delete( this );
/*N*/ 		SetInCache( FALSE );
/*N*/ 	}
/*N*/ 	SetInSwFntCache( FALSE );
/*N*/
/*N*/ 	SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)GetSwAttrSet().GetLRSpace();
/*N*/ 	SwModify::Modify( &rLR, &rLR );
/*N*/
/*N*/ #endif
/*N*/ }

/*N*/ const SwNodeNum* SwTxtNode::UpdateOutlineNum( const SwNodeNum& rNum )
/*N*/ {
/*N*/ 	if( NO_NUMBERING == rNum.GetLevel() )		// kein Nummerierung mehr ?
/*N*/ 	{
/*N*/ 		if( !pNdOutl )
/*N*/ 			return 0;
/*N*/ 		delete pNdOutl, pNdOutl = 0;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		if( !pNdOutl )
/*N*/ 			pNdOutl = new SwNodeNum( rNum );
/*N*/ 		else if( !( *pNdOutl == rNum ))
/*N*/ 			*pNdOutl = rNum;
/*N*/ 	}
/*N*/
/*N*/ 	// 6969: Aktualisierung der NumPortions auch bei leeren Zeilen!
/*N*/ 	NumRuleChgd();
/*N*/ 	return pNdOutl;
/*N*/ }

/*N*/ SwTxtNode* SwTxtNode::_MakeNewTxtNode( const SwNodeIndex& rPos, BOOL bNext,
/*N*/ 										BOOL bChgFollow )
/*N*/ {
/*N*/ 	/* hartes PageBreak/PageDesc/ColumnBreak aus AUTO-Set ignorieren */
/*N*/ 	SwAttrSet* pNewAttrSet = 0;
/*N*/ 	if( GetpSwAttrSet() )
/*N*/ 	{
/*N*/ 		pNewAttrSet = new SwAttrSet( *GetpSwAttrSet() );
/*N*/ 		SwAttrSet* pTmpSet = GetpSwAttrSet();
/*N*/
/*N*/ 		if( bNext )		// der naechste erbt keine Breaks!
/*N*/ 			pTmpSet = pNewAttrSet;
/*N*/
/*N*/ 		// PageBreaks/PageDesc/ColBreak rausschmeissen.
/*N*/ 		BOOL bRemoveFromCache = 0 != pTmpSet->ClearItem( RES_PAGEDESC );
/*N*/ 		if( SFX_ITEM_SET == pTmpSet->GetItemState( RES_BREAK, FALSE ) )
/*N*/ 		{
/*N*/ 			pTmpSet->ClearItem( RES_BREAK );
/*N*/ 			bRemoveFromCache = TRUE;
/*N*/ 		}
/*N*/ 		if( !bNext && bRemoveFromCache && IsInCache() )
/*N*/ 		{
/*N*/ 			SwFrm::GetCache().Delete( this );
/*N*/ 			SetInCache( FALSE );
/*N*/ 		}
/*N*/ 	}
/*N*/ 	SwNodes& rNds = GetNodes();
/*N*/
/*N*/ 	SwTxtFmtColl* pColl = GetTxtColl();
/*N*/
/*N*/ 	SwTxtNode *pNode = new SwTxtNode( rPos, pColl, pNewAttrSet );
/*N*/
/*N*/ 	if( pNewAttrSet )
/*N*/ 		delete pNewAttrSet;
/*N*/
/*N*/ 	const SwNumRule* pRule = GetNumRule();
/*N*/ 	if( pRule && rNds.IsDocNodes() )
/*N*/ 	{
/*N*/ 		// ist am Node eine Nummerierung gesetzt und wird dieser vor dem
/*N*/ 		// alten eingefuegt, so kopiere die Nummer
/*N*/ 		if( !bNext && pNdNum && NO_NUMBERING != pNdNum->GetLevel() )
/*N*/ 		{
/*N*/ 			if( pNode->pNdNum )
/*N*/ 				*pNode->pNdNum = *pNdNum;
/*N*/ 			else
/*N*/ 				pNode->pNdNum = new SwNodeNum( *pNdNum );
/*N*/
/*N*/ 			// SetValue immer auf default zurueck setzem
/*N*/ 			pNdNum->SetSetValue( USHRT_MAX );
/*N*/ 			if( pNdNum->IsStart() )
/*N*/ 			{
/*N*/ 				pNdNum->SetStart( FALSE );
/*N*/ 				pNode->pNdNum->SetStart( TRUE );
/*N*/ 			}
/*N*/
/*N*/ 			// Ein SplitNode erzeugt !!immer!! einen neuen Level, NO_NUM
/*N*/ 			// kann nur ueber eine entsprechende Methode erzeugt werden !!
/*N*/ 			if( NO_NUMLEVEL & pNdNum->GetLevel() )
/*N*/ 			{
/*N*/ 				pNdNum->SetLevel( pNdNum->GetLevel() & ~NO_NUMLEVEL );
/*N*/ #ifndef NUM_RELSPACE
/*N*/ 				SetNumLSpace( TRUE );
/*N*/ #endif
/*N*/ 			}
/*N*/ 		}
/*N*/ 		rNds.GetDoc()->UpdateNumRule( pRule->GetName(), pNode->GetIndex() );
/*N*/ 	}
/*N*/
/*N*/ 	// jetzt kann es sein, das durch die Nummerierung dem neuen Node eine
/*N*/ 	// Vorlage aus dem Pool zugewiesen wurde. Dann darf diese nicht
/*N*/ 	// nochmal uebergeplaettet werden !!
/*N*/ 	if( pColl != pNode->GetTxtColl() ||
/*N*/ 		( bChgFollow && pColl != GetTxtColl() ))
/*N*/ 		return pNode;		// mehr duerfte nicht gemacht werden oder ????
/*N*/
/*N*/ 	pNode->_ChgTxtCollUpdateNum( 0, pColl ); // fuer Nummerierung/Gliederung
/*N*/ 	if( bNext || !bChgFollow )
/*N*/ 		return pNode;
/*N*/
/*N*/ 	SwTxtFmtColl *pNextColl = &pColl->GetNextTxtFmtColl();
/*N*/ 	ChgFmtColl( pNextColl );
/*N*/
/*N*/ 	return pNode;
/*N*/ }

/*N*/ SwCntntNode* SwTxtNode::AppendNode( const SwPosition & rPos )
/*N*/ {
/*N*/ 	// Position hinter dem eingefuegt wird
/*N*/ 	SwNodeIndex aIdx( rPos.nNode, 1 );
/*N*/ 	SwTxtNode* pNew = _MakeNewTxtNode( aIdx, TRUE );
/*N*/ 	if( GetDepends() )
/*N*/ 		MakeFrms( *pNew );
/*N*/ 	return pNew;
/*N*/ }

/*************************************************************************
 *						SwTxtNode::GetTxtAttr
 *
 * Diese Methode liefert nur Textattribute auf der Position nIdx
 * zurueck, die kein EndIdx besitzen und denselben Which besitzen.
 * Ueblicherweise steht an dieser Position ein CH_TXTATR.
 * Bei RES_TXTATR_END entfaellt die Pruefung auf den Which-Wert.
 *************************************************************************/

/*N*/ SwTxtAttr *SwTxtNode::GetTxtAttr( const xub_StrLen nIdx,
/*N*/ 								  const USHORT nWhichHt ) const
/*N*/ {
/*N*/ 	if( pSwpHints )
/*N*/ 	{
/*N*/ 		for( USHORT i = 0; i < pSwpHints->Count(); ++i )
/*N*/ 		{
/*N*/ 			SwTxtAttr *pPos = pSwpHints->GetHt(i);
/*N*/ 			const xub_StrLen nStart = *pPos->GetStart();
/*N*/ 			if( nIdx < nStart )
/*N*/ 				return 0;
/*N*/ 			if( nIdx == nStart && !pPos->GetEnd() )
/*N*/ 			{
/*N*/ 				if( RES_TXTATR_END == nWhichHt || nWhichHt == pPos->Which() )
/*N*/ 					return pPos;
/*N*/ 				else
/*N*/ 					return 0;
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*************************************************************************
 *						SwTxtNode::GetExpandTxt
 *************************************************************************/
// Felder werden expandiert:

/*N*/ XubString SwTxtNode::GetNumString() const
/*N*/ {
/*N*/ 	const SwNodeNum* pNum;
/*N*/ 	const SwNumRule* pRule;
/*N*/ 	if( (( 0 != ( pNum = GetNum() ) &&
/*N*/ 			0 != ( pRule = GetNumRule() )) ||
/*N*/ 			( 0 != ( pNum = GetOutlineNum() ) &&
/*N*/ 			0 != ( pRule = GetDoc()->GetOutlineNumRule() ) ) ) &&
/*N*/ 		pNum->GetLevel() < MAXLEVEL &&
/*N*/ 		pRule->Get( pNum->GetLevel() ).IsTxtFmt() )
/*?*/ 		return pRule->MakeNumString( *pNum );
/*N*/ 	return aEmptyStr;
/*N*/ }

/*N*/ long SwTxtNode::GetLeftMarginWithNum( BOOL bTxtLeft ) const
/*N*/ {
/*N*/ 	long nOffset;
/*N*/ 	const SwNodeNum* pNum;
/*N*/ 	const SwNumRule* pRule;
/*N*/ 	if( (( 0 != ( pNum = GetNum() ) &&
/*N*/ 			0 != ( pRule = GetNumRule() )) ||
/*N*/ 			( 0 != ( pNum = GetOutlineNum() ) &&
/*N*/ 			0 != ( pRule = GetDoc()->GetOutlineNumRule() ) ) ) &&
/*N*/ 			pNum->GetLevel() < NO_NUM )
/*N*/ 	{
/*N*/ 		const SwNumFmt& rFmt = pRule->Get( GetRealLevel( pNum->GetLevel() ) );
/*N*/ 		nOffset = rFmt.GetAbsLSpace();
/*N*/
/*N*/ 		if( !bTxtLeft )
/*N*/ 		{
/*N*/ 			if( 0 > rFmt.GetFirstLineOffset() &&
/*N*/ 				nOffset > -rFmt.GetFirstLineOffset() )
/*N*/ 				nOffset += rFmt.GetFirstLineOffset();
/*N*/ 			else
/*N*/ 				nOffset = 0;
/*N*/ 		}
/*N*/
/*N*/ 		if( pRule->IsAbsSpaces() )
/*N*/ 			nOffset -= GetSwAttrSet().GetLRSpace().GetLeft();
/*N*/ 	}
/*N*/ 	else
/*N*/ 		nOffset = 0;
/*N*/ 	return nOffset;
/*N*/ }

/*N*/ BOOL SwTxtNode::GetFirstLineOfsWithNum( short& rFLOffset ) const
/*N*/ {
/*N*/ 	const SwNodeNum* pNum;
/*N*/ 	const SwNumRule* pRule;
/*N*/ 	if( (( 0 != ( pNum = GetNum() ) &&
/*N*/ 			0 != ( pRule = GetNumRule() )) ||
/*N*/ 			( 0 != ( pNum = GetOutlineNum() ) &&
/*N*/ 			0 != ( pRule = GetDoc()->GetOutlineNumRule() ) ) ) &&
/*N*/ 			pNum->GetLevel() < NO_NUM )
/*N*/ 	{
/*N*/ 		if( NO_NUMLEVEL & pNum->GetLevel() )
/*N*/ 			rFLOffset = 0;
/*N*/ 		else
/*N*/ 			rFLOffset = pRule->Get( pNum->GetLevel() ).GetFirstLineOffset();
/*N*/ 		return TRUE;
/*N*/ 	}
/*N*/ 	rFLOffset = GetSwAttrSet().GetLRSpace().GetTxtFirstLineOfst();
/*N*/ 	return FALSE;
/*N*/ }

/*N*/ void SwTxtNode::Replace0xFF( XubString& rTxt, xub_StrLen& rTxtStt,
/*N*/ 							xub_StrLen nEndPos, BOOL bExpandFlds ) const
/*N*/ {
/*N*/ 	if( GetpSwpHints() )
/*N*/ 	{
/*N*/ 		sal_Unicode cSrchChr = CH_TXTATR_BREAKWORD;
/*N*/ 		for( int nSrchIter = 0; 2 > nSrchIter; ++nSrchIter,
/*N*/ 								cSrchChr = CH_TXTATR_INWORD )
/*N*/ 		{
/*N*/ 			xub_StrLen nPos = rTxt.Search( cSrchChr );
/*N*/ 			while( STRING_NOTFOUND != nPos && nPos < nEndPos )
/*N*/ 			{
/*N*/ 				const SwTxtAttr* pAttr = GetTxtAttr( rTxtStt + nPos );
/*N*/ 				if( pAttr )
/*N*/ 				{
/*N*/ 					switch( pAttr->Which() )
/*N*/ 					{
/*N*/ 					case RES_TXTATR_FIELD:
/*N*/ 						rTxt.Erase( nPos, 1 );
/*N*/ 						if( bExpandFlds )
/*N*/ 						{
/*N*/ 							const XubString aExpand( ((SwTxtFld*)pAttr)->GetFld().
/*N*/ 													GetFld()->Expand() );
/*N*/ 							rTxt.Insert( aExpand, nPos );
/*N*/ 							nPos += aExpand.Len();
/*N*/ 							nEndPos += aExpand.Len();
/*N*/ 							rTxtStt -= aExpand.Len();
/*N*/ 						}
/*N*/ 						++rTxtStt;
/*N*/ 						break;
/*N*/ 					case RES_TXTATR_HARDBLANK:
                            {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/ 						rTxt.SetChar( nPos, ((SwTxtHardBlank*)pAttr)->GetChar() );
/*?*/ 						break;
/*N*/ 					case RES_TXTATR_FTN:
/*N*/ 						rTxt.Erase( nPos, 1 );
/*N*/ 						if( bExpandFlds )
/*N*/ 						{
/*N*/ 							const SwFmtFtn& rFtn = pAttr->GetFtn();
/*N*/ 							XubString sExpand;
/*N*/ 							if( rFtn.GetNumStr().Len() )
/*N*/ 								sExpand = rFtn.GetNumStr();
/*N*/ 							else if( rFtn.IsEndNote() )
/*?*/ 								sExpand = GetDoc()->GetEndNoteInfo().aFmt.
/*?*/ 												GetNumStr( rFtn.GetNumber() );
/*N*/ 							else
/*N*/ 								sExpand = GetDoc()->GetFtnInfo().aFmt.
/*N*/ 												GetNumStr( rFtn.GetNumber() );
/*N*/ 							rTxt.Insert( sExpand, nPos );
/*N*/ 							nPos += sExpand.Len();
/*N*/ 							nEndPos += sExpand.Len();
/*N*/ 							rTxtStt -= sExpand.Len();
/*N*/ 						}
/*N*/ 						++rTxtStt;
/*N*/ 						break;
/*N*/ 					default:
/*N*/ 						rTxt.Erase( nPos, 1 );
/*N*/ 						++rTxtStt;
/*N*/ 					}
/*N*/ 				}
/*N*/ 				else
/*N*/ 					++nPos, ++nEndPos;
/*N*/ 				nPos = rTxt.Search( cSrchChr, nPos );
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ XubString SwTxtNode::GetExpandTxt( const xub_StrLen nIdx, const xub_StrLen nLen,
/*N*/ 								const BOOL bWithNum  ) const
/*N*/ {
/*N*/ 	XubString aTxt( GetTxt().Copy( nIdx, nLen ) );
/*N*/ 	xub_StrLen nTxtStt = nIdx;
/*N*/ 	Replace0xFF( aTxt, nTxtStt, aTxt.Len(), TRUE );
/*N*/
/*N*/ 	if( bWithNum )
/*?*/ 		aTxt.Insert( GetNumString(), 0 );
/*N*/
/*N*/ 	return aTxt;
/*N*/ }




/*************************************************************************
 *						SwTxtNode::GetExpandTxt
 *************************************************************************/
// Felder werden expandiert:

/*N*/ void SwTxtNode::Replace( const SwIndex& rStart, xub_Unicode cCh )
/*N*/ {
/*N*/
/*N*/ 	ASSERT( rStart.GetIndex() < aText.Len(), "ausserhalb des Strings" );
/*N*/ 	SwTxtAttr* pHt;
/*N*/ 	if( ( CH_TXTATR_BREAKWORD == aText.GetChar( rStart.GetIndex() ) ||
/*N*/ 		  CH_TXTATR_INWORD == aText.GetChar( rStart.GetIndex() )) &&
/*N*/ 		0 != ( pHt = GetTxtAttr( rStart.GetIndex() ) ))
/*N*/ 	{
/*?*/ 		Delete( pHt );
/*?*/ 		aText.Insert( cCh, rStart.GetIndex() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		aText.SetChar( rStart.GetIndex(), cCh );
/*N*/
/*N*/ 	SwDelTxt aDelHint( rStart.GetIndex(), 1 );
/*N*/ 	SwModify::Modify( 0, &aDelHint );
/*N*/
/*N*/ 	SwInsTxt aHint( rStart.GetIndex(), 1 );
/*N*/ 	SwModify::Modify( 0, &aHint );
/*N*/ }



/*N*/ void SwTxtNode::Modify( SfxPoolItem* pOldValue, SfxPoolItem* pNewValue )
/*N*/ {
/*N*/ 	// Bug 24616/24617:
/*N*/ 	// 		Modify ueberladen, damit beim Loeschen von Vorlagen diese
/*N*/ 	// 		wieder richtig verwaltet werden (Outline-Numerierung!!)
/*N*/ 	// 	Bug25481:
/*N*/ 	//		bei Nodes im Undo nie _ChgTxtCollUpdateNum rufen.
/*N*/ 	if( pOldValue && pNewValue && RES_FMT_CHG == pOldValue->Which() &&
/*N*/ 		pRegisteredIn == ((SwFmtChg*)pNewValue)->pChangedFmt &&
/*N*/ 		GetNodes().IsDocNodes() )
/*N*/ 		_ChgTxtCollUpdateNum(
/*N*/ 						(SwTxtFmtColl*)((SwFmtChg*)pOldValue)->pChangedFmt,
/*N*/ 						(SwTxtFmtColl*)((SwFmtChg*)pNewValue)->pChangedFmt );
/*N*/
/*N*/ 	SwCntntNode::Modify( pOldValue, pNewValue );
/*N*/ }



}
