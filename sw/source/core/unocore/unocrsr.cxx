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

#include <unocrsr.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>

sw::UnoCursorHint::~UnoCursorHint() {}

SwUnoCursor::SwUnoCursor( const SwPosition &rPos )
    : SwCursor( rPos, nullptr )
    , m_bRemainInSection(true)
    , m_bSkipOverHiddenSections(false)
    , m_bSkipOverProtectSections(false)
{}

SwUnoCursor::~SwUnoCursor()
{
    SwDoc* pDoc = GetDoc();
    if( !pDoc->IsInDtor() )
    {
        assert(!m_aNotifier.HasListeners());
    }

    // delete the whole ring
    while( GetNext() != this )
    {
        Ring* pNxt = GetNextInRing();
        pNxt->MoveTo(nullptr); // remove from chain
        delete pNxt;       // and delete
    }
}

bool SwUnoCursor::IsReadOnlyAvailable() const
{
    return true;
}

const SwContentFrame*
SwUnoCursor::DoSetBidiLevelLeftRight( bool &, bool, bool )
{
    return nullptr; // not for uno cursor
}

void SwUnoCursor::DoSetBidiLevelUpDown()
{
    // not for uno cursor
}

bool SwUnoCursor::IsSelOvr( SwCursorSelOverFlags eFlags )
{
    if (m_bRemainInSection)
    {
        SwDoc* pDoc = GetDoc();
        SwNodeIndex aOldIdx( *pDoc->GetNodes()[ GetSavePos()->nNode ] );
        SwNodeIndex& rPtIdx = GetPoint()->nNode;
        SwStartNode *pOldSttNd = aOldIdx.GetNode().StartOfSectionNode(),
                    *pNewSttNd = rPtIdx.GetNode().StartOfSectionNode();
        if( pOldSttNd != pNewSttNd )
        {
            bool bMoveDown = GetSavePos()->nNode < rPtIdx.GetIndex();
            bool bValidPos = false;

            // search the correct surrounded start node - which the index
            // can't leave.
            while( pOldSttNd->IsSectionNode() )
                pOldSttNd = pOldSttNd->StartOfSectionNode();

            // is the new index inside this surrounded section?
            if( rPtIdx > *pOldSttNd &&
                rPtIdx < pOldSttNd->EndOfSectionIndex() )
            {
                // check if it a valid move inside this section
                // (only over SwSection's !)
                const SwStartNode* pInvalidNode;
                do {
                    pInvalidNode = nullptr;
                    pNewSttNd = rPtIdx.GetNode().StartOfSectionNode();

                    const SwStartNode *pSttNd = pNewSttNd, *pEndNd = pOldSttNd;
                    if( pSttNd->EndOfSectionIndex() >
                        pEndNd->EndOfSectionIndex() )
                    {
                        pEndNd = pNewSttNd;
                        pSttNd = pOldSttNd;
                    }

                    while( pSttNd->GetIndex() > pEndNd->GetIndex() )
                    {
                        if( !pSttNd->IsSectionNode() )
                            pInvalidNode = pSttNd;
                        pSttNd = pSttNd->StartOfSectionNode();
                    }
                    if( pInvalidNode )
                    {
                        if( bMoveDown )
                        {
                            rPtIdx.Assign( *pInvalidNode->EndOfSectionNode(), 1 );

                            if( !rPtIdx.GetNode().IsContentNode() &&
                                ( !pDoc->GetNodes().GoNextSection( &rPtIdx ) ||
                                  rPtIdx > pOldSttNd->EndOfSectionIndex() ) )
                                break;
                        }
                        else
                        {
                            rPtIdx.Assign( *pInvalidNode, -1 );

                            if( !rPtIdx.GetNode().IsContentNode() &&
                                ( !SwNodes::GoPrevSection( &rPtIdx ) ||
                                  rPtIdx < *pOldSttNd ) )
                                break;
                        }
                    }
                    else
                        bValidPos = true;
                } while ( pInvalidNode );
            }

            if( bValidPos )
            {
                SwContentNode* pCNd = GetContentNode();
                GetPoint()->nContent.Assign( pCNd, (pCNd && !bMoveDown) ? pCNd->Len() : 0);
            }
            else
            {
                rPtIdx = GetSavePos()->nNode;
                GetPoint()->nContent.Assign( GetContentNode(), GetSavePos()->nContent );
                return true;
            }
        }
    }
    return SwCursor::IsSelOvr( eFlags );
}

SwUnoTableCursor::SwUnoTableCursor(const SwPosition& rPos)
    : SwCursor(rPos, nullptr)
    , SwUnoCursor(rPos)
    , SwTableCursor(rPos)
    , m_aTableSel(rPos, nullptr)
{
    SetRemainInSection(false);
}

SwUnoTableCursor::~SwUnoTableCursor()
{
    while (m_aTableSel.GetNext() != &m_aTableSel)
        delete m_aTableSel.GetNext();
}

bool SwUnoTableCursor::IsSelOvr( SwCursorSelOverFlags eFlags )
{
    bool bRet = SwUnoCursor::IsSelOvr( eFlags );
    if( !bRet )
    {
        const SwTableNode* pTNd = GetPoint()->nNode.GetNode().FindTableNode();
        bRet = !(pTNd == GetDoc()->GetNodes()[ GetSavePos()->nNode ]->
                FindTableNode() && (!HasMark() ||
                pTNd == GetMark()->nNode.GetNode().FindTableNode() ));
    }
    return bRet;
}

void SwUnoTableCursor::MakeBoxSels()
{
    const SwContentNode* pCNd;
    bool bMakeTableCursors = true;
    if( GetPoint()->nNode.GetIndex() && GetMark()->nNode.GetIndex() &&
            nullptr != ( pCNd = GetContentNode() ) && pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) &&
            nullptr != ( pCNd = GetContentNode(false) ) && pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() ) )
        bMakeTableCursors = GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->MakeTableCursors( *this );

    if ( !bMakeTableCursors )
    {
        SwSelBoxes const& rTmpBoxes = GetSelectedBoxes();
        while (!rTmpBoxes.empty())
        {
            DeleteBox(0);
        }
    }

    if( IsChgd() )
    {
        SwTableCursor::MakeBoxSels( &m_aTableSel );
        if (!GetSelectedBoxesCount())
        {
            const SwTableBox* pBox;
            const SwNode* pBoxNd = GetPoint()->nNode.GetNode().FindTableBoxStartNode();
            const SwTableNode* pTableNd = pBoxNd ? pBoxNd->FindTableNode() : nullptr;
            if( pTableNd && nullptr != ( pBox = pTableNd->GetTable().GetTableBox( pBoxNd->GetIndex() )) )
                InsertBox( *pBox );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
