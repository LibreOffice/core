/*************************************************************************
 *
 *  $RCSfile: edsect.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 15:39:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _EDITSH_HXX
#include <editsh.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>       // fuer die UndoIds
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _EDIMP_HXX
#include <edimp.hxx>
#endif

#ifndef _SECTFRM_HXX
#include <sectfrm.hxx>      // SwSectionFrm
#endif
#ifndef _CNTFRM_HXX
#include <cntfrm.hxx>       // SwCntntFrm
#endif
#ifndef _TABFRM_HXX
#include <tabfrm.hxx>       // SwTabFrm
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>      // SwRootFrm
#endif


    // SS fuer Bereiche
const SwSection* SwEditShell::InsertSection( const SwSection& rNew,
                                             const SfxItemSet* pAttr )
{
    const SwSection* pRet = 0;
    if( !IsTableMode() )
    {
        StartAllAction();
        GetDoc()->StartUndo( UNDO_INSSECTION );

        FOREACHPAM_START(this)
            const SwSection* pNew = GetDoc()->Insert( *PCURCRSR,
                                                        rNew, pAttr );
            if( !pRet )
                pRet = pNew;
        FOREACHPAM_END()

        // Undo-Klammerung hier beenden
        GetDoc()->EndUndo( UNDO_INSSECTION );
        EndAllAction();
    }
    return pRet;
}


BOOL SwEditShell::IsInsRegionAvailable() const
{
    SwPaM* pCrsr;
    if( IsTableMode() || ( pCrsr = GetCrsr() )->GetNext() != pCrsr )
        return FALSE;
    if( pCrsr->HasMark() )
        return 0 != GetDoc()->IsInsRegionAvailable( *pCrsr );

    return TRUE;
}


const SwSection* SwEditShell::GetCurrSection() const
{
    if( IsTableMode() )
        return 0;

    return GetDoc()->GetCurrSection( *GetCrsr()->GetPoint() );
}

/*-----------------17.03.99 11:53-------------------
 * SwEditShell::GetAnySection liefert den fuer Spalten
 * zustaendigen Bereich, bei Fussnoten kann es nicht der
 * Bereich innerhalb der Fussnote sein.
 * --------------------------------------------------*/

const SwSection* SwEditShell::GetAnySection( BOOL bOutOfTab, const Point* pPt ) const
{
    SwFrm *pFrm;
    if ( pPt )
    {
        SwPosition aPos( *GetCrsr()->GetPoint() );
        Point aPt( *pPt );
        GetLayout()->GetCrsrOfst( &aPos, aPt );
        SwCntntNode *pNd = aPos.nNode.GetNode().GetCntntNode();
        pFrm = pNd->GetFrm( pPt );
    }
    else
        pFrm = GetCurrFrm( FALSE );

    if( bOutOfTab && pFrm )
        pFrm = pFrm->FindTabFrm();
    if( pFrm && pFrm->IsInSct() )
    {
        SwSectionFrm* pSect = pFrm->FindSctFrm();
        ASSERT( pSect, "GetAnySection: Where's my Sect?" );
        if( pSect->IsInFtn() && pSect->GetUpper()->IsInSct() )
        {
            pSect = pSect->GetUpper()->FindSctFrm();
            ASSERT( pSect, "GetAnySection: Where's my SectFrm?" );
        }
        return pSect->GetSection();
    }
    return NULL;
}

USHORT SwEditShell::GetSectionFmtCount() const
{
    return GetDoc()->GetSections().Count();
}


BOOL SwEditShell::IsAnySectionInDoc( BOOL bChkReadOnly, BOOL bChkHidden, BOOL bChkTOX ) const
{
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    USHORT nCnt = rFmts.Count();
    for( USHORT n = 0; n < nCnt; ++n )
    {
        SectionType eTmpType;
        const SwSectionFmt* pFmt = rFmts[ n ];
        if( pFmt->IsInNodesArr() &&
            (bChkTOX  ||
                (eTmpType = pFmt->GetSection()->GetType()) != TOX_CONTENT_SECTION
                && TOX_HEADER_SECTION != eTmpType ))
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

USHORT SwEditShell::GetSectionFmtPos( const SwSectionFmt& rFmt ) const
{
    SwSectionFmt* pFmt = (SwSectionFmt*)&rFmt;
    return GetDoc()->GetSections().GetPos( pFmt );
}

const SwSectionFmt& SwEditShell::GetSectionFmt( USHORT nFmt ) const
{
    return *GetDoc()->GetSections()[ nFmt ];
}


void SwEditShell::DelSectionFmt( USHORT nFmt )
{
    StartAllAction();
    GetDoc()->DelSectionFmt( GetDoc()->GetSections()[ nFmt ] );
    // rufe das AttrChangeNotify auf der UI-Seite.
    CallChgLnk();
    EndAllAction();
}


void SwEditShell::ChgSection( USHORT nSect, const SwSection& rSect,
                                const SfxItemSet* pAttr )
{
    StartAllAction();
    GetDoc()->ChgSection( nSect, rSect, pAttr );
    // rufe das AttrChangeNotify auf der UI-Seite.
    CallChgLnk();
    EndAllAction();
}

void SwEditShell::ChgSectionPasswd(
        const ::com::sun::star::uno::Sequence <sal_Int8>& rNew,
        const SwSection& rSection )
{
    GetDoc()->ChgSectionPasswd( rNew, rSection );
}


String SwEditShell::GetUniqueSectionName( const String* pChkStr ) const
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
                                    FindStartNode()->GetSectionNode())) )
                            _SetSectionAttr( *pSttSectNd->GetSection().GetFmt(),
                                            rSet );
                        aSIdx++;
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
    if(SFX_ITEM_SET == rSet.GetItemState(RES_CNTNT, FALSE))
    {
        SfxItemSet aSet(rSet);
        aSet.ClearItem(RES_CNTNT);
        GetDoc()->SetAttr( aSet, rSectFmt );
    }
    else
        GetDoc()->SetAttr( rSet, rSectFmt );

    // rufe das AttrChangeNotify auf der UI-Seite.
    CallChgLnk();
    EndAllAction();
}

// search inside the cursor selection for full selected sections.
// if any part of section in the selection return 0.
// if more than one in the selection return the count
USHORT SwEditShell::GetFullSelectedSectionCount() const
{
    USHORT nRet = 0;
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!
// what about table at start or end ?
//      There is no selection possible!
// What about only a table inside the section ?
//      There is only a table selection possible!

        SwNodeIndex aSIdx( pStt->nNode, -1 ), aEIdx( pEnd->nNode, +1 );
        if( !aSIdx.GetNode().IsSectionNode() ||
            !aEIdx.GetNode().IsEndNode() ||
            !aEIdx.GetNode().FindStartNode()->IsSectionNode() )
        {
            nRet = 0;
            break;
        }

        ++nRet;
        if( &aSIdx.GetNode() != aEIdx.GetNode().FindStartNode() )
            ++nRet;

    FOREACHPAM_END()
    return nRet;
}


/**
 * Find the suitable node for a special insert (alt-enter).
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
const SwNode* lcl_SpecialInsertNode(const SwPosition* pCurrentPos)
{
    const SwNode* pReturn = NULL;

    // the current position
    //    const SwPosition* pCurrentPos = GetCrsr()->GetPoint();
    DBG_ASSERT( pCurrentPos != NULL, "Strange, we have no position!" );
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
        DBG_ASSERT( pInnermostNode->IsTableNode() ||
                    pInnermostNode->IsSectionNode(), "wrong node found" );
        DBG_ASSERT( ( pInnermostNode->GetIndex() <= rCurrentNode.GetIndex() )&&
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
            aEnd++;
        while( (aEnd != pInnermostNode->EndOfSectionNode()->GetIndex()) &&
               aEnd.GetNode().IsEndNode() )
            aEnd++;
        bool bEnd = ( aEnd == pInnermostNode->EndOfSectionNode()->GetIndex() );

        // evalutate result: if both start + end, end is preferred
        if( bEnd )
            pReturn = pInnermostNode->EndOfSectionNode();
        else if ( bStart )
            pReturn = pInnermostNode;
        // else pReturn = NULL;
    }
    // else: pReturn = NULL


    DBG_ASSERT( ( pReturn == NULL ) || pReturn->IsStartNode() ||
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


/** check whether a node cen be special-inserted (alt-Enter), and do so. Return
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

