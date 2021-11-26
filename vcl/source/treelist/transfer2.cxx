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

#include <config_features.h>

#include <osl/mutex.hxx>
#include <sot/exchange.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/datatransfer/clipboard/CurrentViewClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/SystemClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetDragContext.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <svl/urlbmk.hxx>
#include <vcl/transfer.hxx>

#include <svdata.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;


DragSourceHelper::DragGestureListener::DragGestureListener( DragSourceHelper& rDragSourceHelper ) :
    mrParent( rDragSourceHelper )
{
}


DragSourceHelper::DragGestureListener::~DragGestureListener()
{
}


void SAL_CALL DragSourceHelper::DragGestureListener::disposing( const EventObject& )
{
}


void SAL_CALL DragSourceHelper::DragGestureListener::dragGestureRecognized( const DragGestureEvent& rDGE )
{
    const SolarMutexGuard aGuard;

    const Point aPtPixel( rDGE.DragOriginX, rDGE.DragOriginY );
    mrParent.StartDrag( rDGE.DragAction, aPtPixel );
}


DragSourceHelper::DragSourceHelper( vcl::Window* pWindow ) :
    mxDragGestureRecognizer( pWindow->GetDragGestureRecognizer() )
{
    if( mxDragGestureRecognizer.is() )
    {
        mxDragGestureListener = new DragSourceHelper::DragGestureListener( *this );
        mxDragGestureRecognizer->addDragGestureListener( mxDragGestureListener );
    }
}


void DragSourceHelper::dispose()
{
    Reference<XDragGestureRecognizer> xTmp;
    {
        osl::MutexGuard aGuard( maMutex );
        xTmp = mxDragGestureRecognizer;
        mxDragGestureRecognizer.clear();
    }
    if( xTmp.is()  )
        xTmp->removeDragGestureListener( mxDragGestureListener );
}

DragSourceHelper::~DragSourceHelper()
{
    dispose();
}


void DragSourceHelper::StartDrag( sal_Int8, const Point& )
{
}


DropTargetHelper::DropTargetListener::DropTargetListener( DropTargetHelper& rDropTargetHelper ) :
    mrParent( rDropTargetHelper )
{
}


DropTargetHelper::DropTargetListener::~DropTargetListener()
{
}


void SAL_CALL DropTargetHelper::DropTargetListener::disposing( const EventObject& )
{
}


void SAL_CALL DropTargetHelper::DropTargetListener::drop( const DropTargetDropEvent& rDTDE )
{
    const SolarMutexGuard aGuard;

    try
    {
        AcceptDropEvent  aAcceptEvent;
        ExecuteDropEvent aExecuteEvt( rDTDE.DropAction & ~DNDConstants::ACTION_DEFAULT, Point( rDTDE.LocationX, rDTDE.LocationY ), rDTDE );

        aExecuteEvt.mbDefault = ( ( rDTDE.DropAction & DNDConstants::ACTION_DEFAULT ) != 0 );

        // in case of a default action, call ::AcceptDrop first and use the returned
        // accepted action as the execute action in the call to ::ExecuteDrop
        aAcceptEvent.mnAction = aExecuteEvt.mnAction;
        aAcceptEvent.maPosPixel = aExecuteEvt.maPosPixel;
        static_cast<DropTargetEvent&>(const_cast<DropTargetDragEvent&>( aAcceptEvent.maDragEvent )) = rDTDE;
        const_cast<DropTargetDragEvent&>( aAcceptEvent.maDragEvent ).DropAction = rDTDE.DropAction;
        const_cast<DropTargetDragEvent&>( aAcceptEvent.maDragEvent ).LocationX = rDTDE.LocationX;
        const_cast<DropTargetDragEvent&>( aAcceptEvent.maDragEvent ).LocationY = rDTDE.LocationY;
        const_cast<DropTargetDragEvent&>( aAcceptEvent.maDragEvent ).SourceActions = rDTDE.SourceActions;
        aAcceptEvent.mbLeaving = false;
        aAcceptEvent.mbDefault = aExecuteEvt.mbDefault;

        sal_Int8 nRet = mrParent.AcceptDrop( aAcceptEvent );

        if( DNDConstants::ACTION_NONE != nRet )
        {
            rDTDE.Context->acceptDrop( nRet );

            if( aExecuteEvt.mbDefault )
                aExecuteEvt.mnAction = nRet;

            nRet = mrParent.ExecuteDrop( aExecuteEvt );
        }

        rDTDE.Context->dropComplete( DNDConstants::ACTION_NONE != nRet );

        mpLastDragOverEvent.reset();
    }
    catch( const css::uno::Exception& )
    {
    }
}


void SAL_CALL DropTargetHelper::DropTargetListener::dragEnter( const DropTargetDragEnterEvent& rDTDEE )
{
    const SolarMutexGuard aGuard;

    try
    {
        mrParent.ImplBeginDrag( rDTDEE.SupportedDataFlavors );
    }
    catch( const css::uno::Exception& )
    {
    }

    dragOver( rDTDEE );
}


void SAL_CALL DropTargetHelper::DropTargetListener::dragOver( const DropTargetDragEvent& rDTDE )
{
    const SolarMutexGuard aGuard;

    try
    {
        mpLastDragOverEvent.reset( new AcceptDropEvent( rDTDE.DropAction & ~DNDConstants::ACTION_DEFAULT, Point( rDTDE.LocationX, rDTDE.LocationY ), rDTDE ) );
        mpLastDragOverEvent->mbDefault = ( ( rDTDE.DropAction & DNDConstants::ACTION_DEFAULT ) != 0 );

        const sal_Int8 nRet = mrParent.AcceptDrop( *mpLastDragOverEvent );

        if( DNDConstants::ACTION_NONE == nRet )
            rDTDE.Context->rejectDrag();
        else
            rDTDE.Context->acceptDrag( nRet );
    }
    catch( const css::uno::Exception& )
    {
    }
}


void SAL_CALL DropTargetHelper::DropTargetListener::dragExit( const DropTargetEvent& )
{
    const SolarMutexGuard aGuard;

    try
    {
        if( mpLastDragOverEvent )
        {
            mpLastDragOverEvent->mbLeaving = true;
            mrParent.AcceptDrop( *mpLastDragOverEvent );
            mpLastDragOverEvent.reset();
        }

        mrParent.ImplEndDrag();
    }
    catch( const css::uno::Exception& )
    {
    }
}


void SAL_CALL DropTargetHelper::DropTargetListener::dropActionChanged( const DropTargetDragEvent& )
{
}


DropTargetHelper::DropTargetHelper( vcl::Window* pWindow ) :
    mxDropTarget( pWindow->GetDropTarget() )
{
    ImplConstruct();
}


DropTargetHelper::DropTargetHelper( const Reference< XDropTarget >& rxDropTarget ) :
    mxDropTarget( rxDropTarget )
{
    ImplConstruct();
}


void DropTargetHelper::dispose()
{
    Reference< XDropTarget >  xTmp;
    {
        osl::MutexGuard aGuard( maMutex );
        xTmp = mxDropTarget;
        mxDropTarget.clear();
    }
    if( xTmp.is() )
        xTmp->removeDropTargetListener( mxDropTargetListener );
}

DropTargetHelper::~DropTargetHelper()
{
    dispose();
}


void DropTargetHelper::ImplConstruct()
{
    if( mxDropTarget.is() )
    {
        mxDropTargetListener = new DropTargetHelper::DropTargetListener( *this );
        mxDropTarget->addDropTargetListener( mxDropTargetListener );
        mxDropTarget->setActive( true );
    }
}


void DropTargetHelper::ImplBeginDrag( const Sequence< DataFlavor >& rSupportedDataFlavors )
{
    maFormats.clear();
    TransferableDataHelper::FillDataFlavorExVector( rSupportedDataFlavors, maFormats );
}


void DropTargetHelper::ImplEndDrag()
{
    maFormats.clear();
}


sal_Int8 DropTargetHelper::AcceptDrop( const AcceptDropEvent& )
{
    return DNDConstants::ACTION_NONE;
}


sal_Int8 DropTargetHelper::ExecuteDrop( const ExecuteDropEvent& )
{
    return DNDConstants::ACTION_NONE;
}


bool DropTargetHelper::IsDropFormatSupported(SotClipboardFormatId nFormat) const
{
    return std::any_of(maFormats.begin(), maFormats.end(),
           [&](const DataFlavorEx& data) { return data.mnSotId == nFormat; });
}


// TransferDataContainer

namespace {

struct TDataCntnrEntry_Impl
{
    css::uno::Any aAny;
    SotClipboardFormatId nId;
};

}

struct TransferDataContainer_Impl
{
    std::vector< TDataCntnrEntry_Impl > aFmtList;
    Link<sal_Int8,void> aFinishedLnk;
    std::unique_ptr<INetBookmark> pBookmk;

    TransferDataContainer_Impl()
    {
    }
};


TransferDataContainer::TransferDataContainer()
    : pImpl( new TransferDataContainer_Impl )
{
}


TransferDataContainer::~TransferDataContainer()
{
}


void TransferDataContainer::AddSupportedFormats()
{
}


bool TransferDataContainer::GetData(
    const css::datatransfer::DataFlavor& rFlavor, const OUString& /*rDestDoc*/ )
{
    bool bFnd = false;
    SotClipboardFormatId nFmtId = SotExchange::GetFormat( rFlavor );

    // test first the list
    for (auto const& format : pImpl->aFmtList)
    {
        if( nFmtId == format.nId )
        {
            bFnd = SetAny( format.aAny );
            break;
        }
    }

    // test second the bookmark pointer
    if( !bFnd )
        switch( nFmtId )
        {
         case SotClipboardFormatId::STRING:
         case SotClipboardFormatId::SOLK:
         case SotClipboardFormatId::NETSCAPE_BOOKMARK:
         case SotClipboardFormatId::FILECONTENT:
         case SotClipboardFormatId::FILEGRPDESCRIPTOR:
         case SotClipboardFormatId::UNIFORMRESOURCELOCATOR:
            if( pImpl->pBookmk )
                bFnd = SetINetBookmark( *pImpl->pBookmk, rFlavor );
            break;

        default: break;
        }

    return bFnd;
}


void TransferDataContainer::CopyINetBookmark( const INetBookmark& rBkmk )
{
    if( !pImpl->pBookmk )
        pImpl->pBookmk.reset( new INetBookmark( rBkmk ) );
    else
        *pImpl->pBookmk = rBkmk;

    AddFormat( SotClipboardFormatId::STRING );
    AddFormat( SotClipboardFormatId::SOLK );
    AddFormat( SotClipboardFormatId::NETSCAPE_BOOKMARK );
    AddFormat( SotClipboardFormatId::FILECONTENT );
    AddFormat( SotClipboardFormatId::FILEGRPDESCRIPTOR );
    AddFormat( SotClipboardFormatId::UNIFORMRESOURCELOCATOR );
}


void TransferDataContainer::CopyAnyData( SotClipboardFormatId nFormatId,
                                        const char* pData, sal_uLong nLen )
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


void TransferDataContainer::CopyByteString( SotClipboardFormatId nFormatId,
                                            const OString& rStr )
{
    CopyAnyData( nFormatId, rStr.getStr(), rStr.getLength() );
}


void TransferDataContainer::CopyString( SotClipboardFormatId nFmt, const OUString& rStr )
{
    if( !rStr.isEmpty() )
    {
        TDataCntnrEntry_Impl aEntry;
        aEntry.nId = nFmt;
        aEntry.aAny <<= rStr;
        pImpl->aFmtList.push_back( aEntry );
        AddFormat( aEntry.nId );
    }
}


void TransferDataContainer::CopyString( const OUString& rStr )
{
    CopyString( SotClipboardFormatId::STRING, rStr );
}


bool TransferDataContainer::HasAnyData() const
{
    return !pImpl->aFmtList.empty() ||
            nullptr != pImpl->pBookmk;
}


void TransferDataContainer::StartDrag(
        vcl::Window* pWindow, sal_Int8 nDragSourceActions,
        const Link<sal_Int8,void>& rLnk )
{
    pImpl->aFinishedLnk = rLnk;
    TransferableHelper::StartDrag( pWindow, nDragSourceActions );
}


void TransferDataContainer::DragFinished( sal_Int8 nDropAction )
{
    pImpl->aFinishedLnk.Call( nDropAction );
}

Reference<XClipboard> GetSystemClipboard()
{
    // On Windows, the css.datatransfer.clipboard.SystemClipboard UNO service is implemented as a
    // single-instance service (dtrans_CWinClipboard_get_implementation in
    // vcl/win/dtrans/WinClipboard.cxx) that needs timely disposing to join a spawned thread
    // (done in DeInitVCL, vcl/source/app/svmain.cxx), while on other platforms it is implemented as
    // a multi-instance service (ClipboardFactory, vcl/source/components/dtranscomp.cxx) so we
    // should not hold on to a single instance here:
#if defined _WIN32
    DBG_TESTSOLARMUTEX();
    auto const data = ImplGetSVData();
    if (!data->m_xSystemClipboard.is())
    {
        try
        {
            data->m_xSystemClipboard = css::datatransfer::clipboard::SystemClipboard::create(
                comphelper::getProcessComponentContext());
        }
        catch (DeploymentException const &) {}
    }
    return data->m_xSystemClipboard;
#else
    Reference<XClipboard> xClipboard;
    try
    {
        if (comphelper::LibreOfficeKit::isActive())
        {
            xClipboard = css::datatransfer::clipboard::CurrentViewClipboard::create(
                    comphelper::getProcessComponentContext());
        }
        else
        {
            xClipboard = css::datatransfer::clipboard::SystemClipboard::create(
                comphelper::getProcessComponentContext());
        }
    }
    catch (DeploymentException const &) {}
    return xClipboard;
#endif
}

Reference<XClipboard> GetSystemPrimarySelection()
{
    Reference<XClipboard> xSelection;
    try
    {
        Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
#if HAVE_FEATURE_X11
        // A hack, making the primary selection available as an instance
        // of the SystemClipboard service on X11:
        Sequence< Any > args(1);
        args[0] <<= OUString("PRIMARY");
        xSelection.set(xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
            "com.sun.star.datatransfer.clipboard.SystemClipboard", args, xContext), UNO_QUERY_THROW);
#else
        static Reference< XClipboard > s_xSelection(
            xContext->getServiceManager()->createInstanceWithContext(
                "com.sun.star.datatransfer.clipboard.GenericClipboard", xContext), UNO_QUERY);
        xSelection = s_xSelection;
#endif
    }
    catch (RuntimeException const &) {}
    return xSelection;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
