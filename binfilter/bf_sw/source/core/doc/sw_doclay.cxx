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

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#include <hintids.hxx>

#include <bf_sfx2/progress.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/fmglob.hxx>
#include <bf_svx/svdouno.hxx>
#include <bf_svx/fmpage.hxx>
#include <bf_svx/frmdiritem.hxx>
#include <rtl/logfile.hxx>

#include <frmatr.hxx>

#include <horiornt.hxx>

#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <fmtflcnt.hxx>
#include <fmtcnct.hxx>
#include <txtflcnt.hxx>
#include <viscrs.hxx>
#include <docfld.hxx>   // fuer Expression-Felder
#include <ndtxt.hxx>
#include <ndole.hxx>
#include <rootfrm.hxx>
#include <cntfrm.hxx>
#include <flyfrm.hxx>
#include <fesh.hxx>
#include <docsh.hxx>
#include <dflyobj.hxx>
#include <dcontact.hxx>
#include <poolfmt.hxx>		// PoolVorlagen-Id's
#include <docary.hxx>
#include <fldupde.hxx>
#include <txtftn.hxx>
#include <ftnidx.hxx>

#include <comcore.hrc>		// STR-ResId's

// #i11176#
#include <unoframe.hxx>
namespace binfilter {

using namespace ::com::sun::star;
using namespace ::rtl;

#define DEF_FLY_WIDTH	 2268	//Defaultbreite fuer FlyFrms	(2268 == 4cm)

/* #109161# */
static bool lcl_IsItemSet(const SwCntntNode & rNode, USHORT which)
{
    bool bResult = false;

    if (SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState(which))
        bResult = true;

    return bResult;
}

/*************************************************************************
|*
|*	SwDoc::MakeLayoutFmt()
|*
|*	Beschreibung		Erzeugt ein neues Format das in seinen Einstellungen
|* 		Defaultmaessig zu dem Request passt. Das Format wird in das
|* 		entsprechende Formate-Array gestellt.
|* 		Wenn bereits ein passendes Format existiert, so wird dies
|* 		zurueckgeliefert.
|*	Ersterstellung		MA 22. Sep. 92
|*	Letzte Aenderung	JP 08.05.98
|*
|*************************************************************************/

/*N*/ SwFrmFmt *SwDoc::MakeLayoutFmt( RndStdIds eRequest, SwFrmFmt* pFrmFmt,
/*N*/ 								const SfxItemSet* pSet )
/*N*/ {
/*N*/ 	SwFrmFmt *pFmt = 0;
/*N*/ 	const sal_Bool bMod = IsModified();
/*N*/ 	sal_Bool bHeader = sal_False;
/*N*/
/*N*/ 	switch ( eRequest )
/*N*/ 	{
/*N*/ 	case RND_STD_HEADER:
/*N*/ 	case RND_STD_HEADERL:
/*N*/ 	case RND_STD_HEADERR:
/*N*/ 		{
/*N*/ 			bHeader = sal_True;
/*N*/ 			// kein break, es geht unten weiter
/*N*/ 		}
/*N*/ 	case RND_STD_FOOTER:
/*N*/ 	case RND_STD_FOOTERL:
/*N*/ 	case RND_STD_FOOTERR:
/*N*/ 		{
/*N*/
/*N*/
/*N*/ 			pFmt = new SwFrmFmt( GetAttrPool(),
/*N*/ 								(bHeader ? "Header" : "Footer"),
/*N*/ 								GetDfltFrmFmt() );
/*N*/
/*N*/ 			SwNodeIndex aTmpIdx( GetNodes().GetEndOfAutotext() );
/*N*/ 			SwStartNode* pSttNd = GetNodes().MakeTextSection( aTmpIdx,
/*N*/ 								bHeader ? SwHeaderStartNode : SwFooterStartNode,
/*N*/ 								GetTxtCollFromPool(
/*N*/ 								bHeader
/*N*/ 									? ( eRequest == RND_STD_HEADERL
/*N*/ 										? RES_POOLCOLL_HEADERL
/*N*/ 										: eRequest == RND_STD_HEADERR
/*N*/ 											? RES_POOLCOLL_HEADERR
/*N*/ 											: RES_POOLCOLL_HEADER )
/*N*/ 									: ( eRequest == RND_STD_FOOTERL
/*N*/ 										? RES_POOLCOLL_FOOTERL
/*N*/ 										: eRequest == RND_STD_FOOTERR
/*N*/ 											? RES_POOLCOLL_FOOTERR
/*N*/ 											: RES_POOLCOLL_FOOTER )
/*N*/ 									) );
/*N*/ 			pFmt->SetAttr( SwFmtCntnt( pSttNd ));
/*N*/
/*N*/ 			if( pSet )		// noch ein paar Attribute setzen ?
/*N*/ 				pFmt->SetAttr( *pSet );
/*N*/
/*N*/ // JP: warum zuruecksetzen ???	Doc. ist doch veraendert ???
/*N*/ // bei den Fly auf jedenfall verkehrt !!
/*N*/ 			if ( !bMod )
/*?*/ 				ResetModified();
/*N*/ 		}
/*N*/ 		break;
/*N*/
/*N*/ 	case RND_DRAW_OBJECT:
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pFmt = MakeDrawFrmFmt( aEmptyStr, GetDfltFrmFmt() );
/*?*/ 		}
/*?*/ 		break;
/*?*/
/*?*/ #ifdef DBG_UTIL
/*?*/ 	case FLY_PAGE:
/*?*/ 	case FLY_AUTO_CNTNT:
/*?*/ 	case FLY_AT_FLY:
/*?*/ 	case FLY_AT_CNTNT:
/*?*/ 	case FLY_IN_CNTNT:
/*?*/ 		ASSERT( !this,
/*?*/ 				"neue Schnittstelle benutzen: SwDoc::MakeFlySection!" );
/*?*/ 		break;
/*?*/ #endif
/*?*/
/*?*/ 	default:
/*?*/ 		ASSERT( !this,
/*?*/ 				"Layoutformat mit ungueltigem Request angefordert." );
/*?*/
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }
/*************************************************************************
|*
|*	SwDoc::DelLayoutFmt()
|*
|*	Beschreibung		Loescht das angegebene Format, der Inhalt wird mit
|* 		geloescht.
|*	Ersterstellung		MA 23. Sep. 92
|*	Letzte Aenderung	MA 05. Feb. 93
|*
|*************************************************************************/

/*N*/ void SwDoc::DelLayoutFmt( SwFrmFmt *pFmt )
/*N*/ {
/*N*/ 	//Verkettung von Rahmen muss ggf. zusammengefuehrt werden.
/*N*/ 	//Bevor die Frames vernichtet werden, damit die Inhalte der Rahmen
/*N*/ 	//ggf. entsprechend gerichtet werden.
/*N*/ 	const SwFmtChain &rChain = pFmt->GetChain();
/*N*/ 	if ( rChain.GetPrev() )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwFmtChain aChain( rChain.GetPrev()->GetChain() );
/*N*/ 	}
/*N*/ 	if ( rChain.GetNext() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwFmtChain aChain( rChain.GetNext()->GetChain() );
/*N*/ 	}

/*N*/ 	const SwNodeIndex* pCntIdx = pFmt->GetCntnt().GetCntntIdx();
/*N*/   if( pCntIdx )
/*N*/ 	{
/*N*/ 		//Verbindung abbauen, falls es sich um ein OLE-Objekt handelt.
/*N*/ 		SwOLENode* pOLENd = GetNodes()[ pCntIdx->GetIndex()+1 ]->GetOLENode();
/*N*/ 		if( pOLENd && pOLENd->GetOLEObj().IsOleRef() )
/*N*/ 		{
/*N*/ 			SwDoc* pDoc = (SwDoc*)pFmt->GetDoc();
/*N*/ 			if( pDoc )
/*N*/ 			{
/*N*/ 				SvPersist* p = pDoc->GetPersist();
/*N*/ 				if( p )		// muss da sein
/*N*/ 				{
/*N*/ 					SvInfoObjectRef aRef( p->Find( pOLENd->GetOLEObj().GetName() ) );
/*N*/ 					if( aRef.Is() )
/*N*/ 						aRef->SetObj(0);
/*N*/ 				}
/*N*/ 			}
/*N*/
/*N*/ 			pOLENd->GetOLEObj().GetOleRef()->DoClose();
/*N*/ 			pOLENd->GetOLEObj().GetOleRef().Clear();
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	//Frms vernichten.
/*N*/ 	pFmt->DelFrms();

/*N*/ 	const sal_uInt16 nWh = pFmt->Which();
/*N*/ 	{
/*N*/ 		//Inhalt Loeschen.
/*N*/ 		if( pCntIdx )
/*N*/ 		{
/*N*/
/*N*/
/*N*/ 			SwNode *pNode = &pCntIdx->GetNode();
/*N*/ 			((SwFmtCntnt&)pFmt->GetAttr( RES_CNTNT )).SetNewCntntIdx( 0 );
/*N*/ 			DeleteSection( pNode );
/*N*/ 		}
/*N*/
/*N*/ 		// ggfs. bei Zeichengebundenen Flys das Zeichen loeschen
/*N*/ 		const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
/*N*/ 		if( FLY_IN_CNTNT == rAnchor.GetAnchorId() && rAnchor.GetCntntAnchor())
/*N*/ 		{
/*N*/ 			const SwPosition* pPos = rAnchor.GetCntntAnchor();
/*N*/ 			SwTxtNode *pTxtNd = pPos->nNode.GetNode().GetTxtNode();
/*N*/ 			SwTxtFlyCnt* pAttr;
/*N*/
/*N*/ 			// Attribut steht noch im TextNode, loeschen
/*N*/ 			if( pTxtNd && 0 != ( pAttr = ((SwTxtFlyCnt*)pTxtNd->GetTxtAttr(
/*N*/ 											pPos->nContent.GetIndex() ))) &&
/*N*/ 				pAttr->GetFlyCnt().GetFrmFmt() == pFmt )
/*N*/ 			{
/*?*/ 				// Pointer auf 0, nicht loeschen
/*?*/ 				((SwFmtFlyCnt&)pAttr->GetFlyCnt()).SetFlyFmt();
/*?*/ 				SwIndex aIdx( pPos->nContent );
/*?*/ 				pTxtNd->Erase( aIdx, 1 );
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		DelFrmFmt( pFmt );
/*N*/ 	}
/*N*/ 	SetModified();
/*N*/ }

/*************************************************************************
|*
|*	SwDoc::CopyLayoutFmt()
|*
|*	Beschreibung		Kopiert das angegebene Format pSrc in pDest und
|*						returnt pDest. Wenn es noch kein pDest gibt, wird
|*						eins angelegt.
|*						JP: steht das Source Format in einem anderen
|*							Dokument, so kopiere auch dann noch richtig !!
|*							Vom chaos::Anchor-Attribut wird die Position immer
|*							auf 0 gesetzt !!!
|*
|*	Ersterstellung		BP 18.12.92
|*	Letzte Aenderung	MA 17. Jul. 96
|*
|*************************************************************************/

/*N*/ SwFrmFmt *SwDoc::CopyLayoutFmt( const SwFrmFmt& rSource,
/*N*/ 								const SwFmtAnchor& rNewAnchor,
/*N*/ 								sal_Bool bSetTxtFlyAtt, sal_Bool bMakeFrms )
/*N*/ {
/*N*/     const bool bFly = RES_FLYFRMFMT == rSource.Which();
/*N*/     const bool bDraw = RES_DRAWFRMFMT == rSource.Which();
/*N*/     ASSERT( bFly || bDraw, "this method only works for fly or draw" );
/*N*/
/*N*/ 	SwDoc* pSrcDoc = (SwDoc*)rSource.GetDoc();
/*N*/
/*N*/     // #108784# may we copy this object?
/*N*/     // We may, unless it's 1) it's a control (and therfore a draw)
/*N*/     //                     2) anchored in a header/footer
/*N*/     //                     3) anchored (to paragraph?)
/*N*/     bool bMayNotCopy = false;
/*N*/     if( bDraw )
/*N*/     {
/*N*/         const SwDrawContact* pDrawContact =
/*N*/             static_cast<const SwDrawContact*>( rSource.FindContactObj() );
/*N*/
/*N*/         bMayNotCopy =
/*N*/             ( FLY_AT_CNTNT == rNewAnchor.GetAnchorId() ||
/*N*/               FLY_AT_FLY == rNewAnchor.GetAnchorId() ||
/*N*/               FLY_AUTO_CNTNT == rNewAnchor.GetAnchorId() ) &&
/*N*/             rNewAnchor.GetCntntAnchor() &&
/*N*/             IsInHeaderFooter( rNewAnchor.GetCntntAnchor()->nNode ) &&
/*N*/             pDrawContact != NULL  &&
/*N*/             pDrawContact->GetMaster() != NULL  &&
/*N*/             CheckControlLayer( pDrawContact->GetMaster() );
/*N*/     }
/*N*/
/*N*/     // just return if we can't copy this
/*N*/     if( bMayNotCopy )
/*N*/         return NULL;
/*N*/
/*N*/ 	SwFrmFmt* pDest = GetDfltFrmFmt();
/*N*/ 	if( rSource.GetRegisteredIn() != pSrcDoc->GetDfltFrmFmt() )
/*N*/ 		pDest = CopyFrmFmt( *(SwFrmFmt*)rSource.GetRegisteredIn() );
/*N*/ 	if( bFly )
/*N*/ 	{
/*N*/ 		// #i11176#
/*N*/ 		// To do a correct cloning concerning the ZOrder for all objects
/*N*/ 		// it is necessary to actually create a draw object for fly frames, too.
/*N*/ 		// These are then added to the DrawingLayer (which needs to exist).
/*N*/ 		// Together with correct sorting of all drawinglayer based objects
/*N*/ 		// before cloning ZOrder transfer works correctly then.
/*N*/ 		SwFlyFrmFmt *pFormat = MakeFlyFrmFmt( rSource.GetName(), pDest );
/*N*/ 		pDest = pFormat;
/*N*/
/*N*/ 		SwXFrame::GetOrCreateSdrObject(pFormat);
/*N*/ 	}
/*N*/ 	else
/*N*/ 		pDest = MakeDrawFrmFmt( aEmptyStr, pDest );
/*N*/
/*N*/ 	// alle anderen/neue Attribute kopieren.
/*N*/ 	pDest->CopyAttrs( rSource );
/*N*/
/*N*/ 	//Chains werden nicht kopiert.
/*N*/ 	pDest->ResetAttr( RES_CHAIN );
/*N*/
/*N*/ 	if( bFly )
/*N*/ 	{
/*N*/ 		//Der Inhalt wird dupliziert.
/*N*/ 		const SwNode& rCSttNd = rSource.GetCntnt().GetCntntIdx()->GetNode();
/*N*/ 		SwNodeRange aRg( rCSttNd, 1, *rCSttNd.EndOfSectionNode() );
/*N*/
/*N*/ 		SwNodeIndex aIdx( GetNodes().GetEndOfAutotext() );
/*N*/ 		SwStartNode* pSttNd = GetNodes().MakeEmptySection( aIdx, SwFlyStartNode );
/*N*/
/*N*/ 		// erst den chaos::Anchor/CntntIndex setzen, innerhalb des Kopierens
/*N*/ 		// auf die Werte zugegriffen werden kann (DrawFmt in Kopf-/Fusszeilen)
/*N*/ 		aIdx = *pSttNd;
/*N*/ 		SwFmtCntnt aAttr( rSource.GetCntnt() );
/*N*/ 		aAttr.SetNewCntntIdx( &aIdx );
/*N*/ 		pDest->SetAttr( aAttr );
/*N*/ 		pDest->SetAttr( rNewAnchor );
/*N*/
/*N*/ 		if( !bCopyIsMove || this != pSrcDoc )
/*N*/ 		{
/*N*/ 			if( bInReading )
/*N*/ 				pDest->SetName( aEmptyStr );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				// Teste erstmal ob der Name schon vergeben ist.
/*N*/ 				// Wenn ja -> neuen generieren
/*N*/ 				sal_Int8 nNdTyp = aRg.aStart.GetNode().GetNodeType();
/*N*/
/*N*/ 				String sOld( pDest->GetName() );
/*N*/ 				pDest->SetName( aEmptyStr );
/*N*/ 				if( FindFlyByName( sOld, nNdTyp ) )		// einen gefunden
/*N*/ 					switch( nNdTyp )
/*N*/ 					{
/*N*/ 					case ND_GRFNODE:	sOld = GetUniqueGrfName();		break;
/*N*/ 					case ND_OLENODE:	sOld = GetUniqueOLEName();		break;
/*N*/ 					default:			sOld = GetUniqueFrameName();	break;
/*N*/ 					}
/*N*/
/*N*/ 				pDest->SetName( sOld );
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/
/*N*/ 		// sorge dafuer das auch Fly's in Fly's kopiert werden
/*N*/ 		aIdx = *pSttNd->EndOfSectionNode();
/*N*/ 		pSrcDoc->CopyWithFlyInFly( aRg, aIdx, sal_False, sal_True, sal_True );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( RES_DRAWFRMFMT == rSource.Which(), "Weder Fly noch Draw." );
/*N*/ 		SwDrawContact *pContact = (SwDrawContact *)rSource.FindContactObj();
/*N*/
/*N*/ 		pContact = new SwDrawContact( (SwDrawFrmFmt*)pDest,
/*N*/ 								CloneSdrObj( *pContact->GetMaster(),
/*N*/ 										bCopyIsMove && this == pSrcDoc ) );
/*N*/
/*N*/ 		if( pDest->GetAnchor() == rNewAnchor )
/*N*/         {
/*N*/             // OD 03.07.2003 #108784# - do *not* connect to layout, if
/*N*/             // a <MakeFrms> will not be called.
/*N*/             if ( bMakeFrms )
/*N*/             {
/*N*/                 pContact->ConnectToLayout( &rNewAnchor );
/*N*/             }
/*N*/         }
/*N*/ 		else
/*N*/ 			pDest->SetAttr( rNewAnchor );
/*N*/
/*N*/ 	}
/*N*/
/*N*/ 	if( bSetTxtFlyAtt && FLY_IN_CNTNT == rNewAnchor.GetAnchorId() )
/*N*/ 	{
/*N*/ 		SwPosition* pPos = (SwPosition*)rNewAnchor.GetCntntAnchor();
/*N*/ 		pPos->nNode.GetNode().GetTxtNode()->Insert(SwFmtFlyCnt( pDest ),
/*N*/ 											pPos->nContent.GetIndex(), 0 );
/*N*/ 	}
/*N*/
/*N*/ 	if( bMakeFrms )
/*N*/ 		pDest->MakeFrms();
/*N*/
/*N*/ 	return pDest;
/*N*/ }

/*N*/ SdrObject* SwDoc::CloneSdrObj( const SdrObject& rObj, sal_Bool bMoveWithinDoc,
/*N*/ 								sal_Bool bInsInPage )
/*N*/ {
/*N*/ 	SdrPage *pPg = MakeDrawModel()->GetPage( 0 );
/*N*/ 	if( !pPg )
/*N*/ 	{
/*?*/ 		pPg = GetDrawModel()->AllocPage( sal_False );
/*?*/ 		GetDrawModel()->InsertPage( pPg );
/*N*/ 	}
/*N*/
/*N*/ 	SdrObject *pObj = rObj.Clone();
/*N*/ 	if( bMoveWithinDoc && FmFormInventor == pObj->GetObjInventor() )
/*N*/ 	{
/*?*/ 		// bei Controls muss der Name erhalten bleiben
/*?*/ 		uno::Reference< awt::XControlModel >  xModel = ((SdrUnoObj*)pObj)->GetUnoControlModel();
/*?*/ 		sal_Bool bModel = xModel.is();
/*?*/ 		uno::Any aVal;
/*?*/ 		uno::Reference< beans::XPropertySet >  xSet(xModel, uno::UNO_QUERY);
/*?*/ 		OUString sName( ::rtl::OUString::createFromAscii("Name") );
/*?*/ 		if( xSet.is() )
/*?*/ 			aVal = xSet->getPropertyValue( sName );
/*?*/ 		if( bInsInPage )
/*?*/ 			pPg->InsertObject( pObj );
/*?*/ 		if( xSet.is() )
/*?*/ 			xSet->setPropertyValue( sName, aVal );
/*N*/ 	}
/*N*/ 	else if( bInsInPage )
/*N*/ 		pPg->InsertObject( pObj );
/*N*/
/*N*/     // OD 02.07.2003 #108784# - for drawing objects: set layer of cloned object
/*N*/     // to invisible layer
/*N*/     SdrLayerID nLayerIdForClone = rObj.GetLayer();
/*N*/     if ( !pObj->ISA(SwFlyDrawObj) &&
/*N*/          !pObj->ISA(SwVirtFlyDrawObj) &&
/*N*/          !IS_TYPE(SdrObject,pObj) )
/*N*/     {
/*N*/         if ( IsVisibleLayerId( nLayerIdForClone ) )
/*N*/         {
/*N*/             nLayerIdForClone = GetInvisibleLayerIdByVisibleOne( nLayerIdForClone );
/*N*/         }
/*N*/     }
/*N*/     pObj->SetLayer( nLayerIdForClone );
/*N*/
/*N*/
/*N*/ 	return pObj;
/*N*/ }

/*N*/ SwFlyFrmFmt* SwDoc::_MakeFlySection( const SwPosition& rAnchPos,
/*N*/ 									const SwCntntNode& rNode,
/*N*/ 									RndStdIds eRequestId,
/*N*/ 									const SfxItemSet* pFlySet,
/*N*/ 									SwFrmFmt* pFrmFmt )
/*N*/ {
/*N*/ 	if( !pFrmFmt )
/*?*/ 		pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );
/*N*/
/*N*/ 	String sName;
/*N*/ 	if( !bInReading )
/*N*/ 		switch( rNode.GetNodeType() )
/*N*/ 		{
/*N*/ 		case ND_GRFNODE:		sName = GetUniqueGrfName();		break;
/*N*/ 		case ND_OLENODE:        sName = GetUniqueOLEName();		break;
/*N*/ 		default:				sName = GetUniqueFrameName();		break;
/*N*/ 		}
/*N*/ 	SwFlyFrmFmt* pFmt = MakeFlyFrmFmt( sName, pFrmFmt );
/*N*/
/*N*/ 	//Inhalt erzeugen und mit dem Format verbinden.
/*N*/ 	//CntntNode erzeugen und in die Autotextsection stellen
/*N*/ 	SwNodeRange aRange( GetNodes().GetEndOfAutotext(), -1,
/*N*/ 						GetNodes().GetEndOfAutotext() );
/*N*/ 	GetNodes().SectionDown( &aRange, SwFlyStartNode );
/*N*/
/*N*/ 	pFmt->SetAttr( SwFmtCntnt( rNode.StartOfSectionNode() ));
/*N*/
/*N*/
/*N*/ 	const SwFmtAnchor* pAnchor = 0;
/*N*/ 	if( pFlySet )
/*N*/ 	{
/*N*/ 		pFlySet->GetItemState( RES_ANCHOR, sal_False,
/*N*/ 								(const SfxPoolItem**)&pAnchor );
/*N*/ 		if( SFX_ITEM_SET == pFlySet->GetItemState( RES_CNTNT, sal_False ))
/*N*/ 		{
/*?*/ 			SfxItemSet aTmpSet( *pFlySet );
/*?*/ 			aTmpSet.ClearItem( RES_CNTNT );
/*?*/ 			pFmt->SetAttr( aTmpSet );
/*N*/ 		}
/*N*/ 		else
/*N*/ 			pFmt->SetAttr( *pFlySet );
/*N*/ 	}

    // Anker noch nicht gesetzt ?
/*N*/ 	RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
/*N*/ 								  : pFmt->GetAnchor().GetAnchorId();
/*N*/ 	if( !pAnchor ||
/*N*/ 		(FLY_PAGE != pAnchor->GetAnchorId() &&
/*N*/ 		  //Nur Page und nicht:
/*N*/ //		  FLY_AT_CNTNT == pAnchor->GetAnchorId() ||
/*N*/ //		  FLY_IN_CNTNT == pAnchor->GetAnchorId() ||
/*N*/ //		  FLY_AT_FLY == pAnchor->GetAnchorId() ||
/*N*/ //		  FLY_AUTO_CNTNT == pAnchor->GetAnchorId() ) &&
/*N*/ 		!pAnchor->GetCntntAnchor() ))
/*N*/ 	{
/*N*/ 		// dann setze ihn, wird im Undo gebraucht
/*N*/ 		SwFmtAnchor aAnch( pFmt->GetAnchor() );
/*N*/ 		if( pAnchor && FLY_AT_FLY == pAnchor->GetAnchorId() )
/*N*/ 		{
/*?*/ 			SwPosition aPos( *rAnchPos.nNode.GetNode().FindFlyStartNode() );
/*?*/ 			aAnch.SetAnchor( &aPos );
/*?*/ 			eAnchorId = FLY_AT_FLY;
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			if( eRequestId != aAnch.GetAnchorId() &&
/*N*/ 				SFX_ITEM_SET != pFmt->GetItemState( RES_ANCHOR, sal_True ) )
/*?*/ 				aAnch.SetType( eRequestId );
/*N*/
/*N*/ 			eAnchorId = aAnch.GetAnchorId();
/*N*/ 			if ( FLY_PAGE != eAnchorId )
/*N*/ 			//Nur Page und nicht:
/*N*/ //			if( FLY_AT_CNTNT == eAnchorId || FLY_IN_CNTNT == eAnchorId ||
/*N*/ //		  		FLY_AT_FLY == eAnchorId || FLY_AUTO_CNTNT == eAnchorId )
/*N*/ 				aAnch.SetAnchor( &rAnchPos );
/*N*/ 		}
/*N*/ 		pFmt->SetAttr( aAnch );
/*N*/ 	}
/*N*/ 	else
/*N*/ 		eAnchorId = pFmt->GetAnchor().GetAnchorId();
/*N*/
/*N*/ 	if( FLY_IN_CNTNT == eAnchorId )
/*N*/ 	{
/*N*/ 		xub_StrLen nStt = rAnchPos.nContent.GetIndex();
/*N*/ 		rAnchPos.nNode.GetNode().GetTxtNode()->Insert(
/*N*/ 										SwFmtFlyCnt( pFmt ), nStt, nStt );
/*N*/ 	}
/*N*/
/*N*/ 	if( SFX_ITEM_SET != pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE ))
/*N*/ 	{
/*?*/ 		SwFmtFrmSize aFmtSize( ATT_VAR_SIZE, 0, DEF_FLY_WIDTH );
/*?*/ 		const SwNoTxtNode* pNoTxtNode = rNode.GetNoTxtNode();
/*?*/ 		if( pNoTxtNode )
/*?*/ 		{
/*?*/ 			//Groesse einstellen.
/*?*/ 		 	Size aSize( pNoTxtNode->GetTwipSize() );
/*?*/ 			if( MINFLY > aSize.Width() )
/*?*/ 				aSize.Width() = DEF_FLY_WIDTH;
/*?*/ 			aFmtSize.SetWidth( aSize.Width() );
/*?*/ 			if( aSize.Height() )
/*?*/ 			{
/*?*/ 				aFmtSize.SetHeight( aSize.Height() );
/*?*/ 				aFmtSize.SetSizeType( ATT_FIX_SIZE );
/*?*/ 			}
/*?*/ 		}
/*?*/ 		pFmt->SetAttr( aFmtSize );
/*N*/ 	}

    // Frames anlegen
/*N*/ 	if( GetRootFrm() )
/*N*/ 		pFmt->MakeFrms();			// ???
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*N*/ SwFlyFrmFmt* SwDoc::MakeFlySection( RndStdIds eAnchorType,
/*N*/ 									const SwPosition* pAnchorPos,
/*N*/ 									const SfxItemSet* pFlySet,
/*N*/ 									SwFrmFmt* pFrmFmt, BOOL bCalledFromShell )
/*N*/ {
/*N*/ 	SwFlyFrmFmt* pFmt = 0;
/*N*/ 	sal_Bool bCallMake = sal_True;
/*N*/ 	if( !pAnchorPos && FLY_PAGE != eAnchorType )
/*N*/ 	{
/*N*/ 		const SwFmtAnchor* pAnch;
/*N*/ 		if( (pFlySet && SFX_ITEM_SET == pFlySet->GetItemState(
/*N*/ 				RES_ANCHOR, sal_False, (const SfxPoolItem**)&pAnch )) ||
/*N*/ 			( pFrmFmt && SFX_ITEM_SET == pFrmFmt->GetItemState(
/*N*/ 				RES_ANCHOR, sal_True, (const SfxPoolItem**)&pAnch )) )
/*N*/ 		{
/*?*/ 			if( FLY_PAGE != pAnch->GetAnchorId() &&
/*?*/ 				0 == ( pAnchorPos = pAnch->GetCntntAnchor() ) )
/*?*/ 				bCallMake = sal_False;
/*N*/ 		}
/*N*/ 	}

/*N*/ 	if( bCallMake )
/*N*/ 	{
/*N*/ 		if( !pFrmFmt )
/*?*/ 			pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_FRAME );
/*N*/
/*N*/ 		sal_uInt16 nCollId = IsHTMLMode() ? RES_POOLCOLL_TEXT : RES_POOLCOLL_FRAME;
/*N*/
        /* #109161# If there exists no adjust item in the paragraph
            style for the content node of the new fly section
            propagate an existing adjust item at the anchor to the new
            content node. */
/*N*/         SwCntntNode * pNewTxtNd = GetNodes().MakeTxtNode
/*N*/             (SwNodeIndex( GetNodes().GetEndOfAutotext()),
/*N*/              GetTxtCollFromPool( nCollId ));
/*N*/         SwCntntNode * pAnchorNode = pAnchorPos->nNode.GetNode().GetCntntNode();
/*N*/
/*N*/         const SfxPoolItem * pItem = NULL;
/*N*/
/*N*/         if (bCalledFromShell && !lcl_IsItemSet(*pNewTxtNd, RES_PARATR_ADJUST) &&
/*N*/             SFX_ITEM_SET == pAnchorNode->GetSwAttrSet().
/*N*/             GetItemState(RES_PARATR_ADJUST, TRUE, &pItem))
/*N*/             static_cast<SwCntntNode *>(pNewTxtNd)->SetAttr(*pItem);
/*N*/
/*N*/  		pFmt = _MakeFlySection( *pAnchorPos, *pNewTxtNd,
/*N*/ 								eAnchorType, pFlySet, pFrmFmt );
/*N*/ 	}
/*N*/ 	return pFmt;
/*N*/ }



    //Einfuegen eines DrawObjectes. Das Object muss bereits im DrawModel
    // angemeldet sein.
/*N*/ SwDrawFrmFmt* SwDoc::Insert( const SwPaM &rRg,
/*N*/ 							 SdrObject& rDrawObj,
/*N*/ 							 const SfxItemSet* pFlyAttrSet,
/*N*/ 							 SwFrmFmt* pDefFmt )
/*N*/ {
/*N*/ 	SwDrawFrmFmt *pFmt = MakeDrawFrmFmt( aEmptyStr,
/*N*/ 										pDefFmt ? pDefFmt : GetDfltFrmFmt() );
/*N*/
/*N*/ 	const SwFmtAnchor* pAnchor = 0;
/*N*/ 	if( pFlyAttrSet )
/*N*/ 	{
/*N*/ 		pFlyAttrSet->GetItemState( RES_ANCHOR, sal_False,
/*N*/ 									(const SfxPoolItem**)&pAnchor );
/*N*/ 		pFmt->SetAttr( *pFlyAttrSet );
/*N*/ 	}
/*N*/
/*N*/ 	RndStdIds eAnchorId = pAnchor ? pAnchor->GetAnchorId()
/*N*/ 								  : pFmt->GetAnchor().GetAnchorId();
/*N*/
/*N*/ 	// Anker noch nicht gesetzt ?
/*N*/ 	// DrawObjecte duerfen niemals in Kopf-/Fusszeilen landen.
/*N*/ 	sal_Bool bIsAtCntnt = FLY_PAGE != eAnchorId;
/*N*/ //					  FLY_AT_CNTNT == eAnchorId || FLY_IN_CNTNT == eAnchorId ||
/*N*/ //		  	 		  FLY_AT_FLY == eAnchorId || FLY_AUTO_CNTNT == eAnchorId;
/*N*/
/*N*/ 	const SwNodeIndex* pChkIdx = 0;
/*N*/ 	if( !pAnchor )
/*N*/     {
/*N*/ 		pChkIdx = &rRg.GetPoint()->nNode;
/*N*/     }
/*N*/ 	else if( bIsAtCntnt )
/*N*/     {
/*N*/ 		pChkIdx = pAnchor->GetCntntAnchor()
/*N*/ 					? &pAnchor->GetCntntAnchor()->nNode
/*N*/ 					: &rRg.GetPoint()->nNode;
/*N*/     }
/*N*/
/*N*/     // OD 24.06.2003 #108784# - allow drawing objects in header/footer, but
/*N*/     // control objects aren't allowed in header/footer.
/*N*/     if( pChkIdx &&
/*N*/         ::binfilter::CheckControlLayer( &rDrawObj ) &&
/*N*/         IsInHeaderFooter( *pChkIdx ) )
/*N*/     {
/*N*/        pFmt->SetAttr( SwFmtAnchor( eAnchorId = FLY_PAGE ) );
/*N*/     }
/*N*/ 	else if( !pAnchor || (bIsAtCntnt && !pAnchor->GetCntntAnchor() ))
/*N*/ 	{
/*N*/ 		// dann setze ihn, wird im Undo gebraucht
/*N*/ 		SwFmtAnchor aAnch( pAnchor ? *pAnchor : pFmt->GetAnchor() );
/*N*/ 		eAnchorId = aAnch.GetAnchorId();
/*N*/ 		if( FLY_AT_FLY == eAnchorId )
/*N*/ 		{
/*?*/ 			SwPosition aPos( *rRg.GetNode()->FindFlyStartNode() );
/*?*/ 			aAnch.SetAnchor( &aPos );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			aAnch.SetAnchor( rRg.GetPoint() );
/*N*/ 			if( FLY_PAGE == eAnchorId )
/*N*/ 			{
/*?*/ 				eAnchorId = rDrawObj.ISA( SdrUnoObj )
/*?*/ 									? FLY_IN_CNTNT : FLY_AT_CNTNT;
/*?*/ 				aAnch.SetType( eAnchorId );
/*?*/ 			}
/*N*/ 		}
/*N*/ 		pFmt->SetAttr( aAnch );
/*N*/ 	}
/*N*/
/*N*/ 	// bei als Zeichen gebundenen Draws das Attribut im Absatz setzen
/*N*/ 	if( FLY_IN_CNTNT == eAnchorId )
/*N*/ 	{
/*N*/ 		xub_StrLen nStt = rRg.GetPoint()->nContent.GetIndex();
/*N*/ 		rRg.GetPoint()->nNode.GetNode().GetTxtNode()->Insert(
/*N*/ 										SwFmtFlyCnt( pFmt ), nStt, nStt );
/*N*/ 	}
/*N*/
/*N*/ 	new SwDrawContact( pFmt, &rDrawObj );
/*N*/
/*N*/ 	// ggfs. Frames anlegen
/*N*/ 	if( GetRootFrm() )
/*N*/ 		pFmt->MakeFrms();
/*N*/
/*N*/ 	SetModified();
/*N*/ 	return pFmt;
/*N*/ }

/*************************************************************************
|*
|*	SwDoc::GetAllFlyFmts
|*
|*	Ersterstellung		MA 14. Jul. 93
|*	Letzte Aenderung	MD 23. Feb. 95
|*
|*************************************************************************/

/*sal_Bool TstFlyRange( const SwPaM* pPam, sal_uInt32 nFlyPos )
{
    sal_Bool bOk = sal_False;
    const SwPaM* pTmp = pPam;
    do {
        bOk = pTmp->Start()->nNode.GetIndex() < nFlyPos &&
                pTmp->End()->nNode.GetIndex() > nFlyPos;
    } while( !bOk && pPam != ( pTmp = (const SwPaM*)pTmp->GetNext() ));
    return bOk;
}
*/
/* -----------------------------04.04.00 10:55--------------------------------
    paragraph frames - o.k. if the PaM includes the paragraph from the beginning
                       to the beginning of the next paragraph at least
    frames at character - o.k. if the pam start at least at the same position
                         as the frame
 ---------------------------------------------------------------------------*/


/*N*/ void SwDoc::GetAllFlyFmts( SwPosFlyFrms& rPosFlyFmts,
/*N*/ 						   const SwPaM* pCmpRange, sal_Bool bDrawAlso ) const
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwPosFlyFrm *pFPos = 0;
/*N*/ }

/*************************************************************************
|*
|*	SwDoc::InsertLabel()
|*
|*	Ersterstellung		MA 11. Feb. 94
|*	Letzte Aenderung	MA 12. Nov. 97
|*
|*************************************************************************/

/* #i6447# changed behaviour if lcl_CpyAttr:

   If the old item set contains the item to set (no inheritance) copy the item
   into the new set.

   If the old item set contains the item by inheritance and the new set
   contains the item, too:
      If the two items differ copy the item from the old set to the new set.

   Otherwise the new set will not be changed.
*/




/*************************************************************************
|*
|*	SwDoc::InsertDrawLabel()
|*
|*	Ersterstellung		MIB 7. Dez. 98
|*	Letzte Aenderung	MIB 7. Dez. 98
|*
|*************************************************************************/


/*************************************************************************
|*
|*	SwDoc::DoIdleJobs()
|*
|*	Ersterstellung		OK 30.03.94
|*	Letzte Aenderung	MA 09. Jun. 95
|*
|*************************************************************************/

/*N*/ IMPL_LINK( SwDoc, DoIdleJobs, Timer *, pTimer )
/*N*/ {
/*N*/ #ifdef TIMELOG
/*N*/ 	static ::rtl::Logfile* pModLogFile = 0;
/*N*/ 	if( !pModLogFile )
/*N*/ 		pModLogFile = new ::rtl::Logfile( "First DoIdleJobs" );
/*N*/ #endif
/*N*/
/*N*/ 	if( !SfxProgress::GetActiveProgress( pDocShell ) &&
/*N*/ 		GetRootFrm() && GetRootFrm()->GetCurrShell() )
/*N*/ 	{
/*N*/ 		ViewShell *pSh, *pStartSh;
/*N*/ 		pSh = pStartSh = GetRootFrm()->GetCurrShell();
/*N*/ 		do {
/*N*/ 			if( pSh->ActionPend() )
/*N*/ 				return 0;
/*N*/ 			pSh = (ViewShell*)pSh->GetNext();
/*N*/ 		} while( pSh != pStartSh );
/*N*/
/*N*/ 		sal_uInt16 nFldUpdFlag;
/*N*/ 		if( GetRootFrm()->IsIdleFormat() )
/*N*/ 			GetRootFrm()->GetCurrShell()->LayoutIdle();
/*N*/ 		else if( ( AUTOUPD_FIELD_ONLY == ( nFldUpdFlag = GetFldUpdateFlags() )
/*N*/ 					|| AUTOUPD_FIELD_AND_CHARTS == nFldUpdFlag ) &&
/*N*/ 				GetUpdtFlds().IsFieldsDirty() &&
/*N*/ 				!GetUpdtFlds().IsInUpdateFlds() &&
/*N*/ 				!IsExpFldsLocked()
/*N*/ 				// das umschalten der Feldname fuehrt zu keinem Update der
/*N*/ 				// Felder, also der "Hintergrund-Update" immer erfolgen
/*N*/ 				/* && !pStartSh->GetViewOptions()->IsFldName()*/ )
/*N*/ 		{
/*N*/ 			// chaos::Action-Klammerung!
/*N*/ 			GetUpdtFlds().SetInUpdateFlds( sal_True );
/*N*/
/*N*/ 			GetRootFrm()->StartAllAction();
/*N*/
/*N*/ 			GetSysFldType( RES_CHAPTERFLD )->Modify( 0, 0 );	// KapitelFld
/*N*/ 			UpdateExpFlds( 0, sal_False );		// Expression-Felder Updaten
/*N*/ 			UpdateTblFlds();				// Tabellen
/*N*/ 			UpdateRefFlds();				// Referenzen
/*N*/
/*N*/ 			if( AUTOUPD_FIELD_AND_CHARTS == nFldUpdFlag )
/*N*/ 				aChartTimer.Start();
/*N*/
/*N*/ 			GetRootFrm()->EndAllAction();
/*N*/
/*N*/ 			GetUpdtFlds().SetInUpdateFlds( sal_False );
/*N*/ 			GetUpdtFlds().SetFieldsDirty( sal_False );
/*N*/ 		}
/*N*/ 	}
/*N*/ #ifdef TIMELOG
/*N*/ 	if( pModLogFile && 1 != (long)pModLogFile )
/*N*/ 		delete pModLogFile, ((long&)pModLogFile) = 1;
/*N*/ #endif
/*N*/ 	return 0;
/*N*/ }


/*N*/ String lcl_GetUniqueFlyName( const SwDoc* pDoc, sal_uInt16 nDefStrId )
/*N*/ {
/*N*/ 	ResId aId( nDefStrId, *pSwResMgr );
/*N*/ 	String aName( aId );
/*N*/ 	xub_StrLen nNmLen = aName.Len();
/*N*/
/*N*/ 	const SwSpzFrmFmts& rFmts = *pDoc->GetSpzFrmFmts();
/*N*/
/*N*/ 	sal_uInt16 nNum, nTmp, nFlagSize = ( rFmts.Count() / 8 ) +2;
/*N*/ 	sal_uInt8* pSetFlags = new sal_uInt8[ nFlagSize ];
/*N*/ 	memset( pSetFlags, 0, nFlagSize );
/*N*/
        sal_uInt16 n=0;
/*N*/ 	for( n = 0; n < rFmts.Count(); ++n )
/*N*/ 	{
/*N*/ 		const SwFrmFmt* pFlyFmt = rFmts[ n ];
/*N*/ 		if( RES_FLYFRMFMT == pFlyFmt->Which() &&
/*N*/ 			pFlyFmt->GetName().Match( aName ) == nNmLen )
/*N*/ 		{
/*N*/ 			// Nummer bestimmen und das Flag setzen
/*N*/ 			nNum = pFlyFmt->GetName().Copy( nNmLen ).ToInt32();
/*N*/ 			if( nNum-- && nNum < rFmts.Count() )
/*N*/ 				pSetFlags[ nNum / 8 ] |= (0x01 << ( nNum & 0x07 ));
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	// alle Nummern entsprechend geflag, also bestimme die richtige Nummer
/*N*/ 	nNum = rFmts.Count();
/*N*/ 	for( n = 0; n < nFlagSize; ++n )
/*N*/ 		if( 0xff != ( nTmp = pSetFlags[ n ] ))
/*N*/ 		{
/*N*/ 			// also die Nummer bestimmen
/*N*/ 			nNum = n * 8;
/*N*/ 			while( nTmp & 1 )
/*N*/ 				++nNum, nTmp >>= 1;
/*N*/ 			break;
/*N*/ 		}
/*N*/
/*N*/ 	delete [] pSetFlags;
/*N*/ 	return aName += String::CreateFromInt32( ++nNum );
/*N*/ }

/*N*/ String SwDoc::GetUniqueGrfName() const
/*N*/ {
/*N*/ 	return ::binfilter::lcl_GetUniqueFlyName( this, STR_GRAPHIC_DEFNAME );
/*N*/ }

/*N*/ String SwDoc::GetUniqueOLEName() const
/*N*/ {
/*N*/ 	return ::binfilter::lcl_GetUniqueFlyName( this, STR_OBJECT_DEFNAME );
/*N*/ }

/*N*/ String SwDoc::GetUniqueFrameName() const
/*N*/ {
/*N*/ 	return ::binfilter::lcl_GetUniqueFlyName( this, STR_FRAME_DEFNAME );
/*N*/ }

/*N*/ const SwFlyFrmFmt* SwDoc::FindFlyByName( const String& rName, sal_Int8 nNdTyp ) const
/*N*/ {
/*N*/ 	const SwSpzFrmFmts& rFmts = *GetSpzFrmFmts();
/*N*/ 	for( sal_uInt16 n = rFmts.Count(); n; )
/*N*/ 	{
/*N*/ 		const SwFrmFmt* pFlyFmt = rFmts[ --n ];
/*N*/ 		const SwNodeIndex* pIdx;
/*N*/ 		if( RES_FLYFRMFMT == pFlyFmt->Which() && pFlyFmt->GetName() == rName &&
/*N*/ 			0 != ( pIdx = pFlyFmt->GetCntnt().GetCntntIdx() ) &&
/*N*/ 			pIdx->GetNode().GetNodes().IsDocNodes() )
/*N*/ 		{
/*?*/ 			if( nNdTyp )
/*?*/ 			{
/*?*/ 				// dann noch auf den richtigen Node-Typ abfragen
/*?*/ 				const SwNode* pNd = GetNodes()[ pIdx->GetIndex()+1 ];
/*?*/ 				if( nNdTyp == ND_TEXTNODE
/*?*/ 						? !pNd->IsNoTxtNode()
/*?*/ 						: nNdTyp == pNd->GetNodeType() )
/*?*/ 					return (SwFlyFrmFmt*)pFlyFmt;
/*?*/ 			}
/*?*/ 			else
/*?*/ 				return (SwFlyFrmFmt*)pFlyFmt;
/*?*/ 		}
/*N*/ 	}
/*N*/ 	return 0;
/*N*/ }

/*N*/ void SwDoc::SetFlyName( SwFlyFrmFmt& rFmt, const String& rName )
/*N*/ {
/*N*/ 	String sName( rName );
/*N*/ 	if( !rName.Len() || FindFlyByName( rName ) )
/*N*/ 	{
/*?*/ 		sal_uInt16 nTyp = STR_FRAME_DEFNAME;
/*?*/ 		const SwNodeIndex* pIdx = rFmt.GetCntnt().GetCntntIdx();
/*?*/ 		if( pIdx && pIdx->GetNode().GetNodes().IsDocNodes() )
/*?*/ 			switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
/*?*/ 			{
/*?*/ 			case ND_GRFNODE:	nTyp = STR_GRAPHIC_DEFNAME;	break;
/*?*/ 			case ND_OLENODE:	nTyp = STR_OBJECT_DEFNAME;	break;
/*?*/ 			}
/*?*/ 		sName = ::binfilter::lcl_GetUniqueFlyName( this, nTyp );
/*N*/ 	}
/*N*/ 	rFmt.SetName( sName, sal_True );
/*N*/ }

/*N*/ void SwDoc::SetAllUniqueFlyNames()
/*N*/ {
/*N*/ 	sal_uInt16 n, nFlyNum = 0, nGrfNum = 0, nOLENum = 0;
/*N*/
/*N*/ 	ResId nFrmId( STR_FRAME_DEFNAME, *pSwResMgr ),
/*N*/ 		  nGrfId( STR_GRAPHIC_DEFNAME, *pSwResMgr ),
/*N*/ 		  nOLEId( STR_OBJECT_DEFNAME, *pSwResMgr );
/*N*/ 	String sFlyNm( nFrmId );
/*N*/ 	String sGrfNm( nGrfId );
/*N*/ 	String sOLENm( nOLEId );
/*N*/
/*N*/ 	if( 255 < ( n = GetSpzFrmFmts()->Count() ))
/*N*/ 		n = 255;
/*N*/ 	SwSpzFrmFmts aArr( (sal_Int8)n, 10 );
/*N*/ 	SwFrmFmtPtr pFlyFmt;
/*N*/ 	sal_Bool bLoadedFlag = sal_True;			// noch etwas fuers Layout
/*N*/
/*N*/ 	for( n = GetSpzFrmFmts()->Count(); n; )
/*N*/ 	{
/*N*/ 		if( RES_FLYFRMFMT == (pFlyFmt = (*GetSpzFrmFmts())[ --n ])->Which() )
/*N*/ 		{
/*N*/ 			sal_uInt16 *pNum = 0;
/*N*/ 			xub_StrLen nLen;
/*N*/ 			const String& rNm = pFlyFmt->GetName();
/*N*/ 			if( rNm.Len() )
/*N*/ 			{
/*N*/ 				if( rNm.Match( sGrfNm ) == ( nLen = sGrfNm.Len() ))
/*?*/ 					pNum = &nGrfNum;
/*N*/ 				else if( rNm.Match( sFlyNm ) == ( nLen = sFlyNm.Len() ))
/*N*/ 					pNum = &nFlyNum;
/*N*/ 				else if( rNm.Match( sOLENm ) == ( nLen = sOLENm.Len() ))
/*?*/ 					pNum = &nOLENum;
/*N*/
/*N*/ 				if( pNum && *pNum < ( nLen = rNm.Copy( nLen ).ToInt32() ))
/*N*/ 					*pNum = nLen;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				// das wollen wir nachher setzen
/*?*/ 				aArr.Insert( pFlyFmt, aArr.Count() );
/*N*/
/*N*/ 		}
/*N*/ 		if( bLoadedFlag )
/*N*/ 		{
/*N*/ 			const SwFmtAnchor& rAnchor = pFlyFmt->GetAnchor();
/*N*/ 			if( ( FLY_PAGE == rAnchor.GetAnchorId() &&
/*N*/ 					rAnchor.GetCntntAnchor() ) ||
/*N*/ 				// oder werden DrawObjecte rel. zu irgendetwas ausgerichtet?
/*N*/ 				( RES_DRAWFRMFMT == pFlyFmt->Which() && (
/*N*/ 					SFX_ITEM_SET == pFlyFmt->GetItemState(
/*N*/ 										RES_VERT_ORIENT )||
/*N*/ 					SFX_ITEM_SET == pFlyFmt->GetItemState(
/*N*/ 										RES_HORI_ORIENT ))) )
/*N*/ 				bLoadedFlag = sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	const SwNodeIndex* pIdx;
/*N*/
/*N*/ 	for( n = aArr.Count(); n; )
/*?*/ 		if( 0 != ( pIdx = ( pFlyFmt = aArr[ --n ])->GetCntnt().GetCntntIdx() )
/*?*/ 			&& pIdx->GetNode().GetNodes().IsDocNodes() )
/*?*/ 		{
/*?*/ 			sal_uInt16 nNum;
/*?*/ 			String sNm;
/*?*/ 			switch( GetNodes()[ pIdx->GetIndex() + 1 ]->GetNodeType() )
/*?*/ 			{
/*?*/ 			case ND_GRFNODE:
/*?*/ 				sNm = sGrfNm;
/*?*/ 				nNum = ++nGrfNum;
/*?*/ 				break;
/*?*/ 			case ND_OLENODE:
/*?*/ 				sNm = sOLENm;
/*?*/ 				nNum = ++nOLENum;
/*?*/ 				break;
/*?*/ 			default:
/*?*/ 				sNm = sFlyNm;
/*?*/ 				nNum = ++nFlyNum;
/*?*/ 				break;
/*?*/ 			}
/*?*/ 			pFlyFmt->SetName( sNm += String::CreateFromInt32( nNum ));
/*?*/ 		}
/*N*/ 	aArr.Remove( 0, aArr.Count() );
/*N*/
/*N*/ 	if( GetFtnIdxs().Count() )
/*N*/ 	{
/*N*/ 		SwTxtFtn::SetUniqueSeqRefNo( *this );
/*N*/ 		SwNodeIndex aTmp( GetNodes() );
/*N*/ 		GetFtnIdxs().UpdateFtn( aTmp );
/*N*/ 	}
/*N*/
/*N*/ 	// neues Document und keine seitengebundenen Rahmen/DrawObjecte gefunden,
/*N*/ 	// die an einem Node verankert sind.
/*N*/ 	if( bLoadedFlag )
/*N*/ 		SetLoaded( sal_True );
/*N*/ }

/*N*/ sal_Bool SwDoc::IsInHeaderFooter( const SwNodeIndex& rIdx ) const
/*N*/ {
/*N*/ 	// gibt es ein Layout, dann ueber das laufen!!
/*N*/ 	//	(Das kann dann auch Fly in Fly in Kopfzeile !)
/*N*/ 	// MIB 9.2.98: Wird auch vom sw3io benutzt, um festzustellen, ob sich
/*N*/ 	// ein Redline-Objekt in einer Kopf- oder Fusszeile befindet. Da
/*N*/ 	// Redlines auch an Start- und Endnodes haengen, muss der Index nicht
/*N*/ 	// unbedingt der eines Content-Nodes sein.
/*N*/ 	SwNode* pNd = &rIdx.GetNode();
/*N*/ 	if( pNd->IsCntntNode() && pLayout )
/*N*/ 	{
/*N*/ 		SwFrm *pFrm = pNd->GetCntntNode()->GetFrm();
/*N*/ 		if( pFrm )
/*N*/ 		{
/*N*/ 			SwFrm *pUp = pFrm->GetUpper();
/*N*/ 			while ( pUp && !pUp->IsHeaderFrm() && !pUp->IsFooterFrm() )
/*N*/ 			{
/*N*/ 				if ( pUp->IsFlyFrm() )
/*N*/ 					pUp = ((SwFlyFrm*)pUp)->GetAnchor();
/*N*/ 				pUp = pUp->GetUpper();
/*N*/ 			}
/*N*/ 			if ( pUp )
/*N*/ 				return sal_True;
/*N*/
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/
/*N*/ 	const SwNode* pFlyNd = pNd->FindFlyStartNode();
/*N*/ 	while( pFlyNd )
/*N*/ 	{
/*N*/ 		// dann ueber den Anker nach oben "hangeln"
            sal_uInt16 n=0;
/*N*/ 		for( n = 0; n < GetSpzFrmFmts()->Count(); ++n )
/*N*/ 		{
/*N*/ 			const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[ n ];
/*N*/ 			const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
/*N*/ 			if( pIdx && pFlyNd == &pIdx->GetNode() )
/*N*/ 			{
/*N*/ 				const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
/*N*/ 				if( FLY_PAGE == rAnchor.GetAnchorId() ||
/*N*/ 					!rAnchor.GetCntntAnchor() )
/*N*/ 					return sal_False;
/*N*/
/*N*/ 				pNd = &rAnchor.GetCntntAnchor()->nNode.GetNode();
/*N*/ 				pFlyNd = pNd->FindFlyStartNode();
/*N*/ 				break;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( n >= GetSpzFrmFmts()->Count() )
/*N*/ 		{
/*N*/ 			ASSERT( bInReading, "Fly-Section aber kein Format gefunden" );
/*N*/ 			return sal_False;
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	return 0 != pNd->FindHeaderStartNode() ||
/*N*/ 			0 != pNd->FindFooterStartNode();
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
