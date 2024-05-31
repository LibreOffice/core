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
#include <txtfld.hxx>
#include <doc.hxx>
#include <docsh.hxx>
#include <wrtsh.hxx>
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
#include <osl/diagnose.h>
#include <editeng/prntitem.hxx>
#include <comphelper/lok.hxx>

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
            assert(dynamic_cast<SwContentIndexReg*>(&pPos->GetNode())
                    == pPos->GetContentNode());

            SwTextNode* pTextNode = pPos->GetNode().GetTextNode();
            if( pTextNode == nullptr )
            {
                assert(pPos->GetContentIndex() == 0);
            }
            else
            {
                assert(pPos->GetContentIndex() >= 0 && pPos->GetContentIndex() <= pTextNode->Len());
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
        void lcl_CheckRedline( const IDocumentRedlineAccess& redlineAccess )
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
    if (rDoc.IsClipBoard())
    {
        return;
    }
    // no need to call UpdateFootnoteNums for FTNNUM_PAGE:
    // the AppendFootnote/RemoveFootnote will do it by itself!
    rDoc.GetFootnoteIdxs().UpdateFootnote(rPam.Start()->GetNode());
    SwPosition currentStart(*rPam.Start());
    SwTextNode * pStartNode(rPam.Start()->GetNode().GetTextNode());
    while (!pStartNode)
    {
        // note: branch only taken for redlines, not fieldmarks
        SwStartNode *const pTableOrSectionNode(
            currentStart.GetNode().IsTableNode()
                ? static_cast<SwStartNode*>(currentStart.GetNode().GetTableNode())
                : static_cast<SwStartNode*>(currentStart.GetNode().GetSectionNode()));
        if ( !pTableOrSectionNode )
        {
            SAL_WARN("sw.core", "UpdateFramesForAddDeleteRedline:: known pathology (or ChangesInRedline mode)");
            return;
        }
        for (SwNodeOffset j = pTableOrSectionNode->GetIndex(); j <= pTableOrSectionNode->EndOfSectionIndex(); ++j)
        {
            pTableOrSectionNode->GetNodes()[j]->SetRedlineMergeFlag(SwNode::Merge::Hidden);
        }
        for (SwRootFrame const*const pLayout : rDoc.GetAllLayouts())
        {
            if (pLayout->HasMergedParas())
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
        currentStart.Assign( pTableOrSectionNode->EndOfSectionIndex() + 1 );
        pStartNode = currentStart.GetNode().GetTextNode();
    }
    if (currentStart < *rPam.End())
    {
        SwTextNode * pNode(pStartNode);
        do
        {
            // deleted text node: remove it from "hidden" list
            // to update numbering in Show Changes mode
            SwPosition aPos( *pNode, pNode->Len() );
            if ( pNode->GetNumRule() && aPos < *rPam.End() )
                pNode->RemoveFromListRLHidden();

            std::vector<SwTextFrame*> frames;
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pNode);
            for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
            {
                if (pFrame->getRootFrame()->HasMergedParas())
                {
                    frames.push_back(pFrame);
                }
                // set anchored objects as deleted
                pFrame->SetDrawObjsAsDeleted(true);
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
            pNode = static_cast<SwTextNode*>(SwNodes::GoNextSection(&tmp, /*bSkipHidden=*/true, /*bSkipProtect=*/false));
        }
        while (pNode && pNode->GetIndex() <= rPam.End()->GetNodeIndex());
    }
    // fields last - SwGetRefField::UpdateField requires up-to-date frames
    UpdateFieldsForRedline(rDoc.getIDocumentFieldsAccess()); // after footnotes

    // update SwPostItMgr / notes in the margin
    rDoc.GetDocShell()->Broadcast(
            SwFormatFieldHint(nullptr, SwFormatFieldHintWhich::REMOVED) );
}

void UpdateFramesForRemoveDeleteRedline(SwDoc & rDoc, SwPaM const& rPam)
{
    // tdf#147006 fieldmark command may be empty => do not call AppendAllObjs()
    if (rDoc.IsClipBoard() || *rPam.GetPoint() == *rPam.GetMark())
    {
        return;
    }
    bool isAppendObjsCalled(false);
    rDoc.GetFootnoteIdxs().UpdateFootnote(rPam.Start()->GetNode());
    SwPosition currentStart(*rPam.Start());
    SwTextNode * pStartNode(rPam.Start()->GetNode().GetTextNode());
    while (!pStartNode)
    {
        // note: branch only taken for redlines, not fieldmarks
        SwStartNode *const pTableOrSectionNode(
            currentStart.GetNode().IsTableNode()
                ? static_cast<SwStartNode*>(currentStart.GetNode().GetTableNode())
                : static_cast<SwStartNode*>(currentStart.GetNode().GetSectionNode()));
        assert(pTableOrSectionNode); // known pathology
        for (SwNodeOffset j = pTableOrSectionNode->GetIndex(); j <= pTableOrSectionNode->EndOfSectionIndex(); ++j)
        {
            pTableOrSectionNode->GetNodes()[j]->SetRedlineMergeFlag(SwNode::Merge::None);
        }
        if (rDoc.getIDocumentLayoutAccess().GetCurrentLayout()->HasMergedParas())
        {
            // note: this will also create frames for all currently hidden flys
            // because it calls AppendAllObjs
            ::MakeFrames(&rDoc, currentStart.GetNode(), *pTableOrSectionNode->EndOfSectionNode());
            isAppendObjsCalled = true;
        }
        currentStart.Assign( pTableOrSectionNode->EndOfSectionIndex() + 1 );
        pStartNode = currentStart.GetNode().GetTextNode();
    }
    if (currentStart < *rPam.End())
    {
        SwTextNode * pNode(pStartNode);
        do
        {
            // undeleted text node: add it to the "hidden" list
            // to update numbering in Show Changes mode
            SwPosition aPos( *pNode, pNode->Len() );
            if ( pNode->GetNumRule() && aPos < *rPam.End() )
                pNode->AddToListRLHidden();

            std::vector<SwTextFrame*> frames;
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pNode);
            for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
            {
                if (pFrame->getRootFrame()->HasMergedParas())
                {
                    frames.push_back(pFrame);
                }
                // set anchored objects as not deleted
                pFrame->SetDrawObjsAsDeleted(false);
            }
            if (frames.empty())
            {
                // in SwUndoSaveSection::SaveSection(), DelFrames() preceded this call
                if (!pNode->FindTableBoxStartNode() && !pNode->FindFlyStartNode())
                {
                    auto const& layouts(rDoc.GetAllLayouts());
                    assert(std::none_of(layouts.begin(), layouts.end(),
                        [](SwRootFrame const*const pLayout) { return pLayout->IsHideRedlines(); }));
                    (void) layouts;
                }
                isAppendObjsCalled = true; // skip that!
                break;
            }

            // no nodes can be unmerged by this - skip MakeFrames() etc.
            if (rPam.GetPoint()->GetNode() == rPam.GetMark()->GetNode())
            {
                break; // continue with AppendAllObjs()
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
                    // update pNode so MakeFrames starts on 2nd node
                    pNode = &rFirstNode;
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
                ::MakeFrames(&rDoc, start.GetNode(), end.GetNode());
                isAppendObjsCalled = true;
                // re-use this to move flys that are now on the wrong frame, with end
                // of redline as "second" node; the nodes between start and end should
                // be complete with MakeFrames already
                sw::MoveMergedFlysAndFootnotes(frames, *pNode, *pLast, false);
            }
            SwNodeIndex tmp(*pLast);
            // skip over hidden sections!
            pNode = static_cast<SwTextNode*>(SwNodes::GoNextSection(&tmp, /*bSkipHidden=*/true, /*bSkipProtect=*/false));
        }
        while (pNode && pNode->GetIndex() <= rPam.End()->GetNodeIndex());
    }

    if (!isAppendObjsCalled)
    {   // recreate flys in the one node the hard way...
        for (auto const& pLayout : rDoc.GetAllLayouts())
        {
            if (pLayout->HasMergedParas())
            {
                AppendAllObjs(rDoc.GetSpzFrameFormats(), pLayout);
                break;
            }
        }
    }
    // fields last - SwGetRefField::UpdateField requires up-to-date frames
    UpdateFieldsForRedline(rDoc.getIDocumentFieldsAccess()); // after footnotes

    const SwTextNode *pTextNode = rPam.GetPointNode().GetTextNode();
    SwTextAttr* pTextAttr = pTextNode ? pTextNode->GetFieldTextAttrAt(rPam.GetPoint()->GetContentIndex() - 1, ::sw::GetTextAttrMode::Default) : nullptr;
    SwTextField *const pTextField(static_txtattr_cast<SwTextField*>(pTextAttr));
    if (pTextField && comphelper::LibreOfficeKit::isActive() )
        rDoc.GetDocShell()->Broadcast(
            SwFormatFieldHint(&pTextField->GetFormatField(), SwFormatFieldHintWhich::INSERTED));
    else
        rDoc.GetDocShell()->Broadcast(
            SwFormatFieldHint(nullptr, SwFormatFieldHintWhich::INSERTED) );
}

} // namespace sw

namespace
{
    bool IsPrevPos( const SwPosition & rPos1, const SwPosition & rPos2 )
    {
        const SwContentNode* pCNd;
        if( 0 != rPos2.GetContentIndex() )
            return false;
        if( rPos2.GetNodeIndex() - 1 != rPos1.GetNodeIndex() )
            return false;
        pCNd = rPos1.GetNode().GetContentNode();
        return pCNd && rPos1.GetContentIndex() == pCNd->Len();
    }

    // copy style or return with SwRedlineExtra_FormatColl with reject data of the upcoming copy
    SwRedlineExtraData_FormatColl* lcl_CopyStyle( const SwPosition & rFrom, const SwPosition & rTo, bool bCopy = true )
    {
        SwTextNode* pToNode = rTo.GetNode().GetTextNode();
        SwTextNode* pFromNode = rFrom.GetNode().GetTextNode();
        if (pToNode != nullptr && pFromNode != nullptr && pToNode != pFromNode)
        {
            const SwPaM aPam(*pToNode);
            SwDoc& rDoc = aPam.GetDoc();
            // using Undo, copy paragraph style
            SwTextFormatColl* pFromColl = pFromNode->GetTextColl();
            SwTextFormatColl* pToColl = pToNode->GetTextColl();
            if (bCopy && pFromColl != pToColl)
                rDoc.SetTextFormatColl(aPam, pFromColl);

            // using Undo, remove direct paragraph formatting of the "To" paragraph,
            // and apply here direct paragraph formatting of the "From" paragraph
            SfxItemSetFixed<
                    RES_PARATR_BEGIN, RES_PARATR_END - 3, // skip RSID and GRABBAG
                    RES_PARATR_LIST_BEGIN, RES_UL_SPACE,  // skip PAGEDESC and BREAK
                    RES_CNTNT, RES_FRMATR_END - 1>
                 aTmp(rDoc.GetAttrPool());
            SfxItemSet aTmp2(aTmp);

            pToNode->GetParaAttr(aTmp, 0, 0);
            pFromNode->GetParaAttr(aTmp2, 0, 0);

            bool bSameSet = aTmp == aTmp2;

            if (!bSameSet)
            {
                for( sal_uInt16 nItem = 0; nItem < aTmp.TotalCount(); ++nItem)
                {
                    sal_uInt16 nWhich = aTmp.GetWhichByOffset(nItem);
                    if( SfxItemState::SET == aTmp.GetItemState( nWhich, false ) &&
                        SfxItemState::SET != aTmp2.GetItemState( nWhich, false ) )
                            aTmp2.Put( aTmp.GetPool()->GetUserOrPoolDefaultItem(nWhich) );
                }
            }

            if (bCopy && !bSameSet)
                rDoc.getIDocumentContentOperations().InsertItemSet(aPam, aTmp2);
            else if (!bCopy && (!bSameSet || pFromColl != pToColl))
                return new SwRedlineExtraData_FormatColl( pFromColl->GetName(), USHRT_MAX, &aTmp2 );
        }
        return nullptr;
    }

    // delete the empty tracked table row (i.e. if it's last tracked deletion was accepted)
    void lcl_DeleteTrackedTableRow ( const SwPosition* pPos )
    {
        const SwTableBox* pBox = pPos->GetNode().GetTableBox();
        if ( !pBox )
            return;

        // tracked column deletion

        const SvxPrintItem *pHasBoxTextChangesOnlyProp =
                pBox->GetFrameFormat()->GetAttrSet().GetItem<SvxPrintItem>(RES_PRINT);
        // empty table cell with property "HasTextChangesOnly" = false
        if ( pHasBoxTextChangesOnlyProp && !pHasBoxTextChangesOnlyProp->GetValue() )
        {
            SwCursor aCursor( *pPos, nullptr );
            if ( pBox->IsEmpty() )
            {
                // tdf#155747 remove table cursor
                pPos->GetDoc().GetDocShell()->GetWrtShell()->EnterStdMode();
                // TODO check the other cells of the column
                // before removing the column
                pPos->GetDoc().DeleteCol( aCursor );
                return;
            }
            else
            {
                SvxPrintItem aHasTextChangesOnly(RES_PRINT, false);
                pPos->GetDoc().SetBoxAttr( aCursor, aHasTextChangesOnly );
            }
        }

        // tracked row deletion

        const SwTableLine* pLine = pBox->GetUpper();
        const SvxPrintItem *pHasTextChangesOnlyProp =
                pLine->GetFrameFormat()->GetAttrSet().GetItem<SvxPrintItem>(RES_PRINT);
        // empty table row with property "HasTextChangesOnly" = false
        if ( pHasTextChangesOnlyProp && !pHasTextChangesOnlyProp->GetValue() )
        {
            if ( pLine->IsEmpty() )
            {
                SwCursor aCursor( *pPos, nullptr );
                pPos->GetDoc().DeleteRow( aCursor );
            }
            else
            {
                // update property "HasTextChangesOnly"
                SwRedlineTable::size_type nPos = 0;
                (void)pLine->UpdateTextChangesOnly(nPos);
            }
        }
    }

    // at rejection of a deletion in a table, remove the tracking of the table row
    // (also at accepting the last redline insertion of a tracked table row insertion)
    void lcl_RemoveTrackingOfTableRow( const SwPosition* pPos, bool bRejectDeletion )
    {
        const SwTableBox* pBox = pPos->GetNode().GetTableBox();
        if ( !pBox )
            return;

        // tracked column deletion

        const SvxPrintItem *pHasBoxTextChangesOnlyProp =
                pBox->GetFrameFormat()->GetAttrSet().GetItem<SvxPrintItem>(RES_PRINT);
        // table cell property "HasTextChangesOnly" is set and its value is false
        if ( pHasBoxTextChangesOnlyProp && !pHasBoxTextChangesOnlyProp->GetValue() )
        {
            SvxPrintItem aUnsetTracking(RES_PRINT, true);
            SwCursor aCursor( *pPos, nullptr );
            pPos->GetDoc().SetBoxAttr( aCursor, aUnsetTracking );
        }

        // tracked row deletion

        const SwTableLine* pLine = pBox->GetUpper();
        const SvxPrintItem *pHasTextChangesOnlyProp =
                pLine->GetFrameFormat()->GetAttrSet().GetItem<SvxPrintItem>(RES_PRINT);
        // table row property "HasTextChangesOnly" is set and its value is false
        if ( pHasTextChangesOnlyProp && !pHasTextChangesOnlyProp->GetValue() )
        {
            bool bNoMoreInsertion = false;
            if ( !bRejectDeletion )
            {
                SwRedlineTable::size_type nPos = 0;
                SwRedlineTable::size_type nInsert = pLine->UpdateTextChangesOnly(nPos, /*bUpdateProperty=*/false);

                if ( SwRedlineTable::npos == nInsert )
                    bNoMoreInsertion = true;
            }
            if ( bRejectDeletion || bNoMoreInsertion )
            {
                SvxPrintItem aUnsetTracking(RES_PRINT, true);
                SwCursor aCursor( *pPos, nullptr );
                pPos->GetDoc().SetRowNotTracked( aCursor, aUnsetTracking );
            }
        }
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
                    {
                        bool bInsert = RedlineType::Insert == pRedl->GetType();
                        SwPosition aPos(pRedl->Start()->GetNode());
                        rArr.DeleteAndDestroy( rPos-- );

                        // remove tracking of the table row, if needed
                        if ( bInsert )
                            lcl_RemoveTrackingOfTableRow( &aPos, /*bRejectDelete=*/false );
                    }
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
                SwDoc& rDoc = pRedl->GetDoc();
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
                    SwContentNode* pCSttNd = pDelStt->GetNode().GetContentNode();
                    SwContentNode* pCEndNd = pDelEnd->GetNode().GetContentNode();
                    pRStt = pRedl->Start();
                    pREnd = pRedl->End();

                    // keep style of the empty paragraph after deletion of wholly paragraphs
                    if( pCSttNd && pCEndNd && pRStt && pREnd && pRStt->GetContentIndex() == 0 )
                        lcl_CopyStyle(*pREnd, *pRStt);

                    if( bDelRedl )
                        delete pRedl;

                    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
                    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

                    if( pCSttNd && pCEndNd )
                    {
                        rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                        lcl_DeleteTrackedTableRow( aPam.End() );
                    }
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
        SwDoc& rDoc = pRedl->GetDoc();
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

                    SwContentNode* pCSttNd = pDelStt->GetNode().GetContentNode();
                    SwContentNode* pCEndNd = pDelEnd->GetNode().GetContentNode();

                    if( bDelRedl )
                        delete pRedl;

                    RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
                    rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore));

                    if( pCSttNd && pCEndNd )
                    {
                        rDoc.getIDocumentContentOperations().DeleteAndJoin( aPam );
                        lcl_DeleteTrackedTableRow( aPam.End() );
                    }
                    else if (pCSttNd && !pCEndNd)
                        {
                            aPam.GetBound().nContent.Assign( nullptr, 0 );
                            aPam.GetBound( false ).nContent.Assign( nullptr, 0 );
                            if (aPam.End()->GetNode().IsStartNode())
                            {   // end node will be deleted too! see nNodeDiff+1
                                aPam.End()->Adjust(SwNodeOffset(-1));
                            }
                            assert(!aPam.End()->GetNode().IsStartNode());
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

                // remove tracking of the table row, if needed
                lcl_RemoveTrackingOfTableRow( updatePaM.End(), /*bRejectDelete=*/true );

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
                    SwTextNode* pTNd = pStt->GetNode().GetTextNode();
                    if( pTNd )
                    {
                        // expand range to the whole paragraph
                        // and reset only the paragraph attributes
                        SwPaM aPam( *pTNd, pTNd->GetText().getLength() );
                        o3tl::sorted_vector<sal_uInt16> aResetAttrsArray;

                        constexpr std::pair<sal_uInt16, sal_uInt16> aResetableSetRange[] = {
                            { RES_PARATR_BEGIN, RES_PARATR_END - 1 },
                            { RES_PARATR_LIST_BEGIN, RES_FRMATR_END - 1 },
                        };

                        for (const auto& [nBegin, nEnd] : aResetableSetRange)
                        {
                            for (sal_uInt16 i = nBegin; i <= nEnd; ++i)
                                aResetAttrsArray.insert( i );
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

    bool lcl_AcceptInnerInsertRedline(SwRedlineTable& rArr, SwRedlineTable::size_type& rPos,
                                      int nDepth)
    {
        SwRangeRedline* pRedl = rArr[rPos];
        SwDoc& rDoc = pRedl->GetDoc();
        SwPaM const updatePaM(*pRedl->Start(), *pRedl->End());

        pRedl->PopAllDataAfter(nDepth);
        sw::UpdateFramesForRemoveDeleteRedline(rDoc, updatePaM);
        return true;
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
                        * pEnd = rPam.End();
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
        auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*
        SwDoc& rDoc = rPam.GetDoc();
        if( !pStt->GetContentIndex() &&
            !rDoc.GetNodes()[ pStt->GetNodeIndex() - 1 ]->IsContentNode() )
        {
            const SwRangeRedline* pRedl = rDoc.getIDocumentRedlineAccess().GetRedline( *pStt, nullptr );
            if( pRedl )
            {
                const SwPosition* pRStt = pRedl->Start();
                if( !pRStt->GetContentIndex() && pRStt->GetNodeIndex() ==
                    pStt->GetNodeIndex() - 1 )
                    *pStt = *pRStt;
            }
        }
        if( pEnd->GetNode().IsContentNode() &&
            !rDoc.GetNodes()[ pEnd->GetNodeIndex() + 1 ]->IsContentNode() &&
            pEnd->GetContentIndex() == pEnd->GetNode().GetContentNode()->Len()    )
        {
            const SwRangeRedline* pRedl = rDoc.getIDocumentRedlineAccess().GetRedline( *pEnd, nullptr );
            if( pRedl )
            {
                const SwPosition* pREnd = pRedl->End();
                if( !pREnd->GetContentIndex() && pREnd->GetNodeIndex() ==
                    pEnd->GetNodeIndex() + 1 )
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
                m_rRedline.GetMark()->Assign(rDoc.GetNodes().GetEndOfContent());
            }
            m_rRedline.GetPoint()->Assign(rDoc.GetNodes().GetEndOfContent());
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
        void (SwRangeRedline::*pFnc)(sal_uInt16, size_t, bool); // Allow compiler warn if use of
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
            for (size_t i = 0; i < maRedlineTable.size(); ++i)
            {
                SwRangeRedline *const pRedline = maRedlineTable[i];
                (pRedline->*pFnc)(nLoop, i, false);
                while (maRedlineTable.size() <= i
                    || maRedlineTable[i] != pRedline)
                {        // ensure current position
                    --i; // a previous redline may have been deleted
                }
            }

        //SwRangeRedline::MoveFromSection routinely changes
        //the keys that mpRedlineTable is sorted by
        maRedlineTable.Resort();

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
    return maRedlineTable;
}

SwRedlineTable& DocumentRedlineManager::GetRedlineTable()
{
    return maRedlineTable;
}

const SwExtraRedlineTable& DocumentRedlineManager::GetExtraRedlineTable() const
{
    return maExtraRedlineTable;
}

SwExtraRedlineTable& DocumentRedlineManager::GetExtraRedlineTable()
{
    return maExtraRedlineTable;
}

bool DocumentRedlineManager::IsInRedlines(const SwNode & rNode) const
{
    if (&rNode.GetNodes() != &m_rDoc.GetNodes())
        return false;

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
DocumentRedlineManager::AppendRedline(SwRangeRedline* pNewRedl, bool const bCallDelete,
                                      sal_uInt32 nMoveIDToDelete)
{
    CHECK_REDLINE( *this )

    if (!IsRedlineOn() || IsShowOriginal(meRedlineFlags))
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
        CHECK_REDLINE( *this )
        return AppendResult::IGNORED;
    }

    // Collect MoveID's of the redlines we delete.
    // If there is only 1, then we should use its ID. (continuing the move)
    std::set<sal_uInt32> deletedMoveIDs;

    bool bMerged = false;

    pNewRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);

    if( m_rDoc.IsAutoFormatRedline() )
    {
        pNewRedl->SetAutoFormat();
        if( moAutoFormatRedlnComment && !moAutoFormatRedlnComment->isEmpty() )
        {
            pNewRedl->SetComment( *moAutoFormatRedlnComment );
            pNewRedl->SetSeqNo( mnAutoFormatRedlnCommentNo );
        }
    }

    auto [pStt, pEnd] = pNewRedl->StartEnd(); // SwPosition*
    {
        SwTextNode* pTextNode = pStt->GetNode().GetTextNode();
        if( pTextNode == nullptr )
        {
            if( pStt->GetContentIndex() > 0 )
            {
                OSL_ENSURE( false, "Redline start: non-text-node with content" );
                pStt->SetContent( 0 );
            }
        }
        else
        {
            if( pStt->GetContentIndex() > pTextNode->Len() )
            {
                OSL_ENSURE( false, "Redline start: index after text" );
                pStt->SetContent( pTextNode->Len() );
            }
        }
        pTextNode = pEnd->GetNode().GetTextNode();
        if( pTextNode == nullptr )
        {
            if( pEnd->GetContentIndex() > 0 )
            {
                OSL_ENSURE( false, "Redline end: non-text-node with content" );
                pEnd->SetContent(0);
            }
        }
        else
        {
            if( pEnd->GetContentIndex() > pTextNode->Len() )
            {
                OSL_ENSURE( false, "Redline end: index after text" );
                pEnd->SetContent( pTextNode->Len() );
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
    const SwRedlineTable::size_type nStartPos = n;
    bool bDec = false;

    for( ; pNewRedl && n < maRedlineTable.size(); bDec ? n : ++n )
    {
        bDec = false;

        SwRangeRedline* pRedl = maRedlineTable[ n ];
        auto [pRStt, pREnd] = pRedl->StartEnd();

        // #i8518# remove empty redlines while we're at it
        if( ( *pRStt == *pREnd ) &&
            ( pRedl->GetContentIdx() == nullptr ) )
        {
            maRedlineTable.DeleteAndDestroy(n);
            continue;
        }

        SwComparePosition eCmpPos = ComparePosition( *pStt, *pEnd, *pRStt, *pREnd );

        if ( SwComparePosition::Before == eCmpPos && !IsPrevPos( *pEnd, *pRStt ))
            break;

        switch( pNewRedl->GetType() )
        {
        case RedlineType::Insert:
            switch( pRedl->GetType() )
            {
            case RedlineType::Insert:
                if( pRedl->IsOwnRedline( *pNewRedl ) &&
                    // don't join inserted characters with moved text
                    !pRedl->IsMoved() )
                {
                    bool bDelete = false;
                    bool bMaybeNotify = false;

                    // Merge if applicable?
                    if( (( SwComparePosition::Behind == eCmpPos &&
                           IsPrevPos( *pREnd, *pStt ) ) ||
                         ( SwComparePosition::CollideStart == eCmpPos ) ||
                         ( SwComparePosition::OverlapBehind == eCmpPos ) ) &&
                        pRedl->CanCombine( *pNewRedl ) &&
                        ( n+1 >= maRedlineTable.size() ||
                         ( *maRedlineTable[ n+1 ]->Start() >= *pEnd &&
                         *maRedlineTable[ n+1 ]->Start() != *pREnd ) ) )
                    {
                        pRedl->SetEnd( *pEnd, pREnd );
                        if( !pRedl->HasValidRange() )
                        {
                            // re-insert
                            maRedlineTable.Remove( n );
                            maRedlineTable.Insert( pRedl );
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
                         *maRedlineTable[ n-1 ]->End() != *pRStt ))
                    {
                        pRedl->SetStart( *pStt, pRStt );
                        // re-insert
                        maRedlineTable.Remove( n );
                        maRedlineTable.Insert( pRedl );

                        bMerged = true;
                        bDelete = true;
                    }
                    else if ( SwComparePosition::Outside == eCmpPos )
                    {
                        // own insert-over-insert redlines:
                        // just scrap the inside ones
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = true;
                    }
                    else if( SwComparePosition::OverlapBehind == eCmpPos )
                    {
                        *pStt = *pREnd;
                        if( ( *pStt == *pEnd ) &&
                            ( pNewRedl->GetContentIdx() == nullptr ) )
                            bDelete = bMaybeNotify = true;
                    }
                    else if( SwComparePosition::OverlapBefore == eCmpPos )
                    {
                        *pEnd = *pRStt;
                        if( ( *pStt == *pEnd ) &&
                            ( pNewRedl->GetContentIdx() == nullptr ) )
                            bDelete = bMaybeNotify = true;
                    }
                    else if( SwComparePosition::Inside == eCmpPos )
                    {
                        bDelete = bMaybeNotify = true;
                        bMerged = true;
                    }
                    else if( SwComparePosition::Equal == eCmpPos )
                        bDelete = bMaybeNotify = true;

                    if( bDelete )
                    {
                        delete pNewRedl;
                        pNewRedl = nullptr;
                        bCompress = true;

                        if (bMaybeNotify)
                            MaybeNotifyRedlineModification(*pRedl, m_rDoc);

                        // set IsMoved checking nearby redlines
                        if (n < maRedlineTable.size()) // in case above 're-insert' failed
                            maRedlineTable.isMoved(n);
                    }
                }
                else if( SwComparePosition::Inside == eCmpPos )
                {
                    // split up
                    if( *pEnd != *pREnd )
                    {
                        SwRangeRedline* pCpy = new SwRangeRedline( *pRedl );
                        pCpy->SetStart( *pEnd );
                        maRedlineTable.Insert( pCpy );
                    }
                    pRedl->SetEnd( *pStt, pREnd );
                    if( ( *pStt == *pRStt ) &&
                        ( pRedl->GetContentIdx() == nullptr ) )
                    {
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = true;
                    }
                    else if( !pRedl->HasValidRange() )
                    {
                        // re-insert
                        maRedlineTable.Remove( n );
                        maRedlineTable.Insert( pRedl );
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
                    maRedlineTable.Insert( pSplit );
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

                        MaybeNotifyRedlineModification(*pRedl, m_rDoc);
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
                        maRedlineTable.Insert( pCpy );
                    }
                    pRedl->SetEnd( *pStt, pREnd );
                    if( ( *pStt == *pRStt ) &&
                        ( pRedl->GetContentIdx() == nullptr ) )
                    {
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = true;
                    }
                    else if( !pRedl->HasValidRange() )
                    {
                        // re-insert
                        maRedlineTable.Remove( n );
                        maRedlineTable.Insert( pRedl, n );
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
                    maRedlineTable.Insert( pSplit );
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
                    maRedlineTable.DeleteAndDestroy( n );
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
                    maRedlineTable.Remove( n );
                    maRedlineTable.Insert( pRedl, n );
                    bDec = true;
                    break;

                case SwComparePosition::OverlapBehind:
                    pRedl->SetEnd( *pStt, pREnd );
                    if( *pStt == *pRStt && pRedl->GetContentIdx() == nullptr )
                    {
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = true;
                    }
                    break;

                case SwComparePosition::Equal:
                case SwComparePosition::Outside:
                    // Overlaps the current one completely or has the
                    // same dimension, delete the old one
                    maRedlineTable.DeleteAndDestroy( n );
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
                                maRedlineTable.DeleteAndDestroy( n );
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

                    MaybeNotifyRedlineModification(*pRedl, m_rDoc);
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
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = true;
                    }
                    else if( SwComparePosition::OverlapBehind == eCmpPos )
                        pNewRedl->SetStart( *pREnd, pStt );
                    else
                        pNewRedl->SetEnd( *pRStt, pEnd );
                    break;

                case SwComparePosition::CollideEnd:
                    if (pRStt->GetContentIndex() != 0
                        && pRStt->GetNode() != pREnd->GetNode())
                    {   // tdf#147466 HACK: don't combine in this case to avoid the tdf#119571 code from *undeleting* section nodes
                        break;
                    }
                    [[fallthrough]];
                case SwComparePosition::CollideStart:
                    if( pRedl->IsOwnRedline( *pNewRedl ) &&
                        pRedl->CanCombine( *pNewRedl ) )
                    {
                        if( IsHideChanges( meRedlineFlags ))
                        {
                            // Before we can merge, we make it visible!
                            // We insert temporarily so that pNew is
                            // also dealt with when moving the indices.
                            maRedlineTable.Insert(pNewRedl);
                            pRedl->Show(0, maRedlineTable.GetPos(pRedl));
                            maRedlineTable.Remove( pNewRedl );
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

                        maRedlineTable.DeleteAndDestroy( nToBeDeleted );
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
                    pRedl->IsOwnRedline( *pNewRedl ) &&
                    // tdf#116084 tdf#121176 don't combine anonymized deletion
                    // and anonymized insertion, i.e. with the same dummy timestamp
                    !pRedl->GetRedlineData(0).IsAnonymized() )
                {
                    // Collect MoveID's of the redlines we delete.
                    if (nMoveIDToDelete > 1 && maRedlineTable[n]->GetMoved() > 0
                        && (eCmpPos == SwComparePosition::Equal
                            || eCmpPos == SwComparePosition::Inside
                            || eCmpPos == SwComparePosition::Outside
                            || eCmpPos == SwComparePosition::OverlapBefore
                            || eCmpPos == SwComparePosition::OverlapBehind))
                    {
                        deletedMoveIDs.insert(maRedlineTable[n]->GetMoved());
                    }

                    // Set to NONE, so that the Delete::Redo merges the Redline data correctly!
                    // The ShowMode needs to be retained!
                    meRedlineFlags = eOld & ~RedlineFlags(RedlineFlags::On | RedlineFlags::Ignore);
                    switch( eCmpPos )
                    {
                    case SwComparePosition::Equal:
                        bCompress = true;
                        maRedlineTable.DeleteAndDestroy( n );
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
                            if( (pStt->GetContentIndex() == 0) &&
                                pEnd->GetNode().IsEndNode() )
                            {
                                pEnd->Adjust(SwNodeOffset(-1));
                                m_rDoc.getIDocumentContentOperations().DelFullPara( *pNewRedl );
                            }
                            else
                                m_rDoc.getIDocumentContentOperations().DeleteAndJoin( *pNewRedl );

                            bCompress = true;
                        }
                        delete pNewRedl;
                        pNewRedl = nullptr;

                        // No need to call MaybeNotifyRedlineModification, because a notification
                        // was already sent in DocumentRedlineManager::DeleteRedline
                        break;

                    case SwComparePosition::Outside:
                        {
                            maRedlineTable.Remove( n );
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
                                maRedlineTable.DeleteAndDestroy( n );
                            else
                            {
                                pRedl->SetStart( *pEnd, pRStt );
                                // re-insert
                                maRedlineTable.Remove( n );
                                maRedlineTable.Insert( pRedl, n );
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
                                maRedlineTable.DeleteAndDestroy( n );
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
                                pRedl->Hide(0, maRedlineTable.GetPos(pRedl));
                            }
                            bCompress = true;

                            // set IsMoved checking nearby redlines
                            SwRedlineTable::size_type nRIdx = maRedlineTable.GetPos(pRedl);
                            if (nRIdx < maRedlineTable.size()) // in case above 're-insert' failed
                                maRedlineTable.isMoved(nRIdx);

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
                                    maRedlineTable.Remove( n );
                                    maRedlineTable.Insert( pRedl, n );
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
                                    maRedlineTable.Remove( n );
                                    maRedlineTable.Insert( pRedl, n );
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
                                    maRedlineTable.Insert(pNewRedl);
                                    pRedl->Hide(0, maRedlineTable.GetPos(pRedl));
                                    maRedlineTable.Remove( pNewRedl );
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
                                maRedlineTable.Remove( n );
                                maRedlineTable.Insert( pRedl );
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
                                    maRedlineTable.Insert( pNewRedl );
                                    pRedl->Hide(0, maRedlineTable.GetPos(pRedl));
                                    maRedlineTable.Remove( pNewRedl );
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
                                    maRedlineTable.Remove( n );
                                    maRedlineTable.Insert( pRedl );
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
                        maRedlineTable.Insert( pNew );

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
                    maRedlineTable.Remove( n );
                    maRedlineTable.Insert( pRedl, n );
                    bDec = true;
                    break;

                case SwComparePosition::OverlapBehind:
                    pRedl->SetEnd( *pStt, pREnd );
                    break;

                case SwComparePosition::Equal:
                case SwComparePosition::Outside:
                    // Overlaps the current one completely or has the
                    // same dimension, delete the old one
                    maRedlineTable.DeleteAndDestroy( n );
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
                                maRedlineTable.DeleteAndDestroy( n );
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

                    MaybeNotifyRedlineModification(*pRedl, m_rDoc);
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
                        maRedlineTable.DeleteAndDestroy( n );
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

                        MaybeNotifyRedlineModification(*pRedl, m_rDoc);
                    }
                    else if( *pREnd == *pEnd )
                        // or else only shorten the current one
                        pRedl->SetEnd( *pStt, pREnd );
                    else if( *pRStt == *pStt )
                    {
                        // or else only shorten the current one
                        pRedl->SetStart( *pEnd, pRStt );
                        // re-insert
                        maRedlineTable.Remove( n );
                        maRedlineTable.Insert( pRedl, n );
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
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = false;
                    }
                    else if( SwComparePosition::OverlapBehind == eCmpPos )
                        pNewRedl->SetStart( *pREnd, pStt );
                    else
                        pNewRedl->SetEnd( *pRStt, pEnd );
                    break;

                case SwComparePosition::CollideEnd:
                    if( pRedl->IsOwnRedline( *pNewRedl ) &&
                        pRedl->CanCombine( *pNewRedl ) &&
                        (n == 0 || *maRedlineTable[ n-1 ]->End() < *pStt))
                    {
                        // If that's the case we can merge it, meaning
                        // the new one covers this well
                        pNewRedl->SetEnd( *pREnd, pEnd );
                        maRedlineTable.DeleteAndDestroy( n );
                        bDec = true;
                    }
                    break;
                case SwComparePosition::CollideStart:
                    if( pRedl->IsOwnRedline( *pNewRedl ) &&
                        pRedl->CanCombine( *pNewRedl ) &&
                        (n+1 >= maRedlineTable.size() ||
                         (*maRedlineTable[ n+1 ]->Start() >= *pEnd &&
                          *maRedlineTable[ n+1 ]->Start() != *pREnd)))
                    {
                        // If that's the case we can merge it, meaning
                        // the new one covers this well
                        pNewRedl->SetStart( *pRStt, pStt );
                        maRedlineTable.DeleteAndDestroy( n );
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
                if ( pStt->GetContentIndex() != 0 )
                {
                    // tdf#119571 update the style of the joined paragraph
                    // after a partially deleted paragraph to show its correct style
                    // in "Show changes" mode, too. All removed paragraphs
                    // get the style of the first (partially deleted) paragraph
                    // to avoid text insertion with bad style in the deleted
                    // area later (except paragraphs of the removed tables).

                    SwContentNode* pDelNd = pStt->GetNode().GetContentNode();
                    // start copying the style of the first paragraph from the end of the range
                    SwContentNode* pTextNd = pEnd->GetNode().GetContentNode();
                    SwNodeIndex aIdx( pEnd->GetNode() );
                    bool bFirst = true;

                    while (pTextNd != nullptr && pDelNd->GetIndex() < pTextNd->GetIndex())
                    {
                        if( pTextNd->IsTextNode() )
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

                                // skip empty redlines without ExtraData
                                // FIXME: maybe checking pExtraData is redundant here
                                if ( pExtraData || *pPar->Start() != *pPar->End() )
                                    maRedlineTable.Insert( pPar );
                                else
                                    delete pPar;
                            }

                            // modify paragraph formatting
                            lcl_CopyStyle(*pStt, aPos);
                        }

                        if (bFirst)
                            bFirst = false;

                        // Jump to the previous paragraph and if needed, skip paragraphs of
                        // the removed table(s) in the range to avoid leaving empty tables
                        // because of the non-continuous redline range over the table.
                        // FIXME: this is not enough for tables with inner redlines, where
                        // tracked deletion of the text containing such a table leaves an
                        // empty table at the place of the table (a problem inherited from OOo).
                        pTextNd = nullptr;
                        while( --aIdx > *pDelNd && !aIdx.GetNode().IsContentNode() )
                        {
                            // possible table end
                            if( aIdx.GetNode().IsEndNode() && aIdx.GetNode().FindTableNode() )
                            {
                                SwNodeIndex aIdx2 = aIdx;
                                // search table start and skip table paragraphs
                                while ( pDelNd->GetIndex() < aIdx2.GetIndex() )
                                {
                                    SwTableNode* pTable = aIdx2.GetNode().GetTableNode();
                                    if( pTable &&
                                        pTable->EndOfSectionNode()->GetIndex() == aIdx.GetIndex() )
                                    {
                                       aIdx = aIdx2;
                                       break;
                                    }
                                    --aIdx2;
                                }
                            }
                        }

                        if (aIdx.GetNode().IsContentNode())
                            pTextNd = aIdx.GetNode().GetContentNode();
                    }
                }

                // delete tables of the deletion explicitly, to avoid
                // remaining empty tables after accepting the rejection
                // and visible empty tables in Hide Changes mode
                // (this was the case, if tables have already contained
                // other tracked changes)
                // FIXME: because of recursive nature of AppendRedline,
                // this doesn't work for selections with multiple tables
                if ( m_rDoc.GetIDocumentUndoRedo().DoesUndo() )
                {
                    SwNodeIndex aSttIdx( pStt->GetNode() );
                    SwNodeIndex aEndIdx( pEnd->GetNode() );
                    while ( aSttIdx < aEndIdx )
                    {
                        if ( aSttIdx.GetNode().IsTableNode() )
                        {
                            SvxPrintItem aHasTextChangesOnly(RES_PRINT, false);
                            SwCursor aCursor( SwPosition(aSttIdx), nullptr );
                            m_rDoc.SetRowNotTracked( aCursor, aHasTextChangesOnly, /*bAll=*/true );
                        }
                        ++aSttIdx;
                    }
                }
            }
            bool const ret = maRedlineTable.Insert( pNewRedl );
            assert(ret || !pNewRedl);
            if (ret && !pNewRedl)
            {
                bMerged = true; // treat InsertWithValidRanges as "merge"
            }
        }
    }

    // If we deleted moved redlines, and there was only 1 MoveID, then we should use that
    // We overwrite those that was given right now, so it cannot be deeper under other redline
    if (nMoveIDToDelete > 1 && deletedMoveIDs.size() == 1)
    {
        sal_uInt32 nNewMoveID = *(deletedMoveIDs.begin());
        if (nNewMoveID > 1)     // MoveID==1 is for old, unrecognised moves, leave them alone
        {
            for (n = 0; n < maRedlineTable.size(); ++n)
            {
                if (maRedlineTable[n]->GetMoved() == nMoveIDToDelete)
                {
                    maRedlineTable[n]->SetMoved(nNewMoveID);
                }
            }
        }
    }

    if( bCompress )
        CompressRedlines(nStartPos);

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

        maExtraRedlineTable.Insert( pNewRedl );
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

        maExtraRedlineTable.Insert( pNewRedl );
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

void DocumentRedlineManager::CompressRedlines(size_t nStartIndex)
{
    CHECK_REDLINE( *this )

    void (SwRangeRedline::*pFnc)(sal_uInt16, size_t, bool) = nullptr;
    RedlineFlags eShow = RedlineFlags::ShowMask & meRedlineFlags;
    if( eShow == (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete))
        pFnc = &SwRangeRedline::Show;
    else if (eShow == RedlineFlags::ShowInsert)
        pFnc = &SwRangeRedline::Hide;

    // Try to merge identical ones
    if (nStartIndex == 0)
        nStartIndex = 1;
    for( SwRedlineTable::size_type n = nStartIndex; n < maRedlineTable.size(); ++n )
    {
        SwRangeRedline* pPrev = maRedlineTable[ n-1 ],
                    * pCur = maRedlineTable[ n ];
        auto [pPrevStt,pPrevEnd] = pPrev->StartEnd();
        auto [pCurStt, pCurEnd] = pCur->StartEnd();

        if( *pPrevEnd == *pCurStt && pPrev->CanCombine( *pCur ) &&
            pPrevStt->GetNode().StartOfSectionNode() ==
            pCurEnd->GetNode().StartOfSectionNode() &&
            !pCurEnd->GetNode().StartOfSectionNode()->IsTableNode() )
        {
            // we then can merge them
            SwRedlineTable::size_type nPrevIndex = n-1;
            pPrev->Show(0, nPrevIndex);
            pCur->Show(0, n);

            pPrev->SetEnd( *pCur->End() );
            maRedlineTable.DeleteAndDestroy( n );
            --n;
            if( pFnc )
                (pPrev->*pFnc)(0, nPrevIndex, false);
        }
    }
    CHECK_REDLINE( *this )

    // #TODO - add 'SwExtraRedlineTable' also ?
}

bool DocumentRedlineManager::SplitRedline( const SwPaM& rRange )
{
    if (maRedlineTable.empty())
        return false;
    auto [pStt, pEnd] = rRange.StartEnd(); // SwPosition*
    // tdf#144208 this happens a lot during load of some DOCX files.
    if (*pEnd > maRedlineTable.GetMaxEndPos())
        return false;
    bool bChg = false;
    SwRedlineTable::size_type n = 0;
    //FIXME overlapping problem GetRedline( *pStt, &n );
    for ( ; n < maRedlineTable.size(); ++n)
    {
        SwRangeRedline * pRedline = maRedlineTable[ n ];
        auto [pRedlineStart, pRedlineEnd] = pRedline->StartEnd();
        if (*pRedlineStart <= *pStt && *pEnd <= *pRedlineEnd)
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
                maRedlineTable.DeleteAndDestroy( n-- );
                pRedline = nullptr;
                break;
            }
            if (pRedline && !pRedline->HasValidRange())
            {
                // re-insert
                maRedlineTable.Remove( n );
                maRedlineTable.Insert( pRedline, n );
            }
            if( pNew )
                maRedlineTable.Insert( pNew, n );
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

    auto [pStt, pEnd] = rRange.StartEnd(); // SwPosition*
    SwRedlineTable::size_type n = 0;
    GetRedline( *pStt, &n );
    for( ; n < maRedlineTable.size() ; ++n )
    {
        SwRangeRedline* pRedl = maRedlineTable[ n ];
        if( RedlineType::Any != nDelType && nDelType != pRedl->GetType() )
            continue;

        auto [pRStt, pREnd] = pRedl->StartEnd(); // SwPosition*
        switch( ComparePosition( *pStt, *pEnd, *pRStt, *pREnd ) )
        {
        case SwComparePosition::Equal:
        case SwComparePosition::Outside:
            pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
            maRedlineTable.DeleteAndDestroy( n-- );
            bChg = true;
            break;

        case SwComparePosition::OverlapBefore:
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                pRedl->SetStart( *pEnd, pRStt );
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
                // re-insert
                maRedlineTable.Remove( n );
                maRedlineTable.Insert( pRedl );
                --n;
            break;

        case SwComparePosition::OverlapBehind:
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                pRedl->SetEnd( *pStt, pREnd );
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Add);
                if( !pRedl->HasValidRange() )
                {
                    // re-insert
                    maRedlineTable.Remove( n );
                    maRedlineTable.Insert( pRedl );
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
                    maRedlineTable.Remove( n );
                    maRedlineTable.Insert( pRedl );
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
                        maRedlineTable.Remove( n );
                        maRedlineTable.Insert( pRedl );
                        --n;
                    }
                    if( pCpy )
                        maRedlineTable.Insert( pCpy );
                }
            }
            break;

        case SwComparePosition::CollideEnd:
            // remove (not hidden) empty redlines created for fixing tdf#119571
            // (Note: hidden redlines are all empty, i.e. start and end are equal.)
            if ( pRedl->HasMark() && *pRedl->GetMark() == *pRedl->GetPoint() )
            {
                pRedl->InvalidateRange(SwRangeRedline::Invalidation::Remove);
                maRedlineTable.DeleteAndDestroy( n-- );
                bChg = true;
                break;
            }
            [[fallthrough]];

        case SwComparePosition::Before:
            n = maRedlineTable.size();
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
    const SwNodeOffset nNdIdx = rNd.GetIndex();
    // if the table only contains good (i.e. non-overlapping) data, we can do a binary search
    if (!maRedlineTable.HasOverlappingElements())
    {
        // binary search to the first redline with end >= the needle
        auto it = std::lower_bound(maRedlineTable.begin(), maRedlineTable.end(), rNd,
            [&nNdIdx](const SwRangeRedline* lhs, const SwNode& /*rhs*/)
            {
                return lhs->End()->GetNodeIndex() < nNdIdx;
            });
        for( ; it != maRedlineTable.end(); ++it)
        {
            const SwRangeRedline* pTmp = *it;
            auto [pStart, pEnd] = pTmp->StartEnd(); // SwPosition*
            SwNodeOffset nStart = pStart->GetNodeIndex(),
                         nEnd = pEnd->GetNodeIndex();

            if( ( RedlineType::Any == nType || nType == pTmp->GetType()) &&
                nStart <= nNdIdx && nNdIdx <= nEnd )
                return std::distance(maRedlineTable.begin(), it);

            if( nStart > nNdIdx )
                break;
        }
    }
    else
    {
        for( auto it = maRedlineTable.begin(), itEnd = maRedlineTable.end(); it != itEnd; ++it )
        {
            const SwRangeRedline* pTmp = *it;
            SwNodeOffset nPt = pTmp->GetPoint()->GetNodeIndex(),
                  nMk = pTmp->GetMark()->GetNodeIndex();
            if( nPt < nMk )
                std::swap( nMk, nPt );

            if( ( RedlineType::Any == nType || nType == pTmp->GetType()) &&
                nMk <= nNdIdx && nNdIdx <= nPt )
                return std::distance(maRedlineTable.begin(), it);

            if( nMk > nNdIdx )
                break;
        }
    }
    return SwRedlineTable::npos;

    // #TODO - add 'SwExtraRedlineTable' also ?
}

SwRedlineTable::size_type
DocumentRedlineManager::GetRedlineEndPos(SwRedlineTable::size_type nStartPos, const SwNode& rNd,
                                         RedlineType nType) const
{
    //if the start is already invalid
    if (nStartPos >= maRedlineTable.size())
        return nStartPos;

    const SwNodeOffset nNdIdx = rNd.GetIndex();
    SwRedlineTable::size_type nEndPos = nStartPos;
    SwRedlineTable::size_type nEndPosTry = nEndPos + 1;

    while (nEndPosTry < maRedlineTable.size()
           && maRedlineTable[nEndPosTry]->Start()->GetNodeIndex() <= nNdIdx)
    {
        if (RedlineType::Any == nType || nType == maRedlineTable[nEndPosTry]->GetType())
        {
            nEndPos = nEndPosTry;
        }
        nEndPosTry++;
    }
    return nEndPos;
}

void DocumentRedlineManager::UpdateRedlineContentNode(SwRedlineTable::size_type nStartPos,
                                                      SwRedlineTable::size_type nEndPos) const
{
    for (SwRedlineTable::size_type n = nStartPos; n <= nEndPos; ++n)
    {
        //just in case we got wrong input
        if (n >= maRedlineTable.size())
            return;

        SwPosition* pStart = maRedlineTable[n]->Start();
        SwPosition* pEnd = maRedlineTable[n]->End();
        SwContentNode* pCont = pStart->GetNode().GetContentNode();
        if (pCont)
        {
            pStart->nContent.Assign(pCont, pStart->nContent.GetIndex());
        }
        pCont = pEnd->GetNode().GetContentNode();
        if (pCont)
        {
            pEnd->nContent.Assign(pCont, pEnd->nContent.GetIndex());
        }
    }
}

bool DocumentRedlineManager::HasRedline( const SwPaM& rPam, RedlineType nType, bool bStartOrEndInRange ) const
{
    SwPosition currentStart(*rPam.Start());
    SwPosition currentEnd(*rPam.End());
    const SwNode& rEndNode(currentEnd.GetNode());

    for( SwRedlineTable::size_type n = GetRedlinePos( rPam.Start()->GetNode(), nType );
                    n < maRedlineTable.size(); ++n )
    {
        const SwRangeRedline* pTmp = maRedlineTable[ n ];

        if ( pTmp->Start()->GetNode() > rEndNode )
            break;

        if( RedlineType::Any != nType && nType != pTmp->GetType() )
            continue;

        // redline over the range
        if ( currentStart < *pTmp->End() && *pTmp->Start() <= currentEnd &&
             // starting or ending within the range
             ( !bStartOrEndInRange ||
                 ( currentStart < *pTmp->Start() || *pTmp->End() < currentEnd ) ) )
        {
            return true;
        }
    }
    return false;
}

const SwRangeRedline* DocumentRedlineManager::GetRedline( const SwPosition& rPos,
                                    SwRedlineTable::size_type* pFndPos ) const
{
    SwRedlineTable::size_type nO = maRedlineTable.size(), nM, nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            nM = nU + ( nO - nU ) / 2;
            const SwRangeRedline* pRedl = maRedlineTable[ nM ];
            auto [pStt, pEnd] = pRedl->StartEnd();
            if( pEnd == pStt
                    ? *pStt == rPos
                    : ( *pStt <= rPos && rPos < *pEnd ) )
            {
                while( nM && rPos == *maRedlineTable[ nM - 1 ]->End() &&
                    rPos == *maRedlineTable[ nM - 1 ]->Start() )
                {
                    --nM;
                    pRedl = maRedlineTable[ nM ];
                }
                // if there are format and insert changes in the same position
                // show insert change first.
                // since the redlines are sorted by position, only check the redline
                // before and after the current redline
                if( RedlineType::Format == pRedl->GetType() )
                {
                    if( nM && rPos >= *maRedlineTable[ nM - 1 ]->Start() &&
                        rPos <= *maRedlineTable[ nM - 1 ]->End() &&
                        ( RedlineType::Insert == maRedlineTable[ nM - 1 ]->GetType() ) )
                    {
                        --nM;
                        pRedl = maRedlineTable[ nM ];
                    }
                    else if( ( nM + 1 ) <= nO && rPos >= *maRedlineTable[ nM + 1 ]->Start() &&
                        rPos <= *maRedlineTable[ nM + 1 ]->End() &&
                        ( RedlineType::Insert == maRedlineTable[ nM + 1 ]->GetType() ) )
                    {
                        ++nM;
                        pRedl = maRedlineTable[ nM ];
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

bool DocumentRedlineManager::AcceptRedlineRange(SwRedlineTable::size_type nPosOrigin,
                                                SwRedlineTable::size_type& nPosStart,
                                                SwRedlineTable::size_type& nPosEnd,
                                                bool bCallDelete)
{
    bool bRet = false;

    SwRangeRedline* pTmp = maRedlineTable[nPosOrigin];
    SwRedlineTable::size_type nRdlIdx = nPosEnd + 1;
    SwRedlineData aOrigData = pTmp->GetRedlineData(0);

    SwNodeOffset nPamStartNI = maRedlineTable[nPosStart]->Start()->GetNodeIndex();
    sal_Int32 nPamStartCI = maRedlineTable[nPosStart]->Start()->GetContentIndex();
    SwNodeOffset nPamEndtNI = maRedlineTable[nPosEnd]->End()->GetNodeIndex();
    sal_Int32 nPamEndCI = maRedlineTable[nPosEnd]->End()->GetContentIndex();
    do
    {
        nRdlIdx--;
        pTmp = maRedlineTable[nRdlIdx];
        if (pTmp->Start()->GetNodeIndex() < nPamStartNI
            || (pTmp->Start()->GetNodeIndex() == nPamStartNI
                && pTmp->Start()->GetContentIndex() < nPamStartCI))
            break;

        if (pTmp->End()->GetNodeIndex() > nPamEndtNI
            || (pTmp->End()->GetNodeIndex() == nPamEndtNI
                && pTmp->End()->GetContentIndex() > nPamEndCI))
        {
        }
        else if (pTmp->GetRedlineData(0).CanCombineForAcceptReject(aOrigData))
        {
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoAcceptRedline>(*pTmp));
            }
            nPamEndtNI = pTmp->Start()->GetNodeIndex();
            nPamEndCI = pTmp->Start()->GetContentIndex();
            bRet |= lcl_AcceptRedline(maRedlineTable, nRdlIdx, bCallDelete);
            nRdlIdx++; //we will decrease it in the loop anyway.
        }
        else if (aOrigData.GetType() == RedlineType::Insert
                 && pTmp->GetType() == RedlineType::Delete && pTmp->GetStackCount() > 1
                 && pTmp->GetType(1) == RedlineType::Insert
                 && pTmp->GetRedlineData(1).CanCombineForAcceptReject(aOrigData))
        {
            // The Insert redline we want to accept has a deletion redline too
            // we should leave the deletion redline, and only accept the inner insert.
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoAcceptRedline>(*pTmp, 1));
            }
            nPamEndtNI = pTmp->Start()->GetNodeIndex();
            nPamEndCI = pTmp->Start()->GetContentIndex();
            bRet |= lcl_AcceptInnerInsertRedline(maRedlineTable, nRdlIdx, 1);
            nRdlIdx++; //we will decrease it in the loop anyway.
        }
    } while (nRdlIdx > 0);
    return bRet;
}

bool DocumentRedlineManager::AcceptMovedRedlines(sal_uInt32 nMovedID, bool bCallDelete)
{
    assert(nMovedID > 1);   // 0, and 1 is reserved
    bool bRet = false;
    SwRedlineTable::size_type nRdlIdx = maRedlineTable.size();

    while (nRdlIdx > 0)
    {
        nRdlIdx--;
        SwRangeRedline* pTmp = maRedlineTable[nRdlIdx];
        if (pTmp->GetMoved(0) == nMovedID
            || (pTmp->GetStackCount() > 1 && pTmp->GetMoved(1) == nMovedID))
        {
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoAcceptRedline>(*pTmp));
            }

            if (pTmp->GetMoved(0) == nMovedID)
                bRet |= lcl_AcceptRedline(maRedlineTable, nRdlIdx, bCallDelete);
            else
                bRet |= lcl_AcceptInnerInsertRedline(maRedlineTable, nRdlIdx, 1);

            nRdlIdx++; //we will decrease it in the loop anyway.
        }
    }
    return bRet;
}

bool DocumentRedlineManager::AcceptRedline(SwRedlineTable::size_type nPos, bool bCallDelete,
                                           bool bRange)
{
    bool bRet = false;

    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    SwRangeRedline* pTmp = maRedlineTable[ nPos ];
    bool bAnonym = pTmp->GetRedlineData(0).IsAnonymized();

    pTmp->Show(0, maRedlineTable.GetPos(pTmp), /*bForced=*/true);
    pTmp->Show(1, maRedlineTable.GetPos(pTmp), /*bForced=*/true);
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

        if (bRange && !nSeqNo && !bAnonym
            && !pTmp->Start()->GetNode().StartOfSectionNode()->IsTableNode())
        {
            sal_uInt32 nMovedID = pTmp->GetMoved(0);
            if (nMovedID > 1)
            {
                // Accept all redlineData with this unique move id
                bRet |= AcceptMovedRedlines(nMovedID, bCallDelete);
            }
            else
            {
                SwRedlineTable::size_type nPosStart = nPos;
                SwRedlineTable::size_type nPosEnd = nPos;

                maRedlineTable.getConnectedArea(nPos, nPosStart, nPosEnd, true);

                // Accept redlines between pPamStart-pPamEnd.
                // but only those that can be combined with the selected.
                bRet |= AcceptRedlineRange(nPos, nPosStart, nPosEnd, bCallDelete);
            }
        }
        else do {

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoAcceptRedline>(*pTmp) );
            }

            bRet |= lcl_AcceptRedline( maRedlineTable, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( SwRedlineTable::npos == nPos )
                    nPos = 0;
                SwRedlineTable::size_type nFndPos = 2 == nLoopCnt
                                    ? maRedlineTable.FindNextSeqNo( nSeqNo, nPos )
                                    : maRedlineTable.FindPrevSeqNo( nSeqNo, nPos );
                if( SwRedlineTable::npos != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    SwRedlineTable::npos != ( nFndPos =
                        maRedlineTable.FindPrevSeqNo( nSeqNo, nPos ))) )
                {
                    nPos = nFndPos;
                    pTmp = maRedlineTable[ nPos ];
                }
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while (nLoopCnt);

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

bool DocumentRedlineManager::AcceptRedline( const SwPaM& rPam, bool bCallDelete, sal_Int8 nDepth )
{
    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    // The Selection is only in the ContentSection. If there are Redlines
    // to Non-ContentNodes before or after that, then the Selections
    // expand to them.
    std::shared_ptr<SwUnoCursor> const pPam(m_rDoc.CreateUnoCursor(*rPam.GetPoint(), false));
    if (rPam.HasMark())
    {
        pPam->SetMark();
        *pPam->GetMark() = *rPam.GetMark();
    }
    lcl_AdjustRedlineRange(*pPam);

    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        m_rDoc.GetIDocumentUndoRedo().StartUndo( SwUndoId::ACCEPT_REDLINE, nullptr );
        m_rDoc.GetIDocumentUndoRedo().AppendUndo(
            std::make_unique<SwUndoAcceptRedline>(*pPam, nDepth));
    }

    int nRet = 0;
    if (nDepth == 0)
    {
        nRet = lcl_AcceptRejectRedl(lcl_AcceptRedline, maRedlineTable, bCallDelete, *pPam);
    }
    else
    {
        // For now it is called only if it is an Insert redline in a delete redline.
        SwRedlineTable::size_type nRdlIdx = 0;
        maRedlineTable.FindAtPosition(*rPam.Start(), nRdlIdx);
        if (lcl_AcceptInnerInsertRedline(maRedlineTable, nRdlIdx, 1))
            nRet = 1;
    }
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
    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*

    const SwNodeOffset nSttIdx = pStt->GetNodeIndex();
    const SwNodeOffset nEndIdx = pEnd->GetNodeIndex();

    for( SwRedlineTable::size_type n = 0; n < maRedlineTable.size() ; ++n )
    {
        const SwRangeRedline* pTmp = maRedlineTable[ n ];
        SwNodeOffset nPt = pTmp->GetPoint()->GetNodeIndex(),
              nMk = pTmp->GetMark()->GetNodeIndex();
        if( nPt < nMk )
            std::swap( nMk, nPt );

        if( RedlineType::ParagraphFormat == pTmp->GetType() &&
            ( (nSttIdx <= nMk && nMk <= nEndIdx) || (nSttIdx <= nPt && nPt <= nEndIdx) ) )
                AcceptRedline( n, false );

        if( nMk > nEndIdx )
            break;
    }
}

bool DocumentRedlineManager::RejectRedlineRange(SwRedlineTable::size_type nPosOrigin,
                                                SwRedlineTable::size_type& nPosStart,
                                                SwRedlineTable::size_type& nPosEnd,
                                                bool bCallDelete)
{
    bool bRet = false;

    SwRangeRedline* pTmp = maRedlineTable[nPosOrigin];
    SwRedlineTable::size_type nRdlIdx = nPosEnd + 1;
    SwRedlineData aOrigData = pTmp->GetRedlineData(0);

    SwNodeOffset nPamStartNI = maRedlineTable[nPosStart]->Start()->GetNodeIndex();
    sal_Int32 nPamStartCI = maRedlineTable[nPosStart]->Start()->GetContentIndex();
    SwNodeOffset nPamEndtNI = maRedlineTable[nPosEnd]->End()->GetNodeIndex();
    sal_Int32 nPamEndCI = maRedlineTable[nPosEnd]->End()->GetContentIndex();
    do
    {
        nRdlIdx--;
        pTmp = maRedlineTable[nRdlIdx];
        if (pTmp->Start()->GetNodeIndex() < nPamStartNI
            || (pTmp->Start()->GetNodeIndex() == nPamStartNI
                && pTmp->Start()->GetContentIndex() < nPamStartCI))
            break;

        if (pTmp->End()->GetNodeIndex() > nPamEndtNI
            || (pTmp->End()->GetNodeIndex() == nPamEndtNI
                && pTmp->End()->GetContentIndex() > nPamEndCI))
        {
        }
        else if (pTmp->GetRedlineData(0).CanCombineForAcceptReject(aOrigData))
        {
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                std::unique_ptr<SwUndoRejectRedline> pUndoRdl
                    = std::make_unique<SwUndoRejectRedline>(*pTmp);
#if OSL_DEBUG_LEVEL > 0
                pUndoRdl->SetRedlineCountDontCheck(true);
#endif
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndoRdl));
            }
            nPamEndtNI = pTmp->Start()->GetNodeIndex();
            nPamEndCI = pTmp->Start()->GetContentIndex();
            bRet |= lcl_RejectRedline(maRedlineTable, nRdlIdx, bCallDelete);
            nRdlIdx++; //we will decrease it in the loop anyway.
        }
        else if (aOrigData.GetType() == RedlineType::Insert
                 && pTmp->GetType() == RedlineType::Delete && pTmp->GetStackCount() > 1
                 && pTmp->GetType(1) == RedlineType::Insert
                 && pTmp->GetRedlineData(1).CanCombineForAcceptReject(aOrigData))
        {
            // The Insert redline we want to reject has a deletion redline too
            // without the insert, the delete is meaningless
            // so we rather just accept the deletion redline
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                std::unique_ptr<SwUndoRejectRedline> pUndoRdl
                    = std::make_unique<SwUndoRejectRedline>(*pTmp, 1);
#if OSL_DEBUG_LEVEL > 0
                pUndoRdl->SetRedlineCountDontCheck(true);
#endif
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndoRdl));
            }
            nPamEndtNI = pTmp->Start()->GetNodeIndex();
            nPamEndCI = pTmp->Start()->GetContentIndex();
            bRet |= lcl_AcceptRedline(maRedlineTable, nRdlIdx, bCallDelete);
            nRdlIdx++; //we will decrease it in the loop anyway.
        }

    } while (nRdlIdx > 0);
    return bRet;
}

bool DocumentRedlineManager::RejectMovedRedlines(sal_uInt32 nMovedID, bool bCallDelete)
{
    assert(nMovedID > 1); // 0, and 1 is reserved
    bool bRet = false;
    SwRedlineTable::size_type nRdlIdx = maRedlineTable.size();

    while (nRdlIdx > 0)
    {
        nRdlIdx--;
        SwRangeRedline* pTmp = maRedlineTable[nRdlIdx];
        if (pTmp->GetMoved(0) == nMovedID
            || (pTmp->GetStackCount() > 1 && pTmp->GetMoved(1) == nMovedID))
        {
            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                std::unique_ptr<SwUndoRejectRedline> pUndoRdl
                    = std::make_unique<SwUndoRejectRedline>(*pTmp);
#if OSL_DEBUG_LEVEL > 0
                pUndoRdl->SetRedlineCountDontCheck(true);
#endif
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(std::move(pUndoRdl));
            }

            if (pTmp->GetMoved(0) == nMovedID)
                bRet |= lcl_RejectRedline(maRedlineTable, nRdlIdx, bCallDelete);
            else
                bRet |= lcl_AcceptRedline(maRedlineTable, nRdlIdx, bCallDelete);

            nRdlIdx++; //we will decrease it in the loop anyway.
        }
    }
    return bRet;
}

bool DocumentRedlineManager::RejectRedline(SwRedlineTable::size_type nPos,
                                           bool bCallDelete, bool bRange)
{
    bool bRet = false;

    // Switch to visible in any case
    if( (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) !=
        (RedlineFlags::ShowMask & meRedlineFlags) )
      SetRedlineFlags( RedlineFlags::ShowInsert | RedlineFlags::ShowDelete | meRedlineFlags );

    SwRangeRedline* pTmp = maRedlineTable[ nPos ];
    bool bAnonym = pTmp->GetRedlineData(0).IsAnonymized();

    pTmp->Show(0, maRedlineTable.GetPos(pTmp), /*bForced=*/true);
    pTmp->Show(1, maRedlineTable.GetPos(pTmp), /*bForced=*/true);
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

        if (bRange && !nSeqNo && !bAnonym
            && !pTmp->Start()->GetNode().StartOfSectionNode()->IsTableNode())
        {
            sal_uInt32 nMovedID = pTmp->GetMoved(0);
            if (nMovedID > 1)
            {
                // Reject all redlineData with this unique move id
                bRet |= RejectMovedRedlines(nMovedID, bCallDelete);
            }
            else
            {
                SwRedlineTable::size_type nPosStart = nPos;
                SwRedlineTable::size_type nPosEnd = nPos;
                maRedlineTable.getConnectedArea(nPos, nPosStart, nPosEnd, true);

                // Reject items between pPamStart-pPamEnd
                // but only those that can be combined with the selected.

                bRet |= RejectRedlineRange(nPos, nPosStart, nPosEnd, bCallDelete);
            }
        }
        else do {

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    std::make_unique<SwUndoRejectRedline>( *pTmp ) );
            }

            bRet |= lcl_RejectRedline( maRedlineTable, nPos, bCallDelete );

            if( nSeqNo )
            {
                if( SwRedlineTable::npos == nPos )
                    nPos = 0;
                SwRedlineTable::size_type nFndPos = 2 == nLoopCnt
                                    ? maRedlineTable.FindNextSeqNo( nSeqNo, nPos )
                                    : maRedlineTable.FindPrevSeqNo( nSeqNo, nPos );
                if( SwRedlineTable::npos != nFndPos || ( 0 != ( --nLoopCnt ) &&
                    SwRedlineTable::npos != ( nFndPos =
                            maRedlineTable.FindPrevSeqNo( nSeqNo, nPos ))) )
                {
                    nPos = nFndPos;
                    pTmp = maRedlineTable[ nPos ];
                }
                else
                    nLoopCnt = 0;
            }
            else
                nLoopCnt = 0;

        } while (nLoopCnt);

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

bool DocumentRedlineManager::RejectRedline( const SwPaM& rPam, bool bCallDelete, sal_Int8 nDepth )
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
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwUndoRejectRedline>(aPam, nDepth) );
    }

    int nRet = 0;
    if (nDepth == 0)
    {
        nRet = lcl_AcceptRejectRedl(lcl_RejectRedline, maRedlineTable, bCallDelete, aPam);
    }
    else
    {
        // For now it is called only if it is an Insert redline in a delete redline.
        SwRedlineTable::size_type nRdlIdx = 0;
        maRedlineTable.FindAtPosition(*rPam.Start(), nRdlIdx);
        if (lcl_AcceptRedline(maRedlineTable, nRdlIdx, bCallDelete))
            nRet = 1;
    }

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

    if (maRedlineTable.size() > 1)
    {
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(maRedlineTable.size()));
            sUndoStr = aRewriter.Apply(SwResId(STR_N_REDLINES));
        }

        SwRewriter aRewriter;
        aRewriter.AddRule(UndoArg1, sUndoStr);
        rUndoMgr.StartUndo(bAccept ? SwUndoId::ACCEPT_REDLINE : SwUndoId::REJECT_REDLINE, &aRewriter);
    }

    while (!maRedlineTable.empty() && bSuccess)
    {
        if (bAccept)
            bSuccess = AcceptRedline(maRedlineTable.size() - 1, true);
        else
            bSuccess = RejectRedline(maRedlineTable.size() - 1, true);
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
        if( !pEnd->GetNode().IsContentNode() )
        {
            SwNodeIndex aTmp( pEnd->GetNode() );
            SwContentNode* pCNd = SwNodes::GoPrevSection( &aTmp );
            if( !pCNd || ( aTmp == rSttPos.GetNode() &&
                pCNd->Len() == rSttPos.GetContentIndex() ))
                pFnd = nullptr;
        }
        if( pFnd )
            rSttPos = *pFnd->End();
    }

    do {
        bRestart = false;

        for( ; !pFnd && n < maRedlineTable.size(); ++n )
        {
            pFnd = maRedlineTable[ n ];
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
            while( ++n < maRedlineTable.size() )
            {
                const SwRangeRedline* pTmp = maRedlineTable[ n ];
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
            SwPosition* pPos = rPam.GetMark();
            if( !pPos->GetNode().IsContentNode() )
            {
                pCNd = SwNodes::GoNextSection(pPos);
                if( pCNd )
                {
                    if( pPos->GetNode() <= rPam.GetPoint()->GetNode() )
                        pPos->Assign( *pCNd, 0 );
                    else
                        pFnd = nullptr;
                }
            }

            if( pFnd )
            {
                pPos = rPam.GetPoint();
                if( !pPos->GetNode().IsContentNode() )
                {
                    pCNd = SwNodes::GoPrevSection( pPos );
                    if( pCNd )
                    {
                        if( pPos->GetNode() >= rPam.GetMark()->GetNode() )
                            pPos->Assign( *pCNd, pCNd->Len() );
                        else
                            pFnd = nullptr;
                    }
                }
            }

            if( !pFnd || *rPam.GetMark() == *rPam.GetPoint() )
            {
                if( n < maRedlineTable.size() )
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
        if( !pStt->GetNode().IsContentNode() )
        {
            SwNodeIndex aTmp( pStt->GetNode() );
            SwContentNode* pCNd = SwNodes::GoNextSection(&aTmp);
            if( !pCNd || ( aTmp == rSttPos.GetNode() &&
                !rSttPos.GetContentIndex() ))
                pFnd = nullptr;
        }
        if( pFnd )
            rSttPos = *pFnd->Start();
    }

    do {
        bRestart = false;

        while( !pFnd && 0 < n )
        {
            pFnd = maRedlineTable[ --n ];
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
                const SwRangeRedline* pTmp = maRedlineTable[ --n ];
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
            SwPosition* pPos = rPam.GetMark();
            if( !pPos->GetNode().IsContentNode() )
            {
                pCNd = SwNodes::GoPrevSection( pPos );
                if( pCNd )
                {
                    if( pPos->GetNode() >= rPam.GetPoint()->GetNode() )
                        pPos->Assign( *pCNd, pCNd->Len() );
                    else
                        pFnd = nullptr;
                }
            }

            if( pFnd )
            {
                pPos = rPam.GetPoint();
                if( !pPos->GetNode().IsContentNode() )
                {
                    pCNd = SwNodes::GoNextSection(pPos);
                    if( pCNd )
                    {
                        if( pPos->GetNode() <= rPam.GetMark()->GetNode() )
                            pPos->Assign( *pCNd, 0 );
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
    auto [pStt, pEnd] = rPaM.StartEnd(); // SwPosition*
    SwRedlineTable::size_type n = 0;
    if( GetRedlineTable().FindAtPosition( *pStt, n ) )
    {
        for( ; n < maRedlineTable.size(); ++n )
        {
            bRet = true;
            SwRangeRedline* pTmp = maRedlineTable[ n ];
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
        moAutoFormatRedlnComment = *pText;
    }
    else
    {
        moAutoFormatRedlnComment.reset();
    }

    mnAutoFormatRedlnCommentNo = nSeqNo;
}

void DocumentRedlineManager::HideAll( bool bDeletion )
{
    const SwRedlineTable& rTable = GetRedlineTable();
    for (SwRedlineTable::size_type i = rTable.size(); i > 0; --i)
    {
        SwRangeRedline* pRedline = rTable[i-1];
        if ( pRedline->GetType() == RedlineType::Delete )
        {
            if ( bDeletion && pRedline->IsVisible() )
            {
                pRedline->Hide(0, rTable.GetPos(pRedline), false);
                pRedline->Hide(1, rTable.GetPos(pRedline), false);
            }
            else if ( !bDeletion && !pRedline->IsVisible() )
            {
                pRedline->Show(0, rTable.GetPos(pRedline), true);
                pRedline->Show(1, rTable.GetPos(pRedline), true);
            }
        }
        else if ( pRedline->GetType() == RedlineType::Insert )
        {
            if ( !bDeletion && pRedline->IsVisible() )
            {
                pRedline->ShowOriginal(0, rTable.GetPos(pRedline), false);
                pRedline->ShowOriginal(1, rTable.GetPos(pRedline), false);
            }
            else if ( bDeletion && !pRedline->IsVisible() )
            {
                pRedline->Show(0, rTable.GetPos(pRedline), true);
                pRedline->Show(1, rTable.GetPos(pRedline), true);
            }
        }
    }
}

void DocumentRedlineManager::ShowAll()
{
    const SwRedlineTable& rTable = GetRedlineTable();
    for (SwRedlineTable::size_type i = rTable.size(); i > 0; --i)
    {
        SwRangeRedline* pRedline = rTable[i-1];
        if ( !pRedline->IsVisible() )
        {
            pRedline->Show(0, rTable.GetPos(pRedline), true);
            pRedline->Show(1, rTable.GetPos(pRedline), true);
        }
    }
}

DocumentRedlineManager::~DocumentRedlineManager()
{
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
