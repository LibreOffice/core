/*************************************************************************
 *
 *  $RCSfile: undobj1.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

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

/*************************************************************************

   $Log: not supported by cvs2svn $
   Revision 1.55  2000/09/18 16:04:29  willem.vandorp
   OpenOffice header added.

   Revision 1.54  2000/05/09 10:04:08  jp
   Changes for Unicode

   Revision 1.53  1999/11/29 17:24:12  jp
   some changes for the compat. attribut

   Revision 1.52  1999/11/10 09:30:30  jp
   DelFly: call to uno before change any attributes

   Revision 1.51  1998/12/18 14:40:34  JP
   Bug #60505#: InsFly - CntFly Attr erst setzen, wenn der Inhalt vorhanden ist


      Rev 1.50   18 Dec 1998 15:40:34   JP
   Bug #60505#: InsFly - CntFly Attr erst setzen, wenn der Inhalt vorhanden ist

      Rev 1.49   12 May 1998 15:48:18   JP
   rund um Flys/DrawObjs im Doc umgestellt/optimiert

      Rev 1.48   24 Apr 1998 17:54:12   JP
   neues UndoObject fuers Setzen von Rahmenvorlagen

      Rev 1.47   02 Apr 1998 15:12:50   JP
   Redo: Undo-Flag wird schon von der EditShell abgeschaltet

      Rev 1.46   27 Feb 1998 09:23:02   JP
   Vorm loeschen der FlyFrms aus dem SpzArray den UNO Objecten bescheid geben

      Rev 1.45   27 Jan 1998 22:36:14   JP
   GetNumDepend durch GetDepends ersetzt

      Rev 1.44   19 Dec 1997 12:15:34   JP
   MSG/NOTE entfernt

      Rev 1.43   03 Nov 1997 13:06:08   MA
   precomp entfernt

      Rev 1.42   09 Oct 1997 15:45:30   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.41   10 Sep 1997 14:33:16   JP
   Bug #43672#: InsFly::Repeat - ggfs. die vom CopyLayoutFmt angelegten FlyFrms wieder entfernen

      Rev 1.40   03 Sep 1997 10:29:32   JP
   zusaetzliches include von docary

      Rev 1.39   18 Aug 1997 10:34:52   OS
   includes

      Rev 1.38   15 Aug 1997 12:37:40   OS
   charatr/frmatr/txtatr aufgeteilt

      Rev 1.37   11 Jun 1997 10:43:44   JP
   pure virtual Repeat wurde zur virtual Methode, Segment Pragma entfernt

      Rev 1.36   15 Apr 1997 14:51:52   AMA
   New: Rahmengebundene Rahmen und auto.positionierte Rahmen

      Rev 1.35   31 Oct 1996 16:19:00   MA
   opt: Which nicht mehr virtuell, kann ISA ersetzen

      Rev 1.34   29 Oct 1996 14:53:46   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.33   29 Aug 1996 10:09:28   JP
   beim StartNode muss jetzt ein spz. Sectiontype gesetzt werden

      Rev 1.32   29 May 1996 15:33:42   JP
   Bug #28123#: Draw-Objecte nur selektieren wenns vorgegeben wurde

      Rev 1.31   17 May 1996 15:04:42   AMA
   Fix: Undo von zeichengebundenen Zeichenobjekten

      Rev 1.30   04 Dec 1995 16:37:12   MA
   chg: eigener Undotext fuer einfuegen Zeichenobjekt

      Rev 1.29   24 Nov 1995 17:13:56   OM
   PCH->PRECOMPILED

      Rev 1.28   26 Sep 1995 18:53:20   JP
   UndoDelFly: unterscheide die Inhaltsform (Rahmen/OLE/GRF)

      Rev 1.27   08 Sep 1995 19:11:40   ER
   _SFXUNDO_HXX --> _UNDO_HXX

      Rev 1.26   22 Jun 1995 19:33:32   JP
   virt. Methode GetUndoRange vernichtet, Objecte rufen jetzt die Basis-Klasse

      Rev 1.25   31 Mar 1995 18:54:10   MA
   include undo.hxx

      Rev 1.24   04 Mar 1995 13:30:24   MA
   unnoetiges SEXPORT entfernt.

      Rev 1.23   23 Feb 1995 23:02:26   ER
   sexport

      Rev 1.22   23 Feb 1995 17:53:24   MA
   Rudimentaer Undo/Redo fuer Zeichenobjekte.

      Rev 1.21   08 Feb 1995 23:53:02   ER
   undo.hxx -> swundo.hxx wegen solar undo.hxx

      Rev 1.20   11 Jan 1995 19:06:52   MA
   CurrShell statt OleShell.

      Rev 1.19   15 Dec 1994 20:47:46   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.18   28 Oct 1994 19:19:28   MA
   Reimport frmatr.

      Rev 1.17   25 Oct 1994 14:50:34   MA
   PreHdr.

      Rev 1.16   07 Sep 1994 17:29:44   SWG
   Umstellung Attribute

      Rev 1.15   25 Aug 1994 18:05:52   JP
   Umstellung Attribute (von SwHint -> SfxPoolItem)

      Rev 1.14   04 Aug 1994 13:29:54   SWG
   swg32: SED Size to SSize, LSize to Size etc.

      Rev 1.13   17 Feb 1994 08:31:08   MI
   SEG_FUNCDEFS ausgefuellt

      Rev 1.12   16 Feb 1994 13:19:16   MI
   Pragmas zurechtgerueckt

*************************************************************************/

