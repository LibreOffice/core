/*************************************************************************
 *
 *  $RCSfile: tabfrm.cxx,v $
 *
 *  $Revision: 1.66 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:13:28 $
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


#pragma hdrstop

#include "pagefrm.hxx"
#include "rootfrm.hxx"
#include "cntfrm.hxx"
#include "viewsh.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "viewimp.hxx"
#include "swtable.hxx"
#include "dflyobj.hxx"
#include "flyfrm.hxx"
#include "frmtool.hxx"
#include "frmfmt.hxx"
#include "dcontact.hxx"
#include "viewopt.hxx"
#include "hints.hxx"
#include "dbg_lay.hxx"

#include <ftnidx.hxx>

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
// --> collapsing borders FME 2005-05-27 #i29550#
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
// <--
#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif

#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTROWSPLT_HXX //autogen
#include <fmtrowsplt.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SWTBLFMT_HXX
#include <swtblfmt.hxx>
#endif

#include "tabfrm.hxx"
#include "rowfrm.hxx"
#include "cellfrm.hxx"
#include "flyfrms.hxx"
#include "txtfrm.hxx"       //HasFtn()
#include "htmltbl.hxx"
#include "frmsh.hxx"
#include "sectfrm.hxx"  //SwSectionFrm
// OD 30.09.2003 #i18732#
#ifndef _FMTFOLLOWTEXTFLOW_HXX
#include <fmtfollowtextflow.hxx>
#endif
// --> OD 2004-06-28 #i28701#
#ifndef _SORTEDOBJS_HXX
#include <sortedobjs.hxx>
#endif
#ifndef _OBJECTFORMATTER_HXX
#include <objectformatter.hxx>
#endif
// <--

extern void AppendObjs( const SwSpzFrmFmts *pTbl, ULONG nIndex,
                        SwFrm *pFrm, SwPageFrm *pPage );


/*************************************************************************
|*
|*  SwTabFrm::SwTabFrm(), ~SwTabFrm()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
SwTabFrm::SwTabFrm( SwTable &rTab ):
    SwLayoutFrm( rTab.GetFrmFmt() ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( &rTab )
{
    bComplete = bCalcLowers = bONECalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = bHasFollowFlowLine = bIsRebuildLastLine =
    bRestrictTableGrowth = FALSE;
    BFIXHEIGHT = FALSE;     //Nicht nochmal auf die Importfilter hereinfallen.
    nType = FRMC_TAB;

    //Gleich die Zeilen erzeugen und einfuegen.
    const SwTableLines &rLines = rTab.GetTabLines();
    SwFrm *pTmpPrev = 0;
    for ( USHORT i = 0; i < rLines.Count(); ++i )
    {
        SwRowFrm *pNew = new SwRowFrm( *rLines[i] );
        if( pNew->Lower() )
        {
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
        else
            delete pNew;
    }
}

SwTabFrm::SwTabFrm( SwTabFrm &rTab ) :
    SwLayoutFrm( rTab.GetFmt() ),
    SwFlowFrm( (SwFrm&)*this ),
    pTable( rTab.GetTable() )
{
    bIsFollow = TRUE;
    bLockJoin = bComplete = bONECalcLowers = bCalcLowers = bLowersFormatted = bLockBackMove =
    bResizeHTMLTable = bHasFollowFlowLine = bIsRebuildLastLine =
    bRestrictTableGrowth = FALSE;
    BFIXHEIGHT = FALSE;     //Nicht nochmal auf die Importfilter hereinfallen.
    nType = FRMC_TAB;

    SetFollow( rTab.GetFollow() );
    rTab.SetFollow( this );
}

extern const SwTable   *pColumnCacheLastTable;
extern const SwTabFrm  *pColumnCacheLastTabFrm;
extern const SwFrm     *pColumnCacheLastCellFrm;
extern const SwTable   *pRowCacheLastTable;
extern const SwTabFrm  *pRowCacheLastTabFrm;
extern const SwFrm     *pRowCacheLastCellFrm;

SwTabFrm::~SwTabFrm()
{
    // There is some terrible code in fetab.cxx, that
    // makes use of these global pointers. Obviously
    // this code did not consider that a TabFrm can be
    // deleted.
    if ( this == pColumnCacheLastTabFrm )
    {
        pColumnCacheLastTable  = NULL;
        pColumnCacheLastTabFrm = NULL;
        pColumnCacheLastCellFrm= NULL;
        pRowCacheLastTable  = NULL;
        pRowCacheLastTabFrm = NULL;
        pRowCacheLastCellFrm= NULL;
    }
}

/*************************************************************************
|*
|*  SwTabFrm::JoinAndDelFollows()
|*
|*  Ersterstellung      MA 30. May. 96
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
void SwTabFrm::JoinAndDelFollows()
{
    SwTabFrm *pFoll = GetFollow();
    if ( pFoll->HasFollow() )
        pFoll->JoinAndDelFollows();
    pFoll->Cut();
    SetFollow( pFoll->GetFollow() );
    delete pFoll;
}

/*************************************************************************
|*
|*  SwTabFrm::RegistFlys()
|*
|*  Ersterstellung      MA 08. Jul. 93
|*  Letzte Aenderung    MA 27. Jan. 99
|*
|*************************************************************************/
void SwTabFrm::RegistFlys()
{
    ASSERT( Lower() && Lower()->IsRowFrm(), "Keine Zeilen." );

    SwPageFrm *pPage = FindPageFrm();
    if ( pPage )
    {
        SwRowFrm *pRow = (SwRowFrm*)Lower();
        do
        {   pRow->RegistFlys( pPage );
            pRow = (SwRowFrm*)pRow->GetNext();
        } while ( pRow );
    }
}

/*************************************************************************
|*  Some prototypes
|*************************************************************************/
void MA_FASTCALL SwInvalidateAll( SwFrm *pFrm, long nBottom );
BOOL MA_FASTCALL lcl_CalcLowers( SwLayoutFrm *pLay, long nBottom );
void MA_FASTCALL lcl_CalcLayout( SwLayoutFrm *pLay, long nBottom );
BOOL lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, BOOL bInva );
BOOL MA_FASTCALL lcl_InnerCalcLayout( SwFrm *pFrm, long nBottom );
// OD 2004-02-18 #106629# - correct type of 1st parameter
SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwRowFrm *pRow );
SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrm&, const SwBorderAttrs& );

/*************************************************************************
|*  START: local helper functions for repeated headlines
|*************************************************************************/

SwTwips lcl_GetHeightOfRows( const SwTabFrm& rTabFrm, USHORT nCount )
{
    if ( !nCount )
        return 0;

    SwTwips nRet = 0;
    const SwFrm* pRow = rTabFrm.Lower();
    SWRECTFN( pRow )
    while ( pRow && nCount > 0 )
    {
        nRet += (pRow->Frm().*fnRect->fnGetHeight)();
        pRow = pRow->GetNext();
        --nCount;
    }

    return nRet;

}

/*************************************************************************
|*  END: local helper functions for repeated headlines
|*************************************************************************/


/*************************************************************************
|*  START: local helper functions for splitting row frames
|*************************************************************************/

//
// Local helper function to insert a new follow flow line
//
SwRowFrm* lcl_InsertNewFollowFlowLine( SwTabFrm& rTab, const SwFrm& rTmpRow,
                                       USHORT nRepeat )
{
    ASSERT( rTmpRow.IsRowFrm(), "No row frame to copy for FollowFlowLine" )
    const SwRowFrm& rRow = (SwRowFrm&)rTmpRow;

    rTab.SetFollowFlowLine( TRUE );
    SwRowFrm *pFollowFlowLine = new SwRowFrm(*rRow.GetTabLine(), false );
    SwFrm* pFirstRow = rTab.GetFollow()->GetFirstNonHeadlineRow();
    pFollowFlowLine->InsertBefore( rTab.GetFollow(), pFirstRow );
    return pFollowFlowLine;
}

//
// Local helper function to shrink all lowers of rRow to 0 height
//
void lcl_ShrinkCellsAndAllContent( SwRowFrm& rRow )
{
    SwFrm* pCurrMasterCell = rRow.Lower();
    SWRECTFN( pCurrMasterCell )

    while ( pCurrMasterCell )
    {
        // TODO: Optimize number of frames which are set to 0 height
        SwFrm* pTmp = ((SwCellFrm*)pCurrMasterCell)->Lower();

        // we have to start with the last lower frame, otherwise
        // the shrink will not shrink the current cell
        while ( pTmp && pTmp->GetNext() )
            pTmp = pTmp->GetNext();

        if ( pTmp && pTmp->IsRowFrm() )
        {
            SwRowFrm* pTmpRow = (SwRowFrm*)pTmp;
            lcl_ShrinkCellsAndAllContent( *pTmpRow );
        }
        else
        {
            // TODO: Optimize number of frames which are set to 0 height
            while ( pTmp )
            {
                // the frames have to be shrunk
                if ( pTmp && pTmp->IsTabFrm() )
                {
                    SwRowFrm* pTmpRow = (SwRowFrm*)((SwTabFrm*)pTmp)->Lower();
                    while ( pTmpRow )
                    {
                        lcl_ShrinkCellsAndAllContent( *pTmpRow );
                        pTmpRow = (SwRowFrm*)pTmpRow->GetNext();
                    }
                }
                else
                {
                    pTmp->Shrink( (pTmp->Frm().*fnRect->fnGetHeight)() );
                    (pTmp->Prt().*fnRect->fnSetTop)( 0 );
                    (pTmp->Prt().*fnRect->fnSetHeight)( 0 );
                }

                pTmp = pTmp->GetPrev();
            }

            // all lowers should have the correct position
            lcl_ArrangeLowers( (SwCellFrm*)pCurrMasterCell,
                               (pCurrMasterCell->*fnRect->fnGetPrtTop)(),
                               sal_False );
        }

        pCurrMasterCell = pCurrMasterCell->GetNext();
    }
}

//
// Local helper function to move the content from rSourceLine to rDestLine
// The content is inserted behind the last content in the corresponding
// cell in rDestLine.
//
void lcl_MoveRowContent( SwRowFrm& rSourceLine, SwRowFrm& rDestLine )
{
    SwCellFrm* pCurrDestCell = (SwCellFrm*)rDestLine.Lower();
    SwCellFrm* pCurrSourceCell = (SwCellFrm*)rSourceLine.Lower();

    SWRECTFN( pCurrDestCell )

    // Move content of follow cells into master cells
    while ( pCurrSourceCell )
    {
        if ( pCurrSourceCell->Lower() && pCurrSourceCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
            while ( pTmpSourceRow )
            {
                if ( pTmpSourceRow->IsFollowFlowRow() )
                {
                    // move content from follow flow row to pTmpDestRow:
                    SwRowFrm* pTmpDestRow = (SwRowFrm*)pCurrDestCell->Lower();
                    while ( pTmpDestRow->GetNext() )
                        pTmpDestRow = (SwRowFrm*)pTmpDestRow->GetNext();

                    ASSERT( pTmpDestRow->GetFollowRow() == pTmpSourceRow, "Knoten in der Tabelle" )

                    lcl_MoveRowContent( *pTmpSourceRow, *pTmpDestRow );
                    pTmpDestRow->SetFollowRow( pTmpSourceRow->GetFollowRow() );
                    pTmpSourceRow->Remove();
                    delete pTmpSourceRow;
                }
                else
                {
                    // move complete row:
                    pTmpSourceRow->Remove();
                    pTmpSourceRow->InsertBefore( pCurrDestCell, 0 );
                }

                pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
            }
        }
        else
        {
            SwFrm *pTmp = ::SaveCntnt( (SwCellFrm*)pCurrSourceCell );
            if ( pTmp )
            {
                // Find last content
                SwFrm* pFrm = ((SwCellFrm*)pCurrDestCell)->Lower();
                while ( pFrm && pFrm->GetNext() )
                    pFrm = pFrm->GetNext();
                ::RestoreCntnt( pTmp, (SwCellFrm*)pCurrDestCell, pFrm, true );
            }
        }
        pCurrDestCell = (SwCellFrm*)pCurrDestCell->GetNext();
        pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
    }
}

//
// Local helper function to move all footnotes in rRowFrm from
// the footnote boss of rSource to the footnote boss of rDest.
//
void lcl_MoveFootnotes( SwTabFrm& rSource, SwTabFrm& rDest, SwLayoutFrm& rRowFrm )
{
    if ( 0 != rSource.GetFmt()->GetDoc()->GetFtnIdxs().Count() )
    {
        SwFtnBossFrm* pOldBoss = rSource.FindFtnBossFrm( TRUE );
        SwFtnBossFrm* pNewBoss = rDest.FindFtnBossFrm( TRUE );
        rRowFrm.MoveLowerFtns( 0, pOldBoss, pNewBoss, TRUE );
    }
}

//
// Local helper function to handle nested table cells before the split process
//
void lcl_PreprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine,
                                SwRowFrm& rFollowFlowLine, SwTwips nRemain )
{
    SwCellFrm* pCurrLastLineCell = (SwCellFrm*)rLastLine.Lower();
    SwCellFrm* pCurrFollowFlowLineCell = (SwCellFrm*)rFollowFlowLine.Lower();

    SWRECTFN( pCurrLastLineCell )

    //
    // Move content of follow cells into master cells
    //
    while ( pCurrLastLineCell )
    {
        if ( pCurrLastLineCell->Lower() && pCurrLastLineCell->Lower()->IsRowFrm() )
        {
            SwTwips nTmpCut = nRemain;
            SwRowFrm* pTmpLastLineRow = (SwRowFrm*)pCurrLastLineCell->Lower();

            SwTwips nCurrentHeight = lcl_CalcMinRowHeight( pTmpLastLineRow );;
            while ( pTmpLastLineRow && pTmpLastLineRow->GetNext() && nTmpCut > nCurrentHeight )
            {
                nTmpCut -= nCurrentHeight;
                pTmpLastLineRow = (SwRowFrm*)pTmpLastLineRow->GetNext();
                nCurrentHeight = lcl_CalcMinRowHeight( pTmpLastLineRow );
            }

            //
            // pTmpLastLineRow does not fit to the line or it is the last line
            //
            if ( pTmpLastLineRow )
            {
                //
                // Check if we can move pTmpLastLineRow to the follow table,
                // or if we have to split the line:
                //
                SwFrm* pCell = pTmpLastLineRow->Lower();
                bool bTableLayoutToComplex = false;
                long nMinHeight = 0;

                //
                // We have to take into account:
                // 1. The fixed height of the row
                // 2. The borders of the cells inside the row
                // 3. The minimum height of the row
                //
                if ( pTmpLastLineRow->HasFixSize() )
                    nMinHeight = (pTmpLastLineRow->Frm().*fnRect->fnGetHeight)();
                else
                {
                    while ( pCell )
                    {
                        if ( ((SwCellFrm*)pCell)->Lower() &&
                             ((SwCellFrm*)pCell)->Lower()->IsRowFrm() )
                        {
                            bTableLayoutToComplex = true;
                            break;
                        }

                        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCell );
                        const SwBorderAttrs &rAttrs = *aAccess.Get();
                        nMinHeight = Max( nMinHeight, lcl_CalcTopAndBottomMargin( *(SwLayoutFrm*)pCell, rAttrs ) );
                        pCell = pCell->GetNext();
                    }

                    const SwFmtFrmSize &rSz = pTmpLastLineRow->GetFmt()->GetFrmSize();
                    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE )
                        nMinHeight = Max( nMinHeight, rSz.GetHeight() );
                }

                //
                // 1. Case:
                // The line completely fits into the master table.
                // Nevertheless, we build a follow (otherwise painting problems
                // with empty cell).
                //
                // 2. Case:
                // The line has to be split, the minimum height still fits into
                // the master table, and the table structure is not to complex.
                //
                if ( nTmpCut > nCurrentHeight ||
                     ( pTmpLastLineRow->IsRowSplitAllowed() &&
                      !bTableLayoutToComplex && nMinHeight < nTmpCut ) )
                {
                    // The line has to be split:
                    SwRowFrm* pNewRow = new SwRowFrm( *pTmpLastLineRow->GetTabLine(), false );
                    pNewRow->SetFollowFlowRow( true );
                    pNewRow->SetFollowRow( pTmpLastLineRow->GetFollowRow() );
                    pTmpLastLineRow->SetFollowRow( pNewRow );
                    pNewRow->InsertBehind( pCurrFollowFlowLineCell, 0 );
                    pTmpLastLineRow = (SwRowFrm*)pTmpLastLineRow->GetNext();
                }

                //
                // The following lines have to be moved:
                //
                while ( pTmpLastLineRow )
                {
                    SwRowFrm* pTmp = (SwRowFrm*)pTmpLastLineRow->GetNext();
                    lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pTmpLastLineRow );
                    pTmpLastLineRow->Remove();
                    pTmpLastLineRow->InsertBefore( pCurrFollowFlowLineCell, 0 );
                    pTmpLastLineRow->Shrink( ( pTmpLastLineRow->Frm().*fnRect->fnGetHeight)() );
                    pCurrFollowFlowLineCell->Grow( ( pTmpLastLineRow->Frm().*fnRect->fnGetHeight)() );
                    pTmpLastLineRow = pTmp;
                }
            }
        }

        pCurrLastLineCell = (SwCellFrm*)pCurrLastLineCell->GetNext();
        pCurrFollowFlowLineCell = (SwCellFrm*)pCurrFollowFlowLineCell->GetNext();
    }
}

//
// Local helper function to handle nested table cells after the split process
//
void lcl_PostprocessRowsInCells( SwTabFrm& rTab, SwRowFrm& rLastLine,
                                 SwRowFrm& rFollowFlowLine )
{
    SwCellFrm* pCurrMasterCell = (SwCellFrm*)rLastLine.Lower();
    while ( pCurrMasterCell )
    {
        if ( pCurrMasterCell->Lower() &&
             pCurrMasterCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pRowFrm = (SwRowFrm*)pCurrMasterCell->Lower();
            while ( pRowFrm->GetNext() )
                pRowFrm = (SwRowFrm*)pRowFrm->GetNext();

            if ( NULL != pRowFrm->GetPrev() && !pRowFrm->ContainsCntnt() )
            {
                ASSERT( pRowFrm->GetFollowRow(), "Deleting row frame without follow" )

                // The footnotes have to be moved:
                lcl_MoveFootnotes( rTab, *rTab.GetFollow(), *pRowFrm );
                pRowFrm->Cut();
                SwRowFrm* pFollowRow = pRowFrm->GetFollowRow();
                pRowFrm->Paste( pFollowRow->GetUpper(), pFollowRow );
                pRowFrm->SetFollowRow( pFollowRow->GetFollowRow() );
                lcl_MoveRowContent( *pFollowRow, *pRowFrm );
                pFollowRow->Cut();
                delete pFollowRow;
                ::SwInvalidateAll( pCurrMasterCell, LONG_MAX );
            }
        }

        pCurrMasterCell = (SwCellFrm*)pCurrMasterCell->GetNext();
    }
}

//
// Local helper function to re-calculate the split line.
//
inline void TableSplitRecalcLock( SwFlowFrm *pTab ) { pTab->LockJoin(); }
inline void TableSplitRecalcUnlock( SwFlowFrm *pTab ) { pTab->UnlockJoin(); }

bool lcl_RecalcSplitLine( SwRowFrm& rLastLine, SwRowFrm& rFollowLine,
                          SwTwips nRemainingSpaceForLastRow )
{
    bool bRet = true;

    SwTabFrm& rTab = (SwTabFrm&)*rLastLine.GetUpper();

    //
    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some preprocessing.
    //
    lcl_PreprocessRowsInCells( rTab, rLastLine, rFollowLine, nRemainingSpaceForLastRow );

    //
    // Here the recalculation process starts:
    //
    rTab.SetRebuildLastLine( TRUE );
    SWRECTFN( rTab.GetUpper() )

    //
    // manipulate row and cell sizes
    //
    ::lcl_ShrinkCellsAndAllContent( rLastLine );

    //
    // invalidate last line
    //
    ::SwInvalidateAll( &rLastLine, LONG_MAX );

    //
    // Lock this tab frame and its follow
    //
    bool bUnlockMaster = false;
    bool bUnlockFollow = false;
    SwTabFrm* pMaster = rTab.IsFollow() ? (SwTabFrm*)rTab.FindMaster() : 0;
    if ( pMaster && !pMaster->IsJoinLocked() )
    {
        bUnlockMaster = true;
        ::TableSplitRecalcLock( pMaster );
    }
    if ( !rTab.GetFollow()->IsJoinLocked() )
    {
        bUnlockFollow = true;
        ::TableSplitRecalcLock( rTab.GetFollow() );
    }

    //
    // Do the recalculation
    //
    ::lcl_CalcLayout( &rLastLine, LONG_MAX );

    //
    // Unlock this tab frame and its follow
    //
    if ( bUnlockFollow )
        ::TableSplitRecalcUnlock( rTab.GetFollow() );
    if ( bUnlockMaster )
        ::TableSplitRecalcUnlock( pMaster );

    //
    // If there are nested cells in rLastLine, the recalculation of the last
    // line needs some postprocessing.
    //
    lcl_PostprocessRowsInCells( rTab, rLastLine, rFollowLine );

    //
    // Do a couple of checks on the current situation.
    //
    // If we are not happy with the current situation we return false.
    // This will start a new try to split the table, this time we do not
    // try to split the table rows.
    //

    //
    // 1. Check if table fits to its upper.
    //
    const SwTwips nDistanceToUpperPrtBottom =
            (rTab.Frm().*fnRect->fnBottomDist)( (rTab.GetUpper()->*fnRect->fnGetPrtBottom)());
    if ( nDistanceToUpperPrtBottom < 0 )
        bRet = false;

    //
    // 2. Check if each cell in the last line has at least one content frame.
    //
    // Note: a FollowFlowRow may contains empty cells!
    //
    if ( bRet )
    {
        if ( !rLastLine.IsInFollowFlowRow() )
        {
            SwCellFrm* pCurrMasterCell = (SwCellFrm*)rLastLine.Lower();
            while ( pCurrMasterCell )
            {
                if ( !pCurrMasterCell->ContainsCntnt() )
                {
                    bRet = false;
                    break;
                }
                pCurrMasterCell = (SwCellFrm*)pCurrMasterCell->GetNext();
            }
        }
    }

    //
    // 3. Check if follow flow line does not contain content:
    //
    if ( bRet )
    {
        if ( !rLastLine.ContainsCntnt() || !rFollowLine.ContainsCntnt() )
        {
            bRet = false;
        }
    }

    if ( bRet )
    {
        //
        // Everything looks fine. Splitting seems to be successful. We invalidate
        // rFollowLine to force a new formatting.
        //
        ASSERT( rTab.HasFollowFlowLine(), "!rTab.HasFollowFlowLine()" )
        ::SwInvalidateAll( &rFollowLine, LONG_MAX );
    }
    else
    {
        //
        // Splitting the table row gave us an unexpected result.
        // Everything has to be prepared for a second try to split
        // the table, this time without splitting the row.
        //
        ::SwInvalidateAll( &rLastLine, LONG_MAX );
    }

    rTab.SetRebuildLastLine( FALSE );

    return bRet;
}

/*************************************************************************
|*  END: local helper functions for splitting row frames
|*************************************************************************/

//
// Function to remove the FollowFlowLine of rTab.
// The content of the FollowFlowLine is moved to the associated line in the
// master table.
//
bool SwTabFrm::RemoveFollowFlowLine()
{
    // find FollowFlowLine
    SwFrm* pFollowFlowLine = GetFollow()->GetFirstNonHeadlineRow();

    ASSERT( HasFollowFlowLine() &&
            pFollowFlowLine, "There should be a flowline in the follow" )

    // find last row in master
    SwFrm* pLastLine = Lower();
    while ( pLastLine->GetNext() )
        pLastLine = pLastLine->GetNext();

    // We have to reset the flag here, because lcl_MoveRowContent
    // calls a GrowFrm(), which has a different bahavior if
    // this flag is set.
    SetFollowFlowLine( FALSE );
    lcl_MoveRowContent( *(SwRowFrm*)pFollowFlowLine, *(SwRowFrm*)pLastLine );

    bool bJoin = !pFollowFlowLine->GetNext();
    pFollowFlowLine->Cut();
    delete pFollowFlowLine;

    return bJoin;
}

#ifndef OD_FLYFRAMES_FINISHED

bool lcl_FindObjectsInRow( const SwRowFrm& rRow )
{
    bool bRet = false;
    SwCellFrm* pLower = (SwCellFrm*)rRow.Lower();
    while ( pLower )
    {
        if ( pLower->IsVertical() != rRow.IsVertical() )
            return true;

        SwFrm* pTmpFrm = pLower->Lower();
        while ( pTmpFrm )
        {
            if ( pTmpFrm->IsRowFrm() )
                bRet = lcl_FindObjectsInRow( *(SwRowFrm*)pTmpFrm );
            else
                bRet = 0 != pTmpFrm->GetDrawObjs() || pTmpFrm->IsSctFrm();

            if ( bRet )
                return true;
            pTmpFrm = pTmpFrm->GetNext();
        }

        pLower = (SwCellFrm*)pLower->GetNext();
    }
    return bRet;
}

#endif



/*************************************************************************
|*
|*  SwTabFrm::Split(), Join()
|*
|*  Ersterstellung      MA 03. Jun. 93
|*  Letzte Aenderung    MA 03. Sep. 96
|*
|*************************************************************************/
bool SwTabFrm::Split( const SwTwips nCutPos, bool bTryToSplit )
{
    bool bRet = true;

    SWRECTFN( this )
    ASSERT( bVert ? nCutPos >= Frm().Left()
            && nCutPos <= Frm().Left() + Frm().Width() :
            nCutPos >= Frm().Top() && nCutPos <= Frm().Bottom(),
            "SplitLine out of table." );

    //Um die Positionen der Zellen mit der CutPos zu vergleichen muessen sie
    //ausgehend von der Tabelle nacheinander berechnet werden. Sie koennen
    //wg. Positionsaenderungen der Tabelle durchaus ungueltig sein.
    SwFrm *pRow = Lower();
    if( !pRow )
        return bRet;

    const USHORT nRepeat = GetTable()->GetRowsToRepeat();
    USHORT nRowCount = 0;           // pRow currently points to the first row

    SwTwips nRemainingSpaceForLastRow =
        (*fnRect->fnYDiff)( nCutPos, (Frm().*fnRect->fnGetTop)() );
    nRemainingSpaceForLastRow -= (this->*fnRect->fnGetTopMargin)();

    //
    // Make pRow point to the line that does not fit anymore:
    //
    while( pRow->GetNext() &&
           nRemainingSpaceForLastRow >= (pRow->Frm().*fnRect->fnGetHeight)() )
    {
        nRemainingSpaceForLastRow -= (pRow->Frm().*fnRect->fnGetHeight)();
        pRow = pRow->GetNext();
        ++nRowCount;
    }

    //
    // bSplitRowAllowed: Row may be split according to its attributes.
    // bTryToSplit:      Row will never be split if bTryToSplit = false.
    //                   This can either be passed as a parameter, indicating
    //                   that we are currently doing the second try to split the
    //                   table, or it will be set to falseunder certain
    //                   conditions that are not suitable for splitting
    //                   the row.
    //
    bool bSplitRowAllowed = ((SwRowFrm*)pRow)->IsRowSplitAllowed();

#ifndef OD_FLYFRAMES_FINISHED
    // --> FME 2004-06-03 #i29438#
    // Special DoNotSplit case 1:
    // Search for objects anchored at content inside pRow:
    //
    if ( lcl_FindObjectsInRow( *(SwRowFrm*)pRow ) )
    {
        bTryToSplit = false;
    }
    // <--
#endif

    // --> FME 2004-06-07 #i29771#
    // To avoid loops, we do some checks before actually trying to split
    // the row. Maybe we should keep the next row in this table.
    // Note: This is only done if we are at the beginning of our upper
    bool bKeepNextRow = false;
    if ( nRowCount < nRepeat )
    {
        //
        // First case: One of the repeated headline does not fit to the page anymore.
        // At least one more non-heading row has to stay in this table in
        // order to avoid loops:
        //
        ASSERT( !GetIndPrev(), "Table is supposed to be at beginning" )
        bKeepNextRow = true;
    }
    else if ( !GetIndPrev() && nRepeat == nRowCount )
    {
        //
        // Second case: The first non-headline row does not fit to the page.
        // If it is not allowed to be split, or it contains a sub-row that
        // is not allowed to be split, we keep the row in this table:
        //
        if ( bTryToSplit && bSplitRowAllowed )
        {
            // Check if there are (first) rows inside this row,
            // which are not allowed to be split.
            SwCellFrm* pLower = pRow ? (SwCellFrm*)((SwRowFrm*)pRow)->Lower() : 0;
            while ( pLower )
            {
                if ( pLower->Lower() && pLower->Lower()->IsRowFrm() )
                {
                    const SwRowFrm* pLowerRow = (SwRowFrm*)pLower->Lower();
                    if ( !pLowerRow->IsRowSplitAllowed() &&
                        (pLowerRow->Frm().*fnRect->fnGetHeight)() >
                        nRemainingSpaceForLastRow )
                    {
                        bKeepNextRow = true;
                        break;
                    }
                }
                pLower = (SwCellFrm*)pLower->GetNext();
            }
        }
        else
            bKeepNextRow = true;
    }

    //
    // Better keep the next row in this table:
    //
    if ( bKeepNextRow )
    {
        pRow = GetFirstNonHeadlineRow();
        if ( pRow )
            pRow = pRow->GetNext();
    }

    //
    // No more row to split or to move to follow table:
    //
    if ( !pRow )
        return bRet;

    //
    // We try to split the row if
    // - the attributes of the row are set accordingly and
    // - we are allowed to do so
    //
    bSplitRowAllowed = bSplitRowAllowed && bTryToSplit;

    //
    // If we do not indent to split pRow, we check if we are
    // allowed to move pRow to a follow. Otherwise we return
    // false, indicating an error
    //
    if ( !bSplitRowAllowed )
    {
        if ( pRow == GetFirstNonHeadlineRow() )
            return false;
    }

    //
    // Build follow table if not already done:
    //
    FASTBOOL bNewFollow;
    SwTabFrm *pFoll;
    if ( GetFollow() )
    {
        pFoll = GetFollow();
        bNewFollow = FALSE;
    }
    else
    {
        bNewFollow = TRUE;
        pFoll = new SwTabFrm( *this );

        //
        // We give the follow table an initial width.
        //
        (pFoll->Frm().*fnRect->fnAddWidth)( (Frm().*fnRect->fnGetWidth)() );
        (pFoll->Prt().*fnRect->fnAddWidth)( (Prt().*fnRect->fnGetWidth)() );

        //
        // Insert the new follow table
        //
        pFoll->InsertBehind( GetUpper(), this );

        //
        // Repeat the headlines.
        //
        for ( nRowCount = 0; nRowCount < nRepeat; ++nRowCount )
        {
            // Insert new headlines:
            bDontCreateObjects = TRUE;              //frmtool
            SwRowFrm* pHeadline = new SwRowFrm(
                                    *GetTable()->GetTabLines()[ nRowCount ] );
            pHeadline->SetRepeatedHeadline( true );
            bDontCreateObjects = FALSE;
            pHeadline->InsertBefore( pFoll, 0 );

            SwPageFrm *pPage = pHeadline->FindPageFrm();
            const SwSpzFrmFmts *pTbl = GetFmt()->GetDoc()->GetSpzFrmFmts();
            if( pTbl->Count() )
            {
                ULONG nIndex;
                SwCntntFrm* pFrm = pHeadline->ContainsCntnt();
                while( pFrm )
                {
                    nIndex = pFrm->GetNode()->GetIndex();
                    AppendObjs( pTbl, nIndex, pFrm, pPage );
                    pFrm = pFrm->GetNextCntntFrm();
                    if( !pHeadline->IsAnLower( pFrm ) )
                        break;
                }
            }
        }
    }

    // If the row that does not fit anymore is allowed
    // to be split, the next row has to be moved to the follow table.
    SwRowFrm* pLastRow = (SwRowFrm*)pRow;
    SwRowFrm* pFollowRow = 0;
    if ( bSplitRowAllowed )
    {
        pRow = pRow->GetNext();

        // new follow flow line for last row of master table
        pFollowRow = lcl_InsertNewFollowFlowLine( *this, *pLastRow, nRepeat );
    }

    SwTwips nRet = 0;
    SwFrm* pNxt = 0;
    SwFrm* pPrv = pFollowRow;

    //Optimierung beim neuen Follow braucht's kein Paste und dann kann
    //das Optimierte Insert verwendet werden (nur dann treten gluecklicher weise
    //auch groessere Mengen von Rows auf).
    if ( bNewFollow )
    {
        if ( !bSplitRowAllowed )
        {
            pPrv = pFoll->Lower();
            while( pPrv && pPrv->GetNext() )
                pPrv = pPrv->GetNext();
        }

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();
            // The footnotes do not have to be moved, this is done in the
            // MoveFwd of the follow table!!!
            pRow->Remove();
            pRow->InsertBehind( pFoll, pPrv );
            pRow->_InvalidateAll();
            pPrv = pRow;
            pRow = pNxt;
        }
    }
    else
    {
        if ( !bSplitRowAllowed )
            pPrv = pFoll->GetFirstNonHeadlineRow();
        else
            pPrv = pPrv->GetNext();

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nRet += (pRow->Frm().*fnRect->fnGetHeight)();

            // The footnotes have to be moved:
            lcl_MoveFootnotes( *this, *GetFollow(), (SwRowFrm&)*pRow );

            pRow->Remove();
            pRow->Paste( pFoll, pPrv );

            pRow->CheckDirChange();
            pRow = pNxt;
        }
    }

    Shrink( nRet );

    // we build a new last line to assure that it will be fully formatted
    if ( bSplitRowAllowed )
    {
        ASSERT( pFollowRow, "Table Split: FollowFlowRow is missing" )

        // recalculate the split line
        bRet = lcl_RecalcSplitLine( *pLastRow, *pFollowRow, nRemainingSpaceForLastRow );
    }

    return bRet;
}

bool SwTabFrm::Join()
{
    ASSERT( !HasFollowFlowLine(), "Joining follow flow line" )

    SwTabFrm *pFoll = GetFollow();
    SwTwips nHeight = 0;    //Gesamthoehe der eingefuegten Zeilen als Return.

    if ( !pFoll->IsJoinLocked() )
    {
        SWRECTFN( this )
        pFoll->Cut();   //Erst ausschneiden um unuetze Benachrichtigungen zu
                        //minimieren.

        SwFrm *pRow = pFoll->GetFirstNonHeadlineRow(),
              *pNxt;

        SwFrm *pPrv = Lower();
        while ( pPrv && pPrv->GetNext() )
            pPrv = pPrv->GetNext();

        while ( pRow )
        {
            pNxt = pRow->GetNext();
            nHeight += (pRow->Frm().*fnRect->fnGetHeight)();
            pRow->Remove();
            pRow->_InvalidateAll();
            pRow->InsertBehind( this, pPrv );
            pRow->CheckDirChange();
            pPrv = pRow;
            pRow = pNxt;
        }

        SetFollow( pFoll->GetFollow() );
        SetFollowFlowLine( pFoll->HasFollowFlowLine() );
        delete pFoll;

        Grow( nHeight PHEIGHT );
    }

    return true;
}

/*************************************************************************
|*
|*  SwTabFrm::MakeAll()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 10. Apr. 97
|*
|*************************************************************************/
void MA_FASTCALL SwInvalidatePositions( SwFrm *pFrm, long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    BOOL bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {   pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        if( pFrm->IsLayoutFrm() )
        {
            if ( ((SwLayoutFrm*)pFrm)->Lower() )
                ::SwInvalidatePositions( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
        }
        else
            pFrm->Prepare( PREP_ADJUST_FRM );
        pFrm = pFrm->GetNext();
    } while ( pFrm &&
              ( bAll ||
              (*fnRect->fnYDiff)( (pFrm->Frm().*fnRect->fnGetTop)(), nBottom ) < 0 ) );
}

void MA_FASTCALL SwInvalidateAll( SwFrm *pFrm, long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    BOOL bAll = LONG_MAX == nBottom;
    SWRECTFN( pFrm )
    do
    {   pFrm->_InvalidatePos();
        pFrm->_InvalidateSize();
        pFrm->_InvalidatePrt();
        if( pFrm->IsLayoutFrm() )
        {
            if ( ((SwLayoutFrm*)pFrm)->Lower() )
                ::SwInvalidateAll( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
        }
        else
            pFrm->Prepare( PREP_CLEAR );
        pFrm = pFrm->GetNext();
    } while ( pFrm &&
              ( bAll ||
              (*fnRect->fnYDiff)( (pFrm->Frm().*fnRect->fnGetTop)(), nBottom ) < 0 ) );
}

// --> collapsing borders FME 2005-05-27 #i29550#
void lcl_InvalidateAllLowersPrt( SwLayoutFrm* pLayFrm )
{
    pLayFrm->_InvalidatePrt();
    pLayFrm->_InvalidateSize();
    pLayFrm->SetCompletePaint();

    SwFrm* pFrm = pLayFrm->Lower();

    while ( pFrm )
    {
        if ( pFrm->IsLayoutFrm() )
            lcl_InvalidateAllLowersPrt( (SwLayoutFrm*)pFrm );
        else
        {
            pFrm->_InvalidatePrt();
            pFrm->_InvalidateSize();
            pFrm->SetCompletePaint();
        }

        pFrm = pFrm->GetNext();
    }
}
// <-- collapsing

BOOL MA_FASTCALL lcl_CalcLowers( SwLayoutFrm *pLay, long nBottom )
{
    if ( !pLay )
        return TRUE;

    // LONG_MAX == nBottom means we have to calculate all
    BOOL bAll = LONG_MAX == nBottom;
    BOOL bRet = FALSE;
    SwCntntFrm *pCnt = pLay->ContainsCntnt();
    SWRECTFN( pLay )
    // OD 2004-06-03 #i28701#
    const SwPageFrm& rPageFrm = *(pLay->FindPageFrm());
    while ( pCnt && pLay->GetUpper()->IsAnLower( pCnt ) )
    {
        bRet |= !pCnt->IsValid();
        // OD 2004-07-01 #i28701# - use new method <SwFrm::InvalidateObjs(..)>
        // No format is performed for the floating screen objects.
        pCnt->InvalidateObjs( true );
        pCnt->Calc();
        // OD 2004-05-11 #i28701# - usage of new method <::FormatObjsAtFrm(..)>
        // to format the floating screen objects
        if ( pCnt->IsTxtFrm() )
        {
            if ( !SwObjectFormatter::FormatObjsAtFrm( *pCnt, rPageFrm ) )
            {
                // restart format with first content
                pCnt = pLay->ContainsCntnt();
                continue;
            }
        }
        pCnt->GetUpper()->Calc();
        if( ! bAll && (*fnRect->fnYDiff)((pCnt->Frm().*fnRect->fnGetTop)(), nBottom) > 0 )
            break;
        pCnt = pCnt->GetNextCntntFrm();
    }
    return bRet;
}

BOOL MA_FASTCALL lcl_InnerCalcLayout( SwFrm *pFrm, long nBottom )
{
    // LONG_MAX == nBottom means we have to calculate all
    BOOL bAll = LONG_MAX == nBottom;
    BOOL bRet = FALSE;
    const SwFrm* pOldUp = pFrm->GetUpper();
    SWRECTFN( pFrm )
    do
    {
        if( pFrm->IsLayoutFrm() )
        {
            bRet |= !pFrm->IsValid();
            pFrm->Calc();
            if( ((SwLayoutFrm*)pFrm)->Lower() )
                bRet |= lcl_InnerCalcLayout( ((SwLayoutFrm*)pFrm)->Lower(), nBottom);
        }
        pFrm = pFrm->GetNext();
    } while( pFrm &&
            ( bAll ||
              (*fnRect->fnYDiff)((pFrm->Frm().*fnRect->fnGetTop)(), nBottom) < 0 )
            && pFrm->GetUpper() == pOldUp );
    return bRet;
}

void MA_FASTCALL lcl_CalcLayout( SwLayoutFrm *pLay, long nBottom )
{
    BOOL bCheck = TRUE;
    do
    {
        while( lcl_InnerCalcLayout( pLay, nBottom ) )
            bCheck = TRUE;
        if( bCheck )
        {
            bCheck = FALSE;
            if( lcl_CalcLowers( pLay, nBottom ) )
                continue;
        }
        break;
    } while( TRUE );
}

void MA_FASTCALL lcl_FirstTabCalc( SwTabFrm *pTab )
{
    SWRECTFN( pTab )
    if ( !pTab->IsFollow() && !pTab->GetTable()->IsTblComplex() )
    {
        SwLayoutFrm *pRow = (SwLayoutFrm*)pTab->Lower();
        do
        {
            SwLayoutFrm *pCell = (SwLayoutFrm*)pRow->Lower();
            SwFrm *pCnt = pCell->Lower();
            pCnt->Calc();
            const long nCellHeight = (pCell->Frm().*fnRect->fnGetHeight)();
            const long nCellY      = (pCell->Frm().*fnRect->fnGetTop)()-1;
            const long nCntHeight  = (pCnt->Frm().*fnRect->fnGetHeight)();
            const long nCntY       = (pCnt->Frm().*fnRect->fnGetTop)()-1;
            if ( 0 != (pCell = (SwLayoutFrm*)pCell->GetNext()) )
                do
                {   (pCell->Frm().*fnRect->fnSetTopAndHeight)
                                                        ( nCellY, nCellHeight );
                    (pCell->Prt().*fnRect->fnSetHeight)( nCellHeight );
                    pCell->_InvalidateAll();

                    pCnt = pCell->Lower();
                    (pCnt->Frm().*fnRect->fnSetTopAndHeight)(nCntY, nCntHeight);
                    (pCnt->Prt().*fnRect->fnSetHeight)( nCntHeight );
                    pCnt->_InvalidateAll();

                    pCell = (SwLayoutFrm*)pCell->GetNext();
                } while ( pCell );

            SwTwips nRowTop = (pRow->Frm().*fnRect->fnGetTop)();
            SwTwips nUpBot = (pTab->GetUpper()->Frm().*fnRect->fnGetBottom)();
            if( (*fnRect->fnYDiff)( nUpBot, nRowTop ) < 0 )
                break;
            pRow = (SwLayoutFrm*)pRow->GetNext();

        } while ( pRow );
    }
    SwFrm *pUp = pTab->GetUpper();
    long nBottom = (pUp->*fnRect->fnGetPrtBottom)();
    if ( pTab->GetFmt()->GetDoc()->IsBrowseMode() )
        nBottom += pUp->Grow( LONG_MAX, TRUE );
    lcl_CalcLowers( (SwLayoutFrm*)pTab->Lower(), LONG_MAX );
}

void MA_FASTCALL lcl_Recalc( SwTabFrm *pTab,
                             SwLayoutFrm *pFirstRow,
                             SwLayNotify &rNotify )
{
    if ( pTab->Lower() )
    {
        SWRECTFN( pTab )
        const SwTwips nOldHeight = (pTab->Frm().*fnRect->fnGetHeight)();
        const SwTwips nOldWidth  = (pTab->Frm().*fnRect->fnGetWidth)();
        if ( !pFirstRow )
        {
            pFirstRow = (SwLayoutFrm*)pTab->Lower();
            rNotify.SetLowersComplete( TRUE );
        }
        ::SwInvalidatePositions( pFirstRow, LONG_MAX );
        ::lcl_CalcLayout( pFirstRow, LONG_MAX );
    }
}

// This is a new function to check the first condition whether
// a tab frame may move backward. It replaces the formerly used
// GetIndPrev(), which did not work correctly for #i5947#
bool lcl_NoPrev( const SwFrm& rFrm )
{
    if ( rFrm.GetPrev() )
        return false;

    if ( !rFrm.GetIndPrev() )
        return true;

    // I do not have a direct prev, but I have an indirect prev.
    // In section frames I have to check if I'm located inside
    // the first column:
    if ( rFrm.IsInSct() )
    {
        const SwFrm* pSct = rFrm.GetUpper();
        if ( pSct && pSct->IsColBodyFrm() &&
            (pSct = pSct->GetUpper()->GetUpper())->IsSctFrm() )
        {
            const SwFrm* pPrevCol = rFrm.GetUpper()->GetUpper()->GetPrev();
            if ( pPrevCol )
                // I'm not inside the first column and do not have a direct
                // prev. I can try to go backward.
                return true;
        }
    }

    return false;
}

#define KEEPTAB ( !GetFollow() && !IsFollow() )

void SwTabFrm::MakeAll()
{
    if ( IsJoinLocked() || StackHack::IsLocked() || StackHack::Count() > 50 )
        return;

    if ( HasFollow() )
    {
        SwTabFrm* pFollow = (SwTabFrm*)GetFollow();
        ASSERT( !pFollow->IsJoinLocked() || !pFollow->IsRebuildLastLine(),
                "SwTabFrm::MakeAll for master while follow is in RebuildLastLine()" )
        if ( pFollow->IsJoinLocked() && pFollow->IsRebuildLastLine() )
            return;
    }

    PROTOCOL_ENTER( this, PROT_MAKEALL, 0, 0 )

    LockJoin(); //Ich lass mich nicht unterwegs vernichten.
    SwLayNotify aNotify( this );    //uebernimmt im DTor die Benachrichtigung
    // If pos is invalid, we have to call a SetInvaKeep at aNotify.
    // Otherwise the keep atribute would not work in front of a table.
    const BOOL bOldValidPos = GetValidPosFlag();

    //Wenn mein direkter Nachbar gleichzeitig mein Follow ist
    //verleibe ich mir das Teil ein.
    // OD 09.04.2003 #108698# - join all follows, which are placed on the
    // same page/column.
    // OD 29.04.2003 #109213# - join follow, only if join for the follow
    // is not locked. Otherwise, join will not be performed and this loop
    // will be endless.
    while ( GetNext() && GetNext() == GetFollow() &&
            !GetFollow()->IsJoinLocked()
          )
    {
        if ( HasFollowFlowLine() )
            RemoveFollowFlowLine();
        Join();
    }

    // The bRemoveFollowFlowLinePending is set if the split attribute of the
    // last line is set:
    if ( IsRemoveFollowFlowLinePending() && HasFollowFlowLine() )
    {
        if ( RemoveFollowFlowLine() )
            Join();
        SetRemoveFollowFlowLinePending( FALSE );
    }

    if ( bResizeHTMLTable ) //Optimiertes Zusammenspiel mit Grow/Shrink des Inhaltes
    {
        bResizeHTMLTable = FALSE;
        SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
        if ( pLayout )
            bCalcLowers = pLayout->Resize(
                            pLayout->GetBrowseWidthByTabFrm( *this ), FALSE );
    }


    BOOL bMakePage  = TRUE;     //solange TRUE kann eine neue Seite
                                //angelegt werden (genau einmal)
    BOOL bMovedBwd  = FALSE;    //Wird TRUE wenn der Frame zurueckfliesst
    BOOL bMovedFwd  = FALSE;    //solange FALSE kann der Frm zurueck-
                                //fliessen (solange, bis er einmal
                                //vorwaerts ge'moved wurde).
    BOOL bSplit     = FALSE;    //Wird TRUE wenn der Frm gesplittet wurde.
    const BOOL bFtnsInDoc = 0 != GetFmt()->GetDoc()->GetFtnIdxs().Count();
    BOOL bMoveable;
    const BOOL bFly     = IsInFly();

    SwBorderAttrAccess  *pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
    const SwBorderAttrs *pAttrs = pAccess->Get();

    // The beloved keep attribute
    const bool bKeep = IsKeep( *pAttrs );
    // All rows should keep together
    // OD 2004-05-25 #i21478# - don't split table, if it has to keep with next
    const bool bDontSplit = !IsFollow() &&
                            ( !GetFmt()->GetLayoutSplit().GetValue() || bKeep );
    // The number of repeated headlines
    const USHORT nRepeat = GetTable()->GetRowsToRepeat();
    // This flag indicates that something we are allowed to try to split the
    // table rows.
    bool bTryToSplit = true;

    // Join follow table, if this table is not allowed to split:
    if ( bDontSplit )
    {
        while ( GetFollow() && !GetFollow()->IsJoinLocked() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    // Join follow table, if this does not have enough (repeated) lines:
    if ( nRepeat )
    {
        if( GetFollow() && !GetFollow()->IsJoinLocked() &&
            0 == GetFirstNonHeadlineRow() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            Join();
        }
    }

    //Einen Frischling moven wir gleich schon einmal vorwaerts...
    if ( !Frm().Top() && IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
        {
            if ( !MoveFwd( bMakePage, FALSE ) )
                bMakePage = FALSE;
            bMovedFwd = TRUE;
        }
    }

    SWRECTFN( this )
    while ( !bValidPos || !bValidSize || !bValidPrtArea )
    {
        if ( TRUE == (bMoveable = IsMoveable()) )
            if ( CheckMoveFwd( bMakePage, bKeep && KEEPTAB, bMovedBwd ) )
            {
                bMovedFwd = TRUE;
                bCalcLowers = TRUE;
            }

        Point aOldPos( (Frm().*fnRect->fnGetPos)() );
        MakePos();

        if ( aOldPos != (Frm().*fnRect->fnGetPos)() )
        {
            if ( aOldPos.Y() != (Frm().*fnRect->fnGetTop)() )
            {
                SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
                if( pLayout )
                {
                    delete pAccess;
                    bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ), FALSE );
                    pAccess = new SwBorderAttrAccess( SwFrm::GetCache(), this );
                    pAttrs = pAccess->Get();
                }

                bValidPrtArea = FALSE;
                aNotify.SetLowersComplete( FALSE );
            }
            SwFrm *pPre;
            if ( bKeep || (0 != (pPre = FindPrev()) &&
                           pPre->GetAttrSet()->GetKeep().GetValue()) )
            {
                bCalcLowers = TRUE;
            }
        }

        //Wir muessen die Hoehe der ersten Zeile kennen, denn nur wenn diese
        //kleiner wird muss ggf. der Master angestossen werden um noetigenfalls
        //die Zeile aufzunehmen.
        long n1StLineHeight = 0;
        if ( IsFollow() )
        {
            SwFrm* pFrm = GetFirstNonHeadlineRow();
            if ( pFrm )
                n1StLineHeight = (pFrm->Frm().*fnRect->fnGetHeight)();
        }

        if ( !bValidSize || !bValidPrtArea )
        {
            const BOOL bOptLower = (Frm().*fnRect->fnGetHeight)() == 0;

            const long nOldPrtWidth = (Prt().*fnRect->fnGetWidth)();
            const long nOldFrmWidth = (Frm().*fnRect->fnGetWidth)();
            const Point aOldPrtPos  = (Prt().*fnRect->fnGetPos)();
            Format( pAttrs );

            SwHTMLTableLayout *pLayout = GetTable()->GetHTMLTableLayout();
            if ( /*!bOptLower &&*/ pLayout &&
                 ((Prt().*fnRect->fnGetWidth)() != nOldPrtWidth ||
                  (Frm().*fnRect->fnGetWidth)() != nOldFrmWidth) )
            {
                delete pAccess;
                bCalcLowers |= pLayout->Resize(
                        pLayout->GetBrowseWidthByTabFrm( *this ), FALSE );
//                  GetFmt()->GetDoc()->GetDocShell()->IsReadOnly() ? FALSE : TRUE );
                pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                pAttrs = pAccess->Get();
            }
            if ( !bOptLower && aOldPrtPos != (Prt().*fnRect->fnGetPos)() )
                aNotify.SetLowersComplete( FALSE );

            if ( bOptLower && Lower() )
            {
                //MA 24. May. 95: Optimierungsversuch!
                //Ganz nigel nagel neu das Teil. Damit wir nicht n-fach
                //MakeAll'en formatieren wir flugs den Inhalt.
                //Das erste Format mussten wir allerdings abwarten, damit
                //die Breiten Stimmen!
                //MA: Fix, Kein Calc wenn evtl. noch Seitengebunde Flys
                //an den Cntnt haengen (siehe frmtool.cxx, ~SwCntntNotify).
                SwDoc *pDoc = GetFmt()->GetDoc();
                if ( !pDoc->GetSpzFrmFmts()->Count() ||
                     pDoc->IsLoaded() || pDoc->IsNewDoc() )
                {
                    //MA 28. Nov. 95: Und wieder ein Trick, gleich mal sehen
                    //ob ein Rueckfluss lohnt.
                    if ( bMoveable && !GetPrev() )
                    {
                        GetLeaf( MAKEPAGE_NONE, FALSE ); //setzt das BackMoveJump
                        if ( SwFlowFrm::IsMoveBwdJump() )
                        {
                            BOOL bDummy;
                            SwFtnBossFrm *pOldBoss = bFtnsInDoc ?
                                FindFtnBossFrm( TRUE ) : 0;
                            const FASTBOOL bOldPrev = GetPrev() != 0;
                            if ( MoveBwd( bDummy ) )
                            {
                                SWREFRESHFN( this )
                                bMovedBwd = TRUE;
                                if ( bFtnsInDoc )
                                    MoveLowerFtns( 0, pOldBoss, 0, TRUE );

                                long nOldTop = (Frm().*fnRect->fnGetTop)();
                                MakePos();
                                if( nOldTop != (Frm().*fnRect->fnGetTop)() )
                                {
                                    SwHTMLTableLayout *pLayout =
                                        GetTable()->GetHTMLTableLayout();
                                    if( pLayout )
                                    {
                                        delete pAccess;
                                        bCalcLowers |= pLayout->Resize(
                                            pLayout->GetBrowseWidthByTabFrm(
                                                            *this ), FALSE );
                                        pAccess= new SwBorderAttrAccess(
                                                    SwFrm::GetCache(), this );
                                        pAttrs = pAccess->Get();
                                    }
                                }

                                if ( bOldPrev != (0 != GetPrev()) )
                                {
                                    //Abstand nicht vergessen!
                                    bValidPrtArea = FALSE;
                                    Format( pAttrs );
                                }
                                if ( bKeep && KEEPTAB )
                                {
                                    SwFrm *pNxt = FindNextCnt();
                                    // FindNextCnt geht ggf. in einen Bereich
                                    // hinein, in eine Tabelle allerdings auch
                                    if( pNxt && pNxt->IsInTab() )
                                        pNxt = pNxt->FindTabFrm();
                                    if ( pNxt )
                                    {
                                        pNxt->Calc();
                                        if ( !GetNext() )
                                            bValidPos = FALSE;
                                    }
                                }
                            }
                        }
                    }
                    ::lcl_FirstTabCalc( this );
                    bValidSize = bValidPrtArea = FALSE;
                    Format( pAttrs );
                    aNotify.SetLowersComplete( TRUE );
                }
            }
        }

        //Wenn ich der erste einer Kette bin koennte ich mal sehen ob
        //ich zurueckfliessen kann (wenn ich mich ueberhaupt bewegen soll).
        //Damit es keine Oszillation gibt, darf ich nicht gerade vorwaerts
        //geflosssen sein.
        if ( !bMovedFwd && (bMoveable || bFly) && lcl_NoPrev( *this ) )
        {
            //Bei Follows muss der Master benachrichtigt
            //werden. Der Follow muss nur dann Moven, wenn er leere Blaetter
            //ueberspringen muss.
            if ( IsFollow() )
            {
                //Nur wenn die Hoehe der ersten Zeile kleiner geworder ist.
                SwFrm *pFrm = GetFirstNonHeadlineRow();
                if( pFrm && n1StLineHeight >(pFrm->Frm().*fnRect->fnGetHeight )() )
                {
                    SwTabFrm *pMaster = (SwTabFrm*)FindMaster();
                    BOOL bDummy;
                    if ( ShouldBwdMoved( pMaster->GetUpper(), FALSE, bDummy ) )
                        pMaster->InvalidatePos();
                }
            }
            SwFtnBossFrm *pOldBoss = bFtnsInDoc ? FindFtnBossFrm( TRUE ) : 0;
            BOOL bReformat;
            if ( MoveBwd( bReformat ) )
            {
                SWREFRESHFN( this )
                bMovedBwd = TRUE;
                aNotify.SetLowersComplete( FALSE );
                if ( bFtnsInDoc )
                    MoveLowerFtns( 0, pOldBoss, 0, TRUE );
                if ( bReformat || bKeep )
                {
                    long nOldTop = (Frm().*fnRect->fnGetTop)();
                    MakePos();
                    if( nOldTop != (Frm().*fnRect->fnGetTop)() )
                    {
                        SwHTMLTableLayout *pLayout =
                            GetTable()->GetHTMLTableLayout();
                        if( pLayout )
                        {
                            delete pAccess;
                            bCalcLowers |= pLayout->Resize(
                                pLayout->GetBrowseWidthByTabFrm( *this ),
                                FALSE );
                            pAccess= new SwBorderAttrAccess(
                                        SwFrm::GetCache(), this );
                            pAttrs = pAccess->Get();
                        }

                        bValidPrtArea = FALSE;
                        Format( pAttrs );
                    }
                    ::lcl_Recalc( this, 0, aNotify );
                    bLowersFormatted = TRUE;
                    if ( bKeep && KEEPTAB )
                    {
                        SwFrm *pNxt = FindNextCnt();
                        if( pNxt && pNxt->IsInTab() )
                            pNxt = pNxt->FindTabFrm();
                        if ( pNxt )
                        {
                            pNxt->Calc();
                            if ( !GetNext() )
                                bValidPos = FALSE;
                        }
                    }
                }
            }
        }

        //Wieder ein Wert ungueltig? - dann nochmal das ganze...
        if ( !bValidPos || !bValidSize || !bValidPrtArea )
            continue;

        // check, if calculation of table frame is ready.

        /// OD 23.10.2002 #103517# - Local variable <nDistanceToUpperPrtBottom>
        ///     Introduce local variable and init it with the distance from the
        ///     table frame bottom to the bottom of the upper printing area.
        /// Note: negative values denotes the situation that table frame doesn't
        ///     fit in its upper.

        SwTwips nDistanceToUpperPrtBottom =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());

        /// OD 23.10.2002 #103517# - In online layout try to grow upper of table
        /// frame, if table frame doesn't fit in its upper.
        if ( nDistanceToUpperPrtBottom < 0 &&
             GetFmt()->GetDoc()->IsBrowseMode() )
        {
            if ( GetUpper()->Grow( -nDistanceToUpperPrtBottom ) )
            {
                // upper is grown --> recalculate <nDistanceToUpperPrtBottom>
                nDistanceToUpperPrtBottom =
                    (Frm().*fnRect->fnBottomDist)( (GetUpper()->*fnRect->fnGetPrtBottom)());
            }
        }

        if( nDistanceToUpperPrtBottom >= 0 )
        {
            // OD 23.10.2002 - translate german commentary
            // If there is space left in the upper printing area, join as for trial
            // at least one further row of an existing follow.
            if ( !bSplit && GetFollow() )
            {
                BOOL bDummy;
                if ( GetFollow()->ShouldBwdMoved( GetUpper(), FALSE, bDummy ) )
                {
                    SwFrm *pTmp = GetUpper();
                    SwTwips nDeadLine = (pTmp->*fnRect->fnGetPrtBottom)();
                    if ( GetFmt()->GetDoc()->IsBrowseMode() )
                        nDeadLine += pTmp->Grow( LONG_MAX, TRUE );
                    if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
                    {
                        //
                        // First, we remove an existing follow flow line.
                        //
                        if ( HasFollowFlowLine() )
                        {
                            RemoveFollowFlowLine();
                            // invalidate and rebuild last row
                            SwFrm* pLastLine = Lower();
                            while ( pLastLine->GetNext() )
                                pLastLine = pLastLine->GetNext();

                            ::SwInvalidateAll( pLastLine, LONG_MAX );
                            SetRebuildLastLine( TRUE );
                            ::lcl_CalcLayout( (SwLayoutFrm*)pLastLine, LONG_MAX );
                            SetRebuildLastLine( FALSE );

                            SwFrm* pRow = GetFollow()->GetFirstNonHeadlineRow();

                            if ( !pRow || !pRow->GetNext() )
                                //Der Follow wird leer und damit ueberfluessig.
                                Join();

                            continue;
                        }

                        //
                        // If there is no follow flow line, we move the first
                        // row in the follow table to the master table.
                        //
                        SwFrm *pRow = GetFollow()->GetFirstNonHeadlineRow();

                        const SwTwips nOld = (Frm().*fnRect->fnGetHeight)();

                        const BOOL bMoveFtns = bFtnsInDoc && pRow &&
                                               !GetFollow()->IsJoinLocked();

                        SwFtnBossFrm *pOldBoss;
                        if ( bMoveFtns )
                            pOldBoss = pRow->FindFtnBossFrm( TRUE );

                        //fix(8680): Row kann 0 werden.
                        if ( !pRow || !pRow->GetNext() )
                            //Der Follow wird leer und damit ueberfluessig.
                            Join();
                        else
                        {
                            pRow->Cut();
                            pRow->Paste( this );
                        }
                        //Die Fussnoten verschieben!
                        if ( pRow && bMoveFtns )
                            if ( ((SwLayoutFrm*)pRow)->MoveLowerFtns(
                                 0, pOldBoss, FindFtnBossFrm( TRUE ), TRUE ) )
                                GetUpper()->Calc();

                        if ( pRow && nOld != (Frm().*fnRect->fnGetHeight)() )
                            ::lcl_Recalc( this, (SwLayoutFrm*)pRow, aNotify );
                        continue;
                    }
                }
            }
            else if ( bKeep && KEEPTAB )
            {
                SwFrm *pNxt = FindNextCnt();
                if( pNxt && pNxt->IsInTab() )
                    pNxt = pNxt->FindTabFrm();
                if ( pNxt )
                    pNxt->Calc();
            }
            if ( IsValid() )
            {
                if ( bCalcLowers )
                {
                    ::lcl_Recalc( this, 0, aNotify );
                    bLowersFormatted = TRUE;
                    bCalcLowers = FALSE;
                }
                else if ( bONECalcLowers )
                {
                    lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
                    bONECalcLowers = FALSE;
                }
            }
            continue;
        }

        //Ich passe nicht mehr in meinen Uebergeordneten, also ist es jetzt
        //an der Zeit moeglichst konstruktive Veranderungen vorzunehmen

        //Wenn ich den uebergeordneten Frm nicht verlassen darf, habe
        //ich ein Problem; Frei nach Artur Dent tun wir das einzige das man
        //mit einen nicht loesbaren Problem tun kann: wir ignorieren es - und
        //zwar mit aller Kraft.
        if ( !bMoveable )
        {
            if ( bCalcLowers && IsValid() )
            {
                lcl_Recalc( this, 0, aNotify );
                bLowersFormatted = TRUE;
                bCalcLowers = FALSE;
            }
            else if ( bONECalcLowers )
            {
                lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
                bONECalcLowers = FALSE;
            }
            continue;
        }

        if ( bCalcLowers && IsValid() )
        {
            ::lcl_Recalc( this, 0, aNotify );
            bLowersFormatted = TRUE;
            bCalcLowers = FALSE;
            if( !IsValid() )
                continue;
        }

        //Der erste Versuch muss natuerlich das Aufspalten der Tabelle sein.
        //Das funktioniert natuerlich nur dann, wenn die Tabelle mehr als eine
        //Zeile enthaelt und wenn die Unterkante des Upper unter der ersten
        //Zeile liegt.
        SwFrm* pIndPrev = GetIndPrev();
        const SwRowFrm* pFirstNonHeadlineRow = GetFirstNonHeadlineRow();
        if ( pFirstNonHeadlineRow && ( !bDontSplit || !pIndPrev ) )
        {
            // --> FME 2004-06-03 #i29438#
            // Special DoNotSplit case:
            // We better avoid splitting of a row frame if we are inside a columned
            // section which has a height of 0, because this is not growable and thus
            // all kinds of unexpected things could happen.
            const SwSectionFrm* pTmpSct = 0;
            if ( IsInSct() &&
                (pTmpSct = FindSctFrm())->Lower()->IsColumnFrm() &&
                0 == (GetUpper()->Frm().*fnRect->fnGetHeight)()  )
            {
                bTryToSplit = false;
            }
            // <--

            // 1. Try: bTryToSplit = true  => Try to split the row.
            // 2. Try: bTryToSplit = false => Split the table between the rows.
            if ( pFirstNonHeadlineRow->GetNext() || bTryToSplit )
            {
                SwTwips nDeadLine = (GetUpper()->*fnRect->fnGetPrtBottom)();
                if( IsInSct() || GetUpper()->IsInTab() ) // TABLE IN TABLE)
                    nDeadLine = (*fnRect->fnYInc)( nDeadLine,
                                        GetUpper()->Grow( LONG_MAX, TRUE ) );

                ::lcl_CalcLayout( (SwLayoutFrm*)Lower(), nDeadLine );
                bLowersFormatted = TRUE;
                aNotify.SetLowersComplete( TRUE );
                if( (Frm().*fnRect->fnBottomDist)( nDeadLine ) > 0 )
                {
                    continue;
                }

                USHORT nMinNumOfLines = nRepeat;
                if ( !bTryToSplit )
                    ++nMinNumOfLines;

                const SwTwips nBreakLine = (*fnRect->fnYInc)(
                        (Frm().*fnRect->fnGetTop)(),
                        (this->*fnRect->fnGetTopMargin)() +
                         lcl_GetHeightOfRows( *this, nMinNumOfLines ) );

                if( (*fnRect->fnYDiff)(nDeadLine, nBreakLine) >=0 || !pIndPrev )
                {
                    aNotify.SetLowersComplete( FALSE );
                    bSplit = TRUE;

                    //
                    // An existing follow flow line has to be removed.
                    //
                    if ( HasFollowFlowLine() )
                        RemoveFollowFlowLine();

                    const bool bSplitError = !Split( nDeadLine, bTryToSplit );
                    ASSERT( bTryToSplit || !bSplitError,
                            "We did not try to split, why to we get a split error?" )
                    bTryToSplit = !bSplitError;

                    // --> FME 2004-06-09 #i29771# Two tries to split the table:
                    if ( bSplitError )
                    {
                        // An error occured during splitting. We start a second
                        // try, this time without splitting of table rows.
                        if ( HasFollowFlowLine() )
                            RemoveFollowFlowLine();

                        if ( GetFollow() && !GetFollow()->GetFirstNonHeadlineRow() )
                            Join();

                        // We want to restore the situation before the failed
                        // split operation as good as possible. Therefore we
                        // do some more calculations. Note: Restricting this
                        // to nDeadLine may not be enough.
                        ::lcl_CalcLayout( (SwLayoutFrm*)Lower(), LONG_MAX );
                        bValidPos = FALSE;
                        continue;
                    }
                    // <--

                    //Damit es nicht zu Oszillationen kommt, muss der
                    //Follow gleich gueltig gemacht werden.
                    if ( GetFollow() )
                    {
                        SWRECTFN( GetFollow() )

                        static BYTE nStack = 0;
                        if ( !StackHack::IsLocked() && nStack < 4 )
                        {
                            ++nStack;
                            StackHack aHack;
                            delete pAccess;

                            GetFollow()->MakeAll();

                            pAccess= new SwBorderAttrAccess( SwFrm::GetCache(), this );
                            pAttrs = pAccess->Get();

                            ((SwTabFrm*)GetFollow())->SetLowersFormatted(FALSE);
                            ::lcl_CalcLayout((SwLayoutFrm*)GetFollow()->Lower(),
                                (GetFollow()->GetUpper()->Frm().*fnRect->fnGetBottom)() );

                            if ( !GetFollow()->GetFollow() )
                            {
                                SwFrm* pNxt = ((SwFrm*)GetFollow())->FindNext();
                                if ( pNxt )
                                {
                                    // OD 26.08.2003 #i18103# - no formatting
                                    // of found next frame, if its a follow
                                    // section of the 'ColLocked' section,
                                    // the follow table is in.
                                    bool bCalcNxt = true;
                                    if ( GetFollow()->IsInSct() && pNxt->IsSctFrm() )
                                    {
                                        SwSectionFrm* pSct = GetFollow()->FindSctFrm();
                                        if ( pSct->IsColLocked() &&
                                             pSct->GetFollow() == pNxt )
                                        {
                                            bCalcNxt = false;
                                        }
                                    }
                                    if ( bCalcNxt )
                                    {
                                        pNxt->Calc();
                                    }
                                }
                            }
                            --nStack;
                        }
                        else if ( GetFollow() == GetNext() )
                            ((SwTabFrm*)GetFollow())->MoveFwd( TRUE, FALSE );
                        ViewShell *pSh;
                        if ( 0 != (pSh = GetShell()) )
                            pSh->Imp()->ResetScroll();
                    }
                    continue;
                }
            }
        }

        if( IsInSct() && bMovedFwd && bMakePage && GetUpper()->IsColBodyFrm() &&
            GetUpper()->GetUpper()->GetUpper()->IsSctFrm() &&
            ( GetUpper()->GetUpper()->GetPrev() || GetIndPrev() ) &&
            ((SwSectionFrm*)GetUpper()->GetUpper()->GetUpper())->MoveAllowed(this) )
            bMovedFwd = FALSE;

        // --> FME 2004-06-09 #i29771# Reset bTryToSplit flag on change of upper
        const SwFrm* pOldUpper = GetUpper();
        // <--

        //Mal sehen ob ich irgenwo Platz finde...
        if ( !bMovedFwd && !MoveFwd( bMakePage, FALSE ) )
            bMakePage = FALSE;

        // --> FME 2004-06-09 #i29771# Reset bSplitError flag on change of upper
        if ( GetUpper() != pOldUpper )
            bTryToSplit = true;
        // <--

        SWREFRESHFN( this )
        bMovedFwd = bCalcLowers = TRUE;
        aNotify.SetLowersComplete( FALSE );
        if ( IsFollow() )
        {   //Um Oszillationen zu vermeiden sollte kein ungueltiger Master
            //zurueckbleiben.
            SwTabFrm *pTab = FindMaster();
            if ( pTab->GetUpper() )
                pTab->GetUpper()->Calc();
            pTab->Calc();
            pTab->SetLowersFormatted( FALSE );
        }

        //Wenn mein direkter Nachbar jetzt gleichzeitig mein Follow ist
        //verleibe ich mir das Teil ein.
        if ( ( GetNext() && GetNext() == GetFollow() ) || !GetLower() )
        {
            if ( HasFollowFlowLine() )
                RemoveFollowFlowLine();
            if ( GetFollow() )
                Join();
        }

        if ( bMovedBwd && GetUpper() )
            //Beim zurueckfliessen wurde der Upper angeregt sich vollstaendig
            //zu Painten, dass koennen wir uns jetzt nach dem hin und her
            //fliessen sparen.
            GetUpper()->ResetCompletePaint();

        if ( bCalcLowers && IsValid() )
        {
            ::lcl_Recalc( this, 0, aNotify );
            bLowersFormatted = TRUE;
            bCalcLowers = FALSE;
        }

    } //while ( !bValidPos || !bValidSize || !bValidPrtArea )

    //Wenn mein direkter Vorgaenger jetzt mein Master ist, so kann er mich
    //bei der nachstbesten Gelegenheit vernichten.
    if ( IsFollow() )
    {
        SwFrm *pPre = GetPrev();
        if ( pPre && pPre->IsTabFrm() && ((SwTabFrm*)pPre)->GetFollow() == this)
            pPre->InvalidatePos();
    }

    bCalcLowers = bONECalcLowers = FALSE;
    delete pAccess;
    UnlockJoin();
    if ( bMovedFwd || bMovedBwd || !bOldValidPos )
        aNotify.SetInvaKeep();
}

/*************************************************************************
|*
|*  SwTabFrm::CalcFlyOffsets()
|*
|*  Beschreibung:       Berechnet die Offsets, die durch FlyFrames
|*                      entstehen.
|*  Ersterstellung      MA/MIB 14. Apr. 99
|*  Letzte Aenderung
|*
|*************************************************************************/
BOOL SwTabFrm::CalcFlyOffsets( SwTwips& rUpper,
                               long& rLeftOffset,
                               long& rRightOffset ) const
{
    BOOL bInvalidatePrtArea = FALSE;
    const SwPageFrm *pPage = FindPageFrm();
    const SwFlyFrm* pMyFly = FindFlyFrm();

    // --> #108724# Page header/footer content doesn't have to wrap around
    //              floating screen objects
    const SwDoc* pDoc = GetFmt()->GetDoc();
    const sal_Bool bWrapAllowed = pDoc->IsFormerTextWrapping() ||
                                ( !IsInFtn() && 0 == FindFooterOrHeader() );
    // <--

    if ( pPage->GetSortedObjs() && bWrapAllowed )
    {
        SWRECTFN( this )
        long nPrtPos = (Frm().*fnRect->fnGetTop)();
        nPrtPos = (*fnRect->fnYInc)( nPrtPos, rUpper );
        SwRect aRect( Frm() );
        long nYDiff = (*fnRect->fnYDiff)( (Prt().*fnRect->fnGetTop)(), rUpper );
        if( nYDiff > 0 )
            (aRect.*fnRect->fnAddBottom)( -nYDiff );
        for ( USHORT i = 0; i < pPage->GetSortedObjs()->Count(); ++i )
        {
            SwAnchoredObject* pAnchoredObj = (*pPage->GetSortedObjs())[i];
            if ( pAnchoredObj->ISA(SwFlyFrm) )
            {
                SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);
                const SwRect aFlyRect = pFly->GetObjRectWithSpaces();
                if ( WEIT_WECH != (pFly->Frm().*fnRect->fnGetTop)() &&
                     pFly->IsFlyAtCntFrm() && aFlyRect.IsOver( aRect ) &&
                     // OD 25.02.2003 #i9040# - use '<=' instead of '<'
                     (*fnRect->fnYDiff)(
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetBottom)(),
                            (Frm().*fnRect->fnGetTop)() ) <= 0 &&
                     !IsAnLower( pFly ) && !pFly->IsAnLower( this ) &&
                     ( !pMyFly || pMyFly->IsAnLower( pFly ) ) &&
                     pPage->GetPhyPageNum() >=
                     pFly->GetAnchorFrm()->FindPageFrm()->GetPhyPageNum() &&
                     // anchor should be in same page body/header/footer
                     ( pFly->GetAnchorFrm()->FindFooterOrHeader() ==
                       FindFooterOrHeader() ) )
                {
                    const SwFmtSurround   &rSur = pFly->GetFmt()->GetSurround();
                    const SwFmtHoriOrient &rHori= pFly->GetFmt()->GetHoriOrient();
                    if ( SURROUND_NONE == rSur.GetSurround() )
                    {
                        long nBottom = (aFlyRect.*fnRect->fnGetBottom)();
                        if( (*fnRect->fnYDiff)( nPrtPos, nBottom ) < 0 )
                            nPrtPos = nBottom;
                        bInvalidatePrtArea = TRUE;
                    }
                    if ( (SURROUND_RIGHT    == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         HORI_LEFT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (aFlyRect.*fnRect->fnGetRight)(),
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetLeft)() );
                        rLeftOffset = Max( rLeftOffset, nWidth );
                        bInvalidatePrtArea = TRUE;
                    }
                    if ( (SURROUND_LEFT     == rSur.GetSurround() ||
                          SURROUND_PARALLEL == rSur.GetSurround())&&
                         HORI_RIGHT == rHori.GetHoriOrient() )
                    {
                        const long nWidth = (*fnRect->fnXDiff)(
                            (pFly->GetAnchorFrm()->Frm().*fnRect->fnGetRight)(),
                            (aFlyRect.*fnRect->fnGetLeft)() );
                        rRightOffset = Max( rRightOffset, nWidth );
                        bInvalidatePrtArea = TRUE;
                    }
                }
            }
        }
        rUpper = (*fnRect->fnYDiff)( nPrtPos, (Frm().*fnRect->fnGetTop)() );
    }

    return bInvalidatePrtArea;
}

/*************************************************************************
|*
|*  SwTabFrm::Format()
|*
|*  Beschreibung:       "Formatiert" den Frame; Frm und PrtArea
|*                      Die Fixsize wird hier nicht eingestellt.
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 18. Jun. 97
|*
|*************************************************************************/
void SwTabFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "TabFrm::Format, pAttrs ist 0." );

    SWRECTFN( this )
    if ( !bValidSize )
    {
        long nDiff = (GetUpper()->Prt().*fnRect->fnGetWidth)() -
                     (Frm().*fnRect->fnGetWidth)();
        if( nDiff )
            (aFrm.*fnRect->fnAddRight)( nDiff );
    }

    //VarSize ist immer die Hoehe.
    //Fuer den oberen/unteren Rand gelten die selben Regeln wie fuer
    //cntfrms (sie MakePrtArea() von diesen).

    SwTwips nUpper = CalcUpperSpace( pAttrs );

    //Wir wollen Rahmen ausweichen. Zwei Moeglichkeiten:
    //1. Es gibt Rahmen mit SurroundNone, diesen wird vollsaendig ausgewichen
    //2. Es gibt Rahmen mit Umlauf nur rechts bzw. nur links und diese sind
    //   rechts bzw. links ausgerichtet, diese geben ein Minimum fuer die
    //   Raender vor.
    long nTmpRight = -1000000,
         nLeftOffset  = 0;
    if( CalcFlyOffsets( nUpper, nLeftOffset, nTmpRight ) )
        bValidPrtArea = FALSE;
    long nRightOffset = Max( 0L, nTmpRight );

    SwTwips nLower = pAttrs->CalcBottomLine();
    // --> collapsing borders FME 2005-05-27 #i29550#
    if ( IsCollapsingBorders() )
        nLower += GetBottomLineSize();
    // <-- collapsing

    if ( !bValidPrtArea )
    {   bValidPrtArea = TRUE;

        //Die Breite der PrtArea wird vom FrmFmt vorgegeben, die Raender
        //sind entsprechend einzustellen.
        //Mindestraender werden von Umrandung und Schatten vorgegeben.
        //Die Rander werden so eingestellt, dass die PrtArea nach dem
        //angegebenen Adjustment im Frm ausgerichtet wird.
        //Wenn das Adjustment 0 ist, so werden die Rander anhand des
        //Randattributes eingestellt.

        const SwTwips nOldHeight = (Prt().*fnRect->fnGetHeight)();
        const SwTwips nMax = (aFrm.*fnRect->fnGetWidth)();

        // OD 14.03.2003 #i9040# - adjust variable names.
        const SwTwips nLeftLine  = pAttrs->CalcLeftLine();
        const SwTwips nRightLine = pAttrs->CalcRightLine();

        //Die Breite ist evtl. eine Prozentangabe. Wenn die Tabelle irgendwo
        //'drinsteckt bezieht sie sich auf die Umgebung. Ist es der Body, so
        //bezieht sie sich in der BrowseView auf die Bildschirmbreite.
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        // OD 14.03.2003 #i9040# - adjust variable name.
        const SwTwips nWishedTableWidth = CalcRel( rSz, TRUE );

        BOOL bCheckBrowseWidth = FALSE;

        // OD 14.03.2003 #i9040# - insert new variables for left/right spacing.
        SwTwips nLeftSpacing  = 0;
        SwTwips nRightSpacing = 0;
        switch ( GetFmt()->GetHoriOrient().GetHoriOrient() )
        {
            case HORI_LEFT:
                {
                    // left indent:
                    nLeftSpacing = nLeftLine + nLeftOffset;
                    // OD 06.03.2003 #i9040# - correct calculation of right indent:
                    // - Consider right indent given by right line attributes.
                    // - Consider negative right indent.
                    // wished right indent determined by wished table width and
                    // left offset given by surround fly frames on the left:
                    const SwTwips nWishRight = nMax - nWishedTableWidth - nLeftOffset;
                    if ( nRightOffset > 0 )
                    {
                        // surrounding fly frames on the right
                        // -> right indent is maximun of given right offset
                        //    and wished right offset.
                        nRightSpacing = nRightLine + Max( nRightOffset, nWishRight );
                    }
                    else
                    {
                        // no surrounding fly frames on the right
                        // If intrinsic right indent (intrinsic means not considering
                        // determined left indent) is negative,
                        //      then hold this intrinsic indent,
                        //      otherwise non negative wished right indent is hold.
                        nRightSpacing = nRightLine +
                                        ( ( (nWishRight+nLeftOffset) < 0 ) ?
                                            (nWishRight+nLeftOffset) :
                                            Max( 0L, nWishRight ) );
                    }
                }
                break;
            case HORI_RIGHT:
                {
                    // right indent:
                    nRightSpacing = nRightLine + nRightOffset;
                    // OD 06.03.2003 #i9040# - correct calculation of left indent:
                    // - Consider left indent given by left line attributes.
                    // - Consider negative left indent.
                    // wished left indent determined by wished table width and
                    // right offset given by surrounding fyl frames on the right:
                    const SwTwips nWishLeft = nMax - nWishedTableWidth - nRightOffset;
                    if ( nLeftOffset > 0 )
                    {
                        // surrounding fly frames on the left
                        // -> right indent is maximun of given left offset
                        //    and wished left offset.
                        nLeftSpacing = nLeftLine + Max( nLeftOffset, nWishLeft );
                    }
                    else
                    {
                        // no surrounding fly frames on the left
                        // If intrinsic left indent (intrinsic = not considering
                        // determined right indent) is negative,
                        //      then hold this intrinsic indent,
                        //      otherwise non negative wished left indent is hold.
                        nLeftSpacing = nLeftLine +
                                       ( ( (nWishLeft+nRightOffset) < 0 ) ?
                                           (nWishLeft+nRightOffset) :
                                           Max( 0L, nWishLeft ) );
                    }
                }
                break;
            case HORI_CENTER:
                {
                    // OD 07.03.2003 #i9040# - consider left/right line attribute.
                    // OD 10.03.2003 #i9040# -
                    const SwTwips nCenterSpacing = ( nMax - nWishedTableWidth ) / 2;
                    nLeftSpacing = nLeftLine +
                                   ( (nLeftOffset > 0) ?
                                     Max( nCenterSpacing, nLeftOffset ) :
                                     nCenterSpacing );
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      Max( nCenterSpacing, nRightOffset ) :
                                      nCenterSpacing );
                }
                break;
            case HORI_FULL:
                    //Das Teil dehnt sich ueber die gesamte Breite aus.
                    //Nur die fuer die Umrandung benoetigten Freiraeume
                    //werden beruecksichtigt.
                    //Die Attributwerte von LRSpace werden bewusst missachtet!
                    bCheckBrowseWidth = TRUE;
                    nLeftSpacing  = nLeftLine + nLeftOffset;
                    nRightSpacing = nRightLine + nRightOffset;
                break;
            case HORI_NONE:
                {
                    //Die Raender werden vom Randattribut bestimmt.
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of left spacing and left offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = Max( nLeftSpacing, ( nLeftOffset + nLeftLine ) );
                    }
                    // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                    nRightSpacing = pAttrs->CalcRight( this );
                    if( nRightOffset )
                    {
                        // OD 07.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider right line attribute.
                        nRightSpacing = Max( nRightSpacing, ( nRightOffset + nRightLine ) );
                    }
                    // OD 10.03.2003 #i9040# - do not hold wished table width.
                    /*
                    if ( !pAttrs->GetLRSpace().GetRight() )
                        nRight = Max( nRight, nMax - (nWish + nLeft + nRight));
                    */
                }
                break;
            case HORI_LEFT_AND_WIDTH:
                {
                    //Linker Rand und die Breite zaehlen (Word-Spezialitaet)
                    // OD 10.03.2003 #i9040# - no width alignment in online mode.
                    //bCheckBrowseWidth = TRUE;
                    nLeftSpacing = pAttrs->CalcLeft( this );
                    if( nLeftOffset )
                    {
                        // OD 10.03.2003 #i9040# - surround fly frames only, if
                        // they overlap with the table.
                        // Thus, take maximun of right spacing and right offset.
                        // OD 10.03.2003 #i9040# - consider left line attribute.
                        nLeftSpacing = Max( nLeftSpacing, ( pAttrs->CalcLeftLine() + nLeftOffset ) );
                    }
                    // OD 10.03.2003 #i9040# - consider right and left line attribute.
                    const SwTwips nWishRight =
                            nMax - (nLeftSpacing-pAttrs->CalcLeftLine()) - nWishedTableWidth;
                    nRightSpacing = nRightLine +
                                    ( (nRightOffset > 0) ?
                                      Max( nWishRight, nRightOffset ) :
                                      nWishRight );
                }
                break;
            default:
                ASSERT( FALSE, "Ungueltige orientation fuer Table." );
        }

        // --> OD 2004-07-15 #i26250# - extend bottom printing area, if table
        // is last content inside a table cell.
        if ( GetFmt()->GetDoc()->IsAddParaSpacingToTableCells() &&
             GetUpper()->IsInTab() && !GetIndNext() )
        {
            nLower += pAttrs->GetULSpace().GetLower();
        }
        // <--
        (this->*fnRect->fnSetYMargins)( nUpper, nLower );
        if( (nMax - MINLAY) < (nLeftSpacing + nRightSpacing) )
            (this->*fnRect->fnSetXMargins)( 0, 0 );
        else
            (this->*fnRect->fnSetXMargins)( nLeftSpacing, nRightSpacing );

        ViewShell *pSh;
        if ( bCheckBrowseWidth && GetFmt()->GetDoc()->IsBrowseMode() &&
             GetUpper()->IsPageBodyFrm() &&  // nur PageBodyFrms, nicht etwa ColBodyFrms
             0 != (pSh = GetShell()) && pSh->VisArea().Width() )
        {
            //Nicht ueber die Kante des sichbaren Bereiches hinausragen.
            //Die Seite kann breiter sein, weil es Objekte mit "ueberbreite"
            //geben kann (RootFrm::ImplCalcBrowseWidth())
            const Size aBorder = pSh->GetOut()->PixelToLogic( pSh->GetBrowseBorder() );
            long nWidth = pSh->VisArea().Width() - 2 * aBorder.Width();
            nWidth -= Prt().Left();
            nWidth -= pAttrs->CalcRightLine();
            Prt().Width( Min( nWidth, Prt().Width() ) );
        }

        if ( nOldHeight != (Prt().*fnRect->fnGetHeight)() )
            bValidSize = FALSE;
    }

    if ( !bValidSize )
    {
        bValidSize = TRUE;

        //Die Groesse wird durch den Inhalt plus den Raendern bestimmt.
        SwTwips nRemaining = 0, nDiff;
        SwFrm *pFrm = pLower;
        while ( pFrm )
        {
            nRemaining += (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }
        //Jetzt noch die Raender addieren
        nRemaining += nUpper + nLower;

        nDiff = (Frm().*fnRect->fnGetHeight)() - nRemaining;
        if ( nDiff > 0 )
            Shrink( nDiff PHEIGHT );
        else if ( nDiff < 0 )
            Grow( -nDiff PHEIGHT );
    }
}
/*************************************************************************
|*
|*  SwTabFrm::GrowFrm()
|*
|*  Ersterstellung      MA 12. Mar. 93
|*  Letzte Aenderung    MA 23. Sep. 96
|*
|*************************************************************************/
SwTwips SwTabFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    SWRECTFN( this )
    SwTwips nHeight =(Frm().*fnRect->fnGetHeight)();
    if( nHeight > 0 && nDist > ( LONG_MAX - nHeight ) )
        nDist = LONG_MAX - nHeight;

    if ( bTst && !IsRestrictTableGrowth() )
        return nDist;

    if ( GetUpper() )
    {
        SwRect aOldFrm( Frm() );

        //Der Upper wird nur soweit wie notwendig gegrowed. In nReal wird erstmal
        //die bereits zur Verfuegung stehende Strecke bereitgestellt.
        SwTwips nReal = (GetUpper()->Prt().*fnRect->fnGetHeight)();
        SwFrm *pFrm = GetUpper()->Lower();
        while ( pFrm && GetFollow() != pFrm )
        {
            nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
            pFrm = pFrm->GetNext();
        }

        long nTmp = 0;
        if ( nReal < nDist )
        {
            nTmp = GetUpper()->Grow( nDist - ( nReal > 0 ? nReal : 0), bTst, bInfo );

            if ( IsRestrictTableGrowth() )
            {
                nTmp = Min( nDist, nReal + nTmp );
                nDist = nTmp < 0 ? 0 : nTmp;
            }
        }

        if ( !bTst )
        {
            (Frm().*fnRect->fnAddBottom)( nDist );

            SwRootFrm *pRootFrm = FindRootFrm();
            if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
                pRootFrm->GetCurrShell() )
            {
                pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( this, aOldFrm );
            }
        }
    }

    if ( !bTst && ( nDist || IsRestrictTableGrowth() ) )
    {
        SwPageFrm *pPage = FindPageFrm();
        if ( GetNext() )
        {
            GetNext()->_InvalidatePos();
            if ( GetNext()->IsCntntFrm() )
                GetNext()->InvalidatePage( pPage );
        }
        // --> OD 2004-07-05 #i28701# - Due to the new object positioning the
        // frame on the next page/column can flow backward (e.g. it was moved
        // forward due to the positioning of its objects ). Thus, invalivate this
        // next frame, if document compatibility option 'Consider wrapping style
        // influence on object positioning' is ON.
        else if ( GetFmt()->GetDoc()->ConsiderWrapOnObjPos() )
        {
            InvalidateNextPos();
        }
        // <--
        _InvalidateAll();
        InvalidatePage( pPage );
        SetComplete();

        const SvxGraphicPosition ePos = GetFmt()->GetBackground().GetGraphicPos();
        if ( GPOS_NONE != ePos && GPOS_TILED != ePos )
            SetCompletePaint();
    }

    return nDist;
}
/*************************************************************************
|*
|*    SwTabFrm::Modify()
|*
|*    Ersterstellung    MA 14. Mar. 93
|*    Letzte Aenderung  MA 06. Dec. 96
|*
|*************************************************************************/
void SwTabFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BYTE nInvFlags = 0;
    BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();

    if( bAttrSetChg )
    {
        SfxItemIter aNIter( *((SwAttrSetChg*)pNew)->GetChgSet() );
        SfxItemIter aOIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
        SwAttrSetChg aOldSet( *(SwAttrSetChg*)pOld );
        SwAttrSetChg aNewSet( *(SwAttrSetChg*)pNew );
        while( TRUE )
        {
            _UpdateAttr( (SfxPoolItem*)aOIter.GetCurItem(),
                         (SfxPoolItem*)aNIter.GetCurItem(), nInvFlags,
                         &aOldSet, &aNewSet );
            if( aNIter.IsAtEnd() )
                break;
            aNIter.NextItem();
            aOIter.NextItem();
        }
        if ( aOldSet.Count() || aNewSet.Count() )
            SwLayoutFrm::Modify( &aOldSet, &aNewSet );
    }
    else
        _UpdateAttr( pOld, pNew, nInvFlags );

    if ( nInvFlags != 0 )
    {
        SwPageFrm *pPage = FindPageFrm();
        InvalidatePage( pPage );
//      if ( nInvFlags & 0x01 )
//          SetCompletePaint();
        if ( nInvFlags & 0x02 )
            _InvalidatePrt();
        if ( nInvFlags & 0x40 )
            _InvalidatePos();
        SwFrm *pTmp;
        if ( 0 != (pTmp = GetIndNext()) )
        {
            if ( nInvFlags & 0x04 )
            {
                pTmp->_InvalidatePrt();
                if ( pTmp->IsCntntFrm() )
                    pTmp->InvalidatePage( pPage );
            }
            if ( nInvFlags & 0x10 )
                pTmp->SetCompletePaint();
        }
        if ( nInvFlags & 0x08 && 0 != (pTmp = GetPrev()) )
        {
            pTmp->_InvalidatePrt();
            if ( pTmp->IsCntntFrm() )
                pTmp->InvalidatePage( pPage );
        }
        if ( nInvFlags & 0x20  )
        {
            if ( pPage && pPage->GetUpper() && !IsFollow() )
                ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
        }
        if ( nInvFlags & 0x80 )
            InvalidateNextPos();
    }
}

void SwTabFrm::_UpdateAttr( SfxPoolItem *pOld, SfxPoolItem *pNew,
                            BYTE &rInvFlags,
                            SwAttrSetChg *pOldSet, SwAttrSetChg *pNewSet )
{
    BOOL bClear = TRUE;
    const USHORT nWhich = pOld ? pOld->Which() : pNew ? pNew->Which() : 0;
    switch( nWhich )
    {
        case RES_TBLHEADLINECHG:
            if ( IsFollow() )
            {
                // Delete remaining headlines:
                SwRowFrm* pLower = 0;
                while ( ( pLower = (SwRowFrm*)Lower() ) && pLower->IsRepeatedHeadline() )
                {
                    pLower->Cut();
                    delete pLower;
                }

                // insert new headlines
                const USHORT nNewRepeat = GetTable()->GetRowsToRepeat();
                for ( USHORT nIdx = 0; nIdx < nNewRepeat; ++nIdx )
                {
                    bDontCreateObjects = TRUE;          //frmtool
                    SwRowFrm* pHeadline = new SwRowFrm( *GetTable()->GetTabLines()[ nIdx ] );
                    pHeadline->SetRepeatedHeadline( true );
                    bDontCreateObjects = FALSE;
                    pHeadline->Paste( this, pLower );
                }
            }
            rInvFlags |= 0x02;
            break;

        case RES_FRM_SIZE:
        case RES_HORI_ORIENT:
            rInvFlags |= 0x22;
            break;

        case RES_PAGEDESC:                      //Attributaenderung (an/aus)
            if ( IsInDocBody() )
            {
                rInvFlags |= 0x40;
                SwPageFrm *pPage = FindPageFrm();
                if ( !GetPrev() )
                    CheckPageDescs( pPage );
                if ( pPage && GetFmt()->GetPageDesc().GetNumOffset() )
                    ((SwRootFrm*)pPage->GetUpper())->SetVirtPageNum( TRUE );
                SwDocPosUpdate aMsgHnt( pPage->Frm().Top() );
                GetFmt()->GetDoc()->UpdatePageFlds( &aMsgHnt );
            }
            break;

        case RES_BREAK:
            rInvFlags |= 0xC0;
            break;

        case RES_LAYOUT_SPLIT:
            if ( !IsFollow() )
                rInvFlags |= 0x40;
            break;
        case RES_FRAMEDIR :
            SetDerivedR2L( sal_False );
            CheckDirChange();
            break;
        case RES_COLLAPSING_BORDERS :
            rInvFlags |= 0x02;
            lcl_InvalidateAllLowersPrt( this );
            break;
        case RES_UL_SPACE:
            rInvFlags |= 0x1C;
            /* kein Break hier */

        default:
            bClear = FALSE;
    }
    if ( bClear )
    {
        if ( pOldSet || pNewSet )
        {
            if ( pOldSet )
                pOldSet->ClearItem( nWhich );
            if ( pNewSet )
                pNewSet->ClearItem( nWhich );
        }
        else
            SwLayoutFrm::Modify( pOld, pNew );
    }
}

/*************************************************************************
|*
|*    SwTabFrm::GetInfo()
|*
|*    Ersterstellung    MA 06. Dec. 96
|*    Letzte Aenderung  MA 26. Jun. 98
|*
|*************************************************************************/
BOOL SwTabFrm::GetInfo( SfxPoolItem &rHnt ) const
{
    if ( RES_VIRTPAGENUM_INFO == rHnt.Which() && IsInDocBody() )
    {
        SwVirtPageNumInfo &rInfo = (SwVirtPageNumInfo&)rHnt;
        const SwPageFrm *pPage = FindPageFrm();
        if ( pPage  )
        {
            if ( pPage == rInfo.GetOrigPage() && !GetPrev() )
            {
                //Das sollte er sein (kann allenfalls temporaer anders sein,
                //                    sollte uns das beunruhigen?)
                rInfo.SetInfo( pPage, this );
                return FALSE;
            }
            if ( pPage->GetPhyPageNum() < rInfo.GetOrigPage()->GetPhyPageNum() &&
                 (!rInfo.GetPage() || pPage->GetPhyPageNum() > rInfo.GetPage()->GetPhyPageNum()))
            {
                //Das koennte er sein.
                rInfo.SetInfo( pPage, this );
            }
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|*    SwTabFrm::FindLastCntnt()
|*
|*    Ersterstellung    MA 13. Apr. 93
|*    Letzte Aenderung  MA 15. May. 98
|*
|*************************************************************************/
SwCntntFrm *SwTabFrm::FindLastCntnt()
{
    SwFrm *pRet = pLower;

    while ( pRet && !pRet->IsCntntFrm() )
    {
        SwFrm *pOld = pRet;

        SwFrm *pTmp = pRet;             // To skip empty section frames
        while ( pRet->GetNext() )
        {
            pRet = pRet->GetNext();
            if( !pRet->IsSctFrm() || ((SwSectionFrm*)pRet)->GetSection() )
                pTmp = pRet;
        }
        pRet = pTmp;

        if ( pRet->GetLower() )
            pRet = pRet->GetLower();
        if ( pRet == pOld )
        {
            // Wenn am Ende der letzten Zelle ein spaltiger Bereich steht,
            // der eine leere letzte Spalte hat, muessen wir noch die anderen
            // Spalten abklappern, dies erledigt SwSectionFrm::FindLastCntnt
            if( pRet->IsColBodyFrm() )
            {
#ifndef PRODUCT
                SwSectionFrm* pSect = pRet->FindSctFrm();
                ASSERT( pSect, "Wo kommt denn die Spalte her?")
                ASSERT( IsAnLower( pSect ), "Gespaltene Zelle?" );
#endif
                return pRet->FindSctFrm()->FindLastCntnt();
            }

            //
            // pRet may be a cell frame without a lower (cell has been split).
            // We have to find the last content the hard way:
            //
            ASSERT( pRet->IsCellFrm(), "SwTabFrm::FindLastCntnt failed" )
            const SwFrm* pRow = pRet->GetUpper();
            while ( pRow && !pRow->GetUpper()->IsTabFrm() )
                pRow = pRow->GetUpper();
            SwCntntFrm* pCntntFrm = ((SwLayoutFrm*)pRow)->ContainsCntnt();
            pRet = 0;

            while ( pCntntFrm && ((SwLayoutFrm*)pRow)->IsAnLower( pCntntFrm ) )
            {
                pRet = pCntntFrm;
                pCntntFrm = pCntntFrm->GetNextCntntFrm();
            }
        }
    }

    // #112929# There actually is a situation, which results in pRet = 0:
    // Insert frame, insert table via text <-> table. This gives you a frame
    // containing a table without any other content frames. Split the table
    // and undo the splitting. This operation gives us a table frame without
    // a lower.
    if ( pRet )
    {
        while ( pRet->GetNext() )
            pRet = pRet->GetNext();

        if( pRet->IsSctFrm() )
            pRet = ((SwSectionFrm*)pRet)->FindLastCntnt();
    }

    return (SwCntntFrm*)pRet;
}

/*************************************************************************
|*
|*  SwTabFrm::GetLeaf()
|*
|*  Ersterstellung      MA 19. Mar. 93
|*  Letzte Aenderung    MA 25. Apr. 95
|*
|*************************************************************************/
SwLayoutFrm *SwTabFrm::GetLeaf( MakePageType eMakePage, BOOL bFwd )
{
    SwLayoutFrm *pRet;
    if ( bFwd )
    {
        pRet = GetNextLeaf( eMakePage );
        while ( IsAnLower( pRet ) )
            pRet = pRet->GetNextLeaf( eMakePage );
    }
    else
        pRet = GetPrevLeaf();
    if ( pRet )
        pRet->Calc();
    return pRet;
}

/*************************************************************************
|*
|*  SwTabFrm::ShouldBwdMoved()
|*
|*  Beschreibung        Returnwert sagt ob der Frm verschoben werden sollte
|*  Ersterstellung      MA 10. Jul. 95
|*  Letzte Aenderung    MA 04. Mar. 97
|*
|*************************************************************************/
BOOL SwTabFrm::ShouldBwdMoved( SwLayoutFrm *pNewUpper, BOOL bHead, BOOL &rReformat )
{
    rReformat = FALSE;
    if ( (SwFlowFrm::IsMoveBwdJump() || !IsPrevObjMove()) )
    {
        //Das zurueckfliessen von Frm's ist leider etwas Zeitintensiv.
        //Der haufigste Fall ist der, dass dort wo der Frm hinfliessen
        //moechte die FixSize die gleiche ist, die der Frm selbst hat.
        //In diesem Fall kann einfach geprueft werden, ob der Frm genug
        //Platz fuer seine VarSize findet, ist dies nicht der Fall kann
        //gleich auf das Verschieben verzichtet werden.
        //Die Pruefung, ob der Frm genug Platz findet fuehrt er selbst
        //durch, dabei wird beruecksichtigt, dass er sich moeglicherweise
        //aufspalten kann.
        //Wenn jedoch die FixSize eine andere ist oder Flys im Spiel sind
        //(an der alten oder neuen Position) hat alle Prueferei keinen Sinn
        //der Frm muss dann halt Probehalber verschoben werden (Wenn ueberhaupt
        //etwas Platz zur Verfuegung steht).

        //Die FixSize der Umgebungen in denen Tabellen herumlungern ist immer
        //Die Breite.

        SwPageFrm *pOldPage = FindPageFrm(),
                  *pNewPage = pNewUpper->FindPageFrm();
        BOOL bMoveAnyway = FALSE;
        SwTwips nSpace = 0;

        SWRECTFN( this )
        if ( !SwFlowFrm::IsMoveBwdJump() )
        {

            long nOldWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            SWRECTFNX( pNewUpper );
            long nNewWidth = (pNewUpper->Prt().*fnRectX->fnGetWidth)();
            if( Abs( nNewWidth - nOldWidth ) < 2 )
            {
                if( FALSE ==
                    ( bMoveAnyway = BwdMoveNecessary( pOldPage, Frm() ) > 1 ) )
                {
                    SwRect aRect( pNewUpper->Prt() );
                    aRect.Pos() += pNewUpper->Frm().Pos();
                    const SwFrm *pPrevFrm = pNewUpper->Lower();
                    while ( pPrevFrm )
                    {
                        (aRect.*fnRectX->fnSetTop)( (pPrevFrm->Frm().*fnRectX->
                                                    fnGetBottom)() );
                        pPrevFrm = pPrevFrm->GetNext();
                    }
                    bMoveAnyway = BwdMoveNecessary( pNewPage, aRect) > 1;
                    nSpace = (aRect.*fnRectX->fnGetHeight)();
                    if ( GetFmt()->GetDoc()->IsBrowseMode() )
                        nSpace += pNewUpper->Grow( LONG_MAX, TRUE );
                }
            }
            else if( !bLockBackMove )
                bMoveAnyway = TRUE;
        }
        else if( !bLockBackMove )
            bMoveAnyway = TRUE;

        if ( bMoveAnyway )
            return rReformat = TRUE;
        else if ( !bLockBackMove && nSpace > 0 )
        {
            SwTwips nTmpHeight = CalcHeightOfFirstContentLine();
            return nTmpHeight < nSpace;
        }
    }
    return FALSE;
}

/*************************************************************************
|*
|*  SwTabFrm::Cut()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 09. Sep. 98
|*
|*************************************************************************/
void SwTabFrm::Cut()
{
    ASSERT( GetUpper(), "Cut ohne Upper()." );

    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );
    SwFrm *pFrm = GetNext();
    if( pFrm )
    {   //Der alte Nachfolger hat evtl. einen Abstand zum Vorgaenger
        //berechnet der ist jetzt wo er der erste wird obsolete
        pFrm->_InvalidatePrt();
        pFrm->_InvalidatePos();
        if ( pFrm->IsCntntFrm() )
            pFrm->InvalidatePage( pPage );
        if( IsInSct() && !GetPrev() )
        {
            SwSectionFrm* pSct = FindSctFrm();
            if( !pSct->IsFollow() )
            {
                pSct->_InvalidatePrt();
                pSct->InvalidatePage( pPage );
            }
        }
    }
    else
    {
        InvalidateNextPos();
        //Einer muss die Retusche uebernehmen: Vorgaenger oder Upper
        if ( 0 != (pFrm = GetPrev()) )
        {   pFrm->SetRetouche();
            pFrm->Prepare( PREP_WIDOWS_ORPHANS );
            pFrm->_InvalidatePos();
            if ( pFrm->IsCntntFrm() )
                pFrm->InvalidatePage( pPage );
        }
        //Wenn ich der einzige FlowFrm in meinem Upper bin (war), so muss
        //er die Retouche uebernehmen.
        //Ausserdem kann eine Leerseite entstanden sein.
        else
        {   SwRootFrm *pRoot = (SwRootFrm*)pPage->GetUpper();
            pRoot->SetSuperfluous();
            GetUpper()->SetCompletePaint();
            if( IsInSct() )
            {
                SwSectionFrm* pSct = FindSctFrm();
                if( !pSct->IsFollow() )
                {
                    pSct->_InvalidatePrt();
                    pSct->InvalidatePage( pPage );
                }
            }
        }
    }

    //Erst removen, dann Upper Shrinken.
    SwLayoutFrm *pUp = GetUpper();
    SWRECTFN( this )
    Remove();
    if ( pUp )
    {
        ASSERT( !pUp->IsFtnFrm(), "Tabelle in Fussnote." );
        SwSectionFrm *pSct = 0;
        if( !pUp->Lower() && pUp->IsInSct() &&
            !(pSct = pUp->FindSctFrm())->ContainsCntnt() )
        {
            if ( pUp->GetUpper() )
            {
                pSct->DelEmpty( FALSE );
                pSct->_InvalidateSize();
            }
        }
        else if( (Frm().*fnRect->fnGetHeight)() )
        {
            // OD 26.08.2003 #i18103# - *no* 'ColUnlock' of section -
            // undo changes of fix for #104992#
            pUp->Shrink( Frm().Height() PHEIGHT );
        }
    }

    if ( pPage && !IsFollow() && pPage->GetUpper() )
        ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();
}

/*************************************************************************
|*
|*  SwTabFrm::Paste()
|*
|*  Ersterstellung      MA 23. Feb. 94
|*  Letzte Aenderung    MA 09. Sep. 98
|*
|*************************************************************************/
void SwTabFrm::Paste( SwFrm* pParent, SwFrm* pSibling )
{
    ASSERT( pParent, "Kein Parent fuer Paste." );
    ASSERT( pParent->IsLayoutFrm(), "Parent ist CntntFrm." );
    ASSERT( pParent != this, "Bin selbst der Parent." );
    ASSERT( pSibling != this, "Bin mein eigener Nachbar." );
    ASSERT( !GetPrev() && !GetNext() && !GetUpper(),
            "Bin noch irgendwo angemeldet." );

    //In den Baum einhaengen.
    InsertBefore( (SwLayoutFrm*)pParent, pSibling );

    _InvalidateAll();
    SwPageFrm *pPage = FindPageFrm();
    InvalidatePage( pPage );

    if ( GetNext() )
    {
        GetNext()->_InvalidatePos();
        GetNext()->_InvalidatePrt();
        if ( GetNext()->IsCntntFrm() )
            GetNext()->InvalidatePage( pPage );
    }

    SWRECTFN( this )
    if( (Frm().*fnRect->fnGetHeight)() )
        pParent->Grow( (Frm().*fnRect->fnGetHeight)() );

    if( (Frm().*fnRect->fnGetWidth)() != (pParent->Prt().*fnRect->fnGetWidth)() )
        Prepare( PREP_FIXSIZE_CHG );
    if ( GetPrev() )
    {
        if ( !IsFollow() )
        {
            GetPrev()->InvalidateSize();
            if ( GetPrev()->IsCntntFrm() )
                GetPrev()->InvalidatePage( pPage );
        }
    }
    else if ( GetNext() )
        //Bei CntntFrm's gilt es den Abstand zum Vorgaenger/Nachfolger
        //zu beachten. Faelle (beide treten immer gleichzeitig auf):
        //a) Der Cntnt wird der erste einer Kette
        //b) Der neue Nachfolger war vorher der erste einer Kette
        GetNext()->_InvalidatePrt();

    if ( pPage && !IsFollow() )
    {
        if ( pPage->GetUpper() )
            ((SwRootFrm*)pPage->GetUpper())->InvalidateBrowseWidth();

        if ( !GetPrev() )//Mindestens fuer HTML mit Tabelle am Anfang notwendig.
        {
            const SwPageDesc *pDesc = GetFmt()->GetPageDesc().GetPageDesc();
            if ( (pDesc && pDesc != pPage->GetPageDesc()) ||
                 (!pDesc && pPage->GetPageDesc() !=
                  &(const_cast<const SwDoc *>(GetFmt()->GetDoc())
                    ->GetPageDesc(0))) )
                CheckPageDescs( pPage, TRUE );
        }
    }
}

/*************************************************************************
|*
|*  SwTabFrm::Prepare()
|*
|*  Created        AMA 01/10/02
|*  Last Change    AMA 01/10/02
|*
|*************************************************************************/
void SwTabFrm::Prepare( const PrepareHint eHint, const void *, BOOL )
{
    if( PREP_BOSS_CHGD == eHint )
        CheckDirChange();
}

/*************************************************************************
|*
|*  SwRowFrm::SwRowFrm(), ~SwRowFrm()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
SwRowFrm::SwRowFrm( const SwTableLine &rLine, bool bInsertContent ):
    SwLayoutFrm( rLine.GetFrmFmt() ),
    pTabLine( &rLine ),
    pFollowRow( 0 ),
    // --> collapsing borders FME 2005-05-27 #i29550#
    mnTopMarginForLowers( 0 ),
    mnBottomMarginForLowers( 0 ),
    mnBottomLineSize( 0 ),
    // <-- collapsing
    // --> split table rows
    bIsFollowFlowRow( false ),
    // <-- split table rows
    bIsRepeatedHeadline( false )
{
    nType = FRMC_ROW;

    //Gleich die Boxen erzeugen und einfuegen.
    const SwTableBoxes &rBoxes = rLine.GetTabBoxes();
    SwFrm *pTmpPrev = 0;
    for ( USHORT i = 0; i < rBoxes.Count(); ++i )
    {
        SwCellFrm *pNew = new SwCellFrm( *rBoxes[i], bInsertContent );
        pNew->InsertBehind( this, pTmpPrev );
        pTmpPrev = pNew;
    }
}

SwRowFrm::~SwRowFrm()
{
    SwModify* pMod = GetFmt();
    if( pMod )
    {
        pMod->Remove( this );           // austragen,
        if( !pMod->GetDepends() )
            delete pMod;                // und loeschen
    }
}

/*************************************************************************
|*
|*  SwRowFrm::RegistFlys()
|*
|*  Ersterstellung      MA 08. Jul. 93
|*  Letzte Aenderung    MA 08. Jul. 93
|*
|*************************************************************************/
void SwRowFrm::RegistFlys( SwPageFrm *pPage )
{
    ::RegistFlys( pPage ? pPage : FindPageFrm(), this );
}

/*************************************************************************
|*
|*    SwRowFrm::Modify()
|*
|*    Ersterstellung    MA 12. Nov. 97
|*    Letzte Aenderung  MA 12. Nov. 97
|*
|*************************************************************************/
void SwRowFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
    {
        const SwAttrSet* pChgSet = ((SwAttrSetChg*)pNew)->GetChgSet();
        pChgSet->GetItemState( RES_FRM_SIZE, FALSE, &pItem);
        if ( !pItem )
            pChgSet->GetItemState( RES_ROW_SPLIT, FALSE, &pItem);
    }
    else if ( RES_FRM_SIZE == pNew->Which() || RES_ROW_SPLIT == pNew->Which() )
        pItem = pNew;

    if ( pItem )
    {
        SwTabFrm *pTab = FindTabFrm();
        const USHORT nRepeat = pTab->GetTable()->GetRowsToRepeat();
        if ( pTab && pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow() )
        {
            pTab->FindMaster()->InvalidatePos();
        }
    }

    SwLayoutFrm::Modify( pOld, pNew );
}



/*************************************************************************
|*
|*  SwRowFrm::MakeAll()
|*
|*  Ersterstellung      MA 01. Mar. 94
|*  Letzte Aenderung    MA 01. Mar. 94
|*
|*************************************************************************/
void SwRowFrm::MakeAll()
{
    if ( !GetNext() )
        bValidSize = FALSE;
    SwLayoutFrm::MakeAll();
}

/*************************************************************************
|*
|*  SwRowFrm::Format()
|*
|*  Ersterstellung      MA 13. Mar. 93
|*  Letzte Aenderung    MA 20. Jun. 96
|*
|*************************************************************************/
long MA_FASTCALL CalcHeightWidthFlys( const SwFrm *pFrm )
{
    SWRECTFN( pFrm )
    long nHeight = 0;
    const SwFrm* pTmp = pFrm->IsSctFrm() ?
            ((SwSectionFrm*)pFrm)->ContainsCntnt() : pFrm;
    while( pTmp )
    {
        if ( pTmp->GetDrawObjs() )
        {
            for ( USHORT i = 0; i < pTmp->GetDrawObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pTmp->GetDrawObjs())[i];
                if ( pAnchoredObj->ISA(SwFlyFrm) )
                {
                    const SwFlyFrm *pFly = static_cast<const SwFlyFrm*>(pAnchoredObj);
                    // OD 30.09.2003 #i18732# - only objects, which follow
                    // the text flow have to be considered.
                    const SwFrmFmt& rFrmFmt = pAnchoredObj->GetFrmFmt();
                    const bool bConsiderFly =
                            !pFly->IsFlyInCntFrm() &&
                            pFly->Frm().Top() != WEIT_WECH &&
                            rFrmFmt.GetFollowTextFlow().GetValue();
                    if ( bConsiderFly )
                    {
                        const SwFmtFrmSize &rSz = pFly->GetFmt()->GetFrmSize();
                        if( !rSz.GetHeightPercent() )
                        {
                            const SwTwips nFlyWidth =
                                (pFly->Frm().*fnRect->fnGetHeight)() +
                                    ( bVert ?
                                      pFly->GetCurrRelPos().X() :
                                      pFly->GetCurrRelPos().Y() );

                            const SwTwips nFrmDiff =
                                (*fnRect->fnYDiff)(
                                    (pTmp->Frm().*fnRect->fnGetTop)(),
                                    (pFrm->Frm().*fnRect->fnGetTop)() );

                            nHeight = Max( nHeight, nFlyWidth + nFrmDiff -
                                            (pFrm->Frm().*fnRect->fnGetHeight)() );
                        }
                    }
                }
            }
        }
        if( !pFrm->IsSctFrm() )
            break;
        pTmp = pTmp->FindNextCnt();
        if( !((SwSectionFrm*)pFrm)->IsAnLower( pTmp ) )
            break;
    }
    return nHeight;
}

SwTwips lcl_CalcTopAndBottomMargin( const SwLayoutFrm& rCell, const SwBorderAttrs& rAttrs )
{
    const SwTabFrm* pTab = rCell.FindTabFrm();
    SwTwips nTopSpace = 0;
    SwTwips nBottomSpace = 0;

    // --> collapsing borders FME 2005-05-27 #i29550#
    if ( pTab->IsCollapsingBorders() && rCell.Lower() && !rCell.Lower()->IsRowFrm() )
    {
        nTopSpace    = ((SwRowFrm*)rCell.GetUpper())->GetTopMarginForLowers();
        nBottomSpace = ((SwRowFrm*)rCell.GetUpper())->GetBottomMarginForLowers();
    }
    // <-- collapsing
    else
    {
        if ( pTab->IsVertical() != rCell.IsVertical() )
        {
            nTopSpace    = rAttrs.CalcLeft( &rCell );
            nBottomSpace = rAttrs.CalcRight( &rCell );
        }
        else
        {
            nTopSpace    = rAttrs.CalcTop();
            nBottomSpace = rAttrs.CalcBottom();
        }
    }

    return nTopSpace + nBottomSpace;
}

SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwLayoutFrm *pRow );

SwTwips MA_FASTCALL lcl_CalcMinCellHeight( SwLayoutFrm *_pCell,
                                  const SwBorderAttrs *pAttrs = 0 )
{
    SWRECTFN( _pCell )
    SwTwips nHeight = 0;
    SwFrm* pLow = _pCell->Lower();
    if ( pLow )
    {
        long nFlyAdd = 0;
        while ( pLow )
        {
            // OD 2004-02-18 #106629# - change condition and switch then-body
            // and else-body
            if ( pLow->IsRowFrm() )
            {
                nHeight += ::lcl_CalcMinRowHeight( static_cast<SwRowFrm*>(pLow) );
            }
            else
            {
                long nLowHeight = (pLow->Frm().*fnRect->fnGetHeight)();
                nHeight += nLowHeight;
                nFlyAdd = Max( 0L, nFlyAdd - nLowHeight );
                nFlyAdd = Max( nFlyAdd, ::CalcHeightWidthFlys( pLow ) );
            }

            pLow = pLow->GetNext();
        }
        if ( nFlyAdd )
            nHeight += nFlyAdd;
    }
    //Der Border will natuerlich auch mitspielen, er kann leider nicht
    //aus PrtArea und Frm errechnet werden, da diese in beliebiger
    //Kombination ungueltig sein koennen.
    if ( _pCell->Lower() )
    {
        if ( pAttrs )
            nHeight += lcl_CalcTopAndBottomMargin( *_pCell, *pAttrs );
        else
        {
            SwBorderAttrAccess aAccess( SwFrm::GetCache(), _pCell );
            const SwBorderAttrs &rAttrs = *aAccess.Get();
            nHeight += lcl_CalcTopAndBottomMargin( *_pCell, rAttrs );
        }
    }
    return nHeight;
}

// OD 2004-02-18 #106629# - correct type of 1st parameter
SwTwips MA_FASTCALL lcl_CalcMinRowHeight( SwRowFrm* _pRow )
{
    SWRECTFN( _pRow )
    const SwFmtFrmSize &rSz = _pRow->GetFmt()->GetFrmSize();

    if ( _pRow->HasFixSize() )
    {
        ASSERT( ATT_FIX_SIZE == rSz.GetHeightSizeType(), "pRow claims to have fixed size" )
        return rSz.GetHeight();
    }

    SwTwips nHeight = 0;
    SwCellFrm* pLow = static_cast<SwCellFrm*>(_pRow->Lower());
    while ( pLow )
    {
        SwTwips nTmp = ::lcl_CalcMinCellHeight( pLow );
        if ( ( 0 != pLow->IsVertical() ) == ( 0 != bVert ) && nTmp > nHeight )
            nHeight = nTmp;
        pLow = static_cast<SwCellFrm*>(pLow->GetNext());
    }
    if ( rSz.GetHeightSizeType() == ATT_MIN_SIZE )
        nHeight = Max( nHeight, rSz.GetHeight() );
    return nHeight;
}

// --> collapsing borders FME 2005-05-27 #i29550#

// Calculate the maximum of (TopLineSize + TopLineDist) over all lowers:
USHORT lcl_GetTopSpace( const SwRowFrm& rRow )
{
    USHORT nTopSpace = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        USHORT nTmpTopSpace = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
            nTmpTopSpace = lcl_GetTopSpace( *(SwRowFrm*)pCurrLower->Lower() );
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopSpace = rBoxItem.CalcLineSpace( BOX_LINE_TOP, sal_True );
        }
        nTopSpace  = Max( nTopSpace, nTmpTopSpace );
    }
    return nTopSpace;
}

// Calculate the maximum of TopLineDist over all lowers:
USHORT lcl_GetTopLineDist( const SwRowFrm& rRow )
{
    USHORT nTopLineDist = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        USHORT nTmpTopLineDist = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
            nTmpTopLineDist = lcl_GetTopLineDist( *(SwRowFrm*)pCurrLower->Lower() );
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpTopLineDist = rBoxItem.GetDistance( BOX_LINE_TOP );
        }
        nTopLineDist = Max( nTopLineDist, nTmpTopLineDist );
    }
    return nTopLineDist;
}

// Calculate the maximum of BottomLineSize over all lowers:
USHORT lcl_GetBottomLineSize( const SwRowFrm& rRow )
{
    USHORT nBottomLineSize = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        USHORT nTmpBottomLineSize = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
        {
            SwFrm* pRow = pCurrLower->Lower();
            while ( pRow->GetNext() )
                pRow = pRow->GetNext();
            nTmpBottomLineSize = lcl_GetBottomLineSize( *(SwRowFrm*)pRow );
        }
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineSize = rBoxItem.CalcLineSpace( BOX_LINE_BOTTOM, sal_True ) -
                                 rBoxItem.GetDistance( BOX_LINE_BOTTOM );
        }
        nBottomLineSize = Max( nBottomLineSize, nTmpBottomLineSize );
    }
    return nBottomLineSize;
}

// Calculate the maximum of BottomLineDist over all lowers:
USHORT lcl_GetBottomLineDist( const SwRowFrm& rRow )
{
    USHORT nBottomLineDist = 0;
    for ( SwCellFrm* pCurrLower = (SwCellFrm*)rRow.Lower(); pCurrLower;
          pCurrLower = (SwCellFrm*)pCurrLower->GetNext() )
    {
        USHORT nTmpBottomLineDist = 0;
        if ( pCurrLower->Lower() && pCurrLower->Lower()->IsRowFrm() )
        {
            SwFrm* pRow = pCurrLower->Lower();
            while ( pRow->GetNext() )
                pRow = pRow->GetNext();
            nTmpBottomLineDist = lcl_GetBottomLineDist( *(SwRowFrm*)pRow );
        }
        else
        {
            const SwAttrSet& rSet = ((SwCellFrm*)pCurrLower)->GetFmt()->GetAttrSet();
            const SvxBoxItem& rBoxItem = rSet.GetBox();
            nTmpBottomLineDist = rBoxItem.GetDistance( BOX_LINE_BOTTOM );
        }
        nBottomLineDist = Max( nBottomLineDist, nTmpBottomLineDist );
    }
    return nBottomLineDist;
}

// <-- collapsing

void SwRowFrm::Format( const SwBorderAttrs *pAttrs )
{
    SWRECTFN( this )
    ASSERT( pAttrs, "SwRowFrm::Format ohne Attrs." );

    const BOOL bFix = BFIXHEIGHT;

    if ( !bValidPrtArea )
    {
        //RowFrms haben keine Umrandung usw. also entspricht die PrtArea immer
        //dem Frm.
        bValidPrtArea = TRUE;
        aPrt.Left( 0 );
        aPrt.Top( 0 );
        aPrt.Width ( aFrm.Width() );
        aPrt.Height( aFrm.Height() );

        // --> collapsing borders FME 2005-05-27 #i29550#
        // Here we calculate the top-printing area for the lower cell frames
        SwTabFrm* pTabFrm = FindTabFrm();
        if ( pTabFrm->IsCollapsingBorders() )
        {
            const USHORT nTopSpace        = lcl_GetTopSpace(       *this );
            const USHORT nTopLineDist     = lcl_GetTopLineDist(    *this );
            const USHORT nBottomLineSize  = lcl_GetBottomLineSize( *this );
            const USHORT nBottomLineDist  = lcl_GetBottomLineDist( *this );

            // Get the previous row, calculate prt top for lowers:
            const SwRowFrm* pPreviousRow = (SwRowFrm*)GetPrev();
            if ( pTabFrm->IsFollow() &&
                 ( !pPreviousRow || this == pTabFrm->GetFirstNonHeadlineRow() ) )
            {
                const SwTable* pTable = pTabFrm->GetTable();
                SwTableLine* pPrevTabLine = 0;

                USHORT nIdx = 0;
                const SwTableLines& rLines = GetTabLine()->GetUpper() ?
                                             GetTabLine()->GetUpper()->GetTabLines() :
                                             pTable->GetTabLines();

                while ( rLines[ nIdx ] != GetTabLine() )
                    ++nIdx;

                if ( nIdx > 0 )
                {
                    --nIdx;
                    pPrevTabLine = pTable->GetTabLines()[ nIdx ];

                    SwClientIter aIter( *pPrevTabLine->GetFrmFmt() );
                    SwClient* pLast;
                    for ( pLast = aIter.First( TYPE( SwFrm ) ); pLast; pLast = aIter.Next() )
                    {
                       ASSERT( ((SwFrm*)pLast)->IsRowFrm(),
                                    "Non-row frame registered in table line" )
                        SwRowFrm* pRow = (SwRowFrm*)pLast;
                        if ( pRow->GetTabLine() == pPrevTabLine )
                        {
                            pPreviousRow = pRow;
                            break;
                        }
                    }
                }
            }

            USHORT nTopPrtMargin = nTopSpace;
            if ( pPreviousRow )
            {
                const USHORT nTmpPrtMargin = pPreviousRow->GetBottomLineSize() + nTopLineDist;
                if ( nTmpPrtMargin > nTopPrtMargin )
                    nTopPrtMargin = nTmpPrtMargin;
            }

            // table has to be notified if it has to change its lower
            // margin due to changes of nBottomLineSize:
            if ( !GetNext() && nBottomLineSize != GetBottomLineSize() )
                 pTabFrm->_InvalidatePrt();

            // If there are rows nested inside this row, the nested rows
            // may not have been calculated yet. Therefore the
            // ::lcl_CalcMinRowHeight( this ) operation later in this
            // function cannot consider the correct border values. We
            // have to trigger the invalidation of the outer row frame
            // manually:
            // Note: If any further invalidations should be necessary, we
            // should consider moving the invalidation stuff to the
            // appropriate SwNotify object.
            if ( GetUpper()->GetUpper()->IsRowFrm() &&
                 ( nBottomLineDist != GetBottomMarginForLowers() ||
                   nTopPrtMargin   != GetTopMarginForLowers() ) )
                GetUpper()->GetUpper()->_InvalidateSize();

            SetBottomMarginForLowers( nBottomLineDist );    //  3.
            SetBottomLineSize( nBottomLineSize );           //  4.
            SetTopMarginForLowers( nTopPrtMargin );         //  5.

        }
// <-- collapsing
    }

    while ( !bValidSize )
    {
        bValidSize = TRUE;

#ifndef PRODUCT
        if ( HasFixSize() )
        {
            const SwFmtFrmSize &rFrmSize = GetFmt()->GetFrmSize();
            ASSERT( rFrmSize.GetSize().Height() > 0, "Hat ihn" );
        }
#endif
        const SwTwips nDiff = (Frm().*fnRect->fnGetHeight)() - (HasFixSize() ?
                                                pAttrs->GetSize().Height() :
                                                ::lcl_CalcMinRowHeight( this ));
        if ( nDiff )
        {
            BFIXHEIGHT = FALSE;
            if ( nDiff > 0 )
                Shrink( nDiff PHEIGHT, FALSE, TRUE );
            else if ( nDiff < 0 )
                Grow( -nDiff PHEIGHT );
            BFIXHEIGHT = bFix;
        }
    }

    // last row will fill the space in its upper.
    if ( !GetNext() )
    {
        //Der letzte fuellt den verbleibenden Raum im Upper aus.
        SwTwips nDiff = (GetUpper()->Prt().*fnRect->fnGetHeight)();
        SwFrm *pSibling = GetUpper()->Lower();
        do
        {   nDiff -= (pSibling->Frm().*fnRect->fnGetHeight)();
            pSibling = pSibling->GetNext();
        } while ( pSibling );
        if ( nDiff > 0 )
        {
            BFIXHEIGHT = FALSE;
            Grow( nDiff PHEIGHT );
            BFIXHEIGHT = bFix;
            bValidSize = TRUE;
        }
    }
}

/*************************************************************************
|*
|*  SwRowFrm::AdjustCells()
|*
|*  Ersterstellung      MA 10. Aug. 93
|*  Letzte Aenderung    MA 16. Dec. 96
|*
|*************************************************************************/
void SwRowFrm::AdjustCells( const SwTwips nHeight, const BOOL bHeight )
{
    SwFrm *pFrm = Lower();
    if ( bHeight )
    {
        SwRootFrm *pRootFrm = 0;
        SWRECTFN( this )
        while ( pFrm )
        {
            long nDiff = nHeight - (pFrm->Frm().*fnRect->fnGetHeight)();
            if( nDiff )
            {
                SwRect aOldFrm( pFrm->Frm() );
                (pFrm->Frm().*fnRect->fnAddBottom)( nDiff );

                if( !pRootFrm )
                    pRootFrm = FindRootFrm();
                if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
                    pRootFrm->GetCurrShell() )
                {
                    pRootFrm->GetCurrShell()->Imp()->MoveAccessibleFrm( pFrm, aOldFrm );
                }
                pFrm->_InvalidatePrt();
            }
            pFrm = pFrm->GetNext();
        }
    }
    else
    {   while ( pFrm )
        {
            pFrm->_InvalidateAll();
            pFrm = pFrm->GetNext();
        }
    }
    InvalidatePage();
}

/*************************************************************************
|*
|*  SwRowFrm::Cut()
|*
|*  Ersterstellung      MA 12. Nov. 97
|*  Letzte Aenderung    MA 12. Nov. 97
|*
|*************************************************************************/
void SwRowFrm::Cut()
{
    SwTabFrm *pTab = FindTabFrm();
    const USHORT nRepeat = pTab->GetTable()->GetRowsToRepeat();
    if ( pTab && pTab->IsFollow() && this == pTab->GetFirstNonHeadlineRow() )
    {
        pTab->FindMaster()->InvalidatePos();
    }
    SwLayoutFrm::Cut();
}

/*************************************************************************
|*
|*  SwRowFrm::GrowFrm()
|*
|*  Ersterstellung      MA 15. Mar. 93
|*  Letzte Aenderung    MA 05. May. 94
|*
|*************************************************************************/


SwTwips SwRowFrm::GrowFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    SwTwips nReal = 0;

    SwTabFrm* pTab = FindTabFrm();
    SWRECTFN( pTab )

    bool bRestrictTableGrowth;
    bool bHasFollowFlowLine = pTab->HasFollowFlowLine();

    if ( GetUpper()->IsTabFrm() )
        bRestrictTableGrowth = NULL != IsInSplitTableRow();
    else
    {
        ASSERT( GetUpper()->IsCellFrm(), "RowFrm->GetUpper neither table nor cell" )
        bRestrictTableGrowth = GetFollowRow() && bHasFollowFlowLine;
        ASSERT( !bRestrictTableGrowth || !GetNext(),
                "GetFollowRow for row frame that has a Next" )

        //
        // There may still be some space left in my direct upper:
        //
        const SwTwips nAdditionalSpace =
                (Frm().*fnRect->fnBottomDist)( (GetUpper()->GetUpper()->*fnRect->fnGetPrtBottom)() );
        if ( bRestrictTableGrowth && nAdditionalSpace > 0 )
        {
            nReal = Min( nAdditionalSpace, nDist );
            nDist -= nReal;
            if ( !bTst )
                (Frm().*fnRect->fnAddBottom)( nReal );
        }
    }

    if ( bRestrictTableGrowth )
        pTab->SetRestrictTableGrowth( TRUE );
    else
    {
        // Ok, this looks like a hack, indeed, it is a hack.
        // If the current row frame is inside another cell frame,
        // and the current row frame has no follow, it should not
        // be allowed to grow. In fact, setting bRestrictTableGrowth
        // to 'false' does not work, because the surrounding RowFrm
        // would set this to 'true'.
        pTab->SetFollowFlowLine( FALSE );
    }

    nReal += SwLayoutFrm::GrowFrm( nDist, bTst, bInfo);

    pTab->SetRestrictTableGrowth( FALSE );
    pTab->SetFollowFlowLine( bHasFollowFlowLine );

    //Hoehe der Zellen auf den neuesten Stand bringen.
    if ( !bTst )
    {
        SWRECTFN( this )
        AdjustCells( (Prt().*fnRect->fnGetHeight)() + nReal, TRUE );
        if ( nReal )
            SetCompletePaint();
    }

    return nReal;
}

/*************************************************************************
|*
|*  SwRowFrm::ShrinkFrm()
|*
|*  Ersterstellung      MA 15. Mar. 93
|*  Letzte Aenderung    MA 20. Jun. 96
|*
|*************************************************************************/
SwTwips SwRowFrm::ShrinkFrm( SwTwips nDist, BOOL bTst, BOOL bInfo )
{
    SWRECTFN( this )
    if( HasFixSize() )
    {
        AdjustCells( (Prt().*fnRect->fnGetHeight)(), TRUE );
        return 0L;
    }

    //bInfo wird ggf. vom SwRowFrm::Format auf TRUE gesetzt, hier muss dann
    //entsprechend reagiert werden
    const BOOL bShrinkAnyway = bInfo;

    //Nur soweit Shrinken, wie es der Inhalt der groessten Zelle zulaesst.
    SwTwips nRealDist = nDist;
    {
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        SwTwips nMinHeight = rSz.GetHeightSizeType() == ATT_MIN_SIZE ?
                             rSz.GetHeight() : 0;
        if( nMinHeight < (Frm().*fnRect->fnGetHeight)() )
        {
            SwCellFrm* pCell = static_cast<SwCellFrm*>(Lower());
            while ( pCell )
            {
                SwTwips nAct = ::lcl_CalcMinCellHeight( pCell );
                if ( nAct > nMinHeight )
                    nMinHeight = nAct;
                if ( nMinHeight >= (Frm().*fnRect->fnGetHeight)() )
                    break;
                pCell = static_cast<SwCellFrm*>(pCell->GetNext());
            }
        }
        if ( ((Frm().*fnRect->fnGetHeight)() - nRealDist) < nMinHeight )
            nRealDist = (Frm().*fnRect->fnGetHeight)() - nMinHeight;
    }
    if ( nRealDist < 0 )
        nRealDist = 0;

    SwTwips nReal = nRealDist;
    if ( nReal )
    {
        if ( !bTst )
        {
            SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
            (Frm().*fnRect->fnSetHeight)( nHeight - nReal );
            if( IsVertical() && !bRev )
                Frm().Pos().X() += nReal;
        }

        SwTwips nTmp = GetUpper()->Shrink( nReal, bTst );
        if ( !bShrinkAnyway && !GetNext() && nTmp != nReal )
        {
            //Der letzte bekommt den Rest im Upper und nimmt deshalb
            //ggf. Ruecksichten (sonst: Endlosschleife)
            if ( !bTst )
            {
                nReal -= nTmp;
                SwTwips nHeight = (Frm().*fnRect->fnGetHeight)();
                (Frm().*fnRect->fnSetHeight)( nHeight + nReal );
                if( IsVertical() && !bRev )
                    Frm().Pos().X() -= nReal;
            }
            nReal = nTmp;
        }
    }

    //Geeignet invalidieren und die Hoehe der Zellen auf den neuesten
    //Stand bringen.
    if ( !bTst )
    {
        if ( nReal )
        {
            if ( GetNext() )
                GetNext()->_InvalidatePos();
            _InvalidateAll();
            SetCompletePaint();

            SwTabFrm *pTab = FindTabFrm();
            const USHORT nRepeat = pTab->GetTable()->GetRowsToRepeat();
            if ( !pTab->IsRebuildLastLine() && pTab->IsFollow() &&
                 this == pTab->GetFirstNonHeadlineRow() )
            {
                SwTabFrm* pMasterTab = const_cast< SwTabFrm* >( pTab->FindMaster() );
                pMasterTab->InvalidatePos();
            }
        }
        AdjustCells( (Prt().*fnRect->fnGetHeight)() - nReal, TRUE );
    }
    return nReal;
}

/*************************************************************************
|*
|*  SwRowFrm::IsRowSplitAllowed()
|*
|*************************************************************************/
bool SwRowFrm::IsRowSplitAllowed() const
{
    // Fixed size rows are never allowed to split:
    if ( HasFixSize() )
    {
        const SwFmtFrmSize &rSz = GetFmt()->GetFrmSize();
        ASSERT( ATT_FIX_SIZE == rSz.GetHeightSizeType(), "pRow claims to have fixed size" )
        return false;
    }

    // Repeated headlines are never allowed to split:
    const SwTabFrm* pTabFrm = FindTabFrm();
    if ( pTabFrm->GetTable()->GetRowsToRepeat() > 0 &&
         pTabFrm->IsInHeadline( *this ) )
        return false;

    const SwTableLine* pTabLine = GetTabLine();
    const SwTableLineFmt* pFrmFmt = (SwTableLineFmt*)pTabLine->GetFrmFmt();
    const SwFmtRowSplit& rLP = pFrmFmt->GetRowSplit();
    return 0 != rLP.GetValue();
}

/*************************************************************************
|*
|*  SwCellFrm::SwCellFrm(), ~SwCellFrm()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 30. May. 96
|*
|*************************************************************************/
SwCellFrm::SwCellFrm( const SwTableBox &rBox, bool bInsertContent ) :
    SwLayoutFrm( rBox.GetFrmFmt() ),
    pTabBox( &rBox )
{
    nType = FRMC_CELL;

    if ( !bInsertContent )
        return;

    //Wenn ein StartIdx vorhanden ist, so werden CntntFrms in der Zelle
    //angelegt, andernfalls muessen Rows vorhanden sein und diese werden
    //angelegt.
    if ( rBox.GetSttIdx() )
    {
        ULONG nIndex = rBox.GetSttIdx();
        ::_InsertCnt( this, rBox.GetFrmFmt()->GetDoc(), ++nIndex );
    }
    else
    {
    const SwTableLines &rLines = rBox.GetTabLines();
        SwFrm *pTmpPrev = 0;
        for ( USHORT i = 0; i < rLines.Count(); ++i )
        {
            SwRowFrm *pNew = new SwRowFrm( *rLines[i], bInsertContent );
            pNew->InsertBehind( this, pTmpPrev );
            pTmpPrev = pNew;
        }
    }
}

SwCellFrm::~SwCellFrm()
{
    SwModify* pMod = GetFmt();
    if( pMod )
    {
        // At this stage the lower frames aren't destroyed already,
        // therfor we have to do a recursive dispose.
        SwRootFrm *pRootFrm = FindRootFrm();
        if( pRootFrm && pRootFrm->IsAnyShellAccessible() &&
            pRootFrm->GetCurrShell() )
        {
            pRootFrm->GetCurrShell()->Imp()->DisposeAccessibleFrm( this, sal_True );
        }

        pMod->Remove( this );           // austragen,
        if( !pMod->GetDepends() )
            delete pMod;                // und loeschen
    }
}

/*************************************************************************
|*
|*  SwCellFrm::Format()
|*
|*  Ersterstellung      MA 09. Mar. 93
|*  Letzte Aenderung    MA 29. Jan. 98
|*
|*************************************************************************/
BOOL lcl_ArrangeLowers( SwLayoutFrm *pLay, long lYStart, BOOL bInva )
{
    BOOL bRet = FALSE;
    SwFrm *pFrm = pLay->Lower();
    SwPageFrm* pPg = NULL;
    SWRECTFN( pLay )
    while ( pFrm )
    {
        long nFrmTop = (pFrm->Frm().*fnRect->fnGetTop)();
        if( nFrmTop != lYStart )
        {
            bRet = TRUE;
            const long lDiff = (*fnRect->fnYDiff)( lYStart, nFrmTop );
            const long lDiffX = lYStart - nFrmTop;
            (pFrm->Frm().*fnRect->fnSubTop)( -lDiff );
            (pFrm->Frm().*fnRect->fnAddBottom)( lDiff );
            pFrm->SetCompletePaint();
            if ( !pFrm->GetNext() )
                pFrm->SetRetouche();
            if( bInva )
                pFrm->Prepare( PREP_POS_CHGD );
            if ( pFrm->IsLayoutFrm() && ((SwLayoutFrm*)pFrm)->Lower() )
                lcl_ArrangeLowers( (SwLayoutFrm*)pFrm,
                    (((SwLayoutFrm*)pFrm)->Lower()->Frm().*fnRect->fnGetTop)()
                    + lDiffX, bInva );
            if ( pFrm->GetDrawObjs() )
            {
                for ( USHORT i = 0; i < pFrm->GetDrawObjs()->Count(); ++i )
                {
                    SwAnchoredObject* pAnchoredObj = (*pFrm->GetDrawObjs())[i];
                    // --> OD 2004-05-18 #i28701# - unlock position and set
                    // <mbConsiderForTextWrap> to <false> of object
                    pAnchoredObj->SetConsiderForTextWrap( false );
                    pAnchoredObj->UnlockPosition();
                    // <--
                    if ( pAnchoredObj->ISA(SwFlyFrm) )
                    {
                        SwFlyFrm *pFly = static_cast<SwFlyFrm*>(pAnchoredObj);

                        // OD 2004-05-18 #i28701# - no direct move of objects,
                        // which are anchored to-paragraph/to-character, if
                        // the wrapping style influence has to be considered
                        // on the object positioning.
                        const bool bNoDirectMove =
                                WEIT_WECH == pFly->Frm().Top() ||
                                pFly->ConsiderObjWrapInfluenceOnObjPos();
                        if ( !bNoDirectMove )
                        {
                            (pFly->Frm().*fnRect->fnSubTop)( -lDiff );
                            (pFly->Frm().*fnRect->fnAddBottom)( lDiff );
                        }
                        pFly->GetVirtDrawObj()->_SetRectsDirty();

                        if ( pFly->IsFlyInCntFrm() )
                            ((SwFlyInCntFrm*)pFly)->AddRefOfst( lDiff );
                        else
                        {
                            if( !pPg )
                                pPg = pLay->FindPageFrm();
                            SwPageFrm* pOld = pFly->FindPageFrm();
                            if( pPg != pOld )
                                pOld->MoveFly( pFly, pPg );
                            if( pFly->IsAutoPos() )
                            {
                                pFly->AddLastCharY( lDiff );
                                // OD 2004-05-18 #i28701# - follow-up of #i22341#
                                // <mnLastTopOfLine> has also been adjusted.
                                pFly->AddLastTopOfLineY( lDiff );
                            }
                        }
                        // OD 2004-05-11 #i28701# - Because of the introduction
                        // of new positionings and alignments (e.g. aligned at
                        // page area, but anchored at-character), the position
                        // of the Writer fly frame has to be invalidated.
                        pFly->InvalidatePos();

                        if ( ::lcl_ArrangeLowers( pFly,
                                                  (pFly->*fnRect->fnGetPrtTop)(),
                                                  bInva ) )
                        {
                            pFly->SetCompletePaint();
                        }
                    }
                    else
                    {
                        // --> OD 2004-07-01 #i28701# - adjust last character
                        // rectangle and last top of line.
                        pAnchoredObj->AddLastCharY( lDiff );
                        pAnchoredObj->AddLastTopOfLineY( lDiff );
                        // OD 2004-04-06 #i26791#, #i28701# - Direct object
                        // positioning no longer needed. Instead invalidate position
                        pAnchoredObj->InvalidateObjPos();
                    }
                }
            }
        }
        // Columns and cells are ordered horizontal, not vertical
        if( !pFrm->IsColumnFrm() && !pFrm->IsCellFrm() )
            lYStart = (*fnRect->fnYInc)( lYStart,
                                        (pFrm->Frm().*fnRect->fnGetHeight)() );

        // Nowadays, the content inside a cell can flow into the follow table.
        // Thus, the cell may only grow up to the end of the environment.
        // So the content may have grown, but the cell could not grow.
        // Therefore we have to trigger a formatting for the frames, which do
        // not fit into the cell anymore:
        SwTwips nDistanceToUpperPrtBottom =
            (pFrm->Frm().*fnRect->fnBottomDist)( (pLay->*fnRect->fnGetPrtBottom)());
        if ( nDistanceToUpperPrtBottom < 0 && pFrm->IsInSplitTableRow() )
            pFrm->InvalidatePos();

        pFrm = pFrm->GetNext();
    }
    return bRet;
}

void SwCellFrm::Format( const SwBorderAttrs *pAttrs )
{
    ASSERT( pAttrs, "CellFrm::Format, pAttrs ist 0." );
    const SwTabFrm* pTab = FindTabFrm();
    SWRECTFN( pTab )

    if ( !bValidPrtArea )
    {
        bValidPrtArea = TRUE;

        //Position einstellen.
        if ( Lower() )
        {
            SwTwips nTopSpace, nBottomSpace, nLeftSpace, nRightSpace;
            // --> collapsing borders FME 2005-05-27 #i29550#
            if ( pTab->IsCollapsingBorders() && !Lower()->IsRowFrm()  )
            {
                const SvxBoxItem& rBoxItem = pAttrs->GetBox();
                nLeftSpace   = rBoxItem.GetDistance( BOX_LINE_LEFT );
                nRightSpace  = rBoxItem.GetDistance( BOX_LINE_RIGHT );
                nTopSpace    =  ((SwRowFrm*)GetUpper())->GetTopMarginForLowers();
                nBottomSpace =  ((SwRowFrm*)GetUpper())->GetBottomMarginForLowers();
            }
            else
            {
            // <-- collapsing
                // OD 23.01.2003 #106895# - add 1st param to <SwBorderAttrs::CalcRight(..)>
                nLeftSpace   = pAttrs->CalcLeft( this );
                nRightSpace  = pAttrs->CalcRight( this );
                nTopSpace    = pAttrs->CalcTop();
                nBottomSpace = pAttrs->CalcBottom();
            }
            (this->*fnRect->fnSetXMargins)( nLeftSpace, nRightSpace );
            (this->*fnRect->fnSetYMargins)( nTopSpace, nBottomSpace );
        }
    }
    long nRemaining = ::lcl_CalcMinCellHeight( this, pAttrs );
    if ( !bValidSize )
    {
        bValidSize = TRUE;

        //Die VarSize der CellFrms ist immer die Breite.
        //Tatsaechlich ist die Breite jedoch nicht Variabel, sie wird durch das
        //Format vorgegeben. Dieser Vorgegebene Wert muss aber nun wiederum
        //nicht der tatsaechlichen Breite entsprechen. Die Breite wird auf
        //Basis des Attributes errechnet, der Wert im Attribut passt zu dem
        //gewuenschten Wert des TabFrms. Anpassungen die dort vorgenommen
        //wurden werden hier Proportional beruecksichtigt.
        //Wenn die Celle keinen Nachbarn mehr hat beruecksichtigt sie nicht
        //die Attribute, sonder greift sich einfach den Rest des
        //Uppers.
        SwTwips nWidth;
        if ( GetNext() )
        {
            SwTwips nWish = pTab->GetFmt()->GetFrmSize().GetWidth();
            nWidth = pAttrs->GetSize().Width();

            ASSERT( nWish, "Tabelle ohne Breite?" );
            ASSERT( nWidth <= nWish, "Zelle breiter als Tabelle." );
            ASSERT( nWidth > 0, "Box without width" );

            long nPrtWidth = (pTab->Prt().*fnRect->fnGetWidth)();
            if ( nWish != nPrtWidth )
            {
                // #i12092# use double instead of long,
                // otherwise this cut lead to overflows
                double nTmpWidth = nWidth;
                nTmpWidth *= nPrtWidth;
                nTmpWidth /= nWish;
                nWidth = (SwTwips)nTmpWidth;
            }
        }
        else
        {
            ASSERT( pAttrs->GetSize().Width() > 0, "Box without width" );
            nWidth = (GetUpper()->Prt().*fnRect->fnGetWidth)();
            SwFrm *pPre = GetUpper()->Lower();
            while ( pPre != this )
            {   nWidth -= (pPre->Frm().*fnRect->fnGetWidth)();
                pPre = pPre->GetNext();
            }
        }
        const long nDiff = nWidth - (Frm().*fnRect->fnGetWidth)();
        if( IsNeighbourFrm() && IsRightToLeft() )
            (Frm().*fnRect->fnSubLeft)( nDiff );
        else
            (Frm().*fnRect->fnAddRight)( nDiff );
        (Prt().*fnRect->fnAddRight)( nDiff );

        //Jetzt die Hoehe einstellen, sie wird vom Inhalt und den Raendern
        //bestimmt.
        const long nDiffHeight = nRemaining - (Frm().*fnRect->fnGetHeight)();
        if ( nDiffHeight )
        {
            if ( nDiffHeight > 0 )
            {
                //Wieder validieren wenn kein Wachstum stattgefunden hat.
                //Invalidiert wird durch AdjustCells von der Row.
                if ( !Grow( nDiffHeight PHEIGHT ) )
                    bValidSize = bValidPrtArea = TRUE;
            }
            else
            {
                //Nur dann invalidiert lassen, wenn tatsaechlich
                //geshrinkt wurde; das kann abgelehnt werden, weil alle
                //nebeneinanderliegenden Zellen gleichgross sein muessen.
                if ( !Shrink( -nDiffHeight PHEIGHT ) )
                    bValidSize = bValidPrtArea = TRUE;
            }
        }
    }
    const SwFmtVertOrient &rOri = pAttrs->GetAttrSet().GetVertOrient();

    if ( !Lower() )
        return;

    // From now on, all operations are related to the table cell.
    SWREFRESHFN( this )

    SwPageFrm* pPg = 0;
    if ( !FindTabFrm()->IsRebuildLastLine() && VERT_NONE != rOri.GetVertOrient() &&
    // --> FME 2004-06-29 #116532# Do not consider vertical alignment in grid mode
         !(pPg = FindPageFrm())->HasGrid() )
    // <--
    {
        if ( !Lower()->IsCntntFrm() && !Lower()->IsSctFrm() )
        {
            //ASSERT fuer HTML-Import!
            ASSERT( !this, "VAlign an Zelle ohne Inhalt" );
            return;
        }
        BOOL bVertDir = TRUE;
        //Keine Ausrichtung wenn Rahmen mit Umlauf in die Zelle ragen.
        if ( pPg->GetSortedObjs() )
        {
            SwRect aRect( Prt() ); aRect += Frm().Pos();
            for ( USHORT i = 0; i < pPg->GetSortedObjs()->Count(); ++i )
            {
                const SwAnchoredObject* pAnchoredObj = (*pPg->GetSortedObjs())[i];
                SwRect aTmp( pAnchoredObj->GetObjRect() );
                if ( aTmp.IsOver( aRect ) )
                {
                    const SwFmtSurround &rSur = pAnchoredObj->GetFrmFmt().GetSurround();
                    if ( SURROUND_THROUGHT != rSur.GetSurround() )
                    {
                        if ( pAnchoredObj->ISA(SwFlyFrm) )
                        {
                            const SwFlyFrm *pFly =
                                    static_cast<const SwFlyFrm*>(pAnchoredObj);
                            if ( pFly->IsAnLower( this ) )
                                continue;
                        }

                        const SwFrm* pAnch = pAnchoredObj->GetAnchorFrm();
                        if ( !IsAnLower( pAnch ) )
                        {
                            bVertDir = FALSE;
                            break;
                        }
                    }
                }
            }
        }

        long nPrtHeight = (Prt().*fnRect->fnGetHeight)();
        if( ( bVertDir && ( nRemaining -= lcl_CalcTopAndBottomMargin( *this, *pAttrs ) ) < nPrtHeight ) ||
            (Lower()->Frm().*fnRect->fnGetTop)() != (this->*fnRect->fnGetPrtTop)() )
        {
            long lTopOfst = 0,
                    nDiff = (Prt().*fnRect->fnGetHeight)() - nRemaining;
            if ( nDiff >= 0 )
            {
                if ( bVertDir )
                {
                    switch ( rOri.GetVertOrient() )
                    {
                        case VERT_CENTER:   lTopOfst = nDiff / 2; break;
                        case VERT_BOTTOM:   lTopOfst = nDiff;     break;
                    };
                }
                long nTmp = (*fnRect->fnYInc)(
                                    (this->*fnRect->fnGetPrtTop)(), lTopOfst );
                if ( lcl_ArrangeLowers( this, nTmp, !bVertDir ) )
                    SetCompletePaint();
            }
        }
    }
    else
    {
        //Ist noch eine alte Ausrichtung beruecksichtigt worden?
        if ( Lower()->IsCntntFrm() )
        {
            const long lYStart = (this->*fnRect->fnGetPrtTop)();
            lcl_ArrangeLowers( this, lYStart, TRUE );
        }
    }
}

/*************************************************************************
|*
|*    SwCellFrm::Modify()
|*
|*    Ersterstellung    MA 20. Dec. 96
|*    Letzte Aenderung  MA 20. Dec. 96
|*
|*************************************************************************/

void SwCellFrm::Modify( SfxPoolItem * pOld, SfxPoolItem * pNew )
{
    BOOL bAttrSetChg = pNew && RES_ATTRSET_CHG == pNew->Which();
    const SfxPoolItem *pItem = 0;

    if( bAttrSetChg )
        ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_VERT_ORIENT, FALSE, &pItem);
    else if ( RES_VERT_ORIENT == pNew->Which() )
        pItem = pNew;

    if ( pItem )
    {
        BOOL bInva = TRUE;
        if ( VERT_NONE == ((SwFmtVertOrient*)pItem)->GetVertOrient() &&
             // OD 04.11.2003 #112910#
             Lower() && Lower()->IsCntntFrm() )
        {
            SWRECTFN( this )
            const long lYStart = (this->*fnRect->fnGetPrtTop)();
            bInva = lcl_ArrangeLowers( this, lYStart, FALSE );
        }
        if ( bInva )
        {
            SetCompletePaint();
            InvalidatePrt();
        }
    }

    if ( ( bAttrSetChg &&
           SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_PROTECT, FALSE ) ) ||
         RES_PROTECT == pNew->Which() )
    {
        ViewShell *pSh = GetShell();
        if( pSh && pSh->GetLayout()->IsAnyShellAccessible() )
            pSh->Imp()->InvalidateAccessibleEditableState( sal_True, this );
    }

    if ( bAttrSetChg &&
         SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_FRAMEDIR, FALSE, &pItem ) )
    {
        SetDerivedVert( FALSE );
        CheckDirChange();
    }

    // --> collapsing borders FME 2005-05-27 #i29550#
    if ( bAttrSetChg &&
         SFX_ITEM_SET == ((SwAttrSetChg*)pNew)->GetChgSet()->GetItemState( RES_BOX, FALSE, &pItem ) )
    {
        SwFrm* pTmpUpper = GetUpper();
        while ( pTmpUpper->GetUpper() && !pTmpUpper->GetUpper()->IsTabFrm() )
            pTmpUpper = pTmpUpper->GetUpper();

        SwTabFrm* pTabFrm = (SwTabFrm*)pTmpUpper->GetUpper();
        if ( pTabFrm->IsCollapsingBorders() )
        {
            // Invalidate lowers of this and next row:
            lcl_InvalidateAllLowersPrt( (SwRowFrm*)pTmpUpper );
            pTmpUpper = pTmpUpper->GetNext();
            if ( pTmpUpper )
                lcl_InvalidateAllLowersPrt( (SwRowFrm*)pTmpUpper );
            else
                pTabFrm->InvalidatePrt();
        }
    }
    // <-- collapsing

    SwLayoutFrm::Modify( pOld, pNew );
}

//
// Helper functions for repeated headlines:
//

/*
 * SwTabFrm::IsInHeadline( const SwFrm& rFrm )
 */
bool SwTabFrm::IsInHeadline( const SwFrm& rFrm ) const
{
    ASSERT( IsAnLower( &rFrm ) && rFrm.IsInTab(),
             "SwTabFrm::IsInHeadline called for frame not lower of table" )

    const SwFrm* pTmp = &rFrm;
    while ( !pTmp->GetUpper()->IsTabFrm() )
        pTmp = pTmp->GetUpper();

    return GetTable()->IsHeadline( *((SwRowFrm*)pTmp)->GetTabLine() );
}

/*
 * SwTabFrm::GetFirstNonHeadlineRow()
 *
 * If this is a master table, we can may assume, that there are at least
 * nRepeat lines in the table.
 * If this is a follow table, there are intermediate states for the table
 * layout, e.g., during deletion of rows, which makes it necessary to find
 * the first non-headline row by evaluating the headline flag at the row frame.
 */
SwRowFrm* SwTabFrm::GetFirstNonHeadlineRow() const
{
    SwRowFrm* pRet = (SwRowFrm*)Lower();
    if ( pRet )
    {
        if ( IsFollow() )
        {
            while ( pRet && pRet->IsRepeatedHeadline() )
                pRet = (SwRowFrm*)pRet->GetNext();
        }
        else
        {
            USHORT nRepeat = GetTable()->GetRowsToRepeat();
            while ( pRet && nRepeat > 0 )
            {
                pRet = (SwRowFrm*)pRet->GetNext();
                --nRepeat;
            }
        }
    }

    return (SwRowFrm*)pRet;
}

/*
 * SwTable::IsHeadline()
 */
bool SwTable::IsHeadline( const SwTableLine& rLine ) const
{
    for ( USHORT i = 0; i < GetRowsToRepeat(); ++i )
        if ( GetTabLines()[ i ] == &rLine )
            return true;

    return false;
}

bool SwTabFrm::IsLayoutSplitAllowed() const
{
    return GetFmt()->GetLayoutSplit().GetValue();
}

// --> collapsing borders FME 2005-05-27 #i29550#

USHORT SwTabFrm::GetBottomLineSize() const
{
    ASSERT( IsCollapsingBorders(),
            "BottomLineSize only required for collapsing borders" )

    const SwFrm* pTmp = Lower();
    while ( pTmp->GetNext() )
        pTmp = pTmp->GetNext();

    return ((SwRowFrm*)pTmp)->GetBottomLineSize();
}

bool SwTabFrm::IsCollapsingBorders() const
{
    return ((SfxBoolItem&)GetFmt()->GetAttrSet().Get( RES_COLLAPSING_BORDERS )).GetValue();
}

// <-- collapsing


//
// Local helper function to calculate height of first text row
//
SwTwips lcl_CalcHeightOfFirstContentLine( const SwRowFrm& rSourceLine )
{
    // Find corresponding split line in master table
    const SwTabFrm* pTab = rSourceLine.FindTabFrm();
    SWRECTFN( pTab )
    const SwCellFrm* pCurrSourceCell = (SwCellFrm*)rSourceLine.Lower();

    //
    // 1. Case: rSourceLine is a follow flow line.
    // In this case we have to return the minimum of the heights
    // of the first lines in rSourceLine.
    //
    // 2. Case: rSourceLine is not a follow flow line.
    // In this case we have to return the maximum of the heights
    // of the first lines in rSourceLine.
    //
    bool bIsInFollowFlowLine = rSourceLine.IsInFollowFlowRow();
    SwTwips nHeight = bIsInFollowFlowLine ? LONG_MAX : 0;

    while ( pCurrSourceCell )
    {
        if ( pCurrSourceCell->Lower() && pCurrSourceCell->Lower()->IsRowFrm() )
        {
            SwRowFrm* pTmpSourceRow = (SwRowFrm*)pCurrSourceCell->Lower();
            while ( pTmpSourceRow )
            {
                // TODO: recursion
                pTmpSourceRow = (SwRowFrm*)pTmpSourceRow->GetNext();
            }
        }
        else
        {
            const SwFrm *pTmp = pCurrSourceCell->Lower();
            if ( pTmp )
            {
                SwTwips nTmpHeight = USHRT_MAX;
                if ( pTmp->IsTabFrm() )
                {
                    nTmpHeight = ((SwTabFrm*)pTmp)->CalcHeightOfFirstContentLine();
                }
                else if ( pTmp->IsTxtFrm() )
                {
                    SwTxtFrm* pTxtFrm = (SwTxtFrm*)pTmp;
                    pTxtFrm->GetFormatted();
                    nTmpHeight = pTxtFrm->FirstLineHeight();
                }

                if ( USHRT_MAX != nTmpHeight )
                {
                    const SwCellFrm* pPrevCell = pCurrSourceCell->GetPreviousCell();
                    if ( pPrevCell )
                    {
                        // If we are in a split row, there may be some space
                        // left in the cell frame of the master row.
                        // We look for the minimum of all first line heights;
                        SwTwips nReal = (pPrevCell->Prt().*fnRect->fnGetHeight)();
                        const SwFrm* pFrm = pPrevCell->Lower();
                        const SwFrm* pLast = pFrm;
                        while ( pFrm )
                        {
                            nReal -= (pFrm->Frm().*fnRect->fnGetHeight)();
                            pLast = pFrm;
                            pFrm = pFrm->GetNext();
                        }

                        // --> FME, OD 2004-07-15 #i26831#, #i26520#
                        // The additional lower space of the current last.
                        if ( pLast )
                        {
                            nReal += SwFlowFrm::CastFlowFrm(pLast)->CalcAddLowerSpaceAsLastInTableCell();
                        }
                        // Don't forget the upper space and lower space,
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace( NULL, pLast);
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
                        // <--

                        if ( nReal > 0 )
                            nTmpHeight -= nReal;
                    }
                    else
                    {
                        // pFirstRow is not a FollowFlowRow. In this case,
                        // we look for the maximum of all first line heights:
                        SwBorderAttrAccess aAccess( SwFrm::GetCache(), pCurrSourceCell );
                        const SwBorderAttrs &rAttrs = *aAccess.Get();
                        nTmpHeight += rAttrs.CalcTop() + rAttrs.CalcBottom();
                        // --> OD 2004-07-16 #i26250#
                        // Don't forget the upper space and lower space,
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcUpperSpace();
                        nTmpHeight += SwFlowFrm::CastFlowFrm(pTmp)->CalcLowerSpace();
                        // <--
                    }
                }

                if ( bIsInFollowFlowLine )
                {
                    // minimum
                    if ( nTmpHeight < nHeight )
                        nHeight = nTmpHeight;
                }
                else
                {
                    // maximum
                    if ( nTmpHeight > nHeight && USHRT_MAX != nTmpHeight )
                        nHeight = nTmpHeight;
                }

            }
        }

        pCurrSourceCell = (SwCellFrm*)pCurrSourceCell->GetNext();
    }

    return ( LONG_MAX == nHeight ) ? 0 : nHeight;
}

//
// Function to calculate height of first text row
//
SwTwips SwTabFrm::CalcHeightOfFirstContentLine() const
{
    SWRECTFN( this )

    const USHORT nRepeat = GetTable()->GetRowsToRepeat();
    SwTwips nTmpHeight = 0;
    if ( !IsFollow() && nRepeat > 0 )
        nTmpHeight = lcl_GetHeightOfRows( *this, nRepeat );

    SwFrm* pFirstRow = GetFirstNonHeadlineRow();
    ASSERT( !IsFollow() || pFirstRow, "FollowTable without Lower" )

    // pFirstRow row is the first non-heading row.
    // nTmpHeight is the height of the heading row if we are a follow.
    if ( pFirstRow )
    {
        const bool bSplittable = ((SwRowFrm*)pFirstRow)->IsRowSplitAllowed();
        const bool bDontSplit = !IsFollow() && !GetFmt()->GetLayoutSplit().GetValue();

        if ( bDontSplit )
            nTmpHeight = (Frm().*fnRect->fnGetHeight)();
        else if ( !bSplittable )
            nTmpHeight += (pFirstRow->Frm().*fnRect->fnGetHeight)();
        else
        {
            const bool bOldJoinLock = IsJoinLocked();
            ((SwTabFrm*)this)->LockJoin();
            nTmpHeight += lcl_CalcHeightOfFirstContentLine( *(SwRowFrm*)pFirstRow );
            if ( !bOldJoinLock )
                ((SwTabFrm*)this)->UnlockJoin();
        }
    }

    return nTmpHeight;
}


