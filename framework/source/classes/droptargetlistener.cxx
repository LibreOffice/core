/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <classes/droptargetlistener.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <targets.h>
#include <services.h>

#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <svtools/transfer.hxx>
#include <unotools/localfilehelper.hxx>
#include <sot/filelist.hxx>
#include <comphelper/processfactory.hxx>

#include <osl/file.hxx>
#include <vcl/svapp.hxx>

namespace framework
{

DropTargetListener::DropTargetListener( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory,
                                        const css::uno::Reference< css::frame::XFrame >&              xFrame  )
        : ThreadHelpBase  ( &Application::GetSolarMutex() )
        , m_xFactory      ( xFactory                      )
        , m_xTargetFrame  ( xFrame                        )
        , m_pFormats      ( new DataFlavorExVector        )
{
}

// -----------------------------------------------------------------------------

DropTargetListener::~DropTargetListener()
{
    m_xTargetFrame = css::uno::WeakReference< css::frame::XFrame >();
    m_xFactory     = css::uno::Reference< css::lang::XMultiServiceFactory >();
    delete m_pFormats;
    m_pFormats = NULL;
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::disposing( const css::lang::EventObject& ) throw( css::uno::RuntimeException )
{
    m_xTargetFrame = css::uno::WeakReference< css::frame::XFrame >();
    m_xFactory     = css::uno::Reference< css::lang::XMultiServiceFactory >();
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) throw( css::uno::RuntimeException )
{
    const sal_Int8 nAction = dtde.DropAction;

    try
    {
        if ( css::datatransfer::dnd::DNDConstants::ACTION_NONE != nAction )
        {
            TransferableDataHelper aHelper( dtde.Transferable );
            sal_Bool bFormatFound = sal_False;
            FileList aFileList;

            // at first check filelist format
            if ( aHelper.GetFileList( SOT_FORMAT_FILE_LIST, aFileList ) )
            {
                sal_uLong i, nCount = aFileList.Count();
                for ( i = 0; i < nCount; ++i )
                    implts_OpenFile( aFileList.GetFile(i) );
                bFormatFound = sal_True;
            }

            // then, if necessary, the file format
            String aFilePath;
            if ( !bFormatFound && aHelper.GetString( SOT_FORMAT_FILE, aFilePath ) )
                implts_OpenFile( aFilePath );
        }
        dtde.Context->dropComplete( css::datatransfer::dnd::DNDConstants::ACTION_NONE != nAction );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw( css::uno::RuntimeException )
{
    try
    {
        implts_BeginDrag( dtdee.SupportedDataFlavors );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }

    dragOver( dtdee );
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dragExit( const css::datatransfer::dnd::DropTargetEvent& ) throw( css::uno::RuntimeException )
{
    try
    {
        implts_EndDrag();
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) throw( css::uno::RuntimeException )
{
    try
    {
        sal_Bool bAccept = ( implts_IsDropFormatSupported( SOT_FORMAT_FILE ) ||
                             implts_IsDropFormatSupported( SOT_FORMAT_FILE_LIST ) );

        if ( !bAccept )
            dtde.Context->rejectDrag();
        else
            dtde.Context->acceptDrag( css::datatransfer::dnd::DNDConstants::ACTION_COPY );
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
    }
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& ) throw( css::uno::RuntimeException )
{
}

void DropTargetListener::implts_BeginDrag( const css::uno::Sequence< css::datatransfer::DataFlavor >& rSupportedDataFlavors )
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_pFormats->clear();
    TransferableDataHelper::FillDataFlavorExVector(rSupportedDataFlavors,*m_pFormats);
    aWriteLock.unlock();
    /* } SAFE */
}

void DropTargetListener::implts_EndDrag()
{
    /* SAFE { */
    WriteGuard aWriteLock(m_aLock);
    m_pFormats->clear();
    aWriteLock.unlock();
    /* } SAFE */
}

sal_Bool DropTargetListener::implts_IsDropFormatSupported( SotFormatStringId nFormat )
{
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    DataFlavorExVector::iterator aIter( m_pFormats->begin() ), aEnd( m_pFormats->end() );
    sal_Bool bRet = sal_False;

    while ( aIter != aEnd )
    {
        if ( nFormat == (*aIter++).mnSotId )
        {
            bRet = sal_True;
            aIter = aEnd;
        }
    }
    aReadLock.unlock();
    /* } SAFE */

    return bRet;
}

void DropTargetListener::implts_OpenFile( const String& rFilePath )
{
    rtl::OUString aFileURL;
    if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFilePath, aFileURL ) )
        aFileURL = rFilePath;

    ::osl::FileStatus aStatus( osl_FileStatus_Mask_FileURL );
    ::osl::DirectoryItem aItem;
    if( ::osl::FileBase::E_None == ::osl::DirectoryItem::get( aFileURL, aItem ) &&
        ::osl::FileBase::E_None == aItem.getFileStatus( aStatus ) )
            aFileURL = aStatus.getFileURL();

    // open file
    /* SAFE { */
    ReadGuard aReadLock(m_aLock);
    css::uno::Reference< css::frame::XFrame >         xTargetFrame( m_xTargetFrame.get(), css::uno::UNO_QUERY );
    css::uno::Reference< css::util::XURLTransformer > xParser     ( css::util::URLTransformer::create(::comphelper::getComponentContext(m_xFactory)) );
    aReadLock.unlock();
    /* } SAFE */
    if (xTargetFrame.is() && xParser.is())
    {
        css::util::URL aURL;
        aURL.Complete = aFileURL;
        xParser->parseStrict(aURL);

        css::uno::Reference < css::frame::XDispatchProvider > xProvider( xTargetFrame, css::uno::UNO_QUERY );
        css::uno::Reference< css::frame::XDispatch > xDispatcher = xProvider->queryDispatch( aURL, SPECIALTARGET_DEFAULT, 0 );
        if ( xDispatcher.is() )
            xDispatcher->dispatch( aURL, css::uno::Sequence < css::beans::PropertyValue >() );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
