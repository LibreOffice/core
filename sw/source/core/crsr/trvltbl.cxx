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

/// set cursor into next/previous cell
sal_Bool SwCrsrShell::GoNextCell( sal_Bool bAppendLine )
{
    sal_Bool bRet = sal_False;
    const SwTableNode* pTblNd = 0;

    if( IsTableMode() || 0 != ( pTblNd = IsCrsrInTbl() ))
    {
        SwCursor* pCrsr = m_pTblCrsr ? m_pTblCrsr : m_pCurCrsr;
        SwCallLink aLk( *this ); // watch Crsr-Moves
        bRet = sal_True;

        // Check if we have to move the cursor to a covered cell before
        // proceeding:
        const SwNode* pTableBoxStartNode = pCrsr->GetNode()->FindTableBoxStartNode();
        const SwTableBox* pTableBox = 0;

        if ( pCrsr->GetCrsrRowSpanOffset() )
        {
            pTableBox = pTableBoxStartNode->GetTblBox();
            if ( pTableBox->getRowSpan() > 1 )
            {
                if ( !pTblNd )
                    pTblNd = IsCrsrInTbl();
                pTableBox = & pTableBox->FindEndOfRowSpan( pTblNd->GetTable(),
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
                bRet = sal_False;
            else
            {
                // if there is no list anymore then create new one
                if ( !pTableBox )
                    pTableBox = pTblNd->GetTable().GetTblBox(
                                    pCrsr->GetPoint()->nNode.GetNode().
                                    StartOfSectionIndex() );

                OSL_ENSURE( pTableBox, "Box is not in this table" );
                SwSelBoxes aBoxes;

                // the document might change; w/o Action views would not be notified
                ((SwEditShell*)this)->StartAllAction();
                bRet = mpDoc->InsertRow( pTblNd->GetTable().
                                    SelLineFromBox( pTableBox, aBoxes, false ));
                ((SwEditShell*)this)->EndAllAction();
            }
        }
        if( bRet && 0 != ( bRet = pCrsr->GoNextCell() ))
            UpdateCrsr();
    }
    return bRet;
}

sal_Bool SwCrsrShell::GoPrevCell()
{
    sal_Bool bRet = sal_False;
    if( IsTableMode() || IsCrsrInTbl() )
    {
        SwCursor* pCrsr = m_pTblCrsr ? m_pTblCrsr : m_pCurCrsr;
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

bool SwCrsrShell::_SelTblRowOrCol( bool bRow, bool bRowSimple )
{
    // check if the current cursor's SPoint/Mark are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return false;

    const SwTabFrm* pTabFrm = pFrm->FindTabFrm();
    const SwTabFrm* pMasterTabFrm = pTabFrm->IsFollow() ? pTabFrm->FindMaster( true ) : pTabFrm;
    const SwTable* pTable = pTabFrm->GetTable();

    SET_CURR_SHELL( this );

    const SwTableBox* pStt = 0;
    const SwTableBox* pEnd = 0;

    // search box based on layout
    SwSelBoxes aBoxes;
    SwTblSearchType eType = bRow ? nsSwTblSearchType::TBLSEARCH_ROW : nsSwTblSearchType::TBLSEARCH_COL;
    const bool bCheckProtected = !IsReadOnlyAvailable();

    if( bCheckProtected )
        eType = (SwTblSearchType)(eType | nsSwTblSearchType::TBLSEARCH_PROTECT);

    if ( !bRowSimple )
    {
        GetTblSel( *this, aBoxes, eType );

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
        const SwCntntNode *pCNd = pCrsr->GetCntntNode( sal_False );
        const SwFrm* pEndFrm   = pCNd ? pCNd->getLayoutFrm( GetLayout(), &pCrsr->GetMkPos() ) : 0;

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
        // GetTblSel. However, selecting downwards requires the frames
        // located at the corners of the selection. This does not work
        // for column selections in vertical tables:
        const bool bSelectUp = ( bVert && !bRow ) ||
                                *pCrsr->GetPoint() <= *pCrsr->GetMark();
        SwCellFrms aCells;
        GetTblSel( static_cast<const SwCellFrm*>(pStartFrm),
                   static_cast<const SwCellFrm*>(pEndFrm),
                   aBoxes, bSelectUp ? 0 : &aCells, eType );

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
            pStt = aCells[ bVert ? (bRow ? 0 : 3) : (bRow ? 2 : 1) ]->GetTabBox();
            // will become mark of table cursor
            pEnd = aCells[ bVert ? (bRow ? 3 : 0) : (bRow ? 1 : 2) ]->GetTabBox();
        }
    }

    // if no table cursor exists, create one
    if( !m_pTblCrsr )
    {
        m_pTblCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
    }

    m_pTblCrsr->DeleteMark();

    // set start and end of a column
    m_pTblCrsr->GetPoint()->nNode = *pEnd->GetSttNd();
    m_pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    m_pTblCrsr->SetMark();
    m_pTblCrsr->GetPoint()->nNode = *pStt->GetSttNd()->EndOfSectionNode();
    m_pTblCrsr->Move( fnMoveBackward, fnGoCntnt );

    // set PtPos 'close' to the reference table, otherwise we might get problems
    // with the repeated headlines check in UpdateCrsr():
    if ( !bRow )
        m_pTblCrsr->GetPtPos() = pMasterTabFrm->IsVertical()
                                   ? pMasterTabFrm->Frm().TopRight()
                                   : pMasterTabFrm->Frm().TopLeft();

    UpdateCrsr();
    return true;
}

sal_Bool SwCrsrShell::SelTbl()
{
    // check if the current cursor's SPoint/Mark are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm->IsInTab() )
        return sal_False;

    const SwTabFrm *pTblFrm = pFrm->ImplFindTabFrm();
    const SwTabFrm* pMasterTabFrm = pTblFrm->IsFollow() ? pTblFrm->FindMaster( true ) : pTblFrm;
    const SwTableNode* pTblNd = pTblFrm->GetTable()->GetTableNode();

    SET_CURR_SHELL( this );

    if( !m_pTblCrsr )
    {
        m_pTblCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
    }

    m_pTblCrsr->DeleteMark();
    m_pTblCrsr->GetPoint()->nNode = *pTblNd;
    m_pTblCrsr->Move( fnMoveForward, fnGoCntnt );
    m_pTblCrsr->SetMark();
    // set MkPos 'close' to the master table, otherwise we might get problems
    // with the repeated headlines check in UpdateCrsr():
    m_pTblCrsr->GetMkPos() = pMasterTabFrm->IsVertical() ? pMasterTabFrm->Frm().TopRight() : pMasterTabFrm->Frm().TopLeft();
    m_pTblCrsr->GetPoint()->nNode = *pTblNd->EndOfSectionNode();
    m_pTblCrsr->Move( fnMoveBackward, fnGoCntnt );
    UpdateCrsr();
    return sal_True;
}

sal_Bool SwCrsrShell::SelTblBox()
{
    // if we're in a table, create a table cursor, and select the cell
    // that the current cursor's point resides in

    // search for start node of our table box. If not found, exit realy
    const SwStartNode* pStartNode =
        m_pCurCrsr->GetPoint()->nNode.GetNode().FindTableBoxStartNode();

#if OSL_DEBUG_LEVEL > 0
    // the old code checks whether we're in a table by asking the
    // frame. This should yield the same result as searching for the
    // table box start node, right?
    SwFrm *pFrm = GetCurrFrm();
    OSL_ENSURE( !pFrm->IsInTab() == !(pStartNode != NULL),
                "Schroedinger's table: We're in a box, and also we aren't." );
#endif

    if( pStartNode == NULL )
        return sal_False;

    SET_CURR_SHELL( this );

    // create a table cursor, if there isn't one already
    if( !m_pTblCrsr )
    {
        m_pTblCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
    }

    // select the complete box with our shiny new m_pTblCrsr
    // 1. delete mark, and move point to first content node in box

    m_pTblCrsr->DeleteMark();
    *(m_pTblCrsr->GetPoint()) = SwPosition( *pStartNode );
    m_pTblCrsr->Move( fnMoveForward, fnGoNode );

    // 2. set mark, and move point to last content node in box
    m_pTblCrsr->SetMark();
    *(m_pTblCrsr->GetPoint()) = SwPosition( *(pStartNode->EndOfSectionNode()) );
    m_pTblCrsr->Move( fnMoveBackward, fnGoNode );

    // 3. exchange
    m_pTblCrsr->Exchange();

    // with some luck, UpdateCrsr() will now update everything that
    // needs updateing
    UpdateCrsr();

    return sal_True;
}

// TODO: provide documentation
/** get the next non-protected cell inside a table

    @param[in,out] rIdx is on a table node
    @param bInReadOnly  ???

    @return <false> if no suitable cell could be found, otherwise <rIdx> points
            to content in a suitable cell and <true> is returned.
*/
static bool lcl_FindNextCell( SwNodeIndex& rIdx, sal_Bool bInReadOnly )
{
    // check protected cells
    SwNodeIndex aTmp( rIdx, 2 ); // TableNode + StartNode

    // the resulting cell should be in that table:
    const SwTableNode* pTblNd = rIdx.GetNode().GetTableNode();

    if ( !pTblNd )
    {
        OSL_FAIL( "lcl_FindNextCell not celled with table start node!" );
        return false;
    }

    const SwNode* pTableEndNode = pTblNd->EndOfSectionNode();

    SwNodes& rNds = aTmp.GetNode().GetNodes();
    SwCntntNode* pCNd = aTmp.GetNode().GetCntntNode();

    // no content node => go to next content node
    if( !pCNd )
        pCNd = rNds.GoNext( &aTmp );

    // robust
    if ( !pCNd )
        return false;

    SwCntntFrm* pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() );

    if ( 0 == pFrm || pCNd->FindTableNode() != pTblNd ||
        (!bInReadOnly && pFrm->IsProtected() ) )
    {
        // we are not located inside a 'valid' cell. We have to continue searching...

        // skip behind current section. This might be the end of the table cell
        // or behind a inner section or or or...
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
            pCNd = aTmp.GetNode().GetCntntNode();
            if( 0 == pCNd )
                pCNd = rNds.GoNext( &aTmp );

            // robust:
            if ( !pCNd )
                return false;

            // check if we have found a suitable table cell:
            pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() );

            if ( 0 != pFrm && pCNd->FindTableNode() == pTblNd &&
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
static bool lcl_FindPrevCell( SwNodeIndex& rIdx, sal_Bool bInReadOnly  )
{
    SwNodeIndex aTmp( rIdx, -2 ); // TableNode + EndNode

    const SwNode* pTableEndNode = &rIdx.GetNode();
    const SwTableNode* pTblNd = pTableEndNode->StartOfSectionNode()->GetTableNode();

    if ( !pTblNd )
    {
        OSL_FAIL( "lcl_FindPrevCell not celled with table start node!" );
        return false;
    }

    SwNodes& rNds = aTmp.GetNode().GetNodes();
    SwCntntNode* pCNd = aTmp.GetNode().GetCntntNode();

    if( !pCNd )
        pCNd = rNds.GoPrevious( &aTmp );

    if ( !pCNd )
        return false;

    SwCntntFrm* pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() );

    if( 0 == pFrm || pCNd->FindTableNode() != pTblNd ||
        (!bInReadOnly && pFrm->IsProtected() ))
    {
        // skip before current section
        aTmp.Assign( *pCNd->StartOfSectionNode(), -1 );
        for( ;; )
        {
            SwNode* pNd = &aTmp.GetNode();

            if( pNd == pTblNd || pNd->GetIndex() < pTblNd->GetIndex() )
                return false;

            pCNd = aTmp.GetNode().GetCntntNode();
            if( 0 == pCNd )
                pCNd = rNds.GoPrevious( &aTmp );

            if ( !pCNd )
                return false;

            pFrm = pCNd->getLayoutFrm( pCNd->GetDoc()->GetCurrentLayout() );

            if( 0 != pFrm && pCNd->FindTableNode() == pTblNd &&
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

sal_Bool GotoPrevTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
                        sal_Bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );

    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();
    if( pTblNd )
    {
        // #i26532#: If we are inside a table, we may not go backward to the
        // table start node, because we would miss any tables inside this table.
        SwTableNode* pInnerTblNd = 0;
        SwNodeIndex aTmpIdx( aIdx );
        while( aTmpIdx.GetIndex() &&
                0 == ( pInnerTblNd = aTmpIdx.GetNode().StartOfSectionNode()->GetTableNode()) )
            aTmpIdx--;

        if( pInnerTblNd == pTblNd )
            aIdx.Assign( *pTblNd, - 1 );
    }

    do {
        while( aIdx.GetIndex() &&
            0 == ( pTblNd = aIdx.GetNode().StartOfSectionNode()->GetTableNode()) )
            aIdx--;

        if( pTblNd ) // any further table node?
        {
            if( fnPosTbl == fnMoveForward ) // at the beginning?
            {
                aIdx = *aIdx.GetNode().StartOfSectionNode();
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTblNd, -1 );
                    continue;
                }
            }
            else
            {
                // check protected cells
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTblNd, -1 );
                    continue;
                }
            }

            SwTxtNode* pTxtNode = aIdx.GetNode().GetTxtNode();
            if ( pTxtNode )
            {
                rCurCrsr.GetPoint()->nNode = *pTxtNode;
                rCurCrsr.GetPoint()->nContent.Assign( pTxtNode, fnPosTbl == fnMoveBackward ?
                                                      pTxtNode->Len() :
                                                      0 );
            }
            return sal_True;
        }
    } while( pTblNd );

    return sal_False;
}

sal_Bool GotoNextTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
                        sal_Bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwTableNode* pTblNd = aIdx.GetNode().FindTableNode();

    if( pTblNd )
        aIdx.Assign( *pTblNd->EndOfSectionNode(), 1 );

    sal_uLong nLastNd = rCurCrsr.GetDoc()->GetNodes().Count() - 1;
    do {
        while( aIdx.GetIndex() < nLastNd &&
                0 == ( pTblNd = aIdx.GetNode().GetTableNode()) )
            ++aIdx;
        if( pTblNd ) // any further table node?
        {
            if( fnPosTbl == fnMoveForward ) // at the beginning?
            {
                if( !lcl_FindNextCell( aIdx, bInReadOnly ))
                {
                    // skip table
                    aIdx.Assign( *pTblNd->EndOfSectionNode(), + 1 );
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
                    aIdx.Assign( *pTblNd->EndOfSectionNode(), + 1 );
                    continue;
                }
            }

            SwTxtNode* pTxtNode = aIdx.GetNode().GetTxtNode();
            if ( pTxtNode )
            {
                rCurCrsr.GetPoint()->nNode = *pTxtNode;
                rCurCrsr.GetPoint()->nContent.Assign( pTxtNode, fnPosTbl == fnMoveBackward ?
                                                      pTxtNode->Len() :
                                                      0 );
            }
            return sal_True;
        }
    } while( pTblNd );

    return sal_False;
}

sal_Bool GotoCurrTable( SwPaM& rCurCrsr, SwPosTable fnPosTbl,
                        sal_Bool bInReadOnly )
{
    SwTableNode* pTblNd = rCurCrsr.GetPoint()->nNode.GetNode().FindTableNode();
    if( !pTblNd )
        return sal_False;

    SwTxtNode* pTxtNode = 0;
    if( fnPosTbl == fnMoveBackward ) // to the end of the table
    {
        SwNodeIndex aIdx( *pTblNd->EndOfSectionNode() );
        if( !lcl_FindPrevCell( aIdx, bInReadOnly ))
            return sal_False;
        pTxtNode = aIdx.GetNode().GetTxtNode();
    }
    else
    {
        SwNodeIndex aIdx( *pTblNd );
        if( !lcl_FindNextCell( aIdx, bInReadOnly ))
            return sal_False;
        pTxtNode = aIdx.GetNode().GetTxtNode();
    }

    if ( pTxtNode )
    {
        rCurCrsr.GetPoint()->nNode = *pTxtNode;
        rCurCrsr.GetPoint()->nContent.Assign( pTxtNode, fnPosTbl == fnMoveBackward ?
                                                        pTxtNode->Len() :
                                                        0 );
    }

    return sal_True;
}

sal_Bool SwCursor::MoveTable( SwWhichTable fnWhichTbl, SwPosTable fnPosTbl )
{
    sal_Bool bRet = sal_False;
    SwTableCursor* m_pTblCrsr = dynamic_cast<SwTableCursor*>(this);

    if( m_pTblCrsr || !HasMark() )
    {
        SwCrsrSaveState aSaveState( *this );
        bRet = (*fnWhichTbl)( *this, fnPosTbl, IsReadOnlyAvailable() ) &&
                !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                           nsSwCursorSelOverFlags::SELOVER_TOGGLE );
    }
    return bRet;
}

sal_Bool SwCrsrShell::MoveTable( SwWhichTable fnWhichTbl, SwPosTable fnPosTbl )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed

    SwShellCrsr* pCrsr = m_pTblCrsr ? m_pTblCrsr : m_pCurCrsr;
    bool bCheckPos;
    sal_Bool bRet;
    sal_uLong nPtNd = 0;
    xub_StrLen nPtCnt = 0;

    if ( !m_pTblCrsr && m_pCurCrsr->HasMark() )
    {
        // switch to table mode
        m_pTblCrsr = new SwShellTableCrsr( *this, *m_pCurCrsr->GetPoint() );
        m_pCurCrsr->DeleteMark();
        m_pCurCrsr->SwSelPaintRects::Hide();
        m_pTblCrsr->SetMark();
        pCrsr = m_pTblCrsr;
        bCheckPos = false;
    }
    else
    {
        bCheckPos = true;
        nPtNd = pCrsr->GetPoint()->nNode.GetIndex();
        nPtCnt = pCrsr->GetPoint()->nContent.GetIndex();
    }

    bRet = pCrsr->MoveTable( fnWhichTbl, fnPosTbl );

    if( bRet )
    {
        // #i45028# - set "top" position for repeated headline rows
        pCrsr->GetPtPos() = Point();

        UpdateCrsr(SwCrsrShell::SCROLLWIN|SwCrsrShell::CHKRANGE|SwCrsrShell::READONLY);

        if( bCheckPos &&
            pCrsr->GetPoint()->nNode.GetIndex() == nPtNd &&
            pCrsr->GetPoint()->nContent.GetIndex() == nPtCnt )
            bRet = sal_False;
    }
    return bRet;
}

bool SwCrsrShell::IsTblComplexForChart()
{
    bool bRet = false;

    // Here we may trigger table formatting so we better do that inside an action
    StartAction();
    const SwTableNode* pTNd = m_pCurCrsr->GetPoint()->nNode.GetNode().FindTableNode();
    if( pTNd )
    {
        // in a table; check if table or section is balanced
        String sSel;
        if( m_pTblCrsr )
            sSel = GetBoxNms();
        bRet = pTNd->GetTable().IsTblComplexForChart( sSel );
    }

    EndAction();

    return bRet;
}

String SwCrsrShell::GetBoxNms() const
{
    String sNm;
    const SwPosition* pPos;
    SwFrm* pFrm;

    if( IsTableMode() )
    {
        SwCntntNode *pCNd = m_pTblCrsr->Start()->nNode.GetNode().GetCntntNode();
        pFrm = pCNd ? pCNd->getLayoutFrm( GetLayout() ) : 0;
        if( !pFrm )
            return sNm;

        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );

        OSL_ENSURE( pFrm, "no frame for this box" );

        if( !pFrm )
            return sNm;

        sNm = ((SwCellFrm*)pFrm)->GetTabBox()->GetName();
        sNm += ':';
        pPos = m_pTblCrsr->End();
    }
    else
    {
        const SwTableNode* pTblNd = IsCrsrInTbl();
        if( !pTblNd )
            return sNm;
        pPos = GetCrsr()->GetPoint();
    }

    SwCntntNode* pCNd = pPos->nNode.GetNode().GetCntntNode();
    pFrm = pCNd ? pCNd->getLayoutFrm( GetLayout() ) : 0;

    if( pFrm )
    {
        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );

        if( pFrm )
            sNm += ((SwCellFrm*)pFrm)->GetTabBox()->GetName();
    }
    return sNm;
}

bool SwCrsrShell::GotoTable( const String& rName )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves
    bool bRet = !m_pTblCrsr && m_pCurCrsr->GotoTable( rName );
    if( bRet )
    {
        m_pCurCrsr->GetPtPos() = Point();
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    }
    return bRet;
}

sal_Bool SwCrsrShell::CheckTblBoxCntnt( const SwPosition* pPos )
{
    if( !m_pBoxIdx || !m_pBoxPtr || IsSelTblCells() || !IsAutoUpdateCells() )
        return sal_False;

    // check if box content is consistent with given box format, reset if not
    SwTableBox* pChkBox = 0;
    SwStartNode* pSttNd = 0;
    if( !pPos )
    {
        // get stored position
        if( m_pBoxIdx && m_pBoxPtr &&
            0 != ( pSttNd = m_pBoxIdx->GetNode().GetStartNode() ) &&
            SwTableBoxStartNode == pSttNd->GetStartNodeType() &&
            m_pBoxPtr == pSttNd->FindTableNode()->GetTable().
                        GetTblBox( m_pBoxIdx->GetIndex() ) )
            pChkBox = m_pBoxPtr;
    }
    else if( 0 != ( pSttNd = pPos->nNode.GetNode().
                                FindSttNodeByType( SwTableBoxStartNode )) )
    {
        pChkBox = pSttNd->FindTableNode()->GetTable().GetTblBox( pSttNd->GetIndex() );
    }

    // box has more than one paragraph
    if( pChkBox && pSttNd->GetIndex() + 2 != pSttNd->EndOfSectionIndex() )
        pChkBox = 0;

    // destroy pointer before next action starts
    if( !pPos && !pChkBox )
        ClearTblBoxCntnt();

    // cursor not anymore in this section?
    if( pChkBox && !pPos &&
        ( m_pCurCrsr->HasMark() || m_pCurCrsr->GetNext() != m_pCurCrsr ||
          pSttNd->GetIndex() + 1 == m_pCurCrsr->GetPoint()->nNode.GetIndex() ))
        pChkBox = 0;

    // Did the content of a box change at all? This is important if e.g. Undo
    // could not restore the content properly.
    if( pChkBox )
    {
        const SwTxtNode* pNd = GetDoc()->GetNodes()[
                                    pSttNd->GetIndex() + 1 ]->GetTxtNode();
        if( !pNd ||
            ( pNd->GetTxt() == ViewShell::GetShellRes()->aCalc_Error &&
              SFX_ITEM_SET == pChkBox->GetFrmFmt()->
                            GetItemState( RES_BOXATR_FORMULA )) )
            pChkBox = 0;
    }

    if( pChkBox )
    {
        // destroy pointer before next action starts
        ClearTblBoxCntnt();
        StartAction();
        GetDoc()->ChkBoxNumFmt( *pChkBox, sal_True );
        EndAction();
    }

    return 0 != pChkBox;
}

void SwCrsrShell::SaveTblBoxCntnt( const SwPosition* pPos )
{
    if( IsSelTblCells() || !IsAutoUpdateCells() )
        return ;

    if( !pPos )
        pPos = m_pCurCrsr->GetPoint();

    SwStartNode* pSttNd = pPos->nNode.GetNode().FindSttNodeByType( SwTableBoxStartNode );

    bool bCheckBox = false;
    if( pSttNd && m_pBoxIdx )
    {
        if( pSttNd == &m_pBoxIdx->GetNode() )
            pSttNd = 0;
        else
            bCheckBox = true;
    }
    else
        bCheckBox = 0 != m_pBoxIdx;

    if( bCheckBox )
    {
        // check m_pBoxIdx
        SwPosition aPos( *m_pBoxIdx );
        CheckTblBoxCntnt( &aPos );
    }

    if( pSttNd )
    {
        m_pBoxPtr = pSttNd->FindTableNode()->GetTable().GetTblBox( pSttNd->GetIndex() );

        if( m_pBoxIdx )
            *m_pBoxIdx = *pSttNd;
        else
            m_pBoxIdx = new SwNodeIndex( *pSttNd );
    }
}

void SwCrsrShell::ClearTblBoxCntnt()
{
    delete m_pBoxIdx, m_pBoxIdx = 0;
    m_pBoxPtr = 0;
}

sal_Bool SwCrsrShell::EndAllTblBoxEdit()
{
    sal_Bool bRet = sal_False;
    ViewShell *pSh = this;
    do {
        if( pSh->IsA( TYPE( SwCrsrShell ) ) )
            bRet |= ((SwCrsrShell*)pSh)->CheckTblBoxCntnt(
                        ((SwCrsrShell*)pSh)->m_pCurCrsr->GetPoint() );

    } while( this != (pSh = (ViewShell *)pSh->GetNext()) );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
