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

#include <hintids.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <pam.hxx>
#include <flyfrm.hxx>
#include <ndtxt.hxx>
#include <frmfmt.hxx>
#include <fmtflcnt.hxx>
#include <txtflcnt.hxx>
#include <fmtanchr.hxx>
#include <txtfrm.hxx>
#include <flyfrms.hxx>
#include <objectformatter.hxx>
#include <calbck.hxx>
#include <dcontact.hxx>
#include <textboxhelper.hxx>
#include <osl/diagnose.h>

SwFormatFlyCnt::SwFormatFlyCnt( SwFrameFormat *pFrameFormat )
    : SfxPoolItem( RES_TXTATR_FLYCNT ),
    m_pTextAttr( nullptr ),
    m_pFormat( pFrameFormat )
{
    setNonShareable();
}

bool SwFormatFlyCnt::operator==( const SfxPoolItem& rAttr ) const
{
    assert(SfxPoolItem::operator==(rAttr));
    return( m_pTextAttr && static_cast<const SwFormatFlyCnt&>(rAttr).m_pTextAttr &&
            m_pTextAttr->GetStart() == static_cast<const SwFormatFlyCnt&>(rAttr).m_pTextAttr->GetStart() &&
            m_pFormat == static_cast<const SwFormatFlyCnt&>(rAttr).GetFrameFormat() );
}

SwFormatFlyCnt* SwFormatFlyCnt::Clone( SfxItemPool* ) const
{
    return new SwFormatFlyCnt( m_pFormat );
}

void SwFormatFlyCnt::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwFormatFlyCnt"));
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", this);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("text-attr"), "%p", m_pTextAttr);
    (void)xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("format"), "%p", m_pFormat);

    SfxPoolItem::dumpAsXml(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SwTextFlyCnt::SwTextFlyCnt(
    const SfxPoolItemHolder& rAttr,
    sal_Int32 nStartPos )
    : SwTextAttr( rAttr, nStartPos )
{
    SwFormatFlyCnt& rSwFormatFlyCnt(static_cast<SwFormatFlyCnt&>(GetAttr()));
    rSwFormatFlyCnt.m_pTextAttr = this;
    SetHasDummyChar(true);
}

/** An overview of how a new SwTextFlyCnt is created:
 * MakeTextAttr() is called e.g. by SwTextNode::CopyText().
 * The following steps are required to clone:
 * 1) copying the pFormat with content, attributes etc.
 * 2) setting the anchor
 * 3) notification
 * Because not all required information is available at all times,
 * the steps are distributed variously:
 * ad 1) MakeTextAttr() calls DocumentLayoutManager::CopyLayoutFormat()
 *  which creates the new SwFlyFrameFormat and copies the content of the
 *  fly frame.
 * ad 2) SetAnchor() is called by SwTextNode::InsertHint() and sets the anchor
 *  position in the SwFlyFrameFormat to the SwPosition of the dummy
 *  CH_TXTATR_BREAKWORD.  This cannot be done in MakeTextAttr() because it
 *  doesn't know the target text node.
 * ad 3) GetFlyFrame_() is called during text formatting by SwTextFormatter
 *  and searches for the SwFlyFrame for the dummy char of the current
 *  SwTextFrame.  If none is found, a new SwFlyInContentFrame is created.
 *  Important: pTextFrame->AppendFly() immediately triggers a reformat
 *  of pTextFrame.  However, the recursion is blocked by the lock mechanism
 *  in SwTextFrame::Format().
 * The advantage of all this is that it's not necessary to explicitly iterate
 * over all SwTextFrames that depend on the SwTextNode to create the
 * SwFlyInContentFrame - this is done automatically already.
 */

void SwTextFlyCnt::CopyFlyFormat( SwDoc& rDoc )
{
    SwFrameFormat* pFormat = GetFlyCnt().GetFrameFormat();
    assert(pFormat);
    // The FlyFrameFormat must be copied - CopyLayoutFormat
    // (DocumentLayoutManager.cxx) creates the FlyFrameFormat and copies the
    // content.

    // disable undo while copying attribute
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    SwFormatAnchor aAnchor( pFormat->GetAnchor() );
    if ((RndStdIds::FLY_AT_PAGE != aAnchor.GetAnchorId()) &&
        (&rDoc != &pFormat->GetDoc()))   // different documents?
    {
        // JP 03.06.96: ensure that the copied anchor points to valid content!
        //              setting it to the correct position is done later.
        SwNodeIndex aIdx( rDoc.GetNodes().GetEndOfExtras(), +2 );
        SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
        if( !pCNd )
            pCNd = SwNodes::GoNext(&aIdx);

        SwPosition pos(aIdx.GetNode());
        aAnchor.SetAnchor( &pos );
    }

    SwFrameFormat* pNew = rDoc.getIDocumentLayoutAccess().CopyLayoutFormat( *pFormat, aAnchor, false, false );
    const_cast<SwFormatFlyCnt&>(GetFlyCnt()).SetFlyFormat( pNew );
}

/** SetAnchor() is called by SwTextNode::InsertHint() and sets the anchor
 *  position in the SwFlyFrameFormat to the SwPosition of the dummy
 *  CH_TXTATR_BREAKWORD.  This cannot be done in MakeTextAttr() because it
 *  doesn't know the target text node.
 */
void SwTextFlyCnt::SetAnchor( const SwTextNode *pNode )
{
    // for Undo, the new anchor must be known already!

    SwDoc& rDoc = const_cast<SwDoc&>(pNode->GetDoc());

    SwFrameFormat* pFormat = GetFlyCnt().GetFrameFormat();
    SwFormatAnchor aAnchor( pFormat->GetAnchor() );
    SwNode *const pOldNode(aAnchor.GetAnchorNode());

    std::optional<SwPosition> oPos;
    if (!pOldNode || !pOldNode->GetNodes().IsDocNodes() ||
        pOldNode != static_cast<SwNode const *>(pNode))
    {
        oPos.emplace( *pNode, GetStart() );
    }
    else
    {
        oPos.emplace( *pOldNode, pOldNode->GetContentNode(), GetStart() );
    }

    aAnchor.SetType( RndStdIds::FLY_AS_CHAR );        // default!
    aAnchor.SetAnchor( &*oPos );

    // in case of anchor change, delete all FlyFrames
    // JP 25.04.95: if the Frames can be moved within SplitNode, they don't
    //              need to be deleted
    if( ( !pNode->GetpSwpHints() || !pNode->GetpSwpHints()->IsInSplitNode() )
        && RES_DRAWFRMFMT != pFormat->Which() )
        pFormat->DelFrames();

    // copy into a different document?
    if( &rDoc != &pFormat->GetDoc() )
    {
        // disable undo while copying attribute
        ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
        SwFrameFormat* pNew = rDoc.getIDocumentLayoutAccess().CopyLayoutFormat( *pFormat, aAnchor, false, false );

        ::sw::UndoGuard const undoGuardFormat(
            pFormat->GetDoc().GetIDocumentUndoRedo());
        pFormat->GetDoc().getIDocumentLayoutAccess().DelLayoutFormat( pFormat );
        const_cast<SwFormatFlyCnt&>(GetFlyCnt()).SetFlyFormat( pNew );
    }
    else if( pNode->GetpSwpHints() &&
            pNode->GetpSwpHints()->IsInSplitNode() &&
            RES_DRAWFRMFMT != pFormat->Which() )
    {
        pFormat->LockModify();
        pFormat->SetFormatAttr( aAnchor );  // only set the anchor
        // tdf#91228 must notify the anchor nodes despite LockModify
        assert(pOldNode);
        pOldNode->RemoveAnchoredFly(pFormat);
        oPos->GetNode().AddAnchoredFly(pFormat);
        pFormat->UnlockModify();
    }
    else
    {
        assert(!pFormat->IsModifyLocked()); // need to notify anchor node
        if (RES_DRAWFRMFMT == pFormat->Which())
        {
            if (SdrObject const*const pObj = pFormat->FindSdrObject())
            {   // tdf#123259 disconnect with *old* anchor position
                if (SwDrawContact* pContact = static_cast<SwDrawContact*>(::GetUserCall( pObj )))
                    pContact->DisconnectFromLayout(false);
            }
        }
        pFormat->SetFormatAttr( aAnchor );  // only set the anchor

        // If the draw format has a TextBox, then set its anchor as well.
        if (SwFrameFormat* pTextBox
            = SwTextBoxHelper::getOtherTextBoxFormat(pFormat, RES_DRAWFRMFMT))
        {
            SwFormatAnchor aTextBoxAnchor(pTextBox->GetAnchor());
            aTextBoxAnchor.SetAnchor(aAnchor.GetContentAnchor());

            // SwFlyAtContentFrame::SwClientNotify() assumes the anchor has a matching layout frame, which
            // may not be the case when we're in the process of a node split, so block
            // notifications.
            bool bIsInSplitNode = pNode->GetpSwpHints() && pNode->GetpSwpHints()->IsInSplitNode();
            if (bIsInSplitNode)
            {
                pTextBox->LockModify();
            }
            else
            {
                // Otherwise delete fly frames on anchor change.
                pTextBox->DelFrames();
            }

            pTextBox->SetFormatAttr(aTextBoxAnchor);

            if (bIsInSplitNode)
            {
                pOldNode->RemoveAnchoredFly(pTextBox);
                oPos->GetNode().AddAnchoredFly(pTextBox);
                pTextBox->UnlockModify();
            }
            else
            {
                pTextBox->MakeFrames();
            }
        }
    }

    // The node may have several SwTextFrames - for every SwTextFrame a
    // SwFlyInContentFrame is created.
}


/** GetFlyFrame_() is called during text formatting by SwTextFormatter
 *  and searches for the SwFlyFrame for the dummy char of the current
 *  SwTextFrame.  If none is found, a new SwFlyInContentFrame is created.
 */
SwFlyInContentFrame *SwTextFlyCnt::GetFlyFrame_( const SwFrame *pCurrFrame )
{
    SwFrameFormat* pFrameFormat = GetFlyCnt().GetFrameFormat();
    if( RES_DRAWFRMFMT == pFrameFormat->Which() )
    {
        OSL_ENSURE(  false, "SwTextFlyCnt::GetFlyFrame_: DrawInCnt-under construction!" );
        return nullptr;
    }

    SwIterator<SwFlyFrame,SwFormat> aIter( *GetFlyCnt().m_pFormat );
    assert(pCurrFrame->IsTextFrame());
    SwFrame* pFrame = aIter.First();
    if ( pFrame )
    {
        SwTextFrame *pFirst = const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pCurrFrame));
        while ( pFirst->IsFollow() )
            pFirst = pFirst->FindMaster();
        do
        {
                SwTextFrame *pTmp = pFirst;
                do
                {   if( static_cast<SwFlyFrame*>(pFrame)->GetAnchorFrame() == static_cast<SwFrame*>(pTmp) )
                    {
                        if ( pTmp != pCurrFrame )
                        {
                            pTmp->RemoveFly( static_cast<SwFlyFrame*>(pFrame) );
                            const_cast<SwTextFrame*>(static_cast<const SwTextFrame*>(pCurrFrame))->AppendFly( static_cast<SwFlyFrame*>(pFrame) );
                        }
                        return static_cast<SwFlyInContentFrame*>(pFrame);
                    }
                    pTmp = pTmp->GetFollow();
                } while ( pTmp );

                pFrame = aIter.Next();

        } while( pFrame );
    }

    // We did not find a matching FlyFrame, so create a new one.
    // AppendFly() triggers a reformat of pCurrentFrame.  However, the
    // recursion is blocked by the lock mechanism in SwTextFrame::Format().
    SwFrame* pCurrentFrame = const_cast<SwFrame*>(pCurrFrame);
    SwFlyInContentFrame *pFly = new SwFlyInContentFrame(static_cast<SwFlyFrameFormat*>(pFrameFormat), pCurrentFrame, pCurrentFrame);
    pCurrentFrame->AppendFly(pFly);
    pFly->RegistFlys();

    // We must ensure that the content of the FlyInCnt is fully formatted
    // right after construction.
    // #i26945# - Use new object formatter to format Writer
    // fly frame and its content.
    SwObjectFormatter::FormatObj( *pFly, const_cast<SwFrame*>(pCurrFrame),
                                  pCurrFrame->FindPageFrame() );

    return pFly;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
