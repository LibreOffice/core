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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <crsrsh.hxx>
#include <doc.hxx>
#include <swcrsr.hxx>
#include <docary.hxx>
#include <fmtcntnt.hxx>
#include <viscrs.hxx>
#include <callnk.hxx>
#include <pamtyp.hxx>
#include <section.hxx>



sal_Bool GotoPrevRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        sal_Bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd, - 1 );

    do {
        while( aIdx.GetIndex() &&
            0 == ( pNd = aIdx.GetNode().StartOfSectionNode()->GetSectionNode()) )
            aIdx--;

        if( pNd )       // gibt einen weiteren SectionNode ?
        {
            if( pNd->GetSection().IsHiddenFlag() ||
                ( !bInReadOnly &&
                  pNd->GetSection().IsProtectFlag() ))
            {
                // geschuetzte/versteckte ueberspringen wir
                aIdx.Assign( *pNd, - 1 );
            }
            else if( fnPosRegion == fnMoveForward )
            {
                aIdx = *pNd;
                SwCntntNode* pCNd = pNd->GetNodes().GoNextSection( &aIdx,
                                                sal_True, !bInReadOnly );
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
                                                sal_True, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx.Assign( *pNd, - 1 );
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }

            rCurCrsr.GetPoint()->nNode = aIdx;
            return sal_True;
        }
    } while( pNd );
    return sal_False;
}


sal_Bool GotoNextRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        sal_Bool bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd->EndOfSectionNode(), - 1 );

    sal_uLong nEndCount = aIdx.GetNode().GetNodes().Count()-1;
    do {
        while( aIdx.GetIndex() < nEndCount &&
                0 == ( pNd = aIdx.GetNode().GetSectionNode()) )
            aIdx++;

        if( pNd )       // gibt einen weiteren SectionNode ?
        {
            if( pNd->GetSection().IsHiddenFlag() ||
                ( !bInReadOnly &&
                  pNd->GetSection().IsProtectFlag() ))
            {
                // geschuetzte/versteckte ueberspringen wir
                aIdx.Assign( *pNd->EndOfSectionNode(), +1 );
            }
            else if( fnPosRegion == fnMoveForward )
            {
                aIdx = *pNd;
                SwCntntNode* pCNd = pNd->GetNodes().GoNextSection( &aIdx,
                                                sal_True, !bInReadOnly );
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
                                                sal_True, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx++;
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }

            rCurCrsr.GetPoint()->nNode = aIdx;
            return sal_True;
        }
    } while( pNd );
    return sal_False;
}


sal_Bool GotoCurrRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        sal_Bool bInReadOnly )
{
    SwSectionNode* pNd = rCurCrsr.GetNode()->FindSectionNode();
    if( !pNd )
        return sal_False;

    SwPosition* pPos = rCurCrsr.GetPoint();
    sal_Bool bMoveBackward = fnPosRegion == fnMoveBackward;

    SwCntntNode* pCNd;
    if( bMoveBackward )
    {
        SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
        pCNd = pNd->GetNodes().GoPrevSection( &aIdx, sal_True, !bInReadOnly );
    }
    else
    {
        SwNodeIndex aIdx( *pNd );
        pCNd = pNd->GetNodes().GoNextSection( &aIdx, sal_True, !bInReadOnly );
    }

    if( pCNd )
    {
        pPos->nNode = *pCNd;
        xub_StrLen nTmpPos = bMoveBackward ? pCNd->Len() : 0;
        pPos->nContent.Assign( pCNd, nTmpPos );
    }
    return 0 != pCNd;
}


sal_Bool GotoCurrRegionAndSkip( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                                sal_Bool bInReadOnly )
{
    SwNode* pCurrNd = rCurCrsr.GetNode();
    SwSectionNode* pNd = pCurrNd->FindSectionNode();
    if( !pNd )
        return sal_False;

    SwPosition* pPos = rCurCrsr.GetPoint();
    xub_StrLen nCurrCnt = pPos->nContent.GetIndex();
    sal_Bool bMoveBackward = fnPosRegion == fnMoveBackward;

    do {
        SwCntntNode* pCNd;
        if( bMoveBackward ) // ans Ende vom Bereich
        {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
            pCNd = pNd->GetNodes().GoPrevSection( &aIdx, sal_True, !bInReadOnly );
            if( !pCNd )
                return sal_False;
            pPos->nNode = aIdx;
        }
        else
        {
            SwNodeIndex aIdx( *pNd );
            pCNd = pNd->GetNodes().GoNextSection( &aIdx, sal_True, !bInReadOnly );
            if( !pCNd )
                return sal_False;
            pPos->nNode = aIdx;
        }

        xub_StrLen nTmpPos = bMoveBackward ? pCNd->Len() : 0;
        pPos->nContent.Assign( pCNd, nTmpPos );

        if( &pPos->nNode.GetNode() != pCurrNd ||
            pPos->nContent.GetIndex() != nCurrCnt )
            // es gab eine Veraenderung
            return sal_True;

        // dann versuche mal den "Parent" dieser Section
        SwSection* pParent = pNd->GetSection().GetParent();
        pNd = pParent ? pParent->GetFmt()->GetSectionNode() : 0;
    } while( pNd );
    return sal_False;
}



sal_Bool SwCursor::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCrsrSaveState aSaveState( *this );
    return !dynamic_cast<SwTableCursor*>(this) &&
            (*fnWhichRegion)( *this, fnPosRegion, IsReadOnlyAvailable()  ) &&
            !IsSelOvr() &&
            ( GetPoint()->nNode.GetIndex() != pSavePos->nNode ||
                GetPoint()->nContent.GetIndex() != pSavePos->nCntnt );
}

sal_Bool SwCrsrShell::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    sal_Bool bRet = !pTblCrsr && pCurCrsr->MoveRegion( fnWhichRegion, fnPosRegion );
    if( bRet )
        UpdateCrsr();
    return bRet;
}


sal_Bool SwCursor::GotoRegion( const String& rName )
{
    sal_Bool bRet = sal_False;
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( sal_uInt16 n = rFmts.Count(); n; )
    {
        const SwSectionFmt* pFmt = rFmts[ --n ];
        const SwNodeIndex* pIdx;
        const SwSection* pSect;
        if( 0 != ( pSect = pFmt->GetSection() ) &&
            pSect->GetSectionName() == rName &&
            0 != ( pIdx = pFmt->GetCntnt().GetCntntIdx() ) &&
            pIdx->GetNode().GetNodes().IsDocNodes() )
        {
            // ein Bereich im normalen NodesArr
            SwCrsrSaveState aSaveState( *this );

            GetPoint()->nNode = *pIdx;
            Move( fnMoveForward, fnGoCntnt );
            bRet = !IsSelOvr();
        }
    }
    return bRet;
}

sal_Bool SwCrsrShell::GotoRegion( const String& rName )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    sal_Bool bRet = !pTblCrsr && pCurCrsr->GotoRegion( rName );
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY ); // und den akt. Updaten
    return bRet;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
