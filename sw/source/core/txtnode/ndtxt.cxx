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

#include <hintids.hxx>
#include <hints.hxx>

#include <comphelper/configuration.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/rsiditem.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <anchoredobject.hxx>
#include <txtfld.hxx>
#include <txtinet.hxx>
#include <fmtanchr.hxx>
#include <fmtinfmt.hxx>
#include <fmtrfmrk.hxx>
#include <txttxmrk.hxx>
#include <fchrfmt.hxx>
#include <txtftn.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <frmatr.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <fmtftn.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentListsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <docufld.hxx>
#include <pam.hxx>
#include <fldbas.hxx>
#include <paratr.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <expfld.hxx>
#include <section.hxx>
#include <mvsave.hxx>
#include <SwGrammarMarkUp.hxx>
#include <redline.hxx>
#include <IMark.hxx>
#include <scriptinfo.hxx>
#include <istyleaccess.hxx>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>
#include <docsh.hxx>
#include <SwNodeNum.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/intitem.hxx>
#include <sortedobjs.hxx>
#include <calbck.hxx>
#include <attrhint.hxx>
#include <memory>
#include <unoparagraph.hxx>
#include <unotext.hxx>
#include <wrtsh.hxx>
#include <fmtpdsc.hxx>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <svl/itemiter.hxx>
#include <undobj.hxx>
#include <formatflysplit.hxx>
#include <fmtcntnt.hxx>
#include <poolfmt.hxx>
#include <names.hxx>

using namespace ::com::sun::star;

typedef std::vector<SwTextAttr*> SwpHts;

namespace sw {
    class TextNodeNotificationSuppressor {
        SwTextNode& m_rNode;
        bool m_bWasNotifiable;
        public:
            TextNodeNotificationSuppressor(SwTextNode& rNode)
                : m_rNode(rNode)
                , m_bWasNotifiable(rNode.m_bNotifiable)
            {
                m_rNode.m_bNotifiable = false;
            }
            ~TextNodeNotificationSuppressor()
            {
                m_rNode.m_bNotifiable = m_bWasNotifiable;
            }
    };
}

// unfortunately everyone can change Hints without ensuring order or the linking between them
#ifdef DBG_UTIL
#define CHECK_SWPHINTS(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc().IsInReading() ) \
                                  pNd->GetpSwpHints()->Check(true); }
#define CHECK_SWPHINTS_IF_FRM(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc().IsInReading() ) \
    pNd->GetpSwpHints()->Check(getLayoutFrame(nullptr, nullptr, nullptr) != nullptr); }
#else
#define CHECK_SWPHINTS(pNd)
#define CHECK_SWPHINTS_IF_FRM(pNd)
#endif

SwTextNode *SwNodes::MakeTextNode( const SwNode& rWhere,
                                 SwTextFormatColl *pColl, bool const bNewFrames)
{
    OSL_ENSURE( pColl, "Collection pointer is 0." );

    SwTextNode *pNode = new SwTextNode( rWhere, pColl, nullptr );

    SwNodeIndex aIdx( *pNode );

    // if there is no layout or it is in a hidden section, MakeFrames is not needed
    const SwSectionNode* pSectNd;
    if (!bNewFrames ||
        !GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell() ||
        ( nullptr != (pSectNd = pNode->FindSectionNode()) &&
            pSectNd->GetSection().IsHiddenFlag() ))
        return pNode;

    SwNodeIndex aTmp( rWhere );
    do {
        // max. 2 loops:
        // 1. take the successor
        // 2. take the predecessor

        SwNode * pNd = & aTmp.GetNode();
        switch (pNd->GetNodeType())
        {
        case SwNodeType::Table:
            static_cast<SwTableNode*>(pNd)->MakeFramesForAdjacentContentNode(aIdx);
            return pNode;

        case SwNodeType::Section:
            if( static_cast<SwSectionNode*>(pNd)->GetSection().IsHidden() ||
                static_cast<SwSectionNode*>(pNd)->IsContentHidden() )
            {
                pNd = FindPrvNxtFrameNode( *pNode, pNode );
                if( !pNd )
                    return pNode;
                aTmp = *pNd;
                break;
            }
            static_cast<SwSectionNode*>(pNd)->MakeFramesForAdjacentContentNode(aIdx);
            return pNode;

        case SwNodeType::Text:
        case SwNodeType::Grf:
        case SwNodeType::Ole:
            static_cast<SwContentNode*>(pNd)->MakeFramesForAdjacentContentNode(*pNode);
            return pNode;

        case SwNodeType::End:
            if( pNd->StartOfSectionNode()->IsSectionNode() &&
                aTmp.GetIndex() < rWhere.GetIndex() )
            {
                if( pNd->StartOfSectionNode()->GetSectionNode()->GetSection().IsHiddenFlag())
                {
                    if( !GoPrevSection( &aTmp, true, false ) ||
                        aTmp.GetNode().FindTableNode() !=
                            pNode->FindTableNode() )
                        return pNode;
                }
                else
                    aTmp = *pNd->StartOfSectionNode();
                break;
            }
            else if( pNd->StartOfSectionNode()->IsTableNode() &&
                    aTmp.GetIndex() < rWhere.GetIndex() )
            {
                // after a table node
                aTmp = *pNd->StartOfSectionNode();
                break;
            }
            [[fallthrough]];
        default:
            if( &rWhere == &aTmp.GetNode() )
                aTmp -= SwNodeOffset(2);
            else
                return pNode;
            break;
        }
    } while( true );
}

SwTextNode::SwTextNode( const SwNode& rWhere, SwTextFormatColl *pTextColl, const SfxItemSet* pAutoAttr )
:   SwContentNode( rWhere, SwNodeType::Text, pTextColl ),
    m_bContainsHiddenChars(false),
    m_bHiddenCharsHidePara(false),
    m_bRecalcHiddenCharFlags(false),
    m_bLastOutlineState( false ),
    m_bNotifiable( true ),
    mbEmptyListStyleSetDueToSetOutlineLevelAttr( false ),
    mbInSetOrResetAttr( false ),
    m_bInUndo(false)
{
    {
        sw::TextNodeNotificationSuppressor(*this);

        if( pAutoAttr )
            SetAttr( *pAutoAttr );

        if (!IsInList() && GetNumRule() && !GetListId().isEmpty())
        {
            // #i101516#
            // apply paragraph style's assigned outline style list level as
            // list level of the paragraph, if it has none set already.
            if ( !HasAttrListLevel() &&
                 pTextColl && pTextColl->IsAssignedToListLevelOfOutlineStyle() )
            {
                SetAttrListLevel( pTextColl->GetAssignedOutlineStyleLevel() );
            }
            AddToList();
        }

        // call method <UpdateOutlineNode(..)> only for the document nodes array
        if (GetNodes().IsDocNodes())
            GetNodes().UpdateOutlineNode(*this);
    }

    m_bContainsHiddenChars = m_bHiddenCharsHidePara = false;
    m_bRecalcHiddenCharFlags = true;
}

SwTextNode::~SwTextNode()
{
    // delete only removes the pointer not the array elements!
    if ( m_pSwpHints )
    {
        // do not delete attributes twice when those delete their content
        std::unique_ptr<SwpHints> pTmpHints(std::move(m_pSwpHints));

        for( size_t j = pTmpHints->Count(); j; )
        {
            // first remove the attribute from the array otherwise
            // if would delete itself
            DestroyAttr( pTmpHints->Get( --j ) );
        }
    }

    // must be removed from outline nodes by now
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    SwOutlineNodes::size_type foo;
    assert(!GetNodes().GetOutLineNds().Seek_Entry(this, &foo));
#endif

    RemoveFromList();

    DelFrames(nullptr); // must be called here while it's still a SwTextNode
    DelFrames_TextNodePart();

    // If this Node should have Outline Numbering but that state hasn't been
    // crystallized by SwNodes::UpdateOutlineNode yet, and so it currently isn't
    // added to SwNodes::m_aOutlineNodes, then set LastOutlineState so it won't
    // be added if ResetAttr() triggers UpdateOutlineNode() during destruction,
    // and avoid leaving a dangling pointer in m_aOutlineNodes.
    if (IsOutline() && !m_bLastOutlineState)
        m_bLastOutlineState = true;

#if defined(FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION)
    if (!GetDoc().IsInDtor())
#endif
    {
        ResetAttr(RES_PAGEDESC);
    }
    InvalidateInSwCache();
}

void SwTextNode::FileLoadedInitHints()
{
    if (m_pSwpHints)
    {
        m_pSwpHints->MergePortions(*this);
    }
}

SwContentFrame *SwTextNode::MakeFrame( SwFrame* pSib )
{
    SwContentFrame *pFrame = sw::MakeTextFrame(*this, pSib, sw::FrameMode::New);
    return pFrame;
}

sal_Int32 SwTextNode::Len() const
{
    return m_Text.getLength();
}

// After a split node, it's necessary to actualize the ref-pointer of the ftnfrms.
static void lcl_ChangeFootnoteRef( SwTextNode &rNode )
{
    SwpHints *pSwpHints = rNode.GetpSwpHints();
    if( !(pSwpHints && rNode.GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell()) )
        return;

    SwContentFrame* pFrame = nullptr;
    // OD 07.11.2002 #104840# - local variable to remember first footnote
    // of node <rNode> in order to invalidate position of its first content.
    // Thus, in its <MakeAll()> it will checked its position relative to its reference.
    SwFootnoteFrame* pFirstFootnoteOfNode = nullptr;
    for( size_t j = pSwpHints->Count(); j; )
    {
        SwTextAttr* pHt = pSwpHints->Get(--j);
        if (RES_TXTATR_FTN == pHt->Which())
        {
            if( !pFrame )
            {
                pFrame = SwIterator<SwContentFrame, SwTextNode, sw::IteratorMode::UnwrapMulti>(rNode).First();
                if (!pFrame)
                    return;
            }
            SwTextFootnote *pAttr = static_cast<SwTextFootnote*>(pHt);
            OSL_ENSURE( pAttr->GetStartNode(), "FootnoteAtr without StartNode." );
            SwNodeIndex aIdx( *pAttr->GetStartNode(), 1 );
            SwContentNode *pNd = aIdx.GetNode().GetContentNode();
            if ( !pNd )
                pNd = SwNodes::GoNextSection(&aIdx, true, false);
            if ( !pNd )
                continue;

            SwIterator<SwContentFrame, SwContentNode, sw::IteratorMode::UnwrapMulti> aIter(*pNd);
            SwContentFrame* pContent = aIter.First();
            if( pContent )
            {
                OSL_ENSURE( pContent->getRootFrame() == pFrame->getRootFrame(),
                        "lcl_ChangeFootnoteRef: Layout double?" );
                SwFootnoteFrame *pFootnote = pContent->FindFootnoteFrame();
                if( pFootnote && pFootnote->GetAttr() == pAttr )
                {
                    while( pFootnote->GetMaster() )
                        pFootnote = pFootnote->GetMaster();
                    // #104840# - remember footnote frame
                    pFirstFootnoteOfNode = pFootnote;
                    while ( pFootnote )
                    {
                        pFootnote->SetRef( pFrame );
                        pFootnote = pFootnote->GetFollow();
                        static_cast<SwTextFrame*>(pFrame)->SetFootnote( true );
                    }
                }
#if OSL_DEBUG_LEVEL > 0
                while( nullptr != (pContent = aIter.Next()) )
                {
                    SwFootnoteFrame *pDbgFootnote = pContent->FindFootnoteFrame();
                    OSL_ENSURE( !pDbgFootnote || pDbgFootnote->GetRef() == pFrame,
                            "lcl_ChangeFootnoteRef: Who's that guy?" );
                }
#endif
            }
        }
    } // end of for-loop on <SwpHints>
    // #104840# - invalidate
    if ( pFirstFootnoteOfNode )
    {
        SwContentFrame* pContent = pFirstFootnoteOfNode->ContainsContent();
        if ( pContent )
        {
            pContent->InvalidatePos_();
        }
    }
}

namespace sw {

// check if there are flys on the existing frames (now on "pNode")
// that need to be moved to the new frames of "this"
void MoveMergedFlysAndFootnotes(std::vector<SwTextFrame*> const& rFrames,
        SwTextNode const& rFirstNode, SwTextNode & rSecondNode,
        bool isSplitNode)
{
    if (!isSplitNode)
    {
        lcl_ChangeFootnoteRef(rSecondNode);
    }
    for (SwNodeOffset nIndex = rSecondNode.GetIndex() + 1; ; ++nIndex)
    {
        SwNode *const pTmp(rSecondNode.GetNodes()[nIndex]);
        if (pTmp->IsCreateFrameWhenHidingRedlines() || pTmp->IsEndNode())
        {
            break;
        }
        else if (pTmp->IsStartNode())
        {
            nIndex = pTmp->EndOfSectionIndex();
        }
        else if (pTmp->GetRedlineMergeFlag() == SwNode::Merge::NonFirst
              && pTmp->IsTextNode())
        {
            lcl_ChangeFootnoteRef(*pTmp->GetTextNode());
        }
    }
    for (SwTextFrame *const pFrame : rFrames)
    {
        if (SwSortedObjs *const pObjs = pFrame->GetDrawObjs())
        {
            std::vector<SwAnchoredObject*> objs;
            objs.reserve(pObjs->size());
            for (SwAnchoredObject *const pObj : *pObjs)
            {
                objs.push_back(pObj);
            }
            for (SwAnchoredObject *const pObj : objs)
            {
                SwFrameFormat* pFormat(pObj->GetFrameFormat());
                SwFormatAnchor const& rAnchor(pFormat->GetAnchor());
                if (rFirstNode.GetIndex() < rAnchor.GetAnchorNode()->GetIndex())
                {
                    // move it to the new frame of "this"
                    pFormat->CallSwClientNotify(sw::LegacyModifyHint(&rAnchor, &rAnchor));
                    // note pObjs will be deleted if it becomes empty
                    assert(!pFrame->GetDrawObjs() || !pObjs->Contains(*pObj));
                }
            }
        }
    }
}

} // namespace

SwTextNode *SwTextNode::SplitContentNode(const SwPosition & rPos,
        std::function<void (SwTextNode *, sw::mark::RestoreMode, bool AtStart)> const*const pContentIndexRestore)
{
    bool isHide(false);
    SwNode::Merge const eOldMergeFlag(GetRedlineMergeFlag());
    bool parentIsOutline = IsOutline();

    // create a node "in front" of me
    const sal_Int32 nSplitPos = rPos.GetContentIndex();
    const sal_Int32 nTextLen = m_Text.getLength();
    SwTextNode* const pNode =
        MakeNewTextNode( rPos.GetNode(), false, nSplitPos==nTextLen );

    // the first paragraph gets the XmlId,
    // _except_ if it is empty and the second is not empty
    if (nSplitPos != 0) {
        pNode->RegisterAsCopyOf(*this, true);
        if (nSplitPos == nTextLen)
        {
            RemoveMetadataReference();
            // NB: SwUndoSplitNode will call pNode->JoinNext,
            // which is sufficient even in this case!
        }
    }

    ResetAttr( RES_PARATR_LIST_ISRESTART );
    ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
    ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    if ( GetNumRule() == nullptr || (parentIsOutline && !IsOutline()) )
    {
        ResetAttr( RES_PARATR_LIST_ID );
        ResetAttr( RES_PARATR_LIST_LEVEL );
    }

    bool bSplitFly = false;
    std::optional<std::vector<SwFrameFormat*>> oFlys = sw::GetFlysAnchoredAt(GetDoc(), GetIndex(), false);
    if (oFlys.has_value())
    {
        // See if one of the flys is a split fly. If so, we need to keep
        // the potentially split text frames unchanged and create a new
        // text frame at the end.
        for (const auto& rFly : *oFlys)
        {
            if (rFly->GetFlySplit().GetValue())
            {
                bSplitFly = true;
                break;
            }
        }
    }

    if ( HasWriterListeners() && !m_Text.isEmpty() && ((nTextLen / 2) < nSplitPos || bSplitFly) )
    {
        // optimization for SplitNode: If a split is at the end of a node then
        // move the frames from the current to the new one and create new ones
        // for the current one.

        // If fly frames are moved, they don't need to destroy their layout
        // frames.  Set a flag that is checked in SwTextFlyCnt::SetAnchor.
        if ( HasHints() )
        {
            pNode->GetOrCreateSwpHints().SetInSplitNode(true);
        }

        // Move the first part of the content to the new node and delete
        // it in the old node.
        SwContentIndex aIdx( this );
        CutText( pNode, aIdx, nSplitPos );

        if( GetWrong() )
        {
            pNode->SetWrong( GetWrong()->SplitList( nSplitPos ) );
        }
        SetWrongDirty(sw::WrongState::TODO);

        if( GetGrammarCheck() )
        {
            pNode->SetGrammarCheck( GetGrammarCheck()->SplitGrammarList( nSplitPos ) );
        }
        SetGrammarCheckDirty( true );

        SetWordCountDirty( true );

        if( GetSmartTags() )
        {
            pNode->SetSmartTags( GetSmartTags()->SplitList( nSplitPos ) );
        }
        SetSmartTagDirty( true );

        resetAndQueueAccessibilityCheck();
        pNode->resetAndQueueAccessibilityCheck();

        if ( pNode->HasHints() )
        {
            if ( pNode->m_pSwpHints->CanBeDeleted() )
            {
                pNode->m_pSwpHints.reset();
            }
            else
            {
                pNode->m_pSwpHints->SetInSplitNode(false);
            }

            // All fly frames anchored as char that are moved to the new
            // node must have their layout frames deleted.
            // This comment is sort of silly because we actually delete the
            // layout frames of those which were not moved?
            // JP 01.10.96: delete all empty and not-to-be-expanded attributes
            if ( HasHints() )
            {
                for ( size_t j = m_pSwpHints->Count(); j; )
                {
                    SwTextAttr* const pHt = m_pSwpHints->Get( --j );
                    if ( RES_TXTATR_FLYCNT == pHt ->Which() )
                    {
                        pHt->GetFlyCnt().GetFrameFormat()->DelFrames();
                    }
                    else if ( pHt->DontExpand() )
                    {
                        const sal_Int32* const pEnd = pHt->GetEnd();
                        if (pEnd && pHt->GetStart() == *pEnd )
                        {
                            // delete it!
                            m_pSwpHints->DeleteAtPos( j );
                            DestroyAttr( pHt );
                        }
                    }
                }
            }

        }

        if (pContentIndexRestore)
        {   // call before making frames and before RegisterToNode
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::NonFlys, false);
        }
        if (eOldMergeFlag != SwNode::Merge::None)
        {   // clear before making frames and before RegisterToNode
            SetRedlineMergeFlag(SwNode::Merge::None);
        }   // now RegisterToNode will set merge flags in both nodes properly!

        std::vector<SwTextFrame*> frames;
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
        for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
        {
            if (pFrame->getRootFrame()->HasMergedParas())
            {
                isHide = true;
            }
            frames.push_back(pFrame);
        }
        for (SwTextFrame * pFrame : frames)
        {
            pFrame->RegisterToNode( *pNode );
            if (!pFrame->IsFollow() && pFrame->GetOffset())
            {
                pFrame->SetOffset( TextFrameIndex(0) );
            }
        }

        InvalidateInSwCache();

        if ( HasHints() )
        {
            MoveTextAttr_To_AttrSet();
            pNode->MoveTextAttr_To_AttrSet();
        }
        // in case there are frames, the RegisterToNode has set the merge flag
        pNode->MakeFramesForAdjacentContentNode(*this);
        lcl_ChangeFootnoteRef( *this );
        if (pContentIndexRestore)
        {   // call after making frames; listeners will take care of adding to the right frame
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::Flys, false);
        }
        if (eOldMergeFlag != SwNode::Merge::None)
        {
            MoveMergedFlysAndFootnotes(frames, *pNode, *this, true);
        }
    }
    else
    {
        std::unique_ptr<SwWrongList> pList = ReleaseWrong();
        SetWrongDirty(sw::WrongState::TODO);

        std::unique_ptr<SwGrammarMarkUp> pList3 = ReleaseGrammarCheck();
        SetGrammarCheckDirty( true );

        SetWordCountDirty( true );

        std::unique_ptr<SwWrongList> pList2 = ReleaseSmartTags();
        SetSmartTagDirty( true );

        SwContentIndex aIdx( this );
        CutText( pNode, aIdx, nSplitPos );

        // JP 01.10.96: delete all empty and not-to-be-expanded attributes
        if ( HasHints() )
        {
            for ( size_t j = m_pSwpHints->Count(); j; )
            {
                SwTextAttr* const pHt = m_pSwpHints->Get( --j );
                const sal_Int32* const pEnd = pHt->GetEnd();
                if ( pHt->DontExpand() && pEnd && (pHt->GetStart() == *pEnd) )
                {
                    // delete it!
                    m_pSwpHints->DeleteAtPos( j );
                    DestroyAttr( pHt );
                }
            }
            MoveTextAttr_To_AttrSet();
            pNode->MoveTextAttr_To_AttrSet();
        }

        if( pList )
        {
            pNode->SetWrong( pList->SplitList( nSplitPos ) );
            SetWrong( std::move(pList) );
        }

        if( pList3 )
        {
            pNode->SetGrammarCheck( pList3->SplitGrammarList( nSplitPos ) );
            SetGrammarCheck( std::move(pList3) );
        }

        if( pList2 )
        {
            pNode->SetSmartTags( pList2->SplitList( nSplitPos ) );
            SetSmartTags( std::move(pList2) );
        }

        resetAndQueueAccessibilityCheck();
        pNode->resetAndQueueAccessibilityCheck();

        if (pContentIndexRestore)
        {   // call before making frames and before RegisterToNode
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::NonFlys, false);
        }

        std::vector<SwTextFrame*> frames;
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
        for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
        {
            frames.push_back(pFrame);
            if (pFrame->getRootFrame()->HasMergedParas())
            {
                isHide = true;
            }
        }
        bool bNonMerged(false);
        bool bRecreateThis(false);
        for (SwTextFrame * pFrame : frames)
        {
            // sw_redlinehide: for this to work properly with hidden nodes,
            // the frame needs to listen on them too.
            // also: have to check the frame; this->GetRedlineMergeFlag()
            // is None in case there's a delete redline inside the paragraph,
            // but that could still result in a merged frame after split...
            if (pFrame->GetMergedPara())
            {
                // Can't special case this == First here - that could (if
                // both nodes are still merged by redline) lead to
                // duplicate frames on "this".
                // Update the extents with new node; also inits merge flag,
                // so the MakeFramesForAdjacentContentNode below respects it
                pFrame->RegisterToNode(*pNode);
                if (nSplitPos == 0)
                {
                    // in this case, it was not
                    // invalidated because Cut didn't sent it any hints,
                    // so we have to invalidate it here!
                    pFrame->Prepare(PrepareHint::Clear, nullptr, false);
                }
                if (!pFrame->GetMergedPara() ||
                    !pFrame->GetMergedPara()->listener.IsListeningTo(this))
                {
                    // it's no longer listening - need to recreate frame
                    // (note this is idempotent, can be done once per frame)
                    SetRedlineMergeFlag(SwNode::Merge::None);
                    bRecreateThis = true;
                }
            }
            else
            {
                bNonMerged = true;
            }
        }
        assert(!(bNonMerged && bRecreateThis)); // 2 layouts not handled yet - maybe best to simply use the other branch then?
        if (!frames.empty() && bNonMerged)
        {
            // the existing frame on "this" should have been updated by Cut
            MakeFramesForAdjacentContentNode(*pNode);
            lcl_ChangeFootnoteRef(*pNode);
        }
        else if (bRecreateThis)
        {
            assert(pNode->HasWriterListeners()); // was just moved there
            pNode->MakeFramesForAdjacentContentNode(*this);
            lcl_ChangeFootnoteRef(*this);
        }

        if (pContentIndexRestore)
        {   // call after making frames; listeners will take care of adding to the right frame
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::Flys, nSplitPos == 0);
        }

        if (bRecreateThis)
        {
            MoveMergedFlysAndFootnotes(frames, *pNode, *this, true);
        }
    }

    // pNode is the previous node, 'this' is the next node from the split.
    if (nSplitPos == nTextLen && m_pSwpHints)
    {
        // We just created an empty next node: avoid unwanted superscript in the new node if it's
        // there.
        ResetAttr(RES_CHRATR_ESCAPEMENT);
    }

#ifndef NDEBUG
    if (isHide) // otherwise flags won't be set anyway
    {
        // First
        // -> First,NonFirst
        // -> First,Hidden
        // -> None,First
        // Hidden
        // -> Hidden,Hidden (if still inside merge rl)
        // -> NonFirst,First (if redline was split)
        // NonFirst
        // -> NonFirst,First (if split after end of "incoming" redline &
        //                    before start of "outgoing" redline)
        // -> NonFirst,None (if split after end of "incoming" redline)
        // -> NonFirst,Hidden (if split after start of "outgoing" redline)
        // -> Hidden, NonFirst (if split before end of "incoming" redline)
        // None
        // -> None,None
        // -> First,NonFirst (if splitting inside a delete redline)
        SwNode::Merge const eFirst(pNode->GetRedlineMergeFlag());
        SwNode::Merge const eSecond(GetRedlineMergeFlag());
        switch (eOldMergeFlag)
        {
            case Merge::First:
                assert((eFirst == Merge::First && eSecond == Merge::NonFirst)
                    || (eFirst == Merge::First && eSecond == Merge::Hidden)
                    || (eFirst == Merge::None && eSecond == Merge::First));
            break;
            case Merge::Hidden:
                assert((eFirst == Merge::Hidden && eSecond == Merge::Hidden)
                    || (eFirst == Merge::NonFirst && eSecond == Merge::First)
                        // next ones can happen temp. in UndoDelete :(
                    || (eFirst == Merge::Hidden && eSecond == Merge::NonFirst)
                    || (eFirst == Merge::NonFirst && eSecond == Merge::None));
            break;
            case Merge::NonFirst:
                assert((eFirst == Merge::NonFirst && eSecond == Merge::First)
                    || (eFirst == Merge::NonFirst && eSecond == Merge::None)
                    || (eFirst == Merge::NonFirst && eSecond == Merge::Hidden)
                    || (eFirst == Merge::Hidden && eSecond == Merge::NonFirst));
            break;
            case Merge::None:
                assert((eFirst == Merge::None && eSecond == Merge::None)
                    || (eFirst == Merge::First && eSecond == Merge::NonFirst));
            break;
        }
    }
#else
    (void) isHide;
#endif

    {
        // Send Hint for PageDesc. This should be done in the Layout when
        // pasting the frames, but that causes other problems that look
        // expensive to solve.
        const SwFormatPageDesc *pItem;
        if(HasWriterListeners() && (pItem = pNode->GetSwAttrSet().GetItemIfSet(RES_PAGEDESC)))
            pNode->TriggerNodeUpdate(sw::LegacyModifyHint(pItem, pItem));
    }
    return pNode;
}

void SwTextNode::MoveTextAttr_To_AttrSet()
{
    OSL_ENSURE( m_pSwpHints, "MoveTextAttr_To_AttrSet without SwpHints?" );
    for ( size_t i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTextAttr *pHt = m_pSwpHints->Get(i);

        if( pHt->GetStart() )
            break;

        const sal_Int32* pHtEndIdx = pHt->GetEnd();

        if( !pHtEndIdx )
            continue;

        if (*pHtEndIdx < m_Text.getLength() || pHt->IsCharFormatAttr())
            break;

        if (!pHt->IsDontMoveAttr())
        {
            bool isInserted(false);
            if (pHt->Which() == RES_TXTATR_AUTOFMT)
            {
                isInserted = SetAttr(*pHt->GetAutoFormat().GetStyleHandle());
            }
            else
            {
                isInserted = SetAttr(pHt->GetAttr());
            }
            if (isInserted)
            {
                m_pSwpHints->DeleteAtPos(i);
                DestroyAttr( pHt );
                --i;
            }
        }
    }

}

namespace sw {

/// if first node is deleted & second survives, then the first node's frame
/// will be deleted too; prevent this by moving the frame to the second node
/// if necessary.
void MoveDeletedPrevFrames(const SwTextNode & rDeletedPrev, SwTextNode & rNode)
{
    std::vector<SwTextFrame*> frames;
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(rDeletedPrev);
    for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
    {
        if (pFrame->getRootFrame()->HasMergedParas())
        {
            frames.push_back(pFrame);
        }
    }
    {
        auto frames2(frames);
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIt(rNode);
        for (SwTextFrame* pFrame = aIt.First(); pFrame; pFrame = aIt.Next())
        {
            if (pFrame->getRootFrame()->HasMergedParas())
            {
                auto const it(std::find(frames2.begin(), frames2.end(), pFrame));
                assert(it != frames2.end());
                frames2.erase(it);
            }
        }
        assert(frames2.empty());
    }
    for (SwTextFrame *const pFrame : frames)
    {
        pFrame->RegisterToNode(rNode, true);
    }
}

// typical Join:
// None,Node->None
// None,First->First
// First,NonFirst->First
// NonFirst,First->NonFirst
// NonFirst,None->NonFirst

/// if first node is First, its frames may need to be moved, never deleted.
/// if first node is NonFirst, second node's own frames (First/None) must be deleted
void CheckResetRedlineMergeFlag(SwTextNode & rNode, Recreate const eRecreateMerged)
{
    if (eRecreateMerged != sw::Recreate::No)
    {
        SwTextNode * pMergeNode(&rNode);
        if (eRecreateMerged == sw::Recreate::Predecessor
            // tdf#135018 check that there is a predecessor node, i.e. rNode
            // isn't the first node after the body start node
            && rNode.GetNodes()[rNode.GetIndex() - 1]->StartOfSectionIndex() != SwNodeOffset(0))
        {
            for (SwNodeOffset i = rNode.GetIndex() - 1; ; --i)
            {
                SwNode *const pNode(rNode.GetNodes()[i]);
                assert(!pNode->IsStartNode());
                if (pNode->IsEndNode())
                {
                    i = pNode->StartOfSectionIndex();
                }
                else if (pNode->IsTextNode())
                {
                    pMergeNode = pNode->GetTextNode(); // use predecessor to merge
                    break;
                }
            }
        }
        std::vector<SwTextFrame*> frames;
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pMergeNode);
        for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
        {
            if (pFrame->getRootFrame()->HasMergedParas())
            {
                frames.push_back(pFrame);
            }
        }
        auto eMode(sw::FrameMode::Existing);
        for (SwTextFrame * pFrame : frames)
        {
            SwTextNode & rFirstNode(pFrame->GetMergedPara()
                ? *pFrame->GetMergedPara()->pFirstNode
                : *pMergeNode);
            assert(rFirstNode.GetIndex() <= rNode.GetIndex());
            pFrame->SetMergedPara(sw::CheckParaRedlineMerge(
                        *pFrame, rFirstNode, eMode));
            // there is no merged para in case the deleted node had one but
            // nothing was actually hidden
            if (pFrame->GetMergedPara())
            {
                assert(pFrame->GetMergedPara()->listener.IsListeningTo(&rNode));
                assert(rNode.GetIndex() <= pFrame->GetMergedPara()->pLastNode->GetIndex());
                // tdf#135978 Join: recreate fly frames anchored to subsequent nodes
                if (eRecreateMerged == sw::Recreate::ThisNode)
                {
                    AddRemoveFlysAnchoredToFrameStartingAtNode(*pFrame, rNode, nullptr);
                }
            }
            eMode = sw::FrameMode::New; // Existing is not idempotent!
        }
    }
    else if (rNode.GetRedlineMergeFlag() != SwNode::Merge::None)
    {
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(rNode);
        for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
        {
            if (auto const pMergedPara = pFrame->GetMergedPara())
            {
                if (pMergedPara->pFirstNode == pMergedPara->pLastNode)
                {
                    assert(pMergedPara->pFirstNode == &rNode);
                    rNode.SetRedlineMergeFlag(SwNode::Merge::None);
                }
                break; // checking once is enough
            }
            else if (pFrame->getRootFrame()->HasMergedParas())
            {
                rNode.SetRedlineMergeFlag(SwNode::Merge::None);
                break; // checking once is enough
            }
        }
    }
}

bool HasNumberingWhichNeedsLayoutUpdate(const SwTextNode& rTextNode)
{
    const SwNodeNum* pNodeNum = rTextNode.GetNum();
    if (!pNodeNum)
    {
        return false;
    }

    const SwNumRule* pNumRule = pNodeNum->GetNumRule();
    if (!pNumRule)
    {
        return false;
    }

    const SwNumFormat* pFormat
        = pNumRule->GetNumFormat(o3tl::narrowing<sal_uInt16>(rTextNode.GetAttrListLevel()));
    if (!pFormat)
    {
        return false;
    }

    switch (pFormat->GetNumberingType())
    {
        case SVX_NUM_NUMBER_NONE:
        case SVX_NUM_CHAR_SPECIAL:
        case SVX_NUM_BITMAP:
            return false;
        default:
            return true;
    }
}
} // namespace

SwContentNode *SwTextNode::JoinNext()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwContentNode::CanJoinNext( &aIdx ) )
    {
        SwDoc& rDoc = rNds.GetDoc();
        const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
        pContentStore->Save(rDoc, aIdx.GetIndex(), SAL_MAX_INT32);
        SwTextNode *pTextNode = aIdx.GetNode().GetTextNode();
        sal_Int32 nOldLen = m_Text.getLength();

        // METADATA: merge
        JoinMetadatable(*pTextNode, !Len(), !pTextNode->Len());

        std::unique_ptr<SwWrongList> pList = ReleaseWrong();
        if( pList )
        {
            pList->JoinList( pTextNode->GetWrong(), nOldLen );
            SetWrongDirty(sw::WrongState::TODO);
        }
        else
        {
            pList = pTextNode->ReleaseWrong();
            if( pList )
            {
                pList->Move( 0, nOldLen );
                SetWrongDirty(sw::WrongState::TODO);
            }
        }

        std::unique_ptr<SwGrammarMarkUp> pList3 = ReleaseGrammarCheck();
        if( pList3 )
        {
            pList3->JoinGrammarList( pTextNode->GetGrammarCheck(), nOldLen );
            SetGrammarCheckDirty( true );
        }
        else
        {
            pList3 = pTextNode->ReleaseGrammarCheck();
            if( pList3 )
            {
                pList3->MoveGrammar( 0, nOldLen );
                SetGrammarCheckDirty( true );
            }
        }

        std::unique_ptr<SwWrongList> pList2 = ReleaseSmartTags();
        if( pList2 )
        {
            pList2->JoinList( pTextNode->GetSmartTags(), nOldLen );
            SetSmartTagDirty( true );
        }
        else
        {
            pList2 = pTextNode->ReleaseSmartTags();
            if( pList2 )
            {
                pList2->Move( 0, nOldLen );
                SetSmartTagDirty( true );
            }
        }

        { // scope for SwContentIndex
            pTextNode->CutText( this, SwContentIndex(pTextNode), pTextNode->Len() );
        }
        // move all Bookmarks/TOXMarks
        if( !pContentStore->Empty())
            pContentStore->Restore( rDoc, GetIndex(), nOldLen );

        if( pTextNode->HasAnyIndex() )
        {
            // move all ShellCursor/StackCursor/UnoCursor out of delete range
            rDoc.CorrAbs( aIdx.GetNode(), SwPosition( *this ), nOldLen, true );
        }
        SwNode::Merge const eOldMergeFlag(pTextNode->GetRedlineMergeFlag());
        auto eRecreateMerged(eOldMergeFlag == SwNode::Merge::First
                    ? sw::Recreate::ThisNode
                    : sw::Recreate::No);
        if (eRecreateMerged == sw::Recreate::No)
        {
            // tdf#137318 if a delete is inside one node, flag is still None!
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pTextNode);
            for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
            {
                if (pFrame->GetMergedPara())
                {
                    eRecreateMerged = sw::Recreate::ThisNode;
                    break;
                }
            }
        }
        bool bOldHasNumberingWhichNeedsLayoutUpdate = HasNumberingWhichNeedsLayoutUpdate(*pTextNode);

        rNds.Delete(aIdx);
        SetWrong( std::move(pList) );
        SetGrammarCheck( std::move(pList3) );
        SetSmartTags( std::move(pList2) );

        resetAndQueueAccessibilityCheck();

        if (bOldHasNumberingWhichNeedsLayoutUpdate || HasNumberingWhichNeedsLayoutUpdate(*this))
        {
            // Repaint all text frames that belong to this numbering to avoid outdated generated
            // numbers.
            InvalidateNumRule();
        }

        CheckResetRedlineMergeFlag(*this, eRecreateMerged);
    }
    else {
        OSL_FAIL( "No TextNode." );
    }

    return this;
}

void SwTextNode::JoinPrev()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwContentNode::CanJoinPrev( &aIdx ) )
    {
        SwDoc& rDoc = rNds.GetDoc();
        const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
        pContentStore->Save( rDoc, aIdx.GetIndex(), SAL_MAX_INT32);
        SwTextNode *pTextNode = aIdx.GetNode().GetTextNode();
        const sal_Int32 nLen = pTextNode->Len();

        std::unique_ptr<SwWrongList> pList = pTextNode->ReleaseWrong();
        if( pList )
        {
            pList->JoinList( GetWrong(), Len() );
            SetWrongDirty(sw::WrongState::TODO);
            ClearWrong();
        }
        else
        {
            pList = ReleaseWrong();
            if( pList )
            {
                pList->Move( 0, nLen );
                SetWrongDirty(sw::WrongState::TODO);
            }
        }

        std::unique_ptr<SwGrammarMarkUp> pList3 = pTextNode->ReleaseGrammarCheck();
        if( pList3 )
        {
            pList3->JoinGrammarList( GetGrammarCheck(), Len() );
            SetGrammarCheckDirty( true );
            ClearGrammarCheck();
        }
        else
        {
            pList3 = ReleaseGrammarCheck();
            if( pList3 )
            {
                pList3->MoveGrammar( 0, nLen );
                SetGrammarCheckDirty( true );
            }
        }

        std::unique_ptr<SwWrongList> pList2 = pTextNode->ReleaseSmartTags();
        if( pList2 )
        {
            pList2->JoinList( GetSmartTags(), Len() );
            SetSmartTagDirty( true );
            ClearSmartTags();
        }
        else
        {
            pList2 = ReleaseSmartTags();
            if( pList2 )
            {
                pList2->Move( 0, nLen );
                SetSmartTagDirty( true );
            }
        }

        { // scope for SwContentIndex
            pTextNode->CutText( this, SwContentIndex(this), SwContentIndex(pTextNode), nLen );
        }
        // move all Bookmarks/TOXMarks
        if( !pContentStore->Empty() )
            pContentStore->Restore( rDoc, GetIndex() );

        if( pTextNode->HasAnyIndex() )
        {
            // move all ShellCursor/StackCursor/UnoCursor out of delete range
            rDoc.CorrAbs( aIdx.GetNode(), SwPosition( *this ), nLen, true );
        }
        SwNode::Merge const eOldMergeFlag(pTextNode->GetRedlineMergeFlag());
        if (eOldMergeFlag == SwNode::Merge::First
            && !IsCreateFrameWhenHidingRedlines())
        {
            sw::MoveDeletedPrevFrames(*pTextNode, *this);
        }
        rNds.Delete(aIdx);
        SetWrong( std::move(pList) );
        SetGrammarCheck( std::move(pList3) );
        SetSmartTags( std::move(pList2) );
        resetAndQueueAccessibilityCheck();
        InvalidateNumRule();
        sw::CheckResetRedlineMergeFlag(*this,
                eOldMergeFlag == SwNode::Merge::NonFirst
                    ? sw::Recreate::Predecessor
                    : sw::Recreate::No);
    }
    else {
        OSL_FAIL( "No TextNode." );
    }
}

// create an AttrSet with ranges for Frame-/Para/Char-attributes
void SwTextNode::NewAttrSet( SwAttrPool& rPool )
{
    OSL_ENSURE( !mpAttrSet, "AttrSet is set after all" );
    SwAttrSet aNewAttrSet( rPool, aTextNodeSetRange );

    // put names of parent style and conditional style:
    const SwFormatColl* pAnyFormatColl = &GetAnyFormatColl();
    const SwFormatColl* pFormatColl = GetFormatColl();
    ProgName sVal;
    SwStyleNameMapper::FillProgName( pAnyFormatColl->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
    SfxStringItem aAnyFormatColl( RES_FRMATR_STYLE_NAME, sVal.toString() );
    if ( pFormatColl != pAnyFormatColl )
        SwStyleNameMapper::FillProgName( pFormatColl->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
    SfxStringItem aFormatColl( RES_FRMATR_CONDITIONAL_STYLE_NAME, sVal.toString() );
    aNewAttrSet.Put( aAnyFormatColl );
    aNewAttrSet.Put( aFormatColl );

    aNewAttrSet.SetParent( &pAnyFormatColl->GetAttrSet() );
    mpAttrSet = GetDoc().GetIStyleAccess().getAutomaticStyle( aNewAttrSet, IStyleAccess::AUTO_STYLE_PARA, &sVal );
}

namespace
{
class SwContentNodeTmp : public SwContentNode
{
public:
    SwContentNodeTmp() : SwContentNode() {}
    virtual void NewAttrSet(SwAttrPool&) override {}
    virtual SwContentFrame *MakeFrame(SwFrame*) override { return nullptr; }
    virtual SwContentNode* MakeCopy(SwDoc&, SwNode&, bool /*bNewFrames*/) const override { return nullptr; };
};
};

// override SwContentIndexReg::Update => text hints do not need SwContentIndex for start/end!
void SwTextNode::Update(
    SwContentIndex const & rPos,
    const sal_Int32 nChangeLen,
    UpdateMode const eMode)
{
    assert(rPos.GetContentNode() == this);
    SetAutoCompleteWordDirty( true );

    SwpHts aCollector;
    const sal_Int32 nChangePos = rPos.GetIndex();

    if ( HasHints() )
    {
        if (eMode & UpdateMode::Negative)
        {
            std::vector<SwTextInputField*> aTextInputFields;

            const sal_Int32 nChangeEnd = nChangePos + nChangeLen;
            for ( size_t n = 0; n < m_pSwpHints->Count(); ++n )
            {
                bool bTextAttrChanged = false;
                bool bStartOfTextAttrChanged = false;
                SwTextAttr * const pHint = m_pSwpHints->GetWithoutResorting(n);
                if ( pHint->GetStart() > nChangePos )
                {
                    if ( pHint->GetStart() > nChangeEnd )
                    {
                         pHint->SetStart( pHint->GetStart() - nChangeLen );
                    }
                    else
                    {
                         pHint->SetStart( nChangePos );
                    }
                    bStartOfTextAttrChanged = true;
                }

                const sal_Int32 * const pEnd = pHint->GetEnd();
                if (pEnd && *pEnd > nChangePos )
                {
                    if( *pEnd > nChangeEnd )
                    {
                        pHint->SetEnd(*pEnd - nChangeLen);
                    }
                    else
                    {
                        pHint->SetEnd(nChangePos);
                    }
                    bTextAttrChanged = !bStartOfTextAttrChanged;
                }

                if ( bTextAttrChanged
                     && pHint->Which() == RES_TXTATR_INPUTFIELD )
                {
                    SwTextInputField* pTextInputField = dynamic_cast<SwTextInputField*>(pHint);
                    if ( pTextInputField )
                        aTextInputFields.push_back(pTextInputField);
                }
            }

            //wait until all the attribute positions are correct
            //before updating the field contents
            for (SwTextInputField* pTextInputField : aTextInputFields)
            {
                pTextInputField->UpdateFieldContent();
            }

            m_pSwpHints->MergePortions( *this );
        }
        else
        {
            bool bNoExp = false;
            bool bResort = false;
            bool bMergePortionsNeeded = false;
            const int coArrSz = RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN;
            std::vector<SwTextInputField*> aTextInputFields;

            bool aDontExp[ coArrSz ] = {};

            for ( size_t n = 0; n < m_pSwpHints->Count(); ++n )
            {
                bool bTextAttrChanged = false;
                SwTextAttr * const pHint = m_pSwpHints->GetWithoutResorting(n);
                const sal_Int32 * const pEnd = pHint->GetEnd();
                if ( pHint->GetStart() >= nChangePos )
                {
                    pHint->SetStart( pHint->GetStart() + nChangeLen );
                    if ( pEnd )
                    {
                        pHint->SetEnd(*pEnd + nChangeLen);
                    }
                }
                else if ( pEnd && (*pEnd >= nChangePos) )
                {
                    if ( (*pEnd > nChangePos) || IsIgnoreDontExpand() )
                    {
                        pHint->SetEnd(*pEnd + nChangeLen);
                        bTextAttrChanged = true;
                    }
                    else // *pEnd == nChangePos
                    {
                        const sal_uInt16 nWhich = pHint->Which();

                        OSL_ENSURE(!isCHRATR(nWhich), "Update: char attr hint?");
                        if (!(isCHRATR(nWhich) || isTXTATR_WITHEND(nWhich)))
                            continue;

                        const sal_uInt16 nWhPos = nWhich - RES_CHRATR_BEGIN;

                        if( aDontExp[ nWhPos ] )
                            continue;

                        if ( pHint->DontExpand() )
                        {
                            pHint->SetDontExpand( false );
                            bResort = true;
                            // could have a continuation with IgnoreStart()...
                            if (pHint->IsFormatIgnoreEnd())
                            {
                                bMergePortionsNeeded = true;
                            }
                            if ( pHint->IsCharFormatAttr() )
                            {
                                bNoExp = true;
                                aDontExp[ RES_TXTATR_CHARFMT - RES_CHRATR_BEGIN ] = true;
                                aDontExp[ RES_TXTATR_INETFMT - RES_CHRATR_BEGIN ] = true;
                            }
                            else
                                aDontExp[ nWhPos ] = true;
                        }
                        else if( bNoExp )
                        {
                            auto it = std::find_if(aCollector.begin(), aCollector.end(),
                                [nWhich](const SwTextAttr *pTmp) { return nWhich == pTmp->Which(); });
                            if (it != aCollector.end())
                            {
                                SwTextAttr *pTmp = *it;
                                aCollector.erase( it );
                                SwTextAttr::Destroy( pTmp );
                            }
                            SwTextAttr * const pTmp =
                            MakeTextAttr( GetDoc(),
                                pHint->GetAttr(), nChangePos, nChangePos + nChangeLen);
                            aCollector.push_back( pTmp );
                        }
                        else
                        {
                            pHint->SetEnd(*pEnd + nChangeLen);
                            bTextAttrChanged = true;
                        }
                    }
                }

                if ( bTextAttrChanged
                     && pHint->Which() == RES_TXTATR_INPUTFIELD )
                {
                    SwTextInputField* pTextInputField = dynamic_cast<SwTextInputField*>(pHint);
                    if ( pTextInputField )
                        aTextInputFields.push_back(pTextInputField);
                }
            }

            //wait until all the attribute positions are correct
            //before updating the field contents
            for (SwTextInputField* pTextInputField : aTextInputFields)
            {
                pTextInputField->UpdateFieldContent();
            }

            if (bMergePortionsNeeded)
            {
                m_pSwpHints->MergePortions(*this); // does Resort too
            }
            else if (bResort)
            {
                m_pSwpHints->Resort();
            }
        }
    }

    bool bSortMarks = false;
    SwContentNodeTmp aTmpIdxReg;
    if (!(eMode & UpdateMode::Negative) && !(eMode & UpdateMode::Delete))
    {
        o3tl::sorted_vector<SwRangeRedline*> vMyRedlines;
        // walk the list of SwContentIndex attached to me and see if any of them are redlines
        const SwContentIndex* pContentNodeIndex = GetFirstIndex();
        while (pContentNodeIndex)
        {
            if (pContentNodeIndex->GetOwner() && pContentNodeIndex->GetOwner()->GetOwnerType() == SwContentIndexOwnerType::Redline)
            {
                auto pRedl = static_cast<SwRangeRedline*>(pContentNodeIndex->GetOwner());
                if (pRedl && (pRedl->HasMark() || this == &pRedl->GetPoint()->GetNode()))
                    vMyRedlines.insert(pRedl);
            }
            pContentNodeIndex = pContentNodeIndex->GetNext();
        }
        for (SwRangeRedline* pRedl : vMyRedlines)
        {
            if ( pRedl->HasMark() )
            {
                SwPosition* const pEnd = pRedl->End();
                if ( *this == pEnd->GetNode() &&
                     *pRedl->GetPoint() != *pRedl->GetMark() )
                {
                    SwContentIndex & rIdx = pEnd->nContent;
                    if (nChangePos == rIdx.GetIndex())
                    {
                        rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                    }
                }
            }
            else if ( this == &pRedl->GetPoint()->GetNode() )
            {
                SwContentIndex & rIdx = pRedl->GetPoint()->nContent;
                if (nChangePos == rIdx.GetIndex())
                {
                    rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                }
                // the unused position must not be on a SwTextNode
                bool const isOneUsed(&pRedl->GetBound() == pRedl->GetPoint());
                assert(!pRedl->GetBound(!isOneUsed).GetNode().IsTextNode());
                assert(!pRedl->GetBound(!isOneUsed).GetContentNode()); (void)isOneUsed;
            }
        }

        // Bookmarks must never grow to either side, when editing (directly)
        // to the left or right (i#29942)! Exception: if the bookmark has
        // 2 positions and start == end, then expand it (tdf#96479)
        if (!(eMode & UpdateMode::Replace)) // Exception: Replace
        {
            bool bAtLeastOneBookmarkMoved = false;
            bool bAtLeastOneExpandedBookmarkAtInsertionPosition = false;
            // A text node already knows its marks via its SwContentIndexes.
            o3tl::sorted_vector<const sw::mark::MarkBase*> aSeenMarks;
            const SwContentIndex* next;
            for (const SwContentIndex* pIndex = GetFirstIndex(); pIndex; pIndex = next )
            {
                next = pIndex->GetNext();
                if (!pIndex->GetOwner() || pIndex->GetOwner()->GetOwnerType() != SwContentIndexOwnerType::Mark)
                    continue;
                auto const pMark = static_cast<sw::mark::MarkBase const*>(pIndex->GetOwner());
                // filter out ones that cannot match to reduce the max size of aSeenMarks
                const SwPosition* pMarkPos1 = &pMark->GetMarkPos();
                const SwPosition* pMarkPos2 = pMark->IsExpanded() ? &pMark->GetOtherMarkPos() : nullptr;
                if (pMarkPos1->nContent.GetIndex() != rPos.GetIndex()
                    && (pMarkPos2 == nullptr || pMarkPos2->nContent.GetIndex() != rPos.GetIndex()))
                    continue;
                // Only handle bookmarks once, if they start and end at this node as well.
                if (!aSeenMarks.insert(pMark).second)
                    continue;
                const SwPosition* pEnd = &pMark->GetMarkEnd();
                SwContentIndex & rEndIdx = const_cast<SwContentIndex&>(pEnd->nContent);
                if( *this == pEnd->GetNode() &&
                    rPos.GetIndex() == rEndIdx.GetIndex() )
                {
                    if (&rEndIdx == next) // nasty corner case:
                    {   // don't switch to iterating aTmpIdxReg!
                        next = rEndIdx.GetNext();
                    }
                    // tdf#96479: if start == end, ignore the other position
                    // so it is moved!
                    rEndIdx.Assign( &aTmpIdxReg, rEndIdx.GetIndex() );
                    bAtLeastOneBookmarkMoved = true;
                }
                else if ( !bAtLeastOneExpandedBookmarkAtInsertionPosition )
                {
                    if ( pMark->IsExpanded() )
                    {
                        const SwPosition* pStart = &pMark->GetMarkStart();
                        if ( this == &pStart->GetNode()
                             && rPos.GetIndex() == pStart->GetContentIndex() )
                        {
                            bAtLeastOneExpandedBookmarkAtInsertionPosition = true;
                        }
                    }
                }
            }

            bSortMarks = bAtLeastOneBookmarkMoved && bAtLeastOneExpandedBookmarkAtInsertionPosition;
        }

        // at-char anchored flys shouldn't be moved, either.
        if (!m_bInUndo)
        {
            std::vector<SwFrameFormat*> const& rFlys(GetAnchoredFlys());
            for (size_t i = 0; i != rFlys.size(); ++i)
            {
                SwFrameFormat const*const pFormat = rFlys[i];
                const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
                const SwNode* pAnchorNode = rAnchor.GetAnchorNode();
                if (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR && pAnchorNode)
                {
                    // The fly is at-char anchored and has an anchor position.
                    SwContentIndex& rEndIdx = const_cast<SwContentIndex&>(rAnchor.GetContentAnchor()->nContent);
                    if (*pAnchorNode == *this && rEndIdx.GetIndex() == rPos.GetIndex())
                    {
                        // The anchor position is exactly our insert position.
                        rEndIdx.Assign(&aTmpIdxReg, rEndIdx.GetIndex());
                    }
                }
            }
        }

        // The cursors of other shells shouldn't be moved, either.
        if (SwDocShell* pDocShell = GetDoc().GetDocShell())
        {
            if (pDocShell->GetWrtShell())
            {
                for (SwViewShell& rShell : pDocShell->GetWrtShell()->GetRingContainer())
                {
                    auto pWrtShell = dynamic_cast<SwWrtShell*>(&rShell);
                    if (!pWrtShell || pWrtShell == dynamic_cast<SwWrtShell*>(GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell()))
                        continue;

                    SwShellCursor* pCursor = pWrtShell->GetCursor_();
                    if (!pCursor)
                        continue;

                    SwContentIndex& rIndex = pCursor->Start()->nContent;
                    if (pCursor->Start()->GetNode() == *this && rIndex.GetIndex() == rPos.GetIndex())
                    {
                        // The cursor position of this other shell is exactly our insert position.
                        rIndex.Assign(&aTmpIdxReg, rIndex.GetIndex());
                    }
                }
            }
        }
    }

    // base class
    SwContentIndexReg::Update(rPos, nChangeLen, eMode);

    for ( SwTextAttr* pAttr : aCollector )
    {
        m_pSwpHints->TryInsertHint( pAttr, *this );
    }

    aTmpIdxReg.MoveTo( *this );
    if ( bSortMarks )
    {
        getIDocumentMarkAccess()->assureSortedMarkContainers();
    }

    //Any drawing objects anchored into this text node may be sorted by their
    //anchor position which may have changed here, so resort them
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> iter(*this);
    for (SwTextFrame* pFrame = iter.First(); pFrame; pFrame = iter.Next())
    {
        SwSortedObjs * pSortedObjs(pFrame->GetDrawObjs());
        if (pSortedObjs)
        {
            pSortedObjs->UpdateAll();
        }
        // also sort the objs on the page frame
        if (SwPageFrame *pPage = pFrame->FindPageFrame())
            pSortedObjs = pPage->GetSortedObjs();

        if (pSortedObjs) // doesn't exist yet if called for inserting as-char fly
        {
            pSortedObjs->UpdateAll();
        }
    }

    // Update the paragraph signatures.
    if (SwEditShell* pEditShell = GetDoc().GetEditShell())
    {
        pEditShell->ValidateParagraphSignatures(this, true);
    }

    // Inform LOK clients about change in position of redlines (if any)
    // Don't emit notifications during save: redline flags are temporarily changed during save, but
    // it's not useful to let clients know about such changes.
    if (!comphelper::LibreOfficeKit::isActive() || GetDoc().IsInWriting())
        return;

    const SwRedlineTable& rTable = GetDoc().getIDocumentRedlineAccess().GetRedlineTable();
    for (SwRedlineTable::size_type nRedlnPos = 0; nRedlnPos < rTable.size(); ++nRedlnPos)
    {
        SwRangeRedline* pRedln = rTable[nRedlnPos];
        if (pRedln->HasMark())
        {
            if (*this == pRedln->End()->GetNode() && *pRedln->GetPoint() != *pRedln->GetMark())
            {
                // Redline is changed only when some change occurs before it
                if (nChangePos <= pRedln->Start()->GetContentIndex())
                {
                    SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, pRedln);
                }
            }
        }
        else if (this == &pRedln->GetPoint()->GetNode())
            SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, pRedln);
    }
}

void SwTextNode::ChgTextCollUpdateNum(const SwTextFormatColl* pOldColl,
                                      const SwTextFormatColl* pNewColl,
                                      bool bSetListLevel)
{
    SwDoc& rDoc = GetDoc();
    // query the OutlineLevel and if it changed, notify the Nodes-Array!
    const int nOldLevel = pOldColl && pOldColl->IsAssignedToListLevelOfOutlineStyle()
                              ? pOldColl->GetAssignedOutlineStyleLevel()
                              : MAXLEVEL;
    const int nNewLevel = pNewColl && pNewColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pNewColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;

    if ( MAXLEVEL != nNewLevel && -1 != nNewLevel && bSetListLevel )
    {
        SetAttrListLevel(nNewLevel);
    }
    rDoc.GetNodes().UpdateOutlineNode(*this);

    SwNodes& rNds = GetNodes();
    // If Level 0 (Chapter), update the footnotes!
    if( ( !nNewLevel || !nOldLevel) && !rDoc.GetFootnoteIdxs().empty() &&
        FTNNUM_CHAPTER == rDoc.GetFootnoteInfo().m_eNum &&
        rNds.IsDocNodes() )
    {
        rDoc.GetFootnoteIdxs().UpdateFootnote( *rNds[GetIndex()] );
    }

    if( pNewColl && RES_CONDTXTFMTCOLL == pNewColl->Which() )
    {
        // check the condition of the text node again
        ChkCondColl();
    }
}

// If positioned exactly at the end of a CharStyle or Hyperlink,
// set its DontExpand flag.
bool SwTextNode::DontExpandFormat( sal_Int32 nIdx, bool bFlag,
                                bool bFormatToTextAttributes )
{
    if (bFormatToTextAttributes && nIdx == m_Text.getLength())
    {
        FormatToTextAttr( this );
    }

    bool bRet = false;
    if ( HasHints() )
    {
        m_pSwpHints->SortIfNeedBe();
        int nPos = m_pSwpHints->GetLastPosSortedByEnd(nIdx);
        for ( ; nPos >= 0; --nPos)
        {
            SwTextAttr *pTmp = m_pSwpHints->GetSortedByEnd( nPos );
            const sal_Int32 *pEnd = pTmp->GetEnd();
            if( !pEnd )
                continue;
            assert( *pEnd <= nIdx );
            if( nIdx != *pEnd )
                break;
            if( bFlag != pTmp->DontExpand() && !pTmp->IsLockExpandFlag()
                     && *pEnd > pTmp->GetStart())
            {
                bRet = true;
                m_pSwpHints->NoteInHistory( pTmp );
                pTmp->SetDontExpand( bFlag );
            }
        }
    }
    return bRet;
}

static bool lcl_GetTextAttrDefault(sal_Int32 nIndex, sal_Int32 nHintStart, sal_Int32 nHintEnd)
{
    return ((nHintStart <= nIndex) && (nIndex <  nHintEnd));
}
static bool lcl_GetTextAttrExpand(sal_Int32 nIndex, sal_Int32 nHintStart, sal_Int32 nHintEnd)
{
    return ((nHintStart <  nIndex) && (nIndex <= nHintEnd));
}
static bool lcl_GetTextAttrParent(sal_Int32 nIndex, sal_Int32 nHintStart, sal_Int32 nHintEnd)
{
    return ((nHintStart <  nIndex) && (nIndex <  nHintEnd));
}

static void
lcl_GetTextAttrs(
    std::vector<SwTextAttr *> *const pVector,
    SwTextAttr **const ppTextAttr,
    SwpHints const *const pSwpHints,
    sal_Int32 const nIndex, sal_uInt16 const nWhich,
    ::sw::GetTextAttrMode const eMode)
{
    assert(nWhich >= RES_TXTATR_BEGIN && nWhich < RES_TXTATR_END);
    if (!pSwpHints)
        return;
    size_t const nSize = pSwpHints->Count();
    sal_Int32 nPreviousIndex(0); // index of last hint with nWhich
    bool (*pMatchFunc)(sal_Int32, sal_Int32, sal_Int32)=nullptr;
    switch (eMode)
    {
        case ::sw::GetTextAttrMode::Default: pMatchFunc = &lcl_GetTextAttrDefault;
        break;
        case ::sw::GetTextAttrMode::Expand:  pMatchFunc = &lcl_GetTextAttrExpand;
        break;
        case ::sw::GetTextAttrMode::Parent:  pMatchFunc = &lcl_GetTextAttrParent;
        break;
        default: assert(false);
    }

    for( size_t i = pSwpHints->GetFirstPosSortedByWhichAndStart(nWhich); i < nSize; ++i )
    {
        SwTextAttr *const pHint = pSwpHints->GetSortedByWhichAndStart(i);
        if (pHint->Which() != nWhich)
            break; // hints are sorted by which&start, so we are done...

        sal_Int32 const nHintStart = pHint->GetStart();
        if (nIndex < nHintStart)
            break; // hints are sorted by which&start, so we are done...

        sal_Int32 const*const pEndIdx = pHint->GetEnd();
        // cannot have hint with no end and no dummy char
        assert(pEndIdx || pHint->HasDummyChar());
        // If EXPAND is set, simulate the text input behavior, i.e.
        // move the start, and expand the end.
        bool const bContained( pEndIdx
            ? (*pMatchFunc)(nIndex, nHintStart, *pEndIdx)
            : (nHintStart == nIndex) );
        if (bContained)
        {
            if (pVector)
            {
                if (nPreviousIndex < nHintStart)
                {
                    pVector->clear(); // clear hints that are outside pHint
                    nPreviousIndex = nHintStart;
                }
                pVector->push_back(pHint);
            }
            else
            {
                *ppTextAttr = pHint; // and possibly overwrite outer hint
            }
            if (!pEndIdx)
            {
                break;
            }
        }
    }
}

std::vector<SwTextAttr *>
SwTextNode::GetTextAttrsAt(sal_Int32 const nIndex, sal_uInt16 const nWhich) const
{
    assert(nWhich >= RES_TXTATR_BEGIN && nWhich < RES_TXTATR_END);
    std::vector<SwTextAttr *> ret;
    lcl_GetTextAttrs(&ret, nullptr, m_pSwpHints.get(), nIndex, nWhich, ::sw::GetTextAttrMode::Default);
    return ret;
}

SwTextAttr *
SwTextNode::GetTextAttrAt(sal_Int32 const nIndex, sal_uInt16 const nWhich,
        ::sw::GetTextAttrMode const eMode) const
{
    assert(    (nWhich == RES_TXTATR_META)
            || (nWhich == RES_TXTATR_METAFIELD)
            || (nWhich == RES_TXTATR_AUTOFMT)
            || (nWhich == RES_TXTATR_INETFMT)
            || (nWhich == RES_TXTATR_CJK_RUBY)
            || (nWhich == RES_TXTATR_UNKNOWN_CONTAINER)
            || (nWhich == RES_TXTATR_CONTENTCONTROL)
            || (nWhich == RES_TXTATR_INPUTFIELD ) );
            // "GetTextAttrAt() will give wrong result for this hint!")

    SwTextAttr * pRet(nullptr);
    lcl_GetTextAttrs(nullptr, & pRet, m_pSwpHints.get(), nIndex, nWhich, eMode);
    return pRet;
}

const SwTextInputField* SwTextNode::GetOverlappingInputField( const SwTextAttr& rTextAttr ) const
{
    const SwTextInputField* pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextAttrAt(rTextAttr.GetStart(), RES_TXTATR_INPUTFIELD, ::sw::GetTextAttrMode::Parent));

    if ( pTextInputField == nullptr && rTextAttr.End() != nullptr )
    {
        pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextAttrAt(*(rTextAttr.End()), RES_TXTATR_INPUTFIELD, ::sw::GetTextAttrMode::Parent));
    }

    return pTextInputField;
}

void SwTextNode::DelFrames_TextNodePart()
{
    SetWrong( nullptr );
    SetWrongDirty(sw::WrongState::TODO);

    SetGrammarCheck( nullptr );
    SetGrammarCheckDirty( true );

    SetSmartTags( nullptr );
    SetSmartTagDirty( true );

    SetWordCountDirty( true );
    SetAutoCompleteWordDirty( true );
}

SwTextField* SwTextNode::GetFieldTextAttrAt(
    const sal_Int32 nIndex,
    ::sw::GetTextAttrMode const eMode) const
{
    SwTextField* pTextField = dynamic_cast<SwTextField*>(GetTextAttrForCharAt( nIndex, RES_TXTATR_FIELD ));
    if ( pTextField == nullptr )
    {
        pTextField = dynamic_cast<SwTextField*>(GetTextAttrForCharAt( nIndex, RES_TXTATR_ANNOTATION ));
    }
    if ( pTextField == nullptr )
    {
        pTextField =
            dynamic_cast<SwTextField*>( GetTextAttrAt(
                nIndex,
                RES_TXTATR_INPUTFIELD,
                eMode));
    }

    return pTextField;
}

static SwCharFormat* lcl_FindCharFormat( const SwCharFormats* pCharFormats, const UIName& rName )
{
    if( !rName.isEmpty() )
    {
        const size_t nArrLen = pCharFormats->size();
        for( size_t i = 1; i < nArrLen; i++ )
        {
            SwCharFormat* pFormat = (*pCharFormats)[ i ];
            if( pFormat->GetName()==rName )
                return pFormat;
        }
    }
    return nullptr;
}

static void lcl_CopyHint(
    const sal_uInt16 nWhich,
    const SwTextAttr * const pHt,
    SwTextAttr *const pNewHt,
    SwDoc *const pOtherDoc,
    SwTextNode *const pDest )
{
    assert(nWhich == pHt->Which()); // wrong hint-id
    switch( nWhich )
    {
    // copy nodesarray section with footnote content
    case RES_TXTATR_FTN :
            assert(pDest); // "lcl_CopyHint: no destination text node?"
            static_cast<const SwTextFootnote*>(pHt)->CopyFootnote( *static_cast<SwTextFootnote*>(pNewHt), *pDest);
            break;

    // Fields that are copied into different SwDocs must be registered
    // at their new FieldTypes.

    case RES_TXTATR_FIELD :
        {
            if( pOtherDoc != nullptr )
            {
                static_txtattr_cast<const SwTextField*>(pHt)->CopyTextField(
                        static_txtattr_cast<SwTextField*>(pNewHt));
            }

            // Table Formula must be copied relative.
            const SwFormatField& rField = pHt->GetFormatField();
            if( SwFieldIds::Table == rField.GetField()->GetTyp()->Which()
                && static_cast<const SwTableField*>(rField.GetField())->IsIntrnlName())
            {
                // convert internal formula to external
                const SwTableNode* const pDstTableNd =
                    static_txtattr_cast<const SwTextField*>(pHt)->GetTextNode().FindTableNode();
                if( pDstTableNd )
                {
                    SwTableField* const pTableField =
                        const_cast<SwTableField*>(static_cast<const SwTableField*>(
                            pNewHt->GetFormatField().GetField()));
                    pTableField->PtrToBoxNm( &pDstTableNd->GetTable() );
                }
            }
        }
        break;

    case RES_TXTATR_INPUTFIELD :
    case RES_TXTATR_ANNOTATION :
        if( pOtherDoc != nullptr )
        {
            static_txtattr_cast<const SwTextField*>(pHt)->CopyTextField(
                    static_txtattr_cast<SwTextField*>(pNewHt));
        }
        break;

    case RES_TXTATR_TOXMARK :
        if( pOtherDoc && pDest && pDest->GetpSwpHints()
            && pDest->GetpSwpHints()->Contains( pNewHt ) )
        {
            // ToXMarks that are copied to different SwDocs must register
            // at their new ToX (sw::BroadcastingModify).
            static_txtattr_cast<SwTextTOXMark*>(pNewHt)->CopyTOXMark(*pOtherDoc);
        }
        break;

    case RES_TXTATR_CHARFMT :
        // For CharacterStyles, the format must be copied too.
        if( pDest && pDest->GetpSwpHints()
            && pDest->GetpSwpHints()->Contains( pNewHt ) )
        {
            SwCharFormat* pFormat = pHt->GetCharFormat().GetCharFormat();

            if (pOtherDoc)
            {
                pFormat = pOtherDoc->CopyCharFormat( *pFormat );
            }
            const_cast<SwFormatCharFormat&>(
                pNewHt->GetCharFormat() ).SetCharFormat( pFormat );
        }
        break;
    case RES_TXTATR_INETFMT :
        {
            // For Hyperlinks, the format must be copied too.
            if( pOtherDoc && pDest && pDest->GetpSwpHints()
                && pDest->GetpSwpHints()->Contains( pNewHt ) )
            {
                const SwDoc& rDoc = static_txtattr_cast<
                        const SwTextINetFormat*>(pHt)->GetTextNode().GetDoc();
                const SwCharFormats* pCharFormats = rDoc.GetCharFormats();
                const SwFormatINetFormat& rFormat = pHt->GetINetFormat();
                SwCharFormat* pFormat;
                pFormat = lcl_FindCharFormat( pCharFormats, rFormat.GetINetFormat() );
                if( pFormat )
                    pOtherDoc->CopyCharFormat( *pFormat );
                pFormat = lcl_FindCharFormat( pCharFormats, rFormat.GetVisitedFormat() );
                if( pFormat )
                    pOtherDoc->CopyCharFormat( *pFormat );
            }
            //JP 24.04.98: The attribute must point to a text node, so that
            //             the styles can be created.
            SwTextINetFormat *const pINetHt = static_txtattr_cast<SwTextINetFormat*>(pNewHt);
            if ( !pINetHt->GetpTextNode() )
            {
                pINetHt->ChgTextNode( pDest );
            }

            //JP 22.10.97: set up link to char style
            pINetHt->GetCharFormat();
            break;
        }
    case RES_TXTATR_META:
    case RES_TXTATR_METAFIELD:
        OSL_ENSURE( pNewHt, "copying Meta should not fail!" );
        OSL_ENSURE( pDest && pNewHt
                    && (CH_TXTATR_INWORD == pDest->GetText()[pNewHt->GetStart()]),
            "missing CH_TXTATR?");
        break;
    }
}

/// copy attributes at position nTextStartIdx to node pDest
//  BP 7.6.93:      Intentionally copy only attributes _with_ EndIdx!
//                  CopyAttr is usually called when attributes are set on a
//                  node with no text.
void SwTextNode::CopyAttr( SwTextNode *pDest, const sal_Int32 nTextStartIdx,
                          const sal_Int32 nOldPos )
{
    if ( HasHints() )
    {
        SwDoc* const pOtherDoc = (&pDest->GetDoc() != &GetDoc()) ?
                &pDest->GetDoc() : nullptr;

        for ( size_t i = 0; i < m_pSwpHints->Count(); ++i )
        {
            SwTextAttr *const pHt = m_pSwpHints->Get(i);
            sal_Int32 const nAttrStartIdx = pHt->GetStart();
            if ( nTextStartIdx < nAttrStartIdx )
                break; // beyond end of text, because nLen == 0

            const sal_Int32 *const pEndIdx = pHt->GetEnd();
            if ( pEndIdx && !pHt->HasDummyChar() )
            {
                sal_uInt16 const nWhich = pHt->Which();
                if (RES_TXTATR_INPUTFIELD != nWhich // fdo#74981 skip fields
                    && (    *pEndIdx > nTextStartIdx
                        || (*pEndIdx == nTextStartIdx
                            && nAttrStartIdx == nTextStartIdx)))
                {
                    if ( RES_TXTATR_REFMARK != nWhich )
                    {
                        // attribute in the area => copy
                        SwTextAttr *const pNewHt =
                            pDest->InsertItem( pHt->GetAttr(), nOldPos, nOldPos, SetAttrMode::IS_COPY);
                        if ( pNewHt )
                        {
                            lcl_CopyHint( nWhich, pHt, pNewHt,
                                pOtherDoc, pDest );
                        }
                    }
                    else if( !pOtherDoc
                             ? GetDoc().IsCopyIsMove()
                             : nullptr == pOtherDoc->GetRefMark( pHt->GetRefMark().GetRefName() ) )
                    {
                        pDest->InsertItem(
                            pHt->GetAttr(), nOldPos, nOldPos, SetAttrMode::IS_COPY);
                    }
                }
            }
        }
    }

    if( this != pDest )
    {
        // notify layout frames, to prevent disappearance of footnote numbers
        SwUpdateAttr aHint(
            nOldPos,
            nOldPos,
            0);

        pDest->TriggerNodeUpdate(sw::UpdateAttrHint(&aHint, &aHint));
    }
}

/// copy text and attributes to node pDest
void SwTextNode::CopyText( SwTextNode *const pDest,
                      const SwContentIndex &rStart,
                      const sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    SwContentIndex const aIdx( pDest, pDest->m_Text.getLength() );
    CopyText( pDest, aIdx, rStart, nLen, bForceCopyOfAllAttrs );
}

void SwTextNode::CopyText( SwTextNode *const pDest,
                      const SwContentIndex &rDestStart,
                      const SwPosition &rStart,
                      sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    CopyText( pDest, rDestStart, rStart.nContent, nLen, bForceCopyOfAllAttrs );
}

void SwTextNode::EstablishParentChildRelationsOfComments(
    const SwTextNode* pDest,
    std::map<sal_Int32, sal_Int32>& idMapForComments,
    std::map<sal_Int32, SwMarkName>& nameMapForComments
)
{
    if (idMapForComments.size() > 0)
    {
        const SwpHints &rDestHints = pDest->GetSwpHints();
        size_t hintCount = rDestHints.Count();
        for (size_t inDest = 0; inDest < hintCount; inDest++)
        {
            if (rDestHints.Get(inDest)->Which() == RES_TXTATR_ANNOTATION)
            {
                SwPostItField* copiedField = const_cast<SwPostItField*>(static_cast<const SwPostItField*>(rDestHints.Get(inDest)->GetFormatField().GetField()));
                if (copiedField && copiedField->GetParentPostItId() != 0)
                {
                    const auto correspondingParentItem = idMapForComments.find(copiedField->GetParentPostItId());
                    if (correspondingParentItem != idMapForComments.end())
                    {
                        copiedField->SetParentName(nameMapForComments[copiedField->GetParentPostItId()]); // Set name first, parent id will change.
                        copiedField->SetParentPostItId(correspondingParentItem->second);
                    }
                }
            }
        }
    }
}

void SwTextNode::CopyText( SwTextNode *const pDest,
                      const SwContentIndex &rDestStart,
                      const SwContentIndex &rStart,
                      sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    CHECK_SWPHINTS_IF_FRM(this);
    CHECK_SWPHINTS(pDest);
    assert(rDestStart.GetContentNode() == pDest);
    assert(rStart.GetContentNode() == this);
    sal_Int32 nTextStartIdx = rStart.GetIndex();
    sal_Int32 nDestStart = rDestStart.GetIndex();      // remember old Pos

    if (pDest->GetDoc().IsClipBoard() && GetNum())
    {
        // #i111677# cache expansion of source (for clipboard)
        pDest->m_oNumStringCache = (nTextStartIdx != 0)
            ? OUString() // fdo#49076: numbering only if copy from para start
            : GetNumString();
    }

    if( !nLen )
    {
        // if no length is given, copy attributes at position rStart
        CopyAttr( pDest, nTextStartIdx, nDestStart );

        // copy hard attributes on whole paragraph
        if( HasSwAttrSet() )
        {
            // i#96213 all or just the Char attributes?
            if ( !bForceCopyOfAllAttrs &&
                 ( nDestStart ||
                   pDest->HasSwAttrSet() ||
                   nLen != pDest->GetText().getLength()))
            {
                SfxItemSetFixed<
                        RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                        RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>
                    aCharSet( pDest->GetDoc().GetAttrPool() );
                aCharSet.Put( *GetpSwAttrSet() );
                if( aCharSet.Count() )
                {
                    pDest->SetAttr( aCharSet, nDestStart, nDestStart );
                }
            }
            else
            {
                GetpSwAttrSet()->CopyToModify( *pDest );
            }
        }
        return;
    }

    // 1. copy text
    const sal_Int32 oldLen = pDest->m_Text.getLength();
    // JP 15.02.96: missing attribute handling at the end!
    //              hence call InsertText and don't modify m_Text directly
    pDest->InsertText( m_Text.copy(nTextStartIdx, nLen), rDestStart,
                   SwInsertFlags::EMPTYEXPAND );

    // update with actual new size
    nLen = pDest->m_Text.getLength() - oldLen;
    if ( !nLen ) // string not longer?
        return;

    SwDoc* const pOtherDoc = (&pDest->GetDoc() != &GetDoc()) ? &pDest->GetDoc() : nullptr;

    // copy hard attributes on whole paragraph
    if( HasSwAttrSet() )
    {
        // i#96213 all or just the Char attributes?
        if ( !bForceCopyOfAllAttrs &&
             ( nDestStart ||
               pDest->HasSwAttrSet() ||
               nLen != pDest->GetText().getLength()))
        {
            SfxItemSetFixed<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>
                 aCharSet( pDest->GetDoc().GetAttrPool() );
            aCharSet.Put( *GetpSwAttrSet() );
            if( aCharSet.Count() )
            {
                pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
            }
        }
        else
        {
            GetpSwAttrSet()->CopyToModify( *pDest );
        }
    }

    bool const bUndoNodes = !pOtherDoc
                            && GetDoc().GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

    // Fetch end only now, because copying into self updates the start index
    // and all attributes
    nTextStartIdx = rStart.GetIndex();
    const sal_Int32 nEnd = nTextStartIdx + nLen;

    // 2. copy attributes
    // Iterate over attribute array until the start of the attribute
    // is behind the copied range
    const size_t nSize = m_pSwpHints ? m_pSwpHints->Count() : 0;

    // If copying into self, inserting can delete attributes!
    // Hence first copy into temp-array, and then move that into hints array.
    SwpHts aArr;

    // Del-Array for all RefMarks without extent
    SwpHts aRefMrkArr;

    std::vector<std::pair<sal_Int32, sal_Int32>> metaFieldRanges;

    /*
        Annotations are also copied along with other fields.
        Annotations have parentPostItId field, used for parent-child relation.
        So we also need to set parent ids of comments when applicable.
        Below map variable is for memorizing the new ids and names of parent postits in the source node, then we will use them in target node.
    */
    std::map<sal_Int32, sal_Int32> idMapForComments;
    std::map<sal_Int32, SwMarkName> nameMapForComments;

    sal_Int32 nDeletedDummyChars(0);
    for (size_t n = 0; n < nSize; ++n)
    {
        SwTextAttr * const pHt = m_pSwpHints->Get(n);

        const sal_Int32 nAttrStartIdx = pHt->GetStart();
        if ( nAttrStartIdx >= nEnd )
            break;

        const sal_Int32 * const pEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();

        // JP 26.04.94: RefMarks are never copied. If the refmark doesn't have
        //              an extent, there is a dummy char in the text, which
        //              must be removed. So we first copy the attribute,
        //              but remember it, and when we're done delete it,
        //              which also deletes the dummy character!
        // JP 14.08.95: May RefMarks be moved?
        const bool bCopyRefMark = RES_TXTATR_REFMARK == nWhich
                                  && ( bUndoNodes
                                       || ( !pOtherDoc
                                            ? GetDoc().IsCopyIsMove()
                                            : nullptr == pOtherDoc->GetRefMark( pHt->GetRefMark().GetRefName() ) ) );

        if ( pEndIdx
             && RES_TXTATR_REFMARK == nWhich
             && !bCopyRefMark )
        {
            continue;
        }

        // Input Fields are only copied, if completely covered by copied text
        if ( nWhich == RES_TXTATR_INPUTFIELD )
        {
            assert(pEndIdx != nullptr &&
                    "<SwTextNode::CopyText(..)> - RES_TXTATR_INPUTFIELD without EndIndex!" );
            if ( nAttrStartIdx < nTextStartIdx
                 || ( pEndIdx != nullptr
                      && *pEndIdx > nEnd ) )
            {
                continue;
            }
        }

        if (nWhich == RES_TXTATR_METAFIELD)
        {
            // Skip metadata fields. Also remember the range to strip the text later.
            metaFieldRanges.emplace_back(nAttrStartIdx, pEndIdx ? *pEndIdx : nEnd);
            continue;
        }

        sal_Int32 nAttrStt = 0;
        sal_Int32 nAttrEnd = 0;

        if( nAttrStartIdx < nTextStartIdx )
        {
            // start is before selection
            // copy hints with end and CH_TXTATR only if dummy char is copied
            if ( pEndIdx && (*pEndIdx > nTextStartIdx) && !pHt->HasDummyChar() )
            {
                // attribute with extent and the end is in the selection
                nAttrStt = nDestStart;
                nAttrEnd = (*pEndIdx > nEnd)
                    ? rDestStart.GetIndex()
                    : nDestStart + (*pEndIdx) - nTextStartIdx;
            }
            else
            {
                continue;
            }
        }
        else
        {
            // start is in the selection
            nAttrStt = nDestStart + ( nAttrStartIdx - nTextStartIdx );
            if( pEndIdx )
            {
                nAttrEnd = *pEndIdx > nEnd
                    ? rDestStart.GetIndex()
                    : nDestStart + ( *pEndIdx - nTextStartIdx );
            }
            else
            {
                nAttrEnd = nAttrStt;
            }
        }

        SwTextAttr * pNewHt = nullptr;

        if( pDest == this )
        {
            // copy the hint here, but insert it later
            pNewHt = MakeTextAttr( GetDoc(), pHt->GetAttr(),
                    nAttrStt, nAttrEnd, CopyOrNewType::Copy, pDest );

            lcl_CopyHint(nWhich, pHt, pNewHt, nullptr, pDest);
            aArr.push_back( pNewHt );
        }
        else
        {
            pNewHt = pDest->InsertItem(
                pHt->GetAttr(),
                nAttrStt - nDeletedDummyChars,
                nAttrEnd - nDeletedDummyChars,
                SetAttrMode::NOTXTATRCHR | SetAttrMode::IS_COPY);
            if (pNewHt)
            {
                lcl_CopyHint( nWhich, pHt, pNewHt, pOtherDoc, pDest );
                if (nWhich == RES_TXTATR_ANNOTATION)
                {
                    const SwPostItField* annotationField = static_cast<const SwPostItField*>(pHt->GetFormatField().GetField());
                    // Preparation for EstablishParentChildRelationsOfComments.
                    idMapForComments[annotationField->GetPostItId()] = static_cast<const SwPostItField*>(pNewHt->GetFormatField().GetField())->GetPostItId();
                    nameMapForComments[annotationField->GetPostItId()] = static_cast<const SwPostItField*>(pNewHt->GetFormatField().GetField())->GetName();
                }
            }
            else if (pHt->HasDummyChar())
            {
                // The attribute that has failed to be copied would insert
                // dummy char, so positions of the following attributes have
                // to be shifted by one to compensate for that missing char.
                ++nDeletedDummyChars;
            }
        }

        if( RES_TXTATR_REFMARK == nWhich && !pEndIdx && !bCopyRefMark )
        {
            aRefMrkArr.push_back( pNewHt );
        }
    }

    // Strip the metadata fields, since we don't copy the RDF entries
    // yet and so they are inconsistent upon copy/pasting.
    if (!metaFieldRanges.empty())
    {
        // Reverse to remove without messing the offsets.
        std::reverse(metaFieldRanges.begin(), metaFieldRanges.end());
        for (const auto& pair : metaFieldRanges)
        {
            const SwContentIndex aIdx(pDest, pair.first);
            pDest->EraseText(aIdx, pair.second - pair.first);
        }
    }

    EstablishParentChildRelationsOfComments(pDest, idMapForComments, nameMapForComments);

    // this can only happen when copying into self
    for (SwTextAttr* i : aArr)
    {
        InsertHint( i, SetAttrMode::NOTXTATRCHR );
    }

    if( pDest->GetpSwpHints() )
    {
        for (SwTextAttr* pNewHt : aRefMrkArr)
        {
            if( pNewHt->GetEnd() )
            {
                pDest->GetpSwpHints()->Delete( pNewHt );
                pDest->DestroyAttr( pNewHt );
            }
            else
            {
                const SwContentIndex aIdx( pDest, pNewHt->GetStart() );
                pDest->EraseText( aIdx, 1 );
            }
        }
    }

    CHECK_SWPHINTS_IF_FRM(this);
    CHECK_SWPHINTS(pDest);
}

OUString SwTextNode::InsertText( const OUString & rStr, const SwPosition & rIdx,
        const SwInsertFlags nMode )
{
    return InsertText(rStr, rIdx.nContent, nMode);
}

OUString SwTextNode::InsertText( const OUString & rStr, const SwContentIndex & rIdx,
        const SwInsertFlags nMode )
{
    assert(rIdx.GetContentNode() == this);
    assert(rIdx <= m_Text.getLength()); // invalid index

    const sal_Int32 aPos = rIdx.GetIndex();
    sal_Int32 nLen = m_Text.getLength() - aPos;
    sal_Int32 const nOverflow(rStr.getLength() - GetSpaceLeft());
    SAL_WARN_IF(nOverflow > 0, "sw.core",
            "SwTextNode::InsertText: node text with insertion > capacity.");
    OUString const sInserted(
        (nOverflow > 0) ? rStr.copy(0, rStr.getLength() - nOverflow) : rStr);
    if (sInserted.isEmpty())
    {
        return sInserted;
    }
    if (aPos == 0 && m_Text.isEmpty())
        m_Text = sInserted;
    else
        m_Text = m_Text.replaceAt(aPos, 0, sInserted);
    assert(GetSpaceLeft()>=0);
    nLen = m_Text.getLength() - aPos - nLen;
    assert(nLen != 0);

    bool bOldExpFlg = IsIgnoreDontExpand();
    if (nMode & SwInsertFlags::FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( true );
    }

    Update(rIdx, nLen, UpdateMode::Default); // text content changed!

    if (nMode & SwInsertFlags::FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( bOldExpFlg );
    }

    if ( HasWriterListeners() )
    {   // send this before messing with hints, which will send RES_UPDATE_ATTR
        auto aInsHint = sw::MakeInsertText(*this, aPos, nLen);
        CallSwClientNotify(aInsHint);
    }

    if ( HasHints() )
    {
        m_pSwpHints->SortIfNeedBe();
        bool const bHadHints(!m_pSwpHints->CanBeDeleted());
        bool bMergePortionsNeeded(false);
        for ( size_t i = 0; i < m_pSwpHints->Count() &&
                rIdx >= m_pSwpHints->GetWithoutResorting(i)->GetStart(); ++i )
        {
            SwTextAttr * const pHt = m_pSwpHints->GetWithoutResorting( i );
            const sal_Int32 * const pEndIdx = pHt->GetEnd();
            if( !pEndIdx )
                continue;

            if( rIdx == *pEndIdx )
            {
                if (  (nMode & SwInsertFlags::NOHINTEXPAND) ||
                    (!(nMode & SwInsertFlags::FORCEHINTEXPAND)
                     && pHt->DontExpand()) )
                {
                    m_pSwpHints->DeleteAtPos(i);
                    // on empty attributes also adjust Start
                    if( rIdx == pHt->GetStart() )
                        pHt->SetStart( pHt->GetStart() - nLen );
                    pHt->SetEnd(*pEndIdx - nLen);
                    // could be that pHt has IsFormatIgnoreEnd set, and it's
                    // not a RSID-only hint - now we have the inserted text
                    // between pHt and its continuation... which we don't know.
                    // punt the job to MergePortions below.
                    if (pHt->IsFormatIgnoreEnd())
                    {
                        bMergePortionsNeeded = true;
                    }
                    InsertHint( pHt, SetAttrMode::NOHINTADJUST );
                }
                // empty hints at insert position?
                else if ( (nMode & SwInsertFlags::EMPTYEXPAND)
                        && (*pEndIdx == pHt->GetStart()) )
                {
                    m_pSwpHints->DeleteAtPos(i);
                    pHt->SetStart( pHt->GetStart() - nLen );
                    const size_t nCurrentLen = m_pSwpHints->Count();
                    InsertHint( pHt/* AUTOSTYLES:, SetAttrMode::NOHINTADJUST*/ );
                    if ( nCurrentLen > m_pSwpHints->Count() && i )
                    {
                        --i;
                    }
                    continue;
                }
                else
                {
                    continue;
                }
            }
            if ( !(nMode & SwInsertFlags::NOHINTEXPAND) &&
                 rIdx == nLen && pHt->GetStart() == rIdx.GetIndex() &&
                 !pHt->IsDontExpandStartAttr() )
            {
                // no field, at paragraph start, HintExpand
                m_pSwpHints->DeleteAtPos(i);
                pHt->SetStart( pHt->GetStart() - nLen );
                // no effect on format ignore flags here (para start)
                InsertHint( pHt, SetAttrMode::NOHINTADJUST );
            }
        }
        if (bMergePortionsNeeded)
        {
            m_pSwpHints->MergePortions(*this);
        }
        SAL_WARN_IF(bHadHints && m_pSwpHints->CanBeDeleted(), "sw.core",
                "SwTextNode::InsertText: unexpected loss of hints");
    }

    // By inserting a character, the hidden flags
    // at the TextNode can become invalid:
    SetCalcHiddenCharFlags();

    CHECK_SWPHINTS(this);
    return sInserted;
}

void SwTextNode::CutText( SwTextNode * const pDest,
            const SwContentIndex & rStart, const sal_Int32 nLen )
{
    assert(pDest); // Cut requires a destination
    SwContentIndex aDestStt(pDest, pDest->GetText().getLength());
    CutImpl( pDest, aDestStt, rStart, nLen, false );
}

void SwTextNode::CutImpl( SwTextNode * const pDest, const SwContentIndex & rDestStart,
         const SwContentIndex & rStart, sal_Int32 nLen, const bool bUpdate )
{
    assert(pDest); // Cut requires a destination

    assert(&GetDoc() == &pDest->GetDoc()); // must be same document

    assert(pDest != this); // destination must be different node

    assert(rDestStart.GetContentNode() == pDest);
    assert(rStart.GetContentNode() == this);

    if( !nLen )
    {
        // if no length is given, copy attributes at position rStart
        CopyAttr( pDest, rStart.GetIndex(), rDestStart.GetIndex() );
        return;
    }

    sal_Int32 nTextStartIdx = rStart.GetIndex();
    sal_Int32 nDestStart = rDestStart.GetIndex();      // remember old Pos
    const sal_Int32 nInitSize = pDest->m_Text.getLength();

    if (pDest->GetSpaceLeft() < nLen)
    {   // FIXME: could only happen when called from SwRangeRedline::Show.
        // unfortunately can't really do anything here to handle that...
        abort();
    }
    pDest->m_Text = pDest->m_Text.replaceAt(nDestStart, 0,
                        m_Text.subView(nTextStartIdx, nLen));
    OUString const newText = m_Text.replaceAt(nTextStartIdx, nLen, u"");
    nLen = pDest->m_Text.getLength() - nInitSize; // update w/ current size!
    if (!nLen)                 // String didn't grow?
        return;

    if (bUpdate)
    {
        // Update all SwContentIndex
        pDest->Update(rDestStart, nLen, UpdateMode::Default);
    }

    CHECK_SWPHINTS(pDest);

    const sal_Int32 nEnd = rStart.GetIndex() + nLen;
    bool const bUndoNodes =
        GetDoc().GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

    // copy hard attributes on whole paragraph
    if (HasSwAttrSet())
    {
        bool hasSwAttrSet = pDest->HasSwAttrSet();
        if (hasSwAttrSet)
        {
            // if we have our own property set it doesn't mean
            // that this set defines any style different to Standard one.
            hasSwAttrSet = false;

            // so, let's check deeper if property set has defined any property
            if (pDest->GetpSwAttrSet())
            {
                // check all items in the property set
                SfxItemIter aIter( *pDest->GetpSwAttrSet() );
                const SfxPoolItem* pItem = aIter.GetCurItem();
                do
                {
                    // check current item
                    const sal_uInt16 nWhich = IsInvalidItem( pItem )
                        ? aIter.GetCurWhich()
                        : pItem->Which();
                    if( RES_FRMATR_STYLE_NAME != nWhich &&
                        RES_FRMATR_CONDITIONAL_STYLE_NAME != nWhich &&
                        RES_PAGEDESC != nWhich &&
                        RES_BREAK != nWhich &&
                        SfxItemState::SET == pDest->GetpSwAttrSet()->GetItemState( nWhich, false ) )
                    {
                        // check if parent value (original value in style) has the same value as in [pItem]
                        const SfxPoolItem&  rParentItem = pDest->GetpSwAttrSet()->GetParent()->Get( nWhich, true );

                        hasSwAttrSet = (rParentItem != *pItem);

                        // property set is not empty => no need to make anymore checks
                        if (hasSwAttrSet)
                            break;
                    }

                    // let's check next item
                    pItem = aIter.NextItem();
                } while (pItem);
            }
        }

        // all or just the Char attributes?
        if( nInitSize || hasSwAttrSet ||
            nLen != pDest->GetText().getLength())
        {
            SfxItemSetFixed<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>
                 aCharSet( pDest->GetDoc().GetAttrPool() );
            aCharSet.Put( *GetpSwAttrSet() );
            if( aCharSet.Count() )
                pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
        }
        else
        {
            // Copy all attrs except RES_PARATR_LIST_LEVEL: it was initialized before
            // and current SwTextNode can contain not suitable for pDest value
            SfxItemSetFixed<RES_CHRATR_BEGIN, RES_PARATR_LIST_LEVEL - 1,
                   RES_PARATR_LIST_LEVEL + 1, HINT_END>
                 aCharSet(pDest->GetDoc().GetAttrPool());
            aCharSet.Put(*GetpSwAttrSet());
            if (aCharSet.Count())
                pDest->SetAttr(aCharSet, nDestStart, nDestStart + nLen);
        }
    }

    // notify frames - before moving hints, because footnotes
    // want to find their anchor text frame in the follow chain
    // (also ignore fieldmarks, the caller will recreate frames)
    const sw::InsertText aInsHint(nDestStart, nLen, false, false);
    pDest->HandleNonLegacyHint(aInsHint);
    const sw::MoveText aMoveHint(pDest, nDestStart, nTextStartIdx, nLen);
    CallSwClientNotify(aMoveHint);
    const sw::DeleteText aDelText(nTextStartIdx, nLen);
    HandleNonLegacyHint(aDelText);

    // 2. move attributes
    // Iterate over attribute array until the start of the attribute
    // is behind the moved range
    bool bMergePortionsNeeded(false);
    size_t nAttrCnt = 0;
    while (m_pSwpHints && (nAttrCnt < m_pSwpHints->Count()))
    {
        SwTextAttr * const pHt = m_pSwpHints->Get(nAttrCnt);
        const sal_Int32 nAttrStartIdx = pHt->GetStart();
        if ( nAttrStartIdx >= nEnd )
            break;
        const sal_Int32 * const pEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();
        SwTextAttr *pNewHt = nullptr;

        // if the hint has a dummy character, then it must not be split!
        if(nAttrStartIdx < nTextStartIdx)
        {
            // start is before the range
            if (!pHt->HasDummyChar() && ( RES_TXTATR_REFMARK != nWhich
                || bUndoNodes ) && pEndIdx && *pEndIdx > nTextStartIdx)
            {
                // attribute with extent and end of attribute is in the range
                pNewHt = MakeTextAttr( pDest->GetDoc(), pHt->GetAttr(),
                                nDestStart,
                                nDestStart + (
                                    *pEndIdx > nEnd
                                        ? nLen
                                        : *pEndIdx - nTextStartIdx ) );
            }
        }
        else
        {
            // start is inside the range
            if (!pEndIdx || *pEndIdx < nEnd ||
                (!bUndoNodes && RES_TXTATR_REFMARK == nWhich)
                || pHt->HasDummyChar() )
            {
                // do not delete note and later add it -> sidebar flickering
                SwDocShell* pShell = GetDoc().GetDocShell();
                if (pShell)
                {
                    pShell->Broadcast( SfxHint(SfxHintId::SwSplitNodeOperation));
                }
                // move attribute
                m_pSwpHints->Delete( pHt );
                // reset start/end indexes
                if (pHt->IsFormatIgnoreStart() || pHt->IsFormatIgnoreEnd())
                {
                    bMergePortionsNeeded = true;
                }
                pHt->SetStart(nDestStart + (nAttrStartIdx - nTextStartIdx));
                if (pEndIdx)
                {
                    pHt->SetEnd( nDestStart + (
                                    *pEndIdx > nEnd
                                        ? nLen
                                        : *pEndIdx - nTextStartIdx ) );
                }
                pDest->InsertHint( pHt,
                          SetAttrMode::NOTXTATRCHR
                        | SetAttrMode::DONTREPLACE );
                if (pShell)
                {
                    pShell->Broadcast( SfxHint(SfxHintId::SwSplitNodeOperation));
                }
                continue;           // iterate while loop, no ++ !
            }
                // the end is behind the range
            else if (RES_TXTATR_REFMARK != nWhich || bUndoNodes)
            {
                pNewHt = MakeTextAttr( GetDoc(), pHt->GetAttr(),
                              nDestStart + (nAttrStartIdx - nTextStartIdx),
                              nDestStart + (*pEndIdx > nEnd
                                             ? nLen
                                             : *pEndIdx - nTextStartIdx));
            }
        }
        if (pNewHt)
        {
            const bool bSuccess( pDest->InsertHint( pNewHt,
                          SetAttrMode::NOTXTATRCHR
                        | SetAttrMode::DONTREPLACE
                        | SetAttrMode::IS_COPY) );
            if (bSuccess)
            {
                lcl_CopyHint( nWhich, pHt, pNewHt, nullptr, pDest );
            }
        }
        ++nAttrCnt;
    }
    // If there are still empty attributes around, they have a higher priority
    // than any attributes that become empty due to the move.
    // So temporarily remove them and Update the array, then re-insert the
    // removed ones so they overwrite the others.
    if (m_pSwpHints && nAttrCnt < m_pSwpHints->Count())
    {
        SwpHts aArr;
        while (nAttrCnt < m_pSwpHints->Count())
        {
            SwTextAttr * const pHt = m_pSwpHints->Get(nAttrCnt);
            if (nEnd != pHt->GetStart())
                break;
            const sal_Int32 * const pEndIdx = pHt->GetEnd();
            if (pEndIdx && *pEndIdx == nEnd)
            {
                aArr.push_back( pHt );
                m_pSwpHints->Delete( pHt );
            }
            else
            {
                ++nAttrCnt;
            }
        }
        Update(rStart, nLen, UpdateMode::Negative|UpdateMode::Delete);

        for (SwTextAttr* pHt : aArr)
        {
            pHt->SetStart( rStart.GetIndex() );
            pHt->SetEnd( rStart.GetIndex() );
            InsertHint( pHt );
        }
    }
    else
    {
        Update(rStart, nLen, UpdateMode::Negative|UpdateMode::Delete);
    }

    // set after moving hints
    m_Text = newText;

    if (bMergePortionsNeeded)
    {
        m_pSwpHints->MergePortions(*this);
    }

    CHECK_SWPHINTS(this);

    TryDeleteSwpHints();
}

void SwTextNode::EraseText(const SwPosition &rIdx, const sal_Int32 nCount,
        const SwInsertFlags nMode )
{
    EraseText(rIdx.nContent, nCount, nMode);
}

void SwTextNode::EraseText(const SwContentIndex &rIdx, const sal_Int32 nCount,
        const SwInsertFlags nMode )
{
    assert(rIdx.GetContentNode() == this);
    assert(rIdx <= m_Text.getLength()); // invalid index

    const sal_Int32 nStartIdx = rIdx.GetIndex();
    const sal_Int32 nCnt = (nCount==SAL_MAX_INT32)
                      ? m_Text.getLength() - nStartIdx : nCount;
    const sal_Int32 nEndIdx = nStartIdx + nCnt;
    if (nEndIdx <= m_Text.getLength())
        m_Text = m_Text.replaceAt(nStartIdx, nCnt, u"");

    // GCAttr(); don't remove all empty ones, just the ones that are in the
    // range but not at the end of the range.

    for ( sal_Int32 i = 0; m_pSwpHints && i < static_cast<sal_Int32>(m_pSwpHints->Count()); ++i )
    {
        SwTextAttr *pHt = m_pSwpHints->Get(i);

        const sal_Int32 nHintStart = pHt->GetStart();

        if ( nHintStart < nStartIdx )
            continue;

        if ( nHintStart > nEndIdx )
            break; // hints are sorted by end, so break here

        const sal_Int32* pHtEndIdx = pHt->GetEnd();
        const sal_uInt16 nWhich = pHt->Which();

        if( !pHtEndIdx )
        {
                    // attribute with neither end nor CH_TXTATR?
            assert(pHt->HasDummyChar());
            if (isTXTATR(nWhich) && (nHintStart < nEndIdx))
            {
                m_pSwpHints->DeleteAtPos(i);
                DestroyAttr( pHt );
                --i;
            }
            continue;
        }

        assert(!( (nHintStart < nEndIdx) && (*pHtEndIdx > nEndIdx)
                    && pHt->HasDummyChar() )
                // next line: deleting exactly dummy char: DeleteAttributes
                || ((nHintStart == nStartIdx) && (nHintStart + 1 == nEndIdx)));
                // "ERROR: deleting left-overlapped attribute with CH_TXTATR");

        // Delete the hint if:
        // 1. The hint ends before the deletion end position or
        // 2. The hint ends at the deletion end position and
        //    we are not in empty expand mode and
        //    the hint is a [toxmark|refmark|ruby|inputfield] text attribute
        // 3. deleting exactly the dummy char of an hint with end and dummy
        //    char deletes the hint
        if (   (*pHtEndIdx < nEndIdx)
            || ( (*pHtEndIdx == nEndIdx)     &&
                 !(SwInsertFlags::EMPTYEXPAND & nMode)  &&
                 (  (RES_TXTATR_TOXMARK == nWhich)  ||
                    (RES_TXTATR_REFMARK == nWhich)  ||
                    (RES_TXTATR_CJK_RUBY == nWhich) ||
                    (RES_TXTATR_INPUTFIELD == nWhich) ) )
            || ( (nHintStart < nEndIdx)     &&
                 pHt->HasDummyChar()        )
           )
        {
            m_pSwpHints->DeleteAtPos(i);
            DestroyAttr( pHt );
            --i;
        }
    }

    OSL_ENSURE(rIdx.GetIndex() == nStartIdx, "huh? start index has changed?");

    TryDeleteSwpHints();

    Update(rIdx, nCnt, UpdateMode::Negative);

    if(1 == nCnt)
    {
        const auto aHint = sw::DeleteChar(nStartIdx);
        CallSwClientNotify(aHint);
    } else {
        const auto aHint = sw::DeleteText(nStartIdx, nCnt);
        CallSwClientNotify(aHint);
    }

    OSL_ENSURE(rIdx.GetIndex() == nStartIdx, "huh? start index has changed?");

    // By deleting a character, the hidden flags
    // at the TextNode can become invalid:
    SetCalcHiddenCharFlags();

    CHECK_SWPHINTS(this);
}

void SwTextNode::GCAttr()
{
    if ( !HasHints() )
        return;

    bool   bChanged = false;
    sal_Int32 nMin = m_Text.getLength();
    sal_Int32 nMax = 0;
    const bool bAll = nMin != 0; // on empty paragraphs only remove INetFormats

    for ( size_t i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
    {
        SwTextAttr * const pHt = m_pSwpHints->Get(i);

        // if end and start are equal, delete it
        const sal_Int32 * const pEndIdx = pHt->GetEnd();
        if (pEndIdx && !pHt->HasDummyChar() && (*pEndIdx == pHt->GetStart())
            && ( bAll || pHt->Which() == RES_TXTATR_INETFMT ) )
        {
            bChanged = true;
            nMin = std::min( nMin, pHt->GetStart() );
            nMax = std::max( nMax, *pHt->GetEnd() );
            DestroyAttr( m_pSwpHints->Cut(i) );
            --i;
        }
        else
        {
            pHt->SetDontExpand( false );
        }
    }
    TryDeleteSwpHints();

    if(bChanged)
    {
        // textframes react to aHint, others to aNew
        SwUpdateAttr aHint(
            nMin,
            nMax,
            0);

        CallSwClientNotify(sw::UpdateAttrHint(nullptr, &aHint));
        CallSwClientNotify(SwFormatChangeHint(nullptr, GetTextColl()));
    }
}

SwNumRule* SwTextNode::GetNumRule(bool bInParent) const
{
    SwNumRule* pRet = nullptr;

    const SfxPoolItem* pItem = GetNoCondAttr( RES_PARATR_NUMRULE, bInParent );
    bool bNoNumRule = false;
    if ( pItem )
    {
        UIName sNumRuleName =
            static_cast<const SwNumRuleItem *>(pItem)->GetValue();
        if (!sNumRuleName.isEmpty())
        {
            pRet = GetDoc().FindNumRulePtr( sNumRuleName );
        }
        else // numbering is turned off
            bNoNumRule = true;
    }

    if ( !bNoNumRule )
    {
        if ( pRet && pRet == GetDoc().GetOutlineNumRule() &&
             ( !HasSwAttrSet() ||
               SfxItemState::SET !=
                GetpSwAttrSet()->GetItemState( RES_PARATR_NUMRULE, false ) ) )
        {
            SwTextFormatColl* pColl = GetTextColl();
            if ( pColl )
            {
                const SwNumRuleItem& rDirectItem = pColl->GetNumRule( false );
                if ( rDirectItem.GetValue().isEmpty() )
                {
                    pRet = nullptr;
                }
            }
        }
    }

    return pRet;
}

void SwTextNode::NumRuleChgd()
{
    if ( IsInList() )
    {
        SwNumRule* pNumRule = GetNumRule();
        if ( pNumRule && pNumRule != GetNum()->GetNumRule() )
        {
            mpNodeNum->ChangeNumRule( *pNumRule );
            if (mpNodeNumRLHidden)
            {
                mpNodeNumRLHidden->ChangeNumRule(*pNumRule);
            }
        }
    }

    // Sending "noop" modify in order to cause invalidations of registered
    // <SwTextFrame> instances to get the list style change respectively the change
    // in the list tree reflected in the layout.
    // Important note:
    {
        SvxTextLeftMarginItem & rLR = const_cast<SvxTextLeftMarginItem&>(GetSwAttrSet().GetTextLeftMargin());
        CallSwClientNotify(sw::LegacyModifyHint(&rLR, &rLR));
    }

    SetWordCountDirty( true );
}

// -> #i27615#
bool SwTextNode::IsNumbered(SwRootFrame const*const pLayout) const
{
    SwNumRule* pRule = GetNum(pLayout) ? GetNum(pLayout)->GetNumRule() : nullptr;
    return pRule && IsCountedInList();
}

bool SwTextNode::HasMarkedLabel() const
{
    bool bResult = false;

    if ( IsInList() )
    {
        SwList* pList = GetDoc().getIDocumentListsAccess().getListByName(GetListId());
        if (pList)
            bResult = pList->IsListLevelMarked(GetActualListLevel());
    }

    return bResult;
}
// <- #i27615#

namespace
{
/// Decides if a list level direct formatting on a paragraph needs copying to a next, new paragraph.
bool CopyDirectListLevel(const SwTextNode* pTextNode)
{
    SwTextFormatColl* pColl = pTextNode->GetTextColl();
    if (!pColl)
    {
        // No style, so can't have a conflict with a direct formatting.
        return false;
    }

    if (&pColl->GetNextTextFormatColl() != pColl)
    {
        // Style has a custom follow style, changing list level is OK.
        return false;
    }

    if (!pColl->IsAssignedToListLevelOfOutlineStyle())
    {
        // Paragraph style has no own list level, no conflict.
        return false;
    }

    // Copy is needed if the old paragraph had a direct formatting, which may be different and has
    // to be kept during the paragraph split.
    return pTextNode->HasAttrListLevel();
}
}

SwTextNode* SwTextNode::MakeNewTextNode( const SwNode& rPosNd, bool bNext,
                                       bool bChgFollow )
{
    // ignore hard PageBreak/PageDesc/ColumnBreak from Auto-Set
    std::optional<SwAttrSet> oNewAttrSet;
    // #i75353#
    bool bClearHardSetNumRuleWhenFormatCollChanges( false );
    if( HasSwAttrSet() )
    {
        oNewAttrSet.emplace( *GetpSwAttrSet() );
        const SfxItemSet* pTmpSet = GetpSwAttrSet();

        if (bNext)     // successor doesn't inherit breaks!
            pTmpSet = &*oNewAttrSet;

        // !bNext: remove PageBreaks/PageDesc/ColBreak from this
        bool bRemoveFromCache = false;
        std::vector<sal_uInt16> aClearWhichIds;
        if ( bNext )
            bRemoveFromCache = ( 0 != oNewAttrSet->ClearItem( RES_PAGEDESC ) );
        else
            aClearWhichIds.push_back( RES_PAGEDESC );

        if( SfxItemState::SET == pTmpSet->GetItemState( RES_BREAK, false ) )
        {
            if ( bNext )
                oNewAttrSet->ClearItem( RES_BREAK );
            else
                aClearWhichIds.push_back( RES_BREAK );
            bRemoveFromCache = true;
        }
        if( SfxItemState::SET == pTmpSet->GetItemState( RES_KEEP, false ) )
        {
            if ( bNext )
                oNewAttrSet->ClearItem( RES_KEEP );
            else
                aClearWhichIds.push_back( RES_KEEP );
            bRemoveFromCache = true;
        }
        if( SfxItemState::SET == pTmpSet->GetItemState( RES_PARATR_SPLIT, false ) )
        {
            if ( bNext )
                oNewAttrSet->ClearItem( RES_PARATR_SPLIT );
            else
                aClearWhichIds.push_back( RES_PARATR_SPLIT );
            bRemoveFromCache = true;
        }
        if(SfxItemState::SET == pTmpSet->GetItemState(RES_PARATR_NUMRULE, false))
        {
            SwNumRule * pRule = GetNumRule();

            if (pRule && IsOutline())
            {
                if ( bNext )
                    oNewAttrSet->ClearItem(RES_PARATR_NUMRULE);
                else
                {
                    // #i75353#
                    // No clear of hard set numbering rule at an outline paragraph at this point.
                    // Only if the paragraph style changes - see below.
                    bClearHardSetNumRuleWhenFormatCollChanges = true;
                }
                bRemoveFromCache = true;
            }
        }

        if ( !aClearWhichIds.empty() )
            bRemoveFromCache = 0 != ClearItemsFromAttrSet( aClearWhichIds );

        if( !bNext && bRemoveFromCache )
        {
            InvalidateInSwCache();
        }
    }
    SwNodes& rNds = GetNodes();

    SwTextFormatColl* pColl = GetTextColl();

    SwTextNode *pNode = new SwTextNode( rPosNd, pColl, oNewAttrSet ? &*oNewAttrSet : nullptr );

    oNewAttrSet.reset();

    const SwNumRule* pRule = GetNumRule();
    if( pRule && pRule == pNode->GetNumRule() && rNds.IsDocNodes() )
    {
        // #i55459#
        // - correction: parameter <bNext> has to be checked, as it was in the
        //   previous implementation.
        if ( !bNext && !IsCountedInList() )
            SetCountedInList(true);
    }

    // In case the numbering caused a style from the pool to be assigned to
    // the new node, don't overwrite that here!
    if( pColl != pNode->GetTextColl() ||
        ( bChgFollow && pColl != GetTextColl() ))
        return pNode;       // that ought to be enough?

    bool bSetListLevel = !CopyDirectListLevel(this);

    pNode->ChgTextCollUpdateNum( nullptr, pColl, bSetListLevel ); // for numbering/outline
    if( bNext || !bChgFollow )
        return pNode;

    SwTextFormatColl *pNextColl = &pColl->GetNextTextFormatColl();
    // i#101870 perform action on different paragraph styles before applying
    // the new paragraph style
    if (pNextColl != pColl)
    {
        // #i75353#
        if ( bClearHardSetNumRuleWhenFormatCollChanges )
        {
            if ( ClearItemsFromAttrSet( { RES_PARATR_NUMRULE } ) != 0 )
            {
                InvalidateInSwCache();
            }
        }
    }
    ChgFormatColl( pNextColl, bSetListLevel );

    return pNode;
}

SwContentNode* SwTextNode::AppendNode( const SwPosition & rPos )
{
    // position behind which it will be inserted
    SwTextNode* pNew = MakeNewTextNode( *rPos.GetNodes()[rPos.GetNodeIndex() + 1] );

    // reset list attributes at appended text node
    pNew->ResetAttr( RES_PARATR_LIST_ISRESTART );
    pNew->ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
    pNew->ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    if ( pNew->GetNumRule() == nullptr )
    {
        pNew->ResetAttr( RES_PARATR_LIST_ID );
        pNew->ResetAttr( RES_PARATR_LIST_LEVEL );
    }

    if (!IsInList() && GetNumRule() && !GetListId().isEmpty())
    {
        AddToList();
    }

    if( HasWriterListeners() )
        MakeFramesForAdjacentContentNode(*pNew);
    return pNew;
}

SwTextAttr * SwTextNode::GetTextAttrForCharAt(
    const sal_Int32 nIndex,
    const sal_uInt16 nWhich ) const
{
    assert(nWhich >= RES_TXTATR_BEGIN && nWhich <= RES_TXTATR_END);
    if ( HasHints() )
    {
        for ( size_t i = 0; i < m_pSwpHints->Count(); ++i )
        {
            SwTextAttr * const pHint = m_pSwpHints->Get(i);
            const sal_Int32 nStartPos = pHint->GetStart();
            if ( nIndex < nStartPos )
            {
                return nullptr;
            }
            if ( (nIndex == nStartPos) && pHint->HasDummyChar() )
            {
                return ( RES_TXTATR_END == nWhich || nWhich == pHint->Which() )
                       ? pHint : nullptr;
            }
        }
    }
    return nullptr;
}

SwTextAttr* SwTextNode::GetTextAttrForEndCharAt(sal_Int32 nIndex, sal_uInt16 nWhich) const
{
    SwTextAttr* pAttr = GetTextAttrAt(nIndex, nWhich, ::sw::GetTextAttrMode::Expand);
    if (!pAttr)
    {
        return nullptr;
    }

    if (!pAttr->End())
    {
        return nullptr;
    }

    // The start-end range covers the end dummy character.
    if (*pAttr->End() - 1 != nIndex)
    {
        return nullptr;
    }

    return pAttr;
}

namespace
{

sal_uInt16 lcl_BoundListLevel(const int nActualLevel)
{
    return o3tl::narrowing<sal_uInt16>( std::clamp( nActualLevel, 0, MAXLEVEL-1 ) );
}

}

// -> #i29560#
bool SwTextNode::HasNumber(SwRootFrame const*const pLayout) const
{
    bool bResult = false;

    const SwNumRule *const pRule = GetNum(pLayout) ? GetNum(pLayout)->GetNumRule() : nullptr;
    if ( pRule )
    {
        const SwNumFormat& aFormat(pRule->Get(lcl_BoundListLevel(GetActualListLevel())));

        // #i40041#
        bResult = aFormat.IsEnumeration();
    }

    return bResult;
}

bool SwTextNode::HasBullet() const
{
    bool bResult = false;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pRule )
    {
        const SwNumFormat& aFormat(pRule->Get(lcl_BoundListLevel(GetActualListLevel())));

        bResult = aFormat.IsItemize();
    }

    return bResult;
}
// <- #i29560#

// #128041# - introduce parameter <_bInclPrefixAndSuffixStrings>
//i53420 added max outline parameter
OUString SwTextNode::GetNumString( const bool _bInclPrefixAndSuffixStrings,
        const unsigned int _nRestrictToThisLevel,
        SwRootFrame const*const pLayout, SwListRedlineType eRedline) const
{
    if (GetDoc().IsClipBoard() && m_oNumStringCache)
    {
        // #i111677# do not expand number strings in clipboard documents
        return *m_oNumStringCache;
    }
    const SwNumRule* pRule = GetNum(pLayout, eRedline) ? GetNum(pLayout, eRedline)->GetNumRule() : nullptr;
    if ( pRule &&
         IsCountedInList() )
    {
        SvxNumberType const& rNumberType(
                pRule->Get( lcl_BoundListLevel(GetActualListLevel(eRedline)) ) );
        if (rNumberType.IsTextFormat() ||

            (style::NumberingType::NUMBER_NONE == rNumberType.GetNumberingType()))
        {
            return pRule->MakeNumString( GetNum(pLayout, eRedline)->GetNumberVector(),
                                     _bInclPrefixAndSuffixStrings,
                                     _nRestrictToThisLevel,
                                     false,
                                     nullptr,
                                     GetLang(0));
        }
    }

    return OUString();
}

tools::Long SwTextNode::GetLeftMarginWithNum( bool bTextLeft ) const
{
    tools::Long nRet = 0;
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if( pRule )
    {
        const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));

        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            nRet = rFormat.GetAbsLSpace();

            if( !bTextLeft )
            {
                if( 0 > rFormat.GetFirstLineOffset() &&
                    nRet > -rFormat.GetFirstLineOffset() )
                    nRet = nRet + rFormat.GetFirstLineOffset();
                else
                    nRet = 0;
            }

            if( pRule->IsAbsSpaces() )
            {
                SvxFirstLineIndentItem const& rFirst(GetSwAttrSet().GetFirstLineIndent());
                nRet
                    = nRet - GetSwAttrSet().GetTextLeftMargin().ResolveLeft(rFirst, /*metrics*/ {});
            }
        }
        else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            ::sw::ListLevelIndents const indents(AreListLevelIndentsApplicable());
            // note: the result is *always* added to either the left-margin
            // or the text-left-margin of the node itself by the caller.
            // so first, subtract what the caller has computed anyway,
            // and then add the value according to combination of
            // list/paragraph items. (this is rather inelegant)
            SvxFirstLineIndentItem firstLine(GetSwAttrSet().GetFirstLineIndent());
            SvxTextLeftMarginItem leftMargin(GetSwAttrSet().GetTextLeftMargin());
            nRet = bTextLeft ? -leftMargin.ResolveTextLeft(/*metrics*/ {})
                             : -leftMargin.ResolveLeft(firstLine, /*metrics*/ {});
            if (indents & ::sw::ListLevelIndents::LeftMargin)
            {
                leftMargin.SetTextLeft(SvxIndentValue::twips(rFormat.GetIndentAt()));
            }
            if (indents & ::sw::ListLevelIndents::FirstLine)
            {
                firstLine.SetTextFirstLineOffset(
                    SvxIndentValue{ static_cast<double>(rFormat.GetFirstLineIndent()),
                                    rFormat.GetFirstLineIndentUnit() });
            }
            nRet += bTextLeft ? leftMargin.ResolveTextLeft(/*metrics*/ {})
                              : leftMargin.ResolveLeft(firstLine, /*metrics*/ {});
        }
    }

    return nRet;
}

bool SwTextNode::GetFirstLineOfsWithNum(short& rFLOffset,
                                        const SvxFontUnitMetrics& rMetrics) const
{
    // #i95907#
    rFLOffset = 0;

    // #i51089#
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pRule )
    {
        if ( IsCountedInList() )
        {
            const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));
            if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                rFLOffset = rFormat.GetFirstLineOffset(); //TODO: overflow

                if (!getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxFirstLineIndentItem const aItem(GetSwAttrSet().GetFirstLineIndent());
                    rFLOffset = rFLOffset + aItem.ResolveTextFirstLineOffset(rMetrics);
                }
            }
            else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                if (AreListLevelIndentsApplicable() & ::sw::ListLevelIndents::FirstLine)
                {
                    rFLOffset = rFormat.GetFirstLineIndent();
                }
                else if (!getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxFirstLineIndentItem const aItem(GetSwAttrSet().GetFirstLineIndent());
                    rFLOffset = aItem.ResolveTextFirstLineOffset(rMetrics);
                }
            }
        }

        return true;
    }

    rFLOffset = GetSwAttrSet().GetFirstLineIndent().ResolveTextFirstLineOffset(rMetrics);
    return false;
}

SwTwips SwTextNode::GetAdditionalIndentForStartingNewList() const
{
    SwTwips nAdditionalIndent = 0;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pRule )
    {
        const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
        {
            SvxFirstLineIndentItem const& rFirst(GetSwAttrSet().GetFirstLineIndent());

            nAdditionalIndent
                = GetSwAttrSet().GetTextLeftMargin().ResolveLeft(rFirst, /*metrics*/ {});

            if (getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
            {
                nAdditionalIndent
                    = nAdditionalIndent
                      - GetSwAttrSet().GetFirstLineIndent().ResolveTextFirstLineOffset({});
            }
        }
        else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            // note: there was an apparent bug here, list GetIndentAt()
            // was interpreted as left-margin not text-left-margin unlike every
            // other use of it.
            ::sw::ListLevelIndents const indents(AreListLevelIndentsApplicable());
            SvxFirstLineIndentItem const aFirst(
                indents & ::sw::ListLevelIndents::FirstLine
                    ? SvxFirstLineIndentItem(
                          SvxIndentValue{ static_cast<double>(rFormat.GetFirstLineIndent()),
                                          rFormat.GetFirstLineIndentUnit() },
                          RES_MARGIN_FIRSTLINE)
                    : GetSwAttrSet().GetFirstLineIndent());
            SvxTextLeftMarginItem const aLeft(
                indents & ::sw::ListLevelIndents::LeftMargin
                    ? SvxTextLeftMarginItem(SvxIndentValue::twips(rFormat.GetIndentAt()),
                                            RES_MARGIN_TEXTLEFT)
                    : GetSwAttrSet().GetTextLeftMargin());
            nAdditionalIndent = aLeft.ResolveLeft(aFirst, /*metrics*/ {});
            if (!(indents & ::sw::ListLevelIndents::FirstLine))
            {
                if (getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    nAdditionalIndent = nAdditionalIndent - aFirst.ResolveTextFirstLineOffset({});
                }
            }
        }
    }
    else
    {
        SvxFirstLineIndentItem const& rFirst(GetSwAttrSet().GetFirstLineIndent());
        nAdditionalIndent = GetSwAttrSet().GetTextLeftMargin().ResolveLeft(rFirst, /*metrics*/ {});
    }

    return nAdditionalIndent;
}

// #i91133#
tools::Long SwTextNode::GetLeftMarginForTabCalculation() const
{
    tools::Long nLeftMarginForTabCalc = 0;

    bool bLeftMarginForTabCalcSetToListLevelIndent( false );
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if( pRule )
    {
        const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if (AreListLevelIndentsApplicable() & ::sw::ListLevelIndents::LeftMargin)
            {
                nLeftMarginForTabCalc = rFormat.GetIndentAt();
                bLeftMarginForTabCalcSetToListLevelIndent = true;
            }
        }
    }
    if ( !bLeftMarginForTabCalcSetToListLevelIndent )
    {
        nLeftMarginForTabCalc = GetSwAttrSet().GetTextLeftMargin().ResolveTextLeft({});
    }

    return nLeftMarginForTabCalc;
}

static void Replace0xFF(
    SwTextNode const& rNode,
    OUStringBuffer & rText,
    sal_Int32 & rTextStt,
    sal_Int32 nEndPos )
{
    if (!rNode.GetpSwpHints())
        return;

    sal_Unicode cSrchChr = CH_TXTATR_BREAKWORD;
    for( int nSrchIter = 0; 2 > nSrchIter; ++nSrchIter, cSrchChr = CH_TXTATR_INWORD )
    {
        sal_Int32 nPos = rText.indexOf(cSrchChr);
        while (-1 != nPos && nPos < nEndPos)
        {
            const SwTextAttr* const pAttr =
                rNode.GetTextAttrForCharAt(rTextStt + nPos);
            if( pAttr )
            {
                switch( pAttr->Which() )
                {
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                    rText.remove(nPos, 1);
                    ++rTextStt;
                    break;

                case RES_TXTATR_FTN:
                    rText.remove(nPos, 1);
                    ++rTextStt;
                    break;

                default:
                    rText.remove(nPos, 1);
                    ++rTextStt;
                }
            }
            else
            {
                ++nPos;
                ++nEndPos;
            }
            nPos = rText.indexOf(cSrchChr, nPos);
        }
    }
}

// Expand fields
// #i83479# - handling of new parameters
OUString SwTextNode::GetExpandText(SwRootFrame const*const pLayout,
                                   const sal_Int32 nIdx,
                                   const sal_Int32 nLen,
                                   const bool bWithNum,
                                   const bool bAddSpaceAfterListLabelStr,
                                   const bool bWithSpacesForLevel,
                                   const ExpandMode eAdditionalMode) const

{
    ExpandMode eMode = ExpandMode::ExpandFields | eAdditionalMode;
    if (pLayout && pLayout->IsHideRedlines())
    {
        eMode |= ExpandMode::HideDeletions;
    }

    ModelToViewHelper aConversionMap(*this, pLayout, eMode);
    const OUString& aExpandText = aConversionMap.getViewText();
    const sal_Int32 nExpandBegin = aConversionMap.ConvertToViewPosition( nIdx );
    sal_Int32 nEnd = nLen == -1 ? GetText().getLength() : nIdx + nLen;
    const sal_Int32 nExpandEnd = aConversionMap.ConvertToViewPosition( nEnd );
    OUStringBuffer aText(aExpandText.subView(nExpandBegin, nExpandEnd-nExpandBegin));

    // remove dummy characters of Input Fields
    comphelper::string::remove(aText, CH_TXT_ATR_INPUTFIELDSTART);
    comphelper::string::remove(aText, CH_TXT_ATR_INPUTFIELDEND);
    comphelper::string::remove(aText, CH_TXTATR_BREAKWORD);

    if( bWithNum )
    {
        if (!GetNumString(true, MAXLEVEL, pLayout).isEmpty())
        {
            if ( bAddSpaceAfterListLabelStr )
            {
                const sal_Unicode aSpace = ' ';
                aText.insert(0, aSpace);
            }
            aText.insert(0, GetNumString(true, MAXLEVEL, pLayout));
        }
    }

    if (bWithSpacesForLevel)
    {
        const sal_Unicode aSpace = ' ';
        for (int nLevel = GetActualListLevel(); nLevel > 0; --nLevel)
        {
            aText.insert(0, aSpace);
            aText.insert(0, aSpace);
        }
    }

    return aText.makeStringAndClear();
}

bool SwTextNode::CopyExpandText(SwTextNode& rDestNd, const SwContentIndex* pDestIdx,
                        sal_Int32 nIdx, sal_Int32 nLen,
                        SwRootFrame const*const pLayout,
                        bool bWithFootnote, bool bReplaceTabsWithSpaces ) const
{
    if( &rDestNd == this )
        return false;
    assert(!pDestIdx || pDestIdx->GetContentNode() == &rDestNd);

    SwContentIndex aDestIdx(&rDestNd, rDestNd.GetText().getLength());
    if( pDestIdx )
        aDestIdx = *pDestIdx;
    const sal_Int32 nDestStt = aDestIdx.GetIndex();

    // first, start with the text
    OUStringBuffer buf(GetText());
    if( bReplaceTabsWithSpaces )
        buf.replace('\t', ' ');

    // mask hidden characters
    const sal_Unicode cChar = CH_TXTATR_BREAKWORD;
    SwScriptInfo::MaskHiddenRanges(*this, buf, 0, buf.getLength(), cChar);

    buf.remove(0, nIdx);
    if (nLen != -1)
    {
        buf.truncate(nLen);
    }
    // remove dummy characters of Input Fields
    {
        comphelper::string::remove(buf, CH_TXT_ATR_INPUTFIELDSTART);
        comphelper::string::remove(buf, CH_TXT_ATR_INPUTFIELDEND);
    }
    rDestNd.InsertText(buf.makeStringAndClear(), aDestIdx);
    nLen = aDestIdx.GetIndex() - nDestStt;

    // set all char attributes with Symbol font
    if ( HasHints() )
    {
        sal_Int32 nInsPos = nDestStt - nIdx;
        for ( size_t i = 0; i < m_pSwpHints->Count(); ++i )
        {
            const SwTextAttr* pHt = m_pSwpHints->Get(i);
            const sal_Int32 nAttrStartIdx = pHt->GetStart();
            const sal_uInt16 nWhich = pHt->Which();
            if (nIdx + nLen <= nAttrStartIdx)
                break;      // behind end of text

            const sal_Int32 *pEndIdx = pHt->End();
            if( pEndIdx && *pEndIdx > nIdx &&
                ( RES_CHRATR_FONT == nWhich ||
                  RES_TXTATR_CHARFMT == nWhich ||
                  RES_TXTATR_AUTOFMT == nWhich ))
            {
                const SvxFontItem* const pFont =
                    CharFormat::GetItem( *pHt, RES_CHRATR_FONT );
                if ( pFont && RTL_TEXTENCODING_SYMBOL == pFont->GetCharSet() )
                {
                    // attribute in area => copy
                    rDestNd.InsertItem( *const_cast<SvxFontItem*>(pFont),
                            nInsPos + nAttrStartIdx, nInsPos + *pEndIdx );
                }
            }
            else if ( pHt->HasDummyChar() && (nAttrStartIdx >= nIdx) )
            {
                aDestIdx = nInsPos + nAttrStartIdx;
                switch( nWhich )
                {
                case RES_TXTATR_FIELD:
                case RES_TXTATR_ANNOTATION:
                    {
                        OUString const aExpand(
                            static_txtattr_cast<SwTextField const*>(pHt)->GetFormatField().GetField()->ExpandField(true, pLayout));
                        if (!aExpand.isEmpty())
                        {
                            ++aDestIdx;     // insert behind
                            OUString const ins(
                                rDestNd.InsertText( aExpand, aDestIdx));
                            SAL_INFO_IF(ins.getLength() != aExpand.getLength(),
                                    "sw.core", "GetExpandText lossage");
                            aDestIdx = nInsPos + nAttrStartIdx;
                            nInsPos += ins.getLength();
                        }
                        rDestNd.EraseText( aDestIdx, 1 );
                        --nInsPos;
                    }
                    break;

                case RES_TXTATR_FTN:
                    {
                        if ( bWithFootnote )
                        {
                            const SwFormatFootnote& rFootnote = pHt->GetFootnote();
                            OUString sExpand;
                            auto const number(pLayout && pLayout->IsHideRedlines()
                                    ? rFootnote.GetNumberRLHidden()
                                    : rFootnote.GetNumber());
                            if( !rFootnote.GetNumStr().isEmpty() )
                                sExpand = rFootnote.GetNumStr();
                            else if( rFootnote.IsEndNote() )
                                sExpand = GetDoc().GetEndNoteInfo().m_aFormat.
                                            GetNumStr(number);
                            else
                                sExpand = GetDoc().GetFootnoteInfo().m_aFormat.
                                            GetNumStr(number);
                            if( !sExpand.isEmpty() )
                            {
                                ++aDestIdx;     // insert behind
                                SvxEscapementItem aItem( SvxEscapement::Superscript, RES_CHRATR_ESCAPEMENT );
                                rDestNd.InsertItem(
                                        aItem,
                                        aDestIdx.GetIndex(),
                                        aDestIdx.GetIndex() );
                                OUString const ins( rDestNd.InsertText(sExpand, aDestIdx, SwInsertFlags::EMPTYEXPAND));
                                SAL_INFO_IF(ins.getLength() != sExpand.getLength(),
                                        "sw.core", "GetExpandText lossage");
                                aDestIdx = nInsPos + nAttrStartIdx;
                                nInsPos += ins.getLength();
                            }
                        }
                        rDestNd.EraseText( aDestIdx, 1 );
                        --nInsPos;
                    }
                    break;

                default:
                    rDestNd.EraseText( aDestIdx, 1 );
                    --nInsPos;
                }
            }
        }
    }

    aDestIdx = 0;
    sal_Int32 nStartDelete(-1);
    while (aDestIdx < rDestNd.GetText().getLength())
    {
        sal_Unicode const cur(rDestNd.GetText()[aDestIdx.GetIndex()]);
        if (   (cChar == cur) // filter substituted hidden text
            || (CH_TXT_ATR_FIELDSTART  == cur) // filter all fieldmarks
            || (CH_TXT_ATR_FIELDSEP    == cur)
            || (CH_TXT_ATR_FIELDEND    == cur)
            || (CH_TXT_ATR_FORMELEMENT == cur))
        {
            if (-1 == nStartDelete)
            {
                nStartDelete = aDestIdx.GetIndex(); // start deletion range
            }
            ++aDestIdx;
            if (aDestIdx < rDestNd.GetText().getLength())
            {
                continue;
            } // else: end of paragraph => delete, see below
        }
        else
        {
            if (-1 == nStartDelete)
            {
                ++aDestIdx;
                continue;
            } // else: delete, see below
        }
        assert(-1 != nStartDelete); // without delete range, would have continued
        rDestNd.EraseText(
            SwContentIndex(&rDestNd, nStartDelete),
            aDestIdx.GetIndex() - nStartDelete);
        assert(aDestIdx.GetIndex() == nStartDelete);
        nStartDelete = -1; // reset
    }

    return true;
}

OUString SwTextNode::GetRedlineText() const
{
    std::vector<sal_Int32> aRedlArr;
    const SwDoc& rDoc = GetDoc();
    SwRedlineTable::size_type nRedlPos = rDoc.getIDocumentRedlineAccess().GetRedlinePos( *this, RedlineType::Delete );
    if( SwRedlineTable::npos != nRedlPos )
    {
        // some redline-delete object exists for the node
        const SwNodeOffset nNdIdx = GetIndex();
        for( ; nRedlPos < rDoc.getIDocumentRedlineAccess().GetRedlineTable().size() ; ++nRedlPos )
        {
            const SwRangeRedline* pTmp = rDoc.getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
            if( RedlineType::Delete == pTmp->GetType() )
            {
                const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                if( pRStt->GetNodeIndex() < nNdIdx )
                {
                    if( pREnd->GetNodeIndex() > nNdIdx )
                        // paragraph is fully deleted
                        return OUString();
                    else if( pREnd->GetNodeIndex() == nNdIdx )
                    {
                        // deleted from 0 to nContent
                        aRedlArr.push_back( 0 );
                        aRedlArr.push_back( pREnd->GetContentIndex() );
                    }
                }
                else if( pRStt->GetNodeIndex() == nNdIdx )
                {
                    //aRedlArr.Insert( pRStt->GetContentIndex(), aRedlArr.Count() );
                    aRedlArr.push_back( pRStt->GetContentIndex() );
                    if( pREnd->GetNodeIndex() == nNdIdx )
                        aRedlArr.push_back( pREnd->GetContentIndex() );
                    else
                    {
                        aRedlArr.push_back(GetText().getLength());
                        break;  // that was all
                    }
                }
                else
                    break;      // that was all
            }
        }
    }

    OUStringBuffer aText(GetText());

    sal_Int32 nTextStt = 0;
    sal_Int32 nIdxEnd = aText.getLength();
    for( size_t n = 0; n < aRedlArr.size(); n += 2 )
    {
        sal_Int32 nStt = aRedlArr[ n ];
        sal_Int32 nEnd = aRedlArr[ n+1 ];
        if( ( 0 <= nStt && nStt <= nIdxEnd ) ||
            ( 0 <= nEnd && nEnd <= nIdxEnd ))
        {
            if( nStt < 0 ) nStt = 0;
            if( nIdxEnd < nEnd ) nEnd = nIdxEnd;
            const sal_Int32 nDelCnt = nEnd - nStt;
            aText.remove(nStt - nTextStt, nDelCnt);
            Replace0xFF(*this, aText, nTextStt, nStt - nTextStt);
            nTextStt += nDelCnt;
        }
        else if( nStt >= nIdxEnd )
            break;
    }
    Replace0xFF(*this, aText, nTextStt, aText.getLength());

    return aText.makeStringAndClear();
}

void SwTextNode::ReplaceText( const SwContentIndex& rStart, const sal_Int32 nDelLen,
                             const OUString & rStr)
{
    assert(rStart.GetContentNode() == this);
    assert( rStart.GetIndex() < m_Text.getLength()     // index out of bounds
         && rStart.GetIndex() + nDelLen <= m_Text.getLength());

    sal_Int32 const nOverflow(rStr.getLength() - nDelLen - GetSpaceLeft());
    SAL_WARN_IF(nOverflow > 0, "sw.core",
            "SwTextNode::ReplaceText: node text with insertion > node capacity.");
    OUString const sInserted(
        (nOverflow > 0) ? rStr.copy(0, rStr.getLength() - nOverflow) : rStr);
    if (sInserted.isEmpty() && 0 == nDelLen)
    {
        return; // nothing to do
    }

    const sal_Int32 nStartPos = rStart.GetIndex();
    sal_Int32 nEndPos = nStartPos + nDelLen;
    sal_Int32 nLen = nDelLen;
    for( sal_Int32 nPos = nStartPos; nPos < nEndPos; ++nPos )
    {
        if ((CH_TXTATR_BREAKWORD == m_Text[nPos]) ||
            (CH_TXTATR_INWORD    == m_Text[nPos]))
        {
            SwTextAttr *const pHint = GetTextAttrForCharAt( nPos );
            if (pHint)
            {
                assert(!( pHint->GetEnd() && pHint->HasDummyChar()
                            && (pHint->GetStart() < nEndPos)
                            && (*pHint->GetEnd()   > nEndPos) ));
                    // "deleting left-overlapped attribute with CH_TXTATR"
                DeleteAttribute( pHint );
                --nEndPos;
                --nLen;
            }
        }
    }

    bool bOldExpFlg = IsIgnoreDontExpand();
    SetIgnoreDontExpand( true );

    const sal_Int32 nInsLen = sInserted.getLength();
    if (nLen && nInsLen)
    {
        m_Text = m_Text.replaceAt(nStartPos, nLen, sInserted);

        if (nLen > nInsLen) // short insert
        {
            // delete up to the point that the user specified
            const SwContentIndex aNegIdx(rStart, nInsLen);
            Update(aNegIdx, nLen - nInsLen, UpdateMode::Negative);
        }
        else if (nLen < nInsLen) // long insert
        {
            const SwContentIndex aIdx(rStart, nLen);
            Update(aIdx, nInsLen - nLen, UpdateMode::Replace);
        }

        for (sal_Int32 i = 0; i < nInsLen; i++)
        {
            ++const_cast<SwContentIndex&>(rStart);
        }
    }
    else
    {
        m_Text = m_Text.replaceAt(nStartPos, nLen, u"");
        Update(rStart, nLen, UpdateMode::Negative);

        m_Text = m_Text.replaceAt(nStartPos, 0, sInserted);
        Update(rStart, sInserted.getLength(), UpdateMode::Replace);
    }

    SetIgnoreDontExpand( bOldExpFlg );
    auto aDelHint = sw::DeleteText(nStartPos, nDelLen);
    CallSwClientNotify(aDelHint);

    if (sInserted.getLength())
    {
        auto aInsHint = sw::MakeInsertText(*this, nStartPos, sInserted.getLength());
        CallSwClientNotify(aInsHint);
    }
}

void SwTextNode::ReplaceText( const SwPosition& rStart, const sal_Int32 nDelLen,
                             const OUString & rStr)
{
    ReplaceText(rStart.nContent, nDelLen, rStr);
}

namespace {
    void lcl_ResetParAttrs( SwTextNode &rTextNode )
    {
        const o3tl::sorted_vector<sal_uInt16> aAttrs{ RES_PARATR_LIST_ID, RES_PARATR_LIST_LEVEL,
                                                      RES_PARATR_LIST_ISRESTART,
                                                      RES_PARATR_LIST_RESTARTVALUE,
                                                      RES_PARATR_LIST_ISCOUNTED };
        SwPaM aPam( rTextNode );
        // #i96644#
        // suppress side effect "send data changed events"
        rTextNode.GetDoc().ResetAttrs( aPam, false, aAttrs, false );
    }

    void HandleApplyTextNodeFormatChange( SwTextNode& rTextNode, const UIName& sNumRule, const UIName& sOldNumRule, bool bNumRuleSet, bool bParagraphStyleChanged );

    // Helper method for special handling of modified attributes at text node.
    // The following is handled:
    // (1) on changing the paragraph style - RES_FMT_CHG:
    // Check, if list style of the text node is changed. If yes, add respectively
    // remove the text node to the corresponding list.
    // (2) on changing the attributes - RES_ATTRSET_CHG:
    // Same as (1).
    // (3) on changing the list style - RES_PARATR_NUMRULE:
    // Same as (1).
    void HandleModifyAtTextNode( SwTextNode& rTextNode,
                                const SfxPoolItem* pOldValue,
                                const SfxPoolItem* pNewValue )
    {
        const sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
                              pNewValue ? pNewValue->Which() : 0 ;
        bool bNumRuleSet = false;
        bool bParagraphStyleChanged = false;
        UIName sNumRule;
        UIName sOldNumRule;
        switch ( nWhich )
        {
            case RES_PARATR_NUMRULE:
            {
                if ( rTextNode.GetNodes().IsDocNodes() )
                {
                    const SwNumRule* pFormerNumRuleAtTextNode =
                        rTextNode.GetNum() ? rTextNode.GetNum()->GetNumRule() : nullptr;
                    if ( pFormerNumRuleAtTextNode )
                    {
                        sOldNumRule = pFormerNumRuleAtTextNode->GetName();
                    }

                    if ( pNewValue )
                    {
                        // #i70748#
                        rTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                        bNumRuleSet = true;
                    }
                    // #i70748#
                    // The new list style set at the paragraph.
                    const SwNumRule* pNumRuleAtTextNode = rTextNode.GetNumRule();
                    if ( pNumRuleAtTextNode )
                    {
                        sNumRule = pNumRuleAtTextNode->GetName();
                    }
                }
                break;
            }
        }
        HandleApplyTextNodeFormatChange(rTextNode, sNumRule, sOldNumRule, bNumRuleSet, bParagraphStyleChanged);
    }
    // End of method <HandleModifyAtTextNode>

    void HandleModifyAtTextNode( SwTextNode& rTextNode,
                                const SwAttrSetChg* /*pOldValue*/,
                                const SwAttrSetChg* pNewValue )
    {
        bool bNumRuleSet = false;
        UIName sNumRule;
        UIName sOldNumRule;
        const SwNumRule* pFormerNumRuleAtTextNode =
            rTextNode.GetNum() ? rTextNode.GetNum()->GetNumRule() : nullptr;
        if ( pFormerNumRuleAtTextNode )
        {
            sOldNumRule = pFormerNumRuleAtTextNode->GetName();
        }

        if ( pNewValue && pNewValue->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, false ) ==
                SfxItemState::SET )
        {
            // #i70748#
            rTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            bNumRuleSet = true;
        }
        // #i70748#
        // The new list style set at the paragraph.
        const SwNumRule* pNumRuleAtTextNode = rTextNode.GetNumRule();
        if ( pNumRuleAtTextNode )
        {
            sNumRule = pNumRuleAtTextNode->GetName();
        }
        HandleApplyTextNodeFormatChange(rTextNode, sNumRule, sOldNumRule, bNumRuleSet, /*bParagraphStyleChanged*/false);
    }

    // Helper method for special handling of modified attributes at text node.
    // The following is handled:
    // (1) on changing the paragraph style - RES_FMT_CHG:
    // Check, if list style of the text node is changed. If yes, add respectively
    // remove the text node to the corresponding list.
    void HandleModifyAtTextNodeFormatChange( SwTextNode& rTextNode )
    {
        bool bNumRuleSet = false;
        bool bParagraphStyleChanged = true;
        UIName sNumRule;
        UIName sOldNumRule;
        if( rTextNode.GetNodes().IsDocNodes() )
        {
            const SwNumRule* pFormerNumRuleAtTextNode =
                rTextNode.GetNum() ? rTextNode.GetNum()->GetNumRule() : nullptr;
            if ( pFormerNumRuleAtTextNode )
            {
                sOldNumRule = pFormerNumRuleAtTextNode->GetName();
            }
            if ( rTextNode.IsEmptyListStyleDueToSetOutlineLevelAttr() )
            {
                const SwNumRuleItem& rNumRuleItem = rTextNode.GetTextColl()->GetNumRule();
                if ( !rNumRuleItem.GetValue().isEmpty() )
                {
                    rTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }
            }
            const SwNumRule* pNumRuleAtTextNode = rTextNode.GetNumRule();
            if ( pNumRuleAtTextNode )
            {
                bNumRuleSet = true;
                sNumRule = pNumRuleAtTextNode->GetName();
            }
        }
        HandleApplyTextNodeFormatChange(rTextNode, sNumRule, sOldNumRule, bNumRuleSet, bParagraphStyleChanged);
    }

    void HandleApplyTextNodeFormatChange( SwTextNode& rTextNode, const UIName& sNumRule, const UIName& sOldNumRule, bool bNumRuleSet, bool bParagraphStyleChanged )
    {
        if ( sNumRule != sOldNumRule )
        {
            if ( bNumRuleSet )
            {
                if (sNumRule.isEmpty())
                {
                    rTextNode.RemoveFromList();
                    if ( bParagraphStyleChanged )
                    {
                        lcl_ResetParAttrs(rTextNode);
                    }
                }
                else
                {
                    rTextNode.RemoveFromList();
                    // If new list style is the outline style, apply outline
                    // level as the list level.
                    if (sNumRule==SwNumRule::GetOutlineRuleName())
                    {
                        // #i70748#
                        OSL_ENSURE( rTextNode.GetTextColl()->IsAssignedToListLevelOfOutlineStyle(),
                                "<HandleModifyAtTextNode()> - text node with outline style, but its paragraph style is not assigned to outline style." );
                        const int nNewListLevel =
                            rTextNode.GetTextColl()->GetAssignedOutlineStyleLevel();
                        if ( 0 <= nNewListLevel && nNewListLevel < MAXLEVEL )
                        {
                            rTextNode.SetAttrListLevel( nNewListLevel );
                        }
                    }
                    rTextNode.AddToList();
                }
            }
            else // <sNumRule.Len() == 0 && sOldNumRule.Len() != 0>
            {
                rTextNode.RemoveFromList();
                if ( bParagraphStyleChanged )
                {
                    lcl_ResetParAttrs(rTextNode);
                    // #i70748#
                    if ( rTextNode.GetAttr( RES_PARATR_OUTLINELEVEL, false ).GetValue() > 0 )
                    {
                        rTextNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                    }
                }
            }
        }
        else if (!sNumRule.isEmpty() && !rTextNode.IsInList())
        {
            rTextNode.AddToList();
        }
    }
}

SwFormatColl* SwTextNode::ChgFormatColl( SwFormatColl *pNewColl, bool bSetListLevel )
{
    OSL_ENSURE( pNewColl,"ChgFormatColl: Collectionpointer has value 0." );
    assert( dynamic_cast<const SwTextFormatColl *>(pNewColl) && "ChgFormatColl: is not a Text Collection pointer." );

    SwTextFormatColl *pOldColl = GetTextColl();
    if( pNewColl != pOldColl )
    {
        SetCalcHiddenCharFlags();
        SwContentNode::ChgFormatColl( pNewColl );
        OSL_ENSURE( !mbInSetOrResetAttr,
                "DEBUG OSL_ENSURE(ON - <SwTextNode::ChgFormatColl(..)> called during <Set/ResetAttr(..)>" );
        if ( !mbInSetOrResetAttr )
        {
            HandleModifyAtTextNodeFormatChange( *this  );
        }

        // reset fill information on parent style change
        if(maFillAttributes)
        {
            maFillAttributes.reset();
        }
    }

    // only for real nodes-array
    if( GetNodes().IsDocNodes() )
    {
        ChgTextCollUpdateNum( pOldColl, static_cast<SwTextFormatColl *>(pNewColl), bSetListLevel );
    }

    return pOldColl;
}

const SwNodeNum* SwTextNode::GetNum(SwRootFrame const*const pLayout, SwListRedlineType eRedline) const
{
    // invariant: it's only in list in Hide mode if it's in list in normal mode
    assert(mpNodeNum || !mpNodeNumRLHidden);
    return (pLayout && pLayout->IsHideRedlines()) || SwListRedlineType::HIDDEN == eRedline
            ? mpNodeNumRLHidden.get()
            : ( SwListRedlineType::ORIGTEXT == eRedline ? mpNodeNumOrig.get() : mpNodeNum.get() );
}

void SwTextNode::DoNum(std::function<void (SwNodeNum &)> const& rFunc)
{
    // temp. clear because GetActualListLevel() may be called and the assert
    // there triggered during update, which is unhelpful
    std::unique_ptr<SwNodeNum> pBackup = std::move(mpNodeNumRLHidden);
    std::unique_ptr<SwNodeNum> pBackup2 = std::move(mpNodeNumOrig);
    assert(mpNodeNum);
    rFunc(*mpNodeNum);
    if (pBackup)
    {
        mpNodeNumRLHidden = std::move(pBackup);
        rFunc(*mpNodeNumRLHidden);
    }
    if (pBackup2)
    {
        mpNodeNumOrig = std::move(pBackup2);
        rFunc(*mpNodeNumOrig);
    }
}

SwNumberTree::tNumberVector
SwTextNode::GetNumberVector(SwRootFrame const*const pLayout, SwListRedlineType eRedline) const
{
    if (SwNodeNum const*const pNum = GetNum(pLayout, eRedline))
    {
        return pNum->GetNumberVector();
    }
    else
    {
        SwNumberTree::tNumberVector aResult;
        return aResult;
    }
}

bool SwTextNode::IsOutline() const
{
    bool bResult = false;

    if ( GetAttrOutlineLevel() > 0 )
    {
        bResult = !IsInRedlines();
    }
    else
    {
        const SwNumRule* pRule( GetNum() ? GetNum()->GetNumRule() : nullptr );
        if ( pRule && pRule->IsOutlineRule() )
        {
            bResult = !IsInRedlines();
        }
    }

    return bResult;
}

bool SwTextNode::IsOutlineStateChanged() const
{
    return IsOutline() != m_bLastOutlineState;
}

void SwTextNode::UpdateOutlineState()
{
    m_bLastOutlineState = IsOutline();
}

int SwTextNode::GetAttrOutlineLevel(bool bInlineHeading) const
{
    sal_uInt16 nLevel = GetAttr(RES_PARATR_OUTLINELEVEL).GetValue();
    // not outline node, so if bIblineHeading = true, look for the
    // outline level of the inline heading (i.e the outline node in
    // an Inline Heading frame, which frame anchored as character to this node)
    if ( !nLevel && bInlineHeading && HasHints() )
    {
        // are we in a fly
        for ( size_t j = m_pSwpHints->Count(); j; )
        {
            SwTextAttr* const pHt = m_pSwpHints->Get( --j );
            if ( RES_TXTATR_FLYCNT == pHt->Which() )
            {
                SwFrameFormat* pFrameFormat = pHt->GetFlyCnt().GetFrameFormat();
                const SwFormat* pParent = pFrameFormat->DerivedFrom();
                SwFormatAnchor const& rAnchor(pFrameFormat->GetAnchor());
                bool bInlineHeadingFrame = pParent &&
                        pParent->GetPoolFormatId() == RES_POOLFRM_INLINE_HEADING &&
                        RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId();
                const SwNodeIndex* pNdIdx = bInlineHeadingFrame
                                             ? pFrameFormat->GetContent().GetContentIdx()
                                             : nullptr;
                const SwNodes* pNodesArray = (pNdIdx != nullptr)
                                             ? &(pNdIdx->GetNodes())
                                             : nullptr;
                const SwTextNode *pTextNode = (pNodesArray != nullptr)
                                        ? (*pNodesArray)[pNdIdx->GetIndex() + 1]->GetTextNode()
                                        : nullptr;
                if ( pTextNode )
                    return pTextNode->GetAttrOutlineLevel();
            }
        }
    }
    return nLevel;
}

void SwTextNode::SetAttrOutlineLevel(int nLevel)
{
    assert(0 <= nLevel && nLevel <= MAXLEVEL); // Level Out Of Range
    if ( 0 <= nLevel && nLevel <= MAXLEVEL )
    {
        SetAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                                o3tl::narrowing<sal_uInt16>(nLevel) ) );
    }
}

bool SwTextNode::GetAttrOutlineContentVisible() const
{
    bool bOutlineContentVisibleAttr = true;
    const SfxGrabBagItem & rGrabBagItem = GetAttr(RES_PARATR_GRABBAG);
    auto it = rGrabBagItem.GetGrabBag().find(u"OutlineContentVisibleAttr"_ustr);
    if (it != rGrabBagItem.GetGrabBag().end())
        it->second >>= bOutlineContentVisibleAttr;
    return bOutlineContentVisibleAttr;
}

void SwTextNode::SetAttrOutlineContentVisible(bool bVisible)
{
    SfxGrabBagItem aGrabBagItem(
        RES_PARATR_GRABBAG,
        std::map<OUString, css::uno::Any>{
            { u"OutlineContentVisibleAttr"_ustr, css::uno::Any(bVisible) } });
    SetAttr(aGrabBagItem);
}

// #i70748#

void SwTextNode::SetEmptyListStyleDueToSetOutlineLevelAttr()
{
    if ( !mbEmptyListStyleSetDueToSetOutlineLevelAttr )
    {
        SetAttr( SwNumRuleItem() );
        mbEmptyListStyleSetDueToSetOutlineLevelAttr = true;
    }
}

void SwTextNode::ResetEmptyListStyleDueToResetOutlineLevelAttr()
{
    if ( mbEmptyListStyleSetDueToSetOutlineLevelAttr )
    {
        ResetAttr( RES_PARATR_NUMRULE );
        mbEmptyListStyleSetDueToSetOutlineLevelAttr = false;
    }
}

void SwTextNode::SetAttrListLevel( int nLevel )
{
    if ( nLevel < 0 || nLevel >= MAXLEVEL )
    {
        assert(false); // invalid level
        return;
    }

    SfxInt16Item aNewListLevelItem( RES_PARATR_LIST_LEVEL,
                                    static_cast<sal_Int16>(nLevel) );
    SetAttr( aNewListLevelItem );
}

bool SwTextNode::HasAttrListLevel() const
{
    return GetpSwAttrSet() &&
           GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_LEVEL, false ) == SfxItemState::SET;
}

int SwTextNode::GetAttrListLevel() const
{
    int nAttrListLevel = 0;

    const SfxInt16Item& aListLevelItem =
        GetAttr( RES_PARATR_LIST_LEVEL );
    nAttrListLevel = static_cast<int>(aListLevelItem.GetValue());

    return nAttrListLevel;
}

int SwTextNode::GetActualListLevel(SwListRedlineType eRedline) const
{
    assert(SwListRedlineType::SHOW != eRedline ||
        !GetNum(nullptr, SwListRedlineType::SHOW) || !mpNodeNumRLHidden || // must be in sync
        GetNum(nullptr, SwListRedlineType::SHOW)->GetLevelInListTree() ==
                                                        mpNodeNumRLHidden->GetLevelInListTree());
    return GetNum(nullptr, eRedline) ? GetNum(nullptr, eRedline)->GetLevelInListTree() : -1;
}

void SwTextNode::SetListRestart( bool bRestart )
{
    if ( !bRestart )
    {
        // attribute not contained in paragraph style's attribute set. Thus,
        // it can be reset to the attribute pool default by resetting the attribute.
        ResetAttr( RES_PARATR_LIST_ISRESTART );
    }
    else
    {
        SfxBoolItem aNewIsRestartItem( RES_PARATR_LIST_ISRESTART,
                                       true );
        SetAttr( aNewIsRestartItem );
    }
}

bool SwTextNode::IsListRestart() const
{
    const SfxBoolItem& aIsRestartItem = GetAttr( RES_PARATR_LIST_ISRESTART );

    return aIsRestartItem.GetValue();
}

/** Returns if the paragraph has a visible numbering or bullet.
    This includes all kinds of numbering/bullet/outlines.
    The concrete list label string has to be checked, too.
 */
bool SwTextNode::HasVisibleNumberingOrBullet() const
{
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pRule && IsCountedInList())
    {
        const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));
        if (getIDocumentSettingAccess()->get(DocumentSettingId::NO_NUMBERING_SHOW_FOLLOWBY))
            // True if we have something in label text or there is a non-empty
            // FollowedBy separator (space, tab or whatsoever)
            return rFormat.GetLabelFollowedBy() != SvxNumberFormat::LabelFollowedBy::NOTHING ||
                !pRule->MakeNumString(*GetNum()).isEmpty();
        else
            // #i87154#
            // Correction of #newlistlevelattrs#:
            // The numbering type has to be checked for bullet lists.
            return SVX_NUM_NUMBER_NONE != rFormat.GetNumberingType() ||
                !pRule->MakeNumString(*(GetNum())).isEmpty();
    }

    return false;
}

void SwTextNode::SetAttrListRestartValue( SwNumberTree::tSwNumTreeNumber nNumber )
{
    const bool bChanged( HasAttrListRestartValue()
                         ? GetAttrListRestartValue() != nNumber
                         : nNumber != USHRT_MAX );

    if ( !bChanged && HasAttrListRestartValue() )
        return;

    if ( nNumber == USHRT_MAX )
    {
        ResetAttr( RES_PARATR_LIST_RESTARTVALUE );
    }
    else
    {
        SfxInt16Item aNewListRestartValueItem( RES_PARATR_LIST_RESTARTVALUE,
                                               static_cast<sal_Int16>(nNumber) );
        SetAttr( aNewListRestartValueItem );
    }
}

bool SwTextNode::HasAttrListRestartValue() const
{
    return GetpSwAttrSet() &&
           GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_RESTARTVALUE, false ) == SfxItemState::SET;
}
SwNumberTree::tSwNumTreeNumber SwTextNode::GetAttrListRestartValue() const
{
    OSL_ENSURE( HasAttrListRestartValue(),
            "<SwTextNode::GetAttrListRestartValue()> - only ask for list restart value, if attribute is set at text node." );

    const SfxInt16Item& aListRestartValueItem =
        GetAttr( RES_PARATR_LIST_RESTARTVALUE );
    return static_cast<SwNumberTree::tSwNumTreeNumber>(aListRestartValueItem.GetValue());
}

SwNumberTree::tSwNumTreeNumber SwTextNode::GetActualListStartValue() const
{
    SwNumberTree::tSwNumTreeNumber nListRestartValue = 1;

    if ( IsListRestart() && HasAttrListRestartValue() )
    {
        nListRestartValue = GetAttrListRestartValue();
    }
    else
    {
        SwNumRule* pRule = GetNumRule();
        if ( pRule )
        {
            const SwNumFormat* pFormat =
                    pRule->GetNumFormat( o3tl::narrowing<sal_uInt16>(GetAttrListLevel()) );
            if ( pFormat )
            {
                nListRestartValue = pFormat->GetStart();
            }
        }
    }

    return nListRestartValue;
}

bool SwTextNode::IsNotifiable() const
{
    return m_bNotifiable && IsNotificationEnabled();
}

bool SwTextNode::IsNotificationEnabled() const
{
    const SwDoc& rDoc = GetDoc();
    return !rDoc.IsInReading() && !rDoc.IsInDtor();
}

void SwTextNode::SetCountedInList( bool bCounted )
{
    if ( bCounted )
    {
        // attribute not contained in paragraph style's attribute set. Thus,
        // it can be reset to the attribute pool default by resetting the attribute.
        ResetAttr( RES_PARATR_LIST_ISCOUNTED );
    }
    else
    {
        SfxBoolItem aIsCountedInListItem( RES_PARATR_LIST_ISCOUNTED, false );
        SetAttr( aIsCountedInListItem );
    }
}

bool SwTextNode::IsCountedInList() const
{
    const SfxBoolItem& aIsCountedInListItem = GetAttr( RES_PARATR_LIST_ISCOUNTED );

    return aIsCountedInListItem.GetValue();
}

static SwList * FindList(SwTextNode *const pNode)
{
    const OUString sListId = pNode->GetListId();
    if (!sListId.isEmpty())
    {
        auto & rIDLA(pNode->GetDoc().getIDocumentListsAccess());
        SwList* pList = rIDLA.getListByName( sListId );
        if ( pList == nullptr )
        {
            // Create corresponding list.
            SwNumRule* pNumRule = pNode->GetNumRule();
            if ( pNumRule )
            {
                pList = rIDLA.createList(sListId, pNode->GetNumRule()->GetName());
            }
        }
        OSL_ENSURE( pList != nullptr,
                "<SwTextNode::AddToList()> - no list for given list id. Serious defect" );
        return pList;
    }
    return nullptr;
}

void SwTextNode::AddToList()
{
    if ( IsInList() )
    {
        OSL_FAIL( "<SwTextNode::AddToList()> - the text node is already added to a list. Serious defect" );
        return;
    }

    SwList *const pList(FindList(this));
    if (!(pList && GetNodes().IsDocNodes())) // not for undo nodes
        return;

    assert(!mpNodeNum);
    mpNodeNum.reset(new SwNodeNum(this, false));
    pList->InsertListItem(*mpNodeNum, SwListRedlineType::SHOW, GetAttrListLevel(), GetDoc());

    // set redline lists
    // "default" list: visible items in Show Changes mode (tracked insertions and deletions)
    // "hidden" list: visible items in Hide Changes mode (tracked insertions, but not deletions)
    // "orig" list: visible items rejecting all changes (no tracked insertions and deletions)
    SwDocShell* pShell = GetDoc().GetDocShell();
    bool bRecordChanges = pShell && pShell->IsChangeRecording();
    if (!bRecordChanges || GetDoc().IsInXMLImport() || GetDoc().IsInWriterfilterImport() )
    {
        const SwRedlineTable& rRedTable = GetDoc().getIDocumentRedlineAccess().GetRedlineTable();
        SwRedlineTable::size_type nRedlPos = GetDoc().getIDocumentRedlineAccess().GetRedlinePos(*this, RedlineType::Insert);
        // paragraph start is not in a tracked insertion
        if ( SwRedlineTable::npos == nRedlPos || GetIndex() <= rRedTable[nRedlPos]->Start()->GetNode().GetIndex() )
        {
            AddToListOrig();

            // if the paragraph is not deleted, add to the "hidden" list, too
            SwRedlineTable::size_type nRedlPosDel = GetDoc().getIDocumentRedlineAccess().GetRedlinePos(*this, RedlineType::Delete);
            if ( SwRedlineTable::npos == nRedlPosDel )
                AddToListRLHidden();
            else
            {
                const SwNodeOffset nNdIdx = GetIndex();
                const SwRangeRedline* pTmp = rRedTable[nRedlPosDel];
                const SwPosition* pRStt = pTmp->Start();
                if (pRStt->GetNodeIndex() >= nNdIdx)
                {
                    // paragraph is partly deleted, add to the "hidden" list, too
                    AddToListRLHidden();
                }
            }
        }
        // inserted paragraph, e.g. during file load, add to the "hidden" list
        else if ( SwRedlineTable::npos != nRedlPos )
            AddToListRLHidden();
    }
    else if ( bRecordChanges )
        AddToListRLHidden();

    // iterate all frames & if there's one with hidden layout...
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> iter(*this);
    for (SwTextFrame* pFrame = iter.First(); pFrame && !mpNodeNumRLHidden; pFrame = iter.Next())
    {
        if (pFrame->getRootFrame()->IsHideRedlines())
        {
            if (pFrame->GetTextNodeForParaProps() == this)
            {
                AddToListRLHidden();
            }
            break; // assume it's consistent, need to check only once
        }
    }
}

void SwTextNode::AddToListRLHidden()
{
    if (mpNodeNumRLHidden)
        return;

    SwList *const pList(FindList(this));
    if (pList)
    {
        assert(!mpNodeNumRLHidden);
        mpNodeNumRLHidden.reset(new SwNodeNum(this, true));
        pList->InsertListItem(*mpNodeNumRLHidden, SwListRedlineType::HIDDEN, GetAttrListLevel(), GetDoc());
    }
}

void SwTextNode::AddToListOrig()
{
    if (mpNodeNumOrig)
        return;

    SwList *const pList(FindList(this));
    if (pList)
    {
        assert(!mpNodeNumOrig);
        mpNodeNumOrig.reset(new SwNodeNum(this, true));
        pList->InsertListItem(*mpNodeNumOrig, SwListRedlineType::ORIGTEXT, GetAttrListLevel(), GetDoc());
    }
}

void SwTextNode::RemoveFromList()
{
    // sw_redlinehide: ensure it's removed from the other half too!
    RemoveFromListRLHidden();
    RemoveFromListOrig();
    if ( IsInList() )
    {
        SwList::RemoveListItem(*mpNodeNum, GetDoc());
        mpNodeNum.reset();

        SetWordCountDirty( true );
    }
}

void SwTextNode::RemoveFromListRLHidden()
{
    if (mpNodeNumRLHidden) // direct access because RemoveFromList doesn't have layout
    {
        assert(mpNodeNumRLHidden->GetParent() || !GetNodes().IsDocNodes());
        SwList::RemoveListItem(*mpNodeNumRLHidden, GetDoc());
        mpNodeNumRLHidden.reset();

        SetWordCountDirty( true );
    }
}

void SwTextNode::RemoveFromListOrig()
{
    if (mpNodeNumOrig) // direct access because RemoveFromList doesn't have layout
    {
        assert(mpNodeNumOrig->GetParent() || !GetNodes().IsDocNodes());
        SwList::RemoveListItem(*mpNodeNumOrig, GetDoc());
        mpNodeNumOrig.reset();

        SetWordCountDirty( true );
    }
}

bool SwTextNode::IsInList() const
{
    return GetNum() != nullptr && GetNum()->GetParent() != nullptr;
}

bool SwTextNode::IsInListFromStyle()  const
{
    if (IsInList())
    {
        const SwFormatColl* pFormatColl = GetFormatColl();
        if (pFormatColl->GetItemState(RES_PARATR_NUMRULE, true) == SfxItemState::SET)
            return true;
    }
    return false;

}
bool SwTextNode::IsFirstOfNumRule(SwRootFrame const& rLayout) const
{
    bool bResult = false;

    SwNodeNum const*const pNum(GetNum(&rLayout));
    if (pNum && pNum->GetNumRule())
        bResult = pNum->IsFirst();

    return bResult;
}

void SwTextNode::SetListId(OUString const& rListId)
{
    const SfxStringItem& rListIdItem =
            GetAttr( RES_PARATR_LIST_ID );
    if (rListIdItem.GetValue() != rListId)
    {
        if (rListId.isEmpty())
        {
            ResetAttr( RES_PARATR_LIST_ID );
        }
        else
        {
            SfxStringItem aNewListIdItem(RES_PARATR_LIST_ID, rListId);
            SetAttr( aNewListIdItem );
        }
    }
}

OUString SwTextNode::GetListId() const
{
    const SfxStringItem& rListIdItem =
                GetAttr( RES_PARATR_LIST_ID );
    const OUString& sListId {rListIdItem.GetValue()};

    // As long as no explicit list id attribute is set, use the list id of
    // the list, which has been created for the applied list style.
    if (sListId.isEmpty())
    {
        SwNumRule* pRule = GetNumRule();
        if ( pRule )
        {
            return pRule->GetDefaultListId();
        }
    }

    return sListId;
}

/** Determines, if the list level indent attributes can be applied to the
    paragraph.

    The list level indents can be applied to the paragraph under the one
    of following conditions:
    - the list style is directly applied to the paragraph and the paragraph
      has no own indent attributes.
    - the list style is applied to the paragraph through one of its paragraph
      styles, the paragraph has no own indent attributes and on the paragraph
      style hierarchy from the paragraph to the paragraph style with the
      list style no indent attributes are found.

    @return bitmask
*/
::sw::ListLevelIndents SwTextNode::AreListLevelIndentsApplicable() const
{
    ::sw::ListLevelIndents ret(::sw::ListLevelIndents::No);
    if (AreListLevelIndentsApplicableImpl(RES_MARGIN_FIRSTLINE))
    {
        ret |= ::sw::ListLevelIndents::FirstLine;
    }
    if (AreListLevelIndentsApplicableImpl(RES_MARGIN_TEXTLEFT))
    {
        ret |= ::sw::ListLevelIndents::LeftMargin;
    }
    return ret;
}

bool SwTextNode::AreListLevelIndentsApplicableImpl(sal_uInt16 const nWhich) const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( !GetNum() || !GetNum()->GetNumRule() )
    {
        // no list style applied to paragraph
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
             GetpSwAttrSet()->GetItemState(nWhich, false) == SfxItemState::SET)
    {
        // paragraph has hard-set indent attributes
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
              GetpSwAttrSet()->GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET )
    {
        // list style is directly applied to paragraph and paragraph has no
        // hard-set indent attributes
        bAreListLevelIndentsApplicable = true;
    }
    else
    {
        // list style is applied through one of the paragraph styles and
        // paragraph has no hard-set indent attributes

        // check, paragraph's
        const SwTextFormatColl* pColl = GetTextColl();
        while ( pColl )
        {
            if (pColl->GetAttrSet().GetItemState(nWhich, false) == SfxItemState::SET)
            {
                // indent attributes found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = false;
                break;
            }

            if ( pColl->GetAttrSet().GetItemState( RES_PARATR_NUMRULE, false ) == SfxItemState::SET )
            {
                // paragraph style with the list style found and until now no
                // indent attributes are found in the paragraph style hierarchy.
                bAreListLevelIndentsApplicable = true;
                break;
            }

            pColl = dynamic_cast<const SwTextFormatColl*>(pColl->DerivedFrom());
            OSL_ENSURE( pColl,
                    "<SwTextNode::AreListLevelIndentsApplicable()> - something wrong in paragraph's style hierarchy. The applied list style is not found." );
        }
    }

    return bAreListLevelIndentsApplicable;
}

/** Retrieves the list tab stop position, if the paragraph's list level defines
    one and this list tab stop has to merged into the tap stops of the paragraph

    @param nListTabStopPosition
    output parameter - containing the list tab stop position

    @return boolean - indicating, if a list tab stop position is provided
*/
bool SwTextNode::GetListTabStopPosition( tools::Long& nListTabStopPosition ) const
{
    bool bListTabStopPositionProvided(false);

    const SwNumRule* pNumRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pNumRule && HasVisibleNumberingOrBullet() && GetActualListLevel() >= 0 )
    {
        const SwNumFormat& rFormat = pNumRule->Get( o3tl::narrowing<sal_uInt16>(GetActualListLevel()) );
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
             rFormat.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
        {
            bListTabStopPositionProvided = true;
            nListTabStopPosition = rFormat.GetListtabPos();

            if ( getIDocumentSettingAccess()->get(DocumentSettingId::TABS_RELATIVE_TO_INDENT) )
            {
                // tab stop position are treated to be relative to the "before text"
                // indent value of the paragraph. Thus, adjust <nListTabStopPos>.
                if (AreListLevelIndentsApplicable() & ::sw::ListLevelIndents::LeftMargin)
                {
                    nListTabStopPosition -= rFormat.GetIndentAt();
                }
                else if (!getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxTextLeftMarginItem const aItem(GetSwAttrSet().GetTextLeftMargin());
                    nListTabStopPosition -= aItem.ResolveTextLeft({});
                }
            }
        }
    }

    return bListTabStopPositionProvided;
}

OUString SwTextNode::GetLabelFollowedBy() const
{
    const SwNumRule* pNumRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pNumRule && HasVisibleNumberingOrBullet() && GetActualListLevel() >= 0 )
    {
        const SwNumFormat& rFormat = pNumRule->Get( o3tl::narrowing<sal_uInt16>(GetActualListLevel()) );
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            return rFormat.GetLabelFollowedByAsString();
        }
    }

    return OUString();
}

void SwTextNode::CalcHiddenCharFlags() const
{
    sal_Int32 nStartPos;
    sal_Int32 nEndPos;
    // Update of the flags is done inside GetBoundsOfHiddenRange()
    SwScriptInfo::GetBoundsOfHiddenRange( *this, 0, nStartPos, nEndPos );
}

// #i12836# enhanced pdf export
bool SwTextNode::IsHidden() const
{
    if ( IsHiddenByParaField() || HasHiddenCharAttribute( true ) )
        return true;

    const SwSectionNode* pSectNd = FindSectionNode();
    return pSectNd && pSectNd->GetSection().IsHiddenFlag();
}

namespace {
    // Helper class for special handling of setting attributes at text node:
    // In constructor an instance of the helper class recognize whose attributes
    // are set and perform corresponding actions before the intrinsic set of
    // attributes has been taken place.
    // In the destructor - after the attributes have been set at the text
    // node - corresponding actions are performed.
    // The following is handled:
    // (1) When the list style attribute - RES_PARATR_NUMRULE - is set,
    //     (A) list style attribute is empty -> the text node is removed from
    //         its list.
    //     (B) list style attribute is not empty
    //         (a) text node has no list style -> add text node to its list after
    //             the attributes have been set.
    //         (b) text node has list style -> change of list style is notified
    //             after the attributes have been set.
    // (2) When the list id attribute - RES_PARATR_LIST_ID - is set and changed,
    //     the text node is removed from its current list before the attributes
    //     are set and added to its new list after the attributes have been set.
    // (3) Notify list tree, if list level - RES_PARATR_LIST_LEVEL - is set
    //     and changed after the attributes have been set
    // (4) Notify list tree, if list restart - RES_PARATR_LIST_ISRESTART - is set
    //     and changed after the attributes have been set
    // (5) Notify list tree, if list restart value - RES_PARATR_LIST_RESTARTVALUE -
    //     is set and changed after the attributes have been set
    // (6) Notify list tree, if count in list - RES_PARATR_LIST_ISCOUNTED - is set
    //     and changed after the attributes have been set
    // (7) Set or Reset empty list style due to changed outline level - RES_PARATR_OUTLINELEVEL.
    class HandleSetAttrAtTextNode
    {
        public:
            HandleSetAttrAtTextNode( SwTextNode& rTextNode,
                                    const SfxPoolItem& pItem );
            HandleSetAttrAtTextNode( SwTextNode& rTextNode,
                                    const SfxItemSet& rItemSet );
            ~HandleSetAttrAtTextNode() COVERITY_NOEXCEPT_FALSE;

        private:
            SwTextNode& mrTextNode;
            bool mbAddTextNodeToList;
            bool mbUpdateListLevel;
            bool mbUpdateListRestart;
            bool mbUpdateListCount;
            // #i70748#
            bool mbOutlineLevelSet;
    };

    HandleSetAttrAtTextNode::HandleSetAttrAtTextNode( SwTextNode& rTextNode,
                                                    const SfxPoolItem& pItem )
        : mrTextNode( rTextNode ),
          mbAddTextNodeToList( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false ),
          // #i70748#
          mbOutlineLevelSet( false )
    {
        switch ( pItem.Which() )
        {
            // handle RES_PARATR_NUMRULE
            case RES_PARATR_NUMRULE:
            {
                mrTextNode.RemoveFromList();

                const SwNumRuleItem& rNumRuleItem = pItem.StaticWhichCast(RES_PARATR_NUMRULE);
                if ( !rNumRuleItem.GetValue().isEmpty() )
                {
                    mbAddTextNodeToList = true;
                    // #i105562#

                    mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }
            }
            break;

            // handle RES_PARATR_LIST_ID
            case RES_PARATR_LIST_ID:
            {
                const SfxStringItem& rListIdItem = pItem.StaticWhichCast(RES_PARATR_LIST_ID);
                OSL_ENSURE( rListIdItem.GetValue().getLength() > 0,
                        "<HandleSetAttrAtTextNode(..)> - empty list id attribute not expected. Serious defect." );
                const OUString sListIdOfTextNode = rTextNode.GetListId();
                if ( rListIdItem.GetValue() != sListIdOfTextNode )
                {
                    mbAddTextNodeToList = true;
                    if ( mrTextNode.IsInList() )
                    {
                        mrTextNode.RemoveFromList();
                    }
                }
            }
            break;

            // handle RES_PARATR_LIST_LEVEL
            case RES_PARATR_LIST_LEVEL:
            {
                const SfxInt16Item& aListLevelItem = pItem.StaticWhichCast(RES_PARATR_LIST_LEVEL);
                if ( aListLevelItem.GetValue() != mrTextNode.GetAttrListLevel() )
                {
                    mbUpdateListLevel = true;
                }
            }
            break;

            // handle RES_PARATR_LIST_ISRESTART
            case RES_PARATR_LIST_ISRESTART:
            {
                const SfxBoolItem& aListIsRestartItem = pItem.StaticWhichCast(RES_PARATR_LIST_ISRESTART);
                if ( aListIsRestartItem.GetValue() !=
                                    mrTextNode.IsListRestart() )
                {
                    mbUpdateListRestart = true;
                }
            }
            break;

            // handle RES_PARATR_LIST_RESTARTVALUE
            case RES_PARATR_LIST_RESTARTVALUE:
            {
                const SfxInt16Item& aListRestartValueItem = pItem.StaticWhichCast(RES_PARATR_LIST_RESTARTVALUE);
                if ( !mrTextNode.HasAttrListRestartValue() ||
                     aListRestartValueItem.GetValue() != mrTextNode.GetAttrListRestartValue() )
                {
                    mbUpdateListRestart = true;
                }
            }
            break;

            // handle RES_PARATR_LIST_ISCOUNTED
            case RES_PARATR_LIST_ISCOUNTED:
            {
                const SfxBoolItem& aIsCountedInListItem = pItem.StaticWhichCast(RES_PARATR_LIST_ISCOUNTED);
                if ( aIsCountedInListItem.GetValue() !=
                                    mrTextNode.IsCountedInList() )
                {
                    mbUpdateListCount = true;
                }
            }
            break;

            // #i70748#
            // handle RES_PARATR_OUTLINELEVEL
            case RES_PARATR_OUTLINELEVEL:
            {
                const SfxUInt16Item& aOutlineLevelItem = pItem.StaticWhichCast(RES_PARATR_OUTLINELEVEL);
                if ( aOutlineLevelItem.GetValue() != mrTextNode.GetAttrOutlineLevel() )
                {
                    mbOutlineLevelSet = true;
                }
            }
            break;
        }

    }

    HandleSetAttrAtTextNode::HandleSetAttrAtTextNode( SwTextNode& rTextNode,
                                                    const SfxItemSet& rItemSet )
        : mrTextNode( rTextNode ),
          mbAddTextNodeToList( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false ),
          // #i70748#
          mbOutlineLevelSet( false )
    {
        // handle RES_PARATR_NUMRULE
        if ( const SwNumRuleItem* pNumRuleItem = rItemSet.GetItemIfSet( RES_PARATR_NUMRULE, false ) )
        {
            mrTextNode.RemoveFromList();

            if ( !pNumRuleItem->GetValue().isEmpty() )
            {
                mbAddTextNodeToList = true;
                // #i70748#
                mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }

        // handle RES_PARATR_LIST_ID
        if ( const SfxStringItem* pListIdItem = rItemSet.GetItemIfSet( RES_PARATR_LIST_ID, false ) )
        {
            const OUString sListIdOfTextNode = mrTextNode.GetListId();
            if ( pListIdItem->GetValue() != sListIdOfTextNode )
            {
                mbAddTextNodeToList = true;
                if ( mrTextNode.IsInList() )
                {
                    mrTextNode.RemoveFromList();
                }
            }
        }

        // handle RES_PARATR_LIST_LEVEL
        if ( const SfxInt16Item* pListLevelItem = rItemSet.GetItemIfSet( RES_PARATR_LIST_LEVEL, false ) )
        {
            if (pListLevelItem->GetValue() != mrTextNode.GetAttrListLevel())
            {
                mbUpdateListLevel = true;
            }
        }

        // handle RES_PARATR_LIST_ISRESTART
        if ( const SfxBoolItem* pListIsRestartItem = rItemSet.GetItemIfSet( RES_PARATR_LIST_ISRESTART, false ) )
        {
            if (pListIsRestartItem->GetValue() != mrTextNode.IsListRestart())
            {
                mbUpdateListRestart = true;
            }
        }

        // handle RES_PARATR_LIST_RESTARTVALUE
        if ( const SfxInt16Item* pListRestartValueItem = rItemSet.GetItemIfSet( RES_PARATR_LIST_RESTARTVALUE, false ) )
        {
            if ( !mrTextNode.HasAttrListRestartValue() ||
                 pListRestartValueItem->GetValue() != mrTextNode.GetAttrListRestartValue() )
            {
                mbUpdateListRestart = true;
            }
        }

        // handle RES_PARATR_LIST_ISCOUNTED
        if ( const SfxBoolItem* pIsCountedInListItem = rItemSet.GetItemIfSet( RES_PARATR_LIST_ISCOUNTED, false ) )
        {
            if (pIsCountedInListItem->GetValue() != mrTextNode.IsCountedInList())
            {
                mbUpdateListCount = true;
            }
        }

        // #i70748#
        // handle RES_PARATR_OUTLINELEVEL
        if ( const SfxUInt16Item* pOutlineLevelItem = rItemSet.GetItemIfSet( RES_PARATR_OUTLINELEVEL, false ) )
        {
            if (pOutlineLevelItem->GetValue() != mrTextNode.GetAttrOutlineLevel())
            {
                mbOutlineLevelSet = true;
            }
        }
    }

    HandleSetAttrAtTextNode::~HandleSetAttrAtTextNode() COVERITY_NOEXCEPT_FALSE
    {
        if ( mbAddTextNodeToList )
        {
            SwNumRule* pNumRuleAtTextNode = mrTextNode.GetNumRule();
            if ( pNumRuleAtTextNode )
            {
                mrTextNode.AddToList();
            }
        }
        else
        {
            if ( mbUpdateListLevel && mrTextNode.IsInList() )
            {
                auto const nLevel(mrTextNode.GetAttrListLevel());
                const SwDoc& rDoc(mrTextNode.GetDoc());
                mrTextNode.DoNum(
                    [nLevel, &rDoc](SwNodeNum & rNum) { rNum.SetLevelInListTree(nLevel, rDoc); });
            }

            if ( mbUpdateListRestart && mrTextNode.IsInList() )
            {
                const SwDoc& rDoc(mrTextNode.GetDoc());
                mrTextNode.DoNum(
                    [&rDoc](SwNodeNum & rNum) {
                        rNum.InvalidateMe();
                        rNum.NotifyInvalidSiblings(rDoc);
                    });
            }

            if (mbUpdateListCount && mrTextNode.IsInList() && HasNumberingWhichNeedsLayoutUpdate(mrTextNode))
            {
                // Repaint all text frames that belong to this numbering to avoid outdated generated
                // numbers.
                const SwDoc& rDoc(mrTextNode.GetDoc());
                mrTextNode.DoNum(
                    [&rDoc](SwNodeNum & rNum) { rNum.InvalidateAndNotifyTree(rDoc); });
            }
        }

        // #i70748#
        if (!mbOutlineLevelSet)
            return;

        mrTextNode.GetNodes().UpdateOutlineNode(mrTextNode);
        if (mrTextNode.GetAttrOutlineLevel() == 0)
        {
            mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
        }
        else
        {
            if ( mrTextNode.GetSwAttrSet().GetItemState( RES_PARATR_NUMRULE )
                                                            != SfxItemState::SET )
            {
                mrTextNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
            }
        }
    }
    // End of class <HandleSetAttrAtTextNode>
}

bool SwTextNode::SetAttr( const SfxPoolItem& pItem )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleSetAttrAtTextNode aHandleSetAttr( *this, pItem );

    bool bRet = SwContentNode::SetAttr( pItem );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

bool SwTextNode::SetAttr( const SfxItemSet& rSet )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleSetAttrAtTextNode aHandleSetAttr( *this, rSet );

    bool bRet = SwContentNode::SetAttr( rSet );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

void SwTextNode::SetInSwUndo(bool bInUndo)
{
    m_bInUndo = bInUndo;
}

namespace {
    // Helper class for special handling of resetting attributes at text node:
    // In constructor an instance of the helper class recognize whose attributes
    // are reset and perform corresponding actions before the intrinsic reset of
    // attributes has been taken place.
    // In the destructor - after the attributes have been reset at the text
    // node - corresponding actions are performed.
    // The following is handled:
    // (1) When the list style attribute - RES_PARATR_NUMRULE - is reset,
    //     the text is removed from its list before the attributes have been reset.
    // (2) When the list id attribute - RES_PARATR_LIST_ID - is reset,
    //     the text is removed from its list before the attributes have been reset.
    // (3) Notify list tree, if list level - RES_PARATR_LIST_LEVEL - is reset.
    // (4) Notify list tree, if list restart - RES_PARATR_LIST_ISRESTART - is reset.
    // (5) Notify list tree, if list restart value - RES_PARATR_LIST_RESTARTVALUE - is reset.
    // (6) Notify list tree, if count in list - RES_PARATR_LIST_ISCOUNTED - is reset.
    // (7) Reset empty list style, if outline level attribute - RES_PARATR_OUTLINELEVEL - is reset.
    class HandleResetAttrAtTextNode
    {
        public:
            HandleResetAttrAtTextNode( SwTextNode& rTextNode,
                                      const sal_uInt16 nWhich1,
                                      sal_uInt16 nWhich2 );
            HandleResetAttrAtTextNode( SwTextNode& rTextNode,
                                      const std::vector<sal_uInt16>& rWhichArr );
            explicit HandleResetAttrAtTextNode( SwTextNode& rTextNode );

            ~HandleResetAttrAtTextNode() COVERITY_NOEXCEPT_FALSE;

        private:
            SwTextNode& mrTextNode;
            bool mbListStyleOrIdReset;
            bool mbUpdateListLevel;
            bool mbUpdateListRestart;
            bool mbUpdateListCount;

            void init( sal_uInt16 nWhich, bool& rbRemoveFromList );
    };

    HandleResetAttrAtTextNode::HandleResetAttrAtTextNode( SwTextNode& rTextNode,
                                                        const sal_uInt16 nWhich1,
                                                        sal_uInt16 nWhich2 )
        : mrTextNode( rTextNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        if ( nWhich2 < nWhich1 )
            nWhich2 = nWhich1;
        bool bRemoveFromList( false );
        for ( sal_uInt16 nWhich = nWhich1; nWhich <= nWhich2; ++nWhich )
            init( nWhich, bRemoveFromList );
        if ( bRemoveFromList && mrTextNode.IsInList() )
            mrTextNode.RemoveFromList();
    }

    HandleResetAttrAtTextNode::HandleResetAttrAtTextNode( SwTextNode& rTextNode,
                                                        const std::vector<sal_uInt16>& rWhichArr )
        : mrTextNode( rTextNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        bool bRemoveFromList( false );
        for ( sal_uInt16 nWhich : rWhichArr )
            init( nWhich, bRemoveFromList );
        if ( bRemoveFromList && mrTextNode.IsInList() )
            mrTextNode.RemoveFromList();
    }

    HandleResetAttrAtTextNode::HandleResetAttrAtTextNode( SwTextNode& rTextNode )
        : mrTextNode( rTextNode ),
          mbListStyleOrIdReset( true ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        if ( rTextNode.IsInList() )
        {
            rTextNode.RemoveFromList();
        }
        // #i70748#
        mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
    }

    void HandleResetAttrAtTextNode::init( sal_uInt16 rWhich, bool& rbRemoveFromList )
    {
        if ( rWhich == RES_PARATR_NUMRULE )
        {
            rbRemoveFromList = rbRemoveFromList ||
                              mrTextNode.GetNumRule() != nullptr;
            mbListStyleOrIdReset = true;
        }
        else if ( rWhich == RES_PARATR_LIST_ID )
        {
            rbRemoveFromList = rbRemoveFromList ||
                ( mrTextNode.GetpSwAttrSet() &&
                  mrTextNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SfxItemState::SET );
            mbListStyleOrIdReset = true;
        }
        else if ( rWhich == RES_PARATR_OUTLINELEVEL )
            mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
        else if ( rWhich == RES_BACKGROUND )
            mrTextNode.ResetAttr( XATTR_FILL_FIRST, XATTR_FILL_LAST );

        if ( !rbRemoveFromList )
        {
            // RES_PARATR_LIST_LEVEL
            mbUpdateListLevel = mbUpdateListLevel ||
                                ( rWhich == RES_PARATR_LIST_LEVEL &&
                                  mrTextNode.HasAttrListLevel() );

            // RES_PARATR_LIST_ISRESTART and RES_PARATR_LIST_RESTARTVALUE
            mbUpdateListRestart = mbUpdateListRestart ||
                                  ( rWhich == RES_PARATR_LIST_ISRESTART &&
                                    mrTextNode.IsListRestart() ) ||
                                  ( rWhich == RES_PARATR_LIST_RESTARTVALUE &&
                                    mrTextNode.HasAttrListRestartValue() );

            // RES_PARATR_LIST_ISCOUNTED
            mbUpdateListCount = mbUpdateListCount ||
                                ( rWhich == RES_PARATR_LIST_ISCOUNTED &&
                                  !mrTextNode.IsCountedInList() );
        }
    }

    HandleResetAttrAtTextNode::~HandleResetAttrAtTextNode() COVERITY_NOEXCEPT_FALSE
    {
        if ( mbListStyleOrIdReset && !mrTextNode.IsInList() )
        {
            // check, if in spite of the reset of the list style or the list id
            // the paragraph still has to be added to a list.
            if (mrTextNode.GetNumRule() && !mrTextNode.GetListId().isEmpty())
            {
                // #i96062#
                // If paragraph has no list level attribute set and list style
                // is the outline style, apply outline level as the list level.
                if ( !mrTextNode.HasAttrListLevel() &&
                     mrTextNode.GetNumRule()->GetName()==SwNumRule::GetOutlineRuleName() &&
                     mrTextNode.GetTextColl()->IsAssignedToListLevelOfOutlineStyle() )
                {
                    int nNewListLevel = mrTextNode.GetTextColl()->GetAssignedOutlineStyleLevel();
                    if ( 0 <= nNewListLevel && nNewListLevel < MAXLEVEL )
                    {
                        mrTextNode.SetAttrListLevel( nNewListLevel );
                    }
                }
                mrTextNode.AddToList();
            }
            // #i70748#
            // #i105562#
            else
            {
                if (mrTextNode.GetpSwAttrSet()
                    && mrTextNode.GetAttr(RES_PARATR_OUTLINELEVEL, false).GetValue() > 0)
                {
                    mrTextNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                }
            }
        }

        if ( !mrTextNode.IsInList() )
            return;

        // just incredibly slow to do this
        if (comphelper::IsFuzzing())
            return;

        if ( mbUpdateListLevel )
        {
            auto const nLevel(mrTextNode.GetAttrListLevel());
            const SwDoc& rDoc(mrTextNode.GetDoc());
            mrTextNode.DoNum(
                [nLevel, &rDoc](SwNodeNum & rNum) { rNum.SetLevelInListTree(nLevel, rDoc); });
        }

        if ( mbUpdateListRestart )
        {
            const SwDoc& rDoc(mrTextNode.GetDoc());
            mrTextNode.DoNum(
                [&rDoc](SwNodeNum & rNum) {
                    rNum.InvalidateMe();
                    rNum.NotifyInvalidSiblings(rDoc);
                });
        }

        if ( mbUpdateListCount )
        {
            const SwDoc& rDoc(mrTextNode.GetDoc());
            mrTextNode.DoNum(
                [&rDoc](SwNodeNum & rNum) { rNum.InvalidateAndNotifyTree(rDoc); });
        }
    }
    // End of class <HandleResetAttrAtTextNode>
}

bool SwTextNode::ResetAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleResetAttrAtTextNode aHandleResetAttr( *this, nWhich1, nWhich2 );

    bool bRet = SwContentNode::ResetAttr( nWhich1, nWhich2 );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

bool SwTextNode::ResetAttr( const std::vector<sal_uInt16>& rWhichArr )
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleResetAttrAtTextNode aHandleResetAttr( *this, rWhichArr );

    bool bRet = SwContentNode::ResetAttr( rWhichArr );

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return bRet;
}

sal_uInt16 SwTextNode::ResetAllAttr()
{
    const bool bOldIsSetOrResetAttr( mbInSetOrResetAttr );
    mbInSetOrResetAttr = true;

    HandleResetAttrAtTextNode aHandleResetAttr( *this );

    const sal_uInt16 nRet = SwContentNode::ResetAllAttr();

    mbInSetOrResetAttr = bOldIsSetOrResetAttr;

    return nRet;
}

void SwTextNode::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextNode"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(sal_Int32(GetIndex())).getStr()));

    OUString sText = GetText();
    for (int i = 0; i < 32; ++i)
        sText = sText.replace(i, '*');
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("m_Text"));
    (void)xmlTextWriterWriteString(pWriter, BAD_CAST(sText.toUtf8().getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    if (GetFormatColl())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextFormatColl"));
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetFormatColl()->GetName().toString().toUtf8().getStr()));
        (void)xmlTextWriterEndElement(pWriter);
    }

    if (HasSwAttrSet())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwAttrSet"));
        GetSwAttrSet().dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }

    if (HasHints())
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwpHints"));
        const SwpHints& rHints = GetSwpHints();
        for (size_t i = 0; i < rHints.Count(); ++i)
            rHints.Get(i)->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

sal_uInt32 SwTextNode::GetRsid( sal_Int32 nStt, sal_Int32 nEnd ) const
{
    SfxItemSetFixed<RES_CHRATR_RSID, RES_CHRATR_RSID> aSet( const_cast<SwAttrPool&>((GetDoc().GetAttrPool())) );
    if (GetParaAttr(aSet, nStt, nEnd))
    {
        const SvxRsidItem* pRsid = aSet.GetItem<SvxRsidItem>(RES_CHRATR_RSID);
        if( pRsid )
            return pRsid->GetValue();
    }

    return 0;
}

sal_uInt32 SwTextNode::GetParRsid() const
{
    return reinterpret_cast<const SvxRsidItem&>(GetAttr( RES_PARATR_RSID )).GetValue();
}

bool SwTextNode::CompareParRsid( const SwTextNode &rTextNode ) const
{
    sal_uInt32 nThisRsid = GetParRsid();
    sal_uInt32 nRsid = rTextNode.GetParRsid();

    return nThisRsid == nRsid;
}

bool SwTextNode::CompareRsid( const SwTextNode &rTextNode, sal_Int32 nStt1, sal_Int32 nStt2 ) const
{
    sal_uInt32 nThisRsid = GetRsid( nStt1, nStt1 );
    sal_uInt32 nRsid = rTextNode.GetRsid( nStt2, nStt2 );

    return nThisRsid == nRsid;
}

// sw::Metadatable
::sfx2::IXmlIdRegistry& SwTextNode::GetRegistry()
{
    return GetDoc().GetXmlIdRegistry();
}

bool SwTextNode::IsInClipboard() const
{
    return GetDoc().IsClipBoard();
}

bool SwTextNode::IsInUndo() const
{
    return GetDoc().GetIDocumentUndoRedo().IsUndoNodes(GetNodes());
}

bool SwTextNode::IsInContent() const
{
    return !GetDoc().IsInHeaderFooter( *this );
}

void SwTextNode::HandleNonLegacyHint(const SfxHint& rHint)
{
    assert(!dynamic_cast<const sw::LegacyModifyHint*>(&rHint));
    sw::TextNodeNotificationSuppressor(*this);
    CallSwClientNotify(rHint);

    SwDoc& rDoc = GetDoc();
    // #125329# - assure that text node is in document nodes array
    if ( !rDoc.IsInDtor() && &rDoc.GetNodes() == &GetNodes() )
    {
        rDoc.GetNodes().UpdateOutlineNode(*this);
    }
}

void SwTextNode::UpdateDocPos(const SwTwips nDocPos, const sal_uInt32 nIndex)
{
    const sw::DocPosUpdateAtIndex aHint(nDocPos, *this, nIndex);
    CallSwClientNotify(aHint);
}

void SwTextNode::TriggerNodeUpdate(const sw::LegacyModifyHint& rHint)
{
    const auto pOldValue = rHint.m_pOld;
    const auto pNewValue = rHint.m_pNew;
    {
        sw::TextNodeNotificationSuppressor(*this);

        if ( !mbInSetOrResetAttr )
        {
            HandleModifyAtTextNode( *this, pOldValue, pNewValue );
        }

        SwContentNode::SwClientNotify(*this, rHint);

        SwDoc& rDoc = GetDoc();
        // #125329# - assure that text node is in document nodes array
        if ( !rDoc.IsInDtor() && &rDoc.GetNodes() == &GetNodes() )
        {
            rDoc.GetNodes().UpdateOutlineNode(*this);
        }
    }
}

void SwTextNode::TriggerNodeUpdate(const SfxHint& rHint)
{
    sw::TextNodeNotificationSuppressor(*this);

    SwContentNode::SwClientNotify(*this, rHint);

    SwDoc& rDoc = GetDoc();
    // #125329# - assure that text node is in document nodes array
    if ( !rDoc.IsInDtor() && &rDoc.GetNodes() == &GetNodes() )
    {
        rDoc.GetNodes().UpdateOutlineNode(*this);
    }
}

void SwTextNode::TriggerNodeUpdate(const sw::AttrSetChangeHint& rHint)
{
    const SwAttrSetChg* pOldValue = rHint.m_pOld;
    const SwAttrSetChg* pNewValue = rHint.m_pNew;
    {
        sw::TextNodeNotificationSuppressor(*this);

        // reset fill information
        if (maFillAttributes && pNewValue)
        {
            bool bReset(false);

            // ..on ItemChange from DrawingLayer FillAttributes
            SfxItemIter aIter(*pNewValue->GetChgSet());

            for(const SfxPoolItem* pItem = aIter.GetCurItem(); pItem && !bReset; pItem = aIter.NextItem())
            {
                bReset = !IsInvalidItem(pItem) && pItem->Which() >= XATTR_FILL_FIRST && pItem->Which() <= XATTR_FILL_LAST;
            }

            if(bReset)
            {
                maFillAttributes.reset();
            }
        }

        if ( !mbInSetOrResetAttr )
        {
            HandleModifyAtTextNode( *this, pOldValue, pNewValue );
        }

        SwContentNode::SwClientNotify(*this, rHint);

        SwDoc& rDoc = GetDoc();
        // #125329# - assure that text node is in document nodes array
        if ( !rDoc.IsInDtor() && &rDoc.GetNodes() == &GetNodes() )
        {
            rDoc.GetNodes().UpdateOutlineNode(*this);
        }
    }
}

void SwTextNode::TriggerNodeUpdate(const SwFormatChangeHint& rHint)
{
    assert(!rHint.m_pOldFormat || dynamic_cast<const SwTextFormatColl*>(rHint.m_pOldFormat));
    assert(!rHint.m_pNewFormat || dynamic_cast<const SwTextFormatColl*>(rHint.m_pNewFormat));
    const SwTextFormatColl* pTxtFmtColOld = static_cast<const SwTextFormatColl*>(
            rHint.m_pOldFormat);
    const SwTextFormatColl* pTxtFmtColNew = static_cast<const SwTextFormatColl*>(
            rHint.m_pNewFormat);
    {
        sw::TextNodeNotificationSuppressor(*this);

        // Override Modify so that deleting styles works properly (outline
        // numbering!).
        // Never call ChgTextCollUpdateNum for Nodes in Undo.
        if( GetRegisteredIn() == rHint.m_pNewFormat
                && GetNodes().IsDocNodes() )
        {
            if (pTxtFmtColOld)
            {
                ChgTextCollUpdateNum(pTxtFmtColOld, pTxtFmtColNew);
            }
        }

        // reset fill information
        if (maFillAttributes)
        {
            maFillAttributes.reset();
        }

        if ( !mbInSetOrResetAttr )
        {
            HandleModifyAtTextNodeFormatChange( *this );
        }

        SwContentNode::SwClientNotify(*this, rHint);

        SwDoc& rDoc = GetDoc();
        // #125329# - assure that text node is in document nodes array
        if ( !rDoc.IsInDtor() && &rDoc.GetNodes() == &GetNodes() )
        {
            rDoc.GetNodes().UpdateOutlineNode(*this);
        }
    }
}

void SwTextNode::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    if(rHint.GetId() == SfxHintId::SwAutoFormatUsedHint)
    {
        static_cast<const sw::AutoFormatUsedHint&>(rHint).CheckNode(this);
    }
    else if(SfxHintId::SwRemoveUnoObject == rHint.GetId())
    {
        TriggerNodeUpdate(static_cast<const sw::RemoveUnoObjectHint&>(rHint));
    }
    else if (rHint.GetId() == SfxHintId::SwObjectDying)
    {
        auto pDyingHint = static_cast<const sw::ObjectDyingHint*>(&rHint);
        TriggerNodeUpdate(*pDyingHint);
    }
    else if (rHint.GetId() == SfxHintId::SwLegacyModify)
    {
        auto pLegacyHint = static_cast<const sw::LegacyModifyHint*>(&rHint);
        TriggerNodeUpdate(*pLegacyHint);
    }
    else if (rHint.GetId() == SfxHintId::SwUpdateAttr)
    {
        auto pUpdateHint = static_cast<const sw::UpdateAttrHint*>(&rHint);
        TriggerNodeUpdate(*pUpdateHint);
    }
    else if (rHint.GetId() == SfxHintId::SwAttrSetChange)
    {
        auto pChangeHint = static_cast<const sw::AttrSetChangeHint*>(&rHint);
        TriggerNodeUpdate(*pChangeHint);
    }
    else if (rHint.GetId() == SfxHintId::SwFormatChange)
    {
        auto pChangeHint = static_cast<const SwFormatChangeHint*>(&rHint);
        TriggerNodeUpdate(*pChangeHint);
    }
    else if (rHint.GetId() == SfxHintId::SwVirtPageNumHint)
    {
        CallSwClientNotify(rHint);
    }
    else if (rHint.GetId() == SfxHintId::SwAttr)
    {
        if (&rModify == GetRegisteredIn())
            ChkCondColl();
    }
}

uno::Reference< rdf::XMetadatable >
SwTextNode::MakeUnoObject()
{
    const rtl::Reference<SwXParagraph> xMeta(
            SwXParagraph::CreateXParagraph(GetDoc(), this, nullptr));
    return xMeta;
}

drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwTextNode::getSdrAllFillAttributesHelper() const
{
    // create SdrAllFillAttributesHelper on demand
    if(!maFillAttributes)
    {
        const_cast< SwTextNode* >(this)->maFillAttributes = std::make_shared<drawinglayer::attribute::SdrAllFillAttributesHelper>(GetSwAttrSet());
    }

    return maFillAttributes;
}

void SwTextNode::SetXParagraph(rtl::Reference<SwXParagraph> const & xParagraph)
{
    m_wXParagraph = xParagraph.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
