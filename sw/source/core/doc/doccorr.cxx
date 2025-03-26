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
#include <IDocumentRedlineAccess.hxx>
#include <node.hxx>
#include <editsh.hxx>
#include <viscrs.hxx>
#include <redline.hxx>
#include <mvsave.hxx>
#include <docary.hxx>
#include <unocrsr.hxx>

namespace
{
    /// find the relevant section in which the SwUnoCursor may wander.
    /// returns NULL if no restrictions apply
    const SwStartNode* lcl_FindUnoCursorSection( const SwNode& rNode )
    {
        const SwStartNode* pStartNode = rNode.IsStartNode() ? rNode.GetStartNode() : rNode.StartOfSectionNode();
        while( ( pStartNode != nullptr ) &&
               ( pStartNode->StartOfSectionNode() != pStartNode ) &&
               // section node is only start node allowing overlapped delete
               pStartNode->IsSectionNode() )
        {
            pStartNode = pStartNode->StartOfSectionNode();
        }

        return pStartNode;
    }

    bool lcl_PosCorrAbs(SwPosition & rPos,
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

    bool lcl_PaMCorrAbs(SwPaM & rPam,
        const SwPosition& rStart,
        const SwPosition& rEnd,
        const SwPosition& rNewPos)
    {
        bool bRet = false;
        bRet |= lcl_PosCorrAbs(rPam.GetBound(), rStart, rEnd, rNewPos);
        bRet |= lcl_PosCorrAbs(rPam.GetBound(false), rStart, rEnd, rNewPos);
        return bRet;
    };

    void lcl_PaMCorrRel1(SwPaM * pPam,
        SwNode const * const pOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nCntIdx)
    {
        for(int nb = 0; nb < 2; ++nb)
        {
            SwPosition & rPos = pPam->GetBound(bool(nb));
            if(&rPos.GetNode() == pOldNode)
            {
                rPos.Assign(rNewPos.GetNode(), SwNodeOffset(0),
                            nCntIdx + rPos.GetContentIndex());
            }
        }
    }
}

void PaMCorrAbs( const SwPaM& rRange,
                const SwPosition& rNewPos )
{
    SwPosition const aStart( *rRange.Start() );
    SwPosition const aEnd( *rRange.End() );
    SwPosition const aNewPos( rNewPos );
    SwDoc& rDoc = aStart.GetNode().GetDoc();

    if (SwCursorShell *const pShell = rDoc.GetEditShell())
    {
        for(const SwViewShell& rShell : pShell->GetRingContainer())
        {
            const SwCursorShell* pCursorShell = dynamic_cast<const SwCursorShell*>(&rShell);
            if(!pCursorShell)
                continue;
            SwPaM *_pStackCursor = pCursorShell->GetStackCursor();
            if( _pStackCursor )
                for (;;)
                {
                    lcl_PaMCorrAbs( *_pStackCursor, aStart, aEnd, aNewPos );
                    if( !_pStackCursor )
                        break;
                    _pStackCursor = _pStackCursor->GetNext();
                    if( _pStackCursor == pCursorShell->GetStackCursor() )
                        break;
                }

            for(SwPaM& rPaM : const_cast<SwShellCursor*>(pCursorShell->GetCursor_())->GetRingContainer())
            {
                lcl_PaMCorrAbs( rPaM, aStart, aEnd, aNewPos );
            }

            if( pCursorShell->IsTableMode() )
                lcl_PaMCorrAbs( const_cast<SwPaM &>(*pCursorShell->GetTableCrs()), aStart, aEnd, aNewPos );
        }
    }

    rDoc.cleanupUnoCursorTable();
    for(const auto& pWeakUnoCursor : rDoc.mvUnoCursorTable)
    {
        auto pUnoCursor(pWeakUnoCursor.lock());
        if(!pUnoCursor)
            continue;

        bool bChange = false; // has the UNO cursor been corrected?

        // determine whether the UNO cursor will leave it's designated
        // section
        bool const bLeaveSection =
            pUnoCursor->IsRemainInSection() &&
            ( lcl_FindUnoCursorSection( aNewPos.GetNode() ) !=
                lcl_FindUnoCursorSection(
                    pUnoCursor->GetPoint()->GetNode() ) );

        for(SwPaM& rPaM : pUnoCursor->GetRingContainer())
        {
            bChange |= lcl_PaMCorrAbs( rPaM, aStart, aEnd, aNewPos );
        }

        SwUnoTableCursor *const pUnoTableCursor =
            dynamic_cast<SwUnoTableCursor *>(pUnoCursor.get());
        if( pUnoTableCursor )
        {
            for(SwPaM& rPaM : pUnoTableCursor->GetSelRing().GetRingContainer())
            {
                bChange |=
                    lcl_PaMCorrAbs( rPaM, aStart, aEnd, aNewPos );
            }
        }

        // if a UNO cursor leaves its designated section, we must inform
        // (and invalidate) said cursor
        if (bChange && bLeaveSection)
        {
            // the UNO cursor has left its section. We need to notify it!
            sw::UnoCursorHint aHint;
            pUnoCursor->m_aNotifier.Broadcast(aHint);
        }
    }
}

void SwDoc::CorrAbs(const SwNode& rOldNode,
    const SwPosition& rNewPos,
    const sal_Int32 nOffset,
    bool bMoveCursor)
{
    const SwContentNode *const pContentNode( rOldNode.GetContentNode() );
    SwPaM const aPam(rOldNode, 0,
                     rOldNode, pContentNode ? pContentNode->Len() : 0);
    SwPosition aNewPos(rNewPos);
    aNewPos.AdjustContent(nOffset);

    if (!mbDontCorrectBookmarks)
        getIDocumentMarkAccess()->correctMarksAbsolute(rOldNode, rNewPos, nOffset);
    // fix redlines
    {
        SwRedlineTable& rTable = getIDocumentRedlineAccess().GetRedlineTable();
        for (SwRedlineTable::size_type n = 0; n < rTable.size(); )
        {
            // is on position ??
            SwRangeRedline *const pRedline( rTable[ n ] );
            bool const bChanged =
                lcl_PaMCorrAbs(*pRedline, *aPam.Start(), *aPam.End(), aNewPos);
            // clean up empty redlines: docredln.cxx asserts these as invalid
            if (bChanged && (*pRedline->GetPoint() == *pRedline->GetMark())
                         && (pRedline->GetContentIdx() == nullptr))
            {
                rTable.DeleteAndDestroy(n);
            }
            else
            {
                ++n;
            }
        }

        // To-Do - need to add here 'SwExtraRedlineTable' also ?
    }

    if(bMoveCursor)
    {
        ::PaMCorrAbs(aPam, aNewPos);
    }
}

void SwDoc::CorrAbs(
    const SwPaM& rRange,
    const SwPosition& rNewPos,
    bool bMoveCursor )
{
    const SwPosition& aStart(*rRange.Start());
    const SwPosition& aEnd(*rRange.End());

    DelBookmarks( aStart.GetNode(), aEnd.GetNode(), nullptr, aStart.GetContentIndex(), aEnd.GetContentIndex() );

    if(bMoveCursor)
        ::PaMCorrAbs(rRange, rNewPos);
}

void SwDoc::CorrAbs(
    const SwNodeIndex& rStartNode,
    const SwNodeIndex& rEndNode,
    const SwPosition& rNewPos,
    bool bMoveCursor )
{
    DelBookmarks( rStartNode.GetNode(), rEndNode.GetNode() );

    if(bMoveCursor)
    {
        SwContentNode *const pContentNode( rEndNode.GetNode().GetContentNode() );
        SwPaM const aPam(rStartNode, 0,
                         rEndNode, pContentNode ? pContentNode->Len() : 0);
        ::PaMCorrAbs(aPam, rNewPos);
    }
}

void PaMCorrRel( const SwNode &rOldNode,
                 const SwPosition &rNewPos,
                 const sal_Int32 nOffset )
{
    const SwNode* pOldNode = &rOldNode;
    SwPosition aNewPos( rNewPos );
    const SwDoc& rDoc = pOldNode->GetDoc();

    const sal_Int32 nCntIdx = rNewPos.GetContentIndex() + nOffset;

    if (SwCursorShell const* pShell = rDoc.GetEditShell())
    {
        for(const SwViewShell& rShell : pShell->GetRingContainer())
        {
            SwCursorShell* pCursorShell = const_cast<SwCursorShell*>(dynamic_cast<const SwCursorShell*>(&rShell));
            if(!pCursorShell)
                continue;
            SwPaM *_pStackCursor = pCursorShell->GetStackCursor();
            if( _pStackCursor )
                for (;;)
                {
                    lcl_PaMCorrRel1( _pStackCursor, pOldNode, aNewPos, nCntIdx );
                    if( !_pStackCursor )
                        break;
                    _pStackCursor = _pStackCursor->GetNext();
                    if( _pStackCursor == pCursorShell->GetStackCursor() )
                        break;
                }

            SwPaM* pStartPaM = pCursorShell->GetCursor_();
            for(SwPaM& rPaM : pStartPaM->GetRingContainer())
            {
                lcl_PaMCorrRel1( &rPaM, pOldNode, aNewPos, nCntIdx);
            }

            if( pCursorShell->IsTableMode() )
                lcl_PaMCorrRel1( pCursorShell->GetTableCrs(), pOldNode, aNewPos, nCntIdx );
        }
    }

    rDoc.cleanupUnoCursorTable();
    for(const auto& pWeakUnoCursor : rDoc.mvUnoCursorTable)
    {
        auto pUnoCursor(pWeakUnoCursor.lock());
        if(!pUnoCursor)
            continue;
        for(SwPaM& rPaM : pUnoCursor->GetRingContainer())
        {
            lcl_PaMCorrRel1( &rPaM, pOldNode, aNewPos, nCntIdx );
        }

        SwUnoTableCursor* pUnoTableCursor =
            dynamic_cast<SwUnoTableCursor*>(pUnoCursor.get());
        if( pUnoTableCursor )
        {
            for(SwPaM& rPaM : pUnoTableCursor->GetSelRing().GetRingContainer())
            {
                lcl_PaMCorrRel1( &rPaM, pOldNode, aNewPos, nCntIdx );
            }
        }
    }
}

void SwDoc::CorrRel(const SwNode& rOldNode,
    const SwPosition& rNewPos,
    const sal_Int32 nOffset,
    bool bMoveCursor)
{
    getIDocumentMarkAccess()->correctMarksRelative(rOldNode, rNewPos, nOffset);

    { // fix the Redlines
        SwRedlineTable& rTable = getIDocumentRedlineAccess().GetRedlineTable();
        SwPosition aNewPos(rNewPos);
        for(SwRangeRedline* p : rTable)
        {
            // lies on the position ??
            lcl_PaMCorrRel1( p, &rOldNode, aNewPos, aNewPos.GetContentIndex() + nOffset );
        }

        // To-Do - need to add here 'SwExtraRedlineTable' also ?
    }

    if(bMoveCursor)
        ::PaMCorrRel(rOldNode, rNewPos, nOffset);
}

SwEditShell const * SwDoc::GetEditShell() const
{
    SwViewShell const *pCurrentView = getIDocumentLayoutAccess().GetCurrentViewShell();
    // Layout and OLE shells should be available
    if( pCurrentView )
    {
        for(const SwViewShell& rCurrentSh : pCurrentView->GetRingContainer())
        {
            // look for an EditShell (if it exists)
            if( auto pEditShell = dynamic_cast<const SwEditShell *>(&rCurrentSh) )
            {
                return pEditShell;
            }
        }
    }
    return nullptr;
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
