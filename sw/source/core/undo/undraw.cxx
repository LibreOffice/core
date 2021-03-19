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

#include <UndoDraw.hxx>

#include <svx/svdogrp.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>

#include <hintids.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <txtflcnt.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <dcontact.hxx>
#include <viewsh.hxx>
#include <frameformats.hxx>

struct SwUndoGroupObjImpl
{
    SwDrawFrameFormat* pFormat;
    SdrObject* pObj;
    sal_uLong nNodeIdx;
};

// Draw-Objecte

void SwDoc::AddDrawUndo( std::unique_ptr<SdrUndoAction> pUndo )
{
    if (GetIDocumentUndoRedo().DoesUndo() &&
        GetIDocumentUndoRedo().DoesDrawUndo())
    {
        const SdrMarkList* pMarkList = nullptr;
        SwViewShell* pSh = getIDocumentLayoutAccess().GetCurrentViewShell();
        if( pSh && pSh->HasDrawView() )
            pMarkList = &pSh->GetDrawView()->GetMarkedObjectList();

        GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwSdrUndo>(std::move(pUndo), pMarkList, *this) );
    }
}

SwSdrUndo::SwSdrUndo( std::unique_ptr<SdrUndoAction> pUndo, const SdrMarkList* pMrkLst, const SwDoc& rDoc )
    : SwUndo( SwUndoId::DRAWUNDO, &rDoc ), m_pSdrUndo( std::move(pUndo) )
{
    if( pMrkLst && pMrkLst->GetMarkCount() )
        m_pMarkList.reset( new SdrMarkList( *pMrkLst ) );
}

SwSdrUndo::~SwSdrUndo()
{
    m_pSdrUndo.reset();
    m_pMarkList.reset();
}

void SwSdrUndo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_pSdrUndo->Undo();
    rContext.SetSelections(nullptr, m_pMarkList.get());
}

void SwSdrUndo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    m_pSdrUndo->Redo();
    rContext.SetSelections(nullptr, m_pMarkList.get());
}

OUString SwSdrUndo::GetComment() const
{
    return m_pSdrUndo->GetComment();
}

static void lcl_SaveAnchor( SwFrameFormat* pFormat, sal_uLong& rNodePos )
{
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if (!((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId())))
        return;

    rNodePos = rAnchor.GetContentAnchor()->nNode.GetIndex();
    sal_Int32 nContentPos = 0;

    if (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId())
    {
        nContentPos = rAnchor.GetContentAnchor()->nContent.GetIndex();

        // destroy TextAttribute
        SwTextNode *pTextNd = pFormat->GetDoc()->GetNodes()[ rNodePos ]->GetTextNode();
        OSL_ENSURE( pTextNd, "No text node found!" );
        SwTextFlyCnt* pAttr = static_cast<SwTextFlyCnt*>(
            pTextNd->GetTextAttrForCharAt( nContentPos, RES_TXTATR_FLYCNT ));
        // attribute still in text node, delete
        if( pAttr && pAttr->GetFlyCnt().GetFrameFormat() == pFormat )
        {
            // just set pointer to 0, don't delete
            const_cast<SwFormatFlyCnt&>(pAttr->GetFlyCnt()).SetFlyFormat();
            SwIndex aIdx( pTextNd, nContentPos );
            pTextNd->EraseText( aIdx, 1 );
        }
    }
    else if (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId())
    {
        nContentPos = rAnchor.GetContentAnchor()->nContent.GetIndex();
    }

    pFormat->SetFormatAttr( SwFormatAnchor( rAnchor.GetAnchorId(), nContentPos ) );
}

static void lcl_RestoreAnchor( SwFrameFormat* pFormat, sal_uLong nNodePos )
{
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if (!((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId())))
        return;

    const sal_Int32 nContentPos = rAnchor.GetPageNum();
    SwNodes& rNds = pFormat->GetDoc()->GetNodes();

    SwNodeIndex aIdx( rNds, nNodePos );
    SwPosition aPos( aIdx );

    SwFormatAnchor aTmp( rAnchor.GetAnchorId() );
    if ((RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()))
    {
        aPos.nContent.Assign( aIdx.GetNode().GetContentNode(), nContentPos );
    }
    aTmp.SetAnchor( &aPos );
    RndStdIds nAnchorId = rAnchor.GetAnchorId();
    pFormat->SetFormatAttr( aTmp );

    if (RndStdIds::FLY_AS_CHAR == nAnchorId)
    {
        SwTextNode *pTextNd = aIdx.GetNode().GetTextNode();
        OSL_ENSURE( pTextNd, "no Text Node" );
        SwFormatFlyCnt aFormat( pFormat );
        pTextNd->InsertItem( aFormat, nContentPos, nContentPos );
    }
}

SwUndoDrawGroup::SwUndoDrawGroup( sal_uInt16 nCnt, const SwDoc& rDoc )
    : SwUndo( SwUndoId::DRAWGROUP, &rDoc ), m_nSize( nCnt + 1 ), m_bDeleteFormat( true )
{
    m_pObjArray.reset( new SwUndoGroupObjImpl[ m_nSize ] );
}

SwUndoDrawGroup::~SwUndoDrawGroup()
{
    if( m_bDeleteFormat )
    {
        SwUndoGroupObjImpl* pTmp = m_pObjArray.get() + 1;
        for( sal_uInt16 n = 1; n < m_nSize; ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    else
        delete m_pObjArray[0].pFormat;
}

void SwUndoDrawGroup::UndoImpl(::sw::UndoRedoContext &)
{
    m_bDeleteFormat = false;

    // save group object
    SwDrawFrameFormat* pFormat = m_pObjArray[0].pFormat;

    pFormat->CallSwClientNotify(sw::ContactChangedHint(&m_pObjArray[0].pObj));
    auto pObj = m_pObjArray[0].pObj;
    pObj->SetUserCall(nullptr);

    ::lcl_SaveAnchor( pFormat, m_pObjArray[0].nNodeIdx );

    pFormat->RemoveAllUnos();

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    for( sal_uInt16 n = 1; n < m_nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = m_pObjArray[n];

        ::lcl_RestoreAnchor( rSave.pFormat, rSave.nNodeIdx );
        rFlyFormats.push_back( rSave.pFormat );

        pObj = rSave.pObj;

        SwDrawContact *pContact = new SwDrawContact( rSave.pFormat, pObj );
        pContact->ConnectToLayout();
        // #i45718# - follow-up of #i35635# move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );

        SwDrawFrameFormat* pDrawFrameFormat = rSave.pFormat;

        // #i45952# - notify that position attributes are already set
        OSL_ENSURE(pDrawFrameFormat,
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
        if (pDrawFrameFormat)
            pDrawFrameFormat->PosAttrSet();
    }
}

void SwUndoDrawGroup::RedoImpl(::sw::UndoRedoContext &)
{
    m_bDeleteFormat = true;

    // remove from array
    SwDoc* pDoc = m_pObjArray[0].pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();

    for( sal_uInt16 n = 1; n < m_nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = m_pObjArray[n];

        SdrObject* pObj = rSave.pObj;

        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

        // object will destroy itself
        pContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
        pObj->SetUserCall( nullptr );

        ::lcl_SaveAnchor( rSave.pFormat, rSave.nNodeIdx );

        rSave.pFormat->RemoveAllUnos();

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), rSave.pFormat ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( m_pObjArray[0].pFormat, m_pObjArray[0].nNodeIdx );
    rFlyFormats.push_back( m_pObjArray[0].pFormat );

    SwDrawContact *pContact = new SwDrawContact( m_pObjArray[0].pFormat, m_pObjArray[0].pObj );
    // #i26791# - correction: connect object to layout
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( m_pObjArray[0].pObj );

    SwDrawFrameFormat* pDrawFrameFormat = m_pObjArray[0].pFormat;

    // #i45952# - notify that position attributes are already set
    OSL_ENSURE(pDrawFrameFormat,
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
    if (pDrawFrameFormat)
        pDrawFrameFormat->PosAttrSet();
}

void SwUndoDrawGroup::AddObj( sal_uInt16 nPos, SwDrawFrameFormat* pFormat, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = m_pObjArray[nPos + 1];
    rSave.pObj = pObj;
    rSave.pFormat = pFormat;
    ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );

    pFormat->RemoveAllUnos();

    // remove from array
    SwFrameFormats& rFlyFormats = *pFormat->GetDoc()->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
}

void SwUndoDrawGroup::SetGroupFormat( SwDrawFrameFormat* pFormat )
{
    m_pObjArray[0].pObj = nullptr;
    m_pObjArray[0].pFormat = pFormat;
}

SwUndoDrawUnGroup::SwUndoDrawUnGroup( SdrObjGroup* pObj, const SwDoc& rDoc )
    : SwUndo( SwUndoId::DRAWUNGROUP, &rDoc ), m_bDeleteFormat( false )
{
    m_nSize = static_cast<sal_uInt16>(pObj->GetSubList()->GetObjCount()) + 1;
    m_pObjArray.reset( new SwUndoGroupObjImpl[ m_nSize ] );

    SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
    SwDrawFrameFormat* pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());

    m_pObjArray[0].pObj = pObj;
    m_pObjArray[0].pFormat = pFormat;

    // object will destroy itself
    pContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
    pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, m_pObjArray[0].nNodeIdx );

    pFormat->RemoveAllUnos();

    // remove from array
    SwFrameFormats& rFlyFormats = *pFormat->GetDoc()->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
}

SwUndoDrawUnGroup::~SwUndoDrawUnGroup()
{
    if( m_bDeleteFormat )
    {
        SwUndoGroupObjImpl* pTmp = m_pObjArray.get() + 1;
        for( sal_uInt16 n = 1; n < m_nSize; ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    else
        delete m_pObjArray[0].pFormat;
}

void SwUndoDrawUnGroup::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_bDeleteFormat = true;

    SwDoc *const pDoc = & rContext.GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();

    // remove from array
    for( sal_uInt16 n = 1; n < m_nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = m_pObjArray[n];

        ::lcl_SaveAnchor( rSave.pFormat, rSave.nNodeIdx );

        rSave.pFormat->RemoveAllUnos();

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), rSave.pFormat ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( m_pObjArray[0].pFormat, m_pObjArray[0].nNodeIdx );
    rFlyFormats.push_back( m_pObjArray[0].pFormat );

    SwDrawContact *pContact = new SwDrawContact( m_pObjArray[0].pFormat, m_pObjArray[0].pObj );
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( m_pObjArray[0].pObj );

    SwDrawFrameFormat* pDrawFrameFormat = m_pObjArray[0].pFormat;

    // #i45952# - notify that position attributes are already set
    OSL_ENSURE(pDrawFrameFormat,
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
    if (pDrawFrameFormat)
        pDrawFrameFormat->PosAttrSet();
}

void SwUndoDrawUnGroup::RedoImpl(::sw::UndoRedoContext &)
{
    m_bDeleteFormat = false;

    // save group object
    SwDrawFrameFormat* pFormat = m_pObjArray[0].pFormat;
    pFormat->CallSwClientNotify(sw::ContactChangedHint(&(m_pObjArray[0].pObj)));
    m_pObjArray[0].pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, m_pObjArray[0].nNodeIdx );

    pFormat->RemoveAllUnos();

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    for( sal_uInt16 n = 1; n < m_nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = m_pObjArray[n];

        ::lcl_RestoreAnchor( rSave.pFormat, rSave.nNodeIdx );
        rFlyFormats.push_back( rSave.pFormat );

        SwDrawFrameFormat* pDrawFrameFormat = rSave.pFormat;

        // #i45952# - notify that position attributes are already set
        OSL_ENSURE(pDrawFrameFormat,
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if (pDrawFrameFormat)
            pDrawFrameFormat->PosAttrSet();
    }
}

void SwUndoDrawUnGroup::AddObj( sal_uInt16 nPos, SwDrawFrameFormat* pFormat )
{
    SwUndoGroupObjImpl& rSave = m_pObjArray[ nPos + 1 ];
    rSave.pFormat = pFormat;
    rSave.pObj = nullptr;
}

SwUndoDrawUnGroupConnectToLayout::SwUndoDrawUnGroupConnectToLayout(const SwDoc& rDoc)
    : SwUndo( SwUndoId::DRAWUNGROUP, &rDoc )
{
}

SwUndoDrawUnGroupConnectToLayout::~SwUndoDrawUnGroupConnectToLayout()
{
}

void
SwUndoDrawUnGroupConnectToLayout::UndoImpl(::sw::UndoRedoContext &)
{
    for (const std::pair< SwDrawFrameFormat*, SdrObject* > & rPair : m_aDrawFormatsAndObjs)
    {
        SdrObject* pObj( rPair.second );
        SwDrawContact* pDrawContact( dynamic_cast<SwDrawContact*>(pObj->GetUserCall()) );
        OSL_ENSURE( pDrawContact,
                "<SwUndoDrawUnGroupConnectToLayout::Undo(..)> -- missing SwDrawContact instance" );
        if ( pDrawContact )
        {
            // deletion of instance <pDrawContact> and thus disconnection from
            // the Writer layout.
            pDrawContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
            pObj->SetUserCall( nullptr );
        }
    }
}

void
SwUndoDrawUnGroupConnectToLayout::RedoImpl(::sw::UndoRedoContext &)
{
    for (const std::pair< SwDrawFrameFormat*, SdrObject* > & rPair : m_aDrawFormatsAndObjs)
    {
        SwDrawFrameFormat* pFormat( rPair.first );
        SdrObject* pObj( rPair.second );
        SwDrawContact *pContact = new SwDrawContact( pFormat, pObj );
        pContact->ConnectToLayout();
        pContact->MoveObjToVisibleLayer( pObj );
    }
}

void SwUndoDrawUnGroupConnectToLayout::AddFormatAndObj( SwDrawFrameFormat* pDrawFrameFormat,
                                                     SdrObject* pDrawObject )
{
    m_aDrawFormatsAndObjs.emplace_back( pDrawFrameFormat, pDrawObject );
}

SwUndoDrawDelete::SwUndoDrawDelete( sal_uInt16 nCnt, const SwDoc& rDoc )
    : SwUndo( SwUndoId::DRAWDELETE, &rDoc ), m_bDeleteFormat( true )
{
    m_pObjArray.reset( new SwUndoGroupObjImpl[ nCnt ] );
    m_pMarkList.reset( new SdrMarkList() );
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if( m_bDeleteFormat )
    {
        SwUndoGroupObjImpl* pTmp = m_pObjArray.get();
        for( size_t n = 0; n < m_pMarkList->GetMarkCount(); ++n, ++pTmp )
            delete pTmp->pFormat;
    }
}

void SwUndoDrawDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_bDeleteFormat = false;
    SwFrameFormats & rFlyFormats = *rContext.GetDoc().GetSpzFrameFormats();
    for( size_t n = 0; n < m_pMarkList->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = m_pObjArray[n];
        ::lcl_RestoreAnchor( rSave.pFormat, rSave.nNodeIdx );
        rFlyFormats.push_back( rSave.pFormat );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = new SwDrawContact( rSave.pFormat, pObj );
        pContact->Changed_( *pObj, SdrUserCallType::Inserted, nullptr );
        // #i45718# - follow-up of #i35635# move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );

        SwDrawFrameFormat* pDrawFrameFormat = rSave.pFormat;

        // #i45952# - notify that position attributes are already set
        OSL_ENSURE(pDrawFrameFormat,
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
        if (pDrawFrameFormat)
            pDrawFrameFormat->PosAttrSet();
    }
    rContext.SetSelections(nullptr, m_pMarkList.get());
}

void SwUndoDrawDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    m_bDeleteFormat = true;
    SwFrameFormats & rFlyFormats = *rContext.GetDoc().GetSpzFrameFormats();
    for( size_t n = 0; n < m_pMarkList->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = m_pObjArray[n];
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        SwDrawFrameFormat *pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());

        // object will destroy itself
        pContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
        pObj->SetUserCall( nullptr );

        pFormat->RemoveAllUnos();

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
        ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );
    }
}

void SwUndoDrawDelete::AddObj( SwDrawFrameFormat* pFormat,
                                const SdrMark& rMark )
{
    SwUndoGroupObjImpl& rSave = m_pObjArray[ m_pMarkList->GetMarkCount() ];
    rSave.pObj = rMark.GetMarkedSdrObj();
    rSave.pFormat = pFormat;
    ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );

    pFormat->RemoveAllUnos();

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    m_pMarkList->InsertEntry( rMark );
}

void SwUndoDrawDelete::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUndoDrawDelete"));

    for (size_t i = 0; i < m_pMarkList->GetMarkCount(); ++i)
    {
        SwUndoGroupObjImpl& rObj = m_pObjArray[i];
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUndoGroupObjImpl"));
        (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("index"),
                                    BAD_CAST(OString::number(i).getStr()));

        if (rObj.pFormat)
        {
            (void)xmlTextWriterStartElement(pWriter, BAD_CAST("pFormat"));
            rObj.pFormat->dumpAsXml(pWriter);
            (void)xmlTextWriterEndElement(pWriter);
        }
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
