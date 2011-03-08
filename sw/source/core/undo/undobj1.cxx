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


#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <txtflcnt.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <undobj.hxx>
#include <rolbck.hxx>       // fuer die Attribut History
#include <doc.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>
#include <swundo.hxx>           // fuer die UndoIds
#include <pam.hxx>
#include <ndtxt.hxx>
// OD 26.06.2003 #108784#
#include <dcontact.hxx>
#include <ndole.hxx>

// Inline Methode vom UndoIter
inline SwDoc& SwUndoIter::GetDoc() const { return *pAktPam->GetDoc(); }

//---------------------------------------------------------------------

SwUndoFlyBase::SwUndoFlyBase( SwFrmFmt* pFormat, SwUndoId nUndoId )
    : SwUndo( nUndoId ), pFrmFmt( pFormat )
{
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

    // OD 26.06.2003 #108784# - insert 'master' drawing object into drawing page
    if ( RES_DRAWFRMFMT == pFrmFmt->Which() )
    {
        SwDrawContact* pDrawContact =
            static_cast<SwDrawContact*>(pFrmFmt->FindContactObj());
        if ( pDrawContact )
        {
            pDrawContact->InsertMasterIntoDrawPage();
            // --> OD 2005-01-31 #i40845# - follow-up of #i35635#
            // move object to visible layer
            pDrawContact->MoveObjToVisibleLayer( pDrawContact->GetMaster() );
            // <--
        }
    }

    SwFmtAnchor aAnchor( (RndStdIds)nRndId );

    if (FLY_AT_PAGE == nRndId)
    {
        aAnchor.SetPageNum( (USHORT)nNdPgPos );
    }
    else
    {
        SwPosition aNewPos( *rUndoIter.pAktPam->GetPoint() );
        aNewPos.nNode = nNdPgPos;
        if ((FLY_AS_CHAR == nRndId) || (FLY_AT_CHAR == nRndId))
        {
            aNewPos.nContent.Assign( aNewPos.nNode.GetNode().GetCntntNode(),
                                    nCntPos );
        }
        aAnchor.SetAnchor( &aNewPos );
    }

    pFrmFmt->SetFmtAttr( aAnchor );     // Anker neu setzen

    if( RES_DRAWFRMFMT != pFrmFmt->Which() )
    {
        // Content holen und -Attribut neu setzen
        SwNodeIndex aIdx( pDoc->GetNodes() );
        RestoreSection( pDoc, &aIdx, SwFlyStartNode );
        pFrmFmt->SetFmtAttr( SwFmtCntnt( aIdx.GetNode().GetStartNode() ));
    }

    //JP 18.12.98: Bug 60505 - InCntntAttribut erst setzen, wenn der Inhalt
    //              vorhanden ist! Sonst wuerde das Layout den Fly vorher
    //              formatieren, aber keine Inhalt finden; so geschene bei
    //              Grafiken aus dem Internet
    if (FLY_AS_CHAR == nRndId)
    {
        // es muss mindestens das Attribut im TextNode stehen
        SwCntntNode* pCNd = aAnchor.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
        OSL_ENSURE( pCNd->IsTxtNode(), "no Text Node at position." );
        SwFmtFlyCnt aFmt( pFrmFmt );
        static_cast<SwTxtNode*>(pCNd)->InsertItem( aFmt, nCntPos, nCntPos );
    }

    pFrmFmt->MakeFrms();

    if( bShowSelFrm )
        rUndoIter.pSelFmt = pFrmFmt;

    if( GetHistory() )
        GetHistory()->Rollback( pDoc );

    switch( nRndId )
    {
    case FLY_AS_CHAR:
    case FLY_AT_CHAR:
        {
            const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
            nNdPgPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
            nCntPos = rAnchor.GetCntntAnchor()->nContent.GetIndex();
        }
        break;
    case FLY_AT_PARA:
    case FLY_AT_FLY:
        {
            const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
            nNdPgPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        }
        break;
    case FLY_AT_PAGE:
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
        OSL_ENSURE( rCntnt.GetCntntIdx(), "Fly ohne Inhalt" );

        SaveSection( pDoc, *rCntnt.GetCntntIdx() );
        ((SwFmtCntnt&)rCntnt).SetNewCntntIdx( (const SwNodeIndex*)0 );
    }
    // OD 02.07.2003 #108784# - remove 'master' drawing object from drawing page
    else if ( RES_DRAWFRMFMT == pFrmFmt->Which() )
    {
        SwDrawContact* pDrawContact =
            static_cast<SwDrawContact*>(pFrmFmt->FindContactObj());
        if ( pDrawContact )
        {
            pDrawContact->RemoveMasterFromDrawPage();
        }
    }

    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    const SwPosition* pPos = rAnchor.GetCntntAnchor();
    // die Positionen im Nodes-Array haben sich verschoben
    nRndId = static_cast<USHORT>(rAnchor.GetAnchorId());
    if (FLY_AS_CHAR == nRndId)
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
        SwTxtNode *pTxtNd = pDoc->GetNodes()[ pPos->nNode ]->GetTxtNode();
        OSL_ENSURE( pTxtNd, "Kein Textnode gefunden" );
        SwTxtFlyCnt* const pAttr = static_cast<SwTxtFlyCnt*>(
            pTxtNd->GetTxtAttrForCharAt( nCntPos, RES_TXTATR_FLYCNT ) );
        // Attribut steht noch im TextNode, loeschen
        if( pAttr && pAttr->GetFlyCnt().GetFrmFmt() == pFrmFmt )
        {
            // Pointer auf 0, nicht loeschen
            ((SwFmtFlyCnt&)pAttr->GetFlyCnt()).SetFlyFmt();
            SwIndex aIdx( pPos->nContent );
            pTxtNd->EraseText( aIdx, 1 );
        }
    }
    else if (FLY_AT_CHAR == nRndId)
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
    }
    else if ((FLY_AT_PARA == nRndId) || (FLY_AT_FLY == nRndId))
    {
        nNdPgPos = pPos->nNode.GetIndex();
    }
    else
    {
        nNdPgPos = rAnchor.GetPageNum();
    }

    pFrmFmt->ResetFmtAttr( RES_ANCHOR );        // Anchor loeschen


    // aus dem Array austragen
    SwSpzFrmFmts& rFlyFmts = *(SwSpzFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.Remove( rFlyFmts.GetPos( pFrmFmt ));
}

// ----- Undo-InsertFly ------

SwUndoInsLayFmt::SwUndoInsLayFmt( SwFrmFmt* pFormat, ULONG nNodeIdx, xub_StrLen nCntIdx )
    : SwUndoFlyBase( pFormat, RES_DRAWFRMFMT == pFormat->Which() ?
                                            UNDO_INSDRAWFMT : UNDO_INSLAYFMT ),
    mnCrsrSaveIndexPara( nNodeIdx ), mnCrsrSaveIndexPos( nCntIdx )
{
    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    nRndId = static_cast<USHORT>(rAnchor.GetAnchorId());
    bDelFmt = FALSE;
    switch( nRndId )
    {
    case FLY_AT_PAGE:
        nNdPgPos = rAnchor.GetPageNum();
        break;
    case FLY_AT_PARA:
    case FLY_AT_FLY:
        nNdPgPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
        break;
    case FLY_AS_CHAR:
    case FLY_AT_CHAR:
        {
            const SwPosition* pPos = rAnchor.GetCntntAnchor();
            nCntPos = pPos->nContent.GetIndex();
            nNdPgPos = pPos->nNode.GetIndex();
        }
        break;
    default:
        OSL_ENSURE( FALSE, "Was denn fuer ein FlyFrame?" );
    }
}

SwUndoInsLayFmt::~SwUndoInsLayFmt()
{
}

void SwUndoInsLayFmt::Undo( SwUndoIter& rUndoIter )
{
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // kein Inhalt
    {
        bool bRemoveIdx = true;
        if( mnCrsrSaveIndexPara > 0 )
        {
            SwTxtNode *pNode = rUndoIter.GetDoc().GetNodes()[mnCrsrSaveIndexPara]->GetTxtNode();
            if( pNode )
            {
                SwNodeIndex aIdx( rUndoIter.GetDoc().GetNodes(), rCntnt.GetCntntIdx()->GetIndex() );
                SwNodeIndex aEndIdx( rUndoIter.GetDoc().GetNodes(), aIdx.GetNode().EndOfSectionIndex() );
                SwIndex aIndex( pNode, mnCrsrSaveIndexPos );
                SwPosition aPos( *pNode, aIndex );
                rUndoIter.GetDoc().CorrAbs( aIdx, aEndIdx, aPos, TRUE );
                bRemoveIdx = false;
            }
        }
        if( bRemoveIdx )
            RemoveIdxFromSection( rUndoIter.GetDoc(),
                                rCntnt.GetCntntIdx()->GetIndex() );
    }
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
    if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
        (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
        (FLY_AS_CHAR == aAnchor.GetAnchorId()))
    {
        SwPosition aPos( *rUndoIter.pAktPam->GetPoint() );
        if (FLY_AT_PARA == aAnchor.GetAnchorId())
        {
            aPos.nContent.Assign( 0, 0 );
        }
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
    else if (FLY_AT_PAGE == aAnchor.GetAnchorId())
    {
        aAnchor.SetPageNum( pDoc->GetRootFrm()->GetCurrPage(
                                        rUndoIter.pAktPam ));
    }
    else {
        OSL_ENSURE( FALSE, "was fuer ein Anker ist es denn nun?" );
    }

    SwFrmFmt* pFlyFmt = pDoc->CopyLayoutFmt( *pFrmFmt, aAnchor, true, true );
    rUndoIter.pSelFmt = pFlyFmt;

    rUndoIter.pLastUndoObj = this;
}

// #111827#
String SwUndoInsLayFmt::GetComment() const
{
    String aResult;

    if (! pComment)
    {
        /*
          If frame format is present and has an SdrObject use the undo
          comment of the SdrObject. Otherwise use the default comment.
        */

        bool bDone = false;
        if (pFrmFmt)
        {
            const SdrObject * pSdrObj = pFrmFmt->FindSdrObject();
            if ( pSdrObj )
            {
                aResult = SdrUndoNewObj::GetComment( *pSdrObj );
                bDone = true;
            }
        }

        if (! bDone)
            aResult = SwUndo::GetComment();
    }
    else
        aResult = *pComment;

    return aResult;
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
        {
            SetId( UNDO_DELETE );

        }
    }
}

SwRewriter SwUndoDelLayFmt::GetRewriter() const
{
    SwRewriter aRewriter;

    SwDoc * pDoc = pFrmFmt->GetDoc();

    if (pDoc)
    {
        SwNodeIndex* pIdx = GetMvSttIdx();
        if( 1 == GetMvNodeCnt() && pIdx)
        {
            SwNode * pNd = (*pDoc->GetUndoNds())[ *pIdx ];

            if ( pNd->IsNoTxtNode() && pNd->IsOLENode())
            {
                SwOLENode * pOLENd = pNd->GetOLENode();

                aRewriter.AddRule(UNDO_ARG1, pOLENd->GetDescription());
            }
        }
    }

    return aRewriter;
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
    pOldFmt( (SwFrmFmt*)rFlyFmt.DerivedFrom() ), pNewFmt( &rNewFrmFmt ),
    pItemSet( new SfxItemSet( *rFlyFmt.GetAttrSet().GetPool(),
                                rFlyFmt.GetAttrSet().GetRanges() )),
    nOldNode( 0 ), nNewNode( 0 ),
    nOldCntnt( 0 ), nNewCntnt( 0 ),
    nOldAnchorTyp( 0 ), nNewAnchorTyp( 0 ), bAnchorChgd( FALSE )
{
}

SwRewriter SwUndoSetFlyFmt::GetRewriter() const
{
    SwRewriter aRewriter;

    if (pNewFmt)
        aRewriter.AddRule(UNDO_ARG1, pNewFmt->GetName());

    return aRewriter;
}


SwUndoSetFlyFmt::~SwUndoSetFlyFmt()
{
    delete pItemSet;
}

void SwUndoSetFlyFmt::GetAnchor( SwFmtAnchor& rAnchor,
                                ULONG nNode, xub_StrLen nCntnt )
{
    RndStdIds nAnchorTyp = rAnchor.GetAnchorId();
    if (FLY_AT_PAGE != nAnchorTyp)
    {
        SwNode* pNd = pFrmFmt->GetDoc()->GetNodes()[ nNode ];

        if( FLY_AT_FLY == nAnchorTyp
                ? ( !pNd->IsStartNode() || SwFlyStartNode !=
                    ((SwStartNode*)pNd)->GetStartNodeType() )
                : !pNd->IsTxtNode() )
        {
            pNd = 0;    // invalid position
        }
        else
        {
            SwPosition aPos( *pNd );
            if ((FLY_AS_CHAR == nAnchorTyp) ||
                (FLY_AT_CHAR == nAnchorTyp))
            {
                if ( nCntnt > static_cast<SwTxtNode*>(pNd)->GetTxt().Len() )
                {
                    pNd = 0;    // invalid position
                }
                else
                {
                    aPos.nContent.Assign(static_cast<SwTxtNode*>(pNd), nCntnt);
                }
            }
            if ( pNd )
            {
                rAnchor.SetAnchor( &aPos );
            }
        }

        if( !pNd )
        {
            // ungueltige Position - setze auf 1. Seite
            rAnchor.SetType( FLY_AT_PAGE );
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
                pFrmFmt->ResetFmtAttr( pItemSet->GetWhichByPos(
                                        aIter.GetCurPos() ));
            else
                pFrmFmt->SetFmtAttr( *pItem );

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }

        if( bAnchorChgd )
        {
            const SwFmtAnchor& rOldAnch = pFrmFmt->GetAnchor();
            if (FLY_AS_CHAR == rOldAnch.GetAnchorId())
            {
                // Bei InCntnt's wird es spannend: Das TxtAttribut muss
                // vernichtet werden. Leider reisst dies neben den Frms
                // auch noch das Format mit in sein Grab. Um dass zu
                // unterbinden loesen wir vorher die Verbindung zwischen
                // Attribut und Format.
                const SwPosition *pPos = rOldAnch.GetCntntAnchor();
                SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                const xub_StrLen nIdx = pPos->nContent.GetIndex();
                SwTxtAttr * pHnt = pTxtNode->GetTxtAttrForCharAt(
                        nIdx, RES_TXTATR_FLYCNT );
                OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                            "Missing FlyInCnt-Hint." );
                OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pFrmFmt,
                            "Wrong TxtFlyCnt-Hint." );
                const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt();

                // Die Verbindung ist geloest, jetzt muss noch das Attribut
                // vernichtet werden.
                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }

            // Anker umsetzen
            SwFmtAnchor aNewAnchor( (RndStdIds) nOldAnchorTyp );
            GetAnchor( aNewAnchor, nOldNode, nOldCntnt );
            pFrmFmt->SetFmtAttr( aNewAnchor );

            if (FLY_AS_CHAR == aNewAnchor.GetAnchorId())
            {
                SwPosition* pPos = (SwPosition*)aNewAnchor.GetCntntAnchor();
                SwFmtFlyCnt aFmt( pFrmFmt );
                pPos->nNode.GetNode().GetTxtNode()->InsertItem( aFmt,
                    nOldCntnt, 0 );
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
            OSL_ENSURE( !bAnchorChgd, "mehrfacher Ankerwechsel nicht erlaubt!" );

            bAnchorChgd = TRUE;

            const SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
            switch( nOldAnchorTyp = static_cast<USHORT>(pAnchor->GetAnchorId()) )
            {
            case FLY_AS_CHAR:
            case FLY_AT_CHAR:
                nOldCntnt = pAnchor->GetCntntAnchor()->nContent.GetIndex();
            case FLY_AT_PARA:
            case FLY_AT_FLY:
                nOldNode = pAnchor->GetCntntAnchor()->nNode.GetIndex();
                break;

            default:
                nOldCntnt = pAnchor->GetPageNum();
            }

            pAnchor = (SwFmtAnchor*)&pFrmFmt->GetAnchor();
            switch( nNewAnchorTyp = static_cast<USHORT>(pAnchor->GetAnchorId()) )
            {
            case FLY_AS_CHAR:
            case FLY_AT_CHAR:
                nNewCntnt = pAnchor->GetCntntAnchor()->nContent.GetIndex();
            case FLY_AT_PARA:
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
