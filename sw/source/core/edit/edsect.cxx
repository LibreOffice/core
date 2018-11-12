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

#include <editsh.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentContentOperations.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <swundo.hxx>
#include <section.hxx>
#include <edimp.hxx>
#include <sectfrm.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <rootfrm.hxx>

SwSection const*
SwEditShell::InsertSection(
        SwSectionData & rNewData, SfxItemSet const*const pAttr)
{
    const SwSection* pRet = nullptr;
    if( !IsTableMode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSSECTION, nullptr );

        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            SwSection const*const pNew =
                GetDoc()->InsertSwSection( rPaM, rNewData, nullptr, pAttr );
            if( !pRet )
                pRet = pNew;
        }

        GetDoc()->GetIDocumentUndoRedo().EndUndo( SwUndoId::INSSECTION, nullptr );
        EndAllAction();
    }
    return pRet;
}

bool SwEditShell::IsInsRegionAvailable() const
{
    if( IsTableMode() )
        return false;
    SwPaM* pCursor = GetCursor();
    if( pCursor->GetNext() != pCursor )
        return false;
    if( pCursor->HasMark() )
        return 0 != SwDoc::IsInsRegionAvailable( *pCursor );

    return true;
}

const SwSection* SwEditShell::GetCurrSection() const
{
    if( IsTableMode() )
        return nullptr;

    return SwDoc::GetCurrSection( *GetCursor()->GetPoint() );
}

/** Deliver the responsible area of the columns.
 *
 * In footnotes it may not be the area within the footnote.
 */
SwSection* SwEditShell::GetAnySection( bool bOutOfTab, const Point* pPt )
{
    SwFrame *pFrame;
    if ( pPt )
    {
        SwPosition aPos( *GetCursor()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCursorOfst( &aPos, aPt );
        SwContentNode *pNd = aPos.nNode.GetNode().GetContentNode();
        std::pair<Point, bool> const tmp(*pPt, true);
        pFrame = pNd->getLayoutFrame(GetLayout(), nullptr, &tmp);
    }
    else
        pFrame = GetCurrFrame( false );

    if( bOutOfTab && pFrame )
        pFrame = pFrame->FindTabFrame();
    if( pFrame && pFrame->IsInSct() )
    {
        SwSectionFrame* pSect = pFrame->FindSctFrame();
        OSL_ENSURE( pSect, "GetAnySection: Where's my Sect?" );
        if( pSect->IsInFootnote() && pSect->GetUpper()->IsInSct() )
        {
            pSect = pSect->GetUpper()->FindSctFrame();
            OSL_ENSURE( pSect, "GetAnySection: Where's my SectFrame?" );
        }
        return pSect->GetSection();
    }
    return nullptr;
}

size_t SwEditShell::GetSectionFormatCount() const
{
    return GetDoc()->GetSections().size();
}

bool SwEditShell::IsAnySectionInDoc() const
{
    const SwSectionFormats& rFormats = GetDoc()->GetSections();

    for( const SwSectionFormat* pFormat : rFormats )
    {
        SectionType eTmpType;
        if( pFormat->IsInNodesArr() &&
            ( (eTmpType = pFormat->GetSection()->GetType()) != TOX_CONTENT_SECTION
               && TOX_HEADER_SECTION != eTmpType ) )
        {
            return true;
        }
    }
    return false;
}

size_t SwEditShell::GetSectionFormatPos( const SwSectionFormat& rFormat ) const
{
    SwSectionFormat* pFormat = const_cast<SwSectionFormat*>(&rFormat);
    return GetDoc()->GetSections().GetPos( pFormat );
}

const SwSectionFormat& SwEditShell::GetSectionFormat(size_t nFormat) const
{
    return *GetDoc()->GetSections()[ nFormat ];
}

void SwEditShell::DelSectionFormat(size_t nFormat)
{
    StartAllAction();
    GetDoc()->DelSectionFormat( GetDoc()->GetSections()[ nFormat ] );
    // Call the AttrChangeNotify on the UI page.
    CallChgLnk();
    EndAllAction();
}

void SwEditShell::UpdateSection(size_t const nSect,
        SwSectionData & rNewData, SfxItemSet const*const pAttr)
{
    StartAllAction();
    GetDoc()->UpdateSection( nSect, rNewData, pAttr );
    // Call the AttrChangeNotify on the UI page.
    CallChgLnk();
    EndAllAction();
}

OUString SwEditShell::GetUniqueSectionName( const OUString* pChkStr ) const
{
    return GetDoc()->GetUniqueSectionName( pChkStr );
}

void SwEditShell::SetSectionAttr( const SfxItemSet& rSet,
                                    SwSectionFormat* pSectFormat )
{
    if( pSectFormat )
        SetSectionAttr_( *pSectFormat, rSet );
    else
    {
        // for all section in the selection

        for(SwPaM& rPaM : GetCursor()->GetRingContainer())
        {
            const SwPosition* pStt = rPaM.Start(),
                            * pEnd = rPaM.End();

            SwSectionNode* pSttSectNd = pStt->nNode.GetNode().FindSectionNode(),
                               * pEndSectNd = pEnd->nNode.GetNode().FindSectionNode();

            if( pSttSectNd || pEndSectNd )
            {
                if( pSttSectNd )
                    SetSectionAttr_( *pSttSectNd->GetSection().GetFormat(),
                                    rSet );
                if( pEndSectNd && pSttSectNd != pEndSectNd )
                    SetSectionAttr_( *pEndSectNd->GetSection().GetFormat(),
                                    rSet );

                if( pSttSectNd && pEndSectNd )
                {
                    SwNodeIndex aSIdx( pStt->nNode );
                    SwNodeIndex aEIdx( pEnd->nNode );
                    if( pSttSectNd->EndOfSectionIndex() <
                        pEndSectNd->GetIndex() )
                    {
                        aSIdx = pSttSectNd->EndOfSectionIndex() + 1;
                        aEIdx = *pEndSectNd;
                    }

                    while( aSIdx < aEIdx )
                    {
                        if( nullptr != (pSttSectNd = aSIdx.GetNode().GetSectionNode())
                            || ( aSIdx.GetNode().IsEndNode() &&
                                nullptr != ( pSttSectNd = aSIdx.GetNode().
                                    StartOfSectionNode()->GetSectionNode())) )
                            SetSectionAttr_( *pSttSectNd->GetSection().GetFormat(),
                                            rSet );
                        ++aSIdx;
                    }
                }
            }

        }
    }
}

void SwEditShell::SetSectionAttr_( SwSectionFormat& rSectFormat,
                                    const SfxItemSet& rSet )
{
    StartAllAction();
    if(SfxItemState::SET == rSet.GetItemState(RES_CNTNT, false))
    {
        SfxItemSet aSet(rSet);
        aSet.ClearItem(RES_CNTNT);
        GetDoc()->SetAttr( aSet, rSectFormat );
    }
    else
        GetDoc()->SetAttr( rSet, rSectFormat );

    // Call the AttrChangeNotify on the UI page.
    CallChgLnk();
    EndAllAction();
}

/** Search inside the cursor selection for full selected sections.
 *
 * @return If any part of section in the selection return 0, if more than one return the count.
 */
sal_uInt16 SwEditShell::GetFullSelectedSectionCount() const
{
    sal_uInt16 nRet = 0;
    for(SwPaM& rPaM : GetCursor()->GetRingContainer())
    {

        const SwPosition* pStt = rPaM.Start(),
                        * pEnd = rPaM.End();
        const SwContentNode* pCNd;
        // check the selection, if Start at Node begin and End at Node end
        if( pStt->nContent.GetIndex() ||
            ( nullptr == ( pCNd = pEnd->nNode.GetNode().GetContentNode() )) ||
            pCNd->Len() != pEnd->nContent.GetIndex() )
        {
            nRet = 0;
            break;
        }

// !!!
// what about table at start or end ?
//      There is no selection possible!
// What about only a table inside the section ?
//      There is only a table selection possible!

        SwNodeIndex aSIdx( pStt->nNode, -1 ), aEIdx( pEnd->nNode, +1 );
        if( !aSIdx.GetNode().IsSectionNode() ||
            !aEIdx.GetNode().IsEndNode() ||
            !aEIdx.GetNode().StartOfSectionNode()->IsSectionNode() )
        {
            nRet = 0;
            break;
        }

        ++nRet;
        if( &aSIdx.GetNode() != aEIdx.GetNode().StartOfSectionNode() )
            ++nRet;

    }
    return nRet;
}

/** Find the suitable node for a special insert (alt-enter).
 *
 * This should enable inserting text before/after sections and tables.
 *
 * A node is found if:
 * 1) the innermost table/section is not in a write-protected area
 * 2) pCurrentPos is at or just before an end node
 *    (or at or just after a start node)
 * 3) there are only start/end nodes between pCurrentPos and the innermost
 *    table/section
 *
 * If a suitable node is found, an SwNode* is returned; else it is NULL.
 */
static const SwNode* lcl_SpecialInsertNode(const SwPosition* pCurrentPos)
{
    const SwNode* pReturn = nullptr;

    // the current position
    OSL_ENSURE( pCurrentPos != nullptr, "Strange, we have no position!" );
    const SwNode& rCurrentNode = pCurrentPos->nNode.GetNode();

    // find innermost section or table.  At the end of this scope,
    // pInntermostNode contain the section/table before/after which we should
    // insert our empty paragraph, or it will be NULL if none is found.
    const SwNode* pInnermostNode = nullptr;
    {
        const SwNode* pTableNode = rCurrentNode.FindTableNode();
        const SwNode* pSectionNode = rCurrentNode.FindSectionNode();

        // find the table/section which is close
        if( pTableNode == nullptr )
            pInnermostNode = pSectionNode;
        else if ( pSectionNode == nullptr )
            pInnermostNode = pTableNode;
        else
        {
            // compare and choose the larger one
            pInnermostNode =
                ( pSectionNode->GetIndex() > pTableNode->GetIndex() )
                ? pSectionNode : pTableNode;
        }
    }

    // The previous version had a check to skip empty read-only sections. Those
    // shouldn't occur, so we only need to check whether our pInnermostNode is
    // inside a protected area.

    // Now, pInnermostNode is NULL or the innermost section or table node.
    if( (pInnermostNode != nullptr) && !pInnermostNode->IsProtect() )
    {
        OSL_ENSURE( pInnermostNode->IsTableNode() ||
                    pInnermostNode->IsSectionNode(), "wrong node found" );
        OSL_ENSURE( ( pInnermostNode->GetIndex() <= rCurrentNode.GetIndex() )&&
                    ( pInnermostNode->EndOfSectionNode()->GetIndex() >=
                      rCurrentNode.GetIndex() ), "wrong node found" );

        // we now need to find the possible start/end positions

        // we found a start if
        // - we're at or just before a start node
        // - there are only start nodes between the current and pInnermostNode
        SwNodeIndex aBegin( pCurrentPos->nNode );
        if( rCurrentNode.IsContentNode() &&
            (pCurrentPos->nContent.GetIndex() == 0))
            --aBegin;
        while( (aBegin != pInnermostNode->GetIndex()) &&
               aBegin.GetNode().IsStartNode() )
            --aBegin;
        bool bStart = ( aBegin == pInnermostNode->GetIndex() );

        // we found an end if
        // - we're at or just before an end node
        // - there are only end nodes between the current node and
        //   pInnermostNode's end node
        SwNodeIndex aEnd( pCurrentPos->nNode );
        if( rCurrentNode.IsContentNode() &&
            ( pCurrentPos->nContent.GetIndex() ==
              rCurrentNode.GetContentNode()->Len() ) )
            ++aEnd;
        while( (aEnd != pInnermostNode->EndOfSectionNode()->GetIndex()) &&
               aEnd.GetNode().IsEndNode() )
            ++aEnd;
        bool bEnd = ( aEnd == pInnermostNode->EndOfSectionNode()->GetIndex() );

        // evaluate result: if both start + end, end is preferred
        if( bEnd )
            pReturn = pInnermostNode->EndOfSectionNode();
        else if ( bStart )
            pReturn = pInnermostNode;
    }

    OSL_ENSURE( ( pReturn == nullptr ) || pReturn->IsStartNode() ||
                                       pReturn->IsEndNode(),
                "SpecialInsertNode failed" );
    return pReturn;
}

/** a node can be special-inserted (alt-Enter) whenever lcl_SpecialInsertNode
    finds a suitable position
*/
bool SwEditShell::CanSpecialInsert() const
{
    return nullptr != lcl_SpecialInsertNode( GetCursor()->GetPoint() );
}

/** check whether a node can be special-inserted (alt-Enter), and do so. Return
    whether insertion was possible.
 */
void SwEditShell::DoSpecialInsert()
{
    // get current node
    SwPosition* pCursorPos = GetCursor()->GetPoint();
    const SwNode* pInsertNode = lcl_SpecialInsertNode( pCursorPos );
    if( pInsertNode != nullptr )
    {
        StartAllAction();

        // adjust insert position to insert before start nodes and after end
        // nodes
        SwNodeIndex aInsertIndex( *pInsertNode,
                                  pInsertNode->IsStartNode() ? -1 : 0 );
        SwPosition aInsertPos( aInsertIndex );

        // insert a new text node, and set the cursor
        GetDoc()->getIDocumentContentOperations().AppendTextNode( aInsertPos );
        *pCursorPos = aInsertPos;

        // call AttrChangeNotify for the UI
        CallChgLnk();

        EndAllAction();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
