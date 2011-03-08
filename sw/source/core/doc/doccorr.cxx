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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <doc.hxx>
#include <node.hxx>
#include <rootfrm.hxx>
#include <editsh.hxx>
#include <viscrs.hxx>
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <redline.hxx>
#include <mvsave.hxx>
#include <docary.hxx>
#include <unocrsr.hxx>
#include <swundo.hxx>
#include <hints.hxx>

/*  */

/*
 * MACROS um ueber alle CrsrShells zu iterieren
 */
#define PCURSH ((SwCrsrShell*)_pStartShell)
#define FOREACHSHELL_START( pEShell ) \
    {\
        ViewShell *_pStartShell = pEShell; \
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

namespace
{
    // find the relevant section in which the SwUnoCrsr may wander. returns NULL if
    // no restrictions apply
    const SwStartNode* lcl_FindUnoCrsrSection( const SwNode& rNode )
    {
        const SwStartNode* pStartNode = rNode.StartOfSectionNode();
        while( ( pStartNode != NULL ) &&
               ( pStartNode->StartOfSectionNode() != pStartNode ) &&
               ( pStartNode->GetStartNodeType() == SwNormalStartNode ) )
            pStartNode = pStartNode->StartOfSectionNode();

        return pStartNode;
    }

    static inline void lcl_PaMCorrAbs1(SwPaM * pPam,
        SwNode const * const pOldNode,
        const SwPosition& rNewPos,
        const xub_StrLen nOffset)
    {
        for(int nb = 0; nb < 2; ++nb)
            if(&((pPam)->GetBound(BOOL(nb)).nNode.GetNode()) == pOldNode)
            {
                (pPam)->GetBound(BOOL(nb)) = rNewPos;
                (pPam)->GetBound(BOOL(nb)).nContent += nOffset;
            }
    };

    static inline bool lcl_PaMCorrAbs2(SwPaM* pPam,
        const SwPosition& rNewPos,
        ULONG nSttNode,
        ULONG nEndNode)
    {
        bool bRet = false;

        for(int nb = 0; nb < 2; ++nb)
            if((pPam)->GetBound(BOOL(nb)).nNode >= nSttNode &&
                (pPam)->GetBound(BOOL(nb)).nNode <= nEndNode)
            {
                (pPam)->GetBound(BOOL(nb)) = rNewPos;
                bRet = true;
            }
        return bRet;
    };

    static inline void lcl_PaMCorrAbs3(SwPaM * pPam,
        const SwPosition& rStart,
        const SwPosition& rEnd,
        const SwPosition& rNewPos)
    {
        for(int nb = 0; nb < 2; ++nb)
            if(rStart <= (pPam)->GetBound(BOOL(nb)) &&
                (pPam)->GetBound(BOOL(nb)) <= rEnd )
                (pPam)->GetBound(BOOL(nb)) = rNewPos;
    };

    static inline void lcl_PaMCorrRel1(SwPaM * pPam,
        SwNode const * const pOldNode,
        const SwPosition& rNewPos,
        const xub_StrLen nCntIdx)
    {
        for(int nb = 0; nb < 2; ++nb)
            if(&((pPam)->GetBound(BOOL(nb)).nNode.GetNode()) == pOldNode)
            {
                (pPam)->GetBound(BOOL(nb)).nNode = rNewPos.nNode;
                (pPam)->GetBound(BOOL(nb)).nContent.Assign(
                    const_cast<SwIndexReg*>(rNewPos.nContent.GetIdxReg()),
                    nCntIdx + (pPam)->GetBound(BOOL(nb)).nContent.GetIndex());
            }
    }
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
            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
            if( _pStkCrsr )
            do {
                lcl_PaMCorrAbs1( _pStkCrsr, pOldNode, aNewPos, nOffset );
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                lcl_PaMCorrAbs1( PCURCRSR, pOldNode, aNewPos, nOffset );
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                lcl_PaMCorrAbs1( PCURSH->GetTblCrs(), pOldNode, aNewPos, nOffset );

        FOREACHSHELL_END( pShell )
    }

    {
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                lcl_PaMCorrAbs1( PCURCRSR, pOldNode, aNewPos, nOffset );
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr*>(rTbl[ n ]);
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    lcl_PaMCorrAbs1( PCURCRSR, pOldNode, aNewPos, nOffset );
                FOREACHPAM_END()
            }
        }
    }
}


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
            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
            if( _pStkCrsr )
            do {
                lcl_PaMCorrAbs2( _pStkCrsr, aNewPos, nSttNode, nEndNode );
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                lcl_PaMCorrAbs2( PCURCRSR, aNewPos, nSttNode, nEndNode );
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                lcl_PaMCorrAbs2( PCURSH->GetTblCrs(), aNewPos, nSttNode, nEndNode );

        FOREACHSHELL_END( pShell )
    }

    {
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            bool bChange = false;

            SwUnoCrsr* pUnoCursor = rTbl[ n ];

            // determine whether the UNO cursor will leave it's designated
            // section
            bool bLeaveSection =
                pUnoCursor->IsRemainInSection() &&
                ( lcl_FindUnoCrsrSection( aNewPos.nNode.GetNode() ) !=
                  lcl_FindUnoCrsrSection(
                      pUnoCursor->GetPoint()->nNode.GetNode() ) );

            FOREACHPAM_START( pUnoCursor )
                bChange |= lcl_PaMCorrAbs2(PCURCRSR, aNewPos, nSttNode, nEndNode);
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr*>(pUnoCursor);
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    bChange |=
                        lcl_PaMCorrAbs2( PCURCRSR, aNewPos, nSttNode, nEndNode );
                FOREACHPAM_END()
            }

            // if a UNO cursor leaves its designated section, we must inform
            // (and invalidate) said cursor
            if( bChange && bLeaveSection )
            {
                // the UNO cursor has left its section. We need to notify it!
                SwMsgPoolItem aHint( RES_UNOCURSOR_LEAVES_SECTION );
                pUnoCursor->Modify( &aHint, NULL );
            }
        }
    }
}


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
            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
            if( _pStkCrsr )
            do {
                lcl_PaMCorrAbs3( _pStkCrsr, aStart, aEnd, aNewPos );
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                lcl_PaMCorrAbs3( PCURCRSR, aStart, aEnd, aNewPos );
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                lcl_PaMCorrAbs3( PCURSH->GetTblCrs(), aStart, aEnd, aNewPos );

        FOREACHSHELL_END( pShell )
    }
    {
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                lcl_PaMCorrAbs3( PCURCRSR, aStart, aEnd, aNewPos );
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr*>(rTbl[ n ]);
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    lcl_PaMCorrAbs3( PCURCRSR, aStart, aEnd, aNewPos );
                FOREACHPAM_END()
            }
        }
    }
}

void SwDoc::CorrAbs(const SwNodeIndex& rOldNode,
    const SwPosition& rNewPos,
    const xub_StrLen nOffset,
    BOOL bMoveCrsr)
{
    getIDocumentMarkAccess()->correctMarksAbsolute(rOldNode, rNewPos, nOffset);
    { // fix readlines
        SwRedlineTbl& rTbl = *pRedlineTbl;
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            // is on position ??
            lcl_PaMCorrAbs1( rTbl[ n ], &rOldNode.GetNode(), SwPosition(rNewPos), nOffset );
        }
    }

    if(bMoveCrsr)
        ::PaMCorrAbs(rOldNode, rNewPos, nOffset);
}

void SwDoc::CorrAbs(const SwPaM& rRange,
    const SwPosition& rNewPos,
    BOOL bMoveCrsr)
{
    SwPosition aStart(*rRange.Start());
    SwPosition aEnd(*rRange.End());
    SwPosition aNewPos(rNewPos);

    _DelBookmarks(aStart.nNode, aEnd.nNode, NULL,
        &aStart.nContent, &aEnd.nContent);
    if(bMoveCrsr)
        ::PaMCorrAbs(rRange, rNewPos);
}

void SwDoc::CorrAbs(const SwNodeIndex& rStartNode,
     const SwNodeIndex& rEndNode,
     const SwPosition& rNewPos,
     BOOL bMoveCrsr)
{
    SwPosition aNewPos(rNewPos);

    _DelBookmarks(rStartNode, rEndNode);

    if(bMoveCrsr)
        ::PaMCorrAbs(rStartNode, rEndNode, rNewPos);
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
            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
            if( _pStkCrsr )
            do {
                lcl_PaMCorrRel1( _pStkCrsr, pOldNode, aNewPos, nCntIdx );
            } while ( (_pStkCrsr != 0 ) &&
                ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                lcl_PaMCorrRel1( PCURCRSR, pOldNode, aNewPos, nCntIdx);
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                lcl_PaMCorrRel1( PCURSH->GetTblCrs(), pOldNode, aNewPos, nCntIdx );

        FOREACHSHELL_END( pShell )
    }
    {
        SwUnoCrsrTbl& rTbl = (SwUnoCrsrTbl&)pDoc->GetUnoCrsrTbl();
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            FOREACHPAM_START( rTbl[ n ] )
                lcl_PaMCorrRel1( PCURCRSR, pOldNode, aNewPos, nCntIdx );
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr*>(rTbl[ n ]);
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    lcl_PaMCorrRel1( PCURCRSR, pOldNode, aNewPos, nCntIdx );
                FOREACHPAM_END()
            }
        }
    }
}

void SwDoc::CorrRel(const SwNodeIndex& rOldNode,
    const SwPosition& rNewPos,
    const xub_StrLen nOffset,
    BOOL bMoveCrsr)
{
    getIDocumentMarkAccess()->correctMarksRelative(rOldNode, rNewPos, nOffset);

    { // dann die Redlines korrigieren
        SwRedlineTbl& rTbl = *pRedlineTbl;
        SwPosition aNewPos(rNewPos);
        for( USHORT n = 0; n < rTbl.Count(); ++n )
        {
            // liegt auf der Position ??
            lcl_PaMCorrRel1( rTbl[ n ], &rOldNode.GetNode(), aNewPos, aNewPos.nContent.GetIndex() + nOffset );
        }
    }

    if(bMoveCrsr)
        ::PaMCorrRel(rOldNode, rNewPos, nOffset);
}


SwEditShell* SwDoc::GetEditShell( ViewShell** ppSh ) const
{
    // Layout und OLE-Shells sollten vorhanden sein!
    if( pLayout && pLayout->GetCurrShell() )
    {
        ViewShell *pSh = pLayout->GetCurrShell(), *pVSh = pSh;
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
