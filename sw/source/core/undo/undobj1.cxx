/*************************************************************************
 *
 *  $RCSfile: undobj1.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:38:30 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FLYFRM_HXX //autogen
#include <flyfrm.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>       // fuer die Attribut History
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _ROOTFRM_HXX
#include <rootfrm.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>           // fuer die UndoIds
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif

// Inline Methode vom UndoIter
inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

//---------------------------------------------------------------------

SwUndoFlyBase::SwUndoFlyBase( SwFrmFmt* pFormat, USHORT nUndoId )
    : SwUndo( nUndoId ), pFrmFmt( pFormat )
{
#ifdef COMPACT
    pFormat->GetDoc()->DelUndoGroups();
#endif
}

SwUndoFlyBase::~SwUndoFlyBase()
{
    if( bDelFmt )       // loeschen waehrend eines Undo's ??
        delete pFrmFmt;
}

void SwUndoFlyBase::InsFly( SwUndoIter& rUndoIter, BOOL bShowSelFrm )
{
    SwDoc* pDoc = &rUndoIter.GetDoc();

    // ins Array wieder eintragen
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Insert( pFrmFmt, rFlyFmts.Count() );
    SwFmtAnchor aAnchor( (RndStdIds)nRndId );

    if( FLY_PAGE == nRndId )
        aAnchor.SetPageNum( (USHORT)nNdPgPos );
    else
    {
        SwPosition aNewPos( *rUndoIter.pAktPam->GetPoint() );
        aNewPos.nNode = nNdPgPos;
        if( FLY_IN_CNTNT == nRndId || FLY_AUTO_CNTNT == nRndId )
            aNewPos.nContent.Assign( aNewPos.nNode.GetNode().GetCntntNode(),
                                    nCntPos );
        aAnchor.SetAnchor( &aNewPos );
    }

    pFrmFmt->SetAttr( aAnchor );        // Anker neu setzen

    if( RES_DRAWFRMFMT != pFrmFmt->Which() )
    {
        // Content holen und -Attribut neu setzen
        SwNodeIndex aIdx( pDoc->GetNodes() );
        RestoreSection( pDoc, &aIdx, SwFlyStartNode );
        pFrmFmt->SetAttr( SwFmtCntnt( aIdx.GetNode().GetStartNode() ));
    }

    //JP 18.12.98: Bug 60505 - InCntntAttribut erst setzen, wenn der Inhalt
    //              vorhanden ist! Sonst wuerde das Layout den Fly vorher
    //              formatieren, aber keine Inhalt finden; so geschene bei
    //              Grafiken aus dem Internet
    if( FLY_IN_CNTNT == nRndId )
    {
        // es muss mindestens das Attribut im TextNode stehen
        SwCntntNode* pCNd = aAnchor.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
        ASSERT( pCNd->IsTxtNode(), "Kein Textnode an dieser Position" );
        ((SwTxtNode*)pCNd)->Insert( SwFmtFlyCnt(
                                (SwFlyFrmFmt*)pFrmFmt ), nCntPos, nCntPos );
    }

    pFrmFmt->MakeFrms();

    if( bShowSelFrm )
        rUndoIter.pSelFmt = pFrmFmt;

    if( GetHistory() )
        GetHistory()->Rollback( pDoc );

    switch( nRndId )
    {
    case FLY_IN_CNTNT:
    case FLY_AUTO_CNTNT:
        {
            const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
            nNdPgPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
            nCntPos = rAnchor.GetCntntAnchor()->nContent.GetIndex();
        }
        break;
    case FLY_AT_CNTNT:
    case FLY_AT_FLY:
        {
            const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
            nNdPgPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        }
        break;
    case FLY_PAGE:
        break;
    }
    bDelFmt =  FALSE;
}

void SwUndoFlyBase::DelFly( SwDoc* pDoc )
{
    bDelFmt = TRUE;                     // im DTOR das Format loeschen
    pFrmFmt->DelFrms();                 // Frms vernichten.

    // alle Uno-Objecte sollten sich jetzt abmelden
    {
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFrmFmt );
        pFrmFmt->Modify( &aMsgHint, &aMsgHint );
    }

    if ( RES_DRAWFRMFMT != pFrmFmt->Which() )
    {
        // gibt es ueberhaupt Inhalt, dann sicher diesen
        const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
        ASSERT( rCntnt.GetCntntIdx(), "Fly ohne Inhalt" );

        SaveSection( pDoc, *rCntnt.GetCntntIdx() );
        ((SwFmtCntnt&)rCntnt).SetNewCntntIdx( (const SwNodeIndex*)0 );
    }

    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    const SwPosition* pPos = rAnchor.GetCntntAnchor();
    // die Positionen im Nodes-Array haben sich verschoben
    if( FLY_IN_CNTNT == ( nRndId = rAnchor.GetAnchorId() ) )
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
        SwTxtNode *pTxtNd = pDoc->GetNodes()[ pPos->nNode ]->GetTxtNode();
        ASSERT( pTxtNd, "Kein Textnode gefunden" );
        SwTxtFlyCnt* pAttr = (SwTxtFlyCnt*)pTxtNd->GetTxtAttr( nCntPos );
        // Attribut steht noch im TextNode, loeschen
        if( pAttr && pAttr->GetFlyCnt().GetFrmFmt() == pFrmFmt )
        {
            // Pointer auf 0, nicht loeschen
            ((SwFmtFlyCnt&)pAttr->GetFlyCnt()).SetFlyFmt();
            SwIndex aIdx( pPos->nContent );
            pTxtNd->Erase( aIdx, 1 );
        }
    }
    else if( FLY_AUTO_CNTNT == nRndId )
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
    }
    else if( FLY_AT_CNTNT == nRndId || FLY_AT_FLY == nRndId )
        nNdPgPos = pPos->nNode.GetIndex();
    else
        nNdPgPos = rAnchor.GetPageNum();

    pFrmFmt->ResetAttr( RES_ANCHOR );       // Anchor loeschen


    // aus dem Array austragen
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFrmFmt ));
}

// ----- Undo-InsertFly ------

SwUndoInsLayFmt::SwUndoInsLayFmt( SwFrmFmt* pFormat )
    : SwUndoFlyBase( pFormat,
            RES_DRAWFRMFMT == pFormat->Which() ? UNDO_INSDRAWFMT : UNDO_INSLAYFMT )
{
    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    nRndId = rAnchor.GetAnchorId();
    bDelFmt = FALSE;
    switch( nRndId )
    {
    case FLY_PAGE:
        nNdPgPos = rAnchor.GetPageNum();
        break;
    case FLY_AT_CNTNT:
    case FLY_AT_FLY:
        nNdPgPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        break;
    case FLY_IN_CNTNT:
    case FLY_AUTO_CNTNT:
        {
            const SwPosition* pPos = rAnchor.GetCntntAnchor();
            nCntPos = pPos->nContent.GetIndex();
            nNdPgPos = pPos->nNode.GetIndex();
        }
        break;
    default:
        ASSERT( FALSE, "Was denn fuer ein FlyFrame?" );
    }
}

void SwUndoInsLayFmt::Undo( SwUndoIter& rUndoIter )
{
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // kein Inhalt
        RemoveIdxFromSection( rUndoIter.GetDoc(),
                                rCntnt.GetCntntIdx()->GetIndex() );

    DelFly( &rUndoIter.GetDoc() );
}

void SwUndoInsLayFmt::Redo( SwUndoIter& rUndoIter )
{
    rUndoIter.pLastUndoObj = 0;
    InsFly( rUndoIter );
}

void SwUndoInsLayFmt::Repeat( SwUndoIter& rUndoIter )
{
    if( UNDO_INSLAYFMT == rUndoIter.GetLastUndoId() &&
        pFrmFmt == ((SwUndoInsLayFmt*)rUndoIter.pLastUndoObj)->pFrmFmt )
        return;

    SwDoc* pDoc = &rUndoIter.GetDoc();
    // erfrage und setze den Anker neu
    SwFmtAnchor aAnchor( pFrmFmt->GetAnchor() );
    if( FLY_AT_CNTNT == aAnchor.GetAnchorId() ||
        FLY_AUTO_CNTNT == aAnchor.GetAnchorId() ||
        FLY_IN_CNTNT == aAnchor.GetAnchorId() )
    {
        SwPosition aPos( *rUndoIter.pAktPam->GetPoint() );
        if( FLY_AT_CNTNT == aAnchor.GetAnchorId() )
            aPos.nContent.Assign( 0, 0 );
        aAnchor.SetAnchor( &aPos );
    }
    else if( FLY_AT_FLY == aAnchor.GetAnchorId() )
    {
        const SwStartNode* pSttNd = rUndoIter.pAktPam->GetNode()->FindFlyStartNode();
        if( pSttNd )
        {
            SwPosition aPos( *pSttNd );
            aAnchor.SetAnchor( &aPos );
        }
        else
        {
            rUndoIter.pLastUndoObj = this;
            return ;
        }
    }
    else if( FLY_PAGE == aAnchor.GetAnchorId() )
    {
        aAnchor.SetPageNum( pDoc->GetRootFrm()->GetCurrPage(
                                        rUndoIter.pAktPam ));
    }
    else
        ASSERT( FALSE, "was fuer ein Anker ist es denn nun?" );

    SwFrmFmt* pFlyFmt = pDoc->CopyLayoutFmt( *pFrmFmt, aAnchor );
    rUndoIter.pSelFmt = pFlyFmt;

    rUndoIter.pLastUndoObj = this;
}

// ----- Undo-DeleteFly ------

SwUndoDelLayFmt::SwUndoDelLayFmt( SwFrmFmt* pFormat )
    : SwUndoFlyBase( pFormat, UNDO_DELLAYFMT ), bShowSelFrm( TRUE )
{
    SwDoc* pDoc = pFormat->GetDoc();
    DelFly( pDoc );

    SwNodeIndex* pIdx = GetMvSttIdx();
    SwNode* pNd;
    if( 1 == GetMvNodeCnt() && pIdx &&
        ( pNd = (*pDoc->GetUndoNds())[ *pIdx ] )->IsNoTxtNode() )
    {
        // dann setze eine andere Undo-ID; Grafik oder OLE
        if( pNd->IsGrfNode() )
            SetId( UNDO_DELGRF );
        else if( pNd->IsOLENode() )
            SetId( UNDO_DELOLE );
    }
}

void SwUndoDelLayFmt::Undo( SwUndoIter& rUndoIter )
{
    InsFly( rUndoIter, bShowSelFrm );
}

void SwUndoDelLayFmt::Redo( SwUndoIter& rUndoIter )
{
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // kein Inhalt
        RemoveIdxFromSection( rUndoIter.GetDoc(),
                                rCntnt.GetCntntIdx()->GetIndex() );

    DelFly( &rUndoIter.GetDoc() );
}

void SwUndoDelLayFmt::Redo()
{
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // kein Inhalt
        RemoveIdxFromSection( *pFrmFmt->GetDoc(),
                                rCntnt.GetCntntIdx()->GetIndex() );

    DelFly( pFrmFmt->GetDoc() );
}

/*  */

SwUndoSetFlyFmt::SwUndoSetFlyFmt( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFrmFmt )
    : SwUndo( UNDO_SETFLYFRMFMT ), SwClient( &rFlyFmt ), pFrmFmt( &rFlyFmt ),
    pNewFmt( &rNewFrmFmt ), pOldFmt( (SwFrmFmt*)rFlyFmt.DerivedFrom() ),
    pItemSet( new SfxItemSet( *rFlyFmt.GetAttrSet().GetPool(),
                                rFlyFmt.GetAttrSet().GetRanges() )),
    nOldNode( 0 ), nOldCntnt( 0 ), nOldAnchorTyp( 0 ),
    nNewNode( 0 ), nNewCntnt( 0 ), nNewAnchorTyp( 0 ), bAnchorChgd( FALSE )
{
}

SwUndoSetFlyFmt::~SwUndoSetFlyFmt()
{
    delete pItemSet;
}

void SwUndoSetFlyFmt::GetAnchor( SwFmtAnchor& rAnchor,
                                ULONG nNode, xub_StrLen nCntnt )
{
    RndStdIds nAnchorTyp = rAnchor.GetAnchorId();
    if( FLY_PAGE != nAnchorTyp )
    {
        SwNode* pNd = pFrmFmt->GetDoc()->GetNodes()[ nNode ];

        if( FLY_AT_FLY == nAnchorTyp
                ? ( !pNd->IsStartNode() || SwFlyStartNode !=
                    ((SwStartNode*)pNd)->GetStartNodeType() )
                : !pNd->IsTxtNode() )
            pNd = 0;                // ungueltige Position
        else
        {
            SwPosition aPos( *pNd );
            if( FLY_IN_CNTNT == nAnchorTyp ||
                FLY_AUTO_CNTNT == nAnchorTyp )
            {
                if( nCntnt > ((SwTxtNode*)pNd)->GetTxt().Len() )
                    pNd = 0;        // ungueltige Position
                else
                    aPos.nContent.Assign( (SwTxtNode*)pNd, nCntnt );
            }
            if( pNd )
                rAnchor.SetAnchor( &aPos );
        }

        if( !pNd )
        {
            // ungueltige Position - setze auf 1. Seite
            rAnchor.SetType( FLY_PAGE );
            rAnchor.SetPageNum( 1 );
        }
    }
    else
        rAnchor.SetPageNum( nCntnt );
}

void SwUndoSetFlyFmt::Undo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();

    // ist das neue Format noch vorhanden ??
    if( USHRT_MAX != rDoc.GetFrmFmts()->GetPos( (const SwFrmFmtPtr)pOldFmt ) )
    {
        if( bAnchorChgd )
            pFrmFmt->DelFrms();

        if( pFrmFmt->DerivedFrom() != pOldFmt )
            pFrmFmt->SetDerivedFrom( pOldFmt );

        SfxItemIter aIter( *pItemSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( pItem )
        {
            if( IsInvalidItem( pItem ))
                pFrmFmt->ResetAttr( pItemSet->GetWhichByPos(
                                        aIter.GetCurPos() ));
            else
                pFrmFmt->SetAttr( *pItem );

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }

        if( bAnchorChgd )
        {
            const SwFmtAnchor& rOldAnch = pFrmFmt->GetAnchor();
            if( FLY_IN_CNTNT == rOldAnch.GetAnchorId() )
            {
                // Bei InCntnt's wird es spannend: Das TxtAttribut muss
                // vernichtet werden. Leider reisst dies neben den Frms
                // auch noch das Format mit in sein Grab. Um dass zu
                // unterbinden loesen wir vorher die Verbindung zwischen
                // Attribut und Format.
                const SwPosition *pPos = rOldAnch.GetCntntAnchor();
                SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                ASSERT( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                const xub_StrLen nIdx = pPos->nContent.GetIndex();
                SwTxtAttr * pHnt = pTxtNode->GetTxtAttr( nIdx, RES_TXTATR_FLYCNT );
#ifndef PRODUCT
                ASSERT( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                            "Missing FlyInCnt-Hint." );
                ASSERT( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pFrmFmt,
                            "Wrong TxtFlyCnt-Hint." );
#endif
                ((SwFmtFlyCnt&)pHnt->GetFlyCnt()).SetFlyFmt();

                // Die Verbindung ist geloest, jetzt muss noch das Attribut
                // vernichtet werden.
                pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }

            // Anker umsetzen
            SwFmtAnchor aNewAnchor( (RndStdIds) nOldAnchorTyp );
            GetAnchor( aNewAnchor, nOldNode, nOldCntnt );
            pFrmFmt->SetAttr( aNewAnchor );

            if( FLY_IN_CNTNT == aNewAnchor.GetAnchorId() )
            {
                SwPosition* pPos = (SwPosition*)aNewAnchor.GetCntntAnchor();
                pPos->nNode.GetNode().GetTxtNode()->Insert(
                        SwFmtFlyCnt( (SwFlyFrmFmt*)pFrmFmt ), nOldCntnt, 0 );
            }

            pFrmFmt->MakeFrms();
        }
        rIter.pSelFmt = pFrmFmt;
    }
}

void SwUndoSetFlyFmt::Redo( SwUndoIter& rIter )
{
    SwDoc& rDoc = rIter.GetDoc();

    // ist das neue Format noch vorhanden ??
    if( USHRT_MAX != rDoc.GetFrmFmts()->GetPos( (const SwFrmFmtPtr)pNewFmt ) )
    {

        if( bAnchorChgd )
        {
            SwFmtAnchor aNewAnchor( (RndStdIds) nNewAnchorTyp );
            GetAnchor( aNewAnchor, nNewNode, nNewCntnt );
            SfxItemSet aSet( rDoc.GetAttrPool(), aFrmFmtSetRange );
            aSet.Put( aNewAnchor );
            rDoc.SetFrmFmtToFly( *pFrmFmt, *pNewFmt, &aSet );
        }
        else
            rDoc.SetFrmFmtToFly( *pFrmFmt, *pNewFmt, 0 );

        rIter.pSelFmt = pFrmFmt;
    }
}

void SwUndoSetFlyFmt::PutAttr( USHORT nWhich, const SfxPoolItem* pItem )
{
    if( pItem && pItem != GetDfltAttr( nWhich ) )
    {
        // Sonderbehandlung fuer den Anchor
        if( RES_ANCHOR == nWhich )
        {
            // nur den 1. Ankerwechsel vermerken
            ASSERT( !bAnchorChgd, "mehrfacher Ankerwechsel nicht erlaubt!" );

            bAnchorChgd = TRUE;

            const SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
            switch( nOldAnchorTyp = pAnchor->GetAnchorId() )
            {
            case FLY_IN_CNTNT:
            case FLY_AUTO_CNTNT:
                nOldCntnt = pAnchor->GetCntntAnchor()->nContent.GetIndex();
            case FLY_AT_CNTNT:
            case FLY_AT_FLY:
                nOldNode = pAnchor->GetCntntAnchor()->nNode.GetIndex();
                break;

            default:
                nOldCntnt = pAnchor->GetPageNum();
            }

            pAnchor = (SwFmtAnchor*)&pFrmFmt->GetAnchor();
            switch( nNewAnchorTyp = pAnchor->GetAnchorId() )
            {
            case FLY_IN_CNTNT:
            case FLY_AUTO_CNTNT:
                nNewCntnt = pAnchor->GetCntntAnchor()->nContent.GetIndex();
            case FLY_AT_CNTNT:
            case FLY_AT_FLY:
                nNewNode = pAnchor->GetCntntAnchor()->nNode.GetIndex();
                break;

            default:
                nNewCntnt = pAnchor->GetPageNum();
            }
        }
        else
            pItemSet->Put( *pItem );
    }
    else
        pItemSet->InvalidateItem( nWhich );
}

void SwUndoSetFlyFmt::Modify( SfxPoolItem* pOld, SfxPoolItem* )
{
    if( pOld )
    {
        USHORT nWhich = pOld->Which();

        if( nWhich < POOLATTR_END )
            PutAttr( nWhich, pOld );
        else if( RES_ATTRSET_CHG == nWhich )
        {
            SfxItemIter aIter( *((SwAttrSetChg*)pOld)->GetChgSet() );
            const SfxPoolItem* pItem = aIter.GetCurItem();
            while( pItem )
            {
                PutAttr( pItem->Which(), pItem );
                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
        }
    }
}

