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

#include <stdlib.h>
#include <libxml/xmlwriter.h>
#include <osl/diagnose.h>
#include <tools/json_writer.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <sfx2/viewsh.hxx>
#include <comphelper/lok.hxx>

#include <node.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pam.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <numrule.hxx>
#include <ndtxt.hxx>
#include <ndnotxt.hxx>
#include <swtable.hxx>
#include <section.hxx>
#include <ddefld.hxx>
#include <swddetbl.hxx>
#include <txtatr.hxx>
#include <tox.hxx>
#include <fmtrfmrk.hxx>
#include <fmtftn.hxx>
#include <docsh.hxx>
#include <rootfrm.hxx>
#include <txtfrm.hxx>

typedef std::vector<SwStartNode*> SwStartNodePointers;

// function to determine the highest level in the given range
static sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange );

/** Constructor
 *
 * creates the base sections (PostIts, Inserts, AutoText, RedLines, Content)
 *
 * @param rDocument TODO: provide documentation
 */
SwNodes::SwNodes( SwDoc& rDocument )
    : m_vIndices(nullptr), m_rMyDoc( rDocument )
{
    m_bInNodesDel = m_bInDelUpdOutline = false;

    SwNodeOffset nPos(0);
    SwStartNode* pSttNd = new SwStartNode( *this, nPos++ );
    m_pEndOfPostIts = new SwEndNode( *this, nPos++, *pSttNd );

    SwStartNode* pTmp = new SwStartNode( *this, nPos++ );
    m_pEndOfInserts = new SwEndNode( *this, nPos++, *pTmp );

    pTmp = new SwStartNode( *this, nPos++ );
    pTmp->m_pStartOfSection = pSttNd;
    m_pEndOfAutotext = new SwEndNode( *this, nPos++, *pTmp );

    pTmp = new SwStartNode( *this, nPos++ );
    pTmp->m_pStartOfSection = pSttNd;
    m_pEndOfRedlines = new SwEndNode( *this, nPos++, *pTmp );

    pTmp = new SwStartNode( *this, nPos++ );
    pTmp->m_pStartOfSection = pSttNd;
    m_pEndOfContent.reset(new SwEndNode( *this, nPos++, *pTmp ));

    m_aOutlineNodes.clear();
}

/** Destructor
 *
 * Deletes all nodes whose pointer are in a dynamic array. This should be no
 * problem as nodes cannot be created outside this array and, thus, cannot be
 * part of multiple arrays.
 */
SwNodes::~SwNodes()
{
    m_aOutlineNodes.clear();

    {
        SwNodeIndex aNdIdx( *this );
        while( true )
        {
            SwNode *pNode = &aNdIdx.GetNode();
            if( pNode == m_pEndOfContent.get() )
                break;

            ++aNdIdx;
            delete pNode;
        }
    }

    // here, all SwNodeIndices must be unregistered
    m_pEndOfContent.reset();
}

static bool IsInsertOutline(SwNodes const& rNodes, SwNodeOffset const nIndex)
{
    if (!rNodes.IsDocNodes())
    {
        return false;
    }
    return nIndex < rNodes.GetEndOfRedlines().StartOfSectionNode()->GetIndex()
        || rNodes.GetEndOfRedlines().GetIndex() < nIndex;
}

void SwNodes::ChgNode( SwNodeIndex const & rDelPos, SwNodeOffset nSz,
                        SwNodeIndex& rInsPos, bool bNewFrames )
{
    // no need for frames in the UndoArea
    SwNodes& rNds = rInsPos.GetNodes();
    const SwNode* pPrevInsNd = rNds[ rInsPos.GetIndex() -SwNodeOffset(1) ];

    // declare all fields as invalid, updating will happen
    // in the idle-handler of the doc
    if( GetDoc().getIDocumentFieldsAccess().SetFieldsDirty( true, &rDelPos.GetNode(), nSz ) &&
        &rNds.GetDoc() != &GetDoc() )
        rNds.GetDoc().getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, SwNodeOffset(0) );

    // NEVER include nodes from the RedLineArea
    SwNodeOffset nNd = rInsPos.GetIndex();
    bool const bInsOutlineIdx = IsInsertOutline(rNds, nNd);

    if( &rNds == this ) // if in the same node array -> move
    {
        // Move order: from front to back, so that new entries are added at
        // first position, thus, deletion position stays the same
        const SwNodeOffset nDiff(rDelPos.GetIndex() < rInsPos.GetIndex() ? 0 : 1);

        for( SwNodeOffset n = rDelPos.GetIndex(); nSz; n += nDiff, --nSz )
        {
            SwNodeIndex aDelIdx( *this, n );
            SwNode& rNd = aDelIdx.GetNode();

            // #i57920# - correction of refactoring done by cws swnumtree:
            // - <SwTextNode::SetLevel( NO_NUMBERING ) is deprecated and
            //   set <IsCounted> state of the text node to <false>, which
            //   isn't correct here.
            if ( rNd.IsTextNode() )
            {
                SwTextNode* pTextNode = rNd.GetTextNode();

                pTextNode->RemoveFromList();

                if (pTextNode->IsOutline())
                {
                    SwNode* pSrch = &rNd;
                    m_aOutlineNodes.erase( pSrch );
                }
            }

            BigPtrArray::Move( sal_Int32(aDelIdx.GetIndex()), sal_Int32(rInsPos.GetIndex()) );

            if( rNd.IsTextNode() )
            {
                SwTextNode& rTextNd = static_cast<SwTextNode&>(rNd);

                rTextNd.AddToList();

                if (bInsOutlineIdx && rTextNd.IsOutline())
                {
                    SwNode* pSrch = &rNd;
                    m_aOutlineNodes.insert( pSrch );
                }
                rTextNd.InvalidateNumRule();

                if( RES_CONDTXTFMTCOLL == rTextNd.GetTextColl()->Which() )
                    rTextNd.ChkCondColl();
            }
            else if( rNd.IsContentNode() )
                static_cast<SwContentNode&>(rNd).InvalidateNumRule();
        }
    }
    else
    {
        bool bSavePersData(GetDoc().GetIDocumentUndoRedo().IsUndoNodes(rNds));
        bool bRestPersData(GetDoc().GetIDocumentUndoRedo().IsUndoNodes(*this));
        SwDoc* pDestDoc = &rNds.GetDoc() != &GetDoc() ? &rNds.GetDoc() : nullptr;
        OSL_ENSURE(!pDestDoc, "SwNodes::ChgNode(): "
            "the code to handle text fields here looks broken\n"
            "if the target is in a different document.");
        if( !bRestPersData && !bSavePersData && pDestDoc )
            bSavePersData = bRestPersData = true;

        OUString sNumRule;
        for( SwNodeOffset n(0); n < nSz; n++ )
        {
            SwNode* pNd = &rDelPos.GetNode();

            // NoTextNode keep their persistent data
            if( pNd->IsNoTextNode() )
            {
                if( bSavePersData )
                    static_cast<SwNoTextNode*>(pNd)->SavePersistentData();
            }
            else if( pNd->IsTextNode() )
            {
                SwTextNode* pTextNd = static_cast<SwTextNode*>(pNd);

                // remove outline index from old nodes array
                if (pTextNd->IsOutline())
                {
                    m_aOutlineNodes.erase( pNd );
                }

                // copy rules if needed
                if( pDestDoc )
                {
                    const SwNumRule* pNumRule = pTextNd->GetNumRule();
                    if( pNumRule && sNumRule != pNumRule->GetName() )
                    {
                        sNumRule = pNumRule->GetName();
                        SwNumRule* pDestRule = pDestDoc->FindNumRulePtr( sNumRule );
                        if( pDestRule )
                            pDestRule->SetInvalidRule( true );
                        else
                            pDestDoc->MakeNumRule( sNumRule, pNumRule );
                    }
                }
                else
                {
                    // if movement into the UndoNodes-array, update numbering
                    if (sw::HasNumberingWhichNeedsLayoutUpdate(*pTextNd))
                    {
                        pTextNd->InvalidateNumRule();
                    }
                }

                pTextNd->RemoveFromList();
            }

            RemoveNode( rDelPos.GetIndex(), SwNodeOffset(1), false ); // move indices
            SwContentNode * pCNd = pNd->GetContentNode();
            rNds.InsertNode( pNd, rInsPos );

            if( pCNd )
            {
                SwTextNode* pTextNd = pCNd->GetTextNode();
                if( pTextNd )
                {
                    SwpHints * const pHts = pTextNd->GetpSwpHints();
                    // OutlineNodes set the new nodes in the array
                    if (bInsOutlineIdx && pTextNd->IsOutline())
                    {
                        rNds.m_aOutlineNodes.insert( pTextNd );
                    }

                    pTextNd->AddToList();

                    // special treatment for fields
                    if( pHts && pHts->Count() )
                    {
                        bool const bToUndo = !pDestDoc &&
                            GetDoc().GetIDocumentUndoRedo().IsUndoNodes(rNds);
                        for( size_t i = pHts->Count(); i; )
                        {
                            SwTextAttr * const pAttr = pHts->Get( --i );
                            switch ( pAttr->Which() )
                            {
                            case RES_TXTATR_FIELD:
                            case RES_TXTATR_ANNOTATION:
                            case RES_TXTATR_INPUTFIELD:
                                {
                                    SwTextField* pTextField = static_txtattr_cast<SwTextField*>(pAttr);
                                    rNds.GetDoc().getIDocumentFieldsAccess().InsDelFieldInFieldLst( !bToUndo, *pTextField );

                                    const SwFieldType* pTyp = pTextField->GetFormatField().GetField()->GetTyp();
                                    if ( SwFieldIds::Postit == pTyp->Which() )
                                    {
                                        rNds.GetDoc().GetDocShell()->Broadcast(
                                            SwFormatFieldHint(
                                                &pTextField->GetFormatField(),
                                                ( pTextField->GetFormatField().IsFieldInDoc()
                                                  ? SwFormatFieldHintWhich::INSERTED
                                                  : SwFormatFieldHintWhich::REMOVED ) ) );
                                    }
                                    else if( SwFieldIds::Dde == pTyp->Which() )
                                    {
                                        if( bToUndo )
                                            const_cast<SwDDEFieldType*>(static_cast<const SwDDEFieldType*>(pTyp))->DecRefCnt();
                                        else
                                            const_cast<SwDDEFieldType*>(static_cast<const SwDDEFieldType*>(pTyp))->IncRefCnt();
                                    }
                                    static_cast<SwFormatField&>(pAttr->GetAttr())
                                        .InvalidateField();
                                }
                                break;

                            case RES_TXTATR_FTN:
                                static_cast<SwFormatFootnote&>(pAttr->GetAttr())
                                    .InvalidateFootnote();
                                break;

                            case RES_TXTATR_TOXMARK:
                                static_cast<SwTOXMark&>(pAttr->GetAttr())
                                    .InvalidateTOXMark();
                                break;

                            case RES_TXTATR_REFMARK:
                                static_cast<SwFormatRefMark&>(pAttr->GetAttr())
                                    .InvalidateRefMark();
                                break;

                            case RES_TXTATR_META:
                            case RES_TXTATR_METAFIELD:
                                {
                                    SwTextMeta *const pTextMeta(
                                        static_txtattr_cast<SwTextMeta*>(pAttr));
                                    // force removal of UNO object
                                    pTextMeta->ChgTextNode(nullptr);
                                    pTextMeta->ChgTextNode(pTextNd);
                                }
                                break;

                            default:
                                break;
                            }
                        }
                    }

                    if( RES_CONDTXTFMTCOLL == pTextNd->GetTextColl()->Which() )
                        pTextNd->ChkCondColl();
                }
                else
                {
                    // Moved into different Docs? Persist data again!
                    if( pCNd->IsNoTextNode() && bRestPersData )
                        static_cast<SwNoTextNode*>(pCNd)->RestorePersistentData();
                }

                // reset Accessibility issue state
                pCNd->resetAndQueueAccessibilityCheck();
            }
        }
    }

    // declare all fields as invalid, updating will happen
    // in the idle-handler of the doc
    GetDoc().getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, SwNodeOffset(0) );
    if( &rNds.GetDoc() != &GetDoc() )
        rNds.GetDoc().getIDocumentFieldsAccess().SetFieldsDirty( true, nullptr, SwNodeOffset(0) );

    if( bNewFrames )
        bNewFrames = &GetDoc().GetNodes() == &rNds &&
                    GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell();

    if( !bNewFrames )
        return;

    // get the frames:
    SwNodeIndex aIdx( *pPrevInsNd, 1 );
    SwNode* pFrameNd = rNds.FindPrvNxtFrameNode( aIdx.GetNode(),
                                    rNds[ rInsPos.GetIndex() - 1 ] );

    if( !pFrameNd )
        return;

    while( aIdx != rInsPos )
    {
        SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
        if( pCNd )
        {
            if( pFrameNd->IsTableNode() )
                static_cast<SwTableNode*>(pFrameNd)->MakeFramesForAdjacentContentNode(aIdx);
            else if( pFrameNd->IsSectionNode() )
                static_cast<SwSectionNode*>(pFrameNd)->MakeFramesForAdjacentContentNode(aIdx);
            else
                static_cast<SwContentNode*>(pFrameNd)->MakeFramesForAdjacentContentNode(*pCNd);
            pFrameNd = pCNd;
        }
        ++aIdx;
    }
}

// TODO: provide documentation
/** move the node pointer
 *
 * Move the node pointer from "(inclusive) start position to (exclusive) end
 * position" to target position.
 * If the target is in front of the first or in the area between first and
 * last element to move, nothing happens.
 * If the area to move is empty or the end position is before the start
 * position, nothing happens.
 *
 * @param aRange range to move (excluding end node)
 * @return
 */
bool SwNodes::MoveNodes( const SwNodeRange& aRange, SwNodes & rNodes,
                    SwNode& rPos, bool bNewFrames )
{
    SwNode * pCurrentNode;
    if( rPos.GetIndex() == SwNodeOffset(0) ||
        ( (pCurrentNode = &rPos)->GetStartNode() &&
          !pCurrentNode->StartOfSectionIndex() ))
        return false;

    SwNodeRange aRg( aRange );

    // skip "simple" start or end nodes
    while( SwNodeType::Start == (pCurrentNode = &aRg.aStart.GetNode())->GetNodeType()
            || ( pCurrentNode->IsEndNode() &&
                !pCurrentNode->m_pStartOfSection->IsSectionNode() ) )
        ++aRg.aStart;
    --aRg.aStart;

    // if aEnd-1 points to no ContentNode, search previous one
    --aRg.aEnd;
    while( ( (( pCurrentNode = &aRg.aEnd.GetNode())->GetStartNode() &&
            !pCurrentNode->IsSectionNode() ) ||
            ( pCurrentNode->IsEndNode() &&
            SwNodeType::Start == pCurrentNode->m_pStartOfSection->GetNodeType()) ) &&
            aRg.aEnd > aRg.aStart )
        --aRg.aEnd;

    // if in same array, check insertion position
    if( aRg.aStart >= aRg.aEnd )
        return false;

    if( this == &rNodes )
    {
        if( ( rPos.GetIndex()-SwNodeOffset(1) >= aRg.aStart.GetIndex() &&
              rPos.GetIndex()-SwNodeOffset(1) < aRg.aEnd.GetIndex()) ||
            ( rPos.GetIndex()-SwNodeOffset(1) == aRg.aEnd.GetIndex() ) )
            return false;
    }

    SwNodeOffset nInsPos(0); // counter for tmp array

    // array as a stack, storing all StartOfSelections
    SwStartNodePointers aSttNdStack;
    SwStartNodePointers::size_type nLevel = 0; // level counter

    // set start index
    SwNodeIndex  aIdx( rPos );

    SwStartNode* pStartNode = aIdx.GetNode().m_pStartOfSection;
    aSttNdStack.insert( aSttNdStack.begin(), pStartNode );

    SwNodeRange aOrigInsPos( aIdx, SwNodeOffset(-1), aIdx ); // original insertion position

    // call DelFrames/MakeFrames for the upmost SectionNode
    int nSectNdCnt = 0;
    bool bSaveNewFrames = bNewFrames;

    // continue until everything has been moved
    while( aRg.aStart < aRg.aEnd )
    {
        pCurrentNode = &aRg.aEnd.GetNode();
        switch( pCurrentNode->GetNodeType() )
        {
        case SwNodeType::End:
            {
                if( nInsPos ) // move everything until here
                {
                    // delete and copy. CAUTION: all indices after
                    // "aRg.aEnd+1" will be moved as well!
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrames );
                    aIdx -= nInsPos;
                    nInsPos = SwNodeOffset(0);
                }

                SwStartNode* pSttNd = pCurrentNode->m_pStartOfSection;
                if( pSttNd->IsTableNode() )
                {
                    SwTableNode* pTableNd = static_cast<SwTableNode*>(pSttNd);

                    // move the whole table/range
                    nInsPos = (aRg.aEnd.GetIndex() -
                                    pSttNd->GetIndex() )+1;
                    aRg.aEnd -= nInsPos;

                    // NEVER include nodes from the RedLineArea
                    SwNodeOffset nNd = aIdx.GetIndex();
                    bool const bInsOutlineIdx = IsInsertOutline(rNodes, nNd);

                    if( bNewFrames )
                        // delete all frames
                        pTableNd->DelFrames(nullptr);
                    if( &rNodes == this ) // move into self?
                    {
                        // move all Start/End/ContentNodes
                        // ContentNodes: delete also the frames!
                        pTableNd->m_pStartOfSection = aIdx.GetNode().m_pStartOfSection;
                        for( SwNodeOffset n(0); n < nInsPos; ++n )
                        {
                            SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                            SwContentNode* pCNd = nullptr;
                            SwNode* pTmpNd = &aMvIdx.GetNode();
                            if( pTmpNd->IsContentNode() )
                            {
                                pCNd = static_cast<SwContentNode*>(pTmpNd);
                                if( pTmpNd->IsTextNode() )
                                    static_cast<SwTextNode*>(pTmpNd)->RemoveFromList();

                                // remove outline index from old nodes array
                                if (pCNd->IsTextNode() && pCNd->GetTextNode()->IsOutline())
                                {
                                    m_aOutlineNodes.erase( pCNd );
                                }
                                else
                                    pCNd = nullptr;
                            }

                            BigPtrArray::Move( sal_Int32(aMvIdx.GetIndex()), sal_Int32(aIdx.GetIndex()) );

                            if( bInsOutlineIdx && pCNd )
                                m_aOutlineNodes.insert( pCNd );
                            if( pTmpNd->IsTextNode() )
                                static_cast<SwTextNode*>(pTmpNd)->AddToList();
                        }
                    }
                    else
                    {
                        // get StartNode
                        // Even aIdx points to a startnode, we need the startnode
                        // of the environment of aIdx (#i80941)
                        SwStartNode* pSttNode = aIdx.GetNode().m_pStartOfSection;

                        // get all boxes with content because their indices
                        // pointing to the StartNodes need to be reset
                        // (copying the array and deleting all found ones eases
                        // searching)
                        SwNodeIndex aMvIdx( aRg.aEnd, 1 );
                        for( SwNodeOffset n(0); n < nInsPos; ++n )
                        {
                            SwNode* pNd = &aMvIdx.GetNode();

                            const bool bOutlNd = pNd->IsTextNode() && pNd->GetTextNode()->IsOutline();
                            // delete outline indices from old node array
                            if( bOutlNd )
                                m_aOutlineNodes.erase( pNd );

                            RemoveNode( aMvIdx.GetIndex(), SwNodeOffset(1), false );
                            pNd->m_pStartOfSection = pSttNode;
                            rNodes.InsertNode( pNd, aIdx );

                            // set correct indices in Start/EndNodes
                            if( bInsOutlineIdx && bOutlNd )
                                // and put them into the new node array
                                rNodes.m_aOutlineNodes.insert( pNd );
                            else if( pNd->IsStartNode() )
                                pSttNode = static_cast<SwStartNode*>(pNd);
                            else if( pNd->IsEndNode() )
                            {
                                pSttNode->m_pEndOfSection = static_cast<SwEndNode*>(pNd);
                                if( pSttNode->IsSectionNode() )
                                    static_cast<SwSectionNode*>(pSttNode)->NodesArrChgd();
                                pSttNode = pSttNode->m_pStartOfSection;
                            }
                        }

                        if( auto pDDETable = dynamic_cast<SwDDETable*>(&pTableNd->GetTable()) )
                        {
                            SwDDEFieldType* pTyp = pDDETable->GetDDEFieldType();
                            if( pTyp )
                            {
                                if( rNodes.IsDocNodes() )
                                    pTyp->IncRefCnt();
                                else
                                    pTyp->DecRefCnt();
                            }
                        }

                        if (GetDoc().GetIDocumentUndoRedo().IsUndoNodes(rNodes))
                        {
                            SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();
                            pTableFormat->GetNotifier().Broadcast(SfxHint(SfxHintId::Dying));
                        }
                    }
                    if( bNewFrames )
                    {
                        pTableNd->MakeOwnFrames();
                    }
                    aIdx -= nInsPos;
                    nInsPos = SwNodeOffset(0);
                }
                else if( pSttNd->GetIndex() < aRg.aStart.GetIndex() )
                {
                    // SectionNode: not the whole section will be moved, thus,
                    //              move only the ContentNodes
                    // StartNode:   create a new section at the given position
                    do {        // middle check loop
                        if( !pSttNd->IsSectionNode() )
                        {
                            // create StartNode and EndNode at InsertPos
                            SwStartNode* pTmp = new SwStartNode( aIdx.GetNode(),
                                                    SwNodeType::Start,
/*?? NodeType ??*/                                  SwNormalStartNode );

                            nLevel++; // put the index to StartNode on the stack
                            aSttNdStack.insert( aSttNdStack.begin() + nLevel, pTmp );

                            // create EndNode
                            new SwEndNode( aIdx.GetNode(), *pTmp );
                        }
                        else if (GetDoc().GetIDocumentUndoRedo().IsUndoNodes(
                                    rNodes))
                        {
                            // use placeholder in UndoNodes array
                            new SwPlaceholderNode(aIdx.GetNode());
                        }
                        else
                        {
                            // JP 18.5.2001 (Bug 70454) creating new section?
                            --aRg.aEnd;
                            break;

                        }

                        --aRg.aEnd;
                        --aIdx;
                    } while( false );
                }
                else
                {
                    // move StartNode and EndNode in total

                    // if Start is exactly the Start of the area,
                    // then the Node needs to be re-visited
                    if( &aRg.aStart.GetNode() == pSttNd )
                        --aRg.aStart;

                    SwSectionNode* pSctNd = pSttNd->GetSectionNode();
                    if( bNewFrames && pSctNd )
                    {   // tdf#135056 skip over code in DelFrames() that moves
                        // SwNodeIndex around because in case of nested
                        // sections, m_pStartOfSection will point between
                        // undo nodes-array and doc nodes-array
                        pSctNd->DelFrames(nullptr, true);
                    }

                    RemoveNode( aRg.aEnd.GetIndex(), SwNodeOffset(1), false ); // delete EndNode
                    SwNodeOffset nSttPos = pSttNd->GetIndex();

                    // this StartNode will be removed later
                    SwStartNode* pTmpSttNd = new SwStartNode( *this, nSttPos+1 );
                    pTmpSttNd->m_pStartOfSection = pSttNd->m_pStartOfSection;

                    RemoveNode( nSttPos, SwNodeOffset(1), false ); // delete SttNode

                    pSttNd->m_pStartOfSection = aIdx.GetNode().m_pStartOfSection;
                    rNodes.InsertNode( pSttNd, aIdx  );
                    rNodes.InsertNode( pCurrentNode, aIdx );
                    --aIdx;
                    pSttNd->m_pEndOfSection = static_cast<SwEndNode*>(pCurrentNode);

                    --aRg.aEnd;

                    nLevel++; // put the index pointing to the StartNode onto the stack
                    aSttNdStack.insert( aSttNdStack.begin() + nLevel, pSttNd );

                    // reset remaining indices if SectionNode
                    if( pSctNd )
                    {
                        pSctNd->NodesArrChgd();
                        ++nSectNdCnt;
                        // tdf#132326 do not let frames survive in undo nodes
                        if (!GetDoc().GetIDocumentUndoRedo().IsUndoNodes(rNodes))
                        {
                            bNewFrames = false;
                        }
                    }
                }
            }
            break;

        case SwNodeType::Section:
            if( !nLevel &&
                GetDoc().GetIDocumentUndoRedo().IsUndoNodes(rNodes))
            {
                // here, a SectionDummyNode needs to be inserted at the current position
                if( nInsPos ) // move everything until here
                {
                    // delete and copy. CAUTION: all indices after
                    // "aRg.aEnd+1" will be moved as well!
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrames );
                    aIdx -= nInsPos;
                    nInsPos = SwNodeOffset(0);
                }
                new SwPlaceholderNode(aIdx.GetNode());
                --aRg.aEnd;
                --aIdx;
                break;
            }
            [[fallthrough]];
        case SwNodeType::Table:
        case SwNodeType::Start:
            {
                // empty section -> nothing to do
                //  and only if it's a top level section
                if( !nInsPos && !nLevel )
                {
                    --aRg.aEnd;
                    break;
                }

                if( !nLevel ) // level is decreasing
                {
                    // create decrease
                    SwNodeIndex aTmpSIdx( aOrigInsPos.aStart, 1 );
                    SwStartNode* pTmpStt = new SwStartNode( aTmpSIdx.GetNode(),
                                SwNodeType::Start,
                                static_cast<SwStartNode*>(pCurrentNode)->GetStartNodeType() );

                    --aTmpSIdx;

                    SwNodeIndex aTmpEIdx( aOrigInsPos.aEnd );
                    new SwEndNode( aTmpEIdx.GetNode(), *pTmpStt );
                    --aTmpEIdx;
                    ++aTmpSIdx;

                    // set correct StartOfSection
                    ++aRg.aEnd;
                    {
                        SwNodeIndex aCntIdx( aRg.aEnd );
                        for( SwNodeOffset n(0); n < nInsPos; n++, ++aCntIdx)
                            aCntIdx.GetNode().m_pStartOfSection = pTmpStt;
                    }

                    // also set correct StartNode for all decreased nodes
                    while( aTmpSIdx < aTmpEIdx )
                        if( nullptr != (( pCurrentNode = &aTmpEIdx.GetNode())->GetEndNode()) )
                            aTmpEIdx = pCurrentNode->StartOfSectionIndex();
                        else
                        {
                            pCurrentNode->m_pStartOfSection = pTmpStt;
                            --aTmpEIdx;
                        }

                    --aIdx;     // after the inserted StartNode
                    --aRg.aEnd; // before StartNode
                    // copy array. CAUTION: all indices after
                    // "aRg.aEnd+1" will be moved as well!
                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrames );
                    aIdx -= nInsPos+1;
                    nInsPos = SwNodeOffset(0);
                }
                else // all nodes between StartNode and EndNode were moved
                {
                    OSL_ENSURE( pCurrentNode == aSttNdStack[nLevel] ||
                            ( pCurrentNode->IsStartNode() &&
                                aSttNdStack[nLevel]->IsSectionNode()),
                             "wrong StartNode" );

                    SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                    ChgNode( aSwIndex, nInsPos, aIdx, bNewFrames );
                    aIdx -= nInsPos+1; // before inserted StartNode
                    nInsPos = SwNodeOffset(0);

                    // remove pointer from node array
                    RemoveNode( aRg.aEnd.GetIndex(), SwNodeOffset(1), true );
                    --aRg.aEnd;

                    SwSectionNode* pSectNd = aSttNdStack[ nLevel ]->GetSectionNode();
                    if( pSectNd && !--nSectNdCnt )
                    {
                        SwNodeIndex aTmp( *pSectNd );
                        pSectNd->MakeOwnFrames(&aTmp);
                        bNewFrames = bSaveNewFrames;
                    }
                    aSttNdStack.erase( aSttNdStack.begin() + nLevel ); // remove from stack
                    nLevel--;
                }

                // delete all resulting empty start/end node pairs
                SwNode* pTmpNode = (*this)[ aRg.aEnd.GetIndex()+1 ]->GetEndNode();
                if( pTmpNode && SwNodeType::Start == (pCurrentNode = &aRg.aEnd.GetNode())
                    ->GetNodeType() && pCurrentNode->StartOfSectionIndex() &&
                    pTmpNode->StartOfSectionNode() == pCurrentNode )
                {
                    DelNodes( aRg.aEnd, SwNodeOffset(2) );
                    --aRg.aEnd;
                }
            }
            break;

        case SwNodeType::Text:
            //Add special function to text node.
            {
                if( bNewFrames && pCurrentNode->GetContentNode() )
                    static_cast<SwContentNode*>(pCurrentNode)->DelFrames(nullptr);
                pCurrentNode->m_pStartOfSection = aSttNdStack[ nLevel ];
                nInsPos++;
                --aRg.aEnd;
            }
            break;
        case SwNodeType::Grf:
        case SwNodeType::Ole:
            {
                if( bNewFrames && pCurrentNode->GetContentNode() )
                    static_cast<SwContentNode*>(pCurrentNode)->DelFrames(nullptr);

                pCurrentNode->m_pStartOfSection = aSttNdStack[ nLevel ];
                nInsPos++;
                --aRg.aEnd;

                // reset Accessibility issue state
                pCurrentNode->resetAndQueueAccessibilityCheck();
            }
            break;

        case SwNodeType::PlaceHolder:
            if (GetDoc().GetIDocumentUndoRedo().IsUndoNodes(*this))
            {
                if( &rNodes == this ) // inside UndoNodesArray
                {
                    // move everything
                    pCurrentNode->m_pStartOfSection = aSttNdStack[ nLevel ];
                    nInsPos++;
                }
                else // move into "normal" node array
                {
                    // than a SectionNode (start/end) is needed at the current
                    // InsPos; if so skip it, otherwise ignore current node
                    if( nInsPos ) // move everything until here
                    {
                        // delete and copy. CAUTION: all indices after
                        // "aRg.aEnd+1" will be moved as well!
                        SwNodeIndex aSwIndex( aRg.aEnd, 1 );
                        ChgNode( aSwIndex, nInsPos, aIdx, bNewFrames );
                        aIdx -= nInsPos;
                        nInsPos = SwNodeOffset(0);
                    }
                    SwNode* pTmpNd = &aIdx.GetNode();
                    if( pTmpNd->IsSectionNode() ||
                        pTmpNd->StartOfSectionNode()->IsSectionNode() )
                        --aIdx; // skip
                }
            }
            else {
                assert(!"How can this node be in the node array?");
            }
            --aRg.aEnd;
            break;

        default:
            assert(!"Unknown node type");
            break;
        }
    }

    if( nInsPos ) // copy remaining rest
    {
        // rest should be ok
        SwNodeIndex aSwIndex( aRg.aEnd, 1 );
        ChgNode( aSwIndex, nInsPos, aIdx, bNewFrames );
    }
    ++aRg.aEnd; // again, exclusive end

    // delete all resulting empty start/end node pairs
    if( ( pCurrentNode = &aRg.aStart.GetNode())->GetStartNode() &&
        pCurrentNode->StartOfSectionIndex() &&
        aRg.aEnd.GetNode().GetEndNode() )
            DelNodes( aRg.aStart, SwNodeOffset(2) );

    // initialize numbering update
    ++aOrigInsPos.aStart;
    // Moved in same node array? Then call update top down!
    if( this == &rNodes &&
        aRg.aEnd.GetIndex() >= aOrigInsPos.aStart.GetIndex() )
    {
        UpdateOutlineIdx( aOrigInsPos.aStart.GetNode() );
        UpdateOutlineIdx( aRg.aEnd.GetNode() );
    }
    else
    {
        UpdateOutlineIdx( aRg.aEnd.GetNode() );
        rNodes.UpdateOutlineIdx( aOrigInsPos.aStart.GetNode() );
    }

    return true;
}

/** create a start/end section pair
 *
 * Other nodes might be in between.
 *
 * After this method call, the start node of pRange will be pointing to the
 * first node after the start section node and the end node will be the index
 * of the end section node. If this method is called multiple times with the
 * same input, multiple sections containing the previous ones will be created
 * (no content nodes between start or end node).
 *
 * @note Start and end node of the range must be on the same level but MUST
 *       NOT be on the top level.
 *
 * @param [IN,OUT] pRange the range (excl. end)
 * @param eSttNdTyp type of the start node
 */
void SwNodes::SectionDown(SwNodeRange *pRange, SwStartNodeType eSttNdTyp )
{
    if( pRange->aStart >= pRange->aEnd ||
        pRange->aEnd >= Count() ||
        !::CheckNodesRange(pRange->aStart.GetNode(), pRange->aEnd.GetNode(), false))
    {
        return;
    }

    // If the beginning of a range is before or at a start node position, so
    // delete it, otherwise empty S/E or E/S nodes would be created.
    // For other nodes, create a new start node.
    SwNode * pCurrentNode = &pRange->aStart.GetNode();
    SwNodeIndex aTmpIdx( *pCurrentNode->StartOfSectionNode() );

    if( pCurrentNode->GetEndNode() )
        DelNodes( pRange->aStart ); // prevent empty section
    else
    {
        // insert a new StartNode
        SwNode* pSttNd = new SwStartNode( pRange->aStart.GetNode(), SwNodeType::Start, eSttNdTyp );
        pRange->aStart = *pSttNd;
        aTmpIdx = pRange->aStart;
    }

    // If the end of a range is before or at a StartNode, so delete it,
    // otherwise empty S/E or E/S nodes would be created.
    // For other nodes, insert a new end node.
    --pRange->aEnd;
    if( pRange->aEnd.GetNode().GetStartNode() )
        DelNodes( pRange->aEnd );
    else
    {
        ++pRange->aEnd;
        // insert a new EndNode
        new SwEndNode( pRange->aEnd.GetNode(), *pRange->aStart.GetNode().GetStartNode() );
    }
    --pRange->aEnd;

    SectionUpDown( aTmpIdx, pRange->aEnd );
}

/** increase level of the given range
 *
 * The range contained in pRange will be lifted to the next higher level.
 * This is done by adding an end node at pRange.start and a start node at
 * pRange.end. Furthermore all indices for this range will be updated.
 *
 * After this method call, the start node of pRange will be pointing to the
 * first node inside the lifted range and the end node will be pointing to the
 * last position inside the lifted range.
 *
 * @param [IN,OUT] pRange the range of nodes where the level should be increased
 */
void SwNodes::SectionUp(SwNodeRange *pRange)
{
    if( pRange->aStart >= pRange->aEnd ||
        pRange->aEnd >= Count() ||
        !::CheckNodesRange(pRange->aStart.GetNode(), pRange->aEnd.GetNode(), false) ||
        ( HighestLevel( *this, *pRange ) <= 1 ))
    {
        return;
    }

    // If the beginning of a range is before or at a start node position, so
    // delete it, otherwise empty S/E or E/S nodes would be created.
    // For other nodes, create a new start node.
    SwNode * pCurrentNode = &pRange->aStart.GetNode();
    SwNodeIndex aIdx( *pCurrentNode->StartOfSectionNode() );
    if( pCurrentNode->IsStartNode() )       // is StartNode itself
    {
        SwEndNode* pEndNd = pRange->aEnd.GetNode().GetEndNode();
        if (pEndNd && pCurrentNode == pEndNd->m_pStartOfSection)
        {
            // there was a pairwise reset, adjust only those in the range
            SwStartNode* pTmpSttNd = pCurrentNode->m_pStartOfSection;
            RemoveNode( pRange->aStart.GetIndex(), SwNodeOffset(1), true );
            RemoveNode( pRange->aEnd.GetIndex(), SwNodeOffset(1), true );

            SwNodeIndex aTmpIdx( pRange->aStart );
            while( aTmpIdx < pRange->aEnd )
            {
                pCurrentNode = &aTmpIdx.GetNode();
                pCurrentNode->m_pStartOfSection = pTmpSttNd;
                if( pCurrentNode->IsStartNode() )
                    aTmpIdx = pCurrentNode->EndOfSectionIndex() + 1;
                else
                    ++aTmpIdx;
            }
            return ;
        }
        DelNodes( pRange->aStart );
    }
    else if( aIdx == pRange->aStart.GetIndex()-1 ) // before StartNode
        DelNodes( aIdx );
    else
        new SwEndNode( pRange->aStart.GetNode(), *aIdx.GetNode().GetStartNode() );

    // If the end of a range is before or at a StartNode, so delete it,
    // otherwise empty S/E or E/S nodes would be created.
    // For other nodes, insert a new end node.
    SwNodeIndex aTmpIdx( pRange->aEnd );
    if( pRange->aEnd.GetNode().IsEndNode() )
        DelNodes( pRange->aEnd );
    else
    {
        new SwStartNode( pRange->aEnd.GetNode() );
/*?? which NodeType ??*/
        aTmpIdx = *pRange->aEnd.GetNode().EndOfSectionNode();
        --pRange->aEnd;
    }

    SectionUpDown( aIdx, aTmpIdx );
}

/** correct indices after movement
 *
 * Update all indices after movement so that the levels are consistent again.
 *
 * @param aStart index of the start node
 * @param aEnd index of the end point
 *
 * @see SwNodes::SectionUp
 * @see SwNodes::SectionDown
 */
void SwNodes::SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd )
{
    SwNodeIndex aTmpIdx( aStart, +1 );
    // array forms a stack, holding all StartOfSelections
    SwStartNodePointers aSttNdStack;
    SwStartNode* pTmp = aStart.GetNode().GetStartNode();
    aSttNdStack.push_back( pTmp );

    // loop until the first start node that needs to be change was found
    // (the indices are updated from the end node backwards to the start)
    for( ;; ++aTmpIdx )
    {
        SwNode * pCurrentNode = &aTmpIdx.GetNode();
        pCurrentNode->m_pStartOfSection = aSttNdStack[ aSttNdStack.size()-1 ];

        if( pCurrentNode->GetStartNode() )
        {
            pTmp = static_cast<SwStartNode*>(pCurrentNode);
            aSttNdStack.push_back( pTmp );
        }
        else if( pCurrentNode->GetEndNode() )
        {
            SwStartNode* pSttNd = aSttNdStack[ aSttNdStack.size() - 1 ];
            pSttNd->m_pEndOfSection = static_cast<SwEndNode*>(pCurrentNode);
            aSttNdStack.pop_back();
            if( !aSttNdStack.empty() )
                continue; // still enough EndNodes on the stack

            else if( aTmpIdx < aEnd ) // too many StartNodes
                // if the end is not reached, yet, get the start of the section above
            {
                aSttNdStack.insert( aSttNdStack.begin(), pSttNd->m_pStartOfSection );
            }
            else // finished, as soon as out of the range
                break;
        }
    }
}

void SwNodes::Delete(const SwNodeIndex &rIndex, SwNodeOffset nNodes)
{
    Delete(rIndex.GetNode(), nNodes);
}

/** delete nodes
 *
 * This is a specific implementation of a delete function for a variable array.
 * It is necessary as there might be inconsistencies after deleting start or
 * end nodes. This method can clean those up.
 *
 * @param rIndex position to delete at (unchanged afterwards)
 * @param nNodes number of nodes to delete (default: 1)
 */
void SwNodes::Delete(const SwNode &rIndex, SwNodeOffset nNodes)
{
    int nLevel = 0; // level counter
    SwNode * pCurrentNode;

    SwNodeOffset nCnt = Count() - rIndex.GetIndex() - 1;
    if( nCnt > nNodes ) nCnt = nNodes;

    if( nCnt == SwNodeOffset(0) ) // no count -> return
        return;

    SwNodeRange aRg( rIndex, SwNodeOffset(0), rIndex, nCnt-1 );
    // check if [rIndex..rIndex + nCnt] is larger than the range
    if( ( !aRg.aStart.GetNode().StartOfSectionIndex() &&
            !aRg.aStart.GetIndex() ) ||
            !::CheckNodesRange(aRg.aStart.GetNode(), aRg.aEnd.GetNode(), false))
    {
        return;
    }

    // if aEnd is not on a ContentNode, search the previous one
    while( ( pCurrentNode = &aRg.aEnd.GetNode())->GetStartNode() ||
             ( pCurrentNode->GetEndNode() &&
                !pCurrentNode->m_pStartOfSection->IsTableNode() ))
        --aRg.aEnd;

    nCnt = SwNodeOffset(0);
//TODO: check/improve comment
    // increase start so that we are able to use "<" (using "<=" might cause
    // problems if aEnd == aStart and aEnd is deleted, so aEnd <= aStart)
    --aRg.aStart;

    bool bSaveInNodesDel = m_bInNodesDel;
    m_bInNodesDel = true;
    bool bUpdateOutline = false;

    // loop until everything is deleted
    while( aRg.aStart < aRg.aEnd )
    {
        pCurrentNode = &aRg.aEnd.GetNode();

        if( pCurrentNode->GetEndNode() )
        {
            // delete the whole section?
            if( pCurrentNode->StartOfSectionIndex() > aRg.aStart.GetIndex() )
            {
                SwTableNode* pTableNd = pCurrentNode->m_pStartOfSection->GetTableNode();
                if( pTableNd )
                    pTableNd->DelFrames();

                SwNode *pNd, *pChkNd = pCurrentNode->m_pStartOfSection;
                SwOutlineNodes::size_type nIdxPos;
                do {
                    pNd = &aRg.aEnd.GetNode();

                    if( pNd->IsTextNode() )
                    {
                        SwTextNode *const pTextNode(pNd->GetTextNode());
                        if (pTextNode->IsOutline() &&
                                m_aOutlineNodes.Seek_Entry( pNd, &nIdxPos ))
                        {
                            // remove outline indices
                            m_aOutlineNodes.erase_at(nIdxPos);
                            bUpdateOutline = true;
                        }
                        pTextNode->InvalidateNumRule();
                    }
                    else if( pNd->IsEndNode() &&
                            pNd->m_pStartOfSection->IsTableNode() )
                        static_cast<SwTableNode*>(pNd->m_pStartOfSection)->DelFrames();

                    --aRg.aEnd;
                    nCnt++;

                } while( pNd != pChkNd );
            }
            else
            {
                RemoveNode( aRg.aEnd.GetIndex()+1, nCnt, true ); // delete
                nCnt = SwNodeOffset(0);
                --aRg.aEnd; // before the EndNode
                nLevel++;
            }
        }
        else if( pCurrentNode->GetStartNode() ) // found StartNode
        {
            if( nLevel == 0 ) // decrease one level
            {
                if( nCnt )
                {
                    // now delete array
                    ++aRg.aEnd;
                    RemoveNode( aRg.aEnd.GetIndex(), nCnt, true );
                    nCnt = SwNodeOffset(0);
                }
            }
            else // remove all nodes between start and end node (incl. both)
            {
                RemoveNode( aRg.aEnd.GetIndex(), nCnt + 2, true ); // delete array
                nCnt = SwNodeOffset(0);
                nLevel--;
            }

            // after deletion, aEnd might point to an EndNode...
            // delete all empty start/end node pairs
            SwNode* pTmpNode = aRg.aEnd.GetNode().GetEndNode();
            --aRg.aEnd;
            while(  pTmpNode &&
                    ( pCurrentNode = &aRg.aEnd.GetNode())->GetStartNode() &&
                    pCurrentNode->StartOfSectionIndex() )
            {
                // remove end and start node
                DelNodes( aRg.aEnd, SwNodeOffset(2) );
                pTmpNode = aRg.aEnd.GetNode().GetEndNode();
                --aRg.aEnd;
            }
        }
        else // "normal" node, so insert into TmpArray
        {
            SwTextNode* pTextNd = pCurrentNode->GetTextNode();
            if( pTextNd )
            {
                if( pTextNd->IsOutline())
                {
                    // delete outline indices
                    m_aOutlineNodes.erase( pTextNd );
                    bUpdateOutline = true;
                }
                if (sw::HasNumberingWhichNeedsLayoutUpdate(*pTextNd))
                {
                    pTextNd->InvalidateNumRule();
                }
            }
            else if( pCurrentNode->IsContentNode() )
                static_cast<SwContentNode*>(pCurrentNode)->InvalidateNumRule();

            --aRg.aEnd;
            nCnt++;
        }
    }

    ++aRg.aEnd;
    if( nCnt != SwNodeOffset(0) )
        RemoveNode( aRg.aEnd.GetIndex(), nCnt, true ); // delete the rest

    // delete all empty start/end node pairs
    while( aRg.aEnd.GetNode().GetEndNode() &&
            ( pCurrentNode = &aRg.aStart.GetNode())->GetStartNode() &&
            pCurrentNode->StartOfSectionIndex() )
    // but none of the holy 5. (???)
    {
        DelNodes( aRg.aStart, SwNodeOffset(2) );  // delete start and end node
        --aRg.aStart;
    }

    m_bInNodesDel = bSaveInNodesDel;

    if( !m_bInNodesDel )
    {
        // update numbering
        if( bUpdateOutline || m_bInDelUpdOutline )
        {
            UpdateOutlineIdx( aRg.aEnd.GetNode() );
            m_bInDelUpdOutline = false;
        }

    }
    else
    {
        if( bUpdateOutline )
            m_bInDelUpdOutline = true;
    }
}

/** get section level at the given position
 *
 * @note The first node in an array should always be a start node.
 *       Because of this, there is a special treatment here based on the
 *       assumption that this is true in this context as well.
 *
 * @param rIdx position of the node
 * @return section level at the given position
 */
sal_uInt16 SwNodes::GetSectionLevel(const SwNode &rIdx)
{
    // special treatment for 1st Node
    if(rIdx.GetIndex() == SwNodeOffset(0)) return 1;
    // no recursion! This calls a SwNode::GetSectionLevel (missing "s")
    return rIdx.GetSectionLevel();
}

void SwNodes::GoStartOfSection(SwNodeIndex *pIdx)
{
    // after the next start node
    SwNodeIndex aTmp( *pIdx->GetNode().StartOfSectionNode(), +1 );

    // If index points to no ContentNode, then go to one.
    // If there is no further available, do not change the index' position!
    while( !aTmp.GetNode().IsContentNode() )
    {   // go from this StartNode (can only be one) to its end
        if( *pIdx <= aTmp )
            return;     // ERROR: already after the section
        aTmp = aTmp.GetNode().EndOfSectionIndex()+1;
        if( *pIdx <= aTmp )
            return;     // ERROR: already after the section
    }
    (*pIdx) = aTmp;     // is on a ContentNode
}

void SwNodes::GoEndOfSection(SwNodeIndex *pIdx)
{
    if( !pIdx->GetNode().IsEndNode() )
        (*pIdx) = *pIdx->GetNode().EndOfSectionNode();
}

SwContentNode* SwNodes::GoNext(SwNodeIndex *pIdx) const
{
    if( pIdx->GetIndex() >= Count() - 1 )
        return nullptr;

    SwNodeIndex aTmp(*pIdx, +1);
    SwNode* pNd = nullptr;
    while( aTmp < Count()-1 && !( pNd = &aTmp.GetNode())->IsContentNode() )
        ++aTmp;

    if( aTmp == Count()-1 )
        pNd = nullptr;
    else
        (*pIdx) = aTmp;
    return static_cast<SwContentNode*>(pNd);
}

SwContentNode* SwNodes::GoNext(SwPosition *pIdx) const
{
    if( pIdx->GetNodeIndex() >= Count() - 1 )
        return nullptr;

    SwNodeIndex aTmp(pIdx->GetNode(), +1);
    SwNode* pNd = nullptr;
    while( aTmp < Count()-1 && !( pNd = &aTmp.GetNode())->IsContentNode() )
        ++aTmp;

    if( aTmp == Count()-1 )
        pNd = nullptr;
    else
        pIdx->Assign(aTmp);
    return static_cast<SwContentNode*>(pNd);
}

SwNodeOffset SwNodes::StartOfGlobalSection(const SwNode& node) const
{
    const SwNodeOffset pos = node.GetIndex();
    if (GetEndOfExtras().GetIndex() < pos)
        // Regular ContentSection
        return GetEndOfExtras().GetIndex() + SwNodeOffset(1);
    if (GetEndOfAutotext().GetIndex() < pos)
        // Redlines
        return GetEndOfAutotext().GetIndex() + SwNodeOffset(1);
    if (GetEndOfInserts().GetIndex() < pos)
    {
        // Flys/Headers/Footers
        if (auto* p = node.FindFlyStartNode())
            return p->GetIndex();
        if (auto* p = node.FindHeaderStartNode())
            return p->GetIndex();
        if (auto* p = node.FindFooterStartNode())
            return p->GetIndex();
        return GetEndOfInserts().GetIndex() + SwNodeOffset(1);
    }
    if (GetEndOfPostIts().GetIndex() < pos)
    {
        // Footnotes
        if (auto* p = node.FindFootnoteStartNode())
            return p->GetIndex();
        return GetEndOfPostIts().GetIndex() + SwNodeOffset(1);
    }
    return SwNodeOffset(0);
}

SwContentNode* SwNodes::GoPrevious(SwNodeIndex* pIdx, bool canCrossBoundary)
{
    if( !pIdx->GetIndex() )
        return nullptr;

    SwNodeIndex aTmp( *pIdx, -1 );
    SwNodeOffset aGlobalStart(
        canCrossBoundary ? SwNodeOffset(0) : aTmp.GetNodes().StartOfGlobalSection(pIdx->GetNode()));
    SwNode* pNd = nullptr;
    while (aTmp > aGlobalStart && !(pNd = &aTmp.GetNode())->IsContentNode())
        --aTmp;

    if (aTmp <= aGlobalStart)
        pNd = nullptr;
    else
        (*pIdx) = aTmp;
    return static_cast<SwContentNode*>(pNd);
}

SwContentNode* SwNodes::GoPrevious(SwPosition* pIdx, bool canCrossBoundary)
{
    if( !pIdx->GetNodeIndex() )
        return nullptr;

    SwNodeIndex aTmp( pIdx->GetNode(), -1 );
    SwNodeOffset aGlobalStart(
        canCrossBoundary ? SwNodeOffset(0) : aTmp.GetNodes().StartOfGlobalSection(pIdx->GetNode()));
    SwNode* pNd = nullptr;
    while( aTmp > aGlobalStart && !( pNd = &aTmp.GetNode())->IsContentNode() )
        --aTmp;

    if (aTmp <= aGlobalStart)
        pNd = nullptr;
    else
        pIdx->Assign(aTmp);
    return static_cast<SwContentNode*>(pNd);
}

/** Delete a number of nodes
 *
 * @param rStart starting position in this nodes array
 * @param nCnt number of nodes to delete
 */
void SwNodes::DelNodes( const SwNodeIndex & rStart, SwNodeOffset nCnt )
{
    SwNodeOffset nSttIdx = rStart.GetIndex();

    if( !nSttIdx && nCnt == GetEndOfContent().GetIndex()+1 )
    {
        // The whole nodes array will be destroyed, you're in the Doc's DTOR!
        // The initial start/end nodes should be only destroyed in the SwNodes' DTOR!
        SwNode* aEndNdArr[] = { m_pEndOfContent.get(),
                                m_pEndOfPostIts, m_pEndOfInserts,
                                m_pEndOfAutotext, m_pEndOfRedlines,
                                nullptr
                              };

        SwNode** ppEndNdArr = aEndNdArr;
        while( *ppEndNdArr )
        {
            nSttIdx = (*ppEndNdArr)->StartOfSectionIndex() + 1;
            SwNodeOffset nEndIdx = (*ppEndNdArr)->GetIndex();

            if( nSttIdx != nEndIdx )
                RemoveNode( nSttIdx, nEndIdx - nSttIdx, true );

            ++ppEndNdArr;
        }
    }
    else
    {
        int bUpdateNum = 0;
        for( SwNodeOffset n = nSttIdx, nEnd = nSttIdx + nCnt; n < nEnd; ++n )
        {
            SwNode* pNd = (*this)[ n ];

            if (pNd->IsTextNode() && pNd->GetTextNode()->IsOutline())
            {
                // remove the outline indices
                if (m_aOutlineNodes.erase(pNd))
                    bUpdateNum = 1;
            }
            if( pNd->IsContentNode() )
            {
                static_cast<SwContentNode*>(pNd)->InvalidateNumRule();
                static_cast<SwContentNode*>(pNd)->DelFrames(nullptr);
            }
        }
        RemoveNode( nSttIdx, nCnt, true );

        // update numbering
        if( bUpdateNum )
            UpdateOutlineIdx( rStart.GetNode() );
    }
}

namespace {

struct HighLevel
{
    sal_uInt16 nLevel, nTop;
    explicit HighLevel( sal_uInt16 nLv ) : nLevel( nLv ), nTop( nLv ) {}
};

}

static bool lcl_HighestLevel( SwNode* pNode, void * pPara )
{
    HighLevel * pHL = static_cast<HighLevel*>(pPara);
    if( pNode->GetStartNode() )
    {
        pHL->nLevel++;
        if( pHL->nTop > pHL->nLevel )
            pHL->nTop = pHL->nLevel;
    }
    else if( pNode->GetEndNode() )
        pHL->nLevel--;
    return true;

}

/** Calculate the highest level in a range
 *
 * @param rNodes the nodes array
 * @param rRange the range to inspect
 * @return the highest level
 */
sal_uInt16 HighestLevel( SwNodes & rNodes, const SwNodeRange & rRange )
{
    HighLevel aPara( SwNodes::GetSectionLevel( rRange.aStart.GetNode() ));
    rNodes.ForEach( rRange.aStart, rRange.aEnd, lcl_HighestLevel, &aPara );
    return aPara.nTop;

}

/** move a range
 *
 * @param rPam the range to move
 * @param rPos to destination position in the given nodes array
 * @param rNodes the node array to move the range into
 */
void SwNodes::MoveRange( SwPaM & rPam, SwPosition & rPos, SwNodes& rNodes )
{
    auto [pStt, pEnd] = rPam.StartEnd(); // SwPosition*

    if( !rPam.HasMark() || *pStt >= *pEnd )
        return;

    if( this == &rNodes && *pStt <= rPos && rPos < *pEnd )
        return;

    SwNodeIndex aEndIdx( pEnd->GetNode() );
    SwNodeIndex aSttIdx( pStt->GetNode() );
    SwTextNode *const pSrcNd = aSttIdx.GetNode().GetTextNode();
    SwTextNode * pDestNd = rPos.GetNode().GetTextNode();
    bool bSplitDestNd = true;
    bool bCopyCollFormat = pDestNd && pDestNd->GetText().isEmpty();

    if( pSrcNd )
    {
        // if the first node is a TextNode, then there must
        // be also a TextNode in the NodesArray to store the content
        if( !pDestNd )
        {
            pDestNd = rNodes.MakeTextNode( rPos.GetNode(), pSrcNd->GetTextColl() );
            --rPos.nNode;
            rPos.nContent.Assign( pDestNd, 0 );
            bCopyCollFormat = true;
        }
        bSplitDestNd = pDestNd->Len() > rPos.GetContentIndex() ||
                        pEnd->GetNode().IsTextNode();

        // move the content into the new node
        bool bOneNd = pStt->GetNode() == pEnd->GetNode();
        const sal_Int32 nLen =
                ( bOneNd ? std::min(pEnd->GetContentIndex(), pSrcNd->Len()) : pSrcNd->Len() )
                - pStt->GetContentIndex();

        if( !pEnd->GetNode().IsContentNode() )
        {
            bOneNd = true;
            SwNodeOffset nSttNdIdx = pStt->GetNodeIndex() + 1;
            const SwNodeOffset nEndNdIdx = pEnd->GetNodeIndex();
            for( ; nSttNdIdx < nEndNdIdx; ++nSttNdIdx )
            {
                if( (*this)[ nSttNdIdx ]->IsContentNode() )
                {
                    bOneNd = false;
                    break;
                }
            }
        }

        // templates must be copied/set after a split
        if( !bOneNd && bSplitDestNd )
        {
            if( !rPos.GetContentIndex() )
            {
                bCopyCollFormat = true;
            }
            if( rNodes.IsDocNodes() )
            {
                SwDoc& rInsDoc = pDestNd->GetDoc();
                ::sw::UndoGuard const ug(rInsDoc.GetIDocumentUndoRedo());
                rInsDoc.getIDocumentContentOperations().SplitNode( rPos, false );
            }
            else
            {
                pDestNd->SplitContentNode(rPos, nullptr);
            }

            if( rPos.GetNode() == aEndIdx.GetNode() )
            {
                --aEndIdx;
            }
            bSplitDestNd = true;

            pDestNd = rNodes[ rPos.GetNodeIndex() - 1 ]->GetTextNode();
            if( nLen )
            {
                pSrcNd->CutText( pDestNd, SwContentIndex( pDestNd, pDestNd->Len()),
                            pStt->nContent, nLen );
            }
        }
        else if ( nLen )
        {
            pSrcNd->CutText( pDestNd, rPos.nContent, pStt->nContent, nLen );
        }

        if( bCopyCollFormat )
        {
            SwDoc& rInsDoc = pDestNd->GetDoc();
            ::sw::UndoGuard const undoGuard(rInsDoc.GetIDocumentUndoRedo());
            pSrcNd->CopyCollFormat( *pDestNd );
        }

        if( bOneNd )
        {
            // Correct the PaM, because it might have happened that the move
            // went over the node borders (so the data might be in different nodes).
            // Also, a selection is invalidated.
            pEnd->nContent = pStt->nContent;
            rPam.DeleteMark();
            GetDoc().GetDocShell()->Broadcast( SwFormatFieldHint( nullptr,
                rNodes.IsDocNodes() ? SwFormatFieldHintWhich::INSERTED : SwFormatFieldHintWhich::REMOVED ) );
            return;
        }

        ++aSttIdx;
    }
    else if( pDestNd )
    {
        if( rPos.GetContentIndex() )
        {
            if( rPos.GetContentIndex() == pDestNd->Len() )
            {
                ++rPos.nNode;
            }
            else if( rPos.GetContentIndex() )
            {
                // if the EndNode is split than correct the EndIdx
                const bool bCorrEnd = aEndIdx == rPos.nNode;

                // if no text is attached to the TextNode, split it
                if( rNodes.IsDocNodes() )
                {
                    SwDoc& rInsDoc = pDestNd->GetDoc();
                    ::sw::UndoGuard const ug(rInsDoc.GetIDocumentUndoRedo());
                    rInsDoc.getIDocumentContentOperations().SplitNode( rPos, false );
                }
                else
                {
                    pDestNd->SplitContentNode(rPos, nullptr);
                }

                if ( bCorrEnd )
                {
                    --aEndIdx;
                }
            }
        }
        // at the end only an empty TextNode is left over
        bSplitDestNd = true;
    }

    SwTextNode* const pEndSrcNd = aEndIdx.GetNode().GetTextNode();
    if ( pEndSrcNd )
    {
        // at the end of this range a new TextNode will be created
        if( !bSplitDestNd )
        {
            if( rPos.GetNode() < rNodes.GetEndOfContent() )
            {
                ++rPos.nNode;
            }

            pDestNd =
                rNodes.MakeTextNode( rPos.GetNode(), pEndSrcNd->GetTextColl() );
            --rPos.nNode;
            rPos.nContent.Assign( pDestNd, 0 );
        }
        else
        {
            pDestNd = rPos.GetNode().GetTextNode();
        }

        if (pDestNd && pEnd->GetContentIndex())
        {
            // move the content into the new node
            SwContentIndex aIdx( pEndSrcNd, 0 );
            pEndSrcNd->CutText( pDestNd, rPos.nContent, aIdx,
                            pEnd->GetContentIndex());
        }

        if (pDestNd && bCopyCollFormat)
        {
            SwDoc& rInsDoc = pDestNd->GetDoc();
            ::sw::UndoGuard const ug(rInsDoc.GetIDocumentUndoRedo());
            pEndSrcNd->CopyCollFormat( *pDestNd );
        }
    }
    else
    {
        if ( pSrcNd && aEndIdx.GetNode().IsContentNode() )
        {
            ++aEndIdx;
        }
        if( !bSplitDestNd )
        {
            rPos.Adjust(SwNodeOffset(1));
        }
    }

    if( aEndIdx != aSttIdx )
    {
        // move the nodes into the NodesArray
        const SwNodeOffset nSttDiff = aSttIdx.GetIndex() - pStt->GetNodeIndex();
        SwNodeRange aRg( aSttIdx, aEndIdx );
        MoveNodes( aRg, rNodes, rPos.GetNode() );

        // if in the same node array, all indices are now at new positions (so correct them)
        if( &rNodes == this )
        {
            pStt->nNode = aRg.aEnd.GetIndex() - nSttDiff;
        }
    }

    // if the StartNode was moved to whom the cursor pointed, so
    // the content must be registered in the current content!
    if ( pStt->GetNode() == GetEndOfContent() )
    {
        const bool bSuccess = GoPrevious( &pStt->nNode );
        OSL_ENSURE( bSuccess, "Move() - no ContentNode here" );
    }
    pStt->nContent.Assign( pStt->GetNode().GetContentNode(),
                            pStt->GetContentIndex() );
    // Correct the PaM, because it might have happened that the move
    // went over the node borders (so the data might be in different nodes).
    // Also, a selection is invalidated.
    *pEnd = *pStt;
    rPam.DeleteMark();
    GetDoc().GetDocShell()->Broadcast( SwFormatFieldHint( nullptr,
                rNodes.IsDocNodes() ? SwFormatFieldHintWhich::INSERTED : SwFormatFieldHintWhich::REMOVED ) );
}

///@see SwNodes::MoveNodes (TODO: seems to be C&P programming here)
void SwNodes::CopyNodes( const SwNodeRange& rRange,
            SwNode& rPos, bool bNewFrames, bool bTableInsDummyNode ) const
{
    SwDoc& rDoc = rPos.GetDoc();

    SwNode * pCurrentNode;
    if( rPos.GetIndex() == SwNodeOffset(0) ||
        ( (pCurrentNode = &rPos)->GetStartNode() &&
          !pCurrentNode->StartOfSectionIndex() ))
        return;

    SwNodeRange aRg( rRange );

    // skip "simple" StartNodes or EndNodes
    while( SwNodeType::Start == (pCurrentNode = & aRg.aStart.GetNode())->GetNodeType()
            || ( pCurrentNode->IsEndNode() &&
                !pCurrentNode->m_pStartOfSection->IsSectionNode() ) )
        ++aRg.aStart;

    const SwNode *aEndNode = &aRg.aEnd.GetNode();
    SwNodeOffset nIsEndOfContent((aEndNode == &aEndNode->GetNodes().GetEndOfContent()) ? 1 : 0);

    if (SwNodeOffset(0) == nIsEndOfContent)
    {
        // if aEnd-1 points to no ContentNode, search previous one
        --aRg.aEnd;
        // #i107142#: if aEnd is start node of a special section, do nothing.
        // Otherwise this could lead to crash: going through all previous
        // special section nodes and then one before the first.
        if (aRg.aEnd.GetNode().StartOfSectionIndex() != SwNodeOffset(0))
        {
            while( ((pCurrentNode = & aRg.aEnd.GetNode())->GetStartNode() &&
                    !pCurrentNode->IsSectionNode() ) ||
                    ( pCurrentNode->IsEndNode() &&
                    SwNodeType::Start == pCurrentNode->m_pStartOfSection->GetNodeType()) )
            {
                --aRg.aEnd;
            }
        }
        ++aRg.aEnd;
    }

    // is there anything left to copy?
    if( aRg.aStart >= aRg.aEnd )
        return;

    // when inserting into the source range, nothing need to be done
    OSL_ENSURE( &aRg.aStart.GetNodes() == this,
                "aRg should use this node array" );
    OSL_ENSURE( &aRg.aStart.GetNodes() == &aRg.aEnd.GetNodes(),
               "Range across different nodes arrays? You deserve punishment!");
    if( &rPos.GetNodes() == &aRg.aStart.GetNodes() &&
        rPos.GetIndex() >= aRg.aStart.GetIndex() &&
        rPos.GetIndex() < aRg.aEnd.GetIndex() )
            return;

    SwNodeIndex aInsPos( rPos );
    SwNodeIndex aOrigInsPos( rPos, -1 ); // original insertion position
    int nLevel = 0;                        // level counter

    for( SwNodeOffset nNodeCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
            nNodeCnt > SwNodeOffset(0); --nNodeCnt )
    {
        pCurrentNode = &aRg.aStart.GetNode();
        switch( pCurrentNode->GetNodeType() )
        {
        case SwNodeType::Table:
            // Does it copy a table in(to) a footnote?
            if( aInsPos < rDoc.GetNodes().GetEndOfInserts().GetIndex() &&
                    rDoc.GetNodes().GetEndOfInserts().StartOfSectionIndex()
                    < aInsPos.GetIndex() )
            {
                const SwNodeOffset nDistance =
                    pCurrentNode->EndOfSectionIndex() -
                        aRg.aStart.GetIndex();
                if (nDistance < nNodeCnt)
                    nNodeCnt -= nDistance;
                else
                    nNodeCnt = SwNodeOffset(1);

                // insert a DummyNode for a TableNode
                if( bTableInsDummyNode )
                    new SwPlaceholderNode(aInsPos.GetNode());

                // copy all of the table's nodes into the current cell
                for( ++aRg.aStart; aRg.aStart.GetIndex() <
                    pCurrentNode->EndOfSectionIndex();
                    ++aRg.aStart )
                {
                    // insert a DummyNode for the box-StartNode?
                    if( bTableInsDummyNode )
                        new SwPlaceholderNode(aInsPos.GetNode());

                    SwStartNode* pSttNd = aRg.aStart.GetNode().GetStartNode();
                    CopyNodes( SwNodeRange( *pSttNd, SwNodeOffset(+ 1),
                                            *pSttNd->EndOfSectionNode() ),
                                aInsPos.GetNode(), bNewFrames );

                    // insert a DummyNode for the box-EndNode?
                    if( bTableInsDummyNode )
                        new SwPlaceholderNode(aInsPos.GetNode());
                    aRg.aStart = *pSttNd->EndOfSectionNode();
                }
                // insert a DummyNode for the table-EndNode
                if( bTableInsDummyNode )
                    new SwPlaceholderNode(aInsPos.GetNode());
                aRg.aStart = *pCurrentNode->EndOfSectionNode();
            }
            else
            {
                SwNodeIndex nStt( aInsPos, -1 );
                SwTableNode* pTableNd = static_cast<SwTableNode*>(pCurrentNode)->
                                        MakeCopy( rDoc, aInsPos );
                const SwNodeOffset nDistance = aInsPos.GetIndex() - nStt.GetIndex() - 2;
                if (nDistance < nNodeCnt)
                    nNodeCnt -= nDistance;
                else
                    nNodeCnt = SwNodeOffset(1) - nIsEndOfContent;

                aRg.aStart = pCurrentNode->EndOfSectionIndex();

                if( bNewFrames && pTableNd )
                    pTableNd->MakeOwnFrames();
            }
            break;

        case SwNodeType::Section:
            // If the end of the section is outside the copy range,
            // the section node will skipped, not copied!
            // If someone want to change this behaviour, he has to adjust the function
            // lcl_NonCopyCount() which relies on it.
            if( pCurrentNode->EndOfSectionIndex() < aRg.aEnd.GetIndex() )
            {
                // copy of the whole section, so create a new SectionNode
                SwNodeIndex nStt( aInsPos, -1 );
                SwSectionNode* pSectNd = static_cast<SwSectionNode*>(pCurrentNode)->
                                    MakeCopy( rDoc, aInsPos );

                const SwNodeOffset nDistance = aInsPos.GetIndex() - nStt.GetIndex() - 2;
                if (nDistance < nNodeCnt)
                    nNodeCnt -= nDistance;
                else
                    nNodeCnt = SwNodeOffset(1) - nIsEndOfContent;
                aRg.aStart = pCurrentNode->EndOfSectionIndex();

                if( bNewFrames && pSectNd &&
                    !pSectNd->GetSection().IsHidden() )
                    pSectNd->MakeOwnFrames(&nStt);
            }
            break;

        case SwNodeType::Start:
            {
                SwStartNode* pTmp = new SwStartNode( aInsPos.GetNode(), SwNodeType::Start,
                            static_cast<SwStartNode*>(pCurrentNode)->GetStartNodeType() );
                new SwEndNode( aInsPos.GetNode(), *pTmp );
                --aInsPos;
                nLevel++;
            }
            break;

        case SwNodeType::End:
            if( nLevel ) // complete section
            {
                --nLevel;
                ++aInsPos; // EndNode already exists
            }
            else if( SwNodeOffset(1) == nNodeCnt && SwNodeOffset(1) == nIsEndOfContent )
                // we have reached the EndOfContent node - nothing to do!
                continue;
            else if( !pCurrentNode->m_pStartOfSection->IsSectionNode() )
            {
                // create a section at the original InsertPosition
                SwNodeRange aTmpRg( aOrigInsPos, SwNodeOffset(1), aInsPos );
                rDoc.GetNodes().SectionDown( &aTmpRg,
                        pCurrentNode->m_pStartOfSection->GetStartNodeType() );
            }
            break;

        case SwNodeType::Text:
        case SwNodeType::Grf:
        case SwNodeType::Ole:
            {
                 static_cast<SwContentNode*>(pCurrentNode)->MakeCopy(
                                            rDoc, aInsPos.GetNode(), bNewFrames);
            }
            break;

        case SwNodeType::PlaceHolder:
            if (GetDoc().GetIDocumentUndoRedo().IsUndoNodes(*this))
            {
                // than a SectionNode (start/end) is needed at the current
                // InsPos; if so skip it, otherwise ignore current node
                SwNode *const pTmpNd = & aInsPos.GetNode();
                if( pTmpNd->IsSectionNode() ||
                    pTmpNd->StartOfSectionNode()->IsSectionNode() )
                    ++aInsPos;  // skip
            }
            else {
                assert(!"How can this node be in the node array?");
            }
            break;

        default:
            assert(false);
        }
        ++aRg.aStart;
    }
}

void SwNodes::DelDummyNodes( const SwNodeRange& rRg )
{
    SwNodeIndex aIdx( rRg.aStart );
    while( aIdx.GetIndex() < rRg.aEnd.GetIndex() )
    {
        if (SwNodeType::PlaceHolder == aIdx.GetNode().GetNodeType())
            RemoveNode( aIdx.GetIndex(), SwNodeOffset(1), true );
        else
            ++aIdx;
    }
}

SwStartNode* SwNodes::MakeEmptySection( SwNode& rWhere,
                                        SwStartNodeType eSttNdTyp )
{
    SwStartNode* pSttNd = new SwStartNode( rWhere, SwNodeType::Start, eSttNdTyp );
    new SwEndNode( rWhere, *pSttNd );
    return pSttNd;
}

SwStartNode* SwNodes::MakeTextSection( const SwNode & rWhere,
                                        SwStartNodeType eSttNdTyp,
                                        SwTextFormatColl *pColl )
{
    SwStartNode* pSttNd = new SwStartNode( rWhere, SwNodeType::Start, eSttNdTyp );
    new SwEndNode( rWhere, *pSttNd );
    MakeTextNode( SwNodeIndex( rWhere, - 1 ).GetNode(), pColl );
    return pSttNd;
}

//TODO: provide better documentation
/** go to next section that is not protected nor hidden
 *
 * @note if !bSkipHidden and !bSkipProtect, use GoNext/GoPrevious
 *
 * @param pIdx
 * @param bSkipHidden
 * @param bSkipProtect
 * @return
 * @see SwNodes::GoNext
 * @see SwNodes::GoPrevious
 * @see SwNodes::GoNextSection (TODO: seems to be C&P programming here)
*/
SwContentNode* SwNodes::GoNextSection( SwNodeIndex * pIdx,
                            bool bSkipHidden, bool bSkipProtect ) const
{
    bool bFirst = true;
    SwNodeIndex aTmp( *pIdx );
    const SwNode* pNd;
    while( aTmp < Count() - 1 )
    {
        pNd = & aTmp.GetNode();
        if (SwNodeType::Section == pNd->GetNodeType())
        {
            const SwSection& rSect = static_cast<const SwSectionNode*>(pNd)->GetSection();
            if( (bSkipHidden && rSect.CalcHiddenFlag()) ||
                (bSkipProtect && rSect.IsProtectFlag()) )
                // than skip the section
                aTmp = *pNd->EndOfSectionNode();
        }
        else if( bFirst )
        {
            if( pNd->m_pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = static_cast<SwSectionNode*>(pNd->
                                m_pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.CalcHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // than skip the section
                    aTmp = *pNd->EndOfSectionNode();
            }
        }
        else if( SwNodeType::ContentMask & pNd->GetNodeType() )
        {
            const SwSectionNode* pSectNd;
            if( ( bSkipHidden || bSkipProtect ) &&
                nullptr != (pSectNd = pNd->FindSectionNode() ) &&
                ( ( bSkipHidden && pSectNd->GetSection().CalcHiddenFlag() ) ||
                  ( bSkipProtect && pSectNd->GetSection().IsProtectFlag() )) )
            {
                aTmp = *pSectNd->EndOfSectionNode();
            }
            else
            {
                (*pIdx) = aTmp;
                return const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd));
            }
        }
        ++aTmp;
        bFirst = false;
    }
    return nullptr;
}

//TODO: provide better documentation
/** go to next section that is not protected nor hidden
 *
 * @note if !bSkipHidden and !bSkipProtect, use GoNext/GoPrevious
 *
 * @param pIdx
 * @param bSkipHidden
 * @param bSkipProtect
 * @return
 * @see SwNodes::GoNext
 * @see SwNodes::GoPrevious
 * @see SwNodes::GoNextSection (TODO: seems to be C&P programming here)
*/
SwContentNode* SwNodes::GoNextSection( SwPosition * pIdx,
                            bool bSkipHidden, bool bSkipProtect ) const
{
    bool bFirst = true;
    SwNodeIndex aTmp( pIdx->GetNode() );
    const SwNode* pNd;
    while( aTmp < Count() - 1 )
    {
        pNd = & aTmp.GetNode();
        if (SwNodeType::Section == pNd->GetNodeType())
        {
            const SwSection& rSect = static_cast<const SwSectionNode*>(pNd)->GetSection();
            if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                (bSkipProtect && rSect.IsProtectFlag()) )
                // than skip the section
                aTmp = *pNd->EndOfSectionNode();
        }
        else if( bFirst )
        {
            if( pNd->m_pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = static_cast<SwSectionNode*>(pNd->
                                m_pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // than skip the section
                    aTmp = *pNd->EndOfSectionNode();
            }
        }
        else if( SwNodeType::ContentMask & pNd->GetNodeType() )
        {
            const SwSectionNode* pSectNd;
            if( ( bSkipHidden || bSkipProtect ) &&
                nullptr != (pSectNd = pNd->FindSectionNode() ) &&
                ( ( bSkipHidden && pSectNd->GetSection().IsHiddenFlag() ) ||
                  ( bSkipProtect && pSectNd->GetSection().IsProtectFlag() )) )
            {
                aTmp = *pSectNd->EndOfSectionNode();
            }
            else
            {
                pIdx->Assign(aTmp);
                return const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd));
            }
        }
        ++aTmp;
        bFirst = false;
    }
    return nullptr;
}

///@see SwNodes::GoNextSection (TODO: seems to be C&P programming here)
SwContentNode* SwNodes::GoPrevSection( SwNodeIndex * pIdx,
                            bool bSkipHidden, bool bSkipProtect )
{
    bool bFirst = true;
    SwNodeIndex aTmp( *pIdx );
    SwNodeOffset aGlobalStart(aTmp.GetNodes().StartOfGlobalSection(pIdx->GetNode()));
    const SwNode* pNd;
    while (aTmp > aGlobalStart)
    {
        pNd = & aTmp.GetNode();
        if (SwNodeType::End == pNd->GetNodeType())
        {
            if( pNd->m_pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = static_cast<SwSectionNode*>(pNd->
                                            m_pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // than skip section
                    aTmp = *pNd->StartOfSectionNode();
            }
            bFirst = false;
        }
        else if( bFirst )
        {
            bFirst = false;
            if( pNd->m_pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = static_cast<SwSectionNode*>(pNd->
                                m_pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // than skip section
                    aTmp = *pNd->StartOfSectionNode();
            }
        }
        else if( SwNodeType::ContentMask & pNd->GetNodeType() )
        {
            const SwSectionNode* pSectNd;
            if( ( bSkipHidden || bSkipProtect ) &&
                nullptr != (pSectNd = pNd->FindSectionNode() ) &&
                ( ( bSkipHidden && pSectNd->GetSection().IsHiddenFlag() ) ||
                  ( bSkipProtect && pSectNd->GetSection().IsProtectFlag() )) )
            {
                aTmp = *pSectNd;
            }
            else
            {
                (*pIdx) = aTmp;
                return const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd));
            }
        }
        --aTmp;
    }
    return nullptr;
}

///@see SwNodes::GoNextSection (TODO: seems to be C&P programming here)
SwContentNode* SwNodes::GoPrevSection( SwPosition * pIdx,
                            bool bSkipHidden, bool bSkipProtect )
{
    bool bFirst = true;
    SwNodeIndex aTmp( pIdx->GetNode() );
    SwNodeOffset aGlobalStart(aTmp.GetNodes().StartOfGlobalSection(pIdx->GetNode()));
    const SwNode* pNd;
    while (aTmp > aGlobalStart)
    {
        pNd = & aTmp.GetNode();
        if (SwNodeType::End == pNd->GetNodeType())
        {
            if( pNd->m_pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = static_cast<SwSectionNode*>(pNd->
                                            m_pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // than skip section
                    aTmp = *pNd->StartOfSectionNode();
            }
            bFirst = false;
        }
        else if( bFirst )
        {
            bFirst = false;
            if( pNd->m_pStartOfSection->IsSectionNode() )
            {
                const SwSection& rSect = static_cast<SwSectionNode*>(pNd->
                                m_pStartOfSection)->GetSection();
                if( (bSkipHidden && rSect.IsHiddenFlag()) ||
                    (bSkipProtect && rSect.IsProtectFlag()) )
                    // than skip section
                    aTmp = *pNd->StartOfSectionNode();
            }
        }
        else if( SwNodeType::ContentMask & pNd->GetNodeType() )
        {
            const SwSectionNode* pSectNd;
            if( ( bSkipHidden || bSkipProtect ) &&
                nullptr != (pSectNd = pNd->FindSectionNode() ) &&
                ( ( bSkipHidden && pSectNd->GetSection().IsHiddenFlag() ) ||
                  ( bSkipProtect && pSectNd->GetSection().IsProtectFlag() )) )
            {
                aTmp = *pSectNd;
            }
            else
            {
                pIdx->Assign(aTmp);
                return const_cast<SwContentNode*>(static_cast<const SwContentNode*>(pNd));
            }
        }
        --aTmp;
    }
    return nullptr;
}

//TODO: The inventor of the "single responsibility principle" will be crying if you ever show this code to him!
/** find the next/previous ContentNode or table node that should have layout
 * frames that are siblings to the ones of the node at rFrameNd.
 *
 * Search is started backward with the one before rFrameNd and
 * forward after pEnd.
 *
 * @param rFrameNd node with frames to search in
 * @param pEnd last node after rFrameNd that should be excluded from search
 * @return result node; nullptr if not found
 */
SwNode* SwNodes::FindPrvNxtFrameNode( const SwNode& rFrameNd,
        SwNode const*const pEnd,
        SwRootFrame const*const pLayout) const
{
    assert(pEnd != nullptr); // every caller currently

    // no layout -> skip
    if (!GetDoc().getIDocumentLayoutAccess().GetCurrentViewShell())
        return nullptr;

    const SwNode *const pSttNd = &rFrameNd;

    // inside a hidden section?
    const SwSectionNode *const pSectNd = pSttNd->IsSectionNode()
                ? pSttNd->StartOfSectionNode()->FindSectionNode()
                : pSttNd->FindSectionNode();
    if (pSectNd && pSectNd->GetSection().CalcHiddenFlag())
        return nullptr;

    // in a table in table situation we have to assure that we don't leave the
    // outer table cell when the inner table is looking for a PrvNxt...
    const SwTableNode *const pTableNd = pSttNd->IsTableNode()
            ? pSttNd->StartOfSectionNode()->FindTableNode()
            : pSttNd->FindTableNode();
    SwNodeIndex aIdx( rFrameNd );

    // search backward for a content or table node

    --aIdx;
    SwNode* pFrameNd = &aIdx.GetNode();

    do
    {
        if (pFrameNd->IsContentNode())
        {
            // TODO why does this not check for nested tables like forward direction
            return pFrameNd;
        }
        else if (pFrameNd->IsEndNode() && pFrameNd->StartOfSectionNode()->IsTableNode())
        {
            if (pLayout == nullptr
                || !pLayout->HasMergedParas()
                || pFrameNd->StartOfSectionNode()->GetRedlineMergeFlag() != SwNode::Merge::Hidden)
            {
                pFrameNd = pFrameNd->StartOfSectionNode();
                return pFrameNd;
            }
            else
            {
                aIdx = *pFrameNd->StartOfSectionNode();
                --aIdx;
                pFrameNd = &aIdx.GetNode();
            }
        }
        else if (pFrameNd->IsSectionNode()
            || (pFrameNd->IsEndNode() && pFrameNd->StartOfSectionNode()->IsSectionNode()))
        {
            pFrameNd = GoPrevSection( &aIdx, true, false );
            // did we move *into* a table?
            if (pFrameNd && ::CheckNodesRange(aIdx.GetNode(), rFrameNd, true))
            {
                for (SwTableNode * pTable = pFrameNd->FindTableNode();
                    pTable && pTable->EndOfSectionIndex() < rFrameNd.GetIndex();
                    pTable = pTable->StartOfSectionNode()->FindTableNode())
                {
                    pFrameNd = pTable->EndOfSectionNode();
                }
                if (pFrameNd->IsEndNode())
                {   // GoPrevSection() checks that text node isn't section-hidden,
                    // so table node between can't be section-hidden either
                    assert(pFrameNd->StartOfSectionNode()->IsTableNode());
                    continue; // check other hidden conditions on next iteration
                }
            }
            if ( nullptr != pFrameNd && !(
                    ::CheckNodesRange( aIdx.GetNode(), rFrameNd, true ) &&
                    // Never out of the table at the start
                    pFrameNd->FindTableNode() == pTableNd &&
                    // Bug 37652: Never out of the table at the end
                    (!pFrameNd->FindTableNode() || pFrameNd->FindTableBoxStartNode()
                        == pSttNd->FindTableBoxStartNode() ) &&
                     (!pSectNd || pSttNd->IsSectionNode() ||
                      pSectNd->GetIndex() < pFrameNd->GetIndex())
                    ))
            {
                pFrameNd = nullptr; // no preceding content node, stop search
            }
        }
        else
        {
            pFrameNd = nullptr; // no preceding content node, stop search
        }
    }
    while (pFrameNd != nullptr);

    // search forward for a content or table node

    aIdx = pEnd->GetIndex() + 1;
    pFrameNd = &aIdx.GetNode();

    do
    {
        if (pFrameNd->IsContentNode())
        {
            // Undo when merging a table with one before, if there is also one after it.
            // However, if the node is in a table, it needs to be returned if the
            // SttNode is a section or a table!
            SwTableNode *const pTableNode = pFrameNd->FindTableNode();
            if (pSttNd->IsTableNode() &&
                nullptr != pTableNode &&
                // TABLE IN TABLE:
                pTableNode != pSttNd->StartOfSectionNode()->FindTableNode())
            {
                pFrameNd = pTableNode;
            }
            return pFrameNd;
        }
        else if (pFrameNd->IsTableNode())
        {
            if (pLayout == nullptr
                || !pLayout->HasMergedParas()
                || pFrameNd->GetRedlineMergeFlag() != SwNode::Merge::Hidden)
            {
                return pFrameNd;
            }
            else
            {
                aIdx = *pFrameNd->EndOfSectionNode();
                ++aIdx;
                pFrameNd = &aIdx.GetNode();
            }
        }
        else if (pFrameNd->IsSectionNode()
            || (pFrameNd->IsEndNode() && pFrameNd->StartOfSectionNode()->IsSectionNode()))
        {
            pFrameNd = GoNextSection( &aIdx, true, false );
            // did we move *into* a table?
            if (pFrameNd && ::CheckNodesRange(aIdx.GetNode(), rFrameNd, true))
            {
                for (SwTableNode * pTable = pFrameNd->FindTableNode();
                    pTable && pEnd->GetIndex() < pTable->GetIndex();
                    pTable = pTable->StartOfSectionNode()->FindTableNode())
                {
                    pFrameNd = pTable;
                }
                if (pFrameNd->IsTableNode())
                {   // GoNextSection() checks that text node isn't section-hidden,
                    // so table node between can't be section-hidden either
                    continue; // check other hidden conditions on next iteration
                }
            }
            // NEVER leave the section when doing this!
            if (pFrameNd
                && !(::CheckNodesRange(aIdx.GetNode(), rFrameNd, true)
                     && (pFrameNd->FindTableNode() == pTableNd &&
                        // NEVER go out of the table cell at the end
                        (!pFrameNd->FindTableNode() || pFrameNd->FindTableBoxStartNode()
                            == pSttNd->FindTableBoxStartNode()))
                     && (!pSectNd || pSttNd->IsSectionNode() ||
                       pSectNd->EndOfSectionIndex() > pFrameNd->GetIndex()))
                )
            {
                pFrameNd = nullptr; // no following content node, stop search
            }
        }
        else
        {
            pFrameNd = nullptr; // no preceding content node, stop search
        }
    }
    while (pFrameNd != nullptr);

    return pFrameNd;
}

void SwNodes::ForEach( SwNodeOffset nStart, SwNodeOffset nEnd,
                       FnForEach_SwNodes fn, void* pArgs )
{
    if( nEnd > SwNodeOffset(m_nSize) )
        nEnd = SwNodeOffset(m_nSize);

    if( nStart >= nEnd )
        return;

    sal_uInt16 cur = Index2Block( sal_Int32(nStart) );
    BlockInfo** pp = m_ppInf.get() + cur;
    BlockInfo* p = *pp;
    sal_uInt16 nElem = sal_uInt16( sal_Int32(nStart) - p->nStart );
    auto pElem = p->mvData.begin() + nElem;
    nElem = p->nElem - nElem;
    for(;;)
    {
        if( !(*fn)( static_cast<SwNode *>(*pElem++), pArgs ) || ++nStart >= nEnd )
            break;

        // next element
        if( !--nElem )
        {
            // new block
            p = *++pp;
            pElem = p->mvData.begin();
            nElem = p->nElem;
        }
    }
}

void SwNodes::ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs )
{
    ForEach( rStart.GetIndex(), rEnd.GetIndex(), fnForEach, pArgs );
}

void SwNodes::ForEach( SwNode& rStart, SwNode& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs )
{
    ForEach( rStart.GetIndex(), rEnd.GetIndex(), fnForEach, pArgs );
}

void SwNodes::RemoveNode( SwNodeOffset nDelPos, SwNodeOffset nSz, bool bDel )
{
#ifndef NDEBUG
    SwNode *const pFirst((*this)[nDelPos]);
#endif
    for (SwNodeOffset nCnt(0); nCnt < nSz; nCnt++)
    {
        SwNode* pNode = (*this)[ nDelPos + nCnt ];
        SwTextNode * pTextNd = pNode->GetTextNode();

        if (pTextNd)
        {
            pTextNd->RemoveFromList();
            // remove RndStdIds::FLY_AS_CHAR *before* adjusting SwNodeIndex
            // so their anchor still points to correct node when deleted!
            // NOTE: this will call RemoveNode() recursively!
            // so adjust our indexes to account for removed nodes
            SwNodeOffset const nPos = pTextNd->GetIndex();
            SwpHints *const pHints(pTextNd->GetpSwpHints());
            if (pHints)
            {
                std::vector<SwTextAttr*> flys;
                for (size_t i = 0; i < pHints->Count(); ++i)
                {
                    SwTextAttr *const pHint(pHints->Get(i));
                    if (RES_TXTATR_FLYCNT == pHint->Which())
                    {
                        flys.push_back(pHint);
                    }
                }
                for (SwTextAttr * pHint : flys)
                {
                    pTextNd->DeleteAttribute(pHint);
                }   // pHints may be dead now
                SwNodeOffset const nDiff = nPos - pTextNd->GetIndex();
                if (nDiff)
                {
                    nDelPos -= nDiff;
                }
                assert(pTextNd == (*this)[nDelPos + nCnt]);
                assert(pFirst == (*this)[nDelPos]);
            }
        }
        SwTableNode* pTableNode = pNode->GetTableNode();
        if (pTableNode)
        {
            // The node that is deleted is a table node.
            // Need to make sure that all the redlines that are
            // related to this table are removed from the
            // 'Extra Redlines' array
            pTableNode->RemoveRedlines();
        }

        SwSectionNode* pSectionNode = pNode->GetSectionNode();
        if (comphelper::LibreOfficeKit::isActive() && pSectionNode && !GetDoc().IsClipBoard() && SfxViewShell::Current())
        {
            OUString fieldCommand = pSectionNode->GetSection().GetSectionName();
            tools::JsonWriter aJson;
            aJson.put("commandName", ".uno:DeleteSection");
            aJson.put("success", true);
            {
                auto result = aJson.startNode("result");
                aJson.put("DeleteSection", fieldCommand);
            }

            SfxViewShell::Current()->libreOfficeKitViewCallback(LOK_CALLBACK_UNO_COMMAND_RESULT, aJson.finishAndGetAsOString());

        }
    }

    SwNodeOffset nEnd = nDelPos + nSz;
    SwNode* pNew = (*this)[ nEnd ];

    for (SwNodeIndex& rIndex : m_vIndices->GetRingContainer())
    {
        SwNodeOffset const nIdx = rIndex.GetIndex();
        if (nDelPos <= nIdx && nIdx < nEnd)
            rIndex = *pNew;
    }

    std::vector<BigPtrEntry> aTempEntries;
    if( bDel )
    {
        SwNodeOffset nCnt = nSz;
        BigPtrEntry *pDel = (*this)[ nDelPos+nCnt-1 ], *pPrev = (*this)[ nDelPos+nCnt-2 ];

        // set temporary object
        // JP 24.08.98: this should actually be removed because one could
        // call Remove recursively, e.g. for character bound frames. However,
        // since there happens way too much here, this temporary object was
        // inserted that will be deleted in Remove again (see Bug 55406)
        aTempEntries.resize(sal_Int32(nCnt));

        while( nCnt-- )
        {
            delete pDel;
            // coverity[use_after_free : FALSE] - pPrev will be reassigned if there will be another iteration to the loop
            pDel = pPrev;
            sal_uLong nPrevNdIdx = pPrev->GetPos();
            BigPtrEntry* pTempEntry = &aTempEntries[sal_Int32(nCnt)];
            BigPtrArray::Replace( nPrevNdIdx+1, pTempEntry );
            if( nCnt )
                pPrev = BigPtrArray::operator []( nPrevNdIdx  - 1 );
                    // the accessed element can be a naked BigPtrEntry from
                    // aTempEntries, so the downcast to SwNode* in
                    // SwNodes::operator[] would be illegal (and unnecessary)
        }
        nDelPos = SwNodeOffset(pDel->GetPos() + 1);
    }

    BigPtrArray::Remove( sal_Int32(nDelPos), sal_Int32(nSz) );
}

void SwNodes::InsertNode( SwNode* pNode, const SwNodeIndex& rPos )
{
    BigPtrEntry* pIns = pNode;
    BigPtrArray::Insert( pIns, sal_Int32(rPos.GetIndex()) );
}

void SwNodes::InsertNode( SwNode* pNode, SwNodeOffset nPos )
{
    BigPtrEntry* pIns = pNode;
    BigPtrArray::Insert( pIns, sal_Int32(nPos) );
}

// ->#112139#
SwNode * SwNodes::DocumentSectionStartNode(SwNode * pNode) const
{
    if (nullptr != pNode)
    {
        SwNodeIndex aIdx(*pNode);

        if (aIdx <= (*this)[SwNodeOffset(0)]->EndOfSectionIndex())
            pNode = (*this)[SwNodeOffset(0)];
        else
        {
            while ((*this)[SwNodeOffset(0)] != pNode->StartOfSectionNode())
                pNode = pNode->StartOfSectionNode();
        }
    }

    return pNode;
}

SwNode * SwNodes::DocumentSectionEndNode(SwNode * pNode) const
{
    return DocumentSectionStartNode(pNode)->EndOfSectionNode();
}

bool SwNodes::IsDocNodes() const
{
    return this == &m_rMyDoc.GetNodes();
}

void SwNodes::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwNodes"));
    for (SwNodeOffset i(0); i < Count(); ++i)
        (*this)[i]->dumpAsXml(pWriter);
    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
