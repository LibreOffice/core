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

#include <ftnidx.hxx>
#include <rootfrm.hxx>
#include <txtftn.hxx>
#include <fmtftn.hxx>
#include <pam.hxx>
#include <pagedesc.hxx>
#include <charfmt.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <ftninfo.hxx>
#include <hints.hxx>
namespace binfilter {


/*********************** SwFtnInfo ***************************/

/*N*/ SwEndNoteInfo& SwEndNoteInfo::operator=(const SwEndNoteInfo& rInfo)
/*N*/ {
/*N*/ 	if( rInfo.GetFtnTxtColl() )
/*?*/ 		rInfo.GetFtnTxtColl()->Add(this);
/*N*/ 	else if ( pRegisteredIn)
/*?*/ 		pRegisteredIn->Remove(this);

/*N*/ 	if ( rInfo.aPageDescDep.GetRegisteredIn() )
/*N*/ 		((SwModify*)rInfo.aPageDescDep.GetRegisteredIn())->Add( &aPageDescDep );
/*N*/ 	else if ( aPageDescDep.GetRegisteredIn() )
/*?*/ 		((SwModify*)aPageDescDep.GetRegisteredIn())->Remove( &aPageDescDep );

/*N*/ 	if ( rInfo.aCharFmtDep.GetRegisteredIn() )
/*N*/ 		((SwModify*)rInfo.aCharFmtDep.GetRegisteredIn())->Add( &aCharFmtDep );
/*N*/ 	else if ( aCharFmtDep.GetRegisteredIn() )
/*?*/ 		((SwModify*)aCharFmtDep.GetRegisteredIn())->Remove( &aCharFmtDep );

/*N*/ 	if ( rInfo.aAnchorCharFmtDep.GetRegisteredIn() )
/*N*/ 		((SwModify*)rInfo.aAnchorCharFmtDep.GetRegisteredIn())->Add(
/*N*/ 													&aAnchorCharFmtDep );
/*N*/ 	else if( aAnchorCharFmtDep.GetRegisteredIn() )
/*N*/ 		((SwModify*)aAnchorCharFmtDep.GetRegisteredIn())->Remove(
/*N*/ 													&aAnchorCharFmtDep );
/*N*/
/*N*/ 	aFmt = rInfo.aFmt;
/*N*/ 	nFtnOffset = rInfo.nFtnOffset;
/*N*/ 	bEndNote = rInfo.bEndNote;
/*N*/ 	sPrefix = rInfo.sPrefix;
/*N*/ 	sSuffix = rInfo.sSuffix;
/*N*/ 	return *this;
/*N*/ }


/*N*/ BOOL SwEndNoteInfo::operator==( const SwEndNoteInfo& rInfo ) const
/*N*/ {
/*N*/ 	return  aPageDescDep.GetRegisteredIn() ==
/*N*/ 								rInfo.aPageDescDep.GetRegisteredIn() &&
/*N*/ 			aCharFmtDep.GetRegisteredIn() ==
/*N*/ 								rInfo.aCharFmtDep.GetRegisteredIn() &&
/*N*/ 			aAnchorCharFmtDep.GetRegisteredIn() ==
/*N*/ 								rInfo.aAnchorCharFmtDep.GetRegisteredIn() &&
/*N*/ 			GetFtnTxtColl() == rInfo.GetFtnTxtColl() &&
/*N*/ 			aFmt.GetNumberingType() == rInfo.aFmt.GetNumberingType() &&
/*N*/ 			nFtnOffset == rInfo.nFtnOffset &&
/*N*/ 			bEndNote == rInfo.bEndNote &&
/*N*/ 			sPrefix == rInfo.sPrefix &&
/*N*/ 			sSuffix == rInfo.sSuffix;
/*N*/ }


/*N*/ SwEndNoteInfo::SwEndNoteInfo(const SwEndNoteInfo& rInfo) :
/*N*/ 	SwClient( rInfo.GetFtnTxtColl() ),
/*N*/ 	aPageDescDep( this, 0 ),
/*N*/ 	aCharFmtDep( this, 0 ),
/*N*/ 	aAnchorCharFmtDep( this, 0 ),
/*N*/ 	aFmt( rInfo.aFmt ),
/*N*/ 	nFtnOffset( rInfo.nFtnOffset ),
/*N*/ 	sPrefix( rInfo.sPrefix ),
/*N*/ 	sSuffix( rInfo.sSuffix ),
/*N*/ 	bEndNote( TRUE )
/*N*/ {
/*N*/ 	if( rInfo.GetPageDescDep()->GetRegisteredIn() )
/*N*/ 		((SwModify*)rInfo.GetPageDescDep()->GetRegisteredIn())->Add( &aPageDescDep );
/*N*/
/*N*/ 	if( rInfo.aCharFmtDep.GetRegisteredIn() )
/*N*/ 		((SwModify*)rInfo.aCharFmtDep.GetRegisteredIn())->Add( &aCharFmtDep );
/*N*/
/*N*/ 	if( rInfo.aAnchorCharFmtDep.GetRegisteredIn() )
/*N*/ 		((SwModify*)rInfo.aAnchorCharFmtDep.GetRegisteredIn())->Add(
/*?*/ 				&aAnchorCharFmtDep );
/*N*/ }

/*N*/ SwEndNoteInfo::SwEndNoteInfo(SwTxtFmtColl *pFmt) :
/*N*/ 	SwClient(pFmt),
/*N*/ 	aPageDescDep( this, 0 ),
/*N*/ 	aCharFmtDep( this, 0 ),
/*N*/ 	aAnchorCharFmtDep( this, 0 ),
/*N*/ 	nFtnOffset( 0 ),
/*N*/ 	bEndNote( TRUE )
/*N*/ {
/*N*/ 	aFmt.SetNumberingType(SVX_NUM_ROMAN_LOWER);
/*N*/ }

/*N*/ SwPageDesc *SwEndNoteInfo::GetPageDesc( SwDoc &rDoc ) const
/*N*/ {
/*N*/ 	if ( !aPageDescDep.GetRegisteredIn() )
/*N*/ 	{
/*N*/ 		SwPageDesc *pDesc = rDoc.GetPageDescFromPool( bEndNote
/*N*/ 											? RES_POOLPAGE_ENDNOTE
/*N*/ 											: RES_POOLPAGE_FOOTNOTE );
/*N*/ 		pDesc->Add( &((SwClient&)aPageDescDep) );
/*N*/ 	}
/*N*/ 	return (SwPageDesc*)aPageDescDep.GetRegisteredIn();
/*N*/ }

/*N*/ void SwEndNoteInfo::ChgPageDesc( SwPageDesc *pDesc )
/*N*/ {
/*N*/ 	pDesc->Add( &((SwClient&)aPageDescDep) );
/*N*/ }

/*N*/ void SwEndNoteInfo::SetFtnTxtColl(SwTxtFmtColl& rFmt)
/*N*/ {
/*N*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 rFmt.Add(this);
/*N*/ }

/*N*/ SwCharFmt* SwEndNoteInfo::GetCharFmt(SwDoc &rDoc) const
/*N*/ {
/*N*/ 	if ( !aCharFmtDep.GetRegisteredIn() )
/*N*/ 	{
/*N*/ 		SwCharFmt* pFmt = rDoc.GetCharFmtFromPool( bEndNote
/*N*/ 											? RES_POOLCHR_ENDNOTE
/*N*/ 											: RES_POOLCHR_FOOTNOTE );
/*N*/ 		pFmt->Add( &((SwClient&)aCharFmtDep) );
/*N*/ 	}
/*N*/ 	return (SwCharFmt*)aCharFmtDep.GetRegisteredIn();
/*N*/ }

/*N*/ void SwEndNoteInfo::SetCharFmt( SwCharFmt* pChFmt )
/*N*/ {
/*N*/ 	DBG_ASSERT(pChFmt, "kein CharFmt?");
/*N*/ 	pChFmt->Add( &((SwClient&)aCharFmtDep) );
/*N*/ }

/*N*/ SwCharFmt* SwEndNoteInfo::GetAnchorCharFmt(SwDoc &rDoc) const
/*N*/ {
/*N*/ 	if( !aAnchorCharFmtDep.GetRegisteredIn() )
/*N*/ 	{
/*N*/ 		SwCharFmt* pFmt = rDoc.GetCharFmtFromPool( bEndNote
/*N*/ 											? RES_POOLCHR_ENDNOTE_ANCHOR
/*N*/ 											: RES_POOLCHR_FOOTNOTE_ANCHOR );
/*N*/ 		pFmt->Add( &((SwClient&)aAnchorCharFmtDep) );
/*N*/ 	}
/*N*/ 	return (SwCharFmt*)aAnchorCharFmtDep.GetRegisteredIn();
/*N*/ }

/*N*/ void SwEndNoteInfo::SetAnchorCharFmt( SwCharFmt* pChFmt )
/*N*/ {
/*N*/ 	DBG_ASSERT(pChFmt, "kein CharFmt?");
/*N*/ 	pChFmt->Add( &((SwClient&)aAnchorCharFmtDep) );
/*N*/ }

/*N*/ void SwEndNoteInfo::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
/*N*/ {
/*N*/ 	USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0 ;
/*N*/
/*N*/ 	if( RES_ATTRSET_CHG == nWhich ||
/*N*/ 		RES_FMT_CHG == nWhich )
/*N*/ 	{
/*N*/ 		SwDoc* pDoc;
/*N*/ 		if( aCharFmtDep.GetRegisteredIn() )
/*N*/ 			pDoc = ((SwCharFmt*)aCharFmtDep.GetRegisteredIn())->GetDoc();
/*N*/ 		else
/*?*/ 			pDoc = ((SwCharFmt*)aAnchorCharFmtDep.GetRegisteredIn())->GetDoc();
/*N*/ 		SwFtnIdxs& rFtnIdxs = pDoc->GetFtnIdxs();
/*N*/ 		for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
/*N*/ 		{
/*?*/ 			SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
/*?*/ 			const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 			if ( rFtn.IsEndNote() == bEndNote)
/*?*/ 				pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
/*N*/ 		}
/*N*/ 	}
/*N*/ 	else
/*?*/ 		SwClient::Modify( pOld, pNew );
/*N*/ }

/*N*/ SwFtnInfo& SwFtnInfo::operator=(const SwFtnInfo& rInfo)
/*N*/ {
/*N*/ 	SwEndNoteInfo::operator=(rInfo);
/*N*/ 	aQuoVadis =  rInfo.aQuoVadis;
/*N*/ 	aErgoSum = rInfo.aErgoSum;
/*N*/ 	ePos = rInfo.ePos;
/*N*/ 	eNum = rInfo.eNum;
/*N*/ 	return *this;
/*N*/ }


/*N*/ BOOL SwFtnInfo::operator==( const SwFtnInfo& rInfo ) const
/*N*/ {
/*N*/ 	return	ePos == rInfo.ePos &&
/*N*/ 			eNum == rInfo.eNum &&
/*N*/ 			SwEndNoteInfo::operator==(rInfo) &&
/*N*/ 			aQuoVadis == rInfo.aQuoVadis &&
/*N*/ 			aErgoSum == rInfo.aErgoSum;
/*N*/ }


/*N*/ SwFtnInfo::SwFtnInfo(const SwFtnInfo& rInfo) :
/*N*/ 	SwEndNoteInfo( rInfo ),
/*N*/ 	aQuoVadis( rInfo.aQuoVadis ),
/*N*/ 	aErgoSum( rInfo.aErgoSum ),
/*N*/ 	ePos( rInfo.ePos ),
/*N*/ 	eNum( rInfo.eNum )
/*N*/ {
/*N*/ 	bEndNote = FALSE;
/*N*/ }

/*N*/ SwFtnInfo::SwFtnInfo(SwTxtFmtColl *pFmt) :
/*N*/ 	SwEndNoteInfo( pFmt ),
/*N*/ 	eNum( FTNNUM_DOC ),
/*N*/ 	ePos( FTNPOS_PAGE )
/*N*/ {
/*N*/ 	aFmt.SetNumberingType(SVX_NUM_ARABIC);
/*N*/ 	bEndNote = FALSE;
/*N*/ }

/*********************** SwDoc ***************************/


/*N*/ void SwDoc::SetFtnInfo(const SwFtnInfo& rInfo)
/*N*/ {
/*N*/ 	if( !(GetFtnInfo() == rInfo) )
/*N*/ 	{
/*N*/ 		const SwFtnInfo &rOld = GetFtnInfo();
/*N*/
/*N*/
/*N*/ 		FASTBOOL bPageNum = rInfo.eNum == FTNNUM_PAGE &&
/*N*/ 							rOld.eNum != FTNNUM_PAGE;
/*N*/ 		FASTBOOL bFtnPos  = rInfo.ePos != rOld.ePos;
/*N*/ 		FASTBOOL bFtnDesc = rOld.ePos == FTNPOS_CHAPTER &&
/*N*/ 							rInfo.GetPageDesc( *this ) != rOld.GetPageDesc( *this );
/*N*/ 		FASTBOOL bExtra   = rInfo.aQuoVadis != rOld.aQuoVadis ||
/*N*/ 							rInfo.aErgoSum != rOld.aErgoSum ||
/*N*/ 							rInfo.aFmt.GetNumberingType() != rOld.aFmt.GetNumberingType() ||
/*N*/ 							rInfo.GetPrefix() != rOld.GetPrefix() ||
/*N*/ 							rInfo.GetSuffix() != rOld.GetSuffix();
/*N*/ 		SwCharFmt *pOldChrFmt = rOld.GetCharFmt( *this ),
/*N*/ 				  *pNewChrFmt = rInfo.GetCharFmt( *this );
/*N*/ 		FASTBOOL bFtnChrFmts = pOldChrFmt != pNewChrFmt;
/*N*/
/*N*/ 		*pFtnInfo = rInfo;
/*N*/
/*N*/ 		if ( GetRootFrm() )
/*N*/ 		{
/*?*/ 			if ( bFtnPos )
/*?*/ 				GetRootFrm()->RemoveFtns();
/*?*/ 			else
/*?*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 GetRootFrm()->UpdateFtnNums();
/*?*/ 			}
/*N*/ 		}
/*N*/ 		if( FTNNUM_PAGE != rInfo.eNum )
/*N*/ 			GetFtnIdxs().UpdateAllFtn();
/*N*/ 		else if( bFtnChrFmts )
/*N*/ 		{
/*?*/ 			SwFmtChg aOld( pOldChrFmt );
/*?*/ 			SwFmtChg aNew( pNewChrFmt );
/*?*/ 			pFtnInfo->Modify( &aOld, &aNew );
/*?*/ 		}
/*N*/
/*N*/ 		UpdateRefFlds();
/*N*/
/*N*/ 		SetModified();
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::SetEndNoteInfo(const SwEndNoteInfo& rInfo)
/*N*/ {
/*N*/ 	if( !(GetEndNoteInfo() == rInfo) )
/*N*/ 	{
/*N*/ 		FASTBOOL bNumChg  = rInfo.nFtnOffset != GetEndNoteInfo().nFtnOffset;
/*N*/ 		FASTBOOL bExtra   = !bNumChg &&
/*N*/ 							rInfo.aFmt.GetNumberingType() != GetEndNoteInfo().aFmt.GetNumberingType()||
/*N*/ 							rInfo.GetPrefix() != GetEndNoteInfo().GetPrefix() ||
/*N*/ 							rInfo.GetSuffix() != GetEndNoteInfo().GetSuffix();
/*N*/ 		FASTBOOL bFtnDesc = rInfo.GetPageDesc( *this ) !=
/*N*/ 							GetEndNoteInfo().GetPageDesc( *this );
/*N*/ 		SwCharFmt *pOldChrFmt = GetEndNoteInfo().GetCharFmt( *this ),
/*N*/ 				  *pNewChrFmt = rInfo.GetCharFmt( *this );
/*N*/ 		FASTBOOL bFtnChrFmts = pOldChrFmt != pNewChrFmt;
/*N*/
/*N*/ 		*pEndNoteInfo = rInfo;
/*N*/
/*N*/ 		if ( GetRootFrm() )
/*N*/ 		{
/*?*/ 			if ( bFtnDesc )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 	GetRootFrm()->CheckFtnPageDescs( TRUE );
/*?*/ 			if ( bExtra )
/*?*/ 			{
/*?*/ 				//Fuer die Benachrichtung bezueglich ErgoSum usw. sparen wir uns
/*?*/ 				//extra-Code und nutzen die vorhandenen Wege.
/*?*/ 				SwFtnIdxs& rFtnIdxs = GetFtnIdxs();
/*?*/ 				for( USHORT nPos = 0; nPos < rFtnIdxs.Count(); ++nPos )
/*?*/ 				{
/*?*/ 					SwTxtFtn *pTxtFtn = rFtnIdxs[ nPos ];
/*?*/ 					const SwFmtFtn &rFtn = pTxtFtn->GetFtn();
/*?*/ 					if ( rFtn.IsEndNote() )
/*?*/ 						pTxtFtn->SetNumber( rFtn.GetNumber(), &rFtn.GetNumStr());
/*?*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/ 		if( bNumChg )
/*?*/ 			GetFtnIdxs().UpdateAllFtn();
/*N*/ 		else if( bFtnChrFmts )
/*N*/ 		{
/*?*/ 			SwFmtChg aOld( pOldChrFmt );
/*?*/ 			SwFmtChg aNew( pNewChrFmt );
/*?*/ 			pEndNoteInfo->Modify( &aOld, &aNew );
/*N*/ 		}
/*N*/
/*N*/ 		UpdateRefFlds();
/*N*/ 		SetModified();
/*N*/ 	}
/*N*/ }


/*N*/ BOOL SwDoc::SetCurFtn( const SwPaM& rPam, const String& rNumStr,
/*N*/ 						USHORT nNumber, BOOL bIsEndNote )
/*N*/ {
/*N*/ 	SwFtnIdxs& rFtnArr = GetFtnIdxs();
/*N*/
/*N*/ 	const SwPosition* pStt = rPam.Start(), *pEnd = rPam.End();
/*N*/ 	const ULONG nSttNd = pStt->nNode.GetIndex();
/*N*/ 	const xub_StrLen nSttCnt = pStt->nContent.GetIndex();
/*N*/ 	const ULONG nEndNd = pEnd->nNode.GetIndex();
/*N*/ 	const xub_StrLen nEndCnt = pEnd->nContent.GetIndex();
/*N*/
/*N*/ 	USHORT nPos;
/*N*/ 	rFtnArr.SeekEntry( pStt->nNode, &nPos );
/*N*/
/*N*/ 	SwTxtFtn* pTxtFtn;
/*N*/ 	ULONG nIdx;
/*N*/ 	BOOL bChg = FALSE;
/*N*/ 	BOOL bTypeChgd = FALSE;
/*N*/ 	USHORT n = nPos;		// sichern
/*N*/ 	while( nPos < rFtnArr.Count() &&
/*N*/ 			(( nIdx = _SwTxtFtn_GetIndex((pTxtFtn = rFtnArr[ nPos++ ] )))
/*N*/ 				< nEndNd || ( nIdx == nEndNd &&
/*N*/ 				nEndCnt >= *pTxtFtn->GetStart() )) )
/*N*/ 		if( nIdx > nSttNd || ( nIdx == nSttNd &&
/*N*/ 				nSttCnt <= *pTxtFtn->GetStart() ) )
/*N*/ 		{
/*N*/ 			const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
/*N*/ 			if( /*rFtn.GetNumber() != nNumber ||*/
/*N*/ 				rFtn.GetNumStr() != rNumStr ||
/*N*/ 				rFtn.IsEndNote() != bIsEndNote )
/*N*/ 			{
/*N*/ 				bChg = TRUE;
/*N*/
/*N*/ 				pTxtFtn->SetNumber( nNumber, &rNumStr );
/*N*/ 				if( rFtn.IsEndNote() != bIsEndNote )
/*N*/ 				{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	((SwFmtFtn&)rFtn).SetEndNote( bIsEndNote );
/*N*/ 				}
/*N*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 	nPos = n;		// nach vorne gibt es auch noch welche !
/*N*/ 	while( nPos &&
/*N*/ 			(( nIdx = _SwTxtFtn_GetIndex((pTxtFtn = rFtnArr[ --nPos ] )))
/*N*/ 				> nSttNd || ( nIdx == nSttNd &&
/*N*/ 				nSttCnt <= *pTxtFtn->GetStart() )) )
/*N*/ 		if( nIdx < nEndNd || ( nIdx == nEndNd &&
/*N*/ 			nEndCnt >= *pTxtFtn->GetStart() ) )
/*N*/ 		{
/*?*/ 			const SwFmtFtn& rFtn = pTxtFtn->GetFtn();
/*?*/ 			if( /*rFtn.GetNumber() != nNumber ||*/
/*?*/ 				rFtn.GetNumStr() != rNumStr ||
/*?*/ 				rFtn.IsEndNote() != bIsEndNote )
/*?*/ 			{
/*?*/ 				bChg = TRUE;
/*?*/
/*?*/ 				pTxtFtn->SetNumber( nNumber, &rNumStr );
/*?*/ 				if( rFtn.IsEndNote() != bIsEndNote )
/*?*/ 				{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	((SwFmtFtn&)rFtn).SetEndNote( bIsEndNote );
/*?*/ 				}
/*?*/ 			}
/*N*/ 		}
/*N*/
/*N*/ 	// wer muss angestossen werden ??
/*N*/ 	if( bChg )
/*N*/ 	{
/*N*/
/*N*/ 		if ( bTypeChgd )
/*?*/ 			rFtnArr.UpdateAllFtn();
/*N*/ 		if( FTNNUM_PAGE != GetFtnInfo().eNum )
/*N*/ 		{
/*N*/ 			if ( !bTypeChgd )
/*N*/ 				rFtnArr.UpdateAllFtn();
/*N*/ 		}
/*N*/ 		else if( GetRootFrm() )
/*?*/ 			{DBG_BF_ASSERT(0, "STRIP");} //STRIP001 GetRootFrm()->UpdateFtnNums();
/*N*/ 	}
/*N*/ 	return bChg;
/*N*/ }





}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
