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

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif
#ifndef _ERRHDL_HXX
#include <errhdl.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _MVSAVE_HXX
#include <mvsave.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _UNOCRSR_HXX
#include <unocrsr.hxx>
#endif




#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
namespace binfilter {

/*  */

/*
 * MACROS um ueber alle CrsrShells zu iterieren
 */
#define PCURSH ((SwCrsrShell*)_pStartShell)
#define FOREACHSHELL_START( pEShell ) \
    {\
        register ViewShell *_pStartShell = pEShell; \
        do { \
            if( _pStartShell->IsA( TYPE( SwCrsrShell )) ) \
            {

#define FOREACHSHELL_END( pEShell ) \
            } \
        } while((_pStartShell=(ViewShell*)_pStartShell->GetNext())!= pEShell ); \
    }

#define PCURCRSR (_pCurrCrsr)
#define FOREACHPAM_START(pSttCrsr) \
    {\
        SwPaM *_pStartCrsr = pSttCrsr, *_pCurrCrsr = pSttCrsr; \
        do {

#define FOREACHPAM_END() \
        } while( (_pCurrCrsr=(SwPaM *)_pCurrCrsr->GetNext()) != _pStartCrsr ); \
    }

/*  */

#define _PaMCorrAbs1( pPam ) \
    for( int nb = 0; nb < 2; ++nb )	\
        if( &((pPam)->GetBound( BOOL(nb) ).nNode.GetNode()) == pOldNode )	\
        { \
            (pPam)->GetBound( BOOL(nb) ) = aNewPos; \
            (pPam)->GetBound( BOOL(nb) ).nContent += nOffset; \
        }



/*N*/ void PaMCorrAbs( const SwNodeIndex &rOldNode,
/*N*/ 				const SwPosition &rNewPos,
/*N*/ 				const xub_StrLen nOffset)
/*N*/ {
/*N*/ 	const SwNode* pOldNode = &rOldNode.GetNode();
/*N*/ 	const SwPosition aNewPos( rNewPos );
/*N*/ 	const SwDoc* pDoc = pOldNode->GetDoc();
/*N*/ 	SwCrsrShell* pShell = pDoc->GetEditShell();
/*N*/ 
/*N*/ 	if( pShell )
/*N*/ 	{
/*N*/ 	 	FOREACHSHELL_START( pShell )
/*N*/ 			register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
/*N*/ 	// Alle ueberfluessigen Crsr sind vom Stack, oder ??
/*N*/ 	//		ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
/*N*/ 			if( _pStkCrsr )
/*?*/ 			do {
/*?*/ 				_PaMCorrAbs1( _pStkCrsr )
/*?*/ 			} while ( (_pStkCrsr != 0 ) &&
/*?*/ 				((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );
/*?*/ 
/*N*/ 			FOREACHPAM_START( PCURSH->_GetCrsr() )
/*N*/ 				_PaMCorrAbs1( PCURCRSR )
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			if( PCURSH->IsTableMode() )
/*N*/ 				_PaMCorrAbs1( PCURSH->GetTblCrs() )
/*N*/ 
/*N*/ 		FOREACHSHELL_END( pShell )
/*N*/ 	}
/*N*/ 
/*N*/ 	{
/*N*/ 		register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
/*N*/ 		for( USHORT n = 0; n < rTbl.Count(); ++n )
/*N*/ 		{
/*N*/ 			FOREACHPAM_START( rTbl[ n ] )
/*N*/ 				_PaMCorrAbs1( PCURCRSR )
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
/*N*/ 			if( pUnoTblCrsr )
/*N*/ 			{
/*?*/ 				FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
/*?*/ 					_PaMCorrAbs1( PCURCRSR )
/*?*/ 				FOREACHPAM_END()
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ void SwDoc::CorrAbs( const SwNodeIndex& rOldNode,
/*N*/ 					 const SwPosition& rNewPos,
/*N*/ 					 const xub_StrLen nOffset,
/*N*/ 					 BOOL bMoveCrsr )
/*N*/ {
/*N*/ 	const SwNode* pOldNode = &rOldNode.GetNode();
/*N*/ 	SwPosition aNewPos( rNewPos );
/*N*/ 
/*N*/ 	{ // erstmal die Bookmark korrigieren
/*N*/ 		register SwBookmarks& rBkmks = *pBookmarkTbl;
/*N*/ 		register SwBookmark* pBkmk;
/*N*/ 		for( USHORT n = 0; n < rBkmks.Count(); ++n )
/*N*/ 		{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 // liegt auf der Position ??
/*N*/ 		}
/*N*/ 	}
/*N*/ 	{ // dann die Redlines korrigieren
/*N*/ 		register SwRedlineTbl& rTbl = *pRedlineTbl;
/*N*/ 		for( USHORT n = 0; n < rTbl.Count(); ++n )
/*N*/ 		{
/*?*/ 			// liegt auf der Position ??
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 _PaMCorrAbs1( rTbl[ n ] )
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bMoveCrsr )
/*N*/ 		::binfilter::PaMCorrAbs( rOldNode, rNewPos, nOffset );
/*N*/ }

/*  */

/*N*/ bool _PaMCorrAbs2( SwPaM* pPam, 
/*N*/                    const SwPosition& rNewPos, 
/*N*/                    ULONG nSttNode, ULONG nEndNode )
/*N*/ {
/*N*/     bool bRet = false;
/*N*/ 
/*N*/ 	for( int nb = 0; nb < 2; ++nb )	\
/*N*/ 		if( (pPam)->GetBound( BOOL(nb) ).nNode >= nSttNode &&
/*N*/ 			(pPam)->GetBound( BOOL(nb) ).nNode <= nEndNode )
/*N*/         {
/*N*/ 			(pPam)->GetBound( BOOL(nb) ) = rNewPos;
/*N*/             bRet = true;
/*N*/         }
/*N*/ 
/*N*/     return bRet;
/*N*/ }

// find the relevant section in which the SwUnoCrsr may wander. returns NULL if
// no restrictions apply
/*N*/ const SwStartNode* lcl_FindUnoCrsrSection( const SwNode& rNode )
/*N*/ {
/*N*/     const SwStartNode* pStartNode = rNode.StartOfSectionNode();
/*N*/     while( ( pStartNode != NULL ) && 
/*N*/            ( pStartNode->StartOfSectionNode() != pStartNode ) &&
/*N*/            ( pStartNode->GetStartNodeType() == SwNormalStartNode ) )
/*N*/         pStartNode = pStartNode->StartOfSectionNode();
/*N*/ 
/*N*/     return pStartNode;
/*N*/ }

/*N*/ void PaMCorrAbs( const SwNodeIndex &rStartNode,
/*N*/ 				 const SwNodeIndex &rEndNode,
/*N*/ 				 const SwPosition &rNewPos )
/*N*/ {
/*N*/ 	const ULONG nSttNode = rStartNode.GetIndex();
/*N*/ 	const ULONG nEndNode = rEndNode.GetIndex();
/*N*/ 	const SwPosition aNewPos( rNewPos );
/*N*/ 	SwDoc* pDoc = rStartNode.GetNode().GetDoc();
/*N*/ 
/*N*/ 	SwCrsrShell* pShell = pDoc->GetEditShell();
/*N*/ 	if( pShell )
/*N*/ 	{
/*N*/ 		FOREACHSHELL_START( pShell )
/*N*/ 			register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
/*N*/ 	// Alle ueberfluessigen Crsr sind vom Stack, oder ??
/*N*/ 	//		ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
/*N*/ 			if( _pStkCrsr )
/*N*/ 			do {
/*N*/ 				_PaMCorrAbs2( _pStkCrsr, aNewPos, nSttNode, nEndNode );
/*N*/ 			} while ( (_pStkCrsr != 0 ) &&
/*N*/ 				((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );
/*N*/ 
/*N*/ 			FOREACHPAM_START( PCURSH->_GetCrsr() )
/*N*/ 				_PaMCorrAbs2( PCURCRSR, aNewPos, nSttNode, nEndNode );
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			if( PCURSH->IsTableMode() )
/*N*/ 				_PaMCorrAbs2( PCURSH->GetTblCrs(), aNewPos, nSttNode, nEndNode );
/*N*/ 
/*N*/ 		FOREACHSHELL_END( pShell )
/*N*/ 	}
/*N*/ 
/*N*/ 	{
/*N*/ 		register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
/*N*/ 		for( USHORT n = 0; n < rTbl.Count(); ++n )
/*N*/ 		{
/*N*/             bool bChange = false;
/*N*/ 
/*N*/             SwUnoCrsr* pUnoCursor = rTbl[ n ];
/*N*/ 
/*N*/             // determine whether the UNO cursor will leave it's designated
/*N*/             // section
/*N*/             bool bLeaveSection = 
/*N*/                 pUnoCursor->IsRemainInSection() &&
/*N*/                 ( lcl_FindUnoCrsrSection( aNewPos.nNode.GetNode() ) !=
/*N*/                   lcl_FindUnoCrsrSection( 
/*N*/                       pUnoCursor->GetPoint()->nNode.GetNode() ) );
/*N*/ 
/*N*/ 			FOREACHPAM_START( pUnoCursor )
/*N*/ 				bChange |= _PaMCorrAbs2(PCURCRSR, aNewPos, nSttNode, nEndNode);
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*pUnoCursor;
/*N*/ 			if( pUnoTblCrsr )
/*N*/ 			{
/*?*/ 				FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
/*?*/ 					bChange |= 
/*?*/                         _PaMCorrAbs2( PCURCRSR, aNewPos, nSttNode, nEndNode );
/*?*/ 				FOREACHPAM_END()
/*N*/ 			}
/*N*/ 
/*N*/             // if a UNO cursor leaves its designated section, we must inform
/*N*/             // (and invalidate) said cursor
/*N*/             if( bChange && bLeaveSection )
/*N*/             {
/*N*/                 // the UNO cursor has left its section. We need to notify it!
/*?*/                 SwMsgPoolItem aHint( RES_UNOCURSOR_LEAVES_SECTION );
/*?*/                 pUnoCursor->Modify( &aHint, NULL );
/*N*/             }
/*N*/ 		}
/*N*/ 	}
/*N*/ }



/*N*/ void SwDoc::CorrAbs( const SwNodeIndex& rStartNode,
/*N*/ 					 const SwNodeIndex& rEndNode,
/*N*/ 					 const SwPosition& rNewPos,
/*N*/ 					 BOOL bMoveCrsr )
/*N*/ {
/*N*/ 	const ULONG nSttNode = rStartNode.GetIndex();
/*N*/ 	const ULONG nEndNode = rEndNode.GetIndex();
/*N*/ 	SwPosition aNewPos( rNewPos );
/*N*/ 
/*N*/ //	if( !DoesUndo() )
/*N*/ 		// erstmal die Bookmarks/Redlines korrigieren
/*N*/ 		_DelBookmarks( rStartNode, rEndNode );
/*N*/ 
/*N*/ 	if( bMoveCrsr )
/*N*/ 		::binfilter::PaMCorrAbs( rStartNode, rEndNode, rNewPos );
/*N*/ }


/*  */

#define _PaMCorrAbs3( pPam ) \
    for( int nb = 0; nb < 2; ++nb )	\
        if( aStart <= (pPam)->GetBound( BOOL(nb) ) && \
            (pPam)->GetBound( BOOL(nb) ) <= aEnd ) \
            (pPam)->GetBound( BOOL(nb) ) = aNewPos;

/*N*/ void PaMCorrAbs( const SwPaM& rRange,
/*N*/ 				const SwPosition& rNewPos )
/*N*/ {
/*N*/ 	SwPosition aStart( *rRange.Start() );
/*N*/ 	SwPosition aEnd( *rRange.End() );
/*N*/ 	SwPosition aNewPos( rNewPos );
/*N*/ 	SwDoc* pDoc = aStart.nNode.GetNode().GetDoc();
/*N*/ 	SwCrsrShell* pShell = pDoc->GetEditShell();
/*N*/ 
/*N*/ 	if( pShell )
/*N*/ 	{
/*N*/ 		FOREACHSHELL_START( pShell )
/*N*/ 			register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
/*N*/ 	// Alle ueberfluessigen Crsr sind vom Stack, oder ??
/*N*/ 	//		ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
/*N*/ 			if( _pStkCrsr )
/*N*/ 			do {
/*?*/ 				_PaMCorrAbs3( _pStkCrsr )
/*?*/ 			} while ( (_pStkCrsr != 0 ) &&
/*?*/ 				((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );
/*N*/ 
/*N*/ 			FOREACHPAM_START( PCURSH->_GetCrsr() )
/*N*/ 				_PaMCorrAbs3( PCURCRSR )
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			if( PCURSH->IsTableMode() )
/*?*/ 				_PaMCorrAbs3( PCURSH->GetTblCrs() )
/*N*/ 
/*N*/ 		FOREACHSHELL_END( pShell )
/*N*/ 	}
/*N*/ 	{
/*N*/ 		register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
/*N*/ 		for( USHORT n = 0; n < rTbl.Count(); ++n )
/*N*/ 		{
/*N*/ 			FOREACHPAM_START( rTbl[ n ] )
/*N*/ 				_PaMCorrAbs3( PCURCRSR )
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
/*N*/ 			if( pUnoTblCrsr )
/*N*/ 			{
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 	FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }




/*  */

#define _PaMCorrRel1( pPam ) \
    for( int nb = 0; nb < 2; ++nb )	\
        if( &((pPam)->GetBound( BOOL(nb) ).nNode.GetNode()) == pOldNode ) \
        { \
            (pPam)->GetBound( BOOL(nb) ).nNode = aNewPos.nNode; \
            (pPam)->GetBound( BOOL(nb) ).nContent.Assign( (SwIndexReg*) \
                            aNewPos.nContent.GetIdxReg(), \
                            nCntIdx + (pPam)->GetBound( BOOL(nb) ).nContent. \
                                GetIndex() ); \
        }



/*N*/ void PaMCorrRel( const SwNodeIndex &rOldNode,
/*N*/ 				 const SwPosition &rNewPos,
/*N*/ 				 const xub_StrLen nOffset )
/*N*/ {
/*N*/ 	const SwNode* pOldNode = &rOldNode.GetNode();
/*N*/ 	SwPosition aNewPos( rNewPos );
/*N*/ 	const SwDoc* pDoc = pOldNode->GetDoc();
/*N*/ 
/*N*/ 	xub_StrLen nCntIdx = rNewPos.nContent.GetIndex() + nOffset;
/*N*/ 
/*N*/ 	SwCrsrShell* pShell = pDoc->GetEditShell();
/*N*/ 	if( pShell )
/*N*/ 	{
/*N*/ 		FOREACHSHELL_START( pShell )
/*N*/ 			register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
/*N*/ 	// Alle ueberfluessigen Crsr sind vom Stack, oder ??
/*N*/ 	//		ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
/*N*/ 			if( _pStkCrsr )
/*N*/ 			do {
/*?*/ 				_PaMCorrRel1( _pStkCrsr )
/*?*/ 			} while ( (_pStkCrsr != 0 ) &&
/*?*/ 				((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );
/*N*/ 
/*N*/ 			FOREACHPAM_START( PCURSH->_GetCrsr() )
/*N*/ 				_PaMCorrRel1( PCURCRSR )
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			if( PCURSH->IsTableMode() )
/*?*/ 				_PaMCorrRel1( PCURSH->GetTblCrs() )
/*N*/ 
/*N*/ 		FOREACHSHELL_END( pShell )
/*N*/ 	}
/*N*/ 	{
/*N*/ 		register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
/*N*/ 		for( USHORT n = 0; n < rTbl.Count(); ++n )
/*N*/ 		{
/*N*/ 			FOREACHPAM_START( rTbl[ n ] )
/*N*/ 				_PaMCorrRel1( PCURCRSR )
/*N*/ 			FOREACHPAM_END()
/*N*/ 
/*N*/ 			SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
/*N*/ 			if( pUnoTblCrsr )
/*N*/ 			{
/*?*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ void SwDoc::CorrRel( const SwNodeIndex& rOldNode,
/*N*/ 					 const SwPosition& rNewPos,
/*N*/ 					 const xub_StrLen nOffset,
/*N*/ 					 BOOL bMoveCrsr )
/*N*/ {
/*N*/ 	const SwNode* pOldNode = &rOldNode.GetNode();
/*N*/ 	SwPosition aNewPos( rNewPos );
/*N*/ 	xub_StrLen nCntIdx = aNewPos.nContent.GetIndex() + nOffset;
/*N*/ 
/*N*/ 	{ // erstmal die Bookmark korrigieren
/*N*/ 		register SwBookmarks& rBkmks = *pBookmarkTbl;
/*N*/ 		register SwBookmark* pBkmk;
/*N*/ 		for( USHORT n = 0; n < rBkmks.Count(); ++n )
/*N*/ 		{
/*N*/ 			// liegt auf der Position ??
/*N*/ 			int bChgd = FALSE;
/*N*/ 			if( &( pBkmk = (SwBookmark*)rBkmks[ n ])->pPos1->nNode.GetNode()
/*N*/ 				== pOldNode )
/*N*/ 			{
/*?*/ 				pBkmk->pPos1->nNode = aNewPos.nNode;
/*?*/ 				pBkmk->pPos1->nContent.Assign( (SwIndexReg*)
/*?*/ 							aNewPos.nContent.GetIdxReg(),
/*?*/ 							nCntIdx + pBkmk->pPos1->nContent.GetIndex() );
/*?*/ 				bChgd = 1;
/*N*/ 			}
/*N*/ 			if( pBkmk->pPos2 && &pBkmk->pPos2->nNode.GetNode() == pOldNode )
/*N*/ 			{
/*?*/ 				pBkmk->pPos2->nNode = aNewPos.nNode;
/*?*/ 				pBkmk->pPos2->nContent.Assign( (SwIndexReg*)
/*?*/ 							aNewPos.nContent.GetIdxReg(),
/*?*/ 							nCntIdx + pBkmk->pPos2->nContent.GetIndex() );
/*?*/ 				bChgd = 2;
/*N*/ 			}
/*N*/ 			// ungueltige Selektion? Dann die Klammerung aufheben
/*N*/ 			if( bChgd && pBkmk->pPos2 &&
/*N*/ 				pBkmk->pPos2->nNode.GetNode().FindTableBoxStartNode() !=
/*N*/ 				pBkmk->pPos1->nNode.GetNode().FindTableBoxStartNode() )
/*N*/ 			{
/*N*/ 				DBG_BF_ASSERT(0, "STRIP"); //STRIP001 if( 1 == bChgd )
/*?*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	{ // dann die Redlines korrigieren
/*N*/ 		register SwRedlineTbl& rTbl = *pRedlineTbl;
/*N*/ 		for( USHORT n = 0; n < rTbl.Count(); ++n )
/*N*/ 		{
/*N*/ 			// liegt auf der Position ??
/*?*/ 			DBG_BF_ASSERT(0, "STRIP"); //STRIP001 _PaMCorrRel1( rTbl[ n ] )
/*N*/ 		}
/*N*/ 	}
/*N*/ 
/*N*/ 	if( bMoveCrsr )
/*N*/ 		::binfilter::PaMCorrRel( rOldNode, rNewPos, nOffset );
/*N*/ }


/*  */

/*N*/ SwEditShell* SwDoc::GetEditShell( ViewShell** ppSh ) const
/*N*/ {
/*N*/ 	// Layout und OLE-Shells sollten vorhanden sein!
/*N*/ 	if( pLayout && pLayout->GetCurrShell() )
/*N*/ 	{
/*N*/ 		register ViewShell *pSh = pLayout->GetCurrShell(), *pVSh = pSh;
/*N*/ 		if( ppSh )
/*N*/ 			*ppSh = pSh;
/*N*/ 
/*N*/ 		// wir suchen uns eine EditShell, falls diese existiert
/*N*/ 		do {
/*N*/ 			if( pSh->IsA( TYPE( SwEditShell ) ) )
/*N*/ 				return (SwEditShell*)pSh;
/*N*/ 
/*?*/ 		} while( pVSh != ( pSh = (ViewShell*)pSh->GetNext() ));
/*N*/ 	}
/*N*/ 	else if( ppSh )
/*N*/ 		*ppSh = 0;
/*N*/ 
/*N*/ 	return 0;
/*N*/ }

// #102505# ->

// <- #102505#
}
