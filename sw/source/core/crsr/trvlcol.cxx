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
    if( !m_pTblCrsr )
    {
        SwLayoutFrm* pLayFrm = GetCurrFrm()->GetUpper();
        if( pLayFrm && 0 != ( pLayFrm = (*fnWhichCol)( pLayFrm )) )
        {
            SwCntntFrm* pCnt = (*fnPosCol)( pLayFrm );
            if( pCnt )
            {
                SET_CURR_SHELL( this );
                SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
                SwCrsrSaveState aSaveState( *m_pCurCrsr );

                pCnt->Calc();

                Point aPt( pCnt->Frm().Pos() + pCnt->Prt().Pos() );
                if( fnPosCol == GetColumnEnd )
                {
                    aPt.setX(aPt.getX() + pCnt->Prt().Width());
                    aPt.setY(aPt.getY() + pCnt->Prt().Height());
                }

                pCnt->GetCrsrOfst( m_pCurCrsr->GetPoint(), aPt );

                if( !m_pCurCrsr->IsInProtectTable( sal_True ) &&
                    !m_pCurCrsr->IsSelOvr() )
                {
                    UpdateCrsr();
                    bRet = sal_True;
                }
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
