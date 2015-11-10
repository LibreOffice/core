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

IMPL_LINK_TYPED( SwDoc, AddDrawUndo, SdrUndoAction *, pUndo, void )
{
#if OSL_DEBUG_LEVEL > 1
    SAL_INFO("sw.core", "Id: " << pUndo->GetId() << "Comment: " << pUndo->GetComment());
#endif

    if (GetIDocumentUndoRedo().DoesUndo() &&
        GetIDocumentUndoRedo().DoesDrawUndo())
    {
        const SdrMarkList* pMarkList = nullptr;
        SwViewShell* pSh = getIDocumentLayoutAccess().GetCurrentViewShell();
        if( pSh && pSh->HasDrawView() )
            pMarkList = &pSh->GetDrawView()->GetMarkedObjectList();

        GetIDocumentUndoRedo().AppendUndo( new SwSdrUndo(pUndo, pMarkList) );
    }
    else
        delete pUndo;
}

SwSdrUndo::SwSdrUndo( SdrUndoAction* pUndo, const SdrMarkList* pMrkLst )
    : SwUndo( UNDO_DRAWUNDO ), pSdrUndo( pUndo )
{
    if( pMrkLst && pMrkLst->GetMarkCount() )
        pMarkList = new SdrMarkList( *pMrkLst );
    else
        pMarkList = nullptr;
}

SwSdrUndo::~SwSdrUndo()
{
    delete pSdrUndo;
    delete pMarkList;
}

void SwSdrUndo::UndoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Undo();
    rContext.SetSelections(nullptr, pMarkList);
}

void SwSdrUndo::RedoImpl(::sw::UndoRedoContext & rContext)
{
    pSdrUndo->Redo();
    rContext.SetSelections(nullptr, pMarkList);
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
    if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (FLY_AS_CHAR == rAnchor.GetAnchorId()))
    {
        rNodePos = rAnchor.GetContentAnchor()->nNode.GetIndex();
        sal_Int32 nContentPos = 0;

        if (FLY_AS_CHAR == rAnchor.GetAnchorId())
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
        else if (FLY_AT_CHAR == rAnchor.GetAnchorId())
        {
            nContentPos = rAnchor.GetContentAnchor()->nContent.GetIndex();
        }

        pFormat->SetFormatAttr( SwFormatAnchor( rAnchor.GetAnchorId(), nContentPos ) );
    }
}

static void lcl_RestoreAnchor( SwFrameFormat* pFormat, sal_uLong& rNodePos )
{
    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    if ((FLY_AT_PARA == rAnchor.GetAnchorId()) ||
        (FLY_AT_CHAR == rAnchor.GetAnchorId()) ||
        (FLY_AT_FLY  == rAnchor.GetAnchorId()) ||
        (FLY_AS_CHAR == rAnchor.GetAnchorId()))
    {
        const sal_Int32 nContentPos = rAnchor.GetPageNum();
        SwNodes& rNds = pFormat->GetDoc()->GetNodes();

        SwNodeIndex aIdx( rNds, rNodePos );
        SwPosition aPos( aIdx );

        SwFormatAnchor aTmp( rAnchor.GetAnchorId() );
        if ((FLY_AS_CHAR == rAnchor.GetAnchorId()) ||
            (FLY_AT_CHAR == rAnchor.GetAnchorId()))
        {
            aPos.nContent.Assign( aIdx.GetNode().GetContentNode(), nContentPos );
        }
        aTmp.SetAnchor( &aPos );
        pFormat->SetFormatAttr( aTmp );

        if (FLY_AS_CHAR == rAnchor.GetAnchorId())
        {
            SwTextNode *pTextNd = aIdx.GetNode().GetTextNode();
            OSL_ENSURE( pTextNd, "no Text Node" );
            SwFormatFlyCnt aFormat( pFormat );
            pTextNd->InsertItem( aFormat, nContentPos, nContentPos );
        }
    }
}

SwUndoDrawGroup::SwUndoDrawGroup( sal_uInt16 nCnt )
    : SwUndo( UNDO_DRAWGROUP ), nSize( nCnt + 1 ), bDelFormat( true )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
}

SwUndoDrawGroup::~SwUndoDrawGroup()
{
    if( bDelFormat )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( sal_uInt16 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    else
        delete pObjArr->pFormat;

    delete [] pObjArr;
}

void SwUndoDrawGroup::UndoImpl(::sw::UndoRedoContext &)
{
    bDelFormat = false;

    // save group object
    SwDrawFrameFormat* pFormat = pObjArr->pFormat;
    SwDrawContact* pDrawContact = static_cast<SwDrawContact*>(pFormat->FindContactObj());
    SdrObject* pObj = pDrawContact->GetMaster();
    pObjArr->pObj = pObj;

    // object will destroy itself
    pDrawContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
    pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, pObjArr->nNodeIdx );

    // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFormat, rSave.nNodeIdx );
        rFlyFormats.push_back( rSave.pFormat );

        pObj = rSave.pObj;

        SwDrawContact *pContact = new SwDrawContact( rSave.pFormat, pObj );
        pContact->ConnectToLayout();
        // #i45718# - follow-up of #i35635# move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );

        SwDrawFrameFormat* pDrawFrameFormat = dynamic_cast<SwDrawFrameFormat*>(rSave.pFormat);

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
    SwDoc* pDoc = pObjArr->pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        SdrObject* pObj = rSave.pObj;

        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));

        // object will destroy itself
        pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
        pObj->SetUserCall( nullptr );

        ::lcl_SaveAnchor( rSave.pFormat, rSave.nNodeIdx );

        // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *rSave.pFormat );

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), rSave.pFormat ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( pObjArr->pFormat, pObjArr->nNodeIdx );
    rFlyFormats.push_back( pObjArr->pFormat );

    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFormat, pObjArr->pObj );
    // #i26791# - correction: connect object to layout
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr->pObj );

    SwDrawFrameFormat* pDrawFrameFormat = dynamic_cast<SwDrawFrameFormat*>(pObjArr->pFormat);

    // #i45952# - notify that position attributes are already set
    OSL_ENSURE(pDrawFrameFormat,
            "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
    if (pDrawFrameFormat)
        pDrawFrameFormat->PosAttrSet();
}

void SwUndoDrawGroup::AddObj( sal_uInt16 nPos, SwDrawFrameFormat* pFormat, SdrObject* pObj )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
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
    pObjArr->pObj = nullptr;
    pObjArr->pFormat = pFormat;
}

SwUndoDrawUnGroup::SwUndoDrawUnGroup( SdrObjGroup* pObj )
    : SwUndo( UNDO_DRAWUNGROUP ), bDelFormat( false )
{
    nSize = (sal_uInt16)pObj->GetSubList()->GetObjCount() + 1;
    pObjArr = new SwUndoGroupObjImpl[ nSize ];

    SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
    SwDrawFrameFormat* pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());

    pObjArr->pObj = pObj;
    pObjArr->pFormat = pFormat;

    // object will destroy itself
    pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
    pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, pObjArr->nNodeIdx );

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
        SwUndoGroupObjImpl* pTmp = pObjArr + 1;
        for( sal_uInt16 n = 1; n < nSize; ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    else
        delete pObjArr->pFormat;

    delete [] pObjArr;
}

void SwUndoDrawUnGroup::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFormat = true;

    SwDoc *const pDoc = & rContext.GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();

    // remove from array
    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_SaveAnchor( rSave.pFormat, rSave.nNodeIdx );

           // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *rSave.pFormat );

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), rSave.pFormat ));
    }

    // re-insert group object
    ::lcl_RestoreAnchor( pObjArr->pFormat, pObjArr->nNodeIdx );
    rFlyFormats.push_back( pObjArr->pFormat );

    SwDrawContact *pContact = new SwDrawContact( pObjArr->pFormat, pObjArr->pObj );
    pContact->ConnectToLayout();
    // #i45718# - follow-up of #i35635# move object to visible layer
    pContact->MoveObjToVisibleLayer( pObjArr->pObj );

    SwDrawFrameFormat* pDrawFrameFormat = dynamic_cast<SwDrawFrameFormat*>(pObjArr->pFormat);

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
    SwDrawFrameFormat* pFormat = pObjArr->pFormat;
    SwDrawContact* pContact = static_cast<SwDrawContact*>(pFormat->FindContactObj());

        // object will destroy itself
    pContact->Changed( *pObjArr->pObj, SDRUSERCALL_DELETE,
        pObjArr->pObj->GetLastBoundRect() );
    pObjArr->pObj->SetUserCall( nullptr );

    ::lcl_SaveAnchor( pFormat, pObjArr->nNodeIdx );

       // notify UNO objects to decouple
    ::lcl_SendRemoveToUno( *pFormat );

    // remove from array
    SwDoc* pDoc = pFormat->GetDoc();
    SwFrameFormats& rFlyFormats = *pDoc->GetSpzFrameFormats();
    rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));

    for( sal_uInt16 n = 1; n < nSize; ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );

        ::lcl_RestoreAnchor( rSave.pFormat, rSave.nNodeIdx );
        rFlyFormats.push_back( rSave.pFormat );

        SwDrawFrameFormat* pDrawFrameFormat = dynamic_cast<SwDrawFrameFormat*>(rSave.pFormat);

        // #i45952# - notify that position attributes are already set
        OSL_ENSURE(pDrawFrameFormat,
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object" );
        if (pDrawFrameFormat)
            pDrawFrameFormat->PosAttrSet();
    }
}

void SwUndoDrawUnGroup::AddObj( sal_uInt16 nPos, SwDrawFrameFormat* pFormat )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + nPos + 1 );
    rSave.pFormat = pFormat;
    rSave.pObj = nullptr;
}

SwUndoDrawUnGroupConnectToLayout::SwUndoDrawUnGroupConnectToLayout()
    : SwUndo( UNDO_DRAWUNGROUP )
{
}

SwUndoDrawUnGroupConnectToLayout::~SwUndoDrawUnGroupConnectToLayout()
{
}

void
SwUndoDrawUnGroupConnectToLayout::UndoImpl(::sw::UndoRedoContext &)
{
    for ( std::vector< SdrObject >::size_type i = 0;
          i < aDrawFormatsAndObjs.size(); ++i )
    {
        SdrObject* pObj( aDrawFormatsAndObjs[i].second );
        SwDrawContact* pDrawContact( dynamic_cast<SwDrawContact*>(pObj->GetUserCall()) );
        OSL_ENSURE( pDrawContact,
                "<SwUndoDrawUnGroupConnectToLayout::Undo(..)> -- missing SwDrawContact instance" );
        if ( pDrawContact )
        {
            // deletion of instance <pDrawContact> and thus disconnection from
            // the Writer layout.
            pDrawContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
            pObj->SetUserCall( nullptr );
        }
    }
}

void
SwUndoDrawUnGroupConnectToLayout::RedoImpl(::sw::UndoRedoContext &)
{
    for ( std::vector< std::pair< SwDrawFrameFormat*, SdrObject* > >::size_type i = 0;
          i < aDrawFormatsAndObjs.size(); ++i )
    {
        SwDrawFrameFormat* pFormat( aDrawFormatsAndObjs[i].first );
        SdrObject* pObj( aDrawFormatsAndObjs[i].second );
        SwDrawContact *pContact = new SwDrawContact( pFormat, pObj );
        pContact->ConnectToLayout();
        pContact->MoveObjToVisibleLayer( pObj );
    }
}

void SwUndoDrawUnGroupConnectToLayout::AddFormatAndObj( SwDrawFrameFormat* pDrawFrameFormat,
                                                     SdrObject* pDrawObject )
{
    aDrawFormatsAndObjs.push_back(
            std::pair< SwDrawFrameFormat*, SdrObject* >( pDrawFrameFormat, pDrawObject ) );
}

SwUndoDrawDelete::SwUndoDrawDelete( sal_uInt16 nCnt )
    : SwUndo( UNDO_DRAWDELETE ), nSize( nCnt ), bDelFormat( true )
{
    pObjArr = new SwUndoGroupObjImpl[ nSize ];
    pMarkLst = new SdrMarkList();
}

SwUndoDrawDelete::~SwUndoDrawDelete()
{
    if( bDelFormat )
    {
        SwUndoGroupObjImpl* pTmp = pObjArr;
        for( size_t n = 0; n < pMarkLst->GetMarkCount(); ++n, ++pTmp )
            delete pTmp->pFormat;
    }
    delete [] pObjArr;
    delete pMarkLst;
}

void SwUndoDrawDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFormat = false;
    SwFrameFormats & rFlyFormats = *rContext.GetDoc().GetSpzFrameFormats();
    for( size_t n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        ::lcl_RestoreAnchor( rSave.pFormat, rSave.nNodeIdx );
        rFlyFormats.push_back( rSave.pFormat );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = new SwDrawContact( rSave.pFormat, pObj );
        pContact->_Changed( *pObj, SDRUSERCALL_INSERTED, nullptr );
        // #i45718# - follow-up of #i35635# move object to visible layer
        pContact->MoveObjToVisibleLayer( pObj );

        SwDrawFrameFormat* pDrawFrameFormat = dynamic_cast<SwDrawFrameFormat*>(rSave.pFormat);

        // #i45952# - notify that position attributes are already set
        OSL_ENSURE(pDrawFrameFormat,
                "<SwUndoDrawGroup::Undo(..)> - wrong type of frame format for drawing object");
        if (pDrawFrameFormat)
            pDrawFrameFormat->PosAttrSet();
    }
    rContext.SetSelections(nullptr, pMarkLst);
}

void SwUndoDrawDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    bDelFormat = true;
    SwFrameFormats & rFlyFormats = *rContext.GetDoc().GetSpzFrameFormats();
    for( size_t n = 0; n < pMarkLst->GetMarkCount(); ++n )
    {
        SwUndoGroupObjImpl& rSave = *( pObjArr + n );
        SdrObject *pObj = rSave.pObj;
        SwDrawContact *pContact = static_cast<SwDrawContact*>(GetUserCall(pObj));
        SwDrawFrameFormat *pFormat = static_cast<SwDrawFrameFormat*>(pContact->GetFormat());

        // object will destroy itself
        pContact->Changed( *pObj, SDRUSERCALL_DELETE, pObj->GetLastBoundRect() );
        pObj->SetUserCall( nullptr );

           // notify UNO objects to decouple
        ::lcl_SendRemoveToUno( *pFormat );

        rFlyFormats.erase( std::find( rFlyFormats.begin(), rFlyFormats.end(), pFormat ));
        ::lcl_SaveAnchor( pFormat, rSave.nNodeIdx );
    }
}

void SwUndoDrawDelete::AddObj( sal_uInt16 , SwDrawFrameFormat* pFormat,
                                const SdrMark& rMark )
{
    SwUndoGroupObjImpl& rSave = *( pObjArr + pMarkLst->GetMarkCount() );
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
