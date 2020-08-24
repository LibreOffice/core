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
#include <DocumentRedlineManager.hxx>
#include <frmfmt.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <fmtfld.hxx>
#include <frmtool.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentState.hxx>
#include <redline.hxx>
#include <UndoRedline.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <unocrsr.hxx>
#include <ftnidx.hxx>
#include <authfld.hxx>
#include <strings.hrc>
#include <swmodule.hxx>

using namespace com::sun::star;

#ifdef DBG_UTIL

    #define ERROR_PREFIX "redline table corrupted: "

    namespace
    {
        // helper function for lcl_CheckRedline
        // 1. make sure that pPos->nContent points into pPos->nNode
        // 2. check that position is valid and doesn't point after text
        void lcl_CheckPosition( const SwPosition* pPos )
        {
            assert(dynamic_cast<SwIndexReg*>(&pPos->nNode.GetNode())
                    == pPos->nContent.GetIdxReg());

            SwTextNode* pTextNode = pPos->nNode.GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                assert(pPos->nContent == 0);
            }
            else
            {
                assert(pPos->nContent >= 0 && pPos->nContent <= pTextNode->Len());
            }
        }

        void lcl_CheckPam( const SwPaM* pPam )
        {
            assert(pPam);
            lcl_CheckPosition( pPam->GetPoint() );
            lcl_CheckPosition( pPam->GetMark() );
        }

        // check validity of the redline table. Checks redline bounds, and make
        // sure the redlines are sorted and non-overlapping.
        void lcl_CheckRedline( IDocumentRedlineAccess& redlineAccess )
        {
            const SwRedlineTable& rTable = redlineAccess.GetRedlineTable();

            // verify valid redline positions
            for(SwRangeRedline* i : rTable)
                lcl_CheckPam( i );

            for(SwRangeRedline* j : rTable)
            {
                // check for empty redlines
                // note: these can destroy sorting in SwTextNode::Update()
                // if there's another one without mark on the same pos.
                OSL_ENSURE( ( *(j->GetPoint()) != *(j->GetMark()) ) ||
                            ( j->GetContentIdx() != nullptr ),
                            ERROR_PREFIX "empty redline" );
             }

            // verify proper redline sorting
            for( size_t n = 1; n < rTable.size(); ++n )
            {
                const SwRangeRedline* pPrev = rTable[ n-1 ];
                const SwRangeRedline* pCurrent = rTable[ n ];

                // check redline sorting
                SAL_WARN_IF( *pPrev->Start() > *pCurrent->Start(), "sw",
                             ERROR_PREFIX "not sorted correctly" );

                // check for overlapping redlines
                SAL_WARN_IF( *pPrev->End() > *pCurrent->Start(), "sw",
                             ERROR_PREFIX "overlapping redlines" );
            }

            assert(std::is_sorted(rTable.begin(), rTable.end(), CompareSwRedlineTable()));
        }
    }

    #define CHECK_REDLINE( pDoc ) lcl_CheckRedline( pDoc );

#else

    #define CHECK_REDLINE( pDoc )

#endif

namespace sw {

static void UpdateFieldsForRedline(IDocumentFieldsAccess & rIDFA)
{
    auto const pAuthType(static_cast<SwAuthorityFieldType*>(rIDFA.GetFieldType(
        SwFieldIds::TableOfAuthorities, OUString(), false)));
    if (pAuthType) // created on demand...
    {
        pAuthType->DelSequenceArray();
    }
    rIDFA.GetFieldType(SwFieldIds::RefPageGet, OUString(), false)->UpdateFields();
    rIDFA.GetSysFieldType(SwFieldIds::Chapter)->UpdateFields();
    rIDFA.UpdateExpFields(nullptr, false);
    rIDFA.UpdateRefFields();
}

void UpdateFramesForAddDeleteRedline(SwDoc & rDoc, SwPaM const& rPam)
{
    // no need to call UpdateFootnoteNums for FTNNUM_PAGE:
    // the AppendFootnote/RemoveFootnote will do it by itself!
    rDoc.GetFootnoteIdxs().UpdateFootnote(rPam.Start()->nNode);
    SwPosition currentStart(*rPam.Start());
    SwTextNode * pStartNode(rPam.Start()->nNode.GetNode().GetTextNode());
    while (!pStartNode)
    {
        SwStartNode *const pTableOrSectionNode(
            currentStart.nNode.GetNode().IsTableNode()
                ? static_cast<SwStartNode*>(currentStart.nNode.GetNode().GetTableNode())
                : static_cast<SwStartNode*>(currentStart.nNode.GetNode().GetSectionNode()));
        assert(pTableOrSectionNode); // known pathology
        for (sal_uLong j = pTableOrSectionNode->GetIndex(); j <= pTableOrSectionNode->EndOfSectionIndex(); ++j)
        {
            pTableOrSectionNode->GetNodes()[j]->SetRedlineMergeFlag(SwNode::Merge::Hidden);
        }
        for (SwRootFrame const*const pLayout : rDoc.GetAllLayouts())
        {
            if (pLayout->IsHideRedlines())
            {
                if (pTableOrSectionNode->IsTableNode())
                {
                    static_cast<SwTableNode*>(pTableOrSectionNode)->DelFrames(pLayout);
                }
                else
                {
                    static_cast<SwSectionNode*>(pTableOrSectionNode)->DelFrames(pLayout);
                }
            }
        }
        currentStart.nNode = pTableOrSectionNode->EndOfSectionIndex() + 1;
        currentStart.nContent.Assign(currentStart.nNode.GetNode().GetContentNode(), 0);
        pStartNode = currentStart.nNode.GetNode().GetTextNode();
    }
    if (currentStart < *rPam.End())
    {
        SwTextNode * pNode(pStartNode);
        do
        {
            std::vector<SwTextFrame*> frames;
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pNode);
            for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
            {
                if (pFrame->getRootFrame()->IsHideRedlines())
                {
                    frames.push_back(pFrame);
                }
            }
            if (frames.empty())
            {
                auto const& layouts(rDoc.GetAllLayouts());
                assert(std::none_of(layouts.begin(), layouts.end(),
                    [](SwRootFrame const*const pLayout) { return pLayout->IsHideRedlines(); }));
                (void) layouts;
                break;
            }
            auto eMode(sw::FrameMode::Existing);
            SwTextNode * pLast(pNode);
            for (SwTextFrame * pFrame : frames)
            {
                SwTextNode & rFirstNode(pFrame->GetMergedPara()
                    ? *pFrame->GetMergedPara()->pFirstNode
                    : *pNode);
                assert(pNode == pStartNode
                        ? rFirstNode.GetIndex() <= pNode->GetIndex()
                        : &rFirstNode == pNode);
                // clear old one first to avoid DelFrames confusing updates & asserts...
                pFrame->SetMergedPara(nullptr);
                pFrame->SetMergedPara(sw::CheckParaRedlineMerge(
                    *pFrame, rFirstNode, eMode));
                eMode = sw::FrameMode::New; // Existing is not idempotent!
                // the first node of the new redline is not necessarily the first
                // node of the merged frame, there could be another redline nearby
                sw::AddRemoveFlysAnchoredToFrameStartingAtNode(*pFrame, *pNode, nullptr);
                // if redline is split across table and table cell is empty, there's no redline in the cell and so no merged para
                if (pFrame->GetMergedPara())
                {
                    pLast = const_cast<SwTextNode*>(pFrame->GetMergedPara()->pLastNode);
                }
            }
            SwNodeIndex tmp(*pLast);
            // skip over hidden sections!
            pNode = static_cast<SwTextNode*>(pLast->GetNodes().GoNextSection(&tmp, /*bSkipHidden=*/true, /*bSkipProtect=*/false));
        }
        while (pNode && pNode->GetIndex() <= rPam.End()->nNode.GetIndex());
    }
    // fields last - SwGetRefField::UpdateField requires up-to-date frames
    UpdateFieldsForRedline(rDoc.getIDocumentFieldsAccess()); // after footnotes

    // update SwPostItMgr / notes in the margin
    rDoc.GetDocShell()->Broadcast(
            SwFormatFieldHint(nullptr, SwFormatFieldHintWhich::REMOVED) );
}

void UpdateFramesForRemoveDeleteRedline(SwDoc & rDoc, SwPaM const& rPam)
{
    bool isAppendObjsCalled(false);
    rDoc.GetFootnoteIdxs().UpdateFootnote(rPam.Start()->nNode);
    SwPosition currentStart(*rPam.Start());
    SwTextNode * pStartNode(rPam.Start()->nNode.GetNode().GetTextNode());
    while (!pStartNode)
    {
        SwStartNode const*const pTableOrSectionNode(
            currentStart.nNode.GetNode().IsTableNode()
                ? static_cast<SwStartNode*>(currentStart.nNode.GetNode().GetTableNode())
                : static_cast<SwStartNode*>(currentStart.nNode.GetNode().GetSectionNode()));
        assert(pTableOrSectionNode); // known pathology
        for (sal_uLong j = pTableOrSectionNode->GetIndex(); j <= pTableOrSectionNode->EndOfSectionIndex(); ++j)
        {
            pTableOrSectionNode->GetNodes()[j]->SetRedlineMergeFlag(SwNode::Merge::None);
        }
        if (rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->IsHideRedlines())
        {
            // note: this will also create frames for all currently hidden flys
            // because it calls AppendAllObjs
            SwNodeIndex const end(*pTableOrSectionNode->EndOfSectionNode());
            ::MakeFrames(&rDoc, currentStart.nNode, end);
            isAppendObjsCalled = true;
        }
        currentStart.nNode = pTableOrSectionNode->EndOfSectionIndex() + 1;
        currentStart.nContent.Assign(currentStart.nNode.GetNode().GetContentNode(), 0);
        pStartNode = currentStart.nNode.GetNode().GetTextNode();
    }
    if (currentStart < *rPam.End())
    {
        SwTextNode * pNode(pStartNode);
        do
        {
            std::vector<SwTextFrame*> frames;
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pNode);
            for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
            {
                if (pFrame->getRootFrame()->IsHideRedlines())
                {
                    frames.push_back(pFrame);
                }
            }
            if (frames.empty())
            {
                auto const& layouts(rDoc.GetAllLayouts());
                assert(std::none_of(layouts.begin(), layouts.end(),
                    [](SwRootFrame const*const pLayout) { return pLayout->IsHideRedlines(); }));
                (void) layouts;
                break;
            }

            // first, call CheckParaRedlineMerge on the first paragraph,
            // to init flag on new merge range (if any) + 1st node post the merge
            auto eMode(sw::FrameMode::Existing);
            SwTextNode * pLast(pNode);
            for (SwTextFrame * pFrame : frames)
            {
                if (auto const pMergedPara = pFrame->GetMergedPara())
                {
                    pLast = const_cast<SwTextNode*>(pMergedPara->pLastNode);
                    assert(pNode == pStartNode
                        ? pMergedPara->pFirstNode->GetIndex() <= pNode->GetIndex()
                        : pMergedPara->pFirstNode == pNode);
                    // clear old one first to avoid DelFrames confusing updates & asserts...
                    SwTextNode & rFirstNode(*pMergedPara->pFirstNode);
                    pFrame->SetMergedPara(nullptr);
                    pFrame->SetMergedPara(sw::CheckParaRedlineMerge(
                        *pFrame, rFirstNode, eMode));
                    eMode = sw::FrameMode::New; // Existing is not idempotent!
                }
            }
            if (pLast != pNode)
            {
                // now start node until end of merge + 1 has proper flags; MakeFrames
                // should pick up from the next node in need of frames by checking flags
                SwNodeIndex const start(*pNode, +1);
                SwNodeIndex const end(*pLast, +1); // end is exclusive
                // note: this will also create frames for all currently hidden flys
                // both on first and non-first nodes because it calls AppendAllObjs
                ::MakeFrames(&rDoc, start, end);
                isAppendObjsCalled = true;
                // re-use this to move flys that are now on the wrong frame, with end
                // of redline as "second" node; the nodes between start and end should
                // be complete with MakeFrames already
                sw::MoveMergedFlysAndFootnotes(frames, *pNode, *pLast, false);
            }
            SwNodeIndex tmp(*pLast);
            // skip over hidden sections!
            pNode = static_cast<SwTextNode*>(pLast->GetNodes().GoNextSection(&tmp, /*bSkipHidden=*/true, /*bSkipProtect=*/false));
        }
        while (pNode && pNode->GetIndex() <= rPam.End()->nNode.GetIndex());
    }

    if (!isAppendObjsCalled)
    {   // recreate flys in the one node the hard way...
        for (auto const& pLayout : rDoc.GetAllLayouts())
        {
            if (pLayout->IsHideRedlines())
            {
                AppendAllObjs(rDoc.GetSpzFrameFormats(), pLayout);
                break;
            }
        }
    }
    // fields last - SwGetRefField::UpdateField requires up-to-date frames
    UpdateFieldsForRedline(rDoc.getIDocumentFieldsAccess()); // after footnotes

    // update SwPostItMgr / notes in the margin
    rDoc.GetDocShell()->Broadcast(
            SwFormatFieldHint(nullptr, SwFormatFieldHintWhich::INSERTED) );
}

} // namespace sw

namespace
{
    bool IsPrevPos( const SwPosition & rPos1, const SwPosition & rPos2 )
    {
        const SwContentNode* pCNd;
        if( 0 != rPos2.nContent.GetIndex() )
            return false;
        if( rPos2.nNode.GetIndex() - 1 != rPos1.nNode.GetIndex() )
            return false;
        pCNd = rPos1.nNode.GetNode().GetContentNode();
        return pCNd && rPos1.nContent.GetIndex() == pCNd->Len();
    }

    // copy style or return with SwRedlineExtra_FormatColl with reject data of the upcoming copy
    SwRedlineExtraData_FormatColl* lcl_CopyStyle( const SwPosition & rFrom, const SwPosition & rTo, bool bCopy = true )
    {
        SwTextNode* pToNode = rTo.nNode.GetNode().GetTextNode();
        SwTextNode* pFromNode = rFrom.nNode.GetNode().GetTextNode();
        if (pToNode != nullptr && pFromNode != nullptr && pToNode != pFromNode)
        {
            const SwPaM aPam(*pToNode);
            SwDoc* pDoc = aPam.GetDoc();
            // using Undo, copy paragraph style
            SwTextFormatColl* pFromColl = pFromNode->GetTextColl();
            SwTextFormatColl* pToColl = pToNode->GetTextColl();
            if (bCopy && pFromColl != pToColl)
                pDoc->SetTextFormatColl(aPam, pFromColl);

            // using Undo, remove direct paragraph formatting of the "To" paragraph,
            // and apply here direct paragraph formatting of the "From" paragraph
            SfxItemSet aTmp(
                pDoc->GetAttrPool(),
                svl::Items<
                    RES_PARATR_BEGIN, RES_PARATR_END - 3, // skip RSID and GRABBAG
                    RES_PARATR_LIST_BEGIN, RES_UL_SPACE,  // skip PAGEDESC and BREAK
                    RES_CNTNT, RES_FRMATR_END - 1>{});
            SfxItemSet aTmp2(aTmp);

            pToNode->GetParaAttr(aTmp, 0, 0);
            pFromNode->GetParaAttr(aTmp2, 0, 0);

            bool bSameSet = aTmp == aTmp2;

            if (!bSameSet)
            {
                for( sal_uInt16 nItem = 0; nItem < aTmp.TotalCount(); ++nItem)
                {
                    sal_uInt16 nWhich = aTmp.GetWhichByPos(nItem);
                    if( SfxItemState::SET == aTmp.GetItemState( nWhich, false ) &&
                        SfxItemState::SET != aTmp2.GetItemState( nWhich, false ) )
                            aTmp2.Put( aTmp.GetPool()->GetDefaultItem(nWhich), nWhich );
                }
            }

            if (bCopy && !bSameSet)
                pDoc->getIDocumentContentOperations().InsertItemSet(aPam, aTmp2);
            else if (!bCopy && (!bSameSet || pFromColl != pToColl))
                return new SwRedlineExtraData_FormatColl( pFromColl->GetName(), USHRT_MAX, &aTmp2 );
        }
        return nullptr;
    }

    bool lcl_AcceptRedline( SwRedlineTable& rArr, SwRedlineTable::size_type& rPos,
                            bool bCallDelete,
                            const SwPosition* pSttRng = nullptr,
                            const SwPosition* pEndRng = nullptr )
    {
        bool bRet = true;
        SwRangeRedline* pRedl = rArr[ rPos ];
        SwPosition *pRStt = nullptr, *pREnd = nullptr;
        SwComparePosition eCmp = SwComparePosition::Outside;
        if( pSttRng && pEndRng )
        {
            pRStt = pRedl->Start();
            pREnd = pRedl->End();
            eCmp = ComparePosition( *pSttRng, *pEndRng, *pRStt, *pREnd );
        }

        pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);

        switch( pRedl->GetType() )
        {
        case RedlineType::Insert:
        case RedlineType::Format:
            {
                bool bCheck = false, bReplace = false;
                switch( eCmp )
                {
                case SwComparePosition::Inside:
                    if( *pSttRng == *pRStt )
                        pRedl->SetStart( *pEndRng, pRStt );
                    else
                    {
                        if( *pEndRng != *pREnd )
                        {
                            // split up
                            SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                            pNew->SetStart( *pEndRng );
                            rArr.Insert( pNew ); ++rPos;
                        }
                        pRedl->SetEnd( *pSttRng, pREnd );
                        bCheck = true;
                    }
                    break;

                case SwComparePosition::OverlapBefore:
                    pRedl->SetStart( *pEndRng, pRStt );
                    bReplace = true;
                    break;

                case SwComparePosition::OverlapBehind:
                    pRedl->SetEnd( *pSttRng, pREnd );
                    bCheck = true;
                    break;

                case SwComparePosition::Outside:
                case SwComparePosition::Equal:
                    rArr.DeleteAndDestroy( rPos-- );
                    break;

                default:
                    bRet = false;
                }

                if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
                {
                    // re-insert
                    rArr.Remove( pRedl );
                    rArr.Insert( pRedl );
                }
            }
            break;
        case RedlineType::Delete:
            {
                SwDoc& rDoc = *pRedl->GetDoc();
                const SwPosition *pDelStt = nullptr, *pDelEnd = nullptr;
                bool bDelRedl = false;
                switch( eCmp )
                {
                case SwComparePosition::Inside:
                    if( bCallDelete )
                    {
                        pDelStt = pSttRng;
                        pDelEnd = pEndRng;
                    }
                    break;

                case SwComparePosition::OverlapBefore:
                    if( bCallDelete )
                    {
                        pDelStt = pRStt;
                        pDelEnd = pEndRng;
                    }
                    break;
                case SwComparePosition::OverlapBehind:
                    if( bCallDelete )
                    {
                        pDelStt = pREnd;
                        pDelEnd = pSttRng;
                    }
                    break;

                case SwComparePosition::Outside:
                case SwComparePosition::Equal:
                    {
                        rArr.Remove( rPos-- );
                        bDelRedl = true;
                        if( bCallDelete )
                        {
                            pDelStt = pRedl->Start();
                            pDelEnd = pRedl->End();
                        }
                    }
                    break;
                default:
                    bRet = false;
                }

                if( pDelStt && pDelEnd )
                {
                    SwPaM aPam( *pDelStt, *pDelEnd );
                    SwContentNode* pCSttNd = pDelStt->nNode.GetNode().GetContentNode();
                    SwContentNode* pCEndNd = pDelEnd->nNode.GetNode().GetContentNode();
                    pRStt = pRedl->Start();
                    pREnd = pRedl->End();

                    // keep style of the empty paragraph after deletion of wholly paragraphs
                    if( pCSttNd && pCEndNd && pRStt && pREnd && pRStt->nContent == 0 )
                        lcl_CopyStyle(*pREnd, *pRStt);

                    if( bDelRedl )
                        delete pRedl;

                    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
                    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

                    if( pCSttNd && pCEndNd )
                        rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                    else if (pCSttNd && !pCEndNd)
                        {
                            aPam.GetBound().nContent.Assign( nullptr, 0 );
                            aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
                            rDoc.getIDocumentContentOperations().DelFullPara( aPam );
                        }
                    else
                    {
                        rDoc.getIDocumentContentOperations().DeleteRange(aPam);
                    }
                    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
                }
                else if( bDelRedl )
                    delete pRedl;
            }
            break;

        case RedlineType::FmtColl:
            rArr.DeleteAndDestroy( rPos-- );
            break;

        case RedlineType::ParagraphFormat:
            rArr.DeleteAndDestroy( rPos-- );
            break;

        default:
            bRet = false;
        }
        return bRet;
    }

    bool lcl_RejectRedline( SwRedlineTable& rArr, SwRedlineTable::size_type& rPos,
                            bool bCallDelete,
                            const SwPosition* pSttRng = nullptr,
                            const SwPosition* pEndRng = nullptr )
    {
        bool bRet = true;
        SwRangeRedline* pRedl = rArr[ rPos ];
        SwDoc& rDoc = *pRedl->GetDoc();
        SwPosition *pRStt = nullptr, *pREnd = nullptr;
        SwComparePosition eCmp = SwComparePosition::Outside;
        if( pSttRng && pEndRng )
        {
            pRStt = pRedl->Start();
            pREnd = pRedl->End();
            eCmp = ComparePosition( *pSttRng, *pEndRng, *pRStt, *pREnd );
        }

        pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);

        switch( pRedl->GetType() )
        {
        case RedlineType::Insert:
            {
                const SwPosition *pDelStt = nullptr, *pDelEnd = nullptr;
                bool bDelRedl = false;
                switch( eCmp )
                {
                case SwComparePosition::Inside:
                    if( bCallDelete )
                    {
                        pDelStt = pSttRng;
                        pDelEnd = pEndRng;
                    }
                    break;

                case SwComparePosition::OverlapBefore:
                    if( bCallDelete )
                    {
                        pDelStt = pRStt;
                        pDelEnd = pEndRng;
                    }
                    break;
                case SwComparePosition::OverlapBehind:
                    if( bCallDelete )
                    {
                        pDelStt = pREnd;
                        pDelEnd = pSttRng;
                    }
                    break;
                case SwComparePosition::Outside:
                case SwComparePosition::Equal:
                    {
                        // delete the range again
                        rArr.Remove( rPos-- );
                        bDelRedl = true;
                        if( bCallDelete )
                        {
                            pDelStt = pRedl->Start();
                            pDelEnd = pRedl->End();
                        }
                    }
                    break;

                default:
                    bRet = false;
                }
                if( pDelStt && pDelEnd )
                {
                    SwPaM aPam( *pDelStt, *pDelEnd );

                    SwContentNode* pCSttNd = pDelStt->nNode.GetNode().GetContentNode();
                    SwContentNode* pCEndNd = pDelEnd->nNode.GetNode().GetContentNode();

                    if( bDelRedl )
                        delete pRedl;

                    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
                    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

                    if( pCSttNd && pCEndNd )
                        rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                    else if (pCSttNd && !pCEndNd)
                        {
                            aPam.GetBound().nContent.Assign( nullptr, 0 );
                            aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
                            rDoc.getIDocumentContentOperations().DelFullPara( aPam );
                        }
                    else
                    {
                        rDoc.getIDocumentContentOperations().DeleteRange(aPam);
                    }
                    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
                }
                else if( bDelRedl )
                    delete pRedl;
            }
            break;
        case RedlineType::Delete:
            {
                SwRangeRedline* pNew = nullptr;
                bool bCheck = false, bReplace = false;
                SwPaM const updatePaM(pSttRng ? *pSttRng : *pRedl->Start(),
                                      pEndRng ? *pEndRng : *pRedl->End());

                if( pRedl->GetExtraData() )
                    pRedl->GetExtraData()->Reject( *pRedl );

                switch( eCmp )
                {
                case SwComparePosition::Inside:
                    {
                        if( 1 < pRedl->GetStackCount() )
                        {
                            pNew = new SwRangeRedline( *pRedl );
                            pNew->PopData();
                        }
                        if( *pSttRng == *pRStt )
                        {
                            pRedl->SetStart( *pEndRng, pRStt );
                            bReplace = true;
                            if( pNew )
                                pNew->SetEnd( *pEndRng );
                        }
                        else
                        {
                            if( *pEndRng != *pREnd )
                            {
                                // split up
                                SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                                pCpy->SetStart( *pEndRng );
                                rArr.Insert( pCpy ); ++rPos;
                                if( pNew )
                                    pNew->SetEnd( *pEndRng );
                            }

                            pRedl->SetEnd( *pSttRng, pREnd );
                            bCheck = true;
                            if( pNew )
                                pNew->SetStart( *pSttRng );
                        }
                    }
                    break;

                case SwComparePosition::OverlapBefore:
                    if( 1 < pRedl->GetStackCount() )
                    {
                        pNew = new SwRangeRedline( *pRedl );
                        pNew->PopData();
                    }
                    pRedl->SetStart( *pEndRng, pRStt );
                    bReplace = true;
                    if( pNew )
                        pNew->SetEnd( *pEndRng );
                    break;

                case SwComparePosition::OverlapBehind:
                    if( 1 < pRedl->GetStackCount() )
                    {
                        pNew = new SwRangeRedline( *pRedl );
                        pNew->PopData();
                    }
                    pRedl->SetEnd( *pSttRng, pREnd );
                    bCheck = true;
                    if( pNew )
                        pNew->SetStart( *pSttRng );
                    break;

                case SwComparePosition::Outside:
                case SwComparePosition::Equal:
                    if( !pRedl->PopData() )
                        // deleting the RedlineObject is enough
                        rArr.DeleteAndDestroy( rPos-- );
                    break;

                default:
                    bRet = false;
                }

                if( pNew )
                {
                    rArr.Insert( pNew ); ++rPos;
                }

                if( bReplace || ( bCheck && !pRedl->HasValidRange() ))
                {
                    // re-insert
                    rArr.Remove( pRedl );
                    rArr.Insert( pRedl );
                }

                sw::UpdateFramesForRemoveDeleteRedline(rDoc, updatePaM);
            }
            break;

        case RedlineType::Format:
        case RedlineType::FmtColl:
        case RedlineType::ParagraphFormat:
            {
                // tdf#52391 instead of hidden acception at the requested
                // rejection, remove direct text formatting to get the potential
                // original state of the text (FIXME if the original text
                // has already contained direct text formatting: unfortunately
                // ODF 1.2 doesn't support rejection of format-only changes)
                if ( pRedl->GetType() == RedlineType::Format )
                {
                    SwPaM aPam( *(pRedl->Start()), *(pRedl->End()) );
                    rDoc.ResetAttrs(aPam);
                }
                else if ( pRedl->GetType() == RedlineType::ParagraphFormat )
                {
                    // handle paragraph formatting changes
                    // (range is only a full paragraph or a part of it)
                    const SwPosition* pStt = pRedl->Start();
                    SwTextNode* pTNd = pStt->nNode.GetNode().GetTextNode();
                    if( pTNd )
                    {
                        // expand range to the whole paragraph
                        // and reset only the paragraph attributes
                        SwPaM aPam( *pTNd, pTNd->GetText().getLength() );
                        std::set<sal_uInt16> aResetAttrsArray;

                        sal_uInt16 aResetableSetRange[] = {
                                RES_PARATR_BEGIN, RES_PARATR_END - 1,
                                RES_PARATR_LIST_BEGIN, RES_FRMATR_END - 1,
                                0
                        };

                        const sal_uInt16 *pUShorts = aResetableSetRange;
                        while (*pUShorts)
                        {
                            for (sal_uInt16 i = pUShorts[0]; i <= pUShorts[1]; ++i)
                                aResetAttrsArray.insert( aResetAttrsArray.end(), i );
                            pUShorts += 2;
                        }

                        rDoc.ResetAttrs(aPam, false, aResetAttrsArray);

                        // remove numbering
                        if ( pTNd->GetNumRule() )
                            rDoc.DelNumRules(aPam);
                    }
                }

                if( pRedl->GetExtraData() )
                    pRedl->GetExtraData()->Reject( *pRedl );

                rArr.DeleteAndDestroy( rPos-- );
            }
            break;

        default:
            bRet = false;
        }
        return bRet;
    }

    typedef bool (*Fn_AcceptReject)( SwRedlineTable& rArr, SwRedlineTable::size_type& rPos,
                            bool bCallDelete,
                            const SwPosition* pSttRng,
                            const SwPosition* pEndRng);


    int lcl_AcceptRejectRedl( Fn_AcceptReject fn_AcceptReject,
                                SwRedlineTable& rArr, bool bCallDelete,
                                const SwPaM& rPam)
    {
        SwRedlineTable::size_type n = 0;
        int nCount = 0;

        const SwPosition* pStt = rPam.Start(),
                        * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                         : rPam.GetPoint();
        const SwRangeRedline* pFnd = rArr.FindAtPosition( *pStt, n );
        if( pFnd &&     // Is new a part of it?
            ( *pFnd->Start() != *pStt || *pFnd->End() > *pEnd ))
        {
            // Only revoke the partial selection
            if( (*fn_AcceptReject)( rArr, n, bCallDelete, pStt, pEnd ))
                nCount++;
            ++n;
        }

        // tdf#119824 first we will accept only overlapping paragraph format changes
        // in the first loop to avoid potential content changes during Redo
        bool bHasParagraphFormatChange = false;
        for( int m = 0 ; m < 2 && !bHasParagraphFormatChange; ++m )
        {
            for(SwRedlineTable::size_type o = n ; o < rArr.size(); ++o )
            {
                SwRangeRedline* pTmp = rArr[ o ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    if( *pTmp->End() <= *pEnd )
                    {
                        if( (m > 0 || RedlineType::ParagraphFormat == pTmp->GetType()) &&
                            (*fn_AcceptReject)( rArr, o, bCallDelete, nullptr, nullptr ))
                        {
                            bHasParagraphFormatChange = true;
                            nCount++;
                        }
                    }
                    else
                    {
                        if( *pTmp->Start() < *pEnd )
                        {
                            // Only revoke the partial selection
                            if( (m > 0 || RedlineType::ParagraphFormat == pTmp->GetType()) &&
                                (*fn_AcceptReject)( rArr, o, bCallDelete, pStt, pEnd ))
                            {
                                bHasParagraphFormatChange = true;
                                nCount++;
                            }
                        }
                        break;
                    }
                }
            }
        }
        return nCount;
    }

    void lcl_AdjustRedlineRange( SwPaM& rPam )
    {
        // The Selection is only in the ContentSection. If there are Redlines
        // to Non-ContentNodes before or after that, then the Selections
        // expand to them.
        SwPosition* pStt = rPam.Start(),
                  * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                   : rPam.GetPoint();
        SwDoc* pDoc = rPam.GetDoc();
        if( !pStt->nContent.GetIndex() &&
            !pDoc->GetNodes()[ pStt->nNode.GetIndex() - 1 ]->IsContentNode() )
        {
            const SwRangeRedline* pRedl = pDoc->getIDocumentRedlineAccess().GetRedline( *pStt, nullptr );
            if( pRedl )
            {
                const SwPosition* pRStt = pRedl->Start();
                if( !pRStt->nContent.GetIndex() && pRStt->nNode.GetIndex() ==
                    pStt->nNode.GetIndex() - 1 )
                    *pStt = *pRStt;
            }
        }
        if( pEnd->nNode.GetNode().IsContentNode() &&
            !pDoc->GetNodes()[ pEnd->nNode.GetIndex() + 1 ]->IsContentNode() &&
            pEnd->nContent.GetIndex() == pEnd->nNode.GetNode().GetContentNode()->Len()    )
        {
            const SwRangeRedline* pRedl = pDoc->getIDocumentRedlineAccess().GetRedline( *pEnd, nullptr );
            if( pRedl )
            {
                const SwPosition* pREnd = pRedl->End();
                if( !pREnd->nContent.GetIndex() && pREnd->nNode.GetIndex() ==
                    pEnd->nNode.GetIndex() + 1 )
                    *pEnd = *pREnd;
            }
        }
    }

    /// in case some text is deleted, ensure that the not-yet-inserted
    /// SwRangeRedline has its positions corrected not to point to deleted node
    class TemporaryRedlineUpdater
    {
    private:
        SwRangeRedline & m_rRedline;
        std::shared_ptr<SwUnoCursor> m_pCursor;
    public:
        TemporaryRedlineUpdater(SwDoc & rDoc, SwRangeRedline & rRedline)
            : m_rRedline(rRedline)
            , m_pCursor(rDoc.CreateUnoCursor(*rRedline.GetPoint(), false))
        {
            if (m_rRedline.HasMark())
            {
                m_pCursor->SetMark();
                *m_pCursor->GetMark() = *m_rRedline.GetMark();
                *m_rRedline.GetMark() = SwPosition(rDoc.GetNodes().GetEndOfContent());
            }
            *m_rRedline.GetPoint() = SwPosition(rDoc.GetNodes().GetEndOfContent());
        }
        ~TemporaryRedlineUpdater()
        {
            static_cast<SwPaM&>(m_rRedline) = *m_pCursor;
        }
    };
}

namespace sw
{

DocumentRedlineManager::DocumentRedlineManager(SwDoc& i_rSwdoc)
    : m_rDoc(i_rSwdoc)
    , meRedlineFlags(RedlineFlags::ShowInsert | RedlineFlags::ShowDelete)
    , mpRedlineTable(new SwRedlineTable)
    , mpExtraRedlineTable(new SwExtraRedlineTable)
    , mbIsRedlineMove(false)
    , mnAutoFormatRedlnCommentNo(0)
{
}

RedlineFlags DocumentRedlineManager::GetRedlineFlags() const
{
    return meRedlineFlags;
}

void DocumentRedlineManager::SetRedlineFlags( RedlineFlags eMode )
{
    if( meRedlineFlags == eMode )
        return;

    if( (RedlineFlags::ShowMask & meRedlineFlags) != (RedlineFlags::ShowMask & eMode)
        || !(RedlineFlags::ShowMask & eMode) )
    {
        bool bSaveInXMLImportFlag = m_rDoc.IsInXMLImport();
        m_rDoc.SetInXMLImport( false );
        // and then hide/display everything
        void (SwRangeRedline::*pFnc)(sal_uInt16, size_t); // Allow compiler warn if use of
                                                          // uninitialized ptr is possible

        RedlineFlags eShowMode = RedlineFlags::ShowMask & eMode;
        if (eShowMode == (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete))
            pFnc = &SwRangeRedline::Show;
        else if (eShowMode == RedlineFlags::ShowInsert)
            pFnc = &SwRangeRedline::Hide;
        else if (eShowMode == RedlineFlags::ShowDelete)
            pFnc = &SwRangeRedline::ShowOriginal;
        else
        {
            pFnc = &SwRangeRedline::Hide;
            eMode |= RedlineFlags::ShowInsert;
        }

        CheckAnchoredFlyConsistency(m_rDoc);
        CHECK_REDLINE( *this )

        o3tl::sorted_vector<SwRootFrame *> hiddenLayouts;
        if (eShowMode == (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete))
        {
            // sw_redlinehide: the problem here is that MoveFromSection
            // creates the frames wrongly (non-merged), because its own
            // SwRangeRedline has wrong positions until after the nodes
            // are all moved, so fix things up by force by re-creating
            // all merged frames from scratch.
            o3tl::sorted_vector<SwRootFrame *> const layouts(m_rDoc.GetAllLayouts());
            for (SwRootFrame *const pLayout : layouts)
            {
                if (pLayout->IsHideRedlines())
                {
                    pLayout->SetHideRedlines(false);
                    hiddenLayouts.insert(pLayout);
                }
            }
        }

        for (sal_uInt16 nLoop = 1; nLoop <= 2; ++nLoop)
            for (size_t i = 0; i < mpRedlineTable->size(); ++i)
            {
                SwRangeRedline *const pRedline((*mpRedlineTable)[i]);
                (pRedline->*pFnc)(nLoop, i);
                while (mpRedlineTable->size() <= i
                    || (*mpRedlineTable)[i] != pRedline)
                {        // ensure current position
                    --i; // a previous redline may have been deleted
                }
            }

        //SwRangeRedline::MoveFromSection routinely changes
        //the keys that mpRedlineTable is sorted by
        mpRedlineTable->Resort();

        CheckAnchoredFlyConsistency(m_rDoc);
        CHECK_REDLINE( *this )

        for (SwRootFrame *const pLayout : hiddenLayouts)
        {
            pLayout->SetHideRedlines(true);
        }

        m_rDoc.SetInXMLImport( bSaveInXMLImportFlag );
    }
    meRedlineFlags = eMode;
    m_rDoc.getIDocumentState().SetModified();

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::IsRedlineOn() const
{
    return IDocumentRedlineAccess::IsRedlineOn(meRedlineFlags);
}

bool DocumentRedlineManager::IsIgnoreRedline() const
{
    return bool(RedlineFlags::Ignore & meRedlineFlags);
}

void DocumentRedlineManager::SetRedlineFlags_intern(RedlineFlags eMode)
{
    meRedlineFlags = eMode;
}

const SwRedlineTable& DocumentRedlineManager::GetRedlineTable() const
{
    return *mpRedlineTable;
}

SwRedlineTable& DocumentRedlineManager::GetRedlineTable()
{
    return *mpRedlineTable;
}

const SwExtraRedlineTable& DocumentRedlineManager::GetExtraRedlineTable() const
{
    return *mpExtraRedlineTable;
}

SwExtraRedlineTable& DocumentRedlineManager::GetExtraRedlineTable()
{
    return *mpExtraRedlineTable;
}

bool DocumentRedlineManager::HasExtraRedlineTable() const
{
    return mpExtraRedlineTable != nullptr;
}

bool DocumentRedlineManager::IsInRedlines(const SwNode & rNode) const
{
    SwPosition aPos(rNode);
    SwNode & rEndOfRedlines = m_rDoc.GetNodes().GetEndOfRedlines();
    SwPaM aPam(SwPosition(*rEndOfRedlines.StartOfSectionNode()),
               SwPosition(rEndOfRedlines));

    return aPam.ContainsPosition(aPos);
}

bool DocumentRedlineManager::IsRedlineMove() const
{
    return mbIsRedlineMove;
}

void DocumentRedlineManager::SetRedlineMove(bool bFlag)
{
    mbIsRedlineMove = bFlag;
}

/*
Text means Text not "polluted" by Redlines.

Behaviour of Insert-Redline:
    - in the Text                       - insert Redline Object
    - in InsertRedline (own)            - ignore, existing is extended
    - in InsertRedline (others)         - split up InsertRedline and
                                          insert Redline Object
    - in DeleteRedline                  - split up DeleteRedline or
                                          move at the end/beginning

Behaviour of Delete-Redline:
    - in the Text                       - insert Redline Object
    - in DeleteRedline (own/others)     - ignore
    - in InsertRedline (own)            - ignore, but delete character
    - in InsertRedline (others)         - split up InsertRedline and
                                          insert Redline Object
    - Text and own Insert overlap       - delete Text in the own Insert,
                                          extend in the other Text
                                          (up to the Insert!)
    - Text and other Insert overlap     - insert Redline Object, the
                                          other Insert is overlapped by
                                          the Delete
*/
IDocumentRedlineAccess::AppendResult
DocumentRedlineManager::AppendRedline(SwRangeRedline* pNewRedl, bool const bCallDelete)
{
    bool bMerged = false;
    CHECK_REDLINE( *this )

    if (IsRedlineOn() && !IsShowOriginal(meRedlineFlags))
    {
        pNewRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);

        if( m_rDoc.IsAutoFormatRedline() )
        {
            pNewRedl->SetAutoFormat();
            if( mpAutoFormatRedlnComment && !mpAutoFormatRedlnComment->isEmpty() )
            {
                pNewRedl->SetComment( *mpAutoFormatRedlnComment );
                pNewRedl->SetSeqNo( mnAutoFormatRedlnCommentNo );
            }
        }

        SwPosition* pStt = pNewRedl->Start(),
                  * pEnd = pStt == pNewRedl->GetPoint() ? pNewRedl->GetMark()
                                                        : pNewRedl->GetPoint();
        {
            SwTextNode* pTextNode = pStt->nNode.GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                if( pStt->nContent > 0 )
                {
                    OSL_ENSURE( false, "Redline start: non-text-node with content" );
                    pStt->nContent = 0;
                }
            }
            else
            {
                if( pStt->nContent > pTextNode->Len() )
                {
                    OSL_ENSURE( false, "Redline start: index after text" );
                    pStt->nContent = pTextNode->Len();
                }
            }
            pTextNode = pEnd->nNode.GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                if( pEnd->nContent > 0 )
                {
                    OSL_ENSURE( false, "Redline end: non-text-node with content" );
                    pEnd->nContent = 0;
                }
            }
            else
            {
                if( pEnd->nContent > pTextNode->Len() )
                {
                    OSL_ENSURE( false, "Redline end: index after text" );
                    pEnd->nContent = pTextNode->Len();
                }
            }
        }
        if( ( *pStt == *pEnd ) &&
            ( pNewRedl->GetContentIdx() == nullptr ) )
        {   // Do not insert empty redlines
            delete pNewRedl;
            return AppendResult::IGNORED;
        }
        bool bCompress = false;
        SwRedlineTable::size_type n = 0;
        // look up the first Redline for the starting position
        if( !GetRedline( *pStt, &n ) && n )
            --n;
        bool bDec = false;

        for( ; pNewRedl && n < mpRedlineTable->size(); bDec ? n : ++n )
        {
            bDec = false;

            SwRangeRedline* pRedl = (*mpRedlineTable)[ n ];
            SwPosition* pRStt = pRedl->Start(),
                      * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                           : pRedl->GetPoint();

            // #i8518# remove empty redlines while we're at it
            if( ( *pRStt == *pREnd ) &&
                ( pRedl->GetContentIdx() == nullptr ) )
            {
                mpRedlineTable->DeleteAndDestroy(n);
                continue;
            }

            SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );

            switch( pNewRedl->GetType() )
            {
            case RedlineType::Insert:
                switch( pRedl->GetType() )
                {
                case RedlineType::Insert:
                    if( pRedl->IsOwnRedline( *pNewRedl ) )
                    {
                        bool bDelete = false;

                        // Merge if applicable?
                        if( (( SwComparePosition::Behind == eCmpPos &&
                               IsPrevPos( *pREnd, *pStt ) ) ||
                             ( SwComparePosition::CollideStart == eCmpPos ) ||
                             ( SwComparePosition::OverlapBehind == eCmpPos ) ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( n+1 >= mpRedlineTable->size() ||
                             ( *(*mpRedlineTable)[ n+1 ]->Start() >= *pEnd &&
                             *(*mpRedlineTable)[ n+1 ]->Start() != *pREnd ) ) )
                        {
                            pRedl->SetEnd( *pEnd, pREnd );
                            if( !pRedl->HasValidRange() )
                            {
                                // re-insert
                                mpRedlineTable->Remove( n );
                                mpRedlineTable->Insert( pRedl );
                            }

                            bMerged = true;
                            bDelete = true;
                        }
                        else if( (( SwComparePosition::Before == eCmpPos &&
                                    IsPrevPos( *pEnd, *pRStt ) ) ||
                                   ( SwComparePosition::CollideEnd == eCmpPos ) ||
                                  ( SwComparePosition::OverlapBefore == eCmpPos ) ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            ( !n ||
                             *(*mpRedlineTable)[ n-1 ]->End() != *pRStt ))
                        {
                            pRedl->SetStart( *pStt, pRStt );
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl );

                            bMerged = true;
                            bDelete = true;
                        }
                        else if ( SwComparePosition::Outside == eCmpPos )
                        {
                            // own insert-over-insert redlines:
                            // just scrap the inside ones
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( SwComparePosition::OverlapBehind == eCmpPos )
                        {
                            *pStt = *pREnd;
                            if( ( *pStt == *pEnd ) &&
                                ( pNewRedl->GetContentIdx() == nullptr ) )
                                bDelete = true;
                        }
                        else if( SwComparePosition::OverlapBefore == eCmpPos )
                        {
                            *pEnd = *pRStt;
                            if( ( *pStt == *pEnd ) &&
                                ( pNewRedl->GetContentIdx() == nullptr ) )
                                bDelete = true;
                        }
                        else if( SwComparePosition::Inside == eCmpPos )
                        {
                            bDelete = true;
                            bMerged = true;
                        }
                        else if( SwComparePosition::Equal == eCmpPos )
                            bDelete = true;

                        if( bDelete )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    else if( SwComparePosition::Inside == eCmpPos )
                    {
                        // split up
                        if( *pEnd != *pREnd )
                        {
                            SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            mpRedlineTable->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( ( *pStt == *pRStt ) &&
                            ( pRedl->GetContentIdx() == nullptr ) )
                        {
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl );
                        }
                    }
                    else if ( SwComparePosition::Outside == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents

                        // split up the new redline, since it covers the
                        // existing redline. Insert the first part, and
                        // progress with the remainder as usual
                        SwRangeRedline* pSplit = new SwRangeRedline( *pNewRedl );
                        pSplit->SetEnd( *pRStt );
                        pNewRedl->SetStart( *pREnd );
                        mpRedlineTable->Insert( pSplit );
                        if( *pStt == *pEnd && pNewRedl->GetContentIdx() == nullptr )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    else if ( SwComparePosition::OverlapBehind == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents
                        pNewRedl->SetStart( *pREnd );
                    }
                    else if ( SwComparePosition::OverlapBefore == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents
                        *pEnd = *pRStt;
                        if( ( *pStt == *pEnd ) &&
                            ( pNewRedl->GetContentIdx() == nullptr ) )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    break;
                case RedlineType::Delete:
                    if( SwComparePosition::Inside == eCmpPos )
                    {
                        // split up
                        if( *pEnd != *pREnd )
                        {
                            SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                            pCpy->SetStart( *pEnd );
                            mpRedlineTable->Insert( pCpy );
                        }
                        pRedl->SetEnd( *pStt, pREnd );
                        if( ( *pStt == *pRStt ) &&
                            ( pRedl->GetContentIdx() == nullptr ) )
                        {
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl, n );
                        }
                    }
                    else if ( SwComparePosition::Outside == eCmpPos )
                    {
                        // handle overlapping redlines in broken documents

                        // split up the new redline, since it covers the
                        // existing redline. Insert the first part, and
                        // progress with the remainder as usual
                        SwRangeRedline* pSplit = new SwRangeRedline( *pNewRedl );
                        pSplit->SetEnd( *pRStt );
                        pNewRedl->SetStart( *pREnd );
                        mpRedlineTable->Insert( pSplit );
                        if( *pStt == *pEnd && pNewRedl->GetContentIdx() == nullptr )
                        {
                            delete pNewRedl;
                            pNewRedl = nullptr;
                            bCompress = true;
                        }
                    }
                    else if ( SwComparePosition::Equal == eCmpPos )
                    {
                        // handle identical redlines in broken documents
                        // delete old (delete) redline
                        mpRedlineTable->DeleteAndDestroy( n );
                        bDec = true;
                    }
                    else if ( SwComparePosition::OverlapBehind == eCmpPos )
                    {   // Another workaround for broken redlines
                        pNewRedl->SetStart( *pREnd );
                    }
                    break;
                case RedlineType::Format:
                    switch( eCmpPos )
                    {
                    case SwComparePosition::OverlapBefore:
                        pRedl->SetStart( *pEnd, pRStt );
                        // re-insert
                        mpRedlineTable->Remove( n );
                        mpRedlineTable->Insert( pRedl, n );
                        bDec = true;
                        break;

                    case SwComparePosition::OverlapBehind:
                        pRedl->SetEnd( *pStt, pREnd );
                        if( *pStt == *pRStt && pRedl->GetContentIdx() == nullptr )
                        {
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;

                    case SwComparePosition::Equal:
                    case SwComparePosition::Outside:
                        // Overlaps the current one completely or has the
                        // same dimension, delete the old one
                        mpRedlineTable->DeleteAndDestroy( n );
                        bDec = true;
                        break;

                    case SwComparePosition::Inside:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                                pNew->SetStart( *pEnd );
                                pRedl->SetEnd( *pStt, pREnd );
                                if( *pStt == *pRStt && pRedl->GetContentIdx() == nullptr )
                                    mpRedlineTable->DeleteAndDestroy( n );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                        }
                        else
                            pRedl->SetEnd( *pStt, pREnd );
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;

            case RedlineType::Delete:
                switch( pRedl->GetType() )
                {
                case RedlineType::Delete:
                    switch( eCmpPos )
                    {
                    case SwComparePosition::Outside:
                        {
                            // Overlaps the current one completely,
                            // split the new one
                            if (*pEnd == *pREnd)
                            {
                                pNewRedl->SetEnd(*pRStt, pEnd);
                            }
                            else if (*pStt == *pRStt)
                            {
                                pNewRedl->SetStart(*pREnd, pStt);
                            }
                            else
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pNewRedl );
                                pNew->SetStart( *pREnd );
                                pNewRedl->SetEnd( *pRStt, pEnd );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                        }
                        break;

                    case SwComparePosition::Inside:
                    case SwComparePosition::Equal:
                        delete pNewRedl;
                        pNewRedl = nullptr;
                        bCompress = true;
                        break;

                    case SwComparePosition::OverlapBefore:
                    case SwComparePosition::OverlapBehind:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            if( SwComparePosition::OverlapBehind == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        else if( SwComparePosition::OverlapBehind == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case SwComparePosition::CollideStart:
                    case SwComparePosition::CollideEnd:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) )
                        {
                            if( IsHideChanges( meRedlineFlags ))
                            {
                                // Before we can merge, we make it visible!
                                // We insert temporarily so that pNew is
                                // also dealt with when moving the indices.
                                mpRedlineTable->Insert(pNewRedl);
                                pRedl->Show(0, mpRedlineTable->GetPos(pRedl));
                                mpRedlineTable->Remove( pNewRedl );
                                pRStt = pRedl->Start();
                                pREnd = pRedl->End();
                            }

                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            if( SwComparePosition::CollideStart == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );

                            // delete current (below), and restart process with
                            // previous
                            SwRedlineTable::size_type nToBeDeleted = n;
                            bDec = true;

                            if( *(pNewRedl->Start()) <= *pREnd )
                            {
                                // Whoooah, we just extended the new 'redline'
                                // beyond previous redlines, so better start
                                // again. Of course this is not supposed to
                                // happen, and in an ideal world it doesn't,
                                // but unfortunately this code is buggy and
                                // totally rotten so it does happen and we
                                // better fix it.
                                n = 0;
                                bDec = true;
                            }

                            mpRedlineTable->DeleteAndDestroy( nToBeDeleted );
                        }
                        break;
                    default:
                        break;
                    }
                    break;

                case RedlineType::Insert:
                {
                    // b62341295: Do not throw away redlines
                    // even if they are not allowed to be combined
                    RedlineFlags eOld = meRedlineFlags;
                    if( !( eOld & RedlineFlags::DontCombineRedlines ) &&
                        pRedl->IsOwnRedline( *pNewRedl ) )
                    {

                        // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
                        // The ShowMode needs to be retained!
                        meRedlineFlags = eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore);
                        switch( eCmpPos )
                        {
                        case SwComparePosition::Equal:
                            bCompress = true;
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                            [[fallthrough]];

                        case SwComparePosition::Inside:
                            if( bCallDelete )
                            {
                                // DeleteAndJoin does not yield the
                                // desired result if there is no paragraph to
                                // join with, i.e. at the end of the document.
                                // For this case, we completely delete the
                                // paragraphs (if, of course, we also start on
                                // a paragraph boundary).
                                if( (pStt->nContent == 0) &&
                                    pEnd->nNode.GetNode().IsEndNode() )
                                {
                                    pEnd->nNode--;
                                    pEnd->nContent.Assign(
                                        pEnd->nNode.GetNode().GetTextNode(), 0);
                                    m_rDoc.getIDocumentContentOperations().DelFullPara( *pNewRedl );
                                }
                                else
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pNewRedl );

                                bCompress = true;
                            }
                            if( !bCallDelete && !bDec && *pEnd == *pREnd )
                            {
                                m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pNewRedl );
                                bCompress = true;
                            }
                            else if ( bCallDelete || !bDec )
                            {
                                // delete new redline, except in some cases of fallthrough from previous
                                // case ::Equal (eg. same portion w:del in w:ins in OOXML import)
                                delete pNewRedl;
                                pNewRedl = nullptr;
                            }
                            break;

                        case SwComparePosition::Outside:
                            {
                                mpRedlineTable->Remove( n );
                                bDec = true;
                                if( bCallDelete )
                                {
                                    TemporaryRedlineUpdater const u(m_rDoc, *pNewRedl);
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pRedl );
                                    n = 0;      // re-initialize
                                }
                                delete pRedl;
                            }
                            break;

                        case SwComparePosition::OverlapBefore:
                            {
                                SwPaM aPam( *pRStt, *pEnd );

                                if( *pEnd == *pREnd )
                                    mpRedlineTable->DeleteAndDestroy( n );
                                else
                                {
                                    pRedl->SetStart( *pEnd, pRStt );
                                    // re-insert
                                    mpRedlineTable->Remove( n );
                                    mpRedlineTable->Insert( pRedl, n );
                                }

                                if( bCallDelete )
                                {
                                    TemporaryRedlineUpdater const u(m_rDoc, *pNewRedl);
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                                    n = 0;      // re-initialize
                                }
                                bDec = true;
                            }
                            break;

                        case SwComparePosition::OverlapBehind:
                            {
                                SwPaM aPam( *pStt, *pREnd );

                                if( *pStt == *pRStt )
                                {
                                    mpRedlineTable->DeleteAndDestroy( n );
                                    bDec = true;
                                }
                                else
                                    pRedl->SetEnd( *pStt, pREnd );

                                if( bCallDelete )
                                {
                                    TemporaryRedlineUpdater const u(m_rDoc, *pNewRedl);
                                    m_rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                                    n = 0;      // re-initialize
                                    bDec = true;
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        meRedlineFlags = eOld;
                    }
                    else
                    {
                        // it may be necessary to split the existing redline in
                        // two. In this case, pRedl will be changed to cover
                        // only part of its former range, and pNew will cover
                        // the remainder.
                        SwRangeRedline* pNew = nullptr;

                        switch( eCmpPos )
                        {
                        case SwComparePosition::Equal:
                            {
                                pRedl->PushData( *pNewRedl );
                                delete pNewRedl;
                                pNewRedl = nullptr;
                                if( IsHideChanges( meRedlineFlags ))
                                {
                                    pRedl->Hide(0, mpRedlineTable->GetPos(pRedl));
                                }
                                bCompress = true;
                            }
                            break;

                        case SwComparePosition::Inside:
                            {
                                if( *pRStt == *pStt )
                                {
                                    // #i97421#
                                    // redline w/out extent loops
                                    if (*pStt != *pEnd)
                                    {
                                        pNewRedl->PushData( *pRedl, false );
                                        pRedl->SetStart( *pEnd, pRStt );
                                        // re-insert
                                        mpRedlineTable->Remove( n );
                                        mpRedlineTable->Insert( pRedl, n );
                                        bDec = true;
                                    }
                                }
                                else
                                {
                                    pNewRedl->PushData( *pRedl, false );
                                    if( *pREnd != *pEnd )
                                    {
                                        pNew = new SwRangeRedline( *pRedl );
                                        pNew->SetStart( *pEnd );
                                    }
                                    pRedl->SetEnd( *pStt, pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // re-insert
                                        mpRedlineTable->Remove( n );
                                        mpRedlineTable->Insert( pRedl, n );
                                    }
                                }
                            }
                            break;

                        case SwComparePosition::Outside:
                            {
                                pRedl->PushData( *pNewRedl );
                                if( *pEnd == *pREnd )
                                {
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                }
                                else if (*pStt == *pRStt)
                                {
                                    pNewRedl->SetStart(*pREnd, pStt);
                                }
                                else
                                {
                                    pNew = new SwRangeRedline( *pNewRedl );
                                    pNew->SetEnd( *pRStt );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                }
                                bCompress = true;
                            }
                            break;

                        case SwComparePosition::OverlapBefore:
                            {
                                if( *pEnd == *pREnd )
                                {
                                    pRedl->PushData( *pNewRedl );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    if( IsHideChanges( meRedlineFlags ))
                                    {
                                        mpRedlineTable->Insert(pNewRedl);
                                        pRedl->Hide(0, mpRedlineTable->GetPos(pRedl));
                                        mpRedlineTable->Remove( pNewRedl );
                                    }
                                }
                                else
                                {
                                    pNew = new SwRangeRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetEnd( *pEnd );
                                    pNewRedl->SetEnd( *pRStt, pEnd );
                                    pRedl->SetStart( *pNew->End(), pRStt ) ;
                                    // re-insert
                                    mpRedlineTable->Remove( n );
                                    mpRedlineTable->Insert( pRedl );
                                    bDec = true;
                                }
                            }
                            break;

                        case SwComparePosition::OverlapBehind:
                            {
                                if( *pStt == *pRStt )
                                {
                                    pRedl->PushData( *pNewRedl );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    if( IsHideChanges( meRedlineFlags ))
                                    {
                                        mpRedlineTable->Insert( pNewRedl );
                                        pRedl->Hide(0, mpRedlineTable->GetPos(pRedl));
                                        mpRedlineTable->Remove( pNewRedl );
                                    }
                                }
                                else
                                {
                                    pNew = new SwRangeRedline( *pRedl );
                                    pNew->PushData( *pNewRedl );
                                    pNew->SetStart( *pStt );
                                    pNewRedl->SetStart( *pREnd, pStt );
                                    pRedl->SetEnd( *pNew->Start(), pREnd );
                                    if( !pRedl->HasValidRange() )
                                    {
                                        // re-insert
                                        mpRedlineTable->Remove( n );
                                        mpRedlineTable->Insert( pRedl );
                                    }
                                }
                            }
                            break;
                        default:
                            break;
                        }

                        // insert the pNew part (if it exists)
                        if( pNew )
                        {
                            mpRedlineTable->Insert( pNew );

                            // pNew must be deleted if Insert() wasn't
                            // successful. But that can't happen, since pNew is
                            // part of the original pRedl redline.
                            // OSL_ENSURE( bRet, "Can't insert existing redline?" );

                            // restart (now with pRedl being split up)
                            n = 0;
                            bDec = true;
                        }
                    }
                }
                break;

                case RedlineType::Format:
                    switch( eCmpPos )
                    {
                    case SwComparePosition::OverlapBefore:
                        pRedl->SetStart( *pEnd, pRStt );
                        // re-insert
                        mpRedlineTable->Remove( n );
                        mpRedlineTable->Insert( pRedl, n );
                        bDec = true;
                        break;

                    case SwComparePosition::OverlapBehind:
                        pRedl->SetEnd( *pStt, pREnd );
                        break;

                    case SwComparePosition::Equal:
                    case SwComparePosition::Outside:
                        // Overlaps the current one completely or has the
                        // same dimension, delete the old one
                        mpRedlineTable->DeleteAndDestroy( n );
                        bDec = true;
                        break;

                    case SwComparePosition::Inside:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if( *pEnd != *pREnd )
                        {
                            if( *pEnd != *pRStt )
                            {
                                SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                                pNew->SetStart( *pEnd );
                                pRedl->SetEnd( *pStt, pREnd );
                                if( ( *pStt == *pRStt ) &&
                                    ( pRedl->GetContentIdx() == nullptr ) )
                                    mpRedlineTable->DeleteAndDestroy( n );
                                AppendRedline( pNew, bCallDelete );
                                n = 0;      // re-initialize
                                bDec = true;
                            }
                        }
                        else
                            pRedl->SetEnd( *pStt, pREnd );
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;

            case RedlineType::Format:
                switch( pRedl->GetType() )
                {
                case RedlineType::Insert:
                case RedlineType::Delete:
                    switch( eCmpPos )
                    {
                    case SwComparePosition::OverlapBefore:
                        pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case SwComparePosition::OverlapBehind:
                        pNewRedl->SetStart( *pREnd, pStt );
                        break;

                    case SwComparePosition::Equal:
                    case SwComparePosition::Inside:
                        delete pNewRedl;
                        pNewRedl = nullptr;
                        break;

                    case SwComparePosition::Outside:
                        // Overlaps the current one completely,
                        // split or shorten the new one
                        if (*pEnd == *pREnd)
                        {
                            pNewRedl->SetEnd(*pRStt, pEnd);
                        }
                        else if (*pStt == *pRStt)
                        {
                            pNewRedl->SetStart(*pREnd, pStt);
                        }
                        else
                        {
                            SwRangeRedline* pNew = new SwRangeRedline( *pNewRedl );
                            pNew->SetStart( *pREnd );
                            pNewRedl->SetEnd( *pRStt, pEnd );
                            AppendRedline( pNew, bCallDelete );
                            n = 0;      // re-initialize
                            bDec = true;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                case RedlineType::Format:
                    switch( eCmpPos )
                    {
                    case SwComparePosition::Outside:
                    case SwComparePosition::Equal:
                        {
                            // Overlaps the current one completely or has the
                            // same dimension, delete the old one
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;

                    case SwComparePosition::Inside:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // own one can be ignored completely
                            delete pNewRedl;
                            pNewRedl = nullptr;
                        }
                        else if( *pREnd == *pEnd )
                            // or else only shorten the current one
                            pRedl->SetEnd( *pStt, pREnd );
                        else if( *pRStt == *pStt )
                        {
                            // or else only shorten the current one
                            pRedl->SetStart( *pEnd, pRStt );
                            // re-insert
                            mpRedlineTable->Remove( n );
                            mpRedlineTable->Insert( pRedl, n );
                            bDec = true;
                        }
                        else
                        {
                            // If it lies completely within the current one
                            // we need to split it
                            SwRangeRedline* pNew = new SwRangeRedline( *pRedl );
                            pNew->SetStart( *pEnd );
                            pRedl->SetEnd( *pStt, pREnd );
                            AppendRedline( pNew, bCallDelete );
                            n = 0;      // re-initialize
                            bDec = true;
                        }
                        break;

                    case SwComparePosition::OverlapBefore:
                    case SwComparePosition::OverlapBehind:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ))
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            if( SwComparePosition::OverlapBehind == eCmpPos )
                                pNewRedl->SetStart( *pRStt, pStt );
                            else
                                pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = false;
                        }
                        else if( SwComparePosition::OverlapBehind == eCmpPos )
                            pNewRedl->SetStart( *pREnd, pStt );
                        else
                            pNewRedl->SetEnd( *pRStt, pEnd );
                        break;

                    case SwComparePosition::CollideEnd:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) && n &&
                            *(*mpRedlineTable)[ n-1 ]->End() < *pStt )
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            pNewRedl->SetEnd( *pREnd, pEnd );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;
                    case SwComparePosition::CollideStart:
                        if( pRedl->IsOwnRedline( *pNewRedl ) &&
                            pRedl->CanCombine( *pNewRedl ) &&
                            n+1 < mpRedlineTable->size() &&
                            *(*mpRedlineTable)[ n+1 ]->Start() < *pEnd )
                        {
                            // If that's the case we can merge it, meaning
                            // the new one covers this well
                            pNewRedl->SetStart( *pRStt, pStt );
                            mpRedlineTable->DeleteAndDestroy( n );
                            bDec = true;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                break;

            case RedlineType::FmtColl:
                // How should we behave here?
                // insert as is
                break;
            default:
                break;
            }
        }

        if( pNewRedl )
        {
            if( ( *pStt == *pEnd ) &&
                ( pNewRedl->GetContentIdx() == nullptr ) )
            {   // Do not insert empty redlines
                delete pNewRedl;
                pNewRedl = nullptr;
            }
            else
            {
                if ( bCallDelete && RedlineType::Delete == pNewRedl->GetType() )
                {
                    if ( pStt->nContent != 0 )
                    {
                        // tdf#119571 update the style of the joined paragraph
                        // after a partially deleted paragraph to show its correct style
                        // in "Show changes" mode, too. All removed paragraphs
                        // get the style of the first (partially deleted) paragraph
                        // to avoid text insertion with bad style in the deleted
                        // area later.

                        SwContentNode* pDelNd = pStt->nNode.GetNode().GetContentNode();
                        SwContentNode* pTextNd = pEnd->nNode.GetNode().GetContentNode();
                        SwTextNode* pDelNode = pStt->nNode.GetNode().GetTextNode();
                        SwTextNode* pTextNode;
                        SwNodeIndex aIdx( pEnd->nNode.GetNode() );
                        bool bFirst = true;

                        while (pDelNode != nullptr && pTextNd != nullptr && pDelNd->GetIndex() < pTextNd->GetIndex())
                        {
                            pTextNode = pTextNd->GetTextNode();
                            if (pTextNode && pDelNode != pTextNode )
                            {
                                SwPosition aPos(aIdx);

                                if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
                                {
                                    bCompress = true;

                                    // split redline to store ExtraData per paragraphs
                                    SwRangeRedline* pPar = new SwRangeRedline( *pNewRedl );
                                    pPar->SetStart( aPos );
                                    pNewRedl->SetEnd( aPos );

                                    // get extradata for reset formatting of the modified paragraph
                                    SwRedlineExtraData_FormatColl* pExtraData = lcl_CopyStyle(aPos, *pStt, false);
                                    if (pExtraData)
                                    {
                                        std::unique_ptr<SwRedlineExtraData_FormatColl> xRedlineExtraData;
                                        if (!bFirst)
                                            pExtraData->SetFormatAll(false);
                                        xRedlineExtraData.reset(pExtraData);
                                        pPar->SetExtraData( xRedlineExtraData.get() );
                                    }
                                    mpRedlineTable->Insert( pPar );
                                }

                                // modify paragraph formatting
                                lcl_CopyStyle(*pStt, aPos);
                            }
                            pTextNd = SwNodes::GoPrevious( &aIdx );

                            if (bFirst)
                                bFirst = false;
                        }
                    }
                }
                bool const ret = mpRedlineTable->Insert( pNewRedl );
                assert(ret || !pNewRedl);
                if (ret && !pNewRedl)
                {
                    bMerged = true; // treat InsertWithValidRanges as "merge"
                }
            }
        }

        if( bCompress )
            CompressRedlines();
    }
    else
    {
        if( bCallDelete && RedlineType::Delete == pNewRedl->GetType() )
        {
            RedlineFlags eOld = meRedlineFlags;
            // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
            // The ShowMode needs to be retained!
            meRedlineFlags = eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore);
            m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pNewRedl );
            meRedlineFlags = eOld;
        }
        delete pNewRedl;
        pNewRedl = nullptr;
    }
    CHECK_REDLINE( *this )

    return (nullptr != pNewRedl)
        ? AppendResult::APPENDED
        : (bMerged ? AppendResult::MERGED : AppendResult::IGNORED);
}

bool DocumentRedlineManager::AppendTableRowRedline( SwTableRowRedline* pNewRedl )
{
    // #TODO - equivalent for 'SwTableRowRedline'
    /*
    CHECK_REDLINE( this )
    */

    if (IsRedlineOn() && !IsShowOriginal(meRedlineFlags))
    {
        // #TODO - equivalent for 'SwTableRowRedline'
        /*
        pNewRedl->InvalidateRange();
        */

        // Make equivalent of 'AppendRedline' checks inside here too

        mpExtraRedlineTable->Insert( pNewRedl );
    }
    else
    {
        // TO DO - equivalent for 'SwTableRowRedline'
        /*
        if( bCallDelete && RedlineType::Delete == pNewRedl->GetType() )
        {
            RedlineFlags eOld = meRedlineFlags;
            // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
            // The ShowMode needs to be retained!
            meRedlineFlags = eOld & ~(RedlineFlags::On | RedlineFlags::Ignore);
            DeleteAndJoin( *pNewRedl );
            meRedlineFlags = eOld;
        }
        delete pNewRedl, pNewRedl = 0;
        */
    }
    // #TODO - equivalent for 'SwTableRowRedline'
    /*
    CHECK_REDLINE( this )
    */

    return nullptr != pNewRedl;
}

bool DocumentRedlineManager::AppendTableCellRedline( SwTableCellRedline* pNewRedl )
{
    // #TODO - equivalent for 'SwTableCellRedline'
    /*
    CHECK_REDLINE( this )
    */

    if (IsRedlineOn() && !IsShowOriginal(meRedlineFlags))
    {
        // #TODO - equivalent for 'SwTableCellRedline'
        /*
        pNewRedl->InvalidateRange();
        */

        // Make equivalent of 'AppendRedline' checks inside here too

        mpExtraRedlineTable->Insert( pNewRedl );
    }
    else
    {
        // TO DO - equivalent for 'SwTableCellRedline'
        /*
        if( bCallDelete && RedlineType::Delete == pNewRedl->GetType() )
        {
            RedlineFlags eOld = meRedlineFlags;
            // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
            // The ShowMode needs to be retained!
            meRedlineFlags = eOld & ~(RedlineFlags::On | RedlineFlags::Ignore);
            DeleteAndJoin( *pNewRedl );
            meRedlineFlags = eOld;
        }
        delete pNewRedl, pNewRedl = 0;
        */
    }
    // #TODO - equivalent for 'SwTableCellRedline'
    /*
    CHECK_REDLINE( this )
    */

    return nullptr != pNewRedl;
}

void DocumentRedlineManager::CompressRedlines()
{
    CHECK_REDLINE( *this )

    void (SwRangeRedline::*pFnc)(sal_uInt16, size_t) = nullptr;
    RedlineFlags eShow = RedlineFlags::ShowMask & meRedlineFlags;
    if( eShow == (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete))
        pFnc = &SwRangeRedline::Show;
    else if (eShow == RedlineFlags::ShowInsert)
        pFnc = &SwRangeRedline::Hide;

    // Try to merge identical ones
    for( SwRedlineTable::size_type n = 1; n < mpRedlineTable->size(); ++n )
    {
        SwRangeRedline* pPrev = (*mpRedlineTable)[ n-1 ],
                    * pCur = (*mpRedlineTable)[ n ];
        const SwPosition* pPrevStt = pPrev->Start(),
                        * pPrevEnd = pPrevStt == pPrev->GetPoint()
                            ? pPrev->GetMark() : pPrev->GetPoint();
        const SwPosition* pCurStt = pCur->Start(),
                        * pCurEnd = pCurStt == pCur->GetPoint()
                            ? pCur->GetMark() : pCur->GetPoint();
        if( *pPrevEnd == *pCurStt && pPrev->CanCombine( *pCur ) &&
            pPrevStt->nNode.GetNode().StartOfSectionNode() ==
            pCurEnd->nNode.GetNode().StartOfSectionNode() &&
            !pCurEnd->nNode.GetNode().StartOfSectionNode()->IsTableNode() )
        {
            // we then can merge them
            SwRedlineTable::size_type nPrevIndex = n-1;
            pPrev->Show(0, nPrevIndex);
            pCur->Show(0, n);

            pPrev->SetEnd( *pCur->End() );
            mpRedlineTable->DeleteAndDestroy( n );
            --n;
            if( pFnc )
                (pPrev->*pFnc)(0, nPrevIndex);
        }
    }
    CHECK_REDLINE( *this )

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::SplitRedline( const SwPaM& rRange )
{
    bool bChg = false;
    SwRedlineTable::size_type n = 0;
    const SwPosition* pStt = rRange.Start();
    const SwPosition* pEnd = rRange.End();
    GetRedline( *pStt, &n );
    for ( ; n < mpRedlineTable->size(); ++n)
    {
        SwRangeRedline * pRedline = (*mpRedlineTable)[ n ];
        SwPosition *const pRedlineStart = pRedline->Start();
        SwPosition *const pRedlineEnd = pRedline->End();
        if (*pRedlineStart <= *pStt && *pStt <= *pRedlineEnd &&
            *pRedlineStart <= *pEnd && *pEnd <= *pRedlineEnd)
        {
            bChg = true;
            int nn = 0;
            if (*pStt == *pRedlineStart)
                nn += 1;
            if (*pEnd == *pRedlineEnd)
                nn += 2;

            SwRangeRedline* pNew = nullptr;
            switch( nn )
            {
            case 0:
                pNew = new SwRangeRedline( *pRedline );
                pRedline->SetEnd( *pStt, pRedlineEnd );
                pNew->SetStart( *pEnd );
                break;

            case 1:
                *pRedlineStart = *pEnd;
                break;

            case 2:
                *pRedlineEnd = *pStt;
                break;

            case 3:
                pRedline->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                mpRedlineTable->DeleteAndDestroy( n-- );
                pRedline = nullptr;
                break;
            }
            if (pRedline && !pRedline->HasValidRange())
            {
                // re-insert
                mpRedlineTable->Remove( n );
                mpRedlineTable->Insert( pRedline, n );
            }
            if( pNew )
                mpRedlineTable->Insert( pNew, n );
        }
        else if (*pEnd < *pRedlineStart)
            break;
    }
    return bChg;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::DeleteRedline( const SwPaM& rRange, bool bSaveInUndo,
                            RedlineType nDelType )
{
    if( !rRange.HasMark() || *rRange.GetMark() == *rRange.GetPoint() )
        return false;

    bool bChg = false;

    if (bSaveInUndo && m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        std::unique_ptr<SwUndoRedline> pUndo(new SwUndoRedline( SwUndoId::REDLINE, rRange ));
        if( pUndo->GetRedlSaveCount() )
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndo));
        }
    }

    const SwPosition* pStt = rRange.Start(),
                    * pEnd = pStt == rRange.GetPoint() ? rRange.GetMark()
                                                       : rRange.GetPoint();
    SwRedlineTable::size_type n = 0;
    GetRedline( *pStt, &n );
    for( ; n < mpRedlineTable->size() ; ++n )
    {
        SwRangeRedline* pRedl = (*mpRedlineTable)[ n ];
        if( RedlineType::Any != nDelType && nDelType != pRedl->GetType() )
            continue;

        SwPosition* pRStt = pRedl->Start(),
                  * pREnd = pRStt == pRedl->GetPoint() ? pRedl->GetMark()
                                                       : pRedl->GetPoint();
        switch( ComparePosition( *pStt, *pEnd, *pRStt, *pREnd ) )
        {
        case SwComparePosition::Equal:
        case SwComparePosition::Outside:
            pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
            mpRedlineTable->DeleteAndDestroy( n-- );
            bChg = true;
            break;

        case SwComparePosition::OverlapBefore:
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                pRedl->SetStart( *pEnd, pRStt );
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
                // re-insert
                mpRedlineTable->Remove( n );
                mpRedlineTable->Insert( pRedl );
                --n;
            break;

        case SwComparePosition::OverlapBehind:
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                pRedl->SetEnd( *pStt, pREnd );
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
                if( !pRedl->HasValidRange() )
                {
                    // re-insert
                    mpRedlineTable->Remove( n );
                    mpRedlineTable->Insert( pRedl );
                    --n;
                }
            break;

        case SwComparePosition::Inside:
            {
                // this one needs to be split
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                if( *pRStt == *pStt )
                {
                    pRedl->SetStart( *pEnd, pRStt );
                    pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
                    // re-insert
                    mpRedlineTable->Remove( n );
                    mpRedlineTable->Insert( pRedl );
                    --n;
                }
                else
                {
                    SwRangeRedline* pCpy;
                    if( *pREnd != *pEnd )
                    {
                        pCpy = new SwRangeRedline( *pRedl );
                        pCpy->SetStart( *pEnd );
                        pCpy->InvalidateRange(SwRangeRedline::Invalidation::Add);
                    }
                    else
                        pCpy = nullptr;
                    pRedl->SetEnd( *pStt, pREnd );
                    pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
                    if( !pRedl->HasValidRange() )
                    {
                        // re-insert
                        mpRedlineTable->Remove( n );
                        mpRedlineTable->Insert( pRedl );
                        --n;
                    }
                    if( pCpy )
                        mpRedlineTable->Insert( pCpy );
                }
            }
            break;

        case SwComparePosition::CollideEnd:
        case SwComparePosition::Before:
            n = mpRedlineTable->size();
            break;
        default:
            break;
        }
    }

    if( bChg )
        m_rDoc.getIDocumentState().SetModified();

    return bChg;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::DeleteRedline( const SwStartNode& rNode, bool bSaveInUndo,
                            RedlineType nDelType )
{
    SwPaM aTemp(*rNode.EndOfSectionNode(), rNode);
    return DeleteRedline(aTemp, bSaveInUndo, nDelType);
}

SwRedlineTable::size_type DocumentRedlineManager::GetRedlinePos( const SwNode& rNd, RedlineType nType ) const
{
    const sal_uLong nNdIdx = rNd.GetIndex();
    for( SwRedlineTable::size_type n = 0; n < mpRedlineTable->size() ; ++n )
    {
        const SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
        sal_uLong nPt = pTmp->GetPoint()->nNode.GetIndex(),
              nMk = pTmp->GetMark()->nNode.GetIndex();
        if( nPt < nMk ) { long nTmp = nMk; nMk = nPt; nPt = nTmp; }

        if( ( RedlineType::Any == nType || nType == pTmp->GetType()) &&
            nMk <= nNdIdx && nNdIdx <= nPt )
            return n;

        if( nMk > nNdIdx )
            break;
    }
    return SwRedlineTable::npos;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const SwRangeRedline* DocumentRedlineManager::GetRedline( const SwPosition& rPos,
                                    SwRedlineTable::size_type* pFndPos ) const
{
    SwRedlineTable::size_type nO = mpRedlineTable->size(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            const SwRangeRedline* pRedl = (*mpRedlineTable)[ nM ];
            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();
            if( pEnd == pStt
                    ? *pStt == rPos
                    : ( *pStt <= rPos && rPos < *pEnd ) )
            {
                while( nM && rPos == *(*mpRedlineTable)[ nM - 1 ]->End() &&
                    rPos == *(*mpRedlineTable)[ nM - 1 ]->Start() )
                {
                    --nM;
                    pRedl = (*mpRedlineTable)[ nM ];
                }
                // if there are format and insert changes in the same position
                // show insert change first.
                // since the redlines are sorted by position, only check the redline
                // before and after the current redline
                if( RedlineType::Format == pRedl->GetType() )
                {
                    if( nM && rPos >= *(*mpRedlineTable)[ nM - 1 ]->Start() &&
                        rPos <= *(*mpRedlineTable)[ nM - 1 ]->End() &&
                        ( RedlineType::Insert == (*mpRedlineTable)[ nM - 1 ]->GetType() ) )
                    {
                        --nM;
                        pRedl = (*mpRedlineTable)[ nM ];
                    }
                    else if( ( nM + 1 ) <= nO && rPos >= *(*mpRedlineTable)[ nM + 1 ]->Start() &&
                        rPos <= *(*mpRedlineTable)[ nM + 1 ]->End() &&
                        ( RedlineType::Insert == (*mpRedlineTable)[ nM + 1 ]->GetType() ) )
                    {
                        ++nM;
                        pRedl = (*mpRedlineTable)[ nM ];
                    }
                }

                if( pFndPos )
                    *pFndPos = nM;
                return pRedl;
            }
            else if( *pEnd <= rPos )
                nU = nM + 1;
            else if( nM == 0 )
            {
                if( pFndPos )
                    *pFndPos = nU;
                return nullptr;
            }
            else
                nO = nM - 1;
        }
    }
    if( pFndPos )
        *pFndPos = nU;
    return nullptr;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::AcceptRedline( SwRedlineTable::size_type nPos, bool bCallDelete )
{
    bool bRet = false;

    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    SwRangeRedline* pTmp = (*mpRedlineTable)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pTmp->GetDescr());
            m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::ACCEPT_REDLINE, &aRewriter);
        }

        int nLoopCnt = 2;
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();

        do {

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoAcceptRedline>(*pTmp) );
            }

            bRet |= lcl_AcceptRedline( *mpRedlineTable, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( SwRedlineTable::npos == nPos )
                    nPos = 0;
                SwRedlineTable::size_type nFndPos = 2 == nLoopCnt
                                    ? mpRedlineTable->FindNextSeqNo( nSeqNo, nPos )
                                    : mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos );
                if( SwRedlineTable::npos != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    SwRedlineTable::npos != ( nFndPos =
                        mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos ))) )
                {
                    nPos = nFndPos;
                    pTmp = (*mpRedlineTable)[ nPos ];
                }
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            m_rDoc.getIDocumentState().SetModified();
        }

        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
        }
    }
    return bRet;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::AcceptRedline( const SwPaM& rPam, bool bCallDelete )
{
    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    // The Selection is only in the ContentSection. If there are Redlines
    // to Non-ContentNodes before or after that, then the Selections
    // expand to them.
    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    lcl_AdjustRedlineRange( aPam );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::ACCEPT_REDLINE, nullptr );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoAcceptRedline>( aPam ));
    }

    int nRet = lcl_AcceptRejectRedl( lcl_AcceptRedline, *mpRedlineTable,
                                     bCallDelete, aPam );
    if( nRet > 0 )
    {
        CompressRedlines();
        m_rDoc.getIDocumentState().SetModified();
    }
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        OUString aTmpStr;

        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRet));
            aTmpStr = aRewriter.Apply(SwResId(STR_N_REDLINES));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        m_rDoc.GetIDocumentUndoRedo().EndUndo( SwUndoId::ACCEPT_REDLINE, &aRewriter );
    }
    return nRet != 0;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

void DocumentRedlineManager::AcceptRedlineParagraphFormatting( const SwPaM &rPam )
{
    const SwPosition* pStt = rPam.Start(),
                    * pEnd = pStt == rPam.GetPoint() ? rPam.GetMark()
                                                     : rPam.GetPoint();

    const sal_uLong nSttIdx = pStt->nNode.GetIndex();
    const sal_uLong nEndIdx = pEnd->nNode.GetIndex();

    for( SwRedlineTable::size_type n = 0; n < mpRedlineTable->size() ; ++n )
    {
        const SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
        sal_uLong nPt = pTmp->GetPoint()->nNode.GetIndex(),
              nMk = pTmp->GetMark()->nNode.GetIndex();
        if( nPt < nMk ) { long nTmp = nMk; nMk = nPt; nPt = nTmp; }

        if( RedlineType::ParagraphFormat == pTmp->GetType() &&
            ( (nSttIdx <= nMk && nMk <= nEndIdx) || (nSttIdx <= nPt && nPt <= nEndIdx) ) )
                AcceptRedline( n, false );

        if( nMk > nEndIdx )
            break;
    }
}

bool DocumentRedlineManager::RejectRedline( SwRedlineTable::size_type nPos, bool bCallDelete )
{
    bool bRet = false;

    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    SwRangeRedline* pTmp = (*mpRedlineTable)[ nPos ];
    if( pTmp->HasMark() && pTmp->IsVisible() )
    {
        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            SwRewriter aRewriter;

            aRewriter.AddRule(UndoArg1, pTmp->GetDescr());
            m_rDoc.GetIDocumentUndoRedo().StartUndo(SwUndoId::REJECT_REDLINE, &aRewriter);
        }

        int nLoopCnt = 2;
        sal_uInt16 nSeqNo = pTmp->GetSeqNo();

        do {

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoRejectRedline>( *pTmp ) );
            }

            bRet |= lcl_RejectRedline( *mpRedlineTable, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( SwRedlineTable::npos == nPos )
                    nPos = 0;
                SwRedlineTable::size_type nFndPos = 2 == nLoopCnt
                                    ? mpRedlineTable->FindNextSeqNo( nSeqNo, nPos )
                                    : mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos );
                if( SwRedlineTable::npos != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    SwRedlineTable::npos != ( nFndPos =
                            mpRedlineTable->FindPrevSeqNo( nSeqNo, nPos ))) )
                {
                    nPos = nFndPos;
                    pTmp = (*mpRedlineTable)[ nPos ];
                }
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while( nLoopCnt );

        if( bRet )
        {
            CompressRedlines();
            m_rDoc.getIDocumentState().SetModified();
        }

        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().EndUndo(SwUndoId::END, nullptr);
        }
    }
    return bRet;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::RejectRedline( const SwPaM& rPam, bool bCallDelete )
{
    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    // The Selection is only in the ContentSection. If there are Redlines
    // to Non-ContentNodes before or after that, then the Selections
    // expand to them.
    SwPaM aPam( *rPam.GetMark(), *rPam.GetPoint() );
    lcl_AdjustRedlineRange( aPam );

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::REJECT_REDLINE, nullptr );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoRejectRedline>(aPam) );
    }

    int nRet = lcl_AcceptRejectRedl( lcl_RejectRedline, *mpRedlineTable,
                                        bCallDelete, aPam );
    if( nRet > 0 )
    {
        CompressRedlines();
        m_rDoc.getIDocumentState().SetModified();
    }
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        OUString aTmpStr;

        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nRet));
            aTmpStr = aRewriter.Apply(SwResId(STR_N_REDLINES));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, aTmpStr);

        m_rDoc.GetIDocumentUndoRedo().EndUndo( SwUndoId::REJECT_REDLINE, &aRewriter );
    }

    return nRet != 0;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

void DocumentRedlineManager::AcceptAllRedline(bool bAccept)
{
    bool bSuccess = true;
    OUString sUndoStr;
    IDocumentUndoRedo& rUndoMgr = m_rDoc.GetIDocumentUndoRedo();

    if (mpRedlineTable->size() > 1)
    {
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(mpRedlineTable->size()));
            sUndoStr = aRewriter.Apply(SwResId(STR_N_REDLINES));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, sUndoStr);
        rUndoMgr.StartUndo(bAccept ? SwUndoId::ACCEPT_REDLINE : SwUndoId::REJECT_REDLINE, &aRewriter);
    }

    while (!mpRedlineTable->empty() && bSuccess)
    {
        if (bAccept)
            bSuccess = AcceptRedline(mpRedlineTable->size() - 1, true);
        else
            bSuccess = RejectRedline(mpRedlineTable->size() - 1, true);
    }

    if (!sUndoStr.isEmpty())
    {
        rUndoMgr.EndUndo(SwUndoId::EMPTY, nullptr);
    }
}

const SwRangeRedline* DocumentRedlineManager::SelNextRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    bool bRestart;

    // If the starting position points to the last valid ContentNode,
    // we take the next Redline in any case.
    SwRedlineTable::size_type n = 0;
    const SwRangeRedline* pFnd = GetRedlineTable().FindAtPosition( rSttPos, n );
    if( pFnd )
    {
        const SwPosition* pEnd = pFnd->End();
        if( !pEnd->nNode.GetNode().IsContentNode() )
        {
            SwNodeIndex aTmp( pEnd->nNode );
            SwContentNode* pCNd = SwNodes::GoPrevSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                pCNd->Len() == rSttPos.nContent.GetIndex() ))
                pFnd = nullptr;
        }
        if( pFnd )
            rSttPos = *pFnd->End();
    }

    do {
        bRestart = false;

        for( ; !pFnd && n < mpRedlineTable->size(); ++n )
        {
            pFnd = (*mpRedlineTable)[ n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->Start();
                rSttPos = *pFnd->End();
                break;
            }
            else
                pFnd = nullptr;
        }

        if( pFnd )
        {
            // Merge all of the same type and author that are
            // consecutive into one Selection.
            const SwPosition* pPrevEnd = pFnd->End();
            while( ++n < mpRedlineTable->size() )
            {
                const SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    const SwPosition *pRStt;
                    if( pFnd->GetType() != pTmp->GetType() ||
                        pFnd->GetAuthor() != pTmp->GetAuthor() )
                        break;
                    pRStt = pTmp->Start();
                    if( *pPrevEnd == *pRStt || IsPrevPos( *pPrevEnd, *pRStt ) )
                    {
                        pPrevEnd = pTmp->End();
                        rSttPos = *pPrevEnd;
                    }
                    else
                        break;
                }
            }
        }

        if( pFnd )
        {
            const SwRangeRedline* pSaveFnd = pFnd;

            SwContentNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsContentNode() )
            {
                pCNd = m_rDoc.GetNodes().GoNextSection( pIdx );
                if( pCNd )
                {
                    if( *pIdx <= rPam.GetPoint()->nNode )
                        rPam.GetMark()->nContent.Assign( pCNd, 0 );
                    else
                        pFnd = nullptr;
                }
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsContentNode() )
                {
                    pCNd = SwNodes::GoPrevSection( pIdx );
                    if( pCNd )
                    {
                        if( *pIdx >= rPam.GetMark()->nNode )
                            rPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
                        else
                            pFnd = nullptr;
                    }
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n < mpRedlineTable->size() )
                {
                    bRestart = true;
                    *rPam.GetPoint() = *pSaveFnd->End();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = nullptr;
            }
        }
    } while( bRestart );

    return pFnd;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const SwRangeRedline* DocumentRedlineManager::SelPrevRedline( SwPaM& rPam ) const
{
    rPam.DeleteMark();
    rPam.SetMark();

    SwPosition& rSttPos = *rPam.GetPoint();
    SwPosition aSavePos( rSttPos );
    bool bRestart;

    // If the starting position points to the last valid ContentNode,
    // we take the previous Redline in any case.
    SwRedlineTable::size_type n = 0;
    const SwRangeRedline* pFnd = GetRedlineTable().FindAtPosition( rSttPos, n, false );
    if( pFnd )
    {
        const SwPosition* pStt = pFnd->Start();
        if( !pStt->nNode.GetNode().IsContentNode() )
        {
            SwNodeIndex aTmp( pStt->nNode );
            SwContentNode* pCNd = m_rDoc.GetNodes().GoNextSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.nNode &&
                !rSttPos.nContent.GetIndex() ))
                pFnd = nullptr;
        }
        if( pFnd )
            rSttPos = *pFnd->Start();
    }

    do {
        bRestart = false;

        while( !pFnd && 0 < n )
        {
            pFnd = (*mpRedlineTable)[ --n ];
            if( pFnd->HasMark() && pFnd->IsVisible() )
            {
                *rPam.GetMark() = *pFnd->End();
                rSttPos = *pFnd->Start();
            }
            else
                pFnd = nullptr;
        }

        if( pFnd )
        {
            // Merge all of the same type and author that are
            // consecutive into one Selection.
            const SwPosition* pNextStt = pFnd->Start();
            while( 0 < n )
            {
                const SwRangeRedline* pTmp = (*mpRedlineTable)[ --n ];
                if( pTmp->HasMark() && pTmp->IsVisible() )
                {
                    const SwPosition *pREnd;
                    if( pFnd->GetType() == pTmp->GetType() &&
                        pFnd->GetAuthor() == pTmp->GetAuthor() &&
                        ( *pNextStt == *( pREnd = pTmp->End() ) ||
                          IsPrevPos( *pREnd, *pNextStt )) )
                    {
                        pNextStt = pTmp->Start();
                        rSttPos = *pNextStt;
                    }
                    else
                    {
                        ++n;
                        break;
                    }
                }
            }
        }

        if( pFnd )
        {
            const SwRangeRedline* pSaveFnd = pFnd;

            SwContentNode* pCNd;
            SwNodeIndex* pIdx = &rPam.GetMark()->nNode;
            if( !pIdx->GetNode().IsContentNode() )
            {
                pCNd = SwNodes::GoPrevSection( pIdx );
                if( pCNd )
                {
                    if( *pIdx >= rPam.GetPoint()->nNode )
                        rPam.GetMark()->nContent.Assign( pCNd, pCNd->Len() );
                    else
                        pFnd = nullptr;
                }
            }

            if( pFnd )
            {
                pIdx = &rPam.GetPoint()->nNode;
                if( !pIdx->GetNode().IsContentNode() )
                {
                    pCNd = m_rDoc.GetNodes().GoNextSection( pIdx );
                    if( pCNd )
                    {
                        if( *pIdx <= rPam.GetMark()->nNode )
                            rPam.GetPoint()->nContent.Assign( pCNd, 0 );
                        else
                            pFnd = nullptr;
                    }
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n )
                {
                    bRestart = true;
                    *rPam.GetPoint() = *pSaveFnd->Start();
                }
                else
                {
                    rPam.DeleteMark();
                    *rPam.GetPoint() = aSavePos;
                }
                pFnd = nullptr;
            }
        }
    } while( bRestart );

    return pFnd;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

// Set comment at the Redline
bool DocumentRedlineManager::SetRedlineComment( const SwPaM& rPaM, const OUString& rS )
{
    bool bRet = false;
    const SwPosition* pStt = rPaM.Start(),
                    * pEnd = pStt == rPaM.GetPoint() ? rPaM.GetMark()
                                                     : rPaM.GetPoint();
    SwRedlineTable::size_type n = 0;
    if( GetRedlineTable().FindAtPosition( *pStt, n ) )
    {
        for( ; n < mpRedlineTable->size(); ++n )
        {
            bRet = true;
            SwRangeRedline* pTmp = (*mpRedlineTable)[ n ];
            if( pStt != pEnd && *pTmp->Start() > *pEnd )
                break;

            pTmp->SetComment( rS );
            if( *pTmp->End() >= *pEnd )
                break;
        }
    }
    if( bRet )
        m_rDoc.getIDocumentState().SetModified();

    return bRet;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

// Create a new author if necessary
std::size_t DocumentRedlineManager::GetRedlineAuthor()
{
    return SW_MOD()->GetRedlineAuthor();
}

/// Insert new author into the Table for the Readers etc.
std::size_t DocumentRedlineManager::InsertRedlineAuthor( const OUString& rNew )
{
    return SW_MOD()->InsertRedlineAuthor(rNew);
}

void DocumentRedlineManager::UpdateRedlineAttr()
{
    const SwRedlineTable& rTable = GetRedlineTable();
    for(SwRangeRedline* pRedl : rTable)
    {
        if( pRedl->IsVisible() )
            pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
    }

    // #TODO - add 'SwExtraRedlineTable' also ?
}

const uno::Sequence <sal_Int8>& DocumentRedlineManager::GetRedlinePassword() const
{
    return maRedlinePasswd;
}

void DocumentRedlineManager::SetRedlinePassword(
            /*[in]*/const uno::Sequence <sal_Int8>& rNewPassword)
{
    maRedlinePasswd = rNewPassword;
    m_rDoc.getIDocumentState().SetModified();
}

/// Set comment text for the Redline, which is inserted later on via
/// AppendRedline. Is used by Autoformat.
/// A null pointer resets the mode. The pointer is not copied, so it
/// needs to stay valid!
void DocumentRedlineManager::SetAutoFormatRedlineComment( const OUString* pText, sal_uInt16 nSeqNo )
{
    m_rDoc.SetAutoFormatRedline( nullptr != pText );
    if( pText )
    {
        mpAutoFormatRedlnComment.reset( new OUString( *pText ) );
    }
    else
    {
        mpAutoFormatRedlnComment.reset();
    }

    mnAutoFormatRedlnCommentNo = nSeqNo;
}

DocumentRedlineManager::~DocumentRedlineManager()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
