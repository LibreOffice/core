/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
#include <edimp.hxx>

/*
 * Macros to iterate over all CrsrShells
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

namespace
{
    /// find the relevant section in which the SwUnoCrsr may wander.
    /// returns NULL if no restrictions apply
    static const SwStartNode* lcl_FindUnoCrsrSection( const SwNode& rNode )
    {
        const SwStartNode* pStartNode = rNode.StartOfSectionNode();
        while( ( pStartNode != NULL ) &&
               ( pStartNode->StartOfSectionNode() != pStartNode ) &&
               ( pStartNode->GetStartNodeType() == SwNormalStartNode ) )
            pStartNode = pStartNode->StartOfSectionNode();

        return pStartNode;
    }

    static inline bool lcl_PosCorrAbs(SwPosition & rPos,
        const SwPosition& rStart,
        const SwPosition& rEnd,
        const SwPosition& rNewPos)
    {
        if ((rStart <= rPos) && (rPos <= rEnd))
        {
            rPos = rNewPos;
            return true;
        }
        return false;
    };

    static inline bool lcl_PaMCorrAbs(SwPaM & rPam,
        const SwPosition& rStart,
        const SwPosition& rEnd,
        const SwPosition& rNewPos)
    {
        bool bRet = false;
        bRet |= lcl_PosCorrAbs(rPam.GetBound(true ), rStart, rEnd, rNewPos);
        bRet |= lcl_PosCorrAbs(rPam.GetBound(false), rStart, rEnd, rNewPos);
        return bRet;
    };

    static inline void lcl_PaMCorrRel1(SwPaM * pPam,
        SwNode const * const pOldNode,
        const SwPosition& rNewPos,
        const xub_StrLen nCntIdx)
    {
        for(int nb = 0; nb < 2; ++nb)
            if(&((pPam)->GetBound(sal_Bool(nb)).nNode.GetNode()) == pOldNode)
            {
                (pPam)->GetBound(sal_Bool(nb)).nNode = rNewPos.nNode;
                (pPam)->GetBound(sal_Bool(nb)).nContent.Assign(
                    const_cast<SwIndexReg*>(rNewPos.nContent.GetIdxReg()),
                    nCntIdx + (pPam)->GetBound(sal_Bool(nb)).nContent.GetIndex());
            }
    }
}

void PaMCorrAbs( const SwPaM& rRange,
                const SwPosition& rNewPos )
{
    SwPosition const aStart( *rRange.Start() );
    SwPosition const aEnd( *rRange.End() );
    SwPosition const aNewPos( rNewPos );
    SwDoc *const pDoc = aStart.nNode.GetNode().GetDoc();
    SwCrsrShell *const pShell = pDoc->GetEditShell();

    if( pShell )
    {
        FOREACHSHELL_START( pShell )
            SwPaM *_pStkCrsr = PCURSH->GetStkCrsr();
            if( _pStkCrsr )
                do {
                    lcl_PaMCorrAbs( *_pStkCrsr, aStart, aEnd, aNewPos );
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr=(SwPaM *)_pStkCrsr->GetNext()) != PCURSH->GetStkCrsr()) );

            FOREACHPAM_START( PCURSH->_GetCrsr() )
                lcl_PaMCorrAbs( *PCURCRSR, aStart, aEnd, aNewPos );
            FOREACHPAM_END()

            if( PCURSH->IsTableMode() )
                lcl_PaMCorrAbs( *PCURSH->GetTblCrs(), aStart, aEnd, aNewPos );

        FOREACHSHELL_END( pShell )
    }
    {
        SwUnoCrsrTbl& rTbl = const_cast<SwUnoCrsrTbl&>(pDoc->GetUnoCrsrTbl());

        for( SwUnoCrsrTbl::iterator it = rTbl.begin(); it != rTbl.end(); ++it )
        {
            SwUnoCrsr *const pUnoCursor = *it;

            bool bChange = false; // has the UNO cursor been corrected?

            // determine whether the UNO cursor will leave it's designated
            // section
            bool const bLeaveSection =
                pUnoCursor->IsRemainInSection() &&
                ( lcl_FindUnoCrsrSection( aNewPos.nNode.GetNode() ) !=
                  lcl_FindUnoCrsrSection(
                      pUnoCursor->GetPoint()->nNode.GetNode() ) );

            FOREACHPAM_START( pUnoCursor )
                bChange |= lcl_PaMCorrAbs( *PCURCRSR, aStart, aEnd, aNewPos );
            FOREACHPAM_END()

            SwUnoTableCrsr *const pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr *>(*it);
            if( pUnoTblCrsr )
            {
                FOREACHPAM_START( &pUnoTblCrsr->GetSelRing() )
                    bChange |=
                        lcl_PaMCorrAbs( *PCURCRSR, aStart, aEnd, aNewPos );
                FOREACHPAM_END()
            }

            // if a UNO cursor leaves its designated section, we must inform
            // (and invalidate) said cursor
            if (bChange && bLeaveSection)
            {
                // the UNO cursor has left its section. We need to notify it!
                SwMsgPoolItem aHint( RES_UNOCURSOR_LEAVES_SECTION );
                pUnoCursor->ModifyNotification( &aHint, NULL );
            }
        }
    }
}

void SwDoc::CorrAbs(const SwNodeIndex& rOldNode,
    const SwPosition& rNewPos,
    const xub_StrLen nOffset,
    sal_Bool bMoveCrsr)
{
    SwCntntNode *const pCntntNode( rOldNode.GetNode().GetCntntNode() );
    SwPaM const aPam(rOldNode, 0,
                     rOldNode, (pCntntNode) ? pCntntNode->Len() : 0);
    SwPosition aNewPos(rNewPos);
    aNewPos.nContent += nOffset;

    getIDocumentMarkAccess()->correctMarksAbsolute(rOldNode, rNewPos, nOffset);
    {   // fix redlines
        SwRedlineTbl& rTbl = *mpRedlineTbl;
        for (sal_uInt16 n = 0; n < rTbl.size(); )
        {
            // is on position ??
            SwRedline *const pRedline( rTbl[ n ] );
            bool const bChanged =
                lcl_PaMCorrAbs(*pRedline, *aPam.Start(), *aPam.End(), aNewPos);
            // clean up empty redlines: docredln.cxx asserts these as invalid
            if (bChanged && (*pRedline->GetPoint() == *pRedline->GetMark())
                         && (pRedline->GetContentIdx() == NULL))
            {
                rTbl.DeleteAndDestroy(n);
            }
            else
            {
                ++n;
            }
        }
    }

    if(bMoveCrsr)
    {
        ::PaMCorrAbs(aPam, aNewPos);
    }
}

void SwDoc::CorrAbs(const SwPaM& rRange,
    const SwPosition& rNewPos,
    sal_Bool bMoveCrsr)
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
     sal_Bool bMoveCrsr)
{
    _DelBookmarks(rStartNode, rEndNode);

    if(bMoveCrsr)
    {
        SwCntntNode *const pCntntNode( rEndNode.GetNode().GetCntntNode() );
        SwPaM const aPam(rStartNode, 0,
                         rEndNode, (pCntntNode) ? pCntntNode->Len() : 0);
        ::PaMCorrAbs(aPam, rNewPos);
    }
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
        for( SwUnoCrsrTbl::iterator it = rTbl.begin(); it != rTbl.end(); ++it )
        {
            FOREACHPAM_START( *it )
                lcl_PaMCorrRel1( PCURCRSR, pOldNode, aNewPos, nCntIdx );
            FOREACHPAM_END()

            SwUnoTableCrsr* pUnoTblCrsr =
                dynamic_cast<SwUnoTableCrsr*>(*it);
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
    sal_Bool bMoveCrsr)
{
    getIDocumentMarkAccess()->correctMarksRelative(rOldNode, rNewPos, nOffset);

    { // fix the Redlines
        SwRedlineTbl& rTbl = *mpRedlineTbl;
        SwPosition aNewPos(rNewPos);
        for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
        {
            // lies on the position ??
            lcl_PaMCorrRel1( rTbl[ n ], &rOldNode.GetNode(), aNewPos, aNewPos.nContent.GetIndex() + nOffset );
        }
    }

    if(bMoveCrsr)
        ::PaMCorrRel(rOldNode, rNewPos, nOffset);
}

SwEditShell* SwDoc::GetEditShell( ViewShell** ppSh ) const
{
    // Layout and OLE shells should be available
    if( mpCurrentView )
    {
        ViewShell *pSh = mpCurrentView, *pVSh = pSh;
        if( ppSh )
            *ppSh = pSh;

        // look for an EditShell (if it exists)
        do {
            if( pSh->IsA( TYPE( SwEditShell ) ) )
                return (SwEditShell*)pSh;

        } while( pVSh != ( pSh = (ViewShell*)pSh->GetNext() ));
    }
    else if( ppSh )
        *ppSh = 0;  //swmod 071029//swmod 071225

    return 0;
}

::sw::IShellCursorSupplier * SwDoc::GetIShellCursorSupplier()
{
    return GetEditShell(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
