/*************************************************************************
 *
 *  $RCSfile: doccorr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
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
    for( int nb = 0; nb < 2; ++nb ) \
        if( &((pPam)->GetBound( BOOL(nb) ).nNode.GetNode()) == pOldNode )   \
        { \
            (pPam)->GetBound( BOOL(nb) ) = aNewPos; \
            (pPam)->GetBound( BOOL(nb) ).nContent += nOffset; \
        }



void PaMCorrAbs( const SwNodeIndex &rOldNode,
                const SwPosition &rNewPos,
                const xub_StrLen nOffset)
{
    const SwNode* pOldNode = &rOldNode.GetNode();
    const SwPosition aNewPos( rNewPos );
    const SwDoc* pDoc = pOldNode->GetDoc();
    SwCrsrShell* pShell = pDoc->GetEditShell();

    if( pShell )
    {
        FOREACHSHELL_START( pShell )
            register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
    // Alle ueberfluessigen Crsr sind vom Stack, oder ??
    //      ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
            if( _pStkCrsr )
            do {
                _PaMCorrAbs1( _pStkCrsr )
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                _PaMCorrAbs1( PCURCRSR )
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                _PaMCorrAbs1( PCURSH->GetTblCrs() )

        FOREACHSHELL_END( pShell )
    }

    {
        register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                _PaMCorrAbs1( PCURCRSR )
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    _PaMCorrAbs1( PCURCRSR )
                FOREACHPAM_END()
            }
        }
    }
}


void SwCrsrShell::PaMCorrAbs( const SwNodeIndex &rOldNode,
                              const SwPosition &rNewPos,
                              const xub_StrLen nOffset )
{
    // alle Verzeichnisse/Bookmarks/.. verschieben.
    GetDoc()->CorrAbs( rOldNode, rNewPos, nOffset );
    ::PaMCorrAbs( rOldNode, rNewPos, nOffset );
}

void SwDoc::CorrAbs( const SwNodeIndex& rOldNode,
                     const SwPosition& rNewPos,
                     const xub_StrLen nOffset,
                     BOOL bMoveCrsr )
{
    const SwNode* pOldNode = &rOldNode.GetNode();
    SwPosition aNewPos( rNewPos );

    { // erstmal die Bookmark korrigieren
        register SwBookmarks& rBkmks = *pBookmarkTbl;
        register SwBookmark* pBkmk;
        for( USHORT n = 0; n < rBkmks.Count(); ++n )
        {
            // liegt auf der Position ??
            int bChgd = 0;
            if( &( pBkmk = (SwBookmark*)rBkmks[ n ])->pPos1->nNode.GetNode() == pOldNode )
            {
                *pBkmk->pPos1 = aNewPos;
                pBkmk->pPos1->nContent += nOffset;
                bChgd = 1;
            }
            if( pBkmk->pPos2 && &pBkmk->pPos2->nNode.GetNode() == pOldNode )
            {
                *pBkmk->pPos2 = aNewPos;
                pBkmk->pPos2->nContent += nOffset;
                bChgd = 2;
            }
            // ungueltige Selektion? Dann die Klammerung aufheben
            if( bChgd && pBkmk->pPos2 &&
                pBkmk->pPos2->nNode.GetNode().FindTableBoxStartNode() !=
                pBkmk->pPos1->nNode.GetNode().FindTableBoxStartNode() )
            {
                if( 1 == bChgd )
                    *pBkmk->pPos1 = *pBkmk->pPos2;
                delete pBkmk->pPos2, pBkmk->pPos2 = 0;
                if( pBkmk->IsServer() )
                    pBkmk->SetRefObject( 0 );
                // die Sortierung muss aufrecht erhalten bleiben!
                rBkmks.Remove( n-- );
                rBkmks.Insert( pBkmk );
            }
        }
    }
    { // dann die Redlines korrigieren
        register SwRedlineTbl& rTbl = *pRedlineTbl;
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            // liegt auf der Position ??
            _PaMCorrAbs1( rTbl[ n ] )
        }
    }

    if( bMoveCrsr )
        ::PaMCorrAbs( rOldNode, rNewPos, nOffset );
}

/*  */

#define _PaMCorrAbs2( pPam ) \
    for( int nb = 0; nb < 2; ++nb ) \
        if( (pPam)->GetBound( BOOL(nb) ).nNode >= nSttNode && \
            (pPam)->GetBound( BOOL(nb) ).nNode <= nEndNode ) \
            (pPam)->GetBound( BOOL(nb) ) = aNewPos;



void PaMCorrAbs( const SwNodeIndex &rStartNode,
                 const SwNodeIndex &rEndNode,
                 const SwPosition &rNewPos )
{
    const ULONG nSttNode = rStartNode.GetIndex();
    const ULONG nEndNode = rEndNode.GetIndex();
    const SwPosition aNewPos( rNewPos );
    SwDoc* pDoc = rStartNode.GetNode().GetDoc();

    SwCrsrShell* pShell = pDoc->GetEditShell();
    if( pShell )
    {
        FOREACHSHELL_START( pShell )
            register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
    // Alle ueberfluessigen Crsr sind vom Stack, oder ??
    //      ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
            if( _pStkCrsr )
            do {
                _PaMCorrAbs2( _pStkCrsr )
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                _PaMCorrAbs2( PCURCRSR )
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                _PaMCorrAbs2( PCURSH->GetTblCrs() )

        FOREACHSHELL_END( pShell )
    }

    {
        register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                _PaMCorrAbs2( PCURCRSR )
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    _PaMCorrAbs2( PCURCRSR )
                FOREACHPAM_END()
            }
        }
    }
}


void SwCrsrShell::PaMCorrAbs( const SwNodeIndex &rStartNode,
                              const SwNodeIndex &rEndNode,
                              const SwPosition &rNewPos )
{
    // alle Verzeichnisse/Bookmarks/.. verschieben.
    GetDoc()->CorrAbs( rStartNode, rEndNode, rNewPos );
    ::PaMCorrAbs( rStartNode, rEndNode, rNewPos );
}

void SwDoc::CorrAbs( const SwNodeIndex& rStartNode,
                     const SwNodeIndex& rEndNode,
                     const SwPosition& rNewPos,
                     BOOL bMoveCrsr )
{
    const ULONG nSttNode = rStartNode.GetIndex();
    const ULONG nEndNode = rEndNode.GetIndex();
    SwPosition aNewPos( rNewPos );

//  if( !DoesUndo() )
        // erstmal die Bookmarks/Redlines korrigieren
        _DelBookmarks( rStartNode, rEndNode );

    if( bMoveCrsr )
        ::PaMCorrAbs( rStartNode, rEndNode, rNewPos );
}


/*  */

#define _PaMCorrAbs3( pPam ) \
    for( int nb = 0; nb < 2; ++nb ) \
        if( aStart <= (pPam)->GetBound( BOOL(nb) ) && \
            (pPam)->GetBound( BOOL(nb) ) <= aEnd ) \
            (pPam)->GetBound( BOOL(nb) ) = aNewPos;

void PaMCorrAbs( const SwPaM& rRange,
                const SwPosition& rNewPos )
{
    SwPosition aStart( *rRange.Start() );
    SwPosition aEnd( *rRange.End() );
    SwPosition aNewPos( rNewPos );
    SwDoc* pDoc = aStart.nNode.GetNode().GetDoc();
    SwCrsrShell* pShell = pDoc->GetEditShell();

    if( pShell )
    {
        FOREACHSHELL_START( pShell )
            register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
    // Alle ueberfluessigen Crsr sind vom Stack, oder ??
    //      ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
            if( _pStkCrsr )
            do {
                _PaMCorrAbs3( _pStkCrsr )
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                _PaMCorrAbs3( PCURCRSR )
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                _PaMCorrAbs3( PCURSH->GetTblCrs() )

        FOREACHSHELL_END( pShell )
    }
    {
        register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                _PaMCorrAbs3( PCURCRSR )
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    _PaMCorrAbs3( PCURCRSR )
                FOREACHPAM_END()
            }
        }
    }
}


void SwDoc::CorrAbs( const SwPaM& rRange,
                    const SwPosition& rNewPos,
                    BOOL bMoveCrsr )
{
    SwPosition aStart( *rRange.Start() );
    SwPosition aEnd( *rRange.End() );
    SwPosition aNewPos( rNewPos );

//  if( !DoesUndo() )
        // erstmal die Bookmarks/Redlines korrigieren
        _DelBookmarks( aStart.nNode, aEnd.nNode, 0,
                           &aStart.nContent, &aEnd.nContent );
    if( bMoveCrsr )
        ::PaMCorrAbs( rRange, rNewPos );
}


/*  */

#define _PaMCorrRel1( pPam ) \
    for( int nb = 0; nb < 2; ++nb ) \
        if( &((pPam)->GetBound( BOOL(nb) ).nNode.GetNode()) == pOldNode ) \
        { \
            (pPam)->GetBound( BOOL(nb) ).nNode = aNewPos.nNode; \
            (pPam)->GetBound( BOOL(nb) ).nContent.Assign( (SwIndexReg*) \
                            aNewPos.nContent.GetIdxReg(), \
                            nCntIdx + (pPam)->GetBound( BOOL(nb) ).nContent. \
                                GetIndex() ); \
        }



void PaMCorrRel( const SwNodeIndex &rOldNode,
                 const SwPosition &rNewPos,
                 const xub_StrLen nOffset )
{
    const SwNode* pOldNode = &rOldNode.GetNode();
    SwPosition aNewPos( rNewPos );
    const SwDoc* pDoc = pOldNode->GetDoc();

    xub_StrLen nCntIdx = rNewPos.nContent.GetIndex() + nOffset;

    SwCrsrShell* pShell = pDoc->GetEditShell();
    if( pShell )
    {
        FOREACHSHELL_START( pShell )
            register SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
    // Alle ueberfluessigen Crsr sind vom Stack, oder ??
    //      ASSERT( !_pStkCrsr, "Es stehen noch Crsr auf dem CrsrStack" );
            if( _pStkCrsr )
            do {
                _PaMCorrRel1( _pStkCrsr )
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                _PaMCorrRel1( PCURCRSR )
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                _PaMCorrRel1( PCURSH->GetTblCrs() )

        FOREACHSHELL_END( pShell )
    }
    {
        register SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                _PaMCorrRel1( PCURCRSR )
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr = (SwUnoTableCrsr*)*rTbl[ n ];
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    _PaMCorrRel1( PCURCRSR )
                FOREACHPAM_END()
            }
        }
    }
}

void SwDoc::CorrRel( const SwNodeIndex& rOldNode,
                     const SwPosition& rNewPos,
                     const xub_StrLen nOffset,
                     BOOL bMoveCrsr )
{
    const SwNode* pOldNode = &rOldNode.GetNode();
    SwPosition aNewPos( rNewPos );
    xub_StrLen nCntIdx = aNewPos.nContent.GetIndex() + nOffset;

    { // erstmal die Bookmark korrigieren
        register SwBookmarks& rBkmks = *pBookmarkTbl;
        register SwBookmark* pBkmk;
        for( USHORT n = 0; n < rBkmks.Count(); ++n )
        {
            // liegt auf der Position ??
            int bChgd = FALSE;
            if( &( pBkmk = (SwBookmark*)rBkmks[ n ])->pPos1->nNode.GetNode()
                == pOldNode )
            {
                pBkmk->pPos1->nNode = aNewPos.nNode;
                pBkmk->pPos1->nContent.Assign( (SwIndexReg*)
                            aNewPos.nContent.GetIdxReg(),
                            nCntIdx + pBkmk->pPos1->nContent.GetIndex() );
                bChgd = 1;
            }
            if( pBkmk->pPos2 && &pBkmk->pPos2->nNode.GetNode() == pOldNode )
            {
                pBkmk->pPos2->nNode = aNewPos.nNode;
                pBkmk->pPos2->nContent.Assign( (SwIndexReg*)
                            aNewPos.nContent.GetIdxReg(),
                            nCntIdx + pBkmk->pPos2->nContent.GetIndex() );
                bChgd = 2;
            }
            // ungueltige Selektion? Dann die Klammerung aufheben
            if( bChgd && pBkmk->pPos2 &&
                pBkmk->pPos2->nNode.GetNode().FindTableBoxStartNode() !=
                pBkmk->pPos1->nNode.GetNode().FindTableBoxStartNode() )
            {
                if( 1 == bChgd )
                    *pBkmk->pPos1 = *pBkmk->pPos2;
                delete pBkmk->pPos2, pBkmk->pPos2 = 0;
                if( pBkmk->IsServer() )
                    pBkmk->SetRefObject( 0 );

                // die Sortierung muss aufrecht erhalten bleiben!
                rBkmks.Remove( n-- );
                rBkmks.Insert( pBkmk );
            }
        }
    }
    { // dann die Redlines korrigieren
        register SwRedlineTbl& rTbl = *pRedlineTbl;
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            // liegt auf der Position ??
            _PaMCorrRel1( rTbl[ n ] )
        }
    }

    if( bMoveCrsr )
        ::PaMCorrAbs( rOldNode, rNewPos, nOffset );
}


/*  */

SwEditShell* SwDoc::GetEditShell( ViewShell** ppSh ) const
{
    // Layout und OLE-Shells sollten vorhanden sein!
    if( pLayout && pLayout->GetCurrShell() )
    {
        register ViewShell *pSh = pLayout->GetCurrShell(), *pVSh = pSh;
        if( ppSh )
            *ppSh = pSh;

        // wir suchen uns eine EditShell, falls diese existiert
        do {
            if( pSh->IsA( TYPE( SwEditShell ) ) )
                return (SwEditShell*)pSh;

        } while( pVSh != ( pSh = (ViewShell*)pSh->GetNext() ));
    }
    else if( ppSh )
        *ppSh = 0;

    return 0;
}



