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
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <rootfrm.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <ndole.hxx>

SwUndoFlyBase::SwUndoFlyBase( SwFrameFormat* pFormat, SwUndoId nUndoId )
    : SwUndo(nUndoId, pFormat->GetDoc())
    , pFrameFormat(pFormat)
    , nNdPgPos(0)
    , nCntPos(0)
    , nRndId(RndStdIds::FLY_AT_PARA)
    , bDelFormat(false)
{
}

SwUndoFlyBase::~SwUndoFlyBase()
{
    if( bDelFormat )       // delete during an Undo?
    {
        if (pFrameFormat->GetOtherTextBoxFormat())
        {   // clear that before delete
            pFrameFormat->SetOtherTextBoxFormat(nullptr);
        }
        delete pFrameFormat;
    }
}

void SwUndoFlyBase::InsFly(::sw::UndoRedoContext & rContext, bool bShowSelFrame)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    // add again into array
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.push_back( pFrameFormat );

    // OD 26.06.2003 #108784# - insert 'master' drawing object into drawing page
    if ( RES_DRAWFRMFMT == pFrameFormat->Which() )
        pFrameFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PREP_INSERT_FLY));

    SwFormatAnchor aAnchor( nRndId );

    if (RndStdIds::FLY_AT_PAGE == nRndId)
    {
        aAnchor.SetPageNum( static_cast<sal_uInt16>(nNdPgPos) );
    }
    else
    {
        SwPosition aNewPos(pDoc->GetNodes().GetEndOfContent());
        aNewPos.nNode = nNdPgPos;
        if ((RndStdIds::FLY_AS_CHAR == nRndId) || (RndStdIds::FLY_AT_CHAR == nRndId))
        {
            aNewPos.nContent.Assign( aNewPos.nNode.GetNode().GetContentNode(),
                                    nCntPos );
        }
        aAnchor.SetAnchor( &aNewPos );
    }

    pFrameFormat->SetFormatAttr( aAnchor );     // reset anchor

    if( RES_DRAWFRMFMT != pFrameFormat->Which() )
    {
        // get Content and reset ContentAttribute
        SwNodeIndex aIdx( pDoc->GetNodes() );
        RestoreSection( pDoc, &aIdx, SwFlyStartNode );
        pFrameFormat->SetFormatAttr( SwFormatContent( aIdx.GetNode().GetStartNode() ));
    }

    // Set InContentAttribute not until there is content!
    // Otherwise the layout would format the Fly beforehand but would not find
    // content; this happened with graphics from the internet.
    if (RndStdIds::FLY_AS_CHAR == nRndId)
    {
        // there must be at least the attribute in a TextNode
        SwContentNode* pCNd = aAnchor.GetContentAnchor()->nNode.GetNode().GetContentNode();
        OSL_ENSURE( pCNd->IsTextNode(), "no Text Node at position." );
        SwFormatFlyCnt aFormat( pFrameFormat );
        pCNd->GetTextNode()->InsertItem(aFormat, nCntPos, nCntPos, SetAttrMode::NOHINTEXPAND);
    }

    if (pFrameFormat->GetOtherTextBoxFormat())
    {
        // recklessly assume that this thing will live longer than the
        // SwUndoFlyBase - not sure what could be done if that isn't the case...
        pFrameFormat->GetOtherTextBoxFormat()->SetOtherTextBoxFormat(pFrameFormat);
    }

    pFrameFormat->MakeFrames();

    if( bShowSelFrame )
    {
        rContext.SetSelections(pFrameFormat, nullptr);
    }

    if( GetHistory() )
        GetHistory()->Rollback( pDoc );

    switch( nRndId )
    {
    case RndStdIds::FLY_AS_CHAR:
    case RndStdIds::FLY_AT_CHAR:
        {
            const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
            nNdPgPos = rAnchor.GetContentAnchor()->nNode.GetIndex();
            nCntPos = rAnchor.GetContentAnchor()->nContent.GetIndex();
        }
        break;
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_FLY:
        {
            const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
            nNdPgPos = rAnchor.GetContentAnchor()->nNode.GetIndex();
        }
        break;
    case RndStdIds::FLY_AT_PAGE:
        break;
    default: break;
    }
    bDelFormat =  false;
}

void SwUndoFlyBase::DelFly( SwDoc* pDoc )
{
    bDelFormat = true;                 // delete Format in DTOR
    pFrameFormat->DelFrames();                 // destroy Frames

    if (pFrameFormat->GetOtherTextBoxFormat())
    {   // tdf#108867 clear that pointer
        pFrameFormat->GetOtherTextBoxFormat()->SetOtherTextBoxFormat(nullptr);
    }

    // all Uno objects should now log themselves off
    {
        SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, pFrameFormat );
        pFrameFormat->ModifyNotification( &aMsgHint, &aMsgHint );
    }

    if ( RES_DRAWFRMFMT != pFrameFormat->Which() )
    {
        // if there is content than save it
        const SwFormatContent& rContent = pFrameFormat->GetContent();
        OSL_ENSURE( rContent.GetContentIdx(), "Fly without content" );

        SaveSection( *rContent.GetContentIdx() );
        const_cast<SwFormatContent&>(rContent).SetNewContentIdx( nullptr );
    }
    // OD 02.07.2003 #108784# - remove 'master' drawing object from drawing page
    else
        pFrameFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PREP_DELETE_FLY));

    const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
    const SwPosition* pPos = rAnchor.GetContentAnchor();
    // The positions in Nodes array got shifted.
    nRndId = rAnchor.GetAnchorId();
    if (RndStdIds::FLY_AS_CHAR == nRndId)
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
        SwTextNode *const pTextNd = pPos->nNode.GetNode().GetTextNode();
        OSL_ENSURE( pTextNd, "No Textnode found" );
        SwTextFlyCnt* const pAttr = static_cast<SwTextFlyCnt*>(
            pTextNd->GetTextAttrForCharAt( nCntPos, RES_TXTATR_FLYCNT ) );
        // attribute is still in TextNode, delete
        if( pAttr && pAttr->GetFlyCnt().GetFrameFormat() == pFrameFormat )
        {
            // Pointer to 0, do not delete
            const_cast<SwFormatFlyCnt&>(pAttr->GetFlyCnt()).SetFlyFormat();
            SwIndex aIdx( pPos->nContent );
            pTextNd->EraseText( aIdx, 1 );
        }
    }
    else if (RndStdIds::FLY_AT_CHAR == nRndId)
    {
        nNdPgPos = pPos->nNode.GetIndex();
        nCntPos = pPos->nContent.GetIndex();
    }
    else if ((RndStdIds::FLY_AT_PARA == nRndId) || (RndStdIds::FLY_AT_FLY == nRndId))
    {
        nNdPgPos = pPos->nNode.GetIndex();
    }
    else
    {
        nNdPgPos = rAnchor.GetPageNum();
    }

    pFrameFormat->ResetFormatAttr( RES_ANCHOR );        // delete anchor

    // delete from array
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( pFrameFormat );
}

SwUndoInsLayFormat::SwUndoInsLayFormat( SwFrameFormat* pFormat, sal_uLong nNodeIdx, sal_Int32 nCntIdx )
    : SwUndoFlyBase( pFormat, RES_DRAWFRMFMT == pFormat->Which() ?
                                            SwUndoId::INSDRAWFMT : SwUndoId::INSLAYFMT ),
    mnCursorSaveIndexPara( nNodeIdx ), mnCursorSaveIndexPos( nCntIdx )
{
    const SwFormatAnchor& rAnchor = pFrameFormat->GetAnchor();
    nRndId = rAnchor.GetAnchorId();
    bDelFormat = false;
    switch( nRndId )
    {
    case RndStdIds::FLY_AT_PAGE:
        nNdPgPos = rAnchor.GetPageNum();
        break;
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_FLY:
        nNdPgPos = rAnchor.GetContentAnchor()->nNode.GetIndex();
        break;
    case RndStdIds::FLY_AS_CHAR:
    case RndStdIds::FLY_AT_CHAR:
        {
            const SwPosition* pPos = rAnchor.GetContentAnchor();
            nCntPos = pPos->nContent.GetIndex();
            nNdPgPos = pPos->nNode.GetIndex();
        }
        break;
    default:
        OSL_FAIL( "Which FlyFrame?" );
    }
}

SwUndoInsLayFormat::~SwUndoInsLayFormat()
{
}

void SwUndoInsLayFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    if( rContent.GetContentIdx() )  // no content
    {
        bool bRemoveIdx = true;
        if( mnCursorSaveIndexPara > 0 )
        {
            SwTextNode *const pNode =
                rDoc.GetNodes()[mnCursorSaveIndexPara]->GetTextNode();
            if( pNode )
            {
                SwNodeIndex aIdx( rDoc.GetNodes(),
                        rContent.GetContentIdx()->GetIndex() );
                SwNodeIndex aEndIdx( rDoc.GetNodes(),
                        aIdx.GetNode().EndOfSectionIndex() );
                SwIndex aIndex( pNode, mnCursorSaveIndexPos );
                SwPosition aPos( *pNode, aIndex );
                SwDoc::CorrAbs( aIdx, aEndIdx, aPos, true );
                bRemoveIdx = false;
            }
        }
        if( bRemoveIdx )
        {
            RemoveIdxFromSection( rDoc, rContent.GetContentIdx()->GetIndex() );
        }
    }
    DelFly(& rDoc);
}

void SwUndoInsLayFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    InsFly(rContext);
}

void SwUndoInsLayFormat::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    // get anchor and reset it
    SwFormatAnchor aAnchor( pFrameFormat->GetAnchor() );
    if ((RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_CHAR == aAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AS_CHAR == aAnchor.GetAnchorId()))
    {
        SwPosition aPos( *rContext.GetRepeatPaM().GetPoint() );
        if (RndStdIds::FLY_AT_PARA == aAnchor.GetAnchorId())
        {
            aPos.nContent.Assign( nullptr, 0 );
        }
        aAnchor.SetAnchor( &aPos );
    }
    else if( RndStdIds::FLY_AT_FLY == aAnchor.GetAnchorId() )
    {
        SwStartNode const*const pSttNd =
            rContext.GetRepeatPaM().GetNode().FindFlyStartNode();
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
    else if (RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId())
    {
        aAnchor.SetPageNum( pDoc->getIDocumentLayoutAccess().GetCurrentLayout()->GetCurrPage( &rContext.GetRepeatPaM() ));
    }
    else {
        OSL_FAIL( "What kind of anchor is this?" );
    }

    (void) pDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *pFrameFormat, aAnchor, true, true );
}

OUString SwUndoInsLayFormat::GetComment() const
{
    OUString aResult;

    // HACK: disable caching:
    // the SfxUndoManager calls GetComment() too early: the pFrameFormat does not
    // have a SwDrawContact yet, so it will fall back to SwUndo::GetComment(),
    // which sets pComment to a wrong value.
//    if (! pComment)
    if ((true))
    {
        /*
          If frame format is present and has an SdrObject use the undo
          comment of the SdrObject. Otherwise use the default comment.
        */
        bool bDone = false;
        if (pFrameFormat)
        {
            const SdrObject * pSdrObj = pFrameFormat->FindSdrObject();
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
        aResult = *maComment;

    return aResult;
}

static SwUndoId
lcl_GetSwUndoId(SwFrameFormat const *const pFrameFormat)
{
    if (RES_DRAWFRMFMT != pFrameFormat->Which())
    {
        const SwFormatContent& rContent = pFrameFormat->GetContent();
        OSL_ENSURE( rContent.GetContentIdx(), "Fly without content" );

        SwNodeIndex firstNode(*rContent.GetContentIdx(), 1);
        SwNoTextNode *const pNoTextNode(firstNode.GetNode().GetNoTextNode());
        if (pNoTextNode && pNoTextNode->IsGrfNode())
        {
            return SwUndoId::DELGRF;
        }
        else if (pNoTextNode && pNoTextNode->IsOLENode())
        {
            // surprisingly not SwUndoId::DELOLE, which does not seem to work
            return SwUndoId::DELETE;
        }
    }
    return SwUndoId::DELLAYFMT;
}

SwUndoDelLayFormat::SwUndoDelLayFormat( SwFrameFormat* pFormat )
    : SwUndoFlyBase( pFormat, lcl_GetSwUndoId(pFormat) )
    , bShowSelFrame( true )
{
    SwDoc* pDoc = pFormat->GetDoc();
    DelFly( pDoc );
}

SwRewriter SwUndoDelLayFormat::GetRewriter() const
{
    SwRewriter aRewriter;

    SwDoc * pDoc = pFrameFormat->GetDoc();

    if (pDoc)
    {
        SwNodeIndex* pIdx = GetMvSttIdx();
        if( 1 == GetMvNodeCnt() && pIdx)
        {
            SwNode *const pNd = & pIdx->GetNode();

            if ( pNd->IsNoTextNode() && pNd->IsOLENode())
            {
                SwOLENode * pOLENd = pNd->GetOLENode();

                aRewriter.AddRule(UndoArg1, pOLENd->GetDescription());
            }
        }
    }

    return aRewriter;
}

void SwUndoDelLayFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    InsFly( rContext, bShowSelFrame );
}

void SwUndoDelLayFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    if( rContent.GetContentIdx() )  // no content
    {
        RemoveIdxFromSection(rDoc, rContent.GetContentIdx()->GetIndex());
    }

    DelFly(& rDoc);
}

void SwUndoDelLayFormat::RedoForRollback()
{
    const SwFormatContent& rContent = pFrameFormat->GetContent();
    if( rContent.GetContentIdx() )  // no content
        RemoveIdxFromSection( *pFrameFormat->GetDoc(),
                                rContent.GetContentIdx()->GetIndex() );

    DelFly( pFrameFormat->GetDoc() );
}

SwUndoSetFlyFormat::SwUndoSetFlyFormat( SwFrameFormat& rFlyFormat, SwFrameFormat& rNewFrameFormat )
    : SwUndo( SwUndoId::SETFLYFRMFMT, rFlyFormat.GetDoc() ), SwClient( &rFlyFormat ), pFrameFormat( &rFlyFormat ),
    m_DerivedFromFormatName( rFlyFormat.IsDefault() ? "" : rFlyFormat.DerivedFrom()->GetName() ),
    m_NewFormatName( rNewFrameFormat.GetName() ),
    pItemSet( new SfxItemSet( *rFlyFormat.GetAttrSet().GetPool(),
                                rFlyFormat.GetAttrSet().GetRanges() )),
    nOldNode( 0 ), nNewNode( 0 ),
    nOldContent( 0 ), nNewContent( 0 ),
    nOldAnchorTyp( RndStdIds::FLY_AT_PARA ), nNewAnchorTyp( RndStdIds::FLY_AT_PARA ), bAnchorChgd( false )
{
}

SwRewriter SwUndoSetFlyFormat::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, m_NewFormatName);

    return aRewriter;
}

SwUndoSetFlyFormat::~SwUndoSetFlyFormat()
{
}

void SwUndoSetFlyFormat::GetAnchor( SwFormatAnchor& rAnchor,
                                sal_uLong nNode, sal_Int32 nContent )
{
    RndStdIds nAnchorTyp = rAnchor.GetAnchorId();
    if (RndStdIds::FLY_AT_PAGE != nAnchorTyp)
    {
        SwNode* pNd = pFrameFormat->GetDoc()->GetNodes()[ nNode ];

        if( RndStdIds::FLY_AT_FLY == nAnchorTyp
                ? ( !pNd->IsStartNode() || SwFlyStartNode !=
                    static_cast<SwStartNode*>(pNd)->GetStartNodeType() )
                : !pNd->IsTextNode() )
        {
            pNd = nullptr;    // invalid position
        }
        else
        {
            SwPosition aPos( *pNd );
            if ((RndStdIds::FLY_AS_CHAR == nAnchorTyp) ||
                (RndStdIds::FLY_AT_CHAR == nAnchorTyp))
            {
                if (nContent > pNd->GetTextNode()->GetText().getLength())
                {
                    pNd = nullptr;    // invalid position
                }
                else
                {
                    aPos.nContent.Assign(pNd->GetTextNode(), nContent);
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
            rAnchor.SetType( RndStdIds::FLY_AT_PAGE );
            rAnchor.SetPageNum( 1 );
        }
    }
    else
        rAnchor.SetPageNum( nContent );
}

void SwUndoSetFlyFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // Is the new Format still existent?
    SwFrameFormat* pDerivedFromFrameFormat = rDoc.FindFrameFormatByName(m_DerivedFromFormatName);
    if (pDerivedFromFrameFormat)
    {
        if( bAnchorChgd )
            pFrameFormat->DelFrames();

        if( pFrameFormat->DerivedFrom() != pDerivedFromFrameFormat )
            pFrameFormat->SetDerivedFrom( pDerivedFromFrameFormat );

        SfxItemIter aIter( *pItemSet );
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( pItem )
        {
            if( IsInvalidItem( pItem ))
                pFrameFormat->ResetFormatAttr( pItemSet->GetWhichByPos(
                                        aIter.GetCurPos() ));
            else
                pFrameFormat->SetFormatAttr( *pItem );

            if( aIter.IsAtEnd() )
                break;
            pItem = aIter.NextItem();
        }

        if( bAnchorChgd )
        {
            const SwFormatAnchor& rOldAnch = pFrameFormat->GetAnchor();
            if (RndStdIds::FLY_AS_CHAR == rOldAnch.GetAnchorId())
            {
                // With InContents it's tricky: the text attribute needs to be
                // deleted. Unfortunately, this not only destroys the Frames but
                // also the format. To prevent that, first detach the
                // connection between attribute and format.
                const SwPosition *pPos = rOldAnch.GetContentAnchor();
                SwTextNode *pTextNode = pPos->nNode.GetNode().GetTextNode();
                OSL_ENSURE( pTextNode->HasHints(), "Missing FlyInCnt-Hint." );
                const sal_Int32 nIdx = pPos->nContent.GetIndex();
                SwTextAttr * pHint = pTextNode->GetTextAttrForCharAt(
                        nIdx, RES_TXTATR_FLYCNT );
                assert(pHint && "Missing Hint.");
                OSL_ENSURE( pHint->Which() == RES_TXTATR_FLYCNT,
                            "Missing FlyInCnt-Hint." );
                OSL_ENSURE( pHint->GetFlyCnt().GetFrameFormat() == pFrameFormat,
                            "Wrong TextFlyCnt-Hint." );
                const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

                // Connection is now detached, therefore the attribute can be
                // deleted
                pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }

            // reposition anchor
            SwFormatAnchor aNewAnchor( nOldAnchorTyp );
            GetAnchor( aNewAnchor, nOldNode, nOldContent );
            pFrameFormat->SetFormatAttr( aNewAnchor );

            if (RndStdIds::FLY_AS_CHAR == aNewAnchor.GetAnchorId())
            {
                const SwPosition* pPos = aNewAnchor.GetContentAnchor();
                SwFormatFlyCnt aFormat( pFrameFormat );
                pPos->nNode.GetNode().GetTextNode()->InsertItem( aFormat,
                    nOldContent, 0 );
            }

            pFrameFormat->MakeFrames();
        }
        rContext.SetSelections(pFrameFormat, nullptr);
    }
}

void SwUndoSetFlyFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // Is the new Format still existent?
    SwFrameFormat* pNewFrameFormat = rDoc.FindFrameFormatByName(m_NewFormatName);
    if (pNewFrameFormat)
    {
        if( bAnchorChgd )
        {
            SwFormatAnchor aNewAnchor( nNewAnchorTyp );
            GetAnchor( aNewAnchor, nNewNode, nNewContent );
            SfxItemSet aSet( rDoc.GetAttrPool(), aFrameFormatSetRange );
            aSet.Put( aNewAnchor );
            rDoc.SetFrameFormatToFly( *pFrameFormat, *pNewFrameFormat, &aSet );
        }
        else
            rDoc.SetFrameFormatToFly( *pFrameFormat, *pNewFrameFormat );

        rContext.SetSelections(pFrameFormat, nullptr);
    }
}

void SwUndoSetFlyFormat::PutAttr( sal_uInt16 nWhich, const SfxPoolItem* pItem )
{
    if( pItem && pItem != GetDfltAttr( nWhich ) )
    {
        // Special treatment for this anchor
        if( RES_ANCHOR == nWhich )
        {
            // only keep the first change
            OSL_ENSURE( !bAnchorChgd, "multiple changes of an anchor are not allowed!" );

            bAnchorChgd = true;

            const SwFormatAnchor* pAnchor = static_cast<const SwFormatAnchor*>(pItem);
            nOldAnchorTyp = pAnchor->GetAnchorId();
            switch( nOldAnchorTyp )
            {
            case RndStdIds::FLY_AS_CHAR:
            case RndStdIds::FLY_AT_CHAR:
                nOldContent = pAnchor->GetContentAnchor()->nContent.GetIndex();
                SAL_FALLTHROUGH;
            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_FLY:
                nOldNode = pAnchor->GetContentAnchor()->nNode.GetIndex();
                break;

            default:
                nOldContent = pAnchor->GetPageNum();
            }

            pAnchor = &pFrameFormat->GetAnchor();
            nNewAnchorTyp = pAnchor->GetAnchorId();
            switch( nNewAnchorTyp )
            {
            case RndStdIds::FLY_AS_CHAR:
            case RndStdIds::FLY_AT_CHAR:
                nNewContent = pAnchor->GetContentAnchor()->nContent.GetIndex();
                SAL_FALLTHROUGH;
            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_FLY:
                nNewNode = pAnchor->GetContentAnchor()->nNode.GetIndex();
                break;

            default:
                nNewContent = pAnchor->GetPageNum();
            }
        }
        else
            pItemSet->Put( *pItem );
    }
    else
        pItemSet->InvalidateItem( nWhich );
}

void SwUndoSetFlyFormat::Modify( const SfxPoolItem* pOld, const SfxPoolItem* )
{
    if( pOld )
    {
        sal_uInt16 nWhich = pOld->Which();

        if( nWhich < POOLATTR_END )
            PutAttr( nWhich, pOld );
        else if( RES_ATTRSET_CHG == nWhich )
        {
            SfxItemIter aIter( *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet() );
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
