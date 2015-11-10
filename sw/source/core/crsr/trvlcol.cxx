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
    return const_cast<SwLayoutFrm*>(pLayFrm);
}

SwLayoutFrm* GetNextColumn( const SwLayoutFrm* pLayFrm )
{
    SwLayoutFrm* pActCol = GetCurrColumn( pLayFrm );
    return pActCol ? static_cast<SwLayoutFrm*>(pActCol->GetNext()) : nullptr;
}

SwLayoutFrm* GetPrevColumn( const SwLayoutFrm* pLayFrm )
{
    SwLayoutFrm* pActCol = GetCurrColumn( pLayFrm );
    return pActCol ? static_cast<SwLayoutFrm*>(pActCol->GetPrev()) : nullptr;
}

SwContentFrm* GetColumnStt( const SwLayoutFrm* pColFrm )
{
    return pColFrm ? const_cast<SwContentFrm*>(pColFrm->ContainsContent()) : nullptr;
}

SwContentFrm* GetColumnEnd( const SwLayoutFrm* pColFrm )
{
    SwContentFrm *pRet = GetColumnStt( pColFrm );
    if( !pRet )
        return nullptr;

    SwContentFrm *pNxt = pRet->GetNextContentFrm();
    while( pNxt && pColFrm->IsAnLower( pNxt ) )
    {
        pRet = pNxt;
        pNxt = pNxt->GetNextContentFrm();
    }
    return pRet;
}

SwWhichColumn fnColumnPrev = &GetPrevColumn;
SwWhichColumn fnColumnCurr = &GetCurrColumn;
SwWhichColumn fnColumnNext = &GetNextColumn;
SwPosColumn fnColumnStart = &GetColumnStt;
SwPosColumn fnColumnEnd = &GetColumnEnd;

bool SwCrsrShell::MoveColumn( SwWhichColumn fnWhichCol, SwPosColumn fnPosCol )
{
    bool bRet = false;
    if( !m_pTableCrsr )
    {
        SwLayoutFrm* pLayFrm = GetCurrFrm()->GetUpper();
        if( pLayFrm && nullptr != ( pLayFrm = (*fnWhichCol)( pLayFrm )) )
        {
            SwContentFrm* pCnt = (*fnPosCol)( pLayFrm );
            if( pCnt )
            {
                SET_CURR_SHELL( this );
                SwCallLink aLk( *this ); // watch Crsr-Moves; call Link if needed
                SwCrsrSaveState aSaveState( *m_pCurCrsr );

                pCnt->Calc(GetOut());

                Point aPt( pCnt->Frm().Pos() + pCnt->Prt().Pos() );
                if( fnPosCol == GetColumnEnd )
                {
                    aPt.setX(aPt.getX() + pCnt->Prt().Width());
                    aPt.setY(aPt.getY() + pCnt->Prt().Height());
                }

                pCnt->GetCrsrOfst( m_pCurCrsr->GetPoint(), aPt );

                if( !m_pCurCrsr->IsInProtectTable( true ) &&
                    !m_pCurCrsr->IsSelOvr() )
                {
                    UpdateCrsr();
                    bRet = true;
                }
            }
        }
    }
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
