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
#include <DocumentLayoutManager.hxx>
#include <doc.hxx>
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <DocumentContentOperationsManager.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <undobj.hxx>
#include <viewsh.hxx>
#include <layouter.hxx>
#include <poolfmt.hxx>
#include <frmfmt.hxx>
#include <fmtcntnt.hxx>
#include <fmtcnct.hxx>
#include <ndole.hxx>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <fmtanchr.hxx>
#include <txtflcnt.hxx>
#include <fmtflcnt.hxx>
#include <ndtxt.hxx>
#include <dcontact.hxx>
#include <unoframe.hxx>
#include <docary.hxx>
#include <textboxhelper.hxx>

using namespace ::com::sun::star;

namespace sw
{

DocumentLayoutManager::DocumentLayoutManager( SwDoc& i_rSwdoc ) :
    m_rDoc( i_rSwdoc ),
    mpCurrentView( 0 ),
    mpLayouter( 0 )
{
}

const SwViewShell *DocumentLayoutManager::GetCurrentViewShell() const
{
    return mpCurrentView;
}

SwViewShell *DocumentLayoutManager::GetCurrentViewShell()
{
    return mpCurrentView;
}

void DocumentLayoutManager::SetCurrentViewShell( SwViewShell* pNew )
{
    mpCurrentView = pNew;
}

// It must be able to communicate to a SwViewShell. This is going to be removed later.
const SwRootFrm *DocumentLayoutManager::GetCurrentLayout() const
{
    if(GetCurrentViewShell())
        return GetCurrentViewShell()->GetLayout();
    return 0;
}

SwRootFrm *DocumentLayoutManager::GetCurrentLayout()
{
    if(GetCurrentViewShell())
        return GetCurrentViewShell()->GetLayout();
    return 0;
}

bool DocumentLayoutManager::HasLayout() const
{
    // if there is a view, there is always a layout
    return (mpCurrentView != 0);
}

SwLayouter* DocumentLayoutManager::GetLayouter()
{
    return mpLayouter;
}

const SwLayouter* DocumentLayoutManager::GetLayouter() const
{
    return mpLayouter;
}

void DocumentLayoutManager::SetLayouter( SwLayouter* pNew )
{
    mpLayouter = pNew;
}

/** Create a new format whose settings fit to the Request by default.

    The format is put into the respective format array.
    If there already is a fitting format, it is returned instead. */
SwFrameFormat *DocumentLayoutManager::MakeLayoutFormat( RndStdIds eRequest, const SfxItemSet* pSet )
{
    SwFrameFormat *pFormat = 0;
    const bool bMod = m_rDoc.getIDocumentState().IsModified();
    bool bHeader = false;

    switch ( eRequest )
    {
    case RND_STD_HEADER:
    case RND_STD_HEADERL:
    case RND_STD_HEADERR:
        {
            bHeader = true;
            // no break, we continue further down
        }
    case RND_STD_FOOTER:
    case RND_STD_FOOTERL:
    case RND_STD_FOOTERR:
        {
            pFormat = new SwFrameFormat( m_rDoc.GetAttrPool(),
                                 (bHeader ? "Right header" : "Right footer"),
                                 m_rDoc.GetDfltFrameFormat() );

            SwNodeIndex aTmpIdx( m_rDoc.GetNodes().GetEndOfAutotext() );
            SwStartNode* pSttNd =
                m_rDoc.GetNodes().MakeTextSection
                ( aTmpIdx,
                  bHeader ? SwHeaderStartNode : SwFooterStartNode,
                  m_rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool(static_cast<sal_uInt16>( bHeader
                                     ? ( eRequest == RND_STD_HEADERL
                                         ? RES_POOLCOLL_HEADERL
                                         : eRequest == RND_STD_HEADERR
                                         ? RES_POOLCOLL_HEADERR
                                         : RES_POOLCOLL_HEADER )
                                     : ( eRequest == RND_STD_FOOTERL
                                         ? RES_POOLCOLL_FOOTERL
                                         : eRequest == RND_STD_FOOTERR
                                         ? RES_POOLCOLL_FOOTERR
                                         : RES_POOLCOLL_FOOTER )
                                     ) ) );
            pFormat->SetFormatAttr( SwFormatContent( pSttNd ));

            if( pSet )      // Set a few more attributes
                pFormat->SetFormatAttr( *pSet );

            // Why set it back?  Doc has changed, or not?
            // In any case, wrong for the FlyFrames!
            if ( !bMod )
                m_rDoc.getIDocumentState().ResetModified();
        }
        break;

    case RND_DRAW_OBJECT:
        {
            pFormat = m_rDoc.MakeDrawFrameFormat( OUString(), m_rDoc.GetDfltFrameFormat() );
            if( pSet )      // Set a few more attributes
                pFormat->SetFormatAttr( *pSet );

            if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
            {
                m_rDoc.GetIDocumentUndoRedo().AppendUndo(
                    new SwUndoInsLayFormat(pFormat, 0, 0));
            }
        }
        break;

#if OSL_DEBUG_LEVEL > 0
    case FLY_AT_PAGE:
    case FLY_AT_CHAR:
    case FLY_AT_FLY:
    case FLY_AT_PARA:
    case FLY_AS_CHAR:
        OSL_FAIL( "use new interface instead: SwDoc::MakeFlySection!" );
        break;
#endif

    default:
        OSL_ENSURE( false,
                "LayoutFormat was requested with an invalid Request." );

    }
    return pFormat;
}

/// Deletes the denoted format and its content.
void DocumentLayoutManager::DelLayoutFormat( SwFrameFormat *pFormat )
{
    // A chain of frames needs to be merged, if necessary,
    // so that the Frame's contents are adjusted accordingly before we destroy the Frames.
    const SwFormatChain &rChain = pFormat->GetChain();
    if ( rChain.GetPrev() )
    {
        SwFormatChain aChain( rChain.GetPrev()->GetChain() );
        aChain.SetNext( rChain.GetNext() );
        m_rDoc.SetAttr( aChain, *rChain.GetPrev() );
    }
    if ( rChain.GetNext() )
    {
        SwFormatChain aChain( rChain.GetNext()->GetChain() );
        aChain.SetPrev( rChain.GetPrev() );
        m_rDoc.SetAttr( aChain, *rChain.GetNext() );
    }

    const SwNodeIndex* pCntIdx = 0;
    // The draw format doesn't own its content, it just has a pointer to it.
    if (pFormat->Which() != RES_DRAWFRMFMT)
        pCntIdx = pFormat->GetContent().GetContentIdx();
    if (pCntIdx && !m_rDoc.GetIDocumentUndoRedo().DoesUndo())
    {
        // Disconnect if it's an OLE object
        SwOLENode* pOLENd = m_rDoc.GetNodes()[ pCntIdx->GetIndex()+1 ]->GetOLENode();
        if( pOLENd && pOLENd->GetOLEObj().IsOleRef() )
        {

            // TODO: the old object closed the object and cleared all references to it, but didn't remove it from the container.
            // I have no idea, why, nobody could explain it - so I do my very best to mimic this behavior
            //uno::Reference < util::XCloseable > xClose( pOLENd->GetOLEObj().GetOleRef(), uno::UNO_QUERY );
            //if ( xClose.is() )
            {
                try
                {
                    pOLENd->GetOLEObj().GetOleRef()->changeState( embed::EmbedStates::LOADED );
                }
                catch ( uno::Exception& )
                {
                }
            }

        }
    }

    // Destroy Frames
    pFormat->DelFrms();

    // Only FlyFrames are undoable at first
    const sal_uInt16 nWh = pFormat->Which();
    if (m_rDoc.GetIDocumentUndoRedo().DoesUndo() &&
        (RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh))
    {
        m_rDoc.GetIDocumentUndoRedo().AppendUndo( new SwUndoDelLayFormat( pFormat ));
    }
    else
    {
        // #i32089# - delete at-frame anchored objects
        if ( nWh == RES_FLYFRMFMT )
        {
            // determine frame formats of at-frame anchored objects
            const SwNodeIndex* pContentIdx = 0;
            if (pFormat->Which() != RES_DRAWFRMFMT)
                pContentIdx = pFormat->GetContent().GetContentIdx();
            if (pContentIdx)
            {
                const SwFrameFormats* pTable = pFormat->GetDoc()->GetSpzFrameFormats();
                if ( pTable )
                {
                    std::vector<SwFrameFormat*> aToDeleteFrameFormats;
                    const sal_uLong nNodeIdxOfFlyFormat( pContentIdx->GetIndex() );

                    for ( size_t i = 0; i < pTable->size(); ++i )
                    {
                        SwFrameFormat* pTmpFormat = (*pTable)[i];
                        const SwFormatAnchor &rAnch = pTmpFormat->GetAnchor();
                        if ( rAnch.GetAnchorId() == FLY_AT_FLY &&
                             rAnch.GetContentAnchor()->nNode.GetIndex() == nNodeIdxOfFlyFormat )
                        {
                            aToDeleteFrameFormats.push_back( pTmpFormat );
                        }
                    }

                    // delete found frame formats
                    while ( !aToDeleteFrameFormats.empty() )
                    {
                        SwFrameFormat* pTmpFormat = aToDeleteFrameFormats.back();
                        pFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat( pTmpFormat );

                        aToDeleteFrameFormats.pop_back();
                    }
                }
            }
        }

        // Delete content
        if( pCntIdx )
        {
            SwNode *pNode = &pCntIdx->GetNode();
            const_cast<SwFormatContent&>(static_cast<const SwFormatContent&>(pFormat->GetFormatAttr( RES_CNTNT ))).SetNewContentIdx( 0 );
            m_rDoc.getIDocumentContentOperations().DeleteSection( pNode );
        }

        // Delete the character for FlyFrames anchored as char (if necessary)
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) && rAnchor.GetContentAnchor())
        {
            const SwPosition* pPos = rAnchor.GetContentAnchor();
            SwTextNode *pTextNd = pPos->nNode.GetNode().GetTextNode();

            // attribute is still in text node, delete it
            if ( pTextNd )
            {
                SwTextFlyCnt* const pAttr = static_cast<SwTextFlyCnt*>(
                    pTextNd->GetTextAttrForCharAt( pPos->nContent.GetIndex(),
                        RES_TXTATR_FLYCNT ));
                if ( pAttr && (pAttr->GetFlyCnt().GetFrameFormat() == pFormat) )
                {
                    // don't delete, set pointer to 0
                    const_cast<SwFormatFlyCnt&>(pAttr->GetFlyCnt()).SetFlyFormat();
                    SwIndex aIdx( pPos->nContent );
                    pTextNd->EraseText( aIdx, 1 );
                }
            }
        }

        m_rDoc.DelFrameFormat( pFormat );
    }
    m_rDoc.getIDocumentState().SetModified();
}

/** Copies the stated format (pSrc) to pDest and returns pDest.

    If there's no pDest, it is created.
    If the source format is located in another document, also copy correctly
    in this case.
    The Anchor attribute's position is always set to 0! */
SwFrameFormat *DocumentLayoutManager::CopyLayoutFormat(
    const SwFrameFormat& rSource,
    const SwFormatAnchor& rNewAnchor,
    bool bSetTextFlyAtt,
    bool bMakeFrms )
{
    const bool bFly = RES_FLYFRMFMT == rSource.Which();
    const bool bDraw = RES_DRAWFRMFMT == rSource.Which();
    OSL_ENSURE( bFly || bDraw, "this method only works for fly or draw" );

    SwDoc* pSrcDoc = const_cast<SwDoc*>(rSource.GetDoc());

    // May we copy this object?
    // We may, unless it's 1) it's a control (and therefore a draw)
    //                     2) anchored in a header/footer
    //                     3) anchored (to paragraph?)
    bool bMayNotCopy = false;
    if( bDraw )
    {
        const SwDrawContact* pDrawContact =
            static_cast<const SwDrawContact*>( rSource.FindContactObj() );

        bMayNotCopy =
            ((FLY_AT_PARA == rNewAnchor.GetAnchorId()) ||
             (FLY_AT_FLY  == rNewAnchor.GetAnchorId()) ||
             (FLY_AT_CHAR == rNewAnchor.GetAnchorId())) &&
            rNewAnchor.GetContentAnchor() &&
            m_rDoc.IsInHeaderFooter( rNewAnchor.GetContentAnchor()->nNode ) &&
            pDrawContact != NULL  &&
            pDrawContact->GetMaster() != NULL  &&
            CheckControlLayer( pDrawContact->GetMaster() );
    }

    // just return if we can't copy this
    if( bMayNotCopy )
        return NULL;

    SwFrameFormat* pDest = m_rDoc.GetDfltFrameFormat();
    if( rSource.GetRegisteredIn() != pSrcDoc->GetDfltFrameFormat() )
        pDest = m_rDoc.CopyFrameFormat( *static_cast<const SwFrameFormat*>(rSource.GetRegisteredIn()) );
    if( bFly )
    {
        // #i11176#
        // To do a correct cloning concerning the ZOrder for all objects
        // it is necessary to actually create a draw object for fly frames, too.
        // These are then added to the DrawingLayer (which needs to exist).
        // Together with correct sorting of all drawinglayer based objects
        // before cloning ZOrder transfer works correctly then.
        SwFlyFrameFormat *pFormat = m_rDoc.MakeFlyFrameFormat( rSource.GetName(), pDest );
        pDest = pFormat;

        SwXFrame::GetOrCreateSdrObject(*pFormat);
    }
    else
        pDest = m_rDoc.MakeDrawFrameFormat( OUString(), pDest );

    // Copy all other or new attributes
    pDest->CopyAttrs( rSource );

    // Do not copy chains
    pDest->ResetFormatAttr( RES_CHAIN );

    if( bFly )
    {
        // Duplicate the content.
        const SwNode& rCSttNd = rSource.GetContent().GetContentIdx()->GetNode();
        SwNodeRange aRg( rCSttNd, 1, *rCSttNd.EndOfSectionNode() );

        SwNodeIndex aIdx( m_rDoc.GetNodes().GetEndOfAutotext() );
        SwStartNode* pSttNd = SwNodes::MakeEmptySection( aIdx, SwFlyStartNode );

        // Set the Anchor/ContentIndex first.
        // Within the copying part, we can access the values (DrawFormat in Headers and Footers)
        aIdx = *pSttNd;
        SwFormatContent aAttr( rSource.GetContent() );
        aAttr.SetNewContentIdx( &aIdx );
        pDest->SetFormatAttr( aAttr );
        pDest->SetFormatAttr( rNewAnchor );

        if( !m_rDoc.IsCopyIsMove() || &m_rDoc != pSrcDoc )
        {
            if( m_rDoc.IsInReading() || m_rDoc.IsInMailMerge() )
                pDest->SetName( OUString() );
            else
            {
                // Test first if the name is already taken, if so generate a new one.
                sal_Int8 nNdTyp = aRg.aStart.GetNode().GetNodeType();

                OUString sOld( pDest->GetName() );
                pDest->SetName( OUString() );
                if( m_rDoc.FindFlyByName( sOld, nNdTyp ) )     // found one
                    switch( nNdTyp )
                    {
                    case ND_GRFNODE:    sOld = m_rDoc.GetUniqueGrfName();      break;
                    case ND_OLENODE:    sOld = m_rDoc.GetUniqueOLEName();      break;
                    default:            sOld = m_rDoc.GetUniqueFrameName();    break;
                    }

                pDest->SetName( sOld );
            }
        }

        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(new SwUndoInsLayFormat(pDest,0,0));
        }

        // Make sure that FlyFrames in FlyFrames are copied
        aIdx = *pSttNd->EndOfSectionNode();

        //fdo#36631 disable (scoped) any undo operations associated with the
        //contact object itself. They should be managed by SwUndoInsLayFormat.
        const ::sw::DrawUndoGuard drawUndoGuard(m_rDoc.GetIDocumentUndoRedo());

        pSrcDoc->GetDocumentContentOperationsManager().CopyWithFlyInFly( aRg, 0, aIdx, NULL, false, true, true );
    }
    else
    {
        OSL_ENSURE( RES_DRAWFRMFMT == rSource.Which(), "Neither Fly nor Draw." );
        // #i52780# - Note: moving object to visible layer not needed.
        const SwDrawContact* pSourceContact = static_cast<const SwDrawContact *>(rSource.FindContactObj());

        SwDrawContact* pContact = new SwDrawContact( static_cast<SwDrawFrameFormat*>(pDest),
                                m_rDoc.CloneSdrObj( *pSourceContact->GetMaster(),
                                        m_rDoc.IsCopyIsMove() && &m_rDoc == pSrcDoc ) );
        // #i49730# - notify draw frame format that position attributes are
        // already set, if the position attributes are already set at the
        // source draw frame format.
        if ( pDest->ISA(SwDrawFrameFormat) &&
             rSource.ISA(SwDrawFrameFormat) &&
             static_cast<const SwDrawFrameFormat&>(rSource).IsPosAttrSet() )
        {
            static_cast<SwDrawFrameFormat*>(pDest)->PosAttrSet();
        }

        if( pDest->GetAnchor() == rNewAnchor )
        {
            // Do *not* connect to layout, if a <MakeFrms> will not be called.
            if ( bMakeFrms )
            {
                pContact->ConnectToLayout( &rNewAnchor );
            }
        }
        else
            pDest->SetFormatAttr( rNewAnchor );

        if (m_rDoc.GetIDocumentUndoRedo().DoesUndo())
        {
            m_rDoc.GetIDocumentUndoRedo().AppendUndo(new SwUndoInsLayFormat(pDest,0,0));
        }
    }

    if (bSetTextFlyAtt && (FLY_AS_CHAR == rNewAnchor.GetAnchorId()))
    {
        const SwPosition* pPos = rNewAnchor.GetContentAnchor();
        SwFormatFlyCnt aFormat( pDest );
        pPos->nNode.GetNode().GetTextNode()->InsertItem(
            aFormat, pPos->nContent.GetIndex(), 0 );
    }

    if( bMakeFrms )
        pDest->MakeFrms();

    // If the draw format has a TextBox, then copy its fly format as well.
    if (SwFrameFormat* pSourceTextBox = SwTextBoxHelper::findTextBox(&rSource))
    {
        SwFormatAnchor boxAnchor(rNewAnchor);
        if (FLY_AS_CHAR == boxAnchor.GetAnchorId())
        {
            // AS_CHAR *must not* be set on textbox fly-frame
            boxAnchor.SetType(FLY_AT_CHAR);
        }
        // presumably these anchors are supported though not sure
        assert(FLY_AT_CHAR == boxAnchor.GetAnchorId() || FLY_AT_PARA == boxAnchor.GetAnchorId());
        SwFrameFormat* pDestTextBox = CopyLayoutFormat(*pSourceTextBox,
                boxAnchor, bSetTextFlyAtt, bMakeFrms);
        SwAttrSet aSet(pDest->GetAttrSet());
        SwFormatContent aContent(pDestTextBox->GetContent().GetContentIdx()->GetNode().GetStartNode());
        aSet.Put(aContent);
        pDest->SetFormatAttr(aSet);
    }

    return pDest;
}

//Load document from fdo#42534 under valgrind, drag the scrollbar down so full
//document layout is triggered. Close document before layout has completed, and
//SwAnchoredObject objects deleted by the deletion of layout remain referenced
//by the SwLayouter
void DocumentLayoutManager::ClearSwLayouterEntries()
{
    SwLayouter::ClearMovedFwdFrms( m_rDoc );
    SwLayouter::ClearObjsTmpConsiderWrapInfluence( m_rDoc );
    // #i65250#
    SwLayouter::ClearMoveBwdLayoutInfo( m_rDoc );
}

DocumentLayoutManager::~DocumentLayoutManager()
{
    delete mpLayouter;
    mpLayouter = 0L;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

