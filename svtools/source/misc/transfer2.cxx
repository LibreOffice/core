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

#include <osl/mutex.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sot/exchange.hxx>
#include <sot/storage.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include "svl/urlbmk.hxx"
#include <svtools/inetimg.hxx>
#include <svtools/imap.hxx>
#include <svtools/transfer.hxx>

// --------------
// - Namespaces -
// --------------

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;

// -----------------------------------------
// - DragSourceHelper::DragGestureListener -
// -----------------------------------------

DragSourceHelper::DragGestureListener::DragGestureListener( DragSourceHelper& rDragSourceHelper ) :
    mrParent( rDragSourceHelper )
{
}

// -----------------------------------------------------------------------------

DragSourceHelper::DragGestureListener::~DragGestureListener()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL DragSourceHelper::DragGestureListener::disposing( const EventObject& ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL DragSourceHelper::DragGestureListener::dragGestureRecognized( const DragGestureEvent& rDGE ) throw( RuntimeException )
{
    const SolarMutexGuard aGuard;

    const Point aPtPixel( rDGE.DragOriginX, rDGE.DragOriginY );
    mrParent.StartDrag( rDGE.DragAction, aPtPixel );
}

// --------------------
// - DragSourceHelper -
// --------------------

DragSourceHelper::DragSourceHelper( Window* pWindow ) :
    mxDragGestureRecognizer( pWindow->GetDragGestureRecognizer() )
{
    if( mxDragGestureRecognizer.is() )
    {
        mxDragGestureListener = new DragSourceHelper::DragGestureListener( *this );
        mxDragGestureRecognizer->addDragGestureListener( mxDragGestureListener );
    }
}

// -----------------------------------------------------------------------------

DragSourceHelper::~DragSourceHelper()
{
    if( mxDragGestureRecognizer.is()  )
        mxDragGestureRecognizer->removeDragGestureListener( mxDragGestureListener );
}

// -----------------------------------------------------------------------------

void DragSourceHelper::StartDrag( sal_Int8, const Point& )
{
}

// ----------------------------------------
// - DropTargetHelper::DropTargetListener -
// ----------------------------------------

DropTargetHelper::DropTargetListener::DropTargetListener( DropTargetHelper& rDropTargetHelper ) :
    mrParent( rDropTargetHelper ),
    mpLastDragOverEvent( NULL )
{
}

// -----------------------------------------------------------------------------

DropTargetHelper::DropTargetListener::~DropTargetListener()
{
    delete mpLastDragOverEvent;
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::disposing( const EventObject& ) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::drop( const DropTargetDropEvent& rDTDE ) throw( RuntimeException )
{
    const SolarMutexGuard aGuard;

    try
    {
        AcceptDropEvent  aAcceptEvent;
        ExecuteDropEvent aExecuteEvt( rDTDE.DropAction & ~DNDConstants::ACTION_DEFAULT, Point( rDTDE.LocationX, rDTDE.LocationY ), rDTDE );
        sal_Int8         nRet = DNDConstants::ACTION_NONE;

        aExecuteEvt.mbDefault = ( ( rDTDE.DropAction & DNDConstants::ACTION_DEFAULT ) != 0 );

        // in case of a default action, call ::AcceptDrop first and use the returned
        // accepted action as the execute action in the call to ::ExecuteDrop
        aAcceptEvent.mnAction = aExecuteEvt.mnAction;
        aAcceptEvent.maPosPixel = aExecuteEvt.maPosPixel;
        (DropTargetEvent&)( aAcceptEvent.maDragEvent ) = (DropTargetEvent&) rDTDE;
        ( (DropTargetDragEvent&)( aAcceptEvent.maDragEvent ) ).DropAction = rDTDE.DropAction;
        ( (DropTargetDragEvent&)( aAcceptEvent.maDragEvent ) ).LocationX = rDTDE.LocationX;
        ( (DropTargetDragEvent&)( aAcceptEvent.maDragEvent ) ).LocationY = rDTDE.LocationY;
        ( (DropTargetDragEvent&)( aAcceptEvent.maDragEvent ) ).SourceActions = rDTDE.SourceActions;
        aAcceptEvent.mbLeaving = sal_False;
        aAcceptEvent.mbDefault = aExecuteEvt.mbDefault;

        nRet = mrParent.AcceptDrop( aAcceptEvent );

        if( DNDConstants::ACTION_NONE != nRet )
        {
            rDTDE.Context->acceptDrop( nRet );

            if( aExecuteEvt.mbDefault )
                aExecuteEvt.mnAction = nRet;

            nRet = mrParent.ExecuteDrop( aExecuteEvt );
        }

        rDTDE.Context->dropComplete( DNDConstants::ACTION_NONE != nRet );

        if( mpLastDragOverEvent )
        {
            delete mpLastDragOverEvent;
            mpLastDragOverEvent = NULL;
        }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dragEnter( const DropTargetDragEnterEvent& rDTDEE ) throw( RuntimeException )
{
    const SolarMutexGuard aGuard;

    try
    {
        mrParent.ImplBeginDrag( rDTDEE.SupportedDataFlavors );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    dragOver( rDTDEE );
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dragOver( const DropTargetDragEvent& rDTDE ) throw( RuntimeException )
{
    const SolarMutexGuard aGuard;

    try
    {
        if( mpLastDragOverEvent )
            delete mpLastDragOverEvent;

        mpLastDragOverEvent = new AcceptDropEvent( rDTDE.DropAction & ~DNDConstants::ACTION_DEFAULT, Point( rDTDE.LocationX, rDTDE.LocationY ), rDTDE );
        mpLastDragOverEvent->mbDefault = ( ( rDTDE.DropAction & DNDConstants::ACTION_DEFAULT ) != 0 );

        const sal_Int8 nRet = mrParent.AcceptDrop( *mpLastDragOverEvent );

        if( DNDConstants::ACTION_NONE == nRet )
            rDTDE.Context->rejectDrag();
        else
            rDTDE.Context->acceptDrag( nRet );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dragExit( const DropTargetEvent& ) throw( RuntimeException )
{
    const SolarMutexGuard aGuard;

    try
    {
        if( mpLastDragOverEvent )
        {
            mpLastDragOverEvent->mbLeaving = sal_True;
            mrParent.AcceptDrop( *mpLastDragOverEvent );
            delete mpLastDragOverEvent;
            mpLastDragOverEvent = NULL;
        }

        mrParent.ImplEndDrag();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}


// -----------------------------------------------------------------------------

void SAL_CALL DropTargetHelper::DropTargetListener::dropActionChanged( const DropTargetDragEvent& ) throw( RuntimeException )
{
}

// --------------------
// - DropTargetHelper -
// --------------------

DropTargetHelper::DropTargetHelper( Window* pWindow ) :
    mxDropTarget( pWindow->GetDropTarget() ),
    mpFormats( new DataFlavorExVector )
{
    ImplConstruct();
}

// -----------------------------------------------------------------------------

DropTargetHelper::DropTargetHelper( const Reference< XDropTarget >& rxDropTarget ) :
    mxDropTarget( rxDropTarget ),
    mpFormats( new DataFlavorExVector )
{
    ImplConstruct();
}

// -----------------------------------------------------------------------------

DropTargetHelper::~DropTargetHelper()
{
    if( mxDropTarget.is() )
        mxDropTarget->removeDropTargetListener( mxDropTargetListener );

    delete mpFormats;
}

// -----------------------------------------------------------------------------

void DropTargetHelper::ImplConstruct()
{
    if( mxDropTarget.is() )
    {
        mxDropTargetListener = new DropTargetHelper::DropTargetListener( *this );
        mxDropTarget->addDropTargetListener( mxDropTargetListener );
        mxDropTarget->setActive( sal_True );
    }
}

// -----------------------------------------------------------------------------

void DropTargetHelper::ImplBeginDrag( const Sequence< DataFlavor >& rSupportedDataFlavors )
{
    mpFormats->clear();
    TransferableDataHelper::FillDataFlavorExVector( rSupportedDataFlavors, *mpFormats );
}

// -----------------------------------------------------------------------------

void DropTargetHelper::ImplEndDrag()
{
    mpFormats->clear();
}

// -----------------------------------------------------------------------------

sal_Int8 DropTargetHelper::AcceptDrop( const AcceptDropEvent& )
{
    return( DNDConstants::ACTION_NONE );
}

// -----------------------------------------------------------------------------

sal_Int8 DropTargetHelper::ExecuteDrop( const ExecuteDropEvent& )
{
    return( DNDConstants::ACTION_NONE );
}

// -----------------------------------------------------------------------------

sal_Bool DropTargetHelper::IsDropFormatSupported( SotFormatStringId nFormat )
{
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_Bool                        bRet = sal_False;

    while( aIter != aEnd )
    {
        if( nFormat == (*aIter++).mnSotId )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool DropTargetHelper::IsDropFormatSupported( const DataFlavor& rFlavor )
{
    DataFlavorExVector::iterator    aIter( mpFormats->begin() ), aEnd( mpFormats->end() );
    sal_Bool                        bRet = sal_False;

    while( aIter != aEnd )
    {
        if( TransferableDataHelper::IsEqual( rFlavor, *aIter++ ) )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------
// TransferDataContainer
// -----------------------------------------------------------------------------

struct TDataCntnrEntry_Impl
{
    ::com::sun::star::uno::Any aAny;
    SotFormatStringId nId;
};

// -----------------------------------------------------------------------------

typedef ::std::list< TDataCntnrEntry_Impl > TDataCntnrEntryList;

// -----------------------------------------------------------------------------

struct TransferDataContainer_Impl
{
    TDataCntnrEntryList aFmtList;
    Link aFinshedLnk;
    INetBookmark* pBookmk;
    Graphic* pGrf;

    TransferDataContainer_Impl()
        : pBookmk( 0 ), pGrf( 0 )
    {
    }

    ~TransferDataContainer_Impl()
    {
        delete pBookmk;
        delete pGrf;
    }
};

// -----------------------------------------------------------------------------

TransferDataContainer::TransferDataContainer()
    : pImpl( new TransferDataContainer_Impl )
{
}

// -----------------------------------------------------------------------------

TransferDataContainer::~TransferDataContainer()
{
    delete pImpl;
}

// -----------------------------------------------------------------------------

void TransferDataContainer::AddSupportedFormats()
{
}

// -----------------------------------------------------------------------------

sal_Bool TransferDataContainer::GetData( const
            ::com::sun::star::datatransfer::DataFlavor& rFlavor )
{
    TDataCntnrEntryList::iterator   aIter( pImpl->aFmtList.begin() ),
                                    aEnd( pImpl->aFmtList.end() );
    sal_Bool bFnd = sal_False;
    sal_uLong nFmtId = SotExchange::GetFormat( rFlavor );

    // test first the list
    for( ; aIter != aEnd; ++aIter )
    {
        TDataCntnrEntry_Impl& rEntry = (TDataCntnrEntry_Impl&)*aIter;
        if( nFmtId == rEntry.nId )
        {
            bFnd = SetAny( rEntry.aAny, rFlavor );
            break;
        }
    }

    // test second the bookmark pointer
    if( !bFnd )
        switch( nFmtId )
        {
         case SOT_FORMAT_STRING:
         case SOT_FORMATSTR_ID_SOLK:
         case SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK:
         case SOT_FORMATSTR_ID_FILECONTENT:
         case SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR:
         case SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR:
            if( pImpl->pBookmk )
                bFnd = SetINetBookmark( *pImpl->pBookmk, rFlavor );
            break;

        case SOT_FORMATSTR_ID_SVXB:
        case SOT_FORMAT_BITMAP:
        case SOT_FORMAT_GDIMETAFILE:
            if( pImpl->pGrf )
                bFnd = SetGraphic( *pImpl->pGrf, rFlavor );
            break;
        }

    return bFnd;
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyINetBookmark( const INetBookmark& rBkmk )
{
    if( !pImpl->pBookmk )
        pImpl->pBookmk = new INetBookmark( rBkmk );
    else
        *pImpl->pBookmk = rBkmk;

     AddFormat( SOT_FORMAT_STRING );
     AddFormat( SOT_FORMATSTR_ID_SOLK );
     AddFormat( SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK );
     AddFormat( SOT_FORMATSTR_ID_FILECONTENT );
     AddFormat( SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR );
     AddFormat( SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR );
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyAnyData( sal_uLong nFormatId,
                                        const sal_Char* pData, sal_uLong nLen )
{
    if( nLen )
    {
        TDataCntnrEntry_Impl aEntry;
        aEntry.nId = nFormatId;

        Sequence< sal_Int8 > aSeq( nLen  );
        memcpy( aSeq.getArray(), pData, nLen );
        aEntry.aAny <<= aSeq;
        pImpl->aFmtList.push_back( aEntry );
         AddFormat( nFormatId );
    }
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyByteString( sal_uLong nFormatId,
                                            const OString& rStr )
{
    CopyAnyData( nFormatId, rStr.getStr(), rStr.getLength() );
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyINetImage( const INetImage& rINtImg )
{
    SvMemoryStream aMemStm( 1024, 1024 );
    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rINtImg.Write( aMemStm, SOT_FORMATSTR_ID_INET_IMAGE );
    CopyAnyData( SOT_FORMATSTR_ID_INET_IMAGE, (sal_Char*)aMemStm.GetData(),
                    aMemStm.Seek( STREAM_SEEK_TO_END ) );
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyImageMap( const ImageMap& rImgMap )
{
    SvMemoryStream aMemStm( 8192, 8192 );
    aMemStm.SetVersion( SOFFICE_FILEFORMAT_50 );
    rImgMap.Write( aMemStm, OUString() );
    CopyAnyData( SOT_FORMATSTR_ID_SVIM, (sal_Char*)aMemStm.GetData(),
                    aMemStm.Seek( STREAM_SEEK_TO_END ) );
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyGraphic( const Graphic& rGrf )
{
    GraphicType nType = rGrf.GetType();
    if( GRAPHIC_NONE != nType )
    {
        if( !pImpl->pGrf )
            pImpl->pGrf = new Graphic( rGrf );
        else
            *pImpl->pGrf = rGrf;

        AddFormat( SOT_FORMATSTR_ID_SVXB );
        if( GRAPHIC_BITMAP == nType )
            AddFormat( SOT_FORMAT_BITMAP );
        else if( GRAPHIC_GDIMETAFILE == nType )
            AddFormat( SOT_FORMAT_GDIMETAFILE );
    }
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyString( sal_uInt16 nFmt, const OUString& rStr )
{
    if( !rStr.isEmpty() )
    {
        TDataCntnrEntry_Impl aEntry;
        aEntry.nId = nFmt;
        OUString aStr( rStr );
        aEntry.aAny <<= aStr;
        pImpl->aFmtList.push_back( aEntry );
         AddFormat( aEntry.nId );
    }
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyString( const OUString& rStr )
{
    CopyString( SOT_FORMAT_STRING, rStr );
}

// -----------------------------------------------------------------------------

void TransferDataContainer::CopyAny( sal_uInt16 nFmt,
                                    const ::com::sun::star::uno::Any& rAny )
{
    TDataCntnrEntry_Impl aEntry;
    aEntry.nId = nFmt;
    aEntry.aAny = rAny;
    pImpl->aFmtList.push_back( aEntry );
    AddFormat( aEntry.nId );
}

// -----------------------------------------------------------------------------

sal_Bool TransferDataContainer::HasAnyData() const
{
    return pImpl->aFmtList.begin() != pImpl->aFmtList.end() ||
            0 != pImpl->pBookmk;
}

// -----------------------------------------------------------------------------

void TransferDataContainer::StartDrag(
        Window* pWindow, sal_Int8 nDragSourceActions,
        const Link& rLnk, sal_Int32 nDragPointer, sal_Int32 nDragImage )
{
    pImpl->aFinshedLnk = rLnk;
    TransferableHelper::StartDrag( pWindow, nDragSourceActions,
                                    nDragPointer, nDragImage );
}

// -----------------------------------------------------------------------------

void TransferDataContainer::DragFinished( sal_Int8 nDropAction )
{
    if( pImpl->aFinshedLnk.IsSet() )
        pImpl->aFinshedLnk.Call( &nDropAction );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
