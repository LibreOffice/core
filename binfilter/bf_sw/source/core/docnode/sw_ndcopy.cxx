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
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif



#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif
#ifndef _SWDDETBL_HXX
#include <swddetbl.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTCNCT_HXX
#include <fmtcnct.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
namespace binfilter {

// Struktur fuer das Mappen von alten und neuen Frame-Formaten an den
// Boxen und Lines einer Tabelle

/*N*/ struct _MapTblFrmFmt
/*N*/ {
/*N*/ 	const SwFrmFmt *pOld, *pNew;
/*N*/ 	_MapTblFrmFmt( const SwFrmFmt *pOldFmt, const SwFrmFmt*pNewFmt )
/*N*/ 		: pOld( pOldFmt ), pNew( pNewFmt )
/*N*/ 	{}
/*N*/ };

/*N*/ SV_DECL_VARARR( _MapTblFrmFmts, _MapTblFrmFmt, 0, 10 )//STRIP008 ;
/*N*/ SV_IMPL_VARARR( _MapTblFrmFmts, _MapTblFrmFmt );

/*N*/ SwCntntNode* SwTxtNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
/*N*/ {
/*N*/ 	// the Copy-Textnode is the Node with the Text, the Copy-Attrnode is the
/*N*/ 	// node with the collection and hard attributes. Normally ist the same
/*N*/ 	// node, but if insert a glossary without formatting, then the Attrnode
/*N*/ 	// is the prev node of the destionation position in dest. document.
/*N*/ 	SwTxtNode* pCpyTxtNd = (SwTxtNode*)this;
/*N*/ 	SwTxtNode* pCpyAttrNd = pCpyTxtNd;
/*N*/
/*N*/ 	// kopiere die Formate in das andere Dokument:
/*N*/ 	SwTxtFmtColl* pColl = 0;
/*N*/ 	if( pDoc->IsInsOnlyTextGlossary() )
/*N*/ 	{
/*?*/ 		SwNodeIndex aIdx( rIdx, -1 );
/*?*/ 		if( aIdx.GetNode().IsTxtNode() )
/*?*/ 		{
/*?*/ 			pCpyAttrNd = aIdx.GetNode().GetTxtNode();
/*?*/ 			pColl = &pCpyAttrNd->GetTxtColl()->GetNextTxtFmtColl();
/*?*/ 		}
/*N*/ 	}
/*N*/ 	if( !pColl )
/*N*/ 		pColl = pDoc->CopyTxtColl( *GetTxtColl() );
/*N*/
/*N*/ 	SwTxtNode* pTxtNd = pDoc->GetNodes().MakeTxtNode( rIdx, pColl );
/*N*/
/*N*/ 	// kopiere Attribute/Text
/*N*/ 	if( !pCpyAttrNd->GetpSwAttrSet() )
/*N*/ 		// wurde ein AttrSet fuer die Numerierung angelegt, so loesche diesen!
/*N*/ 		pTxtNd->ResetAllAttr();
/*N*/
/*N*/ 	// if Copy-Textnode unequal to Copy-Attrnode, then copy first
/*N*/ 	// the attributes into the new Node.
/*N*/ 	if( pCpyAttrNd != pCpyTxtNd )
/*N*/ 	{
/*?*/ 		pCpyAttrNd->CopyAttr( pTxtNd, 0, 0 );
/*?*/ 		if( pCpyAttrNd->GetpSwAttrSet() )
/*?*/ 		{
/*?*/ 			SwAttrSet aSet( *pCpyAttrNd->GetpSwAttrSet() );
/*?*/ 			aSet.ClearItem( RES_PAGEDESC );
/*?*/ 			aSet.ClearItem( RES_BREAK );
/*?*/ 			aSet.CopyToModify( *pTxtNd );
/*?*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 		// ??? reicht das ??? was ist mit PostIts/Feldern/FeldTypen ???
/*N*/ 	pCpyTxtNd->Copy( pTxtNd, SwIndex( pCpyTxtNd ), pCpyTxtNd->GetTxt().Len() );
/*N*/
/*N*/ 	if( pCpyAttrNd->GetNum() )
/*N*/ 		pTxtNd->UpdateNum( *pCpyAttrNd->GetNum() );
/*N*/
/*N*/ //FEATURE::CONDCOLL
/*N*/ 	if( RES_CONDTXTFMTCOLL == pColl->Which() )
/*N*/ 		pTxtNd->ChkCondColl();
/*N*/ //FEATURE::CONDCOLL
/*N*/
/*N*/ 	return pTxtNd;
/*N*/ }


/*N*/ BOOL lcl_SrchNew( const _MapTblFrmFmt& rMap, void * pPara )
/*N*/ {
/*N*/ 	if( rMap.pOld != *(const SwFrmFmt**)pPara )
/*N*/ 		return TRUE;
/*N*/ 	*((const SwFrmFmt**)pPara) = rMap.pNew;
/*N*/ 	return FALSE;		// abbrechen, Pointer gefunden
/*N*/ }


/*N*/ struct _CopyTable
/*N*/ {
/*N*/ 	SwDoc* pDoc;
/*N*/ 	ULONG nOldTblSttIdx;
/*N*/ 	_MapTblFrmFmts& rMapArr;
/*N*/ 	SwTableLine* pInsLine;
/*N*/ 	SwTableBox* pInsBox;
/*N*/ 	SwTableNode *pTblNd;
/*N*/ 	const SwTable *pOldTable;
/*N*/
/*N*/ 	_CopyTable( SwDoc* pDc, _MapTblFrmFmts& rArr, ULONG nOldStt,
/*N*/ 				SwTableNode& rTblNd, const SwTable* pOldTbl )
/*N*/ 		: pDoc(pDc), pTblNd(&rTblNd), nOldTblSttIdx(nOldStt),
/*N*/ 		rMapArr(rArr), pOldTable( pOldTbl ), pInsLine(0), pInsBox(0)
/*N*/ 	{}
/*N*/ };

/*N*/ BOOL lcl_CopyTblBox( const SwTableBox*& rpBox, void* pPara );

/*N*/ BOOL lcl_CopyTblLine( const SwTableLine*& rpLine, void* pPara );

/*N*/ BOOL lcl_CopyTblBox( const SwTableBox*& rpBox, void* pPara )
/*N*/ {
/*N*/ 	_CopyTable* pCT = (_CopyTable*)pPara;
/*N*/
/*N*/ 	SwTableBoxFmt* pBoxFmt = (SwTableBoxFmt*)rpBox->GetFrmFmt();
/*N*/ 	pCT->rMapArr.ForEach( lcl_SrchNew, &pBoxFmt );
/*N*/ 	if( pBoxFmt == rpBox->GetFrmFmt() )	// ein neues anlegen ??
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem;
/*N*/ 		if( SFX_ITEM_SET == pBoxFmt->GetItemState( RES_BOXATR_FORMULA, FALSE,
/*N*/ 			&pItem ) && ((SwTblBoxFormula*)pItem)->IsIntrnlName() )
/*N*/ 		{
/*?*/ 			((SwTblBoxFormula*)pItem)->PtrToBoxNm( pCT->pOldTable );
/*N*/ 		}
/*N*/
/*N*/ 		pBoxFmt = pCT->pDoc->MakeTableBoxFmt();
/*N*/ 		pBoxFmt->CopyAttrs( *rpBox->GetFrmFmt() );
/*N*/
/*N*/ 		if( rpBox->GetSttIdx() )
/*N*/ 		{
/*N*/ 			SvNumberFormatter* pN = pCT->pDoc->GetNumberFormatter( FALSE );
/*N*/ 			if( pN && pN->HasMergeFmtTbl() && SFX_ITEM_SET == pBoxFmt->
/*N*/ 				GetItemState( RES_BOXATR_FORMAT, FALSE, &pItem ) )
/*N*/ 			{
/*?*/ 				ULONG nOldIdx = ((SwTblBoxNumFormat*)pItem)->GetValue();
/*?*/ 				ULONG nNewIdx = pN->GetMergeFmtIndex( nOldIdx );
/*?*/ 				if( nNewIdx != nOldIdx )
/*?*/ 					pBoxFmt->SetAttr( SwTblBoxNumFormat( nNewIdx ));
/*?*/
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		pCT->rMapArr.Insert( _MapTblFrmFmt( rpBox->GetFrmFmt(), pBoxFmt ),
/*N*/ 								pCT->rMapArr.Count() );
/*N*/ 	}
/*N*/
/*N*/ 	USHORT nLines = rpBox->GetTabLines().Count();
/*N*/ 	SwTableBox* pNewBox;
/*N*/ 	if( nLines )
/*?*/ 		pNewBox = new SwTableBox( pBoxFmt, nLines, pCT->pInsLine );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		SwNodeIndex aNewIdx( *pCT->pTblNd,
/*N*/ 							rpBox->GetSttIdx() - pCT->nOldTblSttIdx );
/*N*/ 		ASSERT( aNewIdx.GetNode().IsStartNode(), "Index nicht auf einem StartNode" );
/*N*/ 		pNewBox = new SwTableBox( pBoxFmt, aNewIdx, pCT->pInsLine );
/*N*/ 	}
/*N*/
/*N*/ 	pCT->pInsLine->GetTabBoxes().C40_INSERT( SwTableBox, pNewBox,
/*N*/ 					pCT->pInsLine->GetTabBoxes().Count() );
/*N*/
/*N*/ 	if( nLines )
/*N*/ 	{
/*?*/ 		_CopyTable aPara( *pCT );
/*?*/ 		aPara.pInsBox = pNewBox;
/*?*/ 		((SwTableBox*)rpBox)->GetTabLines().ForEach( &lcl_CopyTblLine, &aPara );
/*N*/ 	}
/*N*/ 	else if( pNewBox->IsInHeadline( &pCT->pTblNd->GetTable() ))
/*N*/ 		// in der HeadLine sind die Absaetze mit BedingtenVorlage anzupassen
/*N*/ 		pNewBox->GetSttNd()->CheckSectionCondColl();
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ BOOL lcl_CopyTblLine( const SwTableLine*& rpLine, void* pPara )
/*N*/ {
/*N*/ 	_CopyTable* pCT = (_CopyTable*)pPara;
/*N*/ 	SwTableLineFmt* pLineFmt = (SwTableLineFmt*)rpLine->GetFrmFmt();
/*N*/ 	pCT->rMapArr.ForEach( lcl_SrchNew, &pLineFmt );
/*N*/ 	if( pLineFmt == rpLine->GetFrmFmt() )	// ein neues anlegen ??
/*N*/ 	{
/*N*/ 		pLineFmt = pCT->pDoc->MakeTableLineFmt();
/*N*/ 		pLineFmt->CopyAttrs( *rpLine->GetFrmFmt() );
/*N*/ 		pCT->rMapArr.Insert( _MapTblFrmFmt( rpLine->GetFrmFmt(), pLineFmt ),
/*N*/ 								pCT->rMapArr.Count());
/*N*/ 	}
/*N*/ 	SwTableLine* pNewLine = new SwTableLine( pLineFmt,
/*N*/ 							rpLine->GetTabBoxes().Count(), pCT->pInsBox );
/*N*/ 	// die neue Zeile in die Tabelle eintragen
/*N*/ 	if( pCT->pInsBox )
/*N*/ 	{
/*?*/ 		pCT->pInsBox->GetTabLines().C40_INSERT( SwTableLine, pNewLine,
/*?*/ 				pCT->pInsBox->GetTabLines().Count() );
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pCT->pTblNd->GetTable().GetTabLines().C40_INSERT( SwTableLine, pNewLine,
/*N*/ 				pCT->pTblNd->GetTable().GetTabLines().Count() );
/*N*/ 	}
/*N*/ 	pCT->pInsLine = pNewLine;
/*N*/ 	((SwTableLine*)rpLine)->GetTabBoxes().ForEach( &lcl_CopyTblBox, pCT );
/*N*/ 	return TRUE;
/*N*/ }

/*N*/ SwTableNode* SwTableNode::MakeCopy( SwDoc* pDoc, const SwNodeIndex& rIdx ) const
/*N*/ {
/*N*/ 	// in welchen Array steht ich denn Nodes, UndoNodes ??
/*N*/ 	SwNodes& rNds = (SwNodes&)GetNodes();
/*N*/
/*N*/ 	if( pDoc->IsIdxInTbl( rIdx ) )
/*N*/ 		// zur Zeit keine Tabelle in Tabelle kopieren unterstuetzen
/*N*/ 		// (sprich: Text + Tabelle + Text )
/*?*/ 		return 0;
/*N*/
/*N*/ 	{
/*N*/ 		// nicht in Fussnoten kopieren !!
/*
!! Mal ohne Frames
        SwCntntNode* pCNd = pDoc->GetNodes()[ rIdx ]->GetCntntNode();
        SwFrm* pFrm;
        if( (pCNd && 0 != ( pFrm = pCNd->GetFrm()))
                ? pFrm->FindFtnFrm()
                : rIdx < pDoc->GetNodes().EndOfInserts &&
                    pDoc->GetNodes()[pDoc->GetNodes().EndOfInserts]->StartOfSection()
                    < rIdx )
*/
/*N*/ 		if( rIdx < pDoc->GetNodes().GetEndOfInserts().GetIndex() &&
/*N*/ 			rIdx >= pDoc->GetNodes().GetEndOfInserts().StartOfSectionIndex() )
/*?*/ 			return 0;
/*N*/ 	}
/*N*/
/*N*/ 	// das TableFrmFmt kopieren
/*N*/ 	String sTblName( GetTable().GetFrmFmt()->GetName() );
/*N*/ 	if( !pDoc->IsCopyIsMove() )
/*N*/ 	{
/*N*/ 		const SwFrmFmts& rTblFmts = *pDoc->GetTblFrmFmts();
/*N*/ 		for( USHORT n = rTblFmts.Count(); n; )
/*N*/ 			if( rTblFmts[ --n ]->GetName() == sTblName )
/*N*/ 			{
/*N*/ 				sTblName = pDoc->GetUniqueTblName();
/*N*/ 				break;
/*N*/ 			}
/*N*/ 	}
/*N*/
/*N*/ 	SwFrmFmt* pTblFmt = pDoc->MakeTblFrmFmt( sTblName, pDoc->GetDfltFrmFmt() );
/*N*/ 	pTblFmt->CopyAttrs( *GetTable().GetFrmFmt() );
/*N*/ 	SwTableNode* pTblNd = new SwTableNode( rIdx );
/*N*/ 	SwEndNode* pEndNd = new SwEndNode( rIdx, *pTblNd );
/*N*/ 	SwNodeIndex aInsPos( *pEndNd );
/*N*/
/*N*/ 	SwTable& rTbl = (SwTable&)pTblNd->GetTable();
/*N*/ 	pTblFmt->Add( &rTbl );		// das Frame-Format setzen
/*N*/
/*N*/ 	rTbl.SetHeadlineRepeat( GetTable().IsHeadlineRepeat() );
/*N*/ 	rTbl.SetTblChgMode( GetTable().GetTblChgMode() );
/*N*/
/*N*/ 	SwDDEFieldType* pDDEType = 0;
/*N*/ 	if( IS_TYPE( SwDDETable, &GetTable() ))
/*N*/ 	{
/*?*/ 		// es wird eine DDE-Tabelle kopiert
/*?*/ 		// ist im neuen Dokument ueberhaupt der FeldTyp vorhanden ?
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pDDEType = ((SwDDETable&)GetTable()).GetDDEFldType();
/*N*/ 	}
/*N*/ 	// dann kopiere erstmal den Inhalt der Tabelle, die Zuordnung der
/*N*/ 	// Boxen/Lines und das anlegen der Frames erfolgt spaeter
/*N*/ 	SwNodeRange aRg( *this, +1, *EndOfSectionNode() );	// (wo stehe in denn nun ??)
/*N*/ 	rNds._Copy( aRg, aInsPos, FALSE );
/*N*/
/*N*/ 	// Sonderbehandlung fuer eine einzelne Box
/*N*/ 	if( 1 == GetTable().GetTabSortBoxes().Count() )
/*N*/ 	{
/*?*/ 		aRg.aStart.Assign( *pTblNd, 1 );
/*?*/ 		aRg.aEnd.Assign( *pTblNd->EndOfSectionNode() );
/*?*/ 		pDoc->GetNodes().SectionDown( &aRg, SwTableBoxStartNode );
/*N*/ 	}
/*N*/
/*N*/ 	// loesche alle Frames vom kopierten Bereich, diese werden beim
/*N*/ 	// erzeugen des TableFrames angelegt !
/*N*/ 	pTblNd->DelFrms();
/*N*/
/*N*/ 	_MapTblFrmFmts aMapArr;
/*N*/ 	_CopyTable aPara( pDoc, aMapArr, GetIndex(), *pTblNd, &GetTable() );
/*N*/
/*N*/ 	((SwTable&)GetTable()).GetTabLines().ForEach( &lcl_CopyTblLine, &aPara );
/*N*/
/*N*/ 	if( pDDEType && pDoc->GetRootFrm() )
/*?*/ 		pDDEType->IncRefCnt();
/*N*/
/*N*/ 	return pTblNd;
/*N*/ }



//  ----- Copy-Methode vom SwDoc ------

    // verhinder das Kopieren in Fly's, die im Bereich verankert sind.





// Kopieren eines Bereiches im oder in ein anderes Dokument !

/*N*/ BOOL SwDoc::Copy( SwPaM& rPam, SwPosition& rPos ) const
/*N*/ {
/*N*/ 	const SwPosition *pStt = rPam.Start(), *pEnd = rPam.End();
/*N*/ 	// kein Copy abfangen.
/*N*/ 	if( !rPam.HasMark() || *pStt >= *pEnd )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	SwDoc* pDoc = rPos.nNode.GetNode().GetDoc();
/*N*/
/*N*/ 	// verhinder das Kopieren in Fly's, die im Bereich verankert sind.
/*N*/ 	if( pDoc == this )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // Start-/EndNode noch korrigieren
/*N*/ 	}
/*N*/
/*N*/ 	SwPaM* pRedlineRange = 0;
/*N*/ 	if( pDoc->IsRedlineOn() ||
/*N*/ 		(!pDoc->IsIgnoreRedline() && pDoc->GetRedlineTbl().Count() ) )
/*N*/ 		pRedlineRange = new SwPaM( rPos );
/*N*/
/*N*/ 	SwRedlineMode eOld = pDoc->GetRedlineMode();
/*N*/
/*N*/ 	BOOL bRet = FALSE;
/*N*/
/*N*/ 	if( pDoc && pDoc != this )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	bRet = _Copy( rPam, rPos, TRUE, pRedlineRange );	// nur normales Kopieren
/*N*/ 	// Copy in sich selbst (ueber mehrere Nodes wird hier gesondert
/*N*/ 	// behandelt; in einem TextNode wird normal behandelt)
/*N*/ 	else if( ! ( *pStt <= rPos && rPos < *pEnd &&
/*N*/ 			( pStt->nNode != pEnd->nNode ||
/*N*/ 			  !pStt->nNode.GetNode().IsTxtNode() )) )
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	bRet = _Copy( rPam, rPos, TRUE, pRedlineRange );	// nur normales Kopieren
/*N*/
/*N*/ 	else
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ASSERT( this == pDoc, " falscher Copy-Zweig!" );
/*N*/ 	}
/*N*/
/*N*/ 	pDoc->SetRedlineMode_intern( eOld );
/*N*/ 	if( pRedlineRange )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( pDoc->IsRedlineOn() )
/*N*/ 	}
/*N*/
/*N*/ 	return bRet;
/*N*/ }

// Kopieren eines Bereiches im oder in ein anderes Dokument !
// Die Position darf nicht im Bereich liegen !!



//  ----- Copy-Methode vom SwDoc - "kopiere Fly's in Fly's" ------

/*N*/ void SwDoc::CopyWithFlyInFly( const SwNodeRange& rRg,
/*N*/ 							const SwNodeIndex& rInsPos, BOOL bMakeNewFrms,
/*N*/ 							BOOL bDelRedlines, BOOL bCopyFlyAtFly ) const
/*N*/ {
/*N*/ 	SwDoc* pDest = rInsPos.GetNode().GetDoc();
/*N*/
/*N*/ 	_SaveRedlEndPosForRestore aRedlRest( rInsPos );
/*N*/
/*N*/ 	SwNodeIndex aSavePos( rInsPos, -1 );
/*N*/ 	BOOL bEndIsEqualEndPos = rInsPos == rRg.aEnd;
/*N*/ 	GetNodes()._CopyNodes( rRg, rInsPos, bMakeNewFrms, TRUE );
/*N*/ 	aSavePos++;
/*N*/ 	if( bEndIsEqualEndPos )
/*N*/ 		((SwNodeIndex&)rRg.aEnd) = aSavePos;
/*N*/
/*N*/ 	aRedlRest.Restore();
/*N*/
/*N*/ #ifdef DBG_UTIL
/*N*/ 	{
/*N*/ 		//JP 17.06.99: Bug 66973 - check count only if the selection is in
/*N*/ 		//				the same (or no) section. Becaus not full selected
/*N*/ 		//				section are not copied.
/*N*/ 		const SwSectionNode* pSSectNd = rRg.aStart.GetNode().FindSectionNode();
/*N*/ 		SwNodeIndex aTmpI( rRg.aEnd, -1 );
/*N*/ 		const SwSectionNode* pESectNd = aTmpI.GetNode().FindSectionNode();
/*N*/ 		if( pSSectNd == pESectNd &&
/*N*/ 			!rRg.aStart.GetNode().IsSectionNode() &&
/*N*/ 			!aTmpI.GetNode().IsEndNode() )
/*N*/ 		{
/*N*/ 			ASSERT( rInsPos.GetIndex() - aSavePos.GetIndex() ==
/*N*/ 					rRg.aEnd.GetIndex() - rRg.aStart.GetIndex(),
/*N*/ 					"Es wurden zu wenig Nodes kopiert!" )
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/
/*N*/ 	_CopyFlyInFly( rRg, aSavePos, bCopyFlyAtFly );
/*N*/
/*N*/ 	SwNodeRange aCpyRange( aSavePos, rInsPos );
/*N*/
/*N*/ 	// dann kopiere noch alle Bookmarks
/*N*/ 	if( GetBookmarks().Count() )
/*N*/ 	{
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 SwPaM aRgTmp( rRg.aStart, rRg.aEnd );
/*N*/ 	}
/*N*/
/*N*/ 	if( bDelRedlines && ( REDLINE_DELETE_REDLINES & pDest->GetRedlineMode() ))
/*?*/ 	{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 	lcl_DeleteRedlines( rRg, aCpyRange );
/*N*/
/*N*/ 	pDest->GetNodes()._DelDummyNodes( aCpyRange );
/*N*/ }


/*N*/ void SwDoc::_CopyFlyInFly( const SwNodeRange& rRg, const SwNodeIndex& rSttIdx,
/*N*/ 							BOOL bCopyFlyAtFly ) const
/*N*/ {
/*N*/ 	// Bug 22727: suche erst mal alle Flys zusammen, sortiere sie entsprechend
/*N*/ 	//			  ihrer Ordnungsnummer und kopiere sie erst dann. Damit wird
/*N*/ 	//			  die Ordnungsnummer (wird nur im DrawModel verwaltet)
/*N*/ 	//			  beibehalten.
/*N*/ 	SwDoc* pDest = rSttIdx.GetNode().GetDoc();
/*N*/ 	_ZSortFlys aArr;
/*N*/ 	USHORT nArrLen = GetSpzFrmFmts()->Count();
        USHORT n=0;
/*N*/ 	for( n = 0; n < nArrLen; ++n )
/*N*/ 	{
/*N*/ 		const SwFrmFmt* pFmt = (*GetSpzFrmFmts())[n];
/*N*/ 		const SwFmtAnchor* pAnchor = &pFmt->GetAnchor();
/*N*/ 		const SwPosition* pAPos;
/*N*/ 		if ( ( pAnchor->GetAnchorId() == FLY_AT_CNTNT ||
/*N*/ 			   pAnchor->GetAnchorId() == FLY_AT_FLY ||
/*N*/ 			   pAnchor->GetAnchorId() == FLY_AUTO_CNTNT ) &&
/*N*/ 			 0 != ( pAPos = pAnchor->GetCntntAnchor()) &&
/*N*/ 			 (( bCopyFlyAtFly && FLY_AT_FLY == pAnchor->GetAnchorId() )
/*N*/ 					? rRg.aStart <= pAPos->nNode.GetIndex() + 1
/*N*/ 					: ( IsRedlineMove()
/*N*/ 							? rRg.aStart < pAPos->nNode
/*N*/ 							: rRg.aStart <= pAPos->nNode )) &&
/*N*/ 			 pAPos->nNode < rRg.aEnd )
/*N*/ 		{
/*N*/ 			aArr.Insert( _ZSortFly( pFmt, pAnchor, nArrLen + aArr.Count() ));
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	//Alle kopierten (also die neu erzeugten) Rahmen in ein weiteres Array
/*N*/ 	//stopfen. Dort sizten sie passend zu den Originalen, damit hinterher
/*N*/ 	//die Chains entsprechend aufgebaut werden koennen.
/*N*/ 	SvPtrarr aNewArr( 10, 10 );
/*N*/
/*N*/ 	for( n = 0; n < aArr.Count(); ++n )
/*N*/ 	{
/*N*/ 		// neuen Anker anlegen
/*N*/ 		const _ZSortFly& rZSortFly = aArr[ n ];
/*N*/ 		SwFmtAnchor aAnchor( *rZSortFly.GetAnchor() );
/*N*/ 		SwPosition *pNewPos = (SwPosition*)aAnchor.GetCntntAnchor();
/*N*/ 		long nOffset = pNewPos->nNode.GetIndex() -
/*N*/ 							rRg.aStart.GetIndex();
/*N*/ 		SwNodeIndex aIdx( rSttIdx, nOffset );
/*N*/ 		pNewPos->nNode = aIdx;
/*N*/ 		// die am Zeichen Flys wieder ans das vorgegebene Zeichen setzen
/*N*/ 		if( FLY_AUTO_CNTNT == aAnchor.GetAnchorId() &&
/*N*/ 			aIdx.GetNode().IsTxtNode() )
/*?*/ 			pNewPos->nContent.Assign( (SwTxtNode*)&aIdx.GetNode(),
/*?*/ 										pNewPos->nContent.GetIndex() );
/*N*/ 		else
/*N*/ 			pNewPos->nContent.Assign( 0, 0 );
/*N*/
/*N*/ 		// ueberpruefe Rekursion: Inhalt in "seinen eigenen" Frame
/*N*/ 		// kopieren. Dann nicht kopieren
/*N*/ 		FASTBOOL bMakeCpy = TRUE;
/*N*/ 		if( pDest == this )
/*N*/ 		{
/*N*/ 			const SwFmtCntnt& rCntnt = rZSortFly.GetFmt()->GetCntnt();
/*N*/ 			const SwStartNode* pSNd;
/*N*/ 			if( rCntnt.GetCntntIdx() &&
/*N*/ 				0 != ( pSNd = rCntnt.GetCntntIdx()->GetNode().GetStartNode() ) &&
/*N*/ 				pSNd->GetIndex() < rSttIdx.GetIndex() &&
/*N*/ 				rSttIdx.GetIndex() < pSNd->EndOfSectionIndex() )
/*N*/ 			{
/*?*/ 				bMakeCpy = FALSE;
/*?*/ 				aArr.Remove( n, 1 );
/*?*/ 				--n;
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 		// Format kopieren und den neuen Anker setzen
/*N*/ 		if( bMakeCpy )
/*N*/ 			aNewArr.Insert( pDest->CopyLayoutFmt( *rZSortFly.GetFmt(),
/*N*/ 						aAnchor, FALSE, TRUE/*FALSE*/ ), aNewArr.Count() );
/*N*/ 	}
/*N*/
/*N*/ 	//Alle chains, die im Original vorhanden sind, soweit wie moeglich wieder
/*N*/ 	//aufbauen.
/*N*/ 	ASSERT( aArr.Count() == aNewArr.Count(), "Missing new Flys" );
/*N*/ 	if ( aArr.Count() == aNewArr.Count() )
/*N*/ 	{
/*N*/ 		for ( n = 0; n < aArr.Count(); ++n )
/*N*/ 		{
/*N*/ 			const SwFrmFmt *pFmt = aArr[n].GetFmt();
/*N*/ 			const SwFmtChain &rChain = pFmt->GetChain();
/*N*/ 			int nCnt = 0 != rChain.GetPrev();
/*N*/ 			nCnt += rChain.GetNext() ? 1: 0;
/*N*/ 			for ( USHORT k = 0; nCnt && k < aArr.Count(); ++k )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 const _ZSortFly &rTmp = aArr[k];
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }




}
