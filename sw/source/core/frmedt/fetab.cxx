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
#include <hintids.hxx>

#include <vcl/errinf.hxx>
#include <vcl/svapp.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svx/svxids.hrc>
#include <editeng/protitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svtools/ruler.hxx>
#include <swwait.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <docary.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <IDocumentState.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <cntfrm.hxx>
#include <txtfrm.hxx>
#include <notxtfrm.hxx>
#include <rootfrm.hxx>
#include <pagefrm.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <flyfrm.hxx>
#include <dflyobj.hxx>
#include <swtable.hxx>
#include <swddetbl.hxx>
#include <ndtxt.hxx>
#include <calc.hxx>
#include <tabcol.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <pam.hxx>
#include <pamtyp.hxx>
#include <viscrs.hxx>
#include <tblsel.hxx>
#include <swtblfmt.hxx>
#include <swerror.h>
#include <swundo.hxx>
#include <frmtool.hxx>
#include <fmtrowsplt.hxx>
#include <node.hxx>
#include <sortedobjs.hxx>

using namespace ::com::sun::star;

// also see swtable.cxx
#define COLFUZZY 20L

static bool IsSame( long nA, long nB ) { return  std::abs(nA-nB) <= COLFUZZY; }

class TableWait
{
    const std::unique_ptr<SwWait> m_pWait;
    // this seems really fishy: do some locking, if an arbitrary number of lines is exceeded
    static const size_t our_kLineLimit = 20;
    static bool ShouldWait(size_t nCnt, SwFrame *pFrame, size_t nCnt2)
        { return our_kLineLimit < nCnt || our_kLineLimit < nCnt2 || (pFrame && our_kLineLimit < pFrame->ImplFindTabFrame()->GetTable()->GetTabLines().size()); }
public:
    TableWait(size_t nCnt, SwFrame *pFrame, SwDocShell &rDocShell, size_t nCnt2 = 0)
        : m_pWait( ShouldWait(nCnt, pFrame, nCnt2) ? std::make_unique<SwWait>( rDocShell, true ) : nullptr )
    { }
};

void SwFEShell::ParkCursorInTab()
{
    SwCursor * pSwCursor = GetSwCursor();

    OSL_ENSURE(pSwCursor, "no SwCursor");

    SwPosition aStartPos = *pSwCursor->GetPoint(), aEndPos = aStartPos;

    /* Search least and greatest position in current cursor ring.
     */
    for(SwPaM& rTmpCursor : pSwCursor->GetRingContainer())
    {
        SwCursor* pTmpCursor = static_cast<SwCursor *>(&rTmpCursor);
        const SwPosition * pPt = pTmpCursor->GetPoint(),
            * pMk = pTmpCursor->GetMark();

        if (*pPt < aStartPos)
            aStartPos = *pPt;

        if (*pPt > aEndPos)
            aEndPos = *pPt;

        if (*pMk < aStartPos)
            aStartPos = *pMk;

        if (*pMk > aEndPos)
            aEndPos = *pMk;

    }

    KillPams();

    /* @@@ semantic: SwCursor::operator=() is not implemented @@@ */

    /* Set cursor to end of selection to ensure IsLastCellInRow works
       properly. */
    {
        SwCursor aTmpCursor( aEndPos, nullptr );
        *pSwCursor = aTmpCursor;
    }

    /* Move the cursor out of the columns to delete and stay in the
       same row. If the table has only one column the cursor will
       stay in the row and the shell will take care of it. */
    if (IsLastCellInRow())
    {
        /* If the cursor is in the last row of the table, first
           try to move it to the previous cell. If that fails move
           it to the next cell. */

        {
            SwCursor aTmpCursor( aStartPos, nullptr );
            *pSwCursor = aTmpCursor;
        }

        if (! pSwCursor->GoPrevCell())
        {
            SwCursor aTmpCursor( aEndPos, nullptr );
            *pSwCursor = aTmpCursor;
            pSwCursor->GoNextCell();
        }
    }
    else
    {
        /* If the cursor is not in the last row of the table, first
           try to move it to the next cell. If that fails move it
           to the previous cell. */

        {
            SwCursor aTmpCursor( aEndPos, nullptr );
            *pSwCursor = aTmpCursor;
        }

        if (! pSwCursor->GoNextCell())
        {
            SwCursor aTmpCursor( aStartPos, nullptr );
            *pSwCursor = aTmpCursor;
            pSwCursor->GoPrevCell();
        }
    }
}

void SwFEShell::InsertRow( sal_uInt16 nCnt, bool bBehind )
{
    // check if Point/Mark of current cursor are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;

    if( dynamic_cast< const SwDDETable* >(pFrame->ImplFindTabFrame()->GetTable()) != nullptr )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search boxes via the layout
    SwSelBoxes aBoxes;
    bool bSelectAll = StartsWithTable() && ExtendedSelectedAll();
    if (bSelectAll)
    {
        // Set the end of the selection to the last paragraph of the last cell of the table.
        SwPaM* pPaM = getShellCursor(false);
        SwNode* pNode = pPaM->Start()->nNode.GetNode().FindTableNode()->EndOfSectionNode();
        // pNode is the end node of the table, we want the last node before the end node of the last cell.
        pPaM->End()->nNode = pNode->GetIndex() - 2;
        pPaM->End()->nContent.Assign(pPaM->End()->nNode.GetNode().GetContentNode(), 0);
    }
    GetTableSel( *this, aBoxes, SwTableSearchType::Row );

    TableWait aWait( nCnt, pFrame, *GetDoc()->GetDocShell(), aBoxes.size() );

    if ( !aBoxes.empty() )
        GetDoc()->InsertRow( aBoxes, nCnt, bBehind );

    EndAllActionAndCall();
}

void SwFEShell::InsertCol( sal_uInt16 nCnt, bool bBehind )
{
    // check if Point/Mark of current cursor are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;

    if( dynamic_cast< const SwDDETable* >(pFrame->ImplFindTabFrame()->GetTable()) != nullptr )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return;
    }

    SET_CURR_SHELL( this );

    if( !CheckSplitCells( *this, nCnt + 1, SwTableSearchType::Col ) )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLINSCOL_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return;
    }

    StartAllAction();
    // search boxes via the layout
    SwSelBoxes aBoxes;
    GetTableSel( *this, aBoxes, SwTableSearchType::Col );

    TableWait aWait( nCnt, pFrame, *GetDoc()->GetDocShell(), aBoxes.size() );

    if( !aBoxes.empty() )
        GetDoc()->InsertCol( aBoxes, nCnt, bBehind );

    EndAllActionAndCall();
}

//  Determines if the current cursor is in the last row of the table.
bool SwFEShell::IsLastCellInRow() const
{
    SwTabCols aTabCols;
    GetTabCols( aTabCols );
    bool bResult = false;

    if (IsTableRightToLeft())
        /* If the table is right-to-left the last row is the most left one. */
        bResult = 0 == GetCurTabColNum();
    else
        /* If the table is left-to-right the last row is the most right one. */
        bResult = aTabCols.Count() == GetCurTabColNum();

    return bResult;
}

bool SwFEShell::DeleteCol()
{
    // check if Point/Mark of current cursor are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return false;

    if( dynamic_cast< const SwDDETable* >(pFrame->ImplFindTabFrame()->GetTable()) != nullptr )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return false;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search boxes via the layout
    bool bRet;
    SwSelBoxes aBoxes;
    GetTableSel( *this, aBoxes, SwTableSearchType::Col );
    if ( !aBoxes.empty() )
    {
        TableWait aWait( aBoxes.size(), pFrame, *GetDoc()->GetDocShell() );

        // remove crsr from the deletion area.
        // Put them behind/on the table; via the
        // document position they will be put to the old position
        while( !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();

        ParkCursorInTab();

        // then delete the column
        StartUndo(SwUndoId::COL_DELETE);
        bRet = GetDoc()->DeleteRowCol( aBoxes, true );
        EndUndo(SwUndoId::COL_DELETE);

    }
    else
        bRet = false;

    EndAllActionAndCall();
    return bRet;
}

void SwFEShell::DeleteTable()
{
    DeleteRow(true);
}

bool SwFEShell::DeleteRow(bool bCompleteTable)
{
    // check if Point/Mark of current cursor are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return false;

    if( dynamic_cast< const SwDDETable* >(pFrame->ImplFindTabFrame()->GetTable()) != nullptr )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return false;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search for boxes via the layout
    bool bRet;
    SwSelBoxes aBoxes;
    GetTableSel( *this, aBoxes, SwTableSearchType::Row );

    if( !aBoxes.empty() )
    {
        TableWait aWait( aBoxes.size(), pFrame, *GetDoc()->GetDocShell() );

        // Delete cursors from the deletion area.
        // Then the cursor is:
        //  1. the following row, if there is another row after this
        //  2. the preceding row, if there is another row before this
        //  3. otherwise below the table
        {
            SwTableNode* pTableNd = pFrame->IsTextFrame()
                ? static_cast<SwTextFrame*>(pFrame)->GetTextNodeFirst()->FindTableNode()
                : static_cast<SwNoTextFrame*>(pFrame)->GetNode()->FindTableNode();

            // search all boxes / lines
            FndBox_ aFndBox( nullptr, nullptr );
            {
                FndPara aPara( aBoxes, &aFndBox );
                ForEach_FndLineCopyCol( pTableNd->GetTable().GetTabLines(), &aPara );
            }

            if( aFndBox.GetLines().empty() )
            {
                EndAllActionAndCall();
                return false;
            }

            KillPams();

            FndBox_* pFndBox = &aFndBox;
            while( 1 == pFndBox->GetLines().size() &&
                    1 == pFndBox->GetLines().front()->GetBoxes().size())
            {
                FndBox_ *const pTmp = pFndBox->GetLines().front()->GetBoxes()[0].get();
                if( pTmp->GetBox()->GetSttNd() )
                    break;      // otherwise too far
                pFndBox = pTmp;
            }

            SwTableLine* pDelLine = pFndBox->GetLines().back()->GetLine();
            SwTableBox* pDelBox = pDelLine->GetTabBoxes().back();
            while( !pDelBox->GetSttNd() )
            {
                SwTableLine* pLn = pDelBox->GetTabLines().back();
                pDelBox = pLn->GetTabBoxes().back();
            }
            SwTableBox* pNextBox = pDelLine->FindNextBox( pTableNd->GetTable(),
                                                            pDelBox );
            while( pNextBox &&
                    pNextBox->GetFrameFormat()->GetProtect().IsContentProtected() )
                pNextBox = pNextBox->FindNextBox( pTableNd->GetTable(), pNextBox );

            if( !pNextBox )         // no next? then the previous
            {
                pDelLine = pFndBox->GetLines().front()->GetLine();
                pDelBox = pDelLine->GetTabBoxes()[ 0 ];
                while( !pDelBox->GetSttNd() )
                    pDelBox = pDelBox->GetTabLines()[0]->GetTabBoxes()[0];
                pNextBox = pDelLine->FindPreviousBox( pTableNd->GetTable(),
                                                            pDelBox );
                while( pNextBox &&
                        pNextBox->GetFrameFormat()->GetProtect().IsContentProtected() )
                    pNextBox = pNextBox->FindPreviousBox( pTableNd->GetTable(), pNextBox );
            }

            sal_uLong nIdx;
            if( pNextBox )      // put cursor here
                nIdx = pNextBox->GetSttIdx() + 1;
            else                // otherwise below the table
                nIdx = pTableNd->EndOfSectionIndex() + 1;

            SwNodeIndex aIdx( GetDoc()->GetNodes(), nIdx );
            SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );

            if( pCNd )
            {
                SwPaM* pPam = GetCursor();
                pPam->GetPoint()->nNode = aIdx;
                pPam->GetPoint()->nContent.Assign( pCNd, 0 );
                pPam->SetMark();            // both want something
                pPam->DeleteMark();
            }
        }

        // now delete the lines
        StartUndo(bCompleteTable ? SwUndoId::UI_TABLE_DELETE : SwUndoId::ROW_DELETE);
        bRet = GetDoc()->DeleteRowCol( aBoxes );
        EndUndo(bCompleteTable ? SwUndoId::UI_TABLE_DELETE : SwUndoId::ROW_DELETE);
    }
    else
        bRet = false;

    EndAllActionAndCall();
    return bRet;
}

TableMergeErr SwFEShell::MergeTab()
{
    // check if Point/Mark of current cursor are in a table
    TableMergeErr nRet = TableMergeErr::NoSelection;
    if( IsTableMode() )
    {
        SwShellTableCursor* pTableCursor = GetTableCursor();
        const SwTableNode* pTableNd = pTableCursor->GetNode().FindTableNode();
        if( dynamic_cast< const SwDDETable* >(&pTableNd->GetTable()) != nullptr )
        {
            vcl::Window* pWin = GetWin();
            ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                            DialogMask::MessageInfo | DialogMask::ButtonsOk );
        }
        else
        {
            SET_CURR_SHELL( this );
            StartAllAction();

            TableWait aWait(pTableCursor->GetSelectedBoxesCount(), nullptr,
                    *GetDoc()->GetDocShell(),
                     pTableNd->GetTable().GetTabLines().size() );

            nRet = GetDoc()->MergeTable( *pTableCursor );

            KillPams();

            EndAllActionAndCall();
        }
    }
    return nRet;
}

void SwFEShell::SplitTab( bool bVert, sal_uInt16 nCnt, bool bSameHeight )
{
    // check if Point/Mark of current cursor are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;

    if( dynamic_cast< const SwDDETable* >(pFrame->ImplFindTabFrame()->GetTable()) != nullptr  )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return;
    }

    SET_CURR_SHELL( this );

    if( bVert && !CheckSplitCells( *this, nCnt + 1, SwTableSearchType::NONE ) )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLSPLIT_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return;
    }
    StartAllAction();
    // search boxes via the layout
    SwSelBoxes aBoxes;
    GetTableSel( *this, aBoxes );
    if( !aBoxes.empty() )
    {
        TableWait aWait( nCnt, pFrame, *GetDoc()->GetDocShell(), aBoxes.size() );

        // now delete the columns
        GetDoc()->SplitTable( aBoxes, bVert, nCnt, bSameHeight );

        ClearFEShellTabCols(*GetDoc(), nullptr);
    }
    EndAllActionAndCall();
}

void SwFEShell::GetTabCols_(SwTabCols &rToFill, const SwFrame *pBox) const
{
    const SwTabFrame *pTab = pBox->FindTabFrame();
    if (m_pColumnCache)
    {
        bool bDel = true;
        if (m_pColumnCache->pLastTable == pTab->GetTable())
        {
            bDel = false;
            SwRectFnSet aRectFnSet(pTab);

            const SwPageFrame* pPage = pTab->FindPageFrame();
            const sal_uLong nLeftMin = aRectFnSet.GetLeft(pTab->getFrameArea()) -
                                   aRectFnSet.GetLeft(pPage->getFrameArea());
            const sal_uLong nRightMax = aRectFnSet.GetRight(pTab->getFrameArea()) -
                                    aRectFnSet.GetLeft(pPage->getFrameArea());

            if (m_pColumnCache->pLastTabFrame != pTab)
            {
                // if TabFrame was changed, we only shift a little bit
                // as the width is the same
                SwRectFnSet fnRectX(m_pColumnCache->pLastTabFrame);
                if (fnRectX.GetWidth(m_pColumnCache->pLastTabFrame->getFrameArea()) ==
                    aRectFnSet.GetWidth(pTab->getFrameArea()) )
                {
                    m_pColumnCache->pLastCols->SetLeftMin( nLeftMin );

                    m_pColumnCache->pLastTabFrame = pTab;
                }
                else
                    bDel = true;
            }

            if ( !bDel &&
                 m_pColumnCache->pLastCols->GetLeftMin () == static_cast<sal_uInt16>(nLeftMin) &&
                 m_pColumnCache->pLastCols->GetLeft    () == static_cast<sal_uInt16>(aRectFnSet.GetLeft(pTab->getFramePrintArea())) &&
                 m_pColumnCache->pLastCols->GetRight   () == static_cast<sal_uInt16>(aRectFnSet.GetRight(pTab->getFramePrintArea()))&&
                 m_pColumnCache->pLastCols->GetRightMax() == static_cast<sal_uInt16>(nRightMax) - m_pColumnCache->pLastCols->GetLeftMin() )
            {
                if (m_pColumnCache->pLastCellFrame != pBox)
                {
                    pTab->GetTable()->GetTabCols( *m_pColumnCache->pLastCols,
                                        static_cast<const SwCellFrame*>(pBox)->GetTabBox(), true);
                    m_pColumnCache->pLastCellFrame = pBox;
                }
                rToFill = *m_pColumnCache->pLastCols;
            }
            else
                bDel = true;
        }
        if ( bDel )
            m_pColumnCache.reset();
    }
    if (!m_pColumnCache)
    {
        SwDoc::GetTabCols( rToFill, static_cast<const SwCellFrame*>(pBox) );

        m_pColumnCache.reset(new SwColCache);
        m_pColumnCache->pLastCols.reset(new SwTabCols(rToFill));
        m_pColumnCache->pLastTable = pTab->GetTable();
        m_pColumnCache->pLastTabFrame = pTab;
        m_pColumnCache->pLastCellFrame = pBox;
    }
}

void SwFEShell::GetTabRows_(SwTabCols &rToFill, const SwFrame *pBox) const
{
    const SwTabFrame *pTab = pBox->FindTabFrame();
    if (m_pRowCache)
    {
        bool bDel = true;
        if (m_pRowCache->pLastTable == pTab->GetTable())
        {
            bDel = false;
            SwRectFnSet aRectFnSet(pTab);
            const SwPageFrame* pPage = pTab->FindPageFrame();
            const long nLeftMin  = ( aRectFnSet.IsVert() ?
                                     pTab->GetPrtLeft() - pPage->getFrameArea().Left() :
                                     pTab->GetPrtTop() - pPage->getFrameArea().Top() );
            const long nLeft     = aRectFnSet.IsVert() ? LONG_MAX : 0;
            const long nRight    = aRectFnSet.GetHeight(pTab->getFramePrintArea());
            const long nRightMax = aRectFnSet.IsVert() ? nRight : LONG_MAX;

            if (m_pRowCache->pLastTabFrame != pTab || m_pRowCache->pLastCellFrame != pBox)
                bDel = true;

            if ( !bDel &&
                 m_pRowCache->pLastCols->GetLeftMin () == nLeftMin &&
                 m_pRowCache->pLastCols->GetLeft    () == nLeft &&
                 m_pRowCache->pLastCols->GetRight   () == nRight &&
                 m_pRowCache->pLastCols->GetRightMax() == nRightMax )
            {
                rToFill = *m_pRowCache->pLastCols;
            }
            else
                bDel = true;
        }
        if ( bDel )
            m_pRowCache.reset();
    }
    if (!m_pRowCache)
    {
        SwDoc::GetTabRows( rToFill, static_cast<const SwCellFrame*>(pBox) );

        m_pRowCache.reset(new SwColCache);
        m_pRowCache->pLastCols.reset(new SwTabCols(rToFill));
        m_pRowCache->pLastTable = pTab->GetTable();
        m_pRowCache->pLastTabFrame = pTab;
        m_pRowCache->pLastCellFrame = pBox;
    }
}

void SwFEShell::SetTabCols( const SwTabCols &rNew, bool bCurRowOnly )
{
    SwFrame *pBox = GetCurrFrame();
    if( !pBox || !pBox->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do
    {
        pBox = pBox->GetUpper();
    } while (pBox && !pBox->IsCellFrame());

    GetDoc()->SetTabCols( rNew, bCurRowOnly, static_cast<SwCellFrame*>(pBox) );
    EndAllActionAndCall();
}

void SwFEShell::GetTabCols( SwTabCols &rToFill ) const
{
    const SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;
    do
    {
        pFrame = pFrame->GetUpper();
    }
    while (pFrame && !pFrame->IsCellFrame());

    if (!pFrame)
        return;

    GetTabCols_( rToFill, pFrame );
}

void SwFEShell::GetTabRows( SwTabCols &rToFill ) const
{
    const SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;
    do
    {
        pFrame = pFrame->GetUpper();
    } while (pFrame && !pFrame->IsCellFrame());

    if (!pFrame)
        return;

    GetTabRows_( rToFill, pFrame );
}

void SwFEShell::SetTabRows( const SwTabCols &rNew, bool bCurColOnly )
{
    SwFrame *pBox = GetCurrFrame();
    if( !pBox || !pBox->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do
    {
        pBox = pBox->GetUpper();
    } while (pBox && !pBox->IsCellFrame());

    GetDoc()->SetTabRows( rNew, bCurColOnly, static_cast<SwCellFrame*>(pBox) );
    EndAllActionAndCall();
}

void SwFEShell::GetMouseTabRows( SwTabCols &rToFill, const Point &rPt ) const
{
    const SwFrame *pBox = GetBox( rPt );
    if ( pBox )
        GetTabRows_( rToFill, pBox );
}

void SwFEShell::SetMouseTabRows( const SwTabCols &rNew, bool bCurColOnly, const Point &rPt )
{
    const SwFrame *pBox = GetBox( rPt );
    if( pBox )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetTabRows( rNew, bCurColOnly, static_cast<const SwCellFrame*>(pBox) );
        EndAllActionAndCall();
    }
}

void SwFEShell::SetRowSplit( const SwFormatRowSplit& rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowSplit( *getShellCursor( false ), rNew );
    EndAllActionAndCall();
}

std::unique_ptr<SwFormatRowSplit> SwFEShell::GetRowSplit() const
{
    return SwDoc::GetRowSplit( *getShellCursor( false ) );
}

void SwFEShell::SetRowHeight( const SwFormatFrameSize &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowHeight( *getShellCursor( false ), rNew );
    EndAllActionAndCall();
}

std::unique_ptr<SwFormatFrameSize> SwFEShell::GetRowHeight() const
{
    return SwDoc::GetRowHeight( *getShellCursor( false ) );
}

bool SwFEShell::BalanceRowHeight( bool bTstOnly, const bool bOptimize )
{
    SET_CURR_SHELL( this );
    if( !bTstOnly )
        StartAllAction();
    bool bRet = GetDoc()->BalanceRowHeight( *getShellCursor( false ), bTstOnly, bOptimize );
    if( !bTstOnly )
        EndAllActionAndCall();
    return bRet;
}

void SwFEShell::SetRowBackground( const SvxBrushItem &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowBackground( *getShellCursor( false ), rNew );
    EndAllActionAndCall();
}

bool SwFEShell::GetRowBackground( SvxBrushItem &rToFill ) const
{
    return SwDoc::GetRowBackground( *getShellCursor( false ), rToFill );
}

void SwFEShell::SetTabBorders( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetTabBorders( *getShellCursor( false ), rSet );
    EndAllActionAndCall();
}

void SwFEShell::SetTabLineStyle( const Color* pColor, bool bSetLine,
                                 const editeng::SvxBorderLine* pBorderLine )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetTabLineStyle( *getShellCursor( false ),
                                pColor, bSetLine, pBorderLine );
    EndAllActionAndCall();
}

void SwFEShell::GetTabBorders( SfxItemSet& rSet ) const
{
    SwDoc::GetTabBorders( *getShellCursor( false ), rSet );
}

void SwFEShell::SetBoxBackground( const SvxBrushItem &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAttr( *getShellCursor( false ), rNew );
    EndAllActionAndCall();
}

bool SwFEShell::GetBoxBackground( SvxBrushItem &rToFill ) const
{
    return SwDoc::GetBoxAttr( *getShellCursor( false ), rToFill );
}

void SwFEShell::SetBoxDirection( const SvxFrameDirectionItem& rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAttr( *getShellCursor( false ), rNew );
    EndAllActionAndCall();
}

bool SwFEShell::GetBoxDirection( SvxFrameDirectionItem&  rToFill ) const
{
    return SwDoc::GetBoxAttr( *getShellCursor( false ), rToFill );
}

void SwFEShell::SetBoxAlign( sal_uInt16 nAlign )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAlign( *getShellCursor( false ), nAlign );
    EndAllActionAndCall();
}

sal_uInt16 SwFEShell::GetBoxAlign() const
{
    return SwDoc::GetBoxAlign( *getShellCursor( false ) );
}

void SwFEShell::SetTabBackground( const SvxBrushItem &rNew )
{
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetAttr( rNew, *pFrame->ImplFindTabFrame()->GetFormat() );
    EndAllAction(); // no call, nothing changes!
    GetDoc()->getIDocumentState().SetModified();
}

void SwFEShell::GetTabBackground( SvxBrushItem &rToFill ) const
{
    SwFrame *pFrame = GetCurrFrame();
    if( pFrame && pFrame->IsInTab() )
        rToFill = pFrame->ImplFindTabFrame()->GetFormat()->makeBackgroundBrushItem();
}

bool SwFEShell::HasWholeTabSelection() const
{
    // whole table selected?
    if ( IsTableMode() )
    {
        SwSelBoxes aBoxes;
        ::GetTableSelCrs( *this, aBoxes );
        if( !aBoxes.empty() )
        {
            const SwTableNode *pTableNd = IsCursorInTable();
            return pTableNd &&
                aBoxes[0]->GetSttIdx() - 1 == pTableNd->EndOfSectionNode()->StartOfSectionIndex() &&
                aBoxes.back()->GetSttNd()->EndOfSectionIndex() + 1 == pTableNd->EndOfSectionIndex();
        }
    }
    return false;
}

bool SwFEShell::HasBoxSelection() const
{
    if(!IsCursorInTable())
        return false;
    // whole table selected?
    if( IsTableMode() )
        return true;
    SwPaM* pPam = GetCursor();
        // empty boxes are also selected as the absence of selection
    bool bChg = false;
    if( pPam->GetPoint() == pPam->End())
    {
        bChg = true;
        pPam->Exchange();
    }
    SwNode* pNd;
    if( pPam->GetPoint()->nNode.GetIndex() -1 ==
        ( pNd = &pPam->GetNode())->StartOfSectionIndex() &&
        !pPam->GetPoint()->nContent.GetIndex() &&
        pPam->GetMark()->nNode.GetIndex() + 1 ==
        pNd->EndOfSectionIndex())
    {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode(), -1 );
            SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
            if( !pCNd )
            {
                pCNd = SwNodes::GoPrevious( &aIdx );
                OSL_ENSURE( pCNd, "no ContentNode in box ??" );
            }
            if( pPam->GetMark()->nContent == pCNd->Len() )
            {
                if( bChg )
                    pPam->Exchange();
                return true;
            }
    }
    if( bChg )
        pPam->Exchange();
    return false;
}

void SwFEShell::ProtectCells()
{
    SvxProtectItem aProt( RES_PROTECT );
    aProt.SetContentProtect( true );

    SET_CURR_SHELL( this );
    StartAllAction();

    GetDoc()->SetBoxAttr( *getShellCursor( false ), aProt );

    if( !IsCursorReadonly() )
    {
        if( IsTableMode() )
            ClearMark();
        ParkCursorInTab();
    }
    EndAllActionAndCall();
}

// cancel table selection
void SwFEShell::UnProtectCells()
{
    SET_CURR_SHELL( this );
    StartAllAction();

    SwSelBoxes aBoxes;
    if( IsTableMode() )
        ::GetTableSelCrs( *this, aBoxes );
    else
    {
        SwFrame *pFrame = GetCurrFrame();
        do {
            pFrame = pFrame->GetUpper();
        } while ( pFrame && !pFrame->IsCellFrame() );
        if( pFrame )
        {
            SwTableBox *pBox = const_cast<SwTableBox*>(static_cast<SwCellFrame*>(pFrame)->GetTabBox());
            aBoxes.insert( pBox );
        }
    }

    if( !aBoxes.empty() )
        GetDoc()->UnProtectCells( aBoxes );

    EndAllActionAndCall();
}

void SwFEShell::UnProtectTables()
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UnProtectTables( *GetCursor() );
    EndAllActionAndCall();
}

bool SwFEShell::HasTableAnyProtection( const OUString* pTableName,
                                     bool* pFullTableProtection )
{
    return GetDoc()->HasTableAnyProtection( GetCursor()->GetPoint(), pTableName,
                                        pFullTableProtection );
}

bool SwFEShell::CanUnProtectCells() const
{
    bool bUnProtectAvailable = false;
    const SwTableNode *pTableNd = IsCursorInTable();
    if( pTableNd && !pTableNd->IsProtect() )
    {
        SwSelBoxes aBoxes;
        if( IsTableMode() )
            ::GetTableSelCrs( *this, aBoxes );
        else
        {
            SwFrame *pFrame = GetCurrFrame();
            do {
                pFrame = pFrame->GetUpper();
            } while ( pFrame && !pFrame->IsCellFrame() );
            if( pFrame )
            {
                SwTableBox *pBox = const_cast<SwTableBox*>(static_cast<SwCellFrame*>(pFrame)->GetTabBox());
                aBoxes.insert( pBox );
            }
        }
        if( !aBoxes.empty() )
            bUnProtectAvailable = ::HasProtectedCells( aBoxes );
    }
    return bUnProtectAvailable;
}

sal_uInt16 SwFEShell::GetRowsToRepeat() const
{
    const SwFrame *pFrame = GetCurrFrame();
    const SwTabFrame *pTab = pFrame ? pFrame->FindTabFrame() : nullptr;
    if( pTab )
        return pTab->GetTable()->GetRowsToRepeat();
    return 0;
}

void SwFEShell::SetRowsToRepeat( sal_uInt16 nSet )
{
    SwFrame    *pFrame = GetCurrFrame();
    SwTabFrame *pTab = pFrame ? pFrame->FindTabFrame() : nullptr;
    if( pTab && pTab->GetTable()->GetRowsToRepeat() != nSet )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), true );
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetRowsToRepeat( *pTab->GetTable(), nSet );
        EndAllActionAndCall();
    }
}

// returns the number of rows consecutively selected from top
static sal_uInt16 lcl_GetRowNumber( const SwPosition& rPos )
{
    Point aTmpPt;
    const SwContentNode *pNd;
    const SwContentFrame *pFrame;

    std::pair<Point, bool> const tmp(aTmpPt, false);
    if( nullptr != ( pNd = rPos.nNode.GetNode().GetContentNode() ))
        pFrame = pNd->getLayoutFrame(pNd->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout(), &rPos, &tmp);
    else
        pFrame = nullptr;

    const SwFrame* pRow = (pFrame && pFrame->IsInTab()) ? pFrame->GetUpper() : nullptr;

    while (pRow && (!pRow->GetUpper() || !pRow->GetUpper()->IsTabFrame()))
        pRow = pRow->GetUpper();

    if (!pRow)
        return USHRT_MAX;

    const SwTabFrame* pTabFrame = static_cast<const SwTabFrame*>(pRow->GetUpper());
    const SwTableLine* pTabLine = static_cast<const SwRowFrame*>(pRow)->GetTabLine();
    sal_uInt16 nRet = USHRT_MAX;
    sal_uInt16 nI = 0;
    while ( sal::static_int_cast<SwTableLines::size_type>(nI) < pTabFrame->GetTable()->GetTabLines().size() )
    {
        if ( pTabFrame->GetTable()->GetTabLines()[ nI ] == pTabLine )
        {
            nRet = nI;
            break;
        }
        ++nI;
    }

    return nRet;
}

sal_uInt16 SwFEShell::GetRowSelectionFromTop() const
{
    sal_uInt16 nRet = 0;
    const SwPaM* pPaM = IsTableMode() ? GetTableCursor() : GetCursor_();
    const sal_uInt16 nPtLine = lcl_GetRowNumber( *pPaM->GetPoint() );

    if ( !IsTableMode() )
    {
        nRet = 0 == nPtLine ? 1 : 0;
    }
    else
    {
        const sal_uInt16 nMkLine = lcl_GetRowNumber( *pPaM->GetMark() );

        if ( ( nPtLine == 0 && nMkLine != USHRT_MAX ) ||
             ( nMkLine == 0 && nPtLine != USHRT_MAX ) )
        {
            nRet = std::max( nPtLine, nMkLine ) + 1;
        }
    }

    return nRet;
}

/*
 * 1. case: bRepeat = true
 * returns true if the current frame is located inside a table headline in
 * a follow frame
 *
 * 2. case: bRepeat = false
 * returns true if the current frame is located inside a table headline OR
 * inside the first line of a table!!!
 */
bool SwFEShell::CheckHeadline( bool bRepeat ) const
{
    bool bRet = false;
    if ( !IsTableMode() )
    {
        SwFrame *pFrame = GetCurrFrame();  // DONE MULTIIHEADER
        SwTabFrame* pTab = (pFrame && pFrame->IsInTab()) ? pFrame->FindTabFrame() : nullptr;
        if (pTab)
        {
            if ( bRepeat )
            {
                bRet = pTab->IsFollow() && pTab->IsInHeadline( *pFrame );
            }
            else
            {
                bRet = static_cast<SwLayoutFrame*>(pTab->Lower())->IsAnLower( pFrame ) ||
                        pTab->IsInHeadline( *pFrame );
            }
        }
    }
    return bRet;
}

void SwFEShell::AdjustCellWidth( bool bBalance, const bool bNoShrink, const bool bColumnWidth )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    // switch on wait-cursor, as we do not know how
    // much content is affected
    TableWait aWait(std::numeric_limits<size_t>::max(), nullptr,
                  *GetDoc()->GetDocShell());

    GetDoc()->AdjustCellWidth( *getShellCursor( false ), bBalance, bNoShrink, bColumnWidth );
    EndAllActionAndCall();
}

bool SwFEShell::IsAdjustCellWidthAllowed( bool bBalance ) const
{
    // at least one row with content should be contained in the selection

    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return false;

    SwSelBoxes aBoxes;
    ::GetTableSelCrs( *this, aBoxes );

    if ( bBalance )
        return aBoxes.size() > 1;

    if ( aBoxes.empty() )
    {
        do
        {
            pFrame = pFrame->GetUpper();
        }
        while (pFrame && !pFrame->IsCellFrame());

        if (!pFrame)
            return false;

        SwTableBox *pBox = const_cast<SwTableBox*>(static_cast<SwCellFrame*>(pFrame)->GetTabBox());
        aBoxes.insert( pBox );
    }

    for (size_t i = 0; i < aBoxes.size(); ++i)
    {
        SwTableBox *pBox = aBoxes[i];
        if ( pBox->GetSttNd() )
        {
            SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
            SwTextNode* pCNd = aIdx.GetNode().GetTextNode();
            if( !pCNd )
                pCNd = static_cast<SwTextNode*>(GetDoc()->GetNodes().GoNext( &aIdx ));

            while ( pCNd )
            {
                if (!pCNd->GetText().isEmpty())
                    return true;
                ++aIdx;
                pCNd = aIdx.GetNode().GetTextNode();
            }
        }
    }
    return false;
}

bool SwFEShell::SetTableStyle(const OUString& rStyleName)
{
    // make sure SwDoc has the style
    SwTableAutoFormat *pTableFormat = GetDoc()->GetTableStyles().FindAutoFormat(rStyleName);
    if (!pTableFormat)
        return false;

    SwTableNode *pTableNode = const_cast<SwTableNode*>(IsCursorInTable());
    if (!pTableNode)
        return false;

    // set the name & update
    return UpdateTableStyleFormatting(pTableNode, false, &rStyleName);
}

    // AutoFormat for the table/table selection
bool SwFEShell::SetTableStyle(const SwTableAutoFormat& rStyle)
{
    // make sure SwDoc has the style
    GetDoc()->GetTableStyles().AddAutoFormat(rStyle);

    SwTableNode *pTableNode = const_cast<SwTableNode*>(IsCursorInTable());
    if (!pTableNode)
        return false;

    // set the name & update
    return UpdateTableStyleFormatting(pTableNode, false, &rStyle.GetName());
}

bool SwFEShell::UpdateTableStyleFormatting(SwTableNode *pTableNode,
        bool bResetDirect, OUString const*const pStyleName)
{
    if (!pTableNode)
    {
        pTableNode = const_cast<SwTableNode*>(IsCursorInTable());
        if (!pTableNode || pTableNode->GetTable().IsTableComplex())
            return false;
    }

    OUString const aTableStyleName(pStyleName
            ? *pStyleName
            : pTableNode->GetTable().GetTableStyleName());
    SwTableAutoFormat* pTableStyle = GetDoc()->GetTableStyles().FindAutoFormat(aTableStyleName);
    if (!pTableStyle)
        return false;

    SwSelBoxes aBoxes;

    // whole table or only current selection
    if( IsTableMode() )
        ::GetTableSelCrs( *this, aBoxes );
    else
    {
        const SwTableSortBoxes& rTBoxes = pTableNode->GetTable().GetTabSortBoxes();
        for (size_t n = 0; n < rTBoxes.size(); ++n)
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.insert( pBox );
        }
    }

    bool bRet;
    if( !aBoxes.empty() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        bRet = GetDoc()->SetTableAutoFormat(
                aBoxes, *pTableStyle, bResetDirect, pStyleName != nullptr);
        ClearFEShellTabCols(*GetDoc(), nullptr);
        EndAllActionAndCall();
    }
    else
        bRet = false;
    return bRet;
}

bool SwFEShell::GetTableAutoFormat( SwTableAutoFormat& rGet )
{
    const SwTableNode *pTableNd = IsCursorInTable();
    if( !pTableNd || pTableNd->GetTable().IsTableComplex() )
        return false;

    SwSelBoxes aBoxes;

    if ( !IsTableMode() )       // if cursor are not current
        GetCursor();

    // whole table or only current selection
    if( IsTableMode() )
        ::GetTableSelCrs( *this, aBoxes );
    else
    {
        const SwTableSortBoxes& rTBoxes = pTableNd->GetTable().GetTabSortBoxes();
        for (size_t n = 0; n < rTBoxes.size(); ++n)
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.insert( pBox );
        }
    }

    return GetDoc()->GetTableAutoFormat( aBoxes, rGet );
}

bool SwFEShell::DeleteTableSel()
{
    // check if SPoint/Mark of current cursor are in a table
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return false;

    if( dynamic_cast< const SwDDETable* >(pFrame->ImplFindTabFrame()->GetTable()) != nullptr )
    {
        vcl::Window* pWin = GetWin();
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR, pWin ? pWin->GetFrameWeld() : nullptr,
                        DialogMask::MessageInfo | DialogMask::ButtonsOk );
        return false;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search boxes via the layout
    bool bRet;
    SwSelBoxes aBoxes;
    GetTableSelCrs( *this, aBoxes );
    if( !aBoxes.empty() )
    {
        TableWait aWait( aBoxes.size(), pFrame, *GetDoc()->GetDocShell() );

        // cursor should be removed from deletion area.
        // Put them behind/on the table; via the document
        // position they'll be set to the old position
        while( !pFrame->IsCellFrame() )
            pFrame = pFrame->GetUpper();
        ParkCursor( SwNodeIndex( *static_cast<SwCellFrame*>(pFrame)->GetTabBox()->GetSttNd() ));

        bRet = GetDoc()->DeleteRowCol( aBoxes );

        ClearFEShellTabCols(*GetDoc(), nullptr);
    }
    else
        bRet = false;
    EndAllActionAndCall();
    return bRet;
}

size_t SwFEShell::GetCurTabColNum() const
{
    //!!!GetCurMouseTabColNum() mitpflegen!!!!
    SwFrame *pFrame = GetCurrFrame();
    OSL_ENSURE( pFrame, "Cursor parked?" );

    // check if SPoint/Mark of current cursor are in a table
    if (!pFrame || !pFrame->IsInTab())
        return 0;

    do
    {
        // JP 26.09.95: why compare with ContentFrame
        //              and not with CellFrame ????
        pFrame = pFrame->GetUpper();
    } while (pFrame && !pFrame->IsCellFrame());

    if (!pFrame)
        return 0;

    size_t nRet = 0;

    SwRectFnSet aRectFnSet(pFrame);

    const SwPageFrame* pPage = pFrame->FindPageFrame();

    // get TabCols, as only via these we get to the position
    SwTabCols aTabCols;
    GetTabCols( aTabCols );

    if( pFrame->FindTabFrame()->IsRightToLeft() )
    {
        long nX = aRectFnSet.GetRight(pFrame->getFrameArea()) - aRectFnSet.GetLeft(pPage->getFrameArea());

        const long nRight = aTabCols.GetLeftMin() + aTabCols.GetRight();

        if ( !::IsSame( nX, nRight ) )
        {
            nX = nRight - nX + aTabCols.GetLeft();
            for ( size_t i = 0; i < aTabCols.Count(); ++i )
                if ( ::IsSame( nX, aTabCols[i] ) )
                {
                    nRet = i + 1;
                    break;
                }
        }
    }
    else
    {
        const long nX = aRectFnSet.GetLeft(pFrame->getFrameArea()) -
                        aRectFnSet.GetLeft(pPage->getFrameArea());

        const long nLeft = aTabCols.GetLeftMin();

        if ( !::IsSame( nX, nLeft + aTabCols.GetLeft() ) )
        {
            for ( size_t i = 0; i < aTabCols.Count(); ++i )
                if ( ::IsSame( nX, nLeft + aTabCols[i] ) )
                {
                    nRet = i + 1;
                    break;
                }
        }
    }
    return nRet;
}

static const SwFrame *lcl_FindFrameInTab( const SwLayoutFrame *pLay, const Point &rPt, SwTwips nFuzzy )
{
    const SwFrame *pFrame = pLay->Lower();

    while( pFrame && pLay->IsAnLower( pFrame ) )
    {
        if ( pFrame->getFrameArea().IsNear( rPt, nFuzzy ) )
        {
            if ( pFrame->IsLayoutFrame() )
            {
                const SwFrame *pTmp = ::lcl_FindFrameInTab( static_cast<const SwLayoutFrame*>(pFrame), rPt, nFuzzy );
                if ( pTmp )
                    return pTmp;
            }

            return pFrame;
        }

        pFrame = pFrame->FindNext();
    }

    return nullptr;
}

static const SwCellFrame *lcl_FindFrame( const SwLayoutFrame *pLay, const Point &rPt,
                              SwTwips nFuzzy, bool* pbRow, bool* pbCol )
{
    // bMouseMoveRowCols :
    // Method is called for
    // - Moving columns/rows with the mouse or
    // - Enhanced table selection
    const bool bMouseMoveRowCols = nullptr == pbCol;

    bool bCloseToRow = false;
    bool bCloseToCol = false;

    const SwFrame *pFrame = pLay->ContainsContent();
    const SwFrame* pRet = nullptr;

    if ( pFrame )
    {
        do
        {
            if ( pFrame->IsInTab() )
                pFrame = const_cast<SwFrame*>(pFrame)->ImplFindTabFrame();

            if (!pFrame)
                break;

            if ( pFrame->IsTabFrame() )
            {
                Point aPt( rPt );
                bool bSearchForFrameInTab = true;
                SwTwips nTmpFuzzy = nFuzzy;

                if ( !bMouseMoveRowCols )
                {
                    // We ignore nested tables for the enhanced table selection:
                    while ( pFrame->GetUpper()->IsInTab() )
                        pFrame = pFrame->GetUpper()->FindTabFrame();

                    // We first check if the given point is 'close' to the left or top
                    // border of the table frame:
                    OSL_ENSURE( pFrame, "Nested table frame without outer table" );
                    SwRectFnSet aRectFnSet(pFrame);
                    const bool bRTL = pFrame->IsRightToLeft();

                    SwRect aTabRect = pFrame->getFramePrintArea();
                    aTabRect.Pos() += pFrame->getFrameArea().Pos();

                    const SwTwips nLeft = bRTL ?
                                          aRectFnSet.GetRight(aTabRect) :
                                          aRectFnSet.GetLeft(aTabRect);
                    const SwTwips nTop  = aRectFnSet.GetTop(aTabRect);

                    SwTwips const rPointX = aRectFnSet.IsVert() ? aPt.Y() : aPt.X();
                    SwTwips const rPointY = aRectFnSet.IsVert() ? aPt.X() : aPt.Y();

                    const SwTwips nXDiff = aRectFnSet.XDiff( nLeft, rPointX ) * ( bRTL ? -1 : 1 );
                    const SwTwips nYDiff = aRectFnSet.YDiff( nTop, rPointY );

                    bCloseToRow = nXDiff >= 0 && nXDiff < nFuzzy;
                    bCloseToCol = nYDiff >= 0 && nYDiff < nFuzzy;

                    if ( bCloseToCol && 2 * nYDiff > nFuzzy )
                    {
                        const SwFrame* pPrev = pFrame->GetPrev();
                        if ( pPrev )
                        {
                            SwRect aPrevRect = pPrev->getFramePrintArea();
                            aPrevRect.Pos() += pPrev->getFrameArea().Pos();

                            if( aPrevRect.IsInside( rPt ) )
                            {
                                bCloseToCol = false;
                            }
                        }

                    }

                    // If we found the point to be 'close' to the left or top border
                    // of the table frame, we adjust the point to be on that border:
                    if ( bCloseToRow && bCloseToCol )
                        aPt = bRTL ? aTabRect.TopRight() : aRectFnSet.GetPos(aTabRect);
                    else if ( bCloseToRow )
                        aRectFnSet.IsVert() ? aPt.setY(nLeft) : aPt.setX(nLeft);
                    else if ( bCloseToCol )
                        aRectFnSet.IsVert() ? aPt.setX(nTop) : aPt.setY(nTop);

                    if ( !bCloseToRow && !bCloseToCol )
                        bSearchForFrameInTab = false;

                    // Since the point has been adjusted, we call lcl_FindFrameInTab()
                    // with a fuzzy value of 1:
                    nTmpFuzzy = 1;
                }

                const SwFrame* pTmp = bSearchForFrameInTab ?
                                    ::lcl_FindFrameInTab( static_cast<const SwLayoutFrame*>(pFrame), aPt, nTmpFuzzy ) :
                                    nullptr;

                if ( pTmp )
                {
                    pFrame = pTmp;
                    break;
                }
            }
            pFrame = pFrame->FindNextCnt();

        } while ( pFrame && pLay->IsAnLower( pFrame ) );
    }

    if ( pFrame && pFrame->IsInTab() && pLay->IsAnLower( pFrame ) )
    {
        do
        {
            // We allow mouse drag of table borders within nested tables,
            // but disallow hotspot selection of nested tables.
            if ( bMouseMoveRowCols )
            {
                // find the next cell frame
                while ( pFrame && !pFrame->IsCellFrame() )
                    pFrame = pFrame->GetUpper();
            }
            else
            {
                // find the most upper cell frame:
                while ( pFrame &&
                        ( !pFrame->IsCellFrame() ||
                          !pFrame->GetUpper()->GetUpper()->IsTabFrame() ||
                           pFrame->GetUpper()->GetUpper()->GetUpper()->IsInTab() ) )
                    pFrame = pFrame->GetUpper();
            }

            if ( pFrame ) // Note: this condition should be the same like the while condition!!!
            {
                // #i32329# Enhanced table selection
                // used for hotspot selection of tab/cols/rows
                if ( !bMouseMoveRowCols )
                {

                    OSL_ENSURE( pbCol && pbRow, "pbCol or pbRow missing" );

                    if ( bCloseToRow || bCloseToCol )
                    {
                        *pbRow = bCloseToRow;
                        *pbCol = bCloseToCol;
                        pRet = pFrame;
                        break;
                    }
                }
                else
                {
                    // used for mouse move of columns/rows
                    const SwTabFrame* pTabFrame = pFrame->FindTabFrame();
                    SwRect aTabRect = pTabFrame->getFramePrintArea();
                    aTabRect.Pos() += pTabFrame->getFrameArea().Pos();

                    SwRectFnSet aRectFnSet(pTabFrame);

                    const SwTwips nTabTop  = aRectFnSet.GetTop(aTabRect);
                    const SwTwips nMouseTop = aRectFnSet.IsVert() ? rPt.X() : rPt.Y();

                    // Do not allow to drag upper table border:
                    if ( !::IsSame( nTabTop, nMouseTop ) )
                    {
                        if ( ::IsSame( pFrame->getFrameArea().Left(), rPt.X() ) ||
                             ::IsSame( pFrame->getFrameArea().Right(),rPt.X() ) )
                        {
                            if ( pbRow ) *pbRow = false;
                            pRet = pFrame;
                            break;
                        }
                        if ( ::IsSame( pFrame->getFrameArea().Top(), rPt.Y() ) ||
                             ::IsSame( pFrame->getFrameArea().Bottom(),rPt.Y() ) )
                        {
                            if ( pbRow ) *pbRow = true;
                            pRet = pFrame;
                            break;
                        }
                    }
                }

                pFrame = pFrame->GetUpper();
            }
        } while ( pFrame );
    }

    // robust:
    OSL_ENSURE( !pRet || pRet->IsCellFrame(), "lcl_FindFrame() is supposed to find a cell frame!" );
    return pRet && pRet->IsCellFrame() ? static_cast<const SwCellFrame*>(pRet) : nullptr;
}

// pbCol  = 0 => Used for moving table rows/cols with mouse
// pbCol != 0 => Used for selecting table/rows/cols

#define ENHANCED_TABLE_SELECTION_FUZZY 10

const SwFrame* SwFEShell::GetBox( const Point &rPt, bool* pbRow, bool* pbCol ) const
{
    const SwPageFrame *pPage = static_cast<SwPageFrame*>(GetLayout()->Lower());
    vcl::Window* pOutWin = GetWin();
    SwTwips nFuzzy = COLFUZZY;
    if( pOutWin )
    {
        // #i32329# Enhanced table selection
        SwTwips nSize = pbCol ? ENHANCED_TABLE_SELECTION_FUZZY : RULER_MOUSE_MARGINWIDTH;
        Size aTmp( nSize, nSize );
        aTmp = pOutWin->PixelToLogic( aTmp );
        nFuzzy = aTmp.Width();
    }

    while ( pPage && !pPage->getFrameArea().IsNear( rPt, nFuzzy ) )
        pPage = static_cast<const SwPageFrame*>(pPage->GetNext());

    const SwCellFrame *pFrame = nullptr;
    if ( pPage )
    {
        // We cannot search the box by GetCursorOfst or GetContentPos.
        // This would lead to a performance collapse for documents
        // with a lot of paragraphs/tables on one page
        //(BrowseMode!)

        // check flys first
        if ( pPage->GetSortedObjs() )
        {
            for ( size_t i = 0; !pFrame && i < pPage->GetSortedObjs()->size(); ++i )
            {
                SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                if ( dynamic_cast<const SwFlyFrame*>( pObj) !=  nullptr )
                {
                    pFrame = lcl_FindFrame( static_cast<SwFlyFrame*>(pObj),
                                        rPt, nFuzzy, pbRow, pbCol );
                }
            }
        }
        const SwLayoutFrame *pLay = static_cast<const SwLayoutFrame*>(pPage->Lower());
        while ( pLay && !pFrame )
        {
            pFrame = lcl_FindFrame( pLay, rPt, nFuzzy, pbRow, pbCol );
            pLay = static_cast<const SwLayoutFrame*>(pLay->GetNext());
        }
    }
    return pFrame;
}

/* Helper function*/
/* calculated the distance between Point rC and Line Segment (rA, rB) */
static double lcl_DistancePoint2Segment( const Point& rA, const Point& rB, const Point& rC )
{
    double nRet = 0;

    const basegfx::B2DVector aBC( rC.X() - rB.X(), rC.Y() - rB.Y() );
    const basegfx::B2DVector aAB( rB.X() - rA.X(), rB.Y() - rA.Y() );
    const double nDot1 = aBC.scalar( aAB );

    if ( nDot1 > 0 ) // check outside case 1
        nRet = aBC.getLength();
    else
    {
        const basegfx::B2DVector aAC( rC.X() - rA.X(), rC.Y() - rA.Y() );
        const basegfx::B2DVector aBA( rA.X() - rB.X(), rA.Y() - rB.Y() );
        const double nDot2 = aAC.scalar( aBA );

        if ( nDot2 > 0 ) // check outside case 2
            nRet = aAC.getLength();
        else
        {
            const double nDiv = aAB.getLength();
            nRet = nDiv ? aAB.cross( aAC ) / nDiv : 0;
        }
    }

    return std::abs(nRet);
}

/* Helper function*/
static Point lcl_ProjectOntoClosestTableFrame( const SwTabFrame& rTab, const Point& rPoint, bool bRowDrag )
{
    Point aRet( rPoint );
    const SwTabFrame* pCurrentTab = &rTab;
    const bool bVert = pCurrentTab->IsVertical();
    const bool bRTL = pCurrentTab->IsRightToLeft();

    // Western Layout:
    // bRowDrag = true => compare to left border of table
    // bRowDrag = false => compare to top border of table

    // Asian Layout:
    // bRowDrag = true => compare to right border of table
    // bRowDrag = false => compare to top border of table

    // RTL Layout:
    // bRowDrag = true => compare to right border of table
    // bRowDrag = false => compare to top border of table
    bool bLeft = false;
    bool bRight = false;

    if ( bRowDrag )
    {
        if ( bVert || bRTL )
            bRight = true;
        else
            bLeft = true;
    }

    // used to find the minimal distance
    double nMin = -1;
    Point aMin1;
    Point aMin2;

    Point aS1;
    Point aS2;

    while ( pCurrentTab )
    {
        SwRect aTabRect( pCurrentTab->getFramePrintArea() );
        aTabRect += pCurrentTab->getFrameArea().Pos();

        if ( bLeft )
        {
            // distance to left table border
            aS1 = aTabRect.TopLeft();
            aS2 = aTabRect.BottomLeft();
        }
        else if ( bRight )
        {
            // distance to right table border
            aS1 = aTabRect.TopRight();
            aS2 = aTabRect.BottomRight();
        }
        else //if ( bTop )
        {
            // distance to top table border
            aS1 = aTabRect.TopLeft();
            aS2 = aTabRect.TopRight();
        }

        const double nDist = lcl_DistancePoint2Segment( aS1, aS2, rPoint );

        if ( nDist < nMin || -1 == nMin )
        {
            aMin1 = aS1;
            aMin2 = aS2;
            nMin = nDist;
        }

        pCurrentTab = pCurrentTab->GetFollow();
    }

    // project onto closest line:
    if ( bLeft || bRight )
    {
        aRet.setX(aMin1.getX());
        if ( aRet.getY() > aMin2.getY() )
            aRet.setY(aMin2.getY());
        else if ( aRet.getY() < aMin1.getY() )
            aRet.setY(aMin1.getY());
    }
    else
    {
        aRet.setY(aMin1.getY());
        if ( aRet.getX() > aMin2.getX() )
            aRet.setX(aMin2.getX());
        else if ( aRet.getX() < aMin1.getX() )
            aRet.setX(aMin1.getX());
    }

    return aRet;
}

// #i32329# Enhanced table selection
bool SwFEShell::SelTableRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag )
{
    bool bRet = false;
    Point aEndPt;
    if ( pEnd )
        aEndPt = *pEnd;

    SwPosition*  ppPos[2] = { nullptr, nullptr };
    Point        paPt [2] = { rPt, aEndPt };
    bool         pbRow[2] = { false, false };
    bool         pbCol[2] = { false, false };

    // pEnd is set during dragging.
    for ( sal_uInt16 i = 0; i < ( pEnd ? 2 : 1 ); ++i )
    {
        const SwCellFrame* pFrame =
             static_cast<const SwCellFrame*>(GetBox( paPt[i], &pbRow[i], &pbCol[i] ) );

        if( pFrame )
        {
            while( pFrame && pFrame->Lower() && pFrame->Lower()->IsRowFrame() )
                pFrame = static_cast<const SwCellFrame*>( static_cast<const SwLayoutFrame*>( pFrame->Lower() )->Lower() );
            if( pFrame && pFrame->GetTabBox()->GetSttNd() &&
                pFrame->GetTabBox()->GetSttNd()->IsInProtectSect() )
                pFrame = nullptr;
        }

        if ( pFrame )
        {
            const SwContentFrame* pContent = ::GetCellContent( *pFrame );

            if ( pContent && pContent->IsTextFrame() )
            {

                ppPos[i] = new SwPosition(static_cast<SwTextFrame const*>(pContent)->MapViewToModelPos(TextFrameIndex(0)));

                // paPt[i] will not be used any longer, now we use it to store
                // a position inside the content frame
                paPt[i] = pContent->getFrameArea().Center();
            }
        }

        // no calculation of end frame if start frame has not been found.
        if ( 1 == i || !ppPos[0] || !pEnd || !pFrame )
            break;

        // find 'closest' table frame to pEnd:
        const SwTabFrame* pCurrentTab = pFrame->FindTabFrame();
        if ( pCurrentTab->IsFollow() )
            pCurrentTab = pCurrentTab->FindMaster( true );

        const Point aProjection = lcl_ProjectOntoClosestTableFrame( *pCurrentTab, *pEnd, bRowDrag );
        paPt[1] = aProjection;
    }

    if ( ppPos[0] )
    {
        SwShellCursor* pCursor = GetCursor_();
        SwCursorSaveState aSaveState( *pCursor );
        SwPosition aOldPos( *pCursor->GetPoint() );

        pCursor->DeleteMark();
        *pCursor->GetPoint() = *ppPos[0];
        pCursor->GetPtPos() = paPt[0];

        if ( !pCursor->IsInProtectTable() )
        {
            bool bNewSelection = true;

            if ( ppPos[1] )
            {
                if ( ppPos[1]->nNode.GetNode().StartOfSectionNode() !=
                     aOldPos.nNode.GetNode().StartOfSectionNode() )
                {
                    pCursor->SetMark();
                    SwCursorSaveState aSaveState2( *pCursor );
                    *pCursor->GetPoint() = *ppPos[1];
                    pCursor->GetPtPos() = paPt[1];

                    if ( pCursor->IsInProtectTable( false, false ) )
                    {
                        pCursor->RestoreSavePos();
                        bNewSelection = false;
                    }
                }
                else
                {
                    pCursor->RestoreSavePos();
                    bNewSelection = false;
                }
            }

            if ( bNewSelection )
            {
                // #i35543# SelTableRowCol should remove any existing
                // table cursor:
                if ( IsTableMode() )
                    TableCursorToCursor();

                if ( pbRow[0] && pbCol[0] )
                    bRet = SwCursorShell::SelTable();
                else if ( pbRow[0] )
                    bRet = SwCursorShell::SelTableRowOrCol( true, true );
                else if ( pbCol[0] )
                    bRet = SwCursorShell::SelTableRowOrCol( false, true );
            }
            else
                bRet = true;
        }

        delete ppPos[0];
        delete ppPos[1];
    }

    return bRet;
}

SwTab SwFEShell::WhichMouseTabCol( const Point &rPt ) const
{
    SwTab nRet = SwTab::COL_NONE;
    bool bRow = false;
    bool bCol = false;
    bool bSelect = false;

    // First try: Do we get the row/col move cursor?
    const SwCellFrame* pFrame = static_cast<const SwCellFrame*>(GetBox( rPt, &bRow ));

    if ( !pFrame )
    {
        // Second try: Do we get the row/col/tab selection cursor?
        pFrame = static_cast<const SwCellFrame*>(GetBox( rPt, &bRow, &bCol ));
        bSelect = true;
    }

    if( pFrame )
    {
        while( pFrame && pFrame->Lower() && pFrame->Lower()->IsRowFrame() )
            pFrame = static_cast<const SwCellFrame*>(static_cast<const SwLayoutFrame*>(pFrame->Lower())->Lower());
        if( pFrame && pFrame->GetTabBox()->GetSttNd() &&
            pFrame->GetTabBox()->GetSttNd()->IsInProtectSect() )
            pFrame = nullptr;
    }

    if( pFrame )
    {
        if ( !bSelect )
        {
            if ( pFrame->IsVertical() )
                nRet = bRow ? SwTab::COL_VERT : SwTab::ROW_VERT;
            else
                nRet = bRow ? SwTab::ROW_HORI : SwTab::COL_HORI;
        }
        else
        {
            const SwTabFrame* pTabFrame = pFrame->FindTabFrame();
            if ( pTabFrame->IsVertical() )
            {
                if ( bRow && bCol )
                {
                    nRet = SwTab::SEL_VERT;
                }
                else if ( bRow )
                {
                    nRet = SwTab::ROWSEL_VERT;
                }
                else if ( bCol )
                {
                    nRet = SwTab::COLSEL_VERT;
                }
            }
            else
            {
                if ( bRow && bCol )
                {
                    nRet =  pTabFrame->IsRightToLeft() ?
                            SwTab::SEL_HORI_RTL :
                            SwTab::SEL_HORI;
                }
                else if ( bRow )
                {
                    nRet = pTabFrame->IsRightToLeft() ?
                           SwTab::ROWSEL_HORI_RTL :
                           SwTab::ROWSEL_HORI;
                }
                else if ( bCol )
                {
                    nRet = SwTab::COLSEL_HORI;
                }
            }
        }
    }

    return nRet;
}

// -> #i23726#
SwTextNode * SwFEShell::GetNumRuleNodeAtPos( const Point &rPt)
{
    SwTextNode * pResult = nullptr;

    SwContentAtPos aContentAtPos(IsAttrAtPos::NumLabel);

    if( GetContentAtPos(rPt, aContentAtPos) && aContentAtPos.aFnd.pNode)
        pResult = aContentAtPos.aFnd.pNode->GetTextNode();

    return pResult;
}

bool SwFEShell::IsNumLabel( const Point &rPt, int nMaxOffset )
{
    bool bResult = false;

    SwContentAtPos aContentAtPos(IsAttrAtPos::NumLabel);

    if( GetContentAtPos(rPt, aContentAtPos))
    {
        if ((nMaxOffset >= 0 && aContentAtPos.nDist <= nMaxOffset) ||
            (nMaxOffset < 0))
            bResult = true;
    }

    return bResult;
}
// <- #i23726#

// #i42921#
bool SwFEShell::IsVerticalModeAtNdAndPos( const SwTextNode& _rTextNode,
                                          const Point& _rDocPos )
{
    bool bRet( false );

    const SvxFrameDirection nTextDir =
        _rTextNode.GetTextDirection( SwPosition(_rTextNode), &_rDocPos );
    switch ( nTextDir )
    {
        case SvxFrameDirection::Unknown:
        case SvxFrameDirection::Horizontal_RL_TB:
        case SvxFrameDirection::Horizontal_LR_TB:
        {
            bRet = false;
        }
        break;
        case SvxFrameDirection::Vertical_LR_TB:
        case SvxFrameDirection::Vertical_RL_TB:
        {
            bRet = true;
        }
        break;
        default: break;
    }

    return bRet;
}

void SwFEShell::GetMouseTabCols( SwTabCols &rToFill, const Point &rPt ) const
{
    const SwFrame *pBox = GetBox( rPt );
    if ( pBox )
        GetTabCols_( rToFill, pBox );
}

void SwFEShell::SetMouseTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                                 const Point &rPt )
{
    const SwFrame *pBox = GetBox( rPt );
    if( pBox )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetTabCols( rNew, bCurRowOnly, static_cast<const SwCellFrame*>(pBox) );
        EndAllActionAndCall();
    }
}

sal_uInt16 SwFEShell::GetCurMouseColNum( const Point &rPt ) const
{
    return GetCurColNum_( GetBox( rPt ), nullptr );
}

size_t SwFEShell::GetCurMouseTabColNum( const Point &rPt ) const
{
    //!!!GetCurTabColNum() mitpflegen!!!!
    size_t nRet = 0;

    const SwFrame *pFrame = GetBox( rPt );
    OSL_ENSURE( pFrame, "Table not found" );
    if( pFrame )
    {
        const long nX = pFrame->getFrameArea().Left();

        // get TabCols, only via these we get the position
        SwTabCols aTabCols;
        GetMouseTabCols( aTabCols, rPt );

        const long nLeft = aTabCols.GetLeftMin();

        if ( !::IsSame( nX, nLeft + aTabCols.GetLeft() ) )
        {
            for ( size_t i = 0; i < aTabCols.Count(); ++i )
                if ( ::IsSame( nX, nLeft + aTabCols[i] ) )
                {
                    nRet = i + 1;
                    break;
                }
        }
    }
    return nRet;
}

void ClearFEShellTabCols(SwDoc & rDoc, SwTabFrame const*const pFrame)
{
    auto const pShell(rDoc.getIDocumentLayoutAccess().GetCurrentViewShell());
    if (pShell)
    {
        for (SwViewShell& rCurrentShell : pShell->GetRingContainer())
        {
            if (auto const pFE = dynamic_cast<SwFEShell *>(&rCurrentShell))
            {
                pFE->ClearColumnRowCache(pFrame);
            }
        }
    }
}

void SwFEShell::ClearColumnRowCache(SwTabFrame const*const pFrame)
{
    if (m_pColumnCache)
    {
        if (pFrame == nullptr || pFrame == m_pColumnCache->pLastTabFrame)
        {
            m_pColumnCache.reset();
        }
    }
    if (m_pRowCache)
    {
        if (pFrame == nullptr || pFrame == m_pRowCache->pLastTabFrame)
        {
            m_pRowCache.reset();
        }
    }
}

void SwFEShell::GetTableAttr( SfxItemSet &rSet ) const
{
    SwFrame *pFrame = GetCurrFrame();
    if( pFrame && pFrame->IsInTab() )
        rSet.Put( pFrame->ImplFindTabFrame()->GetFormat()->GetAttrSet() );
}

void SwFEShell::SetTableAttr( const SfxItemSet &rNew )
{
    SwFrame *pFrame = GetCurrFrame();
    if( pFrame && pFrame->IsInTab() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        SwTabFrame *pTab = pFrame->FindTabFrame();
        pTab->GetTable()->SetHTMLTableLayout(std::shared_ptr<SwHTMLTableLayout>());
        GetDoc()->SetAttr( rNew, *pTab->GetFormat() );
        GetDoc()->getIDocumentState().SetModified();
        EndAllActionAndCall();
    }
}

// change a cell width/cell height/column width/row height
void SwFEShell::SetColRowWidthHeight( TableChgWidthHeightType eType, sal_uInt16 nDiff )
{
    SwFrame *pFrame = GetCurrFrame();
    if( !pFrame || !pFrame->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pFrame = pFrame->GetUpper();
    } while( !pFrame->IsCellFrame() );

    SwTabFrame *pTab = pFrame->ImplFindTabFrame();

    // if the table is in relative values (USHRT_MAX)
    // then it should be recalculated to absolute values now
    const SwFormatFrameSize& rTableFrameSz = pTab->GetFormat()->GetFrameSize();
    SwRectFnSet aRectFnSet(pTab);
    long nPrtWidth = aRectFnSet.GetWidth(pTab->getFramePrintArea());
    TableChgWidthHeightType eTypePos = extractPosition(eType);
    if( TableChgMode::VarWidthChangeAbs == pTab->GetTable()->GetTableChgMode() &&
        ( eTypePos == TableChgWidthHeightType::ColLeft || eTypePos == TableChgWidthHeightType::ColRight ) &&
        text::HoriOrientation::NONE == pTab->GetFormat()->GetHoriOrient().GetHoriOrient() &&
        nPrtWidth != rTableFrameSz.GetWidth() )
    {
        SwFormatFrameSize aSz( rTableFrameSz );
        aSz.SetWidth( pTab->getFramePrintArea().Width() );
        pTab->GetFormat()->SetFormatAttr( aSz );
    }

    SwTwips nLogDiff = nDiff;
    nLogDiff *= pTab->GetFormat()->GetFrameSize().GetWidth();
    nLogDiff /= nPrtWidth;

    /** The cells are destroyed in here */
    GetDoc()->SetColRowWidthHeight(
                    *const_cast<SwTableBox*>(static_cast<SwCellFrame*>(pFrame)->GetTabBox()),
                    eType, nDiff, nLogDiff );

    ClearFEShellTabCols(*GetDoc(), nullptr);
    EndAllActionAndCall();
}

static bool lcl_IsFormulaSelBoxes( const SwTable& rTable, const SwTableBoxFormula& rFormula,
                            SwCellFrames& rCells )
{
    SwTableBoxFormula aTmp( rFormula );
    SwSelBoxes aBoxes;
    aTmp.GetBoxesOfFormula(rTable, aBoxes);
    for (size_t nSelBoxes = aBoxes.size(); nSelBoxes; )
    {
        SwTableBox* pBox = aBoxes[ --nSelBoxes ];

        if( std::none_of(rCells.begin(), rCells.end(), [&pBox](SwCellFrame* pFrame) { return pFrame->GetTabBox() == pBox; }) )
            return false;
    }

    return true;
}

    // ask formula for auto-sum
void SwFEShell::GetAutoSum( OUString& rFormula ) const
{
    SwFrame *pFrame = GetCurrFrame();
    SwTabFrame *pTab = pFrame ? pFrame->ImplFindTabFrame() : nullptr;
    if( !pTab )
        return;

    SwCellFrames aCells;
    OUString sFields;
    if( ::GetAutoSumSel( *this, aCells ))
    {
        sal_uInt16 nW = 0;
        for( size_t n = aCells.size(); n; )
        {
            SwCellFrame* pCFrame = aCells[ --n ];
            sal_uInt16 nBoxW = pCFrame->GetTabBox()->IsFormulaOrValueBox();
            if( !nBoxW )
                break;

            if( !nW )
            {
                if( USHRT_MAX == nBoxW )
                    continue;       // skip space at beginning

                // formula only if box is contained
                if( RES_BOXATR_FORMULA == nBoxW &&
                    !::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrame->
                    GetTabBox()->GetFrameFormat()->GetTableBoxFormula(), aCells))
                {
                    nW = RES_BOXATR_VALUE;
                    // restore previous spaces!
                    for( size_t i = aCells.size(); n+1 < i; )
                    {
                        sFields = "|<" + aCells[--i]->GetTabBox()->GetName() + ">"
                            + sFields;
                    }
                }
                else
                    nW = nBoxW;
            }
            else if( RES_BOXATR_VALUE == nW )
            {
                // search for values, Value/Formula/Text found -> include
                if( RES_BOXATR_FORMULA == nBoxW &&
                    ::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrame->
                        GetTabBox()->GetFrameFormat()->GetTableBoxFormula(), aCells ))
                    break;
                else if( USHRT_MAX != nBoxW )
                    sFields = OUStringLiteral1(cListDelim) + sFields;
                else
                    break;
            }
            else if( RES_BOXATR_FORMULA == nW )
            {
                // only continue search when the current formula points to
                // all boxes contained in the selection
                if( RES_BOXATR_FORMULA == nBoxW )
                {
                    if( !::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrame->
                        GetTabBox()->GetFrameFormat()->GetTableBoxFormula(), aCells ))
                    {
                        // redo only for values!

                        nW = RES_BOXATR_VALUE;
                        sFields.clear();
                        // restore previous spaces!
                        for( size_t i = aCells.size(); n+1 < i; )
                        {
                            sFields = "|<" + aCells[--i]->GetTabBox()->GetName() + ">"
                                + sFields;
                        }
                    }
                    else
                        sFields = OUStringLiteral1(cListDelim) + sFields;
                }
                else if( USHRT_MAX == nBoxW )
                    break;
                else
                    continue;       // ignore this box
            }
            else
                // all other stuff terminates the loop
                // possibly allow texts??
                break;

            sFields = "<" + pCFrame->GetTabBox()->GetName() + ">" + sFields;
        }
    }

    rFormula = OUString::createFromAscii( sCalc_Sum );
    if (!sFields.isEmpty())
    {
        rFormula += "(" + sFields + ")";
    }
}

bool SwFEShell::IsTableRightToLeft() const
{
    SwFrame *pFrame = GetCurrFrame();
    SwTabFrame *pTab = (pFrame && pFrame->IsInTab()) ? pFrame->ImplFindTabFrame() : nullptr;
    if (!pTab)
        return false;
    return pTab->IsRightToLeft();
}

bool SwFEShell::IsMouseTableRightToLeft(const Point &rPt) const
{
    SwFrame *pFrame = const_cast<SwFrame *>(GetBox( rPt ));
    const SwTabFrame*  pTabFrame = pFrame ? pFrame->ImplFindTabFrame() : nullptr;
    OSL_ENSURE( pTabFrame, "Table not found" );
    return pTabFrame && pTabFrame->IsRightToLeft();
}

bool SwFEShell::IsTableVertical() const
{
    SwFrame *pFrame = GetCurrFrame();
    SwTabFrame *pTab = (pFrame && pFrame->IsInTab()) ? pFrame->ImplFindTabFrame() : nullptr;
    if (!pTab)
        return false;
    return pTab->IsVertical();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
