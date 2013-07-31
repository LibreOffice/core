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

#include <sfx2/droptargetlistener.hxx>

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

const OUString SPECIALTARGET_DEFAULT("_default");

DropTargetListener::DropTargetListener( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                                        const css::uno::Reference< css::frame::XFrame >&          xFrame  )
        : m_xContext      ( xContext                      )
        , m_xTargetFrame  ( xFrame                        )
        , m_pFormats      ( new DataFlavorExVector        )
{
}

// -----------------------------------------------------------------------------

DropTargetListener::~DropTargetListener()
{
    m_xTargetFrame.clear();
    m_xContext.clear();
    delete m_pFormats;
    m_pFormats = NULL;
}

// -----------------------------------------------------------------------------

void SAL_CALL DropTargetListener::disposing( const css::lang::EventObject& ) throw( css::uno::RuntimeException )
{
    m_xTargetFrame.clear();
    m_xContext.clear();
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
    SolarMutexGuard aGuard;
    m_pFormats->clear();
    TransferableDataHelper::FillDataFlavorExVector(rSupportedDataFlavors,*m_pFormats);
    /* } SAFE */
}

void DropTargetListener::implts_EndDrag()
{
    /* SAFE { */
    SolarMutexGuard aGuard;
    m_pFormats->clear();
    /* } SAFE */
}

sal_Bool DropTargetListener::implts_IsDropFormatSupported( SotFormatStringId nFormat )
{
    /* SAFE { */
    SolarMutexGuard aGuard;
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

    return bRet;
    /* } SAFE */
}

void DropTargetListener::implts_OpenFile( const String& rFilePath )
{
    OUString aFileURL;
    if ( !::utl::LocalFileHelper::ConvertPhysicalNameToURL( rFilePath, aFileURL ) )
        aFileURL = rFilePath;

    ::osl::FileStatus aStatus( osl_FileStatus_Mask_FileURL );
    ::osl::DirectoryItem aItem;
    if( ::osl::FileBase::E_None == ::osl::DirectoryItem::get( aFileURL, aItem ) &&
        ::osl::FileBase::E_None == aItem.getFileStatus( aStatus ) )
            aFileURL = aStatus.getFileURL();

    // open file
    /* SAFE { */
    SolarMutexGuard aGuard;
    css::uno::Reference< css::frame::XFrame >         xTargetFrame( m_xTargetFrame.get(), css::uno::UNO_QUERY );
    css::uno::Reference< css::util::XURLTransformer > xParser     ( css::util::URLTransformer::create(m_xContext) );

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
    /* } SAFE */
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
