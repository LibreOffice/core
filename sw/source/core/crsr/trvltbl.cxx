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
#include <vcl/svapp.hxx>
#include <editeng/protitem.hxx>
#include <crsrsh.hxx>
#include <doc.hxx>
#include <cntfrm.hxx>
#include <editsh.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <docary.hxx>
#include <frmatr.hxx>
#include <frmfmt.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <tabfrm.hxx>
#include <ndtxt.hxx>
#include <shellres.hxx>
#include <cellatr.hxx>
#include <cellfrm.hxx>
#include <rowfrm.hxx>
#include <trvltbl.hxx>
#include <IDocumentLayoutAccess.hxx>

/// set cursor into next/previous cell
bool SwCrsrShell::GoNextCell( bool bAppendLine )
{
    bool bRet = false;
    const SwTableNode* pTableNd = nullptr;

    if( IsTableMode() || nullptr != ( pTableNd = IsCrsrInTable() ))
    {
        SwCursor* pCrsr = m_pTableCrsr ? m_pTableCrsr : m_pCurCrsr;
        SwCallLink aLk( *this ); // watch Crsr-Moves
        bRet = true;

        // Check if we have to move the cursor to a covered cell before
        // proceeding:
        const SwNode* pTableBoxStartNode = pCrsr->GetNode().FindTableBoxStartNode();
        const SwTableBox* pTableBox = nullptr;

        if ( pCrsr->GetCrsrRowSpanOffset() )
        {
            pTableBox = pTableBoxStartNode->GetTableBox();
            if ( pTableBox->getRowSpan() > 1 )
            {
                if ( !pTableNd )
                    pTableNd = IsCrsrInTable();
                assert (pTableNd);
                pTableBox = & pTableBox->FindEndOfRowSpan( pTableNd->GetTable(),
                                                           (sal_uInt16)(pTableBox->getRowSpan() + pCrsr->GetCrsrRowSpanOffset() ) );
                pTableBoxStartNode = pTableBox->GetSttNd();
            }
        }

        SwNodeIndex  aCellStt( *pTableBoxStartNode->EndOfSectionNode(), 1 );

        // if there is another StartNode after the EndNode of a cell then
        // there is another cell
        if( !aCellStt.GetNode().IsStartNode() )
        {
            if( pCrsr->HasMark() || !bAppendLine )
                bRet = false;
            else if (pTableNd)
            {
                // if there is no list anymore then create new one
                if ( !pTableBox )
                    pTableBox = pTableNd->GetTable().GetTableBox(
                                    pCrsr->GetPoint()->nNode.GetNode().
                                    StartOfSectionIndex() );

                OSL_ENSURE( pTableBox, "Box is not in this table" );
                SwSelBoxes aBoxes;

                // the document might change; w/o Action views would not be notified
                static_cast<SwEditShell*>(this)->StartAllAction();
                bRet = mpDoc->InsertRow( SwTable::SelLineFromBox( pTableBox, aBoxes, false ));
                static_cast<SwEditShell*>(this)->EndAllAction();
            }
        }
        if( bRet && ( bRet = pCrsr->GoNextCell() ) )
            UpdateCrsr();
    }
    return bRet;
}

bool SwCrsrShell::GoPrevCell()
{
    bool bRet = false;
    if( IsTableMode() || IsCrsrInTable() )
    {
        SwCursor* pCrsr = m_pTableCrsr ? m_pTableCrsr : m_pCurCrsr;
        SwCallLink aLk( *this ); // watch Crsr-Moves
        bRet = pCrsr->GoPrevCell();
        if( bRet )
            UpdateCrsr(); // update current cursor
    }
    return bRet;
}

static const SwFrm* lcl_FindMostUpperCellFrm( const SwFrm* pFrm )
{
    while ( pFrm &&
            ( !pFrm->IsCellFrm() ||
              !pFrm->GetUpper()->GetUpper()->IsTabFrm() ||
               pFrm->GetUpper()->GetUpper()->GetUpper()->IsInTab() ) )
    {
        pFrm = pFrm->GetUpper();
    }
    return pFrm;
}

bool SwCrsrShell::_SelTableRowOrCol( bool bRow, bool bRowSimple )
{
    // check if the current cursor's SPoint/Mark are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return false;

    const SwTabFrm* pTabFrm = pFrm->FindTabFrm();
    const SwTabFrm* pMasterTabFrm = pTabFrm->IsFollow() ? pTabFrm->FindMaster( true ) : pTabFrm;
    const SwTable* pTable = pTabFrm->GetTable();

    SET_CURR_SHELL( this );

    const SwTableBox* pStt = nullptr;
    const SwTableBox* pEnd = nullptr;

    // search box based on layout
    SwSelBoxes aBoxes;
    SwTableSearchType eType = bRow ? nsSwTableSearchType::TBLSEARCH_ROW : nsSwTableSearchType::TBLSEARCH_COL;
    const bool bCheckProtected = !IsReadOnlyAvailable();

    if( bCheckProtected )
        eType = (SwTableSearchType)(eType | nsSwTableSearchType::TBLSEARCH_PROTECT);

    if ( !bRowSimple )
    {
        GetTableSel( *this, aBoxes, eType );

        if( aBoxes.empty() )
            return false;

        pStt = aBoxes[0];
        pEnd = aBoxes.back();
    }
    // #i32329# Enhanced table selection
    else if ( pTable->IsNewModel() )
    {
        const SwShellCrsr *pCrsr = _GetCrsr();
        SwTable::SearchType eSearchType = bRow ? SwTable::SEARCH_ROW : SwTable::SEARCH_COL;
        pTable->CreateSelection( *pCrsr, aBoxes, eSearchType, bCheckProtected );
        if( aBoxes.empty() )
            return false;

        pStt = aBoxes[0];
        pEnd = aBoxes.back();
    }
    else
    {
        const SwShellCrsr *pCrsr = _GetCrsr();
        const SwFrm* pStartFrm = pFrm;
        const SwContentNode *pCNd = pCrsr->GetContentNode( false );
        const SwFrm* pEndFrm   = pCNd ? pCNd->getLayoutFrm( GetLayout(), &pCrsr->GetMkPos() ) : nullptr;

        if ( bRow )
        {
            pStartFrm = lcl_FindMostUpperCellFrm( pStartFrm );
            pEndFrm   = lcl_FindMostUpperCellFrm( pEndFrm   );
        }

        if ( !pStartFrm || !pEndFrm )
            return false;

        const bool bVert = pFrm->ImplFindTabFrm()->IsVertical();

        // If we select upwards it is sufficient to set pStt and pEnd
        // to the first resp. last box of the selection obtained from
        // GetTableSel. However, selecting downwards requires the frames
        // located at the corners of the selection. This does not work
        // for column selections in vertical tables:
        const bool bSelectUp = ( bVert && !bRow ) ||
                                *pCrsr->GetPoint() <= *pCrsr->GetMark();
        SwCellFrms aCells;
        GetTableSel( static_cast<const SwCellFrm*>(pStartFrm),
                   static_cast<const SwCellFrm*>(pEndFrm),
                   aBoxes, bSelectUp ? nullptr : &aCells, eType );

        if( aBoxes.empty() || ( !bSelectUp && 4 != aCells.size() ) )
            return false;

        if ( bSelectUp )
        {
            pStt = aBoxes[0];
            pEnd = aBoxes.back();
        }
        else
        {
            // will become point of table cursor
            pStt = aCells[bVert ? 0 : (bRow ? 2 : 1)]->GetTabBox();
            // will become mark of table cursor
            pEnd = aCells[bVert ? 3 : (bRow ? 1 : 2)]->GetTabBox();
        }
    }

    // if no table cursor exists, create one
    if( !m_pTableCrsr )
    {
        m_pTableCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
    }

    m_pTableCrsr->DeleteMark();

    // set start and end of a column
    m_pTableCrsr->GetPoint()->nNode = *pEnd->GetSttNd();
    m_pTableCrsr->Move( fnMoveForward, fnGoContent );
    m_pTableCrsr->SetMark();
    m_pTableCrsr->GetPoint()->nNode = *pStt->GetSttNd()->EndOfSectionNode();
    m_pTableCrsr->Move( fnMoveBackward, fnGoContent );

    // set PtPos 'close' to the reference table, otherwise we might get problems
    // with the repeated headlines check in UpdateCrsr():
    if ( !bRow )
        m_pTableCrsr->GetPtPos() = pMasterTabFrm->IsVertical()
                                   ? pMasterTabFrm->Frm().TopRight()
                                   : pMasterTabFrm->Frm().TopLeft();

    UpdateCrsr();
    return true;
}

bool SwCrsrShell::SelTable()
{
    // check if the current cursor's SPoint/Mark are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return false;

    const SwTabFrm *pTableFrm = pFrm->ImplFindTabFrm();
    const SwTabFrm* pMasterTabFrm = pTableFrm->IsFollow() ? pTableFrm->FindMaster( true ) : pTableFrm;
    const SwTableNode* pTableNd = pTableFrm->GetTable()->GetTableNode();

    SET_CURR_SHELL( this );

    if( !m_pTableCrsr )
    {
        m_pTableCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
    }

    m_pTableCrsr->DeleteMark();
    m_pTableCrsr->GetPoint()->nNode = *pTableNd;
    m_pTableCrsr->Move( fnMoveForward, fnGoContent );
    m_pTableCrsr->SetMark();
    // set MkPos 'close' to the master table, otherwise we might get problems
    // with the repeated headlines check in UpdateCrsr():
    m_pTableCrsr->GetMkPos() = pMasterTabFrm->IsVertical() ? pMasterTabFrm->Frm().TopRight() : pMasterTabFrm->Frm().TopLeft();
    m_pTableCrsr->GetPoint()->nNode = *pTableNd->EndOfSectionNode();
    m_pTableCrsr->Move( fnMoveBackward, fnGoContent );
    UpdateCrsr();
    return true;
}

bool SwCrsrShell::SelTableBox()
{
    // if we're in a table, create a table cursor, and select the cell
    // that the current cursor's point resides in

    // search for start node of our table box. If not found, exit really
    const SwStartNode* pStartNode =
        m_pCurCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();

#if OSL_DEBUG_LEVEL > 0
    // the old code checks whether we're in a table by asking the
    // frame. This should yield the same result as searching for the
    // table box start node, right?
    SwFrm *pFrm = GetCurrFrm();
    OSL_ENSURE( !pFrm->IsInTab() == !(pStartNode != nullptr),
                "Schroedinger's table: We're in a box, and also we aren't." );
#endif
    if( pStartNode == nullptr )
        return false;

    SET_CURR_SHELL( this );

    // create a table cursor, if there isn't one already
    if( !m_pTableCrsr )
    {
        m_pTableCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
    }

    // select the complete box with our shiny new m_pTableCrsr
    // 1. delete mark, and move point to first content node in box
    m_pTableCrsr->DeleteMark();
    *(m_pTableCrsr->GetPoint()) = SwPosition( *pStartNode );
    m_pTableCrsr->Move( fnMoveForward, fnGoNode );

    // 2. set mark, and move point to last content node in box
    m_pTableCrsr->SetMark();
    *(m_pTableCrsr->GetPoint()) = SwPosition( *(pStartNode->EndOfSectionNode()) );
    m_pTableCrsr->Move( fnMoveBackward, fnGoNode );

    // 3. exchange
    m_pTableCrsr->Exchange();

    // with some luck, UpdateCrsr() will now update everything that
    // needs updating
    UpdateCrsr();

    return true;
}

// TODO: provide documentation
/** get the next non-protected cell inside a table

    @param[in,out] rIdx is on a table node
    @param bInReadOnly  ???

    @return <false> if no suitable cell could be found, otherwise <rIdx> points
            to content in a suitable cell and <true> is returned.
*/
static bool lcl_FindNextCell( SwNodeIndex& rIdx, bool bInReadOnly )
{
    // check protected cells
    SwNodeIndex aTmp( rIdx, 2 ); // TableNode + StartNode

    // the resulting cell should be in that table:
    const SwTableNode* pTableNd = rIdx.GetNode().GetTableNode();

    if ( !pTableNd )
    {
        OSL_FAIL( "lcl_FindNextCell not celled with table start node!" );
        return false;
    }

    const SwNode* pTableEndNode = pTableNd->EndOfSectionNode();

    SwNodes& rNds = aTmp.GetNode().GetNodes();
    SwContentNode* pCNd = aTmp.GetNode().GetContentNode();

    // no content node => go to next content node
    if( !pCNd )
        pCNd = rNds.GoNext( &aTmp );

    // robust
    if ( !pCNd )
        return false;

    SwContentFrm* pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

    if ( nullptr == pFrm || pCNd->FindTableNode() != pTableNd ||
        (!bInReadOnly && pFrm->IsProtected() ) )
    {
        // we are not located inside a 'valid' cell. We have to continue searching...

        // skip behind current section. This might be the end of the table cell
        // or behind a inner section or...
        aTmp.Assign( *pCNd->EndOfSectionNode(), 1 );

        // loop to find a suitable cell...
        for( ;; )
        {
            SwNode* pNd = &aTmp.GetNode();

            // we break this loop if we reached the end of the table.
            // to make this code even more robust, we also break if we are
            // already behind the table end node:
            if( pNd == pTableEndNode || /*robust: */ pNd->GetIndex() > pTableEndNode->GetIndex() )
                return false;

            // ok, get the next content node:
            pCNd = aTmp.GetNode().GetContentNode();
            if( nullptr == pCNd )
                pCNd = rNds.GoNext( &aTmp );

            // robust:
            if ( !pCNd )
                return false;

            // check if we have found a suitable table cell:
            pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

            if ( nullptr != pFrm && pCNd->FindTableNode() == pTableNd &&
                (bInReadOnly || !pFrm->IsProtected() ) )
            {
                // finally, we have found a suitable table cell => set index and return
                rIdx = *pCNd;
                return true;
            }

            // continue behind the current section:
            aTmp.Assign( *pCNd->EndOfSectionNode(), +1 );
        }
    }
    rIdx = *pCNd;
    return true;
}

/// see lcl_FindNextCell()
static bool lcl_FindPrevCell( SwNodeIndex& rIdx, bool bInReadOnly  )
{
    SwNodeIndex aTmp( rIdx, -2 ); // TableNode + EndNode

    const SwNode* pTableEndNode = &rIdx.GetNode();
    const SwTableNode* pTableNd = pTableEndNode->StartOfSectionNode()->GetTableNode();

    if ( !pTableNd )
    {
        OSL_FAIL( "lcl_FindPrevCell not celled with table start node!" );
        return false;
    }

    SwContentNode* pCNd = aTmp.GetNode().GetContentNode();

    if( !pCNd )
        pCNd = SwNodes::GoPrevious( &aTmp );

    if ( !pCNd )
        return false;

    SwContentFrm* pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

    if( nullptr == pFrm || pCNd->FindTableNode() != pTableNd ||
        (!bInReadOnly && pFrm->IsProtected() ))
    {
        // skip before current section
        aTmp.Assign( *pCNd->StartOfSectionNode(), -1 );
        for( ;; )
        {
            SwNode* pNd = &aTmp.GetNode();

            if( pNd == pTableNd || pNd->GetIndex() < pTableNd->GetIndex() )
                return false;

            pCNd = aTmp.GetNode().GetContentNode();
            if( nullptr == pCNd )
                pCNd = SwNodes::GoPrevious( &aTmp );

            if ( !pCNd )
                return false;

            pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

            if( nullptr != pFrm && pCNd->FindTableNode() == pTableNd &&
                (bInReadOnly || !pFrm->IsProtected() ) )
            {
                rIdx = *pCNd;
                return true; // ok, not protected
            }
            aTmp.Assign( *pCNd->StartOfSectionNode(), - 1 );
        }
    }
    rIdx = *pCNd;
    return true;
}

bool GotoPrevTable( SwPaM& rCurCrsr, SwPosTable fnPosTable,
                    bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );

    SwTableNode* pTableNd = aIdx.GetNode().FindTableNode();
    if( pTableNd )
    {
        // #i26532#: If we are inside a table, we may not go backward to the
        // table start node, because we would miss any tables inside this table.
        SwTableNode* pInnerTableNd = nullptr;
        SwNodeIndex aTmpIdx( aIdx );
        while( aTmpIdx.GetIndex() &&
                nullptr == ( pInnerTableNd = aTmpIdx.GetNode().StartOfSectionNode()->GetTableNode()) )
            --aTmpIdx;

        if( pInnerTableNd == pTableNd )
            aIdx.Assign( *pTableNd, - 1 );
    }

    do {
        while( aIdx.GetIndex() &&
            nullptr == ( pTableNd = aIdx.GetNode().StartOfSectionNode()->GetTableNode()) )
            --aIdx;

        if( pTableNd ) // any further table node?
        {
            if( fnPosTable == fnMoveForward ) // at the beginning?
            {
                aIdx = *aIdx.GetNode().StartOfSectionNode();
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTableNd, -1 );
                    continue;
                }
            }
            else
            {
                // check protected cells
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTableNd, -1 );
                    continue;
                }
            }

            SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
            if ( pTextNode )
            {
                rCurCrsr.GetPoint()->nNode = *pTextNode;
                rCurCrsr.GetPoint()->nContent.Assign( pTextNode, fnPosTable == fnMoveBackward ?
                                                      pTextNode->Len() :
                                                      0 );
            }
            return true;
        }
    } while( pTableNd );

    return false;
}

bool GotoNextTable( SwPaM& rCurCrsr, SwPosTable fnPosTable,
                    bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwTableNode* pTableNd = aIdx.GetNode().FindTableNode();

    if( pTableNd )
        aIdx.Assign( *pTableNd->EndOfSectionNode(), 1 );

    sal_uLong nLastNd = rCurCrsr.GetDoc()->GetNodes().Count() - 1;
    do {
        while( aIdx.GetIndex() < nLastNd &&
                nullptr == ( pTableNd = aIdx.GetNode().GetTableNode()) )
            ++aIdx;
        if( pTableNd ) // any further table node?
        {
            if( fnPosTable == fnMoveForward ) // at the beginning?
            {
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTableNd->EndOfSectionNode(), + 1 );
                    continue;
                }
            }
            else
            {
                aIdx = *aIdx.GetNode().EndOfSectionNode();
                // check protected cells
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTableNd->EndOfSectionNode(), + 1 );
                    continue;
                }
            }

            SwTextNode* pTextNode = aIdx.GetNode().GetTextNode();
            if ( pTextNode )
            {
                rCurCrsr.GetPoint()->nNode = *pTextNode;
                rCurCrsr.GetPoint()->nContent.Assign( pTextNode, fnPosTable == fnMoveBackward ?
                                                      pTextNode->Len() :
                                                      0 );
            }
            return true;
        }
    } while( pTableNd );

    return false;
}

bool GotoCurrTable( SwPaM& rCurCrsr, SwPosTable fnPosTable,
                    bool bInReadOnly )
{
    SwTableNode* pTableNd = rCurCrsr.GetPoint()->nNode.GetNode().FindTableNode();
    if( !pTableNd )
        return false;

    SwTextNode* pTextNode = nullptr;
    if( fnPosTable == fnMoveBackward ) // to the end of the table
    {
        SwNodeIndex aIdx( *pTableNd->EndOfSectionNode() );
        if( !lcl_FindPrevCell( aIdx, bInReadOnly ))
            return false;
        pTextNode = aIdx.GetNode().GetTextNode();
    }
    else
    {
        SwNodeIndex aIdx( *pTableNd );
        if( !lcl_FindNextCell( aIdx, bInReadOnly ))
            return false;
        pTextNode = aIdx.GetNode().GetTextNode();
    }

    if ( pTextNode )
    {
        rCurCrsr.GetPoint()->nNode = *pTextNode;
        rCurCrsr.GetPoint()->nContent.Assign( pTextNode, fnPosTable == fnMoveBackward ?
                                                        pTextNode->Len() :
                                                        0 );
    }

    return true;
}

bool SwCursor::MoveTable( SwWhichTable fnWhichTable, SwPosTable fnPosTable )
{
    bool bRet = false;
    SwTableCursor* m_pTableCrsr = dynamic_cast<SwTableCursor*>(this);

    if( m_pTableCrsr || !HasMark() )
    {
        SwCrsrSaveState aSaveState( *this );
        bRet = (*fnWhichTable)( *this, fnPosTable, IsReadOnlyAvailable() ) &&
                !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                           nsSwCursorSelOverFlags::SELOVER_TOGGLE );
    }
    return bRet;
}

bool SwCrsrShell::MoveTable( SwWhichTable fnWhichTable, SwPosTable fnPosTable )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed

    SwShellCrsr* pCrsr = m_pTableCrsr ? m_pTableCrsr : m_pCurCrsr;
    bool bCheckPos;
    bool bRet;
    sal_uLong nPtNd = 0;
    sal_Int32 nPtCnt = 0;

    if ( !m_pTableCrsr && m_pCurCrsr->HasMark() )
    {
        // switch to table mode
        m_pTableCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
        m_pTableCrsr->SetMark();
        pCrsr = m_pTableCrsr;
        bCheckPos = false;
    }
    else
    {
        bCheckPos = true;
        nPtNd = pCrsr->GetPoint()->nNode.GetIndex();
        nPtCnt = pCrsr->GetPoint()->nContent.GetIndex();
    }

    bRet = pCrsr->MoveTable( fnWhichTable, fnPosTable );

    if( bRet )
    {
        // #i45028# - set "top" position for repeated headline rows
        pCrsr->GetPtPos() = Point();

        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);

        if( bCheckPos &&
            pCrsr->GetPoint()->nNode.GetIndex() == nPtNd &&
            pCrsr->GetPoint()->nContent.GetIndex() == nPtCnt )
            bRet = false;
    }
    return bRet;
}

bool SwCrsrShell::IsTableComplexForChart()
{
    bool bRet = false;

    // Here we may trigger table formatting so we better do that inside an action
    StartAction();
    const SwTableNode* pTNd = m_pCurCrsr->GetPoint()->nNode.GetNode().FindTableNode();
    if( pTNd )
    {
        // in a table; check if table or section is balanced
        OUString sSel;
        if( m_pTableCrsr )
            sSel = GetBoxNms();
        bRet = pTNd->GetTable().IsTableComplexForChart( sSel );
    }
    EndAction();

    return bRet;
}

OUString SwCrsrShell::GetBoxNms() const
{
    OUString sNm;
    const SwPosition* pPos;
    SwFrm* pFrm;

    if( IsTableMode() )
    {
        SwContentNode *pCNd = m_pTableCrsr->Start()->nNode.GetNode().GetContentNode();
        pFrm = pCNd ? pCNd->getLayoutFrm( GetLayout() ) : nullptr;
        if( !pFrm )
            return sNm;

        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );

        OSL_ENSURE( pFrm, "no frame for this box" );

        if( !pFrm )
            return sNm;

        sNm = static_cast<SwCellFrm*>(pFrm)->GetTabBox()->GetName();
        sNm += ":";
        pPos = m_pTableCrsr->End();
    }
    else
    {
        const SwTableNode* pTableNd = IsCrsrInTable();
        if( !pTableNd )
            return sNm;
        pPos = GetCrsr()->GetPoint();
    }

    SwContentNode* pCNd = pPos->nNode.GetNode().GetContentNode();
    pFrm = pCNd ? pCNd->getLayoutFrm( GetLayout() ) : nullptr;

    if( pFrm )
    {
        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );

        if( pFrm )
            sNm += static_cast<SwCellFrm*>(pFrm)->GetTabBox()->GetName();
    }
    return sNm;
}

bool SwCrsrShell::GotoTable( const OUString& rName )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves
    bool bRet = !m_pTableCrsr && m_pCurCrsr->GotoTable( rName );
    if( bRet )
    {
        m_pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    }
    return bRet;
}

bool SwCrsrShell::CheckTableBoxContent( const SwPosition* pPos )
{
    if( !m_pBoxIdx || !m_pBoxPtr || IsSelTableCells() || !IsAutoUpdateCells() )
        return false;

    // check if box content is consistent with given box format, reset if not
    SwTableBox* pChkBox = nullptr;
    SwStartNode* pSttNd = nullptr;
    if( !pPos )
    {
        // get stored position
        if( m_pBoxIdx && m_pBoxPtr &&
            nullptr != ( pSttNd = m_pBoxIdx->GetNode().GetStartNode() ) &&
            SwTableBoxStartNode == pSttNd->GetStartNodeType() &&
            m_pBoxPtr == pSttNd->FindTableNode()->GetTable().
                        GetTableBox( m_pBoxIdx->GetIndex() ) )
            pChkBox = m_pBoxPtr;
    }
    else if( nullptr != ( pSttNd = pPos->nNode.GetNode().
                                FindSttNodeByType( SwTableBoxStartNode )) )
    {
        pChkBox = pSttNd->FindTableNode()->GetTable().GetTableBox( pSttNd->GetIndex() );
    }

    // box has more than one paragraph
    if( pChkBox && pSttNd->GetIndex() + 2 != pSttNd->EndOfSectionIndex() )
        pChkBox = nullptr;

    // destroy pointer before next action starts
    if( !pPos && !pChkBox )
        ClearTableBoxContent();

    // cursor not anymore in this section?
    if( pChkBox && !pPos &&
        ( m_pCurCrsr->HasMark() || m_pCurCrsr->GetNext() != m_pCurCrsr ||
          pSttNd->GetIndex() + 1 == m_pCurCrsr->GetPoint()->nNode.GetIndex() ))
        pChkBox = nullptr;

    // Did the content of a box change at all? This is important if e.g. Undo
    // could not restore the content properly.
    if( pChkBox )
    {
        const SwTextNode* pNd = GetDoc()->GetNodes()[
                                    pSttNd->GetIndex() + 1 ]->GetTextNode();
        if( !pNd ||
            ( pNd->GetText() == SwViewShell::GetShellRes()->aCalc_Error &&
              SfxItemState::SET == pChkBox->GetFrameFormat()->
                            GetItemState( RES_BOXATR_FORMULA )) )
            pChkBox = nullptr;
    }

    if( pChkBox )
    {
        // destroy pointer before next action starts
        ClearTableBoxContent();
        StartAction();
        GetDoc()->ChkBoxNumFormat( *pChkBox, true );
        EndAction();
    }

    return nullptr != pChkBox;
}

void SwCrsrShell::SaveTableBoxContent( const SwPosition* pPos )
{
    if( IsSelTableCells() || !IsAutoUpdateCells() )
        return ;

    if( !pPos )
        pPos = m_pCurCrsr->GetPoint();

    SwStartNode* pSttNd = pPos->nNode.GetNode().FindSttNodeByType( SwTableBoxStartNode );

    bool bCheckBox = false;
    if( pSttNd && m_pBoxIdx )
    {
        if( pSttNd == &m_pBoxIdx->GetNode() )
            pSttNd = nullptr;
        else
            bCheckBox = true;
    }
    else
        bCheckBox = nullptr != m_pBoxIdx;

    if( bCheckBox )
    {
        // check m_pBoxIdx
        SwPosition aPos( *m_pBoxIdx );
        CheckTableBoxContent( &aPos );
    }

    if( pSttNd )
    {
        m_pBoxPtr = pSttNd->FindTableNode()->GetTable().GetTableBox( pSttNd->GetIndex() );

        if( m_pBoxIdx )
            *m_pBoxIdx = *pSttNd;
        else
            m_pBoxIdx = new SwNodeIndex( *pSttNd );
    }
}

void SwCrsrShell::ClearTableBoxContent()
{
    delete m_pBoxIdx, m_pBoxIdx = nullptr;
    m_pBoxPtr = nullptr;
}

bool SwCrsrShell::EndAllTableBoxEdit()
{
    bool bRet = false;
    for(SwViewShell& rSh : GetRingContainer())
    {
        if( dynamic_cast<const SwCrsrShell *>(&rSh) != nullptr )
            bRet |= static_cast<SwCrsrShell*>(&rSh)->CheckTableBoxContent(
                        static_cast<SwCrsrShell*>(&rSh)->m_pCurCrsr->GetPoint() );

    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
