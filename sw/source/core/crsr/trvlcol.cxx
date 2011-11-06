/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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



