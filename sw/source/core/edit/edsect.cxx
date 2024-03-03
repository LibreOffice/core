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
#include <sectfrm.hxx>
#include <cntfrm.hxx>
#include <tabfrm.hxx>
#include <rootfrm.hxx>
#include <osl/diagnose.h>

SwSection const*
SwEditShell::InsertSection(
        SwSectionData & rNewData, SfxItemSet const*const pAttr)
{
    const SwSection* pRet = nullptr;
    if( !IsTableMode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo( SwUndoId::INSSECTION, nullptr );

        for(const SwPaM& rPaM : GetCursor()->GetRingContainer())
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
        GetLayout()->GetModelPositionForViewPoint( &aPos, aPt );
        SwContentNode *pNd = aPos.GetNode().GetContentNode();
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
            ( (eTmpType = pFormat->GetSection()->GetType()) != SectionType::ToxContent
               && SectionType::ToxHeader != eTmpType ) )
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
            auto [pStt, pEnd] = rPaM.StartEnd(); // SwPosition*

            SwSectionNode* pSttSectNd = pStt->GetNode().FindSectionNode(),
                               * pEndSectNd = pEnd->GetNode().FindSectionNode();

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
                    SwNodeIndex aSIdx( pStt->GetNode() );
                    SwNodeIndex aEIdx( pEnd->GetNode() );
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

        auto [pStt, pEnd] = rPaM.StartEnd(); // SwPosition*
        const SwContentNode* pCNd;
        // check the selection, if Start at Node begin and End at Node end
        if( pStt->GetContentIndex() ||
            ( nullptr == ( pCNd = pEnd->GetNode().GetContentNode() )) ||
            pCNd->Len() != pEnd->GetContentIndex() )
        {
            nRet = 0;
            break;
        }

// !!!
// what about table at start or end ?
//      There is no selection possible!
// What about only a table inside the section ?
//      There is only a table selection possible!

        SwNodeIndex aSIdx( pStt->GetNode(), -1 ), aEIdx( pEnd->GetNode(), +1 );
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
    const SwNode& rCurrentNode = pCurrentPos->GetNode();

    // find innermost section or table.  At the end of this scope,
    // pInnermostNode contains the section/table before/after which we should
    // insert our empty paragraph, or it will be NULL if none is found.
    const SwNode* pInnermostNode = nullptr;
    const SwSection* pSection = nullptr;
    {
        const SwNode* pTableNode = rCurrentNode.FindTableNode();
        const SwNode* pSectionNode = rCurrentNode.FindSectionNode();

        // find the table/section which is close
        if( pTableNode == nullptr )
        {
            pInnermostNode = pSectionNode;
            pSection = &static_cast<const SwSectionNode*>(pSectionNode)->GetSection();
        }
        else if ( pSectionNode == nullptr )
            pInnermostNode = pTableNode;
        else
        {
            // compare and choose the larger one
            if (pSectionNode->GetIndex() > pTableNode->GetIndex())
            {
                pInnermostNode = pSectionNode;
                pSection = &static_cast<const SwSectionNode*>(pSectionNode)->GetSection();
            }
            else
                pInnermostNode = pTableNode;
        }
    }
    if(pInnermostNode != nullptr)
    {
        bool bIsProtected = pInnermostNode->IsProtect();

        //special case - ToxSection
        // - in this case the inner section could be tox header
        //   section but the new node should be before the content section
        //   protection of the tox should not prevent the insertion
        //   only protection outside needs to be checked
        if( pSection &&
            (SectionType::ToxHeader == pSection->GetType() ||
             SectionType::ToxContent == pSection->GetType()))
        {
            if (SectionType::ToxHeader == pSection->GetType())
            {
                if (const SwSection* pSectionParent = pSection->GetParent())
                    pInnermostNode = pSectionParent->GetFormat()->GetSectionNode();
            }
            bIsProtected = static_cast<const SwSectionNode*>(pInnermostNode)->IsInProtectSect();
        }

        // The previous version had a check to skip empty read-only sections. Those
        // shouldn't occur, so we only need to check whether our pInnermostNode is
        // inside a protected area.

        // Now, pInnermostNode is NULL or the innermost section or table node.
        if(!bIsProtected)
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
            SwNodeIndex aBegin( pCurrentPos->GetNode() );
            if( rCurrentNode.IsContentNode() &&
                (pCurrentPos->GetContentIndex() == 0))
                --aBegin;
            while( (aBegin != pInnermostNode->GetIndex()) &&
                   aBegin.GetNode().IsStartNode() )
                --aBegin;
            bool bStart = ( aBegin == pInnermostNode->GetIndex() );

            // we found an end if
            // - we're at or just before an end node
            // - there are only end nodes between the current node and
            //   pInnermostNode's end node or
            // - there are only end nodes between the last table cell merged with
            //   the current cell and pInnermostNode's end node
            SwNodeIndex aEnd( pCurrentPos->GetNode() );
            if( rCurrentNode.IsContentNode() &&
                ( pCurrentPos->GetContentIndex() ==
                  rCurrentNode.GetContentNode()->Len() ) )
            {
                ++aEnd;

                // tdf#156492 handle cells merged vertically in the bottom right corner
                if ( pInnermostNode->IsTableNode() )
                {
                    const SwNode* pTableBoxStartNode = pCurrentPos->GetNode().FindTableBoxStartNode();
                    const SwTableBox* pTableBox = pTableBoxStartNode->GetTableBox();
                    if ( pTableBox && pTableBox->getRowSpan() > 1 )
                    {
                        const SwTableNode* pTableNd = pInnermostNode->FindTableNode();
                        pTableBox = & pTableBox->FindEndOfRowSpan( pTableNd->GetTable(),
                                                                            pTableBox->getRowSpan() );
                        pTableBoxStartNode = pTableBox->GetSttNd();
                        aEnd = pTableBoxStartNode->GetIndex() + 2;
                    }
                }
            }
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
    if( pInsertNode == nullptr )
        return;

    StartAllAction();

    // adjust insert position to insert before start nodes and after end
    // nodes
    SwNodeIndex aInsertIndex( *pInsertNode,
                              SwNodeOffset(pInsertNode->IsStartNode() ? -1 : 0) );
    SwPosition aInsertPos( aInsertIndex );

    // insert a new text node, and set the cursor
    GetDoc()->getIDocumentContentOperations().AppendTextNode( aInsertPos );
    *pCursorPos = aInsertPos;

    // call AttrChangeNotify for the UI
    CallChgLnk();

    EndAllAction();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
