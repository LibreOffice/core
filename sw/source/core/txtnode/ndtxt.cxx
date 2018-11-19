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

#include <comphelper/lok.hxx>
#include <comphelper/string.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/rsiditem.hxx>
#include <editeng/tstpitem.hxx>
#include <svl/urihelper.hxx>
#include <svl/ctloptions.hxx>
#include <sal/log.hxx>
#include <swmodule.hxx>
#include <txtfld.hxx>
#include <txtinet.hxx>
#include <fmtinfmt.hxx>
#include <fmtpdsc.hxx>
#include <txtatr.hxx>
#include <fmtrfmrk.hxx>
#include <txttxmrk.hxx>
#include <fchrfmt.hxx>
#include <txtftn.hxx>
#include <fmtflcnt.hxx>
#include <fmtfld.hxx>
#include <frmatr.hxx>
#include <charatr.hxx>
#include <ftnidx.hxx>
#include <ftninfo.hxx>
#include <fmtftn.hxx>
#include <fmtmeta.hxx>
#include <charfmt.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentListsAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <pam.hxx>
#include <fldbas.hxx>
#include <paratr.hxx>
#include <txtfrm.hxx>
#include <ftnfrm.hxx>
#include <ftnboss.hxx>
#include <rootfrm.hxx>
#include <pagedesc.hxx>
#include <expfld.hxx>
#include <section.hxx>
#include <mvsave.hxx>
#include <swcache.hxx>
#include <SwGrammarMarkUp.hxx>
#include <dcontact.hxx>
#include <redline.hxx>
#include <doctxm.hxx>
#include <IMark.hxx>
#include <scriptinfo.hxx>
#include <istyleaccess.hxx>
#include <SwStyleNameMapper.hxx>
#include <numrule.hxx>
#include <swtable.hxx>
#include <docsh.hxx>
#include <SwNodeNum.hxx>
#include <svl/intitem.hxx>
#include <list.hxx>
#include <sortedobjs.hxx>
#include <calbck.hxx>
#include <attrhint.hxx>
#include <memory>
#include <unoparagraph.hxx>
#include <wrtsh.hxx>
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <svl/itemiter.hxx>

using namespace ::com::sun::star;

typedef std::vector<SwTextAttr*> SwpHts;


// unfortunately everyone can change Hints without ensuring order or the linking between them
#ifdef DBG_UTIL
#define CHECK_SWPHINTS(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc()->IsInReading() ) \
                                  pNd->GetpSwpHints()->Check(true); }
#define CHECK_SWPHINTS_IF_FRM(pNd)  { if( pNd->GetpSwpHints() && \
                                   !pNd->GetDoc()->IsInReading() ) \
    pNd->GetpSwpHints()->Check(getLayoutFrame(nullptr, nullptr, nullptr) != nullptr); }
#else
#define CHECK_SWPHINTS(pNd)
#define CHECK_SWPHINTS_IF_FRM(pNd)
#endif

SwTextNode *SwNodes::MakeTextNode( const SwNodeIndex & rWhere,
                                 SwTextFormatColl *pColl, bool const bNewFrames)
{
    OSL_ENSURE( pColl, "Collection pointer is 0." );

    SwTextNode *pNode = new SwTextNode( rWhere, pColl, nullptr );

    SwNodeIndex aIdx( *pNode );

    // call method <UpdateOutlineNode(..)> only for the document nodes array
    if ( IsDocNodes() )
        UpdateOutlineNode(*pNode);

    // if there is no layout or it is in a hidden section, MakeFrames is not needed
    const SwSectionNode* pSectNd;
    if (!bNewFrames ||
        !GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() ||
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
                SwNodeIndex aTmpIdx( *pNode );
                pNd = FindPrvNxtFrameNode( aTmpIdx, pNode );
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
            SAL_FALLTHROUGH;
        default:
            if( rWhere == aTmp )
                aTmp -= 2;
            else
                return pNode;
            break;
        }
    } while( true );
}

SwTextNode::SwTextNode( const SwNodeIndex &rWhere, SwTextFormatColl *pTextColl, const SfxItemSet* pAutoAttr )
:   SwContentNode( rWhere, SwNodeType::Text, pTextColl ),
    mpNodeNum( nullptr ),
    mpNodeNumRLHidden(nullptr),
    m_Text(),
    m_pParaIdleData_Impl(nullptr),
    m_bContainsHiddenChars(false),
    m_bHiddenCharsHidePara(false),
    m_bRecalcHiddenCharFlags(false),
    m_bLastOutlineState( false ),
    m_bNotifiable( false ),
    mbEmptyListStyleSetDueToSetOutlineLevelAttr( false ),
    mbInSetOrResetAttr( false ),
    m_pNumStringCache(),
    m_wXParagraph(),
    maFillAttributes()
{
    InitSwParaStatistics( true );

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
    GetNodes().UpdateOutlineNode(*this);

    m_bNotifiable = true;

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

    InitSwParaStatistics( false );
    DelFrames(nullptr); // must be called here while it's still a SwTextNode
    DelFrames_TextNodePart();
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
    // fdo#52028: ODF file import does not result in MergePortions being called
    // for every attribute, since that would be inefficient.  So call it here.
    FileLoadedInitHints();
    SwContentFrame *pFrame = new SwTextFrame( this, pSib );
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
    if( pSwpHints && rNode.GetDoc()->getIDocumentLayoutAccess().GetCurrentViewShell() )
    {
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
                    pNd = pFrame->GetAttrSet()->GetDoc()->
                            GetNodes().GoNextSection( &aIdx, true, false );
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
    for (sal_uLong nIndex = rSecondNode.GetIndex() + 1; ; ++nIndex)
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
                SwFrameFormat & rFormat(pObj->GetFrameFormat());
                SwFormatAnchor const& rAnchor(rFormat.GetAnchor());
                if (rFirstNode.GetIndex() < rAnchor.GetContentAnchor()->nNode.GetIndex())
                {
                    // move it to the new frame of "this"
                    rFormat.NotifyClients(&rAnchor, &rAnchor);
                    // note pObjs will be deleted if it becomes empty
                    assert(!pFrame->GetDrawObjs() || !pObjs->Contains(*pObj));
                }
            }
        }
    }
}

} // namespace

SwTextNode *SwTextNode::SplitContentNode(const SwPosition & rPos,
        std::function<void (SwTextNode *, sw::mark::RestoreMode)> const*const pContentIndexRestore)
{
    bool isHide(false);
    SwNode::Merge const eOldMergeFlag(GetRedlineMergeFlag());
    bool parentIsOutline = IsOutline();

    // create a node "in front" of me
    const sal_Int32 nSplitPos = rPos.nContent.GetIndex();
    const sal_Int32 nTextLen = m_Text.getLength();
    SwTextNode* const pNode =
        MakeNewTextNode( rPos.nNode, false, nSplitPos==nTextLen );

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

    if ( HasWriterListeners() && !m_Text.isEmpty() && (nTextLen / 2) < nSplitPos )
    {
        // optimization for SplitNode: If a split is at the end of a node then
        // move the frames from the current to the new one and create new ones
        // for the current one. As a result, no need for recreating the layout.

        LockModify();   // disable notifications

        // If fly frames are moved, they don't need to destroy their layout
        // frames.  Set a flag that is checked in SwTextFlyCnt::SetAnchor.
        if ( HasHints() )
        {
            pNode->GetOrCreateSwpHints().SetInSplitNode(true);
        }

        // Move the first part of the content to the new node and delete
        // it in the old node.
        SwIndex aIdx( this );
        CutText( pNode, aIdx, nSplitPos );

        if( GetWrong() )
        {
            pNode->SetWrong( GetWrong()->SplitList( nSplitPos ) );
        }
        SetWrongDirty(WrongState::TODO);

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
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::NonFlys);
        }
        if (eOldMergeFlag != SwNode::Merge::None)
        {   // clear before making frames and before RegisterToNode
            SetRedlineMergeFlag(SwNode::Merge::None);
        }   // now RegisterToNode will set merge flags in both nodes properly!

        std::vector<SwTextFrame*> frames;
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
        for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
        {
            if (pFrame->getRootFrame()->IsHideRedlines())
            {
                isHide = true;
            }
            frames.push_back(pFrame);
        }
        for (SwTextFrame * pFrame : frames)
        {
            pFrame->RegisterToNode( *pNode );
            if (!pFrame->IsFollow() && pFrame->GetOfst())
            {
                pFrame->SetOfst( TextFrameIndex(0) );
            }
        }

        if ( IsInCache() )
        {
            SwFrame::GetCache().Delete( this );
            SetInCache( false );
        }

        UnlockModify(); // enable notify again

        // If there is an accessible layout we must call modify even
        // with length zero, because we have to notify about the changed
        // text node.
        const SwRootFrame *pRootFrame;
        if ( (nTextLen != nSplitPos) ||
            ( (pRootFrame = pNode->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()) != nullptr &&
              pRootFrame->IsAnyShellAccessible() ) )
        {
            // tell the frames that something was "deleted" at the end
            if( 1 == nTextLen - nSplitPos )
            {
                SwDelChr aHint( nSplitPos );
                pNode->NotifyClients( nullptr, &aHint );
            }
            else
            {
                SwDelText aHint( nSplitPos, nTextLen - nSplitPos );
                pNode->NotifyClients( nullptr, &aHint );
            }
        }
        if ( HasHints() )
        {
            MoveTextAttr_To_AttrSet();
        }
        // in case there are frames, the RegisterToNode has set the merge flag
        pNode->MakeFramesForAdjacentContentNode(*this);
        lcl_ChangeFootnoteRef( *this );
        if (pContentIndexRestore)
        {   // call after making frames; listeners will take care of adding to the right frame
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::Flys);
        }
        if (eOldMergeFlag != SwNode::Merge::None)
        {
            MoveMergedFlysAndFootnotes(frames, *pNode, *this, true);
        }
    }
    else
    {
        SwWrongList *pList = GetWrong();
        SetWrong( nullptr, false );
        SetWrongDirty(WrongState::TODO);

        SwGrammarMarkUp *pList3 = GetGrammarCheck();
        SetGrammarCheck( nullptr, false );
        SetGrammarCheckDirty( true );

        SetWordCountDirty( true );

        SwWrongList *pList2 = GetSmartTags();
        SetSmartTags( nullptr, false );
        SetSmartTagDirty( true );

        SwIndex aIdx( this );
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
        }

        if( pList )
        {
            pNode->SetWrong( pList->SplitList( nSplitPos ) );
            SetWrong( pList, false );
        }

        if( pList3 )
        {
            pNode->SetGrammarCheck( pList3->SplitGrammarList( nSplitPos ) );
            SetGrammarCheck( pList3, false );
        }

        if( pList2 )
        {
            pNode->SetSmartTags( pList2->SplitList( nSplitPos ) );
            SetSmartTags( pList2, false );
        }

        if (pContentIndexRestore)
        {   // call before making frames and before RegisterToNode
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::NonFlys);
        }

        std::vector<SwTextFrame*> frames;
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*this);
        for (SwTextFrame * pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
        {
            frames.push_back(pFrame);
            if (pFrame->getRootFrame()->IsHideRedlines())
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
                if (pFrame->GetText().isEmpty())
                {
                    // turns out it's empty - in this case, it was not
                    // invalidated because Cut didn't sent it any hints,
                    // so we have to invalidate it here!
                    pFrame->Prepare(PREP_CLEAR, nullptr, false);
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
            (*pContentIndexRestore)(pNode, sw::mark::RestoreMode::Flys);
        }

        if (bRecreateThis)
        {
            MoveMergedFlysAndFootnotes(frames, *pNode, *this, true);
        }
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
        const SfxPoolItem *pItem;
        if( HasWriterListeners() && SfxItemState::SET == pNode->GetSwAttrSet().
            GetItemState( RES_PAGEDESC, true, &pItem ) )
        {
            pNode->ModifyNotification( pItem, pItem );
        }
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

        if( !pHt->IsDontMoveAttr() &&
            SetAttr( pHt->GetAttr() ) )
        {
            m_pSwpHints->DeleteAtPos(i);
            DestroyAttr( pHt );
            --i;
        }
    }

}

namespace sw {

/// if first node is deleted & second survives, then the first node's frame
/// will be deleted too; prevent this by moving the frame to the second node
/// if necessary.
void MoveDeletedPrevFrames(SwTextNode & rDeletedPrev, SwTextNode & rNode)
{
    std::vector<SwTextFrame*> frames;
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(rDeletedPrev);
    for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
    {
        frames.push_back(pFrame);
    }
    {
        auto frames2(frames);
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIt(rNode);
        for (SwTextFrame* pFrame = aIt.First(); pFrame; pFrame = aIt.Next())
        {
            auto const it(std::find(frames2.begin(), frames2.end(), pFrame));
            assert(it != frames2.end());
            frames2.erase(it);
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
        if (eRecreateMerged == sw::Recreate::Predecessor)
        {
            for (sal_uLong i = rNode.GetIndex() - 1; ; --i)
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
            if (pFrame->getRootFrame()->IsHideRedlines())
            {
                frames.push_back(pFrame);
            }
        }
        for (SwTextFrame * pFrame : frames)
        {
            SwTextNode & rFirstNode(pFrame->GetMergedPara()
                ? *pFrame->GetMergedPara()->pFirstNode
                : *pMergeNode);
            assert(rFirstNode.GetIndex() <= rNode.GetIndex());
            pFrame->SetMergedPara(sw::CheckParaRedlineMerge(
                        *pFrame, rFirstNode, sw::FrameMode::Existing));
            assert(pFrame->GetMergedPara());
            assert(pFrame->GetMergedPara()->listener.IsListeningTo(&rNode));
            assert(rNode.GetIndex() <= pFrame->GetMergedPara()->pLastNode->GetIndex());
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
            else if (pFrame->getRootFrame()->IsHideRedlines())
            {
                rNode.SetRedlineMergeFlag(SwNode::Merge::None);
                break; // checking once is enough
            }
        }
    }
}

} // namespace

SwContentNode *SwTextNode::JoinNext()
{
    SwNodes& rNds = GetNodes();
    SwNodeIndex aIdx( *this );
    if( SwContentNode::CanJoinNext( &aIdx ) )
    {
        SwDoc* pDoc = rNds.GetDoc();
        const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
        pContentStore->Save(pDoc, aIdx.GetIndex(), SAL_MAX_INT32);
        SwTextNode *pTextNode = aIdx.GetNode().GetTextNode();
        sal_Int32 nOldLen = m_Text.getLength();

        // METADATA: merge
        JoinMetadatable(*pTextNode, !Len(), !pTextNode->Len());

        SwWrongList *pList = GetWrong();
        if( pList )
        {
            pList->JoinList( pTextNode->GetWrong(), nOldLen );
            SetWrongDirty(WrongState::TODO);
            SetWrong( nullptr, false );
        }
        else
        {
            pList = pTextNode->GetWrong();
            if( pList )
            {
                pList->Move( 0, nOldLen );
                SetWrongDirty(WrongState::TODO);
                pTextNode->SetWrong( nullptr, false );
            }
        }

        SwGrammarMarkUp *pList3 = GetGrammarCheck();
        if( pList3 )
        {
            pList3->JoinGrammarList( pTextNode->GetGrammarCheck(), nOldLen );
            SetGrammarCheckDirty( true );
            SetGrammarCheck( nullptr, false );
        }
        else
        {
            pList3 = pTextNode->GetGrammarCheck();
            if( pList3 )
            {
                pList3->MoveGrammar( 0, nOldLen );
                SetGrammarCheckDirty( true );
                pTextNode->SetGrammarCheck( nullptr, false );
            }
        }

        SwWrongList *pList2 = GetSmartTags();
        if( pList2 )
        {
            pList2->JoinList( pTextNode->GetSmartTags(), nOldLen );
            SetSmartTagDirty( true );
            SetSmartTags( nullptr, false );
        }
        else
        {
            pList2 = pTextNode->GetSmartTags();
            if( pList2 )
            {
                pList2->Move( 0, nOldLen );
                SetSmartTagDirty( true );
                pTextNode->SetSmartTags( nullptr, false );
            }
        }

        { // scope for SwIndex
            pTextNode->CutText( this, SwIndex(pTextNode), pTextNode->Len() );
        }
        // move all Bookmarks/TOXMarks
        if( !pContentStore->Empty())
            pContentStore->Restore( pDoc, GetIndex(), nOldLen );

        if( pTextNode->HasAnyIndex() )
        {
            // move all ShellCursor/StackCursor/UnoCursor out of delete range
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nOldLen, true );
        }
        SwNode::Merge const eOldMergeFlag(pTextNode->GetRedlineMergeFlag());
        rNds.Delete(aIdx);
        SetWrong( pList, false );
        SetGrammarCheck( pList3, false );
        SetSmartTags( pList2, false );
        InvalidateNumRule();
        CheckResetRedlineMergeFlag(*this, eOldMergeFlag == SwNode::Merge::First
                                            ? sw::Recreate::ThisNode
                                            : sw::Recreate::No);
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
        SwDoc* pDoc = rNds.GetDoc();
        const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
        pContentStore->Save( pDoc, aIdx.GetIndex(), SAL_MAX_INT32);
        SwTextNode *pTextNode = aIdx.GetNode().GetTextNode();
        const sal_Int32 nLen = pTextNode->Len();

        SwWrongList *pList = pTextNode->GetWrong();
        if( pList )
        {
            pList->JoinList( GetWrong(), Len() );
            SetWrongDirty(WrongState::TODO);
            pTextNode->SetWrong( nullptr, false );
            SetWrong( nullptr );
        }
        else
        {
            pList = GetWrong();
            if( pList )
            {
                pList->Move( 0, nLen );
                SetWrongDirty(WrongState::TODO);
                SetWrong( nullptr, false );
            }
        }

        SwGrammarMarkUp *pList3 = pTextNode->GetGrammarCheck();
        if( pList3 )
        {
            pList3->JoinGrammarList( GetGrammarCheck(), Len() );
            SetGrammarCheckDirty( true );
            pTextNode->SetGrammarCheck( nullptr, false );
            SetGrammarCheck( nullptr );
        }
        else
        {
            pList3 = GetGrammarCheck();
            if( pList3 )
            {
                pList3->MoveGrammar( 0, nLen );
                SetGrammarCheckDirty( true );
                SetGrammarCheck( nullptr, false );
            }
        }

        SwWrongList *pList2 = pTextNode->GetSmartTags();
        if( pList2 )
        {
            pList2->JoinList( GetSmartTags(), Len() );
            SetSmartTagDirty( true );
            pTextNode->SetSmartTags( nullptr, false );
            SetSmartTags( nullptr );
        }
        else
        {
            pList2 = GetSmartTags();
            if( pList2 )
            {
                pList2->Move( 0, nLen );
                SetSmartTagDirty( true );
                SetSmartTags( nullptr, false );
            }
        }

        { // scope for SwIndex
            pTextNode->CutText( this, SwIndex(this), SwIndex(pTextNode), nLen );
        }
        // move all Bookmarks/TOXMarks
        if( !pContentStore->Empty() )
            pContentStore->Restore( pDoc, GetIndex() );

        if( pTextNode->HasAnyIndex() )
        {
            // move all ShellCursor/StackCursor/UnoCursor out of delete range
            pDoc->CorrAbs( aIdx, SwPosition( *this ), nLen, true );
        }
        SwNode::Merge const eOldMergeFlag(pTextNode->GetRedlineMergeFlag());
        if (eOldMergeFlag == SwNode::Merge::First)
        {
            sw::MoveDeletedPrevFrames(*pTextNode, *this);
        }
        rNds.Delete(aIdx);
        SetWrong( pList, false );
        SetGrammarCheck( pList3, false );
        SetSmartTags( pList2, false );
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
    OSL_ENSURE( !mpAttrSet.get(), "AttrSet is set after all" );
    SwAttrSet aNewAttrSet( rPool, aTextNodeSetRange );

    // put names of parent style and conditional style:
    const SwFormatColl* pAnyFormatColl = &GetAnyFormatColl();
    const SwFormatColl* pFormatColl = GetFormatColl();
    OUString sVal;
    SwStyleNameMapper::FillProgName( pAnyFormatColl->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
    SfxStringItem aAnyFormatColl( RES_FRMATR_STYLE_NAME, sVal );
    if ( pFormatColl != pAnyFormatColl )
        SwStyleNameMapper::FillProgName( pFormatColl->GetName(), sVal, SwGetPoolIdFromName::TxtColl );
    SfxStringItem aFormatColl( RES_FRMATR_CONDITIONAL_STYLE_NAME, sVal );
    aNewAttrSet.Put( aAnyFormatColl );
    aNewAttrSet.Put( aFormatColl );

    aNewAttrSet.SetParent( &pAnyFormatColl->GetAttrSet() );
    mpAttrSet = GetDoc()->GetIStyleAccess().getAutomaticStyle( aNewAttrSet, IStyleAccess::AUTO_STYLE_PARA );
}

// override SwIndexReg::Update => text hints do not need SwIndex for start/end!
void SwTextNode::Update(
    SwIndex const & rPos,
    const sal_Int32 nChangeLen,
    const bool bNegative,
    const bool bDelete )
{
    SetAutoCompleteWordDirty( true );

    std::unique_ptr<SwpHts> pCollector;
    const sal_Int32 nChangePos = rPos.GetIndex();

    if ( HasHints() )
    {
        if ( bNegative )
        {
            std::vector<SwTextInputField*> aTextInputFields;

            const sal_Int32 nChangeEnd = nChangePos + nChangeLen;
            for ( size_t n = 0; n < m_pSwpHints->Count(); ++n )
            {
                bool bTextAttrChanged = false;
                bool bStartOfTextAttrChanged = false;
                SwTextAttr * const pHint = m_pSwpHints->Get(n);
                sal_Int32 &       rStart = pHint->GetStart();
                if ( rStart > nChangePos )
                {
                    if ( rStart > nChangeEnd )
                    {
                         rStart = rStart - nChangeLen;
                    }
                    else
                    {
                         rStart = nChangePos;
                    }
                    bStartOfTextAttrChanged = true;
                }

                sal_Int32 * const pEnd = pHint->GetEnd();
                if (pEnd && *pEnd > nChangePos )
                {
                    if( *pEnd > nChangeEnd )
                    {
                        *pEnd = *pEnd - nChangeLen;
                    }
                    else
                    {
                        *pEnd = nChangePos;
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

            bool aDontExp[ coArrSz ];
            memset( &aDontExp, 0, coArrSz * sizeof(bool) );

            for ( size_t n = 0; n < m_pSwpHints->Count(); ++n )
            {
                bool bTextAttrChanged = false;
                SwTextAttr * const pHint = m_pSwpHints->Get(n);
                sal_Int32 &       rStart = pHint->GetStart();
                sal_Int32 * const pEnd = pHint->GetEnd();
                if ( rStart >= nChangePos )
                {
                    rStart = rStart + nChangeLen;
                    if ( pEnd )
                    {
                        *pEnd = *pEnd + nChangeLen;
                    }
                }
                else if ( pEnd && (*pEnd >= nChangePos) )
                {
                    if ( (*pEnd > nChangePos) || IsIgnoreDontExpand() )
                    {
                        *pEnd = *pEnd + nChangeLen;
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
                            if (!pCollector)
                            {
                               pCollector.reset( new SwpHts );
                            }
                            auto it = std::find_if(pCollector->begin(), pCollector->end(),
                                [nWhich](const SwTextAttr *pTmp) { return nWhich == pTmp->Which(); });
                            if (it != pCollector->end())
                            {
                                SwTextAttr *pTmp = *it;
                                pCollector->erase( it );
                                SwTextAttr::Destroy( pTmp, GetDoc()->GetAttrPool() );
                            }
                            SwTextAttr * const pTmp =
                            MakeTextAttr( *GetDoc(),
                                pHint->GetAttr(), nChangePos, nChangePos + nChangeLen);
                            pCollector->push_back( pTmp );
                        }
                        else
                        {
                            *pEnd = *pEnd + nChangeLen;
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
    SwIndexReg aTmpIdxReg;
    if ( !bNegative && !bDelete )
    {
        const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
        for (SwRangeRedline* pRedl : rTable)
        {
            if ( pRedl->HasMark() )
            {
                SwPosition* const pEnd = pRedl->End();
                if ( this == &pEnd->nNode.GetNode() &&
                     *pRedl->GetPoint() != *pRedl->GetMark() )
                {
                    SwIndex & rIdx = pEnd->nContent;
                    if (nChangePos == rIdx.GetIndex())
                    {
                        rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                    }
                }
            }
            else if ( this == &pRedl->GetPoint()->nNode.GetNode() )
            {
                SwIndex & rIdx = pRedl->GetPoint()->nContent;
                if (nChangePos == rIdx.GetIndex())
                {
                    rIdx.Assign( &aTmpIdxReg, rIdx.GetIndex() );
                }
                // the unused position must not be on a SwTextNode
                bool const isOneUsed(&pRedl->GetBound() == pRedl->GetPoint());
                assert(!pRedl->GetBound(!isOneUsed).nNode.GetNode().IsTextNode());
                assert(!pRedl->GetBound(!isOneUsed).nContent.GetIdxReg()); (void)isOneUsed;
            }
        }

        // Bookmarks must never grow to either side, when editing (directly) to the left or right (#i29942#)!
        // And a bookmark with same start and end must remain to the left of the inserted text (used in XML import).
        {
            bool bAtLeastOneBookmarkMoved = false;
            bool bAtLeastOneExpandedBookmarkAtInsertionPosition = false;
            // A text node already knows its marks via its SwIndexes.
            std::set<const sw::mark::IMark*> aSeenMarks;
            const SwIndex* next;
            for (const SwIndex* pIndex = GetFirstIndex(); pIndex; pIndex = next )
            {
                next = pIndex->GetNext();
                const sw::mark::IMark* pMark = pIndex->GetMark();
                if (!pMark)
                    continue;
                // Only handle bookmarks once, if they start and end at this node as well.
                if (aSeenMarks.find(pMark) != aSeenMarks.end())
                    continue;
                aSeenMarks.insert(pMark);
                const SwPosition* pEnd = &pMark->GetMarkEnd();
                SwIndex & rEndIdx = const_cast<SwIndex&>(pEnd->nContent);
                if( this == &pEnd->nNode.GetNode() &&
                    rPos.GetIndex() == rEndIdx.GetIndex() )
                {
                    if (&rEndIdx == next) // nasty corner case:
                    {   // don't switch to iterating aTmpIdxReg!
                        next = rEndIdx.GetNext();
                    }
                    rEndIdx.Assign( &aTmpIdxReg, rEndIdx.GetIndex() );
                    bAtLeastOneBookmarkMoved = true;
                }
                else if ( !bAtLeastOneExpandedBookmarkAtInsertionPosition )
                {
                    if ( pMark->IsExpanded() )
                    {
                        const SwPosition* pStart = &pMark->GetMarkStart();
                        if ( this == &pStart->nNode.GetNode()
                             && rPos.GetIndex() == pStart->nContent.GetIndex() )
                        {
                            bAtLeastOneExpandedBookmarkAtInsertionPosition = true;
                        }
                    }
                }
            }

            bSortMarks = bAtLeastOneBookmarkMoved && bAtLeastOneExpandedBookmarkAtInsertionPosition;
        }

        // at-char anchored flys shouldn't be moved, either.
#if OSL_DEBUG_LEVEL > 0
        std::vector<SwFrameFormat*> checkFormats;
        const SwFrameFormats& rFormats = *GetDoc()->GetSpzFrameFormats();
        for (auto& rpFormat : rFormats)
        {
            const SwFormatAnchor& rAnchor = rpFormat->GetAnchor();
            const SwPosition* pContentAnchor = rAnchor.GetContentAnchor();
            if (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR && pContentAnchor)
            {
                // The fly is at-char anchored and has an anchor position.
                SwIndex& rEndIdx = const_cast<SwIndex&>(pContentAnchor->nContent);
                if (&pContentAnchor->nNode.GetNode() == this && rEndIdx.GetIndex() == rPos.GetIndex())
                {
                    // The anchor position is exactly our insert position.
                    #if 0
                    rEndIdx.Assign(&aTmpIdxReg, rEndIdx.GetIndex());
                    #endif
                    checkFormats.push_back( rpFormat );
                }
            }
        }
#endif
        std::vector<SwFrameFormat*> const*const pFlys(GetAnchoredFlys());
        for (size_t i = 0; pFlys && i != pFlys->size(); ++i)
        {
            SwFrameFormat const*const pFormat = (*pFlys)[i];
            const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
            const SwPosition* pContentAnchor = rAnchor.GetContentAnchor();
            if (rAnchor.GetAnchorId() == RndStdIds::FLY_AT_CHAR && pContentAnchor)
            {
                // The fly is at-char anchored and has an anchor position.
                SwIndex& rEndIdx = const_cast<SwIndex&>(pContentAnchor->nContent);
                if (&pContentAnchor->nNode.GetNode() == this && rEndIdx.GetIndex() == rPos.GetIndex())
                {
                    // The anchor position is exactly our insert position.
                    rEndIdx.Assign(&aTmpIdxReg, rEndIdx.GetIndex());
#if OSL_DEBUG_LEVEL > 0
                    auto checkPos = std::find( checkFormats.begin(), checkFormats.end(), pFormat );
                    assert( checkPos != checkFormats.end());
                    checkFormats.erase( checkPos );
#endif
                }
            }
        }
#if OSL_DEBUG_LEVEL > 0
        assert( checkFormats.empty());
#endif

        // The cursors of other shells shouldn't be moved, either.
        if (SwDocShell* pDocShell = GetDoc()->GetDocShell())
        {
            if (pDocShell->GetWrtShell())
            {
                for (SwViewShell& rShell : pDocShell->GetWrtShell()->GetRingContainer())
                {
                    auto pWrtShell = dynamic_cast<SwWrtShell*>(&rShell);
                    if (!pWrtShell || pWrtShell == pDocShell->GetWrtShell())
                        continue;

                    SwShellCursor* pCursor = pWrtShell->GetCursor_();
                    if (!pCursor)
                        continue;

                    SwIndex& rIndex = pCursor->Start()->nContent;
                    if (&pCursor->Start()->nNode.GetNode() == this && rIndex.GetIndex() == rPos.GetIndex())
                    {
                        // The cursor position of this other shell is exactly our insert position.
                        rIndex.Assign(&aTmpIdxReg, rIndex.GetIndex());
                    }
                }
            }
        }
    }

    // base class
    SwIndexReg::Update( rPos, nChangeLen, bNegative, bDelete );

    if (pCollector)
    {
        const size_t nCount = pCollector->size();
        for ( size_t i = 0; i < nCount; ++i )
        {
            m_pSwpHints->TryInsertHint( (*pCollector)[ i ], *this );
        }
    }

    aTmpIdxReg.MoveTo( *this );
    if ( bSortMarks )
    {
        getIDocumentMarkAccess()->assureSortedMarkContainers();
    }

    //Any drawing objects anchored into this text node may be sorted by their
    //anchor position which may have changed here, so resort them
    SwContentFrame* pContentFrame = getLayoutFrame(GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout());
    SwSortedObjs* pSortedObjs = pContentFrame ? pContentFrame->GetDrawObjs() : nullptr;
    if (pSortedObjs)
        pSortedObjs->UpdateAll();

    // Update the paragraph signatures.
    if (SwEditShell* pEditShell = GetDoc()->GetEditShell())
    {
        pEditShell->ValidateCurrentParagraphSignatures(true);
    }

    // Inform LOK clients about change in position of redlines (if any)
    if (comphelper::LibreOfficeKit::isActive())
    {
        const SwRedlineTable& rTable = GetDoc()->getIDocumentRedlineAccess().GetRedlineTable();
        for (SwRedlineTable::size_type nRedlnPos = 0; nRedlnPos < rTable.size(); ++nRedlnPos)
        {
            SwRangeRedline* pRedln = rTable[nRedlnPos];
            if (pRedln->HasMark())
            {
                if (this == &pRedln->End()->nNode.GetNode() && *pRedln->GetPoint() != *pRedln->GetMark())
                {
                    // Redline is changed only when some change occurs before it
                    if (nChangePos <= pRedln->Start()->nContent.GetIndex())
                    {
                        SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, pRedln);
                    }
                }
            }
            else if (this == &pRedln->GetPoint()->nNode.GetNode())
                SwRedlineTable::LOKRedlineNotification(RedlineNotification::Modify, pRedln);
        }
    }
}

void SwTextNode::ChgTextCollUpdateNum( const SwTextFormatColl *pOldColl,
                                        const SwTextFormatColl *pNewColl)
{
    SwDoc* pDoc = GetDoc();
    OSL_ENSURE( pDoc, "No Doc?" );
    // query the OutlineLevel and if it changed, notify the Nodes-Array!
    const int nOldLevel = pOldColl && pOldColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pOldColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;
    const int nNewLevel = pNewColl && pNewColl->IsAssignedToListLevelOfOutlineStyle() ?
                     pNewColl->GetAssignedOutlineStyleLevel() : MAXLEVEL;

    if ( MAXLEVEL != nNewLevel && -1 != nNewLevel )
    {
        SetAttrListLevel(nNewLevel);
    }
    if (pDoc)
    {
        pDoc->GetNodes().UpdateOutlineNode(*this);
    }

    SwNodes& rNds = GetNodes();
    // If Level 0 (Chapter), update the footnotes!
    if( ( !nNewLevel || !nOldLevel) && pDoc && !pDoc->GetFootnoteIdxs().empty() &&
        FTNNUM_CHAPTER == pDoc->GetFootnoteInfo().eNum &&
        rNds.IsDocNodes() )
    {
        SwNodeIndex aTmpIndex( rNds, GetIndex());

        pDoc->GetFootnoteIdxs().UpdateFootnote( aTmpIndex);
    }

    if( pNewColl && RES_CONDTXTFMTCOLL == pNewColl->Which() )
    {
        // check the condition of the text node again
        ChkCondColl();
    }
}

// If positioned exactly at the end of a CharStyle or Hyperlink,
// set its DontExpand flag.
bool SwTextNode::DontExpandFormat( const SwIndex& rIdx, bool bFlag,
                                bool bFormatToTextAttributes )
{
    const sal_Int32 nIdx = rIdx.GetIndex();
    if (bFormatToTextAttributes && nIdx == m_Text.getLength())
    {
        FormatToTextAttr( this );
    }

    bool bRet = false;
    if ( HasHints() )
    {
        const size_t nEndCnt = m_pSwpHints->Count();
        size_t nPos = nEndCnt;
        while( nPos )
        {
            SwTextAttr *pTmp = m_pSwpHints->GetSortedByEnd( --nPos );
            sal_Int32 *pEnd = pTmp->GetEnd();
            if( !pEnd || *pEnd > nIdx )
                continue;
            if( nIdx != *pEnd )
                nPos = 0;
            else if( bFlag != pTmp->DontExpand() && !pTmp->IsLockExpandFlag()
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
    enum SwTextNode::GetTextAttrMode const eMode)
{
    assert(nWhich >= RES_TXTATR_BEGIN && nWhich < RES_TXTATR_END);
    size_t const nSize = pSwpHints ? pSwpHints->Count() : 0;
    sal_Int32 nPreviousIndex(0); // index of last hint with nWhich
    bool (*pMatchFunc)(sal_Int32, sal_Int32, sal_Int32)=nullptr;
    switch (eMode)
    {
        case SwTextNode::DEFAULT:   pMatchFunc = &lcl_GetTextAttrDefault; break;
        case SwTextNode::EXPAND:    pMatchFunc = &lcl_GetTextAttrExpand;  break;
        case SwTextNode::PARENT:    pMatchFunc = &lcl_GetTextAttrParent;  break;
        default: assert(false);
    }

    for( size_t i = 0; i < nSize; ++i )
    {
        SwTextAttr *const pHint = pSwpHints->Get(i);
        sal_Int32 const nHintStart = pHint->GetStart();
        if (nIndex < nHintStart)
        {
            return; // hints are sorted by start, so we are done...
        }

        if (pHint->Which() != nWhich)
        {
            continue;
        }

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
    lcl_GetTextAttrs(&ret, nullptr, m_pSwpHints.get(), nIndex, nWhich, DEFAULT);
    return ret;
}

SwTextAttr *
SwTextNode::GetTextAttrAt(sal_Int32 const nIndex, sal_uInt16 const nWhich,
                        enum GetTextAttrMode const eMode) const
{
    assert(    (nWhich == RES_TXTATR_META)
            || (nWhich == RES_TXTATR_METAFIELD)
            || (nWhich == RES_TXTATR_AUTOFMT)
            || (nWhich == RES_TXTATR_INETFMT)
            || (nWhich == RES_TXTATR_CJK_RUBY)
            || (nWhich == RES_TXTATR_UNKNOWN_CONTAINER)
            || (nWhich == RES_TXTATR_INPUTFIELD ) );
            // "GetTextAttrAt() will give wrong result for this hint!")

    SwTextAttr * pRet(nullptr);
    lcl_GetTextAttrs(nullptr, & pRet, m_pSwpHints.get(), nIndex, nWhich, eMode);
    return pRet;
}

const SwTextInputField* SwTextNode::GetOverlappingInputField( const SwTextAttr& rTextAttr ) const
{
    const SwTextInputField* pTextInputField = nullptr;

    pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextAttrAt( rTextAttr.GetStart(), RES_TXTATR_INPUTFIELD, PARENT ));

    if ( pTextInputField == nullptr && rTextAttr.End() != nullptr )
    {
        pTextInputField = dynamic_cast<const SwTextInputField*>(GetTextAttrAt( *(rTextAttr.End()), RES_TXTATR_INPUTFIELD, PARENT ));
    }

    return pTextInputField;
}

void SwTextNode::DelFrames_TextNodePart()
{
    SetWrong( nullptr );
    SetWrongDirty(WrongState::TODO);

    SetGrammarCheck( nullptr );
    SetGrammarCheckDirty( true );

    SetSmartTags( nullptr );
    SetSmartTagDirty( true );

    SetWordCountDirty( true );
    SetAutoCompleteWordDirty( true );
}

SwTextField* SwTextNode::GetFieldTextAttrAt(
    const sal_Int32 nIndex,
    const bool bIncludeInputFieldAtStart ) const
{
    SwTextField* pTextField = nullptr;

    pTextField = dynamic_cast<SwTextField*>(GetTextAttrForCharAt( nIndex, RES_TXTATR_FIELD ));
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
                bIncludeInputFieldAtStart ? DEFAULT : PARENT ));
    }

    return pTextField;
}

static SwCharFormat* lcl_FindCharFormat( const SwCharFormats* pCharFormats, const OUString& rName )
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
            // at their new ToX (SwModify).
            static_txtattr_cast<SwTextTOXMark*>(pNewHt)->CopyTOXMark(pOtherDoc);
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
                const SwDoc* const pDoc = static_txtattr_cast<
                        const SwTextINetFormat*>(pHt)->GetTextNode().GetDoc();
                if ( pDoc )
                {
                    const SwCharFormats* pCharFormats = pDoc->GetCharFormats();
                    const SwFormatINetFormat& rFormat = pHt->GetINetFormat();
                    SwCharFormat* pFormat;
                    pFormat = lcl_FindCharFormat( pCharFormats, rFormat.GetINetFormat() );
                    if( pFormat )
                        pOtherDoc->CopyCharFormat( *pFormat );
                    pFormat = lcl_FindCharFormat( pCharFormats, rFormat.GetVisitedFormat() );
                    if( pFormat )
                        pOtherDoc->CopyCharFormat( *pFormat );
                }
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
        OSL_ENSURE( pDest
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
        SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc()) ?
                pDest->GetDoc() : nullptr;

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
                             ? GetDoc()->IsCopyIsMove()
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

        pDest->ModifyNotification( nullptr, &aHint );
    }
}

/// copy text and attributes to node pDest
void SwTextNode::CopyText( SwTextNode *const pDest,
                      const SwIndex &rStart,
                      const sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    SwIndex const aIdx( pDest, pDest->m_Text.getLength() );
    CopyText( pDest, aIdx, rStart, nLen, bForceCopyOfAllAttrs );
}

void SwTextNode::CopyText( SwTextNode *const pDest,
                      const SwIndex &rDestStart,
                      const SwIndex &rStart,
                      sal_Int32 nLen,
                      const bool bForceCopyOfAllAttrs )
{
    CHECK_SWPHINTS_IF_FRM(this);
    CHECK_SWPHINTS(pDest);
    sal_Int32 nTextStartIdx = rStart.GetIndex();
    sal_Int32 nDestStart = rDestStart.GetIndex();      // remember old Pos

    if (pDest->GetDoc()->IsClipBoard() && GetNum())
    {
        // #i111677# cache expansion of source (for clipboard)
        pDest->m_pNumStringCache.reset( (nTextStartIdx != 0)
            ? new OUString // fdo#49076: numbering only if copy from para start
            : new OUString(GetNumString()));
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
                SfxItemSet aCharSet(
                    pDest->GetDoc()->GetAttrPool(),
                    svl::Items<
                        RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                        RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                        RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>{} );
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

    SwDoc* const pOtherDoc = (pDest->GetDoc() != GetDoc()) ? pDest->GetDoc() : nullptr;

    // copy hard attributes on whole paragraph
    if( HasSwAttrSet() )
    {
        // i#96213 all or just the Char attributes?
        if ( !bForceCopyOfAllAttrs &&
             ( nDestStart ||
               pDest->HasSwAttrSet() ||
               nLen != pDest->GetText().getLength()))
        {
            SfxItemSet aCharSet(
                pDest->GetDoc()->GetAttrPool(),
                svl::Items<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>{});
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
                            && GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

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
                                            ? GetDoc()->IsCopyIsMove()
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
            pNewHt = MakeTextAttr( *GetDoc(), pHt->GetAttr(),
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
            const SwIndex aIdx(pDest, pair.first);
            pDest->EraseText(aIdx, pair.second - pair.first);
        }
    }

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
                const SwIndex aIdx( pDest, pNewHt->GetStart() );
                pDest->EraseText( aIdx, 1 );
            }
        }
    }

    CHECK_SWPHINTS_IF_FRM(this);
    CHECK_SWPHINTS(pDest);
}

OUString SwTextNode::InsertText( const OUString & rStr, const SwIndex & rIdx,
        const SwInsertFlags nMode )
{
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
    m_Text = m_Text.replaceAt(aPos, 0, sInserted);
    assert(GetSpaceLeft()>=0);
    nLen = m_Text.getLength() - aPos - nLen;
    assert(nLen != 0);

    bool bOldExpFlg = IsIgnoreDontExpand();
    if (nMode & SwInsertFlags::FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( true );
    }

    Update( rIdx, nLen ); // text content changed!

    if (nMode & SwInsertFlags::FORCEHINTEXPAND)
    {
        SetIgnoreDontExpand( bOldExpFlg );
    }

    if ( HasWriterListeners() )
    {   // send this before messing with hints, which will send RES_UPDATE_ATTR
        SwInsText aHint( aPos, nLen );
        NotifyClients( nullptr, &aHint );
    }

    if ( HasHints() )
    {
        bool const bHadHints(!m_pSwpHints->CanBeDeleted());
        bool bMergePortionsNeeded(false);
        for ( size_t i = 0; i < m_pSwpHints->Count() &&
                rIdx >= m_pSwpHints->Get(i)->GetStart(); ++i )
        {
            SwTextAttr * const pHt = m_pSwpHints->Get( i );
            sal_Int32 * const pEndIdx = pHt->GetEnd();
            if( !pEndIdx )
                continue;

            if( rIdx == *pEndIdx )
            {
                if (  (nMode & SwInsertFlags::NOHINTEXPAND) ||
                    (!(nMode & SwInsertFlags::FORCEHINTEXPAND)
                     && pHt->DontExpand()) )
                {
                    // on empty attributes also adjust Start
                    if( rIdx == pHt->GetStart() )
                        pHt->GetStart() = pHt->GetStart() - nLen;
                    *pEndIdx = *pEndIdx - nLen;
                    m_pSwpHints->DeleteAtPos(i);
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
                    pHt->GetStart() = pHt->GetStart() - nLen;
                    const size_t nCurrentLen = m_pSwpHints->Count();
                    m_pSwpHints->DeleteAtPos(i);
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
                pHt->GetStart() = pHt->GetStart() - nLen;
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
            const SwIndex & rStart, const sal_Int32 nLen )
{
    assert(pDest); // Cut requires a destination
    SwIndex aDestStt(pDest, pDest->GetText().getLength());
    CutImpl( pDest, aDestStt, rStart, nLen, false );
}

void SwTextNode::CutImpl( SwTextNode * const pDest, const SwIndex & rDestStart,
         const SwIndex & rStart, sal_Int32 nLen, const bool bUpdate )
{
    assert(pDest); // Cut requires a destination

    assert(GetDoc() == pDest->GetDoc()); // must be same document

    assert(pDest != this); // destination must be different node

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
                        m_Text.copy(nTextStartIdx, nLen));
    OUString const newText = m_Text.replaceAt(nTextStartIdx, nLen, "");
    nLen = pDest->m_Text.getLength() - nInitSize; // update w/ current size!
    if (!nLen)                 // String didn't grow?
        return;

    if (bUpdate)
    {
        // Update all SwIndex
        pDest->Update( rDestStart, nLen, false, false/*??? why was it true*/);
    }

    CHECK_SWPHINTS(pDest);

    const sal_Int32 nEnd = rStart.GetIndex() + nLen;
    bool const bUndoNodes =
        GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());

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
                while( true )
                {
                    // check current item
                    sal_uInt16 nWhich = IsInvalidItem( pItem )
                        ? pDest->GetpSwAttrSet()->GetWhichByPos( aIter.GetCurPos() )
                        : pItem->Which();
                    if( RES_FRMATR_STYLE_NAME != nWhich &&
                        RES_FRMATR_CONDITIONAL_STYLE_NAME != nWhich &&
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
                    if( aIter.IsAtEnd() )
                        break;
                    pItem = aIter.NextItem();
                }
            }
        }

        // all or just the Char attributes?
        if( nInitSize || hasSwAttrSet ||
            nLen != pDest->GetText().getLength())
        {
            SfxItemSet aCharSet(
                pDest->GetDoc()->GetAttrPool(),
                svl::Items<
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    RES_TXTATR_INETFMT, RES_TXTATR_CHARFMT,
                    RES_UNKNOWNATR_BEGIN, RES_UNKNOWNATR_END - 1>{});
            aCharSet.Put( *GetpSwAttrSet() );
            if( aCharSet.Count() )
                pDest->SetAttr( aCharSet, nDestStart, nDestStart + nLen );
        }
        else
        {
            GetpSwAttrSet()->CopyToModify( *pDest );
        }
    }

    // notify frames - before moving hints, because footnotes
    // want to find their anchor text frame in the follow chain
    SwInsText aInsHint( nDestStart, nLen );
    pDest->ModifyNotification( nullptr, &aInsHint );
    sw::MoveText const moveHint(pDest, nDestStart, nTextStartIdx, nLen);
    CallSwClientNotify(moveHint);
    SwDelText aDelHint( nTextStartIdx, nLen );
    ModifyNotification( nullptr, &aDelHint );

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
                pNewHt = MakeTextAttr( *pDest->GetDoc(), pHt->GetAttr(),
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
                if (GetDoc()->GetDocShell())
                {
                    GetDoc()->GetDocShell()->Broadcast( SfxHint(SfxHintId::SwSplitNodeOperation));
                }
                // move attribute
                m_pSwpHints->Delete( pHt );
                // reset start/end indexes
                if (pHt->IsFormatIgnoreStart() || pHt->IsFormatIgnoreEnd())
                {
                    bMergePortionsNeeded = true;
                }
                pHt->GetStart() =
                        nDestStart + (nAttrStartIdx - nTextStartIdx);
                if (pEndIdx)
                {
                    *pHt->GetEnd() = nDestStart + (
                                    *pEndIdx > nEnd
                                        ? nLen
                                        : *pEndIdx - nTextStartIdx );
                }
                pDest->InsertHint( pHt,
                          SetAttrMode::NOTXTATRCHR
                        | SetAttrMode::DONTREPLACE );
                if (GetDoc()->GetDocShell())
                {
                    GetDoc()->GetDocShell()->Broadcast( SfxHint(SfxHintId::SwSplitNodeOperation));
                }
                continue;           // iterate while loop, no ++ !
            }
                // the end is behind the range
            else if (RES_TXTATR_REFMARK != nWhich || bUndoNodes)
            {
                pNewHt = MakeTextAttr( *GetDoc(), pHt->GetAttr(),
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
        Update( rStart, nLen, true, true );

        for (SwTextAttr* pHt : aArr)
        {
            pHt->GetStart() = *pHt->GetEnd() = rStart.GetIndex();
            InsertHint( pHt );
        }
    }
    else
    {
        Update( rStart, nLen, true, true );
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

void SwTextNode::EraseText(const SwIndex &rIdx, const sal_Int32 nCount,
        const SwInsertFlags nMode )
{
    assert(rIdx <= m_Text.getLength()); // invalid index

    const sal_Int32 nStartIdx = rIdx.GetIndex();
    const sal_Int32 nCnt = (nCount==SAL_MAX_INT32)
                      ? m_Text.getLength() - nStartIdx : nCount;
    const sal_Int32 nEndIdx = nStartIdx + nCnt;
    if (nEndIdx <= m_Text.getLength())
        m_Text = m_Text.replaceAt(nStartIdx, nCnt, "");

    // GCAttr(); don't remove all empty ones, just the ones that are in the
    // range but not at the end of the range.

    for ( size_t i = 0; m_pSwpHints && i < m_pSwpHints->Count(); ++i )
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

    Update( rIdx, nCnt, true );

    if( 1 == nCnt )
    {
        SwDelChr aHint( nStartIdx );
        NotifyClients( nullptr, &aHint );
    }
    else
    {
        SwDelText aHint( nStartIdx, nCnt );
        NotifyClients( nullptr, &aHint );
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

        NotifyClients( nullptr, &aHint );
        SwFormatChg aNew( GetTextColl() );
        NotifyClients( nullptr, &aNew );
    }
}

SwNumRule* SwTextNode::GetNumRule(bool bInParent) const
{
    SwNumRule* pRet = nullptr;

    const SfxPoolItem* pItem = GetNoCondAttr( RES_PARATR_NUMRULE, bInParent );
    bool bNoNumRule = false;
    if ( pItem )
    {
        OUString sNumRuleName =
            static_cast<const SwNumRuleItem *>(pItem)->GetValue();
        if (!sNumRuleName.isEmpty())
        {
            pRet = GetDoc()->FindNumRulePtr( sNumRuleName );
        }
        else // numbering is turned off
            bNoNumRule = true;
    }

    if ( !bNoNumRule )
    {
        if ( pRet && pRet == GetDoc()->GetOutlineNumRule() &&
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
            mpNodeNumRLHidden->ChangeNumRule( *pNumRule );
        }
    }

    if( IsInCache() )
    {
        SwFrame::GetCache().Delete( this );
        SetInCache( false );
    }
    SetInSwFntCache( false );

    // Sending "noop" modify in order to cause invalidations of registered
    // <SwTextFrame> instances to get the list style change respectively the change
    // in the list tree reflected in the layout.
    // Important note:
    {
        SvxLRSpaceItem& rLR = const_cast<SvxLRSpaceItem&>(GetSwAttrSet().GetLRSpace());
        NotifyClients( &rLR, &rLR );
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
        bResult =
            GetDoc()->getIDocumentListsAccess().getListByName( GetListId() )->IsListLevelMarked( GetActualListLevel() );
    }

    return bResult;
}
// <- #i27615#

SwTextNode* SwTextNode::MakeNewTextNode( const SwNodeIndex& rPos, bool bNext,
                                       bool bChgFollow )
{
    // ignore hard PageBreak/PageDesc/ColumnBreak from Auto-Set
    std::unique_ptr<SwAttrSet> pNewAttrSet;
    // #i75353#
    bool bClearHardSetNumRuleWhenFormatCollChanges( false );
    if( HasSwAttrSet() )
    {
        pNewAttrSet.reset(new SwAttrSet( *GetpSwAttrSet() ));
        const SfxItemSet* pTmpSet = GetpSwAttrSet();

        if (bNext)     // successor doesn't inherit breaks!
            pTmpSet = pNewAttrSet.get();

        // !bNext: remove PageBreaks/PageDesc/ColBreak from this
        bool bRemoveFromCache = false;
        std::vector<sal_uInt16> aClearWhichIds;
        if ( bNext )
            bRemoveFromCache = ( 0 != pNewAttrSet->ClearItem( RES_PAGEDESC ) );
        else
            aClearWhichIds.push_back( RES_PAGEDESC );

        if( SfxItemState::SET == pTmpSet->GetItemState( RES_BREAK, false ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_BREAK );
            else
                aClearWhichIds.push_back( RES_BREAK );
            bRemoveFromCache = true;
        }
        if( SfxItemState::SET == pTmpSet->GetItemState( RES_KEEP, false ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_KEEP );
            else
                aClearWhichIds.push_back( RES_KEEP );
            bRemoveFromCache = true;
        }
        if( SfxItemState::SET == pTmpSet->GetItemState( RES_PARATR_SPLIT, false ) )
        {
            if ( bNext )
                pNewAttrSet->ClearItem( RES_PARATR_SPLIT );
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
                    pNewAttrSet->ClearItem(RES_PARATR_NUMRULE);
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

        if( !bNext && bRemoveFromCache && IsInCache() )
        {
            SwFrame::GetCache().Delete( this );
            SetInCache( false );
        }
    }
    SwNodes& rNds = GetNodes();

    SwTextFormatColl* pColl = GetTextColl();

    SwTextNode *pNode = new SwTextNode( rPos, pColl, pNewAttrSet.get() );

    pNewAttrSet.reset();

    const SwNumRule* pRule = GetNumRule();
    if( pRule && pRule == pNode->GetNumRule() && rNds.IsDocNodes() )
    {
        // #i55459#
        // - correction: parameter <bNext> has to be checked, as it was in the
        //   previous implementation.
        if ( !bNext && !IsCountedInList() )
            SetCountedInList(true);
    }

    // In case the numbering caused a style form the pool to be assigned to
    // the new node, don't overwrite that here!
    if( pColl != pNode->GetTextColl() ||
        ( bChgFollow && pColl != GetTextColl() ))
        return pNode;       // that ought to be enough?

    pNode->ChgTextCollUpdateNum( nullptr, pColl ); // for numbering/outline
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
            std::vector<sal_uInt16> aClearWhichIds;
            aClearWhichIds.push_back( RES_PARATR_NUMRULE );
            if ( ClearItemsFromAttrSet( aClearWhichIds ) != 0 && IsInCache() )
            {
                SwFrame::GetCache().Delete( this );
                SetInCache( false );
            }
        }
    }
    ChgFormatColl( pNextColl );

    return pNode;
}

SwContentNode* SwTextNode::AppendNode( const SwPosition & rPos )
{
    // position behind which it will be inserted
    SwNodeIndex aIdx( rPos.nNode, 1 );
    SwTextNode* pNew = MakeNewTextNode( aIdx );

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

namespace
{

sal_uInt16 lcl_BoundListLevel(const int nActualLevel)
{
    return static_cast<sal_uInt16>( std::min( std::max(nActualLevel, 0), MAXLEVEL-1 ) );
}

}

// -> #i29560#
bool SwTextNode::HasNumber() const
{
    bool bResult = false;

    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pRule )
    {
        const SwNumFormat& aFormat(pRule->Get(lcl_BoundListLevel(GetActualListLevel())));

        // #i40041#
        bResult = aFormat.IsEnumeration() &&
            SVX_NUM_NUMBER_NONE != aFormat.GetNumberingType();
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
        SwRootFrame const*const pLayout) const
{
    if (GetDoc()->IsClipBoard() && m_pNumStringCache.get())
    {
        // #i111677# do not expand number strings in clipboard documents
        return *m_pNumStringCache;
    }
    const SwNumRule* pRule = GetNum(pLayout) ? GetNum(pLayout)->GetNumRule() : nullptr;
    if ( pRule &&
         IsCountedInList() )
    {
        SvxNumberType const& rNumberType(
                pRule->Get( lcl_BoundListLevel(GetActualListLevel()) ) );
        if (rNumberType.IsTextFormat() ||

            (style::NumberingType::NUMBER_NONE == rNumberType.GetNumberingType()))
        {
            return pRule->MakeNumString( GetNum(pLayout)->GetNumberVector(),
                                     _bInclPrefixAndSuffixStrings,
                                     false,
                                     _nRestrictToThisLevel,
                                     nullptr,
                                     GetLang(0));
        }
    }

    return OUString();
}

long SwTextNode::GetLeftMarginWithNum( bool bTextLeft ) const
{
    long nRet = 0;
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
                nRet = nRet - GetSwAttrSet().GetLRSpace().GetLeft();
        }
        else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if ( AreListLevelIndentsApplicable() )
            {
                nRet = rFormat.GetIndentAt();
                // #i90401#
                // Only negative first line indents have consider for the left margin
                if ( !bTextLeft &&
                     rFormat.GetFirstLineIndent() < 0 )
                {
                    nRet = nRet + rFormat.GetFirstLineIndent();
                }
            }
        }
    }

    return nRet;
}

bool SwTextNode::GetFirstLineOfsWithNum( short& rFLOffset ) const
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
                    SvxLRSpaceItem aItem = GetSwAttrSet().GetLRSpace();
                    rFLOffset = rFLOffset + aItem.GetTextFirstLineOfst();
                }
            }
            else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                if ( AreListLevelIndentsApplicable() )
                {
                    rFLOffset = rFormat.GetFirstLineIndent();
                }
                else if (!getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxLRSpaceItem aItem = GetSwAttrSet().GetLRSpace();
                    rFLOffset = aItem.GetTextFirstLineOfst();
                }
            }
        }

        return true;
    }

    rFLOffset = GetSwAttrSet().GetLRSpace().GetTextFirstLineOfst();
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
            nAdditionalIndent = GetSwAttrSet().GetLRSpace().GetLeft();

            if (getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
            {
                nAdditionalIndent = nAdditionalIndent -
                                    GetSwAttrSet().GetLRSpace().GetTextFirstLineOfst();
            }
        }
        else if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if ( AreListLevelIndentsApplicable() )
            {
                nAdditionalIndent = rFormat.GetIndentAt() + rFormat.GetFirstLineIndent();
            }
            else
            {
                nAdditionalIndent = GetSwAttrSet().GetLRSpace().GetLeft();
                if (getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    nAdditionalIndent = nAdditionalIndent -
                                        GetSwAttrSet().GetLRSpace().GetTextFirstLineOfst();
                }
            }
        }
    }
    else
    {
        nAdditionalIndent = GetSwAttrSet().GetLRSpace().GetLeft();
    }

    return nAdditionalIndent;
}

// #i96772#
void SwTextNode::ClearLRSpaceItemDueToListLevelIndents( SvxLRSpaceItem& o_rLRSpaceItem ) const
{
    if ( AreListLevelIndentsApplicable() )
    {
        const SwNumRule* pRule = GetNumRule();
        if ( pRule && GetActualListLevel() >= 0 )
        {
            const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));
            if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                SvxLRSpaceItem aLR( RES_LR_SPACE );
                o_rLRSpaceItem = aLR;
            }
        }
    }
}

// #i91133#
long SwTextNode::GetLeftMarginForTabCalculation() const
{
    long nLeftMarginForTabCalc = 0;

    bool bLeftMarginForTabCalcSetToListLevelIndent( false );
    const SwNumRule* pRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if( pRule )
    {
        const SwNumFormat& rFormat = pRule->Get(lcl_BoundListLevel(GetActualListLevel()));
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            if ( AreListLevelIndentsApplicable() )
            {
                nLeftMarginForTabCalc = rFormat.GetIndentAt();
                bLeftMarginForTabCalcSetToListLevelIndent = true;
            }
        }
    }
    if ( !bLeftMarginForTabCalcSetToListLevelIndent )
    {
        nLeftMarginForTabCalc = GetSwAttrSet().GetLRSpace().GetTextLeft();
    }

    return nLeftMarginForTabCalc;
}

static void Replace0xFF(
    SwTextNode const& rNode,
    OUStringBuffer & rText,
    sal_Int32 & rTextStt,
    sal_Int32 nEndPos )
{
    if (rNode.GetpSwpHints())
    {
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

    ModelToViewHelper aConversionMap(*this, pLayout, eMode);
    const OUString aExpandText = aConversionMap.getViewText();
    const sal_Int32 nExpandBegin = aConversionMap.ConvertToViewPosition( nIdx );
    sal_Int32 nEnd = nLen == -1 ? GetText().getLength() : nIdx + nLen;
    const sal_Int32 nExpandEnd = aConversionMap.ConvertToViewPosition( nEnd );
    OUStringBuffer aText(aExpandText.copy(nExpandBegin, nExpandEnd-nExpandBegin));

    // remove dummy characters of Input Fields
    comphelper::string::remove(aText, CH_TXT_ATR_INPUTFIELDSTART);
    comphelper::string::remove(aText, CH_TXT_ATR_INPUTFIELDEND);

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

bool SwTextNode::CopyExpandText(SwTextNode& rDestNd, const SwIndex* pDestIdx,
                        sal_Int32 nIdx, sal_Int32 nLen,
                        SwRootFrame const*const pLayout, bool bWithNum,
                        bool bWithFootnote, bool bReplaceTabsWithSpaces ) const
{
    if( &rDestNd == this )
        return false;

    SwIndex aDestIdx(&rDestNd, rDestNd.GetText().getLength());
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
                                sExpand = GetDoc()->GetEndNoteInfo().aFormat.
                                            GetNumStr(number);
                            else
                                sExpand = GetDoc()->GetFootnoteInfo().aFormat.
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

    if( bWithNum )
    {
        aDestIdx = nDestStt;
        rDestNd.InsertText( GetNumString(true, MAXLEVEL, pLayout), aDestIdx );
    }

    aDestIdx = 0;
    sal_Int32 nStartDelete(-1);
    while (aDestIdx < rDestNd.GetText().getLength())
    {
        sal_Unicode const cur(rDestNd.GetText()[aDestIdx.GetIndex()]);
        if (   (cChar == cur) // filter substituted hidden text
            || (CH_TXT_ATR_FIELDSTART  == cur) // filter all fieldmarks
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
            SwIndex(&rDestNd, nStartDelete),
            aDestIdx.GetIndex() - nStartDelete);
        assert(aDestIdx.GetIndex() == nStartDelete);
        nStartDelete = -1; // reset
    }

    return true;
}

OUString SwTextNode::GetRedlineText() const
{
    std::vector<sal_Int32> aRedlArr;
    const SwDoc* pDoc = GetDoc();
    SwRedlineTable::size_type nRedlPos = pDoc->getIDocumentRedlineAccess().GetRedlinePos( *this, nsRedlineType_t::REDLINE_DELETE );
    if( SwRedlineTable::npos != nRedlPos )
    {
        // some redline-delete object exists for the node
        const sal_uLong nNdIdx = GetIndex();
        for( ; nRedlPos < pDoc->getIDocumentRedlineAccess().GetRedlineTable().size() ; ++nRedlPos )
        {
            const SwRangeRedline* pTmp = pDoc->getIDocumentRedlineAccess().GetRedlineTable()[ nRedlPos ];
            if( nsRedlineType_t::REDLINE_DELETE == pTmp->GetType() )
            {
                const SwPosition *pRStt = pTmp->Start(), *pREnd = pTmp->End();
                if( pRStt->nNode < nNdIdx )
                {
                    if( pREnd->nNode > nNdIdx )
                        // paragraph is fully deleted
                        return OUString();
                    else if( pREnd->nNode == nNdIdx )
                    {
                        // deleted from 0 to nContent
                        aRedlArr.push_back( 0 );
                        aRedlArr.push_back( pREnd->nContent.GetIndex() );
                    }
                }
                else if( pRStt->nNode == nNdIdx )
                {
                    //aRedlArr.Insert( pRStt->nContent.GetIndex(), aRedlArr.Count() );
                    aRedlArr.push_back( pRStt->nContent.GetIndex() );
                    if( pREnd->nNode == nNdIdx )
                        aRedlArr.push_back( pREnd->nContent.GetIndex() );
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

void SwTextNode::ReplaceText( const SwIndex& rStart, const sal_Int32 nDelLen,
                             const OUString & rStr)
{
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

    if (nLen && sInserted.getLength())
    {
        // Replace the 1st char, then delete the rest and insert.
        // This way the attributes of the 1st char are expanded!
        m_Text = m_Text.replaceAt(nStartPos, 1, sInserted.copy(0, 1));

        ++const_cast<SwIndex&>(rStart);
        m_Text = m_Text.replaceAt(rStart.GetIndex(), nLen - 1, "");
        Update( rStart, nLen - 1, true );

        OUString aTmpText( sInserted.copy(1) );
        m_Text = m_Text.replaceAt(rStart.GetIndex(), 0, aTmpText);
        Update( rStart, aTmpText.getLength() );
    }
    else
    {
        m_Text = m_Text.replaceAt(nStartPos, nLen, "");
        Update( rStart, nLen, true );

        m_Text = m_Text.replaceAt(nStartPos, 0, sInserted);
        Update( rStart, sInserted.getLength() );
    }

    SetIgnoreDontExpand( bOldExpFlg );
    SwDelText aDelHint( nStartPos, nDelLen );
    NotifyClients( nullptr, &aDelHint );

    SwInsText aHint( nStartPos, sInserted.getLength() );
    NotifyClients( nullptr, &aHint );
}

namespace {
    void lcl_ResetParAttrs( SwTextNode &rTextNode )
    {
        std::set<sal_uInt16> aAttrs;
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_ID );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_LEVEL );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_ISRESTART );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_RESTARTVALUE );
        aAttrs.insert( aAttrs.end(), RES_PARATR_LIST_ISCOUNTED );
        SwPaM aPam( rTextNode );
        // #i96644#
        // suppress side effect "send data changed events"
        rTextNode.GetDoc()->ResetAttrs( aPam, false, aAttrs, false );
    }

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
        OUString sNumRule;
        OUString sOldNumRule;
        switch ( nWhich )
        {
            case RES_FMT_CHG:
            {
                bParagraphStyleChanged = true;
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
                break;
            }
            case RES_ATTRSET_CHG:
            {
                const SfxPoolItem* pItem = nullptr;
                const SwNumRule* pFormerNumRuleAtTextNode =
                    rTextNode.GetNum() ? rTextNode.GetNum()->GetNumRule() : nullptr;
                if ( pFormerNumRuleAtTextNode )
                {
                    sOldNumRule = pFormerNumRuleAtTextNode->GetName();
                }

                const SwAttrSetChg* pSet = dynamic_cast<const SwAttrSetChg*>(pNewValue);
                if ( pSet && pSet->GetChgSet()->GetItemState( RES_PARATR_NUMRULE, false, &pItem ) ==
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
                break;
            }
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
                    if ( dynamic_cast<const SfxUInt16Item &>(rTextNode.GetAttr( RES_PARATR_OUTLINELEVEL, false )).GetValue() > 0 )
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
    // End of method <HandleModifyAtTextNode>
}

SwFormatColl* SwTextNode::ChgFormatColl( SwFormatColl *pNewColl )
{
    OSL_ENSURE( pNewColl,"ChgFormatColl: Collectionpointer has value 0." );
    OSL_ENSURE( dynamic_cast<const SwTextFormatColl *>(pNewColl) != nullptr,
                "ChgFormatColl: is not a Text Collection pointer." );

    SwTextFormatColl *pOldColl = GetTextColl();
    if( pNewColl != pOldColl )
    {
        SetCalcHiddenCharFlags();
        SwContentNode::ChgFormatColl( pNewColl );
        OSL_ENSURE( !mbInSetOrResetAttr,
                "DEBUG OSL_ENSURE(ON - <SwTextNode::ChgFormatColl(..)> called during <Set/ResetAttr(..)>" );
        if ( !mbInSetOrResetAttr )
        {
            SwFormatChg aTmp1( pOldColl );
            SwFormatChg aTmp2( pNewColl );
            HandleModifyAtTextNode( *this, &aTmp1, &aTmp2  );
        }

        // reset fill information on parent style change
        if(maFillAttributes.get())
        {
            maFillAttributes.reset();
        }
    }

    // only for real nodes-array
    if( GetNodes().IsDocNodes() )
    {
        ChgTextCollUpdateNum( pOldColl, static_cast<SwTextFormatColl *>(pNewColl) );
    }

    GetNodes().UpdateOutlineNode(*this);

    return pOldColl;
}

const SwNodeNum* SwTextNode::GetNum(SwRootFrame const*const pLayout) const
{
    // invariant: it's only in list in Hide mode if it's in list in normal mode
    assert(mpNodeNum || !mpNodeNumRLHidden);
    return pLayout && pLayout->IsHideRedlines() ? mpNodeNumRLHidden : mpNodeNum;
}

void SwTextNode::DoNum(std::function<void (SwNodeNum &)> const& rFunc)
{
    // temp. clear because GetActualListLevel() may be called and the assert
    // there triggered during update, which is unhelpful
    SwNodeNum * pBackup(mpNodeNumRLHidden);
    mpNodeNumRLHidden = nullptr;
    assert(mpNodeNum);
    rFunc(*mpNodeNum);
    if (pBackup)
    {
        mpNodeNumRLHidden = pBackup;
        rFunc(*mpNodeNumRLHidden);
    }
}

SwNumberTree::tNumberVector
SwTextNode::GetNumberVector(SwRootFrame const*const pLayout) const
{
    if (SwNodeNum const*const pNum = GetNum(pLayout))
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

int SwTextNode::GetAttrOutlineLevel() const
{
    return static_cast<const SfxUInt16Item &>(GetAttr(RES_PARATR_OUTLINELEVEL)).GetValue();
}

void SwTextNode::SetAttrOutlineLevel(int nLevel)
{
    assert(0 <= nLevel && nLevel <= MAXLEVEL); // Level Out Of Range
    if ( 0 <= nLevel && nLevel <= MAXLEVEL )
    {
        SetAttr( SfxUInt16Item( RES_PARATR_OUTLINELEVEL,
                                static_cast<sal_uInt16>(nLevel) ) );
    }
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
        dynamic_cast<const SfxInt16Item&>(GetAttr( RES_PARATR_LIST_LEVEL ));
    nAttrListLevel = static_cast<int>(aListLevelItem.GetValue());

    return nAttrListLevel;
}

int SwTextNode::GetActualListLevel() const
{
    assert(!GetNum() || !mpNodeNumRLHidden || // must be in sync
        GetNum()->GetLevelInListTree() == mpNodeNumRLHidden->GetLevelInListTree());
    return GetNum() ? GetNum()->GetLevelInListTree() : -1;
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
    const SfxBoolItem& aIsRestartItem =
        dynamic_cast<const SfxBoolItem&>(GetAttr( RES_PARATR_LIST_ISRESTART ));

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
        // #i87154#
        // Correction of #newlistlevelattrs#:
        // The numbering type has to be checked for bullet lists.
        const SwNumFormat& rFormat = pRule->Get( lcl_BoundListLevel(GetActualListLevel()) );
        return SVX_NUM_NUMBER_NONE != rFormat.GetNumberingType() ||
               !pRule->MakeNumString( *(GetNum()) ).isEmpty();
    }

    return false;
}

void SwTextNode::SetAttrListRestartValue( SwNumberTree::tSwNumTreeNumber nNumber )
{
    const bool bChanged( HasAttrListRestartValue()
                         ? GetAttrListRestartValue() != nNumber
                         : nNumber != USHRT_MAX );

    if ( bChanged || !HasAttrListRestartValue() )
    {
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
        dynamic_cast<const SfxInt16Item&>(GetAttr( RES_PARATR_LIST_RESTARTVALUE ));
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
                    pRule->GetNumFormat( static_cast<sal_uInt16>(GetAttrListLevel()) );
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
    bool bResult = false;
    const SwDoc * pDoc = GetDoc();
    if( pDoc )
    {
        bResult = !(pDoc->IsInReading() || pDoc->IsInDtor());
    }
    return bResult;
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
    const SfxBoolItem& aIsCountedInListItem =
        dynamic_cast<const SfxBoolItem&>(GetAttr( RES_PARATR_LIST_ISCOUNTED ));

    return aIsCountedInListItem.GetValue();
}

static SwList * FindList(SwTextNode *const pNode)
{
    const OUString sListId = pNode->GetListId();
    if (!sListId.isEmpty())
    {
        auto & rIDLA(pNode->GetDoc()->getIDocumentListsAccess());
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
    if (pList && GetNodes().IsDocNodes()) // not for undo nodes
    {
        assert(!mpNodeNum);
        mpNodeNum = new SwNodeNum(this, false);
        pList->InsertListItem(*mpNodeNum, false, GetAttrListLevel());
        // iterate all frames & if there's one with hidden layout...
        SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> iter(*this);
        for (SwTextFrame* pFrame = iter.First(); pFrame; pFrame = iter.Next())
        {
            if (pFrame->getRootFrame()->IsHideRedlines())
            {
                sw::MergedPara const*const pMerged = pFrame->GetMergedPara();
                if (!pMerged || this == pMerged->pParaPropsNode)
                {
                    AddToListRLHidden();
                }
                break; // assume it's consistent, need to check only once
            }
        }
    }
}

void SwTextNode::AddToListRLHidden()
{
    if (mpNodeNumRLHidden)
    {
        assert(false);
        OSL_FAIL( "<SwTextNode::AddToListRLHidden()> - the text node is already added to a list. Serious defect" );
        return;
    }

    SwList *const pList(FindList(this));
    if (pList)
    {
        assert(!mpNodeNumRLHidden);
        mpNodeNumRLHidden = new SwNodeNum(this, true);
        pList->InsertListItem(*mpNodeNumRLHidden, true, GetAttrListLevel());
    }
}

void SwTextNode::RemoveFromList()
{
    // sw_redlinehide: ensure it's removed from the other half too!
    RemoveFromListRLHidden();
    if ( IsInList() )
    {
        SwList::RemoveListItem( *mpNodeNum );
        delete mpNodeNum;
        mpNodeNum = nullptr;

        SetWordCountDirty( true );
    }
}

void SwTextNode::RemoveFromListRLHidden()
{
    if (mpNodeNumRLHidden) // direct access because RemoveFromList doesn't have layout
    {
        assert(mpNodeNumRLHidden->GetParent() || !GetNodes().IsDocNodes());
        SwList::RemoveListItem(*mpNodeNumRLHidden);
        delete mpNodeNumRLHidden;
        mpNodeNumRLHidden = nullptr;

        SetWordCountDirty( true );
    }
}

bool SwTextNode::IsInList() const
{
    // it looks like an unconnected Num can't happen, except in the undo-array
    assert(!GetNum() || GetNum()->GetParent() || !GetNodes().IsDocNodes());
    return GetNum() != nullptr && GetNum()->GetParent() != nullptr;
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
            dynamic_cast<const SfxStringItem&>(GetAttr( RES_PARATR_LIST_ID ));
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
                dynamic_cast<const SfxStringItem&>(GetAttr( RES_PARATR_LIST_ID ));
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

    @return boolean
*/
bool SwTextNode::AreListLevelIndentsApplicable() const
{
    bool bAreListLevelIndentsApplicable( true );

    if ( !GetNum() || !GetNum()->GetNumRule() )
    {
        // no list style applied to paragraph
        bAreListLevelIndentsApplicable = false;
    }
    else if ( HasSwAttrSet() &&
              GetpSwAttrSet()->GetItemState( RES_LR_SPACE, false ) == SfxItemState::SET )
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
            if ( pColl->GetAttrSet().GetItemState( RES_LR_SPACE, false ) == SfxItemState::SET )
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
bool SwTextNode::GetListTabStopPosition( long& nListTabStopPosition ) const
{
    bool bListTabStopPositionProvided(false);

    const SwNumRule* pNumRule = GetNum() ? GetNum()->GetNumRule() : nullptr;
    if ( pNumRule && HasVisibleNumberingOrBullet() && GetActualListLevel() >= 0 )
    {
        const SwNumFormat& rFormat = pNumRule->Get( static_cast<sal_uInt16>(GetActualListLevel()) );
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT &&
             rFormat.GetLabelFollowedBy() == SvxNumberFormat::LISTTAB )
        {
            bListTabStopPositionProvided = true;
            nListTabStopPosition = rFormat.GetListtabPos();

            if ( getIDocumentSettingAccess()->get(DocumentSettingId::TABS_RELATIVE_TO_INDENT) )
            {
                // tab stop position are treated to be relative to the "before text"
                // indent value of the paragraph. Thus, adjust <nListTabStopPos>.
                if ( AreListLevelIndentsApplicable() )
                {
                    nListTabStopPosition -= rFormat.GetIndentAt();
                }
                else if (!getIDocumentSettingAccess()->get(DocumentSettingId::IGNORE_FIRST_LINE_INDENT_IN_NUMBERING))
                {
                    SvxLRSpaceItem aItem = GetSwAttrSet().GetLRSpace();
                    nListTabStopPosition -= aItem.GetTextLeft();
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
        const SwNumFormat& rFormat = pNumRule->Get( static_cast<sal_uInt16>(GetActualListLevel()) );
        if ( rFormat.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT )
        {
            switch ( rFormat.GetLabelFollowedBy() )
            {
                case SvxNumberFormat::LISTTAB:
                {
                    return OUString("\t");
                }
                break;
                case SvxNumberFormat::SPACE:
                {
                    return OUString(" ");
                }
                break;
                case SvxNumberFormat::NEWLINE:
                {
                    return OUString("\n");
                }
                break;
                case SvxNumberFormat::NOTHING:
                {
                    // intentionally left blank.
                }
                break;
                default:
                {
                    OSL_FAIL( "<SwTextNode::GetLabelFollowedBy()> - unknown SvxNumberFormat::GetLabelFollowedBy() return value" );
                }
            }
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

                const SwNumRuleItem& rNumRuleItem =
                                dynamic_cast<const SwNumRuleItem&>(pItem);
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
                const SfxStringItem& rListIdItem =
                                        dynamic_cast<const SfxStringItem&>(pItem);
                OSL_ENSURE( rListIdItem.GetValue().getLength() > 0,
                        "<HandleSetAttrAtTextNode(..)> - empty list id attribute not excepted. Serious defect." );
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
                const SfxInt16Item& aListLevelItem =
                                    dynamic_cast<const SfxInt16Item&>(pItem);
                if ( aListLevelItem.GetValue() != mrTextNode.GetAttrListLevel() )
                {
                    mbUpdateListLevel = true;
                }
            }
            break;

            // handle RES_PARATR_LIST_ISRESTART
            case RES_PARATR_LIST_ISRESTART:
            {
                const SfxBoolItem& aListIsRestartItem =
                                    dynamic_cast<const SfxBoolItem&>(pItem);
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
                const SfxInt16Item& aListRestartValueItem =
                                    dynamic_cast<const SfxInt16Item&>(pItem);
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
                const SfxBoolItem& aIsCountedInListItem =
                                    dynamic_cast<const SfxBoolItem&>(pItem);
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
                const SfxUInt16Item& aOutlineLevelItem =
                                    dynamic_cast<const SfxUInt16Item&>(pItem);
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
        const SfxPoolItem* pItem = nullptr;
        // handle RES_PARATR_NUMRULE
        if ( rItemSet.GetItemState( RES_PARATR_NUMRULE, false, &pItem ) == SfxItemState::SET )
        {
            mrTextNode.RemoveFromList();

            const SwNumRuleItem* pNumRuleItem =
                            dynamic_cast<const SwNumRuleItem*>(pItem);
            if ( !pNumRuleItem->GetValue().isEmpty() )
            {
                mbAddTextNodeToList = true;
                // #i70748#
                mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }

        // handle RES_PARATR_LIST_ID
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ID, false, &pItem ) == SfxItemState::SET )
        {
            const SfxStringItem* pListIdItem =
                                    dynamic_cast<const SfxStringItem*>(pItem);
            const OUString sListIdOfTextNode = mrTextNode.GetListId();
            if ( pListIdItem &&
                 pListIdItem->GetValue() != sListIdOfTextNode )
            {
                mbAddTextNodeToList = true;
                if ( mrTextNode.IsInList() )
                {
                    mrTextNode.RemoveFromList();
                }
            }
        }

        // handle RES_PARATR_LIST_LEVEL
        if ( rItemSet.GetItemState( RES_PARATR_LIST_LEVEL, false, &pItem ) == SfxItemState::SET )
        {
            const SfxInt16Item* pListLevelItem =
                                dynamic_cast<const SfxInt16Item*>(pItem);
            if (pListLevelItem && pListLevelItem->GetValue() != mrTextNode.GetAttrListLevel())
            {
                mbUpdateListLevel = true;
            }
        }

        // handle RES_PARATR_LIST_ISRESTART
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ISRESTART, false, &pItem ) == SfxItemState::SET )
        {
            const SfxBoolItem* pListIsRestartItem =
                                dynamic_cast<const SfxBoolItem*>(pItem);
            if (pListIsRestartItem && pListIsRestartItem->GetValue() != mrTextNode.IsListRestart())
            {
                mbUpdateListRestart = true;
            }
        }

        // handle RES_PARATR_LIST_RESTARTVALUE
        if ( rItemSet.GetItemState( RES_PARATR_LIST_RESTARTVALUE, false, &pItem ) == SfxItemState::SET )
        {
            const SfxInt16Item* pListRestartValueItem =
                                dynamic_cast<const SfxInt16Item*>(pItem);
            if ( !mrTextNode.HasAttrListRestartValue() || (pListRestartValueItem &&
                 pListRestartValueItem->GetValue() != mrTextNode.GetAttrListRestartValue()) )
            {
                mbUpdateListRestart = true;
            }
        }

        // handle RES_PARATR_LIST_ISCOUNTED
        if ( rItemSet.GetItemState( RES_PARATR_LIST_ISCOUNTED, false, &pItem ) == SfxItemState::SET )
        {
            const SfxBoolItem* pIsCountedInListItem =
                                dynamic_cast<const SfxBoolItem*>(pItem);
            if (pIsCountedInListItem && pIsCountedInListItem->GetValue() !=
                mrTextNode.IsCountedInList())
            {
                mbUpdateListCount = true;
            }
        }

        // #i70748#
        // handle RES_PARATR_OUTLINELEVEL
        if ( rItemSet.GetItemState( RES_PARATR_OUTLINELEVEL, false, &pItem ) == SfxItemState::SET )
        {
            const SfxUInt16Item* pOutlineLevelItem =
                                dynamic_cast<const SfxUInt16Item*>(pItem);
            if (pOutlineLevelItem && pOutlineLevelItem->GetValue() !=
                mrTextNode.GetAttrOutlineLevel())
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
                mrTextNode.DoNum(
                    [nLevel](SwNodeNum & rNum) { rNum.SetLevelInListTree(nLevel); });
            }

            if ( mbUpdateListRestart && mrTextNode.IsInList() )
            {
                mrTextNode.DoNum(
                    [](SwNodeNum & rNum) {
                        rNum.InvalidateMe();
                        rNum.NotifyInvalidSiblings();
                    });
            }

            if ( mbUpdateListCount && mrTextNode.IsInList() )
            {
                mrTextNode.DoNum(
                    [](SwNodeNum & rNum) { rNum.InvalidateAndNotifyTree(); });
            }
        }

        // #i70748#
        if (mbOutlineLevelSet)
        {
            mrTextNode.GetNodes().UpdateOutlineNode(mrTextNode);
            if (mrTextNode.GetAttrOutlineLevel() == 0)
            {
                mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
            else
            {
                const SfxPoolItem* pItem = nullptr;
                if ( mrTextNode.GetSwAttrSet().GetItemState( RES_PARATR_NUMRULE,
                                                            true, &pItem )
                                                                != SfxItemState::SET )
                {
                    mrTextNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                }
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
                                      const sal_uInt16 nWhich2 );
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
    };

    HandleResetAttrAtTextNode::HandleResetAttrAtTextNode( SwTextNode& rTextNode,
                                                        const sal_uInt16 nWhich1,
                                                        const sal_uInt16 nWhich2 )
        : mrTextNode( rTextNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        bool bRemoveFromList( false );
        if ( nWhich2 != 0 && nWhich2 > nWhich1 )
        {
            // RES_PARATR_NUMRULE and RES_PARATR_LIST_ID
            if ( nWhich1 <= RES_PARATR_NUMRULE && RES_PARATR_NUMRULE <= nWhich2 )
            {
                bRemoveFromList = mrTextNode.GetNumRule() != nullptr;
                mbListStyleOrIdReset = true;
            }
            else if ( nWhich1 <= RES_PARATR_LIST_ID && RES_PARATR_LIST_ID <= nWhich2 )
            {
                bRemoveFromList = mrTextNode.GetpSwAttrSet() &&
                    mrTextNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SfxItemState::SET;
                // #i92898#
                mbListStyleOrIdReset = true;
            }

            if ( !bRemoveFromList )
            {
                // RES_PARATR_LIST_LEVEL
                mbUpdateListLevel = ( nWhich1 <= RES_PARATR_LIST_LEVEL &&
                                      RES_PARATR_LIST_LEVEL <= nWhich2 &&
                                      mrTextNode.HasAttrListLevel() );

                // RES_PARATR_LIST_ISRESTART and RES_PARATR_LIST_RESTARTVALUE
                mbUpdateListRestart =
                    ( nWhich1 <= RES_PARATR_LIST_ISRESTART && RES_PARATR_LIST_ISRESTART <= nWhich2 &&
                      mrTextNode.IsListRestart() ) ||
                    ( nWhich1 <= RES_PARATR_LIST_RESTARTVALUE && RES_PARATR_LIST_RESTARTVALUE <= nWhich2 &&
                      mrTextNode.HasAttrListRestartValue() );

                // RES_PARATR_LIST_ISCOUNTED
                mbUpdateListCount =
                    ( nWhich1 <= RES_PARATR_LIST_ISCOUNTED && RES_PARATR_LIST_ISCOUNTED <= nWhich2 &&
                      !mrTextNode.IsCountedInList() );
            }

            // #i70748#
            // RES_PARATR_OUTLINELEVEL
            if ( nWhich1 <= RES_PARATR_OUTLINELEVEL && RES_PARATR_OUTLINELEVEL <= nWhich2 )
            {
                mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }
        }
        else
        {
            // RES_PARATR_NUMRULE and RES_PARATR_LIST_ID
            if ( nWhich1 == RES_PARATR_NUMRULE )
            {
                bRemoveFromList = mrTextNode.GetNumRule() != nullptr;
                mbListStyleOrIdReset = true;
            }
            else if ( nWhich1 == RES_PARATR_LIST_ID )
            {
                bRemoveFromList = mrTextNode.GetpSwAttrSet() &&
                    mrTextNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SfxItemState::SET;
                // #i92898#
                mbListStyleOrIdReset = true;
            }
            // #i70748#
            // RES_PARATR_OUTLINELEVEL
            else if ( nWhich1 == RES_PARATR_OUTLINELEVEL )
            {
                mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
            }

            if ( !bRemoveFromList )
            {
                // RES_PARATR_LIST_LEVEL
                mbUpdateListLevel = nWhich1 == RES_PARATR_LIST_LEVEL &&
                                    mrTextNode.HasAttrListLevel();

                // RES_PARATR_LIST_ISRESTART and RES_PARATR_LIST_RESTARTVALUE
                mbUpdateListRestart = ( nWhich1 == RES_PARATR_LIST_ISRESTART &&
                                        mrTextNode.IsListRestart() ) ||
                                      ( nWhich1 == RES_PARATR_LIST_RESTARTVALUE &&
                                        mrTextNode.HasAttrListRestartValue() );

                // RES_PARATR_LIST_ISCOUNTED
                mbUpdateListCount = nWhich1 == RES_PARATR_LIST_ISCOUNTED &&
                                    !mrTextNode.IsCountedInList();
            }
        }

        if ( bRemoveFromList && mrTextNode.IsInList() )
        {
            mrTextNode.RemoveFromList();
        }
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
        {
            for (const auto& rWhich : rWhichArr)
            {
                // RES_PARATR_NUMRULE and RES_PARATR_LIST_ID
                if ( rWhich == RES_PARATR_NUMRULE )
                {
                    bRemoveFromList = bRemoveFromList ||
                                      mrTextNode.GetNumRule() != nullptr;
                    mbListStyleOrIdReset = true;
                }
                else if ( rWhich == RES_PARATR_LIST_ID )
                {
                    bRemoveFromList = bRemoveFromList ||
                        ( mrTextNode.GetpSwAttrSet() &&
                          mrTextNode.GetpSwAttrSet()->GetItemState( RES_PARATR_LIST_ID, false ) == SfxItemState::SET );
                    // #i92898#
                    mbListStyleOrIdReset = true;
                }
                // #i70748#
                // RES_PARATR_OUTLINELEVEL
                else if ( rWhich == RES_PARATR_OUTLINELEVEL )
                {
                    mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
                }

                if ( !bRemoveFromList )
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
        }

        if ( bRemoveFromList && mrTextNode.IsInList() )
        {
            mrTextNode.RemoveFromList();
        }
    }

    HandleResetAttrAtTextNode::HandleResetAttrAtTextNode( SwTextNode& rTextNode )
        : mrTextNode( rTextNode ),
          mbListStyleOrIdReset( false ),
          mbUpdateListLevel( false ),
          mbUpdateListRestart( false ),
          mbUpdateListCount( false )
    {
        mbListStyleOrIdReset = true;
        if ( rTextNode.IsInList() )
        {
            rTextNode.RemoveFromList();
        }
        // #i70748#
        mrTextNode.ResetEmptyListStyleDueToResetOutlineLevelAttr();
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
                assert(!mrTextNode.GetpSwAttrSet()
                       || dynamic_cast<const SfxUInt16Item*>(
                              &mrTextNode.GetAttr(RES_PARATR_OUTLINELEVEL, false)));
                if (mrTextNode.GetpSwAttrSet()
                    && static_cast<const SfxUInt16Item&>(
                           mrTextNode.GetAttr(RES_PARATR_OUTLINELEVEL, false)).GetValue() > 0)
                {
                    mrTextNode.SetEmptyListStyleDueToSetOutlineLevelAttr();
                }
            }
        }

        if ( mrTextNode.IsInList() )
        {
            if ( mbUpdateListLevel )
            {
                auto const nLevel(mrTextNode.GetAttrListLevel());
                mrTextNode.DoNum(
                    [nLevel](SwNodeNum & rNum) { rNum.SetLevelInListTree(nLevel); });
            }

            if ( mbUpdateListRestart )
            {
                mrTextNode.DoNum(
                    [](SwNodeNum & rNum) {
                        rNum.InvalidateMe();
                        rNum.NotifyInvalidSiblings();
                    });
            }

            if ( mbUpdateListCount )
            {
                mrTextNode.DoNum(
                    [](SwNodeNum & rNum) { rNum.InvalidateAndNotifyTree(); });
            }
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
    xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextNode"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"), BAD_CAST(OString::number(GetIndex()).getStr()));

    OUString sText = GetText();
    for (int i = 0; i < 32; ++i)
        sText = sText.replace(i, '*');
    xmlTextWriterStartElement(pWriter, BAD_CAST("m_Text"));
    xmlTextWriterWriteString(pWriter, BAD_CAST(sText.toUtf8().getStr()));
    xmlTextWriterEndElement(pWriter);

    if (GetFormatColl())
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("SwTextFormatColl"));
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST("name"), BAD_CAST(GetFormatColl()->GetName().toUtf8().getStr()));
        xmlTextWriterEndElement(pWriter);
    }

    if (HasSwAttrSet())
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("SwAttrSet"));
        GetSwAttrSet().dumpAsXml(pWriter);
        xmlTextWriterEndElement(pWriter);
    }

    if (HasHints())
    {
        xmlTextWriterStartElement(pWriter, BAD_CAST("SwpHints"));
        const SwpHints& rHints = GetSwpHints();
        for (size_t i = 0; i < rHints.Count(); ++i)
            rHints.Get(i)->dumpAsXml(pWriter);
        xmlTextWriterEndElement(pWriter);
    }

    if (GetNumRule())
        GetNumRule()->dumpAsXml(pWriter);

    xmlTextWriterEndElement(pWriter);
}

sal_uInt32 SwTextNode::GetRsid( sal_Int32 nStt, sal_Int32 nEnd ) const
{
    SfxItemSet aSet( const_cast<SfxItemPool&>(static_cast<SfxItemPool const &>(GetDoc()->GetAttrPool())), svl::Items<RES_CHRATR_RSID, RES_CHRATR_RSID>{} );
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
    return GetDoc()->GetXmlIdRegistry();
}

bool SwTextNode::IsInClipboard() const
{
    return GetDoc()->IsClipBoard();
}

bool SwTextNode::IsInUndo() const
{
    return GetDoc()->GetIDocumentUndoRedo().IsUndoNodes(GetNodes());
}

bool SwTextNode::IsInContent() const
{
    return !GetDoc()->IsInHeaderFooter( SwNodeIndex(*this) );
}

void SwTextNode::SwClientNotify( const SwModify& rModify, const SfxHint& rHint )
{
    if (auto pLegacyHint = dynamic_cast<const sw::LegacyModifyHint*>(&rHint))
    {
        bool bWasNotifiable = m_bNotifiable;
        m_bNotifiable = false;

        const auto pOldValue = pLegacyHint->m_pOld;
        const auto pNewValue = pLegacyHint->m_pNew;
        // Override Modify so that deleting styles works properly (outline
        // numbering!).
        // Never call ChgTextCollUpdateNum for Nodes in Undo.
        if( pOldValue
                && pNewValue
                && RES_FMT_CHG == pOldValue->Which()
                && GetRegisteredIn() == static_cast<const SwFormatChg*>(pNewValue)->pChangedFormat
                && GetNodes().IsDocNodes() )
        {
            ChgTextCollUpdateNum(
                    static_cast<const SwTextFormatColl*>(static_cast<const SwFormatChg*>(pOldValue)->pChangedFormat),
                    static_cast<const SwTextFormatColl*>(static_cast<const SwFormatChg*>(pNewValue)->pChangedFormat) );
        }

        // reset fill information
        if (maFillAttributes.get() && pNewValue)
        {
            const sal_uInt16 nWhich = pNewValue->Which();
            bool bReset(RES_FMT_CHG == nWhich); // ..on format change (e.g. style changed)

            if(!bReset && RES_ATTRSET_CHG == nWhich) // ..on ItemChange from DrawingLayer FillAttributes
            {
                SfxItemIter aIter(*static_cast<const SwAttrSetChg*>(pNewValue)->GetChgSet());

                for(const SfxPoolItem* pItem = aIter.FirstItem(); pItem && !bReset; pItem = aIter.NextItem())
                {
                    bReset = !IsInvalidItem(pItem) && pItem->Which() >= XATTR_FILL_FIRST && pItem->Which() <= XATTR_FILL_LAST;
                }
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

        SwContentNode::SwClientNotify(rModify, rHint);

        SwDoc* pDoc = GetDoc();
        // #125329# - assure that text node is in document nodes array
        if ( pDoc && !pDoc->IsInDtor() && &pDoc->GetNodes() == &GetNodes() )
        {
            pDoc->GetNodes().UpdateOutlineNode(*this);
        }

        m_bNotifiable = bWasNotifiable;

        if (pOldValue && (RES_REMOVE_UNO_OBJECT == pOldValue->Which()))
        {   // invalidate cached uno object
            SetXParagraph(css::uno::Reference<css::text::XTextContent>(nullptr));
        }
    }
    else if (dynamic_cast<const SwAttrHint*>(&rHint))
    {
        if (&rModify == GetRegisteredIn())
            ChkCondColl();
    }
}

uno::Reference< rdf::XMetadatable >
SwTextNode::MakeUnoObject()
{
    const uno::Reference<rdf::XMetadatable> xMeta(
            SwXParagraph::CreateXParagraph(*GetDoc(), this), uno::UNO_QUERY);
    return xMeta;
}

drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwTextNode::getSdrAllFillAttributesHelper() const
{
    // create SdrAllFillAttributesHelper on demand
    if(!maFillAttributes.get())
    {
        const_cast< SwTextNode* >(this)->maFillAttributes.reset(new drawinglayer::attribute::SdrAllFillAttributesHelper(GetSwAttrSet()));
    }

    return maFillAttributes;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
