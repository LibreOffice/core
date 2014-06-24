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
#include <swcrsr.hxx>
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <pamtyp.hxx>
#include <section.hxx>
#include <trvlreg.hxx>

bool GotoPrevRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd, - 1 );

    do {
        while( aIdx.GetIndex() &&
            0 == ( pNd = aIdx.GetNode().StartOfSectionNode()->GetSectionNode()) )
            aIdx--;

        if( pNd ) // is there another section node?
        {
            if( pNd->GetSection().IsHiddenFlag() ||
                ( !bInReadOnly &&
                  pNd->GetSection().IsProtectFlag() ))
            {
                // skip protected or hidden ones
                aIdx.Assign( *pNd, - 1 );
            }
            else if( fnPosRegion == fnMoveForward )
            {
                aIdx = *pNd;
                SwCntntNode* pCNd = pNd->GetNodes().GoNextSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx--;
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, 0 );
            }
            else
            {
                aIdx = *pNd->EndOfSectionNode();
                SwCntntNode* pCNd = pNd->GetNodes().GoPrevSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx.Assign( *pNd, - 1 );
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }
            rCurCrsr.GetPoint()->nNode = aIdx;
            return true;
        }
    } while( pNd );
    return false;
}

bool GotoNextRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd->EndOfSectionNode(), - 1 );

    sal_uLong nEndCount = aIdx.GetNode().GetNodes().Count()-1;
    do {
        while( aIdx.GetIndex() < nEndCount &&
                0 == ( pNd = aIdx.GetNode().GetSectionNode()) )
            ++aIdx;

        if( pNd ) // is there another section node?
        {
            if( pNd->GetSection().IsHiddenFlag() ||
                ( !bInReadOnly &&
                  pNd->GetSection().IsProtectFlag() ))
            {
                // skip protected or hidden ones
                aIdx.Assign( *pNd->EndOfSectionNode(), +1 );
            }
            else if( fnPosRegion == fnMoveForward )
            {
                aIdx = *pNd;
                SwCntntNode* pCNd = pNd->GetNodes().GoNextSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx.Assign( *pNd->EndOfSectionNode(), +1 );
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, 0 );
            }
            else
            {
                aIdx = *pNd->EndOfSectionNode();
                SwCntntNode* pCNd = pNd->GetNodes().GoPrevSection( &aIdx,
                                                true, !bInReadOnly );
                if( !pCNd )
                {
                    ++aIdx;
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }
            rCurCrsr.GetPoint()->nNode = aIdx;
            return true;
        }
    } while( pNd );
    return false;
}

bool GotoCurrRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        bool bInReadOnly )
{
    SwSectionNode* pNd = rCurCrsr.GetNode().FindSectionNode();
    if( !pNd )
        return false;

    SwPosition* pPos = rCurCrsr.GetPoint();
    bool bMoveBackward = fnPosRegion == fnMoveBackward;

    SwCntntNode* pCNd;
    if( bMoveBackward )
    {
        SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
        pCNd = pNd->GetNodes().GoPrevSection( &aIdx, true, !bInReadOnly );
    }
    else
    {
        SwNodeIndex aIdx( *pNd );
        pCNd = pNd->GetNodes().GoNextSection( &aIdx, true, !bInReadOnly );
    }

    if( pCNd )
    {
        pPos->nNode = *pCNd;
        pPos->nContent.Assign( pCNd, bMoveBackward ? pCNd->Len() : 0 );
    }
    return 0 != pCNd;
}

bool GotoCurrRegionAndSkip( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                                bool bInReadOnly )
{
    SwNode& rCurrNd = rCurCrsr.GetNode();
    SwSectionNode* pNd = rCurrNd.FindSectionNode();
    if( !pNd )
        return false;

    SwPosition* pPos = rCurCrsr.GetPoint();
    const sal_Int32 nCurrCnt = pPos->nContent.GetIndex();
    bool bMoveBackward = fnPosRegion == fnMoveBackward;

    do {
        SwCntntNode* pCNd;
        if( bMoveBackward ) // to the end of the section
        {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
            pCNd = pNd->GetNodes().GoPrevSection( &aIdx, true, !bInReadOnly );
            if( !pCNd )
                return false;
            pPos->nNode = aIdx;
        }
        else
        {
            SwNodeIndex aIdx( *pNd );
            pCNd = pNd->GetNodes().GoNextSection( &aIdx, true, !bInReadOnly );
            if( !pCNd )
                return false;
            pPos->nNode = aIdx;
        }

        pPos->nContent.Assign( pCNd, bMoveBackward ? pCNd->Len() : 0 );

        if( &pPos->nNode.GetNode() != &rCurrNd ||
            pPos->nContent.GetIndex() != nCurrCnt )
            // there was a change
            return true;

        // try also the parent of this section
        SwSection* pParent = pNd->GetSection().GetParent();
        pNd = pParent ? pParent->GetFmt()->GetSectionNode() : 0;
    } while( pNd );
    return false;
}

bool SwCursor::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCrsrSaveState aSaveState( *this );
    return !dynamic_cast<SwTableCursor*>(this) &&
            (*fnWhichRegion)( *this, fnPosRegion, IsReadOnlyAvailable()  ) &&
            !IsSelOvr() &&
            ( GetPoint()->nNode.GetIndex() != pSavePos->nNode ||
                GetPoint()->nContent.GetIndex() != pSavePos->nCntnt );
}

bool SwCrsrShell::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves;call Link if needed
    bool bRet = !m_pTblCrsr && m_pCurCrsr->MoveRegion( fnWhichRegion, fnPosRegion );
    if( bRet )
        UpdateCrsr();
    return bRet;
}

bool SwCursor::GotoRegion( const OUString& rName )
{
    bool bRet = false;
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = rFmts.size(); n; )
    {
        const SwSectionFmt* pFmt = rFmts[ --n ];
        const SwNodeIndex* pIdx = 0;
        const SwSection* pSect;
        if( 0 != ( pSect = pFmt->GetSection() ) &&
            pSect->GetSectionName() == rName &&
            0 != ( pIdx = pFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            // area in normal nodes array
            SwCrsrSaveState aSaveState( *this );

            GetPoint()->nNode = *pIdx;
            Move( fnMoveForward, fnGoCntnt );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

bool SwCrsrShell::GotoRegion( const OUString& rName )
{
    SwCallLink aLk( *this ); // watch Crsr-Moves;call Link if needed
    bool bRet = !m_pTblCrsr && m_pCurCrsr->GotoRegion( rName );
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY );
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
