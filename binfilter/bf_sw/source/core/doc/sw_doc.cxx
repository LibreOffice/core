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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL
#include <com/sun/star/i18n/WordType.hdl>
#endif
#ifndef _IPOBJ_HXX //autogen
#include <bf_so3/ipobj.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <bf_sfx2/docinf.hxx>
#endif
#ifndef SMDLL0_HXX //autogen
#include <bf_starmath/smdll0.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _LINKENUM_HXX
#include <linkenum.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAGEFRM_HXX
#include <pagefrm.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> //DTor
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _NDOLE_HXX
#include <ndole.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>			// fuer die Bookmarks
#endif
#ifndef _DOCTXM_HXX
#include <doctxm.hxx>			// fuer die Verzeichnisse
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>			// PoolVorlagen-Id's
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>			// fuer Server-Funktionalitaet
#endif
#ifndef _WRONG_HXX
#include <wrong.hxx>			// fuer OnlineSpell-Invalidierung
#endif
#ifndef	_DRAWFONT_HXX
#include <drawfont.hxx>
#endif
#ifndef _DOCSTAT_HXX
#include <docstat.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _FLDUPDE_HXX
#include <fldupde.hxx>
#endif

#ifndef _STATSTR_HRC
#include <statstr.hrc>      	// StatLine-String
#endif
namespace binfilter {

// Seiten-Deskriptoren
/*N*/ SV_IMPL_PTRARR(SwPageDescs,SwPageDescPtr);
// Autoren
// IMPL_PTRREGARR_NOTL(Authors,Author) USED?
// Verzeichnisse
/*N*/ SV_IMPL_PTRARR( SwTOXTypes, SwTOXTypePtr )
// FeldTypen
/*N*/ SV_IMPL_PTRARR( SwFldTypes, SwFldTypePtr)


/*
 * Dokumenteditieren (Doc-SS) zum Fuellen des Dokuments
 * durch den RTF Parser und fuer die EditShell.
 */
/*N*/ void SwDoc::ChgDBData(const SwDBData& rNewData)
/*N*/ {
/*N*/ 	if( rNewData != aDBData )
/*N*/ 	{
/*N*/ 		aDBData = rNewData;
/*N*/ 		SetModified();
/*N*/ 	}
/*N*/ 	GetSysFldType(RES_DBNAMEFLD)->UpdateFlds();
/*N*/ }

/*N*/ BOOL SwDoc::SplitNode( const SwPosition &rPos, BOOL bChkTableStart )
/*N*/ {
/*N*/ 	SwCntntNode *pNode = rPos.nNode.GetNode().GetCntntNode();
/*N*/ 	if(0 == pNode)
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	{
/*N*/ 		// Bug 26675:	DataChanged vorm loeschen verschicken, dann bekommt
/*N*/ 		//			man noch mit, welche Objecte sich im Bereich befinden.
/*N*/ 		//			Danach koennen sie vor/hinter der Position befinden.
/*N*/ 		SwDataChanged aTmp( this, rPos, 0 );
/*N*/ 	}
/*N*/
/*N*/ 	//JP 28.01.97: Sonderfall fuer SplitNode am Tabellenanfang:
/*N*/ 	//				steht die am Doc/Fly/Footer/..-Anfang oder direkt
/*N*/ 	//				hinter einer Tabelle, dann fuege davor
/*N*/ 	//				einen Absatz ein
/*N*/ 	if( bChkTableStart && !rPos.nContent.GetIndex() && pNode->IsTxtNode() )
/*N*/ 	{
/*?*/ 		ULONG nPrevPos = rPos.nNode.GetIndex() - 1;
/*?*/ 		const SwTableNode* pTblNd;
/*?*/ 		const SwNode* pNd = GetNodes()[ nPrevPos ];
/*?*/ 		if( pNd->IsStartNode() &&
/*?*/ 			SwTableBoxStartNode == ((SwStartNode*)pNd)->GetStartNodeType() &&
/*?*/ 			0 != ( pTblNd = GetNodes()[ --nPrevPos ]->GetTableNode() ) &&
/*?*/ 			((( pNd = GetNodes()[ --nPrevPos ])->IsStartNode() &&
/*?*/ 			   SwTableBoxStartNode != ((SwStartNode*)pNd)->GetStartNodeType() )
/*?*/ 			   || ( pNd->IsEndNode() && pNd->FindStartNode()->IsTableNode() )
/*?*/ 			   || pNd->IsCntntNode() ))
/*?*/ 		{
/*?*/ 			if( pNd->IsCntntNode() )
/*?*/ 			{
/*?*/ 				//JP 30.04.99 Bug 65660:
/*?*/ 				// ausserhalb des normalen BodyBereiches gibt es keine
/*?*/ 				// Seitenumbrueche, also ist das hier kein gueltige
/*?*/ 				// Bedingung fuers einfuegen eines Absatzes
/*?*/ 				if( nPrevPos < GetNodes().GetEndOfExtras().GetIndex() )
/*?*/ 					pNd = 0;
/*?*/ 				else
/*?*/ 				{
/*?*/ 					// Dann nur, wenn die Tabelle Umbrueche traegt!
/*?*/ 					const SwFrmFmt* pFrmFmt = pTblNd->GetTable().GetFrmFmt();
/*?*/ 					if( SFX_ITEM_SET != pFrmFmt->GetItemState(RES_PAGEDESC, FALSE) &&
/*?*/ 						SFX_ITEM_SET != pFrmFmt->GetItemState( RES_BREAK, FALSE ) )
/*?*/ 						pNd = 0;
/*?*/ 				}
/*?*/ 			}
/*?*/
/*?*/ 			if( pNd )
/*?*/ 			{
/*?*/ 				SwTxtNode* pTxtNd = GetNodes().MakeTxtNode(
/*?*/ 										SwNodeIndex( *pTblNd ),
/*?*/ 										GetTxtCollFromPool( RES_POOLCOLL_TEXT ));
/*?*/ 				if( pTxtNd )
/*?*/ 				{
/*?*/ 					((SwPosition&)rPos).nNode = pTblNd->GetIndex()-1;
/*?*/ 					((SwPosition&)rPos).nContent.Assign( pTxtNd, 0 );
/*?*/
/*?*/ 					// nur im BodyBereich den SeitenUmbruch/-Vorlage umhaengem
/*?*/ 					if( nPrevPos > GetNodes().GetEndOfExtras().GetIndex() )
/*?*/ 					{
/*?*/ 						SwFrmFmt* pFrmFmt = pTblNd->GetTable().GetFrmFmt();
/*?*/ 						const SfxPoolItem *pItem;
/*?*/ 						if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_PAGEDESC,
/*?*/ 							FALSE, &pItem ) )
/*?*/ 						{
/*?*/ 							pTxtNd->SwCntntNode::SetAttr( *pItem );
/*?*/ 							pFrmFmt->ResetAttr( RES_PAGEDESC );
/*?*/ 						}
/*?*/ 						if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_BREAK,
/*?*/ 							FALSE, &pItem ) )
/*?*/ 						{
/*?*/ 							pTxtNd->SwCntntNode::SetAttr( *pItem );
/*?*/ 							pFrmFmt->ResetAttr( RES_BREAK );
/*?*/ 						}
/*?*/ 					}
/*?*/
/*?*/ 					SetModified();
/*?*/ 					return TRUE;
/*?*/ 				}
/*?*/ 			}
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SvULongs aBkmkArr( 15, 15 );
/*N*/ 	_SaveCntntIdx( this, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(),
/*N*/ 					aBkmkArr, SAVEFLY_SPLIT );
/*N*/ 	if( 0 != ( pNode = pNode->SplitNode( rPos ) ))
/*N*/ 	{
/*N*/ 		// verschiebe noch alle Bookmarks/TOXMarks/FlyAtCnt
/*N*/ 		if( aBkmkArr.Count() )
/*?*/   {   _RestoreCntntIdx( this, aBkmkArr, rPos.nNode.GetIndex()-1, 0, TRUE );}
/*N*/
/*N*/ 		if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aPam( rPos );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL SwDoc::AppendTxtNode( SwPosition& rPos )
/*N*/ {
    /*
     * Neuen Node vor EndOfContent erzeugen.
     */
/*N*/ 	SwTxtNode *pCurNode = GetNodes()[ rPos.nNode ]->GetTxtNode();
/*N*/ 	if( !pCurNode )
/*N*/ 	{
/*?*/ 		// dann kann ja einer angelegt werden!
/*?*/ 		SwNodeIndex aIdx( rPos.nNode, 1 );
/*?*/ 		pCurNode = GetNodes().MakeTxtNode( aIdx,
/*?*/ 						GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pCurNode = (SwTxtNode*)pCurNode->AppendNode( rPos );
/*N*/
/*N*/ 	rPos.nNode++;
/*N*/ 	rPos.nContent.Assign( pCurNode, 0 );
/*N*/
/*N*/ 	if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aPam( rPos );
/*N*/ 	}
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL SwDoc::Insert( const SwPaM &rRg, const String &rStr, BOOL bHintExpand )
/*N*/ {
/*N*/ 	const SwPosition* pPos = rRg.GetPoint();
/*N*/
/*N*/ 	SwTxtNode *pNode = pPos->nNode.GetNode().GetTxtNode();
/*N*/ 	if(!pNode)
/*N*/ 		return FALSE;

/*N*/ 	const USHORT nInsMode = bHintExpand ? INS_EMPTYEXPAND
/*N*/ 									 : INS_NOHINTEXPAND;
/*N*/ 	SwDataChanged aTmp( rRg, 0 );
/*N*/
/*N*/ 		pNode->Insert( rStr, pPos->nContent, nInsMode );
/*N*/
/*N*/ 	if( IsRedlineOn() || (!IsIgnoreRedline() && pRedlineTbl->Count() ))
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aPam( pPos->nNode, aTmp.GetCntnt(),
/*N*/ 	}
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ SwFlyFrmFmt* SwDoc::_InsNoTxtNode( const SwPosition& rPos, SwNoTxtNode* pNode,
/*N*/ 									const SfxItemSet* pFlyAttrSet,
/*N*/ 									const SfxItemSet* pGrfAttrSet,
/*N*/ 									SwFrmFmt* pFrmFmt)
/*N*/ {
/*N*/ 	SwFlyFrmFmt *pFmt = 0;
/*N*/ 	if( pNode )
/*N*/ 	{
/*N*/ 		pFmt = _MakeFlySection( rPos, *pNode, FLY_AT_CNTNT,
/*N*/ 								pFlyAttrSet, pFrmFmt );
/*N*/ 		if( pGrfAttrSet )
/*N*/ 			pNode->SetAttr( *pGrfAttrSet );
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ SwFlyFrmFmt* SwDoc::Insert( const SwPaM &rRg,
/*N*/ 							const String& rGrfName,
/*N*/ 							const String& rFltName,
/*N*/ 							const Graphic* pGraphic,
/*N*/ 							const SfxItemSet* pFlyAttrSet,
/*N*/ 							const SfxItemSet* pGrfAttrSet,
/*N*/ 							SwFrmFmt* pFrmFmt )
/*N*/ {
/*N*/ 	if( !pFrmFmt )
/*?*/ 		pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC );
/*N*/ 	return _InsNoTxtNode( *rRg.GetPoint(), GetNodes().MakeGrfNode(
/*N*/ 							SwNodeIndex( GetNodes().GetEndOfAutotext() ),
/*N*/ 							rGrfName, rFltName, pGraphic,
/*N*/ 							pDfltGrfFmtColl ),
/*N*/ 							pFlyAttrSet, pGrfAttrSet, pFrmFmt );
/*N*/ }
/*N*/ SwFlyFrmFmt* SwDoc::Insert( const SwPaM &rRg, const BfGraphicObject& rGrfObj,
/*N*/ 							const SfxItemSet* pFlyAttrSet,
/*N*/ 							const SfxItemSet* pGrfAttrSet,
/*N*/ 							SwFrmFmt* pFrmFmt )
/*N*/ {
/*N*/ 	if( !pFrmFmt )
/*?*/ 		pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC );
/*N*/ 	return _InsNoTxtNode( *rRg.GetPoint(), GetNodes().MakeGrfNode(
/*N*/ 							SwNodeIndex( GetNodes().GetEndOfAutotext() ),
/*N*/ 							rGrfObj, pDfltGrfFmtColl ),
/*N*/ 							pFlyAttrSet, pGrfAttrSet, pFrmFmt );
/*N*/ }

/*N*/ SwFlyFrmFmt* SwDoc::Insert(const SwPaM &rRg, SvInPlaceObject *pObj,
/*N*/ 						const SfxItemSet* pFlyAttrSet,
/*N*/ 						const SfxItemSet* pGrfAttrSet,
/*N*/ 						SwFrmFmt* pFrmFmt )
/*N*/ {
/*N*/ 	if( !pFrmFmt )
/*N*/ 	{
/*N*/ 		USHORT nId = RES_POOLFRM_OLE;
/*N*/
/*N*/ 		FASTBOOL bMath = SmModuleDummy::HasID( *pObj->GetSvFactory() );
/*N*/         if ( bMath )
/*?*/ 			nId = RES_POOLFRM_FORMEL;
/*N*/
/*N*/ 		pFrmFmt = GetFrmFmtFromPool( nId );
/*N*/ 	}
/*N*/ 	return _InsNoTxtNode( *rRg.GetPoint(), GetNodes().MakeOLENode(
/*N*/ 							SwNodeIndex( GetNodes().GetEndOfAutotext() ),
/*N*/ 							pObj,
/*N*/ 							pDfltGrfFmtColl ),
/*N*/ 							pFlyAttrSet, pGrfAttrSet,
/*N*/ 							pFrmFmt );
/*N*/ }

/*N*/ SwFlyFrmFmt* SwDoc::InsertOLE(const SwPaM &rRg, String& rObjName,
/*N*/ 						const SfxItemSet* pFlyAttrSet,
/*N*/ 						const SfxItemSet* pGrfAttrSet,
/*N*/ 						SwFrmFmt* pFrmFmt )
/*N*/ {
/*N*/ 	if( !pFrmFmt )
/*N*/ 		pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_OLE );
/*N*/
/*N*/ 	return _InsNoTxtNode( *rRg.GetPoint(), GetNodes().MakeOLENode(
/*N*/ 							SwNodeIndex( GetNodes().GetEndOfAutotext() ),
/*N*/ 							rObjName,
/*N*/ 							pDfltGrfFmtColl ),
/*N*/ 							pFlyAttrSet, pGrfAttrSet,
/*N*/ 							pFrmFmt );
/*N*/ }




/*************************************************************************
|*				  SwDoc::GetFldType()
|*	  Beschreibung: liefert den am Doc eingerichteten Feldtypen zurueck
*************************************************************************/

/*N*/ SwFieldType *SwDoc::GetSysFldType( const USHORT eWhich ) const
/*N*/ {
/*N*/ 	for( register int i = 0; i < INIT_FLDTYPES; i++ )
/*N*/ 		if( eWhich == (*pFldTypes)[i]->Which() )
/*N*/ 			return (*pFldTypes)[i];
/*N*/ 	return 0;
/*N*/ }

//----- Macro ---------------------------------------------------------



void SwDoc::SetGlobalMacro( USHORT nEvent, const SvxMacro& rMacro )
{
    SvxMacro *pMacro;
    SetModified();
    if ( 0 != (pMacro=pMacroTable->Get(nEvent)) )
    {
        delete pMacro;
        pMacroTable->Replace(nEvent, new SvxMacro(rMacro));
        return;
    }
    pMacroTable->Insert(nEvent, new SvxMacro(rMacro));
}


/*************************************************************************
 *			   void SetDocStat( const SwDocStat& rStat );
 *************************************************************************/

/*N*/ void SwDoc::SetDocStat( const SwDocStat& rStat )
/*N*/ {
/*N*/ 	*pDocStat = rStat;
/*N*/ }


/*M*/ sal_uInt16 SwDoc::GetPageCount() const
/*M*/ {
/*M*/     return GetRootFrm() ? GetRootFrm()->GetPageNum() : 0;
/*M*/ }

/*M*/ const Size SwDoc::GetPageSize( sal_uInt16 nPageNum ) const
/*M*/ {
/*M*/     Size aSize;
/*M*/     if( GetRootFrm() && nPageNum )
/*M*/     {
/*M*/         const SwPageFrm* pPage = static_cast<const SwPageFrm*>
/*M*/                                  (GetRootFrm()->Lower());
/*M*/         while( --nPageNum && pPage->GetNext() )
/*M*/             pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );
/*M*/         if( pPage->IsEmptyPage() && pPage->GetNext() )
/*M*/             pPage = static_cast<const SwPageFrm*>( pPage->GetNext() );
/*M*/         aSize = pPage->Frm().SSize();
/*M*/     }
/*M*/     return aSize;
/*M*/ }


/*************************************************************************
 *			  void UpdateDocStat( const SwDocStat& rStat );
 *************************************************************************/

/*M*/ void SwDoc::UpdateDocStat( SwDocStat& rStat )
/*M*/ {
/*M*/ 	if( rStat.bModified )
/*M*/ 	{
/*M*/ 		rStat.Reset();
/*M*/ 		rStat.nPara = 0;        // Default ist auf 1 !!
/*M*/ 		SwNode* pNd;
/*M*/
/*M*/ 		for( ULONG n = GetNodes().Count(); n; )
/*M*/ 			switch( ( pNd = GetNodes()[ --n ])->GetNodeType() )
/*M*/ 			{
/*M*/             case ND_TEXTNODE:
/*M*/                 {
/*M*/                     const String& rStr = ((SwTxtNode*)pNd)->GetTxt();
/*M*/
/*M*/                     if( rStr.Len() && pBreakIt->xBreak.is() )
/*M*/                     {
/*M*/                         SwScanner aScanner( *((SwTxtNode*)pNd), NULL,
/*M*/                                             ::com::sun::star::i18n::WordType::WORD_COUNT,
/*M*/                                             0, rStr.Len(), sal_False, sal_False );
/*M*/
/*M*/                         while ( aScanner.NextWord() )
/*M*/                         {
/*M*/                             if ( aScanner.GetLen() > 1 ||
/*M*/                                  CH_TXTATR_BREAKWORD != rStr.GetChar( aScanner.GetBegin() ) )
/*M*/                                 ++rStat.nWord;
/*M*/                         }
/*M*/                     }
/*M*/                     rStat.nChar += rStr.Len();
/*M*/                     ++rStat.nPara;
/*M*/                 }
/*M*/                 break;
/*M*/ 			case ND_TABLENODE:		++rStat.nTbl;	break;
/*M*/ 			case ND_GRFNODE:		++rStat.nGrf;	break;
/*M*/ 			case ND_OLENODE:		++rStat.nOLE;	break;
/*M*/ 			case ND_SECTIONNODE:	break;
/*M*/ 			}
/*M*/
/*M*/ 		rStat.nPage 	= GetRootFrm() ? GetRootFrm()->GetPageNum() : 0;
/*M*/ 		rStat.bModified = FALSE;
/*M*/ 		SetDocStat( rStat );
/*M*/ 		// event. Stat. Felder Updaten
/*M*/ 		SwFieldType *pType = GetSysFldType(RES_DOCSTATFLD);
/*M*/ 		pType->UpdateFlds();
/*M*/ 	}
/*M*/ }


// Dokument - Info

/*N*/ void SwDoc::DocInfoChgd( const SfxDocumentInfo& rInfo )
/*N*/ {
/*N*/ 	delete pSwgInfo;
/*N*/ 	pSwgInfo = new SfxDocumentInfo(rInfo);
/*N*/
/*N*/ 	GetSysFldType( RES_DOCINFOFLD )->UpdateFlds();
/*N*/ 	GetSysFldType( RES_TEMPLNAMEFLD )->UpdateFlds();
/*N*/ 	SetModified();
/*N*/ }

    // returne zum Namen die im Doc gesetzte Referenz
/*N*/ const SwFmtRefMark* SwDoc::GetRefMark( const String& rName ) const
/*N*/ {
/*N*/ 	const SfxPoolItem* pItem;
/*N*/ 	USHORT nMaxItems = GetAttrPool().GetItemCount( RES_TXTATR_REFMARK );
/*N*/ 	for( USHORT n = 0; n < nMaxItems; ++n )
/*N*/ 	{
/*N*/ 		if( 0 == (pItem = GetAttrPool().GetItem( RES_TXTATR_REFMARK, n ) ))
/*N*/ 			continue;
/*N*/
/*N*/ 		const SwFmtRefMark* pFmtRef = (SwFmtRefMark*)pItem;
/*N*/ 		const SwTxtRefMark* pTxtRef = pFmtRef->GetTxtRefMark();
/*N*/ 		if( pTxtRef && &pTxtRef->GetTxtNode().GetNodes() == &GetNodes() &&
/*N*/ 			rName.Equals( pFmtRef->GetRefName() ) )
/*N*/ 			return pFmtRef;
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

    // returne die RefMark per Index - fuer Uno

    // returne die Namen aller im Doc gesetzten Referenzen
    //JP 24.06.96: Ist der ArrayPointer 0 dann returne nur, ob im Doc. eine
    //				RefMark gesetzt ist
    // OS 25.06.96: ab jetzt wird immer die Anzahl der Referenzen returnt
/*N*/ USHORT SwDoc::GetRefMarks( SvStringsDtor* pNames ) const
/*N*/ {
/*N*/ DBG_BF_ASSERT(0, "STRIP"); return 0;//STRIP001 	const SfxPoolItem* pItem;
/*N*/ }

/*N*/ void SwDoc::SetModified()
/*N*/ {
/*N*/ 	// dem Link wird der Status returnt, wie die Flags waren und werden
/*N*/ 	// 	Bit 0:	-> alter Zustand
/*N*/ 	//	Bit 1: 	-> neuer Zustand
/*N*/ 	long nCall = bModified ? 3 : 2;
/*N*/ 	bModified = TRUE;
/*N*/ 	pDocStat->bModified = TRUE;
/*N*/ 	if( aOle2Link.IsSet() )
/*N*/ 	{
/*N*/ 		bInCallModified = TRUE;
/*N*/ 		aOle2Link.Call( (void*)nCall );
/*N*/ 		bInCallModified = FALSE;
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::ResetModified()
/*N*/ {
/*N*/ 	// dem Link wird der Status returnt, wie die Flags waren und werden
/*N*/ 	// 	Bit 0:	-> alter Zustand
/*N*/ 	//	Bit 1: 	-> neuer Zustand
/*N*/ 	long nCall = bModified ? 1 : 0;
/*N*/ 	bModified = FALSE;
/*N*/ 	if( nCall && aOle2Link.IsSet() )
/*N*/ 	{
/*N*/ 		bInCallModified = TRUE;
/*N*/ 		aOle2Link.Call( (void*)nCall );
/*N*/ 		bInCallModified = FALSE;
/*N*/ 	}
/*N*/ }


/*N*/ void SwDoc::ReRead( SwPaM& rPam, const String& rGrfName,
/*N*/ 					const String& rFltName, const Graphic* pGraphic,
/*N*/ 					const BfGraphicObject* pGrafObj )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwGrfNode *pGrfNd;
/*N*/ }

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

/*N*/ USHORT SwDoc::GetLinkUpdMode() const
/*N*/ {
/*N*/ 	USHORT nRet = nLinkUpdMode;
/*N*/ 	if( GLOBALSETTING == nRet )
/*N*/ 		nRet = SW_MOD()->GetLinkUpdMode(IsHTMLMode());
/*N*/ 	return nRet;
/*N*/ }

/*N*/ USHORT SwDoc::GetFldUpdateFlags() const
/*N*/ {
/*N*/ 	USHORT nRet = nFldUpdMode;
/*N*/ 	if( AUTOUPD_GLOBALSETTING == nRet )
/*N*/ 		nRet = SW_MOD()->GetFldUpdateFlags(IsHTMLMode());
/*N*/ 	return nRet;
/*N*/ }




}
