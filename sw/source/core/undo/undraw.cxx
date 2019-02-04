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

#include <rtl/string.h>

#include <svx/svdogrp.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmark.hxx>

#include <hintids.hxx>
#include <hints.hxx>
#include <fmtanchr.hxx>
#include <fmtflcnt.hxx>
#include <txtflcnt.hxx>
#include <frmfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <docary.hxx>
#include <frame.hxx>
#include <swundo.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <UndoCore.hxx>
#include <dcontact.hxx>
#include <dview.hxx>
#include <rootfrm.hxx>
#include <viewsh.hxx>

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

        GetIDocumentUndoRedo().AppendUndo( std::make_unique<SwSdrUndo>(std::move(pUndo), pMarkList, this) );
    }
}

SwSdrUndo::SwSdrUndo( std::unique_ptr<SdrUndoAction> pUndo, const SdrMarkList* pMrkLst, const SwDoc* pDoc )
    : SwUndo( SwUndoId::DRAWUNDO, pDoc ), pSdrUndo( std::move(pUndo) )
{
    if( pMrkLst && pMrkLst->GetMarkCount() )
        pMarkList.reset( new SdrMarkList( *pMrkLst ) );
}

SwSdrUndo::~SwSdrUndo()
{
    pSdrUndo.reset();
    pMarkList.reset();
}

void SwSdrUndo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Undo();
    rContext.SetSelections(nullptr, pMarkList.get());
}

void SwSdrUndo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Redo();
    rContext.SetSelections(nullptr, pMarkList.get());
}

OUString SwSdrUndo::GetComment() const
{
    return pSdrUndo->GetComment();
}

static void lcl_SendRemoveToUno( SwFormat& rFormat )
{
    SwPtrMsgPoolItem aMsgHint( RES_REMOVE_UNO_OBJECT, &rFormat );
    rFormat.ModifyNotification( &aMsgHint, &aMsgHint );
}

static void lcl_SaveAnchor( SwFrameFormat* pFormat, sal_uLong& rNodePos )
{
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if ((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId()))
    {
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
}

static void lcl_RestoreAnchor( SwFrameFormat* pFormat, sal_uLong nNodePos )
{
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if ((RndStdIds::FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId()))
    {
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
        pFormat->SetFormatAttr( aTmp );

        if (RndStdIds::FLY_AS_CHAR == rAnchor.GetAnchorId())
        {
            SwTextNode *pTextNd = aIdx.GetNode().GetTextNode();
            OSL_ENSURE( pTextNd, "no Text Node" );
            SwFormatFlyCnt aFormat( pFormat );
            pTextNd->InsertItem( aFormat, nContentPos, nContentPos );
        }
    }
}

SwUndoDrawGroup::SwUndoDrawGroup( sal_uInt16 nCnt, const SwDoc* pDoc )
    : SwUndo( SwUndoId::DRAWGROUP, pDoc ), nSize( nCnt + 1 ), bDelFormat( true )
{
    pObjArr.reset( new SwUndoGroupObjImpl[ nSize ] );
}

SwUndoDrawGroup::~SwUndoDrawGroup()
{
    if( bDelFormat )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr.get() + 1;
        for( sal_uInt16 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    else
        delete pObjArr[0].pFormat;
}

void SwUndoDrawGroup::UndoImpl(::sw::UndoRedoContext &)
{
    bDelFormat = false;

    // save group object
    SwDrawFrameFormat* pFormat = pObjArr[0].pFormat;

    pFormat->CallSwClientNotify(sw::ContactChangedHint(&pObjArr[0].pObj));
    auto pObj = pObjArr[0].pObj;
    pObj->SetUserCall(nullptr);

    ::lcl_SaveAnchor( pFormat, pObjArr[0].nNodeIdx );

    // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = pObjArr[n];

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
    bDelFormat = true;

    // remove from array
    SwDoc* pDoc = pObjArr[0].pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = pObjArr[n];

        SdrObject* pObj = rSave.pObj;

        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

        // object will destroy itself
        pContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
        pObj->SetUserCall( nullptr );

        ::lcl_SaveAnchor( rSave.pFormat, rSave.nNodeIdx );

        // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *rSave.pFormat );

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), rSave.pFormat ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( pObjArr[0].pFormat, pObjArr[0].nNodeIdx );
    rFlyFormats.push_back( pObjArr[0].pFormat );

    SwDrawContact *pContact = new SwDrawContact( pObjArr[0].pFormat, pObjArr[0].pObj );
    // #i26791# - correction: connect object to layout
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr[0].pObj );

    SwDrawFrameFormat* pDrawFrameFormat = pObjArr[0].pFormat;

    // #i45952# - notify that position attributes are already set
    OSL_ENSURE(pDrawFrameFormat,
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
    if (pDrawFrameFormat)
        pDrawFrameFormat->PosAttrSet();
}

void SwUndoDrawGroup::AddObj( sal_uInt16 nPos, SwDrawFrameFormat* pFormat, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = pObjArr[nPos + 1];
    rSave.pObj = pObj;
    rSave.pFormat = pFormat;
    ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwFrameFormats& rFlyFormats = *pFormat->GetDoc()->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
}

void SwUndoDrawGroup::SetGroupFormat( SwDrawFrameFormat* pFormat )
{
    pObjArr[0].pObj = nullptr;
    pObjArr[0].pFormat = pFormat;
}

SwUndoDrawUnGroup::SwUndoDrawUnGroup( SdrObjGroup* pObj, const SwDoc* pDoc )
    : SwUndo( SwUndoId::DRAWUNGROUP, pDoc ), bDelFormat( false )
{
    nSize = static_cast<sal_uInt16>(pObj->GetSubList()->GetObjCount()) + 1;
    pObjArr.reset( new SwUndoGroupObjImpl[ nSize ] );

    SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
    SwDrawFrameFormat* pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());

    pObjArr[0].pObj = pObj;
    pObjArr[0].pFormat = pFormat;

    // object will destroy itself
    pContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
    pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, pObjArr[0].nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwFrameFormats& rFlyFormats = *pFormat->GetDoc()->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
}

SwUndoDrawUnGroup::~SwUndoDrawUnGroup()
{
    if( bDelFormat )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr.get() + 1;
        for( sal_uInt16 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    else
        delete pObjArr[0].pFormat;
}

void SwUndoDrawUnGroup::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFormat = true;

    SwDoc *const pDoc = & rContext.GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();

    // remove from array
    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = pObjArr[n];

        ::lcl_SaveAnchor( rSave.pFormat, rSave.nNodeIdx );

           // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *rSave.pFormat );

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), rSave.pFormat ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( pObjArr[0].pFormat, pObjArr[0].nNodeIdx );
    rFlyFormats.push_back( pObjArr[0].pFormat );

    SwDrawContact *pContact = new SwDrawContact( pObjArr[0].pFormat, pObjArr[0].pObj );
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr[0].pObj );

    SwDrawFrameFormat* pDrawFrameFormat = pObjArr[0].pFormat;

    // #i45952# - notify that position attributes are already set
    OSL_ENSURE(pDrawFrameFormat,
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
    if (pDrawFrameFormat)
        pDrawFrameFormat->PosAttrSet();
}

void SwUndoDrawUnGroup::RedoImpl(::sw::UndoRedoContext &)
{
    bDelFormat = false;

    // save group object
    SwDrawFrameFormat* pFormat = pObjArr[0].pFormat;
    pFormat->CallSwClientNotify(sw::ContactChangedHint(&(pObjArr[0].pObj)));
    pObjArr[0].pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, pObjArr[0].nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = pObjArr[n];

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
    SwUndoGroupObjImpl& rSave = pObjArr[ nPos + 1 ];
    rSave.pFormat = pFormat;
    rSave.pObj = nullptr;
}

SwUndoDrawUnGroupConnectToLayout::SwUndoDrawUnGroupConnectToLayout(const SwDoc* pDoc)
    : SwUndo( SwUndoId::DRAWUNGROUP, pDoc )
{
}

SwUndoDrawUnGroupConnectToLayout::~SwUndoDrawUnGroupConnectToLayout()
{
}

void
SwUndoDrawUnGroupConnectToLayout::UndoImpl(::sw::UndoRedoContext &)
{
    for (const std::pair< SwDrawFrameFormat*, SdrObject* > & rPair : aDrawFormatsAndObjs)
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
    for (const std::pair< SwDrawFrameFormat*, SdrObject* > & rPair : aDrawFormatsAndObjs)
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
    aDrawFormatsAndObjs.emplace_back( pDrawFrameFormat, pDrawObject );
}

SwUndoDrawDelete::SwUndoDrawDelete( sal_uInt16 nCnt, const SwDoc* pDoc )
    : SwUndo( SwUndoId::DRAWDELETE, pDoc ), bDelFormat( true )
{
    pObjArr.reset( new SwUndoGroupObjImpl[ nCnt ] );
    pMarkLst.reset( new SdrMarkList() );
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if( bDelFormat )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr.get();
        for( size_t n = 0; n < pMarkLst->GetMarkCount(); ++n, ++pTmp )
            delete pTmp->pFormat;
    }
}

void SwUndoDrawDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFormat = false;
    SwFrameFormats & rFlyFormats = *rContext.GetDoc().GetSpzFrameFormats();
    for( size_t n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = pObjArr[n];
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
    rContext.SetSelections(nullptr, pMarkLst.get());
}

void SwUndoDrawDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFormat = true;
    SwFrameFormats & rFlyFormats = *rContext.GetDoc().GetSpzFrameFormats();
    for( size_t n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = pObjArr[n];
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        SwDrawFrameFormat *pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());

        // object will destroy itself
        pContact->Changed( *pObj, SdrUserCallType::Delete, pObj->GetLastBoundRect() );
        pObj->SetUserCall( nullptr );

           // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *pFormat );

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
        ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );
    }
}

void SwUndoDrawDelete::AddObj( SwDrawFrameFormat* pFormat,
                                const SdrMark& rMark )
{
    SwUndoGroupObjImpl& rSave = pObjArr[ pMarkLst->GetMarkCount() ];
    rSave.pObj = rMark.GetMarkedSdrObj();
    rSave.pFormat = pFormat;
    ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    pMarkLst->InsertEntry( rMark );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
