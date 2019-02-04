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

#include <memory>
#include <UndoSort.hxx>
#include <doc.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <UndoTable.hxx>
#include <sortopt.hxx>
#include <docsort.hxx>
#include <redline.hxx>
#include <node2lay.hxx>

// Undo for Sorting
SwSortUndoElement::~SwSortUndoElement()
{
    // are there string pointers saved?
    if( 0xffffffff != SORT_TXT_TBL.TXT.nID )
    {
        delete SORT_TXT_TBL.TBL.pSource;
        delete SORT_TXT_TBL.TBL.pTarget;
    }
}

SwUndoSort::SwUndoSort(const SwPaM& rRg, const SwSortOptions& rOpt)
    : SwUndo(SwUndoId::SORT_TXT, rRg.GetDoc())
    , SwUndRng(rRg)
    , nTableNd(0)
{
    pSortOpt.reset( new SwSortOptions(rOpt) );
}

SwUndoSort::SwUndoSort( sal_uLong nStt, sal_uLong nEnd, const SwTableNode& rTableNd,
                        const SwSortOptions& rOpt, bool bSaveTable )
    : SwUndo(SwUndoId::SORT_TBL, rTableNd.GetDoc())
{
    nSttNode = nStt;
    nEndNode = nEnd;
    nTableNd   = rTableNd.GetIndex();

    pSortOpt.reset( new SwSortOptions(rOpt) );
    if( bSaveTable )
        pUndoTableAttr.reset( new SwUndoAttrTable( rTableNd ) );
}

SwUndoSort::~SwUndoSort()
{
    pSortOpt.reset();
    pUndoTableAttr.reset();
}

void SwUndoSort::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    if(pSortOpt->bTable)
    {
        // Undo for Table
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        if( pUndoTableAttr )
        {
            pUndoTableAttr->UndoImpl(rContext);
        }

        SwTableNode* pTableNd = rDoc.GetNodes()[ nTableNd ]->GetTableNode();

        // #i37739# A simple 'MakeFrames' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2LayoutSaveUpperFrames aNode2Layout(*pTableNd);

        pTableNd->DelFrames();
        const SwTable& rTable = pTableNd->GetTable();

        SwMovedBoxes aMovedList;
        for (std::unique_ptr<SwSortUndoElement> & i : m_SortList)
        {
            const SwTableBox* pSource = rTable.GetTableBox(
                    *i->SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTable.GetTableBox(
                    *i->SORT_TXT_TBL.TBL.pTarget );

            // move back
            MoveCell(&rDoc, pTarget, pSource,
                     USHRT_MAX != aMovedList.GetPos(pSource) );

            // store moved entry in list
            aMovedList.push_back(pTarget);
        }

        // Restore table frames:
        // #i37739# A simple 'MakeFrames' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const sal_uLong nIdx = pTableNd->GetIndex();
        aNode2Layout.RestoreUpperFrames( rDoc.GetNodes(), nIdx, nIdx + 1 );
    }
    else
    {
        // Undo for Text
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        RemoveIdxFromRange(rPam, true);

        // create index for (sorted) positions
        // The IndexList must be created based on (asc.) sorted SourcePosition.
        std::vector<SwNodeIndex> aIdxList;
        aIdxList.reserve(m_SortList.size());

        for (size_t i = 0; i < m_SortList.size(); ++i)
        {
            for (std::unique_ptr<SwSortUndoElement> & j : m_SortList)
            {
                if (j->SORT_TXT_TBL.TXT.nSource == nSttNode + i)
                {
                    aIdxList.push_back( SwNodeIndex( rDoc.GetNodes(),
                                            j->SORT_TXT_TBL.TXT.nTarget ) );
                    break;
                }
            }
        }

        for (size_t i = 0; i < m_SortList.size(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i );
            SwNodeRange aRg( aIdxList[i], 0, aIdxList[i], 1 );
            rDoc.getIDocumentContentOperations().MoveNodeRange(aRg, aIdx,
                SwMoveFlags::DEFAULT);
        }
        // delete indices
        aIdxList.clear();
        SetPaM(rPam, true);
    }
}

void SwUndoSort::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    if(pSortOpt->bTable)
    {
        // Redo for Table
        RemoveIdxFromSection( rDoc, nSttNode, &nEndNode );

        SwTableNode* pTableNd = rDoc.GetNodes()[ nTableNd ]->GetTableNode();

        // #i37739# A simple 'MakeFrames' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        SwNode2LayoutSaveUpperFrames aNode2Layout(*pTableNd);

        pTableNd->DelFrames();
        const SwTable& rTable = pTableNd->GetTable();

        SwMovedBoxes aMovedList;
        for (std::unique_ptr<SwSortUndoElement> & i : m_SortList)
        {
            const SwTableBox* pSource = rTable.GetTableBox(
                    *i->SORT_TXT_TBL.TBL.pSource );
            const SwTableBox* pTarget = rTable.GetTableBox(
                    *i->SORT_TXT_TBL.TBL.pTarget );

            // move back
            MoveCell(&rDoc, pSource, pTarget,
                     USHRT_MAX != aMovedList.GetPos( pTarget ) );
            // store moved entry in list
            aMovedList.push_back( pSource );
        }

        if( pUndoTableAttr )
        {
            pUndoTableAttr->RedoImpl(rContext);
        }

        // Restore table frames:
        // #i37739# A simple 'MakeFrames' after the node sorting
        // does not work if the table is inside a frame and has no prev/next.
        const sal_uLong nIdx = pTableNd->GetIndex();
        aNode2Layout.RestoreUpperFrames( rDoc.GetNodes(), nIdx, nIdx + 1 );
    }
    else
    {
        // Redo for Text
        SwPaM & rPam( AddUndoRedoPaM(rContext) );
        SetPaM(rPam);
        RemoveIdxFromRange(rPam, true);

        std::vector<SwNodeIndex> aIdxList;
        aIdxList.reserve(m_SortList.size());

        for (size_t i = 0; i < m_SortList.size(); ++i)
        {   // current position is starting point
            aIdxList.push_back( SwNodeIndex( rDoc.GetNodes(),
                                             m_SortList[i]->SORT_TXT_TBL.TXT.nSource) );
        }

        for (size_t i = 0; i < m_SortList.size(); ++i)
        {
            SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode + i);
            SwNodeRange aRg( aIdxList[i], 0, aIdxList[i], 1 );
            rDoc.getIDocumentContentOperations().MoveNodeRange(aRg, aIdx,
                SwMoveFlags::DEFAULT);
        }
        // delete indices
        aIdxList.clear();
        SetPaM(rPam, true);
        SwTextNode const*const pTNd = rPam.GetNode().GetTextNode();
        if( pTNd )
        {
            rPam.GetPoint()->nContent = pTNd->GetText().getLength();
        }
    }
}

void SwUndoSort::RepeatImpl(::sw::RepeatContext & rContext)
{
    // table not repeat capable
    if(!pSortOpt->bTable)
    {
        SwPaM *const pPam = & rContext.GetRepeatPaM();
        SwDoc& rDoc = *pPam->GetDoc();

        if( !rDoc.IsIdxInTable( pPam->Start()->nNode ) )
            rDoc.SortText(*pPam, *pSortOpt);
    }
}

void SwUndoSort::Insert( const OUString& rOrgPos, const OUString& rNewPos)
{
    m_SortList.push_back(std::make_unique< SwSortUndoElement>(rOrgPos, rNewPos));
}

void SwUndoSort::Insert( sal_uLong nOrgPos, sal_uLong nNewPos)
{
    m_SortList.push_back(std::make_unique<SwSortUndoElement>(nOrgPos, nNewPos));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
