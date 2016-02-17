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
bool SwCursorShell::GoNextCell( bool bAppendLine )
{
    bool bRet = false;
    const SwTableNode* pTableNd = nullptr;

    if( IsTableMode() || nullptr != ( pTableNd = IsCursorInTable() ))
    {
        SwCursor* pCursor = m_pTableCursor ? m_pTableCursor : m_pCurrentCursor;
        SwCallLink aLk( *this ); // watch Cursor-Moves
        bRet = true;

        // Check if we have to move the cursor to a covered cell before
        // proceeding:
        const SwNode* pTableBoxStartNode = pCursor->GetNode().FindTableBoxStartNode();
        const SwTableBox* pTableBox = nullptr;

        if ( pCursor->GetCursorRowSpanOffset() )
        {
            pTableBox = pTableBoxStartNode->GetTableBox();
            if ( pTableBox->getRowSpan() > 1 )
            {
                if ( !pTableNd )
                    pTableNd = IsCursorInTable();
                assert (pTableNd);
                pTableBox = & pTableBox->FindEndOfRowSpan( pTableNd->GetTable(),
                                                           (sal_uInt16)(pTableBox->getRowSpan() + pCursor->GetCursorRowSpanOffset() ) );
                pTableBoxStartNode = pTableBox->GetSttNd();
            }
        }

        SwNodeIndex  aCellStt( *pTableBoxStartNode->EndOfSectionNode(), 1 );

        // if there is another StartNode after the EndNode of a cell then
        // there is another cell
        if( !aCellStt.GetNode().IsStartNode() )
        {
            if( pCursor->HasMark() || !bAppendLine )
                bRet = false;
            else if (pTableNd)
            {
                // if there is no list anymore then create new one
                if ( !pTableBox )
                    pTableBox = pTableNd->GetTable().GetTableBox(
                                    pCursor->GetPoint()->nNode.GetNode().
                                    StartOfSectionIndex() );

                OSL_ENSURE( pTableBox, "Box is not in this table" );
                SwSelBoxes aBoxes;

                // the document might change; w/o Action views would not be notified
                static_cast<SwEditShell*>(this)->StartAllAction();
                bRet = mpDoc->InsertRow( SwTable::SelLineFromBox( pTableBox, aBoxes, false ));
                static_cast<SwEditShell*>(this)->EndAllAction();
            }
        }
        if( bRet && ( bRet = pCursor->GoNextCell() ) )
            UpdateCursor();
    }
    return bRet;
}

bool SwCursorShell::GoPrevCell()
{
    bool bRet = false;
    if( IsTableMode() || IsCursorInTable() )
    {
        SwCursor* pCursor = m_pTableCursor ? m_pTableCursor : m_pCurrentCursor;
        SwCallLink aLk( *this ); // watch Cursor-Moves
        bRet = pCursor->GoPrevCell();
        if( bRet )
            UpdateCursor(); // update current cursor
    }
    return bRet;
}

static const SwFrame* lcl_FindMostUpperCellFrame( const SwFrame* pFrame )
{
    while ( pFrame &&
            ( !pFrame->IsCellFrame() ||
              !pFrame->GetUpper()->GetUpper()->IsTabFrame() ||
               pFrame->GetUpper()->GetUpper()->GetUpper()->IsInTab() ) )
    {
        pFrame = pFrame->GetUpper();
    }
    return pFrame;
}

bool SwCursorShell::_SelTableRowOrCol( bool bRow, bool bRowSimple )
{
    // check if the current cursor's SPoint/Mark are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame->IsInTab() )
        return false;

    const SwTabFrame* pTabFrame = pFrame->FindTabFrame();
    const SwTabFrame* pMasterTabFrame = pTabFrame->IsFollow() ? pTabFrame->FindMaster( true ) : pTabFrame;
    const SwTable* pTable = pTabFrame->GetTable();

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
        const SwShellCursor *pCursor = _GetCursor();
        SwTable::SearchType eSearchType = bRow ? SwTable::SEARCH_ROW : SwTable::SEARCH_COL;
        pTable->CreateSelection( *pCursor, aBoxes, eSearchType, bCheckProtected );
        if( aBoxes.empty() )
            return false;

        pStt = aBoxes[0];
        pEnd = aBoxes.back();
    }
    else
    {
        const SwShellCursor *pCursor = _GetCursor();
        const SwFrame* pStartFrame = pFrame;
        const SwContentNode *pCNd = pCursor->GetContentNode( false );
        const SwFrame* pEndFrame   = pCNd ? pCNd->getLayoutFrame( GetLayout(), &pCursor->GetMkPos() ) : nullptr;

        if ( bRow )
        {
            pStartFrame = lcl_FindMostUpperCellFrame( pStartFrame );
            pEndFrame   = lcl_FindMostUpperCellFrame( pEndFrame   );
        }

        if ( !pStartFrame || !pEndFrame )
            return false;

        const bool bVert = pFrame->ImplFindTabFrame()->IsVertical();

        // If we select upwards it is sufficient to set pStt and pEnd
        // to the first resp. last box of the selection obtained from
        // GetTableSel. However, selecting downwards requires the frames
        // located at the corners of the selection. This does not work
        // for column selections in vertical tables:
        const bool bSelectUp = ( bVert && !bRow ) ||
                                *pCursor->GetPoint() <= *pCursor->GetMark();
        SwCellFrames aCells;
        GetTableSel( static_cast<const SwCellFrame*>(pStartFrame),
                   static_cast<const SwCellFrame*>(pEndFrame),
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
    if( !m_pTableCursor )
    {
        m_pTableCursor = new SwShellTableCursor( *this, *m_pCurrentCursor->GetPoint() );
        m_pCurrentCursor->DeleteMark();
        m_pCurrentCursor->SwSelPaintRects::Hide();
    }

    m_pTableCursor->DeleteMark();

    // set start and end of a column
    m_pTableCursor->GetPoint()->nNode = *pEnd->GetSttNd();
    m_pTableCursor->Move( fnMoveForward, fnGoContent );
    m_pTableCursor->SetMark();
    m_pTableCursor->GetPoint()->nNode = *pStt->GetSttNd()->EndOfSectionNode();
    m_pTableCursor->Move( fnMoveBackward, fnGoContent );

    // set PtPos 'close' to the reference table, otherwise we might get problems
    // with the repeated headlines check in UpdateCursor():
    if ( !bRow )
        m_pTableCursor->GetPtPos() = pMasterTabFrame->IsVertical()
                                   ? pMasterTabFrame->Frame().TopRight()
                                   : pMasterTabFrame->Frame().TopLeft();

    UpdateCursor();
    return true;
}

bool SwCursorShell::SelTable()
{
    // check if the current cursor's SPoint/Mark are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame->IsInTab() )
        return false;

    const SwTabFrame *pTableFrame = pFrame->ImplFindTabFrame();
    const SwTabFrame* pMasterTabFrame = pTableFrame->IsFollow() ? pTableFrame->FindMaster( true ) : pTableFrame;
    const SwTableNode* pTableNd = pTableFrame->GetTable()->GetTableNode();

    SET_CURR_SHELL( this );

    if( !m_pTableCursor )
    {
        m_pTableCursor = new SwShellTableCursor( *this, *m_pCurrentCursor->GetPoint() );
        m_pCurrentCursor->DeleteMark();
        m_pCurrentCursor->SwSelPaintRects::Hide();
    }

    m_pTableCursor->DeleteMark();
    m_pTableCursor->GetPoint()->nNode = *pTableNd;
    m_pTableCursor->Move( fnMoveForward, fnGoContent );
    m_pTableCursor->SetMark();
    // set MkPos 'close' to the master table, otherwise we might get problems
    // with the repeated headlines check in UpdateCursor():
    m_pTableCursor->GetMkPos() = pMasterTabFrame->IsVertical() ? pMasterTabFrame->Frame().TopRight() : pMasterTabFrame->Frame().TopLeft();
    m_pTableCursor->GetPoint()->nNode = *pTableNd->EndOfSectionNode();
    m_pTableCursor->Move( fnMoveBackward, fnGoContent );
    UpdateCursor();
    return true;
}

bool SwCursorShell::SelTableBox()
{
    // if we're in a table, create a table cursor, and select the cell
    // that the current cursor's point resides in

    // search for start node of our table box. If not found, exit really
    const SwStartNode* pStartNode =
        m_pCurrentCursor->GetPoint()->nNode.GetNode().FindTableBoxStartNode();

#if OSL_DEBUG_LEVEL > 0
    // the old code checks whether we're in a table by asking the
    // frame. This should yield the same result as searching for the
    // table box start node, right?
    SwFrame *pFrame = GetCurrFrame();
    OSL_ENSURE( !pFrame->IsInTab() == !(pStartNode != nullptr),
                "Schroedinger's table: We're in a box, and also we aren't." );
#endif
    if( pStartNode == nullptr )
        return false;

    SET_CURR_SHELL( this );

    // create a table cursor, if there isn't one already
    if( !m_pTableCursor )
    {
        m_pTableCursor = new SwShellTableCursor( *this, *m_pCurrentCursor->GetPoint() );
        m_pCurrentCursor->DeleteMark();
        m_pCurrentCursor->SwSelPaintRects::Hide();
    }

    // select the complete box with our shiny new m_pTableCursor
    // 1. delete mark, and move point to first content node in box
    m_pTableCursor->DeleteMark();
    *(m_pTableCursor->GetPoint()) = SwPosition( *pStartNode );
    m_pTableCursor->Move( fnMoveForward, fnGoNode );

    // 2. set mark, and move point to last content node in box
    m_pTableCursor->SetMark();
    *(m_pTableCursor->GetPoint()) = SwPosition( *(pStartNode->EndOfSectionNode()) );
    m_pTableCursor->Move( fnMoveBackward, fnGoNode );

    // 3. exchange
    m_pTableCursor->Exchange();

    // with some luck, UpdateCursor() will now update everything that
    // needs updating
    UpdateCursor();

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

    SwContentFrame* pFrame = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

    if ( nullptr == pFrame || pCNd->FindTableNode() != pTableNd ||
        (!bInReadOnly && pFrame->IsProtected() ) )
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
            pFrame = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

            if ( nullptr != pFrame && pCNd->FindTableNode() == pTableNd &&
                (bInReadOnly || !pFrame->IsProtected() ) )
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

    SwContentFrame* pFrame = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

    if( nullptr == pFrame || pCNd->FindTableNode() != pTableNd ||
        (!bInReadOnly && pFrame->IsProtected() ))
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

            pFrame = pCNd->getLayoutFrame( pCNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout() );

            if( nullptr != pFrame && pCNd->FindTableNode() == pTableNd &&
                (bInReadOnly || !pFrame->IsProtected() ) )
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

bool GotoPrevTable( SwPaM& rCurrentCursor, SwPosTable fnPosTable,
                    bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurrentCursor.GetPoint()->nNode );

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
                rCurrentCursor.GetPoint()->nNode = *pTextNode;
                rCurrentCursor.GetPoint()->nContent.Assign( pTextNode, fnPosTable == fnMoveBackward ?
                                                      pTextNode->Len() :
                                                      0 );
            }
            return true;
        }
    } while( pTableNd );

    return false;
}

bool GotoNextTable( SwPaM& rCurrentCursor, SwPosTable fnPosTable,
                    bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurrentCursor.GetPoint()->nNode );
    SwTableNode* pTableNd = aIdx.GetNode().FindTableNode();

    if( pTableNd )
        aIdx.Assign( *pTableNd->EndOfSectionNode(), 1 );

    sal_uLong nLastNd = rCurrentCursor.GetDoc()->GetNodes().Count() - 1;
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
                rCurrentCursor.GetPoint()->nNode = *pTextNode;
                rCurrentCursor.GetPoint()->nContent.Assign( pTextNode, fnPosTable == fnMoveBackward ?
                                                      pTextNode->Len() :
                                                      0 );
            }
            return true;
        }
    } while( pTableNd );

    return false;
}

bool GotoCurrTable( SwPaM& rCurrentCursor, SwPosTable fnPosTable,
                    bool bInReadOnly )
{
    SwTableNode* pTableNd = rCurrentCursor.GetPoint()->nNode.GetNode().FindTableNode();
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
        rCurrentCursor.GetPoint()->nNode = *pTextNode;
        rCurrentCursor.GetPoint()->nContent.Assign( pTextNode, fnPosTable == fnMoveBackward ?
                                                        pTextNode->Len() :
                                                        0 );
    }

    return true;
}

bool SwCursor::MoveTable( SwWhichTable fnWhichTable, SwPosTable fnPosTable )
{
    bool bRet = false;
    SwTableCursor* m_pTableCursor = dynamic_cast<SwTableCursor*>(this);

    if( m_pTableCursor || !HasMark() )
    {
        SwCursorSaveState aSaveState( *this );
        bRet = (*fnWhichTable)( *this, fnPosTable, IsReadOnlyAvailable() ) &&
                !IsSelOvr( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                           nsSwCursorSelOverFlags::SELOVER_TOGGLE );
    }
    return bRet;
}

bool SwCursorShell::MoveTable( SwWhichTable fnWhichTable, SwPosTable fnPosTable )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves; call Link if needed

    SwShellCursor* pCursor = m_pTableCursor ? m_pTableCursor : m_pCurrentCursor;
    bool bCheckPos;
    bool bRet;
    sal_uLong nPtNd = 0;
    sal_Int32 nPtCnt = 0;

    if ( !m_pTableCursor && m_pCurrentCursor->HasMark() )
    {
        // switch to table mode
        m_pTableCursor = new SwShellTableCursor( *this, *m_pCurrentCursor->GetPoint() );
        m_pCurrentCursor->DeleteMark();
        m_pCurrentCursor->SwSelPaintRects::Hide();
        m_pTableCursor->SetMark();
        pCursor = m_pTableCursor;
        bCheckPos = false;
    }
    else
    {
        bCheckPos = true;
        nPtNd = pCursor->GetPoint()->nNode.GetIndex();
        nPtCnt = pCursor->GetPoint()->nContent.GetIndex();
    }

    bRet = pCursor->MoveTable( fnWhichTable, fnPosTable );

    if( bRet )
    {
        // #i45028# - set "top" position for repeated headline rows
        pCursor->GetPtPos() = Point();

        UpdateCursor(SwCursorShell::SCROLLWIN|SwCursorShell::CHKRANGE|SwCursorShell::READONLY);

        if( bCheckPos &&
            pCursor->GetPoint()->nNode.GetIndex() == nPtNd &&
            pCursor->GetPoint()->nContent.GetIndex() == nPtCnt )
            bRet = false;
    }
    return bRet;
}

bool SwCursorShell::IsTableComplexForChart()
{
    bool bRet = false;

    // Here we may trigger table formatting so we better do that inside an action
    StartAction();
    const SwTableNode* pTNd = m_pCurrentCursor->GetPoint()->nNode.GetNode().FindTableNode();
    if( pTNd )
    {
        // in a table; check if table or section is balanced
        OUString sSel;
        if( m_pTableCursor )
            sSel = GetBoxNms();
        bRet = pTNd->GetTable().IsTableComplexForChart( sSel );
    }
    EndAction();

    return bRet;
}

OUString SwCursorShell::GetBoxNms() const
{
    OUString sNm;
    const SwPosition* pPos;
    SwFrame* pFrame;

    if( IsTableMode() )
    {
        SwContentNode *pCNd = m_pTableCursor->Start()->nNode.GetNode().GetContentNode();
        pFrame = pCNd ? pCNd->getLayoutFrame( GetLayout() ) : nullptr;
        if( !pFrame )
            return sNm;

        do {
            pFrame = pFrame->GetUpper();
        } while ( pFrame && !pFrame->IsCellFrame() );

        OSL_ENSURE( pFrame, "no frame for this box" );

        if( !pFrame )
            return sNm;

        sNm = static_cast<SwCellFrame*>(pFrame)->GetTabBox()->GetName();
        sNm += ":";
        pPos = m_pTableCursor->End();
    }
    else
    {
        const SwTableNode* pTableNd = IsCursorInTable();
        if( !pTableNd )
            return sNm;
        pPos = GetCursor()->GetPoint();
    }

    SwContentNode* pCNd = pPos->nNode.GetNode().GetContentNode();
    pFrame = pCNd ? pCNd->getLayoutFrame( GetLayout() ) : nullptr;

    if( pFrame )
    {
        do {
            pFrame = pFrame->GetUpper();
        } while ( pFrame && !pFrame->IsCellFrame() );

        if( pFrame )
            sNm += static_cast<SwCellFrame*>(pFrame)->GetTabBox()->GetName();
    }
    return sNm;
}

bool SwCursorShell::GotoTable( const OUString& rName )
{
    SwCallLink aLk( *this ); // watch Cursor-Moves
    bool bRet = !m_pTableCursor && m_pCurrentCursor->GotoTable( rName );
    if( bRet )
    {
        m_pCurrentCursor->GetPtPos() = Point();
        UpdateCursor( SwCursorShell::SCROLLWIN | SwCursorShell::CHKRANGE |
                    SwCursorShell::READONLY );
    }
    return bRet;
}

bool SwCursorShell::CheckTableBoxContent( const SwPosition* pPos )
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
        ( m_pCurrentCursor->HasMark() || m_pCurrentCursor->GetNext() != m_pCurrentCursor ||
          pSttNd->GetIndex() + 1 == m_pCurrentCursor->GetPoint()->nNode.GetIndex() ))
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

void SwCursorShell::SaveTableBoxContent( const SwPosition* pPos )
{
    if( IsSelTableCells() || !IsAutoUpdateCells() )
        return ;

    if( !pPos )
        pPos = m_pCurrentCursor->GetPoint();

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

void SwCursorShell::ClearTableBoxContent()
{
    delete m_pBoxIdx;
    m_pBoxIdx = nullptr;
    m_pBoxPtr = nullptr;
}

bool SwCursorShell::EndAllTableBoxEdit()
{
    bool bRet = false;
    for(SwViewShell& rSh : GetRingContainer())
    {
        if( dynamic_cast<const SwCursorShell *>(&rSh) != nullptr )
            bRet |= static_cast<SwCursorShell*>(&rSh)->CheckTableBoxContent(
                        static_cast<SwCursorShell*>(&rSh)->m_pCurrentCursor->GetPoint() );

    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
