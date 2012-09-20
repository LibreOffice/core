/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <hintids.hxx>

#include <tools/errinf.hxx>
#include <vcl/svapp.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <svx/svxids.hrc>
#include <editeng/protitem.hxx>
#include <editeng/brshitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svtools/ruler.hxx>
#include <swwait.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <frmatr.hxx>
#include <docary.hxx>
#include <fesh.hxx>
#include <doc.hxx>
#include <cntfrm.hxx>
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
#include <cellatr.hxx>
#include <pam.hxx>
#include <viscrs.hxx>
#include <tblsel.hxx>
#include <swtblfmt.hxx>
#include <swerror.h>
#include <swundo.hxx>
#include <frmtool.hxx>

#include <node.hxx> // #i23726#
// OD 2004-05-24 #i28701#
#include <sortedobjs.hxx>

using namespace ::com::sun::star;


// also see swtable.cxx
#define COLFUZZY 20L

inline sal_Bool IsSame( long nA, long nB ) { return  Abs(nA-nB) <= COLFUZZY; }
inline sal_Bool IsNear( long nA, long nB, long nTolerance ) { return Abs( nA - nB ) <= nTolerance; }

// table column cache
SwTabCols *pLastCols   = 0;
const SwTable   *pColumnCacheLastTable  = 0;
const SwTabFrm  *pColumnCacheLastTabFrm = 0;
const SwFrm     *pColumnCacheLastCellFrm = 0;

// table row cache
SwTabCols *pLastRows   = 0;
const SwTable   *pRowCacheLastTable  = 0;
const SwTabFrm  *pRowCacheLastTabFrm = 0;
const SwFrm     *pRowCacheLastCellFrm = 0;


class TblWait
{
    SwWait *pWait;
public:
    TblWait( sal_uInt16 nCnt, SwFrm *pFrm, SwDocShell &rDocShell, sal_uInt16 nCnt2 = 0);
    ~TblWait() { delete pWait; }
};

TblWait::TblWait( sal_uInt16 nCnt, SwFrm *pFrm, SwDocShell &rDocShell, sal_uInt16 nCnt2):
    pWait( 0 )
{
    sal_Bool bWait = 20 < nCnt || 20 < nCnt2 || (pFrm &&
                 20 < pFrm->ImplFindTabFrm()->GetTable()->GetTabLines().size());
    if( bWait )
        pWait = new SwWait( rDocShell, sal_True );
}


void SwFEShell::ParkCursorInTab()
{
    SwCursor * pSwCrsr = GetSwCrsr();

    OSL_ENSURE(pSwCrsr, "no SwCursor");

    SwPosition aStartPos = *pSwCrsr->GetPoint(), aEndPos = aStartPos;

    SwCursor * pTmpCrsr = (SwCursor *) pSwCrsr;

    /* Search least and greatest position in current cursor ring.
     */
    do
    {
        const SwPosition * pPt = pTmpCrsr->GetPoint(),
            * pMk = pTmpCrsr->GetMark();

        if (*pPt < aStartPos)
            aStartPos = *pPt;

        if (*pPt > aEndPos)
            aEndPos = *pPt;

        if (*pMk < aStartPos)
            aStartPos = *pMk;

        if (*pMk > aEndPos)
            aEndPos = *pMk;

        pTmpCrsr = (SwCursor *) pTmpCrsr->GetNext();
    }
    while (pTmpCrsr != pSwCrsr);

    KillPams();

    /* @@@ semantic: SwCursor::operator=() is not implemented @@@ */

    /* Set cursor to end of selection to ensure IsLastCellInRow works
       properly. */
    {
        SwCursor aTmpCrsr( aEndPos, 0, false );
        *pSwCrsr = aTmpCrsr;
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
            SwCursor aTmpCrsr( aStartPos, 0, false );
            *pSwCrsr = aTmpCrsr;
        }

        if (! pSwCrsr->GoPrevCell())
        {
            SwCursor aTmpCrsr( aEndPos, 0, false );
            *pSwCrsr = aTmpCrsr;
            pSwCrsr->GoNextCell();
        }
    }
    else
    {
        /* If the cursor is not in the last row of the table, first
           try to move it to the next cell. If that fails move it
           to the previous cell. */

        {
            SwCursor aTmpCrsr( aEndPos, 0, false );
            *pSwCrsr = aTmpCrsr;
        }

        if (! pSwCrsr->GoNextCell())
        {
            SwCursor aTmpCrsr( aStartPos, 0, false );
            *pSwCrsr = aTmpCrsr;
            pSwCrsr->GoPrevCell();
        }
    }
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  InsertRow(), InsertCol
#***********************************************************************/
sal_Bool SwFEShell::InsertRow( sal_uInt16 nCnt, sal_Bool bBehind )
{
    // check if Point/Mark of current cursor are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search boxes via the layout
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );

    TblWait( nCnt, pFrm, *GetDoc()->GetDocShell(), aBoxes.size() );

    sal_Bool bRet = sal_False;
    if ( aBoxes.size() )
        bRet = GetDoc()->InsertRow( aBoxes, nCnt, bBehind );

    EndAllActionAndCall();
    return bRet;
}

sal_Bool SwFEShell::InsertCol( sal_uInt16 nCnt, sal_Bool bBehind )
{
    // check if Point/Mark of current cursor are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );

    if( !CheckSplitCells( *this, nCnt + 1, nsSwTblSearchType::TBLSEARCH_COL ) )
    {
        ErrorHandler::HandleError( ERR_TBLINSCOL_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    StartAllAction();
    // search boxes via the layout
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );

    TblWait( nCnt, pFrm, *GetDoc()->GetDocShell(), aBoxes.size() );

    sal_Bool bRet = sal_False;
    if( !aBoxes.empty() )
        bRet = GetDoc()->InsertCol( aBoxes, nCnt, bBehind );

    EndAllActionAndCall();
    return bRet;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  DeleteRow(), DeleteCol()
#***********************************************************************/

/**
   Determines if the current cursor is in the last row of the table.
*/
sal_Bool SwFEShell::IsLastCellInRow() const
{
    SwTabCols aTabCols;
    GetTabCols( aTabCols );
    sal_Bool bResult = sal_False;

    if (IsTableRightToLeft())
        /* If the table is right-to-left the last row is the most left one. */
        bResult = 0 == GetCurTabColNum();
    else
        /* If the table is left-to-right the last row is the most right one. */
        bResult = aTabCols.Count() == GetCurTabColNum();

    return bResult;
}

sal_Bool SwFEShell::DeleteCol()
{
    // check if Point/Mark of current cursor are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search boxes via the layout
    sal_Bool bRet;
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );
    if ( !aBoxes.empty() )
    {
        TblWait( aBoxes.size(), pFrm, *GetDoc()->GetDocShell() );

        // remove crsr from the deletion area.
        // Put them behind/on the table; via the
        // document position they will be put to the old position
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();

        ParkCursorInTab();

        // then delete the column
        StartUndo(UNDO_COL_DELETE);
        bRet = GetDoc()->DeleteRowCol( aBoxes, true );
        EndUndo(UNDO_COL_DELETE);

    }
    else
        bRet = sal_False;

    EndAllActionAndCall();
    return bRet;
}

sal_Bool SwFEShell::DeleteRow()
{
    // check if Point/Mark of current cursor are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search for boxes via the layout
    sal_Bool bRet;
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );

    if( !aBoxes.empty() )
    {
        TblWait( aBoxes.size(), pFrm, *GetDoc()->GetDocShell() );

        // Delete cursors from the deletion area.
        // Then the cursor is:
        //  1. the following row, if there is another row after this
        //  2. the preceding row, if there is another row before this
        //  3. otherwise below the table
        {
            SwTableNode* pTblNd = ((SwCntntFrm*)pFrm)->GetNode()->FindTableNode();

            // search all boxes / lines
            _FndBox aFndBox( 0, 0 );
            {
                _FndPara aPara( aBoxes, &aFndBox );
                ForEach_FndLineCopyCol( pTblNd->GetTable().GetTabLines(), &aPara );
            }

            if( aFndBox.GetLines().empty() )
            {
                EndAllActionAndCall();
                return sal_False;
            }

            KillPams();

            _FndBox* pFndBox = &aFndBox;
            while( 1 == pFndBox->GetLines().size() &&
                    1 == pFndBox->GetLines().front().GetBoxes().size() )
            {
                _FndBox* pTmp = &pFndBox->GetLines().front().GetBoxes()[0];
                if( pTmp->GetBox()->GetSttNd() )
                    break;      // otherwise too far
                pFndBox = pTmp;
            }

            SwTableLine* pDelLine = pFndBox->GetLines().back().GetLine();
            SwTableBox* pDelBox = pDelLine->GetTabBoxes().back();
            while( !pDelBox->GetSttNd() )
            {
                SwTableLine* pLn = pDelBox->GetTabLines().back();
                pDelBox = pLn->GetTabBoxes().back();
            }
            SwTableBox* pNextBox = pDelLine->FindNextBox( pTblNd->GetTable(),
                                                            pDelBox, sal_True );
            while( pNextBox &&
                    pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                pNextBox = pNextBox->FindNextBox( pTblNd->GetTable(), pNextBox );

            if( !pNextBox )         // no next? then the previous
            {
                pDelLine = pFndBox->GetLines().front().GetLine();
                pDelBox = pDelLine->GetTabBoxes()[ 0 ];
                while( !pDelBox->GetSttNd() )
                    pDelBox = pDelBox->GetTabLines()[0]->GetTabBoxes()[0];
                pNextBox = pDelLine->FindPreviousBox( pTblNd->GetTable(),
                                                            pDelBox, sal_True );
                while( pNextBox &&
                        pNextBox->GetFrmFmt()->GetProtect().IsCntntProtected() )
                    pNextBox = pNextBox->FindPreviousBox( pTblNd->GetTable(), pNextBox );
            }

            sal_uLong nIdx;
            if( pNextBox )      // put cursor here
                nIdx = pNextBox->GetSttIdx() + 1;
            else                // otherwise below the table
                nIdx = pTblNd->EndOfSectionIndex() + 1;

            SwNodeIndex aIdx( GetDoc()->GetNodes(), nIdx );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
                pCNd = GetDoc()->GetNodes().GoNext( &aIdx );

            if( pCNd )
            {
                SwPaM* pPam = GetCrsr();
                pPam->GetPoint()->nNode = aIdx;
                pPam->GetPoint()->nContent.Assign( pCNd, 0 );
                pPam->SetMark();            // both want something
                pPam->DeleteMark();
            }
        }

        // now delete the lines
        StartUndo(UNDO_ROW_DELETE);
        bRet = GetDoc()->DeleteRowCol( aBoxes );
        EndUndo(UNDO_ROW_DELETE);
    }
    else
        bRet = sal_False;

    EndAllActionAndCall();
    return bRet;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  MergeTab(), SplitTab()
#***********************************************************************/

sal_uInt16 SwFEShell::MergeTab()
{
    // check if Point/Mark of current cursor are in a table
    sal_uInt16 nRet = TBLMERGE_NOSELECTION;
    if( IsTableMode() )
    {
        SwShellTableCrsr* pTableCrsr = GetTableCrsr();
        const SwTableNode* pTblNd = pTableCrsr->GetNode()->FindTableNode();
        if( pTblNd->GetTable().ISA( SwDDETable ))
        {
            ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                            ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        }
        else
        {
            SET_CURR_SHELL( this );
            StartAllAction();

            TblWait( pTableCrsr->GetBoxesCount(), 0, *GetDoc()->GetDocShell(),
                     pTblNd->GetTable().GetTabLines().size() );

            nRet = GetDoc()->MergeTbl( *pTableCrsr );

            KillPams();

            EndAllActionAndCall();
        }
    }
    return nRet;
}

sal_Bool SwFEShell::SplitTab( sal_Bool bVert, sal_uInt16 nCnt, sal_Bool bSameHeight )
{
    // check if Point/Mark of current cursor are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );

    if( bVert && !CheckSplitCells( *this, nCnt + 1 ) )
    {
        ErrorHandler::HandleError( ERR_TBLSPLIT_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }
    StartAllAction();
    // search boxes via the layout
    sal_Bool bRet;
    SwSelBoxes aBoxes;
    GetTblSel( *this, aBoxes );
    if( !aBoxes.empty() )
    {
        TblWait( nCnt, pFrm, *GetDoc()->GetDocShell(), aBoxes.size() );

        // now delete the columns
        bRet = GetDoc()->SplitTbl( aBoxes, bVert, nCnt, bSameHeight );

        DELETEZ( pLastCols );
        DELETEZ( pLastRows );
    }
    else
        bRet = sal_False;
    EndAllActionAndCall();
    return bRet;
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  _GetTabCols
#***********************************************************************/
void SwFEShell::_GetTabCols( SwTabCols &rToFill, const SwFrm *pBox ) const
{
    const SwTabFrm *pTab = pBox->FindTabFrm();
    if ( pLastCols )
    {
        sal_Bool bDel = sal_True;
        if ( pColumnCacheLastTable == pTab->GetTable() )
        {
            bDel = sal_False;
            SWRECTFN( pTab )

            const SwPageFrm* pPage = pTab->FindPageFrm();
            const sal_uLong nLeftMin = (pTab->Frm().*fnRect->fnGetLeft)() -
                                   (pPage->Frm().*fnRect->fnGetLeft)();
            const sal_uLong nRightMax = (pTab->Frm().*fnRect->fnGetRight)() -
                                    (pPage->Frm().*fnRect->fnGetLeft)();

            if ( pColumnCacheLastTabFrm != pTab )
            {
                // if TabFrm was changed, we only shift a little bit
                // as the width is the same
                SWRECTFNX( pColumnCacheLastTabFrm )
                if( (pColumnCacheLastTabFrm->Frm().*fnRectX->fnGetWidth)() ==
                    (pTab->Frm().*fnRect->fnGetWidth)() )
                {
                    pLastCols->SetLeftMin( nLeftMin );

                    pColumnCacheLastTabFrm = pTab;
                }
                else
                    bDel = sal_True;
            }

            if ( !bDel &&
                 pLastCols->GetLeftMin () == (sal_uInt16)nLeftMin &&
                 pLastCols->GetLeft    () == (sal_uInt16)(pTab->Prt().*fnRect->fnGetLeft)() &&
                 pLastCols->GetRight   () == (sal_uInt16)(pTab->Prt().*fnRect->fnGetRight)()&&
                 pLastCols->GetRightMax() == (sal_uInt16)nRightMax - pLastCols->GetLeftMin() )
            {
                if ( pColumnCacheLastCellFrm != pBox )
                {
                    pTab->GetTable()->GetTabCols( *pLastCols,
                                        ((SwCellFrm*)pBox)->GetTabBox(), sal_True);
                    pColumnCacheLastCellFrm = pBox;
                }
                rToFill = *pLastCols;
            }
            else
                bDel = sal_True;
        }
        if ( bDel )
            DELETEZ(pLastCols);
    }
    if ( !pLastCols )
    {
        GetDoc()->GetTabCols( rToFill, 0, (SwCellFrm*)pBox );

        pLastCols   = new SwTabCols( rToFill );
        pColumnCacheLastTable  = pTab->GetTable();
        pColumnCacheLastTabFrm = pTab;
        pColumnCacheLastCellFrm= pBox;
    }

#if OSL_DEBUG_LEVEL > 1
    SwTabColsEntry aEntry;
    for ( sal_uInt16 i = 0; i < rToFill.Count(); ++i )
    {
        aEntry = rToFill.GetEntry( i );
        (void)aEntry;
    }
#endif
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  _GetTabRows
#***********************************************************************/
void SwFEShell::_GetTabRows( SwTabCols &rToFill, const SwFrm *pBox ) const
{
    const SwTabFrm *pTab = pBox->FindTabFrm();
    if ( pLastRows )
    {
        sal_Bool bDel = sal_True;
        if ( pRowCacheLastTable == pTab->GetTable() )
        {
            bDel = sal_False;
            SWRECTFN( pTab )
            const SwPageFrm* pPage = pTab->FindPageFrm();
            const long nLeftMin  = ( bVert ?
                                     pTab->GetPrtLeft() - pPage->Frm().Left() :
                                     pTab->GetPrtTop() - pPage->Frm().Top() );
            const long nLeft     = bVert ? LONG_MAX : 0;
            const long nRight    = (pTab->Prt().*fnRect->fnGetHeight)();
            const long nRightMax = bVert ? nRight : LONG_MAX;

            if ( pRowCacheLastTabFrm != pTab ||
                 pRowCacheLastCellFrm != pBox )
                bDel = sal_True;

            if ( !bDel &&
                 pLastRows->GetLeftMin () == nLeftMin &&
                 pLastRows->GetLeft    () == nLeft &&
                 pLastRows->GetRight   () == nRight &&
                 pLastRows->GetRightMax() == nRightMax )
            {
                rToFill = *pLastRows;
            }
            else
                bDel = sal_True;
        }
        if ( bDel )
            DELETEZ(pLastRows);
    }
    if ( !pLastRows )
    {
        GetDoc()->GetTabRows( rToFill, 0, (SwCellFrm*)pBox );

        pLastRows   = new SwTabCols( rToFill );
        pRowCacheLastTable  = pTab->GetTable();
        pRowCacheLastTabFrm = pTab;
        pRowCacheLastCellFrm= pBox;
    }
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  SetTabCols(), GetTabCols()
#***********************************************************************/
void SwFEShell::SetTabCols( const SwTabCols &rNew, sal_Bool bCurRowOnly )
{
    SwFrm *pBox = GetCurrFrm();
    if( !pBox || !pBox->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pBox = pBox->GetUpper();
    } while ( !pBox->IsCellFrm() );

    GetDoc()->SetTabCols( rNew, bCurRowOnly, 0, (SwCellFrm*)pBox );
    EndAllActionAndCall();
}

void SwFEShell::GetTabCols( SwTabCols &rToFill ) const
{
    const SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( !pFrm->IsCellFrm() );

    _GetTabCols( rToFill, pFrm );
}

void SwFEShell::GetTabRows( SwTabCols &rToFill ) const
{
    const SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return;
    do
    {   pFrm = pFrm->GetUpper();
    } while ( !pFrm->IsCellFrm() );

    _GetTabRows( rToFill, pFrm );
}

void SwFEShell::SetTabRows( const SwTabCols &rNew, sal_Bool bCurColOnly )
{
    SwFrm *pBox = GetCurrFrm();
    if( !pBox || !pBox->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pBox = pBox->GetUpper();
    } while ( !pBox->IsCellFrm() );

    GetDoc()->SetTabRows( rNew, bCurColOnly, 0, (SwCellFrm*)pBox );
    EndAllActionAndCall();
}

void SwFEShell::GetMouseTabRows( SwTabCols &rToFill, const Point &rPt ) const
{
    const SwFrm *pBox = GetBox( rPt );
    if ( pBox )
        _GetTabRows( rToFill, pBox );
}

void SwFEShell::SetMouseTabRows( const SwTabCols &rNew, sal_Bool bCurColOnly, const Point &rPt )
{
    const SwFrm *pBox = GetBox( rPt );
    if( pBox )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetTabRows( rNew, bCurColOnly, 0, (SwCellFrm*)pBox );
        EndAllActionAndCall();
    }
}

/***********************************************************************
 *  Class      :  SwFEShell
 *  Methods    :  SetRowSplit(), GetRowSplit()
 ***********************************************************************/

void SwFEShell::SetRowSplit( const SwFmtRowSplit& rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowSplit( *getShellCrsr( false ), rNew );
    EndAllActionAndCall();
}

void SwFEShell::GetRowSplit( SwFmtRowSplit*& rpSz ) const
{
    GetDoc()->GetRowSplit( *getShellCrsr( false ), rpSz );
}


/***********************************************************************
 *  Class      :  SwFEShell
 *  Methods    :  SetRowHeight(), GetRowHeight()
 ***********************************************************************/
void SwFEShell::SetRowHeight( const SwFmtFrmSize &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowHeight( *getShellCrsr( false ), rNew );
    EndAllActionAndCall();
}

/******************************************************************************
 *               SwTwips SwFEShell::GetRowHeight() const
 ******************************************************************************/
void SwFEShell::GetRowHeight( SwFmtFrmSize *& rpSz ) const
{
    GetDoc()->GetRowHeight( *getShellCrsr( false ), rpSz );
}

sal_Bool SwFEShell::BalanceRowHeight( sal_Bool bTstOnly )
{
    SET_CURR_SHELL( this );
    if( !bTstOnly )
        StartAllAction();
    sal_Bool bRet = GetDoc()->BalanceRowHeight( *getShellCrsr( false ), bTstOnly );
    if( !bTstOnly )
        EndAllActionAndCall();
    return bRet;
}

/******************************************************************************
 *              void SwFEShell::SetRowBackground()
 ******************************************************************************/
void SwFEShell::SetRowBackground( const SvxBrushItem &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetRowBackground( *getShellCrsr( false ), rNew );
    EndAllActionAndCall();
}

/******************************************************************************
 *               SwTwips SwFEShell::GetRowBackground() const
 ******************************************************************************/
sal_Bool SwFEShell::GetRowBackground( SvxBrushItem &rToFill ) const
{
    return GetDoc()->GetRowBackground( *getShellCrsr( false ), rToFill );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  SetTabBorders(), GetTabBorders()
#***********************************************************************/

void SwFEShell::SetTabBorders( const SfxItemSet& rSet )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetTabBorders( *getShellCrsr( false ), rSet );
    EndAllActionAndCall();
}

void SwFEShell::SetTabLineStyle( const Color* pColor, sal_Bool bSetLine,
                                 const editeng::SvxBorderLine* pBorderLine )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetTabLineStyle( *getShellCrsr( false ),
                                pColor, bSetLine, pBorderLine );
    EndAllActionAndCall();
}

void SwFEShell::GetTabBorders( SfxItemSet& rSet ) const
{
    GetDoc()->GetTabBorders( *getShellCrsr( false ), rSet );
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  SetBoxBackground(), GetBoxBackground()
#***********************************************************************/
void SwFEShell::SetBoxBackground( const SvxBrushItem &rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAttr( *getShellCrsr( false ), rNew );
    EndAllActionAndCall();
}

sal_Bool SwFEShell::GetBoxBackground( SvxBrushItem &rToFill ) const
{
    return GetDoc()->GetBoxAttr( *getShellCrsr( false ), rToFill );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  SetBoxDirection(), GetBoxDirection()
#***********************************************************************/
void SwFEShell::SetBoxDirection( const SvxFrameDirectionItem& rNew )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAttr( *getShellCrsr( false ), rNew );
    EndAllActionAndCall();
}

sal_Bool SwFEShell::GetBoxDirection( SvxFrameDirectionItem&  rToFill ) const
{
    return GetDoc()->GetBoxAttr( *getShellCrsr( false ), rToFill );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  SetBoxAlign, SetBoxAlign
#***********************************************************************/
void SwFEShell::SetBoxAlign( sal_uInt16 nAlign )
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetBoxAlign( *getShellCrsr( false ), nAlign );
    EndAllActionAndCall();
}

sal_uInt16 SwFEShell::GetBoxAlign() const
{
    return GetDoc()->GetBoxAlign( *getShellCrsr( false ) );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  SetTabBackground(), GetTabBackground()
#***********************************************************************/
void SwFEShell::SetTabBackground( const SvxBrushItem &rNew )
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return;

    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->SetAttr( rNew, *pFrm->ImplFindTabFrm()->GetFmt() );
    EndAllAction(); // no call, nothing changes!
    GetDoc()->SetModified();
}

void SwFEShell::GetTabBackground( SvxBrushItem &rToFill ) const
{
    SwFrm *pFrm = GetCurrFrm();
    if( pFrm && pFrm->IsInTab() )
        rToFill = pFrm->ImplFindTabFrm()->GetFmt()->GetBackground();
}


/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  HasWholeTabSelection()
#***********************************************************************/
sal_Bool SwFEShell::HasWholeTabSelection() const
{
    // whole table selected?
    if ( IsTableMode() )
    {
        SwSelBoxes aBoxes;
        ::GetTblSelCrs( *this, aBoxes );
        if( !aBoxes.empty() )
        {
            const SwTableNode *pTblNd = IsCrsrInTbl();
            return pTblNd &&
                aBoxes[0]->GetSttIdx() - 1 == pTblNd->EndOfSectionNode()->StartOfSectionIndex() &&
                aBoxes.back()->GetSttNd()->EndOfSectionIndex() + 1 == pTblNd->EndOfSectionIndex();
        }
    }
    return sal_False;
}

sal_Bool SwFEShell::HasBoxSelection() const
{
    if(!IsCrsrInTbl())
        return sal_False;
    // whole table selected?
    if( IsTableMode() )
        return sal_True;
    SwPaM* pPam = GetCrsr();
        // empty boxes are also selected as the absence of selection
    sal_Bool bChg = sal_False;
    if( pPam->GetPoint() == pPam->End())
    {
        bChg = sal_True;
        pPam->Exchange();
    }
    SwNode* pNd;
    if( pPam->GetPoint()->nNode.GetIndex() -1 ==
        ( pNd = pPam->GetNode())->StartOfSectionIndex() &&
        !pPam->GetPoint()->nContent.GetIndex() &&
        pPam->GetMark()->nNode.GetIndex() + 1 ==
        pNd->EndOfSectionIndex())
    {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode(), -1 );
            SwCntntNode* pCNd = aIdx.GetNode().GetCntntNode();
            if( !pCNd )
            {
                pCNd = GetDoc()->GetNodes().GoPrevious( &aIdx );
                OSL_ENSURE( pCNd, "no ContentNode in box ??" );
            }
            if( pPam->GetMark()->nContent == pCNd->Len() )
            {
                if( bChg )
                    pPam->Exchange();
                return sal_True;
            }
    }
    if( bChg )
        pPam->Exchange();
    return sal_False;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  ProtectCells(), UnProtectCells()
#***********************************************************************/
void SwFEShell::ProtectCells()
{
    SvxProtectItem aProt( RES_PROTECT );
    aProt.SetCntntProtect( sal_True );

    SET_CURR_SHELL( this );
    StartAllAction();

    GetDoc()->SetBoxAttr( *getShellCrsr( false ), aProt );

    if( !IsCrsrReadonly() )
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
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        SwFrm *pFrm = GetCurrFrm();
        do {
            pFrm = pFrm->GetUpper();
        } while ( pFrm && !pFrm->IsCellFrm() );
        if( pFrm )
        {
            SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
            aBoxes.insert( pBox );
        }
    }

    if( !aBoxes.empty() )
        GetDoc()->UnProtectCells( aBoxes );

    EndAllActionAndCall();
}

void SwFEShell::UnProtectTbls()
{
    SET_CURR_SHELL( this );
    StartAllAction();
    GetDoc()->UnProtectTbls( *GetCrsr() );
    EndAllActionAndCall();
}

sal_Bool SwFEShell::HasTblAnyProtection( const String* pTblName,
                                    sal_Bool* pFullTblProtection )
{
    return GetDoc()->HasTblAnyProtection( GetCrsr()->GetPoint(), pTblName,
                                        pFullTblProtection );
}

sal_Bool SwFEShell::CanUnProtectCells() const
{
    sal_Bool bUnProtectAvailable = sal_False;
    const SwTableNode *pTblNd = IsCrsrInTbl();
    if( pTblNd && !pTblNd->IsProtect() )
    {
        SwSelBoxes aBoxes;
        if( IsTableMode() )
            ::GetTblSelCrs( *this, aBoxes );
        else
        {
            SwFrm *pFrm = GetCurrFrm();
            do {
                pFrm = pFrm->GetUpper();
            } while ( pFrm && !pFrm->IsCellFrm() );
            if( pFrm )
            {
                SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
                aBoxes.insert( pBox );
            }
        }
        if( !aBoxes.empty() )
            bUnProtectAvailable = ::HasProtectedCells( aBoxes );
    }
    return bUnProtectAvailable;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methods    :  GetRowsToRepeat(), SetRowsToRepeat()
#***********************************************************************/
sal_uInt16 SwFEShell::GetRowsToRepeat() const
{
    const SwFrm *pFrm = GetCurrFrm();
    const SwTabFrm *pTab = pFrm ? pFrm->FindTabFrm() : 0;
    if( pTab )
        return pTab->GetTable()->GetRowsToRepeat();
    return 0;
}

void SwFEShell::SetRowsToRepeat( sal_uInt16 nSet )
{
    SwFrm    *pFrm = GetCurrFrm();
    SwTabFrm *pTab = pFrm ? pFrm->FindTabFrm() : 0;
    if( pTab && pTab->GetTable()->GetRowsToRepeat() != nSet )
    {
        SwWait aWait( *GetDoc()->GetDocShell(), sal_True );
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetRowsToRepeat( *pTab->GetTable(), nSet );
        EndAllActionAndCall();
    }
}

/*-------------------------------------------------------------------------
    returns the number of rows consecutively selected from top
  -----------------------------------------------------------------------*/
sal_uInt16 lcl_GetRowNumber( const SwPosition& rPos )
{
    sal_uInt16 nRet = USHRT_MAX;
    Point aTmpPt;
    const SwCntntNode *pNd;
    const SwCntntFrm *pFrm;

    if( 0 != ( pNd = rPos.nNode.GetNode().GetCntntNode() ))
        pFrm = pNd->getLayoutFrm( pNd->GetDoc()->GetCurrentLayout(), &aTmpPt, &rPos, sal_False );
    else
        pFrm = 0;

    if ( pFrm && pFrm->IsInTab() )
    {
        const SwFrm* pRow = pFrm->GetUpper();
        while ( !pRow->GetUpper()->IsTabFrm() )
            pRow = pRow->GetUpper();

        const SwTabFrm* pTabFrm = (const SwTabFrm*)pRow->GetUpper();
        const SwTableLine* pTabLine = static_cast<const SwRowFrm*>(pRow)->GetTabLine();

        sal_uInt16 nI = 0;
        while ( nI < pTabFrm->GetTable()->GetTabLines().size() )
        {
            if ( pTabFrm->GetTable()->GetTabLines()[ nI ] == pTabLine )
            {
                nRet = nI;
                break;
            }
            ++nI;
        }
    }

    return nRet;
}
sal_uInt16 SwFEShell::GetRowSelectionFromTop() const
{
    sal_uInt16 nRet = 0;
    const SwPaM* pPaM = IsTableMode() ? GetTableCrsr() : _GetCrsr();
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
            nRet = Max( nPtLine, nMkLine ) + 1;
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
 * returns true if the current frame is localed inside a table headline OR
 * inside the first line of a table!!!
 */
sal_Bool SwFEShell::CheckHeadline( bool bRepeat ) const
{
    sal_Bool bRet = sal_False;
    if ( !IsTableMode() )
    {
        SwFrm *pFrm = GetCurrFrm();  // DONE MULTIIHEADER
        if ( pFrm && pFrm->IsInTab() )
        {
            SwTabFrm* pTab = pFrm->FindTabFrm();
            if ( bRepeat )
            {
                bRet = pTab->IsFollow() && pTab->IsInHeadline( *pFrm );
            }
            else
            {
                bRet =  ((SwLayoutFrm*)pTab->Lower())->IsAnLower( pFrm ) ||
                        pTab->IsInHeadline( *pFrm );
            }
        }
    }
    return bRet;
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  AdjustCellWidth()
#***********************************************************************/

void SwFEShell::AdjustCellWidth( sal_Bool bBalance )
{
    SET_CURR_SHELL( this );
    StartAllAction();

    // switch on wait-cursor, as we do not know how
    // much content is affected
    TblWait aWait( USHRT_MAX, 0, *GetDoc()->GetDocShell() );

    GetDoc()->AdjustCellWidth( *getShellCrsr( false ), bBalance );
    EndAllActionAndCall();
}

sal_Bool SwFEShell::IsAdjustCellWidthAllowed( sal_Bool bBalance ) const
{
    // at least one row with content should be contained in the selection

    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    SwSelBoxes aBoxes;
    ::GetTblSelCrs( *this, aBoxes );

    if ( bBalance )
        return aBoxes.size() > 1;

    if ( aBoxes.empty() )
    {
        do
        {   pFrm = pFrm->GetUpper();
        } while ( !pFrm->IsCellFrm() );
        SwTableBox *pBox = (SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox();
        aBoxes.insert( pBox );
    }

    for ( sal_uInt16 i = 0; i < aBoxes.size(); ++i )
    {
        SwTableBox *pBox = aBoxes[i];
        if ( pBox->GetSttNd() )
        {
            SwNodeIndex aIdx( *pBox->GetSttNd(), 1 );
            SwTxtNode* pCNd = aIdx.GetNode().GetTxtNode();
            if( !pCNd )
                pCNd = (SwTxtNode*)GetDoc()->GetNodes().GoNext( &aIdx );

            while ( pCNd )
            {
                if ( pCNd->GetTxt().Len() )
                    return sal_True;
                ++aIdx;
                pCNd = aIdx.GetNode().GetTxtNode();
            }
        }
    }
    return sal_False;
}

    // AutoFormat for the table/table selection
sal_Bool SwFEShell::SetTableAutoFmt( const SwTableAutoFmt& rNew )
{
    SwTableNode *pTblNd = (SwTableNode*)IsCrsrInTbl();
    if( !pTblNd || pTblNd->GetTable().IsTblComplex() )
        return sal_False;

    SwSelBoxes aBoxes;

    if ( !IsTableMode() )       // if cursors are not current
        GetCrsr();

    // whole table or only current selection
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        const SwTableSortBoxes& rTBoxes = pTblNd->GetTable().GetTabSortBoxes();
        for (size_t n = 0; n < rTBoxes.size(); ++n)
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.insert( pBox );
        }
    }

    sal_Bool bRet;
    if( !aBoxes.empty() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        bRet = GetDoc()->SetTableAutoFmt( aBoxes, rNew );
        DELETEZ( pLastCols );
        DELETEZ( pLastRows );
        EndAllActionAndCall();
    }
    else
        bRet = sal_False;
    return bRet;
}

sal_Bool SwFEShell::GetTableAutoFmt( SwTableAutoFmt& rGet )
{
    const SwTableNode *pTblNd = IsCrsrInTbl();
    if( !pTblNd || pTblNd->GetTable().IsTblComplex() )
        return sal_False;

    SwSelBoxes aBoxes;

    if ( !IsTableMode() )       // if cursor are not current
        GetCrsr();

    // whole table or only current selection
    if( IsTableMode() )
        ::GetTblSelCrs( *this, aBoxes );
    else
    {
        const SwTableSortBoxes& rTBoxes = pTblNd->GetTable().GetTabSortBoxes();
        for (size_t n = 0; n < rTBoxes.size(); ++n)
        {
            SwTableBox* pBox = rTBoxes[ n ];
            aBoxes.insert( pBox );
        }
    }

    return GetDoc()->GetTableAutoFmt( aBoxes, rGet );
}

/***********************************************************************
#*  Class      :  SwFEShell
#*  Methoden   :  DeleteTblSel()
#***********************************************************************/
sal_Bool SwFEShell::DeleteTblSel()
{
    // check if SPoint/Mark of current cursor are in a table
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    // search boxes via the layout
    sal_Bool bRet;
    SwSelBoxes aBoxes;
    GetTblSelCrs( *this, aBoxes );
    if( !aBoxes.empty() )
    {
        TblWait( aBoxes.size(), pFrm, *GetDoc()->GetDocShell() );

        // cursor should be removed from deletion area.
        // Put them behind/on the table; via the document
        // position they'll be set to the old position
        while( !pFrm->IsCellFrm() )
            pFrm = pFrm->GetUpper();
        ParkCrsr( SwNodeIndex( *((SwCellFrm*)pFrm)->GetTabBox()->GetSttNd() ));

        bRet = GetDoc()->DeleteRowCol( aBoxes );

        DELETEZ( pLastCols );
        DELETEZ( pLastRows );
    }
    else
        bRet = sal_False;
    EndAllActionAndCall();
    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetCurTabColNum()
|*
|*************************************************************************/
sal_uInt16 SwFEShell::GetCurTabColNum() const
{
    //!!!GetCurMouseTabColNum() mitpflegen!!!!
    sal_uInt16 nRet = 0;

    SwFrm *pFrm = GetCurrFrm();
    OSL_ENSURE( pFrm, "Crsr parked?" );

    // check if SPoint/Mark of current cursor are in a table
    if( pFrm && pFrm->IsInTab() )
    {
        do {            // JP 26.09.95: why compare with CntntFrame
                        //              and not with CellFrame ????
            pFrm = pFrm->GetUpper();
        } while ( !pFrm->IsCellFrm() );
        SWRECTFN( pFrm )

        const SwPageFrm* pPage = pFrm->FindPageFrm();

        // get TabCols, as only via these we get to the position
        SwTabCols aTabCols;
        GetTabCols( aTabCols );

        if( pFrm->FindTabFrm()->IsRightToLeft() )
        {
            long nX = (pFrm->Frm().*fnRect->fnGetRight)() - (pPage->Frm().*fnRect->fnGetLeft)();

            const long nRight = aTabCols.GetLeftMin() + aTabCols.GetRight();;

            if ( !::IsSame( nX, nRight ) )
            {
                nX = nRight - nX + aTabCols.GetLeft();
                for ( sal_uInt16 i = 0; i < aTabCols.Count(); ++i )
                    if ( ::IsSame( nX, aTabCols[i] ) )
                    {
                        nRet = i + 1;
                        break;
                    }
            }
        }
        else
        {
            const long nX = (pFrm->Frm().*fnRect->fnGetLeft)() -
                            (pPage->Frm().*fnRect->fnGetLeft)();

            const long nLeft = aTabCols.GetLeftMin();

            if ( !::IsSame( nX, nLeft + aTabCols.GetLeft() ) )
            {
                for ( sal_uInt16 i = 0; i < aTabCols.Count(); ++i )
                    if ( ::IsSame( nX, nLeft + aTabCols[i] ) )
                    {
                        nRet = i + 1;
                        break;
                    }
            }
        }
    }
    return nRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetBox()
|*
|*************************************************************************/

const SwFrm *lcl_FindFrmInTab( const SwLayoutFrm *pLay, const Point &rPt, SwTwips nFuzzy )
{
    const SwFrm *pFrm = pLay->Lower();

    while( pFrm && pLay->IsAnLower( pFrm ) )
    {
        if ( pFrm->Frm().IsNear( rPt, nFuzzy ) )
        {
            if ( pFrm->IsLayoutFrm() )
            {
                const SwFrm *pTmp = ::lcl_FindFrmInTab( (SwLayoutFrm*)pFrm, rPt, nFuzzy );
                if ( pTmp )
                    return pTmp;
            }

            return pFrm;
        }

        pFrm = pFrm->FindNext();
    }

    return 0;
}

const SwCellFrm *lcl_FindFrm( const SwLayoutFrm *pLay, const Point &rPt,
                              SwTwips nFuzzy, bool* pbRow, bool* pbCol )
{
    // bMouseMoveRowCols :
    // Method is called for
    // - Moving columns/rows with the mouse or
    // - Enhanced table selection
    const bool bMouseMoveRowCols = 0 == pbCol;

    bool bCloseToRow = false;
    bool bCloseToCol = false;

    const SwFrm *pFrm = pLay->ContainsCntnt();
    const SwFrm* pRet = 0;

    if ( pFrm )
    {
        do
        {
            if ( pFrm->IsInTab() )
                pFrm = ((SwFrm*)pFrm)->ImplFindTabFrm();

            if ( pFrm->IsTabFrm() )
            {
                Point aPt( rPt );
                bool bSearchForFrmInTab = true;
                SwTwips nTmpFuzzy = nFuzzy;

                if ( !bMouseMoveRowCols )
                {
                    // We ignore nested tables for the enhanced table selection:
                    while ( pFrm->GetUpper()->IsInTab() )
                        pFrm = pFrm->GetUpper()->FindTabFrm();

                    // We first check if the given point is 'close' to the left or top
                    // border of the table frame:
                    OSL_ENSURE( pFrm, "Nested table frame without outer table" );
                    SWRECTFN( pFrm )
                    const bool bRTL = pFrm->IsRightToLeft();

                    SwRect aTabRect = pFrm->Prt();
                    aTabRect.Pos() += pFrm->Frm().Pos();

                    const SwTwips nLeft = bRTL ?
                                          (aTabRect.*fnRect->fnGetRight)() :
                                          (aTabRect.*fnRect->fnGetLeft)();
                    const SwTwips nTop  = (aTabRect.*fnRect->fnGetTop)();

                    SwTwips& rPointX = bVert ? aPt.Y() : aPt.X();
                    SwTwips& rPointY = bVert ? aPt.X() : aPt.Y();

                    const SwTwips nXDiff = (*fnRect->fnXDiff)( nLeft, rPointX ) * ( bRTL ? (-1) : 1 );
                    const SwTwips nYDiff = (*fnRect->fnYDiff)( nTop, rPointY );

                    bCloseToRow = nXDiff >= 0 && nXDiff < nFuzzy;
                    bCloseToCol = nYDiff >= 0 && nYDiff < nFuzzy;

                    if ( bCloseToCol && 2 * nYDiff > nFuzzy )
                    {
                        const SwFrm* pPrev = pFrm->GetPrev();
                        if ( pPrev )
                        {
                            SwRect aPrevRect = pPrev->Prt();
                            aPrevRect.Pos() += pPrev->Frm().Pos();

                            if( aPrevRect.IsInside( rPt ) )
                            {
                                bCloseToCol = false;
                            }
                        }

                    }

                    // If we found the point to be 'close' to the left or top border
                    // of the table frame, we adjust the point to be on that border:
                    if ( bCloseToRow && bCloseToCol )
                        aPt = bRTL ? aTabRect.TopRight() : (aTabRect.*fnRect->fnGetPos)();
                    else if ( bCloseToRow )
                        rPointX = nLeft;
                    else if ( bCloseToCol )
                        rPointY = nTop;

                    if ( !bCloseToRow && !bCloseToCol )
                        bSearchForFrmInTab = false;

                    // Since the point has been adjusted, we call lcl_FindFrmInTab()
                    // with a fuzzy value of 1:
                    nTmpFuzzy = 1;
                }

                const SwFrm* pTmp = bSearchForFrmInTab ?
                                    ::lcl_FindFrmInTab( (SwLayoutFrm*)pFrm, aPt, nTmpFuzzy ) :
                                    0;

                if ( pTmp )
                {
                    pFrm = pTmp;
                    break;
                }
            }
            pFrm = pFrm->FindNextCnt();

        } while ( pFrm && pLay->IsAnLower( pFrm ) );
    }

    if ( pFrm && pFrm->IsInTab() && pLay->IsAnLower( pFrm ) )
    {
        do
        {
            // We allow mouse drag of table borders within nested tables,
            // but disallow hotspot selection of nested tables.
            if ( bMouseMoveRowCols )
            {
                // find the next cell frame
                while ( pFrm && !pFrm->IsCellFrm() )
                    pFrm = pFrm->GetUpper();
            }
            else
            {
                // find the most upper cell frame:
                while ( pFrm &&
                        ( !pFrm->IsCellFrm() ||
                          !pFrm->GetUpper()->GetUpper()->IsTabFrm() ||
                           pFrm->GetUpper()->GetUpper()->GetUpper()->IsInTab() ) )
                    pFrm = pFrm->GetUpper();
            }

            if ( pFrm ) // Note: this condition should be the same like the while condition!!!
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
                        pRet = pFrm;
                        break;
                    }
                }
                else
                {
                    // used for mouse move of columns/rows
                    const SwTabFrm* pTabFrm = pFrm->FindTabFrm();
                    SwRect aTabRect = pTabFrm->Prt();
                    aTabRect.Pos() += pTabFrm->Frm().Pos();

                    SWRECTFN( pTabFrm )

                    const SwTwips nTabTop  = (aTabRect.*fnRect->fnGetTop)();
                    const SwTwips nMouseTop  = bVert ? rPt.X() : rPt.Y();

                    // Do not allow to drag upper table border:
                    if ( !::IsSame( nTabTop, nMouseTop ) )
                    {
                        if ( ::IsSame( pFrm->Frm().Left(), rPt.X() ) ||
                             ::IsSame( pFrm->Frm().Right(),rPt.X() ) )
                        {
                            if ( pbRow ) *pbRow = false;
                            pRet = pFrm;
                            break;
                        }
                        if ( ::IsSame( pFrm->Frm().Top(), rPt.Y() ) ||
                             ::IsSame( pFrm->Frm().Bottom(),rPt.Y() ) )
                        {
                            if ( pbRow ) *pbRow = true;
                            pRet = pFrm;
                            break;
                        }
                    }
                }

                pFrm = pFrm->GetUpper();
            }
        } while ( pFrm );
    }

    // robust:
    OSL_ENSURE( !pRet || pRet->IsCellFrm(), "lcl_FindFrm() is supposed to find a cell frame!" );
    return pRet && pRet->IsCellFrm() ? static_cast<const SwCellFrm*>(pRet) : 0;
}

//
// pbCol  = 0 => Used for moving table rows/cols with mouse
// pbCol != 0 => Used for selecting table/rows/cols
//
#define ENHANCED_TABLE_SELECTION_FUZZY 10

const SwFrm* SwFEShell::GetBox( const Point &rPt, bool* pbRow, bool* pbCol ) const
{
    const SwPageFrm *pPage = (SwPageFrm*)GetLayout()->Lower();
    Window* pOutWin = GetWin();
    SwTwips nFuzzy = COLFUZZY;
    if( pOutWin )
    {
        // #i32329# Enhanced table selection
        SwTwips nSize = pbCol ? ENHANCED_TABLE_SELECTION_FUZZY : RULER_MOUSE_MARGINWIDTH;
        Size aTmp( nSize, nSize );
        aTmp = pOutWin->PixelToLogic( aTmp );
        nFuzzy = aTmp.Width();
    }

    while ( pPage && !pPage->Frm().IsNear( rPt, nFuzzy ) )
        pPage = (SwPageFrm*)pPage->GetNext();

    const SwCellFrm *pFrm = 0;
    if ( pPage )
    {
        // We cannot search the box by GetCrsrOfst or GetCntntPos.
        // This would lead to a performance collapse for documents
        // with a lot of paragraphs/tables on one page
        //(BrowseMode!)

        // check flys first
        if ( pPage->GetSortedObjs() )
        {
            for ( sal_uInt16 i = 0; !pFrm && i < pPage->GetSortedObjs()->Count(); ++i )
            {
                SwAnchoredObject* pObj = (*pPage->GetSortedObjs())[i];
                if ( pObj->ISA(SwFlyFrm) )
                {
                    pFrm = lcl_FindFrm( static_cast<SwFlyFrm*>(pObj),
                                        rPt, nFuzzy, pbRow, pbCol );
                }
            }
        }
        const SwLayoutFrm *pLay = (SwLayoutFrm*)pPage->Lower();
        while ( pLay && !pFrm )
        {
            pFrm = lcl_FindFrm( pLay, rPt, nFuzzy, pbRow, pbCol );
            pLay = (SwLayoutFrm*)pLay->GetNext();
        }
    }
    return pFrm;
}

/* Helper function*/
/* calculated the distance between Point rC and Line Segment (rA, rB) */
double lcl_DistancePoint2Segment( const Point& rA, const Point& rB, const Point& rC )
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

    return Abs(nRet);
}

/* Helper function*/
Point lcl_ProjectOntoClosestTableFrm( const SwTabFrm& rTab, const Point& rPoint, bool bRowDrag )
{
    Point aRet( rPoint );
    const SwTabFrm* pCurrentTab = &rTab;
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
        SwRect aTabRect( pCurrentTab->Prt() );
        aTabRect += pCurrentTab->Frm().Pos();

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
        aRet.X() = aMin1.X();
        if ( aRet.Y() > aMin2.Y() )
            aRet.Y() = aMin2.Y();
        else if ( aRet.Y() < aMin1.Y() )
            aRet.Y() = aMin1.Y();
    }
    else
    {
        aRet.Y() = aMin1.Y();
        if ( aRet.X() > aMin2.X() )
            aRet.X() = aMin2.X();
        else if ( aRet.X() < aMin1.X() )
            aRet.X() = aMin1.X();
    }

    return aRet;
}

// #i32329# Enhanced table selection
bool SwFEShell::SelTblRowCol( const Point& rPt, const Point* pEnd, bool bRowDrag )
{
    bool bRet = false;
    Point aEndPt;
    if ( pEnd )
        aEndPt = *pEnd;

    SwPosition*  ppPos[2] = { 0, 0 };
    Point        paPt [2] = { rPt, aEndPt };
    bool         pbRow[2] = { 0, 0 };
    bool         pbCol[2] = { 0, 0 };

    // pEnd is set during dragging.
    for ( sal_uInt16 i = 0; i < ( pEnd ? 2 : 1 ); ++i )
    {
        const SwCellFrm* pFrm =
             static_cast<const SwCellFrm*>(GetBox( paPt[i], &pbRow[i], &pbCol[i] ) );

        if( pFrm )
        {
            while( pFrm->Lower() && pFrm->Lower()->IsRowFrm() )
                pFrm = static_cast<const SwCellFrm*>( static_cast<const SwLayoutFrm*>( pFrm->Lower() )->Lower() );
            if( pFrm && pFrm->GetTabBox()->GetSttNd() &&
                pFrm->GetTabBox()->GetSttNd()->IsInProtectSect() )
                pFrm = 0;
        }

        if ( pFrm )
        {
            const SwCntntFrm* pCntnt = ::GetCellCntnt( *pFrm );

            if ( pCntnt && pCntnt->IsTxtFrm() )
            {
                ppPos[i] = new SwPosition( *pCntnt->GetNode() );
                ppPos[i]->nContent.Assign( const_cast<SwCntntNode*>(pCntnt->GetNode()), 0 );

                // paPt[i] will not be used any longer, now we use it to store
                // a position inside the content frame
                paPt[i] = pCntnt->Frm().Center();
            }
        }

        // no calculation of end frame if start frame has not been found.
        if ( 1 == i || !ppPos[0] || !pEnd )
            break;

        // find 'closest' table frame to pEnd:
        const SwTabFrm* pCurrentTab = pFrm->FindTabFrm();
        if ( pCurrentTab->IsFollow() )
            pCurrentTab = pCurrentTab->FindMaster( true );

        const Point aProjection = lcl_ProjectOntoClosestTableFrm( *pCurrentTab, *pEnd, bRowDrag );
        paPt[1] = aProjection;
    }

    if ( ppPos[0] )
    {
        SwShellCrsr* pCrsr = _GetCrsr();
        SwCrsrSaveState aSaveState( *pCrsr );
        SwPosition aOldPos( *pCrsr->GetPoint() );

        pCrsr->DeleteMark();
        *pCrsr->GetPoint() = *ppPos[0];
        pCrsr->GetPtPos() = paPt[0];

        if ( !pCrsr->IsInProtectTable( sal_False, sal_True ) )
        {
            bool bNewSelection = true;

            if ( ppPos[1] )
            {
                if ( ppPos[1]->nNode.GetNode().StartOfSectionNode() !=
                     aOldPos.nNode.GetNode().StartOfSectionNode() )
                {
                    pCrsr->SetMark();
                    SwCrsrSaveState aSaveState2( *pCrsr );
                    *pCrsr->GetPoint() = *ppPos[1];
                    pCrsr->GetPtPos() = paPt[1];

                    if ( pCrsr->IsInProtectTable( sal_False, sal_False ) )
                    {
                        pCrsr->RestoreSavePos();
                        bNewSelection = false;
                    }
                }
                else
                {
                    pCrsr->RestoreSavePos();
                    bNewSelection = false;
                }
            }

            if ( bNewSelection )
            {
                // #i35543# SelTblRowCol should remove any existing
                // table cursor:
                if ( IsTableMode() )
                    TblCrsrToCursor();

                if ( pbRow[0] && pbCol[0] )
                    bRet = SwCrsrShell::SelTbl();
                else if ( pbRow[0] )
                    bRet = SwCrsrShell::_SelTblRowOrCol( true, true );
                else if ( pbCol[0] )
                    bRet = SwCrsrShell::_SelTblRowOrCol( false, true );
            }
            else
                bRet = true;
        }

        delete ppPos[0];
        delete ppPos[1];
    }

    return bRet;
}


/*************************************************************************
|*
|*  SwFEShell::WhichMouseTabCol()
|*
|*************************************************************************/
sal_uInt8 SwFEShell::WhichMouseTabCol( const Point &rPt ) const
{
    sal_uInt8 nRet = SW_TABCOL_NONE;
    bool bRow = false;
    bool bCol = false;
    bool bSelect = false;

    // First try: Do we get the row/col move cursor?
    SwCellFrm* pFrm = (SwCellFrm*)GetBox( rPt, &bRow, 0 );

    if ( !pFrm )
    {
        // Second try: Do we get the row/col/tab selection cursor?
        pFrm = (SwCellFrm*)GetBox( rPt, &bRow, &bCol );
        bSelect = true;
    }

    if( pFrm )
    {
        while( pFrm->Lower() && pFrm->Lower()->IsRowFrm() )
            pFrm = (SwCellFrm*)((SwLayoutFrm*)pFrm->Lower())->Lower();
        if( pFrm && pFrm->GetTabBox()->GetSttNd() &&
            pFrm->GetTabBox()->GetSttNd()->IsInProtectSect() )
            pFrm = 0;
    }

    if( pFrm )
    {
        if ( !bSelect )
        {
            if ( pFrm->IsVertical() )
                nRet = bRow ? SW_TABCOL_VERT : SW_TABROW_VERT;
            else
                nRet = bRow ? SW_TABROW_HORI : SW_TABCOL_HORI;
        }
        else
        {
            const SwTabFrm* pTabFrm = pFrm->FindTabFrm();
            if ( pTabFrm->IsVertical() )
            {
                if ( bRow && bCol )
                {
                    nRet = SW_TABSEL_VERT;
                }
                else if ( bRow )
                {
                    nRet = SW_TABROWSEL_VERT;
                }
                else if ( bCol )
                {
                    nRet = SW_TABCOLSEL_VERT;
                }
            }
            else
            {
                if ( bRow && bCol )
                {
                    nRet =  pTabFrm->IsRightToLeft() ?
                            SW_TABSEL_HORI_RTL :
                            SW_TABSEL_HORI;
                }
                else if ( bRow )
                {
                    nRet = pTabFrm->IsRightToLeft() ?
                           SW_TABROWSEL_HORI_RTL :
                           SW_TABROWSEL_HORI;
                }
                else if ( bCol )
                {
                    nRet = SW_TABCOLSEL_HORI;
                }
            }
        }
    }

    return nRet;
}

// -> #i23726#
SwTxtNode * SwFEShell::GetNumRuleNodeAtPos( const Point &rPt)
{
    SwTxtNode * pResult = NULL;

    SwContentAtPos aCntntAtPos
        (SwContentAtPos::SW_NUMLABEL);

    if( GetContentAtPos(rPt, aCntntAtPos) && aCntntAtPos.aFnd.pNode)
        pResult = aCntntAtPos.aFnd.pNode->GetTxtNode();

    return pResult;
}

sal_Bool SwFEShell::IsNumLabel( const Point &rPt, int nMaxOffset )
{
    sal_Bool bResult = sal_False;

    SwContentAtPos aCntntAtPos
        (SwContentAtPos::SW_NUMLABEL);

    if( GetContentAtPos(rPt, aCntntAtPos))
    {
        if ((nMaxOffset >= 0 && aCntntAtPos.nDist <= nMaxOffset) ||
            (nMaxOffset < 0))
            bResult = sal_True;
    }

    return bResult;
}
// <- #i23726#

// #i42921#
bool SwFEShell::IsVerticalModeAtNdAndPos( const SwTxtNode& _rTxtNode,
                                          const Point& _rDocPos ) const
{
    bool bRet( false );

    const short nTextDir =
        _rTxtNode.GetTextDirection( SwPosition(_rTxtNode), &_rDocPos );
    switch ( nTextDir )
    {
        case -1:
        case FRMDIR_HORI_RIGHT_TOP:
        case FRMDIR_HORI_LEFT_TOP:
        {
            bRet = false;
        }
        break;
        case FRMDIR_VERT_TOP_LEFT:
        case FRMDIR_VERT_TOP_RIGHT:
        {
            bRet = true;
        }
        break;
    }

    return bRet;
}

/*************************************************************************
|*
|*  SwFEShell::GetMouseTabCols()
|*
|*************************************************************************/
void SwFEShell::GetMouseTabCols( SwTabCols &rToFill, const Point &rPt ) const
{
    const SwFrm *pBox = GetBox( rPt );
    if ( pBox )
        _GetTabCols( rToFill, pBox );
}

void SwFEShell::SetMouseTabCols( const SwTabCols &rNew, sal_Bool bCurRowOnly,
                                 const Point &rPt )
{
    const SwFrm *pBox = GetBox( rPt );
    if( pBox )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        GetDoc()->SetTabCols( rNew, bCurRowOnly, 0, (SwCellFrm*)pBox );
        EndAllActionAndCall();
    }
}

/*************************************************************************
|*
|*  SwFEShell::GetMouseColNum(), GetMouseTabColNum()
|*
|*************************************************************************/
sal_uInt16 SwFEShell::GetCurMouseColNum( const Point &rPt,
                                    SwGetCurColNumPara* pPara ) const
{
    return _GetCurColNum( GetBox( rPt ), pPara );
}

sal_uInt16 SwFEShell::GetCurMouseTabColNum( const Point &rPt ) const
{
    //!!!GetCurTabColNum() mitpflegen!!!!
    sal_uInt16 nRet = 0;

    const SwFrm *pFrm = GetBox( rPt );
    OSL_ENSURE( pFrm, "Table not found" );
    if( pFrm )
    {
        const long nX = pFrm->Frm().Left();

        // get TabCols, only via these we get the position
        SwTabCols aTabCols;
        GetMouseTabCols( aTabCols, rPt );

        const long nLeft = aTabCols.GetLeftMin();

        if ( !::IsSame( nX, nLeft + aTabCols.GetLeft() ) )
        {
            for ( sal_uInt16 i = 0; i < aTabCols.Count(); ++i )
                if ( ::IsSame( nX, nLeft + aTabCols[i] ) )
                {
                    nRet = i + 1;
                    break;
                }
        }
    }
    return nRet;
}

void ClearFEShellTabCols()
{
    DELETEZ( pLastCols );
    DELETEZ( pLastRows );
}

/*************************************************************************
|*
|*  SwFEShell::GetTblAttr(), SetTblAttr()
|*
|*************************************************************************/
void SwFEShell::GetTblAttr( SfxItemSet &rSet ) const
{
    SwFrm *pFrm = GetCurrFrm();
    if( pFrm && pFrm->IsInTab() )
        rSet.Put( pFrm->ImplFindTabFrm()->GetFmt()->GetAttrSet() );
}

void SwFEShell::SetTblAttr( const SfxItemSet &rNew )
{
    SwFrm *pFrm = GetCurrFrm();
    if( pFrm && pFrm->IsInTab() )
    {
        SET_CURR_SHELL( this );
        StartAllAction();
        SwTabFrm *pTab = pFrm->FindTabFrm();
        pTab->GetTable()->SetHTMLTableLayout( 0 );
        GetDoc()->SetAttr( rNew, *pTab->GetFmt() );
        GetDoc()->SetModified();
        EndAllActionAndCall();
    }
}

/** move cursor within a table into previous/next row (same column)
 * @param pShell cursor shell whose cursor is to be moved
 * @param bUp true: move up, false: move down
 * @returns true if successful
 */
bool lcl_GoTableRow( SwCrsrShell* pShell, bool bUp )
{
    OSL_ENSURE( pShell != NULL, "need shell" );

    bool bRet = false;

    SwPaM* pPam = pShell->GetCrsr();
    const SwStartNode* pTableBox = pPam->GetNode()->FindTableBoxStartNode();
    OSL_ENSURE( pTableBox != NULL, "I'm living in a box... NOT!" );

    // move cursor to start node of table box
    pPam->GetPoint()->nNode = pTableBox->GetIndex();
    pPam->GetPoint()->nContent.Assign( NULL, 0 );
    GoInCntnt( *pPam, fnMoveForward );

    // go to beginning end of table box
    SwPosSection fnPosSect = bUp ? fnSectionStart : fnSectionEnd;
    pShell->MoveSection( fnSectionCurr, fnPosSect );

    // and go up/down into next content
    bRet = bUp ? pShell->Up() : pShell->Down();

    return bRet;
}

    // aender eine  Zellenbreite/-Hoehe/Spaltenbreite/Zeilenhoehe
sal_Bool SwFEShell::SetColRowWidthHeight( sal_uInt16 eType, sal_uInt16 nDiff )
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    if( nsTblChgWidthHeightType::WH_FLAG_INSDEL & eType &&
        pFrm->ImplFindTabFrm()->GetTable()->ISA( SwDDETable ))
    {
        ErrorHandler::HandleError( ERR_TBLDDECHG_ERROR,
                        ERRCODE_MSG_INFO | ERRCODE_BUTTON_DEF_OK );
        return sal_False;
    }

    SET_CURR_SHELL( this );
    StartAllAction();

    do {
        pFrm = pFrm->GetUpper();
    } while( !pFrm->IsCellFrm() );

    SwTabFrm *pTab = pFrm->ImplFindTabFrm();

    // if the table is in relative values (USHRT_MAX)
    // then it should be recalculated to absolute values now
    const SwFmtFrmSize& rTblFrmSz = pTab->GetFmt()->GetFrmSize();
    SWRECTFN( pTab )
    long nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
    if( TBLVAR_CHGABS == pTab->GetTable()->GetTblChgMode() &&
        ( eType & nsTblChgWidthHeightType::WH_COL_LEFT || eType & nsTblChgWidthHeightType::WH_COL_RIGHT ) &&
        text::HoriOrientation::NONE == pTab->GetFmt()->GetHoriOrient().GetHoriOrient() &&
        nPrtWidth != rTblFrmSz.GetWidth() )
    {
        SwFmtFrmSize aSz( rTblFrmSz );
        aSz.SetWidth( pTab->Prt().Width() );
        pTab->GetFmt()->SetFmtAttr( aSz );
    }

    if( (eType & (nsTblChgWidthHeightType::WH_FLAG_BIGGER | nsTblChgWidthHeightType::WH_FLAG_INSDEL)) ==
        (nsTblChgWidthHeightType::WH_FLAG_BIGGER | nsTblChgWidthHeightType::WH_FLAG_INSDEL) )
    {
        nDiff = sal_uInt16((pFrm->Frm().*fnRect->fnGetWidth)());

        // we must move the cursor outside the current cell before
        // deleting the cells.
        TblChgWidthHeightType eTmp =
            static_cast<TblChgWidthHeightType>( eType & 0xfff );
        switch( eTmp )
        {
        case nsTblChgWidthHeightType::WH_ROW_TOP:
            lcl_GoTableRow( this, true );
            break;
        case nsTblChgWidthHeightType::WH_ROW_BOTTOM:
            lcl_GoTableRow( this, false );
            break;
        case nsTblChgWidthHeightType::WH_COL_LEFT:
            GoPrevCell();
            break;
        case nsTblChgWidthHeightType::WH_COL_RIGHT:
            GoNextCell();
            break;
        default:
            break;
        }
    }

    SwTwips nLogDiff = nDiff;
    nLogDiff *= pTab->GetFmt()->GetFrmSize().GetWidth();
    nLogDiff /= nPrtWidth;

    /** The cells are destroyed in here */
    sal_Bool bRet = GetDoc()->SetColRowWidthHeight(
                    *(SwTableBox*)((SwCellFrm*)pFrm)->GetTabBox(),
                    eType, nDiff, nLogDiff );

    delete pLastCols, pLastCols = 0;
    EndAllActionAndCall();

    if( bRet && (eType & (nsTblChgWidthHeightType::WH_FLAG_BIGGER | nsTblChgWidthHeightType::WH_FLAG_INSDEL)) == nsTblChgWidthHeightType::WH_FLAG_INSDEL )
    {
        switch(eType & ~(nsTblChgWidthHeightType::WH_FLAG_BIGGER | nsTblChgWidthHeightType::WH_FLAG_INSDEL))
        {
        case nsTblChgWidthHeightType::WH_CELL_LEFT:
        case nsTblChgWidthHeightType::WH_COL_LEFT:
                GoPrevCell();
                break;

        case nsTblChgWidthHeightType::WH_CELL_RIGHT:
        case nsTblChgWidthHeightType::WH_COL_RIGHT:
                GoNextCell();
                break;

        case nsTblChgWidthHeightType::WH_CELL_TOP:
        case nsTblChgWidthHeightType::WH_ROW_TOP:
                lcl_GoTableRow( this, true );
                break;

        case nsTblChgWidthHeightType::WH_CELL_BOTTOM:
        case nsTblChgWidthHeightType::WH_ROW_BOTTOM:
                lcl_GoTableRow( this, false );
                break;
        }
    }

    return bRet;
}

sal_Bool lcl_IsFormulaSelBoxes( const SwTable& rTbl, const SwTblBoxFormula& rFml,
                            SwCellFrms& rCells )
{
    SwTblBoxFormula aTmp( rFml );
    SwSelBoxes aBoxes;

    for( sal_uInt16 nSelBoxes = aTmp.GetBoxesOfFormula( rTbl,aBoxes ); nSelBoxes; )
    {
        SwTableBox* pBox = aBoxes[ --nSelBoxes ];
        SwCellFrms::iterator iC;
        for( iC = rCells.begin(); iC != rCells.end(); ++iC )
            if( (*iC)->GetTabBox() == pBox )
                break;      // found

        if( iC == rCells.end() )
            return sal_False;
    }

    return sal_True;
}

    // ask formula for auto-sum
sal_Bool SwFEShell::GetAutoSum( String& rFml ) const
{
    SwFrm *pFrm = GetCurrFrm();
    SwTabFrm *pTab = pFrm ? pFrm->ImplFindTabFrm() : 0;
    if( !pTab )
        return sal_False;

    rFml = rtl::OUString::createFromAscii( sCalc_Sum );

    SwCellFrms aCells;
    if( ::GetAutoSumSel( *this, aCells ))
    {
        sal_uInt16 nW = 0, nInsPos = 0;
        for( size_t n = aCells.size(); n; )
        {
            SwCellFrm* pCFrm = aCells[ --n ];
            sal_uInt16 nBoxW = pCFrm->GetTabBox()->IsFormulaOrValueBox();
            if( !nBoxW )
                break;

            if( !nW )
            {
                if( USHRT_MAX == nBoxW )
                    continue;       // skip space at beginning

                rFml += '(';
                nInsPos = rFml.Len();

                // formula only if box is contained
                if( RES_BOXATR_FORMULA == nBoxW &&
                    !::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrm->
                    GetTabBox()->GetFrmFmt()->GetTblBoxFormula(), aCells))
                {
                    nW = RES_BOXATR_VALUE;
                    // restore previous spaces!
                    for( size_t i = aCells.size(); n+1 < i; )
                    {
                        String sTmp(rtl::OUString("|<"));
                        sTmp += aCells[ --i ]->GetTabBox()->GetName();
                        sTmp += '>';
                        rFml.Insert( sTmp, nInsPos );
                    }
                }
                else
                    nW = nBoxW;
            }
            else if( RES_BOXATR_VALUE == nW )
            {
                // search for values, Value/Formula/Text found -> include
                if( RES_BOXATR_FORMULA == nBoxW &&
                    ::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrm->
                        GetTabBox()->GetFrmFmt()->GetTblBoxFormula(), aCells ))
                    break;
                else if( USHRT_MAX != nBoxW )
                    rFml.Insert( cListDelim, nInsPos );
                else
                    break;
            }
            else if( RES_BOXATR_FORMULA == nW )
            {
                // only continue search when the current formula points to
                // all boxes contained in the selection
                if( RES_BOXATR_FORMULA == nBoxW )
                {
                    if( !::lcl_IsFormulaSelBoxes( *pTab->GetTable(), pCFrm->
                        GetTabBox()->GetFrmFmt()->GetTblBoxFormula(), aCells ))
                    {
                        // redo only for values!

                        nW = RES_BOXATR_VALUE;
                        rFml.Erase( nInsPos );
                        // restore previous spaces!
                        for( size_t i = aCells.size(); n+1 < i; )
                        {
                            String sTmp(rtl::OUString("|<" ));
                            sTmp += aCells[ --i ]->GetTabBox()->GetName();
                            sTmp += '>';
                            rFml.Insert( sTmp, nInsPos );
                        }
                    }
                    else
                        rFml.Insert( cListDelim, nInsPos );
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

            rtl::OUStringBuffer sTmp;
            sTmp.append('<');
            sTmp.append(pCFrm->GetTabBox()->GetName());
            sTmp.append('>');
            rFml.Insert( sTmp.makeStringAndClear(), nInsPos );
        }
        if( nW )
        {
            rFml += ')';
        }
    }

    return sal_True;
}

sal_Bool SwFEShell::IsTableRightToLeft() const
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    return pFrm->ImplFindTabFrm()->IsRightToLeft();
}

sal_Bool SwFEShell::IsMouseTableRightToLeft(const Point &rPt) const
{
    SwFrm *pFrm = (SwFrm *)GetBox( rPt );
    const SwTabFrm*  pTabFrm = pFrm ? pFrm->ImplFindTabFrm() : 0;
    OSL_ENSURE( pTabFrm, "Table not found" );
    return pTabFrm ? pTabFrm->IsRightToLeft() : sal_False;
}

sal_Bool SwFEShell::IsTableVertical() const
{
    SwFrm *pFrm = GetCurrFrm();
    if( !pFrm || !pFrm->IsInTab() )
        return sal_False;

    return pFrm->ImplFindTabFrm()->IsVertical();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
