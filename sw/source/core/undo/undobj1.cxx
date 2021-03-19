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

#include <libxml/xmlwriter.h>

#include <svl/itemiter.hxx>
#include <svx/svdundo.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <txtflcnt.hxx>
#include <frmfmt.hxx>
#include <UndoCore.hxx>
#include <rolbck.hxx>
#include <doc.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <ndole.hxx>
#include <frameformats.hxx>
#include <svx/svdobj.hxx>

SwUndoFlyBase::SwUndoFlyBase( SwFrameFormat* pFormat, SwUndoId nUndoId )
    : SwUndo(nUndoId, pFormat->GetDoc())
    , m_pFrameFormat(pFormat)
    , m_nNodePagePos(0)
    , m_nContentPos(0)
    , m_nRndId(RndStdIds::FLY_AT_PARA)
    , m_bDelFormat(false)
{
}

SwUndoFlyBase::~SwUndoFlyBase()
{
    if( m_bDelFormat )       // delete during an Undo?
    {
        if (m_pFrameFormat->GetOtherTextBoxFormat())
        {   // clear that before delete
            m_pFrameFormat->SetOtherTextBoxFormat(nullptr);
        }
        delete m_pFrameFormat;
    }
}

void SwUndoFlyBase::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUndoFlyBase"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_nNodePagePos"),
                                BAD_CAST(OString::number(m_nNodePagePos).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_nContentPos"),
                                BAD_CAST(OString::number(m_nContentPos).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_nRndId"),
                                BAD_CAST(OString::number(static_cast<int>(m_nRndId)).getStr()));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("m_bDelFormat"),
                                BAD_CAST(OString::boolean(m_bDelFormat).getStr()));

    SwUndo::dumpAsXml(pWriter);
    SwUndoSaveSection::dumpAsXml(pWriter);

    if (m_pFrameFormat)
    {
        m_pFrameFormat->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

void SwUndoFlyBase::InsFly(::sw::UndoRedoContext & rContext, bool bShowSelFrame)
{
    SwDoc *const pDoc = & rContext.GetDoc();

    // add again into array
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.push_back( m_pFrameFormat );

    // OD 26.06.2003 #108784# - insert 'master' drawing object into drawing page
    if ( RES_DRAWFRMFMT == m_pFrameFormat->Which() )
        m_pFrameFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PREP_INSERT_FLY));

    SwFormatAnchor aAnchor( m_nRndId );

    if (RndStdIds::FLY_AT_PAGE == m_nRndId)
    {
        aAnchor.SetPageNum( static_cast<sal_uInt16>(m_nNodePagePos) );
    }
    else
    {
        SwPosition aNewPos(pDoc->GetNodes().GetEndOfContent());
        aNewPos.nNode = m_nNodePagePos;
        if ((RndStdIds::FLY_AS_CHAR == m_nRndId) || (RndStdIds::FLY_AT_CHAR == m_nRndId))
        {
            aNewPos.nContent.Assign( aNewPos.nNode.GetNode().GetContentNode(),
                                    m_nContentPos );
        }
        aAnchor.SetAnchor( &aNewPos );
    }

    m_pFrameFormat->SetFormatAttr( aAnchor );     // reset anchor

    if( RES_DRAWFRMFMT != m_pFrameFormat->Which() )
    {
        // get Content and reset ContentAttribute
        SwNodeIndex aIdx( pDoc->GetNodes() );
        RestoreSection( pDoc, &aIdx, SwFlyStartNode );
        m_pFrameFormat->SetFormatAttr( SwFormatContent( aIdx.GetNode().GetStartNode() ));
    }

    // Set InContentAttribute not until there is content!
    // Otherwise the layout would format the Fly beforehand but would not find
    // content; this happened with graphics from the internet.
    if (RndStdIds::FLY_AS_CHAR == m_nRndId)
    {
        // there must be at least the attribute in a TextNode
        SwContentNode* pCNd = aAnchor.GetContentAnchor()->nNode.GetNode().GetContentNode();
        OSL_ENSURE( pCNd->IsTextNode(), "no Text Node at position." );
        SwFormatFlyCnt aFormat( m_pFrameFormat );
        pCNd->GetTextNode()->InsertItem(aFormat, m_nContentPos, m_nContentPos, SetAttrMode::NOHINTEXPAND);
    }

    if (m_pFrameFormat->GetOtherTextBoxFormat())
    {
        // recklessly assume that this thing will live longer than the
        // SwUndoFlyBase - not sure what could be done if that isn't the case...
        m_pFrameFormat->GetOtherTextBoxFormat()->SetOtherTextBoxFormat(m_pFrameFormat);

        if (m_pFrameFormat->GetOtherTextBoxFormat()->Which() == RES_DRAWFRMFMT)
        {
            SdrObject* pSdrObject = m_pFrameFormat->GetOtherTextBoxFormat()->FindSdrObject();
            if (pSdrObject)
            {
                // Make sure the old UNO wrapper is no longer cached after changing the shape +
                // textframe pair. Otherwise we would have a wrapper which doesn't know about its
                // textframe, even if it's there.
                pSdrObject->setUnoShape(nullptr);
            }
        }
        if (m_pFrameFormat->Which() == RES_DRAWFRMFMT)
        {
            // This is a draw format and we just set the fly format's textbox pointer to this draw
            // format.  Sync the draw format's content with the fly format's content.
            SwFrameFormat* pFlyFormat = m_pFrameFormat->GetOtherTextBoxFormat();
            m_pFrameFormat->SetFormatAttr(pFlyFormat->GetContent());
        }
    }

    m_pFrameFormat->MakeFrames();

    if( bShowSelFrame )
    {
        rContext.SetSelections(m_pFrameFormat, nullptr);
    }

    if( GetHistory() )
        GetHistory()->Rollback( pDoc );

    switch( m_nRndId )
    {
    case RndStdIds::FLY_AS_CHAR:
    case RndStdIds::FLY_AT_CHAR:
        {
            const SwFormatAnchor& rAnchor = m_pFrameFormat->GetAnchor();
            m_nNodePagePos = rAnchor.GetContentAnchor()->nNode.GetIndex();
            m_nContentPos = rAnchor.GetContentAnchor()->nContent.GetIndex();
        }
        break;
    case RndStdIds::FLY_AT_PARA:
    case RndStdIds::FLY_AT_FLY:
        {
            const SwFormatAnchor& rAnchor = m_pFrameFormat->GetAnchor();
            m_nNodePagePos = rAnchor.GetContentAnchor()->nNode.GetIndex();
        }
        break;
    case RndStdIds::FLY_AT_PAGE:
        break;
    default: break;
    }
    m_bDelFormat =  false;
}

void SwUndoFlyBase::DelFly( SwDoc* pDoc )
{
    m_bDelFormat = true;                 // delete Format in DTOR
    m_pFrameFormat->DelFrames();                 // destroy Frames

    if (m_pFrameFormat->GetOtherTextBoxFormat())
    {   // tdf#108867 clear that pointer
        m_pFrameFormat->GetOtherTextBoxFormat()->SetOtherTextBoxFormat(nullptr);
    }

    // all Uno objects should now log themselves off
    m_pFrameFormat->RemoveAllUnos();

    if ( RES_DRAWFRMFMT != m_pFrameFormat->Which() )
    {
        // if there is content than save it
        const SwFormatContent& rContent = m_pFrameFormat->GetContent();
        OSL_ENSURE( rContent.GetContentIdx(), "Fly without content" );

        SaveSection( *rContent.GetContentIdx() );
        const_cast<SwFormatContent&>(rContent).SetNewContentIdx( nullptr );
    }
    // OD 02.07.2003 #108784# - remove 'master' drawing object from drawing page
    else
        m_pFrameFormat->CallSwClientNotify(sw::DrawFrameFormatHint(sw::DrawFrameFormatHintId::PREP_DELETE_FLY));

    const SwFormatAnchor& rAnchor = m_pFrameFormat->GetAnchor();
    const SwPosition* pPos = rAnchor.GetContentAnchor();
    // The positions in Nodes array got shifted.
    m_nRndId = rAnchor.GetAnchorId();
    if (RndStdIds::FLY_AS_CHAR == m_nRndId)
    {
        m_nNodePagePos = pPos->nNode.GetIndex();
        m_nContentPos = pPos->nContent.GetIndex();
        SwTextNode *const pTextNd = pPos->nNode.GetNode().GetTextNode();
        OSL_ENSURE( pTextNd, "No Textnode found" );
        SwTextFlyCnt* const pAttr = static_cast<SwTextFlyCnt*>(
            pTextNd->GetTextAttrForCharAt( m_nContentPos, RES_TXTATR_FLYCNT ) );
        // attribute is still in TextNode, delete
        if( pAttr && pAttr->GetFlyCnt().GetFrameFormat() == m_pFrameFormat )
        {
            // Pointer to 0, do not delete
            const_cast<SwFormatFlyCnt&>(pAttr->GetFlyCnt()).SetFlyFormat();
            SwIndex aIdx( pPos->nContent );
            pTextNd->EraseText( aIdx, 1 );
        }
    }
    else if (RndStdIds::FLY_AT_CHAR == m_nRndId)
    {
        m_nNodePagePos = pPos->nNode.GetIndex();
        m_nContentPos = pPos->nContent.GetIndex();
    }
    else if ((RndStdIds::FLY_AT_PARA == m_nRndId) || (RndStdIds::FLY_AT_FLY == m_nRndId))
    {
        m_nNodePagePos = pPos->nNode.GetIndex();
    }
    else
    {
        m_nNodePagePos = rAnchor.GetPageNum();
    }

    m_pFrameFormat->ResetFormatAttr( RES_ANCHOR );        // delete anchor

    // delete from array
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( m_pFrameFormat );
}

SwUndoInsLayFormat::SwUndoInsLayFormat( SwFrameFormat* pFormat, sal_uLong nNodeIdx, sal_Int32 nCntIdx )
    : SwUndoFlyBase( pFormat, RES_DRAWFRMFMT == pFormat->Which() ?
                                            SwUndoId::INSDRAWFMT : SwUndoId::INSLAYFMT ),
    mnCursorSaveIndexPara( nNodeIdx ), mnCursorSaveIndexPos( nCntIdx )
{
    const SwFormatAnchor& rAnchor = m_pFrameFormat->GetAnchor();
    m_nRndId = rAnchor.GetAnchorId();
    m_bDelFormat = false;
    // note: SwUndoInsLayFormat is called with the content being fully inserted
    // from most places but with only an empty content section from
    // CopyLayoutFormat(); it's not necessary here to init m_nNodePagePos
    // because Undo will do it.
}

SwUndoInsLayFormat::~SwUndoInsLayFormat()
{
}

void SwUndoInsLayFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    const SwFormatContent& rContent = m_pFrameFormat->GetContent();
    if( rContent.GetContentIdx() )  // no content
    {
        assert(&rContent.GetContentIdx()->GetNodes() == &rDoc.GetNodes());
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
    SwFormatAnchor aAnchor( m_pFrameFormat->GetAnchor() );
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

    (void) pDoc->getIDocumentLayoutAccess().CopyLayoutFormat( *m_pFrameFormat, aAnchor, true, true );
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
        if (m_pFrameFormat)
        {
            const SdrObject * pSdrObj = m_pFrameFormat->FindSdrObject();
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
    , m_bShowSelFrame( true )
{
    SwDoc* pDoc = pFormat->GetDoc();
    DelFly( pDoc );
}

SwRewriter SwUndoDelLayFormat::GetRewriter() const
{
    SwRewriter aRewriter;

    SwDoc * pDoc = m_pFrameFormat->GetDoc();

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
    InsFly( rContext, m_bShowSelFrame );
}

void SwUndoDelLayFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc(rContext.GetDoc());
    const SwFormatContent& rContent = m_pFrameFormat->GetContent();
    if( rContent.GetContentIdx() )  // no content
    {
        RemoveIdxFromSection(rDoc, rContent.GetContentIdx()->GetIndex());
    }

    DelFly(& rDoc);
}

void SwUndoDelLayFormat::RedoForRollback()
{
    const SwFormatContent& rContent = m_pFrameFormat->GetContent();
    if( rContent.GetContentIdx() )  // no content
        RemoveIdxFromSection( *m_pFrameFormat->GetDoc(),
                                rContent.GetContentIdx()->GetIndex() );

    DelFly( m_pFrameFormat->GetDoc() );
}

SwUndoSetFlyFormat::SwUndoSetFlyFormat( SwFrameFormat& rFlyFormat, const SwFrameFormat& rNewFrameFormat )
    : SwUndo( SwUndoId::SETFLYFRMFMT, rFlyFormat.GetDoc() ), SwClient( &rFlyFormat ), m_pFrameFormat( &rFlyFormat ),
    m_DerivedFromFormatName( rFlyFormat.IsDefault() ? "" : rFlyFormat.DerivedFrom()->GetName() ),
    m_NewFormatName( rNewFrameFormat.GetName() ),
    m_pItemSet( new SfxItemSet( *rFlyFormat.GetAttrSet().GetPool(),
                                rFlyFormat.GetAttrSet().GetRanges() )),
    m_nOldNode( 0 ), m_nNewNode( 0 ),
    m_nOldContent( 0 ), m_nNewContent( 0 ),
    m_nOldAnchorType( RndStdIds::FLY_AT_PARA ), m_nNewAnchorType( RndStdIds::FLY_AT_PARA ), m_bAnchorChanged( false )
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
        SwNode* pNd = m_pFrameFormat->GetDoc()->GetNodes()[ nNode ];

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
    if (!pDerivedFromFrameFormat)
        return;

    if( m_bAnchorChanged )
        m_pFrameFormat->DelFrames();

    if( m_pFrameFormat->DerivedFrom() != pDerivedFromFrameFormat)
        m_pFrameFormat->SetDerivedFrom(pDerivedFromFrameFormat);

    SfxItemIter aIter( *m_pItemSet );
    for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
    {
        if( IsInvalidItem( pItem ))
            m_pFrameFormat->ResetFormatAttr( m_pItemSet->GetWhichByPos(
                                    aIter.GetCurPos() ));
        else
            m_pFrameFormat->SetFormatAttr( *pItem );
    }

    if( m_bAnchorChanged )
    {
        const SwFormatAnchor& rOldAnch = m_pFrameFormat->GetAnchor();
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
            OSL_ENSURE( pHint->GetFlyCnt().GetFrameFormat() == m_pFrameFormat,
                        "Wrong TextFlyCnt-Hint." );
            const_cast<SwFormatFlyCnt&>(pHint->GetFlyCnt()).SetFlyFormat();

            // Connection is now detached, therefore the attribute can be
            // deleted
            pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
        }

        // reposition anchor
        SwFormatAnchor aNewAnchor( m_nOldAnchorType );
        GetAnchor( aNewAnchor, m_nOldNode, m_nOldContent );
        m_pFrameFormat->SetFormatAttr( aNewAnchor );

        if (RndStdIds::FLY_AS_CHAR == aNewAnchor.GetAnchorId())
        {
            const SwPosition* pPos = aNewAnchor.GetContentAnchor();
            SwFormatFlyCnt aFormat( m_pFrameFormat );
            pPos->nNode.GetNode().GetTextNode()->InsertItem( aFormat,
                m_nOldContent, 0 );
        }

        m_pFrameFormat->MakeFrames();
    }
    rContext.SetSelections(m_pFrameFormat, nullptr);
}

void SwUndoSetFlyFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    // Is the new Format still existent?
    SwFrameFormat* pNewFrameFormat = rDoc.FindFrameFormatByName(m_NewFormatName);
    if (!pNewFrameFormat)
        return;

    if( m_bAnchorChanged )
    {
        SwFormatAnchor aNewAnchor( m_nNewAnchorType );
        GetAnchor( aNewAnchor, m_nNewNode, m_nNewContent );
        SfxItemSet aSet( rDoc.GetAttrPool(), aFrameFormatSetRange );
        aSet.Put( aNewAnchor );
        rDoc.SetFrameFormatToFly( *m_pFrameFormat, *pNewFrameFormat, &aSet );
    }
    else
        rDoc.SetFrameFormatToFly( *m_pFrameFormat, *pNewFrameFormat);

    rContext.SetSelections(m_pFrameFormat, nullptr);
}

void SwUndoSetFlyFormat::PutAttr( sal_uInt16 nWhich, const SfxPoolItem* pItem )
{
    if( pItem && pItem != GetDfltAttr( nWhich ) )
    {
        // Special treatment for this anchor
        if( RES_ANCHOR == nWhich )
        {
            // only keep the first change
            OSL_ENSURE( !m_bAnchorChanged, "multiple changes of an anchor are not allowed!" );

            m_bAnchorChanged = true;

            const SwFormatAnchor* pAnchor = static_cast<const SwFormatAnchor*>(pItem);
            m_nOldAnchorType = pAnchor->GetAnchorId();
            switch( m_nOldAnchorType )
            {
            case RndStdIds::FLY_AS_CHAR:
            case RndStdIds::FLY_AT_CHAR:
                m_nOldContent = pAnchor->GetContentAnchor()->nContent.GetIndex();
                [[fallthrough]];
            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_FLY:
                m_nOldNode = pAnchor->GetContentAnchor()->nNode.GetIndex();
                break;

            default:
                m_nOldContent = pAnchor->GetPageNum();
            }

            pAnchor = &m_pFrameFormat->GetAnchor();
            m_nNewAnchorType = pAnchor->GetAnchorId();
            switch( m_nNewAnchorType )
            {
            case RndStdIds::FLY_AS_CHAR:
            case RndStdIds::FLY_AT_CHAR:
                m_nNewContent = pAnchor->GetContentAnchor()->nContent.GetIndex();
                [[fallthrough]];
            case RndStdIds::FLY_AT_PARA:
            case RndStdIds::FLY_AT_FLY:
                m_nNewNode = pAnchor->GetContentAnchor()->nNode.GetIndex();
                break;

            default:
                m_nNewContent = pAnchor->GetPageNum();
            }
        }
        else
            m_pItemSet->Put( *pItem );
    }
    else
        m_pItemSet->InvalidateItem( nWhich );
}

void SwUndoSetFlyFormat::SwClientNotify(const SwModify&, const SfxHint& rHint)
{
    auto pLegacy = dynamic_cast<const sw::LegacyModifyHint*>(&rHint);
    if(!pLegacy || !pLegacy->m_pOld)
        return;
    const sal_uInt16 nWhich = pLegacy->m_pOld->Which();
    if(nWhich < POOLATTR_END)
        PutAttr(nWhich, pLegacy->m_pOld);
    else if(RES_ATTRSET_CHG == nWhich)
    {
        SfxItemIter aIter(*static_cast<const SwAttrSetChg*>(pLegacy->m_pOld)->GetChgSet());
        for(const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
            PutAttr(pItem->Which(), pItem);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
