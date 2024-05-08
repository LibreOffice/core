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

#include <UndoDelete.hxx>

#include <libxml/xmlwriter.h>
#include <editeng/formatbreakitem.hxx>

#include <hintids.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <unotools/charclass.hxx>
#include <frmfmt.hxx>
#include <fmtanchr.hxx>
#include <doc.hxx>
#include <UndoManager.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <swtable.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <poolfmt.hxx>
#include <mvsave.hxx>
#include <docary.hxx>
#include <frmtool.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <strings.hrc>
#include <frameformats.hxx>
#include <fmtpdsc.hxx>
#include <vector>

// DELETE
/*  lcl_MakeAutoFrames has to call MakeFrames for objects bounded "AtChar"
    ( == AUTO ), if the anchor frame has be moved via MoveNodes(..) and
    DelFrames(..)
*/
static void lcl_MakeAutoFrames(const sw::FrameFormats<sw::SpzFrameFormat*>& rSpzs, SwNodeOffset nMovedIndex )
{
    for(auto pSpz: rSpzs)
    {
        const SwFormatAnchor* pAnchor = &pSpz->GetAnchor();
        if (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_CHAR)
        {
            const SwNode* pAnchorNode = pAnchor->GetAnchorNode();
            if( pAnchorNode && nMovedIndex == pAnchorNode->GetIndex() )
                pSpz->MakeFrames();
        }
    }
}

static SwTextNode * FindFirstAndNextNode(SwDoc & rDoc, SwUndRng const& rRange,
        SwRedlineSaveDatas const& rRedlineSaveData,
        SwTextNode *& o_rpFirstMergedDeletedTextNode)
{
    // redlines are corrected now to exclude the deleted node
    assert(rRange.m_nEndContent == 0);
    SwNodeOffset nEndOfRedline(0);
    for (size_t i = 0; i < rRedlineSaveData.size(); ++i)
    {
        auto const& rRedline(rRedlineSaveData[i]);
        if (rRedline.m_nSttNode <= rRange.m_nSttNode
            // coverity[copy_paste_error : FALSE] : m_nEndNode is intentional here
            && rRedline.m_nSttNode < rRange.m_nEndNode
            && rRange.m_nEndNode <= rRedline.m_nEndNode
            && rRedline.GetType() == RedlineType::Delete)
        {
            nEndOfRedline = rRedline.m_nEndNode;
            o_rpFirstMergedDeletedTextNode = rDoc.GetNodes()[rRedline.m_nSttNode]->GetTextNode();
            assert(rRange.m_nSttNode == rRange.m_nEndNode - 1); // otherwise this needs to iterate more RL to find the first node?
            break;
        }
    }
    if (nEndOfRedline)
    {
        assert(o_rpFirstMergedDeletedTextNode);
        SwTextNode * pNextNode(nullptr);
        for (SwNodeOffset i = rRange.m_nEndNode; /* i <= nEndOfRedline */; ++i)
        {
            SwNode *const pNode(rDoc.GetNodes()[i]);
            assert(!pNode->IsEndNode()); // cannot be both leaving section here *and* overlapping redline
            if (pNode->IsStartNode())
            {
                i = pNode->EndOfSectionIndex(); // will be incremented again
            }
            else if (pNode->IsTextNode())
            {
                pNextNode = pNode->GetTextNode();
                break;
            }
        }
        assert(pNextNode);
        return pNextNode;
    }
    else
    {
        return nullptr;
    }
}

static void DelFullParaMoveFrames(SwDoc & rDoc, SwUndRng const& rRange,
        SwRedlineSaveDatas const& rRedlineSaveData)
{
    SwTextNode * pFirstMergedDeletedTextNode(nullptr);
    SwTextNode *const pNextNode = FindFirstAndNextNode(rDoc, rRange,
            rRedlineSaveData, pFirstMergedDeletedTextNode);
    if (!pNextNode)
        return;

    std::vector<SwTextFrame*> frames;
    SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pFirstMergedDeletedTextNode);
    for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
    {
        if (pFrame->getRootFrame()->HasMergedParas())
        {
            assert(pFrame->GetMergedPara());
            assert(pFrame->GetMergedPara()->pFirstNode == pFirstMergedDeletedTextNode);
            assert(pNextNode->GetIndex() <= pFrame->GetMergedPara()->pLastNode->GetIndex());
            frames.push_back(pFrame);
        }
    }
    for (SwTextFrame *const pFrame : frames)
    {
        // sw_redlinehide: don't need FrameMode::Existing here
        // because everything from pNextNode onwards is already
        // correctly hidden
        pFrame->RegisterToNode(*pNextNode, true);
    }
}

// SwUndoDelete has to perform a deletion and to record anything that is needed
// to restore the situation before the deletion. Unfortunately a part of the
// deletion will be done after calling this Ctor, this has to be kept in mind!
// In this Ctor only the complete paragraphs will be deleted, the joining of
// the first and last paragraph of the selection will be handled outside this
// function.
// Here are the main steps of the function:
// 1. Deletion/recording of content indices of the selection: footnotes, fly
//    frames and bookmarks
// Step 1 could shift all nodes by deletion of footnotes => nNdDiff will be set.
// 2. If the paragraph where the selection ends, is the last content of a
//    section so that this section becomes empty when the paragraphs will be
//    joined we have to do some smart actions ;-) The paragraph will be moved
//    outside the section and replaced by a dummy text node, the complete
//    section will be deleted in step 3. The difference between replacement
//    dummy and original is nReplacementDummy.
// 3. Moving complete selected nodes into the UndoArray. Before this happens the
//    selection has to be extended if there are sections which would become
//    empty otherwise. BTW: sections will be moved into the UndoArray if they
//    are complete part of the selection. Sections starting or ending outside
//    of the selection will not be removed from the DocNodeArray even they got
//    a "dummy"-copy in the UndoArray.
// 4. We have to anticipate the joining of the two paragraphs if the start
//    paragraph is inside a section and the end paragraph not. Then we have to
//    move the paragraph into this section and to record this in nSectDiff.
SwUndoDelete::SwUndoDelete(
    SwPaM& rPam,
    SwDeleteFlags const flags,
    bool bFullPara,
    bool bCalledByTableCpy )
    : SwUndo(SwUndoId::DELETE, &rPam.GetDoc()),
    SwUndRng( rPam ),
    m_nNode(0),
    m_nNdDiff(0),
    m_nSectDiff(0),
    m_nReplaceDummy(0),
    m_nSetPos(0),
    m_bGroup( false ),
    m_bBackSp( false ),
    m_bJoinNext( false ),
    m_bTableDelLastNd( false ),
    // bFullPara is set e.g. if an empty paragraph before a table is deleted
    m_bDelFullPara( bFullPara ),
    m_bResetPgDesc( false ),
    m_bResetPgBrk( false ),
    m_bFromTableCopy( bCalledByTableCpy )
    , m_DeleteFlags(flags)
{
    assert(!m_bDelFullPara || !(m_DeleteFlags & SwDeleteFlags::ArtificialSelection));

    m_bCacheComment = false;

    SwDoc& rDoc = rPam.GetDoc();

    if( !rDoc.getIDocumentRedlineAccess().IsIgnoreRedline() && !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )
    {
        m_pRedlSaveData.reset(new SwRedlineSaveDatas);
        if( !FillSaveData( rPam, *m_pRedlSaveData ))
        {
            m_pRedlSaveData.reset();
        }
    }

    if( !m_pHistory )
        m_pHistory.reset( new SwHistory );

    // delete all footnotes for now
    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*

    // Step 1. deletion/record of content indices
    if( m_bDelFullPara )
    {
        OSL_ENSURE( rPam.HasMark(), "PaM without Mark" );
        DelContentIndex( *rPam.GetMark(), *rPam.GetPoint(),
                        DelContentType(DelContentType::AllMask | DelContentType::CheckNoCntnt) );

        ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
        DelBookmarks(pStt->GetNode(), pEnd->GetNode());
    }
    else
    {
        DelContentIndex(*rPam.GetMark(), *rPam.GetPoint(),
            DelContentType::AllMask
            | ((m_DeleteFlags & SwDeleteFlags::ArtificialSelection) ? DelContentType::Replace : DelContentType(0)));
        ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
        if (m_nEndNode - m_nSttNode > SwNodeOffset(1)) // check for fully selected nodes
        {
            SwNodeIndex const start(pStt->GetNode(), +1);
            DelBookmarks(start.GetNode(), pEnd->GetNode());
        }
    }

    m_nSetPos = m_pHistory ? m_pHistory->Count() : 0;

    // Is already anything deleted?
    m_nNdDiff = m_nSttNode - pStt->GetNodeIndex();

    m_bJoinNext = !bFullPara && pEnd == rPam.GetPoint();
    m_bBackSp = !bFullPara && !m_bJoinNext;

    SwTextNode *pSttTextNd = nullptr, *pEndTextNd = nullptr;
    if( !bFullPara )
    {
        pSttTextNd = pStt->GetNode().GetTextNode();
        pEndTextNd = m_nSttNode == m_nEndNode
                    ? pSttTextNd
                    : pEnd->GetNode().GetTextNode();
    }
    else if (m_pRedlSaveData)
    {
        DelFullParaMoveFrames(rDoc, *this, *m_pRedlSaveData);
    }

    bool bMoveNds = *pStt != *pEnd      // any area still existent?
                && ( SaveContent( pStt, pEnd, pSttTextNd, pEndTextNd ) || m_bFromTableCopy );

    if( pSttTextNd && pEndTextNd && pSttTextNd != pEndTextNd )
    {
        // two different TextNodes, thus save also the TextFormatCollection
        m_pHistory->AddColl(pSttTextNd->GetTextColl(), pStt->GetNodeIndex(), SwNodeType::Text);
        m_pHistory->AddColl(pEndTextNd->GetTextColl(), pEnd->GetNodeIndex(), SwNodeType::Text);

        if( !m_bJoinNext )        // Selection from bottom to top
        {
            // When using JoinPrev() all AUTO-PageBreak's will be copied
            // correctly. To restore them with UNDO, Auto-PageBreak of the
            // EndNode needs to be reset. Same for PageDesc and ColBreak.
            if( pEndTextNd->HasSwAttrSet() )
            {
                SwRegHistory aRegHist( *pEndTextNd, m_pHistory.get() );
                if( SfxItemState::SET == pEndTextNd->GetpSwAttrSet()->GetItemState(
                        RES_BREAK, false ) )
                    pEndTextNd->ResetAttr( RES_BREAK );
                if( pEndTextNd->HasSwAttrSet() &&
                    SfxItemState::SET == pEndTextNd->GetpSwAttrSet()->GetItemState(
                        RES_PAGEDESC, false ) )
                    pEndTextNd->ResetAttr( RES_PAGEDESC );
            }
        }
    }

    // Move now also the PaM. The SPoint is at the beginning of a SSelection.
    if( pEnd == rPam.GetPoint() && ( !bFullPara || pSttTextNd || pEndTextNd ) )
        rPam.Exchange();

    if( !pSttTextNd && !pEndTextNd )
        rPam.GetPoint()->Adjust(SwNodeOffset(-1));
    rPam.DeleteMark();          // the SPoint is in the selection

    if( !pEndTextNd )
        m_nEndContent = 0;
    if( !pSttTextNd )
        m_nSttContent = 0;

    if( bMoveNds )      // Do Nodes exist that need to be moved?
    {
        SwNodes& rNds = rDoc.GetUndoManager().GetUndoNodes();
        SwNodes& rDocNds = rDoc.GetNodes();
        SwNodeRange aRg( rDocNds, m_nSttNode - m_nNdDiff, m_nEndNode - m_nNdDiff );
        if( !bFullPara && !pEndTextNd &&
            aRg.aEnd.GetNode() != rDoc.GetNodes().GetEndOfContent() )
        {
            SwNode* pNode = aRg.aEnd.GetNode().StartOfSectionNode();
            if( pNode->GetIndex() >= m_nSttNode - m_nNdDiff )
                ++aRg.aEnd; // Deletion of a complete table
        }
        SwNode* pTmpNd;
        // Step 2: Expand selection if necessary
        if( m_bJoinNext || bFullPara )
        {
            // If all content of a section will be moved into Undo, the section
            // itself should be moved completely.
            while( aRg.aEnd.GetIndex() + 2  < rDocNds.Count() &&
                ( (pTmpNd = rDocNds[ aRg.aEnd.GetIndex()+1 ])->IsEndNode() &&
                pTmpNd->StartOfSectionNode()->IsSectionNode() &&
                pTmpNd->StartOfSectionNode()->GetIndex() >= aRg.aStart.GetIndex() ) )
                ++aRg.aEnd;
            m_nReplaceDummy = aRg.aEnd.GetIndex() + m_nNdDiff - m_nEndNode;
            if( m_nReplaceDummy )
            {   // The selection has been expanded, because
                ++aRg.aEnd;
                if( pEndTextNd )
                {
                    // The end text node has to leave the (expanded) selection
                    // The dummy is needed because MoveNodes deletes empty
                    // sections
                    ++m_nReplaceDummy;
                    SwNodeRange aMvRg( *pEndTextNd, SwNodeOffset(0), *pEndTextNd, SwNodeOffset(1) );
                    SwPosition aSplitPos( *pEndTextNd );
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    rDoc.getIDocumentContentOperations().SplitNode( aSplitPos, false );
                    rDocNds.MoveNodes( aMvRg, rDocNds, aRg.aEnd.GetNode() );
                    --aRg.aEnd;
                }
                else
                    m_nReplaceDummy = SwNodeOffset(0);
            }
        }
        if( m_bBackSp || bFullPara )
        {
            // See above, the selection has to be expanded if there are "nearly
            // empty" sections and a replacement dummy has to be set if needed.
            while( SwNodeOffset(1) < aRg.aStart.GetIndex() &&
                ( (pTmpNd = rDocNds[ aRg.aStart.GetIndex()-1 ])->IsSectionNode() &&
                pTmpNd->EndOfSectionIndex() < aRg.aEnd.GetIndex() ) )
                --aRg.aStart;
            if( pSttTextNd )
            {
                m_nReplaceDummy = m_nSttNode - m_nNdDiff - aRg.aStart.GetIndex();
                if( m_nReplaceDummy )
                {
                    SwNodeRange aMvRg( *pSttTextNd, SwNodeOffset(0), *pSttTextNd, SwNodeOffset(1) );
                    SwPosition aSplitPos( *pSttTextNd );
                    ::sw::UndoGuard const ug(rDoc.GetIDocumentUndoRedo());
                    rDoc.getIDocumentContentOperations().SplitNode( aSplitPos, false );
                    rDocNds.MoveNodes( aMvRg, rDocNds, aRg.aStart.GetNode() );
                    --aRg.aStart;
                }
            }
        }

        if( m_bFromTableCopy )
        {
            if( !pEndTextNd )
            {
                if( pSttTextNd )
                    ++aRg.aStart;
                else if( !bFullPara && !aRg.aEnd.GetNode().IsContentNode() )
                    --aRg.aEnd;
            }
        }
        else if (pSttTextNd && (pEndTextNd || pSttTextNd->GetText().getLength()))
            ++aRg.aStart;

        // Step 3: Moving into UndoArray...
        m_nNode = rNds.GetEndOfContent().GetIndex();
        rDocNds.MoveNodes( aRg, rNds, rNds.GetEndOfContent() );
        m_oMvStt.emplace( rNds, m_nNode );
        // remember difference!
        m_nNode = rNds.GetEndOfContent().GetIndex() - m_nNode;

        if( pSttTextNd && pEndTextNd )
        {
            //Step 4: Moving around sections
            m_nSectDiff = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            // nSect is the number of sections which starts(ends) between start
            // and end node of the selection. The "loser" paragraph has to be
            // moved into the section(s) of the "winner" paragraph
            if( m_nSectDiff )
            {
                if( m_bJoinNext )
                {
                    SwNodeRange aMvRg( *pEndTextNd, SwNodeOffset(0), *pEndTextNd, SwNodeOffset(1) );
                    rDocNds.MoveNodes( aMvRg, rDocNds, aRg.aStart.GetNode() );
                }
                else
                {
                    SwNodeRange aMvRg( *pSttTextNd, SwNodeOffset(0), *pSttTextNd, SwNodeOffset(1) );
                    rDocNds.MoveNodes( aMvRg, rDocNds, aRg.aEnd.GetNode() );
                }
            }
        }
        if( m_nSectDiff || m_nReplaceDummy )
        {
            SwNodeOffset nIndex;
            if (m_bJoinNext)
            {
                assert(pEndTextNd);
                nIndex = pEndTextNd->GetIndex();
            }
            else
            {
                assert(pSttTextNd);
                nIndex = pSttTextNd->GetIndex();
            }
            lcl_MakeAutoFrames( *rDoc.GetSpzFrameFormats(), nIndex);
        }
    }
    else
        m_nNode = SwNodeOffset(0);      // moved no node -> no difference at the end

    // Are there any Nodes that got deleted before that (FootNotes
    // have ContentNodes)?
    if( !pSttTextNd && !pEndTextNd )
    {
        m_nNdDiff = m_nSttNode - rPam.GetPoint()->GetNodeIndex() - (bFullPara ? 0 : 1);
        rPam.Move( fnMoveForward, GoInNode );
    }
    else
    {
        m_nNdDiff = m_nSttNode;
        if( m_nSectDiff && m_bBackSp )
            m_nNdDiff += m_nSectDiff;
        m_nNdDiff -= rPam.GetPoint()->GetNodeIndex();
    }

    // is a history necessary here at all?
    if( m_pHistory && !m_pHistory->Count() )
        m_pHistory.reset();
}

bool SwUndoDelete::SaveContent( const SwPosition* pStt, const SwPosition* pEnd,
                    SwTextNode* pSttTextNd, SwTextNode* pEndTextNd )
{
    SwNodeOffset nNdIdx = pStt->GetNodeIndex();
    // 1 - copy start in Start-String
    if( pSttTextNd )
    {
        bool bOneNode = m_nSttNode == m_nEndNode;
        SwRegHistory aRHst( *pSttTextNd, m_pHistory.get() );
        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        m_pHistory->CopyAttr( pSttTextNd->GetpSwpHints(), nNdIdx,
                            0, pSttTextNd->GetText().getLength(), true );
        if( !bOneNode && pSttTextNd->HasSwAttrSet() )
                m_pHistory->CopyFormatAttr( *pSttTextNd->GetpSwAttrSet(), nNdIdx );

        // the length might have changed (!!Fields!!)
        sal_Int32 nLen = (bOneNode
                    ? pEnd->GetContentIndex()
                    : pSttTextNd->GetText().getLength())
            - pStt->GetContentIndex();

        // delete now also the text (all attribute changes are added to
        // UNDO history)
        m_aSttStr = pSttTextNd->GetText().copy(m_nSttContent, nLen);
        pSttTextNd->EraseText( *pStt, nLen );
        if( pSttTextNd->GetpSwpHints() )
            pSttTextNd->GetpSwpHints()->DeRegister();

        // METADATA: store
        bool emptied( !m_aSttStr->isEmpty() && !pSttTextNd->Len() );
        if (!bOneNode || emptied) // merging may overwrite xmlids...
        {
            m_pMetadataUndoStart = emptied
                ? pSttTextNd->CreateUndoForDelete()
                : pSttTextNd->CreateUndo();
        }

        if( bOneNode )
            return false;           // stop moving more nodes
    }

    // 2 - copy end into End-String
    if( pEndTextNd )
    {
        SwContentIndex aEndIdx( pEndTextNd );
        nNdIdx = pEnd->GetNodeIndex();
        SwRegHistory aRHst( *pEndTextNd, m_pHistory.get() );

        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        m_pHistory->CopyAttr( pEndTextNd->GetpSwpHints(), nNdIdx, 0,
                            pEndTextNd->GetText().getLength(), true );

        if( pEndTextNd->HasSwAttrSet() )
            m_pHistory->CopyFormatAttr( *pEndTextNd->GetpSwAttrSet(), nNdIdx );

        // delete now also the text (all attribute changes are added to
        // UNDO history)
        m_aEndStr = pEndTextNd->GetText().copy( 0, pEnd->GetContentIndex() );
        pEndTextNd->EraseText( aEndIdx, pEnd->GetContentIndex() );
        if( pEndTextNd->GetpSwpHints() )
            pEndTextNd->GetpSwpHints()->DeRegister();

        // METADATA: store
        bool emptied = !m_aEndStr->isEmpty() && !pEndTextNd->Len();

        m_pMetadataUndoEnd = emptied
            ? pEndTextNd->CreateUndoForDelete()
            : pEndTextNd->CreateUndo();
    }

    // if there are only two Nodes then we're done
    if( ( pSttTextNd || pEndTextNd ) && m_nSttNode + 1 == m_nEndNode )
        return false;           // do not move any Node

    return true;                // move Nodes lying in between
}

bool SwUndoDelete::CanGrouping( SwDoc& rDoc, const SwPaM& rDelPam )
{
    // Is Undo greater than one Node (that is Start and EndString)?
    if( !m_aSttStr || m_aSttStr->isEmpty() || m_aEndStr )
        return false;

    // only the deletion of single char's can be condensed
    if( m_nSttNode != m_nEndNode || ( !m_bGroup && m_nSttContent+1 != m_nEndContent ))
        return false;

    auto [pStt, pEnd] = rDelPam.StartEnd(); // SwPosition*

    if( pStt->GetNode() != pEnd->GetNode() ||
        pStt->GetContentIndex()+1 != pEnd->GetContentIndex() ||
        pEnd->GetNodeIndex() != m_nSttNode )
        return false;

    // Distinguish between BackSpace and Delete because the Undo array needs to
    // be constructed differently!
    if( pEnd->GetContentIndex() == m_nSttContent )
    {
        if( m_bGroup && !m_bBackSp ) return false;
        m_bBackSp = true;
    }
    // note: compare m_nSttContent here because the text isn't there any more!
    else if( pStt->GetContentIndex() == m_nSttContent )
    {
        if( m_bGroup && m_bBackSp ) return false;
        m_bBackSp = false;
    }
    else
        return false;

    // are both Nodes (Node/Undo array) TextNodes at all?
    SwTextNode * pDelTextNd = pStt->GetNode().GetTextNode();
    if( !pDelTextNd ) return false;

    sal_Int32 nUChrPos = m_bBackSp ? 0 : m_aSttStr->getLength()-1;
    sal_Unicode cDelChar = pDelTextNd->GetText()[ pStt->GetContentIndex() ];
    CharClass& rCC = GetAppCharClass();
    if( ( CH_TXTATR_BREAKWORD == cDelChar || CH_TXTATR_INWORD == cDelChar ) ||
        rCC.isLetterNumeric( OUString( cDelChar ), 0 ) !=
        rCC.isLetterNumeric( *m_aSttStr, nUChrPos ) )
        return false;

    // tdf#132725 - if at-char/at-para flys would be deleted, don't group!
    // DelContentIndex() would be called at the wrong time here, the indexes
    // in the stored SwHistoryTextFlyCnt would be wrong when Undo is invoked
    if (IsFlySelectedByCursor(rDoc, *pStt, *pEnd))
    {
        return false;
    }

    if ((m_DeleteFlags & SwDeleteFlags::ArtificialSelection) && m_pHistory)
    {
        IDocumentMarkAccess const& rIDMA(*rDoc.getIDocumentMarkAccess());
        for (auto i = m_pHistory->Count(); 0 < i; )
        {
            --i;
            SwHistoryHint const*const pHistory((*m_pHistory)[i]);
            if (pHistory->Which() == HSTRY_BOOKMARK)
            {
                SwHistoryBookmark const*const pHistoryBM(
                        static_cast<SwHistoryBookmark const*>(pHistory));
                auto const ppMark(rIDMA.findMark(pHistoryBM->GetName()));
                if (ppMark != rIDMA.getAllMarksEnd()
                    && (m_bBackSp
                            ? ((**ppMark).GetMarkPos() == *pStt)
                            : ((**ppMark).IsExpanded()
                                && (**ppMark).GetOtherMarkPos() == *pEnd)))
                {   // prevent grouping that would delete this mark on Redo()
                    return false;
                }
            }
        }
    }

    {
        SwRedlineSaveDatas aTmpSav;
        const bool bSaved = FillSaveData( rDelPam, aTmpSav, false );

        bool bOk = ( !m_pRedlSaveData && !bSaved ) ||
                   ( m_pRedlSaveData && bSaved &&
                SwUndo::CanRedlineGroup( *m_pRedlSaveData, aTmpSav, m_bBackSp ));
        // aTmpSav.DeleteAndDestroyAll();
        if( !bOk )
            return false;

        rDoc.getIDocumentRedlineAccess().DeleteRedline( rDelPam, false, RedlineType::Any );
    }

    // Both 'deletes' can be consolidated, so 'move' the related character
    if( m_bBackSp )
        m_nSttContent--;    // BackSpace: add char to array!
    else
    {
        m_nEndContent++;    // Delete: attach char at the end
        nUChrPos++;
    }
    m_aSttStr = m_aSttStr->replaceAt( nUChrPos, 0, rtl::OUStringChar(cDelChar) );
    pDelTextNd->EraseText( *pStt, 1 );

    m_bGroup = true;
    return true;
}

SwUndoDelete::~SwUndoDelete()
{
    if( m_oMvStt )        // Delete also the selection from UndoNodes array
    {
        // Insert saves content in IconSection
        m_oMvStt->GetNode().GetNodes().Delete( *m_oMvStt, m_nNode );
        m_oMvStt.reset();
    }
    m_pRedlSaveData.reset();
}

static SwRewriter lcl_RewriterFromHistory(SwHistory & rHistory)
{
    SwRewriter aRewriter;

    bool bDone = false;

    for ( sal_uInt16 n = 0; n < rHistory.Count(); n++)
    {
        OUString aDescr = rHistory[n]->GetDescription();

        if (!aDescr.isEmpty())
        {
            aRewriter.AddRule(UndoArg2, aDescr);

            bDone = true;
            break;
        }
    }

    if (! bDone)
    {
        aRewriter.AddRule(UndoArg2, SwResId(STR_FIELD));
    }

    return aRewriter;
}

static bool lcl_IsSpecialCharacter(sal_Unicode nChar)
{
    switch (nChar)
    {
    case CH_TXTATR_BREAKWORD:
    case CH_TXTATR_INWORD:
    case CH_TXTATR_TAB:
    case CH_TXTATR_NEWLINE:
    case CH_TXT_ATR_INPUTFIELDSTART:
    case CH_TXT_ATR_INPUTFIELDEND:
    case CH_TXT_ATR_FORMELEMENT:
    case CH_TXT_ATR_FIELDSTART:
    case CH_TXT_ATR_FIELDSEP:
    case CH_TXT_ATR_FIELDEND:
        return true;

    default:
        break;
    }

    return false;
}

static OUString lcl_DenotedPortion(std::u16string_view rStr, sal_Int32 nStart, sal_Int32 nEnd, bool bQuoted)
{
    OUString aResult;

    auto nCount = nEnd - nStart;
    if (nCount > 0)
    {
        sal_Unicode cLast = rStr[nEnd - 1];
        if (lcl_IsSpecialCharacter(cLast))
        {
            switch(cLast)
            {
            case CH_TXTATR_TAB:
                aResult = SwResId(STR_UNDO_TABS, nCount);

                break;
            case CH_TXTATR_NEWLINE:
                aResult = SwResId(STR_UNDO_NLS, nCount);

                break;

            case CH_TXTATR_INWORD:
            case CH_TXTATR_BREAKWORD:
                aResult = SwRewriter::GetPlaceHolder(UndoArg2);
                break;

            case CH_TXT_ATR_INPUTFIELDSTART:
            case CH_TXT_ATR_INPUTFIELDEND:
            case CH_TXT_ATR_FORMELEMENT:
            case CH_TXT_ATR_FIELDSTART:
            case CH_TXT_ATR_FIELDSEP:
            case CH_TXT_ATR_FIELDEND:
                break; // nothing?

            default:
                assert(!"unexpected special character");
                break;
            }
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, OUString::number(nCount));
            aResult = aRewriter.Apply(aResult);
        }
        else if (bQuoted)
        {
            aResult = SwResId(STR_START_QUOTE) +
                rStr.substr(nStart, nCount) +
                SwResId(STR_END_QUOTE);
        }
        else
            aResult = rStr.substr(nStart, nCount);
    }

    return aResult;
}

OUString DenoteSpecialCharacters(std::u16string_view aStr, bool bQuoted)
{
    OUStringBuffer aResult;

    if (!aStr.empty())
    {
        bool bStart = false;
        sal_Int32 nStart = 0;
        sal_Unicode cLast = 0;

        for( size_t i = 0; i < aStr.size(); i++)
        {
            if (lcl_IsSpecialCharacter(aStr[i]))
            {
                if (cLast != aStr[i])
                    bStart = true;

            }
            else
            {
                if (lcl_IsSpecialCharacter(cLast))
                    bStart = true;
            }

            if (bStart)
            {
                aResult.append(lcl_DenotedPortion(aStr, nStart, i, bQuoted));

                nStart = i;
                bStart = false;
            }

            cLast = aStr[i];
        }

        aResult.append(lcl_DenotedPortion(aStr, nStart, aStr.size(), bQuoted));
    }
    else
        aResult = SwRewriter::GetPlaceHolder(UndoArg2);

    return aResult.makeStringAndClear();
}

SwRewriter SwUndoDelete::GetRewriter() const
{
    SwRewriter aResult;

    if (m_nNode != SwNodeOffset(0))
    {
        if (!m_sTableName.isEmpty())
        {

            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, SwResId(STR_START_QUOTE));
            aRewriter.AddRule(UndoArg2, m_sTableName);
            aRewriter.AddRule(UndoArg3, SwResId(STR_END_QUOTE));

            OUString sTmp = aRewriter.Apply(SwResId(STR_TABLE_NAME));
            aResult.AddRule(UndoArg1, sTmp);
        }
        else
            aResult.AddRule(UndoArg1, SwResId(STR_PARAGRAPHS));
    }
    else
    {
        OUString aStr;

        if (m_aSttStr && m_aEndStr && m_aSttStr->isEmpty() &&
            m_aEndStr->isEmpty())
        {
            aStr = SwResId(STR_PARAGRAPH_UNDO);
        }
        else
        {
            std::optional<OUString> aTmpStr;
            if (m_aSttStr)
                aTmpStr = m_aSttStr;
            else if (m_aEndStr)
                aTmpStr = m_aEndStr;

            if (aTmpStr)
            {
                aStr = DenoteSpecialCharacters(*aTmpStr);
            }
            else
            {
                aStr = SwRewriter::GetPlaceHolder(UndoArg2);
            }
        }

        aStr = ShortenString(aStr, nUndoStringLength, SwResId(STR_LDOTS));
        if (m_pHistory)
        {
            SwRewriter aRewriter = lcl_RewriterFromHistory(*m_pHistory);
            aStr = aRewriter.Apply(aStr);
        }

        aResult.AddRule(UndoArg1, aStr);
    }

    return aResult;
}

// Every object, anchored "AtContent" will be reanchored at rPos
static void lcl_ReAnchorAtContentFlyFrames(const sw::FrameFormats<sw::SpzFrameFormat*>& rSpzs, const SwPosition &rPos, SwNodeOffset nOldIdx )
{
    const SwFormatAnchor* pAnchor;
    for(auto pSpz: rSpzs)
    {
        pAnchor = &pSpz->GetAnchor();
        if (pAnchor->GetAnchorId() == RndStdIds::FLY_AT_PARA)
        {
            SwNode* pAnchorNode = pAnchor->GetAnchorNode();
            if( pAnchorNode && nOldIdx == pAnchorNode->GetIndex() )
            {
                SwFormatAnchor aAnch( *pAnchor );
                aAnch.SetAnchor( &rPos );
                pSpz->SetFormatAttr( aAnch );
            }
        }
    }
}

void SwUndoDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc& rDoc = rContext.GetDoc();

    SwNodeOffset nCalcStt = m_nSttNode - m_nNdDiff;

    if( m_nSectDiff && m_bBackSp )
        nCalcStt += m_nSectDiff;

    SwNodeIndex aIdx(rDoc.GetNodes(), nCalcStt);
    SwNode* pInsNd = &aIdx.GetNode();
    SwNode* pMovedNode = nullptr;

    {   // code block so that SwPosition is detached when deleting a Node
        SwPosition aPos( aIdx );
        if( !m_bDelFullPara )
        {
            assert(!m_bTableDelLastNd || pInsNd->IsTextNode());
            if( pInsNd->IsTableNode() )
            {
                pInsNd = rDoc.GetNodes().MakeTextNode( aIdx.GetNode(),
                        rDoc.GetDfltTextFormatColl() );
                --aIdx;
                aPos.Assign( *pInsNd->GetContentNode(), m_nSttContent );
            }
            else
            {
                if( pInsNd->IsContentNode() )
                    aPos.SetContent( m_nSttContent );
                if( !m_bTableDelLastNd )
                    pInsNd = nullptr;         // do not delete Node!
            }
        }
        else
            pInsNd = nullptr;         // do not delete Node!

        bool bNodeMove = SwNodeOffset(0) != m_nNode;

        if( m_aEndStr )
        {
            // discard attributes since they all saved!
            SwTextNode * pTextNd;
            if (!m_bDelFullPara && aPos.GetNode().IsSectionNode())
            {   // tdf#134250 section node wasn't deleted; but aPos must point to it in bNodeMove case below
                assert(m_nSttContent == 0);
                assert(!m_aSttStr);
                pTextNd = rDoc.GetNodes()[aPos.GetNodeIndex() + 1]->GetTextNode();
            }
            else
            {
                pTextNd = aPos.GetNode().GetTextNode();
            }

            if( pTextNd && pTextNd->HasSwAttrSet() )
                pTextNd->ResetAllAttr();

            if( pTextNd && pTextNd->GetpSwpHints() )
                pTextNd->ClearSwpHintsArr( true );

            if( m_aSttStr && !m_bFromTableCopy )
            {
                SwNodeOffset nOldIdx = aPos.GetNodeIndex();
                rDoc.getIDocumentContentOperations().SplitNode( aPos, false );
                // After the split all objects are anchored at the first
                // paragraph, but the pHistory of the fly frame formats relies
                // on anchoring at the start of the selection
                // => selection backwards needs a correction.
                if( m_bBackSp )
                    lcl_ReAnchorAtContentFlyFrames(*rDoc.GetSpzFrameFormats(), aPos, nOldIdx);
                pTextNd = aPos.GetNode().GetTextNode();
            }
            assert(pTextNd); // else where does m_aEndStr come from?
            if( pTextNd )
            {
                SwContentIndex aTmpIdx(pTextNd, aPos.GetContentIndex());
                OUString const ins( pTextNd->InsertText(*m_aEndStr, aTmpIdx,
                        SwInsertFlags::NOHINTEXPAND) );
                assert(ins.getLength() == m_aEndStr->getLength()); // must succeed
                (void) ins;
                // METADATA: restore
                pTextNd->RestoreMetadata(m_pMetadataUndoEnd);
            }
        }
        else if (m_aSttStr && bNodeMove && pInsNd == nullptr)
        {
            SwTextNode * pNd = aPos.GetNode().GetTextNode();
            if( pNd )
            {
                if (m_nSttContent < pNd->GetText().getLength())
                {
                    SwNodeOffset nOldIdx = aPos.GetNodeIndex();
                    rDoc.getIDocumentContentOperations().SplitNode( aPos, false );
                    if( m_bBackSp )
                        lcl_ReAnchorAtContentFlyFrames(*rDoc.GetSpzFrameFormats(), aPos, nOldIdx);
                }
                else
                    aPos.Adjust(SwNodeOffset(+1));
            }
        }
        if( m_nSectDiff )
        {
            SwNodeOffset nMoveIndex = aPos.GetNodeIndex();
            SwNodeOffset nDiff(0);
            if( m_bJoinNext )
            {
                nMoveIndex += m_nSectDiff + 1;
                pMovedNode = &aPos.GetNode();
            }
            else
            {
                nMoveIndex -= m_nSectDiff + 1;
                ++nDiff;
            }
            SwNodeIndex aMvIdx(rDoc.GetNodes(), nMoveIndex);
            SwNodeRange aRg( aPos.GetNode(), SwNodeOffset(0) - nDiff, aPos.GetNode(), SwNodeOffset(1) - nDiff );
            aPos.Adjust(SwNodeOffset(-1));
            if( !m_bJoinNext )
                pMovedNode = &aPos.GetNode();
            rDoc.GetNodes().MoveNodes(aRg, rDoc.GetNodes(), aMvIdx.GetNode());
            aPos.Adjust(SwNodeOffset(+1));
        }

        if( bNodeMove )
        {
            SwNodeRange aRange( *m_oMvStt, SwNodeOffset(0), *m_oMvStt, m_nNode );
            SwNodeIndex aCopyIndex( aPos.GetNode(), -1 );
            rDoc.GetUndoManager().GetUndoNodes().Copy_(aRange, aPos.GetNode(),
                    // sw_redlinehide: delay creating frames: the flags on the
                    // nodes aren't necessarily up-to-date, and the redlines
                    // from m_pRedlSaveData aren't applied yet...
                    false);

            if( m_nReplaceDummy )
            {
                SwNodeOffset nMoveIndex;
                if( m_bJoinNext )
                {
                    nMoveIndex = m_nEndNode - m_nNdDiff;
                    aPos.Assign( nMoveIndex + m_nReplaceDummy );
                }
                else
                {
                    aPos.Assign( aCopyIndex );
                    nMoveIndex = aPos.GetNodeIndex() + m_nReplaceDummy + 1;
                }
                SwNodeIndex aMvIdx(rDoc.GetNodes(), nMoveIndex);
                SwNodeRange aRg( aPos.GetNode(), SwNodeOffset(0), aPos.GetNode(), SwNodeOffset(1) );
                pMovedNode = &aPos.GetNode();
                // tdf#131684 without deleting frames
                rDoc.GetNodes().MoveNodes(aRg, rDoc.GetNodes(), aMvIdx.GetNode(), false);
                rDoc.GetNodes().Delete( aMvIdx);
            }
        }

        if( m_aSttStr )
        {
            aPos.Assign( m_nSttNode - m_nNdDiff + ( m_bJoinNext ? SwNodeOffset(0) : m_nReplaceDummy ) );
            SwTextNode * pTextNd = aPos.GetNode().GetTextNode();
            // If more than a single Node got deleted, also all "Node"
            // attributes were saved
            if (pTextNd != nullptr)
            {
                if( pTextNd->HasSwAttrSet() && bNodeMove && !m_aEndStr )
                    pTextNd->ResetAllAttr();

                if( pTextNd->GetpSwpHints() )
                    pTextNd->ClearSwpHintsArr( true );

                // SectionNode mode and selection from top to bottom:
                //  -> in StartNode is still the rest of the Join => delete
                aPos.SetContent( m_nSttContent );
                pTextNd->SetInSwUndo(true);
                OUString const ins( pTextNd->InsertText(*m_aSttStr, aPos,
                        SwInsertFlags::NOHINTEXPAND) );
                pTextNd->SetInSwUndo(false);
                assert(ins.getLength() == m_aSttStr->getLength()); // must succeed
                (void) ins;
                // METADATA: restore
                pTextNd->RestoreMetadata(m_pMetadataUndoStart);
            }
        }

        if( m_pHistory )
        {
            m_pHistory->TmpRollback(&rDoc, m_nSetPos, false);
            if( m_nSetPos )       // there were Footnodes/FlyFrames
            {
                // are there others than these ones?
                if( m_nSetPos < m_pHistory->Count() )
                {
                    // if so save the attributes of the others
                    SwHistory aHstr;
                    aHstr.Move( 0, m_pHistory.get(), m_nSetPos );
                    m_pHistory->Rollback(&rDoc);
                    m_pHistory->Move( 0, &aHstr );
                }
                else
                {
                    m_pHistory->Rollback(&rDoc);
                    m_pHistory.reset();
                }
            }
        }

        if( m_bResetPgDesc || m_bResetPgBrk )
        {
            sal_uInt16 nStt = m_bResetPgDesc ? sal_uInt16(RES_PAGEDESC) : sal_uInt16(RES_BREAK);
            sal_uInt16 nEnd = m_bResetPgBrk ? sal_uInt16(RES_BREAK) : sal_uInt16(RES_PAGEDESC);

            SwNode* pNode = rDoc.GetNodes()[ m_nEndNode + 1 ];
            if( pNode->IsContentNode() )
                static_cast<SwContentNode*>(pNode)->ResetAttr( nStt, nEnd );
            else if( pNode->IsTableNode() )
                static_cast<SwTableNode*>(pNode)->GetTable().GetFrameFormat()->ResetFormatAttr( nStt, nEnd );
        }
    }
    // delete the temporarily added Node
    if (pInsNd && !m_bTableDelLastNd)
    {
        assert(&aIdx.GetNode() == pInsNd);
        rDoc.GetNodes().Delete( aIdx );
    }
    if( m_pRedlSaveData )
        SetSaveData(rDoc, *m_pRedlSaveData);

    SwNodeOffset delFullParaEndNode(m_nEndNode);
    if (m_bDelFullPara && m_pRedlSaveData)
    {
        SwTextNode * pFirstMergedDeletedTextNode(nullptr);
        SwTextNode *const pNextNode = FindFirstAndNextNode(rDoc, *this,
                *m_pRedlSaveData, pFirstMergedDeletedTextNode);
        if (pNextNode)
        {
            bool bNonMerged(false);
            std::vector<SwTextFrame*> frames;
            SwIterator<SwTextFrame, SwTextNode, sw::IteratorMode::UnwrapMulti> aIter(*pNextNode);
            for (SwTextFrame* pFrame = aIter.First(); pFrame; pFrame = aIter.Next())
            {
                if (pFrame->getRootFrame()->HasMergedParas())
                {
                    frames.push_back(pFrame);
                }
                else
                {
                    bNonMerged = true;
                }
            }
            for (SwTextFrame *const pFrame : frames)
            {
                // could either destroy the text frames, or move them...
                // destroying them would have the advantage that we don't
                // need special code to *exclude* pFirstMergedDeletedTextNode
                // from MakeFrames  for the layouts in Hide mode but not
                // layouts in Show mode ...
                // ... except that MakeFrames won't create them then :(
                pFrame->RegisterToNode(*pFirstMergedDeletedTextNode);
                assert(pFrame->GetMergedPara());
                assert(!bNonMerged); // delFullParaEndNode is such an awful hack
                (void) bNonMerged;
                delFullParaEndNode = pFirstMergedDeletedTextNode->GetIndex();
            }
        }
    }
    else if (m_aSttStr && (!m_bFromTableCopy || SwNodeOffset(0) != m_nNode))
    {
        // only now do we have redlines in the document again; fix up the split
        // frames
        SwTextNode *const pStartNode(aIdx.GetNodes()[m_nSttNode]->GetTextNode());
        assert(pStartNode);
        sw::RecreateStartTextFrames(*pStartNode);
    }

    // create frames after SetSaveData has recreated redlines
    if (SwNodeOffset(0) != m_nNode)
    {
        // tdf#136453 only if section nodes at the start
        if (m_bBackSp && m_nReplaceDummy != SwNodeOffset(0))
        {
            // tdf#134252 *first* create outer section frames
            // note: text node m_nSttNode currently has frame with an upper;
            // there's a hack in InsertCnt_() to move it below new section frame
            SwNode& start(*rDoc.GetNodes()[m_nSttNode - m_nReplaceDummy]);
            SwNode& end(*rDoc.GetNodes()[m_nSttNode]); // exclude m_nSttNode
            ::MakeFrames(&rDoc, start, end);
        }
        // tdf#121031 if the start node is a text node, it already has a frame;
        // if it's a table, it does not
        // tdf#109376 exception: end on non-text-node -> start node was inserted
        assert(!m_bDelFullPara || (m_nSectDiff == SwNodeOffset(0)));
        SwNode& start(*rDoc.GetNodes()[m_nSttNode +
            ((m_bDelFullPara || !rDoc.GetNodes()[m_nSttNode]->IsTextNode() || pInsNd)
                 ? 0 : 1)]);
        // tdf#158740 fix crash by checking the end node's index
        // I don't know why m_nEndNode is larger than the size of the node
        // array, but adjusting m_nEndNode to the last element in the node
        // array stops the crashing.
        SwNodeOffset nCount(rDoc.GetNodes().Count());
        if (nCount > SwNodeOffset(0))
        {
            if (m_nEndNode > nCount - 1)
                m_nEndNode = nCount - 1;

            // don't include end node in the range: it may have been merged already
            // by the start node, or it may be merged by one of the moved nodes,
            // but if it isn't merged, its current frame(s) should be good...
            SwNode& end(*rDoc.GetNodes()[ m_bDelFullPara
                ? delFullParaEndNode
                // tdf#147310 SwDoc::DeleteRowCol() may delete whole table - end must be node following table!
                : (m_nEndNode + (rDoc.GetNodes()[m_nSttNode]->IsTableNode() && rDoc.GetNodes()[m_nEndNode]->IsEndNode() ? 1 : 0))]);
            ::MakeFrames(&rDoc, start, end);
        }
    }

    if (pMovedNode)
    {   // probably better do this after creating all frames
        lcl_MakeAutoFrames(*rDoc.GetSpzFrameFormats(), pMovedNode->GetIndex());
    }

    // tdf#134021 only after MakeFrames(), because it may be the only node
    // that has layout frames
    if (pInsNd && m_bTableDelLastNd)
    {
        assert(&aIdx.GetNode() == pInsNd);
        SwPaM tmp(aIdx, aIdx);
        rDoc.getIDocumentContentOperations().DelFullPara(tmp);
    }

    AddUndoRedoPaM(rContext, true);
}

void SwUndoDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam = AddUndoRedoPaM(rContext);
    SwDoc& rDoc = rPam.GetDoc();

    if( m_pRedlSaveData )
    {
        const bool bSuccess = FillSaveData(rPam, *m_pRedlSaveData);
        OSL_ENSURE(bSuccess,
            "SwUndoDelete::Redo: used to have redline data, but now none?");
        if (!bSuccess)
        {
            m_pRedlSaveData.reset();
        }
    }

    if( !m_bDelFullPara )
    {
        // tdf#128739 correct cursors but do not delete bookmarks yet
        ::PaMCorrAbs(rPam, *rPam.End());
        SetPaM(rPam);

        if( !m_bJoinNext )           // then restore selection from bottom to top
            rPam.Exchange();
    }

    if( m_pHistory )      // are the attributes saved?
    {
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
        SwHistory aHstr;
        aHstr.Move( 0, m_pHistory.get() );

        if( m_bDelFullPara )
        {
            OSL_ENSURE( rPam.HasMark(), "PaM without Mark" );
            DelContentIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelContentType(DelContentType::AllMask | DelContentType::CheckNoCntnt) );

            DelBookmarks(rPam.GetMark()->GetNode(), rPam.GetPoint()->GetNode());
        }
        else
        {
            DelContentIndex(*rPam.GetMark(), *rPam.GetPoint(),
                DelContentType::AllMask
                | ((m_DeleteFlags & SwDeleteFlags::ArtificialSelection) ? DelContentType::Replace : DelContentType(0)));
        }
        m_nSetPos = m_pHistory ? m_pHistory->Count() : 0;

        m_pHistory->Move( m_nSetPos, &aHstr );
    }
    else
    {
        if( m_bDelFullPara )
        {
            OSL_ENSURE( rPam.HasMark(), "PaM without Mark" );
            DelContentIndex( *rPam.GetMark(), *rPam.GetPoint(),
                            DelContentType(DelContentType::AllMask | DelContentType::CheckNoCntnt) );

            DelBookmarks( rPam.GetMark()->GetNode(), rPam.GetPoint()->GetNode() );
        }
        else
        {
            DelContentIndex(*rPam.GetMark(), *rPam.GetPoint(),
                DelContentType::AllMask
                | ((m_DeleteFlags & SwDeleteFlags::ArtificialSelection) ? DelContentType::Replace : DelContentType(0)));
        }
        m_nSetPos = m_pHistory ? m_pHistory->Count() : 0;
    }

    if( !m_aSttStr && !m_aEndStr )
    {
        if (m_bDelFullPara && m_pRedlSaveData)
        {
            DelFullParaMoveFrames(rDoc, *this, *m_pRedlSaveData);
        }

        SwNode& rSttNd = ( m_bDelFullPara || m_bJoinNext )
                                    ? rPam.GetMark()->GetNode()
                                    : rPam.GetPoint()->GetNode();
        SwTableNode* pTableNd = rSttNd.GetTableNode();
        if( pTableNd )
        {
            if( m_bTableDelLastNd )
            {
                // than add again a Node at the end
                const SwNodeIndex aTmpIdx( *pTableNd->EndOfSectionNode(), 1 );
                rDoc.GetNodes().MakeTextNode( aTmpIdx.GetNode(),
                        rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD ) );
            }

            SwContentNode* pNextNd = rDoc.GetNodes()[
                    pTableNd->EndOfSectionIndex()+1 ]->GetContentNode();
            if( pNextNd )
            {
                SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();

                if( const SwFormatPageDesc* pItem = pTableFormat->GetItemIfSet( RES_PAGEDESC,
                    false ) )
                    pNextNd->SetAttr( *pItem );

                if( const SvxFormatBreakItem* pItem = pTableFormat->GetItemIfSet( RES_BREAK,
                    false ) )
                    pNextNd->SetAttr( *pItem );
            }
            pTableNd->DelFrames();
        }
        else if (*rPam.GetMark() == *rPam.GetPoint())
        {   // paragraph with only footnote or as-char fly, delete that
            // => DelContentIndex has already deleted it! nothing to do here
            assert(m_nEndNode == m_nSttNode);
            return;
        }

        // avoid asserts from ~SwContentIndexReg for deleted nodes
        SwPaM aTmp(*rPam.End());
        if (!aTmp.Move(fnMoveForward, GoInNode))
        {
            *aTmp.GetPoint() = *rPam.Start();
            aTmp.Move(fnMoveBackward, GoInNode);
        }
        // coverity[copy_paste_error : FALSE] : GetNode() is intentional on both branches
        assert(aTmp.GetPoint()->GetNode() != rPam.GetPoint()->GetNode()
            && aTmp.GetPoint()->GetNode() != rPam.GetMark()->GetNode());
        ::PaMCorrAbs(rPam, *aTmp.GetPoint());

        rPam.DeleteMark();

        rDoc.GetNodes().Delete( rSttNd, m_nEndNode - m_nSttNode );
    }
    else if( m_bDelFullPara )
    {
        assert(!"dead code");
        // The Pam was incremented by one at Point (== end) to provide space
        // for UNDO. This now needs to be reverted!
        rPam.End()->Adjust(SwNodeOffset(-1));
        if( rPam.GetPoint()->GetNode() == rPam.GetMark()->GetNode() )
            *rPam.GetMark() = *rPam.GetPoint();
        rDoc.getIDocumentContentOperations().DelFullPara( rPam );
    }
    else
        rDoc.getIDocumentContentOperations().DeleteAndJoin(rPam, m_DeleteFlags);
}

void SwUndoDelete::RepeatImpl(::sw::RepeatContext & rContext)
{
    // this action does not seem idempotent,
    // so make sure it is only executed once on repeat
    if (rContext.m_bDeleteRepeated)
        return;

    SwPaM & rPam = rContext.GetRepeatPaM();
    SwDoc& rDoc = rPam.GetDoc();
    ::sw::GroupUndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    if( !rPam.HasMark() )
    {
        rPam.SetMark();
        rPam.Move( fnMoveForward, GoInContent );
    }
    if( m_bDelFullPara )
        rDoc.getIDocumentContentOperations().DelFullPara( rPam );
    else
        rDoc.getIDocumentContentOperations().DeleteAndJoin( rPam );
    rContext.m_bDeleteRepeated = true;
}

void SwUndoDelete::SetTableName(const OUString & rName)
{
    m_sTableName = rName;
}

void SwUndoDelete::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUndoDelete"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    SwUndo::dumpAsXml(pWriter);
    SwUndoSaveContent::dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
