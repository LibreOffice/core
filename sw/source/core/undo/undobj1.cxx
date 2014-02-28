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

#include <svl/itemiter.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <txtflcnt.hxx>
#include <frmfmt.hxx>
#include <flyfrm.hxx>
#include <UndoCore.hxx>
#include <UndoDraw.hxx>
#include <rolbck.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
// OD 26.06.2003 #108784#
#include <dcontact.hxx>
#include <ndole.hxx>

SwUndoFlyBase::SwUndoFlyBase( SwFrmFmt* pFormat, SwUndoId nUndoId )
    : SwUndo( nUndoId ), pFrmFmt( pFormat )
{
}

SwUndoFlyBase::~SwUndoFlyBase()
{
    if( bDelFmt )       // delete during an Undo?
        delete pFrmFmt;
}

void SwUndoFlyBase::InsFly(::sw::UndoRedoContext & rContext, bool bShowSelFrm)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    // add again into array
    SwFrmFmts& rFlyFmts = *(SwFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.push_back( pFrmFmt );

    // OD 26.06.2003 #108784# - insert 'master' drawing object into drawing page
    if ( RES_DRAWFRMFMT == pFrmFmt->Which() )
    {
        SwDrawContact* pDrawContact =
            static_cast<SwDrawContact*>(pFrmFmt->FindContactObj());
        if ( pDrawContact )
        {
            pDrawContact->InsertMasterIntoDrawPage();
            // #i40845# - follow-up of #i35635#
            // move object to visible layer
            pDrawContact->MoveObjToVisibleLayer( pDrawContact->GetMaster() );
        }
    }

    SwFmtAnchor aAnchor( (RndStdIds)nRndId );

    if (FLY_AT_PAGE == nRndId)
    {
        aAnchor.SetPageNum( (sal_uInt16)nNdPgPos );
    }
    else
    {
        SwPosition aNewPos(pDoc->GetNodes().GetEndOfContent());
        aNewPos.nNode = nNdPgPos;
        if ((FLY_AS_CHAR == nRndId) || (FLY_AT_CHAR == nRndId))
        {
            aNewPos.nContent.Assign( aNewPos.nNode.GetNode().GetCntntNode(),
                                    nCntPos );
        }
        aAnchor.SetAnchor( &aNewPos );
    }

    pFrmFmt->SetFmtAttr( aAnchor );     // reset anchor

    if( RES_DRAWFRMFMT != pFrmFmt->Which() )
    {
        // get Content and reset ContentAttribute
        SwNodeIndex aIdx( pDoc->GetNodes() );
        RestoreSection( pDoc, &aIdx, SwFlyStartNode );
        pFrmFmt->SetFmtAttr( SwFmtCntnt( aIdx.GetNode().GetStartNode() ));
    }

    // Set InCntntAttribute not until there is content!
    // Otherwise the layout would format the Fly beforehand but would not find
    // content; this happened with graphics from the internet.
    if (FLY_AS_CHAR == nRndId)
    {
        // there must be at least the attribute in a TextNode
        SwCntntNode* pCNd = aAnchor.GetCntntAnchor()->nNode.GetNode().GetCntntNode();
        OSL_ENSURE( pCNd->IsTxtNode(), "no Text Node at position." );
        SwFmtFlyCnt aFmt( pFrmFmt );
        static_cast<SwTxtNode*>(pCNd)->InsertItem( aFmt, nCntPos, nCntPos );
    }

    pFrmFmt->MakeFrms();

    if( bShowSelFrm )
    {
        rContext.SetSelections(pFrmFmt, 0);
    }

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
    bDelFmt =  false;
}

void SwUndoFlyBase::DelFly( SwDoc* pDoc )
{
    bDelFmt = true;                 // delete Format in DTOR
    pFrmFmt->DelFrms();                 // destroy Frms

    // all Uno objects should now log themselves off
    {
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFrmFmt );
        pFrmFmt->ModifyNotification( &aMsgHint, &aMsgHint );
    }

    if ( RES_DRAWFRMFMT != pFrmFmt->Which() )
    {
        // if there is content than save it
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
    // The positions in Nodes array got shifted.
    nRndId = static_cast<sal_uInt16>(rAnchor.GetAnchorId());
    if (FLY_AS_CHAR == nRndId)
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
        SwTxtNode *const pTxtNd = pPos->nNode.GetNode().GetTxtNode();
        OSL_ENSURE( pTxtNd, "Kein Textnode gefunden" );
        SwTxtFlyCnt* const pAttr = static_cast<SwTxtFlyCnt*>(
            pTxtNd->GetTxtAttrForCharAt( nCntPos, RES_TXTATR_FLYCNT ) );
        // attribute is still in TextNode, delete
        if( pAttr && pAttr->GetFlyCnt().GetFrmFmt() == pFrmFmt )
        {
            // Pointer to 0, do not delete
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

    pFrmFmt->ResetFmtAttr( RES_ANCHOR );        // delete anchor

    // delete from array
    SwFrmFmts& rFlyFmts = *(SwFrmFmts*)pDoc->GetSpzFrmFmts();
    rFlyFmts.erase( std::find( rFlyFmts.begin(), rFlyFmts.end(), pFrmFmt ));
}

SwUndoInsLayFmt::SwUndoInsLayFmt( SwFrmFmt* pFormat, sal_uLong nNodeIdx, sal_Int32 nCntIdx )
    : SwUndoFlyBase( pFormat, RES_DRAWFRMFMT == pFormat->Which() ?
                                            UNDO_INSDRAWFMT : UNDO_INSLAYFMT ),
    mnCrsrSaveIndexPara( nNodeIdx ), mnCrsrSaveIndexPos( nCntIdx )
{
    const SwFmtAnchor& rAnchor = pFrmFmt->GetAnchor();
    nRndId = static_cast<sal_uInt16>(rAnchor.GetAnchorId());
    bDelFmt = false;
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
        OSL_FAIL( "Which FlyFrame?" );
    }
}

SwUndoInsLayFmt::~SwUndoInsLayFmt()
{
}

void SwUndoInsLayFmt::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // no content
    {
        bool bRemoveIdx = true;
        if( mnCrsrSaveIndexPara > 0 )
        {
            SwTxtNode *const pNode =
                rDoc.GetNodes()[mnCrsrSaveIndexPara]->GetTxtNode();
            if( pNode )
            {
                SwNodeIndex aIdx( rDoc.GetNodes(),
                        rCntnt.GetCntntIdx()->GetIndex() );
                SwNodeIndex aEndIdx( rDoc.GetNodes(),
                        aIdx.GetNode().EndOfSectionIndex() );
                SwIndex aIndex( pNode, mnCrsrSaveIndexPos );
                SwPosition aPos( *pNode, aIndex );
                rDoc.CorrAbs( aIdx, aEndIdx, aPos, sal_True );
                bRemoveIdx = false;
            }
        }
        if( bRemoveIdx )
        {
            RemoveIdxFromSection( rDoc, rCntnt.GetCntntIdx()->GetIndex() );
        }
    }
    DelFly(& rDoc);
}

void SwUndoInsLayFmt::RedoImpl(::sw::UndoRedoContext & rContext)
{
    InsFly(rContext);
}

void SwUndoInsLayFmt::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    // get anchor and reset it
    SwFmtAnchor aAnchor( pFrmFmt->GetAnchor() );
    if ((FLY_AT_PARA == aAnchor.GetAnchorId()) ||
        (FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
        (FLY_AS_CHAR == aAnchor.GetAnchorId()))
    {
        SwPosition aPos( *rContext.GetRepeatPaM().GetPoint() );
        if (FLY_AT_PARA == aAnchor.GetAnchorId())
        {
            aPos.nContent.Assign( 0, 0 );
        }
        aAnchor.SetAnchor( &aPos );
    }
    else if( FLY_AT_FLY == aAnchor.GetAnchorId() )
    {
        SwStartNode const*const pSttNd =
            rContext.GetRepeatPaM().GetNode()->FindFlyStartNode();
        if( pSttNd )
        {
            SwPosition aPos( *pSttNd );
            aAnchor.SetAnchor( &aPos );
        }
        else
        {
            return ;
        }
    }
    else if (FLY_AT_PAGE == aAnchor.GetAnchorId())
    {
        aAnchor.SetPageNum( pDoc->GetCurrentLayout()->GetCurrPage( &rContext.GetRepeatPaM() ));
    }
    else {
        OSL_FAIL( "What kind of anchor is this?" );
    }

    SwFrmFmt* pFlyFmt = pDoc->CopyLayoutFmt( *pFrmFmt, aAnchor, true, true );
    (void) pFlyFmt;
}

// #111827#
OUString SwUndoInsLayFmt::GetComment() const
{
    OUString aResult;

    // HACK: disable caching:
    // the SfxUndoManager calls GetComment() too early: the pFrmFmt does not
    // have a SwDrawContact yet, so it will fall back to SwUndo::GetComment(),
    // which sets pComment to a wrong value.
//    if (! pComment)
    if (true)
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

static SwUndoId
lcl_GetSwUndoId(SwFrmFmt *const pFrmFmt)
{
    if (RES_DRAWFRMFMT != pFrmFmt->Which())
    {
        const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
        OSL_ENSURE( rCntnt.GetCntntIdx(), "Fly without content" );

        SwNodeIndex firstNode(*rCntnt.GetCntntIdx(), 1);
        SwNoTxtNode *const pNoTxtNode(firstNode.GetNode().GetNoTxtNode());
        if (pNoTxtNode && pNoTxtNode->IsGrfNode())
        {
            return UNDO_DELGRF;
        }
        else if (pNoTxtNode && pNoTxtNode->IsOLENode())
        {
            // surprisingly not UNDO_DELOLE, which does not seem to work
            return UNDO_DELETE;
        }
    }
    return UNDO_DELLAYFMT;
}

SwUndoDelLayFmt::SwUndoDelLayFmt( SwFrmFmt* pFormat )
    : SwUndoFlyBase( pFormat, lcl_GetSwUndoId(pFormat) )
    , bShowSelFrm( true )
{
    SwDoc* pDoc = pFormat->GetDoc();
    DelFly( pDoc );
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
            SwNode *const pNd = & pIdx->GetNode();

            if ( pNd->IsNoTxtNode() && pNd->IsOLENode())
            {
                SwOLENode * pOLENd = pNd->GetOLENode();

                aRewriter.AddRule(UndoArg1, pOLENd->GetDescription());
            }
        }
    }

    return aRewriter;
}

void SwUndoDelLayFmt::UndoImpl(::sw::UndoRedoContext & rContext)
{
    InsFly( rContext, bShowSelFrm );
}

void SwUndoDelLayFmt::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // no content
    {
        RemoveIdxFromSection(rDoc, rCntnt.GetCntntIdx()->GetIndex());
    }

    DelFly(& rDoc);
}

void SwUndoDelLayFmt::RedoForRollback()
{
    const SwFmtCntnt& rCntnt = pFrmFmt->GetCntnt();
    if( rCntnt.GetCntntIdx() )  // no content
        RemoveIdxFromSection( *pFrmFmt->GetDoc(),
                                rCntnt.GetCntntIdx()->GetIndex() );

    DelFly( pFrmFmt->GetDoc() );
}

SwUndoSetFlyFmt::SwUndoSetFlyFmt( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFrmFmt )
    : SwUndo( UNDO_SETFLYFRMFMT ), SwClient( &rFlyFmt ), pFrmFmt( &rFlyFmt ),
    pOldFmt( (SwFrmFmt*)rFlyFmt.DerivedFrom() ), pNewFmt( &rNewFrmFmt ),
    pItemSet( new SfxItemSet( *rFlyFmt.GetAttrSet().GetPool(),
                                rFlyFmt.GetAttrSet().GetRanges() )),
    nOldNode( 0 ), nNewNode( 0 ),
    nOldCntnt( 0 ), nNewCntnt( 0 ),
    nOldAnchorTyp( 0 ), nNewAnchorTyp( 0 ), bAnchorChgd( false )
{
}

SwRewriter SwUndoSetFlyFmt::GetRewriter() const
{
    SwRewriter aRewriter;

    if (pNewFmt)
        aRewriter.AddRule(UndoArg1, pNewFmt->GetName());

    return aRewriter;
}

SwUndoSetFlyFmt::~SwUndoSetFlyFmt()
{
    delete pItemSet;
}

void SwUndoSetFlyFmt::DeRegisterFromFormat( SwFmt& rFmt )
{
    rFmt.Remove(this);
}

void SwUndoSetFlyFmt::GetAnchor( SwFmtAnchor& rAnchor,
                                sal_uLong nNode, sal_Int32 nCntnt )
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
                if (nCntnt > static_cast<SwTxtNode*>(pNd)->GetTxt().getLength())
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
            // invalid position - assign first page
            rAnchor.SetType( FLY_AT_PAGE );
            rAnchor.SetPageNum( 1 );
        }
    }
    else
        rAnchor.SetPageNum( nCntnt );
}

void SwUndoSetFlyFmt::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // Is the new Format still existent?
    if( USHRT_MAX != rDoc.GetFrmFmts()->GetPos( (const SwFrmFmt*)pOldFmt ) )
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
                // With InCntnts it's tricky: the text attribute needs to be
                // deleted. Unfortunately, this not only destroys the Frms but
                // also the format. To prevent that, first detach the
                // connection between attribute and format.
                const SwPosition *pPos = rOldAnch.GetCntntAnchor();
                SwTxtNode *pTxtNode = pPos->nNode.GetNode().GetTxtNode();
                OSL_ENSURE( pTxtNode->HasHints(), "Missing FlyInCnt-Hint." );
                const sal_Int32 nIdx = pPos->nContent.GetIndex();
                SwTxtAttr * pHnt = pTxtNode->GetTxtAttrForCharAt(
                        nIdx, RES_TXTATR_FLYCNT );
                OSL_ENSURE( pHnt && pHnt->Which() == RES_TXTATR_FLYCNT,
                            "Missing FlyInCnt-Hint." );
                OSL_ENSURE( pHnt && pHnt->GetFlyCnt().GetFrmFmt() == pFrmFmt,
                            "Wrong TxtFlyCnt-Hint." );
                const_cast<SwFmtFlyCnt&>(pHnt->GetFlyCnt()).SetFlyFmt();

                // Connection is now detached, therefore the attribute can be
                // deleted
                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }

            // reposition anchor
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
        rContext.SetSelections(pFrmFmt, 0);
    }
}

void SwUndoSetFlyFmt::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // Is the new Format still existent?
    if( USHRT_MAX != rDoc.GetFrmFmts()->GetPos( (const SwFrmFmt*)pNewFmt ) )
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

        rContext.SetSelections(pFrmFmt, 0);
    }
}

void SwUndoSetFlyFmt::PutAttr( sal_uInt16 nWhich, const SfxPoolItem* pItem )
{
    if( pItem && pItem != GetDfltAttr( nWhich ) )
    {
        // Special treatment for this anchor
        if( RES_ANCHOR == nWhich )
        {
            // only keep the first change
            OSL_ENSURE( !bAnchorChgd, "multiple changes of an anchor are not allowed!" );

            bAnchorChgd = true;

            const SwFmtAnchor* pAnchor = (SwFmtAnchor*)pItem;
            switch( nOldAnchorTyp = static_cast<sal_uInt16>(pAnchor->GetAnchorId()) )
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
            switch( nNewAnchorTyp = static_cast<sal_uInt16>(pAnchor->GetAnchorId()) )
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

void SwUndoSetFlyFmt::Modify( const SfxPoolItem* pOld, const SfxPoolItem* )
{
    if( pOld )
    {
        sal_uInt16 nWhich = pOld->Which();

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
