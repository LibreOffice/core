/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: trvlreg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:31:15 $
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



#ifndef _CRSRSH_HXX
#include <crsrsh.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _SWCRSR_HXX
#include <swcrsr.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _VISCRS_HXX
#include <viscrs.hxx>
#endif
#ifndef _CALLNK_HXX
#include <callnk.hxx>
#endif
#ifndef _PAMTYP_HXX
#include <pamtyp.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif



BOOL GotoPrevRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        BOOL bInReadOnly )
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
                                                TRUE, !bInReadOnly );
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
                                                TRUE, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx.Assign( *pNd, - 1 );
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }

            rCurCrsr.GetPoint()->nNode = aIdx;
            return TRUE;
        }
    } while( pNd );
    return FALSE;
}


BOOL GotoNextRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        BOOL bInReadOnly )
{
    SwNodeIndex aIdx( rCurCrsr.GetPoint()->nNode );
    SwSectionNode* pNd = aIdx.GetNode().FindSectionNode();
    if( pNd )
        aIdx.Assign( *pNd->EndOfSectionNode(), - 1 );

    ULONG nEndCount = aIdx.GetNode().GetNodes().Count()-1;
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
                                                TRUE, !bInReadOnly );
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
                                                TRUE, !bInReadOnly );
                if( !pCNd )
                {
                    aIdx++;
                    continue;
                }
                rCurCrsr.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );
            }

            rCurCrsr.GetPoint()->nNode = aIdx;
            return TRUE;
        }
    } while( pNd );
    return FALSE;
}


BOOL GotoCurrRegion( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                        BOOL bInReadOnly )
{
    SwSectionNode* pNd = rCurCrsr.GetNode()->FindSectionNode();
    if( !pNd )
        return FALSE;

    SwPosition* pPos = rCurCrsr.GetPoint();
    BOOL bMoveBackward = fnPosRegion == fnMoveBackward;

    SwCntntNode* pCNd;
    if( bMoveBackward )
    {
        SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
        pCNd = pNd->GetNodes().GoPrevSection( &aIdx, TRUE, !bInReadOnly );
    }
    else
    {
        SwNodeIndex aIdx( *pNd );
        pCNd = pNd->GetNodes().GoNextSection( &aIdx, TRUE, !bInReadOnly );
    }

    if( pCNd )
    {
        pPos->nNode = *pCNd;
        xub_StrLen nTmpPos = bMoveBackward ? pCNd->Len() : 0;
        pPos->nContent.Assign( pCNd, nTmpPos );
    }
    return 0 != pCNd;
}


BOOL GotoCurrRegionAndSkip( SwPaM& rCurCrsr, SwPosRegion fnPosRegion,
                                BOOL bInReadOnly )
{
    SwNode* pCurrNd = rCurCrsr.GetNode();
    SwSectionNode* pNd = pCurrNd->FindSectionNode();
    if( !pNd )
        return FALSE;

    SwPosition* pPos = rCurCrsr.GetPoint();
    xub_StrLen nCurrCnt = pPos->nContent.GetIndex();
    BOOL bMoveBackward = fnPosRegion == fnMoveBackward;

    do {
        SwCntntNode* pCNd;
        if( bMoveBackward ) // ans Ende vom Bereich
        {
            SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
            pCNd = pNd->GetNodes().GoPrevSection( &aIdx, TRUE, !bInReadOnly );
            if( !pCNd )
                return FALSE;
            pPos->nNode = aIdx;
        }
        else
        {
            SwNodeIndex aIdx( *pNd );
            pCNd = pNd->GetNodes().GoNextSection( &aIdx, TRUE, !bInReadOnly );
            if( !pCNd )
                return FALSE;
            pPos->nNode = aIdx;
        }

        xub_StrLen nTmpPos = bMoveBackward ? pCNd->Len() : 0;
        pPos->nContent.Assign( pCNd, nTmpPos );

        if( &pPos->nNode.GetNode() != pCurrNd ||
            pPos->nContent.GetIndex() != nCurrCnt )
            // es gab eine Veraenderung
            return TRUE;

        // dann versuche mal den "Parent" dieser Section
        SwSection* pParent = pNd->GetSection().GetParent();
        pNd = pParent ? pParent->GetFmt()->GetSectionNode() : 0;
    } while( pNd );
    return FALSE;
}



BOOL SwCursor::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCrsrSaveState aSaveState( *this );
    return !(SwTableCursor*)*this &&
            (*fnWhichRegion)( *this, fnPosRegion, IsReadOnlyAvailable()  ) &&
            !IsSelOvr() &&
            ( GetPoint()->nNode.GetIndex() != pSavePos->nNode ||
                GetPoint()->nContent.GetIndex() != pSavePos->nCntnt );
}

BOOL SwCrsrShell::MoveRegion( SwWhichRegion fnWhichRegion, SwPosRegion fnPosRegion )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen, evt. Link callen
    BOOL bRet = !pTblCrsr && pCurCrsr->MoveRegion( fnWhichRegion, fnPosRegion );
    if( bRet )
        UpdateCrsr();
    return bRet;
}


BOOL SwCursor::GotoRegion( const String& rName )
{
    BOOL bRet = FALSE;
    const SwSectionFmts& rFmts = GetDoc()->GetSections();
    for( USHORT n = rFmts.Count(); n; )
    {
        const SwSectionFmt* pFmt = rFmts[ --n ];
        const SwNodeIndex* pIdx;
        const SwSection* pSect;
        if( 0 != ( pSect = pFmt->GetSection() ) &&
            pSect->GetName() == rName &&
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

BOOL SwCrsrShell::GotoRegion( const String& rName )
{
    SwCallLink aLk( *this );        // Crsr-Moves ueberwachen,
    BOOL bRet = !pTblCrsr && pCurCrsr->GotoRegion( rName );
    if( bRet )
        UpdateCrsr( SwCrsrShell::SCROLLWIN | SwCrsrShell::CHKRANGE |
                    SwCrsrShell::READONLY ); // und den akt. Updaten
    return bRet;
}



