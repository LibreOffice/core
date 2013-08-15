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
#include <pam.hxx>
#include <docary.hxx>
#include <swundo.hxx>       // for the UndoIds
#include <section.hxx>
#include <edimp.hxx>
#include <sectfrm.hxx>      // SwSectionFrm
#include <cntfrm.hxx>       // SwCntntFrm
#include <tabfrm.hxx>       // SwTabFrm
#include <rootfrm.hxx>      // SwRootFrm


SwSection const*
SwEditShell::InsertSection(
        SwSectionData & rNewData, SfxItemSet const*const pAttr)
{
    const SwSection* pRet = 0;
    if( !IsTableMode() )
    {
        StartAllAction();
        GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_INSSECTION, NULL );

        FOREACHPAM_START(this)
            SwSection const*const pNew =
                GetDoc()->InsertSwSection( *PCURCRSR, rNewData, 0, pAttr );
            if( !pRet )
                pRet = pNew;
        FOREACHPAM_END()

        GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_INSSECTION, NULL );
        EndAllAction();
    }
    return pRet;
}

sal_Bool SwEditShell::IsInsRegionAvailable() const
{
    if( IsTableMode() )
        return sal_False;
    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )
        return sal_False;
    if( pCrsr->HasMark() )
        return 0 != GetDoc()->IsInsRegionAvailable( *pCrsr );

    return sal_True;
}

const SwSection* SwEditShell::GetCurrSection() const
{
    if( IsTableMode() )
        return 0;

    return GetDoc()->GetCurrSection( *GetCrsr()->GetPoint() );
}

/** Deliver the responsible area of the columns.
 *
 * In footnotes it may not be the area within the footnote.
 */
const SwSection* SwEditShell::GetAnySection( sal_Bool bOutOfTab, const Point* pPt ) const
{
    SwFrm *pFrm;
    if ( pPt )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
        pFrm = pNd->getLayoutFrm( GetLayout(), pPt );
    }
    else
        pFrm = GetCurrFrm( sal_False );

    if( bOutOfTab && pFrm )
        pFrm = pFrm->FindTabFrm();
    if( pFrm && pFrm->IsInSct() )
    {
        SwSectionFrm* pSect = pFrm->FindSctFrm();
        OSL_ENSURE( pSect, "GetAnySection: Where's my Sect?" );
        if( pSect->IsInFtn() && pSect->GetUpper()->IsInSct() )
        {
            pSect = pSect->GetUpper()->FindSctFrm();
            OSL_ENSURE( pSect, "GetAnySection: Where's my SectFrm?" );
        }
        return pSect->GetSection();
    }
    return NULL;
}

sal_uInt16 SwEditShell::GetSectionFmtCount() const
{
    return GetDoc()->GetSections().size();
}

sal_Bool SwEditShell::IsAnySectionInDoc( sal_Bool bChkReadOnly, sal_Bool bChkHidden, sal_Bool bChkTOX ) const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    sal_uInt16 nCnt = rFmts.size();
    sal_uInt16 n;

    for( n = 0; n < nCnt; ++n )
    {
        SectionType eTmpType;
        const SwSectionFmt* pFmt = rFmts[ n ];
        if( pFmt->IsInNodesArr() &&
            (bChkTOX  ||
                ( (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                  && TOX_HEADER_SECTION != eTmpType ) ) )
        {
            const SwSection& rSect = *rFmts[ n ]->GetSection();
            if( (!bChkReadOnly && !bChkHidden ) ||
                (bChkReadOnly && rSect.IsProtectFlag() ) ||
                (bChkHidden && rSect.IsHiddenFlag() ) )
                break;
        }
    }
    return n != nCnt;
}

sal_uInt16 SwEditShell::GetSectionFmtPos( const SwSectionFmt& rFmt ) const
{
    SwSectionFmt* pFmt = (SwSectionFmt*)&rFmt;
    return GetDoc()->GetSections().GetPos( pFmt );
}

const SwSectionFmt& SwEditShell::GetSectionFmt( sal_uInt16 nFmt ) const
{
    return *GetDoc()->GetSections()[ nFmt ];
}

void SwEditShell::DelSectionFmt( sal_uInt16 nFmt )
{
    StartAllAction();
    GetDoc()->DelSectionFmt( GetDoc()->GetSections()[ nFmt ] );
    // Call the AttrChangeNotify on the UI page.
    CallChgLnk();
    EndAllAction();
}

void SwEditShell::UpdateSection(sal_uInt16 const nSect,
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
                                    SwSectionFmt* pSectFmt )
{
    if( pSectFmt )
        _SetSectionAttr( *pSectFmt, rSet );
    else
    {
        // for all section in the selection

        FOREACHPAM_START(this)

            const SwPosition* pStt = PCURCRSR->Start(),
                            * pEnd = PCURCRSR->End();

            const SwSectionNode* pSttSectNd = pStt->nNode.GetNode().FindSectionNode(),
                               * pEndSectNd = pEnd->nNode.GetNode().FindSectionNode();

            if( pSttSectNd || pEndSectNd )
            {
                if( pSttSectNd )
                    _SetSectionAttr( *pSttSectNd->GetSection().GetFmt(),
                                    rSet );
                if( pEndSectNd && pSttSectNd != pEndSectNd )
                    _SetSectionAttr( *pEndSectNd->GetSection().GetFmt(),
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
                        if( 0 != (pSttSectNd = aSIdx.GetNode().GetSectionNode())
                            || ( aSIdx.GetNode().IsEndNode() &&
                                0 != ( pSttSectNd = aSIdx.GetNode().
                                    StartOfSectionNode()->GetSectionNode())) )
                            _SetSectionAttr( *pSttSectNd->GetSection().GetFmt(),
                                            rSet );
                        ++aSIdx;
                    }
                }
            }

        FOREACHPAM_END()
    }
}

void SwEditShell::_SetSectionAttr( SwSectionFmt& rSectFmt,
                                    const SfxItemSet& rSet )
{
    StartAllAction();
    if(SFX_ITEM_SET == rSet.GetItemState(RES_CNTNT, sal_False))
    {
        SfxItemSet aSet(rSet);
        aSet.ClearItem(RES_CNTNT);
        GetDoc()->SetAttr( aSet, rSectFmt );
    }
    else
        GetDoc()->SetAttr( rSet, rSectFmt );

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
    FOREACHPAM_START(this)

        const SwPosition* pStt = PCURCRSR->Start(),
                        * pEnd = PCURCRSR->End();
        const SwCntntNode* pCNd;
        // check the selection, if Start at Node begin and End at Node end
        if( pStt->nContent.GetIndex() ||
            ( 0 == ( pCNd = pEnd->nNode.GetNode().GetCntntNode() )) ||
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

    FOREACHPAM_END()
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
    const SwNode* pReturn = NULL;

    // the current position
    OSL_ENSURE( pCurrentPos != NULL, "Strange, we have no position!" );
    const SwNode& rCurrentNode = pCurrentPos->nNode.GetNode();

    // find innermost section or table.  At the end of this scope,
    // pInntermostNode contain the section/table before/after which we should
    // insert our empty paragraph, or it will be NULL if none is found.
    const SwNode* pInnermostNode = NULL;
    {
        const SwNode* pTableNode = rCurrentNode.FindTableNode();
        const SwNode* pSectionNode = rCurrentNode.FindSectionNode();

        // find the table/section which is close
        if( pTableNode == NULL )
            pInnermostNode = pSectionNode;
        else if ( pSectionNode == NULL )
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
    if( (pInnermostNode != NULL) && !pInnermostNode->IsProtect() )
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
        if( rCurrentNode.IsCntntNode() &&
            (pCurrentPos->nContent.GetIndex() == 0))
            aBegin--;
        while( (aBegin != pInnermostNode->GetIndex()) &&
               aBegin.GetNode().IsStartNode() )
            aBegin--;
        bool bStart = ( aBegin == pInnermostNode->GetIndex() );

        // we found an end if
        // - we're at or just before an end node
        // - there are only end nodes between the current node and
        //   pInnermostNode's end node
        SwNodeIndex aEnd( pCurrentPos->nNode );
        if( rCurrentNode.IsCntntNode() &&
            ( pCurrentPos->nContent.GetIndex() ==
              rCurrentNode.GetCntntNode()->Len() ) )
            ++aEnd;
        while( (aEnd != pInnermostNode->EndOfSectionNode()->GetIndex()) &&
               aEnd.GetNode().IsEndNode() )
            ++aEnd;
        bool bEnd = ( aEnd == pInnermostNode->EndOfSectionNode()->GetIndex() );

        // evalutate result: if both start + end, end is preferred
        if( bEnd )
            pReturn = pInnermostNode->EndOfSectionNode();
        else if ( bStart )
            pReturn = pInnermostNode;
    }

    OSL_ENSURE( ( pReturn == NULL ) || pReturn->IsStartNode() ||
                                       pReturn->IsEndNode(),
                "SpecialInsertNode failed" );
    return pReturn;
}

/** a node can be special-inserted (alt-Enter) whenever lcl_SpecialInsertNode
    finds a suitable position
*/
bool SwEditShell::CanSpecialInsert() const
{
    return NULL != lcl_SpecialInsertNode( GetCrsr()->GetPoint() );
}

/** check whether a node can be special-inserted (alt-Enter), and do so. Return
    whether insertion was possible.
 */
bool SwEditShell::DoSpecialInsert()
{
    bool bRet = false;

    // get current node
    SwPosition* pCursorPos = GetCrsr()->GetPoint();
    const SwNode* pInsertNode = lcl_SpecialInsertNode( pCursorPos );
    if( pInsertNode != NULL )
    {
        StartAllAction();

        // adjust insert position to insert before start nodes and after end
        // nodes
        SwNodeIndex aInsertIndex( *pInsertNode,
                                  pInsertNode->IsStartNode() ? -1 : 0 );
        SwPosition aInsertPos( aInsertIndex );

        // insert a new text node, and set the cursor
        bRet = GetDoc()->AppendTxtNode( aInsertPos );
        *pCursorPos = aInsertPos;

        // call AttrChangeNotify for the UI
        CallChgLnk();

        EndAllAction();
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
