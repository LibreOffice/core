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


sal_Bool SwCrsrShell::MoveColumn( SwWhichColumn fnWhichCol, SwPosColumn fnPosCol )
{
    sal_Bool bRet = sal_False;
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

                if( !pCurCrsr->IsInProtectTable( sal_True ) &&
                    !pCurCrsr->IsSelOvr() )
                {
                    UpdateCrsr();
                    bRet = sal_True;
                }
            }
        }
    }
    return bRet;
}



