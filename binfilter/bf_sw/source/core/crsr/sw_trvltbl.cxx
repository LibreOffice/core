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


#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>		//EndAllAction gibts nur an der EditShell
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SHELLRES_HXX
#include <shellres.hxx>
#endif
#ifndef _CELLFRM_HXX //autogen
#include <cellfrm.hxx>
#endif
namespace binfilter {

// setze Crsr in die naechsten/vorherigen Celle












// suche die naechste nicht geschuetzte Zelle innerhalb der Tabelle
// Parameter:
//		rIdx 	- steht auf dem TabellenNode
// 		SwDoc	-
// 	return:
//		0 	- Idx steht auf/in einer nicht geschuetzten Zelle
// 		!0	- Node hinter der Tabelle



// suche die vorherige nicht geschuetzte Zelle innerhalb der Tabelle
// Parameter:
//		rIdx 	- steht auf dem EndNode der Tabelle
// 		SwDoc	-
// 	return:
//		0 	- Idx steht auf/in einer nicht geschuetzten Zelle
// 		!0	- Node hinter der Tabelle





/*N*/ FASTBOOL GotoPrevTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
/*N*/ 						FASTBOOL bInReadOnly )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL GotoNextTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
/*N*/ 						FASTBOOL bInReadOnly )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ FASTBOOL GotoCurrTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
/*N*/ 						FASTBOOL bInReadOnly )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return TRUE;
/*N*/ }


/*N*/ FASTBOOL SwCursor::MoveTable( SwWhichTable fnWhichTbl, SwPosTable fnPosTbl )
/*N*/ {
/*N*/ 	FASTBOOL bRet = FALSE;DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	return bRet;
/*N*/ }


/*N*/ FASTBOOL SwCrsrShell::CheckTblBoxCntnt( const SwPosition* pPos )
/*N*/ {
/*N*/ 	if( !pBoxIdx || !pBoxPtr || IsSelTblCells() || !IsAutoUpdateCells() )
/*N*/ 		return FALSE;

    // ueberpruefe, ob der Box Inhalt mit dem angegebenen Format der Box
    // ueber einstimmt. Wenn nicht, setze neu
/*?*/ 	SwTableBox* pChkBox = 0;
/*?*/ 	SwStartNode* pSttNd;
/*?*/ 	if( !pPos )
/*?*/ 	{
/*?*/ 		// gesicherte Position heraus holen.
/*?*/ 		if( pBoxIdx && pBoxPtr &&
/*?*/ 			0 != ( pSttNd = pBoxIdx->GetNode().GetStartNode() ) &&
/*?*/ 			SwTableBoxStartNode == pSttNd->GetStartNodeType() &&
/*?*/ 			pBoxPtr == pSttNd->FindTableNode()->GetTable().
/*?*/ 						GetTblBox( pBoxIdx->GetIndex() ) )
/*?*/ 			pChkBox = pBoxPtr;
/*?*/ 	}
/*?*/ 	else if( 0 != ( pSttNd = pPos->nNode.GetNode().
/*?*/ 								FindSttNodeByType( SwTableBoxStartNode )) )
/*?*/ 	{
/*?*/ 		pChkBox = pSttNd->FindTableNode()->GetTable().GetTblBox( pSttNd->GetIndex() );
/*?*/ 	}
/*?*/ 
/*?*/ 
/*?*/ 	// Box mehr als 1 Absatz?
/*?*/ 	if( pChkBox && pSttNd->GetIndex() + 2 != pSttNd->EndOfSectionIndex() )
/*?*/ 		pChkBox = 0;
/*?*/ 
/*?*/ 	// jetzt sollten wir mal die Pointer zerstoeren, bevor eine erneute
/*?*/ 	// Actionklammerung kommt.
/*?*/ 	if( !pPos && !pChkBox )
/*?*/ 		ClearTblBoxCntnt();
/*?*/ 
/*?*/ 	// liegt der Cursor nicht mehr in dem Bereich ?
/*?*/ 	if( pChkBox && !pPos &&
/*?*/ 		( pCurCrsr->HasMark() || pCurCrsr->GetNext() != pCurCrsr ||
/*?*/ 		  pSttNd->GetIndex() + 1 == pCurCrsr->GetPoint()->nNode.GetIndex() ))
/*?*/ 		pChkBox = 0;
/*?*/ 
/*?*/ 	//JP 12.01.99: hat sich der Inhalt der Box ueberhaupt veraendert?
/*?*/ 	// Ist wichtig, wenn z.B. Undo nicht den richtigen Inhalt wieder
/*?*/ 	// herstellen konnte.
/*?*/ 	if( pChkBox )
/*?*/ 	{
/*?*/ 		const SwTxtNode* pNd = GetDoc()->GetNodes()[
/*?*/ 									pSttNd->GetIndex() + 1 ]->GetTxtNode();
/*?*/ 		if( !pNd ||
/*?*/ 			( pNd->GetTxt() == ViewShell::GetShellRes()->aCalc_Error &&
/*?*/ 			  SFX_ITEM_SET == pChkBox->GetFrmFmt()->
/*?*/ 							GetItemState( RES_BOXATR_FORMULA )) )
/*?*/ 			pChkBox = 0;
/*?*/ 	}
/*?*/ 
/*?*/ 	if( pChkBox )
/*?*/ 	{
/*?*/ 		// jetzt sollten wir mal die Pointer zerstoeren, bevor ein weiterer
/*?*/ 		// aufruf kommt.
/*?*/ 		DBG_BF_ASSERT(0, "STRIP"); //STRIP001 ClearTblBoxCntnt();
/*?*/ 	}
/*?*/ 
/*?*/ 	return 0 != pChkBox;
/*N*/ }


/*N*/ void SwCrsrShell::ClearTblBoxCntnt()
/*N*/ {
/*N*/ 	delete pBoxIdx, pBoxIdx = 0;
/*N*/ 	pBoxPtr = 0;
/*N*/ }

/*N*/ FASTBOOL SwCrsrShell::EndAllTblBoxEdit()
/*N*/ {
/*N*/ 	FASTBOOL bRet = FALSE;
/*N*/ 	ViewShell *pSh = this;
/*N*/ 	do {
/*N*/ 		if( pSh->IsA( TYPE( SwCrsrShell ) ) )
/*N*/ 			bRet |= ((SwCrsrShell*)pSh)->CheckTblBoxCntnt(
/*N*/ 						((SwCrsrShell*)pSh)->pCurCrsr->GetPoint() );
/*N*/ 
/*N*/ 	} while( this != (pSh = (ViewShell *)pSh->GetNext()) );
/*N*/ 	return bRet;
/*N*/ }




}
