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

#include <tools/shl.hxx>
#include <swmodule.hxx>

#include <horiornt.hxx>
#include <errhdl.hxx>

#include <doc.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <pamtyp.hxx>
#include <viewsh.hxx>
#include <rootfrm.hxx>
namespace binfilter {

#ifndef DBG_UTIL

    #define _CHECK_REDLINE( pDoc )

#else

    // helper function for lcl_CheckRedline
    // make sure that pPos->nContent points into pPos->nNode
    // (or into the 'special' no-content-node-IndexReg)


    // check validity of the redline table. Checks redline bounds, and make
    // sure the redlines are sorted and non-overlapping.
/*N*/ 	void lcl_CheckRedline( const SwDoc* pDoc )
/*N*/ 	{
/*N*/ 		const SwRedlineTbl& rTbl = pDoc->GetRedlineTbl();
/*N*/
/*N*/         // verify valid redline positions
/*N*/ 		for( USHORT i = 0; i < rTbl.Count(); ++i )
/*?*/             {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 lcl_CheckPam( rTbl[ i ] );
/*N*/
/*N*/         for( USHORT j = 0; j < rTbl.Count(); ++j )
/*N*/         {
/*N*/             // check for empty redlines
/*N*/             ASSERT( ( *(rTbl[j]->GetPoint()) != *(rTbl[j]->GetMark()) ) ||
/*N*/                     ( rTbl[j]->GetContentIdx() != NULL ),
/*N*/                     "redline table corrupted: empty redline" );
/*N*/  		}
/*N*/
/*N*/         // verify proper redline sorting
/*N*/ 		for( USHORT n = 1; n < rTbl.Count(); ++n )
/*N*/ 		{
/*?*/ 			const SwRedline* pPrev = rTbl[ n-1 ];
/*?*/             const SwRedline* pCurrent = rTbl[ n ];
/*?*/
/*?*/             // check redline sorting
/*?*/             ASSERT( *pPrev->Start() <= *pCurrent->Start(),
/*?*/                     "redline table corrupted: not sorted correctly" );
/*?*/
/*?*/             // check for overlapping redlines
/*?*/             ASSERT( *pPrev->End() <= *pCurrent->Start(),
/*?*/                     "redline table corrupted: overlapping redlines" );
/*N*/ 		}
/*N*/ 	}
/*N*/
/*N*/ 	#define _CHECK_REDLINE( pDoc ) lcl_CheckRedline( pDoc );
/*N*/
/*N*/ #endif

/*N*/ SV_IMPL_OP_PTRARR_SORT( _SwRedlineTbl, SwRedlinePtr )

/*N*/ void SwDoc::SetRedlineMode( USHORT eMode )
/*N*/ {
/*N*/ 	if( eRedlineMode != eMode )
/*N*/ 	{
/*N*/ 		if( (REDLINE_SHOW_MASK & eRedlineMode) != (REDLINE_SHOW_MASK & eMode)
/*N*/ 			|| 0 == (REDLINE_SHOW_MASK & eMode) )
/*N*/ 		{
/*N*/ 			// und dann alles verstecken, anzeigen
/*N*/ 			void (SwRedline::*pFnc)( USHORT ) = 0;
/*N*/
/*N*/ 			switch( REDLINE_SHOW_MASK & eMode )
/*N*/ 			{
/*N*/ 			case REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE:
/*N*/ 				pFnc = &SwRedline::Show;
/*N*/ 				break;
/*N*/ 			case REDLINE_SHOW_INSERT:
/*N*/ 				pFnc = &SwRedline::Hide;
/*N*/ 				break;
/*N*/ 			case REDLINE_SHOW_DELETE:
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 pFnc = &SwRedline::ShowOriginal;
/*?*/ 				break;
/*N*/
/*N*/ 			default:
/*N*/ 				pFnc = &SwRedline::Hide;
/*N*/ 				eMode |= REDLINE_SHOW_INSERT;
/*N*/ 				break;
/*N*/ 			}
/*N*/
/*N*/ 			_CHECK_REDLINE( this )
/*N*/
/*N*/ 			if( pFnc )
/*N*/ 				for( USHORT nLoop = 1; nLoop <= 2; ++nLoop )
/*N*/ 					for( USHORT i = 0; i < pRedlineTbl->Count(); ++i )
/*N*/ 						((*pRedlineTbl)[ i ]->*pFnc)( nLoop );
/*N*/ 			_CHECK_REDLINE( this )
/*N*/ 		}
/*N*/ 		eRedlineMode = (SwRedlineMode)eMode;
/*N*/ 	}
/*N*/ }


/*

Text heisst, nicht von Redline "verseuchter" Text.

Verhalten von Insert-Redline:
    - im Text							- Redline Object einfuegen
    - im InsertRedline (eigenes)		- ignorieren, bestehendes wird
                                          aufgespannt
    - im InsertRedline (andere)			- Insert Redline aufsplitten
                                          Redline Object einfuegen
    - in DeleteRedline					- Delete Redline aufsplitten oder
                                          am Ende/Anfang verschieben

Verhalten von Delete-Redline:
    - im Text							- Redline Object einfuegen
    - im DeleteRedline (eigenes/andere)	- ignorieren
    - im InsertRedline (eigenes)		- ignorieren, Zeichen aber loeschen
    - im InsertRedline (andere)			- Insert Redline aufsplitten
                                          Redline Object einfuegen
    - Ueberlappung von Text und 		- Text in eigenen Insert loeschen,
      eigenem Insert					  im andereren Text aufspannen (bis
                                          zum Insert!
    - Ueberlappung von Text und 		- Redline Object einfuegen, der
      anderem Insert                      andere Insert wird vom Delete
                                          ueberlappt
*/

/*N*/ BOOL SwDoc::AppendRedline( SwRedline* pNewRedl, BOOL bCallDelete )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); return FALSE;//STRIP001 //STRIP001 	_CHECK_REDLINE( this )
/*N*/ }




/*N*/ BOOL SwDoc::DeleteRedline( const SwPaM& rRange, BOOL bSaveInUndo,
/*N*/ 							USHORT nDelType )
/*N*/ {
/*N*/ 	if( REDLINE_IGNOREDELETE_REDLINES & eRedlineMode ||
/*N*/ 		!rRange.HasMark() || *rRange.GetMark() == *rRange.GetPoint() )
/*N*/ 		return FALSE;
/*N*/
/*N*/ 	BOOL bChg = FALSE;
/*N*/
/*N*/ 	const SwPosition* pStt = rRange.Start(),
/*N*/ 					* pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
/*N*/ 													   : rRange.GetPoint();
/*N*/ 	USHORT n = 0;
/*N*/ 	GetRedline( *pStt, &n );
/*N*/ 	for( ; n < pRedlineTbl->Count() ; ++n )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	SwRedline* pRedl = (*pRedlineTbl)[ n ];
/*N*/ 	}
/*N*/
/*N*/ 	if( bChg )
/*N*/ 		SetModified();
/*N*/
/*N*/ 	return bChg;
/*N*/ }

/*N*/ BOOL SwDoc::DeleteRedline( const SwStartNode& rNode, BOOL bSaveInUndo,
/*N*/ 							USHORT nDelType )
/*N*/ {
/*N*/ 	SwPaM aTemp( *rNode.EndOfSectionNode(), rNode );
/*N*/ 	return DeleteRedline( aTemp, bSaveInUndo, nDelType );
/*N*/ }


/*N*/ USHORT SwDoc::GetRedlinePos( const SwNode& rNd, USHORT nType ) const
/*N*/ {
/*N*/ 	const ULONG nNdIdx = rNd.GetIndex();
/*N*/ 	for( USHORT n = 0; n < pRedlineTbl->Count() ; ++n )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	const SwRedline* pTmp = (*pRedlineTbl)[ n ];
/*N*/ 	}
/*N*/ 	return USHRT_MAX;
/*N*/ }

/*N*/ const SwRedline* SwDoc::GetRedline( const SwPosition& rPos,
/*N*/ 									USHORT* pFndPos ) const
/*N*/ {
/*N*/ 	register USHORT nO = pRedlineTbl->Count(), nM, nU = 0;
/*N*/ 	if( nO > 0 )
/*N*/ 	{
/*?*/ 	DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	nO--;
/*N*/ 	}
/*N*/ 	if( pFndPos )
/*N*/ 		*pFndPos = nU;
/*N*/ 	return 0;
/*N*/ }

typedef BOOL (*Fn_AcceptReject)( SwRedlineTbl& rArr, USHORT& rPos,
                        BOOL bCallDelete,
                        const SwPosition* pSttRng,
                        const SwPosition* pEndRng);















// Kommentar am Redline setzen

// legt gebenenfalls einen neuen Author an
/*N*/ USHORT SwDoc::GetRedlineAuthor()
/*N*/ {
/*N*/ return SW_MOD()->GetRedlineAuthor(); //SW50.SDW
/*N*/ }

    // fuer die Reader usw. - neuen Author in die Tabelle eintragen
/*N*/ USHORT SwDoc::InsertRedlineAuthor( const String& rNew )
/*N*/ {
/*N*/ return SW_MOD()->InsertRedlineAuthor(rNew); //SW50.SDW
/*N*/ }


    // setze Kommentar-Text fuers Redline, das dann per AppendRedline
    // hereinkommt. Wird vom Autoformat benutzt. 0-Pointer setzt den Modus
    // wieder zurueck. Pointer wird nicht kopiert, muss also gueltig bleiben!

/*N*/ void SwDoc::SetRedlinePasswd(
/*N*/ 			const ::com::sun::star::uno::Sequence <sal_Int8>& rNew )
/*N*/ {
/*N*/ 	aRedlinePasswd = rNew;
/*N*/ 	SetModified();
/*N*/ }

/*  */

/*N*/ BOOL SwRedlineTbl::Insert( SwRedlinePtr& p, BOOL bIns ) //SW50.SDW
/*N*/ {
/*N*/ 	BOOL bRet = FALSE;
/*N*/ 	if( p->HasValidRange() )
/*N*/ 	{
/*N*/ 		bRet = _SwRedlineTbl::Insert( p );
/*N*/ 	}
/*N*/ 	else if( bIns )
/*N*/ 		bRet = InsertWithValidRanges( p );
/*N*/ 	else
/*N*/ 	{
/*N*/ 		ASSERT( !this, "Redline: falscher Bereich" );
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ BOOL SwRedlineTbl::InsertWithValidRanges( SwRedlinePtr& p, USHORT* pInsPos ) //SW50.SDW
/*N*/ {
/*N*/ 	// erzeuge aus den Selektion gueltige "Teilbereiche".
/*N*/ 	BOOL bAnyIns = FALSE;
/*N*/ 	SwPosition* pStt = p->Start(),
/*N*/ 			  * pEnd = pStt == p->GetPoint() ? p->GetMark() : p->GetPoint();
/*N*/ 	SwPosition aNewStt( *pStt );
/*N*/ 	SwNodes& rNds = aNewStt.nNode.GetNodes();
/*N*/ 	SwCntntNode* pC;
/*N*/
/*N*/ 	if( !aNewStt.nNode.GetNode().IsCntntNode() )
/*N*/ 	{
/*N*/ 		pC = rNds.GoNext( &aNewStt.nNode );
/*N*/ 		if( pC )
/*N*/ 			aNewStt.nContent.Assign( pC, 0 );
/*N*/ 		else
/*N*/ 			aNewStt.nNode = rNds.GetEndOfContent();
/*N*/ 	}
/*N*/
/*N*/ 	SwRedline* pNew = 0;
/*N*/ 	USHORT nInsPos;
/*N*/
/*N*/ 	if( aNewStt < *pEnd )
/*N*/ 		do {
/*N*/ 			if( !pNew )
/*N*/ 				pNew = new SwRedline( p->GetRedlineData(), aNewStt );
/*N*/ 			else
/*N*/ 			{
/*N*/ 				pNew->DeleteMark();
/*N*/ 				*pNew->GetPoint() = aNewStt;
/*N*/ 			}
/*N*/
/*N*/ 			pNew->SetMark();
/*N*/ 			GoEndSection( pNew->GetPoint() );
/*N*/ 			if( *pNew->GetPoint() > *pEnd )
/*N*/ 			{
/*N*/ 				BOOL bWeiter = TRUE;
/*N*/ 				pC = 0;
/*N*/ 				if( aNewStt.nNode != pEnd->nNode )
/*N*/ 					do {
/*N*/ 						SwNode& rCurNd = aNewStt.nNode.GetNode();
/*N*/ 						if( rCurNd.IsStartNode() )
/*N*/ 						{
/*N*/ 							if( rCurNd.EndOfSectionIndex() < pEnd->nNode.GetIndex() )
/*N*/ 								aNewStt.nNode = *rCurNd.EndOfSectionNode();
/*N*/ 							else
/*N*/ 								break;
/*N*/ 						}
/*N*/ 						else if( rCurNd.IsCntntNode() )
/*N*/ 							pC = rCurNd.GetCntntNode();
/*N*/ 						aNewStt.nNode++;
/*N*/ 					} while( aNewStt.nNode.GetIndex() < pEnd->nNode.GetIndex() );
/*N*/
/*N*/ 				if( aNewStt.nNode == pEnd->nNode )
/*N*/ 					aNewStt.nContent = pEnd->nContent;
/*N*/ 				else if( pC )
/*N*/ 				{
/*N*/ 					aNewStt.nNode = *pC;
/*N*/ 					aNewStt.nContent.Assign( pC, pC->Len() );
/*N*/ 				}
/*N*/
/*N*/ 				if( aNewStt <= *pEnd )
/*N*/ 					*pNew->GetPoint() = aNewStt;
/*N*/ 			}
/*N*/ 			else
/*N*/ 				aNewStt = *pNew->GetPoint();
/*N*/
/*N*/ 			if( *pNew->GetPoint() != *pNew->GetMark() &&
/*N*/ 				_SwRedlineTbl::Insert( pNew, nInsPos ) )
/*N*/ 			{
/*N*/ 				bAnyIns = TRUE;
/*N*/ 				pNew = 0;
/*N*/ 				if( pInsPos && *pInsPos < nInsPos )
/*N*/ 					*pInsPos = nInsPos;
/*N*/ 			}
/*N*/
/*N*/ 			if( aNewStt >= *pEnd ||
/*N*/ 				0 == (pC = rNds.GoNext( &aNewStt.nNode )) )
/*N*/ 				break;
/*N*/
/*N*/ 			aNewStt.nContent.Assign( pC, 0 );
/*N*/
/*N*/ 		} while( aNewStt < *pEnd );
/*N*/
/*N*/ 	delete pNew;
/*N*/ 	delete p, p = 0;
/*N*/ 	return bAnyIns;
/*N*/ }

/*N*/ void SwRedlineTbl::Remove( USHORT nP, USHORT nL ) //SW50.SDW
/*N*/ {
/*N*/ 	SwDoc* pDoc = 0;
/*N*/ 	if( !nP && nL && nL == _SwRedlineTbl::Count() )
/*N*/ 		pDoc = _SwRedlineTbl::GetObject( 0 )->GetDoc();
/*N*/
/*N*/ 	_SwRedlineTbl::Remove( nP, nL );
/*N*/
/*N*/ 	ViewShell* pSh;
/*N*/ 	if( pDoc && !pDoc->IsInDtor() && pDoc->GetRootFrm() &&
/*N*/ 		0 != ( pSh = pDoc->GetRootFrm()->GetCurrShell()) )
/*N*/ 		pSh->InvalidateWindows( SwRect( 0, 0, LONG_MAX, LONG_MAX ) );
/*N*/ }


// suche den naechsten oder vorherigen Redline mit dergleichen Seq.No
// Mit dem Lookahead kann die Suche eingeschraenkt werden. 0 oder
// USHRT_MAX suchen im gesamten Array.




/*  */

/*?*/SwRedlineExtraData::~SwRedlineExtraData()
/*?*/{
/*?*/}



/*?*/int SwRedlineExtraData::operator == ( const SwRedlineExtraData& ) const
/*?*/{
/*?*/	return FALSE;
/*?*/}

/*  */

/*?*/SwRedlineData::SwRedlineData( const SwRedlineData& rCpy, BOOL bCpyNext )
/*?*/	: nAuthor( rCpy.nAuthor ), eType( rCpy.eType ), aStamp( rCpy.aStamp ),
/*?*/	sComment( rCpy.sComment ), nSeqNo( rCpy.nSeqNo ),
/*?*/	pExtraData( rCpy.pExtraData ? rCpy.pExtraData->CreateNew() : 0 ),
/*?*/	pNext( (bCpyNext && rCpy.pNext) ? new SwRedlineData( *rCpy.pNext ) : 0 )
/*?*/{
/*?*/}

    // fuer sw3io: pNext geht in eigenen Besitz ueber!
/*?*/SwRedlineData::SwRedlineData( SwRedlineType eT, USHORT nAut, const DateTime& rDT,
/*?*/				   const String& rCmnt, SwRedlineData *pNxt,
/*?*/				   SwRedlineExtraData* pData )
/*?*/	: eType( eT ), pNext( pNxt ), nAuthor( nAut ), aStamp( rDT ),
/*?*/	  sComment( rCmnt ), pExtraData( pData ), nSeqNo( 0 )
/*?*/{
/*?*/}

/*?*/SwRedlineData::~SwRedlineData()
/*?*/{
/*?*/	delete pExtraData;
/*?*/	delete pNext;
/*?*/}

/*  */

/*?*/SwRedline::SwRedline( const SwRedlineData& rData, const SwPosition& rPos )
/*?*/	: SwPaM( rPos ),
/*?*/	pCntntSect( 0 ),
/*?*/	pRedlineData( new SwRedlineData( rData ))
/*?*/{
/*?*/	bDelLastPara = bIsLastParaDelete = FALSE;
/*?*/	bIsVisible = TRUE;
/*?*/}

/*?*/SwRedline::SwRedline( const SwRedline& rCpy )
/*?*/	: SwPaM( *rCpy.GetMark(), *rCpy.GetPoint() ),
/*?*/	pCntntSect( 0 ),
/*?*/	pRedlineData( new SwRedlineData( *rCpy.pRedlineData ))
/*?*/{
/*?*/	bDelLastPara = bIsLastParaDelete = FALSE;
/*?*/	bIsVisible = TRUE;
/*?*/	if( !rCpy.HasMark() )
/*?*/		DeleteMark();
/*?*/}

/*?*/SwRedline::~SwRedline()
/*?*/{
/*?*/	if( pCntntSect )
/*?*/	{
/*?*/		// dann den Content Bereich loeschen
/*?*/		if( !GetDoc()->IsInDtor() )
/*?*/			GetDoc()->DeleteSection( &pCntntSect->GetNode() );
/*?*/		delete pCntntSect;
/*?*/	}
/*?*/	delete pRedlineData;
/*?*/}

// liegt eine gueltige Selektion vor?
/*N*/ BOOL SwRedline::HasValidRange() const //SW50.SDW
/*N*/ {
/*N*/ 	const SwNode* pPtNd = &GetPoint()->nNode.GetNode(),
/*N*/ 				* pMkNd = &GetMark()->nNode.GetNode();
/*N*/ 	if( pPtNd->FindStartNode() == pMkNd->FindStartNode() &&
/*N*/ 		!pPtNd->FindStartNode()->IsTableNode() &&
/*N*/ 		// JP 18.5.2001: Bug 87222 - invalid if points on the end of content
/*N*/         // DVO 25.03.2002: #96530# end-of-content only invalid if no content
/*N*/         //                 index exists
/*N*/ 		( pPtNd != pMkNd || GetContentIdx() != NULL ||
/*N*/           pPtNd != &pPtNd->GetNodes().GetEndOfContent() )
/*N*/ 		)
/*N*/ 		return TRUE;
/*N*/ 	return FALSE;
/*N*/ }


/*N*/ void SwRedline::Show( USHORT nLoop )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }

/*N*/ void SwRedline::Hide( USHORT nLoop )
/*N*/ {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
/*N*/ }




/*************************************************************************
 *                      SwRedline::CalcStartEnd()
 * Calculates the start and end position of the intersection rTmp and
 * text node nNdIdx
 *************************************************************************/







// fuers Undo
/*N*/ void SwRedline::SetContentIdx( const SwNodeIndex* pIdx )
/*N*/ {
DBG_BF_ASSERT(0, "STRIP"); //STRIP001 //STRIP001 	if( pIdx && !pCntntSect )
/*N*/ }




USHORT SwRedline::GetStackCount() const
{
    USHORT nRet = 1;
    for( SwRedlineData* pCur = pRedlineData; pCur->pNext; ++nRet )
        pCur = pCur->pNext;
    return nRet;
}


/*?*/const String& SwRedline::GetAuthorString( USHORT nPos ) const
/*?*/{
        SwRedlineData* pCur;
/*?*/	for( pCur = pRedlineData; nPos && pCur->pNext; --nPos )
/*?*/		pCur = pCur->pNext;
/*?*/	ASSERT( !nPos, "Pos angabe ist zu gross" );
/*?*/	return SW_MOD()->GetRedlineAuthor(pCur->nAuthor);
/*?*/}

const DateTime& SwRedline::GetTimeStamp( USHORT nPos ) const
{
    SwRedlineData* pCur; for( pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->aStamp;
}

SwRedlineType SwRedline::GetRealType( USHORT nPos ) const
{
    SwRedlineData* pCur; for( pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->eType;
}

const String& SwRedline::GetComment( USHORT nPos ) const
{
    SwRedlineData* pCur; for( pCur = pRedlineData; nPos && pCur->pNext; --nPos )
        pCur = pCur->pNext;
    ASSERT( !nPos, "Pos angabe ist zu gross" );
    return pCur->sComment;
}

/*N*/ int SwRedline::operator==( const SwRedline& rCmp ) const
/*N*/ {
/*N*/ 	return this == &rCmp;
/*N*/ }

/*N*/ int SwRedline::operator<( const SwRedline& rCmp ) const
/*N*/ {
/*N*/ 	    BOOL nResult = FALSE;
/*N*/
/*N*/     if (*Start() < *rCmp.Start())
/*N*/         nResult = TRUE;
/*N*/     else if (*Start() == *rCmp.Start())
/*N*/         if (*End() < *rCmp.End())
/*N*/             nResult = TRUE;
/*N*/
/*N*/     return nResult;
/*N*/ }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
