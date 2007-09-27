/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: trvlcol.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:30:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#include <crsrsh.hxx>
#include <doc.hxx>
#include <layfrm.hxx>
#include <cntfrm.hxx>
#include <swcrsr.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>



SwLayoutFrm* GetCurrColumn( const SwLayoutFrm* pLayFrm )
{
    while( pLayFrm && !pLayFrm->IsColumnFrm() )
        pLayFrm = pLayFrm->GetUpper();
    return (SwLayoutFrm*)pLayFrm;
}


SwLayoutFrm* GetNextColumn( const SwLayoutFrm* pLayFrm )
{
    SwLayoutFrm* pActCol = GetCurrColumn( pLayFrm );
    return pActCol ? (SwLayoutFrm*)pActCol->GetNext() : 0;
}


SwLayoutFrm* GetPrevColumn( const SwLayoutFrm* pLayFrm )
{
    SwLayoutFrm* pActCol = GetCurrColumn( pLayFrm );
    return pActCol ? (SwLayoutFrm*)pActCol->GetPrev() : 0;
}


SwCntntFrm* GetColumnStt( const SwLayoutFrm* pColFrm )
{
    return pColFrm ? (SwCntntFrm*)pColFrm->ContainsCntnt() : 0;
}


SwCntntFrm* GetColumnEnd( const SwLayoutFrm* pColFrm )
{
    SwCntntFrm *pRet = GetColumnStt( pColFrm );
    if( !pRet )
        return 0;

    SwCntntFrm *pNxt = pRet->GetNextCntntFrm();
    while( pNxt && pColFrm->IsAnLower( pNxt ) )
    {
        pRet = pNxt;
        pNxt = pNxt->GetNextCntntFrm();
    }
    return pRet;
}


SwWhichColumn fnColumnPrev = &GetPrevColumn;
SwWhichColumn fnColumnCurr = &GetCurrColumn;
SwWhichColumn fnColumnNext = &GetNextColumn;
SwPosColumn fnColumnStart = &GetColumnStt;
SwPosColumn fnColumnEnd = &GetColumnEnd;


BOOL SwCrsrShell::MoveColumn( SwWhichColumn fnWhichCol, SwPosColumn fnPosCol )
{
    BOOL bRet = FALSE;
    if( !pTblCrsr )
    {
        SwLayoutFrm* pLayFrm = GetCurrFrm()->GetUpper();
        if( pLayFrm && 0 != ( pLayFrm = (*fnWhichCol)( pLayFrm )) )
        {
            SwCntntFrm* pCnt = (*fnPosCol)( pLayFrm );
            if( pCnt )
            {
                SET_CURR_SHELL( this );
                SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
                SwCrsrSaveState aSaveState( *pCurCrsr );

                pCnt->Calc();                   // ???

                Point aPt( pCnt->Frm().Pos() + pCnt->Prt().Pos() );
                if( fnPosCol == GetColumnEnd )
                {
                    aPt.X() += pCnt->Prt().Width();
                    aPt.Y() += pCnt->Prt().Height();
                }

                pCnt->GetCrsrOfst( pCurCrsr->GetPoint(), aPt );

                if( !pCurCrsr->IsInProtectTable( TRUE ) &&
                    !pCurCrsr->IsSelOvr() )
                {
                    UpdateCrsr();
                    bRet = TRUE;
                }
            }
        }
    }
    return bRet;
}



