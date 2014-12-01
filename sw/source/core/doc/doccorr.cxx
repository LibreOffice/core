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
#include <IDocumentLayoutAccess.hxx>
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
        const sal_Int32 nCntIdx)
    {
        for(int nb = 0; nb < 2; ++nb)
            if(&((pPam)->GetBound(bool(nb)).nNode.GetNode()) == pOldNode)
            {
                (pPam)->GetBound(bool(nb)).nNode = rNewPos.nNode;
                (pPam)->GetBound(bool(nb)).nContent.Assign(
                    const_cast<SwIndexReg*>(rNewPos.nContent.GetIdxReg()),
                    nCntIdx + (pPam)->GetBound(bool(nb)).nContent.GetIndex());
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
        BOOST_FOREACH(const SwViewShell& rShell, std::make_pair(pShell->beginRing(), pShell->endRing()))
        {
            if(!rShell.IsA( TYPE( SwCrsrShell )))
                continue;
            const SwCrsrShell* pCrsrShell = static_cast<const SwCrsrShell*>(&rShell);
            SwPaM *_pStkCrsr = pCrsrShell->GetStkCrsr();
            if( _pStkCrsr )
                do {
                    lcl_PaMCorrAbs( *_pStkCrsr, aStart, aEnd, aNewPos );
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr = static_cast<SwPaM *>(_pStkCrsr->GetNext())) != pCrsrShell->GetStkCrsr()) );

            FOREACHPAM_START( const_cast<SwShellCrsr*>(pCrsrShell->_GetCrsr()) )
                lcl_PaMCorrAbs( *PCURCRSR, aStart, aEnd, aNewPos );
            FOREACHPAM_END()

            if( pCrsrShell->IsTableMode() )
                lcl_PaMCorrAbs( const_cast<SwPaM &>(*pCrsrShell->GetTblCrs()), aStart, aEnd, aNewPos );
        }
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
    const sal_Int32 nOffset,
    bool bMoveCrsr)
{
    SwCntntNode *const pCntntNode( rOldNode.GetNode().GetCntntNode() );
    SwPaM const aPam(rOldNode, 0,
                     rOldNode, (pCntntNode) ? pCntntNode->Len() : 0);
    SwPosition aNewPos(rNewPos);
    aNewPos.nContent += nOffset;

    getIDocumentMarkAccess()->correctMarksAbsolute(rOldNode, rNewPos, nOffset);
    // fix redlines
    {
        SwRedlineTbl& rTbl = getIDocumentRedlineAccess().GetRedlineTbl();
        for (sal_uInt16 n = 0; n < rTbl.size(); )
        {
            // is on position ??
            SwRangeRedline *const pRedline( rTbl[ n ] );
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

        // To-Do - need to add here 'SwExtraRedlineTbl' also ?
    }

    if(bMoveCrsr)
    {
        ::PaMCorrAbs(aPam, aNewPos);
    }
}

void SwDoc::CorrAbs(
    const SwPaM& rRange,
    const SwPosition& rNewPos,
    bool bMoveCrsr )
{
    SwPosition aStart(*rRange.Start());
    SwPosition aEnd(*rRange.End());
    SwPosition aNewPos(rNewPos);

    _DelBookmarks( aStart.nNode, aEnd.nNode, NULL, &aStart.nContent, &aEnd.nContent );

    if(bMoveCrsr)
        ::PaMCorrAbs(rRange, rNewPos);
}

void SwDoc::CorrAbs(
    const SwNodeIndex& rStartNode,
    const SwNodeIndex& rEndNode,
    const SwPosition& rNewPos,
    bool bMoveCrsr )
{
    _DelBookmarks( rStartNode, rEndNode );

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
                 const sal_Int32 nOffset )
{
    const SwNode* pOldNode = &rOldNode.GetNode();
    SwPosition aNewPos( rNewPos );
    const SwDoc* pDoc = pOldNode->GetDoc();

    const sal_Int32 nCntIdx = rNewPos.nContent.GetIndex() + nOffset;

    SwCrsrShell const* pShell = pDoc->GetEditShell();
    if( pShell )
    {
        BOOST_FOREACH(const SwViewShell& rShell, std::make_pair(pShell->beginRing(), pShell->endRing()))
        {
            if(!rShell.IsA( TYPE( SwCrsrShell )))
                continue;
            SwCrsrShell* pCrsrShell = const_cast<SwCrsrShell*>(static_cast<const SwCrsrShell*>(&rShell));
            SwPaM *_pStkCrsr = pCrsrShell->GetStkCrsr();
            if( _pStkCrsr )
                do {
                    lcl_PaMCorrRel1( _pStkCrsr, pOldNode, aNewPos, nCntIdx );
                } while ( (_pStkCrsr != 0 ) &&
                    ((_pStkCrsr = static_cast<SwPaM *>(_pStkCrsr->GetNext())) != pCrsrShell->GetStkCrsr()) );

            SwPaM* pStartPaM = pCrsrShell->_GetCrsr();
            BOOST_FOREACH(SwPaM& rPaM, std::make_pair(pStartPaM->beginRing(), pStartPaM->endRing()))
            {
                lcl_PaMCorrRel1( &rPaM, pOldNode, aNewPos, nCntIdx);
            }

            if( pCrsrShell->IsTableMode() )
                lcl_PaMCorrRel1( pCrsrShell->GetTblCrs(), pOldNode, aNewPos, nCntIdx );
       }
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
    const sal_Int32 nOffset,
    bool bMoveCrsr)
{
    getIDocumentMarkAccess()->correctMarksRelative(rOldNode, rNewPos, nOffset);

    { // fix the Redlines
        SwRedlineTbl& rTbl = getIDocumentRedlineAccess().GetRedlineTbl();
        SwPosition aNewPos(rNewPos);
        for( sal_uInt16 n = 0; n < rTbl.size(); ++n )
        {
            // lies on the position ??
            lcl_PaMCorrRel1( rTbl[ n ], &rOldNode.GetNode(), aNewPos, aNewPos.nContent.GetIndex() + nOffset );
        }

        // To-Do - need to add here 'SwExtraRedlineTbl' also ?
    }

    if(bMoveCrsr)
        ::PaMCorrRel(rOldNode, rNewPos, nOffset);
}

SwEditShell const * SwDoc::GetEditShell() const
{
    SwViewShell const *pCurrentView = getIDocumentLayoutAccess().GetCurrentViewShell();
    // Layout and OLE shells should be available
    if( pCurrentView )
    {
        SwViewShell const *pFirstVSh = pCurrentView;
        SwViewShell const *pCurrentVSh = pFirstVSh;
        // look for an EditShell (if it exists)
        do {
            if( pCurrentVSh->IsA( TYPE( SwEditShell ) ) )
            {
                return static_cast<const SwEditShell*>(pCurrentVSh);
            }
        } while( pFirstVSh != ( pCurrentVSh = static_cast<SwViewShell*>(pCurrentVSh->GetNext()) ));
    }
    return 0;
}

SwEditShell* SwDoc::GetEditShell()
{
    return const_cast<SwEditShell*>( const_cast<SwDoc const *>( this )->GetEditShell() );
}

::sw::IShellCursorSupplier * SwDoc::GetIShellCursorSupplier()
{
    return GetEditShell();
}

//bool foo()
//{
//    bool b1 = true ? true : false;
//    bool b2 = (true ? true : false) ? true : false;
//    bool b3 = true ? (true ? true : false) : false;
//    bool b4 = true ? true : (true ? true : false);
//    return false;
//}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
